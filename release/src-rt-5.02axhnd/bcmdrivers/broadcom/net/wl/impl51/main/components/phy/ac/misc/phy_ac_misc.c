/*
 * ACPHY Miscellaneous modules implementation
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
 * $Id: phy_ac_misc.c 766801 2018-08-14 18:08:26Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_cache.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_btcx.h>
#include <phy_misc.h>
#include <phy_misc_api.h>
#include <phy_temp.h>
#include "phy_type_misc.h"
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_misc.h>
#include <phy_ac_antdiv.h>
#include <phy_ac_calmgr.h>
#include <phy_ac_radio.h>
#include <phy_ac_rxgcrs.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_int.h>
#include <phy_stf.h>
#include <phy_rxgcrs_api.h>
/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <phy_ac_rssi.h>
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <hndpmu.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#include <bcmdevs.h>
#include "wlc_radioreg_20691.h"
#include "wlc_radioreg_20693.h"
#include "wlc_radioreg_20694.h"
#include <wlc_radioreg_20695.h>
#include <wlc_radioreg_20697.h>
#include <wlc_radioreg_20698.h>
#include <wlc_radioreg_20704.h>

/* module private states */
struct phy_ac_misc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_misc_info_t *cmn_info;
	uint16	bb_mult_save[PHY_CORE_MAX];
	uint16	saved_bbconf;
	uint16	AfePuCtrl;
	uint8	bb_mult_save_valid;
	bool	ac_rxldpc_override;		/* LDPC override for RX, both band */
	bool	rud_agc_enable;
	int16	iqest[PHY_MAX_CORES];
};

#define DEFAULT_SPB_DEPTH (512)

#ifdef PHY_DUMP_BINARY
/* AUTOGENRATED by the tool : phyreg.py
 * These values cannot be in const memory since
 * the const area might be over-written in case of
 * crash dump
 */
phyradregs_list_t dot11acphy_regs_rev24[] = {
	{0x000,  {0x7f, 0xff, 0xfc, 0x9f}},
	{0x01f,  {0x0, 0x2, 0x1f, 0xfb}},
	{0x040,  {0x0, 0x1, 0xff, 0xff}},
	{0x060,  {0x1f, 0xff, 0x0, 0xff}},
	{0x080,  {0x80, 0x0, 0x0, 0x29}},
	{0x0b0,  {0x80, 0x0, 0x0, 0x5d}},
	{0x120,  {0x0, 0x0, 0x7, 0xff}},
	{0x140,  {0x7d, 0xff, 0xff, 0xff}},
	{0x15f,  {0x67, 0xff, 0xff, 0xff}},
	{0x17e,  {0x7, 0x80, 0x0, 0x7}},
	{0x19d,  {0x7f, 0xfe, 0x7f, 0x87}},
	{0x1bc,  {0x80, 0x0, 0x0, 0x47}},
	{0x210,  {0x0, 0x0, 0x3, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x1, 0xff, 0xfb, 0xff}},
	{0x270,  {0x7f, 0xff, 0x0, 0x3f}},
	{0x28f,  {0x0, 0x1, 0xff, 0xff}},
	{0x2b0,  {0x7, 0xff, 0xf, 0xff}},
	{0x2d0,  {0x7d, 0xff, 0xff, 0xff}},
	{0x2ef,  {0x7f, 0xff, 0x87, 0xff}},
	{0x30e,  {0x80, 0x0, 0x0, 0x3d}},
	{0x34d,  {0x1f, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0xf, 0x0, 0xff}},
	{0x390,  {0x7f, 0xfe, 0x1f, 0xff}},
	{0x3af,  {0x80, 0x0, 0x0, 0x44}},
	{0x400,  {0x3f, 0xff, 0xff, 0xff}},
	{0x420,  {0x7f, 0xff, 0x3e, 0x7f}},
	{0x43f,  {0x80, 0x0, 0x0, 0x5a}},
	{0x49b,  {0x30, 0x0, 0x0, 0x1f}},
	{0x4d6,  {0x70, 0x1, 0xff, 0xff}},
	{0x4f5,  {0x8, 0x0, 0x38, 0x7f}},
	{0x520,  {0x7f, 0xff, 0xf, 0xff}},
	{0x53f,  {0x1f, 0xfe, 0x0, 0x7}},
	{0x580,  {0x3f, 0xff, 0xff, 0xfd}},
	{0x5a0,  {0x80, 0x0, 0x0, 0x2b}},
	{0x5d0,  {0x0, 0x0, 0x3, 0xff}},
	{0x600,  {0x0, 0xff, 0xff, 0xff}},
	{0x620,  {0x0, 0x0, 0xf, 0xff}},
	{0x640,  {0x0, 0x0, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0x0, 0x7}},
	{0x67f,  {0x7e, 0xfe, 0x0, 0x3f}},
	{0x69e,  {0x7f, 0xff, 0xff, 0xff}},
	{0x6c0,  {0x7f, 0x74, 0xf, 0xff}},
	{0x6df,  {0x7f, 0x2f, 0xf7, 0xff}},
	{0x6fe,  {0x0, 0x3c, 0x0, 0x1}},
	{0x720,  {0x7f, 0xff, 0x3, 0xff}},
	{0x741,  {0x0, 0xff, 0x81, 0xff}},
	{0x760,  {0x7c, 0x0, 0x3, 0xff}},
	{0x780,  {0x3, 0xff, 0xff, 0xff}},
	{0x7a0,  {0x7e, 0x67, 0x3f, 0xff}},
	{0x7bf,  {0x7f, 0xff, 0xff, 0xff}},
	{0x7e0,  {0x7f, 0xff, 0x7, 0xff}},
	{0x800,  {0x0, 0xff, 0xff, 0xff}},
	{0x820,  {0x7f, 0xff, 0xf, 0xff}},
	{0x83f,  {0x7f, 0xff, 0xff, 0xff}},
	{0x860,  {0x7f, 0xff, 0x0, 0x7}},
	{0x87f,  {0x7e, 0xfe, 0x0, 0x3f}},
	{0x89e,  {0x7f, 0xff, 0xff, 0xff}},
	{0x8c0,  {0x7f, 0x74, 0xf, 0xff}},
	{0x8df,  {0x7f, 0x2f, 0xf7, 0xff}},
	{0x8fe,  {0x0, 0x3c, 0x0, 0xd}},
	{0x920,  {0x7f, 0xff, 0x3, 0xff}},
	{0x941,  {0x0, 0xff, 0x81, 0xff}},
	{0x960,  {0x7c, 0x0, 0x3, 0xff}},
	{0x980,  {0x3, 0xff, 0xff, 0xff}},
	{0x9a0,  {0x7e, 0x67, 0x3f, 0xff}},
	{0x9bf,  {0x7f, 0xff, 0xff, 0xff}},
	{0x9e0,  {0x7f, 0xff, 0x7, 0xff}},
	{0xb00,  {0x0, 0x0, 0xff, 0xfb}},
	{0xb6a,  {0x0, 0x0, 0x3f, 0xff}},
	{0xbd0,  {0x0, 0x3, 0xbf, 0xff}},
	{0xc00,  {0x1, 0xff, 0xf, 0xef}},
	{0xc30,  {0x0, 0x3f, 0x7f, 0xff}},
	{0xd00,  {0x7f, 0xfe, 0xff, 0xff}},
	{0xd20,  {0x0, 0xff, 0x0, 0xf}},
	{0xd40,  {0x80, 0x0, 0x0, 0x23}},
	{0xd6a,  {0x0, 0x0, 0x3f, 0xff}},
	{0xdb0,  {0x0, 0x0, 0xff, 0xff}},
	{0xdd0,  {0x0, 0x3, 0xbf, 0xff}},
	{0xf00,  {0x7f, 0xf0, 0x3f, 0xff}},
	{0xf1f,  {0x0, 0x0, 0x0, 0x1}},
	{0xfa6,  {0x0, 0x0, 0x0, 0x3}},
	{0xfea,  {0x0, 0x30, 0xff, 0xff}},
};

phyradregs_list_t dot11acphy_regs_rev36[] = {
	{0x000,  {0x7f, 0xff, 0xfe, 0x9f}},
	{0x01f,  {0x0, 0x2, 0x3f, 0xfb}},
	{0x040,  {0x0, 0x3, 0xfe, 0x7f}},
	{0x060,  {0x1f, 0xff, 0x0, 0x1f}},
	{0x080,  {0x0, 0x0, 0xf, 0xff}},
	{0x0a2,  {0x0, 0xff, 0xc0, 0x7f}},
	{0x100,  {0x40, 0x0, 0x3, 0xff}},
	{0x11f,  {0xf, 0xfe, 0x0, 0x23}},
	{0x140,  {0x7d, 0xaf, 0xff, 0xff}},
	{0x15f,  {0x7, 0xff, 0xbf, 0xff}},
	{0x17e,  {0x7f, 0x80, 0x0, 0x7}},
	{0x19d,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x1bc,  {0x7f, 0xfd, 0xc0, 0xff}},
	{0x1db,  {0x80, 0x0, 0x0, 0x29}},
	{0x210,  {0x0, 0x0, 0x3, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x1, 0xff, 0xfb, 0xff}},
	{0x270,  {0x7f, 0xff, 0x0, 0x3f}},
	{0x28f,  {0x0, 0x1f, 0xff, 0xff}},
	{0x2b0,  {0x0, 0x1, 0x3f, 0xff}},
	{0x2d0,  {0x7d, 0xff, 0xef, 0xff}},
	{0x2ef,  {0x7f, 0xff, 0x87, 0xff}},
	{0x30e,  {0x7f, 0xfb, 0xff, 0xff}},
	{0x32d,  {0x3f, 0xff, 0xff, 0xff}},
	{0x34d,  {0x1e, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0xf, 0x0, 0xff}},
	{0x390,  {0x7f, 0xfe, 0x17, 0xff}},
	{0x3af,  {0x80, 0x0, 0x0, 0x44}},
	{0x400,  {0x3f, 0xff, 0xff, 0xff}},
	{0x420,  {0x7f, 0xff, 0x3e, 0x7f}},
	{0x442,  {0x80, 0x0, 0x0, 0x42}},
	{0x490,  {0x0, 0x0, 0x20, 0x41}},
	{0x4b8,  {0x40, 0x0, 0x0, 0x1}},
	{0x4f4,  {0x10, 0x0, 0x70, 0x77}},
	{0x520,  {0x0, 0x0, 0xf, 0xff}},
	{0x550,  {0x0, 0x0, 0xf, 0xff}},
	{0x580,  {0x3f, 0xff, 0xff, 0xfd}},
	{0x5a0,  {0x80, 0x0, 0x0, 0x2b}},
	{0x5d0,  {0x0, 0x0, 0x3, 0xff}},
	{0x600,  {0x0, 0xff, 0xff, 0xff}},
	{0x620,  {0x0, 0x0, 0x0, 0x1f}},
	{0x640,  {0x0, 0x0, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0x0, 0x27}},
	{0x67f,  {0x7e, 0xfe, 0x0, 0x7f}},
	{0x69e,  {0x7f, 0xff, 0xff, 0xff}},
	{0x6c0,  {0x7f, 0xff, 0xf, 0xff}},
	{0x6df,  {0x7f, 0x3f, 0xff, 0xff}},
	{0x6fe,  {0x0, 0x3c, 0x0, 0x3}},
	{0x720,  {0x7f, 0xff, 0x3, 0xff}},
	{0x741,  {0x0, 0xff, 0xff, 0xff}},
	{0x767,  {0x7e, 0xf8, 0x0, 0x5}},
	{0x786,  {0x7c, 0xf, 0xff, 0xff}},
	{0x7a5,  {0x7f, 0xf3, 0x39, 0xff}},
	{0x7c4,  {0x73, 0xff, 0xff, 0xff}},
	{0x7e3,  {0x7, 0xfc, 0x0, 0xff}},
	{0x805,  {0x0, 0x0, 0x0, 0xf}},
	{0x830,  {0x0, 0x0, 0xff, 0xff}},
	{0x850,  {0x0, 0x0, 0x3f, 0xff}},
	{0x900,  {0x0, 0x0, 0x0, 0x3}},
	{0xb03,  {0x0, 0x0, 0x3, 0xff}},
	{0xb6a,  {0x0, 0x0, 0x2, 0x5b}},
	{0xbc0,  {0x3f, 0xff, 0xff, 0xff}},
	{0xbdf,  {0x7f, 0xfe, 0x0, 0x7}},
	{0xbfe,  {0x7, 0xfc, 0x3f, 0xbf}},
	{0xc20,  {0x7f, 0xff, 0xff, 0xff}},
	{0xc40,  {0x7f, 0xff, 0x0, 0x3f}},
	{0xc5f,  {0x7, 0xfe, 0xff, 0xff}},
	{0xc80,  {0x7f, 0xff, 0x0, 0xff}},
	{0xc9f,  {0x0, 0x0, 0x0, 0x3d}},
	{0xd00,  {0x7f, 0xfe, 0x0, 0x7}},
	{0xd1f,  {0x1, 0xfe, 0x0, 0x1f}},
	{0xd40,  {0x0, 0xff, 0xff, 0xff}},
	{0xdb0,  {0x0, 0x7, 0xff, 0xff}},
};

phyradregs_list_t dot11acphy_regs_rev40_main[] = {
	{0x000,  {0x7f, 0xff, 0xfe, 0xbf}},
	{0x01f,  {0x7e, 0x3, 0xff, 0xff}},
	{0x03e,  {0x7f, 0xf7, 0xff, 0xff}},
	{0x05d,  {0x7f, 0xf8, 0x7, 0xf9}},
	{0x07c,  {0x7f, 0xff, 0xff, 0xf3}},
	{0x09b,  {0x7f, 0xe0, 0x3f, 0xff}},
	{0x0ba,  {0x80, 0x0, 0x0, 0x53}},
	{0x120,  {0xf, 0xff, 0x7, 0xff}},
	{0x140,  {0x7f, 0xff, 0xff, 0xff}},
	{0x160,  {0x73, 0xff, 0xff, 0xff}},
	{0x17f,  {0x7f, 0xc0, 0x0, 0x3}},
	{0x19e,  {0x7f, 0xff, 0x3d, 0xff}},
	{0x1bd,  {0x80, 0x0, 0x0, 0x50}},
	{0x210,  {0x0, 0x0, 0x7f, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x80, 0x0, 0x0, 0x2c}},
	{0x280,  {0x80, 0x0, 0x0, 0x3f}},
	{0x2c0,  {0x80, 0x0, 0x0, 0x29}},
	{0x2ee,  {0x0, 0x0, 0x0, 0xf}},
	{0x30e,  {0x7f, 0xfc, 0x0, 0x3f}},
	{0x32d,  {0x20, 0x0, 0x1f, 0xff}},
	{0x34d,  {0x7, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0x3f, 0x3, 0xff}},
	{0x390,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x3af,  {0x80, 0x0, 0x0, 0x4b}},
	{0x400,  {0x80, 0x0, 0x0, 0x27}},
	{0x429,  {0x7f, 0xff, 0xff, 0x9f}},
	{0x448,  {0x80, 0x0, 0x0, 0x2d}},
	{0x476,  {0x80, 0x0, 0x0, 0x23}},
	{0x49b,  {0x30, 0x0, 0x0, 0x3f}},
	{0x4d6,  {0x70, 0x1, 0xff, 0xff}},
	{0x4f5,  {0x8, 0x0, 0x39, 0xff}},
	{0x520,  {0x7f, 0xff, 0x1f, 0xff}},
	{0x53f,  {0x1f, 0xfe, 0x0, 0x7f}},
	{0x570,  {0x7f, 0xff, 0xff, 0x1f}},
	{0x58f,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x5ae,  {0x3f, 0xfc, 0x0, 0xff}},
	{0x5d0,  {0x7f, 0xf9, 0x3, 0xff}},
	{0x5ef,  {0x7f, 0xfe, 0x3, 0xff}},
	{0x60e,  {0x0, 0x0, 0x0, 0x3}},
	{0x63f,  {0x0, 0x1, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0xc, 0x7}},
	{0x67f,  {0x7e, 0xfe, 0xe, 0x3f}},
	{0x69e,  {0x77, 0xff, 0xff, 0xff}},
	{0x6bd,  {0x7f, 0xf9, 0xff, 0xf9}},
	{0x6dc,  {0x80, 0x0, 0x0, 0x32}},
	{0x710,  {0x3, 0xff, 0xff, 0xff}},
	{0x730,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x74f,  {0x7, 0xff, 0xff, 0xff}},
	{0x770,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x78f,  {0x7f, 0xfe, 0x47, 0xff}},
	{0x7ae,  {0x80, 0x0, 0x0, 0x32}},
	{0x7e2,  {0x7f, 0xff, 0xc0, 0x1}},
	{0x801,  {0x0, 0x0, 0x7f, 0xff}},
	{0x83f,  {0x0, 0x1, 0xff, 0xff}},
	{0x860,  {0x7f, 0xff, 0xc, 0x7}},
	{0x87f,  {0x7e, 0xfe, 0xe, 0x3f}},
	{0x89e,  {0x77, 0xff, 0xff, 0xff}},
	{0x8bd,  {0x7f, 0xf9, 0xff, 0xf9}},
	{0x8dc,  {0x80, 0x0, 0x0, 0x32}},
	{0x910,  {0x3, 0xff, 0xff, 0xff}},
	{0x930,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x94f,  {0x7, 0xff, 0xff, 0xff}},
	{0x970,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x98f,  {0x7f, 0xfe, 0x47, 0xff}},
	{0x9ae,  {0x80, 0x0, 0x0, 0x32}},
	{0x9e2,  {0x3f, 0xff, 0xc0, 0x1}},
	{0xe50,  {0x0, 0x0, 0x3, 0xfd}},
	{0x1000,  {0x0, 0x0, 0xff, 0xff}},
	{0x1020,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x103f,  {0x0, 0xf, 0xff, 0xff}},
	{0x1060,  {0x0, 0x1f, 0x1, 0xff}},
	{0x1081,  {0x7f, 0xff, 0xbf, 0xff}},
	{0x10a0,  {0x7f, 0xff, 0x0, 0xff}},
	{0x10bf,  {0x80, 0x0, 0x0, 0x34}},
	{0x10f4,  {0x80, 0x0, 0x0, 0x27}},
	{0x111c,  {0x77, 0x7f, 0xff, 0xf3}},
	{0x113b,  {0x7f, 0xff, 0xff, 0xe3}},
	{0x115a,  {0x4f, 0xff, 0xff, 0xff}},
	{0x1179,  {0x0, 0x0, 0x0, 0x7f}},
	{0x11b0,  {0x0, 0x7, 0x3, 0xff}},
	{0x11d0,  {0x7f, 0xff, 0x0, 0x7f}},
	{0x11ef,  {0x0, 0x3e, 0x0, 0x3f}},
	{0x1220,  {0xf, 0xff, 0xff, 0xff}},
	{0x1240,  {0x3e, 0x0, 0x1f, 0xff}},
	{0x1260,  {0x0, 0x0, 0x0, 0x1f}},
	{0x1280,  {0x7f, 0xff, 0xbf, 0xff}},
	{0x129f,  {0x80, 0x0, 0x0, 0x51}},
	{0x1620,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1640,  {0x0, 0x7f, 0x1, 0xff}},
	{0x1660,  {0x80, 0x0, 0x0, 0x3c}},
	{0x16a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x16c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x16df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x16fe,  {0x1, 0xff, 0xff, 0xff}},
	{0x1720,  {0x7, 0xff, 0xff, 0xff}},
	{0x1740,  {0x0, 0xff, 0xff, 0xff}},
	{0x1760,  {0x1, 0xff, 0x0, 0x3f}},
	{0x17a0,  {0x0, 0x0, 0x0, 0x1f}},
	{0x1800,  {0x0, 0xf, 0xff, 0xe7}},
	{0x1820,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1840,  {0x0, 0x7f, 0x1, 0xff}},
	{0x1860,  {0x80, 0x0, 0x0, 0x3c}},
	{0x18a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x18c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x18df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x18fe,  {0x1, 0xff, 0xff, 0xff}},
	{0x1920,  {0x7, 0xff, 0xff, 0xff}},
	{0x1940,  {0x0, 0xff, 0xff, 0xff}},
	{0x1960,  {0x1, 0xff, 0x0, 0x3f}},
	{0x1980,  {0x0, 0x0, 0x3f, 0xff}},
	{0x19a0,  {0x0, 0x0, 0x0, 0x1f}},
};

phyradregs_list_t dot11acphy_regs_rev40_aux[] = {
	{0x000,  {0x7f, 0xff, 0xfe, 0xbf}},
	{0x01f,  {0x7e, 0x3, 0xff, 0xff}},
	{0x03e,  {0x7f, 0xf7, 0xff, 0xff}},
	{0x05d,  {0x7f, 0xf8, 0x7, 0xf9}},
	{0x07c,  {0x0, 0x0, 0xff, 0xf3}},
	{0x0a2,  {0x0, 0xff, 0xc0, 0x7f}},
	{0x0ec,  {0x40, 0x0, 0x3, 0xff}},
	{0x10b,  {0x7f, 0xe0, 0x0, 0x3}},
	{0x12a,  {0x7f, 0xc3, 0xff, 0xc1}},
	{0x149,  {0x7f, 0xbf, 0xff, 0xff}},
	{0x168,  {0x1, 0xf3, 0xff, 0xff}},
	{0x195,  {0x7e, 0x7b, 0xff, 0xff}},
	{0x1b4,  {0x80, 0x0, 0x0, 0x59}},
	{0x210,  {0x0, 0x0, 0x7f, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x80, 0x0, 0x0, 0x2c}},
	{0x280,  {0x80, 0x0, 0x0, 0x3f}},
	{0x2c0,  {0x80, 0x0, 0x0, 0x29}},
	{0x2ee,  {0x0, 0x0, 0x0, 0xf}},
	{0x30e,  {0x1, 0xc4, 0x0, 0x3f}},
	{0x330,  {0x64, 0x0, 0x2, 0x7}},
	{0x34f,  {0x1, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0x3f, 0x3, 0xff}},
	{0x390,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x3af,  {0x80, 0x0, 0x0, 0x4b}},
	{0x400,  {0x80, 0x0, 0x0, 0x27}},
	{0x429,  {0x7f, 0xff, 0xff, 0x9f}},
	{0x448,  {0x80, 0x0, 0x0, 0x2d}},
	{0x476,  {0x80, 0x0, 0x0, 0x23}},
	{0x49b,  {0x30, 0x0, 0x0, 0x3f}},
	{0x4d6,  {0x70, 0x0, 0x88, 0x61}},
	{0x4f5,  {0x8, 0x0, 0x39, 0xff}},
	{0x520,  {0x7f, 0xff, 0x1f, 0xff}},
	{0x53f,  {0x1f, 0xfe, 0x0, 0x7f}},
	{0x570,  {0x7f, 0xff, 0xff, 0x1f}},
	{0x58f,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x5ae,  {0x3f, 0xfc, 0x0, 0xff}},
	{0x5d0,  {0x7f, 0xf9, 0x3, 0xff}},
	{0x5ef,  {0x7f, 0xfe, 0x3, 0xff}},
	{0x60e,  {0x0, 0x0, 0x0, 0x3}},
	{0x63f,  {0x0, 0x1, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0xc, 0x7}},
	{0x67f,  {0x7e, 0xfe, 0xe, 0x3f}},
	{0x69e,  {0x77, 0xff, 0xff, 0xff}},
	{0x6bd,  {0x7f, 0xf9, 0xff, 0xf9}},
	{0x6dc,  {0x80, 0x0, 0x0, 0x32}},
	{0x710,  {0x3, 0xff, 0xff, 0xf5}},
	{0x730,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x74f,  {0x7, 0xff, 0xff, 0xff}},
	{0x770,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x78f,  {0x7f, 0xfe, 0x47, 0xff}},
	{0x7ae,  {0x80, 0x0, 0x0, 0x32}},
	{0x7e2,  {0x7f, 0xff, 0xc0, 0x1}},
	{0x801,  {0x0, 0x0, 0x7f, 0xff}},
	{0x83f,  {0x0, 0x1, 0xff, 0xff}},
	{0x860,  {0x7f, 0xff, 0xc, 0x7}},
	{0x87f,  {0x7e, 0xfe, 0xe, 0x3f}},
	{0x89e,  {0x77, 0xff, 0xff, 0xff}},
	{0x8bd,  {0x7f, 0xf9, 0xff, 0xf9}},
	{0x8dc,  {0x80, 0x0, 0x0, 0x32}},
	{0x910,  {0x3, 0xff, 0xff, 0xf5}},
	{0x930,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x94f,  {0x7, 0xff, 0xff, 0xff}},
	{0x970,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x98f,  {0x7f, 0xfe, 0x47, 0xff}},
	{0x9ae,  {0x80, 0x0, 0x0, 0x32}},
	{0x9e2,  {0x3f, 0xff, 0xc0, 0x1}},
	{0xe50,  {0x0, 0x0, 0x3, 0xfd}},
	{0x1000,  {0x0, 0x0, 0xff, 0xff}},
	{0x1020,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x103f,  {0x0, 0xf, 0xff, 0xff}},
	{0x1060,  {0x0, 0x1f, 0x1, 0xff}},
	{0x1081,  {0x7f, 0xff, 0xbf, 0xff}},
	{0x10a0,  {0x7f, 0xff, 0x0, 0xef}},
	{0x10bf,  {0x7f, 0xff, 0xff, 0xf1}},
	{0x10de,  {0x7f, 0xdf, 0xff, 0xff}},
	{0x10fd,  {0x3f, 0xff, 0xff, 0xff}},
	{0x111c,  {0x77, 0x7f, 0xff, 0xf3}},
	{0x113b,  {0x7f, 0xff, 0xff, 0xe3}},
	{0x115a,  {0x4f, 0xff, 0xff, 0xff}},
	{0x1179,  {0x0, 0x0, 0x0, 0x7f}},
	{0x11b0,  {0x0, 0x7, 0x3, 0xff}},
	{0x11d0,  {0x7f, 0xff, 0x0, 0x7f}},
	{0x11ef,  {0x0, 0x2, 0x0, 0x3f}},
	{0x1220,  {0xf, 0xff, 0xff, 0xff}},
	{0x1240,  {0x3e, 0x0, 0x1f, 0xf}},
	{0x1260,  {0x0, 0x0, 0x0, 0x1f}},
	{0x1280,  {0x3, 0xff, 0xbf, 0xff}},
	{0x12cc,  {0x40, 0x0, 0x3, 0xff}},
	{0x12eb,  {0x0, 0x0, 0x0, 0x1f}},
	{0x1620,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1640,  {0x0, 0x7f, 0x0, 0x3f}},
	{0x1660,  {0x3f, 0xff, 0xff, 0xff}},
	{0x1680,  {0xf, 0xff, 0xff, 0xff}},
	{0x16a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x16c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x16df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x16fe,  {0x1, 0xff, 0xff, 0xff}},
	{0x1720,  {0x7, 0xff, 0xff, 0xff}},
	{0x1740,  {0x0, 0xff, 0xff, 0xff}},
	{0x1760,  {0x1, 0xff, 0x0, 0x3f}},
	{0x17a0,  {0x0, 0x0, 0x0, 0x1f}},
	{0x1800,  {0x0, 0xf, 0xff, 0xe7}},
	{0x1820,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1840,  {0x0, 0x7f, 0x0, 0x3f}},
	{0x1860,  {0x3f, 0xff, 0xff, 0xff}},
	{0x1880,  {0xf, 0xff, 0xff, 0xff}},
	{0x18a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x18c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x18df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x18fe,  {0x1, 0xff, 0xff, 0xff}},
	{0x1920,  {0x7, 0xff, 0xff, 0xff}},
	{0x1940,  {0x0, 0xff, 0xff, 0xff}},
	{0x1960,  {0x1, 0xff, 0x0, 0x3f}},
	{0x1980,  {0x0, 0x0, 0x3f, 0xff}},
	{0x19a0,  {0x0, 0x0, 0x0, 0x1f}},
};
phyradregs_list_t dot11acphy_regs_rev44_main[] = {
	{0x000,  {0x7f, 0xff, 0xfe, 0xbf}},
	{0x01f,  {0x7e, 0x7f, 0xff, 0xff}},
	{0x03e,  {0x7f, 0xf7, 0xff, 0xff}},
	{0x05d,  {0x7f, 0xf8, 0xf, 0xff}},
	{0x07c,  {0x7f, 0xff, 0xff, 0xfb}},
	{0x09b,  {0x7f, 0xe7, 0xff, 0xff}},
	{0x0ba,  {0x80, 0x0, 0x0, 0x53}},
	{0x110,  {0x7, 0xff, 0x0, 0x3}},
	{0x130,  {0x7f, 0xff, 0xf, 0xff}},
	{0x14f,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x16e,  {0x80, 0x0, 0x0, 0xb}},
	{0x17c,  {0x80, 0x0, 0x0, 0x4}},
	{0x195,  {0x80, 0x0, 0x0, 0x15}},
	{0x1ae,  {0x80, 0x0, 0x0, 0x5f}},
	{0x210,  {0x0, 0x0, 0x7f, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x80, 0x0, 0x0, 0x2e}},
	{0x280,  {0x80, 0x0, 0x0, 0x6a}},
	{0x2ee,  {0x0, 0x7c, 0xf, 0xff}},
	{0x30e,  {0x7f, 0xfc, 0x0, 0x3f}},
	{0x32d,  {0x20, 0x0, 0x1f, 0xff}},
	{0x34d,  {0x7, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0x3f, 0x7f, 0xff}},
	{0x390,  {0x0, 0x0, 0x7f, 0xff}},
	{0x400,  {0x80, 0x0, 0x0, 0x2f}},
	{0x430,  {0x80, 0x0, 0x0, 0x45}},
	{0x476,  {0x80, 0x0, 0x0, 0x23}},
	{0x49b,  {0x30, 0x0, 0x0, 0x3f}},
	{0x4d6,  {0x70, 0x1, 0xff, 0xff}},
	{0x4f5,  {0x8, 0x0, 0x39, 0xff}},
	{0x520,  {0x7f, 0xff, 0x1f, 0xff}},
	{0x53f,  {0x7f, 0xfe, 0x0, 0x7f}},
	{0x55e,  {0x7c, 0x7c, 0x0, 0x1}},
	{0x57d,  {0x80, 0x0, 0x0, 0x22}},
	{0x5a0,  {0x0, 0x3f, 0xff, 0xff}},
	{0x5c0,  {0x3, 0xff, 0xf, 0xff}},
	{0x5e0,  {0x1, 0xff, 0xff, 0xf9}},
	{0x600,  {0x0, 0x0, 0xff, 0xff}},
	{0x63f,  {0x0, 0xd, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0xc, 0x27}},
	{0x67f,  {0x7e, 0xfe, 0xe, 0x7f}},
	{0x69e,  {0x77, 0xff, 0xff, 0xff}},
	{0x6bd,  {0x80, 0x0, 0x0, 0x51}},
	{0x710,  {0x3, 0xff, 0xff, 0xff}},
	{0x730,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x74f,  {0x7, 0xff, 0xff, 0xff}},
	{0x76f,  {0x7f, 0xff, 0x7f, 0xff}},
	{0x78e,  {0x7f, 0xfc, 0x8f, 0xff}},
	{0x7ad,  {0x80, 0x0, 0x0, 0x33}},
	{0x7f0,  {0x80, 0x0, 0x0, 0x20}},
	{0x83f,  {0x0, 0xd, 0xff, 0xff}},
	{0x860,  {0x7f, 0xff, 0xc, 0x27}},
	{0x87f,  {0x7e, 0xfe, 0xe, 0x7f}},
	{0x89e,  {0x77, 0xff, 0xff, 0xff}},
	{0x8bd,  {0x80, 0x0, 0x0, 0x51}},
	{0x910,  {0x3, 0xff, 0xff, 0xff}},
	{0x930,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x94f,  {0x7, 0xff, 0xff, 0xff}},
	{0x96f,  {0x7f, 0xff, 0x7f, 0xff}},
	{0x98e,  {0x7f, 0xfc, 0x8f, 0xff}},
	{0x9ad,  {0x80, 0x0, 0x0, 0x33}},
	{0x9f0,  {0x0, 0x0, 0xff, 0xff}},
	{0xe50,  {0x0, 0x0, 0x3, 0xfd}},
	{0xeff,  {0x0, 0x0, 0x0, 0x1}},
	{0x1000,  {0x0, 0x0, 0xff, 0xff}},
	{0x1020,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x103f,  {0x0, 0x0, 0x3, 0xff}},
	{0x1060,  {0x7f, 0xff, 0x3, 0xff}},
	{0x1081,  {0x0, 0x7, 0xbf, 0xff}},
	{0x10a0,  {0x7f, 0xff, 0x0, 0xff}},
	{0x10bf,  {0x80, 0x0, 0x0, 0x20}},
	{0x10f4,  {0x80, 0x0, 0x0, 0x1b}},
	{0x111c,  {0x77, 0x7f, 0xff, 0xf3}},
	{0x113b,  {0x7f, 0xff, 0xff, 0xf3}},
	{0x115a,  {0x4f, 0xff, 0xff, 0xff}},
	{0x1179,  {0x0, 0x0, 0x0, 0x7f}},
	{0x11b0,  {0x0, 0xf, 0x3, 0xff}},
	{0x11d0,  {0x7f, 0xff, 0xf, 0xff}},
	{0x11ef,  {0x0, 0x3e, 0x6, 0x3f}},
	{0x1259,  {0x7f, 0xf0, 0xff, 0x9f}},
	{0x1278,  {0x7f, 0xbf, 0xff, 0x3f}},
	{0x1297,  {0x80, 0x0, 0x0, 0x59}},
	{0x1300,  {0x80, 0x0, 0x0, 0x39}},
	{0x1340,  {0x7, 0xff, 0x3, 0x3f}},
	{0x1620,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1640,  {0x0, 0x7f, 0x1, 0xff}},
	{0x1660,  {0x80, 0x0, 0x0, 0x3c}},
	{0x16a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x16c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x16df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x16fe,  {0x3, 0xff, 0xff, 0xff}},
	{0x1720,  {0x7, 0xff, 0xff, 0xff}},
	{0x1740,  {0x0, 0xff, 0xff, 0xff}},
	{0x1760,  {0x1, 0xff, 0x0, 0x3f}},
	{0x17a0,  {0x7f, 0xff, 0x0, 0x0}},
	{0x17bf,  {0x7, 0xfe, 0xff, 0xff}},
	{0x1800,  {0x0, 0xf, 0xff, 0xe7}},
	{0x1820,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1840,  {0x0, 0x7f, 0x1, 0xff}},
	{0x1860,  {0x80, 0x0, 0x0, 0x3c}},
	{0x18a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x18c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x18df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x18fe,  {0x3, 0xff, 0xff, 0xff}},
	{0x1920,  {0x7, 0xff, 0xff, 0xff}},
	{0x1940,  {0x0, 0xff, 0xff, 0xff}},
	{0x1960,  {0x1, 0xff, 0x0, 0x3f}},
	{0x1980,  {0x1f, 0xff, 0x3f, 0xff}},
	{0x19a0,  {0x7f, 0xff, 0x0, 0x0}},
	{0x19bf,  {0x3, 0xfe, 0xff, 0xff}},
};

phyradregs_list_t dot11acphy_regs_rev44_aux[] = {
	{0x000,  {0x7f, 0xff, 0xfe, 0xbf}},
	{0x01f,  {0x7e, 0x7f, 0xff, 0xff}},
	{0x03e,  {0x7f, 0xf7, 0xff, 0xff}},
	{0x05d,  {0x7f, 0xf8, 0xf, 0xff}},
	{0x07c,  {0x0, 0x0, 0xff, 0xfb}},
	{0x0a2,  {0x0, 0xff, 0xcf, 0xff}},
	{0x0ec,  {0x40, 0x0, 0x3, 0xff}},
	{0x10b,  {0x7f, 0xe0, 0x0, 0x63}},
	{0x12a,  {0x7f, 0xc3, 0xff, 0xc1}},
	{0x149,  {0x7f, 0xbf, 0xff, 0xff}},
	{0x168,  {0x80, 0x0, 0x0, 0x12}},
	{0x17c,  {0x80, 0x0, 0x0, 0x5}},
	{0x195,  {0x80, 0x0, 0x0, 0x16}},
	{0x1ae,  {0x80, 0x0, 0x0, 0x5f}},
	{0x210,  {0x0, 0x0, 0x7f, 0x7}},
	{0x230,  {0x7, 0xff, 0x3, 0xff}},
	{0x250,  {0x80, 0x0, 0x0, 0x2e}},
	{0x280,  {0x80, 0x0, 0x0, 0x6a}},
	{0x2ee,  {0x0, 0x7c, 0xf, 0xff}},
	{0x30e,  {0x1, 0xc4, 0x0, 0x3f}},
	{0x330,  {0x64, 0x0, 0x2, 0x7}},
	{0x34f,  {0x1, 0xff, 0xff, 0xff}},
	{0x370,  {0x0, 0x3f, 0x7f, 0xff}},
	{0x390,  {0x7f, 0xfe, 0x7f, 0xff}},
	{0x3af,  {0x80, 0x0, 0x0, 0x50}},
	{0x400,  {0x80, 0x0, 0x0, 0x2f}},
	{0x430,  {0x80, 0x0, 0x0, 0x45}},
	{0x476,  {0x80, 0x0, 0x0, 0x23}},
	{0x49b,  {0x30, 0x0, 0x0, 0x3f}},
	{0x4d6,  {0x70, 0x0, 0x88, 0x61}},
	{0x4f5,  {0x8, 0x0, 0x39, 0xff}},
	{0x520,  {0x7f, 0xff, 0x1f, 0xff}},
	{0x53f,  {0x7f, 0xfe, 0x0, 0x7f}},
	{0x55e,  {0x7c, 0x7c, 0x0, 0x1}},
	{0x57d,  {0x80, 0x0, 0x0, 0x22}},
	{0x5a0,  {0x0, 0x3f, 0xff, 0xff}},
	{0x5c0,  {0x3, 0xff, 0xf, 0xff}},
	{0x5e0,  {0x1, 0xff, 0xff, 0xf9}},
	{0x600,  {0x0, 0x0, 0xff, 0xff}},
	{0x63f,  {0x0, 0xd, 0xff, 0xff}},
	{0x660,  {0x7f, 0xff, 0xc, 0x27}},
	{0x67f,  {0x7e, 0xfe, 0xe, 0x7f}},
	{0x69e,  {0x77, 0xff, 0xff, 0xff}},
	{0x6bd,  {0x80, 0x0, 0x0, 0x51}},
	{0x710,  {0x3, 0xff, 0xff, 0xf5}},
	{0x730,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x74f,  {0x7, 0xff, 0xff, 0xff}},
	{0x76f,  {0x7f, 0xff, 0x7f, 0xff}},
	{0x78e,  {0x7f, 0xfc, 0xf, 0xff}},
	{0x7ad,  {0x80, 0x0, 0x0, 0x33}},
	{0x7f0,  {0x80, 0x0, 0x0, 0x20}},
	{0x83f,  {0x0, 0xd, 0xff, 0xff}},
	{0x860,  {0x7f, 0xff, 0xc, 0x27}},
	{0x87f,  {0x7e, 0xfe, 0xe, 0x7f}},
	{0x89e,  {0x77, 0xff, 0xff, 0xff}},
	{0x8bd,  {0x80, 0x0, 0x0, 0x51}},
	{0x910,  {0x3, 0xff, 0xff, 0xf5}},
	{0x930,  {0x7f, 0xfe, 0xff, 0xff}},
	{0x94f,  {0x7, 0xff, 0xff, 0xff}},
	{0x96f,  {0x7f, 0xff, 0x7f, 0xff}},
	{0x98e,  {0x7f, 0xfc, 0xf, 0xff}},
	{0x9ad,  {0x80, 0x0, 0x0, 0x33}},
	{0x9f0,  {0x0, 0x0, 0xff, 0xff}},
	{0xe50,  {0x0, 0x0, 0x3, 0xfd}},
	{0xeff,  {0x0, 0x0, 0x0, 0x1}},
	{0x1000,  {0x0, 0x0, 0xff, 0xff}},
	{0x1020,  {0x7f, 0xff, 0x3f, 0xff}},
	{0x103f,  {0x0, 0x0, 0x0, 0xff}},
	{0x1060,  {0x7f, 0xff, 0x3, 0xff}},
	{0x1081,  {0x0, 0x7, 0xbf, 0xff}},
	{0x10a0,  {0x7f, 0xff, 0x0, 0xff}},
	{0x10bf,  {0x80, 0x0, 0x0, 0x21}},
	{0x10f0,  {0x80, 0x0, 0x0, 0x3}},
	{0x10f4,  {0x80, 0x0, 0x0, 0x1c}},
	{0x111c,  {0x77, 0x7f, 0xff, 0xf3}},
	{0x113b,  {0x7f, 0xff, 0xff, 0xf3}},
	{0x115a,  {0x4f, 0xff, 0xff, 0xff}},
	{0x1179,  {0x0, 0x0, 0x0, 0x7f}},
	{0x11b0,  {0x0, 0xf, 0x3, 0xff}},
	{0x11d0,  {0x7f, 0xff, 0xf, 0xff}},
	{0x11ef,  {0x0, 0x16, 0x6, 0x3f}},
	{0x1260,  {0x0, 0x0, 0xe1, 0xff}},
	{0x1286,  {0x0, 0xf, 0xfe, 0xff}},
	{0x12cc,  {0x40, 0x0, 0x3, 0xff}},
	{0x12eb,  {0x7f, 0xe0, 0x0, 0x1f}},
	{0x130a,  {0x80, 0x0, 0x0, 0x2f}},
	{0x1340,  {0x7, 0xff, 0x3, 0x3f}},
	{0x1620,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1640,  {0x0, 0x7f, 0x0, 0x3f}},
	{0x1660,  {0x3f, 0xff, 0xff, 0xff}},
	{0x167f,  {0x1f, 0xff, 0xff, 0xff}},
	{0x16a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x16c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x16df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x16fe,  {0x3, 0xff, 0xff, 0xff}},
	{0x1720,  {0x7, 0xff, 0xff, 0xff}},
	{0x1740,  {0x0, 0xff, 0xff, 0xff}},
	{0x1760,  {0x1, 0xff, 0x0, 0x3f}},
	{0x17a0,  {0x7f, 0xff, 0x0, 0x1f}},
	{0x17bf,  {0x7, 0xfe, 0xff, 0xff}},
	{0x1800,  {0x0, 0xf, 0xff, 0xe7}},
	{0x1820,  {0x0, 0xc5, 0x1f, 0xff}},
	{0x1840,  {0x0, 0x7f, 0x0, 0x3f}},
	{0x1860,  {0x3f, 0xff, 0xff, 0xff}},
	{0x187f,  {0x1f, 0xff, 0xff, 0xff}},
	{0x18a0,  {0x1f, 0xff, 0x3, 0xff}},
	{0x18c0,  {0x43, 0xff, 0xff, 0xff}},
	{0x18df,  {0x7f, 0xfe, 0x0, 0x1}},
	{0x18fe,  {0x3, 0xff, 0xff, 0xff}},
	{0x1920,  {0x7, 0xff, 0xff, 0xff}},
	{0x1940,  {0x0, 0xff, 0xff, 0xff}},
	{0x1960,  {0x1, 0xff, 0x0, 0x3f}},
	{0x1980,  {0x1f, 0xff, 0x3f, 0xff}},
	{0x19a0,  {0x7f, 0xff, 0x0, 0x1f}},
	{0x19bf,  {0x7, 0xfe, 0xff, 0xff}},
	};

#endif /* PHY_DUMP_BINARY */
/* local functions */
static void wlc_phy_srom_read_rxgainerr_acphy(phy_info_t *pi);
static void phy_ac_misc_nvram_femctrl_read(phy_info_t *pi);
static void phy_ac_misc_nvram_femctrl_clb_read(phy_info_t *pi);

#ifdef PHY_DUMP_BINARY
static int phy_ac_misc_getlistandsize(phy_type_misc_ctx_t *ctx, phyradregs_list_t **phyreglist,
	uint16 *phyreglist_sz);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
static void phy_ac_init_test(phy_type_misc_ctx_t *ctx, bool encals);
static void phy_ac_misc_test_stop(phy_type_misc_ctx_t *ctx);
static int wlc_phy_freq_accuracy_acphy(phy_type_misc_ctx_t *ctx, int channel);
#endif // endif
static uint32 phy_ac_rx_iq_est(phy_type_misc_ctx_t *ctx, uint8 samples, uint8 antsel,
	uint8 resolution, uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                      uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type);
static void phy_ac_misc_deaf_mode(phy_type_misc_ctx_t *ctx, bool user_flag);
static void phy_ac_iovar_tx_tone(phy_type_misc_ctx_t *ctx, int32 int_val);
static void phy_ac_iovar_txlo_tone(phy_type_misc_ctx_t *ctx);
static int phy_ac_iovar_get_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 *ret_int_ptr,
	int32 int_val, int err);
static int phy_ac_iovar_set_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 int_val, int err);
static bool phy_ac_misc_get_rxgainerr(phy_type_misc_ctx_t *ctx, int16 *gainerr);
#ifdef ATE_BUILD
static void phy_ac_gpaio_gpaioconfig(phy_type_misc_ctx_t *ctx, wl_gpaio_option_t option, int core);
static void phy_type_misc_disable_ate_gpiosel(phy_type_misc_ctx_t *ctx);
#endif // endif

static void phy_ac_misc_nvram_attach(phy_ac_misc_info_t *misc_info, phy_info_t *pi);
static uint8 phy_ac_misc_get_bfe_ndp_recvstreams(phy_type_misc_ctx_t *ctx);
static void phy_update_rxldpc_acphy(phy_type_misc_ctx_t *ctx, bool ldpc);
static void phy_ac_misc_set_preamble_override(phy_type_misc_ctx_t *ctx, int8 val);
static void wlc_phy_runsamples_acphy(phy_info_t *pi, uint16 num_samps, uint8 iqmode);
static void wlc_phy_runsamples_acphy_with_counts(phy_info_t *pi, uint16 num_samps, uint8 iqmode,
	uint16 loops, uint16 wait);
static void wlc_phy_runsamples_acphy_mac_based(phy_info_t *pi, uint16 num_samps);

#ifdef WFD_PHY_LL
static void phy_ac_misc_wfdll_chan_active(phy_type_misc_ctx_t *ctx, bool chan_active);
#endif // endif

static uint16
phy_ac_misc_classifier(phy_type_misc_ctx_t *ctx, uint16 mask, uint16 val);

/* register phy type specific implementation */
phy_ac_misc_info_t *
BCMATTACHFN(phy_ac_misc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_misc_info_t *cmn_info)
{
	phy_ac_misc_info_t *misc_info;
	phy_type_misc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((misc_info = phy_malloc(pi, sizeof(phy_ac_misc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	misc_info->pi = pi;
	misc_info->aci = aci;
	misc_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = misc_info;
	fns.phy_type_misc_rx_iq_est = phy_ac_rx_iq_est;
	fns.phy_type_misc_set_deaf = phy_ac_misc_deaf_mode;
	fns.phy_type_misc_clear_deaf = phy_ac_misc_deaf_mode;
#if defined(BCMDBG) || defined(WLTEST)
	fns.phy_type_misc_test_init = phy_ac_init_test;
	fns.phy_type_misc_test_stop = phy_ac_misc_test_stop;
	fns.phy_type_misc_test_freq_accuracy = wlc_phy_freq_accuracy_acphy;
#endif // endif
#ifdef PHY_DUMP_BINARY
	fns.phy_type_misc_getlistandsize = phy_ac_misc_getlistandsize;
#endif // endif
	fns.phy_type_misc_iovar_tx_tone = phy_ac_iovar_tx_tone;
	fns.phy_type_misc_iovar_txlo_tone = phy_ac_iovar_txlo_tone;
	fns.phy_type_misc_iovar_get_rx_iq_est = phy_ac_iovar_get_rx_iq_est;
	fns.phy_type_misc_iovar_set_rx_iq_est = phy_ac_iovar_set_rx_iq_est;
#ifdef ATE_BUILD
	fns.gpaioconfig = phy_ac_gpaio_gpaioconfig;
	fns.disable_ate_gpiosel = phy_type_misc_disable_ate_gpiosel;
#endif // endif
	fns.phy_type_misc_get_rxgainerr = phy_ac_misc_get_rxgainerr;
	fns.get_bfe_ndp_recvstreams = phy_ac_misc_get_bfe_ndp_recvstreams;
	fns.set_ldpc_override = phy_update_rxldpc_acphy;
	fns.set_preamble_override = phy_ac_misc_set_preamble_override;
#ifdef WFD_PHY_LL
	fns.set_wfdll_chan_active = phy_ac_misc_wfdll_chan_active;
#endif // endif

	fns.phy_type_misc_classifier = phy_ac_misc_classifier;

	wlc_phy_srom_read_rxgainerr_acphy(pi);
	phy_ac_misc_nvram_femctrl_read(pi);

	/* pre_init to ON, register POR default setting */
	misc_info->ac_rxldpc_override = ON;

	/* Read srom params from nvram */
	phy_ac_misc_nvram_attach(misc_info, pi);

	if (phy_misc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_misc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	phy_ac_misc_nvram_femctrl_clb_read(pi);

	/* Register the required scratch buffer size, revisit this if the tone_buf size conditions
	 * has changed in wlc_phy_gen_load_samples_acphy()
	 */
	phy_cache_register_reuse_size(pi->cachei, sizeof(math_cint32) * DEFAULT_SPB_DEPTH);

	return misc_info;

	/* error handling */
fail:
	if (misc_info != NULL)
		phy_mfree(pi, misc_info, sizeof(phy_ac_misc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_misc_unregister_impl)(phy_ac_misc_info_t *misc_info)
{
	phy_info_t *pi;
	phy_misc_info_t *cmn_info;

	ASSERT(misc_info);
	pi = misc_info->pi;
	cmn_info = misc_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_misc_unregister_impl(cmn_info);

	phy_mfree(pi, misc_info, sizeof(phy_ac_misc_info_t));
}

void
wlc_phy_conditional_suspend(phy_info_t *pi, bool *suspend)
{
	/* suspend mac before any phyreg access. http://jira.broadcom.com/browse/CRWLDOT11M-2177 */
	/* Suspend MAC if haven't done so */
	*suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!(*suspend)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
}

void
wlc_phy_conditional_resume(phy_info_t *pi, bool *suspend)
{
	/* UnSuspend MAC if haven't done so */
	/* suspend mac before any phyreg access. http://jira.broadcom.com/browse/CRWLDOT11M-2177 */
		if (!(*suspend)) {
			wlapi_enable_mac(pi->sh->physhim);
		}
}

static uint32
phy_ac_rx_iq_est_res0(phy_ac_misc_info_t *info, uint32 cmplx_pwr[], uint8 extra_gain_1dB)
{
	uint32 result = 0;
	phy_info_t *pi = info->pi;
	uint8 core;
	int8 noise_dbm_ant[PHY_CORE_MAX];

	/* pi->phy_noise_win per antenna is updated inside */
	wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, extra_gain_1dB);

	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

	for (core = PHYCORENUM(pi->pubpi->phy_corenum); core >= 1; core--)
		result = (result << 8) | (noise_dbm_ant[core - 1] & 0xff);

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		for (core = 0; core < PHYCORENUM(pi->pubpi->phy_corenum); core++)
		    info->iqest[core] = noise_dbm_ant[core];
	}

	return result;
}

static void
phy_ac_rx_iq_est_res1_calrev0(phy_info_t *pi, int16 noise_dBm_ant_fine[])
{
	int8 subband_idx, core, bw_idx, tmp_range, ant;
	uint8 core_freq_segment_map;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_rssioffset_t *pi_ac_rssioffset = &pi_ac->sromi->rssioffset;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
	} else {
		bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) ? 2 :
			(CHSPEC_IS160(pi->radio_chanspec)) ? 3 :
			(CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
	}

	/* Apply nvram based offset: */
	FOREACH_CORE(pi, core) {
		/* core_freq_segment_map is only required for 80P80 mode.
		For other modes, it is ignored
		*/
		core_freq_segment_map = phy_ac_chanmgr_get_data(pi_ac->chanmgri)
			->core_freq_mapping[core];
		ant = phy_get_rsdbbrd_corenum(pi, core);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			noise_dBm_ant_fine[core] +=
			    pi_ac_rssioffset
			    ->rssi_corr_gain_delta_2g[ant][ACPHY_GAIN_DELTA_ELNA_ON][bw_idx] << 2;
			PHY_RXIQ(("In %s: rssi_gain_delta_offset for"
			          " core %d = %d (in dB) \n",
			          __FUNCTION__, core, pi_ac_rssioffset
			          ->rssi_corr_gain_delta_2g[ant]
			          [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx]));
		} else {
			tmp_range = phy_ac_chanmgr_get_chan_freq_range(pi,
					pi->radio_chanspec, core_freq_segment_map);
			if ((tmp_range > 0) && (tmp_range < 5)) {
				subband_idx = tmp_range -1;
				noise_dBm_ant_fine[core] +=
				pi_ac_rssioffset
				->rssi_corr_gain_delta_5g[ant][0][bw_idx][subband_idx] << 2;
				PHY_RXIQ(("In %s: rssi_gain_delta_offset for"
				         " core %d = %d (in dB) \n",
				         __FUNCTION__, core, pi_ac_rssioffset
				         ->rssi_corr_gain_delta_5g[ant]
				         [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx][subband_idx]));
			}
		}
	}
}

static void
phy_ac_rx_iq_est_res1_calrev1(phy_info_t *pi, int16 noise_dBm_ant_fine[], uint8 gain_correct,
	const phy_ac_rssi_data_t *rssi_data)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	int16 rssi_gain_delta_qdBm[PHY_CORE_MAX];
	int16 rssi_temp_delta_qdBm;
	uint8 core;

	bzero(rssi_gain_delta_qdBm, sizeof(rssi_gain_delta_qdBm));

	if (gain_correct == 4) {
		int16 curr_temp;
		int16 gain_temp_slope;

		/* This is absolute temp gain compensation
		 * This has to be applied only for Rudimentary AGC.
		 * For this, -g 4 option has to be given.
		 * For iqest cali and veri, we do not apply this
		 * absolute gain temp compensation.
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			gain_temp_slope = pi_ac->sromi->rxgain_tempadj_2g;
			/* 57 = 5.7 dB change in gain for 100 degrees change */
		} else {
			int8 tmp_range;

			if (ACMAJORREV_33(pi->pubpi->phy_rev) &&
					PHY_AS_80P80(pi, pi->radio_chanspec)) {
				uint8 chans[NUM_CHANS_IN_CHAN_BONDING];

				phy_ac_chanmgr_get_chan_freq_range_80p80(pi,
					pi->radio_chanspec, chans);
				tmp_range = chans[0];
			} else {
				tmp_range =  phy_ac_chanmgr_get_chan_freq_range(pi,
					pi->radio_chanspec, PRIMARY_FREQ_SEGMENT);
			}
			if (tmp_range == 2) {
				gain_temp_slope = pi_ac->sromi->rxgain_tempadj_5gl;
			} else if (tmp_range == 3) {
				gain_temp_slope = pi_ac->sromi->rxgain_tempadj_5gml;
			} else if (tmp_range == 4) {
				gain_temp_slope = pi_ac->sromi->rxgain_tempadj_5gmu;
			} else if (tmp_range == 5) {
				gain_temp_slope = pi_ac->sromi->rxgain_tempadj_5gh;
			} else {
				gain_temp_slope = 0;
			}
		}
		curr_temp = wlc_phy_tempsense_acphy(pi);

		if (curr_temp >= 0) {
			rssi_temp_delta_qdBm = (curr_temp * gain_temp_slope * 2 + 250)/500;
		} else {
			rssi_temp_delta_qdBm = (curr_temp * gain_temp_slope * 2 - 250)/500;
		}
	} else {
		/* SO, if it is here, -g 4 option was not provided.
		 * And both rssi_cal_rev and rxgaincal_rssical were TRUE.
		 * So, apply gain_cal_temp based compensation for tone
		 * calibration and verification. Hopefully for calibration
		 * the coeff are 0's and thus no compensation is applied.
		 */
		wlc_phy_tempsense_acphy(pi);
		wlc_phy_upd_gain_wrt_gain_cal_temp_phy(pi, &rssi_temp_delta_qdBm);
	}

	if (rssi_data->rssi_cal_rev == TRUE && rssi_data->rxgaincal_rssical == TRUE) {
		int8 subband_idx, bw_idx, ant;
		uint8 core_freq_segment_map;

		acphy_rssioffset_t *pi_ac_rssioffset = &pi_ac->sromi->rssioffset;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		} else {
			bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) ? 2 :
				(CHSPEC_IS160(pi->radio_chanspec)) ? 3 :
				(CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		}

		/* Apply nvram based offset: */
		FOREACH_CORE(pi, core) {
			/* core_freq_segment_map is only required for
			80P80 mode. For other modes, it is ignored
			*/
			core_freq_segment_map = phy_ac_chanmgr_get_data(pi_ac->chanmgri)
				->core_freq_mapping[core];
			ant = phy_get_rsdbbrd_corenum(pi, core);
			subband_idx = wlc_phy_rssi_get_chan_freq_range_acphy(pi,
					core_freq_segment_map, core);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rssi_gain_delta_qdBm[core] =
				pi_ac_rssioffset->rssi_corr_gain_delta_2g_sub[ant]
				[ACPHY_GAIN_DELTA_ELNA_ON][bw_idx][subband_idx];
			} else {
				if ((subband_idx >= 0) && (subband_idx < 5)) {
					rssi_gain_delta_qdBm[core] =
					pi_ac_rssioffset->rssi_corr_gain_delta_5g_sub
					[ant][ACPHY_GAIN_DELTA_ELNA_ON][bw_idx][subband_idx];
				}
			}
		}
	}

	FOREACH_CORE(pi, core) {
		noise_dBm_ant_fine[core] += rssi_temp_delta_qdBm + rssi_gain_delta_qdBm[core];
		PHY_RXIQ(("In %s: | Core %d | temp_delta_qdBm = %d (qdB)"
		         "| gain_delta_qdBm = %d (qdB) | RXIQEST = %d (qdB)\n", __FUNCTION__, core,
		         rssi_temp_delta_qdBm, rssi_gain_delta_qdBm[core],
		         noise_dBm_ant_fine[core]));
	}
}

static uint32
phy_ac_rx_iq_est_res1(phy_ac_misc_info_t *info, uint8 gain_correct,
	int16 tot_gain[], uint32 cmplx_pwr[], uint8 extra_gain_1dB)
{
	uint32 result = 0;
	phy_info_t *pi = info->pi;
	uint8 core;
	int16 noise_dBm_ant_fine[PHY_CORE_MAX];
	uint16 crsmin_pwr[PHY_CORE_MAX];
	/* Reports power in finer resolution than 1 dB (currently 0.25 dB) */
	int16 noisefloor;
	phy_ac_rssi_data_t *rssi_data = phy_ac_rssi_get_data(pi->u.pi_acphy->rssii);

	bzero((uint16 *)noise_dBm_ant_fine, sizeof(noise_dBm_ant_fine));

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev) &&
		!ACMAJORREV_37(pi->pubpi->phy_rev) &&
		!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, crsmin_pwr,
			noise_dBm_ant_fine, extra_gain_1dB, tot_gain);
	}

	/* gainerr and temperature compensation.
	 * If rssi_cal_rev is FALSE, then rssi_gain_delta is in 1 dB steps
	 * So, this has to be adj in the tot_gain which is in 1 dB steps.
	 * Can't apply 0.25 dB steps as we can't apply comp with tot_gain
	 * So, have to apply it with tempsense comp which is in 0.25dB steps
	 */
	if ((gain_correct == 3 || gain_correct == 4) && rssi_data->rssi_cal_rev == FALSE) {
		phy_ac_rx_iq_est_res1_calrev0(pi, noise_dBm_ant_fine);
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, crsmin_pwr,
			noise_dBm_ant_fine, extra_gain_1dB, tot_gain);
	}

	/* This piece of code will be executed only if resolution is 1, => qdBm steps. */
	if ((gain_correct == 4) || ((rssi_data->rssi_cal_rev == TRUE) &&
		(rssi_data->rxgaincal_rssical == TRUE))) {
		phy_ac_rx_iq_est_res1_calrev1(pi, noise_dBm_ant_fine, gain_correct, rssi_data);
	}

	if (gain_correct == 1 || gain_correct == 2 || gain_correct == 3) {
		int16 gainerr[PHY_CORE_MAX];
		int16 gain_err_temp_adj;

		wlc_phy_get_rxgainerr_phy(pi, gainerr);

		/* make and apply temperature correction */
		/* Read and (implicitly) store current temperature */
		wlc_phy_tempsense_acphy(pi);
		wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj);

		FOREACH_CORE(pi, core) {
			/* gainerr is in 0.5dB units;
			 * need to convert to 0.25dB units
			 */
			if (gain_correct == 1) {
				gainerr[core] = gainerr[core] << 1;
				/* Apply gain correction */
				noise_dBm_ant_fine[core] -= gainerr[core];
			}
			noise_dBm_ant_fine[core] += gain_err_temp_adj;
		}
	}

	if (CHSPEC_IS40(pi->radio_chanspec))
		noisefloor = 4 * ACPHY_NOISE_FLOOR_40M;
	else if (CHSPEC_BW_LE20(pi->radio_chanspec))
		noisefloor = 4 * ACPHY_NOISE_FLOOR_20M;
	else
		noisefloor = 4 * ACPHY_NOISE_FLOOR_80M;

	/* DO NOT do flooring of estimate if the Chip is 4350 AND gain correct is 0.
	 * In other words,
	 * DO flooring for ALL chips other than 4350 AND
	 * DO flooring for 4350 if gain correct is done - ie, -g is 1/2/3/4.
	 */
	if (!(ACMAJORREV_2(pi->pubpi->phy_rev) && ACMINORREV_1(pi) && (gain_correct == 0))) {
		FOREACH_CORE(pi, core) {
			if (noise_dBm_ant_fine[core] < noisefloor) {
			        noise_dBm_ant_fine[core] = noisefloor;
			}
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		for (core = 0; core < PHYCORENUM(pi->pubpi->phy_corenum); core++)
		    info->iqest[core] = noise_dBm_ant_fine[core];
	}

	for (core = PHYCORENUM(pi->pubpi->phy_corenum); core >= 1; core--) {
		result = (result << 10) | (noise_dBm_ant_fine[core - 1] & 0x3ff);
	}
	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

	return result;
}

static uint32 phy_ac_rx_iq_est(phy_type_misc_ctx_t *ctx, uint8 samples, uint8 antsel,
	uint8 resolution, uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                      uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	phy_iq_est_t est[PHY_CORE_MAX];
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int16	tot_gain[PHY_CORE_MAX];
	uint8 i, extra_gain_1dB;
	rxgain_t rxgain[PHY_CORE_MAX];
	rxgain_ovrd_t rxgain_ovrd[PHY_CORE_MAX];
	uint8 force_turnoff = 0;
	/* Local copy of phyrxchains & EnTx bits before overwrite */
	uint8 enRx = 0, enTx = 0;

	/* check if sampling is in progress */
	if (pi->phynoise_state != 0) {
		/* RB - http://wlan-rb.sj.broadcom.com/r/105244/ */
		PHY_INFORM(("%s: sampling_in_progress\n", __FUNCTION__));
		return 0;
	}

	pi->phynoise_state |= PHY_NOISE_STATE_MON;

	bzero((uint8 *)est, sizeof(est));
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((int16 *)tot_gain, sizeof(tot_gain));

	/* get IQ power measurements */

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	if ((ACMAJORREV_2((pi)->pubpi->phy_rev) && (ACMINORREV_0(pi) ||
		ACMINORREV_1(pi) || ACMINORREV_4(pi))) ||
	    ACMAJORREV_GE37((pi)->pubpi->phy_rev)) {
	    phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
		if (antsel != stf_shdata->hw_phyrxchain) {
			/* Converting core 0/1/2 to coremask 1/2/4 */
			antsel = (1 << antsel);
			/* Save and overwrite Rx chains */
			wlc_phy_update_rxchains((wlc_phy_t *)pi, &enRx, &enTx,
			    antsel, stf_shdata->phytxchain);
			force_turnoff = 1;
		}
	}
	if (force_gain_type != 0) {
		wlc_phy_get_rxgain_acphy(pi, rxgain, tot_gain, force_gain_type);
		wlc_phy_rfctrl_override_rxgain_acphy(pi, 0, rxgain, rxgain_ovrd);
		PHY_RXIQ(("%s: For gain type = %d | Total gain being applied = %d\n",
			__FUNCTION__, force_gain_type, tot_gain[0]));
	} else {
		PHY_RXIQ(("%s: For gain type = %d | Total gain being applied = %d\n",
			__FUNCTION__, force_gain_type, ACPHY_NOISE_INITGAIN));
	}

#ifdef SAMPLE_COLLECT
	if ((!TINY_RADIO(pi) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev)) && lpf_hpc) {
		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

		/* Override the LPF high pass corners to their lowest values (0x1) */
		phy_ac_lpf_hpc_override(pi_ac, TRUE);
	}
#endif /* SAMPLE_COLLECT */

	/* Overide the digital LPF */
	if (!TINY_RADIO(pi) && dig_lpf && !IS_28NM_RADIO(pi)) {
		wlc_phy_dig_lpf_override_acphy(pi, dig_lpf);
	}

	/* Increase INITgain if requested */
	if (extra_gain_3dB > 0) {
		extra_gain_3dB = wlc_phy_calc_extra_init_gain_acphy(pi, extra_gain_3dB, rxgain);

		/* Override higher INITgain if possible */
		if (extra_gain_3dB > 0) {
			wlc_phy_rfctrl_override_rxgain_acphy(pi, 0, rxgain, rxgain_ovrd);
		}
	}

	/* get IQ power measurements */
	wlc_phy_rx_iq_est_acphy(pi, est, 1 << samples, 32, wait_for_crs, FALSE);
	/* Disable the overrides if they were set */
	if (extra_gain_3dB > 0) {
		wlc_phy_rfctrl_override_rxgain_acphy(pi, 1, NULL, rxgain_ovrd);
	}

#ifdef SAMPLE_COLLECT
	if ((!TINY_RADIO(pi) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev)) && lpf_hpc) {
		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

		/* Restore LPF high pass corners to their original values */
		phy_ac_lpf_hpc_override(pi_ac, FALSE);
	}
#endif /* SAMPLE_COLLECT */

	/* Restore the digital LPF */
	if (!TINY_RADIO(pi) && dig_lpf && !IS_28NM_RADIO(pi)) {
		wlc_phy_dig_lpf_override_acphy(pi, 0);
	}

	if (force_gain_type != 0) {
		if ((force_gain_type == 4) || (force_gain_type == 3)) {
			int16	tot_gain_dummy[PHY_CORE_MAX];

			/* Restore the tr sw setting for type 4 and 3 */
			/* Type 3 may not use elna off; restore anyway */
			wlc_phy_get_rxgain_acphy(pi, rxgain, tot_gain_dummy, 6);
		}
		wlc_phy_rfctrl_override_rxgain_acphy(pi, 1, NULL, rxgain_ovrd);
	}

	/* Restore Rx chains */
	if (force_turnoff) {
		wlc_phy_restore_rxchains((wlc_phy_t *)pi, enRx, enTx);
	}

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* sum I and Q powers for each core, average over num_samps with rounding */
	ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) <= PHY_CORE_MAX);
	FOREACH_CORE(pi, i) {
		cmplx_pwr[i] = ((est[i].i_pwr + est[i].q_pwr) + (1U << (samples - 1))) >> samples;
	}

	/* convert in 1dB gain for gain adjustment */
	extra_gain_1dB = 3 * extra_gain_3dB;

	if (resolution == 0) {
		return phy_ac_rx_iq_est_res0(info, cmplx_pwr, extra_gain_1dB);
	}
	else if (resolution == 1) {
		return phy_ac_rx_iq_est_res1(info, gain_correct, tot_gain, cmplx_pwr,
		                             extra_gain_1dB);
	}

	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

	return 0;
}

static void phy_ac_iovar_tx_tone(phy_type_misc_ctx_t *ctx, int32 int_val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	pi->phy_tx_tone_freq = (int32) int_val;

	if (pi->phy_tx_tone_freq == 0) {
		wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		wlapi_enable_mac(pi->sh->physhim);
	} else {
		pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
		wlc_phy_tx_tone_acphy(pi, (int32)int_val, 151, TX_TONE_IQCAL_MODE_OFF, TRUE);
	}
}

static void
phy_ac_misc_deaf_mode(phy_type_misc_ctx_t *ctx, bool user_flag)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlc_phy_deaf_acphy(pi, user_flag);
}

static void phy_ac_iovar_txlo_tone(phy_type_misc_ctx_t *ctx)
{

	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	pi->phy_tx_tone_freq = 0;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	wlc_phy_tx_tone_acphy(pi, 0, 151, TX_TONE_IQCAL_MODE_OFF, TRUE);
}

static int phy_ac_iovar_get_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 *ret_int_ptr,
	int32 int_val, int err)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	bool suspend;
	bool low_pwr = FALSE;
	uint16 r;
	int temp_dBm;
	int16 tmp;
	void *a;

	if (!pi->sh->up) {
		err = BCME_NOTUP;
		return err;
	}

	/* make sure bt-prisel is on WLAN side */
	wlc_phy_btcx_wlan_critical_enter(pi);

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	phy_utils_phyreg_enter(pi);

	/* For 4350 Olympic program, -i 0 should behave exactly same as -i 1
	 * So, if there is force gain type is 0, then make it 1 for 4350
	 */
	if ((info->rud_agc_enable == TRUE) &&
		(pi->phy_rxiq_force_gain_type == 0) &&
		(pi->phy_rxiq_extra_gain_3dB == 0)) {
		pi->phy_rxiq_force_gain_type = 1;
	}
	/* get IQ power measurements */
	*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps, pi->phy_rxiq_antsel,
	                                 pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
	                                 pi->phy_rxiq_diglpf,
	                                 pi->phy_rxiq_gain_correct,
	                                 pi->phy_rxiq_extra_gain_3dB, 0,
	                                 pi->phy_rxiq_force_gain_type);

	if ((info->rud_agc_enable == TRUE) &&
		(pi->phy_rxiq_force_gain_type == 1) && (pi->phy_rxiq_resln == 1)) {
		uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
		BCM_REFERENCE(phyrxchain);
		FOREACH_ACTV_CORE(pi, phyrxchain, r) {
			temp_dBm = *ret_int_ptr;
			temp_dBm = (temp_dBm >> (10*r)) & 0x3ff;
			temp_dBm = ((int16)(temp_dBm << 6)) >> 6; /* sign extension */
			if ((temp_dBm >> 2) < -82) {
				low_pwr = TRUE;
			}
			PHY_RXIQ(("In %s: | For core %d | iqest_dBm = %d"
				  " \n", __FUNCTION__, r, (temp_dBm >> 2)));
		}
		if (low_pwr) {
			pi->phy_rxiq_force_gain_type = 9;
			*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps,
				pi->phy_rxiq_antsel,
				pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
				pi->phy_rxiq_diglpf, pi->phy_rxiq_gain_correct,
				pi->phy_rxiq_extra_gain_3dB, 0,
				pi->phy_rxiq_force_gain_type);
		}
	}
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (pi->phy_rxiq_resln) {
		  FOREACH_CORE(pi, r) {
		    tmp = (info->iqest[r]) & 0x3ff;
		    tmp = ((int16)(tmp << 6)) >> 6; /* sign extension */
		    info->iqest[r] = tmp;
		  }
		} else {
		  FOREACH_CORE(pi, r) {
		    tmp = (info->iqest[r]) & 0xff;
		    tmp = ((int16)(tmp << 8)) >> 8; /* sign extension */
		    info->iqest[r] = tmp;
		  }
		}
		a = (int32*)ret_int_ptr;
		bcopy(info->iqest, a, PHY_MAX_CORES*sizeof(int16));
	}
	phy_utils_phyreg_exit(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
	wlc_phy_btcx_wlan_critical_exit(pi);

	return err;
}

static int phy_ac_iovar_set_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 int_val, int err)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 samples, antenna, resolution, lpf_hpc, dig_lpf;
	uint8 gain_correct, extra_gain_3dB, force_gain_type;

	extra_gain_3dB = (int_val >> 28) & 0xf;
	gain_correct = (int_val >> 24) & 0xf;
	lpf_hpc = (int_val >> 20) & 0x3;
	dig_lpf = (int_val >> 22) & 0x3;
	resolution = (int_val >> 16) & 0xf;
	samples = (int_val >> 8) & 0xff;
	antenna = int_val & 0xf;
	force_gain_type = (int_val >> 4) & 0xf;
	if (gain_correct > 4) {
		err = BCME_RANGE;
		return err;
	}

	if ((lpf_hpc != 0) && (lpf_hpc != 1)) {
		err = BCME_RANGE;
		return err;
	}
	if (dig_lpf > 2) {
		err = BCME_RANGE;
		return err;
	}

	if ((resolution != 0) && (resolution != 1)) {
		err = BCME_RANGE;
		return err;
	}

	if (samples < 10 || samples > 15) {
		err = BCME_RANGE;
		return err;
	}

	if ((antenna != 0) &&
		(antenna != 1) &&
		(antenna != 2) &&
		(antenna != ANT_RX_DIV_DEF)) {
			err = BCME_RANGE;
			return err;
	}

	pi->phy_rxiq_samps = samples;
	pi->phy_rxiq_antsel = antenna;
	pi->phy_rxiq_resln = resolution;
	pi->phy_rxiq_lpfhpc = lpf_hpc;
	pi->phy_rxiq_diglpf = dig_lpf;
	pi->phy_rxiq_gain_correct = gain_correct;
	pi->phy_rxiq_extra_gain_3dB = extra_gain_3dB;
	pi->phy_rxiq_force_gain_type = force_gain_type;

	return err;
}

static void
BCMATTACHFN(phy_ac_misc_nvram_attach)(phy_ac_misc_info_t *misc_info, phy_info_t *pi)
{
	uint8 i;

	pi->sromi->dBpad = pi->sh->boardflags4 & BFL4_SROM12_4dBPAD;
	pi->sromi->txidxmincap2g = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txidxmincap2g, -1);
	pi->sromi->txidxmincap5g = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txidxmincap5g, -1);

	for (i = 0; i < 2; i++) {
		pi->sromi->txidxcaplow[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_txidxcaplow, i, -40);
		pi->sromi->maxepagain[i] = (uint8)PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_maxepagain, i, 0);
		pi->sromi->maxchipoutpower[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_maxchipoutpower, i, -20);
	}

	misc_info->rud_agc_enable = (bool)PHY_GETINTVAR(pi, rstr_rud_agc_enable);
}
/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* enable/disable receiving of LDPC frame */

void
wlc_phy_force_rfseq_acphy(phy_info_t *pi, uint8 cmd)
{
	uint16 trigger_mask, status_mask;
	uint16 orig_RfseqCoreActv;
	uint8 stall_val, fifo_rst_val = 0;

	if (READ_PHYREGFLD(pi, OCLControl1, ocl_mode_enable) == 1) {
		if (cmd == ACPHY_RFSEQ_RESET2RX)
			cmd = ACPHY_RFSEQ_OCL_RESET2RX;
		if (cmd == ACPHY_RFSEQ_TX2RX)
			cmd = ACPHY_RFSEQ_OCL_TX2RX;
	}
	switch (cmd) {
	case ACPHY_RFSEQ_RX2TX:
		trigger_mask = ACPHY_RfseqTrigger_rx2tx_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_rx2tx_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_TX2RX:
		trigger_mask = ACPHY_RfseqTrigger_tx2rx_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_tx2rx_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_OCL_TX2RX:
		trigger_mask = ACPHY_RfseqTrigger_ocl_tx2rx_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus_Ocl_ocl_tx2rx_MASK(pi->pubpi->phy_rev);
		break;

	case ACPHY_RFSEQ_RESET2RX:
		trigger_mask = ACPHY_RfseqTrigger_reset2rx_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_reset2rx_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_OCL_RESET2RX:
		trigger_mask = ACPHY_RfseqTrigger_ocl_reset2rx_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus_Ocl_ocl_reset2rx_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINH:
		trigger_mask = ACPHY_RfseqTrigger_updategainh_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainh_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINL:
		trigger_mask = ACPHY_RfseqTrigger_updategainl_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainl_MASK(pi->pubpi->phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINU:
		trigger_mask = ACPHY_RfseqTrigger_updategainu_MASK(pi->pubpi->phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainu_MASK(pi->pubpi->phy_rev);
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unknown cmd %d\n", pi->sh->unit, __FUNCTION__, cmd));
		return;
	}

	/* Save */
	orig_RfseqCoreActv = READ_PHYREG(pi, RfseqMode);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	fifo_rst_val = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);

	/* Force gated clocks on */
	if ((pi->pubpi->phy_rev >= 32)) {
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x6); /* set reg(PHY_CTL) 0x6 */

		/* Disable Stalls */
		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);
	}

	if (fifo_rst_val == 0)
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	if (pi->pubpi->phy_rev >= 32) {
		OSL_DELAY(1);
	}

	/* Trigger */
	phy_utils_or_phyreg(pi, ACPHY_RfseqMode(pi->pubpi->phy_rev),
		(ACPHY_RfseqMode_CoreActv_override_MASK(pi->pubpi->phy_rev) |
		ACPHY_RfseqMode_Trigger_override_MASK(pi->pubpi->phy_rev)));
	phy_utils_or_phyreg(pi, ACPHY_RfseqTrigger(pi->pubpi->phy_rev), trigger_mask);

	if ((cmd == ACPHY_RFSEQ_OCL_RESET2RX) || (cmd == ACPHY_RFSEQ_OCL_TX2RX)) {
		SPINWAIT((READ_PHYREG(pi, RfseqStatus_Ocl) & status_mask),
			ACPHY_SPINWAIT_RFSEQ_FORCE);
		if (READ_PHYREG(pi, RfseqStatus_Ocl) & status_mask) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : RFseq status OCL invalid \n",
				__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_RFSEQ_STATUS_OCL_INVALID);
		}
	} else {
		SPINWAIT((READ_PHYREG(pi, RfseqStatus0) & status_mask), ACPHY_SPINWAIT_RFSEQ_FORCE);
		if (READ_PHYREG(pi, RfseqStatus0) & status_mask) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : RFseq status invalid \n",
				__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_RFSEQ_STATUS_INVALID);
		}
	}

	/* Restore */
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, fifo_rst_val);
	if ((pi->pubpi->phy_rev >= 32)) {
		/* Undo Stalls */
		ACPHY_ENABLE_STALL(pi, stall_val);
		OSL_DELAY(1);

		/* Force gated clocks off */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x2); /* set reg(PHY_CTL) 0x2 */
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
	}
	WRITE_PHYREG(pi, RfseqMode, orig_RfseqCoreActv);

	if (ACMAJORREV_1(pi->pubpi->phy_rev))
		return;

	ASSERT((READ_PHYREG(pi, RfseqStatus0) & status_mask) == 0);
}

void
wlc_phy_deaf_acphy(phy_info_t *pi, bool mode)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if (mode) {
	  if (phy_ac_rxgcrs_get_deaf_count(pi_ac->rxgcrsi) == 0)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
		else
			PHY_ERROR(("%s: Deafness already set\n", __FUNCTION__));
	}
	else {
		if (phy_ac_rxgcrs_get_deaf_count(pi_ac->rxgcrsi) > 0)
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		else
			PHY_ERROR(("%s: Deafness already cleared\n", __FUNCTION__));
	}
	wlapi_enable_mac(pi->sh->physhim);
}

bool
wlc_phy_get_deaf_acphy(phy_info_t *pi)
{
	uint8 core;
	uint16 curr_classifctl, val;
	bool isDeaf = TRUE;
	/* Get current classifier and clip_detect settings */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	curr_classifctl = READ_PHYREG(pi, ClassifierCtrl) &
		ACPHY_ClassifierCtrl_classifierSel_MASK(pi->pubpi->phy_rev);

	if (curr_classifctl != 4) {
		isDeaf = FALSE;
	} else {
		if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
			FOREACH_CORE(pi, core) {
				val = READ_PHYREGC(pi, Clip1Threshold, core);
				if (val != 0xffff) {
					isDeaf = FALSE;
					break;
				}
			}
		}
		else {
			FOREACH_CORE(pi, core) {
				val = READ_PHYREGFLDC(pi, computeGainInfo, core,
				                      disableClip1detect);
				if (val != 1) {
					isDeaf = FALSE;
					break;
				}
			}
	        }
	}

	wlapi_enable_mac(pi->sh->physhim);
	return isDeaf;
}

void
wlc_phy_gpiosel_acphy(phy_info_t *pi, uint16 sel, uint8 word_swap)
{
	uint16 save_gpioHiOutEn;

	save_gpioHiOutEn = READ_PHYREG(pi, gpioHiOutEn);
	save_gpioHiOutEn |= 0x8000;

	/* set up acphy GPIO sel */
	WRITE_PHYREG(pi, gpioSel, (word_swap<<8) | sel);
	WRITE_PHYREG(pi, gpioHiOutEn, save_gpioHiOutEn);
}

#if defined(BCMDBG) || defined(WLTEST)
static void
phy_ac_init_test(phy_type_misc_ctx_t *ctx, bool encals)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);
	/* Disable tx power control */
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
	/* Recalibrate for this channel */
	if (encals) {
		wlc_phy_cals_acphy(pi->u.pi_acphy->calmgri, PHY_PERICAL_UNDEF,
		                   PHY_CAL_SEARCHMODE_RESTART);
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
}

#define ACPHY_TO_BPHY_OFF       0x3A1
#define ACPHY_BPHY_TEST         0x08
static void
phy_ac_misc_test_stop(phy_type_misc_ctx_t *ctx)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_REG_LIST_START
			PHY_REG_AND_RAW_ENTRY(ACPHY_TO_BPHY_OFF + ACPHY_BPHY_TEST, 0xfc00)
			PHY_REG_WRITE_RAW_ENTRY(ACPHY_bphytestcontrol(pi->pubpi->phy_rev),
				0x0)
		PHY_REG_LIST_EXECUTE(pi);
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static int
wlc_phy_freq_accuracy_acphy(phy_type_misc_ctx_t *ctx, int channel)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int bcmerror = BCME_OK;

	if (channel == 0) {
		wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
		/* restore the old BBconfig, to restore resampler setting */
		WRITE_PHYREG(pi, BBConfig, info->saved_bbconf);
		WRITE_PHYREG(pi, AfePuCtrl, info->AfePuCtrl);
		wlc_phy_resetcca_acphy(pi);
	} else {
		/* Disable the re-sampler (in case we are in spur avoidance mode) */
		info->saved_bbconf = READ_PHYREG(pi, BBConfig);
		info->AfePuCtrl = READ_PHYREG(pi, AfePuCtrl);
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, AfePuCtrl, tssiSleepEn, 0)
			MOD_PHYREG_ENTRY(pi, bphyTest, dccomp, 0)
			MOD_PHYREG_ENTRY(pi, BBConfig, resample_clk160, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
		/* use 151 since that should correspond to nominal tx output power */
		bcmerror = wlc_phy_tx_tone_acphy(pi, 0, 151, TX_TONE_IQCAL_MODE_OFF, TRUE);
	}
	return bcmerror;
}
#endif /* defined(BCMDBG) || defined(WLTEST) */

#if defined(WLTEST)
void
wlc_phy_test_scraminit_acphy(phy_info_t *pi, int8 init)
{

	if (init < 0) {
		/* auto: clear Mode bit so that scrambler LFSR will be free
		 * running.  ok to leave scramindexctlEn and initState in
		 * whatever current condition, since their contents are unused
		 * when free running.
		 */
		MOD_PHYREG(pi, ScramSigCtrl, scramCtrlMode, 0);
	} else {
		/* fixed init: set Mode bit, clear scramindexctlEn, and write
		 * init to initState, so that scrambler LFSR will be
		 * initialized with specified value for each transmission.
		 */
		MOD_PHYREG(pi, ScramSigCtrl, initStateValue, init);
		MOD_PHYREG(pi, ScramSigCtrl, scramindexctlEn, 0);
		MOD_PHYREG(pi, ScramSigCtrl, scramCtrlMode, 1);
	}
}
#endif // endif

void wlc_acphy_set_scramb_dyn_bw_en(wlc_phy_t *pih, bool enable)
{
	phy_info_t *pi = (phy_info_t *)pih;

	phy_utils_phyreg_enter(pi);
	MOD_PHYREG(pi, NsyncscramInit1, scramb_dyn_bw_en, (enable) ? 1 : 0);
	phy_utils_phyreg_exit(pi);
}

void
wlc_phy_cts2self(phy_info_t *pi, uint16 duration)
{
	int mac_depth = 0;
	while ((mac_depth < 100) && !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC)) {
		/* Unsuspend mac */
		wlapi_enable_mac(pi->sh->physhim);
		mac_depth++;
	}
	ASSERT((R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC) != 0);
	if (duration > 0)
		wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), duration);
	while (mac_depth) {
		/* Leave the mac in its original state */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		mac_depth--;
	}

	/* Make sure mac is suspended when this function is called and over */
	ASSERT((R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC) == 0);

	/* Disable Power control */
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
}

void
wlc_phy_force_rfseq_noLoleakage_acphy(phy_info_t *pi)
{
	uint8 core;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (PHY_IPA(pi)) {
		/* Turn Off iPA in override mode */
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, pa_pwrup, 0);
		}
	}
	wlc_phy_force_femreset_acphy(pi, TRUE);

	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_TX2RX);

	if (PHY_IPA(pi)) {
		/* Remove override for iPA power up */
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 0);
		}
	}
	wlc_phy_force_femreset_acphy(pi, FALSE);
}

void
wlc_phy_force_femreset_acphy(phy_info_t *pi, bool ovr)
{
	uint8 core;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (ovr) {
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			/* Force reset state by zeroing out the FEM ctrl inputs */
			WRITE_PHYREGCE(pi, RfctrlIntc, core, 0x1c00);
		}
		MOD_PHYREG(pi, AntSelConfig, AntCfg_OverrideEn, 1);
		MOD_PHYREG(pi, AntSelConfig, AntCfg_Override, 0);
	} else {
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			/* Remove overrides */
			WRITE_PHYREGCE(pi, RfctrlIntc, core, 0x0000);
		}
		MOD_PHYREG(pi, AntSelConfig, AntCfg_OverrideEn, 0);
	}
}

/* Inter-module interfaces */
static uint16
wlc_phy_gen_load_samples_acphy(phy_info_t *pi, int32 f_kHz, uint16 max_val)
{
	uint8 fs_spb;
	uint16 spb_depth = DEFAULT_SPB_DEPTH;
	uint16 num_samps, t, max_periods, num_periods = 1;
	math_fixed theta = 0, rot = 0;
	uint16 tbl_len, norm_val;
	math_cint32* tone_buf = NULL;

	if (ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
		(ACMAJORREV_44(pi->pubpi->phy_rev) && HW_ACMINORREV(pi) == 0)) {
		spb_depth = 256;
	}

	/* check phy_bw */
	if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1) {
		if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
			fs_spb = 80;
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
			fs_spb = 160;
		} else if (CHSPEC_IS80(pi->radio_chanspec) ||
		           (CHSPEC_IS8080(pi->radio_chanspec) &&
		           !ACMAJORREV_33(pi->pubpi->phy_rev)))
			fs_spb = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) ? 80 : 160;
		else if (CHSPEC_IS40(pi->radio_chanspec))
			fs_spb = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) ? 40 : 80;
		else
			fs_spb = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) ? 20 : 40;
	} else if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 2) {
		fs_spb = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) ? 80 : 160;
	} else { /* dac mode 3 */
		fs_spb = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) ? 40 : 80;
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		tbl_len = fs_spb << 1;
		if ((ACMAJORREV_47(pi->pubpi->phy_rev) && HW_ACMINORREV(pi) >= 1)) {
			if (CHSPEC_IS160(pi->radio_chanspec)) {
				if (max_val == 0) {
					norm_val = 0;
				} else {
					norm_val = ((1<<16) / max_val) & 0x3ff;
					MOD_PHYREG(pi, sampleCmd1, norm, norm_val);
				}
			}
		}
	} else {
		/* In driver, frequency (in KHz) argument to phy_tx_tone IOVAR is used to stop
		   tone transmission and not actually transmit a tone at 0 KHz offset.
	       Tone at 0 KHz offset is required for phase noise test in ATE. Hence,
	       f_Khz = -1 is treated as a special case to transmit tone at 0 Khz offset.
		*/
		if ((f_kHz == 0) || (f_kHz == -1)) {
			tbl_len = fs_spb;
			f_kHz = 0;
		} else {

			max_periods = (spb_depth * ABS(f_kHz)) / (fs_spb * 1000);
			for (t = 1; t <= max_periods; t++) {
				if (((fs_spb * 1000 * t) % ABS(f_kHz)) == 0) {
					num_periods = t;
					break;
				}
			}
			if (((fs_spb * 1000 * num_periods) % ABS(f_kHz)) != 0) {
				PHY_ERROR((
				"%s ERROR: Unable to fit tone period within table boundary\n",
				__FUNCTION__));
				PHY_ERROR(("sample play freq = %d inum_period=%d Tone Freq=%d\n",
				fs_spb, num_periods, f_kHz));
				return 0;
			}

			tbl_len = (fs_spb * 1000 * num_periods) / ABS(f_kHz);
		}
	}

	/* Acquire the memory buffer, phy_cache_register_reuse_size() is called during attach
	 * time to request the maximum possible size for tone_buf.
	 */
	tone_buf = phy_cache_acquire_reuse_buffer(pi->cachei, sizeof(*tone_buf) * tbl_len);

	if (ACMAJORREV_33(pi->pubpi->phy_rev) && PHY_AS_80P80(pi, pi->radio_chanspec)) {
		fs_spb = fs_spb << 1;
	}

	/* set up params to generate tone */
	num_samps  = (uint16)tbl_len;
	rot = FIXED((f_kHz * 36)/fs_spb) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0; /* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
		math_cmplx_cordic(theta, &tone_buf[t]);
		/* update rotation angle */
		theta += rot;
		/* produce sample values for play buffer */
		tone_buf[t].q = (int32)FLOAT(tone_buf[t].q * max_val);
		tone_buf[t].i = (int32)FLOAT(tone_buf[t].i * max_val);
		if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			if (pi->phytxtone_symm) {
			        tone_buf[t].q = 0;
			}
		}
	}
	/* load sample table */
	wlc_phy_loadsampletable_acphy(pi, tone_buf, num_samps, FALSE);

	/* Release the memory buffer */
	phy_cache_release_reuse_buffer(pi->cachei);

	return num_samps;
}

#define ACPHY_MAX_SAMPLEPLAY_BUF_LEN 512
void
wlc_phy_loadsampletable_acphy(phy_info_t *pi, math_cint32 *tone_buf, const uint16 num_samps,
        bool conj)
{
	uint16 t;
	uint32* data_buf = NULL;
	int32 sgn = 1;

	if (num_samps > ACPHY_MAX_SAMPLEPLAY_BUF_LEN) {
		PHY_FATAL_ERROR(pi, PHY_RC_SAMPLEPLAY_LIMIT);
	}

	data_buf = (uint32*)tone_buf;

	if (conj) {
		sgn = -1;
	}

	/* load samples into sample play buffer */
	for (t = 0; t < num_samps; t++) {
		data_buf[t] = ((((unsigned int)tone_buf[t].i) & 0x3ff) << 10) |
			(((unsigned int)(sgn * tone_buf[t].q)) & 0x3ff);
	}
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SAMPLEPLAY, num_samps, 0, 32, data_buf);
}

static void
phy_ac_misc_modify_bbmult(phy_ac_misc_info_t *misci, uint16 max_val, bool modify_bbmult)
{
	phy_info_t *pi = misci->pi;
	uint8 core;
	uint16 bb_mult;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (misci->bb_mult_save_valid == 0) {
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			wlc_phy_get_tx_bbmult_acphy(pi, &misci->bb_mult_save[core], core);
		}
		misci->bb_mult_save_valid = 1;
	}

	if (max_val == 0 || modify_bbmult) {
		if (max_val == 0) {
			bb_mult = 0;
		} else {
			if (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec))
				bb_mult = 64;
			else if (CHSPEC_IS160(pi->radio_chanspec))
				bb_mult = 64;
			else if (CHSPEC_IS40(pi->radio_chanspec))
				bb_mult = 64;
			else
				bb_mult = 64;
		}
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			wlc_phy_set_tx_bbmult_acphy(pi, &bb_mult, core);
		}
	}
}

/* PHY-based TX Tone */
int
wlc_phy_tx_tone_acphy(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 iqmode, bool modify_bbmult)
{
	uint16 num_samps;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		pi_ac->tx_pwr_ctrl_status = pi->txpwrctrl;
		wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
	}

	if (max_val == 0) {
		num_samps = 1;
	} else if ((num_samps = wlc_phy_gen_load_samples_acphy(pi, f_kHz, max_val)) == 0) {
		return BCME_ERROR;
	}

	phy_ac_misc_modify_bbmult(pi_ac->misci, max_val, modify_bbmult);

	if (ACMAJORREV_44(pi->pubpi->phy_rev) && (pi->pubpi->slice == DUALMAC_AUX) &&
		PHY_IPA(pi)) {
		uint16 core, mix_gm_gc, dac_attn;
		FOREACH_CORE(pi, core) {
			mix_gm_gc = READ_RADIO_REGFLD_20697X(pi, RF, TXMIX2G_CFG6, 1, core,
				mx2g_gm_gc);
			dac_attn = READ_RADIO_REGFLD_20697X(pi, RF, TXDAC_REG0, 1, core,
				iqdac_attn);
			if ((dac_attn != IPA_20697_DAC_ATTN_VAL) &&
				(mix_gm_gc > IPA_20697_MAX_MIX_GM_GC_VAL)) {
				PHY_ERROR(("%s: Invalid dac_attn(%d) or mx_gm_gc(%d) value\n",
					__FUNCTION__, dac_attn, mix_gm_gc));
				ASSERT(0);
			}
		}
	}

	if (ACMAJORREV_5(pi->pubpi->phy_rev) && ACMINORREV_0(pi) &&
	    (BFCTL(pi_ac) == 3) &&
	    (BF3_FEMCTRL_SUB(pi_ac) == 0 || BF3_FEMCTRL_SUB(pi_ac) == 3)) {
		/* 43602a0 router boards with PAVREF WAR: turn on PA */
		si_pmu_regcontrol(pi->sh->sih, 0, 0x7, 7);
	}

	wlc_phy_runsamples_acphy(pi, num_samps, iqmode);

	return BCME_OK;
}

/* MAC-based TX Tone */
int
wlc_phy_tx_tone_acphy_mac_based(phy_info_t *pi, int32 f_kHz, uint16 max_val, uint8 iqmode,
	bool modify_bbmult)
{
	uint16 num_samps;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	UNUSED_PARAMETER(iqmode);  /* to keep function prototype same as the PHY-based version */

	if (max_val == 0) {
		num_samps = 1;
	} else if ((num_samps = wlc_phy_gen_load_samples_acphy(pi, f_kHz, max_val)) == 0) {
		return BCME_ERROR;
	}

	phy_ac_misc_modify_bbmult(pi_ac->misci, max_val, modify_bbmult);

	/* Now, play the samples */
	wlc_phy_runsamples_acphy_mac_based(pi, num_samps);

	return BCME_OK;
}

void
wlc_phy_stopplayback_acphy(phy_info_t *pi, bool no_reset)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint16 playback_status, phy_ctl, SampleCollectPlayCtrl;
	uint8 mac_sample_play_on = 0;
	uint16 mask;
	uint8 stall_val;

	if (ACMAJORREV_5(pi->pubpi->phy_rev) && ACMINORREV_0(pi) &&
		(BFCTL(pi_ac) == 3) &&
		(BF3_FEMCTRL_SUB(pi_ac) == 0 || BF3_FEMCTRL_SUB(pi_ac) == 3)) {
		/* 43602a0 router boards with PAVREF WAR: turn off PA */
		si_pmu_regcontrol(pi->sh->sih, 0, 0x7, 0);
	}

	/* Find out if its a mac based sample play or phy based sample play */
	/* If its mac based sample play, unset the appropriate bits based on d11rev */
	if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
		SampleCollectPlayCtrl =
			R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
		mac_sample_play_on = (SampleCollectPlayCtrl >>
			SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT) & 1;
		if (mac_sample_play_on == 1) {
			mask = ~(1 << SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT);
			SampleCollectPlayCtrl &=  mask;
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
				SampleCollectPlayCtrl);
		}
	} else {
		phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
		mac_sample_play_on = (phy_ctl >> PHYCTRL_SAMPLEPLAYSTART_SHIFT) & 1;
		if (mac_sample_play_on == 1) {
			mask = ~(1 << PHYCTRL_SAMPLEPLAYSTART_SHIFT);
			phy_ctl &= mask;
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
		}
	}

	if (mac_sample_play_on == 0) {
		/* check status register */
		playback_status = READ_PHYREG(pi, sampleStatus);
		if (playback_status & 0x1) {
			stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
			ACPHY_DISABLE_STALL(pi);
			phy_utils_or_phyreg(pi, ACPHY_sampleCmd(pi->pubpi->phy_rev),
				ACPHY_sampleCmd_stop_MASK(pi->pubpi->phy_rev));
			ACPHY_ENABLE_STALL(pi, stall_val);
		} else if (playback_status & 0x2) {
			phy_utils_and_phyreg(pi, ACPHY_iqloCalCmdGctl(pi->pubpi->phy_rev),
				(uint16)~ACPHY_iqloCalCmdGctl_iqlo_cal_en_MASK(pi->pubpi->phy_rev));
		} else {
			PHY_CAL(("wlc_phy_stopplayback_acphy: already disabled\n"));
		}
	}
	/* disable the dac_test mode */
	phy_utils_and_phyreg(pi, ACPHY_sampleCmd(pi->pubpi->phy_rev),
		~ACPHY_sampleCmd_DacTestMode_MASK(pi->pubpi->phy_rev));

	/* if bb_mult_save does exist, restore bb_mult and undef bb_mult_save */
	if (pi_ac->misci->bb_mult_save_valid != 0) {
		uint8 core;

		FOREACH_CORE(pi, core) {
			wlc_phy_set_tx_bbmult_acphy(pi, &pi_ac->misci->bb_mult_save[core], core);
		}
		pi_ac->misci->bb_mult_save_valid = 0;
	}

	if (ACMAJORREV_GE40(pi->pubpi->phy_rev)) {
		SPINWAIT(((READ_PHYREG(pi, sampleStatus) & 0x7) != 0),
			ACPHY_SPINWAIT_RUNSAMPLE);
		if ((READ_PHYREG(pi, sampleStatus) & 0x7) != 0) {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0);
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : Sample play stop failed \n",
				__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_RX2TX_FAILED);
		}
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, sampleCmd, enable, 0x0);
		ACPHY_ENABLE_STALL(pi, stall_val);
	}
	if (no_reset == FALSE) {
		wlc_phy_resetcca_acphy(pi);
	}
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_txpwrctrl_enable_acphy(pi, pi_ac->tx_pwr_ctrl_status);
	}
}

#define SAMPLE_LOOP_COUNT_ENDLESS	0xFFFF
#define SAMPLE_WAIT_COUNT	60	/* Should be at least 60 for farrow FIFO depth to settle.
					 * 60 is to support 80mhz mode.  Units are in 25 ns.
					 */
#define DAC_TEST_MODE		0	/* 1 == enable DAC Test Mode */

static void
wlc_phy_runsamples_acphy(phy_info_t *pi, uint16 num_samps, uint8 iqmode)
{
	wlc_phy_runsamples_acphy_with_counts(pi, num_samps, iqmode,
		SAMPLE_LOOP_COUNT_ENDLESS, SAMPLE_WAIT_COUNT);
}

static void
wlc_phy_runsamples_acphy_with_counts(phy_info_t *pi, uint16 num_samps, uint8 iqmode,
	uint16 loops, uint16 wait)
{
	uint16 orig_RfseqCoreActv;
	const uint phy_rev = pi->pubpi->phy_rev;
	uint8 stall_val;

	/* The phy_rev parameter is unused in embedded builds as the compiler optimises it away.
	 * Mark the param as unused to avoid compiler warnings.
	 */
	UNUSED_PARAMETER(phy_rev);

	if (!(iqmode))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	/* Delay for proper RX2TX in sample play ow spurious emissions,radar FD */
	if (!ACMAJORREV_32(phy_rev) && !ACMAJORREV_33(phy_rev) &&
	    !ACMAJORREV_37(phy_rev) && !ACMAJORREV_47_51(phy_rev)) {
		OSL_DELAY(15);
	}

	if (ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, sampleCmd, enable, 0x1);
		ACPHY_ENABLE_STALL(pi, stall_val);
	}

	if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, sampleCmd, enable, 0x1);
	}

	phy_utils_and_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
		~ACPHY_macbasedDACPlay_macBasedDACPlayEn_MASK(phy_rev));

	/* configure sample play buffer */
	WRITE_PHYREG(pi, sampleDepthCount, num_samps-1);

	if (loops == SAMPLE_LOOP_COUNT_ENDLESS) {	/* 0xffff means: keep looping forever */
		WRITE_PHYREG(pi, sampleLoopCount, SAMPLE_LOOP_COUNT_ENDLESS);
	} else {
		WRITE_PHYREG(pi, sampleLoopCount, loops - 1);
	}

	/* Wait time should be atleast 60 for farrow FIFO depth to settle
	 * 60 is to support 80mhz mode.
	 * Though 20 is even for 20mhz mode, and 40 for 80mhz mode,
	 * but just giving some extra wait time
	 */
	WRITE_PHYREG(pi, sampleInitWaitCount, wait);	/* units are in 25 ns */

	PHY_TRACE(("Starting PHY based Sample Play\n"));

	/* start sample play buffer (in regular mode or iqcal mode) */
	orig_RfseqCoreActv = READ_PHYREG(pi, RfseqMode);
	phy_utils_or_phyreg(pi, ACPHY_RfseqMode(phy_rev),
		ACPHY_RfseqMode_CoreActv_override_MASK(phy_rev));
	phy_utils_and_phyreg(pi, ACPHY_sampleCmd(phy_rev),
		~ACPHY_sampleCmd_DacTestMode_MASK(phy_rev));
	phy_utils_and_phyreg(pi, ACPHY_sampleCmd(phy_rev), ~ACPHY_sampleCmd_start_MASK(phy_rev));
	phy_utils_and_phyreg(pi, ACPHY_iqloCalCmdGctl(phy_rev), 0x3FFF);
	if (ACMAJORREV_47_51(phy_rev) && iqmode) {
		OSL_DELAY(1);
	}
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	if (iqmode) {
		phy_utils_or_phyreg(pi, ACPHY_iqloCalCmdGctl(phy_rev), 0x8000);
	} else {
		uint16  sample_cmd = ACPHY_sampleCmd_start_MASK(phy_rev);
#if (DAC_TEST_MODE == 1)
		sample_cmd |= ACPHY_sampleCmd_DacTestMode_MASK(phy_rev);
#endif /* DAC_TEST_MODE */
		/* Disable stall before issue the sample play start
		as the stall can cause it to miss the start
		*/
		phy_utils_or_phyreg(pi, ACPHY_sampleCmd(phy_rev), sample_cmd);
	}

	/* Wait till the Rx2Tx sequencing is done */
	SPINWAIT(((READ_PHYREG(pi, RfseqStatus0) & 0x1) == 1), ACPHY_SPINWAIT_RUNSAMPLE);
	if ((READ_PHYREG(pi, RfseqStatus0) & 0x1) == 1) {
		PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : Rx to Tx failed \n", __FUNCTION__));
		PHY_FATAL_ERROR(pi, PHY_RC_RX2TX_FAILED);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);

	/* restore mimophyreg(RfseqMode.CoreActv_override) */
	WRITE_PHYREG(pi, RfseqMode, orig_RfseqCoreActv);

	if (!(iqmode))
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static void
wlc_phy_runsamples_acphy_mac_based(phy_info_t *pi, uint16 num_samps)
{
	const uint phy_rev = pi->pubpi->phy_rev;
	uint8 stall_val;

	/* The phy_rev parameter is unused in embedded builds as the compiler optimises it away.
	 * Mark the param as unused to avoid compiler warnings.
	 */
	UNUSED_PARAMETER(phy_rev);

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Delay for proper RX2TX in sample play ow spurious emissions,radar FD */
	if (!ACMAJORREV_32(phy_rev) && !ACMAJORREV_33(phy_rev) &&
	    !ACMAJORREV_37(phy_rev) && !ACMAJORREV_47_51(phy_rev)) {
		OSL_DELAY(15);
	}

	if (ACMAJORREV_GE37(phy_rev)) {
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, sampleCmd, enable, 0x1);
		ACPHY_ENABLE_STALL(pi, stall_val);
	}

	if (ACMAJORREV_33(phy_rev)) {
		MOD_PHYREG(pi, sampleCmd, enable, 0x1);
	}

	phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
		ACPHY_macbasedDACPlay_macBasedDACPlayEn_MASK(phy_rev));

	if (CHSPEC_IS80(pi->radio_chanspec) || PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x3 <<
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		ASSERT(0);
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x2 <<
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
	} else {
		phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x1 <<
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
	}

	PHY_TRACE(("Starting MAC based Sample Play"));
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);

	if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
		uint16 SampleCollectPlayCtrl = R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
		SampleCollectPlayCtrl |= (1 << SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT);
		W_REG(pi->sh->osh, D11_SMP_CTRL(pi), SampleCollectPlayCtrl);
	} else {
		uint16 phy_ctl;
		phy_ctl = (1 << PHYCTRL_SAMPLEPLAYSTART_SHIFT)
			| (1 << PHYCTRL_MACPHYFORCEGATEDCLKSON_SHIFT);
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
	}

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
}

static void
BCMATTACHFN(wlc_phy_srom_read_rxgainerr_acphy)(phy_info_t *pi)
{
	/* read and uncompress gain-error values for rx power reporting */

	int8 tmp[PHY_CORE_NUM_4];
	uint8 coreidx[4] = {0, 1, 2, 3};
	int16 tmp2;

	if (phy_get_phymode(pi) == PHYMODE_RSDB) {
		if (phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0) {
			/* update pi[0] to hold pwrdet params for all cores */
			/* This is required for mimo operation */
			pi->pubpi->phy_corenum <<= 1;
		} else {
			coreidx[1] = 0;
		}
	}

	(void)memset(tmp, -1, sizeof(tmp));

	tmp2 = pi->srom_rawtempsense;
	if (tmp2 == 255) {
		/* set to -1, since nothing was written to SROM */
		tmp2 = -1;
	}

	/* 2G: */
	/* read and sign-extend */
	tmp[0] = (int8)(((int8)PHY_GETINTVAR(pi, rstr_rxgainerr2ga0)) << 2) >> 2;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp[1] = (int8)(((int8)PHY_GETINTVAR(pi, rstr_rxgainerr2ga1)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp[2] = (int8)(((int8)PHY_GETINTVAR(pi, rstr_rxgainerr2ga2)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		tmp[3] = (int8)(((int8)PHY_GETINTVAR(pi, rstr_rxgainerr2ga3)) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1) && (tmp2 == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr2g_isempty = TRUE;
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
			tmp[3] = (int8)(((int8)PHY_GETINTVAR(pi, rstr_rxgainerr2ga3)) << 3) >> 3;
	} else {
		pi->rxgainerr2g_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_2g[coreidx[0]] = tmp[0];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		pi->rxgainerr_2g[coreidx[1]] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		pi->rxgainerr_2g[coreidx[2]] = tmp[0] + tmp[2];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		pi->rxgainerr_2g[coreidx[3]] = tmp[0] + tmp[3];

	/* 5G low: */
	/* read and sign-extend */
	tmp[0] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga0, 0)) << 2) >> 2;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp[1] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga1, 0)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp[2] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga2, 0)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		tmp[3] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga3, 0)) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1) && (tmp2 == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gl_isempty = TRUE;
		if ((PHYCORENUM(pi->pubpi->phy_corenum) > 3) && (tmp[3] == -1))
			tmp[3] = 0;
	} else {
		pi->rxgainerr5gl_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gl[coreidx[0]] = tmp[0];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		pi->rxgainerr_5gl[coreidx[1]] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		pi->rxgainerr_5gl[coreidx[2]] = tmp[0] + tmp[2];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		pi->rxgainerr_5gl[coreidx[3]] = tmp[0] + tmp[3];

	/* 5G mid: */
	/* read and sign-extend */
	tmp[0] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga0, 1)) << 2) >> 2;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp[1] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga1, 1)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp[2] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga2, 1)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		tmp[3] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga3, 1)) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1) && (tmp2 == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gm_isempty = TRUE;
		if ((PHYCORENUM(pi->pubpi->phy_corenum) > 3) && (tmp[3] == -1))
			tmp[3] = 0;
	} else {
		pi->rxgainerr5gm_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gm[coreidx[0]] = tmp[0];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		pi->rxgainerr_5gm[coreidx[1]] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		pi->rxgainerr_5gm[coreidx[2]] = tmp[0] + tmp[2];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		pi->rxgainerr_5gm[coreidx[3]] = tmp[0] + tmp[3];

	/* 5G high: */
	/* read and sign-extend */
	tmp[0] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga0, 2)) << 2) >> 2;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp[1] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga1, 2)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp[2] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga2, 2)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		tmp[3] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga3, 2)) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1) && (tmp2 == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gh_isempty = TRUE;
		if ((PHYCORENUM(pi->pubpi->phy_corenum) > 3) && (tmp[3] == -1))
			tmp[3] = 0;
	} else {
		pi->rxgainerr5gh_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gh[coreidx[0]] = tmp[0];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		pi->rxgainerr_5gh[coreidx[1]] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		pi->rxgainerr_5gh[coreidx[2]] = tmp[0] + tmp[2];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		pi->rxgainerr_5gh[coreidx[3]] = tmp[0] + tmp[3];

	/* 5G upper: */
	/* read and sign-extend */
	tmp[0] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga0, 3)) << 2) >> 2;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp[1] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga1, 3)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp[2] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga2, 3)) << 3) >> 3;
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		tmp[3] = (int8)(((int8)getintvararray(pi->vars, rstr_rxgainerr5ga3, 3)) << 3) >> 3;

	if ((tmp[0] == -1) && (tmp[1] == -1) && (tmp[2] == -1) && (tmp2 == -1)) {
		/* If all srom values are -1, then possibly
		 * no gainerror info was written to srom
		 */
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 0;
		pi->rxgainerr5gu_isempty = TRUE;
		if ((PHYCORENUM(pi->pubpi->phy_corenum) > 3) && (tmp[3] == -1))
			tmp[3] = 0;
	} else {
		pi->rxgainerr5gu_isempty = FALSE;
	}
	/* gain errors for cores 1 and 2 are stored in srom as deltas relative to core 0: */
	pi->rxgainerr_5gu[coreidx[0]] = tmp[0];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		pi->rxgainerr_5gu[coreidx[1]] = tmp[0] + tmp[1];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		pi->rxgainerr_5gu[coreidx[2]] = tmp[0] + tmp[2];
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 3)
		pi->rxgainerr_5gu[coreidx[3]] = tmp[0] + tmp[3];

	if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
		(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0))
	{
		/* update pi[0] to hold pwrdet params for all cores */
		/* This is required for mimo operation */
		pi->pubpi->phy_corenum >>= 1;
	}

}

static void
BCMATTACHFN(phy_ac_misc_nvram_femctrl_clb_read)(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	pi_ac->sromi->nvram_femctrl_clb.map_2g[0][0] =
		(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb2gslice0core0, 0x3ff);
	pi_ac->sromi->nvram_femctrl_clb.map_2g[1][0] =
		(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb2gslice1core0, 0x3ff);

	if (PHYCORENUM((pi)->pubpi->phy_corenum) >= 2) {
		pi_ac->sromi->nvram_femctrl_clb.map_2g[0][1] =
			(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb2gslice0core1, 0x3ff);
		pi_ac->sromi->nvram_femctrl_clb.map_2g[1][1] =
			(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb2gslice1core1, 0x3ff);
	}

	if (PHY_BAND5G_ENAB(pi)) {
		pi_ac->sromi->nvram_femctrl_clb.map_5g[0][0] =
			(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb5gslice0core0, 0x3ff);
		pi_ac->sromi->nvram_femctrl_clb.map_5g[1][0] =
			(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb5gslice1core0, 0x3ff);

		if (PHYCORENUM((pi)->pubpi->phy_corenum) >= 2) {
			pi_ac->sromi->nvram_femctrl_clb.map_5g[0][1] =
				(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb5gslice0core1, 0x3ff);
			pi_ac->sromi->nvram_femctrl_clb.map_5g[1][1] =
				(uint32) PHY_GETINTVAR_DEFAULT(pi, rstr_clb5gslice1core1, 0x3ff);
		}
	}

	pi_ac->sromi->nvram_femctrl_clb.btc_prisel_mask =
		(uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_btc_prisel_mask, 0);

	pi_ac->sromi->nvram_femctrl_clb.btc_prisel_ant_mask =
		(uint8) PHY_GETINTVAR_DEFAULT(pi, rstr_btc_prisel_ant_mask, 0x3);

	pi_ac->sromi->clb_swctrl_smask_ant0 =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_clb_swctrl_smask_ant0, 0x3ff);

	pi_ac->sromi->clb_swctrl_smask_ant1 =
		(uint16) PHY_GETINTVAR_DEFAULT(pi, rstr_clb_swctrl_smask_ant1, 0x3ff);
}

static void
BCMATTACHFN(phy_ac_misc_nvram_femctrl_read)(phy_info_t *pi)
{
	uint8 i;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (ACPHY_FEMCTRL_ACTIVE(pi)) {
		return;
	}
	if (PHY_GETVAR_SLICE(pi, rstr_swctrlmap_2g)) {
		for (i = 0; i < ACPHY_SWCTRL_NVRAM_PARAMS; i++) {
			pi_ac->sromi->nvram_femctrl.swctrlmap_2g[i] =
				(uint32) PHY_GETINTVAR_ARRAY_SLICE(pi,
				rstr_swctrlmap_2g, i);
		}
	} else {
		PHY_ERROR(("%s: Switch control map(%s) is NOT found\n",
		           __FUNCTION__, rstr_swctrlmap_2g));
	}

	if (PHY_GETVAR_SLICE(pi, rstr_swctrlmapext_2g)) {
			for (i = 0; i < ACPHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_ac->sromi->nvram_femctrl.swctrlmapext_2g[i] =
					(uint32) PHY_GETINTVAR_ARRAY_SLICE(pi,
					rstr_swctrlmapext_2g, i);
			}
	}

	if (PHY_GETVAR_SLICE(pi, rstr_swctrlmap_5g)) {
			for (i = 0; i < ACPHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_ac->sromi->nvram_femctrl.swctrlmap_5g[i] =
					(uint32) PHY_GETINTVAR_ARRAY_SLICE(pi,
					rstr_swctrlmap_5g, i);
			}
	} else {
		PHY_ERROR(("%s: Switch control map(%s) is NOT found\n",
		           __FUNCTION__, rstr_swctrlmap_5g));
	}

	if (PHY_GETVAR_SLICE(pi, rstr_swctrlmapext_5g)) {
			for (i = 0; i < ACPHY_SWCTRL_NVRAM_PARAMS; i++) {
				pi_ac->sromi->nvram_femctrl.swctrlmapext_5g[i] =
					(uint32) PHY_GETINTVAR_ARRAY_SLICE(pi,
					"swctrlmapext_5g", i);
			}
	}

	pi_ac->sromi->nvram_femctrl.txswctrlmap_2g =
		(uint32) PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txswctrlmap_2g, PAMODE_HI_LIN);

	pi_ac->sromi->nvram_femctrl.txswctrlmap_2g_mask =
		(uint16) PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txswctrlmap_2g_mask, 0x3fff);

	pi_ac->sromi->nvram_femctrl.txswctrlmap_5g =
		(uint32) PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_txswctrlmap_5g, PAMODE_HI_LIN);

	if (PHY_GETVAR(pi, rstr_fem_table_init_val)) {
		pi_ac->sromi->femctrl_init_val_2g =
			(uint32) PHY_GETINTVAR_ARRAY(pi, rstr_fem_table_init_val, 0);
		pi_ac->sromi->femctrl_init_val_5g =
			(uint32) PHY_GETINTVAR_ARRAY(pi, rstr_fem_table_init_val, 1);
	} else {
		pi_ac->sromi->femctrl_init_val_2g = 0;
		pi_ac->sromi->femctrl_init_val_5g = 0;
	}
}

#ifdef ATE_BUILD
void
wlc_phy_gpaio_acphy(phy_info_t *pi, wl_gpaio_option_t option, int core)
{

	uint16 test_en = 0, ana_mux = 0, testpoint = 0;
	uint16 gpaiosel0 = 0, gpaiosel1 = 0, gpaiosel3 = 0;

	if (TINY_RADIO(pi)) {

		/* powerup gpaio block */
		MOD_RADIO_REG_TINY(pi, GPAIO_SEL2, core, gpaio_pu, 1);
		/* powerdown rcal, otherwise it conflicts */
		MOD_RADIO_REG_TINY(pi, RCAL_CFG_NORTH, core, rcal_pu, 0);

		/* To bring out various radio test signals on gpaio. */
		if (option == GPAIO_PMU_CLEAR)
			MOD_RADIO_REG_TINY(pi, GPAIO_SEL0, core, gpaio_sel_0to15_port, (0x1 << 0));
		else if (option == GPAIO_ICTAT_CAL) {
			MOD_RADIO_REG_TINY(pi, GPAIO_SEL0, core,
					gpaio_sel_0to15_port, 0x0);
			MOD_RADIO_REG_TINY(pi, GPAIO_SEL1, core,
					gpaio_sel_16to31_port, (0x1 << 11));
		}
		else
			MOD_RADIO_REG_TINY(pi, GPAIO_SEL0, core, gpaio_sel_0to15_port, (0x1 << 14));

		if (option != GPAIO_ICTAT_CAL)
			MOD_RADIO_REG_TINY(pi, GPAIO_SEL1, core, gpaio_sel_16to31_port, 0x0);
		switch (option) {
			case (GPAIO_PMU_AFELDO): {
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x01);
				MOD_RADIO_REG_TINY(pi, PMU_CFG1, core, wlpmu_ana_mux, 0x00);
				break;
			}
			case (GPAIO_PMU_TXLDO): {
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x01);
				MOD_RADIO_REG_TINY(pi, PMU_CFG1, core, wlpmu_ana_mux, 0x01);
				break;
			}
			case (GPAIO_PMU_VCOLDO): {
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x01);
				MOD_RADIO_REG_TINY(pi, PMU_CFG1, core, wlpmu_ana_mux, 0x02);
				break;
			}
			case GPAIO_PMU_LNALDO: {
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x01);
				MOD_RADIO_REG_TINY(pi, PMU_CFG1, core, wlpmu_ana_mux, 0x03);
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_ana_mux_high, 0x00);
				break;
			}
			case GPAIO_PMU_ADCLDO: {
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x01);
				MOD_RADIO_REG_TINY(pi, PMU_CFG1, core, wlpmu_ana_mux, 0x03);
				MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_ana_mux_high, 0x01);
				break;
			}
			case GPAIO_PMU_CLEAR: {
				  MOD_RADIO_REG_TINY(pi, PMU_CFG3, core, wlpmu_tsten, 0x00);
				  break;
			}
			case GPAIO_OFF: {
					MOD_RADIO_REG_TINY(pi, GPAIO_SEL2, core, gpaio_pu, 0);
					break;
			}
			default:
					break;
		}
	} else {
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL2, core, gpaio_pu, 0x1);
			/* powerdown rcal, otherwise it conflicts */
			MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL2, core, gpaio_rcal_pu, 0);
			/* To bring out various radio test signals on gpaio. */
			switch (option) {
				case (GPAIO_PMU_AFELDO): {
						test_en = 1;
						ana_mux = 0;
						testpoint = 34;
						break;
				}
				case (GPAIO_PMU_TXLDO): {
						test_en = 1;
						ana_mux = 1;
						testpoint = 34;
						break;
				}
				case (GPAIO_PMU_VCOLDO): {  // logen LDO
						test_en = 1;
						ana_mux = 2;
						testpoint = 34;
						break;
				}
				case GPAIO_PMU_LNALDO: {  //   ldo1p6
						test_en = 1;
						ana_mux = 3;
						testpoint = 34;
						break;
				}
				case GPAIO_PMU_CLEAR: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						break;
				}
				case GPAIO_OFF: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL2,
						core, gpaio_pu, 0);
						break;
				}
				default:
						break;
			}

			if (testpoint < 16) {
				gpaiosel0 = 1 << (testpoint - 0);
			} else if (testpoint < 32) {
				gpaiosel1 = 1 << (testpoint - 16);
			} else if (testpoint < 37) {
				gpaiosel3 = 1 << (testpoint - 32);
			}

			MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL0, core,
					gpaio_sel_0to15_port, gpaiosel0);
			MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL1, core,
					gpaio_sel_16to31_port, gpaiosel1);
			MOD_RADIO_REG_28NM(pi, RFP, GPAIO_SEL3, core,
					gpaio_sel_32to47_port, gpaiosel3);

			MOD_RADIO_REG_28NM(pi, RF, PMU_OP2, core, wlpmu_tsten, test_en);
			MOD_RADIO_REG_28NM(pi, RF, PMU_OP2, core, wlpmu_ana_mux, ana_mux);
		} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {

			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL2, core, gpaio_pu, 0x1);
			/* powerdown rcal, otherwise it conflicts */
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL2, core, gpaio_rcal_pu, 0);
			MOD_RADIO_REG_20694(pi, RFP, RCAL_CFG_NORTH, 0, rcal_pu, 0);
			MOD_RADIO_REG_20694(pi, RFP, BG_REG3, 0, bg_rcal_pu, 0);
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL0, core,
					gpaio_sel_0to15_port, 0);
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL1, core,
					gpaio_sel_16to31_port, 0);
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL3, core,
					gpaio_sel_32to47_port, 0);
			MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG5, core, loopback_bias_pu, 1);
			MOD_RADIO_REG_20694(pi, RF, IQCAL_OVR1, core,
			ovr_iqcal_PU_loopback_bias, 1);
			/* To bring out various radio test signals on gpaio. */
			switch (option) {
				case (GPAIO_PMU_AFELDO): {
						test_en = 1;
						ana_mux = 0;
						testpoint = 33;
						break;
				}
				case (GPAIO_PMU_TXLDO): {
						test_en = 1;
						ana_mux = 1;
						testpoint = 33;
						break;
				}
				case (GPAIO_PMU_VCOLDO): {  // logen LDO
						test_en = 1;
						ana_mux = 0;
						testpoint = 33;
						MOD_RADIO_REG_20694(pi, RFP, PLL_HVLDO4,
						0, ldo_1p8_vout_gpaio_test_en, 1);
						break;
				}
				case (GPAIO_PMU_LOGENLDO): {  // logen LDO
						test_en = 1;
						ana_mux = 2;
						testpoint = 33;
						break;
				}
				case GPAIO_PMU_LNALDO: {  //   ldo1p6
						test_en = 1;
						ana_mux = 5;
						testpoint = 33;
						break;
				}
				case GPAIO_PMU_RXLDO2G: {  //   ldo1p6
						test_en = 1;
						ana_mux = 3;
						testpoint = 33;
						MOD_RADIO_REG_20694(pi, RF, RX2G_REG4,
						core, rx_ldo_out_en, 1);
						MOD_RADIO_REG_20694(pi, RF, RX2G_REG4,
						core, rx_ldo_out_5g, 0);
						break;
				}
				case GPAIO_PMU_RXLDO5G: {  //   ldo1p6
						test_en = 1;
						ana_mux = 3;
						testpoint = 33;
						MOD_RADIO_REG_20694(pi, RF, RX2G_REG4,
						core, rx_ldo_out_en, 1);
						MOD_RADIO_REG_20694(pi, RF, RX2G_REG4,
						core, rx_ldo_out_5g, 1);
						break;
				}
				case GPAIO_PMU_ADCLDO:{
						test_en = 1;
						ana_mux = 4;
						testpoint = 33;
						break;
				}
				case GPAIO_PMU_CLEAR: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20694(pi, RF, PA2G_CFG5,
						core, pa2g_gpio_stby_ldo_pu, 0);
						MOD_RADIO_REG_20694(pi, RF, PA5G_CFG3,
						core, pa5g_gpio_stby_ldo_pu, 0);
						MOD_RADIO_REG_20694(pi, RF, TX_PWSW_CFG,
						core, tx_gpio_2p5_ldo_pu, 0);
						MOD_RADIO_REG_20694(pi, RF, TX_MX_CFG1,
						core, mx_gpio_cas_lowbias_en, 0);
						MOD_RADIO_REG_20694(pi, RF, TX_MX_CFG1,
						core, mx_gpio_cas_en, 0);
						if (!(PHY_IPA(pi))) {
							MOD_RADIO_REG_20694(pi, RF, PA5G_CFG3,
							core, pa5g_pu_2branch, 0);
							MOD_RADIO_REG_20694(pi, RF, PA5G_CFG3,
							core, pa5g_pu_6branch, 0);
							MOD_RADIO_REG_20694(pi, RF, PA5G_CFG2,
							core, pa5g_idac_topc_op1, 0);
							MOD_RADIO_REG_20694(pi, RF, PA5G_CFG2,
							core, pa5g_idac_topc_op2, 0);
						}
						MOD_RADIO_REG_20694(pi, RF, RX2G_REG4,
						core, rx_ldo_out_en, 0);
						MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG5,
						core, loopback_bias_pu, 0);
						MOD_RADIO_REG_20694(pi, RF, IQCAL_OVR1,
						core, ovr_iqcal_PU_loopback_bias, 0);
						MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL2,
						core, gpaio_pu, 0);
						break;
				}
				case GPAIO_OFF: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL2,
						core, gpaio_pu, 0);
						break;
				}
				default:
						break;
			}

			if (testpoint < 16) {
				gpaiosel0 = 1 << (testpoint - 0);
			} else if (testpoint < 32) {
				gpaiosel1 = 1 << (testpoint - 16);
			} else if (testpoint < 48) {
				gpaiosel3 = 1 << (testpoint - 32);
			}

			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL0, core,
					gpaio_sel_0to15_port, gpaiosel0);
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL1, core,
					gpaio_sel_16to31_port, gpaiosel1);
			MOD_RADIO_REG_20694(pi, RF, GPAIO_SEL3, core,
					gpaio_sel_32to47_port, gpaiosel3);

			MOD_RADIO_REG_20694(pi, RF, PMU_OP1, core, wlpmu_tsten, test_en);
			MOD_RADIO_REG_20694(pi, RF, PMU_OP1, core, wlpmu_ana_mux, ana_mux);
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			if (pi->pubpi->slice == DUALMAC_AUX) {
				/* Power up gpaio block, powerdown rcal,
				 * clear all test point seletion
				 */
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core, gpaio_pu, 0x1);
				MOD_RADIO_REG_20697(pi, RFP, RCAL_CFG_NORTH, 0, rcal_pu, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
					gpaio_rcal_pu, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core,
						gpaio_sel_16to31_port, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core,
						gpaio_sel_32to47_port, 0);
				MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core,
					loopback_bias_pu, 1);
				MOD_RADIO_REG_20697(pi, RF, IQCAL_OVR1, core,
					ovr_iqcal_PU_loopback_bias, 1);

				/* To bring out various radio test signals on gpaio. */
				switch (option) {
					case (GPAIO_PMU_AFELDO): {
						test_en = 1;
						ana_mux = 0;
						testpoint = 34;
						break;
					}
					case (GPAIO_PMU_LPFTXLDO): {
						test_en = 1;
						ana_mux = 1;
						testpoint = 34;
						break;
					}
					case (GPAIO_PMU_LOGENLDO): {
						test_en = 1;
						ana_mux = 2;
						testpoint = 34;
						break;
					}
					case GPAIO_PMU_RXLDO2G: {
						test_en = 1;
						ana_mux = 3;
						testpoint = 34;
						break;
					}
					case GPAIO_PMU_RXLDO5G: {
						test_en = 1;
						ana_mux = 3;
						testpoint = 34;
						break;
					}
					case GPAIO_PMU_LDO1P6: {
						test_en = 1;
						ana_mux = 5;
						testpoint = 34;
						break;
					}
					case GPAIO_RCAL: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RFP, BG_REG3, 0,
							bg_rcal_pu, 1);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
							gpaio_rcal_pu, 1);
						break;
					}
					case GPAIO_PMU_CLEAR: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core,
							gpaio_sel_0to15_port, 0);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core,
							gpaio_sel_16to31_port, 0);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core,
							gpaio_sel_32to47_port, 0);
						MOD_RADIO_REG_20697(pi, RF, PA2G_CFG1, core,
							pa2g_gpio_sw_pu, 0);
						MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core,
							loopback_bias_pu, 0);
						break;
					}
					case GPAIO_OFF: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
								gpaio_pu, 0x0);
						break;
					}
					default:
						break;
				}
			} else {
				/* Power up gpaio block, powerdown rcal,
				 * clear all test point seletion
				 */
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
					gpaio_pu, 0x1);
				MOD_RADIO_REG_20697(pi, RFP, RCAL_CFG_NORTH, 1,
					rcal_pu, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, 0,
					gpaio_rcal_pu, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core,
					gpaio_sel_0to15_port, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core,
					gpaio_sel_16to31_port, 0);
				MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core,
					gpaio_sel_32to47_port, 0);
				MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core,
					loopback_bias_pu, 1);
				MOD_RADIO_REG_20697(pi, RF, IQCAL_OVR1, core,
					ovr_iqcal_PU_loopback_bias, 1);
				/* To bring out various radio test signals on gpaio. */
				switch (option) {
					case (GPAIO_PMU_AFELDO): {
						test_en = 1;
						ana_mux = 0;
						testpoint = 33;
						break;
					}
					case (GPAIO_PMU_LPFTXLDO): {
						test_en = 1;
						ana_mux = 1;
						testpoint = 33;
						break;
					}
					case (GPAIO_PMU_LOGENLDO): {
						test_en = 1;
						ana_mux = 2;
						testpoint = 33;
						break;
					}
					case GPAIO_PMU_RXLDO5G: {
						test_en = 1;
						ana_mux = 3;
						testpoint = 33;
						break;
					}
					case GPAIO_PMU_ADCLDO: {
						test_en = 1;
						ana_mux = 4;
						testpoint = 33;
						break;
					}
					case GPAIO_PMU_LDO1P6: {
						test_en = 1;
						ana_mux = 5;
						testpoint = 33;
						break;
					}
					case GPAIO_RCAL: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RFP, BG_REG3, 1,
							bg_rcal_pu, 1);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
							gpaio_rcal_pu, 1);
						break;
					}
					case GPAIO_PMU_CLEAR: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core,
							gpaio_sel_0to15_port, 0);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core,
							gpaio_sel_16to31_port, 0);
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core,
							gpaio_sel_32to47_port, 0);
						MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core,
							loopback_bias_pu, 0);
						MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core,
							iqcal_tssi_GPIO_ctrl, 0);
						MOD_RADIO_REG_20697X(pi, RF, PA5G_CFG1, 0, core,
							pa5g_gpio_sw_pu, 0);
						MOD_RADIO_REG_20697X(pi, RF, TX5G_CFG1, 0, core,
							pad5g_gpio_sw_pu, 0);
						break;
					}
					case GPAIO_OFF: {
						test_en = 0;
						ana_mux = 0;
						testpoint = 100;
						MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core,
							gpaio_pu, 0x0);
						break;
					}
					default:
						break;
				}
			}
			if (testpoint < 16) {
				gpaiosel0 = 1 << (testpoint - 0);
			} else if (testpoint < 32) {
				gpaiosel1 = 1 << (testpoint - 16);
			} else if (testpoint < 48) {
				gpaiosel3 = 1 << (testpoint - 32);
			}

			MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core,
				gpaio_sel_0to15_port, gpaiosel0);
			MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core,
				gpaio_sel_16to31_port, gpaiosel1);
			MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core,
				gpaio_sel_32to47_port, gpaiosel3);

			if (pi->pubpi->slice == DUALMAC_AUX) {
				MOD_RADIO_REG_20697(pi, RFP, PMU_OP1, 0,
					wlpmu_tsten, test_en);
				MOD_RADIO_REG_20697(pi, RFP, PMU_OP1, 0,
					wlpmu_ana_mux, ana_mux);
			} else {
				MOD_RADIO_REG_20697(pi, RF, PMU_OP1, core,
					wlpmu_tsten, test_en);
				MOD_RADIO_REG_20697(pi, RF, PMU_OP1, core,
					wlpmu_ana_mux, ana_mux);
			}

		}  else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			uint8 c;
			/* Powerdown gpaio top and other GPAIO blocks, powerdown rcal,
			   clear all test point selection;
			 * powering up of gpaio blocks will be done inside the respective blocks
			 */
			MOD_RADIO_PLLREG_20698(pi, GPAIO_SEL6, 0,
				gpaio_top_pu, 0x0);
			MOD_RADIO_REG_20698(pi, GPAIO_SEL8, 1,
				gpaio_rcal_pu, 0x0);
			MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
				toptestmux_pu, 0x0);
			for (c = 0; c < 4; c++) {
				MOD_RADIO_REG_20698(pi, GPAIO_SEL2, c,
					anatestmux_pu, 0x0);
				MOD_RADIO_REG_20698(pi, GPAIO_SEL2, c,
					gpaio_pu, 0x0);
				if (c < 2) {
					MOD_RADIO_PLLREG_20698(pi, PLL_HVLDO4, c,
						ldo_1p8_vout_gpaio_test_en, 0x0);
					MOD_RADIO_PLLREG_20698(pi, PLL_REFDOUBLER1, 0,
						RefDoubler_ldo_1p8_vout_gpaio_test_en, 0x0);
					MOD_RADIO_PLLREG_20698(pi, PLL_LF1, c,
						rfpll_lf_en_vctrl_tp, 0x0);
					MOD_RADIO_PLLREG_20698(pi, PLL_VCO7, c,
						rfpll_vco_en_ampdet_vco1, 0x0);
					MOD_RADIO_PLLREG_20698(pi, PLL_VCO7, c,
						rfpll_vco_en_ampdet_vco2, 0x0);
					MOD_RADIO_PLLREG_20698(pi, PLL_VCO5, c,
						rfpll_vco_tmx_mode, 0x0);
				}
			}
			/* To bring out various radio test signals on gpaio. */
			switch (option) {
				case (GPAIO_RCAL): {
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL8, 1,
						gpaio_rcal_pu, 0x1);
					MOD_RADIO_PLLREG_20698(pi, BG_REG3, 0,
						bg_rcal_pu, 0x1);
					MOD_RADIO_PLLREG_20698(pi, RCAL_CFG_NORTH, 0,
						rcal_pu, 0x1);
					break;
				}
				case (GPAIO_PMU_AFELDO): {
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x0);
					break;
				}
				case (GPAIO_PMU_LPFTXLDO): {
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x1);
					break;
				}
				case (GPAIO_PMU_LOGENLDO): {
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x2);
					break;
				}
				case GPAIO_PMU_RXLDO2G: // no break
				case GPAIO_PMU_RXLDO5G: {
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x3);
					MOD_RADIO_REG_20698(pi, RX2G_REG4, core,
						rx_ldo_out_en, 0x1);
					break;
				}
				case GPAIO_PMU_ADCLDO:{
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x4);
					break;
				}
				case GPAIO_PMU_LDO1P6:{
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						gpaio2test2_sel, 0x1);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL0, core,
						gpaio_sel_0to15_port, 1 << 12);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL2, core,
						gpaio_pu, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_tsten, 0x1);
					MOD_RADIO_REG_20698(pi, PMU_OP1, core,
						wlpmu_ana_mux, 0x5);
					break;
				}
				case GPAIO_OFF: {
					MOD_RADIO_PLLREG_20698(pi, GPAIO_SEL6, 0,
						gpaio_top_pu, 0x0);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL8, 1,
						gpaio_rcal_pu, 0x0);
					MOD_RADIO_PLLREG_20698(pi, RCAL_CFG_NORTH, 0,
						rcal_pu, 0x0);
					MOD_RADIO_REG_20698(pi, GPAIO_SEL9, 2,
						toptestmux_pu, 0x0);
					for (c = 0; c < 4; c++) {
						MOD_RADIO_REG_20698(pi, GPAIO_SEL2, c,
							anatestmux_pu, 0x0);
						MOD_RADIO_REG_20698(pi, GPAIO_SEL2, c,
							gpaio_pu, 0x0);
						MOD_RADIO_REG_20698(pi, GPAIO_SEL0, c,
							gpaio_sel_0to15_port, 1 << 14);
						MOD_RADIO_REG_20698(pi, LOGEN_CORE_REG1, c,
							logen_pdet_en, 0x0);
						MOD_RADIO_REG_20698(pi, LOGEN_CORE_REG1, c,
							logen_2g_tx_pdet_en, 0x0);
						MOD_RADIO_REG_20698(pi, LOGEN_CORE_REG1, c,
							logen_2g_rx_pdet_en, 0x0);
						MOD_RADIO_REG_20698(pi, LOGEN_CORE_REG1, c,
							logen_5g_tx_rccr_pdet_en, 0x0);
						MOD_RADIO_REG_20698(pi, LOGEN_CORE_REG1, c,
							logen_5g_rx_rccr_pdet_en, 0x0);
						MOD_RADIO_REG_20698(pi, TX2G_MIX_REG0, c,
							tx2g_mx_pu_replica, 0x0);
						MOD_RADIO_REG_20698(pi, TX2G_PAD_REG0, c,
							pad2g_gpio_sw_pu, 0x0);
						MOD_RADIO_REG_20698(pi, TX5G_PAD_REG3, c,
							tx5g_pad_vcas_monitor_sw, 0x0);
						MOD_RADIO_REG_20698(pi, TX5G_PA_REG4, c,
							tx5g_pa_gpaio_seriessw, 0x0);
						MOD_RADIO_REG_20698(pi, LPF_REG8, c,
							lpf_sw_test_gpaio, 0x0);
						if (c < 2) {
							MOD_RADIO_PLLREG_20698(pi, PLL_HVLDO4, c,
								ldo_1p8_vout_gpaio_test_en, 0x0);
				MOD_RADIO_PLLREG_20698(pi, PLL_REFDOUBLER1, 0,
				                       RefDoubler_ldo_1p8_vout_gpaio_test_en,
				                       0x0);
							MOD_RADIO_PLLREG_20698(pi, PLL_LF1, c,
								rfpll_lf_en_vctrl_tp, 0x0);
							MOD_RADIO_PLLREG_20698(pi, PLL_VCO7, c,
								rfpll_vco_en_ampdet_vco1, 0x0);
							MOD_RADIO_PLLREG_20698(pi, PLL_VCO7, c,
								rfpll_vco_en_ampdet_vco2, 0x0);
							MOD_RADIO_PLLREG_20698(pi, PLL_VCO5, c,
								rfpll_vco_tmx_mode, 0x0);
						}
					}
					break;
				}
			default:
				break;
			}
		}
	}
}

static void
phy_ac_gpaio_gpaioconfig(phy_type_misc_ctx_t *ctx, wl_gpaio_option_t option, int core)
{
	bool suspend;
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* Suspend MAC if haven't done so */
	wlc_phy_conditional_suspend(pi, &suspend);
	wlc_phy_gpaio_acphy(pi, option, core);

		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);

	return;
}
#endif /* ATE_BUILD */

void
wlc_phy_cals_mac_susp_en_other_cr(phy_info_t *pi, bool suspend)
{
	phy_info_t *other_pi = phy_get_other_pi(pi);
	/* WAR:  Simultaneous CAL + Tx in RSDB mode results in
	   Chip hang due to excess current consumption. SUSPEND MAC
	   for the other core during cal on current core and enable it
	   after the cal is complete
	 */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) &&
		(phy_get_phymode(pi) == PHYMODE_RSDB) &&
		!PUB_NOT_ASSOC(other_pi)) {
		if (suspend == TRUE) {
			wlapi_suspend_mac_and_wait(other_pi->sh->physhim);
		} else {
			wlapi_enable_mac(other_pi->sh->physhim);
		}
	}
}

static bool
phy_ac_misc_get_rxgainerr(phy_type_misc_ctx_t *ctx, int16 *gainerr)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	bool srom_isempty[PHY_CORE_MAX] = { 0 };
	uint8 core;
#ifdef BAND5G
	uint8 core_freq_segment_map;
	uint16 channel;
	chanspec_t chanspec = pi->radio_chanspec;
	channel = CHSPEC_CHANNEL(chanspec);

	FOREACH_CORE(pi, core) {

		/* For 80P80, retrieve Primary/Secondary based on the mapping */
		if (CHSPEC_IS8080(chanspec)) {
			core_freq_segment_map = phy_ac_chanmgr_get_data
				(info->aci->chanmgri)->core_freq_mapping[core];
			if (PRIMARY_FREQ_SEGMENT == core_freq_segment_map) {
				channel = wf_chspec_primary80_channel(chanspec);
			} else if (SECONDARY_FREQ_SEGMENT == core_freq_segment_map) {
				channel = wf_chspec_secondary80_channel(chanspec);
			}
		}

		if (channel > 14) {
			/* 5G */
			if (channel <= 48) {
				/* 5G-low: channels 36 through 48 */
				gainerr[core] = (int16) pi->rxgainerr_5gl[core];
				srom_isempty[core] = pi->rxgainerr5gl_isempty;
			} else if (channel <= 64) {
				/* 5G-mid: channels 52 through 64 */
				gainerr[core] = (int16) pi->rxgainerr_5gm[core];
				srom_isempty[core] = pi->rxgainerr5gm_isempty;
			} else if (channel <= 128) {
				/* 5G-high: channels 100 through 128 */
				gainerr[core] = (int16) pi->rxgainerr_5gh[core];
				srom_isempty[core] = pi->rxgainerr5gh_isempty;
			} else {
				/* 5G-upper: channels 132 and above */
				gainerr[core] = (int16) pi->rxgainerr_5gu[core];
				srom_isempty[core] = pi->rxgainerr5gu_isempty;
			}
		} else {
			/* 2G */
			gainerr[core] = (int16) pi->rxgainerr_2g[core];
			srom_isempty[core] = pi->rxgainerr2g_isempty;
		}
	}
#else
	/* 2G */
	FOREACH_CORE(pi, core) {
		gainerr[core] = (int16) pi->rxgainerr_2g[core];
		srom_isempty[core] = pi->rxgainerr2g_isempty;
	}
#endif /* BAND5G */
	/* For 80P80, retrun only primary channel value */
	return srom_isempty[0];
}

#ifdef PHY_DUMP_BINARY
/* The function is forced to RAM since it accesses non-const tables */
static int BCMRAMFN(phy_ac_misc_getlistandsize)(phy_type_misc_ctx_t *ctx,
                    phyradregs_list_t **phyreglist, uint16 *phyreglist_sz)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	BCM_REFERENCE(pi);
	if (ACREV_IS(pi->pubpi->phy_rev, 24)) {
		*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev24[0];
		*phyreglist_sz = sizeof(dot11acphy_regs_rev24);
	} else if (ACREV_IS(pi->pubpi->phy_rev, 36)) {
		*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev36[0];
		*phyreglist_sz = sizeof(dot11acphy_regs_rev36);
	} else if (ACREV_IS(pi->pubpi->phy_rev, 40)) {
		if (wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX) {
			*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev40_aux[0];
			*phyreglist_sz = sizeof(dot11acphy_regs_rev40_aux);
		} else {
			*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev40_main[0];
			*phyreglist_sz = sizeof(dot11acphy_regs_rev40_main);
		}
	} else if (ACREV_IS(pi->pubpi->phy_rev, 44)) {
		if (wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX) {
			*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev44_aux[0];
			*phyreglist_sz = sizeof(dot11acphy_regs_rev44_aux);
		} else {
			*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev44_main[0];
			*phyreglist_sz = sizeof(dot11acphy_regs_rev44_main);
		}
	} else if (ACREV_IS(pi->pubpi->phy_rev, 46)) {
		*phyreglist = (phyradregs_list_t *) &dot11acphy_regs_rev44_main[0];
		*phyreglist_sz = sizeof(dot11acphy_regs_rev44_main);
	} else {
		PHY_INFORM(("%s: wl%d: unsupported AC phy rev %d\n",
			__FUNCTION__,  pi->sh->unit,  pi->pubpi->phy_rev));
		return BCME_UNSUPPORTED;
	}

	return BCME_OK;
}
#endif /* PHY_DUMP_BINARY */

int
phy_ac_misc_set_rud_agc_enable(phy_ac_misc_info_t *misci, int32 int_val)
{
	misci->rud_agc_enable = (bool)int_val;
	return BCME_OK;
}

int
phy_ac_misc_get_rud_agc_enable(phy_ac_misc_info_t *misci, int32 *ret_int_ptr)
{
	*ret_int_ptr = misci->rud_agc_enable;
	return BCME_OK;
}

#ifdef ATE_BUILD
static void
phy_type_misc_disable_ate_gpiosel(phy_type_misc_ctx_t *ctx)
{
	phy_ac_misc_info_t *misc_info = (phy_ac_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		phy_utils_phyreg_enter(pi);
		phy_utils_write_phyreg(pi, ACPHY_gpioLoOutEn(pi->pubpi->phy_rev), 0);
		phy_utils_write_phyreg(pi, ACPHY_gpioHiOutEn(pi->pubpi->phy_rev), 0);
		phy_utils_phyreg_exit(pi);
	} else {

		PHY_ERROR(("phy_type_misc_disable_ate_gpiosel not supported in this rev."));
		/* Not supported for other PHYs yet */
		ASSERT(FALSE);
	}
}
#endif /* ATE_BUILD */

static uint8
phy_ac_misc_get_bfe_ndp_recvstreams(phy_type_misc_ctx_t *ctx)
{
	phy_ac_misc_info_t *misc_info = (phy_ac_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;

	BCM_REFERENCE(pi);

	/* AC major 4, 32 and 40 can recv 3 */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
		return 3;
	} else {
		return 2;
	}
}

static void
phy_update_rxldpc_acphy(phy_type_misc_ctx_t *ctx, bool ldpc)
{
	phy_ac_misc_info_t *misc_info = (phy_ac_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	bool suspend = FALSE;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (ldpc != pi_ac->misci->ac_rxldpc_override) {
		pi_ac->misci->ac_rxldpc_override = ldpc;

		/* Suspend MAC if haven't done so */
		wlc_phy_conditional_suspend(pi, &suspend);

		MOD_PHYREG(pi, HTSigTones, support_ldpc, (ldpc) ? 1 : 0);

		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
	}
}

static void
phy_ac_misc_set_preamble_override(phy_type_misc_ctx_t *ctx, int8 val)
{
	phy_ac_misc_info_t *misc_info = (phy_ac_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;

	if (val != WLC_N_PREAMBLE_MIXEDMODE) {
		PHY_ERROR(("wl%d:%s: AC Phy: Ignore request to set preamble mode %d\n",
			pi->sh->unit, __FUNCTION__, val));
		return;
	}
	pi->n_preamble_override = val;
}

#ifdef WFD_PHY_LL
static void
phy_ac_misc_wfdll_chan_active(phy_type_misc_ctx_t *ctx, bool chan_active)
{
	phy_ac_misc_info_t *misc_info = (phy_ac_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;

	pi->wfd_ll_chan_active = chan_active;
}
#endif /* WFD_PHY_LL */

static uint16
phy_ac_misc_classifier(phy_type_misc_ctx_t *ctx, uint16 mask, uint16 val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	BCM_REFERENCE(mask);

	return phy_rxgcrs_sel_classifier(pi, val);
}
