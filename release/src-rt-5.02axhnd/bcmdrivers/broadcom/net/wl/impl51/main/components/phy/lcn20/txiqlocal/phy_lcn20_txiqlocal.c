/*
 * LCN20PHY TXIQLO CAL module implementation
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
 * $Id: phy_lcn20_txiqlocal.c $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_txiqlocal.h>
#include <phy_lcn20.h>
#include <phy_lcn20_txiqlocal.h>

#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_lcn20.h>
#endif // endif

#define LCN20PHY_TBL_ID_IQLOCAL			0x00
#define LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET	96
#define LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET	98

/* register/unregister lcn20PHY specific implementations to/from common */
phy_lcn20_txiqlocal_info_t *phy_lcn20_txiqlocal_register_impl(phy_info_t *pi,
	phy_lcn20_info_t *lcn20i, phy_txiqlocal_info_t *txiqlocali);
void phy_lcn20_txiqlocal_unregister_impl(phy_lcn20_txiqlocal_info_t *info);

/* Inter-module API to other PHY modules */
void wlc_phy_set_tx_iqcc_lcn20phy(phy_info_t *pi, uint16 a, uint16 b);
void wlc_phy_set_tx_locc_lcn20phy(phy_info_t *pi, uint16 didq);

/* module private states */
struct phy_lcn20_txiqlocal_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_txiqlocal_info_t *cmn_info;
};

/* local functions */

static void wlc_phy_get_tx_iqcc_lcn20phy(phy_info_t *pi, uint16 *a, uint16 *b);
static uint16 wlc_phy_get_tx_locc_lcn20phy(phy_info_t *pi);

static void phy_lcn20_txiqlocal_txiqccget(phy_type_txiqlocal_ctx_t *ctx, void *a);
static void phy_lcn20_txiqlocal_txiqccset(phy_type_txiqlocal_ctx_t *ctx, void *b);
static void phy_lcn20_txiqlocal_txloccget(phy_type_txiqlocal_ctx_t *ctx, void *a);
static void phy_lcn20_txiqlocal_txloccset(phy_type_txiqlocal_ctx_t *ctx, void *b);

/* register phy type specific implementation */
phy_lcn20_txiqlocal_info_t *
BCMATTACHFN(phy_lcn20_txiqlocal_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_txiqlocal_info_t *cmn_info)
{
	phy_lcn20_txiqlocal_info_t *lcn20_info;
	phy_type_txiqlocal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((lcn20_info = phy_malloc(pi, sizeof(phy_lcn20_txiqlocal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* initialize ptrs */
	lcn20_info->pi = pi;
	lcn20_info->lcn20i = lcn20i;
	lcn20_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = lcn20_info;
	fns.txiqccget = phy_lcn20_txiqlocal_txiqccget;
	fns.txiqccset = phy_lcn20_txiqlocal_txiqccset;
	fns.txloccget = phy_lcn20_txiqlocal_txloccget;
	fns.txloccset = phy_lcn20_txiqlocal_txloccset;

	if (phy_txiqlocal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return lcn20_info;

	/* error handling */
fail:
	if (lcn20_info) {
		phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_txiqlocal_info_t));
	}
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_txiqlocal_unregister_impl)(phy_lcn20_txiqlocal_info_t *lcn20_info)
{
	phy_txiqlocal_info_t *cmn_info;
	phy_info_t *pi;

	pi = lcn20_info->pi;
	cmn_info = lcn20_info->cmn_info;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_txiqlocal_unregister_impl(cmn_info);

	phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_txiqlocal_info_t));
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static void
wlc_phy_get_tx_iqcc_lcn20phy(phy_info_t *pi, uint16 *a, uint16 *b)
{
	phytbl_info_t tab;
	uint16 iqcc[2];

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tab.tbl_ptr = iqcc; /* ptr to buf */
	tab.tbl_len = 2;        /* # values   */
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL; /* iqloCaltbl */
	tab.tbl_offset = LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_lcn20phy_read_table(pi, &tab);

	*a = iqcc[0];
	*b = iqcc[1];
}

static uint16
wlc_phy_get_tx_locc_lcn20phy(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 didq = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	/* Update iqloCaltbl */
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL; /* iqloCaltbl */
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET;
	wlc_lcn20phy_read_table(pi, &tab);

	return didq;
}

static void phy_lcn20_txiqlocal_txiqccget(phy_type_txiqlocal_ctx_t *ctx, void *a)
{
	phy_lcn20_txiqlocal_info_t *info = (phy_lcn20_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int32 iqccValues[2];
	uint16 valuea = 0;
	uint16 valueb = 0;

	wlc_phy_get_tx_iqcc_lcn20phy(pi, &valuea, &valueb);
	iqccValues[0] = valuea;
	iqccValues[1] = valueb;
	bcopy(iqccValues, a, 2*sizeof(int32));
}

static void phy_lcn20_txiqlocal_txiqccset(phy_type_txiqlocal_ctx_t *ctx, void *p)
{
	phy_lcn20_txiqlocal_info_t *info = (phy_lcn20_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int32 iqccValues[2];
	uint16 valuea, valueb;

	bcopy(p, iqccValues, 2*sizeof(int32));
	valuea = (uint16)(iqccValues[0]);
	valueb = (uint16)(iqccValues[1]);
	wlc_phy_set_tx_iqcc_lcn20phy(pi, valuea, valueb);
}

static void phy_lcn20_txiqlocal_txloccget(phy_type_txiqlocal_ctx_t *ctx, void *a)
{
	phy_lcn20_txiqlocal_info_t *info = (phy_lcn20_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 di0dq0;
	uint8 *loccValues = a;

	di0dq0 = wlc_phy_get_tx_locc_lcn20phy(pi);
	loccValues[0] = (uint8)(di0dq0 >> 8);
	loccValues[1] = (uint8)(di0dq0 & 0xff);
	/* setting analog locc values to 0 */
	loccValues[2] = loccValues[3] = loccValues[4]
		= loccValues[5] = 0;
}
static void phy_lcn20_txiqlocal_txloccset(phy_type_txiqlocal_ctx_t *ctx, void *p)
{
	/* Fix me! There is no effective set operation in original code */
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
void
wlc_phy_set_tx_iqcc_lcn20phy(phy_info_t *pi, uint16 a, uint16 b)
{
	phytbl_info_t tab;
	uint16 iqcc[2];

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Fill buffer with coeffs */
	iqcc[0] = a;
	iqcc[1] = b;
	/* Update iqloCaltbl */
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL; /* iqloCaltbl */
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = iqcc;
	tab.tbl_len = 2;
	tab.tbl_offset = LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);
}

void
wlc_phy_set_tx_locc_lcn20phy(phy_info_t *pi, uint16 didq)
{
	phytbl_info_t tab;

	/* Update iqloCaltbl */
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL;
	tab.tbl_width = 16;
	tab.tbl_ptr = &didq;
	tab.tbl_len = 1;
	tab.tbl_offset = LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);
}
