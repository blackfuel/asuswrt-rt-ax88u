/*
 * NPHY Cache module implementation
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
 * $Id: phy_n_cache.c 671526 2016-11-22 08:37:30Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_cache.h"
#include <phy_n.h>
#include <phy_n_cache.h>
#include <phy_n_info.h>

#include <wlc_phyreg_n.h>
#include <phy_utils_reg.h>
/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phy_radio.h>
#include <wlc_phy_extended_n.h>

/* module private states */
struct phy_n_cache_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_cache_info_t *ci;
};

/* External functions - Include proper header file when functions are moves to appropriate
 * phymod
 */
extern void wlc_phy_txpwrctrl_coeff_setup_nphy(phy_info_t *pi);

/* local functions */
#ifdef PHYCAL_CACHING
static int wlc_phy_cal_cache_restore_nphy(phy_type_cache_ctx_t * cache_ctx);
void wlc_phy_cal_cache_nphy(phy_type_cache_ctx_t * cache_ctx);
static void wlc_phy_cal_cache_dbg_nphy(wlc_phy_t *pih, ch_calcache_t *ctx);
#endif /* PHYCAL_CACHING */
#ifdef BCMDBG
void wlc_phydump_cal_cache_nphy(phy_type_cache_ctx_t * cache_ctx, ch_calcache_t *ctx,
		struct bcmstrbuf *b);
#endif /* BCMDBG */
#if defined(BCMDBG)
/** dump calibration regs/info */
static void
wlc_phy_cal_dump_nphy(phy_type_cache_ctx_t * cache_ct, struct bcmstrbuf *b);
#endif // endif

/* register phy type specific implementation */
phy_n_cache_info_t *
BCMATTACHFN(phy_n_cache_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_cache_info_t *ci)
{
	phy_n_cache_info_t *info;
	phy_type_cache_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_n_cache_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_cache_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ci = ci;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = info;
#ifdef PHYCAL_CACHING
	fns.restore = wlc_phy_cal_cache_restore_nphy;
	fns.cal_cache = wlc_phy_cal_cache_nphy;
#endif /* PHYCAL_CACHING */
#if defined(BCMDBG)
	fns.dump_chanctx = wlc_phydump_cal_cache_nphy;
#endif // endif
#if defined(BCMDBG)
	fns.dump_cal = wlc_phy_cal_dump_nphy;
#endif // endif

	if (phy_cache_register_impl(ci, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_cache_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_cache_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_cache_unregister_impl)(phy_n_cache_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_cache_info_t *ci = info->ci;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_cache_unregister_impl(ci);

	phy_mfree(pi, info, sizeof(phy_n_cache_info_t));
}

#ifdef BCMDBG
void
wlc_phydump_cal_cache_nphy(phy_type_cache_ctx_t * cache_ctx, ch_calcache_t *ctx,
	struct bcmstrbuf *b)
{
	uint i;
	nphy_calcache_t *cache = &ctx->u.nphy_cache;

	for (i = 0; i < 8; i++) {
		bcm_bprintf(b, "txcal_coeffs[%u]:0x%x  ", i, cache->txcal_coeffs[i]);
		if (i % 3 == 0)
			bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "\n");
	for (i = 0; i < 8; i++) {
		bcm_bprintf(b, "txcal_radio_regs[%u]:0x%x  ", i,
		           cache->txcal_radio_regs[i]);
		if (i % 3 == 0)
			bcm_bprintf(b, "\n");
	}
	bcm_bprintf(b, "\n");
	for (i = 0; i < 2; i++) {
		bcm_bprintf(b, "rssical_radio_regs[%u]:0x%x  ", i,
		            cache->rssical_radio_regs[i]);
	}
	bcm_bprintf(b, "\n");
	for (i = 0; i < 12; i++) {
		bcm_bprintf(b, "rssical_phyregs[%u]:0x%x  ", i, cache->rssical_phyregs[i]);
		if (i % 3 == 0)
			PHY_INFORM(("\n"));
	}
	bcm_bprintf(b, "\n IQ comp values-> a0:%d b0:%d a1:%d b1:%d\n\n",
	            cache->rxcal_coeffs.a0,
	            cache->rxcal_coeffs.b0, cache->rxcal_coeffs.a1,
	            cache->rxcal_coeffs.b1);
}
#endif /* BCMDBG */

#if defined(PHYCAL_CACHING)
static int
wlc_phy_cal_cache_restore_nphy(phy_type_cache_ctx_t * cache_ctx)
{
	phy_info_t *pi = (phy_info_t *)cache_ctx;
	uint16 *loft_comp, txcal_coeffs_bphy[4], *tbl_ptr;
	uint8 tx_pwr_ctrl_state;
	ch_calcache_t *ctx;
	nphy_calcache_t *cache = NULL;
	bool suspend;
	int coreNum;

	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	if (!ctx) {
		PHY_ERROR(("wl%d: %s: Chanspec 0x%x not found in calibration cache\n",
		           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));
		return BCME_ERROR;
	}

	if (!ctx->valid) {
		PHY_ERROR(("wl%d: %s: Chanspec 0x%x found, but not valid in phycal cache\n",
		           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));
		return BCME_ERROR;
	}

	PHY_INFORM(("wl%d: %s: Restoring all cal coeffs from calibration cache for chanspec 0x%x\n",
	           pi->sh->unit, __FUNCTION__, pi->radio_chanspec));

	cache = &ctx->u.nphy_cache;

	loft_comp = &cache->txcal_coeffs[5];

	tbl_ptr = cache->txcal_coeffs;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
			/* suspend mac */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	phy_utils_phyreg_enter(pi);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 80, 16,
	                     (void *)cache->txcal_coeffs);

	txcal_coeffs_bphy[0] = tbl_ptr[0];
	txcal_coeffs_bphy[1] = tbl_ptr[1];
	txcal_coeffs_bphy[2] = tbl_ptr[2];
	txcal_coeffs_bphy[3] = tbl_ptr[3];

	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 4, 88, 16, txcal_coeffs_bphy);

	/* Write LO compensation values for OFDM PHY */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 85, 16, loft_comp);
	/* Write LO compensation values for B-PHY */
	wlc_phy_table_write_nphy(pi, NPHY_TBL_ID_IQLOCAL, 2, 93, 16, loft_comp);

	/* Fine LOFT compensation */
	FOREACH_CORE(pi, coreNum) {
		/* The size of txcal_radio_regs is 8 */
		if ((2 * coreNum + 5) >= 8)
			continue;

		/* Fine LOFT compensation */
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_I,
			cache->txcal_radio_regs[2*coreNum]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_Q,
			cache->txcal_radio_regs[2*coreNum+1]);

		/* Coarse LOFT compensation */
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_I,
			cache->txcal_radio_regs[2*coreNum+4]);
		WRITE_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_Q,
			cache->txcal_radio_regs[2*coreNum+5]);
	}

	/* Restore Rx calibration values */
	wlc_phy_rx_iq_coeffs_nphy(pi, 1, &cache->rxcal_coeffs);

	wlc_phy_txpwrctrl_coeff_setup_nphy(pi);
	wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	phy_utils_phyreg_exit(pi);

	/* unsuspend mac */
	if (!suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

	PHY_INFORM(("wl%d: %s: Restored values for chanspec 0x%x are:\n", pi->sh->unit,
	           __FUNCTION__, pi->radio_chanspec));
	wlc_phy_cal_cache_dbg_nphy((wlc_phy_t *)pi, ctx);
	return BCME_OK;
}

void
wlc_phy_cal_cache_nphy(phy_type_cache_ctx_t * cache_ctx)
{
	void *tbl_ptr;
	phy_info_t *pi = (phy_info_t *) cache_ctx;
	ch_calcache_t *ctx;
	nphy_calcache_t *cache;
	int coreNum;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);

	/* A context must have been created before reaching here */
	ASSERT(ctx != NULL);
	if (ctx == NULL)
		return;

	ctx->valid = TRUE;

	cache = &ctx->u.nphy_cache;

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	cache->rssical_radio_regs[0] = phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE0);
	cache->rssical_radio_regs[1] = phy_utils_read_radioreg(pi, RADIO_2057_NB_MASTER_CORE1);

	/* RSSI cal cache values, pulled from wlc_phy_rssi_cal_nphy_rev3() */
	cache->rssical_phyregs[0] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIZ);
	cache->rssical_phyregs[1] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIZ);
	cache->rssical_phyregs[2] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIZ);
	cache->rssical_phyregs[3] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIZ);
	cache->rssical_phyregs[4] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIX);
	cache->rssical_phyregs[5] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIX);
	cache->rssical_phyregs[6] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIX);
	cache->rssical_phyregs[7] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIX);
	cache->rssical_phyregs[8] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0IRSSIY);
	cache->rssical_phyregs[9] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef0QRSSIY);
	cache->rssical_phyregs[10] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1IRSSIY);
	cache->rssical_phyregs[11] = phy_utils_read_phyreg(pi, NPHY_RSSIMultCoef1QRSSIY);

	/* TX/RX IQ cache values */
	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &cache->rxcal_coeffs);

	/* Fine LOFT compensation */
	FOREACH_CORE(pi, coreNum) {
		/* The size of txcal_radio_regs is 8 */
		if ((2 * coreNum + 5) >= 8)
			continue;

		/* Fine LOFT compensation */
		cache->txcal_radio_regs[2*coreNum] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_I);
		cache->txcal_radio_regs[2*coreNum+1] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_FINE_Q);

		/* Coarse LOFT compensation */
		cache->txcal_radio_regs[2*coreNum+4] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_I);
		cache->txcal_radio_regs[2*coreNum+5] =
			READ_RADIO_REG3(pi, RADIO_2057, TX, coreNum, LOFT_COARSE_Q);
	}

	tbl_ptr = cache->txcal_coeffs;
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 80, 16, tbl_ptr);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	PHY_INFORM(("wl%d: %s: Cached cal values for chanspec 0x%x are:\n",
	           pi->sh->unit, __FUNCTION__,  pi->radio_chanspec));
	wlc_phy_cal_cache_dbg_nphy((wlc_phy_t *)pi, ctx);
}

static void
wlc_phy_cal_cache_dbg_nphy(wlc_phy_t *pih, ch_calcache_t *ctx)
{
	uint i;
	nphy_calcache_t *cache = &ctx->u.nphy_cache;
	phy_info_t *pi = (phy_info_t *) pih;

	BCM_REFERENCE(cache);

	if (!ISNPHY(pi))
		return;

	for (i = 0; i < 8; i++) {
		PHY_INFORM(("txcal_coeffs[%u]:0x%x  ", i, cache->txcal_coeffs[i]));
		if (i % 3 == 0)
			PHY_INFORM(("\n"));
	}
	PHY_INFORM(("\n"));
	for (i = 0; i < 8; i++) {
		PHY_INFORM(("txcal_radio_regs[%u]:0x%x  ", i,
		           cache->txcal_radio_regs[i]));
		if (i % 3 == 0)
			PHY_INFORM(("\n"));
	}
	PHY_INFORM(("\n"));
	for (i = 0; i < 2; i++) {
		PHY_INFORM(("rssical_radio_regs[%u]:0x%x  ", i,
		          cache->rssical_radio_regs[i]));
	}
	PHY_INFORM(("\n"));
	for (i = 0; i < 12; i++) {
		PHY_INFORM(("rssical_phyregs[%u]:0x%x  ", i, cache->rssical_phyregs[i]));
		if (i % 3 == 0)
			PHY_INFORM(("\n"));
	}
	PHY_INFORM(("\n IQ comp values-> a0:%d b0:%d a1:%d b1:%d\n\n",
	            cache->rxcal_coeffs.a0,
	            cache->rxcal_coeffs.b0, cache->rxcal_coeffs.a1,
	            cache->rxcal_coeffs.b1));
}
#endif /* PHYCAL_CACHING */

#if defined(BCMDBG)
/** dump calibration regs/info */
static void
wlc_phy_cal_dump_nphy(phy_type_cache_ctx_t * cache_ctx, struct bcmstrbuf *b)
{
	phy_info_t *pi = (phy_info_t *)cache_ctx;
	nphy_iq_comp_t rxcal_coeffs = {0, 0, 0, 0};
	int16 txcal_ofdm_coeffs[8] = {0};
	int16 txcal_bphy_coeffs[8] = {0};
	uint16 rccal_val[NPHY_CORE_NUM] = { 0 };
	uint16 rcal_value = 0;
	int time_elapsed;
	phy_info_nphy_t *pi_nphy = NULL;

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	/* for NPHY, carry out the following steps */
	time_elapsed = pi->sh->now - pi->cal_info->last_cal_time;
	if (time_elapsed < 0)
		time_elapsed = 0;

	bcm_bprintf(b, "time since last cal: %d (sec), mphase_cal_id: %d\n\n",
		time_elapsed, pi->cal_info->cal_phase_id);

	ASSERT(pi_nphy != NULL);

	bcm_bprintf(b, "Pre-calibration txpwr indices: %d, %d\n",
		pi_nphy->nphy_cal_orig_pwr_idx[0],
		pi_nphy->nphy_cal_orig_pwr_idx[1]);

	bcm_bprintf(b, "Tx Calibration pwr indices: %d, %d\n",
		pi_nphy->nphy_txcal_pwr_idx[0], pi_nphy->nphy_txcal_pwr_idx[1]);

	bcm_bprintf(b, "Calibration target txgain Core 0: txlpf=%d,"
		" txgm=%d, pga=%d, pad=%d, ipa=%d\n",
		pi_nphy->nphy_cal_target_gain.txlpf[0],
		pi_nphy->nphy_cal_target_gain.txgm[0],
		pi_nphy->nphy_cal_target_gain.pga[0],
		pi_nphy->nphy_cal_target_gain.pad[0],
		pi_nphy->nphy_cal_target_gain.ipa[0]);

	bcm_bprintf(b, "Calibration target txgain Core 1: txlpf=%d,"
		" txgm=%d, pga=%d, pad=%d, ipa=%d\n",
		pi_nphy->nphy_cal_target_gain.txlpf[1],
		pi_nphy->nphy_cal_target_gain.txgm[1],
		pi_nphy->nphy_cal_target_gain.pga[1],
		pi_nphy->nphy_cal_target_gain.pad[1],
		pi_nphy->nphy_cal_target_gain.ipa[1]);

	bcm_bprintf(b, "Calibration RFSEQ table txgains : 0x%x, 0x%x\n\n",
		pi_nphy->nphy_cal_orig_tx_gain[0],
		pi_nphy->nphy_cal_orig_tx_gain[1]);

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Read Rx calibration co-efficients */
	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxcal_coeffs);

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		rccal_val[0] = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_BCAP_VAL);
		rccal_val[1] = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_SCAP_VAL);
		rcal_value = ((phy_utils_read_radioreg(pi, RADIO_2057_RCAL_STATUS)) >> 1);
	}

	/* Read OFDM Tx calibration co-efficients */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 80, 16, txcal_ofdm_coeffs);

	/* Read BPHY Tx calibration co-efficients */
	wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_IQLOCAL, 8, 88, 16, txcal_bphy_coeffs);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* reg access is done, enable the mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "Tx IQ/LO cal coeffs for OFDM PHY:\n");

	txcal_ofdm_coeffs[0] &= 0x3ff;
	txcal_ofdm_coeffs[1] &= 0x3ff;
	txcal_ofdm_coeffs[2] &= 0x3ff;
	txcal_ofdm_coeffs[3] &= 0x3ff;

	if (txcal_ofdm_coeffs[0] > 511)
		txcal_ofdm_coeffs[0] -= 1024;

	if (txcal_ofdm_coeffs[1] > 511)
		txcal_ofdm_coeffs[1] -= 1024;

	if (txcal_ofdm_coeffs[2] > 511)
		txcal_ofdm_coeffs[2] -= 1024;

	if (txcal_ofdm_coeffs[3] > 511)
		txcal_ofdm_coeffs[3] -= 1024;

	bcm_bprintf(b, "A0=%d, B0=%d, A1=%d, B1=%d\n",
		txcal_ofdm_coeffs[0],
		txcal_ofdm_coeffs[1],
		txcal_ofdm_coeffs[2],
		txcal_ofdm_coeffs[3]);

	bcm_bprintf(b, "Di0=%d, Dq0=%d, Di1=%d, Dq1=%d, m0=%d, m1=%d\n\n\n",
		(int8) ((txcal_ofdm_coeffs[5] & 0xFF00) >> 8),
		(int8) (txcal_ofdm_coeffs[5] & 0x00FF),
		(int8) ((txcal_ofdm_coeffs[6] & 0xFF00) >> 8),
		(int8) (txcal_ofdm_coeffs[6] & 0x00FF),
		(int8) ((txcal_ofdm_coeffs[7] & 0xFF00) >> 8),
		(int8) (txcal_ofdm_coeffs[7] & 0x00FF));

	bcm_bprintf(b, "Tx IQ/LO cal coeffs for BPHY:\n");

	txcal_bphy_coeffs[0] &= 0x3ff;
	txcal_bphy_coeffs[1] &= 0x3ff;
	txcal_bphy_coeffs[2] &= 0x3ff;
	txcal_bphy_coeffs[3] &= 0x3ff;

	if (txcal_bphy_coeffs[0] > 511)
		txcal_bphy_coeffs[0] -= 1024;

	if (txcal_bphy_coeffs[1] > 511)
		txcal_bphy_coeffs[1] -= 1024;

	if (txcal_bphy_coeffs[2] > 511)
		txcal_bphy_coeffs[2] -= 1024;

	if (txcal_bphy_coeffs[3] > 511)
		txcal_bphy_coeffs[3] -= 1024;

	bcm_bprintf(b, "A0=%d, B0=%d, A1=%d, B1=%d\n",
		txcal_bphy_coeffs[0],
		txcal_bphy_coeffs[1],
		txcal_bphy_coeffs[2],
		txcal_bphy_coeffs[3]);

	bcm_bprintf(b, "Di0=%d, Dq0=%d, Di1=%d, Dq1=%d, m0=%d, m1=%d\n\n\n",
		(int8) ((txcal_bphy_coeffs[5] & 0xFF00) >> 8),
		(int8) (txcal_bphy_coeffs[5] & 0x00FF),
		(int8) ((txcal_bphy_coeffs[6] & 0xFF00) >> 8),
		(int8) (txcal_bphy_coeffs[6] & 0x00FF),
		(int8) ((txcal_bphy_coeffs[7] & 0xFF00) >> 8),
		(int8) (txcal_bphy_coeffs[7] & 0x00FF));

	bcm_bprintf(b, "Rx IQ/LO cal coeffs:\n");

	/* Rx calibration coefficients are 10-bit signed integers */
	if (rxcal_coeffs.a0 > 511)
		rxcal_coeffs.a0 -= 1024;

	if (rxcal_coeffs.b0 > 511)
		rxcal_coeffs.b0 -= 1024;

	if (rxcal_coeffs.a1 > 511)
		rxcal_coeffs.a1 -= 1024;

	if (rxcal_coeffs.b1 > 511)
		rxcal_coeffs.b1 -= 1024;

	bcm_bprintf(b, "a0=%d, b0=%d, a1=%d, b1=%d\n\n",
		rxcal_coeffs.a0, rxcal_coeffs.b0, rxcal_coeffs.a1, rxcal_coeffs.b1);

	if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV)) {
		bcm_bprintf(b, "RC CAL value: %d, %d\n", rccal_val[0], rccal_val[1]);
		bcm_bprintf(b, "\nR CAL value: %d\n", rcal_value);
	}
}
#endif // endif
