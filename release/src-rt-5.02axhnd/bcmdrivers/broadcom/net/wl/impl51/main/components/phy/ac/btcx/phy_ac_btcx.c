/*
 * ACPHY BT Coex module implementation
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
 * $Id: phy_ac_btcx.c 702070 2017-05-30 15:43:06Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_btcx.h"
#include <phy_ac.h>
#include <phy_ac_btcx.h>
#include <phy_ac_noise.h>
#include <phy_ac_rxgcrs.h>
#include <phy_rxgcrs.h>
#include <phy_wd.h>
#include <phy_stf.h>
#include <phy_misc_api.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_ac.h>
#include <wlc_phy_int.h>

#include <siutils.h>
#include <sbchipc.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>

#include <phy_rstr.h>
#include <bcmdevs.h>
#include <hndpmu.h>
#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_ac_btcx_info {
	phy_info_t			*pi;
	phy_ac_info_t		*aci;
	phy_btcx_info_t		*cmn_info;
	phy_ac_btcx_data_t	*data; /* shared data */
	uint16	btcx_femctrl_save;
	uint16	btcx_femctrl_bt_val;
	uint16	btcx_femctrl_wlan_val;
	int8	ltecx_elna_bypass_status;
	int8	btswitch; /* bt switch state (-1: AUTO, 0: WL, 1: BT) */
	int8	bt_sw_state;
	bool	swctrl_to_bt_in_coex;
	bool	coex_abstxpwr_ison;
	int8	ack_pwr_offset;
	int8	max_siso_resp_pwr[PHY_CORE_MAX];
	int8	txpwrcap_orig[PHY_CORE_MAX]; /* in qdBm steps */
	int8	btcx_txpwrcap[PHY_CORE_MAX]; /* in qdBm steps */
/* add other variable size variables here at the end */
};

typedef struct {
	phy_ac_btcx_info_t info;
	phy_ac_btcx_data_t data;
} phy_ac_btcx_mem_t;

/* local functions */
static int phy_ac_btcx_init(phy_type_btcx_ctx_t *ctx);
#if !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX)
static bool phy_ac_wd_btcx_desense(phy_wd_ctx_t *ctx);
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX) */
static bool phy_ac_wd_wars(phy_wd_ctx_t *ctx);
static void wlc_phy_btc_adjust_acphy(phy_type_btcx_ctx_t *ctx, bool btactive);
static void wlc_phy_btcx_invert_prisel_polarity(phy_ac_btcx_info_t *btcxi, int8 state);
static void phy_ac_btcx_adjust_preempt_on_bt_activity(phy_ac_btcx_info_t *btcxi);

#define BTC_SISO_RESP_CORE_MASK     0xFFu
#define BTC_SISO_RESP_TXPWR_SHIFT   8u
/* Tx offset is a S5.1 format, so max. offset is 31 h-dB = 15.5 dB */
#define MAX_TXPWRCTRL_OFFSET		31

#define BTC_CURR_PWROFFSET_RET	3
#define ACPHY_UCM_TXPWRCAP	-128
#define DEFAULT_TXPWRCAP	127

/* local functions */
static void phy_ac_btcx_nvram_attach(phy_info_t *pi);
static void wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(phy_type_btcx_ctx_t *ctx, int8 state,
	bool set);
static int8 wlc_phy_get_femctrl_bt_wlan_ovrd_acphy(phy_type_btcx_ctx_t *ctx);
static void phy_ac_btcx_override_enable(phy_type_btcx_ctx_t *ctx);
static void phy_ac_btcx_override_disable(phy_type_btcx_ctx_t *ctx);

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
static void phy_ac_btcx_femctrl_mask(phy_type_btcx_ctx_t *ctx);
#endif // endif

#if defined(WLTEST)
static int phy_ac_btcx_get_preemptstatus(phy_type_btcx_ctx_t *ctx, int32* ret_ptr);
#endif // endif

#if !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX)
static int wlc_phy_desense_ltecx_acphy(phy_type_btcx_ctx_t *ctx, int32 mode);
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX) */
#if !defined(WLC_DISABLE_ACI)
static int wlc_phy_desense_btcoex_acphy(phy_type_btcx_ctx_t *ctx, int32 mode);
#endif /* !defined(WLC_DISABLE_ACI) */
static int phy_ac_btcx_set_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 set_val);
static int phy_ac_btcx_get_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 *ret_val);
static int phy_ac_btcx_set_mode(phy_type_btcx_ctx_t *ctx, int btc_mode);

#ifdef WL_UCM
static void phy_ac_btcx_register_impl_ucm(phy_type_btcx_fns_t *fns);
static int phy_ac_btcx_ucm_update_siso_resp_offset(phy_type_btcx_ctx_t *ctx,
		int8 *siso_resp_pwr, uint8 len);
static void phy_ac_btcx_ucm_update_only_siso_resp_offset(phy_type_btcx_ctx_t *ctx);
static int phy_ac_btcx_ucm_set_desense_rxgain(phy_type_btcx_ctx_t *ctx, uint8 *desense_val);
static int32 phy_ac_btcx_ucm_get_desense_rxgain(phy_type_btcx_ctx_t *ctx);
static void phy_ac_btcx_ucm_get_siso_ack_pwr(phy_type_btcx_ctx_t *ctx, void *a);
static void phy_ac_btcx_ucm_get_curr_siso_resp_pwr_offset(phy_type_btcx_ctx_t *ctx, void *a);
static int phy_ac_btcx_ucm_set_txpwrcaplmt(phy_type_btcx_ctx_t *ctx,
		int8 *tx_power_cap_in_qdbm, bool active);
static void phy_ac_btcx_ucm_set_txpwrcap_orig(phy_type_btcx_ctx_t *ctx, int8 *tx_pwrcap);
static void phy_ac_btcx_ucm_update_ack_pwr_offset(phy_ac_btcx_info_t *btcxi);
#endif /* WL_UCM */

/* register phy type specific implementation */
phy_ac_btcx_info_t *
BCMATTACHFN(phy_ac_btcx_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_btcx_info_t *cmn_info)
{
	phy_ac_btcx_info_t *btcx_info;
	phy_type_btcx_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((btcx_info = phy_malloc(pi, sizeof(phy_ac_btcx_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	btcx_info->pi = pi;
	btcx_info->aci = aci;
	btcx_info->cmn_info = cmn_info;
	btcx_info->data = &(((phy_ac_btcx_mem_t *) btcx_info)->data);

	/* register watchdog fn */
#if !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX)
	if (phy_wd_add_fn(pi->wdi, phy_ac_wd_btcx_desense, btcx_info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_AC_BTCX_DESENSE,
		PHY_WD_FLAG_DEF_SKIP) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX) */

	if (phy_wd_add_fn(pi->wdi, phy_ac_wd_wars, btcx_info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_AC_BTCX_WARS,
		PHY_WD_FLAG_DEF_SKIP) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* Read srom params from nvram */
	phy_ac_btcx_nvram_attach(pi);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init_btcx = phy_ac_btcx_init;
	fns.adjust = wlc_phy_btc_adjust_acphy;
	fns.set_femctrl = wlc_phy_set_femctrl_bt_wlan_ovrd_acphy;
	fns.get_femctrl = wlc_phy_get_femctrl_bt_wlan_ovrd_acphy;
	fns.override_enable = phy_ac_btcx_override_enable;
	fns.override_disable = phy_ac_btcx_override_disable;
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
	fns.femctrl_mask = phy_ac_btcx_femctrl_mask;
#endif // endif
#if defined(WLTEST)
	fns.get_preemptstatus = phy_ac_btcx_get_preemptstatus;
#endif // endif
#if !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX)
	fns.desense_ltecx = wlc_phy_desense_ltecx_acphy;
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX) */
#if !defined(WLC_DISABLE_ACI)
	fns.desense_btc = wlc_phy_desense_btcoex_acphy;
#endif /* !defined(WLC_DISABLE_ACI) */
	fns.set_restage_rxgain = phy_ac_btcx_set_restage_rxgain;
	fns.get_restage_rxgain = phy_ac_btcx_get_restage_rxgain;
	fns.mode_set = phy_ac_btcx_set_mode;
#ifdef WL_UCM
	phy_ac_btcx_register_impl_ucm(&fns);
#endif /* WL_UCM */

	fns.ctx = btcx_info;

	/* Initialize std param */
	btcx_info->data->poll_adc_WAR = FALSE;
	btcx_info->data->btc_mode = 0;

#ifdef WL_UCM
	{
		uint8 core;
		FOREACH_CORE(pi, core) {
			btcx_info->txpwrcap_orig[core] = DEFAULT_TXPWRCAP;
			btcx_info->btcx_txpwrcap[core] = DEFAULT_TXPWRCAP;
		}
	}
#endif /* WL_UCM */

	if (phy_btcx_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_btcx_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	/* PHY-Feature specific parameter initialization */

	return btcx_info;

	/* error handling */
fail:
	if (btcx_info != NULL)
		phy_mfree(pi, btcx_info, sizeof(phy_ac_btcx_mem_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_btcx_unregister_impl)(phy_ac_btcx_info_t *btcx_info)
{
	phy_info_t *pi;
	phy_btcx_info_t *cmn_info;

	ASSERT(btcx_info);
	pi = btcx_info->pi;
	cmn_info = btcx_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_btcx_unregister_impl(cmn_info);

	phy_mfree(pi, btcx_info, sizeof(phy_ac_btcx_mem_t));
}

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
void
phy_ac_btcx_reset_swctrl(phy_ac_btcx_info_t *bti)
{
	bti->swctrl_to_bt_in_coex = (bool) PHY_GETINTVAR_DEFAULT(pi, rstr_swctrl_to_bt_in_coex, 0);
}
#endif /* !defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX) */

/* Initialize the FEMctrl register for platforms where
 * band specific and WL/BT specific FEMctrlmask values
 * are handled within the microcode via shmems. This
 * is to cover the case where BT coex is disabled
 */
static int
phy_ac_btcx_init(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	btcxi->bt_sw_state = AUTO;
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
	if (btcxi->swctrl_to_bt_in_coex) {
		if (ACMAJORREV_2(btcxi->pi->pubpi->phy_rev)) {
			WRITE_PHYREG(btcxi->pi, FemCtrl, 0xffc);
		}
	}
#endif /* !defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX) */
	return BCME_OK;
}

/* inter-module data API */
phy_ac_btcx_data_t *
phy_ac_btcx_get_data(phy_ac_btcx_info_t *btcxi)
{
	return btcxi->data;
}

#if !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX)
static bool
phy_ac_wd_btcx_desense(phy_wd_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	BCM_REFERENCE(pi);
#if !defined(BCMLTECOEX_DISABLED)
	bool suspend = FALSE;
	wlc_phy_conditional_suspend(pi, &suspend);

	if (wlapi_ltecx_get_lte_map(pi->sh->physhim)) {
		int32 ltecx_ed_thresh_val = LTECX_ED_THRESH;
		wlc_phy_adjust_ed_thres(pi, &ltecx_ed_thresh_val, TRUE);
		if (wlapi_ltecx_chk_elna_bypass_mode(pi->sh->physhim) &&
			!btcxi->ltecx_elna_bypass_status) {
			wlc_phy_desense_ltecx_acphy(btcxi, 1);
		} else if (!wlapi_ltecx_chk_elna_bypass_mode(pi->sh->physhim) &&
			btcxi->ltecx_elna_bypass_status) {
			wlc_phy_desense_ltecx_acphy(btcxi, 0);
		}
	} else {
		wlc_phy_apply_default_edthresh_acphy(pi, pi->radio_chanspec);
		if (btcxi->ltecx_elna_bypass_status) {
			wlc_phy_desense_ltecx_acphy(btcxi, 0);
		}
	}
	wlc_phy_conditional_resume(pi, &suspend);
#endif /* !defined(BCMLTECOEX_DISABLED) */
	return TRUE;
}

/********** Desense LTE  ******** */
static int
wlc_phy_desense_ltecx_acphy(phy_type_btcx_ctx_t *ctx, int32 mode)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *) ctx;
	acphy_desense_values_t lte_desense;

	/* Apply these settings if this is called while on an active 2g channel */
	if (!CHSPEC_IS2G(btcxi->pi->radio_chanspec) || SCAN_RM_IN_PROGRESS(btcxi->pi)) {
		return BCME_OK;
	}

	/* Start with everything at 0 */
	bzero(&lte_desense, sizeof(acphy_desense_values_t));
	btcxi->data->ltecx_mode = mode;
	if (mode == 0) {
		btcxi->ltecx_elna_bypass_status = 0;
	}
	lte_desense.on = (mode > 0);
	switch (mode) {
	case 1: /* LTE - Add new cases in the future */
		lte_desense.ofdm_desense = 24;
		lte_desense.bphy_desense = 24;
		lte_desense.elna_bypass = 1;
		lte_desense.nf_hit_lna12 = 24;
		btcxi->ltecx_elna_bypass_status = 1;
		break;
	default:
		break;
	}
	phy_ac_rxgcrs_set_desense(btcxi->aci->rxgcrsi, &lte_desense, LTE_DESENSE);

	/* Apply these settings */
	wlc_phy_desense_apply_acphy(btcxi->pi, TRUE);
	return BCME_OK;
}
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMLTECOEX) */

#if !defined(WLC_DISABLE_ACI)
/********** Desense BT  ******** */
static int
wlc_phy_desense_btcoex_acphy(phy_type_btcx_ctx_t *ctx, int32 mode)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *) ctx;
	phy_info_t *pi = btcxi->pi;
	acphy_desense_values_t bt_desense;
	int32 old_mode = btcxi->data->btc_mode;

	if (ACPHY_ENABLE_FCBS_HWACI(pi) && !ACPHY_HWACI_WITH_DESENSE_ENG(pi)) {
		return BCME_OK;
	}

	/* clear hwaci mitigation params if entering btcoex_desense mode != 0 */
	if (ACHWACIREV(pi) && ACMAJORREV_5(pi->pubpi->phy_rev) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		if (mode != 0) {
			/* disable hwaci mitigation */
			wlc_phy_hwaci_mitigate_acphy(pi, 0);
			/* disable hwaci detection */
			MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable, 0);
		}
	}

	/* Start with everything at 0 */
	bzero(&bt_desense, sizeof(acphy_desense_values_t));
	btcxi->data->btc_mode = mode;
	bt_desense.on = (mode > 0);
	switch (mode) {
	case 1: /* BT power =  -30dBm, -35dBm */
		bt_desense.lna1_gainlmt_desense = 1;   /* 4 */
		bt_desense.lna2_gainlmt_desense = 3;   /* 3 */
		bt_desense.elna_bypass = 0;
		break;
	case 2: /* BT power = -20dBm , -25dB */
		bt_desense.lna1_gainlmt_desense = 0;   /* 5 */
		bt_desense.lna2_gainlmt_desense = 0;   /* 6 */
		bt_desense.elna_bypass = 1;
		break;
	case 3: /* BT power = -15dBm */
		bt_desense.lna1_gainlmt_desense = 0;   /* 5 */
		bt_desense.lna2_gainlmt_desense = 2;   /* 4 */
		bt_desense.elna_bypass = 1;
		bt_desense.nf_hit_lna12 = 2;
		break;
	case 4: /* BT power = -10dBm */
		bt_desense.lna1_gainlmt_desense = 1;   /* 4 */
		bt_desense.lna2_gainlmt_desense = 2;   /* 4 */
		bt_desense.elna_bypass = 1;
		bt_desense.nf_hit_lna12 = 3;
		break;
	case 5: /* BT power = -5dBm */
		bt_desense.lna1_gainlmt_desense = 3;   /* 2 */
		bt_desense.lna2_gainlmt_desense = 0;   /* 6 */
		bt_desense.elna_bypass = 1;
		bt_desense.nf_hit_lna12 = 13;
		break;
	case 6: /* BT power = 0dBm */
		bt_desense.lna1_gainlmt_desense = 3;   /* 2 */
		bt_desense.lna2_gainlmt_desense = 4;   /* 2 */
		bt_desense.elna_bypass = 1;
		bt_desense.nf_hit_lna12 = 24;
		break;
	case 7: /* Case added for 4359 */
		bt_desense.lna1_tbl_desense = 3;	/* 1 */
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			/* To account for CRSMinPwrTh for INIT gain chnage */
			bt_desense.clipgain_desense[0] = 6;
		} else {
			/* Value should be take between 2-11 for Tiny */
			bt_desense.mixer_setting_desense = 4;
		}
		break;

	default:
		break;
	}
	phy_ac_rxgcrs_set_desense(btcxi->aci->rxgcrsi, &bt_desense, BT_DESENSE);

	if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		wlc_phy_preemption_abort_during_timing_search(pi, (mode == 0) || (mode == 2));
	}

	/* Apply these settings if this is called while on an active 2g channel */
	if (CHSPEC_IS2G(pi->radio_chanspec) && !SCAN_RM_IN_PROGRESS(pi)) {
		/* If bt desense changed, then reset aci params. But, keep the aci settings intact
		   if bt is switched off (as you will still need aci desense)
		*/
		if ((mode != old_mode) && (mode > 0))
			wlc_phy_desense_aci_reset_params_acphy(pi, FALSE, FALSE, FALSE);
		wlc_phy_desense_apply_acphy(pi, TRUE);

		if (ACHWACIREV(pi) && ACMAJORREV_5(pi->pubpi->phy_rev)) {
			if (mode == 0) {
				MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable, 0);
			OSL_DELAY(30);	/* allow detection to toggle */
				/* turn on hwaci detection */
			MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable, 1);
			}
		}
	}
	return BCME_OK;
}
#endif /* !defined(WLC_DISABLE_ACI) */

#if defined(WLTEST)
static int
phy_ac_btcx_get_preemptstatus(phy_type_btcx_ctx_t *ctx, int32* ret_ptr)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	bool curr_channel_in_2G = CHSPEC_IS2G(btcxi->pi->radio_chanspec);
	int bt_active_in_curr_band = (btcxi->cmn_info->data->bt_active) & curr_channel_in_2G;
	*ret_ptr = (int32) (bt_active_in_curr_band << 1) |
		(phy_ac_noise_get_data(btcxi->aci->noisei)->current_preemption_status);
	return BCME_OK;
}
#endif // endif

static void
wlc_phy_btc_adjust_acphy(phy_type_btcx_ctx_t *ctx, bool btactive)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;

	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
	  wlapi_suspend_mac_and_wait(pi->sh->physhim);
	  wlc_phy_mlua_adjust_acphy(pi, btactive);
	  wlapi_enable_mac(pi->sh->physhim);
	}

	phy_ac_btcx_adjust_preempt_on_bt_activity(btcxi);
}

static void
wlc_phy_stop_bt_toggle_acphy(phy_ac_btcx_info_t *btcxi)
{
	phy_info_t *pi = btcxi->pi;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
	int8 shared_ant_mask;
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_FEM_BT) {
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_BT_SHARE_ANT0)
			shared_ant_mask = 1; /* 2 chain devices with first core shared */
		else
			shared_ant_mask = 2; /* 2 or 3 chain devices with middle core shared */
	} else
		return;

	if (btcxi->bt_sw_state == AUTO) {
		phy_utils_phyreg_enter(pi);
		if (((stf_shdata->phytxchain & shared_ant_mask) == 0) &&
			((shared_ant_mask & stf_shdata->phyrxchain) == 0)) {
			wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(btcxi, 1, FALSE);
			/* forced bt switch to BT side instead of toggling */
		} else
			wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(btcxi, AUTO, FALSE);
		phy_utils_phyreg_exit(pi);
	}
}

static bool
phy_ac_wd_wars(phy_wd_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	if (CHIPID(btcxi->pi->sh->chip) == BCM4360_CHIP_ID)
		wlc_phy_stop_bt_toggle_acphy(btcxi);
	return TRUE;
}

static void
BCMATTACHFN(phy_ac_btcx_nvram_attach)(phy_info_t *pi)
{
#ifndef BOARD_FLAGS
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	BF_SROM11_BTCOEX(pi_ac) = ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
		BFL_SROM11_BTCOEX) != 0);
#endif /* BOARD_FLAGS */
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

void
wlc_phy_set_bt_on_core1_acphy(phy_info_t *pi, uint8 bt_fem_val, uint16 gpioen)
{
	/* *** NOTE : For boards with BT on sharead antenna, update code in
	   wlc_bmac_set_ctrl_bt_shd0() so that in down mode BT has control of fem
	   Also, BT needs control when insmod (but not up), in that case wlc_phy_ac.c
	   is not even called, and so need to have some code in wlc_bmac.
	*/

	/* chip_bandsel = bandsel */
	MOD_PHYREG(pi, BT_SwControl, bt_sharing_en, 1);
	/* Bring c1_2g ctrls on gpio/srmclk */
	WRITE_PHYREG(pi, shFemMuxCtrl, 0x555);

	/* Setup chipcontrol (chipc[3] for acphy_gpios) */
	si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		CCTRL4360_BTSWCTRL_MODE, CCTRL4360_BTSWCTRL_MODE);

	/* point fem_bt to chip_bt control line */
	MOD_PHYREG(pi, BT_FemControl, bt_fem, bt_fem_val);

	if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
		/* PHY controls bits 5,6,7 of gpio for BT boards (only needed for A0) */
		si_gpiocontrol(pi->sh->sih, 0xffff, 0x00e0, GPIO_DRV_PRIORITY);

		/* acphy_gpios = mux(bt_fem, femctrl[7:4]) */
		WRITE_PHYREG(pi, gpioSel, 0xb);
		/* bt control lines in gpio 5,6,7 */
		WRITE_PHYREG(pi, gpioLoOutEn, 0xe0);

		pi->u.pi_acphy->btcxi->data->poll_adc_WAR = TRUE;

		/* 4360A0 : Force in WLAN mode, as A0 does not have inv_btcx_prisel bit,
		   and we have to change top level MAC definition of prisel (too complicated)
		   We are not supporting BT on 4360A0 anyway
		*/
		MOD_PHYREG_2(pi, BT_FemControl, bt_en, 0, bt_en_ovrd, 1);
	} else {
		pi->u.pi_acphy->btcxi->data->poll_adc_WAR = FALSE;

		/* bt_prisel is active low */
		MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 1);
	}

	/* In wlan Off/sleep mode, Make BT as input, and driver others as 0 */
	si_gpioout(pi->sh->sih, gpioen, 0, GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, gpioen, gpioen, GPIO_DRV_PRIORITY);
	si_gpiocontrol(pi->sh->sih, gpioen, 0, GPIO_DRV_PRIORITY);
}

void
wlc_phy_bt_on_gpio4_acphy(phy_info_t *pi)
{
	uint16 mask = 0x10;    /* gpio 4 = 0 */

	/* Force gpio4 to be 0 */
	si_gpioout(pi->sh->sih, (1 << 4), (0 << 4), GPIO_DRV_PRIORITY);
	si_gpioouten(pi->sh->sih, (1 << 4), (1 << 4), GPIO_DRV_PRIORITY);

	/* Take away gpio4 contorl from phy */
	si_gpiocontrol(pi->sh->sih, mask, 0, GPIO_DRV_PRIORITY);
}

static void
wlc_phy_btcx_invert_prisel_polarity(phy_ac_btcx_info_t *btcxi, int8 state)
{
	uint16 btcx_ctrl = 0;
	btcx_ctrl = R_REG(btcxi->pi->sh->osh, D11_BTCX_CTL(btcxi->pi));
	if (state == ON) { /* invert prisel polarity */
		W_REG(btcxi->pi->sh->osh, D11_BTCX_CTL(btcxi->pi),
				btcx_ctrl |= BTCX_CTRL_PRI_POL);
	} else {
		W_REG(btcxi->pi->sh->osh, D11_BTCX_CTL(btcxi->pi),
				btcx_ctrl &= ~BTCX_CTRL_PRI_POL);
	}
	btcxi->btswitch = state;
}

static void
wlc_phy_set_femctrl_bt_wlan_ovrd_acphy(phy_type_btcx_ctx_t *ctx, int8 state, bool set)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	if (set) {
		btcxi->bt_sw_state = state;
	}
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
	uint16 femctrlmask = 0x3ff;

	if (BCM4350_CHIP(pi->sh->chip)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (state == ON)
			femctrlmask = wlc_phy_set_mask_for_femctrl10(pi);
		MOD_PHYREG(pi, FemCtrl, femCtrlMask, femctrlmask);
		wlapi_enable_mac(pi->sh->physhim);
		/* invert prisel polarity */
		wlc_phy_btcx_invert_prisel_polarity(btcxi, state);
	}
	else
#endif	/* WL_SISOCHIP && SWCTRL_TO_BT_IN_COEX */
	{
		if (BCM4349_CHIP(pi->sh->chip) || CHIPID(pi->sh->chip) == BCM4364_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM4347_CHIP_ID ||
			CHIPID(pi->sh->chip) == BCM4369_CHIP_ID) {
			/* invert prisel polarity */
			wlc_phy_btcx_invert_prisel_polarity(btcxi, state);
		} else {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

			if (state == ON) {
				MOD_PHYREG_2(pi, BT_FemControl, bt_en, 1, bt_en_ovrd, 1);
			} else if (state == OFF) {
				MOD_PHYREG_2(pi, BT_FemControl, bt_en, 0, bt_en_ovrd, 1);
			} else {
				MOD_PHYREG_2(pi, BT_FemControl, bt_en_ovrd, 0, bt_en, 0);
			}

			wlapi_enable_mac(pi->sh->physhim);
		}
	}
}

static int8
wlc_phy_get_femctrl_bt_wlan_ovrd_acphy(phy_type_btcx_ctx_t *ctx)
{
	int8 state;
	uint8 bten, bten_ovrd;
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;

	if (BCM4349_CHIP(pi->sh->chip) || CHIPID(pi->sh->chip) == BCM4364_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM4347_CHIP_ID ||
		CHIPID(pi->sh->chip) == BCM4369_CHIP_ID) {
		state = btcxi->btswitch;
	} else {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		bten = READ_PHYREGFLD(pi, BT_FemControl, bt_en);
		bten_ovrd = READ_PHYREGFLD(pi, BT_FemControl, bt_en_ovrd);

		state = (bten_ovrd == 0) ? AUTO : (bten == 0) ? OFF : ON;

		wlapi_enable_mac(pi->sh->physhim);
	}
	return state;
}

static void
phy_ac_btcx_adjust_preempt_on_bt_activity(phy_ac_btcx_info_t *btcxi)
{
	phy_info_t *pi = btcxi->pi;
	phy_ac_noise_info_t *ni = pi->u.pi_acphy->noisei;

	/* Enter if 4350 in 2G + preemption enabled */
	if (IS_4350(pi) && CHSPEC_IS2G(pi->radio_chanspec) &&
	(pi->sh->interference_mode & ACPHY_ACI_PREEMPTION)) {
		int btc_mode = wlapi_bmac_btc_mode_get(pi->sh->physhim);
		uint8 aci_status = (uint8) (phy_ac_noise_get_data(ni)->hw_aci_status);
		/* btc_mode 0 */
		if (btc_mode == WL_BTC_DISABLE) {
			/* Restore default 2G preemption settings */
			phy_ac_noise_preempt(ni, TRUE, FALSE);
			/* Adjust preeemption clip count thresholds
			 * based on ACI detected/undetected
			 */
			wlc_phy_switch_preemption_settings(pi, aci_status);
		}
		else if (btc_mode == WL_BTC_HYBRID) {
			/* Disable preemption if BT active */
			phy_ac_noise_preempt(ni, !(pi->btcxi->data->bt_active), FALSE);
			/* If BT not active adjust
			 * preeemption clip count thresholds
			 * based on ACI detected/undetected
			 */
			if (!(pi->btcxi->data->bt_active))
				wlc_phy_switch_preemption_settings(pi, aci_status);
		}
	}
}

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
static void wlc_phy_btcx_wlan_femctrl_save(phy_ac_btcx_info_t *btcxi)
{
	btcxi->btcx_femctrl_save = READ_PHYREG(btcxi->pi, FemCtrl);
}

static void wlc_phy_btcx_wlan_femctrl_restore(phy_ac_btcx_info_t *btcxi)
{
	WRITE_PHYREG(btcxi->pi, FemCtrl, btcxi->btcx_femctrl_save);
}
#endif // endif

void
chanspec_btcx(phy_info_t *pi)
{
	phy_ac_btcx_info_t *btcxi = pi->u.pi_acphy->btcxi;
	/* Set the bt priset based on the requested state */
	if (BCM4349_CHIP(pi->sh->chip) ||(CHIPID(pi->sh->chip) == BCM4364_CHIP_ID)) {
		/* invert prisel polarity */
		wlc_phy_btcx_invert_prisel_polarity(btcxi, btcxi->btswitch);
	}
}

static void
phy_ac_btcx_override_enable(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;

	/* This is required only for 2G operation. No BTCX in 5G */
	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		(CHSPEC_IS2G(pi->radio_chanspec) || ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
		/* Release FEM ctrl lines to BT to allow the A2DP flushing */
		if (btcxi->swctrl_to_bt_in_coex) {
			wlc_phy_btcx_wlan_femctrl_save(btcxi);
			phy_utils_write_phyreg(pi, ACPHY_FemCtrl(pi->pubpi->phy_rev),
				btcxi->btcx_femctrl_bt_val);
		}
#endif // endif

		wlapi_coex_flush_a2dp_buffers(pi->sh->physhim);

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi), BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force WLAN antenna and priority */
		OR_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL);

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
		/* Give FEMctrl to WLAN */
		if (btcxi->swctrl_to_bt_in_coex) {
			phy_utils_write_phyreg(pi, ACPHY_FemCtrl(pi->pubpi->phy_rev),
			              btcxi->btcx_femctrl_wlan_val);
		}
#endif // endif
	}
}

static void
phy_ac_btcx_override_disable(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;

	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		(CHSPEC_IS2G(pi->radio_chanspec) || ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
		/* Restore FEMCTRL line */
		if (btcxi->swctrl_to_bt_in_coex) {
			wlc_phy_btcx_wlan_femctrl_restore(btcxi);
		}
#endif // endif

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi), BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force BT priority */
		AND_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
	}
}

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
static void
phy_ac_btcx_femctrl_mask(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	if (btcxi->swctrl_to_bt_in_coex) {
		wlc_phy_ac_femctrl_mask_on_band_change_btcx(btcxi);
	}
}

void
wlc_phy_ac_femctrl_mask_on_band_change_btcx(phy_ac_btcx_info_t *btcxi)
{
	bool suspend;
	phy_info_acphy_t *pi_ac = btcxi->aci;
	phy_info_t *pi = btcxi->pi;
	uint16 femctrlmask = 0x3ff;
	uint8 enBtSignalsToFEMLut;
	bool write_femctrl = FALSE;

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		/* When WLAN is in 5G, WLAN table should control the FEM lines */
		/* and BT should not have any access permissions */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			/* disable BT Fem control table accesses */
			enBtSignalsToFEMLut = 0;
			if (!ACPHY_FEMCTRL_ACTIVE(pi)) {
				femctrlmask = pi_ac->sromi->femctrlmask_5g;
				write_femctrl = TRUE;
			} else {
				if (BFCTL(pi_ac) == 10) {
					femctrlmask = wlc_phy_set_mask_for_femctrl10(pi);
					write_femctrl = TRUE;
				} else if (pi->u.pi_acphy->sromi->femctrl == 2) {
					if (pi->u.pi_acphy->sromi->femctrl_sub == 5)
						si_pmu_switch_off_PARLDO(pi->sh->sih,
							pi->sh->osh);
					if (pi->u.pi_acphy->sromi->femctrl_sub == 6)
						si_pmu_switch_on_PARLDO(pi->sh->sih,
							pi->sh->osh);
					write_femctrl = FALSE;
				} else {
					femctrlmask =  0x3ff;
					write_femctrl = TRUE;
				}
			}
		} else {
			/* When WLAN is in 2G, BT controls should be allowed to go through */
			/* BT should also be able to control FEM Control Table */
			enBtSignalsToFEMLut = 0;
			femctrlmask = 0x3ff;
			write_femctrl = TRUE;
		}
	}
	if (write_femctrl) {
		/* If FEM control lines need to be released to BT based on prisel, then
		 * write the WLAN and BT femctrlmask settings to the shmems; Otherwise
		 * write the FEMctrl register directly
		 */
		if (btcxi->swctrl_to_bt_in_coex) {
			/* Suspend the MAC before writing the shmems and re-enable later
			 * to ensure that there is a BT prisel to WL prisel transition
			 * so that the WLAN FEMctrlmask settings are applied in scenarios
			 * where WL already has the prisel when the shmems are changed
			 */
			suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) &
				MCTL_EN_MAC);
			if (!suspend)
				wlapi_suspend_mac_and_wait(pi->sh->physhim);

			btcxi->btcx_femctrl_bt_val = ((pi_ac->sromi->femctrlmask_5g <<
				ACPHY_FemCtrl_femCtrlMask_SHIFT(pi->pubpi->phy_rev)) |
				(enBtSignalsToFEMLut <<
				ACPHY_FemCtrl_enBtSignalsToFEMLut_SHIFT(pi->pubpi->phy_rev)));

			btcxi->btcx_femctrl_wlan_val = ((femctrlmask <<
				ACPHY_FemCtrl_femCtrlMask_SHIFT(pi->pubpi->phy_rev)) |
				(enBtSignalsToFEMLut <<
				ACPHY_FemCtrl_enBtSignalsToFEMLut_SHIFT(pi->pubpi->phy_rev)));

			if (pi->bt_shm_addr) {
				wlapi_bmac_write_shm(pi->sh->physhim,
					pi->bt_shm_addr + M_BTCX_RFSWMSK_BT_OFFSET(pi),
					btcxi->btcx_femctrl_bt_val);

				wlapi_bmac_write_shm(pi->sh->physhim,
					pi->bt_shm_addr + M_BTCX_RFSWMSK_WL_OFFSET(pi),
					btcxi->btcx_femctrl_wlan_val);
			}

			if (!suspend)
				wlapi_enable_mac(pi->sh->physhim);
		} else {
			MOD_PHYREG_2(pi, FemCtrl, enBtSignalsToFEMLut, enBtSignalsToFEMLut
				femCtrlMask, femctrlmask);
			if (pi->bt_shm_addr) {
				wlapi_bmac_write_shm(pi->sh->physhim,
					pi->bt_shm_addr + M_BTCX_RFSWMSK_BT_OFFSET(pi), 0);
				wlapi_bmac_write_shm(pi->sh->physhim,
					pi->bt_shm_addr + M_BTCX_RFSWMSK_WL_OFFSET(pi), 0);
			}
		}
	}
}
#endif	/* !WL_SISOCHIP && SWCTRL_TO_BT_IN_COEX */

static int
phy_ac_btcx_set_mode(phy_type_btcx_ctx_t *ctx, int btc_mode)
{
	phy_info_t *pi = ((phy_ac_btcx_info_t *)ctx)->pi;
	uint16 pktabortctl = phy_ac_noise_get_data(pi->u.pi_acphy->noisei)->pktabortctl;

	if (!pi->sh->clk || D11REV_LT(pi->sh->corerev, 47)) {
		return BCME_NOTREADY;
	}

	if (btc_mode == WL_BTC_DISABLE || btc_mode == WL_BTC_DEFAULT) {
		WRITE_PHYREG(pi, PktAbortCtrl, pktabortctl);
		pktabortctl = 0;
	}

	wlapi_bmac_write_shm(pi->sh->physhim, M_BTCX_PKTABORTCTL_VAL(pi), pktabortctl);
	return BCME_OK;
}

static int
phy_ac_btcx_set_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 set_val)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	if ((set_val < 0) || (set_val > 7)) {
		return BCME_RANGE;
	}
	if (SCAN_RM_IN_PROGRESS(btcxi->pi)) {
		return BCME_NOTREADY;
	}
#ifndef WLC_DISABLE_ACI
	if (btcxi->data->btc_mode != set_val) {
		wlapi_suspend_mac_and_wait(btcxi->pi->sh->physhim);
		phy_btcx_desense_btc(btcxi->pi, set_val);
		wlapi_enable_mac(btcxi->pi->sh->physhim);
	}
#endif /* !WLC_DISABLE_ACI */
	return BCME_OK;
}

static int
phy_ac_btcx_get_restage_rxgain(phy_type_btcx_ctx_t *ctx, int32 *ret_val)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	*ret_val = btcxi->data->btc_mode;
	return BCME_OK;
}

#ifdef WL_UCM

static void
phy_ac_btcx_register_impl_ucm(phy_type_btcx_fns_t *fns)
{
	fns->ucm_update_siso_resp_offset = phy_ac_btcx_ucm_update_siso_resp_offset;
	fns->ucm_update_only_siso_resp_offset = phy_ac_btcx_ucm_update_only_siso_resp_offset;
	fns->ucm_set_desense_rxgain = phy_ac_btcx_ucm_set_desense_rxgain;
	fns->ucm_get_desense_rxgain = phy_ac_btcx_ucm_get_desense_rxgain;
	fns->ucm_get_siso_ack_pwr = phy_ac_btcx_ucm_get_siso_ack_pwr;
	fns->ucm_get_curr_siso_resp_pwr_offset = phy_ac_btcx_ucm_get_curr_siso_resp_pwr_offset;
	fns->ucm_set_txpwrcaplmt = phy_ac_btcx_ucm_set_txpwrcaplmt;
	fns->ucm_set_txpwrcap_orig = phy_ac_btcx_ucm_set_txpwrcap_orig;
	return;
}

void
phy_ac_btcx_ucm_set_max_siso_resp_pwr(phy_ac_btcx_info_t *btcxi, int8 *siso_resp_pwr)
{
	phy_info_t *pi = btcxi->pi;
	memcpy(btcxi->max_siso_resp_pwr, siso_resp_pwr,
			PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain));
}

#define MIN_RSP_PWR	0
#define DEFAULT_OFFSET	0
#define MAX_TXPWRCTRL_TINY_OFFSET   63

static void
phy_ac_btcx_ucm_update_ack_pwr_offset(phy_ac_btcx_info_t *btcxi)
{
	phy_info_t *pi = btcxi->pi;
	uint8 resp_core = 0, curr_resp_mask, core;
	uint16 curr_resp_word = 0;
	int8 max_resp_pwr = 127;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
	uint8 core_mask = stf_shdata->phytxchain;
	int8 max_offset = MAX_TXPWRCTRL_OFFSET;

	/* Max. power may be unknown, so can't find offset */
	if (!(pi->sh->clk)) {
		ASSERT(0);
		return;
	}
	/* Update Offset to 0 for 5G or coex_abstxpwr_ison is not set */
	if (CHSPEC_IS5G(pi->radio_chanspec) ||
		(btcxi->coex_abstxpwr_ison != TRUE)) {
			btcxi->ack_pwr_offset = DEFAULT_OFFSET;
		goto finish;
	}

	/* Find response core from SISO Ack SHMEM */
	curr_resp_word = wlapi_bmac_read_shm(pi->sh->physhim, M_COREMASK_BTRESP(pi));
	curr_resp_mask = curr_resp_word & BTC_SISO_RESP_CORE_MASK;
	/* if coremask is not available dont do siso_ack */
	if (curr_resp_mask == 0)
		goto finish;
	/* Update Offset to 0 for 5G or not in HYBRID mode */
	if (CHSPEC_IS5G(pi->radio_chanspec) ||
			(wlapi_bmac_btc_mode_get(pi->sh->physhim) != WL_BTC_HYBRID)) {
		btcxi->ack_pwr_offset = DEFAULT_OFFSET;
		goto finish;
	}
	core_mask = (stf_shdata->phytxchain & curr_resp_mask &
			phy_stf_get_data(pi->stfi)->hw_phytxchain);
	/* assert if driver default cm and the ucm cm have no common active core */

	if (core_mask == 0) {
		PHY_ERROR(("%s: drv default cm & ucm cm have no cmn actv core\n", __FUNCTION__));
		ASSERT(0);
	}
	FOREACH_ACTV_CORE(pi, core_mask, core) {
		if (btcxi->max_siso_resp_pwr[core] < max_resp_pwr) {
			resp_core = core;
			max_resp_pwr = btcxi->max_siso_resp_pwr[core];
		}
	}

	/* Default value */
	btcxi->ack_pwr_offset = DEFAULT_OFFSET;

	if (btcxi->max_siso_resp_pwr[resp_core] < MIN_RSP_PWR) {
		max_resp_pwr = MIN_RSP_PWR;
	}
	/* Compute the offset only if a valid max response power is set */
	if ((pi->tx_power_max_per_core[resp_core] >> 1) >= max_resp_pwr) {
		btcxi->ack_pwr_offset =
			(int8)(pi->tx_power_max_per_core[resp_core] >> 1) - max_resp_pwr;
	}
	/* cap offset to max. to avoid overflows */
	if (TINY_RADIO(pi)) {
		/* 4349 and next version of the chips has pppr ofset */
		/* bitwidth got increased to S5.1 */
		max_offset = MAX_TXPWRCTRL_TINY_OFFSET;
	}
	if (btcxi->ack_pwr_offset > max_offset) {
		btcxi->ack_pwr_offset = (int8)max_offset;
	}

finish:
	/* Update shmem */
	wlapi_bmac_write_shm(pi->sh->physhim, M_COREMASK_BTRESP(pi),
			(btcxi->ack_pwr_offset << BTC_SISO_RESP_TXPWR_SHIFT) | core_mask);

	wlapi_bmac_write_shm(pi->sh->physhim, M_BTACK_2GCM_NUMB(pi),
		PHY_BITSCNT(core_mask)-1);
	PHY_INFORM(("BTC SISORESP Offset:%d & Mask:%d \n", btcxi->ack_pwr_offset, core_mask));
	return;
}

static int
phy_ac_btcx_ucm_update_siso_resp_offset(phy_type_btcx_ctx_t *ctx, int8 *siso_resp_pwr, uint8 len)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	if (len < PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain)) {
		return BCME_ERROR;
	}
	phy_ac_btcx_ucm_set_max_siso_resp_pwr(btcxi, siso_resp_pwr);
	/* Update SISO response offset */
	phy_ac_btcx_ucm_update_ack_pwr_offset(btcxi);
	return BCME_OK;
}

static void
phy_ac_btcx_ucm_update_only_siso_resp_offset(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_ac_btcx_ucm_update_ack_pwr_offset(btcxi);
}

static int
phy_ac_btcx_ucm_set_desense_rxgain(phy_type_btcx_ctx_t *ctx, uint8 *desense_val)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	return phy_ac_btcx_set_restage_rxgain(btcxi, desense_val[0]);
}

static int32
phy_ac_btcx_ucm_get_desense_rxgain(phy_type_btcx_ctx_t *ctx)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	return btcxi->data->btc_mode;
}

static void
phy_ac_btcx_ucm_get_siso_ack_pwr(phy_type_btcx_ctx_t *ctx, void *a)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	int8 *resp_pwrs = (int8 *)a;
	int32 *ret_int_ptr = (int32 *)a;
	uint8 idx;
	for (idx = 0; idx < PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain); idx++) {
		resp_pwrs[idx] = btcxi->max_siso_resp_pwr[idx];
	}
	*ret_int_ptr &= MAXBITVAL(PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain) *
			NBITS(*resp_pwrs));
}

static void
phy_ac_btcx_ucm_get_curr_siso_resp_pwr_offset(phy_type_btcx_ctx_t *ctx, void *a)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	int8 *resp_pwrs = (int8 *)a;
	int32 *ret_int_ptr = (int32 *)a;
	if (pi->sh->clk) {
		uint16 curr_resp_word = 0;
		uint8 curr_resp_mask;
		uint8 resp_core = 0, core;
		/* Find response core from SISO Ack SHMEM */
		curr_resp_word = wlapi_bmac_read_shm(pi->sh->physhim, M_COREMASK_BTRESP(pi));
		curr_resp_mask = curr_resp_word & BTC_SISO_RESP_CORE_MASK;
		FOREACH_CORE(pi, core) {
			if ((curr_resp_mask >> core) & 0x1) {
				resp_core = core;
				break;
			}
		}
		/* Max. power for response core on desired channel to 0.5 dB */
		resp_pwrs[0] = pi->tx_power_max_per_core[resp_core] >> 1;
		/* User-defined max. SISO response power */
		resp_pwrs[1] = btcxi->max_siso_resp_pwr[resp_core];
		/* Applied offset from shmem */
		resp_pwrs[2] = (curr_resp_word >> BTC_SISO_RESP_TXPWR_SHIFT) & 0xFF;
	}
	*ret_int_ptr &= MAXBITVAL(BTC_CURR_PWROFFSET_RET * NBITS(*resp_pwrs));
}

static int
phy_ac_btcx_ucm_set_txpwrcaplmt(phy_type_btcx_ctx_t *ctx, int8 *tx_power_cap_in_qdbm, bool active)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	uint8 core;
	int8 min_txpwrcap_btcx[PHY_CORE_MAX];
	if (!active) {
		btcxi->coex_abstxpwr_ison = FALSE;
		return BCME_OK;
	} else {
		btcxi->coex_abstxpwr_ison = TRUE;
	}
	if (!(pi->sh->clk)) {
		return BCME_NOCLK;
	}
	FOREACH_CORE(pi, core) {
		/* If UCM profile passes -128 indicating not to use corresponding core,
		 * host should indicate with txchain command.
		 * In phy we try to apply default TXPwrCap value for that particular core
		 */
		if (tx_power_cap_in_qdbm[core] == ACPHY_UCM_TXPWRCAP) {
			tx_power_cap_in_qdbm[core] = btcxi->txpwrcap_orig[core];
		}
		btcxi->btcx_txpwrcap[core] = tx_power_cap_in_qdbm[core];
		/* Take MIN of original & Btcoex txpwrcap values */
		min_txpwrcap_btcx[core] = MIN(btcxi->txpwrcap_orig[core],
				btcxi->btcx_txpwrcap[core]);
	}
	/* Program cap values in uCode shmems for supported cores */
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C0(pi),
		min_txpwrcap_btcx[0]);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C1(pi),
				min_txpwrcap_btcx[1]);
	}
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C2(pi),
				min_txpwrcap_btcx[2]);
	}
	return BCME_OK;
}

void
phy_ac_btcx_ucm_set_txpwrcap_orig(phy_type_btcx_ctx_t *ctx, int8 *txpwrcap)
{
	phy_ac_btcx_info_t *btcxi = (phy_ac_btcx_info_t *)ctx;
	phy_info_t *pi = btcxi->pi;
	uint8 core;
	int8 min_txpwrcap_btcx[PHY_CORE_MAX];
	// Copy cap values for comparing with Btcoex Profiles Txpwr cap values
	FOREACH_CORE(pi, core) {
		btcxi->txpwrcap_orig[core] = (txpwrcap[core] - pi->tx_pwr_backoff);
		min_txpwrcap_btcx[core] = MIN(btcxi->txpwrcap_orig[core],
		btcxi->btcx_txpwrcap[core]);
	}
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C0(pi),
		min_txpwrcap_btcx[0]);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 1)
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C1(pi),
		min_txpwrcap_btcx[1]);
	if (PHYCORENUM(pi->pubpi->phy_corenum) > 2)
	wlapi_bmac_write_shm(pi->sh->physhim, M_TXPWRCAP_BT_C2(pi),
		min_txpwrcap_btcx[2]);
}
#endif /* WL_UCM */
