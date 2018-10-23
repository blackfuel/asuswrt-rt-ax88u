/*
 * BlueToothCoExistence module implementation.
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
 * $Id: phy_btcx.c 691048 2017-03-20 16:47:17Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include <phy_type_btcx.h>
#include <phy_btcx_api.h>
#include <phy_utils_reg.h>
#include <bcmwifi_channels.h>

/* module private states */
struct phy_btcx_priv_info {
	phy_info_t *pi;
	phy_type_btcx_fns_t	*fns; /* PHY specific function ptrs */
	uint16	mhf1_cache;
};

/* module private states memory layout */
typedef struct {
	phy_btcx_info_t info;
	phy_type_btcx_fns_t fns;
	phy_btcx_priv_info_t priv;
	phy_btcx_data_t data;
/* add other variable size variables here at the end */
} phy_btcx_mem_t;

/* local function declaration */
static bool phy_btcx_wd(phy_wd_ctx_t *ctx);
static bool wlc_phy_btc_adjust(phy_wd_ctx_t *ctx);
static int phy_btcx_init(phy_init_ctx_t *ctx);

#define BTCX_WAIT_MAX_US 5000

/* UCM Support */
#ifdef WL_UCM
#if defined(WL_ENAB_RUNTIME_CHECK)
	#define UCM_ENAB(btcxi)   (phy_feature_enabled(btcxi->priv->pi, PHY_FEATURE_UCM_IDX))
#elif defined(WL_UCM_DISABLED)
	#define UCM_ENAB(btcxi)   0
#else
	#define UCM_ENAB(btcxi)   (phy_feature_enabled(btcxi->priv->pi, PHY_FEATURE_UCM_IDX))
#endif // endif
#else
	#define UCM_ENAB(btcxi)   0
#endif	/* WL_UCM */

/* attach/detach */
phy_btcx_info_t *
BCMATTACHFN(phy_btcx_attach)(phy_info_t *pi)
{
	phy_btcx_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_btcx_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->priv = &((phy_btcx_mem_t *)info)->priv;
	info->priv->pi = pi;
	info->priv->fns = &((phy_btcx_mem_t *)info)->fns;
	info->data = &((phy_btcx_mem_t *)info)->data;
#ifdef WL_UCM
	phy_set_feature_flag(pi, PHY_FEATURE_UCM_IDX, TRUE);
#endif /* WL_UCM */
	/* register watchdog fns */
	if (phy_wd_add_fn(pi->wdi, phy_btcx_wd, info,
	                  PHY_WD_PRD_FAST, PHY_WD_FAST_BTCX, PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	if (phy_wd_add_fn(pi->wdi, wlc_phy_btc_adjust, info, PHY_WD_PRD_1TICK,
	                  PHY_WD_1TICK_BTCX_ADJUST, PHY_WD_FLAG_DEF_SKIP) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_btcx_init, info, PHY_INIT_BTCX) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	return info;

	/* error */
fail:
	phy_btcx_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_btcx_detach)(phy_btcx_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null btcx module\n", __FUNCTION__));
		return;
	}

	pi = info->priv->pi;

	phy_mfree(pi, info, sizeof(phy_btcx_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_btcx_register_impl)(phy_btcx_info_t *cmn_info,
	phy_type_btcx_fns_t *fns)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	*cmn_info->priv->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_btcx_unregister_impl)(phy_btcx_info_t *cmn_info)
{
	PHY_CAL(("%s\n", __FUNCTION__));

	ASSERT(cmn_info);

	cmn_info->priv->fns = NULL;
}

/* watchdog callback */
static bool
phy_btcx_wd(phy_wd_ctx_t *ctx)
{
	phy_btcx_info_t *bi = (phy_btcx_info_t *)ctx;
	phy_info_t *pi = bi->priv->pi;

	phy_utils_phyreg_enter(pi);
	wlapi_update_bt_chanspec(pi->sh->physhim, pi->radio_chanspec,
	                         SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	phy_utils_phyreg_exit(pi);

	return TRUE;
}

/* Bluetooth Coexistence Initialization */
static int
WLBANDINITFN(phy_btcx_init)(phy_init_ctx_t *ctx)
{
	phy_btcx_info_t *btcxi = (phy_btcx_info_t *)ctx;
	phy_type_btcx_fns_t *fns = btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->init_btcx != NULL) {
		return (fns->init_btcx)(fns->ctx);
	}
	return BCME_OK;
}

bool
phy_btcx_is_btactive(phy_btcx_info_t *cmn_info)
{
	return cmn_info->data->bt_active;
}

#if defined(WLTEST)
int
phy_btcx_get_preemptstatus(phy_info_t *pi, int32* ret_ptr)
{
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_preemptstatus != NULL) {
		return (fns->get_preemptstatus)(fns->ctx, ret_ptr);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif // endif

#if !defined(WLC_DISABLE_ACI)
int
phy_btcx_desense_btc(phy_info_t *pi, int32 mode)
{
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->desense_btc != NULL) {
		return (fns->desense_btc)(fns->ctx, mode);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* !defined(WLC_DISABLE_ACI) */

/* adjust phy setting based on bt states */
static bool
wlc_phy_btc_adjust(phy_wd_ctx_t *ctx)
{
	bool btactive = FALSE;
	uint16 btperiod = 0;
	phy_btcx_info_t *bi = (phy_btcx_info_t *)ctx;
	phy_info_t *pi = bi->priv->pi;
	phy_type_btcx_fns_t *fns = bi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (!(wlapi_bmac_btc_mode_get(pi->sh->physhim)))
		return TRUE;

	wlapi_bmac_btc_period_get(pi->sh->physhim, &btperiod, &btactive);

	if (btactive != bi->data->bt_active) {
		if (fns->adjust != NULL) {
			(fns->adjust)(fns->ctx, btactive);
		} else {
			PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		}
	}

	bi->data->bt_period = btperiod;
	bi->data->bt_active = btactive;
	return TRUE;
}

void
phy_btcx_disable_arbiter(phy_btcx_info_t *bi)
{
	phy_info_t* pi = bi->priv->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));
	/* Enable manual BTCX mode */
	OR_REG(pi->sh->osh, D11_BTCX_CTL(pi),
		BTCX_CTRL_EN | BTCX_CTRL_SW);
	/* Reenable WLAN priority, and then wait for BT to finish */
	OR_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi), BTCX_TRANS_TXCONF);
	/* While RF_ACTIVE is asserted... */
	SPINWAIT(R_REG(pi->sh->osh, D11_BTCX_STAT(pi)) & BTCX_STAT_RA,
		BTCX_WAIT_MAX_US);

	if (R_REG(pi->sh->osh, D11_BTCX_STAT(pi)) & BTCX_STAT_RA) {
		PHY_INFORM(("wl%d: %s: BT still active ... overriding prisel\n",
				pi->sh->unit, __FUNCTION__));
	}
	OR_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
		BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL);

	/* Cache the MHF state and disable BT coex */
	bi->priv->mhf1_cache = wlapi_bmac_mhf_get(pi->sh->physhim, MHF1,
		WLC_BAND_2G) & (MHF1_WLAN_CRITICAL | MHF1_BTCOEXIST);
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL | MHF1_BTCOEXIST,
		MHF1_WLAN_CRITICAL, WLC_BAND_2G);
}

void
phy_btcx_enable_arbiter(phy_btcx_info_t *bi)
{
	phy_info_t* pi = bi->priv->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));
	/* Enable manual BTCX mode */
	OR_REG(pi->sh->osh, D11_BTCX_CTL(pi),
		BTCX_CTRL_EN | BTCX_CTRL_SW);
	/* Force BT priority */
	AND_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
		~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
	/* Enable BT coex */
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL | MHF1_BTCOEXIST,
		bi->priv->mhf1_cache, WLC_BAND_2G);
}

static void phy_btcx_override_enable_legacy(phy_info_t *pi)
{
	/* This is required only for 2G operation. No BTCX in 5G */
	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

		wlapi_coex_flush_a2dp_buffers(pi->sh->physhim);

		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi),
			BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force WLAN antenna and priority */
		OR_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL);
	}
}

void
wlc_btcx_override_enable(phy_info_t *pi)
{
	phy_btcx_info_t *btcxi = pi->btcxi;
	phy_type_btcx_fns_t *fns = btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->override_enable != NULL) {
		(fns->override_enable)(fns->ctx);
	} else {
		phy_btcx_override_enable_legacy(pi);
	}
}

static void phy_btcx_override_disable_legacy(phy_info_t *pi)
{
	if ((pi->sh->machwcap & MCAP_BTCX_SUP(pi->sh->corerev)) &&
		CHSPEC_IS2G(pi->radio_chanspec)) {
		/* Ucode better be suspended when we mess with BTCX regs directly */
		ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));
		/* Enable manual BTCX mode */
		OR_REG(pi->sh->osh, D11_BTCX_CTL(pi),
			BTCX_CTRL_EN | BTCX_CTRL_SW);
		/* Force BT priority */
		AND_REG(pi->sh->osh, D11_BTCX_TRANSCTL(pi),
			~(BTCX_TRANS_TXCONF | BTCX_TRANS_ANTSEL));
	}
}

void
wlc_phy_btcx_override_disable(phy_info_t *pi)
{
	phy_btcx_info_t *btcxi = pi->btcxi;
	phy_type_btcx_fns_t *fns = btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->override_disable != NULL) {
		(fns->override_disable)(fns->ctx);
	} else {
		phy_btcx_override_disable_legacy(pi);
	}
}

void
wlc_phy_set_femctrl_bt_wlan_ovrd(wlc_phy_t *pih, int8 state)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_femctrl != NULL) {
		(fns->set_femctrl)(fns->ctx, state, TRUE);
	}
}

int8
wlc_phy_get_femctrl_bt_wlan_ovrd(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;
	int8 state = AUTO;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_femctrl != NULL) {
		state = (fns->get_femctrl)(fns->ctx);
	}
	return state;
}

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
void wlc_phy_femctrl_mask_on_band_change(phy_info_t *pi)
{
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->femctrl_mask != NULL) {
		(fns->femctrl_mask)(fns->ctx);
	}
}
#endif // endif

void
wlc_phy_btcx_wlan_critical_enter(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, MHF1_WLAN_CRITICAL, WLC_BAND_2G);
}

void
wlc_phy_btcx_wlan_critical_exit(phy_info_t *pi)
{
	wlapi_bmac_mhf(pi->sh->physhim, MHF1, MHF1_WLAN_CRITICAL, 0, WLC_BAND_2G);
}

int
wlc_phy_iovar_set_btc_restage_rxgain(phy_btcx_info_t *btcxi, int32 set_val)
{
	phy_type_btcx_fns_t *fns = btcxi->priv->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->set_restage_rxgain != NULL) {
		return (fns->set_restage_rxgain)(fns->ctx, set_val);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
}

int
wlc_phy_iovar_get_btc_restage_rxgain(phy_btcx_info_t *btcxi, int32 *ret_val)
{
	phy_type_btcx_fns_t *fns = btcxi->priv->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->get_restage_rxgain != NULL) {
		return (fns->get_restage_rxgain)(fns->ctx, ret_val);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
}

int
phy_btcx_set_mode(wlc_phy_t *ppi, int btc_mode)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->mode_set != NULL) {
		return (fns->mode_set)(fns->ctx, btc_mode);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
}

#ifdef WL_UCM
int
phy_btcx_ucm_update_siso_resp_offset(wlc_phy_t *ppi, int8 *siso_resp_pwr, uint8 len)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	if (UCM_ENAB(pi->btcxi)) {
		phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;
		PHY_TRACE(("%s\n", __FUNCTION__));
		if (fns->ucm_update_siso_resp_offset != NULL) {
			return (fns->ucm_update_siso_resp_offset)(fns->ctx, siso_resp_pwr, len);
		} else {
			PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
			return BCME_UNSUPPORTED;
		}
	} else {
		return BCME_UNSUPPORTED;
	}
}

void
phy_btcx_ucm_update_only_ack_pwr_offset(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	if (UCM_ENAB(pi->btcxi)) {
		phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;
		PHY_TRACE(("%s\n", __FUNCTION__));
		if (fns->ucm_update_only_siso_resp_offset != NULL) {
			(fns->ucm_update_only_siso_resp_offset)(fns->ctx);
		}
	}
}

int
phy_btcx_ucm_set_desense_rxgain(wlc_phy_t *ppi, uint band, uint8 num_cores,
		uint8 *desense_array)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_btcx_info_t *btcxi = pi->btcxi;
	if (UCM_ENAB(btcxi)) {
		phy_type_btcx_fns_t *fns = btcxi->priv->fns;
		if (fns->ucm_set_desense_rxgain != NULL) {
			return (fns->ucm_set_desense_rxgain)(fns->ctx, desense_array);
		} else {
			PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
			return BCME_UNSUPPORTED;
		}
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_btcx_ucm_get_desense_rxgain(phy_btcx_info_t *btcxi,
		wl_desense_restage_gain_t *desense_restage_gain)
{
	if (UCM_ENAB(btcxi)) {
		phy_type_btcx_fns_t *fns = btcxi->priv->fns;
		phy_info_t *pi = btcxi->priv->pi;
		int32 btc_mode;
		uint8 idx;
		if (fns->ucm_get_desense_rxgain != NULL) {
			btc_mode = (fns->ucm_get_desense_rxgain)(fns->ctx);
			for (idx = 0; idx < PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain);
					idx++) {
				desense_restage_gain->desense_array[idx] = btc_mode;
			}
			desense_restage_gain->num_cores =
					PHY_BITSCNT(phy_stf_get_data(pi->stfi)->hw_phytxchain);
			desense_restage_gain->band = CHSPEC_IS2G(pi->radio_chanspec) ?
				WLC_BAND_2G : WLC_BAND_5G;
			return BCME_OK;
		} else {
			PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
			return BCME_UNSUPPORTED;
		}
	} else {
		return BCME_UNSUPPORTED;
	}
}

void
phy_btcx_ucm_get_siso_ack_pwr(phy_btcx_info_t *btcxi, void *a)
{
	if (UCM_ENAB(btcxi)) {
		phy_type_btcx_fns_t *fns = btcxi->priv->fns;
		if (fns->ucm_get_siso_ack_pwr) {
			(fns->ucm_get_siso_ack_pwr)(fns->ctx, a);
		}
	}
}

void
phy_btcx_ucm_get_curr_siso_resp_pwr_offset(phy_btcx_info_t *btcxi, void *a)
{
	if (UCM_ENAB(btcxi)) {
		phy_type_btcx_fns_t *fns = btcxi->priv->fns;
		if (fns->ucm_get_curr_siso_resp_pwr_offset) {
			(fns->ucm_get_curr_siso_resp_pwr_offset)(fns->ctx, a);
		}
	}
}

int phy_btcx_ucm_txpwrcaplmt(wlc_phy_t *ppi, int8 *tx_power_cap_in_qdbm, bool active)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	if (UCM_ENAB(pi->btcxi)) {
		phy_type_btcx_fns_t *fns = pi->btcxi->priv->fns;
		if (fns->ucm_set_txpwrcaplmt) {
			return (fns->ucm_set_txpwrcaplmt)(fns->ctx, tx_power_cap_in_qdbm, active);
		} else {
			return BCME_OK;
		}
	} else {
		return BCME_UNSUPPORTED;
	}
}

void phy_btcx_ucm_set_txpwrcap_orig(phy_btcx_info_t *btcxi, int8 *txpwrcap)
{
	if (UCM_ENAB(btcxi)) {
		phy_type_btcx_fns_t *fns = btcxi->priv->fns;
		if (fns->ucm_set_txpwrcap_orig) {
			(fns->ucm_set_txpwrcap_orig)(fns->ctx, txpwrcap);
		}
	}
}
#endif /* WL_UCM */
