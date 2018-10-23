/*
 * N PHY TXIQLO CAL module implementation
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
 * $Id: phy_n_txiqlocal.c 620336 2016-02-22 20:09:51Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_txiqlocal.h>
#include <phy_n.h>
#include <phy_n_txiqlocal.h>
#include <phy_n_radio.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_txiqlocal_info {
	phy_info_t *pi;
	phy_n_info_t *aci;
	phy_txiqlocal_info_t *cmn_info;
/* add other variable size variables here at the end */
};

/* local functions */
#if defined(BCMDBG) || defined(WLTEST)
static void wlc_nphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b, uint16 *a1, uint16 *b1);
static void wlc_nphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b, uint16 a1, uint16 b1);
static void wlc_nphy_get_tx_locc(phy_info_t *pi, uint16 *diq0, uint16 *diq1);
static void wlc_nphy_set_tx_locc(phy_info_t *pi, uint16 diq0, uint16 diq1);

static void phy_n_txiqlocal_txiqccget(phy_type_txiqlocal_ctx_t *ctx, void *a);
static void phy_n_txiqlocal_txiqccset(phy_type_txiqlocal_ctx_t *ctx, void *b);
static void phy_n_txiqlocal_txloccget(phy_type_txiqlocal_ctx_t *ctx, void *a);
static void phy_n_txiqlocal_txloccset(phy_type_txiqlocal_ctx_t *ctx, void *b);
#endif /* defined(BCMDBG) || defined(WLTEST) */

/* register phy type specific implementation */
phy_n_txiqlocal_info_t *
BCMATTACHFN(phy_n_txiqlocal_register_impl)(phy_info_t *pi, phy_n_info_t *aci,
	phy_txiqlocal_info_t *cmn_info)
{
	phy_n_txiqlocal_info_t *ac_info;
	phy_type_txiqlocal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_n_txiqlocal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	bzero(ac_info, sizeof(phy_n_txiqlocal_info_t));
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;
	/* Fix me! chip ID should be removed later! */
#if defined(BCMDBG) || defined(WLTEST)
	if (!(CHIPID_43236X_FAMILY(pi))) {
	fns.txiqccget = phy_n_txiqlocal_txiqccget;
	fns.txiqccset = phy_n_txiqlocal_txiqccset;
	fns.txloccget = phy_n_txiqlocal_txloccget;
	fns.txloccset = phy_n_txiqlocal_txloccset;
	}
#endif /* defined(BCMDBG) || defined(WLTEST) */

	if (phy_txiqlocal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_txiqlocal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_n_txiqlocal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_txiqlocal_unregister_impl)(phy_n_txiqlocal_info_t *ac_info)
{
	phy_txiqlocal_info_t *cmn_info = ac_info->cmn_info;
	phy_info_t *pi = ac_info->pi;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_txiqlocal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_n_txiqlocal_info_t));
}

/* Internal API */
#if defined(BCMDBG) || defined(WLTEST)
static void
wlc_nphy_get_tx_iqcc(phy_info_t *pi, uint16 *a, uint16 *b, uint16 *a1, uint16 *b1)
{
	uint16 iqcc[4];
	phytbl_info_t tab;

	tab.tbl_ptr = iqcc; /* ptr to buf */
	tab.tbl_len = 4;        /* # for core 0   */
	tab.tbl_id = 15;         /* iqloCaltbl      */
	tab.tbl_offset = 80; /* tbl offset */
	tab.tbl_width = 16;     /* 16 bit wide */
	wlc_phy_table_read_nphy(pi, tab.tbl_id, tab.tbl_len, tab.tbl_offset, tab.tbl_width, iqcc);
	*a = iqcc[0];
	*b = iqcc[1];
	*a1 = iqcc[2];
	*b1 = iqcc[3];
}

static void
wlc_nphy_set_tx_iqcc(phy_info_t *pi, uint16 a, uint16 b, uint16 a1, uint16 b1)
{
	phytbl_info_t tab;
	uint16 iqcc[4];

	/* Fill buffer with coeffs */
	iqcc[0] = a;
	iqcc[1] = b;
	iqcc[2] = a1;
	iqcc[3] = b1;

	/* Update iqloCaltbl */
	tab.tbl_id = 15;			/* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = iqcc;
	tab.tbl_len = 4;
	tab.tbl_offset = 80;
	wlc_phy_table_write_nphy(pi, tab.tbl_id, tab.tbl_len, tab.tbl_offset, tab.tbl_width, iqcc);
}

static void
wlc_nphy_get_tx_locc(phy_info_t *pi, uint16 *diq0, uint16 *diq1)
{
	phytbl_info_t tab;
	uint16 didq[2];

	/* Update iqloCaltbl */
	tab.tbl_id = 15;			/* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = didq;
	tab.tbl_len = 2;
	tab.tbl_offset = 85;
	wlc_phy_table_read_nphy(pi, tab.tbl_id, tab.tbl_len, tab.tbl_offset, tab.tbl_width, didq);

	*diq0 = didq[0];
	*diq1 = didq[1];
}

static void
wlc_nphy_set_tx_locc(phy_info_t *pi, uint16 diq0, uint16 diq1)
{
	phytbl_info_t tab;
	uint16 didq[2];

	didq[0] = diq0;
	didq[1] = diq1;
	/* Update iqloCaltbl */
	tab.tbl_id = 15;			/* iqloCaltbl		*/
	tab.tbl_width = 16;	/* 16 bit wide	*/
	tab.tbl_ptr = didq;
	tab.tbl_len = 2;
	tab.tbl_offset = 85;
	wlc_phy_table_write_nphy(pi, tab.tbl_id, tab.tbl_len, tab.tbl_offset, tab.tbl_width, didq);
}

static void phy_n_txiqlocal_txiqccget(phy_type_txiqlocal_ctx_t *ctx, void *a)
{
	phy_n_txiqlocal_info_t *info = (phy_n_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int32 iqccValues[4];
	uint16 valuea = 0;
	uint16 valueb = 0;
	uint16 valuea1 = 0;
	uint16 valueb1 = 0;

	wlc_nphy_get_tx_iqcc(pi, &valuea, &valueb, &valuea1, &valueb1);
	iqccValues[0] = valuea;
	iqccValues[1] = valueb;
	iqccValues[2] = valuea1;
	iqccValues[3] = valueb1;
	bcopy(iqccValues, a, 4*sizeof(int32));
}

static void phy_n_txiqlocal_txiqccset(phy_type_txiqlocal_ctx_t *ctx, void *p)
{
	phy_n_txiqlocal_info_t *info = (phy_n_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int32 iqccValues[4];
	uint16 valuea, valueb, valuea1, valueb1;

	bcopy(p, iqccValues, 4*sizeof(int32));
	valuea = (uint16)(iqccValues[0]);
	valueb = (uint16)(iqccValues[1]);
	valuea1 = (uint16)(iqccValues[2]);
	valueb1 = (uint16)(iqccValues[3]);
	wlc_nphy_set_tx_iqcc(pi, valuea, valueb, valuea1, valueb1);
}

static void phy_n_txiqlocal_txloccget(phy_type_txiqlocal_ctx_t *ctx, void *a)
{
	phy_n_txiqlocal_info_t *info = (phy_n_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint16 di0dq0;
	uint16 di1dq1;
	uint8 *loccValues = a;

	/* copy the 6 bytes to a */
	wlc_nphy_get_tx_locc(pi, &di0dq0, &di1dq1);
	loccValues[0] = (uint8)(di0dq0 >> 8);
	loccValues[1] = (uint8)(di0dq0 & 0xff);
	loccValues[6] = (uint8)(di1dq1 >> 8);
	loccValues[7] = (uint8)(di1dq1 & 0xff);
	wlc_nphy_get_radio_loft(pi, &loccValues[2], &loccValues[3],
		&loccValues[4], &loccValues[5], &loccValues[8],
		&loccValues[9], &loccValues[10], &loccValues[11]);
}

static void phy_n_txiqlocal_txloccset(phy_type_txiqlocal_ctx_t *ctx, void *p)
{
	phy_n_txiqlocal_info_t *info = (phy_n_txiqlocal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	/* copy 6 bytes from a to radio */
	uint16 di0dq0, di1dq1;
	uint8 *loccValues = p;

	di0dq0 = ((uint16)loccValues[0] << 8) | loccValues[1];
	di1dq1 = ((uint16)loccValues[6] << 8) | loccValues[7];
	wlc_nphy_set_tx_locc(pi, di0dq0, di1dq1);
	wlc_nphy_set_radio_loft(pi, loccValues[2],
		loccValues[3], loccValues[4], loccValues[5],
		loccValues[8], loccValues[9], loccValues[10],
		loccValues[11]);
}
#endif /* defined(BCMDBG) || defined(WLTEST) */

/* External API */
