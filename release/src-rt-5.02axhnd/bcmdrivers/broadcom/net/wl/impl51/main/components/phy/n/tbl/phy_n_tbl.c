/*
 * NPHY PHYTableInit module implementation
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
 * $Id: phy_n_tbl.c 672101 2016-11-24 06:06:10Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_tbl.h"
#include <phy_n.h>
#include <phy_n_tbl.h>
#include <phy_utils_reg.h>

#include "wlc_phytbl_n.h"

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_n.h>
#include <wlc_phyreg_n.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_tbl_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_tbl_info_t *ti;
};

/* local functions */
static int phy_n_tbl_init(phy_type_tbl_ctx_t *ctx);
static int phy_n_tbl_down(phy_type_tbl_ctx_t *ctx);
#if defined(BCMDBG) && defined(DBG_PHY_IOV)
static bool phy_n_tbl_dump_tblfltr(phy_type_tbl_ctx_t *ctx, phy_table_info_t *ti);
static bool phy_n_tbl_dump_addrfltr(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr);
static void phy_n_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval);
static int phy_n_tbl_dumptbl(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_n_tbl_dump_tblfltr NULL
#define phy_n_tbl_dump_addrfltr NULL
#define phy_n_tbl_read_table NULL
#define phy_n_tbl_dumptbl NULL
#endif // endif

/* Register/unregister NPHY specific implementation to common layer */
phy_n_tbl_info_t *
BCMATTACHFN(phy_n_tbl_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_tbl_info_t *ti)
{
	phy_n_tbl_info_t *info;
	phy_type_tbl_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_tbl_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_tbl_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_n_tbl_init;
	fns.down = phy_n_tbl_down;
#if defined(BCMDBG) && defined(DBG_PHY_IOV)
	fns.tblfltr = phy_n_tbl_dump_tblfltr;
	fns.addrfltr = phy_n_tbl_dump_addrfltr;
	fns.readtbl = phy_n_tbl_read_table;
	fns.dump = phy_n_tbl_dumptbl;
#endif // endif
	fns.ctx = info;

	phy_tbl_register_impl(ti, &fns);

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_tbl_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_tbl_unregister_impl)(phy_n_tbl_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tbl_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tbl_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_n_tbl_info_t));
}

/* h/w init/down */
static int
WLBANDINITFN(phy_n_tbl_init)(phy_type_tbl_ctx_t *ctx)
{
	phy_n_tbl_info_t *ti = (phy_n_tbl_info_t *)ctx;
	phy_info_t *pi = ti->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_init_nphy(pi);

	return BCME_OK;
}

/* down h/w */
static int
BCMUNINITFN(phy_n_tbl_down)(phy_type_tbl_ctx_t *ctx)
{
	phy_n_tbl_info_t *ti = (phy_n_tbl_info_t *)ctx;
	phy_n_info_t *ni = ti->ni;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ni->nphy_iqcal_chanspec_2G = 0;
	ni->nphy_iqcal_chanspec_5G = 0;

	return 0;
}

#if defined(BCMDBG)
#if defined(DBG_PHY_IOV)

static phy_table_info_t nphy3_tables[] = {
	{ 0x00, 0,	99 },
	{ 0x01, 0,	99 },
	{ 0x02, 0,	99 },
	{ 0x03, 0,	99 },
	{ 0x04, 0,	105 },
	{ 0x05, 0,	105 },
	{ 0x07, 0,	320 },
	{ 0x08, 0,	48 },
	{ 0x09, 0,	32 }, /* for REV7+, 32 is changed to 64 in phydump_dumptbl */
	{ 0x0a, 1,	768 },
	{ 0x0b, 0,	128 },
	{ 0x0c, 1,	448 },
	{ 0x0d, 1,	8 },
	{ 0x0e, 1,	768 },
	{ 0x0f, 0,	108 },
	{ 0x10, 1,	282 },
	/*	this table is not needed
	{ 0x11, 1,	1024 },
	*/
	{ 0x12, 0,	128 },
	{ 0x13, 1,	512 },
	{ 0x14, 1,	6 },
	{ 0x16, 1,	96 },
	{ 0x18, 0,	32 },
	{ 0x1a, 1,	704 },
	{ 0x1b, 1,	704 },
	{ 0x1e, 1,	5 },
	{ 0x1f, 1,	64 },
	{ 0x20, 1,	64 },
	{ 0x21, 1,	64 },
	{ 0x22, 1,	64 },
	{ 0xff,	0,	0 }
};

static void
phy_n_tbl_read_table(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval)
{
	phy_n_tbl_info_t *ni = (phy_n_tbl_info_t *)ctx;
	phy_info_t *pi = ni->pi;

	phy_utils_write_phyreg(pi, NPHY_TableAddress, (ti->table << 10) | addr);
	*qval = 0;

	if (ti->q) {
		*qval = phy_utils_read_phyreg(pi, NPHY_TableDataLo);
		*val = phy_utils_read_phyreg(pi, NPHY_TableDataHi);
	} else {
		*val = phy_utils_read_phyreg(pi, NPHY_TableDataLo);
	}
}

static int
phy_n_tbl_dumptbl(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = ((phy_n_tbl_info_t *)ctx)->pi;
	phy_table_info_t *ti = nphy3_tables;

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	phy_tbl_do_dumptbl(((phy_n_tbl_info_t *)ctx)->ti, ti, b);

	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	return BCME_OK;
}

static bool
phy_n_tbl_dump_tblfltr(phy_type_tbl_ctx_t *ctx, phy_table_info_t *ti)
{
	phy_n_tbl_info_t *ni = (phy_n_tbl_info_t *)ctx;
	phy_info_t *pi = ni->pi;

	(void)pi;

	if ((ti->table == 0x7) || (ti->table == 0x13)) {
		return FALSE;
	}

	/* For NREV >= 7, AntSwCtrlLUT has 64 entries, not 32 anymore */
	if (ti->table == 0x9) {
		ti->max = 64;
	}

	return TRUE;
}

static bool
phy_n_tbl_dump_addrfltr(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr)
{
	phy_n_tbl_info_t *ni = (phy_n_tbl_info_t *)ctx;
	phy_info_t *pi = ni->pi;

	(void)pi;

	/* For NREV >=7, the holes in Table 0x8 should not be dumped */
	if ((ti->table == 0x8) && (addr >= 0x20)) {
		return FALSE;
	}

	return TRUE;
}
#endif /* INTERNAL || DBG_PHY_IOV */
#endif // endif
