/*
 * NPHY RSSI Compute module implementation
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
 * $Id: phy_n_rssi.c 671704 2016-11-22 21:37:09Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_misc.h>
#include "phy_type_rssi.h"
#include <phy_n.h>
#include <phy_n_rssi.h>
#include <phy_rssi_iov.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_n.h>
#include <phy_stf.h>
#include <phy_noise_api.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_n.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_rssi_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_rssi_info_t *ri;
	int8 gain_err[PHY_CORE_MAX];
};

/* local functions */
static void phy_n_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);
static void _phy_n_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx);
#if defined(BCMDBG)
static int phy_n_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_n_rssi_dump NULL
#endif // endif
#if defined(WLTEST)
static int phy_n_rssi_get_pkteng_stats(phy_type_rssi_ctx_t *ctx, void *a, int alen,
	wl_pkteng_stats_t stats, int8 *gain_correct);
#endif // endif
static int phy_n_rssi_set_gain_delta_2g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues);
static int phy_n_rssi_get_gain_delta_2g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues);
static int phy_n_rssi_set_gain_delta_5g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues);
static int phy_n_rssi_get_gain_delta_5g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues);

/* register phy type specific functions */
phy_n_rssi_info_t *
BCMATTACHFN(phy_n_rssi_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_rssi_info_t *ri)
{
	phy_n_rssi_info_t *info;
	phy_type_rssi_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_rssi_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_rssi_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ri = ri;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.compute = phy_n_rssi_compute;
	fns.init_gain_err = _phy_n_rssi_init_gain_err;
	fns.dump = phy_n_rssi_dump;
#if defined(WLTEST)
	fns.get_pkteng_stats = phy_n_rssi_get_pkteng_stats;
#endif // endif
	fns.set_gain_delta_2g = phy_n_rssi_set_gain_delta_2g;
	fns.get_gain_delta_2g = phy_n_rssi_get_gain_delta_2g;
	fns.set_gain_delta_5g = phy_n_rssi_set_gain_delta_5g;
	fns.get_gain_delta_5g = phy_n_rssi_get_gain_delta_5g;
	fns.ctx = info;

	phy_rssi_register_impl(ri, &fns);
	phy_rssi_enable_ma(ri, TRUE);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_rssi_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_rssi_unregister_impl)(phy_n_rssi_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_rssi_info_t *ri = info->ri;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_rssi_unregister_impl(ri);

	phy_mfree(pi, info, sizeof(phy_n_rssi_info_t));
}

/* calculate rssi */
static void BCMFASTPATH
phy_n_rssi_compute(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh)
{
	phy_n_rssi_info_t *info = (phy_n_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_info_nphy_t *ni = info->ni;
	d11rxhdr_t *rxh = &wrxh->rxhdr;
	int16 rxpwr, rxpwr0, rxpwr1;
	int16 phyRx0_l, phyRx2_l;
	int32 rssi = 0;
	int32 ii;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	/* mode = 0: rxpwr = max(rxpwr0, rxpwr1)
	 * mode = 1: rxpwr = min(rxpwr0, rxpwr1)
	 * mode = 2: rxpwr = (rxpwr0+rxpwr1)/2
	 */
	rxpwr = 0;
	rxpwr0 = rxh->lt80.PhyRxStatus_1 & PRXS1_nphy_PWR0_MASK;
	rxpwr1 = (rxh->lt80.PhyRxStatus_1 & PRXS1_nphy_PWR1_MASK) >> 8;

	/* Sign extend */
	if (rxpwr0 > 127)
		rxpwr0 -= 256;
	if (rxpwr1 > 127)
		rxpwr1 -= 256;

	/* WAR to deal with the mysterious extra 0-value byte before RX Status bytes. */
	phyRx0_l = rxh->lt80.PhyRxStatus_0 & 0x00ff;
	BCM_REFERENCE(phyRx0_l);
	phyRx2_l = rxh->lt80.PhyRxStatus_2 & 0x00ff;
	if (phyRx2_l > 127)
		phyRx2_l -= 256;

	if (((rxpwr0 == 16) || (rxpwr0 == 32))) {
		rxpwr0 = rxpwr1;
		rxpwr1 = phyRx2_l;
	}

	/* only 2 antennas are valid for now */
	wrxh->rxpwr[0] = (int8)rxpwr0;
	wrxh->rxpwr[1] = (int8)rxpwr1;

	if (stf_shdata->phyrxchain == 0x1)
		rxpwr = rxpwr0;
	else if (stf_shdata->phyrxchain == 0x2)
		rxpwr = rxpwr1;
	else if (stf_shdata->phyrxchain == stf_shdata->hw_phyrxchain) {
		if (pi->sh->rssi_mode == RSSI_ANT_MERGE_MAX)
			rxpwr = (rxpwr0 > rxpwr1) ? rxpwr0 : rxpwr1;
		else if (pi->sh->rssi_mode == RSSI_ANT_MERGE_MIN)
			rxpwr = (rxpwr0 < rxpwr1) ? rxpwr0 : rxpwr1;
		else if (pi->sh->rssi_mode == RSSI_ANT_MERGE_AVG)
			rxpwr = (rxpwr0 + rxpwr1) >> 1;
	}
	else
		ASSERT(0);

	if (rxpwr < -20) /* check less than -10 to be safe */ {
		ni->intf_rssi_vals[ni->intf_rssi_window_idx ] = rxpwr;
		ni->intf_rssi_window_idx ++;
		ni->intf_rssi_window_idx %= PHY_RSSI_WINDOW_SZ;

		for (ii = 0; ii < PHY_RSSI_WINDOW_SZ; ii++) {
			rssi += ni->intf_rssi_vals[ii];
		}
		rssi /= PHY_RSSI_WINDOW_SZ;

		ni->intf_rssi_avg = (int16) rssi;

		if (ni->intf_rssi_avg >= -20) {
			/* SHOULD NEVER GET HERE */
			PHY_ERROR(("WARNING!! pi->nphy->intf_rssi_avg %d >= -20\n",
				ni->intf_rssi_avg));
			ASSERT(0);
		}
	}

	wrxh->rssi = (int8)rxpwr;
	wrxh->rssi_qdb = 0;

	PHY_TRACE(("%s: rssi %d\n", __FUNCTION__, (int8)rxpwr));
}

/* init gain error table */
static void
_phy_n_rssi_init_gain_err(phy_type_rssi_ctx_t *ctx)
{
	phy_n_rssi_info_t *info = (phy_n_rssi_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int16 gainerr[NPHY_CORE_NUM];
	uint16 initgain[NPHY_CORE_NUM];
	uint8 core;
	bool suspend;

	/* Retrieve rxiqest gain error: */
	wlc_phy_get_rxgainerr_phy(pi, gainerr);

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Retrieve init_gain codes from gaintable */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x106, 16, initgain);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	FOREACH_CORE(pi, core) {
		int16 initgain_dB, tmp;

		if (core >= NPHY_CORE_NUM)
			continue;

		/* Convert gain code to dB gain */
		initgain_dB = wlc_phy_rxgaincode_to_dB_nphy(pi, initgain[core]);

		/* gainerr is in 0.5dB steps; round to nearest dB */
		tmp = gainerr[core];
		tmp = ((tmp >= 0) ? ((tmp + 1) >> 1) : -1*((-1*tmp + 1) >> 1));

		info->gain_err[core] = (int8)(NPHY_NOISE_INITGAIN - initgain_dB + tmp);
	}
}

void
phy_n_rssi_init_gain_err(phy_n_rssi_info_t *ri)
{
	_phy_n_rssi_init_gain_err(ri);
}

#if defined(BCMDBG)
static int
phy_n_rssi_dump(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_n_rssi_info_t *info = (phy_n_rssi_info_t *)ctx;
	uint i;

	/* DUMP gain_err... */

	bcm_bprintf(b, "gain err:\n");
	for (i = 0; i < ARRAYSIZE(info->gain_err); i ++)
		bcm_bprintf(b, "  core %u: %u\n", i, info->gain_err[i]);

	return BCME_OK;
}
#endif // endif

#if defined(WLTEST)
static int
phy_n_rssi_get_pkteng_stats(phy_type_rssi_ctx_t *ctx, void *a, int alen,
wl_pkteng_stats_t stats, int8 *gain_correct)
{
	phy_n_rssi_info_t *rssii = (phy_n_rssi_info_t *)ctx;
	phy_info_t *pi = rssii->pi;
	uint16 rxstats_base;
	int i;

	stats.rssi = R_REG(pi->sh->osh, D11_RXE_PHYRS_1(pi)) & 0xff;
	if (stats.rssi > 127) {
		stats.rssi -= 256;
	}
	stats.snr = stats.rssi - PHY_NOISE_FIXED_VAL_NPHY;

	/* rx pkt stats */
	rxstats_base = wlapi_bmac_read_shm(pi->sh->physhim, M_RXSTATS_BLK_PTR(pi));
	for (i = 0; i <= NUM_80211_RATES; i++) {
		stats.rxpktcnt[i] =
			wlapi_bmac_read_shm(pi->sh->physhim, 2*(rxstats_base+i));
	}

	bcopy(&stats, a,
		(sizeof(wl_pkteng_stats_t) < (uint)alen) ? sizeof(wl_pkteng_stats_t) : (uint)alen);

	return BCME_OK;
}
#endif // endif

static int
phy_n_rssi_set_gain_delta_2g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(aid);
	UNUSED_PARAMETER(deltaValues);

	return BCME_UNSUPPORTED;
}

static int
phy_n_rssi_get_gain_delta_2g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(aid);
	UNUSED_PARAMETER(deltaValues);

	return BCME_UNSUPPORTED;
}

static int
phy_n_rssi_set_gain_delta_5g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(aid);
	UNUSED_PARAMETER(deltaValues);

	return BCME_UNSUPPORTED;
}

static int
phy_n_rssi_get_gain_delta_5g(phy_type_rssi_ctx_t *ctx, uint32 aid, int8 *deltaValues)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(aid);
	UNUSED_PARAMETER(deltaValues);

	return BCME_UNSUPPORTED;
}
