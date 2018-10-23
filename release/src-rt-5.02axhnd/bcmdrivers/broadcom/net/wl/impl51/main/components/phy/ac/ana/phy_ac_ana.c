/*
 * ACPHY ANAcore control module implementation
 *
 * Copyright 2018 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: phy_ac_ana.c 742511 2018-01-22 14:14:24Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_ana.h>
#include "phy_type_ana.h"
#include <phy_ac_ana.h>
#include <phy_ac_antdiv.h>
#include <phy_ac_btcx.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>
#include <bcmdevs.h>
#include <phy_radio_api.h>

/**
 * Front End Control table contents for a single radio core. For some chips, the per-core tables are
 * not of equal size. A single table in PHYHW contains a 'subtable' per radio core. This structure
 * is not used in the case of 'sparse' tables.
 */
typedef struct {
	uint8 *subtable;	/* containing values to copy into PHY hardware table */
	uint32 n_entries;	/* number of entries in the subtable */
	uint32 hw_offset;	/* index to write to in the HW table */
} acphy_fe_ctrl_table_t;

/* module private states */
struct phy_ac_ana_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_ana_info_t *ani;
	acphy_fe_ctrl_table_t *fectrl_c; /* array of size PHY_CORE_MAX */
	uint16	*fectrl_idx, *fectrl_val;
	uint16	fectrl_table_len;
	uint16	fectrl_sparse_table_len;
	uint8	fectrl_spl_entry_flag;
	uint8	ldo3p3_2g; /* Program LDO value via NVRAM */
	uint8	ldo3p3_5g; /* Program LDO value via NVRAM */
};

/* local functions */
static int BCMATTACHFN(phy_ac_ana_srom_swctrlmap4_read)(phy_info_t *pi);
static bool BCMATTACHFN(wlc_phy_attach_femctrl_table)(phy_ac_ana_info_t *ani);
static int phy_ac_ana_switch(phy_type_ana_ctx_t *ctx, bool on);
static void phy_ac_ana_reset(phy_type_ana_ctx_t *ctx);

static const uint8 BCMATTACHDATA(fectrl_fem5516_fc1)[] =
{0, 0, 4, 0, 0, 0, 4, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
 4, 0, 0, 0, 4, 0, 0, 2, 0, 0, 0, 0, 0, 0};

/**
 * 43602 FEM table (fem subtype = 3,4) for core 0 (256 entries).
 * Used for e.g. 43602bu (3 ant no BT), 43602cd (X238, 3 Wifi + 1BT antenna) and 43602cs (X87, 3
 * antenna with shared BT) boards.
 */
static const uint8 BCMATTACHDATA(fectrl_fem5517_c0)[] =
{
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6, /* */
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6,
	 6,  6,  4,  6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  6,  6
};

/**
 * 43602 FEM table (fem subtype = 3,4) for core 1 (64 entries).
 * Used for 43602bu, 43602cd (X238, 4 antenna) and 43602cs (X87, 3 antenna) boards.
 */
static const uint8 BCMATTACHDATA(fectrl_fem5517_c1)[] =
{
	 2,  2,  0,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2, /* */
	66, 66, 64, 66, 66, 66, 66, 66, 66, 67, 66, 66, 66, 66, 66, 66,
	 2,  2,  0,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2,
	66, 66, 64, 66, 66, 66, 66, 66, 66, 67, 66, 66, 66, 66, 66, 66
};

/**
 * 43602 FEM table (fem subtype = 3,4) for core 2 (64 entries).
 * Used for 43602bu, 43602cd (X238, 4 antenna) and 43602cs (X87, 3 antenna) boards.
 * On the 43602, for core 2, bit3 in a table element steers the BAND_SEL pin (0=2.4G, 1=5G).
 */
static const uint8 BCMATTACHDATA(fectrl_fem5517_c2)[] =
{
	 2,  2,  0,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2, /* 2.4G */
	10, 10,  8, 10, 10, 10, 10, 10, 10, 11, 10, 10, 10, 10, 10, 10, /*   5G */
	 2,  2,  4,  2,  2,  2,  2,  2,  2,  3,  2,  2,  2,  2,  2,  2, /* 2.4G */
	10, 10,  8, 10, 10, 10, 10, 10, 10, 11, 10, 10, 10, 10, 10, 10  /*   5G */
};

/* 43602 FEM table for MC2 and MC5 (fem type = 6, subtype 0) for cores 0 (256 entries) */
/* and core 1,2 (first 64 entries) */
/* RFMD FEM part may be marked 4501, but inside is 4591 */
static const uint8 BCMATTACHDATA(fectrl_rfmd4591)[] =
{0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0};

static const uint8 BCMATTACHDATA(fectrl_x29c_c1_fc2_sub0)[] =
{0, 0, 0x50, 0x10, 0, 0, 0x50, 0x10, 0, 0x80, 0,
 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0};

static const uint8 BCMATTACHDATA(fectrl_x29c_c1_fc2_sub1)[] =
{0, 0, 0x30, 0x20, 0, 0, 0x30, 0x20, 0, 0x80, 0,
 0, 0, 0, 0, 0, 0x40, 0x40, 0x46, 0x42, 0x40, 0x40, 0x46, 0x42, 0x40, 0x41, 0x40,
 0x40, 0x40, 0x40, 0x40, 0x40};
static const uint8 BCMATTACHDATA(fectrl_femctrl2_sub2_c0)[] =
{6, 6, 4, 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 6, 6, 6, 4,
 6, 6, 6, 6, 6, 6, 7, 6, 6, 6, 6, 6, 6};
static const uint8 BCMATTACHDATA(fectrl_femctrl2_sub2_c12)[] =
{2, 2, 0, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 0,
 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2};

static const uint8 BCMATTACHDATA(fectrl_mch5_c0_p200_p400_fc3_sub0)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x02, 0x2c, 0x03, 0x2d, 0x02, 0x2c, 0x03, 0x2d};
static const uint8 BCMATTACHDATA(fectrl_mch5_c1_p200_p400_fc3_sub0)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 9, 6, 14, 2, 9, 6, 14, 0x02, 0x29, 0x06, 0x2d, 0x02, 0x29, 0x06, 0x2d};
static const uint8 BCMATTACHDATA(fectrl_mch5_c2_p200_p400_fc3_sub0)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 4, 9, 6, 14, 4, 9, 6, 14, 0x04, 0x29, 0x06, 0x2b, 0x04, 0x29, 0x06, 0x2b};

static const uint8 BCMATTACHDATA(fectrl_mch5_c0_fc3_sub1)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 8, 4, 3, 8, 8, 4, 3, 8, 0x08, 0x24, 0x03, 0x25, 0x08, 0x24, 0x03, 0x25};
static const uint8 BCMATTACHDATA(fectrl_mch5_c1_fc3_sub1)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 8, 1, 6, 8, 8, 1, 6, 8, 0x08, 0x21, 0x06, 0x25, 0x08, 0x21, 0x06, 0x25};
static const uint8 BCMATTACHDATA(fectrl_mch5_c2_fc3_sub1)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 8, 1, 6, 8, 8, 1, 6, 8, 0x08, 0x21, 0x06, 0x23, 0x08, 0x21, 0x06, 0x23};

static const uint8 BCMATTACHDATA(fectrl_j28_fc3_sub2)[] =
{2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23,
 0x25, 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25};

static const uint8 BCMATTACHDATA(fectrl3_sub3_c0)[] =
{2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23,
 0x25, 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25};
static const uint8 BCMATTACHDATA(fectrl3_sub3_c1)[] =
{2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26,
 0x25, 2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25};
static const uint8 BCMATTACHDATA(fectrl3_sub3_c2)[] =
{4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26,
 0x23, 4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23};

static const uint8 BCMATTACHDATA(fectrl_43602_mch5_c0)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 12, 3, 11, 2, 12, 3, 11, 0x2, 0x2c, 0x3, 0x2d, 0x2, 0x2c, 0x3, 0x2d};
static const uint8 BCMATTACHDATA(fectrl_43602_mch5_c1)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 9, 6, 14, 2, 9, 6, 14, 0x2, 0x29, 0x6, 0x2d, 0x2, 0x29, 0x6, 0x2d,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 9, 6, 14, 2, 9, 6, 14, 0x2, 0x29, 0x6, 0x2d, 0x2, 0x29, 0x6, 0x2d};
static const uint8 BCMATTACHDATA(fectrl_43602_mch5_c2)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 4, 9, 6, 14, 4, 9, 6, 14, 0x4, 0x29, 0x6, 0x2b, 0x4, 0x29, 0x6, 0x2b,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 4, 9, 6, 14, 4, 9, 6, 14, 0x4, 0x29, 0x6, 0x2b, 0x4, 0x29, 0x6, 0x2b};

/* 43602 MCH2, PAVREF enabled PAs */
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_c0)[] =
{2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25,
 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25};
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_c1)[] =
{2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25,
 2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25,
 2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25,
 2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25};
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_c2)[] =
{4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23,
 4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23,
 4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23,
 4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23};

/* 43602 MCH2, digitally enabled PAs */
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_1_c0)[] =
{2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13,
 2, 4, 3, 2, 2, 4, 3, 2, 10, 12, 11, 13, 10, 12, 11, 13};
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_1_c1)[] =
{2, 1, 6, 2, 2, 1, 6, 2, 10, 9, 14, 13, 10, 9, 14, 13,
 2, 1, 6, 2, 2, 1, 6, 2, 10, 9, 14, 13, 10, 9, 14, 13,
 2, 1, 6, 2, 2, 1, 6, 2, 10, 9, 14, 13, 10, 9, 14, 13,
 2, 1, 6, 2, 2, 1, 6, 2, 10, 9, 14, 13, 10, 9, 14, 13};
static const uint8 BCMATTACHDATA(fectrl_43602_mch2_1_c2)[] =
{4, 1, 6, 4, 4, 1, 6, 4, 12, 9, 14, 11, 12, 9, 14, 11,
 4, 1, 6, 4, 4, 1, 6, 4, 12, 9, 14, 11, 12, 9, 14, 11,
 4, 1, 6, 4, 4, 1, 6, 4, 12, 9, 14, 11, 12, 9, 14, 11,
 4, 1, 6, 4, 4, 1, 6, 4, 12, 9, 14, 11, 12, 9, 14, 11};

static const uint8 BCMATTACHDATA(fectrl3_sub6_43602)[] =
{2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2,
 2, 2, 1, 2, 2, 2, 2, 2, 42, 44, 2, 2, 2, 2, 2, 2};

static const uint8 BCMATTACHDATA(fectrl3_sub7_c0_4360)[] =
{2, 0, 3, 2, 2, 0, 3, 2, 0, 0x24, 0, 0, 0, 0x24, 0, 0,
 2, 0, 3, 2, 2, 0, 3, 2, 0, 0x24, 0, 0, 0, 0x24, 0, 0};
static const uint8 BCMATTACHDATA(fectrl3_sub7_c1_4360)[] =
{2, 0, 6, 2, 2, 0, 6, 2, 0, 0x21, 0, 0, 0, 0x21, 0, 0,
 2, 0, 6, 2, 2, 0, 6, 2, 0, 0x21, 0, 0, 0, 0x21, 0, 0};
static const uint8 BCMATTACHDATA(fectrl3_sub7_c2_4360)[] =
{4, 0, 6, 4, 4, 0, 6, 4, 0, 0x21, 0, 0, 0, 0x21, 0, 0,
 4, 0, 6, 4, 4, 0, 6, 4, 0, 0x21, 0, 0, 0, 0x21, 0, 0};

/* FEMCTRL LUT for SKY 85309/Qorvo QFP4219 :- This LUT is levereged from R8000
 * which used 43602 + 85309 (Same Truth table)
 */
static const uint8 BCMATTACHDATA(fectrl_fem85309)[] =
{0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0,
 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0};
static const uint8 BCMATTACHDATA(fectrl6_sub1_43602)[] =
{0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0,
 0, 0, 2, 6, 0, 0, 2, 6, 0, 5, 0, 0, 0, 0, 0, 0};
static const uint8 BCMATTACHDATA(fectrl6_sub2_43602)[] =
{0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 2, 6, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0};
static const uint8 BCMATTACHDATA(fectrl1_sub5_43602_c0)[] =
{0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14,
0, 4, 2, 6, 0, 4, 2, 6, 14, 12, 10, 14, 14, 12, 10, 14};
static const uint8 BCMATTACHDATA(fectrl1_sub5_43602_c1)[] =
{0, 1, 2, 3, 0, 1, 2, 3, 11, 9, 10, 11, 11, 9, 10, 11,
0, 1, 2, 3, 0, 1, 2, 3, 11, 9, 10, 11, 11, 9, 10, 11,
0, 1, 2, 3, 0, 1, 2, 3, 11, 9, 10, 11, 11, 9, 10, 11,
0, 1, 2, 3, 0, 1, 2, 3, 11, 9, 10, 11, 11, 9, 10, 11};
static const uint8 BCMATTACHDATA(fectrl1_sub5_43602_c2)[] =
{0, 1, 4, 5, 0, 1, 4, 5, 13, 9, 12, 13, 13, 9, 12, 13,
0, 1, 4, 5, 0, 1, 4, 5, 13, 9, 12, 13, 13, 9, 12, 13,
0, 1, 4, 5, 0, 1, 4, 5, 13, 9, 12, 13, 13, 9, 12, 13,
0, 1, 4, 5, 0, 1, 4, 5, 13, 9, 12, 13, 13, 9, 12, 13};

static const uint8 BCMATTACHDATA(fectrl_femctrl5_c1)[] =
{0, 0, 0x50, 0x40, 0, 0, 0x50, 0x40, 0, 0x20, 0, 0, 0, 0, 0,
 0, 0x80, 0x80, 0x86, 0x82, 0x80, 0x80, 0x86, 0x82, 0x80, 0x81, 0x80, 0x80, 0x80,
 0x80, 0x80, 0x80};
static const uint8 BCMATTACHDATA(fectrl_zeros)[] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8 BCMATTACHDATA(fectrl_femctrl6)[] =
{0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0,
 0, 0, 6, 2, 0, 0, 6, 2, 0, 1, 0, 0, 0, 0, 0, 0};
static const sparse_array_entry_t BCMATTACHDATA(fectrl_fcbga_epa_elna_fc4_sub0)[] =
{{2, 264}, {3, 8}, {9, 32}, {18, 5}, {19, 4}, {25, 128}, {130, 64}, {192, 64}};

static const sparse_array_entry_t BCMATTACHDATA(fectrl_wlbga_epa_elna_fc4_sub1)[] =
{{2, 3}, {3, 1}, {9, 256}, {18, 20}, {19, 16}, {25, 8}, {66, 3}, {67, 1},
 {73, 256}, {82, 20}, {83, 16}, {89, 8}, {128, 3}, {129, 1}, {130, 3}, {131, 1},
 {132, 1}, {133, 1}, {134, 1}, {135, 1}, {136, 3}, {137, 1}, {138, 3}, {139, 1},
 {140, 1}, {141, 1}, {142, 1}, {143, 1}, {160, 3}, {161, 1}, {162, 3}, {163, 1},
 {164, 1}, {165, 1}, {166, 1}, {167, 1}, {168, 3}, {169, 1}, {170, 3}, {171, 1},
 {172, 1}, {173, 1}, {174, 1}, {175, 1}, {192, 128}, {193, 128}, {196, 128}, {197, 128},
 {200, 128}, {201, 128}, {204, 128}, {205, 128}, {224, 128}, {225, 128}, {228, 128},
 {229, 128}, {232, 128}, {233, 128}, {236, 128}, {237, 128} };

static const sparse_array_entry_t BCMATTACHDATA(fectrl_fchm_epa_elna_fc4_sub2)[] =
{{2, 280}, {3, 24}, {9, 48}, {18, 21}, {19, 20}, {25, 144}, {34, 776}, {35, 520},
 {41, 544}, {50, 517}, {51, 516}, {57, 640}, {66, 280}, {67, 24}, {73, 48}, {82, 21},
 {83, 20}, {89, 144}, {98, 776}, {99, 520}, {105, 544}, {114, 517}, {115, 516}, {121, 640},
 {128, 280}, {129, 24}, {130, 280}, {131, 24}, {132, 24}, {133, 24}, {134, 24}, {135, 24},
 {136, 280}, {137, 24}, {138, 280}, {139, 24}, {140, 24}, {141, 24}, {142, 24}, {143, 24},
 {160, 776}, {161, 520}, {162, 776}, {163, 520}, {164, 520}, {165, 520}, {166, 520},
 {167, 520}, {168, 776}, {169, 520}, {170, 776}, {171, 520}, {172, 520}, {173, 520},
 {174, 520}, {175, 520},	{192, 16}, {193, 16}, {196, 16}, {197, 16}, {200, 16}, {201, 16},
 {204, 16}, {205, 16}, {224, 512}, {225, 512}, {228, 512}, {229, 512}, {232, 512},
 {233, 512}, {236, 512}, {237, 512}};

static const sparse_array_entry_t BCMATTACHDATA(fectrl_wlcsp_epa_elna_fc4_sub34)[] =
{{2, 34}, {3, 2}, {9, 17}, {18, 80}, {19, 16}, {25, 8}, {66, 34}, {67, 2},
 {73, 1}, {82, 80}, {83, 16}, {89, 8}, {128, 34}, {129, 2}, {130, 34}, {131, 2},
 {132, 2}, {133, 2}, {134, 2}, {135, 2}, {136, 34}, {137, 2}, {138, 34}, {139, 2},
 {140, 2}, {141, 2}, {142, 2}, {143, 2}, {160, 34}, {161, 2}, {162, 34}, {163, 2},
 {164, 2}, {165, 2}, {166, 2}, {167, 2}, {168, 34}, {169, 2}, {170, 34}, {171, 2},
 {172, 2}, {173, 2}, {174, 2}, {175, 2}, {192, 4}, {193, 4}, {196, 4}, {197, 4},
 {200, 4}, {201, 4}, {204, 4}, {205, 4}, {224, 4}, {225, 4}, {228, 4}, {229, 4},
 {232, 4}, {233, 4}, {236, 4}, {237, 4} };

static const sparse_array_entry_t BCMATTACHDATA(fectrl_fp_dpdt_epa_elna_fc4_sub5)[] =
{{2, 280}, {3, 24}, {9, 48}, {18, 21}, {19, 20}, {25, 144}, {34, 776},
 {35, 520}, {41, 544}, {50, 517}, {51, 516}, {57, 640}, {130, 80},
 {192, 80}};

static const sparse_array_entry_t BCMATTACHDATA(fectrl_43162_fcbga_ipa_ilna_fc4_sub6)[] =
{{2, 10}, {3, 2}, {9, 2}, {18, 4}, {19, 12}, {25, 12}, {34, 1}, {35, 8},
 {41, 8}, {50, 6}, {51, 14}, {57, 14}, {66, 10}, {67, 2}, {73, 2},
 {82, 4}, {83, 12}, {89, 12}, {98, 9}, {99, 3}, {105, 3}, {114, 6},
 {115, 14}, {121, 14}, {128, 11}, {129, 11}, {130, 11}, {131, 11},
 {132, 11}, {133, 11}, {134, 11}, {135, 11}, {136, 11}, {137, 11},
 {138, 11}, {139, 11}, {140, 11}, {141, 11}, {142, 11}, {143, 11},
 {146, 5}, {147, 13}, {153, 13}, {178, 7}, {179, 15}, {185, 15}, {192, 9},
 {193, 9}, {196, 9}, {197, 9}, {200, 3}, {201, 3}, {204, 3}, {205, 3},
 {210, 4}, {211, 12}, {217, 12}, {242, 6}, {243, 14}, {249, 14}};

static const sparse_array_entry_t BCMATTACHDATA(fectrl_43162_fcbga_ipa_elna_fc4_sub7)[] =
{{2, 26}, {3, 10}, {9, 2}, {18, 36}, {19, 4}, {25, 12}, {34, 17}, {35, 1},
 {41, 8}, {50, 38}, {51, 6}, {57, 14}, {66, 26}, {67, 10}, {73, 2},
 {82, 36}, {83, 4}, {89, 12}, {98, 25}, {99, 9}, {105, 3}, {114, 38},
 {115, 6}, {121, 14}, {128, 27}, {129, 27}, {130, 27}, {131, 27},
 {132, 27}, {133, 27}, {134, 27}, {135, 27}, {136, 11}, {137, 11},
 {138, 11}, {139, 11}, {140, 11}, {141, 11}, {142, 11}, {143, 11},
 {146, 37}, {147, 5}, {153, 13}, {178, 39}, {179, 7}, {185, 15}, {192, 25},
 {193, 25}, {196, 25}, {197, 25}, {200, 3}, {201, 3}, {204, 3}, {205, 3},
 {210, 36}, {211, 4}, {217, 12}, {242, 38}, {243, 6}, {249, 14}};

static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_WLBGA_add_fc7)[] =
{18, 50, 80, 112, 129, 130, 131, 134, 135, 137,
 145, 146, 147, 150, 151, 153, 161, 162, 163, 166, 167, 169, 177, 178, 179, 182,
 183, 185};
static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_WLBGA_fc7)[] =
{66, 34, 68, 36, 72, 64, 72, 64, 72,
 72, 65, 66, 65,
 66, 65, 65, 40,
 32, 40, 32, 40,
 40, 33, 34, 33,
 34, 33, 33};
static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_WLBGA_iPa_add_fc7)[] =
{2, 3, 9, 18, 19, 25, 34, 35, 41, 50, 51, 57};
static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_WLBGA_iPa_fc7)[] =
{66, 65, 65, 80, 72, 72, 34, 33, 33, 48, 40, 40};

static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_FCBGA_add_fc8)[] =
{161, 162, 163, 166, 167, 169, 177, 178, 179,
 182, 183, 185};
static const uint16 BCMATTACHDATA(fectrl_femctrl_4335_FCBGA_fc8)[] =
{9, 12, 9, 12, 9, 9, 65, 9, 65, 9, 65, 65};

static const uint16 BCMATTACHDATA(fectrl_fcbgabu_epa_elna_idx_fc9)[] =
{2, 3, 9, 18, 19, 25, 130, 192};
static const uint16 BCMATTACHDATA(fectrl_fcbgabu_epa_elna_val_fc9)[] =
{128, 0, 4, 64, 0, 3, 8, 8};

static const uint16 BCMATTACHDATA(fectrl_fcbga_epa_elna_idx_fc10_sub0)[] =
{  2,   3,   9,  18,  19,  25,  66,  67,  73,  82,  83, 128, 129, 130, 131, 132, 133,
 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 160, 161, 162, 163, 164, 165, 166,
 167, 168, 169, 170, 171, 172, 173, 174, 175, 192, 193, 196, 197, 200, 201, 204, 205,
 210, 211, 224, 225, 228, 229, 232, 233, 236, 237, 258, 259, 265, 274, 275, 281};
static const uint16 BCMATTACHDATA(fectrl_fcbga_epa_elna_val_fc10_sub0)[] =
{ 96,  96,   8,   6,   2,   1,  96,  32,   8,   6,   2,  96,  96,  96,  96,  96,  96,
  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,  96,
  96,  96,  96,  96,  96,  96,  96,  96,  96, 128, 128, 128, 128, 128, 128, 128, 128,
 134, 130, 128, 128, 128, 128, 128, 128, 128, 128,   5,   4,   8,  48,  32,  64};

static const uint16 BCMATTACHDATA(fectrl_wlbga_epa_elna_idx_fc10_sub1)[] =
{2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
 192, 193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
static const uint16 BCMATTACHDATA(fectrl_wlbga_epa_elna_val_fc10_sub1)[] =
{48, 32, 8, 6, 2, 1, 48, 32, 8, 6, 2, 48, 32,
 48, 32, 32, 32, 32, 32, 48, 32, 48, 32, 32, 32, 32, 32, 128, 128, 128, 128,
 128, 128, 128, 128, 134, 130, 48, 32, 8, 6, 2, 1};

static const uint16 BCMATTACHDATA(fectrl_wlbga_ipa_ilna_idx_fc10_sub2)[] =
{2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128, 130, 132, 134, 192, 210, 211, 258, 259, 265, 274,
 275, 281};
static const uint16 BCMATTACHDATA(fectrl_wlbga_ipa_ilna_val_fc10_sub2)[] =
{16, 64, 64, 1, 2, 2, 16, 64, 64, 1, 2, 16, 16, 16, 16, 32, 33, 34, 64, 16, 16, 1, 2, 2};
static const uint16 BCMATTACHDATA(fectrl_43556usb_epa_elna_idx_fc10_sub3)[] =
{2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 192,
 193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
static const uint16 BCMATTACHDATA(fectrl_43556usb_epa_elna_val_fc10_sub3)[] =
{96, 32, 8, 6, 2, 1, 96, 32, 8, 6, 2, 96, 32, 96,
 32, 32, 32, 32, 32, 96, 32, 96, 32, 32, 32, 32, 32, 128, 128, 128, 128, 128, 128,
 128, 128, 134, 130, 5, 4, 8, 48, 32, 64};
static const uint16 BCMATTACHDATA(fectrl_fcbga_ipa_ilna_idx_fc10_sub4)[] =
{2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128, 129,
 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 192, 193,
 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 265, 274, 275, 281, 281};
static const uint16 BCMATTACHDATA(fectrl_fcbga_ipa_ilna_val_fc10_sub4)[] =
{128, 32, 32, 8, 1, 1, 128, 32, 32, 8, 8, 128, 32,
 128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 64, 64, 64, 64,
 64, 64, 64, 64, 72, 72, 4, 8, 8, 8, 64, 16, 16, 16};

static const uint16 BCMATTACHDATA(fectrl_idx_fc10_sub5)[] =
{   2,   3,   9,  18,  19,  25,  34,  35,  41,  50,  51,  57,  66,  67,  73,
   82,  83,  89,  98,  99, 105, 114, 115, 121, 128, 129, 130, 131, 132, 133,
  134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 160, 161, 162, 163, 164,
  165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 192, 193, 194, 195,
  196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 224, 225, 226,
  227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 258, 259,
  265, 274, 275, 281, 290, 291, 297, 306, 307, 313};
static const uint16 BCMATTACHDATA(fectrl_val_fc10_sub5)[] =
{  32,  40,  24,   2,   3,   5,  32,  40,  24,   2,   3,   5,  32,  40,  24,
    2,   3,   5,  32,  40,  24,   2,   3,   5,  32,  32,  32,  32,  40,  40,
   40,  40,  32,  32,  32,  32,  40,  40,  40,  40,  32,  32,  32,  32,  40,
   40,  40,  40,  32,  32,  32,  32,  40,  40,  40,  40,  56,  56,  32,  32,
   56,  56,  40,  40,  56,  56,  32,  32,  56,  56,  40,  40,  56,  56,  32,
   32,  56,  56,  40,  40,  56,  56,  32,  32,  56,  56,  40,  40,   8,  40,
   48, 128, 192,  68,   8,  40,  48, 128, 192,  68};

static const uint16 BCMATTACHDATA(fectrl_fcbu_epa_idx_fc11)[] =
{2, 3, 9, 18, 19, 25, 130, 192,
 258, 259, 265, 274, 275, 281, 386, 448,
 514, 515, 521, 530, 531, 537, 642, 704};
static const uint16 BCMATTACHDATA(fectrl_fcbu_epa_val_fc11)[] =
{10, 10, 4, 96, 96, 16, 0, 0,
 10, 10, 4, 96, 96, 16, 0, 0,
 10, 10, 4, 96, 96, 16, 0, 0};
#ifdef PHYWAR_43012_HW43012_211_RF_SW_CTRL
static const uint16 BCMATTACHDATA(fectrl_fem43012_idx_BCM43012_fcbga)[] =
{2, 3, 9, 18, 19, 25, 34, 35, 41, 50, 51, 57, 130, 146, 147, 153, 162,
178, 179, 185, 192, 210, 211, 217, 224, 242, 243, 249};
static const uint16 BCMATTACHDATA(fectrl_fem43012_val_BCM43012_fcbga)[] =
{9, 42, 42, 72, 8, 268, 137, 170, 170, 200, 136, 396, 8, 72, 8, 268, 0,
200, 136, 396, 8, 72, 8, 268, 0, 200, 136, 396};
#else
static const uint16 BCMATTACHDATA(fectrl_fem43012_idx_BCM43012_fcbga)[] =
{ 2, 3, 9, 18, 19, 25, 34, 35, 41, 50, 51, 57, 130, 146, 147, 153, 162,
178, 179, 185, 192, 210, 211, 217, 224, 242, 243, 249};
static const uint16 BCMATTACHDATA(fectrl_fem43012_val_BCM43012_fcbga)[] =
{ 11, 41, 45, 73, 9, 281, 138, 168, 172, 200, 136, 408, 8, 73, 9, 281, 1,
200, 136, 408, 8, 73, 9, 281, 1, 200, 136, 408};
#endif /* PHYWAR_43012_HW43012_211_RF_SW_CTRL */
static const uint16 BCMATTACHDATA(fectrl_fem43012_idx_BCM43012_wlbga)[] =
{ 2, 3, 9, 18, 19, 25, 34, 35, 41, 50, 51, 57, 130, 146, 147, 153, 162, 178,
179, 185, 192, 210, 211, 217, 224, 242, 243, 249};
static const uint16 BCMATTACHDATA(fectrl_fem43012_val_BCM43012_wlbga)[] =
{ 64, 32, 48, 4, 0, 9, 64, 32, 48, 4, 0, 9, 0, 4, 0, 9, 0, 4, 0, 9, 0, 4, 0,
9, 0, 4, 0, 9};

/* local functions */
static void phy_ac_ana_nvram_attach(phy_ac_ana_info_t *anai);

/* Register/unregister ACPHY specific implementation to common layer. */
phy_ac_ana_info_t *
BCMATTACHFN(phy_ac_ana_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_ana_info_t *ani)
{
	phy_ac_ana_info_t *info;
	phy_type_ana_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_ac_ana_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	info->pi = pi;
	info->aci = aci;
	info->ani = ani;

	/* Read srom params from nvram */
	phy_ac_ana_nvram_attach(info);

	/* function to read femctrl params from nvram */
	if (!phy_ac_ana_srom_swctrlmap4_read(pi))
		goto fail;

	if (ACPHY_FEMCTRL_ACTIVE(pi) && !ACPHY_SWCTRLMAP4_EN(pi) &&
		!wlc_phy_attach_femctrl_table(info))
		goto fail;

	phy_ac_ana_switch(info, ON);

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctrl = phy_ac_ana_switch;
	fns.reset = phy_ac_ana_reset;
	fns.ctx = info;
#ifndef BOARD_FLAGS
	BF_ELNA_2G(aci) = FALSE;
	BF_ELNA_5G(aci) = FALSE;
#endif /* BOARD_FLAGS */

	phy_ana_register_impl(ani, &fns);

	return info;
fail:
	phy_ac_ana_unregister_impl(info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_ana_unregister_impl)(phy_ac_ana_info_t *info)
{
	phy_info_t *pi;
	phy_ana_info_t *ani;
	uint8 core = 0;

	if (info == NULL) {
		return;
	}
	pi = info->pi;
	ani = info->ani;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_ana_unregister_impl(ani);

	if (info->fectrl_c != NULL) {
		FOREACH_CORE(pi, core) {
			if (info->fectrl_c[core].subtable != NULL) {
				phy_mfree(pi, info->fectrl_c[core].subtable,
					info->fectrl_c[core].n_entries * sizeof(uint8));
			}
		}
		phy_mfree(pi, info->fectrl_c, sizeof(acphy_fe_ctrl_table_t[PHY_CORE_MAX]));
	}

	if (info->fectrl_idx != NULL) {
		phy_mfree(pi, info->fectrl_idx, info->fectrl_sparse_table_len * sizeof(uint16));
	}
	if (info->fectrl_val != NULL) {
		phy_mfree(pi, info->fectrl_val, info->fectrl_sparse_table_len * sizeof(uint16));
	}
	phy_mfree(pi, info, sizeof(phy_ac_ana_info_t));
}

/* switch anacore on/off */
static int
phy_ac_ana_switch(phy_type_ana_ctx_t *ctx, bool on)
{
	phy_ac_ana_info_t *info = (phy_ac_ana_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: %d\n", __FUNCTION__, on));

	if (on)
		W_REG(pi->sh->osh, D11_PHY_REG_3(pi), 0x0);
	else
		W_REG(pi->sh->osh, D11_PHY_REG_3(pi), 0xF4);

	return BCME_OK;
}

/* reset h/w */
static void
phy_ac_ana_reset(phy_type_ana_ctx_t *ctx)
{
	phy_ac_ana_switch(ctx, ON);
}

static void
BCMATTACHFN(phy_ac_ana_nvram_attach)(phy_ac_ana_info_t *anai)
{
	phy_info_t *pi = anai->pi;
#ifndef BOARD_FLAGS3
	uint32 bfl3; /* boardflags3 */
#endif // endif
#if !defined(BOARD_FLAGS3) || !defined(FEMCTRL)
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
#endif /* !defined(FEMCTRL) || !defined(BOARD_FLAGS3) */

	anai->ldo3p3_2g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_ldo3p3_2g, 0);
	anai->ldo3p3_5g = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_ldo3p3_5g, 0);

#ifndef FEMCTRL
	BFCTL(pi_ac) = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_femctrl, 0);
#endif /* FEMCTRL */

#ifndef BOARD_FLAGS3
	if ((PHY_GETVAR_SLICE(pi, rstr_boardflags3)) != NULL) {
		bfl3 = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_boardflags3);
		BF3_FEMCTRL_SUB(pi_ac) = bfl3 & BFL3_FEMCTRL_SUB;
		BF3_AGC_CFG_2G(pi_ac) = ((bfl3 & BFL3_AGC_CFG_2G) != 0);
		BF3_AGC_CFG_5G(pi_ac) = ((bfl3 & BFL3_AGC_CFG_5G) != 0);
	} else {
		BF3_FEMCTRL_SUB(pi_ac) = 0;
		BF3_AGC_CFG_2G(pi_ac) = 0;
		BF3_AGC_CFG_5G(pi_ac) = 0;
	}
#endif /* BOARD_FLAGS3 */
}
/* ********************************************* */
/*				External Functions					*/
/* ********************************************* */
void
wlc_tiny_dc_static_WAR(phy_info_t *pi)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		if (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_clk_slow_pu, 0);
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_sipo_pu, 0);

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 0);
				} else {
					MOD_RADIO_REG_20691(pi, RX_TOP_2G_OVR_NORTH, core,
						ovr_rxmix2g_pu, 0);
				}
			}
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		} else {
			MOD_RADIO_REG_TINY(pi, ADC_CFG15, core, adc_clk_slow_pu, 1);
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_clk_slow_pu, 1);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_TINY(pi, RXMIX2G_CFG1, core, rxmix2g_pu, 1);
				if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
					MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 1);
				} else {
					MOD_RADIO_REG_20691(pi, RX_TOP_2G_OVR_NORTH, core,
						ovr_rxmix2g_pu, 1);
				}
				MOD_RADIO_REG_TINY(pi, ADC_CFG1, core, adc_sipo_pu, 1);
				MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_sipo_pu, 1);
			}
		}
	}

	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_TX2RX);
}

void
wlc_phy_init_adc_read(phy_info_t* pi, uint16* save_afePuCtrl, uint16* save_gpio,
                      uint32* save_chipc, uint16* fval2g_orig, uint16* fval5g_orig,
                      uint16* fval2g, uint16* fval5g, uint8* stall_val,
                      uint16* save_gpioHiOutEn)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	*stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	*save_afePuCtrl = READ_PHYREGFLD(pi, AfePuCtrl, tssiSleepEn);
	MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
	*save_gpio = READ_PHYREG(pi, gpioSel);
	*save_gpioHiOutEn = READ_PHYREG(pi, gpioHiOutEn);

	if (phy_ac_btcx_get_data(pi_ac->btcxi)->poll_adc_WAR) {
		ACPHY_REG_LIST_START
			ACPHY_DISABLE_STALL_ENTRY(pi)

			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_2g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_5g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, override_ext_pa, 1)
		ACPHY_REG_LIST_EXECUTE(pi);

		*save_chipc = si_corereg(pi->sh->sih, SI_CC_IDX,
		                         OFFSETOF(chipcregs_t, chipcontrol), 0, 0);
		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           0xffffffff, CCTRL4360_EXTRA_FEMCTRL_MODE);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g_orig);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g_orig);

		*fval2g = (*fval2g_orig & 0xf0) << 1;
		*fval5g = (*fval5g_orig & 0xf);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g);

		ACPHY_REG_LIST_START
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_2g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_5g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, override_ext_pa, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
		ACPHY_ENABLE_STALL(pi, *stall_val);
	}
}

void
wlc_phy_restore_after_adc_read(phy_info_t *pi, uint16* save_afePuCtrl, uint16 *save_gpio,
                               uint32* save_chipc, uint16* fval2g_orig, uint16* fval5g_orig,
                               uint16* fval2g, uint16* fval5g, uint8* stall_val,
                               uint16* save_gpioHiOutEn)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	ACPHY_DISABLE_STALL(pi);
	WRITE_PHYREG(pi, gpioSel, *save_gpio);
	WRITE_PHYREG(pi, gpioHiOutEn, *save_gpioHiOutEn);
	if (phy_ac_btcx_get_data(pi_ac->btcxi)->poll_adc_WAR) {
		ACPHY_REG_LIST_START
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_2g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_5g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, override_ext_pa, 1)
		ACPHY_REG_LIST_EXECUTE(pi);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 41, 16, fval2g_orig);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, 57, 16, fval5g_orig);

		si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		           0xffffffff, *save_chipc);

		ACPHY_REG_LIST_START
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_2g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, ext_5g_papu, 0)
			MOD_PHYREGCE_ENTRY(pi, RfctrlIntc, 1, override_ext_pa, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, *save_afePuCtrl);

	ACPHY_ENABLE_STALL(pi, *stall_val);
}

void
wlc_phy_pulse_adc_reset_acphy(phy_info_t *pi)
{
	uint8 core;

	struct _reg_vals {
		uint16 regval;
		uint16 regaddr;
	} cur_reg_val[PHY_CORE_MAX*3]; /* save 3 register values for each core */

	uint core_count = 0;

	/* Set clamp using rfctrl override */
	FOREACH_CORE(pi, core) {
		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlCoreAfeCfg1, core);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, cur_reg_val[core_count].regval |
			ACPHY_RfctrlCoreAfeCfg10_afe_iqadc_reset_MASK(pi->pubpi->phy_rev));
		++core_count;

		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlCoreAfeCfg2, core);
		WRITE_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, cur_reg_val[core_count].regval |
		        ACPHY_RfctrlCoreAfeCfg20_afe_iqadc_clamp_en_MASK(pi->pubpi->phy_rev));
		++core_count;

		cur_reg_val[core_count].regval = READ_PHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		cur_reg_val[core_count].regaddr = ACPHYREGCE(pi, RfctrlOverrideAfeCfg, core);
		WRITE_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, cur_reg_val[core_count].regval |
			ACPHY_RfctrlOverrideAfeCfg0_afe_iqadc_clamp_en_MASK(pi->pubpi->phy_rev) |
			ACPHY_RfctrlOverrideAfeCfg0_afe_iqadc_reset_MASK(pi->pubpi->phy_rev));
		++core_count;
	}

	/* Wait for 1 us */
	OSL_DELAY(1);

	/* Restore values */
	while (core_count > 0) {
		--core_count;
		phy_utils_write_phyreg(pi, cur_reg_val[core_count].regaddr,
		                       cur_reg_val[core_count].regval);
	}
	/* Wait for 1 us */
	OSL_DELAY(1);
}

static bool
BCMATTACHFN(wlc_phy_attach_femctrl_table)(phy_ac_ana_info_t *ani)
{
	phy_info_t *pi = ani->pi;
	phy_ac_info_t *pi_ac = ani->aci;
	const uint16 *fectrl_idx = NULL, *fectrl_val = NULL;
	const uint8 *fectrl_c[3] = {NULL, NULL, NULL};
	uint16 table_len = 0, sparse_table_len = 0;
	const sparse_array_entry_t *fe_ctrl_tbl;
	uint16 kk;
	int core;
	acphy_fe_ctrl_table_t *p_core; /* FEM control data per core for non-sparse tables */

	BCM_REFERENCE(pi_ac);

	ani->fectrl_idx = NULL;
	ani->fectrl_val = NULL;
	ani->fectrl_table_len = 0;
	ani->fectrl_sparse_table_len = 0;
	ani->fectrl_spl_entry_flag = 0;

	if ((ani->fectrl_c = phy_malloc(pi, sizeof(acphy_fe_ctrl_table_t[PHY_CORE_MAX]))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			pi->sh->unit, __FUNCTION__, MALLOCED(pi->sh->osh)));
		return FALSE;
	}
	/* majorrev0 chips don't use sparse tables: they have their entire femctrl table */
	FOREACH_CORE(pi, core) {
		ani->fectrl_c[core].subtable = NULL;
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		table_len = 256;
	} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		const uint32 n_entries_4360[] =  {32, 32, 32};
		const uint32 hw_offset_4360[] =  {0,  32, 64};
		const uint32 n_entries_43602[] = {256, 64, 64};
		const uint32 hw_offset_43602[] = {0, 256, 512};
		const uint32 *n_entries;
		const uint32 *hw_offset;

		if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
			n_entries = n_entries_43602;
			hw_offset = hw_offset_43602;
		} else {
			n_entries = n_entries_4360;
			hw_offset = hw_offset_4360;
		}

		FOREACH_CORE(pi, core) {
			p_core = &ani->fectrl_c[core];
			p_core->n_entries = n_entries[core];
			p_core->hw_offset = hw_offset[core];
			if ((p_core->subtable =
			     phy_malloc(pi, p_core->n_entries * sizeof(uint8))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					pi->sh->unit,
					__FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		table_len = 320;
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		switch (BFCTL(pi_ac)) {
			default:
			case 17:
				table_len = 256;
				switch (BF3_FEMCTRL_SUB(pi_ac)) {
					case FCBGA_43012:
					sparse_table_len =
						ARRAYSIZE(fectrl_fem43012_idx_BCM43012_fcbga);
						break;
					case WLBGA_43012 :
					sparse_table_len =
						ARRAYSIZE(fectrl_fem43012_idx_BCM43012_wlbga);
						break;
					default:
					PHY_ERROR(("wl%d: %s: Provide correct femctrl_sub \n",
						pi->sh->unit, __FUNCTION__));
						ASSERT(0);
				}
				/* malloc table */
				if ((ani->fectrl_idx =
				     phy_malloc(pi,
				            sparse_table_len * sizeof(uint16))) == NULL) {
					PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
						pi->sh->unit,
					           __FUNCTION__, MALLOCED(pi->sh->osh)));
					return FALSE;
				}
				if ((ani->fectrl_val =
				     phy_malloc(pi,
				            sparse_table_len * sizeof(uint16))) == NULL) {
					PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					           pi->sh->unit,
					           __FUNCTION__, MALLOCED(pi->sh->osh)));
					return FALSE;
				}
				switch (BF3_FEMCTRL_SUB(pi_ac)) {
					case FCBGA_43012:
						fectrl_idx = fectrl_fem43012_idx_BCM43012_fcbga;
						fectrl_val = fectrl_fem43012_val_BCM43012_fcbga;
						break;
					case WLBGA_43012 :
						fectrl_idx = fectrl_fem43012_idx_BCM43012_wlbga;
						fectrl_val = fectrl_fem43012_val_BCM43012_wlbga;
						break;
					default:
					PHY_ERROR(("wl%d: %s: Provide correct femctrl_sub \n",
						pi->sh->unit, __FUNCTION__));
						ASSERT(0);
				}
				memcpy(ani->fectrl_idx, fectrl_idx,
				       sparse_table_len* sizeof(uint16));
				memcpy(ani->fectrl_val, fectrl_val,
				       sparse_table_len* sizeof(uint16));
				ani->fectrl_sparse_table_len = sparse_table_len;
				ani->fectrl_table_len = table_len;
				break;
		}
	}

	if (ACPHY_FEMCTRL_ACTIVE(pi) && (!(ACMAJORREV_36(pi->pubpi->phy_rev)))) {
		switch (BFCTL(pi_ac)) {
		case 0:
			/* Chip default, do nothing */
			break;
		default:
			/* same as 1 */

		case 1:
			fectrl_c[0] = fectrl_fem5516_fc1;
			fectrl_c[1] = fectrl_fem5516_fc1;
			fectrl_c[2] = fectrl_fem5516_fc1;
			break;
		case 2:	/* 4360 + 43602 chips */
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				fectrl_c[0] = fectrl_fem5516_fc1;
				fectrl_c[1] = fectrl_x29c_c1_fc2_sub0;
				fectrl_c[2] = fectrl_fem5516_fc1;
				break;
			case 1:
				fectrl_c[0] = fectrl_fem5516_fc1;
				fectrl_c[1] = fectrl_x29c_c1_fc2_sub1;
				fectrl_c[2] = fectrl_fem5516_fc1;
				break;
			case 2:
				fectrl_c[0] = fectrl_femctrl2_sub2_c0;
				fectrl_c[1] = fectrl_femctrl2_sub2_c12;
				fectrl_c[2] = fectrl_femctrl2_sub2_c12;
				break;
			case 3: /* 43602bu and 43602cd (X238) femctrl */
			case 4: /* 43602cs (X87)  femctrl (bt on gpio7) */
				fectrl_c[0] = fectrl_fem5517_c0; /* 256 entries */
				fectrl_c[1] = fectrl_fem5517_c1; /* 64 entries */
				fectrl_c[2] = fectrl_fem5517_c2; /* 64 entries */
				break;
			case 5: /* 4360 + 85309 or QFP 4219 */
				fectrl_c[0] = fectrl_fem85309;
				fectrl_c[1] = fectrl_fem85309;
				fectrl_c[2] = fectrl_fem85309;
				break;

			}
			break;
		case 3:
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				if (BCM43602_CHIP(pi->sh->chip)) {
					fectrl_c[0] = fectrl_43602_mch5_c0;
					fectrl_c[1] = fectrl_43602_mch5_c1;
					fectrl_c[2] = fectrl_43602_mch5_c2;
				} else {
					fectrl_c[0] = fectrl_mch5_c0_p200_p400_fc3_sub0;
					fectrl_c[1] = fectrl_mch5_c1_p200_p400_fc3_sub0;
					fectrl_c[2] = fectrl_mch5_c2_p200_p400_fc3_sub0;
				}
				break;
			case 1:
				fectrl_c[0] = fectrl_mch5_c0_fc3_sub1;
				fectrl_c[1] = fectrl_mch5_c1_fc3_sub1;
				fectrl_c[2] = fectrl_mch5_c2_fc3_sub1;
				break;
			case 2:
				fectrl_c[0] = fectrl_j28_fc3_sub2;
				fectrl_c[1] = fectrl_j28_fc3_sub2;
				fectrl_c[2] = fectrl_j28_fc3_sub2;
				break;
			case 3:
				if (BCM43602_CHIP(pi->sh->chip)) {
					fectrl_c[0] = fectrl_43602_mch2_c0;
					fectrl_c[1] = fectrl_43602_mch2_c1;
					fectrl_c[2] = fectrl_43602_mch2_c2;
				} else {
					fectrl_c[0] = fectrl3_sub3_c0;
					fectrl_c[1] = fectrl3_sub3_c1;
					fectrl_c[2] = fectrl3_sub3_c2;
				}
				break;
			case 5:
				fectrl_c[0] = fectrl_43602_mch2_1_c0;
				fectrl_c[1] = fectrl_43602_mch2_1_c1;
				fectrl_c[2] = fectrl_43602_mch2_1_c2;
				break;
			case 6:
				fectrl_c[0] = fectrl3_sub6_43602;
				fectrl_c[1] = fectrl3_sub6_43602;
				fectrl_c[2] = fectrl3_sub6_43602;
				break;
			case 7:
				fectrl_c[0] = fectrl3_sub7_c0_4360;
				fectrl_c[1] = fectrl3_sub7_c1_4360;
				fectrl_c[2] = fectrl3_sub7_c2_4360;
				break;
			}
			break;
		case 5:
			fectrl_c[0] = fectrl_fem5516_fc1;
			fectrl_c[1] = fectrl_femctrl5_c1;
			fectrl_c[2] = fectrl_zeros;
			break;
		case 6:   /* MC5 medium power router board */
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0: /* MC2, MC5 medium power boards */
				if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
					/* 43602 has same FEM controls on MC5 */
					fectrl_c[0] = fectrl_rfmd4591;
					fectrl_c[1] = fectrl_rfmd4591;
					fectrl_c[2] = fectrl_rfmd4591;
				} else {
					fectrl_c[0] = fectrl_femctrl6;
					fectrl_c[1] = fectrl_femctrl6;
					fectrl_c[2] = fectrl_femctrl6;
				}
				break;
			case 1: /* R8000 (2g & high 5g) */
				fectrl_c[0] = fectrl6_sub1_43602;
				fectrl_c[1] = fectrl6_sub1_43602;
				fectrl_c[2] = fectrl6_sub1_43602;
				break;
			case 2: /* R8000 low 5G */
				fectrl_c[0] = fectrl6_sub2_43602;
				fectrl_c[1] = fectrl6_sub2_43602;
				fectrl_c[2] = fectrl6_sub2_43602;
				break;
			case 5: /* 43602 85201 Pegatron */
				fectrl_c[0] = fectrl1_sub5_43602_c0;
				fectrl_c[1] = fectrl1_sub5_43602_c1;
				fectrl_c[2] = fectrl1_sub5_43602_c2;
				break;
			}
			break;
		case 4:
			/* 4335 epa elna boards */
			/* get table size */
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				sparse_table_len =
					ARRAYSIZE(fectrl_fcbga_epa_elna_fc4_sub0);
				break;
			case 1:
				sparse_table_len =
					ARRAYSIZE(fectrl_wlbga_epa_elna_fc4_sub1);
				break;
			case 2:
				sparse_table_len =
					ARRAYSIZE(fectrl_fchm_epa_elna_fc4_sub2);
				break;
			case 3:
			case 4:
				sparse_table_len =
					ARRAYSIZE(fectrl_wlcsp_epa_elna_fc4_sub34);
				break;
			case 5:
				sparse_table_len =
					ARRAYSIZE(fectrl_fp_dpdt_epa_elna_fc4_sub5);
				break;
			case 6:
				sparse_table_len =
					ARRAYSIZE(fectrl_43162_fcbga_ipa_ilna_fc4_sub6);
				ani->fectrl_spl_entry_flag = 1;
				break;
			case 7:
				sparse_table_len =
					ARRAYSIZE(fectrl_43162_fcbga_ipa_elna_fc4_sub7);
				ani->fectrl_spl_entry_flag = 1;
				break;
			}
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}

			/* pick proper table */
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				fe_ctrl_tbl = fectrl_fcbga_epa_elna_fc4_sub0;
				break;
			case 1:
				fe_ctrl_tbl = fectrl_wlbga_epa_elna_fc4_sub1;
				break;
			case 2:
				fe_ctrl_tbl = fectrl_fchm_epa_elna_fc4_sub2;
				break;
			case 3:
			case 4:
				fe_ctrl_tbl = fectrl_wlcsp_epa_elna_fc4_sub34;
				break;
			case 5:
				fe_ctrl_tbl = fectrl_fp_dpdt_epa_elna_fc4_sub5;
				break;
			case 6:
				fe_ctrl_tbl = fectrl_43162_fcbga_ipa_ilna_fc4_sub6;
				break;
			case 7:
				fe_ctrl_tbl = fectrl_43162_fcbga_ipa_elna_fc4_sub7;
				break;
			}

			/* copy table */
			for (kk = 0; kk < sparse_table_len; kk++) {
				ani->fectrl_idx[kk] = fe_ctrl_tbl[kk].idx;
				ani->fectrl_val[kk] = fe_ctrl_tbl[kk].val;
			}
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		case 7:
			/* pick size */
			if (!PHY_IPA(pi)) {
				sparse_table_len = ARRAYSIZE(fectrl_femctrl_4335_WLBGA_add_fc7);
			}
			else {
				sparse_table_len = ARRAYSIZE(fectrl_femctrl_4335_WLBGA_iPa_add_fc7);
			}
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			/* pick table */
			if (PHY_IPA(pi)) {
				fectrl_idx = fectrl_femctrl_4335_WLBGA_iPa_add_fc7;
				fectrl_val = fectrl_femctrl_4335_WLBGA_iPa_fc7;
			}
			else {
				fectrl_idx = fectrl_femctrl_4335_WLBGA_add_fc7;
				fectrl_val = fectrl_femctrl_4335_WLBGA_fc7;
			}
			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len* sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len* sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		case 8:
			sparse_table_len = ARRAYSIZE(fectrl_femctrl_4335_FCBGA_add_fc8);
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			/* pick table */

			fectrl_idx = fectrl_femctrl_4335_FCBGA_add_fc8;
			fectrl_val = fectrl_femctrl_4335_FCBGA_fc8;

			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len* sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len* sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		case 9:
			sparse_table_len = ARRAYSIZE(fectrl_fcbgabu_epa_elna_idx_fc9);

			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			/* pick table */

			fectrl_idx = fectrl_fcbgabu_epa_elna_idx_fc9;
			fectrl_val = fectrl_fcbgabu_epa_elna_val_fc9;

			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len * sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len * sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		case 10:
			/* 4350 chips have a 320-element fem ctrl table */

			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				sparse_table_len =
					ARRAYSIZE(fectrl_fcbga_epa_elna_idx_fc10_sub0);
				break;
			case 1:
				sparse_table_len =
					ARRAYSIZE(fectrl_wlbga_epa_elna_idx_fc10_sub1);
				break;
			case 2:
				sparse_table_len =
					ARRAYSIZE(fectrl_wlbga_ipa_ilna_idx_fc10_sub2);
				break;
			case 3:
				sparse_table_len =
					ARRAYSIZE(fectrl_43556usb_epa_elna_idx_fc10_sub3);
				break;
			case 4:
				sparse_table_len =
					ARRAYSIZE(fectrl_fcbga_ipa_ilna_idx_fc10_sub4);
				break;
			case 5:
				sparse_table_len =
					ARRAYSIZE(fectrl_idx_fc10_sub5);
				break;
			}
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			default:
			case 0:
				fectrl_idx = fectrl_fcbga_epa_elna_idx_fc10_sub0;
				fectrl_val = fectrl_fcbga_epa_elna_val_fc10_sub0;
				break;
			case 1:
				fectrl_idx = fectrl_wlbga_epa_elna_idx_fc10_sub1;
				fectrl_val = fectrl_wlbga_epa_elna_val_fc10_sub1;
				break;
			case 2:
				fectrl_idx = fectrl_wlbga_ipa_ilna_idx_fc10_sub2;
				fectrl_val = fectrl_wlbga_ipa_ilna_val_fc10_sub2;
				break;
			case 3:
				fectrl_idx = fectrl_43556usb_epa_elna_idx_fc10_sub3;
				fectrl_val = fectrl_43556usb_epa_elna_val_fc10_sub3;
				break;
			case 4:
				fectrl_idx = fectrl_fcbga_ipa_ilna_idx_fc10_sub4;
				fectrl_val = fectrl_fcbga_ipa_ilna_val_fc10_sub4;
				break;
			case 5:
				fectrl_idx = fectrl_idx_fc10_sub5;
				fectrl_val = fectrl_val_fc10_sub5;
				break;
			}
			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len* sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len* sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
			/* LOOK: when adding new cases, follow above pattern to
			 * minimize stack/memory usage!
			 */
		case 11:
			sparse_table_len =
				ARRAYSIZE(fectrl_fcbu_epa_idx_fc11);
			table_len = 3*256;
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			fectrl_idx = fectrl_fcbu_epa_idx_fc11;
			fectrl_val = fectrl_fcbu_epa_val_fc11;

			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len* sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len* sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		case 17:
			table_len = 256;
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			case FCBGA_43012:
				sparse_table_len =
					ARRAYSIZE(fectrl_fem43012_idx_BCM43012_fcbga);
				break;
			case WLBGA_43012 :
				sparse_table_len =
					ARRAYSIZE(fectrl_fem43012_idx_BCM43012_wlbga);
				break;
			default:
				PHY_ERROR(("wl%d: %s: Provide correct femctrl_sub \n",
					pi->sh->unit, __FUNCTION__));
				ASSERT(0);
			}
			/* malloc table */
			if ((ani->fectrl_idx =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			if ((ani->fectrl_val =
			     phy_malloc(pi,
			            sparse_table_len * sizeof(uint16))) == NULL) {
				PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				           pi->sh->unit,
				           __FUNCTION__, MALLOCED(pi->sh->osh)));
				return FALSE;
			}
			switch (BF3_FEMCTRL_SUB(pi_ac)) {
			case FCBGA_43012:
				fectrl_idx = fectrl_fem43012_idx_BCM43012_fcbga;
				fectrl_val = fectrl_fem43012_val_BCM43012_fcbga;
				break;
			case WLBGA_43012 :
				fectrl_idx = fectrl_fem43012_idx_BCM43012_wlbga;
				fectrl_val = fectrl_fem43012_val_BCM43012_wlbga;
				break;
			default:
				PHY_ERROR(("wl%d: %s: Provide correct femctrl_sub \n",
					pi->sh->unit, __FUNCTION__));
				ASSERT(0);
			}
			memcpy(ani->fectrl_idx, fectrl_idx,
			       sparse_table_len* sizeof(uint16));
			memcpy(ani->fectrl_val, fectrl_val,
			       sparse_table_len* sizeof(uint16));
			ani->fectrl_sparse_table_len = sparse_table_len;
			ani->fectrl_table_len = table_len;
			break;
		}

		/* 4360 / 43602 specific */
		if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			FOREACH_CORE(pi, core) {
				p_core = &ani->fectrl_c[core];
				ASSERT(p_core->subtable != NULL);
				if (p_core->subtable != NULL)
					memcpy(p_core->subtable, fectrl_c[core],
						p_core->n_entries * sizeof(uint8));
			}
		}
	}
	return TRUE;
}

void
wlc_acphy_paldo_change(phy_info_t *pi)
{
	si_t *sih = (si_t*)pi->sh->sih;
	osl_t *osh = si_osh(sih);
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint origidx = si_coreidx(sih);

	if (pi_ac->anai->ldo3p3_2g || pi_ac->anai->ldo3p3_5g) {
		si_setcoreidx(sih, SI_CC_IDX);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			si_pmu_set_ldo_voltage(sih, osh, 1, pi_ac->anai->ldo3p3_2g);
		} else {
			si_pmu_set_ldo_voltage(sih, osh, 1, pi_ac->anai->ldo3p3_5g);
		}
		si_setcoreidx(sih, origidx);
	}
}

void
wlc_phy_write_femctrl_table(phy_ac_ana_info_t *ani)
{
	int core;
	acphy_fe_ctrl_table_t *p;
	phy_info_t *pi = ani->pi;

	FOREACH_CORE(pi, core) {
		p = &ani->fectrl_c[core];
		if (p->subtable != NULL) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, p->n_entries,
				p->hw_offset, 8, p->subtable);
		} else {
			PHY_ERROR(("wl%d: %s: Undefined FEM Control table C%d. Radio revision %d\n",
				pi->sh->unit, __FUNCTION__, core, RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
		}
	}
}

#define SWCTRLMAP4_FEMCTRL_TBL_MAP_LEN 16
static void
wlc_phy_write_femctrl_table_swctrlmap4(phy_info_t *pi)
{
	        uint8 core, i;
	        typedef enum {TX = 0, RX, RXBYP, MISC} swctrlmap4_mode_t;
	        swctrlmap4_mode_t femctrl_mapping[2][SWCTRLMAP4_FEMCTRL_TBL_MAP_LEN] = {
	                {
	                        MISC, MISC, RX, RXBYP,
	                        MISC, MISC, RX, RXBYP,
	                        MISC, TX, MISC, TX,
	                        MISC, TX, MISC, TX
	                },
	                {
	                        RXBYP, MISC, RX, RXBYP,
	                        RXBYP, MISC, RX, RXBYP,
	                        RXBYP, TX, RXBYP, TX,
	                        RXBYP, TX, RXBYP, TX
	                }
	        };
	        uint8 num_tbl_entries_band = SWCTRLMAP4_FEMCTRL_TBL_MAP_LEN;
	        uint16 tbl_entries[2*SWCTRLMAP4_FEMCTRL_TBL_MAP_LEN];
	        acphy_swctrlmap4_t *swctrl = pi->u.pi_acphy->sromi->swctrlmap4;

	        if (swctrl->misc_usage >= sizeof(femctrl_mapping)/sizeof(femctrl_mapping[0])) {
	                PHY_ERROR(("wl%d: %s: swctrlmap4->misc_usage = %d unsupported \n",
	                           pi->sh->unit, __FUNCTION__, swctrl->misc_usage));
	                ASSERT(0);
	        }
	        FOREACH_CORE(pi, core) {
	                for (i = 0; i < num_tbl_entries_band; i++) {
	                        switch (femctrl_mapping[swctrl->misc_usage][i]) {
	                        case TX:
	                                tbl_entries[i] = swctrl->tx2g[core];
	                                tbl_entries[num_tbl_entries_band + i] = swctrl->tx5g[core];
	                                break;
	                        case RX:
	                                tbl_entries[i] = swctrl->rx2g[core];
	                                tbl_entries[num_tbl_entries_band + i] = swctrl->rx5g[core];
	                                break;
	                        case RXBYP:
	                                tbl_entries[i] = swctrl->rxbyp2g[core];
					tbl_entries[num_tbl_entries_band + i] =
						swctrl->rxbyp5g[core];
	                                break;
	                        case MISC:
	                                tbl_entries[i] = swctrl->misc2g[core];
					tbl_entries[num_tbl_entries_band + i] =
						swctrl->misc5g[core];
	                                break;
	                        }
	                }
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT,
				2*num_tbl_entries_band, 256*core, 16, tbl_entries);
	        }
}

static INLINE void
wlc_phy_write_sparse_femctrl_table(phy_ac_ana_info_t *anai)
{
	phy_info_t *pi = anai->pi;
	uint16 fectrl_zeroval[] = {0};
	uint16 fectrl_fourval[] = {4};
	uint16 fectrl_nineval[] = {9};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < anai->fectrl_table_len; kk++) {
		if (fem_idx < anai->fectrl_sparse_table_len && kk == anai->fectrl_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(anai->fectrl_val[fem_idx]));
			fem_idx++;
		} else if (anai->fectrl_spl_entry_flag) {
			/* 43162: Fix to avoid all zero output from femctrl during */
			/* tx2rx/rx2tx in 5G which causes popping-sound in BT */
			/* tx2rx/rx2tx in 2G also cause zero state on FEM, add lines for safety */
			if (kk & 0x10) {
				/* 5G */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
					fectrl_fourval);
			} else {
				/* 2G */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
					fectrl_nineval);
			}
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_enable_pavref_war(phy_info_t *pi)
{
	/* 43602a0: power on PARLDO and update RFSeq table */
	const uint16 tx2rx_delay = 0x130;
	const uint16 lna_trsw_timing[] = {0x1, 0x2, 0x2, 0x2, 0x4};
	si_pmu_switch_on_PARLDO(pi->sh->sih, pi->sh->osh);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
		1, 0x80, 16, &tx2rx_delay);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
		ARRAYSIZE(lna_trsw_timing), 0x70, 16, &lna_trsw_timing);

	WRITE_PHYREG(pi, dot11acphycrsTxExtension, 0x1);
	W_REG(pi->sh->osh, D11_IFS_SIFS_RX_TX_TX(pi), 0x7676);
	W_REG(pi->sh->osh, D11_IFS_SIFS_NAV_TX(pi), 0x0276);
	W_REG(pi->sh->osh, D11_PSM_INTSEL_2(pi), 0x5);
}

void
wlc_phy_set_regtbl_on_femctrl(phy_info_t *pi)
{
	uint8 stall_val;
	uint8 bt_fem;	/* bitfield in PHY register BT_FemControl */
	uint8 gpio_en;	/* set which gpio pins are controlled by the PHY and which by ucode */
	bool bt_on_gpio4;
	uint32 chipcontrol_mask; /* chipcommon core chipcontrol register */
	uint32 chipcontrol_val;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (ACPHY_SWCTRLMAP4_EN(pi)) {
		wlc_phy_write_femctrl_table_swctrlmap4(pi);
	} else if (!ACPHY_FEMCTRL_ACTIVE(pi)) {
		wlc_phy_write_regtbl_fc_from_nvram(pi);

	} else {
		switch (BFCTL(pi->u.pi_acphy)) {
		case 0:
			/* Chip default, do nothing */
			break;
		case 1:
			/* chip_bandsel = bandsel */
			MOD_PHYREG(pi, BT_SwControl, bt_sharing_en, 1);
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);
			break;
		case 2:
			/*	X29c & 4352hmb(wiht B0)
				Cores {0, 2} have 5516 fem. Core 1 has separate 2g/5g fems
			*/
			bt_fem = 0; bt_on_gpio4 = FALSE;
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);
			gpio_en = 0xa0;
			chipcontrol_mask = CCTRL4360_SECI_MODE | CCTRL4360_SECI_ON_GPIO01 |
				CCTRL4360_BTSWCTRL_MODE;
			chipcontrol_val = 0;

			if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 0) {
				bt_on_gpio4 = TRUE;  /* fem_bt = gpio4 */
				gpio_en = 0xe0;
			} else if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 3) {
				/*
				 * For 5517. In 43602 turn off VLIN override mux, and always keep
				 * VLIN high through FEM CTRL * table.
				 * bcm943602bu : 3 antenna board, no BT support
				 * bcm943602cd (X238) : 3 Wifi + 1 BT antenna board
				 */
				MOD_PHYREG(pi, RfctrlCoreGlobalPus,
					muxTxVlinOnFemCtrl, 0x0);
				chipcontrol_val = (CCTRL4360_SECI_MODE | CCTRL4360_SECI_ON_GPIO01);
				bt_fem = 4;      /* fem_bt = bt_fem[2] */
				gpio_en = 0xa0;  /* bt on gpio6 */
			} else if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 4) {
				/*
				 * For 5517. In 43602 turn off VLIN override mux, and
				 * always keep VLIN high through FEM CTRL * table.
				 * bcm943602cs (X87) : 3 antenna, middle antenna is shared BT/Wifi.
				 * gpio7 flows towards FEM BT_EN pin.
				 */
				MOD_PHYREG(pi, RfctrlCoreGlobalPus, muxTxVlinOnFemCtrl, 0x0);
				chipcontrol_val = (CCTRL4360_SECI_MODE | CCTRL4360_SECI_ON_GPIO01 |
					CCTRL4360_BTSWCTRL_MODE);
				bt_fem = 2; /* fem_bt = bt_fem[1] */
				gpio_en = 0x60; /* d[7]=0 -> allows ucode to control gpio7 */
			} else if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 5 ||
					BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 6 ||
					BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 7) {
				PHY_TRACE(("wl%d: %s: FEMCTRL2SUB5/6/7 43602 WLCSP\n",
					pi->sh->unit, __FUNCTION__));
				chipcontrol_mask = (CCTRL4360_SECI_ON_GPIO01 |
					CCTRL4360_DISCRETE_FEMCTRL_MODE);
				chipcontrol_val = (CCTRL4360_SECI_ON_GPIO01 |
					CCTRL4360_DISCRETE_FEMCTRL_MODE);
			    MOD_PHYREG(pi, BT_SwControl, bt_sharing_en, 0);
			} else {
				bt_fem = 4; /* fem_bt = bt_fem[2] */
				gpio_en = 0xa0;  /* bt on gpio6 */
			}

			if (chipcontrol_val != 0) {
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, chipcontrol),
					chipcontrol_mask, chipcontrol_val);
			}

			/* Setup middle core for BT */
			wlc_phy_set_bt_on_core1_acphy(pi, bt_fem, gpio_en);

			/* Release control of gpio4 if required */
			if (bt_on_gpio4)
				wlc_phy_bt_on_gpio4_acphy(pi);
			break;
		case 3:
			/*	Routers (MCH5, J28) */
			MOD_PHYREG(pi, BT_SwControl, bt_sharing_en, 0);
			if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
				/* all 43602 chips */
				if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 0 ||
				    BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 3) {
					if (ACMINORREV_0(pi)) {
						/* 43602a0: enable PAVREF WAR for boards
						 * that enable PA with PAVREF
						 */
						wlc_phy_enable_pavref_war(pi);
					} else {
						/* 43602a1 and later: power on PAVREF LDO
						 * for boards that enable PA with PAVREF
						 */
						si_pmu_switch_on_PARLDO(pi->sh->sih, pi->sh->osh);
					}
				}
			} else {
				si_pmu_regcontrol(pi->sh->sih, 0, 0x4, 4); /* pwron pavref ldo */
			}
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);

			if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 5) {
				/* MCH2 with digital PA control */
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, chipcontrol), 0xfffffd,
					CCTRL4360_DISCRETE_FEMCTRL_MODE |
					CCTRL4360_DIGITAL_PACTRL_MODE);
			} else if (BF3_FEMCTRL_SUB(pi->u.pi_acphy) < 2 ||
			           ACMAJORREV_5(pi->pubpi->phy_rev)) {
				/* MCH5 and 43602MHC2: leave bit1 untouched for uart */
				si_corereg(pi->sh->sih, SI_CC_IDX,
					OFFSETOF(chipcregs_t, chipcontrol),
					0xfffffd, CCTRL4360_DISCRETE_FEMCTRL_MODE);
			}
			/* STB (USBH5) */
			if ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags2) &
				BFL2_SROM11_ANAPACTRL_5G) &&
				(BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardtype) ==
				BCM94360USBH5_D11AC5G)) {
				/* power on PA(bit 2) & RF(bit 1) LDO */
				wlapi_bmac_write_shm(pi->sh->physhim, M_RFLDO_ON_L(pi), 0x4);
				wlapi_bmac_write_shm(pi->sh->physhim, M_RFLDO_ON_H(pi), 0x20);
			}
			break;
		case 5:
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);
			/* Setup middle core for BT */
			wlc_phy_set_bt_on_core1_acphy(pi, 8, 0xc0);
			break;
		case 6:
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);
			break;
		case 4:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			wlc_phy_write_sparse_femctrl_table(pi->u.pi_acphy->anai);
			break;
		case 17:
			wlc_phy_write_sparse_femctrl_table(pi->u.pi_acphy->anai);
			break;

			/* LOOK: when adding new cases, follow above pattern to
			 * minimize stack/memory usage!
			 */
		default:
			/* 5516 on all cores */
			/* chip_bandsel = bandsel */
			MOD_PHYREG(pi, BT_SwControl, bt_sharing_en, 1);
			wlc_phy_write_femctrl_table(pi->u.pi_acphy->anai);
			break;
		}
	}

	if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (BF_SROM11_BTCOEX(pi->u.pi_acphy)) {
			if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
				if (ACMINORREV_0(pi)) {
					si_corereg(pi->sh->sih,
						SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
						CCTRL4360_SECI_MODE, CCTRL4360_SECI_MODE);
				} else if (ACMINORREV_1(pi)) {
					si_corereg(pi->sh->sih,
						SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
						CCTRL4360_SECI_ON_GPIO01, CCTRL4360_SECI_ON_GPIO01);
				} else {
					ASSERT(0);
				}
			} else if (ACMAJORREV_1(pi->pubpi->phy_rev) ||
				ACMAJORREV_2(pi->pubpi->phy_rev) ||
				ACMAJORREV_3(pi->pubpi->phy_rev) ||
				ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) ||
				ACMAJORREV_5(pi->pubpi->phy_rev)) {
				PHY_ERROR(("\n"));
			} else {
				ASSERT(0);
			}
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

#ifdef SWCTRL_TO_BT_IN_COEX
uint16
wlc_phy_set_mask_for_femctrl10(phy_info_t *pi)
{
	uint16 femctrlmask;
#ifndef BOARD_FLAGS3
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
#endif // endif

	if (BF3_FEMCTRL_SUB(pi_ac) == 0) {
		femctrlmask = 0x317;
	} else if (BF3_FEMCTRL_SUB(pi_ac) == 1) {
		femctrlmask = 0x347;
	} else if (BF3_FEMCTRL_SUB(pi_ac) == 2) {
		femctrlmask = 0x303;
	} else if (BF3_FEMCTRL_SUB(pi_ac) == 3) {
		femctrlmask = 0x307;
	} else if (BF3_FEMCTRL_SUB(pi_ac) == 4) {
		femctrlmask = 0x309;
	} else if (BF3_FEMCTRL_SUB(pi_ac) == 5) {
		femctrlmask = 0x3c7;
	} else {
		femctrlmask = 0x3ff;
	}

	return femctrlmask;
}
#endif	/* SWCTRL_TO_BT_IN_COEX */

static int
BCMATTACHFN(phy_ac_ana_srom_swctrlmap4_read)(phy_info_t *pi)
{
	uint8 core, slice_ant_core;
	uint16 tmp;
	uint8 mode_idx;
	const char *mode_key[] = {"TX", "RX", "RXByp", "misc"};
	uint16 mode_3to0[] = {0, 0, 0, 0};
	uint16 mode_7to4[] = {0, 0, 0, 0};
	uint16 mode_9to8[] = {0, 0, 0, 0};
	char phy_var_name[40];
	acphy_swctrlmap4_t *swctrl;
	uint8 num_modes = sizeof(mode_3to0)/sizeof(mode_3to0[0]);
	if (pi->u.pi_acphy->sromi->swctrlmap4 == NULL) {
	  if ((pi->u.pi_acphy->sromi->swctrlmap4 =
	       phy_malloc(pi, sizeof(*(pi->u.pi_acphy->sromi->swctrlmap4)))) == NULL) {
	    PHY_ERROR(("%s: swctrlmap4 malloc failed\n", __FUNCTION__));
	    return BCME_NOMEM;
	  }
	}
	swctrl = pi->u.pi_acphy->sromi->swctrlmap4;

	tmp = (uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_swctrlmap4_cfg, 0);
	swctrl->enable = (uint8)(tmp & 0x1);
	swctrl->bitwidth8 = (uint8)((tmp >> 1) & 0x1);
	swctrl->misc_usage = (uint8)((tmp >> 2) & 0x3);
	swctrl->bitwidth10_ext = (uint8)((tmp >> 4) & 0x1) & swctrl->bitwidth8;
	swctrl->bandsel_on_gpio9 = (uint8)((tmp >> 5) & 0x1);
	swctrl->bandsel_on_gpio11 = (uint8)((tmp >> 7) & 0x1);
	if ((ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) &&
		swctrl->enable == 0)
		return TRUE;

	for (mode_idx = 0; mode_idx < num_modes; mode_idx++) {
		(void)snprintf(phy_var_name, sizeof(phy_var_name),
			rstr_swctrlmap4_S2g_fem3to0, mode_key[mode_idx]);
	  mode_3to0[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
	  if (swctrl->bitwidth8 == 1) {
		(void)snprintf(phy_var_name, sizeof(phy_var_name),
			rstr_swctrlmap4_S2g_fem7to4, mode_key[mode_idx]);
	    mode_7to4[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
	  }
	  if (swctrl->bitwidth10_ext == 1) {
		(void)snprintf(phy_var_name, sizeof(phy_var_name),
			rstr_swctrlmap4_S2g_fem9to8, mode_key[mode_idx]);
	    mode_9to8[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
	  }
	}

	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			swctrl->tx2g[core] = (uint8)((((mode_7to4[0] >> (4*core)) & 0xf) << 4) +
			                             ((mode_3to0[0] >> (4*core)) & 0xf));
			swctrl->rx2g[core] = (uint8)((((mode_7to4[1] >> (4*core)) & 0xf) << 4) +
			                             ((mode_3to0[1] >> (4*core)) & 0xf));
			swctrl->rxbyp2g[core] = (uint8)((((mode_7to4[2] >> (4*core)) & 0xf) << 4) +
			                                ((mode_3to0[2] >> (4*core)) & 0xf));
			swctrl->misc2g[core] = (uint8)((((mode_7to4[3] >> (4*core)) & 0xf) << 4) +
			                               ((mode_3to0[3] >> (4*core)) & 0xf));
		} else {
			slice_ant_core = ((phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1) ?
				pi->aux_slice_ant[core] : pi->main_slice_ant[core]);
			swctrl->tx2g[core] = (uint16)((((mode_9to8[0] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[0] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[0] >> (4*slice_ant_core)) & 0xf));
			swctrl->rx2g[core] = (uint16)((((mode_9to8[1] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[1] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[1] >> (4*slice_ant_core)) & 0xf));
			swctrl->rxbyp2g[core] = (uint16)((((mode_9to8[2] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[2] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[2] >> (4*slice_ant_core)) & 0xf));
			swctrl->misc2g[core] = (uint16)((((mode_9to8[3] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[3] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[3] >> (4*slice_ant_core)) & 0xf));
#ifdef BCMDBG
			PHY_INFORM(("core %d \t slice_ant_core %d \n", core, slice_ant_core));
			PHY_INFORM(("swctrl->tx2g[%d] \t 0x%04x\n", core, swctrl->tx2g[core]));
			PHY_INFORM(("swctrl->rx2g[%d] \t 0x%04x\n", core, swctrl->rx2g[core]));
			PHY_INFORM(("swctrl->rxbyp2g[%d] \t 0x%04x\n", core,
				swctrl->rxbyp2g[core]));
			PHY_INFORM(("swctrl->misc2g[%d] \t 0x%04x\n", core, swctrl->misc2g[core]));
#endif // endif
		}
	}

	for (mode_idx = 0; mode_idx < num_modes; mode_idx++) {
		(void)snprintf(phy_var_name, sizeof(phy_var_name),
			rstr_swctrlmap4_S5g_fem3to0, mode_key[mode_idx]);
		mode_3to0[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
		if (swctrl->bitwidth8 == 1) {
			(void)snprintf(phy_var_name, sizeof(phy_var_name),
				rstr_swctrlmap4_S5g_fem7to4, mode_key[mode_idx]);
			mode_7to4[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
		}
		if (swctrl->bitwidth10_ext == 1) {
			(void)snprintf(phy_var_name, sizeof(phy_var_name),
				rstr_swctrlmap4_S5g_fem9to8, mode_key[mode_idx]);
			mode_9to8[mode_idx] = (uint16) PHY_GETINTVAR_DEFAULT(pi, phy_var_name, 0);
		}
	}

	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			swctrl->tx5g[core] = (uint8)((((mode_7to4[0] >> (4*core)) & 0xf) << 4) +
			                             ((mode_3to0[0] >> (4*core)) & 0xf));
			swctrl->rx5g[core] = (uint8)((((mode_7to4[1] >> (4*core)) & 0xf) << 4) +
			                             ((mode_3to0[1] >> (4*core)) & 0xf));
			swctrl->rxbyp5g[core] = (uint8)((((mode_7to4[2] >> (4*core)) & 0xf) << 4) +
			                                ((mode_3to0[2] >> (4*core)) & 0xf));
			swctrl->misc5g[core] = (uint8)((((mode_7to4[3] >> (4*core)) & 0xf) << 4) +
			                               ((mode_3to0[3] >> (4*core)) & 0xf));
		} else {
			slice_ant_core = ((phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1) ?
				pi->aux_slice_ant[core] : pi->main_slice_ant[core]);
			swctrl->tx5g[core] = (uint16)((((mode_9to8[0] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[0] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[0] >> (4*slice_ant_core)) & 0xf));
			swctrl->rx5g[core] = (uint16)((((mode_9to8[1] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[1] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[1] >> (4*slice_ant_core)) & 0xf));
			swctrl->rxbyp5g[core] = (uint16)((((mode_9to8[2] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[2] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[2] >> (4*slice_ant_core)) & 0xf));
			swctrl->misc5g[core] = (uint16)((((mode_9to8[3] >> (2*slice_ant_core)) &
				0x3) << 8) + (((mode_7to4[3] >> (4*slice_ant_core)) & 0xf) << 4) +
				((mode_3to0[3] >> (4*slice_ant_core)) & 0xf));
#ifdef BCMDBG
			PHY_INFORM(("core %d \t slice_ant_core %d \n", core, slice_ant_core));
			PHY_INFORM(("swctrl->tx5g[%d] \t 0x%04x\n", core, swctrl->tx5g[core]));
			PHY_INFORM(("swctrl->rx5g[%d] \t 0x%04x\n", core, swctrl->rx5g[core]));
			PHY_INFORM(("swctrl->rxbyp5g[%d] \t 0x%04x\n", core,
				swctrl->rxbyp5g[core]));
			PHY_INFORM(("swctrl->misc5g[%d] \t 0x%04x\n", core, swctrl->misc5g[core]));
#endif // endif
		}
	}
	return TRUE;
}
