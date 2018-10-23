/*
 * lcn20PHY TxPowerCtrl module implementation
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
 * $Id: phy_lcn20_tpc.c 671526 2016-11-22 08:37:30Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_tpc.h>
#include "phy_type_tpc.h"
#include <phy_lcn20.h>
#include <phy_lcn20_tpc.h>
#include <phy_tpc_api.h>
#include <phy_chanmgr.h>
#include <phy_utils_reg.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#include <d11.h>
#include <phy_stf.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phyreg_lcn20.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn20_tpc_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_tpc_info_t *ti;
};

/* local functions */
static int phy_lcn20_tpc_init(phy_type_tpc_ctx_t *ctx);
static void phy_lcn20_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx);
static void wlc_phy_txpower_recalc_target_lcn20_big(phy_type_tpc_ctx_t *ctx, ppr_t *tx_pwr_target,
    ppr_t *srom_max_txpwr, ppr_t *reg_txpwr_limit, ppr_t *txpwr_targets);
#if (defined(LCN20CONF) && (LCN20CONF != 0))
static void wlc_phy_txpower_sromlimit_get_lcn20phy(phy_type_tpc_ctx_t *ctx, chanspec_t chanspec,
    ppr_t *max_pwr, uint8 core);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
static bool phy_lcn20_tpc_recalc_sw(phy_type_tpc_ctx_t *ctx);
static bool phy_lcn20_tpc_hw_ctrl_get(phy_type_tpc_ctx_t *ctx);
static void phy_lcn20_tpc_set_flags(phy_type_tpc_ctx_t *ctx, phy_tx_power_t *power);
static void phy_lcn20_tpc_set_max(phy_type_tpc_ctx_t *ctx, phy_tx_power_t *power);
#if defined(WLTEST)
static int phy_lcn20_tpc_set_pavars(phy_type_tpc_ctx_t *ctx, void* a, void* p);
static int phy_lcn20_tpc_get_pavars(phy_type_tpc_ctx_t *ctx, void* a, void* p);
#endif // endif
static int phy_lcn20_tpc_get_est_pout(phy_type_tpc_ctx_t *ctx,
	uint8* est_Pout, uint8* est_Pout_adj, uint8* est_Pout_cck);

/* Register/unregister lcn20PHY specific implementation to common layer. */
phy_lcn20_tpc_info_t *
BCMATTACHFN(phy_lcn20_tpc_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_tpc_info_t *ti)
{
	phy_lcn20_tpc_info_t *info;
	phy_type_tpc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_lcn20_tpc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn20_tpc_info_t));
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->ti = ti;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init = phy_lcn20_tpc_init;
	fns.recalc = phy_lcn20_tpc_recalc_tgt;
	fns.recalc_target = wlc_phy_txpower_recalc_target_lcn20_big;
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	fns.get_sromlimit = wlc_phy_txpower_sromlimit_get_lcn20phy;
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
	fns.recalc_sw = phy_lcn20_tpc_recalc_sw;
	fns.get_hwctrl = phy_lcn20_tpc_hw_ctrl_get;
	fns.setflags = phy_lcn20_tpc_set_flags;
	fns.setmax = phy_lcn20_tpc_set_max;
#if defined(WLTEST)
	fns.set_pavars = phy_lcn20_tpc_set_pavars;
	fns.get_pavars = phy_lcn20_tpc_get_pavars;
#endif // endif
	fns.get_est_pout = phy_lcn20_tpc_get_est_pout;
	fns.ctx = info;

	info->ti->data->cfg->cckpwroffset[0] = (int8)PHY_GETINTVAR_DEFAULT(pi,
		rstr_cckpwroffset0, 0);

	phy_tpc_register_impl(ti, &fns);

	return info;

fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn20_tpc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_tpc_unregister_impl)(phy_lcn20_tpc_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_tpc_info_t *ti = info->ti;

	PHY_TRACE(("%s\n", __FUNCTION__));

	phy_tpc_unregister_impl(ti);

	phy_mfree(pi, info, sizeof(phy_lcn20_tpc_info_t));
}

/* init module and h/w */
static int
WLBANDINITFN(phy_lcn20_tpc_init)(phy_type_tpc_ctx_t *ctx)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	return BCME_OK;
}

/* recalc target txpwr and apply to h/w */
static void
phy_lcn20_tpc_recalc_tgt(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	wlc_phy_txpower_recalc_target_lcn20phy(pi);
}

#define CC_CODE_LEN_BYTES (3)
#define NUM_LOG_ENTRIES (8)

/* TODO: The code could be optimized by moving the common code to phy/cmn */
/* [PHY_RE_ARCH] There are two functions: Bigger function wlc_phy_txpower_recalc_target
 * and smaller function phy_tpc_recalc_tgt which in turn call their phy specific functions
 * which are named in a haphazard manner. This needs to be cleaned up.
 */
static void
wlc_phy_txpower_recalc_target_lcn20_big(phy_type_tpc_ctx_t *ctx, ppr_t *tx_pwr_target,
    ppr_t *srom_max_txpwr, ppr_t *reg_txpwr_limit, ppr_t *txpwr_targets)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	int8 tx_pwr_max = 0;
	int8 tx_pwr_min = 255;
	uint8 mintxpwr = 0;
	uint8 core;
	chanspec_t chspec = pi->radio_chanspec;
	bool min_tx_pwr_check = FALSE;
	/* Combine user target, regulatory limit, SROM/HW/board limit and power
	 * percentage to get a tx power target for each rate.
	 */
	FOREACH_CORE(pi, core) {
		/* The user target is the starting point for determining the transmit
		 * power.  If pi->txoverride is true, then use the user target as the
		 * tx power target.
		 */
		ppr_set_cmn_val(tx_pwr_target, info->ti->data->tx_user_target);

#if defined(WLTEST) || defined(WL_EXPORT_TXPOWER)
		/* Only allow tx power override for internal or test builds. */
		if (!info->ti->data->txpwroverride)
#endif // endif
		{
			/* Get board/hw limit */
			wlc_phy_txpower_sromlimit((wlc_phy_t *)pi, chspec,
			    &mintxpwr, srom_max_txpwr, core);

			/* Common Code Start */
			/* Choose minimum of provided regulatory and board/hw limits */
			ppr_compare_min(srom_max_txpwr, reg_txpwr_limit);

			/* Subtract 4 (1.0db) for 4313(IPA) as we are doing PA trimming
			 * otherwise subtract 6 (1.5 db)
			 * to ensure we don't go over
			 * the limit given a noisy power detector.  The result
			 * is now a target, not a limit.
			 */
			ppr_minus_cmn_val(srom_max_txpwr, pi->tx_pwr_backoff);

			/* Choose least of user and now combined regulatory/hw targets */
			ppr_compare_min(tx_pwr_target, srom_max_txpwr);

			/* Board specific fix reduction */

			/* Apply power output percentage */
			if (pi->tpci->data->txpwr_percent < 100) {
				ppr_multiply_percentage(tx_pwr_target,
					pi->tpci->data->txpwr_percent);
			}
			/* Common Code End */

			/* Enforce min power and save result as power target.
			 * LCNPHY references off the minimum so this is not appropriate for it.
			 */
			ppr_disable_vht_mimo_rates(tx_pwr_target);
		}

		tx_pwr_max = ppr_get_max(tx_pwr_target);

		if (tx_pwr_max < (pi->min_txpower * WLC_TXPWR_DB_FACTOR)) {
			tx_pwr_max = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
		}
		tx_pwr_min = ppr_get_min(tx_pwr_target, WL_RATE_DISABLED);

		/* Common Code Start */
		/* Now calculate the tx_power_offset and update the hardware... */
		pi->tx_power_max_per_core[core] = tx_pwr_max;
		pi->tx_power_min_per_core[core] = tx_pwr_min;

#ifdef WLTXPWR_CACHE
		if (wlc_phy_txpwr_cache_is_cached(pi->txpwr_cache, pi->radio_chanspec) == TRUE) {
			wlc_phy_set_cached_pwr_min(pi->txpwr_cache, pi->radio_chanspec, core,
				tx_pwr_min);
			wlc_phy_set_cached_pwr_max(pi->txpwr_cache, pi->radio_chanspec, core,
				tx_pwr_max);
		}
#endif // endif
		pi->openlp_tx_power_min = tx_pwr_min;
		info->ti->data->txpwrnegative = 0;

		min_tx_pwr_check = tx_pwr_min < (LCN20PHY_TXPWR_MIN * WLC_TXPWR_DB_FACTOR);
		if (min_tx_pwr_check) {
			/* PHY_FATAL_ERROR_MESG((" %s: TxMinPwr less than Supported MinPower\n",
			 *		__FUNCTION__));
			 * PHY_FATAL_ERROR(pi, PHY_RC_TXPOWER_LIMITS);
			 */
			/* Log the channel & country info when this issue occurs.
			 * 1st element in the log arrays pi->ccode[] and pi->chanspec_array[]
			 * correspond to the latest failure case.
			  */
			int8 length, i;
			uint16 *chanspec_array = pi->tpci->data->chanspec_array;
			char *ccode = pi->tpci->data->ccode;
			pi->tpci->data->minpwrlimit_fail++;
			if (pi->tpci->data->ccode_ptr) {
				length = CC_CODE_LEN_BYTES;
				for (i = NUM_LOG_ENTRIES-2; i >= 0; i--) {
					strncpy(ccode + length*(i+1), ccode + length*i,
							CC_CODE_LEN_BYTES);
					chanspec_array[i+1] = chanspec_array[i];
				}
				strncpy(ccode, pi->tpci->data->ccode_ptr, length);
			}
			chanspec_array[0] = pi->radio_chanspec;
		}

		PHY_NONE(("wl%d: %s: min %d max %d\n", pi->sh->unit, __FUNCTION__,
		    tx_pwr_min, tx_pwr_max));

		/* determinate the txpower offset by either of the following 2 methods:
		 * txpower_offset = txpower_max - txpower_target OR
		 * txpower_offset = txpower_target - txpower_min
		 * return curpower for last core loop since we are now checking
		 * MIN_cores(MAX_rates(power)) for rate disabling
		 * Only the last core loop info is valid
		 */
		if (core == PHYCORENUM((pi)->pubpi->phy_corenum) - 1) {
			info->ti->data->curpower_display_core =
				PHYCORENUM((pi)->pubpi->phy_corenum) - 1;
			if (txpwr_targets != NULL)
				ppr_copy_struct(tx_pwr_target, txpwr_targets);
		}
		/* Common Code End */

		if (!pi->hwpwrctrl) {
			ppr_cmn_val_minus(tx_pwr_target, pi->tx_power_max_per_core[core]);
		} else {
			ppr_minus_cmn_val(tx_pwr_target, pi->tx_power_min_per_core[core]);
		}

		ppr_compare_max(pi->tx_power_offset, tx_pwr_target);
	}	/* CORE loop */

	/* Common Code Start */
#ifdef WLTXPWR_CACHE
	if (wlc_phy_txpwr_cache_is_cached(pi->txpwr_cache, pi->radio_chanspec) == TRUE) {
		wlc_phy_set_cached_pwr(pi->sh->osh, pi->txpwr_cache, pi->radio_chanspec,
			TXPWR_CACHE_POWER_OFFSETS, pi->tx_power_offset);
	}
#endif // endif
	/*
	 * PHY_ERROR(("#####The final power offset limit########\n"));
	 * ppr_mcs_printf(pi->tx_power_offset);
	 */
	/* Common Code End */
}

static bool
phy_lcn20_tpc_recalc_sw(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* No need to do anything if the hw is doing pwrctrl for us */
	if (pi->hwpwrctrl) {
		/* Do nothing if radio pwr is being overridden */
		if (info->ti->data->radiopwr_override == RADIOPWR_OVERRIDE_DEF) {
			pi->hwpwr_txcur = wlapi_bmac_read_shm(pi->sh->physhim, M_TXPWR_CUR(pi));
		}
	}

	return TRUE;
}

#if (defined(LCN20CONF) && (LCN20CONF != 0))
static void
wlc_phy_txpwr_apply_sromlcn20(phy_info_t *pi, uint8 band, ppr_t *tx_srom_max_pwr)
{
	srom_lcn20_ppr_t *sr_lcn20 = &pi->ppr->u.sr_lcn20;
	int8 max_pwr_ref = 0;

	ppr_dsss_rateset_t ppr_dsss;
	ppr_ofdm_rateset_t ppr_ofdm;
	ppr_ht_mcs_rateset_t ppr_mcs;
	BCM_REFERENCE(band);

	max_pwr_ref = pi->tx_srom_max_2g;

	/* 2G - CCK_20 */
	wlc_phy_txpwr_srom_convert_cck(sr_lcn20->cck202gpo, max_pwr_ref, &ppr_dsss);
	ppr_set_dsss(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_CHAINS_1, &ppr_dsss);

	/* 2G - OFDM_20 */
	wlc_phy_txpwr_srom_convert_ofdm(sr_lcn20->ofdmbw202gpo, max_pwr_ref,
		&ppr_ofdm);
	ppr_set_ofdm(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ppr_ofdm);

	/* 2G - MCS_20 */
	wlc_phy_txpwr_srom_convert_mcs(sr_lcn20->mcsbw202gpo, max_pwr_ref,
		&ppr_mcs);
#if defined(WLPROPRIETARY_11N_RATES)
	/* Set board limits for prop rates */
	ppr_mcs.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_87] =
		max_pwr_ref - ((sr_lcn20->propbw202gpo & 0xf) * 2);
	ppr_mcs.pwr[WL_RATE_GROUP_VHT_INDEX_MCS_88] =
		max_pwr_ref - (((sr_lcn20->propbw202gpo >> 4) & 0xf) * 2);
#endif // endif
	ppr_set_ht_mcs(tx_srom_max_pwr, WL_TX_BW_20, WL_TX_NSS_1, WL_TX_MODE_NONE,
		WL_TX_CHAINS_1, &ppr_mcs);
}

static void
wlc_phy_txpower_sromlimit_get_lcn20phy(phy_type_tpc_ctx_t *ctx, chanspec_t chanspec,
    ppr_t *max_pwr, uint8 core)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	uint8 band;
	uint8 channel = CHSPEC_CHANNEL(chanspec);
	band = phy_tpc_get_band_from_channel(info->ti, channel);
	wlc_phy_txpwr_apply_sromlcn20(pi, band, max_pwr);
	ppr_apply_max(max_pwr, pwrdet->max_pwr[core][band]);
}
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */

static bool
phy_lcn20_tpc_hw_ctrl_get(phy_type_tpc_ctx_t *ctx)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	return pi->hwpwrctrl;
}

static void
phy_lcn20_tpc_set_flags(phy_type_tpc_ctx_t *ctx, phy_tx_power_t *power)
{
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	power->rf_cores = 1;
	power->flags |= (WL_TX_POWER_F_SISO);
	if (info->ti->data->radiopwr_override == RADIOPWR_OVERRIDE_DEF)
		power->flags |= WL_TX_POWER_F_ENABLED;
	if (pi->hwpwrctrl)
		power->flags |= WL_TX_POWER_F_HW;
}

static void
phy_lcn20_tpc_set_max(phy_type_tpc_ctx_t *ctx, phy_tx_power_t *power)
{
#ifdef WL_SARLIMIT
	uint8 core;
#endif // endif
	phy_lcn20_tpc_info_t *info = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	/* Store the maximum target power among all rates */
	if (pi->hwpwrctrl && pi->sh->up) {
		/* If hw (ucode) based, read the hw based estimate in realtime */
		phy_utils_phyreg_enter(pi);
		/* Store the maximum target power among all rates */
		power->tx_power_max[0] = pi->tx_power_max_per_core[0];
		power->tx_power_max[1] = pi->tx_power_max_per_core[0];
		power->flags &= ~(WL_TX_POWER_F_HW | WL_TX_POWER_F_ENABLED);
		phy_utils_phyreg_exit(pi);
	}
#ifdef WL_SARLIMIT
	FOREACH_CORE(pi, core) {
		power->SARLIMIT[core] = WLC_TXPWR_MAX;
	}
#endif // endif
}

void
wlc_lcn20phy_clear_tx_power_offsets(phy_info_t *pi)
{
	uint32 data_buf[64];
	phytbl_info_t tab;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	/* Clear out buffer */
	bzero(data_buf, sizeof(data_buf));

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_ptr = data_buf; /* ptr to buf */

	/* we shouldn't be clearing the rate offset table */
	if (!pi_lcn20->uses_rate_offset_table) {
		/* Per rate power offset */
		tab.tbl_len = 20; /* # values   */
		tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn20phy_write_table(pi, &tab);
	}
	/* Per index power offset */
	tab.tbl_len = 64; /* # values   */
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_MAC_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);
}

static void
wlc_lcn20phy_perpkt_idle_tssi_est(phy_info_t *pi)
{
	bool suspend;
	uint16 SAVE_txpwrctrl;
	uint8 SAVE_indx;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_txpwrctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	SAVE_indx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);

	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	wlc_lcn20phy_tssi_setup(pi);

	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);

	wlc_lcn20phy_set_txpwr_clamp(pi);

	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 0);

	wlc_lcn20phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
		pi->tpci->data->cfg->cckpwroffset[0]);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

static void
wlc_lcn20phy_save_idletssi(phy_info_t *pi, uint16 idleTssi0_regvalue_2C)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			pi_lcn20->idletssi0_cache =
			idleTssi0_regvalue_2C;
			break;
		default:
			PHY_ERROR(("wl%d: %s: Bad channel/band\n",
				pi->sh->unit, __FUNCTION__));
			break;
	}
}

static void
wlc_lcn20phy_restore_idletssi(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlIdleTssi,
			idleTssi0, pi_lcn20->idletssi0_cache);
			break;
		default:
			PHY_ERROR(("wl%d: %s: Bad channel/band\n",
				pi->sh->unit, __FUNCTION__));
			break;
	}
}

void
wlc_lcn20phy_idle_tssi_est(phy_info_t *pi)
{
	bool suspend, tx_gain_override_old;
	phy_txgains_t old_gains;
	uint8 SAVE_bbmult;
	uint16 idleTssi0_2C;
#if TWO_POWER_RANGE_TXPWR_CTRL
	uint16 idleTssi1_2C;
#endif // endif
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16 SAVE_txpwrctrl;
	uint8 SAVE_indx;
	uint16 save_TempSenseCorrection;
	uint16 save_TxPwrCtrlDeltaPwrLimit;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_txpwrctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	SAVE_indx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	SAVE_bbmult = wlc_lcn20phy_get_bbmult(pi);
	save_TempSenseCorrection = PHY_REG_READ(pi, LCN20PHY, TempSenseCorrection, tempsenseCorr);
	save_TxPwrCtrlDeltaPwrLimit = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlDeltaPwrLimit);

	/* Save old tx gains if needed */
	tx_gain_override_old = wlc_lcn20phy_tx_gain_override_enabled(pi);
	wlc_lcn20phy_get_tx_gain(pi, &old_gains);

	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* set txgain override */
	wlc_lcn20phy_enable_tx_gain_override(pi);
	wlc_lcn20phy_set_tx_pwr_by_index(pi, 127);

	/* Include attenuator in the loopback path through override for normal pwr range */
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 1);

	/* This call will set txpwrctrl scheme to 0(single tssi) */
	wlc_lcn20phy_tssi_setup(pi);
	/* Restore TSSI if
	* 1. cal is not possible
	* 2. idle TSSI for the current band/subband is valid
	*/
	if (0 && phy_calmgr_no_cal_possible(pi)) {
		int range = wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec);

		if ((range == WL_CHAN_FREQ_RANGE_2G) &&
			(pi_lcn20->idletssi0_cache)) {
			wlc_lcn20phy_restore_idletssi(pi);
			goto cleanIdleTSSI;
		}
	}

	wlc_lcn20phy_set_bbmult(pi, 0x0);

	wlc_btcx_override_enable(pi);
	phy_tssical_do_dummy_tx(pi, TRUE, OFF);
	/* Disable WLAN priority */
	wlc_phy_btcx_override_disable(pi);

	/* avgTssi value is in 2C (S9.0) format */
	idleTssi0_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn20->lcn20_twopwr_txpwrctrl_en) {
		/* Bypass attenuator in the loopback path through override for low power range */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal, 0);

		wlc_btcx_override_enable(pi);
		phy_tssical_do_dummy_tx(pi, TRUE, OFF);
		/* Disable WLAN priority */
		wlc_phy_btcx_override_disable(pi);

		/* idletssi1 is calculated from path 0 after attenuator setting */
		/* since HW uses path 0 always when txpwrctrl scheme is 0 (single tssi) */
		/* avgTssi value is in 2C (S9.0) format */
		idleTssi1_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi);

		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlIdleTssi1, idleTssi1, idleTssi1_2C);

		/* Clear tssiRangeOverride and have direct control switch between path0 and path1 */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 0);

	}
#endif  /* #if TWO_POWER_RANGE_TXPWR_CTRL */

	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0, idleTssi0_2C);
	/* Cache idle TSSI based on band/subband */
	wlc_lcn20phy_save_idletssi(pi, idleTssi0_2C);

cleanIdleTSSI:

	/* wlc_lcn20phy_set_txpwr_clamp(pi); */

	/* Clear tx PU override */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 0);
	wlc_lcn20phy_set_bbmult(pi, SAVE_bbmult);
	/* restore txgain override */
	wlc_lcn20phy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcn20phy_set_tx_gain(pi, &old_gains);
	phy_utils_write_phyreg(pi, LCN20PHY_TxPwrCtrlDeltaPwrLimit, save_TxPwrCtrlDeltaPwrLimit);
	PHY_REG_MOD(pi, LCN20PHY, TempSenseCorrection, tempsenseCorr, save_TempSenseCorrection);
	wlc_lcn20phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
		pi->tpci->data->cfg->cckpwroffset[0]);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}

void
WLBANDINITFN(wlc_lcn20phy_txpwrctrl_init)(phy_info_t *pi)
{
	bool suspend;
	/* uint8 stall_val; */
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int16 power_correction;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	if (NORADIO_ENAB(pi->pubpi)) {
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		return;
	}

	/* stall_val = READ_LCN20PHYREGFLD(pi, RxFeCtrl1, disable_stalls); */
	/* LCN20PHY_DISABLE_STALL(pi); */

	if (!pi->hwpwrctrl_capable) {
		wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);
	} else {
		/* Clear out all power offsets */
		wlc_lcn20phy_clear_tx_power_offsets(pi);

		if (pi_lcn20->tssical_time)
			wlc_lcn20phy_perpkt_idle_tssi_est(pi);
		else
			wlc_lcn20phy_idle_tssi_est(pi);

		/* Convert tssi to power LUT */
#ifdef WLC_TXCAL
		if (phy_tssical_get_pwr_tssi_tbl_in_use(pi->tssicali) == 1) {
			/* Use Tx cal based pwr_tssi_tbl */
			wlc_phy_apply_pwr_tssi_tble_chan_lcn20phy(pi);
		} else
#endif /* WLC_TXCAL	 */
		{
			/* Use PA Params */
			wlc_lcn20phy_set_estPwrLUT(pi, 0);
		}

		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlPwrMinMaxVal, pwrMinVal, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlPwrMinMaxVal, pwrMaxVal, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, txGainTable_mode, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, interpol_en, 0)
		PHY_REG_LIST_EXECUTE(pi);

#if TWO_POWER_RANGE_TXPWR_CTRL
		if (pi_lcn20->lcn20_twopwr_txpwrctrl_en) {
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2);

			/* Convert tssi to second power LUT */
			wlc_lcn20phy_set_estPwrLUT(pi, 1);

			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlPwrRange2,
					pwrMin_range2, pi_lcn20->pmin);
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlPwrRange2,
					pwrMax_range2, pi_lcn20->pmax);

			PHY_REG_LIST_START
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, pwrMinMaxEnable2, 0)
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlPwrMinMaxVal2, pwrMinVal2, 0)
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlPwrMinMaxVal2, pwrMaxVal2, 0)
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, interpol_en1, 0)
				/* include attenuator for direct control in normal power range */
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiRangeVal0, 1)
				/* bypass attenuator for direct control in low power range */
				PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 0)
			PHY_REG_LIST_EXECUTE(pi);
		}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

		phy_utils_write_phyreg(pi, LCN20PHY_TxPwrCtrlDeltaPwrLimit, 8);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, cckPwrOffset,
			pi->tpci->data->cfg->cckpwroffset[0]);

		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdCCK, baseIndex_cck_en, 1);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdCCK, pwrIndex_init_cck, 180);

		/* txpwroffset2g is in qdB steps */
		pi_lcn20->tempsenseCorr = (int16)(pi_lcn20->txpwroffset2g[channel-1]*2);

		PHY_REG_MOD(pi, LCN20PHY, TempSenseCorrection, tempsenseCorr,
			pi_lcn20->tempsenseCorr);

		if (pi_lcn20->tssical_time) {
			power_correction = pi_lcn20->tempsenseCorr +
				pi_lcn20->idletssi_corr;
			PHY_REG_MOD(pi, LCN20PHY, TempSenseCorrection,
			tempsenseCorr, power_correction);
		}

		wlc_lcn20phy_set_target_tx_pwr(pi, LCN20_TARGET_PWR);

		/* Caching the inital indicies */
		pi_lcn20->tssi_idx = pi_lcn20->init_txpwrindex;
		pi_lcn20->cck_tssi_idx = pi_lcn20->init_ccktxpwrindex;

		ASSERT(pi_lcn20->tssi_idx > 0);
		ASSERT(pi_lcn20->cck_tssi_idx > 0);

		/* Enable hardware power control */
		wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_HW);
	}

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	/* LCN20PHY_ENABLE_STALL(pi, stall_val); */
}

static void
wlc_lcn20phy_get_tssi_floor(phy_info_t *pi, uint16 *floor)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			*floor = pi_lcn20->tssi_floor;
			break;
		default:
			ASSERT(FALSE);
			break;
	}
}

void
wlc_lcn20phy_set_txpwr_clamp(phy_info_t *pi)
{
	uint16 tssi_floor = 0, idle_tssi_shift, adj_tssi_min;
	uint16 idleTssi_2C, idleTssi_OB, target_pwr_reg, intended_target;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int32 a1 = 0, b0 = 0, b1 = 0;
	int32 target_pwr_cck_max, target_pwr_ofdm_max, pwr, max_ovr_pwr;
	int32 fudge = 0*8; /* 1dB */
	phytbl_info_t tab;
	uint32 rate_table[WL_RATESET_SZ];
	uint8 ii;
	uint16 perPktIdleTssi;

	if (pi_lcn20->txpwr_clamp_dis || pi_lcn20->txpwr_tssifloor_clamp_dis) {
		pi_lcn20->target_pwr_ofdm_max = 0x7fffffff;
		pi_lcn20->target_pwr_cck_max = 0x7fffffff;
		if (pi_lcn20->btc_clamp) {
			target_pwr_cck_max = BTC_POWER_CLAMP;
			target_pwr_ofdm_max = BTC_POWER_CLAMP;
		} else {
			return;
		}
	} else {

		wlc_lcn20phy_get_tssi_floor(pi, &tssi_floor);
		phy_tpc_get_paparams_for_band(pi, &a1, &b0, &b1);

		perPktIdleTssi = PHY_REG_READ(pi, LCN20PHY, perPktIdleTssiCtrl,
			perPktIdleTssiUpdate_en);
		if (perPktIdleTssi)
			idleTssi_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew6,
				avgidletssi);
		else
			idleTssi_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi,
				idleTssi0);

		if (idleTssi_2C >= 256)
			idleTssi_OB = idleTssi_2C - 256;
		else
			idleTssi_OB = idleTssi_2C + 256;

		idleTssi_OB = idleTssi_OB >> 2; /* Converting to 7 bits */
		idle_tssi_shift = (127 - idleTssi_OB) + 4;
		adj_tssi_min = MAX(tssi_floor, idle_tssi_shift);
		pwr = wlc_lcn20phy_tssi2dbm(adj_tssi_min, a1, b0, b1);
		target_pwr_ofdm_max = (pwr - fudge) >> 1;
		target_pwr_cck_max = (MIN(pwr,
			(pwr + pi->tpci->data->cfg->cckpwroffset[0])) - fudge) >> 1;
		PHY_TMP(("idleTssi_OB= %d, idle_tssi_shift= %d, adj_tssi_min= %d, "
				"pwr = %d, target_pwr_cck_max = %d, target_pwr_ofdm_max = %d\n",
				idleTssi_OB, idle_tssi_shift, adj_tssi_min, pwr,
				target_pwr_cck_max, target_pwr_ofdm_max));
		pi_lcn20->target_pwr_ofdm_max = target_pwr_ofdm_max;
		pi_lcn20->target_pwr_cck_max = target_pwr_cck_max;

		if (pi_lcn20->btc_clamp) {
			target_pwr_cck_max = MIN(target_pwr_cck_max, BTC_POWER_CLAMP);
			target_pwr_ofdm_max = MIN(target_pwr_ofdm_max, BTC_POWER_CLAMP);
		}
	}

	if (pi->tpci->data->txpwroverride) {
		max_ovr_pwr = MIN(target_pwr_ofdm_max, target_pwr_cck_max);
		{
			uint8 core;
			uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
			BCM_REFERENCE(phyrxchain);
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {
				pi->tx_power_min_per_core[core] =
					MIN(pi->tx_power_min_per_core[core], max_ovr_pwr);
			}
		}
		return;
	}

	for (ii = 0; ii < ARRAYSIZE(rate_table); ii ++)
		rate_table[ii] = pi_lcn20->rate_table[ii];

	/* Adjust Rate Offset Table to ensure intended tx power for every OFDM/CCK */
	/* rate is less than target_power_ofdm_max/target_power_cck_max */
	target_pwr_reg = wlc_lcn20phy_get_target_tx_pwr(pi);
	for (ii = 0; ii < WL_RATESET_SZ_DSSS; ii ++) {
		intended_target = target_pwr_reg - rate_table[ii];
		if (intended_target > target_pwr_cck_max)
			rate_table[ii] = rate_table[ii] + (intended_target - target_pwr_cck_max);
		PHY_TMP(("Rate: %d, maxtar = %d, target = %d, origoff: %d, clampoff: %d\n",
			ii, target_pwr_cck_max, intended_target,
			pi_lcn20->rate_table[ii], rate_table[ii]));
	}
	for (ii = WL_RATESET_SZ_DSSS;
		ii < WL_RATESET_SZ; ii ++) {
		intended_target = target_pwr_reg - rate_table[ii];
		if (intended_target > target_pwr_ofdm_max)
			rate_table[ii] = rate_table[ii] + (intended_target - target_pwr_ofdm_max);
		PHY_TMP(("Rate: %d, maxtar = %d, target = %d, origoff: %d, clampoff: %d\n",
			ii, target_pwr_ofdm_max, intended_target,
			pi_lcn20->rate_table[ii], rate_table[ii]));
	}

	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
	tab.tbl_ptr = rate_table; /* ptr to buf */
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_RATE_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);
}

#if defined(WLTEST)
static int
phy_lcn20_tpc_set_pavars(phy_type_tpc_ctx_t *ctx, void *a, void *p)
{
	phy_lcn20_tpc_info_t *tpci = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = tpci->pi;
	uint16 inpa[WL_PHY_PAVARS_LEN];
	uint j = 3; /* PA parameters start from offset 3 */
	bcopy(p, inpa, sizeof(inpa));

	if (inpa[0] != PHY_TYPE_LCN20) {
		PHY_ERROR(("Wrong phy type %d\n", inpa[0]));
		return BCME_BADARG;
	}

	switch (inpa[1]) {
	case WL_CHAN_FREQ_RANGE_2G:
		pi->txpa_2g[2] = inpa[j++]; /* a1 */
		pi->txpa_2g[0] = inpa[j++]; /* b0 */
		pi->txpa_2g[1] = inpa[j++]; /* b1 */
		return BCME_OK;
	default:
		PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
		return BCME_OUTOFRANGECHAN;
	}
}

static int
phy_lcn20_tpc_get_pavars(phy_type_tpc_ctx_t *ctx, void *a, void *p)
{
	phy_lcn20_tpc_info_t *tpci = (phy_lcn20_tpc_info_t *)ctx;
	phy_info_t *pi = tpci->pi;
	uint16 *outpa = a;
	uint16 inpa[WL_PHY_PAVARS_LEN];
	uint j = 3; /* PA parameters start from offset 3 */

	bcopy(p, inpa, sizeof(inpa));

	outpa[0] = inpa[0]; /* Phy type */
	outpa[1] = inpa[1]; /* Band range */
	outpa[2] = inpa[2]; /* Chain */

	if (inpa[0] != PHY_TYPE_LCN20) {
		PHY_ERROR(("Wrong phy type %d\n", inpa[0]));
		outpa[0] = PHY_TYPE_NULL;
		return BCME_BADARG;
	}

	switch (inpa[1]) {
	case WL_CHAN_FREQ_RANGE_2G:
		outpa[j++] = pi->txpa_2g[2];		/* a1 */
		outpa[j++] = pi->txpa_2g[0];		/* b0 */
		outpa[j++] = pi->txpa_2g[1];		/* b1 */
		return BCME_OK;
	default:
		PHY_ERROR(("bandrange %d is out of scope\n", inpa[1]));
		return BCME_OUTOFRANGECHAN;
	}
}
#endif // endif

static int
phy_lcn20_tpc_get_est_pout(phy_type_tpc_ctx_t *ctx,
	uint8* est_Pout, uint8* est_Pout_adj, uint8* est_Pout_cck)
{
	phy_lcn20_tpc_info_t *tpc_info = (phy_lcn20_tpc_info_t *) ctx;
	phy_info_t *pi = tpc_info->pi;

	*est_Pout_cck = 0;

	if (pi->hwpwrctrl) {

		/* Get power estimates */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_lcn20phy_get_tssi(pi, (int8*)&est_Pout[0], (int8*)est_Pout_cck);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		est_Pout_adj[0] = est_Pout[0];
	}
	return BCME_OK;
}
