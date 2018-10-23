/*
 * TxPowerCtrl module implementation.
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
 * $Id: phy_tpc.c 759380 2018-04-25 06:54:57Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include <phy_rstr.h>
#include "phy_type_tpc.h"
#include <phy_tpc_api.h>
#include <phy_tpc.h>
#include <phy_utils_channel.h>
#include <phy_utils_var.h>
#include <phy_stf.h>
#include <phy_btcx_api.h>

/* ******* Local Functions ************ */

/* ********************************** */

/* module private states */
struct phy_tpc_priv_info {
	phy_info_t *pi;
	phy_type_tpc_fns_t *fns;
	uint8 ucode_tssi_limit_en;
};

/* module private states memory layout */
typedef struct {
	phy_tpc_info_t info;
	phy_type_tpc_fns_t fns;
	phy_tpc_priv_info_t priv;
	phy_tpc_data_t data;
	phy_tpc_config_info_t cfg;
/* add other variable size variables here at the end */
} phy_tpc_mem_t;

/* local function declaration */
static int phy_tpc_init(phy_init_ctx_t *ctx);
static void wlc_phy_read_srom9_txpwr_ppr(phy_info_t *pi);

#if defined(WLTXPWR_CACHE) && defined(TXPWRBACKOFF)
static bool
wlc_phy_ppr_check_vt_backoff(phy_tpc_info_t *tpci, chanspec_t chspec);
#endif /* WLTXPWR_CACHE && TXPWRBACKOFF */

#ifdef BAND5G
static int8 wlc_phy_convert_srom_txpwr40Moffset(uint8 offset);
#endif // endif
/* attach/detach */
phy_tpc_info_t *
BCMATTACHFN(phy_tpc_attach)(phy_info_t *pi)
{
	phy_tpc_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_tpc_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->priv = &((phy_tpc_mem_t *)info)->priv;
	info->priv->pi = pi;
	info->priv->fns = &((phy_tpc_mem_t *)info)->fns;
	info->data = &((phy_tpc_mem_t *)info)->data;
	info->data->cfg = &((phy_tpc_mem_t *)info)->cfg;

#if defined(POWPERCHANNL) && !defined(POWPERCHANNL_DISABLED)
		pi->_powerperchan = TRUE;
#endif // endif
	/* Initialize variables */
	info->priv->ucode_tssi_limit_en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssilimucod, 1);
	info->data->cfg->bphy_scale = (uint16)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_bphyscale, 0);
	info->data->cfg->initbaseidx2govrval = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_initbaseidx2govrval, 255);
	info->data->cfg->initbaseidx5govrval = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_initbaseidx5govrval, 255);
	info->data->cfg->srom_tworangetssi2g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_tworangetssi2g, FALSE);
	info->data->cfg->srom_tworangetssi5g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_tworangetssi5g, FALSE);
	info->data->cfg->srom_2g_pdrange_id = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_pdgain2g, 0);
	info->data->cfg->srom_5g_pdrange_id = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_pdgain5g, 0);
#ifdef FCC_PWR_LIMIT_2G
	info->data->cfg->fccpwrch12 = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_fccpwrch12, 0);
	info->data->cfg->fccpwrch13 = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_fccpwrch13, 0);
	info->data->cfg->fccpwroverride =
		(int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_fccpwroverride, 0);
#endif /* FCC_PWR_LIMIT_2G */
	/* set default power output percentage to 100 percent */
	info->data->txpwr_percent = 100;
	info->data->txpwr_degrade = 0;
	/* initialize our txpwr limit to a large value until we know what band/channel
	 * we settle on in wlc_up() set the txpwr user override to the max
	 */
	info->data->tx_user_target = WLC_TXPWR_MAX;
	/* default radio power */
	info->data->radiopwr_override = RADIOPWR_OVERRIDE_DEF;
#ifdef WL_SARLIMIT
	memset(info->data->sarlimit, WLC_TXPWR_MAX, PHY_MAX_CORES);
#endif /* WL_SARLIMIT */
#ifdef PREASSOC_PWRCTRL
	info->data->channel_short_window = TRUE;
#endif // endif

	/* Register callbacks */

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_tpc_init, info, PHY_INIT_TPC) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register the reuse buffer size to hold 3 ppr instances */
	phy_cache_register_reuse_size(pi->cachei, 3 * ppr_size(ppr_get_max_bw()));

	return info;

	/* error */
fail:
	phy_tpc_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_tpc_detach)(phy_tpc_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null tpc module\n", __FUNCTION__));
		return;
	}

	pi = info->priv->pi;

	phy_mfree(pi, info, sizeof(phy_tpc_mem_t));
}

/* TPC init */
static int
WLBANDINITFN(phy_tpc_init)(phy_init_ctx_t *ctx)
{
	phy_tpc_info_t *tpci = (phy_tpc_info_t *)ctx;
	phy_type_tpc_fns_t *fns = tpci->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->init != NULL) {
		return (fns->init)(fns->ctx);
	}
	return BCME_OK;
}

bool
phy_tpc_get_tworangetssi2g(phy_tpc_info_t *tpci)
{
	return tpci->data->cfg->srom_tworangetssi2g;
}

bool
phy_tpc_get_tworangetssi5g(phy_tpc_info_t *tpci)
{
	return tpci->data->cfg->srom_tworangetssi5g;
}

uint8
phy_tpc_get_2g_pdrange_id(phy_tpc_info_t *tpci)
{
	return tpci->data->cfg->srom_2g_pdrange_id;
}

uint8
phy_tpc_get_5g_pdrange_id(phy_tpc_info_t *tpci)
{
	return tpci->data->cfg->srom_5g_pdrange_id;
}

/* Translates the regulatory power limit array into an array of length TXP_NUM_RATES,
 * which can match the board limit array obtained using the SROM. Moreover, since the NPHY chips
 * currently do not distinguish between Legacy OFDM and MCS0-7, the SISO and CDD regulatory power
 * limits of these rates need to be combined carefully.
 * This internal/static function needs to be called whenever the chanspec or regulatory TX power
 * limits change.
 */
static void
wlc_phy_txpower_reg_limit_calc(phy_info_t *pi, ppr_t *txpwr, chanspec_t chanspec,
	ppr_t *txpwr_limit)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	ppr_ht_mcs_rateset_t mcs_limits;

	ppr_copy_struct(txpwr, txpwr_limit);

#if defined(WLPROPRIETARY_11N_RATES)
	ppr_get_ht_mcs(txpwr_limit, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, &mcs_limits);
	if (mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_87] == WL_RATE_DISABLED) {
		mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_87] =
			mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_7];
		mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_88] =
			mcs_limits.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_7];
		ppr_set_ht_mcs(txpwr_limit, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
			WL_TX_CHAINS_1, &mcs_limits);
	}
#endif // endif

	/* Obtain the regulatory limits for Legacy OFDM and HT-OFDM 11n rates in NPHY chips */
	if (fns->reglimit_calc != NULL) {
		(fns->reglimit_calc)(fns->ctx, txpwr, txpwr_limit, &mcs_limits);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

#ifdef PREASSOC_PWRCTRL
void
phy_preassoc_pwrctrl_upd(phy_info_t *pi, chanspec_t chspec)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	if ((pi->radio_chanspec != chspec) && fns->shortwindow_upd != NULL &&
	    fns->store_setting != NULL && pi->sh->up) {
		(fns->store_setting)(fns->ctx, pi->radio_chanspec);
		(fns->shortwindow_upd)(fns->ctx, TRUE);
	} else {
		ti->data->channel_short_window = TRUE;
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}
#endif /* PREASSOC_PWRCTRL */

#ifdef WLTXPWR_CACHE
/* Retrieve the cached ppr targets and pass them to the hardware function. */
static void
wlc_phy_txpower_retrieve_cached_target(phy_info_t *pi)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		pi->tpci->data->txpwr_max_boardlim_percore[core] =
			wlc_phy_get_cached_boardlim(pi->txpwr_cache, pi->radio_chanspec, core);
		pi->tx_power_max_per_core[core] =
			wlc_phy_get_cached_pwr_max(pi->txpwr_cache, pi->radio_chanspec, core);
		pi->tx_power_min_per_core[core] =
			wlc_phy_get_cached_pwr_min(pi->txpwr_cache, pi->radio_chanspec, core);
		pi->openlp_tx_power_min = pi->tx_power_min_per_core[core];
	}
	pi->tpci->data->txpwrnegative = 0;

#ifdef WL_SARLIMIT
	wlc_phy_sar_limit_set((wlc_phy_t*)pi,
		wlc_phy_get_cached_sar_lims(pi->txpwr_cache, pi->radio_chanspec));
#endif // endif

	phy_tpc_recalc_tgt(pi->tpci);
}

#ifdef TXPWRBACKOFF
static bool
wlc_phy_ppr_check_vt_backoff(phy_tpc_info_t *ti, chanspec_t chspec)
{
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	if (fns->check_vt) {
		return (fns->check_vt)(fns->ctx, chspec);
	} else {
		return TRUE;
	}
}
#endif /* TXPWRBACKOFF */
#endif /* WLTXPWR_CACHE */

/* recalc target txpwr and apply to h/w */
void
phy_tpc_recalc_tgt(phy_tpc_info_t *ti)
{
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	ASSERT(fns->recalc != NULL);
	(fns->recalc)(fns->ctx);
}

/* Recalc target power all phys.  This internal/static function needs to be called whenever
 * the chanspec or TX power values (user target, regulatory limits or SROM/HW limits) change.
 * This happens thorough calls to the PHY public API.
 */
/* TODO: The code could be optimized by moving the common code to phy/cmn */
/* [PHY_RE_ARCH] There are two functions: Bigger function wlc_phy_txpower_recalc_target
 * and smaller function phy_tpc_recalc_tgt which in turn call their phy specific functions
 * which are named in a haphazard manner. This needs to be cleaned up.
 */
void
wlc_phy_txpower_recalc_target(phy_info_t *pi, ppr_t *txpwr_reg, ppr_t *txpwr_targets)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	chanspec_t chspec = pi->radio_chanspec;
	ppr_t *srom_max_txpwr;
	ppr_t *tx_pwr_target;
	ppr_t *reg_txpwr_limit;

	uint pprsize = ppr_size(PPR_CHSPEC_BW(chspec));
	int8 *ppr_buf = NULL;

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	/* Change to current bandwidth */
	ppr_set_ch_bw(pi->tx_power_offset, PPR_CHSPEC_BW(chspec));
	ppr_clear(pi->tx_power_offset);

#ifdef WLTXPWR_CACHE
	if ((!ti->data->txpwroverride) && wlc_phy_get_cached_pwr(pi->txpwr_cache,
		pi->radio_chanspec, TXPWR_CACHE_POWER_OFFSETS, pi->tx_power_offset) &&
#ifdef TXPWRBACKOFF
		wlc_phy_ppr_check_vt_backoff(ti, pi->radio_chanspec) &&
#endif /* TXPWRBACKOFF */
		(txpwr_targets == NULL)) {
		wlc_phy_txpower_retrieve_cached_target(pi);
		PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
#ifdef WL_UCM
		/* Update SISO response power */
		phy_btcx_ucm_update_only_ack_pwr_offset((wlc_phy_t*)pi);
#endif /* WL_UCM */

		return;
	}
#endif /* WLTXPWR_CACHE */

	/* Allocate 3 ppr instance from the scratch buffer */
	ppr_buf = phy_cache_acquire_reuse_buffer(pi->cachei, pprsize * 3);
	reg_txpwr_limit = ppr_create_prealloc(PPR_CHSPEC_BW(chspec), ppr_buf, pprsize);

	if (txpwr_reg != NULL)
		wlc_phy_txpower_reg_limit_calc(pi, txpwr_reg, pi->radio_chanspec,
		reg_txpwr_limit);

	tx_pwr_target = ppr_create_prealloc(PPR_CHSPEC_BW(chspec), ppr_buf + pprsize,
		pprsize);
	srom_max_txpwr = ppr_create_prealloc(PPR_CHSPEC_BW(chspec), ppr_buf + 2 * pprsize,
		pprsize);

	if (fns->recalc_target != NULL) {
		(fns->recalc_target)(fns->ctx, tx_pwr_target, srom_max_txpwr, reg_txpwr_limit,
		    txpwr_targets);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}

	phy_cache_release_reuse_buffer(pi->cachei);

	/* Don't call the hardware specifics if we were just trying to retrieve the target powers */
	if (txpwr_targets == NULL) {
		phy_tpc_recalc_tgt(pi->tpci);
	}
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
#ifdef WL_UCM
	/* Update SISO response power */
	phy_btcx_ucm_update_only_ack_pwr_offset((wlc_phy_t*)pi);
#endif /* WL_UCM */

}

#ifdef PPR_API
/* for CCK case, 4 rates only */
void
wlc_phy_txpwr_srom_convert_cck(uint16 po, uint8 max_pwr, ppr_dsss_rateset_t *dsss)
{
	uint8 i;
	/* Extract offsets for 4 CCK rates, convert from .5 to .25 dbm units. */
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++) {
		dsss->pwr[i] = max_pwr - ((po & 0xf) * 2);
		po >>= 4;
	}
}

/* for OFDM cases, 8 rates only */
void
wlc_phy_txpwr_srom_convert_ofdm(uint32 po, uint8 max_pwr, ppr_ofdm_rateset_t *ofdm)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		ofdm->pwr[i] = max_pwr - ((po & 0xf) * 2);
		po >>= 4;
	}
}

void
wlc_phy_ppr_set_dsss(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_dsss_rateset_t* pwr_offsets, phy_info_t *pi) {
	uint8 chain;
	for (chain = WL_TX_CHAINS_1; chain <= PHYCORENUM(pi->pubpi->phy_corenum); chain++)
		/* for 2g_dsss: S1x1, S1x2, S1x3 */
		ppr_set_dsss(tx_srom_max_pwr, bwtype, chain,
		      (const ppr_dsss_rateset_t*)pwr_offsets);
}

void
wlc_phy_ppr_set_ofdm(ppr_t* tx_srom_max_pwr, uint8 bwtype,
          ppr_ofdm_rateset_t* pwr_offsets, phy_info_t *pi) {

	uint8 chain;
	ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
	       (const ppr_ofdm_rateset_t*)pwr_offsets);
	BCM_REFERENCE(chain);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		for (chain = WL_TX_CHAINS_2; chain <= PHYCORENUM(pi->pubpi->phy_corenum); chain++) {
			ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_CDD, chain,
				(const ppr_ofdm_rateset_t*)pwr_offsets);
#ifdef WL_BEAMFORMING
			/* Add TXBF */
			ppr_set_ofdm(tx_srom_max_pwr, bwtype, WL_TX_MODE_TXBF, chain,
				(const ppr_ofdm_rateset_t*)pwr_offsets);
#endif // endif
		}
	}
}
#endif /* PPR_API */

void
wlc_phy_txpwr_srom_convert_mcs_offset(uint32 po,
	uint8 offset, uint8 max_pwr, ppr_ht_mcs_rateset_t* mcs, int8 mcs7_15_offset)
{
	uint8 i;
	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
		mcs->pwr[i] = max_pwr - ((po & 0xF) * 2) - (offset * 2);
		po >>= 4;
	}

	mcs->pwr[WL_RATESET_SZ_HT_MCS - 1] -= mcs7_15_offset;
}

void
wlc_phy_txpwr_srom_convert_mcs(uint32 po, uint8 max_pwr, ppr_ht_mcs_rateset_t *mcs)
{
	wlc_phy_txpwr_srom_convert_mcs_offset(po, 0, max_pwr, mcs, 0);
}

static void
wlc_phy_txpwr_srom9_convert(phy_info_t *pi, int8 *srom_max,
                                            uint32 pwr_offset, uint8 tmp_max_pwr, uint8 rate_cnt)
{
	uint8 rate;
	uint8 nibble;

	if (pi->sh->sromrev < 9) {
		ASSERT(0 && "SROMREV < 9");
		return;
	}

	for (rate = 0; rate < rate_cnt; rate++) {
		nibble = (uint8)(pwr_offset & 0xf);
		pwr_offset >>= 4;
		/* nibble info indicates offset in 0.5dB units convert to 0.25dB */
		srom_max[rate] = (int8)(tmp_max_pwr - (nibble << 1));
	}
}

static void
wlc_phy_ppr_set_ht_mcs(ppr_t* tx_srom_max_pwr, uint8 bwtype,
         ppr_ht_mcs_rateset_t* pwr_offsets, phy_info_t *pi) {
	ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		/* for ht_S1x2_CDD */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		/* for ht_S2x2_STBC */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		/* for ht_S2x2_SDM */
		ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
			WL_TX_CHAINS_2, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
			/* for ht_S1x3_CDD */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_1, WL_TX_MODE_CDD,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S2x3_STBC */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_STBC,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S2x3_SDM */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_2, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
			/* for ht_S3x3_SDM */
			ppr_set_ht_mcs(tx_srom_max_pwr, bwtype, WL_TX_NSS_3, WL_TX_MODE_NONE,
				WL_TX_CHAINS_3, (const ppr_ht_mcs_rateset_t*)pwr_offsets);
		}
	}
}

void
wlc_phy_txpwr_apply_srom9(phy_info_t *pi, uint8 band_num, chanspec_t chanspec,
	uint8 tmp_max_pwr, ppr_t *tx_srom_max_pwr)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	ppr_dsss_rateset_t cck20_offset_ppr_api, cck20in40_offset_ppr_ppr_api;

	ppr_ofdm_rateset_t ofdm20_offset_ppr_api;
	ppr_ofdm_rateset_t ofdm20in40_offset_ppr_api;
	ppr_ofdm_rateset_t ofdmdup40_offset_ppr_api;

	ppr_ht_mcs_rateset_t mcs20_offset_ppr_api;
	ppr_ht_mcs_rateset_t mcs40_offset_ppr_api;
	ppr_ht_mcs_rateset_t mcs20in40_offset_ppr_api;

	ASSERT(tx_srom_max_pwr);

	tmp_max_pwr = pwrdet->max_pwr[0][band_num];

	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[1][band_num]);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		tmp_max_pwr = MIN(tmp_max_pwr, pwrdet->max_pwr[2][band_num]);

	switch (band_num) {
	case WL_CHAN_FREQ_RANGE_2G:
		if (CHSPEC_BW_LE20(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, cck20_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.cckbw202gpo, tmp_max_pwr,
			                            WL_RATESET_SZ_DSSS);
			/* populating tx_srom_max_pwr = pi->tx_srom_max_pwr[band]
			   structure
			*/
			/* for 2g_dsss_20IN20: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20,
			                     &cck20_offset_ppr_api, pi);
		}
		else if (CHSPEC_IS40(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, cck20in40_offset_ppr_ppr_api.pwr,
			                            pi->ppr->u.sr9.cckbw20ul2gpo, tmp_max_pwr,
			                            WL_RATESET_SZ_DSSS);
			/* for 2g_dsss_20IN40: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &cck20in40_offset_ppr_ppr_api, pi);
		}
		/* Fall through to set OFDM and .11n rates for 2.4GHz band */
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		/* OFDM srom conversion */
		/* ofdm_20IN20: S1x1, S1x2, S1x3 */
		/*  pwr_offsets = pi->ppr->u.sr9.ofdm[band_num].bw20; */
		if (CHSPEC_BW_LE20(chanspec)) {
			wlc_phy_txpwr_srom9_convert(pi, ofdm20_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.ofdm[band_num].bw20,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/* ofdm_20IN20: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, &ofdm20_offset_ppr_api,
				pi);
			/* HT srom conversion  */
			/*  20MHz HT */
			/* rate_cnt = WL_RATESET_SZ_HT_MCS; */
			/* pwr_offsets = pi->ppr->u.sr9.mcs[band_num].bw20; */
			wlc_phy_txpwr_srom9_convert(pi, mcs20_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.mcs[band_num].bw20,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 20MHz HT  */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_20, &mcs20_offset_ppr_api, pi);
		}
		else if (CHSPEC_IS40(chanspec)) {
			/* * ofdm 20 in 40 */
			/* pwr_offsets = pi->ppr->u.sr9.ofdm[band_num].bw20ul; */
			wlc_phy_txpwr_srom9_convert(pi, ofdm20in40_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.ofdm[band_num].bw20ul,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/*  ofdm dup */
			/*  pwr_offsets = pi->ppr->u.sr9.ofdm[band_num].bw40; */
			wlc_phy_txpwr_srom9_convert(pi, ofdmdup40_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.ofdm[band_num].bw40,
			                            tmp_max_pwr, WL_RATESET_SZ_OFDM);
			/* ofdm_20IN40: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20IN40,
			                     &ofdm20in40_offset_ppr_api, pi);
			/* ofdm DUP: S1x1, S1x2, S1x3 */
			wlc_phy_ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_40,
			                     &ofdmdup40_offset_ppr_api, pi);

			/* 40MHz HT  */

			/* pwr_offsets = pi->ppr->u.sr9.mcs[band_num].bw20ul; */
			wlc_phy_txpwr_srom9_convert(pi, mcs20in40_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.mcs[band_num].bw20ul,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 20IN40MHz HT */
			/* pwr_offsets = pi->ppr->u.sr9.mcs[band_num].bw40; */
			wlc_phy_txpwr_srom9_convert(pi, mcs40_offset_ppr_api.pwr,
			                            pi->ppr->u.sr9.mcs[band_num].bw40,
			                            tmp_max_pwr, WL_RATESET_SZ_HT_MCS);
			/* 40MHz HT */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_40, &mcs40_offset_ppr_api, pi);
			/* 20IN40MHz HT */
			wlc_phy_ppr_set_ht_mcs(tx_srom_max_pwr,
			                       WL_TX_BW_20IN40, &mcs20in40_offset_ppr_api, pi);
		}
			break;
		default:
			break;
		}
}

/* CCK Pwr Index Convergence Correction */
void
phy_tpc_cck_corr(phy_info_t *pi)
{
	ppr_dsss_rateset_t dsss;
	uint rate;
	int32 temp;
	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
	for (rate = 0; rate < WL_RATESET_SZ_DSSS; rate++) {
			temp = (int32)(dsss.pwr[rate]);
			temp += pi->sromi->cckPwrIdxCorr;
			dsss.pwr[rate] = (int8)((uint8)temp);
	}
	ppr_set_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
}

/* check limit? */
void
phy_tpc_check_limit(phy_info_t *pi)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!ti->priv->ucode_tssi_limit_en)
		return;

	if (fns->check == NULL)
		return;

	(fns->check)(fns->ctx);
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_tpc_register_impl)(phy_tpc_info_t *ti, phy_type_tpc_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_tpc_unregister_impl)(phy_tpc_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

#ifdef WLOLPC
void
wlc_phy_update_olpc_cal(wlc_phy_t *ppi, bool set, bool dbg)
{
	phy_info_t * pi = (phy_info_t *)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->update_cal != NULL) {
		(fns->update_cal)(fns->ctx, set, dbg);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

int8
wlc_phy_calc_ppr_pwr_cap(wlc_phy_t *ppi, uint8 core)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (core >= PHY_CORE_MAX) {
		PHY_ERROR(("%s: Invalid PHY core[0x%02x], setting cap to %d,"
			" hw_phytxchain[0x%02x] hw_phyrxchain[0x%02x]\n",
			__FUNCTION__, core, 127, stf_shdata->hw_phytxchain,
			stf_shdata->hw_phyrxchain));
		 return 127;
	}
	return pi->tpci->data->adjusted_pwr_cap[core];
}
#endif /* WLOLPC */

static bool
wlc_phy_cal_txpower_recalc_sw(phy_info_t *pi)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* NPHY/ACPHY doesn't ever use SW power control */
	if (fns->recalc_sw != NULL) {
		return (fns->recalc_sw)(fns->ctx);
	} else {
		return FALSE;
	}
}

/* Set tx power limits */
/* BMAC_NOTE: this only needs a chanspec so that it can choose which 20/40 limits
 * to save in phy state. Would not need this if we ether saved all the limits and
 * applied them only when we were on the correct channel, or we restricted this fn
 * to be called only when on the correct channel.
 */
/* FTODO make sure driver functions are calling this version */
#ifdef FCC_PWR_LIMIT_2G
static bool
wlc_phy_isvalid_fcc_pwr_limit(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint prev_ch = CHSPEC_CHANNEL(pi->previous_chanspec);
	uint target_ch = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* channel validity check */
	if ((prev_ch == 12 || prev_ch == 13 ||
		target_ch == 12 || target_ch == 13)) {
		return TRUE;
	}
	return FALSE;
}

void
wlc_phy_prev_chanspec_set(wlc_phy_t *ppi, chanspec_t prev_chanspec)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->previous_chanspec = prev_chanspec;
}
#endif /* FCC_PWR_LIMIT_2G */

void
wlc_phy_txpower_limit_set(wlc_phy_t *ppi, ppr_t *txpwr, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;
#ifdef FCC_PWR_LIMIT_2G
	phy_tpc_data_t *data = pi->tpci->data;
#endif /* FCC_PWR_LIMIT_2G */

#ifdef TXPWR_TIMING
	int time1, time2;
	time1 = hnd_time_us();
#endif // endif
	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	wlc_phy_txpower_recalc_target(pi, txpwr, NULL);
	wlc_phy_cal_txpower_recalc_sw(pi);
	wlapi_enable_mac(pi->sh->physhim);
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);

#ifdef FCC_PWR_LIMIT_2G
	if ((data->cfg->fccpwrch12 > 0 || data->cfg->fccpwrch13 > 0) &&
		(data->fcc_pwr_limit_2g || data->cfg->fccpwroverride)) {
		if (wlc_phy_isvalid_fcc_pwr_limit(ppi)) {
			wlc_phy_fcc_pwr_limit_set(ppi, TRUE);
		}
	}
#endif /* FCC_PWR_LIMIT_2G */

#ifdef TXPWR_TIMING
	time2 = hnd_time_us();
	wlc_phy_txpower_limit_set_time = time2 - time1;
#endif // endif
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_txpower_set(wlc_phy_t *ppi, int8 qdbm, bool override, ppr_t *reg_pwr)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	/* No way for user to set maxpower on individual rates yet.
	 * Same max power is used for all rates
	 */
	ti->data->tx_user_target = qdbm;

	/* Restrict external builds to 100% Tx Power */
#if defined(WLTEST) || defined(WL_EXPORT_TXPOWER)
	ti->data->txpwroverride = override;
	ti->data->txpwroverrideset = override;
#else
	ti->data->txpwroverride = FALSE;
#endif // endif

	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			bool suspend;

			suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) &
			            MCTL_EN_MAC);

			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (fns->set != NULL) {
				(fns->set)(fns->ctx, reg_pwr);
			} else {
				wlc_phy_txpower_recalc_target(pi, reg_pwr, NULL);
				wlc_phy_cal_txpower_recalc_sw(pi);
			}

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
		}
	}
	return (0);
}

/* get sromlimit per rate for given channel. Routine does not account for ant gain */
void
wlc_phy_txpower_sromlimit(wlc_phy_t *ppi, chanspec_t chanspec, uint8 *min_pwr,
    ppr_t *max_pwr, uint8 core)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (min_pwr)
		*min_pwr = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
	if (max_pwr) {
		if (fns->get_sromlimit != NULL) {
			(fns->get_sromlimit)(fns->ctx, chanspec, max_pwr, core);
		} else {
			ppr_set_cmn_val(max_pwr, (int8)WLC_TXPWR_MAX);
		}
	}
}

int
wlc_phy_txpower_get_current(wlc_phy_t *ppi, ppr_t *reg_pwr, phy_tx_power_t *power)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	int ret;

	if (!pi->sh->up) {
		ret = BCME_NOTUP;
		PHY_ERROR(("wl%d: %s: PHY not up: %d\n", pi->sh->unit,
			__FUNCTION__, ret));
		return ret;
	}

	if (fns->setflags != NULL) {
		(fns->setflags)(fns->ctx, power);
	} else {
		power->rf_cores = 1;
		if (ti->data->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
			power->flags |= WL_TX_POWER_F_ENABLED;
		if (pi->hwpwrctrl)
			power->flags |= WL_TX_POWER_F_HW;
	}

	wlc_phy_get_ppr_board_limits(ppi, power->ppr_board_limits);
	/* reuse txpwr for target */
	wlc_phy_txpower_recalc_target(pi, reg_pwr, power->ppr_target_powers);

	power->display_core = ti->data->curpower_display_core;

	/* fill the est_Pout, max target power, and rate index corresponding to the max
	 * target power fields
	 */
	if ((ret = wlc_phy_get_est_pout(ppi, power->est_Pout,
			power->est_Pout_act, &power->est_Pout_cck)) != BCME_OK) {
			PHY_ERROR(("wl%d: %s: PHY func fail: %d\n", pi->sh->unit,
				__FUNCTION__, ret));
			goto fail;
	}

	if (fns->setmax != NULL) {
		(fns->setmax)(fns->ctx, power);
	} else {
		PHY_INFORM(("%s:setmax: No phy specific function\n", __FUNCTION__));
	}

fail:
	return ret;
}

void
wlc_phy_get_ppr_board_limits(wlc_phy_t *ppi, ppr_t *ppr_board_limits)
{
	ppr_t *txpwr_srom;
	phy_info_t *pi = (phy_info_t *)ppi;
	uint8 min_pwr;

	if ((txpwr_srom = ppr_create(pi->sh->osh,
		PPR_CHSPEC_BW(pi->radio_chanspec))) != NULL) {
		wlc_phy_txpower_sromlimit(ppi, pi->radio_chanspec,
			&min_pwr, txpwr_srom, 0);
		ppr_copy_struct(txpwr_srom, ppr_board_limits);
		ppr_delete(pi->sh->osh, txpwr_srom);
	} else {
		ASSERT(0);
	}
}

bool
wlc_phy_txpower_hw_ctrl_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	if (fns->get_hwctrl != NULL) {
		return (fns->get_hwctrl)(fns->ctx);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return pi->hwpwrctrl;
	}
}

#ifdef FCC_PWR_LIMIT_2G
void
wlc_phy_fcc_pwr_limit_set(wlc_phy_t *ppi, bool enable)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_pwr_limit != NULL) {
		(fns->set_pwr_limit)(fns->ctx, enable);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

bool
wlc_phy_fcc_pwr_limit_get(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	bool fccpwrlimit2g = pi->tpci->data->fcc_pwr_limit_2g;

	return fccpwrlimit2g;
}

#endif /* FCC_PWR_LIMIT_2G */

#ifdef WL_SARLIMIT
static void
wlc_phy_sarlimit_set_int(phy_info_t *pi, int8 *sar)
{
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	uint core;

	PHY_TRACE(("%s\n", __FUNCTION__));

	FOREACH_CORE(pi, core) {
		ti->data->sarlimit[core] =
			MAX((sar[core] - pi->tx_pwr_backoff), pi->min_txpower);
	}
	if ((fns->set_sarlimit != NULL) && pi->sh->clk) {
		(fns->set_sarlimit)(fns->ctx);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

void
wlc_phy_sar_limit_set(wlc_phy_t *ppi, uint32 int_val)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	uint core;
	int8 sar[PHY_MAX_CORES];

	FOREACH_CORE(pi, core) {
		sar[core] = (int8)(((int_val) >> (core * 8)) & 0xff);
	}
	/* internal */
	wlc_phy_sarlimit_set_int(pi, sar);
}
#endif /* WL_SARLIMIT */

#ifdef WL_SAR_SIMPLE_CONTROL
static void
wlc_phy_sar_limit_set_percore(wlc_phy_t *ppi, uint32 uint_val)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_tpc_info_t *ti = pi->tpci;
	phy_type_tpc_fns_t *fns = ti->priv->fns;
	uint core;

	PHY_TRACE(("%s\n", __FUNCTION__));

	for (core = 0; core < PHY_CORE_MAX; core++) {
		if (((uint_val) >> (core * SAR_VAL_LENG)) & SAR_ACTIVEFLAG_MASK) {
			ti->data->sarlimit[core] =
				(int8)(((uint_val) >> (core * SAR_VAL_LENG)) & SAR_VAL_MASK);
		} else {
			ti->data->sarlimit[core] = WLC_TXPWR_MAX;
		}
	}
	if ((fns->set_sarlimit != NULL) && pi->sh->clk) {
		(fns->set_sarlimit)(fns->ctx);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

#if (MAX_RSDB_MAC_NUM > 1)
int
wlc_phy_is_rsdb_mode(wlc_phy_t *pi)
{
	uint8 pi_idx, active_pi_count = 0;
	phy_info_t *pi_temp;

	FOREACH_PI(pi, pi_idx) {
		pi_temp = phy_get_pi((phy_info_t*)pi, pi_idx);
		if (pi_temp->sh->clk)
			active_pi_count++;
	}

	return active_pi_count;
}
#endif /* MAX_RSDB_MAC_NUM */

void
wlc_phy_dynamic_sarctrl_set(wlc_phy_t *pi, bool isctrlon)
{
	phy_info_t *piinfo = (phy_info_t*)pi;
	phy_chanmgr_info_t *chanmgri = piinfo->chanmgri;
	uint32 sarctrlmap = 0;
	bool is_rsdb_mode = 0;

#if (MAX_RSDB_MAC_NUM > 1)
	/* get active pi count */
	uint8 active_pi_count = wlc_phy_is_rsdb_mode(pi);

	is_rsdb_mode = (active_pi_count == MAX_RSDB_MAC_NUM) ? TRUE : FALSE;

	/* Check if the pi is up */
	if (!(piinfo->sh->clk))
		return;
#endif /* MAX_RSDB_MAC_NUM */

	/* In RSDB Chips apply different SAR LIMIT's when */
	/* 1. SAR_ENABLE ON and NON-RSDB detected */
	/* 2. SAR_ENABLE ON and RSDB detected */
	/* 3. SAR_ENABLE OFF but RSDB detected */

	if (isctrlon || is_rsdb_mode) {
#if (MAX_RSDB_MAC_NUM > 1)
		if (is_rsdb_mode) {
			/* RSDB Detected */
			if (CHSPEC_IS2G(phy_chanmgr_get_home_chanspec(chanmgri))) {
				if (isctrlon) {
					/* SAR ENABLE ON */
					if (piinfo->tpci->data->fcc_pwr_limit_2g) {
						sarctrlmap =
						piinfo->tpci->data->cfg->dynamic_sarctrl_2g_2;
					} else {
						sarctrlmap =
							piinfo->tpci->data->cfg->dynamic_sarctrl_2g;
					}
				} else {
					/* SAR_ENABLE OFF */
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_2g_1;
				}
			} else {
				if (isctrlon) {
					/* SAR ENABLE ON */
					if (piinfo->tpci->data->fcc_pwr_limit_2g) {
						sarctrlmap =
						piinfo->tpci->data->cfg->dynamic_sarctrl_5g_2;
					} else {
						sarctrlmap =
							piinfo->tpci->data->cfg->dynamic_sarctrl_5g;
					}
				} else {
					/* SAR_ENABLE OFF */
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_5g_1;
				}
			}
		} else {
			/* NON-RSDB detected */
#endif /* MAX_RSDB_MAC_NUM */
			if (CHSPEC_IS2G(phy_chanmgr_get_home_chanspec(chanmgri))) {
				if (piinfo->tpci->data->fcc_pwr_limit_2g) {
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_2g_2;
				} else {
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_2g;
				}
			} else {
				if (piinfo->tpci->data->fcc_pwr_limit_2g) {
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_5g_2;
				} else {
					sarctrlmap = piinfo->tpci->data->cfg->dynamic_sarctrl_5g;
				}
			}
#if (MAX_RSDB_MAC_NUM > 1)
		}
#endif /* MAX_RSDB_MAC_NUM */
	} else {
		sarctrlmap = 0;
	}

	wlc_phy_sar_limit_set_percore(pi, sarctrlmap);
}
#endif /* WL_SAR_SIMPLE_CONTROL */

void
BCMRAMFN(wlc_phy_avvmid_txcal)(wlc_phy_t *ppi, wlc_phy_avvmid_txcal_t *val, bool set)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_avvmid != NULL) {
		(fns->set_avvmid)(fns->ctx, val, set);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

void
BCMATTACHFN(wlc_phy_txpwr_srom11_read_ppr)(phy_info_t *pi)
{

	/* Read and interpret the power-offset parameters from the SROM for each band/subband */
	if (pi->sh->sromrev >= 11) {
		uint16 _tmp;
		uint8 nibble, nibble01;

		PHY_INFORM(("Get SROM 11 Power Offset per rate\n"));
		/* --------------2G------------------- */
		/* 2G CCK */
		pi->ppr->u.sr11.cck.bw20 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
		pi->ppr->u.sr11.cck.bw20in40 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);

		pi->ppr->u.sr11.offset_2g			=
		                (uint16)PHY_GETINTVAR(pi, rstr_ofdmlrbw202gpo);
		/* 2G OFDM_20 */
		_tmp 		= (uint16)PHY_GETINTVAR(pi, rstr_dot11agofdmhrbw202gpo);
		nibble 		= pi->ppr->u.sr11.offset_2g & 0xf;
		nibble01 	= (nibble<<4)|nibble;
		nibble 		= (pi->ppr->u.sr11.offset_2g>>4)& 0xf;
		pi->ppr->u.sr11.ofdm_2g.bw20 		=
		                (((nibble<<8)|(nibble<<12))|(nibble01))&0xffff;
		pi->ppr->u.sr11.ofdm_2g.bw20 		|=
		                (_tmp << 16);
		/* 2G MCS_20 */
		pi->ppr->u.sr11.mcs_2g.bw20 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
		/* 2G MCS_40 */
		pi->ppr->u.sr11.mcs_2g.bw40 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

		pi->ppr->u.sr11.offset_20in40_l 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in40lrpo);
		pi->ppr->u.sr11.offset_20in40_h 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in40hrpo);

		pi->ppr->u.sr11.offset_dup_h 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_dot11agduphrpo);
		pi->ppr->u.sr11.offset_dup_l 		=
		                (uint16)PHY_GETINTVAR(pi, rstr_dot11agduplrpo);

#ifdef BAND5G
		/* ---------------5G--------------- */
		/* 5G 11agnac_20IN20 */
		pi->ppr->u.sr11.ofdm_5g.bw20[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
		pi->ppr->u.sr11.ofdm_5g.bw20[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
		pi->ppr->u.sr11.ofdm_5g.bw20[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);

		pi->ppr->u.sr11.offset_5g[0]			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5glpo);
		pi->ppr->u.sr11.offset_5g[1] 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5gmpo);
		pi->ppr->u.sr11.offset_5g[2] 			=
		                (uint16)PHY_GETINTVAR(pi, rstr_mcslr5ghpo);

		/* 5G 11nac 40IN40 */
		pi->ppr->u.sr11.mcs_5g.bw40[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
		pi->ppr->u.sr11.mcs_5g.bw40[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
		pi->ppr->u.sr11.mcs_5g.bw40[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);

		/* 5G 11nac 80IN80 */
		pi->ppr->u.sr11.mcs_5g.bw80[0] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805glpo);
		pi->ppr->u.sr11.mcs_5g.bw80[1] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805gmpo);
		pi->ppr->u.sr11.mcs_5g.bw80[2] 		=
		                (uint32)PHY_GETINTVAR(pi, rstr_mcsbw805ghpo);

		/* 5G 11agnac_20Ul/20LU */
		pi->ppr->u.sr11.offset_20in80_l[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5glpo);
		pi->ppr->u.sr11.offset_20in80_h[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5glpo);
		pi->ppr->u.sr11.offset_20in80_l[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5gmpo);
		pi->ppr->u.sr11.offset_20in80_h[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5gmpo);
		pi->ppr->u.sr11.offset_20in80_l[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160lr5ghpo);
		pi->ppr->u.sr11.offset_20in80_h[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb20in80and160hr5ghpo);

		/* 5G 11nac_40IN80 */
		pi->ppr->u.sr11.offset_40in80_l[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5glpo);
		pi->ppr->u.sr11.offset_40in80_h[0] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5glpo);
		pi->ppr->u.sr11.offset_40in80_l[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5gmpo);
		pi->ppr->u.sr11.offset_40in80_h[1] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5gmpo);
		pi->ppr->u.sr11.offset_40in80_l[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80lr5ghpo);
		pi->ppr->u.sr11.offset_40in80_h[2] 	=
		                (uint16)PHY_GETINTVAR(pi, rstr_sb40and80hr5ghpo);

#endif /* BAND5G */

#ifdef NO_PROPRIETARY_VHT_RATES
#else
#ifdef WL11AC
	    PHY_INFORM(("Get SROM <= 11 1024 QAM Power Offset per rate\n"));
	    pi->ppr->u.sr11.pp1024qam2g = (uint16)PHY_GETINTVAR(pi, rstr_mcs1024qam2gpo);

	    pi->ppr->u.sr11.ppmcsexp[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcs8poexp);
	    pi->ppr->u.sr11.ppmcsexp[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcs9poexp);
	    pi->ppr->u.sr11.ppmcsexp[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcs10poexp);
	    pi->ppr->u.sr11.ppmcsexp[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcs11poexp);
#ifdef BAND5G
	    /* 1024 qam fields for SROM <= 12 */
	    pi->ppr->u.sr11.pp1024qam5g[0] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5glpo);
	    pi->ppr->u.sr11.pp1024qam5g[1] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gmpo);
	    pi->ppr->u.sr11.pp1024qam5g[2] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5ghpo);
	    pi->ppr->u.sr11.pp1024qam5g[3] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gx1po);
	    pi->ppr->u.sr11.pp1024qam5g[4] = (uint32)PHY_GETINTVAR(pi, rstr_mcs1024qam5gx2po);

#endif /* BAND5G */
#endif /* WL11AC */
#endif /* NO_PROPRIETARY_VHT_RATES */
		if (0) {
			/* printf srom value for verification */
			PHY_ERROR(("		cckbw202gpo=%x\n", pi->ppr->u.sr11.cck.bw20));
			PHY_ERROR(("		cckbw20ul2gpo=%x\n", pi->ppr->u.sr11.cck.bw20in40));
			PHY_ERROR(("		ofdmlrbw202gpo=%x\n", pi->ppr->u.sr11.offset_2g));
			PHY_ERROR(("		dot11agofdmhrbw202gpo=%x\n", _tmp));
			PHY_ERROR(("		mcsbw202gpo=%x\n", pi->ppr->u.sr11.mcs_2g.bw20));
			PHY_ERROR(("		mcsbw402gpo=%x\n", pi->ppr->u.sr11.mcs_2g.bw40));
			PHY_ERROR(("		sb20in40lrpo=%x\n",
				pi->ppr->u.sr11.offset_20in40_l));
			PHY_ERROR(("		sb20in40hrpo=%x\n",
				pi->ppr->u.sr11.offset_20in40_h));
			PHY_ERROR(("		dot11agduphrpo=%x\n",
				pi->ppr->u.sr11.offset_dup_h));
			PHY_ERROR(("		dot11agduplrpo=%x\n",
				pi->ppr->u.sr11.offset_dup_l));
			PHY_ERROR(("		mcsbw205glpo=%x\n",
				pi->ppr->u.sr11.ofdm_5g.bw20[0]));
			PHY_ERROR(("		mcsbw205gmpo=%x\n",
				pi->ppr->u.sr11.ofdm_5g.bw20[1]));
			PHY_ERROR(("		mcsbw205ghpo=%x\n",
				pi->ppr->u.sr11.ofdm_5g.bw20[2]));
			PHY_ERROR(("		mcslr5glpo=%x\n", pi->ppr->u.sr11.offset_5g[0]));
			PHY_ERROR(("		mcslr5gmpo=%x\n", pi->ppr->u.sr11.offset_5g[1]));
			PHY_ERROR(("		mcslr5ghpo=%x\n", pi->ppr->u.sr11.offset_5g[2]));
			PHY_ERROR(("		mcsbw405glpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw40[0]));
			PHY_ERROR(("		mcsbw405gmpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw40[1]));
			PHY_ERROR(("		mcsbw405ghpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw40[2]));
			PHY_ERROR(("		mcsbw805glpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw80[0]));
			PHY_ERROR(("		mcsbw805gmpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw80[1]));
			PHY_ERROR(("		mcsbw805ghpo=%x\n",
				pi->ppr->u.sr11.mcs_5g.bw80[2]));
			PHY_ERROR(("		sb20in80and160lr5glpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_l[0]));
			PHY_ERROR(("		sb20in80and160hr5glpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_h[0]));
			PHY_ERROR(("		sb20in80and160lr5gmpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_l[1]));
			PHY_ERROR(("		sb20in80and160hr5gmpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_h[1]));
			PHY_ERROR(("		sb20in80and160lr5ghpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_l[2]));
			PHY_ERROR(("		sb20in80and160hr5ghpo=%x\n",
			                    pi->ppr->u.sr11.offset_20in80_h[2]));
			PHY_ERROR(("		sb40and80lr5glpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_l[0]));
			PHY_ERROR(("		sb40and80hr5glpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_h[0]));
			PHY_ERROR(("		sb40and80lr5gmpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_l[1]));
			PHY_ERROR(("		sb40and80hr5gmpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_h[1]));
			PHY_ERROR(("		sb40and80lr5ghpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_l[2]));
			PHY_ERROR(("		sb40and80hr5ghpo=%x\n",
			                    pi->ppr->u.sr11.offset_40in80_h[2]));
		}
	}
}

void
BCMATTACHFN(wlc_phy_txpwr_srom12_read_ppr)(phy_info_t *pi)
{
	if (!(SROMREV(pi->sh->sromrev) < 12)) {
	    /* Read and interpret the power-offset parameters from the SROM for each band/subband */
	    uint8 nibble, nibble01;
	    ASSERT(pi->sh->sromrev >= 12);

	    PHY_INFORM(("Get SROM 12 Power Offset per rate\n"));
	    /* --------------2G------------------- */
	    /* 2G CCK */
	    pi->ppr->u.sr11.cck.bw20 = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_cckbw202gpo);
	    pi->ppr->u.sr11.cck.bw20in40 = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_cckbw20ul2gpo);

	    pi->ppr->u.sr11.offset_2g = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_ofdmlrbw202gpo);
	    /* 2G OFDM_20 */
	    nibble = pi->ppr->u.sr11.offset_2g & 0xf;
	    nibble01 = (nibble<<4)|nibble;
	    nibble = (pi->ppr->u.sr11.offset_2g>>4)& 0xf;
	    pi->ppr->u.sr11.ofdm_2g.bw20 = (((nibble<<8)|(nibble<<12))|(nibble01))&0xffff;
	    pi->ppr->u.sr11.ofdm_2g.bw20 |=
	     (((uint16)PHY_GETINTVAR_SLICE(pi, rstr_dot11agofdmhrbw202gpo)) << 16);
	    /* 2G MCS_20 */
	    pi->ppr->u.sr11.mcs_2g.bw20 = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw202gpo);
	    /* 2G MCS_40 */
	    pi->ppr->u.sr11.mcs_2g.bw40 = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw402gpo);

	    pi->ppr->u.sr11.offset_20in40_l = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in40lrpo);
	    pi->ppr->u.sr11.offset_20in40_h = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in40hrpo);

	    pi->ppr->u.sr11.offset_dup_h = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_dot11agduphrpo);
	    pi->ppr->u.sr11.offset_dup_l = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_dot11agduplrpo);

#ifdef BAND5G
	    /* ---------------5G--------------- */
	    /* 5G 11agnac_20IN20 */
	    pi->ppr->u.sr11.ofdm_5g.bw20[0] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw205glpo);
	    pi->ppr->u.sr11.ofdm_5g.bw20[1] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw205gmpo);
	    pi->ppr->u.sr11.ofdm_5g.bw20[2] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw205ghpo);
	    pi->ppr->u.sr11.ofdm_5g.bw20[3] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw205gx1po);
	    pi->ppr->u.sr11.ofdm_5g.bw20[4] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw205gx2po);

	    pi->ppr->u.sr11.offset_5g[0]	= (uint16)PHY_GETINTVAR_SLICE(pi, rstr_mcslr5glpo);
	    pi->ppr->u.sr11.offset_5g[1] = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_mcslr5gmpo);
	    pi->ppr->u.sr11.offset_5g[2] = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_mcslr5ghpo);
	    pi->ppr->u.sr11.offset_5g[3] = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_mcslr5gx1po);
	    pi->ppr->u.sr11.offset_5g[4] = (uint16)PHY_GETINTVAR_SLICE(pi, rstr_mcslr5gx2po);

	    /* 5G 11nac 40IN40 */
	    pi->ppr->u.sr11.mcs_5g.bw40[0] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw405glpo);
	    pi->ppr->u.sr11.mcs_5g.bw40[1] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw405gmpo);

	    pi->ppr->u.sr11.mcs_5g.bw40[2] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw405ghpo);
	    pi->ppr->u.sr11.mcs_5g.bw40[3] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw405gx1po);
	    pi->ppr->u.sr11.mcs_5g.bw40[4] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw405gx2po);

	    /* 5G 11nac 80IN80 */
	    pi->ppr->u.sr11.mcs_5g.bw80[0] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw805glpo);
	    pi->ppr->u.sr11.mcs_5g.bw80[1] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw805gmpo);
	    pi->ppr->u.sr11.mcs_5g.bw80[2] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw805ghpo);
	    pi->ppr->u.sr11.mcs_5g.bw80[3] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw805gx1po);
	    pi->ppr->u.sr11.mcs_5g.bw80[4] = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_mcsbw805gx2po);

	    pi->ppr->u.sr11.offset_20in80_l[0] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160lr5glpo);
	    pi->ppr->u.sr11.offset_20in80_h[0] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160hr5glpo);
	    pi->ppr->u.sr11.offset_20in80_l[1] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160lr5gmpo);
	    pi->ppr->u.sr11.offset_20in80_h[1] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160hr5gmpo);
	    pi->ppr->u.sr11.offset_20in80_l[2] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160lr5ghpo);
	    pi->ppr->u.sr11.offset_20in80_h[2] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160hr5ghpo);
	    pi->ppr->u.sr11.offset_20in80_l[3] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160lr5gx1po);
	    pi->ppr->u.sr11.offset_20in80_h[3] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160hr5gx1po);
	    pi->ppr->u.sr11.offset_20in80_l[4] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160lr5gx2po);
	    pi->ppr->u.sr11.offset_20in80_h[4] =
	     (uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb20in80and160hr5gx2po);

		pi->ppr->u.sr11.offset_40in80_l[0] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80lr5glpo);
		pi->ppr->u.sr11.offset_40in80_h[0] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80hr5glpo);
		pi->ppr->u.sr11.offset_40in80_l[1] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80lr5gmpo);
		pi->ppr->u.sr11.offset_40in80_h[1] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80hr5gmpo);
		pi->ppr->u.sr11.offset_40in80_l[2] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80lr5ghpo);
		pi->ppr->u.sr11.offset_40in80_h[2] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80hr5ghpo);
		pi->ppr->u.sr11.offset_40in80_l[3] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80lr5gx1po);
		pi->ppr->u.sr11.offset_40in80_h[3] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80hr5gx1po);
		pi->ppr->u.sr11.offset_40in80_l[4] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80lr5gx2po);
		pi->ppr->u.sr11.offset_40in80_h[4] =
			(uint16)PHY_GETINTVAR_SLICE(pi, rstr_sb40and80hr5gx2po);
#endif /* BAND5G */
	}
}

#ifdef NO_PROPRIETARY_VHT_RATES
#else
#ifdef WL11AC
/* for MCS10/11 cases, 2 rates only */
void
wlc_phy_txpwr_srom11_ext_1024qam_convert_mcs_5g(uint32 po, chanspec_t chanspec,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {

	if (!(sizeof(*vht) > 10)) {
		PHY_ERROR(("%s: should not call me this file without VHT MCS10/11 supported!\n",
				__FUNCTION__));
		return;
	}

	if (CHSPEC_IS20(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - ((po & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 4) & 0xf) << 1);
	} else if (CHSPEC_IS40(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 8) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 12) & 0xf) << 1);
	} else if (CHSPEC_IS80(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 16) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 20) & 0xf) << 1);
	} else { /* when we are ready to BU 80p80 chanspec, settings have to be updated */
		vht->pwr[10] = tmp_max_pwr - (((po >> 24) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 28) & 0xf) << 1);
	}
}

void
wlc_phy_txpwr_srom11_ext_1024qam_convert_mcs_2g(uint16 po, chanspec_t chanspec,
         uint8 tmp_max_pwr, ppr_vht_mcs_rateset_t* vht) {
	if (!(sizeof(*vht) > 10)) {
		PHY_ERROR(("%s: should not call me this file without VHT MCS10/11 supported!\n",
			__FUNCTION__));
		return;
	}

	if (CHSPEC_IS20(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - ((po & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 4) & 0xf) << 1);
	} else if (CHSPEC_IS40(chanspec)) {
		vht->pwr[10] = tmp_max_pwr - (((po >> 8) & 0xf) << 1);
		vht->pwr[11] = tmp_max_pwr - (((po >> 12) & 0xf) << 1);
	}

}
#endif /* WL11AC */
#endif /* NO_PROPRIETARY_VHT_RATES */

void
wlc_phy_txpwr_percent_set(wlc_phy_t *ppi, uint8 txpwr_percent)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->tpci->data->txpwr_percent = txpwr_percent;
}

void
wlc_phy_txpwr_degrade(wlc_phy_t *ppi, uint8 txpwr_degrade)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->tpci->data->txpwr_degrade = txpwr_degrade;
}

int
wlc_phy_txpower_core_offset_set(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	if (fns->txcorepwroffsetset) {
		err = (fns->txcorepwroffsetset)(fns->ctx, offsets);
	}
	return err;
}

int
wlc_phy_txpower_core_offset_get(wlc_phy_t *ppi, struct phy_txcore_pwr_offsets *offsets)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	int err = BCME_UNSUPPORTED;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	if (fns->txcorepwroffsetget) {
		err = (fns->txcorepwroffsetget)(fns->ctx, offsets);
	}
	return err;
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_txpower_get(wlc_phy_t *ppi, int8 *qdbm, bool *override)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	ASSERT(qdbm != NULL);

	*qdbm = pi->tpci->data->tx_user_target;

	if (pi->openlp_tx_power_on) {
	  if (pi->tpci->data->txpwrnegative)
		*qdbm = (-1 * pi->openlp_tx_power_min) | WL_TXPWR_NEG;
	  else
		*qdbm = pi->openlp_tx_power_min;
	}

	if (override != NULL)
		*override = pi->tpci->data->txpwroverride;
	return (0);
}

/* user txpower limit: in qdbm units with override flag */
int
wlc_phy_neg_txpower_set(wlc_phy_t *ppi, uint qdbm)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (pi->sh->up) {
		if (SCAN_INPROG_PHY(pi)) {
			PHY_TXPWR(("wl%d: Scan in progress, skipping txpower control\n",
				pi->sh->unit));
		} else {
			bool suspend;
			suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			pi->openlp_tx_power_min = -1*qdbm;
			pi->tpci->data->txpwrnegative = 1;
			pi->tpci->data->txpwroverride = 1;

			phy_tpc_recalc_tgt(pi->tpci);

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);

		}
	}
	return (0);
}

#if defined(WLTEST)
int
phy_tpc_set_pavars(phy_tpc_info_t *tpci, void* a, void* p)
{
	phy_type_tpc_fns_t *fns = tpci->priv->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->set_pavars != NULL) {
		return (fns->set_pavars)(fns->ctx, a, p);
	} else {
		PHY_ERROR(("Unsupported PHY type!\n"));
		return BCME_UNSUPPORTED;
	}
}

int
phy_tpc_get_pavars(phy_tpc_info_t *tpci, void* a, void* p)
{
	phy_type_tpc_fns_t *fns = tpci->priv->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->get_pavars != NULL) {
		return (fns->get_pavars)(fns->ctx, a, p);
	} else {
		PHY_ERROR(("Unsupported PHY type!\n"));
		return BCME_UNSUPPORTED;
	}
}
#endif // endif

#ifdef  TXPWRBACKOFF
#if defined(WLTEST)
int
phy_tpc_get_vt_pwrbackoff(phy_tpc_info_t *tpci, int32 *ret_int_ptr)
{
	phy_type_tpc_fns_t *fns = tpci->priv->fns;
	int err = BCME_OK;
	if (fns->get_vt_pwrbackoff)
		*ret_int_ptr = (fns->get_vt_pwrbackoff)(fns->ctx);
	else
		err = BCME_UNSUPPORTED;
	return err;
}
#endif /* WLTEST */

int
phy_tpc_txbackoff_set(wlc_phy_t *ppi, int16 txpwr_offset)
{
	phy_type_tpc_fns_t *fns = ((phy_info_t *)ppi)->tpci->priv->fns;
	int retval = BCME_OK;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->txbackoff_set) {
		retval = (fns->txbackoff_set)(fns->ctx, txpwr_offset);
	}

	return retval;
}

bool
phy_tpc_txbackoff_is_enabled(wlc_phy_t *ppi)
{
	phy_type_tpc_fns_t *fns = ((phy_info_t *)ppi)->tpci->priv->fns;
	bool retval = FALSE;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->tvpm_txbackoff_is_enabled) {
		retval = (fns->tvpm_txbackoff_is_enabled)(fns->ctx);
	}

	return retval;
}

bool
phy_tpc_txbackoff_enable(wlc_phy_t *ppi, bool enable)
{
	phy_type_tpc_fns_t *fns = ((phy_info_t *)ppi)->tpci->priv->fns;
	bool retval = FALSE;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->tvpm_txbackoff_enable) {
		retval = (fns->tvpm_txbackoff_enable)(fns->ctx, enable);
	}

	return retval;
}

int
phy_tpc_txbackoff_dump(wlc_phy_t *ppi, phy_txpwrbackoff_info_t *pdata)
{
	phy_type_tpc_fns_t *fns = ((phy_info_t *)ppi)->tpci->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->tvpm_txbackoff_dump) {
		return (fns->tvpm_txbackoff_dump)(fns->ctx, pdata);
	} else {
		return BCME_ERROR;
	}
}

int16
phy_tpc_get_vtherm_pwrbackoff(wlc_phy_t *ppi)
{
	phy_type_tpc_fns_t *fns = ((phy_info_t *)ppi)->tpci->priv->fns;
	int16 retval = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_vt_pwrbackoff)
		retval = (fns->get_vt_pwrbackoff)(fns->ctx);

	return retval;
}
#endif /* TXPWRBACKOFF */

void
BCMRAMFN(wlc_phy_set_country)(wlc_phy_t *ppi, const char *val)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	pi->tpci->data->ccode_ptr = val;
}

int8
BCMRAMFN(wlc_phy_maxtxpwr_lowlimit)(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;
	if (fns->get_maxtxpwr_lowlimit != NULL)
		return (fns->get_maxtxpwr_lowlimit)(fns->ctx);
	else
		return 0;
}

void
phy_tpc_ipa_upd(phy_tpc_info_t *tpci)
{
	/* this should be expanded to work with all new PHY capable of iPA */
	phy_type_tpc_fns_t *fns = tpci->priv->fns;
	phy_info_t *pi = tpci->priv->pi;
	PHY_TRACE(("%s\n", __FUNCTION__));
	if (fns->ipa_upd != NULL) {
		(fns->ipa_upd)(fns->ctx);
	} else {
		pi->ipa2g_on = FALSE;
		pi->ipa5g_on = FALSE;
	}
	PHY_INFORM(("wlc_phy_txpower_ipa_upd: ipa 2g %d, 5g %d\n", pi->ipa2g_on, pi->ipa5g_on));
}

#ifdef RADIO_HEALTH_CHECK
phy_crash_reason_t phy_radio_health_check_baseindex(phy_info_t *pi)
{
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;
	if (fns->baseindex)
		return (fns->baseindex)(fns->ctx);
	else
		return PHY_RC_NONE;
}
#endif /* RADIO_HEALTH_CHECK */

void
ppr_dsss_printf(ppr_t *p)
{
	int chain, bitN;
	ppr_dsss_rateset_t dsss_boardlimits;

	for (chain = WL_TX_CHAINS_1; chain <= WL_TX_CHAINS_3; chain++) {
		ppr_get_dsss(p, WL_TX_BW_20, chain, &dsss_boardlimits);
		PHY_ERROR(("--------DSSS-BW_20-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_DSSS; bitN++)
			PHY_ERROR(("max-pwr = %d\n", dsss_boardlimits.pwr[bitN]));

		ppr_get_dsss(p, WL_TX_BW_20IN40, chain, &dsss_boardlimits);
		PHY_ERROR(("--------DSSS-BW_20IN40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_DSSS; bitN++)
			PHY_ERROR(("max-pwr = %d\n", dsss_boardlimits.pwr[bitN]));

	}
}

void
ppr_mcs_printf(ppr_t* tx_srom_max_pwr)
{

	int bitN, bwtype;
	ppr_vht_mcs_rateset_t mcs_boardlimits;
#if defined(BCMDBG) || defined(PHYDBG)
	char* bw[6] = { "20IN20", "40IN40", "80IN80", "20IN40", "20IN80", "40IN80" };
#endif /* BCMDBG || PHYDBG */
	for (bwtype = 0; bwtype < 6; bwtype++) {
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x1-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S1x2_CDD */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x2-CDD-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S1x3_CDD */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S1x3-CDD-----\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x2_STBC */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x2-STBC------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x3_STBC */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_STBC, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x3-STBC------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x2_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x2-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S2x3_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 2, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S2x3-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
		/* for ht_20IN20_S3x3_SDM */
		ppr_get_vht_mcs(tx_srom_max_pwr, bwtype, 3, WL_TX_MODE_NONE, WL_TX_CHAINS_3,
			&mcs_boardlimits);
		PHY_INFORM(("--------MCS-%s-S3x3-SDM------\n", bw[bwtype]));
		for (bitN = 0; bitN < WL_RATESET_SZ_VHT_MCS; bitN++)
			PHY_INFORM(("max-pwr = %d\n", mcs_boardlimits.pwr[bitN]));
	}
}

void
ppr_ofdm_printf(ppr_t *p)
{

	int chain, bitN;
	ppr_ofdm_rateset_t ofdm_boardlimits;
	wl_tx_mode_t mode = WL_TX_MODE_NONE;

	for (chain = WL_TX_CHAINS_1; chain <= WL_TX_CHAINS_3; chain++) {
		ppr_get_ofdm(p, WL_TX_BW_20, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_20IN40, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20IN40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_20IN80, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_20IN80-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));

		ppr_get_ofdm(p, WL_TX_BW_40, mode, chain, &ofdm_boardlimits);
		PHY_ERROR(("--------OFDM-BW_DUP40-S1x%d-----\n", chain));
		for (bitN = 0; bitN < WL_RATESET_SZ_OFDM; bitN++)
			PHY_ERROR(("max-pwr = %d\n", ofdm_boardlimits.pwr[bitN]));
		mode = WL_TX_MODE_CDD;
	}
}

int wlc_phy_get_est_pout(wlc_phy_t *ppi, uint8* est_Pout,
	uint8* est_Pout_adj, uint8* est_Pout_cck)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_tpc_info_t *tpci = pi->tpci;
	phy_type_tpc_fns_t *fns = tpci->priv->fns;
	phy_type_tpc_ctx_t *ctx = fns->ctx;

	int status = BCME_OK;

	ASSERT(fns->get_est_pout);

	*est_Pout_cck = 0;

	if (!pi->sh->up) {
		return BCME_NOTUP;
	}

	status = fns->get_est_pout(ctx, est_Pout, est_Pout_adj, est_Pout_cck);
	return status;
}

int32
phy_tpc_get_min_power_limit(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return (int32)pi->min_txpower;
}

uint8
phy_tpc_get_target_min(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	uint8 core;
	uint8 tx_pwr_min = WLC_TXPWR_MAX;

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	BCM_REFERENCE(phyrxchain);
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		tx_pwr_min = MIN(tx_pwr_min, pi->tx_power_min_per_core[core]);
	}

	return tx_pwr_min;
}

uint8
phy_tpc_get_target_max(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	uint8 core;
	uint8 tx_pwr_max = 0;

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	BCM_REFERENCE(phyrxchain);
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		tx_pwr_max = MAX(tx_pwr_max, pi->tx_power_max_per_core[core]);
	}

	return tx_pwr_max;
}

uint8
phy_tpc_get_power_backoff(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	return pi->tx_pwr_backoff;

}

bool
phy_tpc_ipa_ison(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	if (fns->ipa_ison) {
		return (fns->ipa_ison)(fns->ctx);
	} else {
		return FALSE;
	}
}

#ifdef PPR_API
uint8
phy_tpc_get_band_from_channel(phy_tpc_info_t *tpci, uint channel)
{
	/* NOTE: At present this function has only been validated for
	 * LCN and LCN40 phys. It may apply to others, but
	 * that is left as an exercise to the reader.
	 */
	uint8 band = 0;

#ifdef BAND5G
	if ((channel >= FIRST_LOW_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_LOW_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GL;
	} else if ((channel >= FIRST_MID_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_MID_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GM;
	} else if ((channel >= FIRST_HIGH_5G_CHAN_SSLPNPHY) &&
		(channel <= LAST_HIGH_5G_CHAN_SSLPNPHY)) {
		band = WL_CHAN_FREQ_RANGE_5GH;
	} else
#endif /* BAND5G */
	if (channel <= CH_MAX_2G_CHANNEL) {
		band = WL_CHAN_FREQ_RANGE_2G;
	} else {
		PHY_ERROR(("%s: invalid channel %d\n", __FUNCTION__, channel));
		ASSERT(0);
	}

	return band;
}
#endif /* PPR_API */

#ifdef BAND5G
static int8
wlc_phy_convert_srom_txpwr40Moffset(uint8 offset)
{
	if (offset == 0xf)
		return 0;
	else if (offset > 0x7) {
		PHY_ERROR(("ILLEGAL 40MHZ PWRCTRL OFFSET VALUE, APPLYING 0 OFFSET \n"));
		return 0;
	} else {
		if (offset < 4)
			return offset;
		else
			return (-8+offset);
	}
}
#endif /* BAND5G */

bool
BCMATTACHFN(wlc_phy_txpwr_srom9_read)(phy_info_t *pi)
{
	srom_pwrdet_t	*pwrdet  = pi->pwrdet;
#ifdef BAND5G
	uint32 offset_40MHz[PHY_MAX_CORES] = {0};
#endif /* BAND5G */
	int b;

	/* read in antenna-related config */
	pi->aa2g = (uint8) PHY_GETINTVAR(pi, rstr_aa2g);
#ifdef BAND5G
	pi->aa5g = (uint8) PHY_GETINTVAR(pi, rstr_aa5g);
#endif /* BAND5G */

	/* read in FEM stuff */
	pi->fem2g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos2g);
	pi->fem2g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain2g);
	pi->fem2g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange2g);
	pi->fem2g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso2g);
	pi->fem2g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl2g);

#ifdef BAND5G
	pi->fem5g->tssipos = (uint8)PHY_GETINTVAR(pi, rstr_tssipos5g);
	pi->fem5g->extpagain = (uint8)PHY_GETINTVAR(pi, rstr_extpagain5g);
	pi->fem5g->pdetrange = (uint8)PHY_GETINTVAR(pi, rstr_pdetrange5g);
	pi->fem5g->triso = (uint8)PHY_GETINTVAR(pi, rstr_triso5g);
	pi->fem5g->antswctrllut = (uint8)PHY_GETINTVAR(pi, rstr_antswctl5g);
#endif /* BAND5G */

#ifdef BAND5G
	offset_40MHz[PHY_CORE_0] = PHY_GETINTVAR(pi, rstr_pa2gw0a3);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
		offset_40MHz[PHY_CORE_1] = PHY_GETINTVAR(pi, rstr_pa2gw1a3);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
		offset_40MHz[PHY_CORE_2] = PHY_GETINTVAR(pi, rstr_pa2gw2a3);
#endif /* BAND5G */

	/* read pwrdet params for each band/subband */
	for (b = 0; b < NUMSUBBANDS(pi); b++) {
		switch (b) {
		case WL_CHAN_FREQ_RANGE_2G: /* 0 */
			/* 2G band */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa2gw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = 0;

			if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp2ga1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] = 0;
			}
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp2ga2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa2gw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] = 0;
			}
			break;
#ifdef BAND5G
		case WL_CHAN_FREQ_RANGE_5G_BAND0: /* 1 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gla0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5glw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_convert_srom_txpwr40Moffset(
				(offset_40MHz[0] & PWROFFSET40_MASK_0)
					>> PWROFFSET40_SHIFT_0);
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[1] & PWROFFSET40_MASK_0)
						>> PWROFFSET40_SHIFT_0);
			}
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[2] & PWROFFSET40_MASK_0)
						>> PWROFFSET40_SHIFT_0);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND1: /* 2 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a0);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_convert_srom_txpwr40Moffset(
				(offset_40MHz[0] & PWROFFSET40_MASK_1) >> PWROFFSET40_SHIFT_1);
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5ga1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a1);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[1] & PWROFFSET40_MASK_1)
						>> PWROFFSET40_SHIFT_1);
			}

			if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5ga2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5gw2a2);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[2] & PWROFFSET40_MASK_1)
						>> PWROFFSET40_SHIFT_1);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND2: /* 3 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5gha0);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a0);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a0);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a0);
			pwrdet->pwr_offset40[0][b] = wlc_phy_convert_srom_txpwr40Moffset(
				(offset_40MHz[PHY_CORE_0] & PWROFFSET40_MASK_2) >>
					PWROFFSET40_SHIFT_2);
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha1);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a1);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a1);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a1);
				pwrdet->pwr_offset40[1][b] = wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[PHY_CORE_1] & PWROFFSET40_MASK_2) >>
						PWROFFSET40_SHIFT_2);
			}

			if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha2);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a2);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a2);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a2);
				pwrdet->pwr_offset40[2][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[PHY_CORE_2] & PWROFFSET40_MASK_2)
						>> PWROFFSET40_SHIFT_2);
			}
			break;

		case WL_CHAN_FREQ_RANGE_5G_BAND3: /* 4 */
			pwrdet->max_pwr[PHY_CORE_0][b] = (int8)PHY_GETINTVAR(pi, rstr_maxp5ga3);
			pwrdet->pwrdet_a1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw0a3);
			pwrdet->pwrdet_b0[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw1a3);
			pwrdet->pwrdet_b1[PHY_CORE_0][b] = (int16)PHY_GETINTVAR(pi, rstr_pa5gw2a3);
			pwrdet->pwr_offset40[PHY_CORE_0][b] = wlc_phy_convert_srom_txpwr40Moffset(
				(offset_40MHz[0] & PWROFFSET40_MASK_3) >> PWROFFSET40_SHIFT_3);
			if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
				pwrdet->max_pwr[PHY_CORE_1][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gla3);
				pwrdet->pwrdet_a1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw0a3);
				pwrdet->pwrdet_b0[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw1a3);
				pwrdet->pwrdet_b1[PHY_CORE_1][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5glw2a3);
				pwrdet->pwr_offset40[PHY_CORE_1][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[1] & PWROFFSET40_MASK_3)
						>> PWROFFSET40_SHIFT_3);
			}

			if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
				pwrdet->max_pwr[PHY_CORE_2][b] =
					(int8)PHY_GETINTVAR(pi, rstr_maxp5gha3);
				pwrdet->pwrdet_a1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw0a3);
				pwrdet->pwrdet_b0[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw1a3);
				pwrdet->pwrdet_b1[PHY_CORE_2][b] =
					(int16)PHY_GETINTVAR(pi, rstr_pa5ghw2a3);
				pwrdet->pwr_offset40[PHY_CORE_2][b] =
					wlc_phy_convert_srom_txpwr40Moffset(
					(offset_40MHz[2] & PWROFFSET40_MASK_3)
						>> PWROFFSET40_SHIFT_3);
			}
			break;
#endif /* BAND5G */
		}
	}
	wlc_phy_read_srom9_txpwr_ppr(pi);
	return TRUE;
}

static void
BCMATTACHFN(wlc_phy_read_srom9_txpwr_ppr)(phy_info_t *pi)
{

	/* Read and interpret the power-offset parameters from the SROM for each band/subband */
	if (pi->sh->sromrev >= 9) {
		int i, j;

		PHY_INFORM(("Get SROM 9 Power Offset per rate\n"));
		/* 2G CCK */
		pi->ppr->u.sr9.cckbw202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
		pi->ppr->u.sr9.cckbw20ul2gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw20ul2gpo);
		/* 2G OFDM power offsets */
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw202gpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul2gpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul2gpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw402gpo);

#ifdef BAND5G
		/* 5G power offsets */
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205glpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5glpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205gmpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5gmpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205ghpo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5ghpo);

		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205glpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5glpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND0].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405glpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205gmpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5gmpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND1].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405gmpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5ghpo);
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND2].bw40 =
			(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
		if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
		{
			pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw205ghpo);
			pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_legofdmbw20ul5ghpo);

			pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw205ghpo);
			pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw20ul5ghpo);
			pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_5G_BAND3].bw40 =
				(uint32)PHY_GETINTVAR(pi, rstr_mcsbw405ghpo);
		}
#endif /* BAND5G */

		/* 40 Dups */
		pi->ppr->u.sr9.ofdm40duppo = (uint16)PHY_GETINTVAR(pi, rstr_legofdm40duppo);
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40 =
			pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul;
		for (i = 0; i < NUMSUBBANDS(pi); i++) {
			uint32 nibble, dup40_offset = 0;
			nibble = pi->ppr->u.sr9.ofdm40duppo & 0xf;
			for (j = 0; j < WL_NUM_RATES_OFDM; j++) {
				dup40_offset |= nibble;
				nibble <<= 4;
			}
			if (i == 0)
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40 =
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul +
				dup40_offset;
#ifdef BAND5G
			else if (i == 1)
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw40 =
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND0].bw20ul +
				dup40_offset;
			else if (i == 2)
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw40 =
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND1].bw20ul +
				dup40_offset;
			else if (i == 3)
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw40 =
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND2].bw20ul +
				dup40_offset;
			else if (i == 4)
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw40 =
				pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_5G_BAND3].bw20ul +
				dup40_offset;

#endif /* BAND5G */
		}
	}

	PHY_INFORM(("CCK: 0x%04x 0x%04x\n", pi->ppr->u.sr9.cckbw202gpo,
		pi->ppr->u.sr9.cckbw202gpo));
	PHY_INFORM(("OFDM: 0x%08x 0x%08x 0x%02x\n",
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20,
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw20ul,
		pi->ppr->u.sr9.ofdm[WL_CHAN_FREQ_RANGE_2G].bw40));
	PHY_INFORM(("MCS: 0x%08x 0x%08x 0x%08x\n",
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20,
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw20ul,
		pi->ppr->u.sr9.mcs[WL_CHAN_FREQ_RANGE_2G].bw40));
}

#ifdef BCMDBG
void
phy_tpc_dump_txpower_limits(wlc_phy_t *ppi, ppr_t* txpwr)
{
	int i;
	char fraction[4][4] = {"   ", ".25", ".5 ", ".75"};
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	printf("CCK		     ");
	ppr_get_dsss(txpwr, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++) {
		printf(" %2d%s", dsss_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[dsss_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM SISO      ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	printf("20MHz OFDM CDD	     ");
	ppr_get_ofdm(txpwr, WL_TX_BW_20, WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_limits);
	for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
		printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
			fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
	}
	printf("\n");

	if (fns->dump_txpower_limits) { /* PHY specific dumps */
		(fns->dump_txpower_limits)(fns->ctx, txpwr);
	} else { /* Common dumps */
		printf("%s", "20MHz MCS 0-7 SISO   ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "20MHz MCS 0-7 CDD    ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("20MHz MCS 0-7 STBC   ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_STBC, WL_TX_CHAINS_2,
			&mcs_limits);

		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "20MHz MCS 8-15 SDM   ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_20, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("40MHz OFDM SISO      ");
		ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_NONE, WL_TX_CHAINS_1, &ofdm_limits);
		for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
			printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("40MHz OFDM CDD       ");
		ppr_get_ofdm(txpwr, WL_TX_BW_40, WL_TX_MODE_CDD, WL_TX_CHAINS_2, &ofdm_limits);
		for (i = 0; i < WL_RATESET_SZ_OFDM; i++) {
			printf(" %2d%s", ofdm_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[ofdm_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "40MHz MCS 0-7 SISO   ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "40MHz MCS 0-7 CDD    ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_2,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "40MHz MCS 0-7 CDD    ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_1, WL_TX_MODE_CDD, WL_TX_CHAINS_3,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");

		printf("%s", "40MHz MCS8-15 SDM    ");
		ppr_get_ht_mcs(txpwr, WL_TX_BW_40, WL_TX_NSS_2, WL_TX_MODE_NONE, WL_TX_CHAINS_2,
			&mcs_limits);
		for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++) {
			printf(" %2d%s", mcs_limits.pwr[i]/ WLC_TXPWR_DB_FACTOR,
				fraction[mcs_limits.pwr[i] % WLC_TXPWR_DB_FACTOR]);
		}
		printf("\n");
	}
}
#endif /* BCMDBG */

void
phy_tpc_set_txpower_hw_ctrl(wlc_phy_t *ppi, bool hwpwrctrl)
{
	phy_info_t *pi = (phy_info_t *)ppi;
	phy_type_tpc_fns_t *fns = pi->tpci->priv->fns;

	/* validate if hardware power control is capable */
	if (!pi->hwpwrctrl_capable) {
		PHY_ERROR(("wl%d: hwpwrctrl not capable\n", pi->sh->unit));
		return;
	}

	PHY_INFORM(("wl%d: setting the hwpwrctrl to %d\n", pi->sh->unit, hwpwrctrl));
	pi->hwpwrctrl = hwpwrctrl;
	pi->nphy_txpwrctrl = hwpwrctrl;
	pi->txpwrctrl = hwpwrctrl;

	/* if power control mode is changed, propagate it */
	if (fns->set_hwctrl) {
		fns->set_hwctrl(fns->ctx, hwpwrctrl);
	}
}

#ifdef WLTXPWR_CACHE
tx_pwr_cache_entry_t* phy_tpc_get_txpwr_cache(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	return (tx_pwr_cache_entry_t*)pi->txpwr_cache;
}

#if !defined(WLTXPWR_CACHE_PHY_ONLY)
void phy_tpc_set_txpwr_cache(wlc_phy_t *ppi, tx_pwr_cache_entry_t* cacheptr)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	pi->txpwr_cache = cacheptr;
}
#endif // endif
#endif	/* WLTXPWR_CACHE */

void
phy_tpc_get_paparams_for_band(phy_info_t *pi, int32 *a1, int32 *b0, int32 *b1)
{
	/* On lcnphy, estPwrLuts0/1 table entries are in S6.3 format */
	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
	case WL_CHAN_FREQ_RANGE_2G:
			/* 2.4 GHz */
			ASSERT((pi->txpa_2g[0] != -1) && (pi->txpa_2g[1] != -1) &&
				(pi->txpa_2g[2] != -1));
			*b0 = pi->txpa_2g[0];
			*b1 = pi->txpa_2g[1];
			*a1 = pi->txpa_2g[2];
			break;
#ifdef BAND5G
	case WL_CHAN_FREQ_RANGE_5GL:
			/* 5 GHz low */
			ASSERT((pi->txpa_5g_low[0] != -1) &&
				(pi->txpa_5g_low[1] != -1) &&
				(pi->txpa_5g_low[2] != -1));
			*b0 = pi->txpa_5g_low[0];
			*b1 = pi->txpa_5g_low[1];
			*a1 = pi->txpa_5g_low[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GM:
			/* 5 GHz middle */
			ASSERT((pi->txpa_5g_mid[0] != -1) &&
				(pi->txpa_5g_mid[1] != -1) &&
				(pi->txpa_5g_mid[2] != -1));
			*b0 = pi->txpa_5g_mid[0];
			*b1 = pi->txpa_5g_mid[1];
			*a1 = pi->txpa_5g_mid[2];
			break;

		case WL_CHAN_FREQ_RANGE_5GH:
			/* 5 GHz high */
			ASSERT((pi->txpa_5g_hi[0] != -1) &&
				(pi->txpa_5g_hi[1] != -1) &&
				(pi->txpa_5g_hi[2] != -1));
			*b0 = pi->txpa_5g_hi[0];
			*b1 = pi->txpa_5g_hi[1];
			*a1 = pi->txpa_5g_hi[2];
			break;
#endif /* BAND5G */
		default:
			ASSERT(FALSE);
			break;
	}
	return;
}
