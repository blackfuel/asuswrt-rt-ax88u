/*
 * ACPHY TSSI Cal module implementation
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
 * $Id: phy_ac_tssical.c 767605 2018-09-18 16:11:14Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_btcx.h>
#include "phy_type_tssical.h"
#include <phy_ac.h>
#include <phy_ac_tssical.h>
#include <phy_cache_api.h>
#include <phy_stf.h>
#include <phy_misc_api.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_radioreg_20694.h>
#include <wlc_radioreg_20695.h>
#include <wlc_radioreg_20696.h>
#include <wlc_radioreg_20697.h>
#include <wlc_radioreg_20698.h>
#include <wlc_radioreg_20704.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_ac_info.h>
#include <phy_calmgr.h>
#include <phy_rstr.h>
#include <phy_tpc.h>
#include <phy_utils_var.h>
#include <bcmdevs.h>
#include <wlc_phy_iovar.h>

#define OFDM_TBL 0
#define BPHY_TBL 1

/* module private states */

typedef struct phy_ac_tssical_config_info {
	/* low range tssi */
	bool srom_lowpowerrange2g;
	bool srom_lowpowerrange5g;
} phy_ac_tssical_config_info_t;

struct phy_ac_tssical_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_tssical_info_t *cmn_info;
	phy_ac_tssical_config_info_t *cfg;
	int16 idle_tssi[PHY_CORE_MAX];
};

typedef struct {
	phy_ac_tssical_info_t info;
	phy_ac_tssical_config_info_t cfg;
} phy_ac_tssical_mem_t;

/* local functions */
static int8 phy_ac_tssical_get_visible_thresh(phy_type_tssical_ctx_t *ctx);
static void wlc_phy_get_tssisens_min_acphy(phy_type_tssical_ctx_t *ctx, int8 *tssiSensMinPwr);
static void phy_ac_tssical_nvram_attach(phy_ac_tssical_info_t *ti);

#ifdef WLC_TXCAL
/* ******************************************* */
/* Functions used by common layer as callbacks */
/* ******************************************* */

static void phy_ac_tssical_compute_olpc_idx(phy_type_tssical_ctx_t *ctx);
static uint16 phy_ac_tssical_adjusted_tssi(phy_type_tssical_ctx_t *ctx, uint8 core_num);
static uint16 phy_ac_tssical_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx);
static void phy_ac_tssical_set_txpwrindex(phy_type_tssical_ctx_t *ctx, int16 gain_idx,
		uint16 save_TxPwrCtrlCmd);
static void phy_ac_tssical_restore_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx,
		uint16 save_TxPwrCtrlCmd, uint8 txpwr_ctrl_state);
static void phy_ac_tssical_set_olpc_anchor(phy_type_tssical_ctx_t *ctx);
static void phy_ac_tssical_iov_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx, int int_val);
static void phy_ac_tssical_read_est_pwr_lut(phy_type_tssical_ctx_t *ctx,
		void *output_buff, uint8 core);
static void phy_ac_tssical_apply_paparams(phy_type_tssical_ctx_t *ctx);
static void phy_ac_tssical_appy_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx,
		wl_txcal_power_tssi_t* txcal_pwr_tssi, uint8 bphy_tbl);
#endif /* WLC_TXCAL */

/* register phy type specific implementation */
phy_ac_tssical_info_t *
BCMATTACHFN(phy_ac_tssical_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_tssical_info_t *cmn_info)
{
	phy_ac_tssical_info_t *tssical_info;
	phy_type_tssical_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((tssical_info = phy_malloc(pi, sizeof(phy_ac_tssical_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	tssical_info->pi = pi;
	tssical_info->aci = aci;
	tssical_info->cmn_info = cmn_info;
	tssical_info->cfg = &(((phy_ac_tssical_mem_t *) tssical_info)->cfg);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.visible_thresh = phy_ac_tssical_get_visible_thresh;
	fns.sens_min = wlc_phy_get_tssisens_min_acphy;
#ifdef WLC_TXCAL
	fns.compute_olpc_idx = phy_ac_tssical_compute_olpc_idx;
	fns.adjusted_tssi = phy_ac_tssical_adjusted_tssi;
	fns.txpwr_ctrl_cmd = phy_ac_tssical_txpwr_ctrl_cmd;
	fns.set_txpwrindex = phy_ac_tssical_set_txpwrindex;
	fns.restore_txpwr_ctrl_cmd = phy_ac_tssical_restore_txpwr_ctrl_cmd;
	fns.set_olpc_anchor = phy_ac_tssical_set_olpc_anchor;
	fns.iov_apply_pwr_tssi_tbl = phy_ac_tssical_iov_apply_pwr_tssi_tbl;
	fns.read_est_pwr_lut = phy_ac_tssical_read_est_pwr_lut;
	fns.apply_paparams = phy_ac_tssical_apply_paparams;
	fns.apply_pwr_tssi_tbl = phy_ac_tssical_appy_pwr_tssi_tbl;
#endif /* WLC_TXCAL */
	fns.ctx = tssical_info;

	/* Read srom params from nvram */
	phy_ac_tssical_nvram_attach(tssical_info);

	if (phy_tssical_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_tssical_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return tssical_info;

	/* error handling */
fail:
	phy_ac_tssical_unregister_impl(tssical_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_tssical_unregister_impl)(phy_ac_tssical_info_t *tssical_info)
{
	if (tssical_info == NULL)
		return;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_tssical_unregister_impl(tssical_info->cmn_info);

	phy_mfree(tssical_info->pi, tssical_info, sizeof(phy_ac_tssical_mem_t));
}

/* ************************* */
/*		Internal Functions		*/
/* ************************* */
#ifdef NOT_YET
static void wlc_phy_scanroam_tssical_cal_acphy(phy_info_t *pi, bool set)
#endif // endif

#ifdef NOT_YET
static void
wlc_phy_scanroam_tssical_cal_acphy(phy_info_t *pi, bool set)
{
	uint16 ab_int[2];
	uint8 core;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));

	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	PHY_TRACE(("wl%d: %s: in scan/roam set %d\n", pi->sh->unit, __FUNCTION__, set));

	if (set) {
		PHY_CAL(("wl%d: %s: save the txcal for scan/roam\n",
			pi->sh->unit, __FUNCTION__));
		/* save the txcal to tssical */
		uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

		BCM_REFERENCE(phyrxchain);

		FOREACH_ACTV_CORE(pi, phyrxchain, core) {
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
				ab_int, TB_OFDM_COEFFS_AB, core);
			pi->u.pi_acphy->txcal_tssical[core].txa = ab_int[0];
			pi->u.pi_acphy->txcal_tssical[core].txb = ab_int[1];
			wlc_phy_cal_txiqlo_coeffs_acphy(pi, CAL_COEFF_READ,
			&pi->u.pi_acphy->txcal_tssical[core].txd,
				TB_OFDM_COEFFS_D, core);
			pi->u.pi_acphy->txcal_tssical[core].txei =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_FINE_I, core);
			pi->u.pi_acphy->txcal_tssical[core].txeq =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_FINE_Q, core);
			pi->u.pi_acphy->txcal_tssical[core].txfi =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_COARSE_I, core);
			pi->u.pi_acphy->txcal_tssical[core].txfq =
				(uint8)READ_RADIO_REGC(pi, RF, TXGM_LOFT_COARSE_Q, core);
			pi->u.pi_acphy->txcal_tssical[core].rxa =
				READ_PHYREGCE(pi, Core1RxIQCompA, core);
			pi->u.pi_acphy->txcal_tssical[core].rxb =
				READ_PHYREGCE(pi, Core1RxIQCompB, core);
			}

		/* mark the tssical as valid */
		pi->u.pi_acphy->txcal_tssical_cookie = TXCAL_tssical_VALID;
	} else {
		if (pi->u.pi_acphy->txcal_tssical_cookie == TXCAL_tssical_VALID) {
			PHY_CAL(("wl%d: %s: restore the txcal after scan/roam\n",
				pi->sh->unit, __FUNCTION__));
			/* restore the txcal from tssical */
			wlc_phy_cal_coeffs_upd(pi, pi->u.pi_acphy->txcal_tssical);
			/* This function has been split into two:
			 * wlc_phy_txcal_coeffs_upd and wlc_phy_rxcal_coeffs_upd
			 */
		}
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* NOT_YET */

static void
BCMATTACHFN(phy_ac_tssical_nvram_attach)(phy_ac_tssical_info_t *ti)
{
	ti->cfg->srom_lowpowerrange2g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(ti->pi,
		rstr_lowpowerrange2g, FALSE);
	ti->cfg->srom_lowpowerrange5g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(ti->pi,
		rstr_lowpowerrange5g, FALSE);
	return;
}
/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* measure idle TSSI by sending 0-magnitude tone */
void
wlc_phy_txpwrctrl_idle_tssi_meas_acphy(phy_info_t *pi)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)pi->u.pi_acphy->tssicali;
	uint8  core;
	int16  idle_tssi[PHY_CORE_MAX] = {0};
	uint16 orig_RfseqCoreActv2059, orig_RxSdFeConfig6 = 0;
	bool suspend = TRUE;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
	uint16 hwaci_state = 0;

	BCM_REFERENCE(stf_shdata);

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		/* Let WLAN have FEMCTRL to ensure cal is done properly */
		suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
		if (!suspend) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		}
		wlc_btcx_override_enable(pi);
	}

	if ((SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) || PHY_MUTED(pi)) && !TINY_RADIO(pi))
		/* skip idle tssi cal */
		return;

#ifdef ATE_BUILD
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		printf("skipping idle tssi cal \n");
		return;
	} else {
		printf("===> Running Idle TSSI cal\n");
	}
#endif /* ATE_BUILD */

	/* prevent crs trigger */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 1);
	}

	/* Force dcoe compensation to be 0 to prevent it from applying during idletssi cal */
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47(pi->pubpi->phy_rev)) {
		FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
			if (core == 0)
				MOD_PHYREG(pi, dccal_control_140, dcoe_force_zero_ovrride, 1);
			else if (core == 1)
				MOD_PHYREG(pi, dccal_control_141, dcoe_force_zero_ovrride, 1);
			if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				if (core == 2)
				MOD_PHYREG(pi, dccal_control_142, dcoe_force_zero_ovrride, 1);
				else if (core == 3)
				MOD_PHYREG(pi, dccal_control_143, dcoe_force_zero_ovrride, 1);
			}
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* Disable hwaci mitigation */
		hwaci_state = READ_PHYREG(pi, ACI_Mitigation_CTRL);
		MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1);
		MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_switching_disable, 1);
	}

	/* we should not need this but just in case */
	phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_TSSICAL);

	if (TINY_RADIO(pi) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
		orig_RxSdFeConfig6 = READ_PHYREG(pi, RxSdFeConfig6);
		MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0,
			READ_PHYREGFLD(pi, RxSdFeConfig1, farrow_rshift_tx));
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* Enable hwaci mitigation */
		WRITE_PHYREG(pi, ACI_Mitigation_CTRL, hwaci_state);
	}

	// Additional settings for 4365 core3 GPIO WAR
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     1);
		MOD_PHYREG(pi, RfctrlCoreRXGAIN13,   rxgain_dvga,  0);
		MOD_PHYREG(pi, RfctrlCoreLpfGain3,   lpf_bq2_gain, 0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, rxgain,       1);
		MOD_PHYREG(pi, RfctrlOverrideGains3, lpf_bq2_gain, 1);
	}

	//43684 doesn't apply below setting in tcl
	if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		/* Must always be on or tssi capture will fail */
		MOD_PHYREG(pi, TssiAccumCtrl, tssi_accum_en, 1);
		MOD_PHYREG(pi, TssiAccumCtrl, tssi_filter_pos, 1);
	}

	/* force all TX cores on */
	orig_RfseqCoreActv2059 = READ_PHYREG(pi, RfseqCoreActv2059);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx,  stf_shdata->hw_phyrxchain);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, stf_shdata->hw_phyrxchain);

	FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
		wlc_phy_poll_samps_WAR_acphy(pi, idle_tssi, TRUE, TRUE, NULL,
		                             FALSE, TRUE, core, 0);
		tssicali->idle_tssi[core] = idle_tssi[core];
		wlc_phy_txpwrctrl_set_idle_tssi_acphy(pi, idle_tssi[core], core);
		PHY_TRACE(("wl%d: %s: idle_tssi core%d: %d\n",
		           pi->sh->unit, __FUNCTION__, core, tssicali->idle_tssi[core]));
	}

	WRITE_PHYREG(pi, RfseqCoreActv2059, orig_RfseqCoreActv2059);

	if (TINY_RADIO(pi) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0);
		WRITE_PHYREG(pi, RxSdFeConfig6, orig_RxSdFeConfig6);
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		wlc_phy_btcx_override_disable(pi);
		if (!suspend) {
			wlapi_enable_mac(pi->sh->physhim);
		}
	}

	// Additional settings for 4365 core3 GPIO WAR
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, DcFiltAddress,        dcBypass,     0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, rxgain,       0);
		MOD_PHYREG(pi, RfctrlOverrideGains3, lpf_bq2_gain, 0);
	}

	/* Unsetting dcoe override */
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) || ACMAJORREV_47(pi->pubpi->phy_rev)) {
		FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
			if (core == 0)
				MOD_PHYREG(pi, dccal_control_140, dcoe_force_zero_ovrride, 0);
			else if (core == 1)
				MOD_PHYREG(pi, dccal_control_141, dcoe_force_zero_ovrride, 0);
			if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				if (core == 2)
				MOD_PHYREG(pi, dccal_control_142, dcoe_force_zero_ovrride, 0);
				else if (core == 3)
				MOD_PHYREG(pi, dccal_control_143, dcoe_force_zero_ovrride, 0);
			}
		}
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 0);
	}
	/* prevent crs trigger */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

#ifdef ATE_BUILD
	printf("===> Finished Idle TSSI cal\n");
#endif /* ATE_BUILD */

}

void
wlc_phy_tssi_phy_setup_acphy(phy_info_t *pi, uint8 for_iqcal)
{
	uint8 core;
	phy_ac_tssical_info_t *ti = pi->u.pi_acphy->tssicali;
	uint sicoreunit;
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi2g(pi->tpci)) || (CHSPEC_IS5G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi5g(pi->tpci))) && PHY_IPA(pi);
	bool flaglowrangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange5g)) &&
		PHY_IPA(pi);
	uint8 phyrxchain;

	BCM_REFERENCE(phyrxchain);

	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);
	phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		if (ACREV_IS(pi->pubpi->phy_rev, 4) && CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 0);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_ac_set_tssi_params_maj36(pi);
			/* Select Q rail for TSSI on 43012A0, since IQ swap is enabled on RX */
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 1);
			MOD_PHYREG(pi, TSSIMode, tssiPosSlope, 1);
			MOD_PHYREG(pi, TSSIMode, tssiEn, 1);
		} else {
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 1);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_GE40(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, TSSIMode, tssiADCSel, 0);
		}
		if (TINY_RADIO(pi)) {
			if (!PHY_IPA(pi)) {
				if (ACREV_IS(pi->pubpi->phy_rev, 4)) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
				} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
					ACMAJORREV_33(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);
				} else {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x2);
				}
			} else {
				if (CHSPEC_IS2G(pi->radio_chanspec))
					if (pi->sromi->txpwr2gAdcScale != -1)
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							pi->sromi->txpwr2gAdcScale);
					else if (ACMAJORREV_4(pi->pubpi->phy_rev))
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							0x1);
					else
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							0x0);
				else
					if (pi->sromi->txpwr5gAdcScale != -1)
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							pi->sromi->txpwr5gAdcScale);
					else if (ACMAJORREV_4(pi->pubpi->phy_rev))
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							0x1);
					else
						MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx,
							0x0);
			}
		}
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev) ||
		    !ACMAJORREV_GE40(pi->pubpi->phy_rev)) {
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				/* PingPongComp.sdadcloopback_sat = 0 would drop one lsb
				 * bit 1 would saturate 1 msb bit.
				 * Setting this to 0 to drop 1 bit.
				 */
				MOD_PHYREG(pi, PingPongComp, sdadcloopback_sat, 0x0);
				MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x1);

				if (for_iqcal == 1) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0x1);
					MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0x2);
				}
			} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			           ACMAJORREV_33(pi->pubpi->phy_rev) ||
			           ACMAJORREV_37(pi->pubpi->phy_rev) ||
			           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, PingPongComp, sdadcloopback_sat, 0x0);
				MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
				if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0x1);
				}
			}
		}

		if (!(ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_40(pi->pubpi->phy_rev))) {
			MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, tssi_pu, 1);
		}

		if (!PHY_IPA(pi) && (!for_iqcal)) {
			MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, tssi_pu, for_iqcal);
			if (ACMAJORREV_47_51((pi->pubpi->phy_rev))) {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core, tssi_pu, for_iqcal);
			}
		} else {
			if (!(ACMAJORREV_4(pi->pubpi->phy_rev) ||
				ACMAJORREV_40(pi->pubpi->phy_rev))) {
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core, tssi_pu, 1);
			}
			if (for_iqcal) {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core,
					tssi_range, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,     core,
					tssi_range, 1);
			} else if (flag2rangeon && (flaglowrangeon == 0)) {
				if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev) &&
						PHY_IPA(pi) && (sicoreunit == DUALMAC_AUX)) {
					if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
						MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
							tssi_range, 0);
					} else {
						MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
							tssi_range, 1);
						MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core,
							tssi_range, 1);
						MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
							tssi_range, 0);
					}
				} else {
					MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
						tssi_range, 0);
					MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core,
						tssi_range, 0);
				}
			} else if (flaglowrangeon) {
				if (ACMAJORREV_44(pi->pubpi->phy_rev) &&
					PHY_IPA(pi) && (sicoreunit == DUALMAC_AUX)) {
					MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
							tssi_range, 1);
					if (flag2rangeon == 1) {
						MOD_PHYREGCE(pi, RfctrlCoreAuxTssi3, core,
							tssi_range, 1);
					} else {
						MOD_PHYREGCE(pi, RfctrlCoreAuxTssi3, core,
							tssi_range, 5);
					}
				} else {
						MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
							tssi_range, 1);
						MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core,
							tssi_range, 0);
				}

			} else {
				MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi, core,
					tssi_range, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1, core,
					tssi_range, ~(for_iqcal));
			}
		}
	}
}

void
wlc_phy_tssi_radio_setup_acphy_20694(phy_info_t *pi, uint8 for_iqcal)
{
	/* save radio config before changing it */
	// phy_ac_reg_cache_save(ti->aci, RADIOREGS_TSSI);
	uint8 core;
	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	phy_ac_tssical_info_t *ti = pi->u.pi_acphy->tssicali;
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi2g(pi->tpci)) || (CHSPEC_IS5G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi5g(pi->tpci))) && PHY_IPA(pi);
	bool flaglowrangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange5g)) &&
		PHY_IPA(pi);

	ASSERT(for_iqcal == 0);	/* 4347A0 doesn't support 2G/5G iPA tssi for txiqcal gctrl */
	BCM_REFERENCE(for_iqcal);
	BCM_REFERENCE(phyrxchain);

	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		MOD_RADIO_REG_20694(pi, RF, IQCAL_OVR1, core,
			ovr_iqcal_PU_tssi, 1);
		if (PHY_IPA(pi)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {

				RADIO_REG_LIST_START
					MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG1, core,
					iqcal_sel_sw, 0x0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, TX2G_CFG1_OVR, core,
					ovr_pa2g_tssi_ctrl_sel, 0x1)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, PA2G_CFG1, core,
					pa2g_tssi_ctrl_sel, 0x0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, TX2G_CFG1_OVR, core,
					ovr_pa2g_tssi_ctrl_pu, 0x1)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, TX2G_MISC_CFG1, core,
					pa2g_tssi_ctrl_pu, 0x1)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG1, core,
					iqcal_sel_ext_tssi, 0x0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG1, core,
					iqcal_PU_tssi, 0x1)
				RADIO_REG_LIST_EXECUTE(pi, core);

				if (flag2rangeon || flaglowrangeon) {
					if (flaglowrangeon) {
						MOD_RADIO_REG_20694(pi, RF, TX2G_CFG1_OVR, core,
							ovr_pa2g_tssi_ctrl_range, 0x1);
						MOD_RADIO_REG_20694(pi, RF, TX2G_MISC_CFG1, core,
							pa2g_tssi_ctrl_range, 0x0);
					} else {
						MOD_RADIO_REG_20694(pi, RF, TX2G_CFG1_OVR, core,
							ovr_pa2g_tssi_ctrl_range, 0x0);
					}
				} else {
					/* Turning on the attenuator for 4361 to
					 * solve TSSI saturation at high power
					 */
					if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
						MOD_RADIO_REG_20694(pi, RF, TX2G_CFG1_OVR, core,
							ovr_pa2g_tssi_ctrl_range, 0x1);
						MOD_RADIO_REG_20694(pi, RF, TX2G_MISC_CFG1, core,
							pa2g_tssi_ctrl_range, 0x1);
					}
				}

			} else {
				PHY_ERROR(("FIXME: 4347 doesn't suppot 5G iPA tssi yet\n"));
				return;
			}
		} else {

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
			} else {
				MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG1, core, iqcal_sel_sw, 0x1);
			}
			MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1);
			MOD_RADIO_REG_20694(pi, RF, IQCAL_CFG1, core, iqcal_PU_tssi, 0x0);
			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				/* 0: Bypass
				 * 1: 200k
				 * 2: 300k
				 * 3: 400k
				 * 4: 500k
				 * 5: 600k
				 * 6: 700k
				 * 7: 800k
				 */
				MOD_RADIO_REG_20694(pi, RF, TX_TOP_SPARE3, core, TX_spare3, 0x1);
			}
		}
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20694_ENTRY(pi, RF, LPF_REG7, core, lpf_sw_bq2_adc, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, LPF_REG7, core, lpf_sw_bq1_adc, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, LPF_REG7, core, lpf_sw_aux_adc, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, LPF_REG8, core, lpf_sw_adc_test, 0x0)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_OVR1, core, ovr_iqcal_PU_iqcal, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG1, core,
				iqcal_tssi_GPIO_ctrl, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG2, core,
				iqcal_tssi_cm_center, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_IDAC, core, iqcal_tssi_bias, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_GAIN_RFB, core, iqcal_rfb, 0x200)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_GAIN_RIN, core, iqcal_rin, 0x800)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_OVR1, core, ovr_iqcal_PU_wbpga, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG5, core, wbpga_pu, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_OVR1, core,
				ovr_iqcal_PU_loopback_bias, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG5, core, loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG5, core, wbpga_cmref_iqbuf, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG5, core, wbpga_cmref_half, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG6, core, wbpga_cmref, 0x14)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG6, core, wbpga_bias, 0x14)

			/* Selection between auxpga or iqcal(wbpga) */
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG4, core, iqcal2adc, 0x0)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, IQCAL_CFG4, core, auxpga2adc, 0x1)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, TESTBUF_OVR1, core, ovr_testbuf_PU, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, TESTBUF_CFG1, core, testbuf_PU, 0x1)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, TESTBUF_OVR1, core,
				ovr_testbuf_sel_test_port, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, TESTBUF_CFG1, core,
				testbuf_sel_test_port, 0x2)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, AUXPGA_CFG1, core, auxpga_i_pu, 0x1)

			MOD_RADIO_REG_20694_ENTRY(pi, RF, AUXPGA_OVR1, core,
				ovr_auxpga_i_sel_input, 0x1)
			MOD_RADIO_REG_20694_ENTRY(pi, RF, AUXPGA_CFG1, core,
				auxpga_i_sel_input, 0x2)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}
}
void
wlc_phy_tssi_radio_setup_acphy_20697(phy_info_t *pi)
{
	/* save radio config before changing it */
	// phy_ac_reg_cache_save(ti->aci, RADIOREGS_TSSI);
	uint8 core;
	uint8 ovrd, ovrd_mask;
	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

	BCM_REFERENCE(phyrxchain);

	ovrd = 1; /* 0: PHY direct control(default), 1: Radio override */
	ovrd_mask = 0;
	if (ovrd)
		ovrd_mask = 0xf;

	/* Clear GPAIOs */
	/* Powerup gpaio block, powerdown rcal, clear all test point selection */
	if (pi->pubpi->slice == DUALMAC_AUX) {
		MOD_RADIO_REG_20697X(pi, RFP, RCAL_CFG_NORTH, 1, 0, rcal_pu, 0x0);
	} else {
		MOD_RADIO_REG_20697X(pi, RFP, RCAL_CFG_NORTH, 0, 1, rcal_pu, 0x0);
	}
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL2, core, gpaio_pu, 0x1);
		MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL0, core, gpaio_sel_0to15_port, 0x0);
		MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL1, core, gpaio_sel_16to31_port, 0x0);
		MOD_RADIO_REG_20697(pi, RF, GPAIO_SEL3, core, gpaio_sel_32to47_port, 0x0);
		/* PHY Direct_control: */
		MOD_RADIO_REG_20697(pi, RF, IQCAL_OVR1, core, ovr_iqcal_PU_loopback_bias, ovrd);
		MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, loopback_bias_pu, 0);

		MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, iqcal_tssi_GPIO_ctrl, 0x0);
		if (pi->pubpi->slice == DUALMAC_AUX) {
			MOD_RADIO_REG_20697X(pi, RF, PA2G_CFG1, 1, core, pa2g_gpio_sw_pu, 0x0);
		} else {
			MOD_RADIO_REG_20697X(pi, RF, PA5G_CFG1, 0, core, pa5g_gpio_sw_pu, 0x0);
			MOD_RADIO_REG_20697X(pi, RF, TX5G_CFG1, 0, core, pad5g_gpio_sw_pu, 0x0);
		}
	}
	/* Radio Configuration */
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		if (pi->pubpi->slice == DUALMAC_AUX) {
			if (PHY_IPA(pi)) {
				//IPA..Choosing the iTSSI path
				/*
				*TX2G_CFG2_OVR.ovr_pad2g_tssi_ctrl_sel = 0x1;
				*TX2G_CFG2_OVR.ovr_pad2g_tssi_ctrl_range =  0x0;
				*/
				WRITE_RADIO_REG_20697X(pi, RF, TX2G_CFG2_OVR, 1, core, 0x20);
				/*
				*PAD2G_CFG11.pad2g_tssi_ctrl_sel =  0x0;
				*PAD2G_CFG11.pad2g_tssi_ctrl_pu = 0x1;
				*PAD2G_CFG11.pad2g_tssi_ctrl_range = 0x5;
				*PAD2G_CFG11.pad2g_tssi_ctrl_pu_att_bias =  0x1
				pad2g_bias_pu
				*/
				WRITE_RADIO_REG_20697X(pi, RF, PAD2G_CFG11, 1, core, 0x0A);
				MOD_RADIO_REG_20697X(pi, RF, PA2G_CFG14, 1, core, pad2g_tssi_ctrl,
					0x0);
				/*
				*TX2G_CFG1_OVR.ovr_pad2g_tssi_ctrl_pu = 0x1;
				*TX2G_CFG1_OVR.ovr_pad2g_2gtx_pu = 0x0;
				*/

				WRITE_RADIO_REG_20697X(pi, RF, TX2G_CFG1_OVR, 1, core, 0x800);
				/*
				*IQCAL_CFG1.iqcal_sel_sw = 0x0;
				*IQCAL_CFG1.iqcal_sel_ext_tssi =  0x0;
				*/
				WRITE_RADIO_REG_20697X(pi, RF, IQCAL_CFG1, 1, core, 0x0);

			} else {
				//EPA: Choosing the eTSSI path
				// IQCAL_CFG1, core, iqcal_sel_sw, 5
				// IQCAL_CFG1, core, iqcal_sel_ext_tssi, 1
				phy_utils_mod_radioreg(pi,
					RADIO_REG_20697(pi, RF, IQCAL_CFG1, core),
					0xF8, 0xA8);

				MOD_RADIO_REG_20697X(pi, RF, IQCAL_TSSI_CFG1, 1, core,
					iqcal_eTSSI_LPF_bw_ctrl, 0x5);
				MOD_RADIO_REG_20697X(pi, RF, TX2G_CFG1_OVR, 1, core,
					ovr_pad2g_tssi_ctrl_pu, 0x1);
				MOD_RADIO_REG_20697X(pi, RF, PAD2G_CFG11, 1, core,
					pad2g_tssi_ctrl_pu, 0x0);
				MOD_RADIO_REG_20697X(pi, RF, GPAIO_SEL3, 1, core,
					gpaio_sel_32to47_port, 0x20);
			}
			// LPF_NOTCH_REG6, core, lpf_notch_sel_2g_out_gm, 1
			// LPF_NOTCH_REG6, core, lpf_notch_sel_5g_out_gm, 0
			phy_utils_mod_radioreg(pi,
				RADIO_REG_20697(pi, RF, LPF_NOTCH_REG6, core), 0x3, 0x2);
		} else {
			//ePA and eTSSI only
			// MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, iqcal_sel_sw, 7);
			// MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 1);
			phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, IQCAL_CFG1, core),
				0xF8, 0xB8);

			MOD_RADIO_REG_20697X(pi, RF, IQCAL_TSSI_CFG1, 0, core,
				iqcal_eTSSI_LPF_bw_ctrl, 5);
			MOD_RADIO_REG_20697X(pi, RF, TX5G_CFG2_OVR, 0, core,
				ovr_pa5g_tssi_ctrl_pu, 1);
			MOD_RADIO_REG_20697X(pi, RF, TX5G_MISC_CFG1, 0, core,
				pa5g_tssi_ctrl_pu, 0);
			MOD_RADIO_REG_20697X(pi, RF, GPAIO_SEL3, 0, core,
				gpaio_sel_32to47_port, 0x40);
			MOD_RADIO_REG_20697X(pi, RF, LPF_NOTCH_REG6, 0, core,
				lpf_notch_sel_5g_out_gm, 1);

			/* In Main slice , route AUx PGA o/p to ADC instead of WBPGA */
			/* Enable AuxPGA out to ADC in */
			// MOD_RADIO_REG_20697X(pi, RF, IQCAL_CFG4, 0, core, auxpga2adc, 1);
			/* Disable WBPGA out to ADC in */
			// MOD_RADIO_REG_20697X(pi, RF, IQCAL_CFG4, 0, core, iqcal2adc, 0);
			phy_utils_mod_radioreg(pi,
				RADIO_REG_20697(pi, RF, IQCAL_CFG4, core), 0x3, 0x1);
		}
		/* power down iqcal stage */
		// MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, iqcal_PU_iqcal, 0);
		/* power up bias gen ckt */
		//MOD_RADIO_REG_20697(pi, RF, IQCAL_CFG1, core, loopback_bias_pu, 1);
		/* PHY Direct Control */
		phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, IQCAL_CFG1, core), 0x101, 0x1);
		MOD_RADIO_REG_20697(pi, RF, IQCAL_OVR1, core, ovr_iqcal_PU_iqcal, ovrd);

		/* powerup testbuf */
		// MOD_RADIO_REG_20697(pi, RF, TESTBUF_CFG1, core, testbuf_PU, 1);
		// MOD_RADIO_REG_20697(pi, RF, TESTBUF_OVR1, core, ovr_testbuf_PU, 1);
		/* Select TSSI signal */
		// MOD_RADIO_REG_20697(pi, RF, TESTBUF_CFG1, core, testbuf_sel_test_port, 2);
		// MOD_RADIO_REG_20697(pi, RF, TESTBUF_OVR1, core, ovr_testbuf_sel_test_port, 1);
		/* PHY Direct Control */
		phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, TESTBUF_CFG1, core),
			0x71, 0x21);
		phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, TESTBUF_OVR1, core),
			0x3, (0x3 & ovrd_mask));

		/* power up aux pga */
		// MOD_RADIO_REG_20697(pi, RF, AUXPGA_CFG1, core, auxpga_i_pu, 1);
		// MOD_RADIO_REG_20697(pi, RF, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 1);

		/* AuxPGA input mode set to TSSI */
		// MOD_RADIO_REG_20697(pi, RF, AUXPGA_CFG1, core, auxpga_i_sel_input, 2);
		// MOD_RADIO_REG_20697(pi, RF, AUXPGA_OVR1, core, ovr_auxpga_i_sel_input, 1);
		/* PHY Direct Control */
		phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, AUXPGA_CFG1, core), 0xF, 0x5);
		phy_utils_mod_radioreg(pi, RADIO_REG_20697(pi, RF, AUXPGA_OVR1, core),
			0xa, (0xa & ovrd_mask));
	}
}

void
wlc_phy_tssi_radio_setup_acphy_28nm(phy_info_t *pi, uint8 for_iqcal)
{

	if (PHY_IPA(pi) || for_iqcal == 1) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
		RADIO_REG_LIST_START
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG1, 0, iqcal_sel_sw, 0x0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX2G_CFG2_OVR, 0, ovr_pad2g_tssi_ctrl_sel, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX2G_CFG1_OVR, 0, ovr_pad2g_tssi_ctrl_pu, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD2G_CFG11, 0, pad2g_tssi_ctrl_pu, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX2G_CFG2_OVR, 0, ovr_pad2g_tssi_ctrl_range, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD2G_CFG11, 0, pad2g_tssi_ctrl_range, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD2G_CFG11, 0,       pad2g_wrssi0_en, 0x0)
		RADIO_REG_LIST_EXECUTE(pi, 0);
		MOD_RADIO_REG_28NM(pi, RF, PAD2G_CFG11, 0, pad2g_tssi_ctrl_sel, for_iqcal);
		if (for_iqcal == 1) {
			MOD_RADIO_REG_28NM(pi, RF, PA2G_CFG1, 0, pa2g_pu, 0);
			MOD_RADIO_REG_28NM(pi, RF, TX2G_CFG1_OVR, 0, ovr_pa2g_pu, 1);
		}
		} else {
		RADIO_REG_LIST_START
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG1, 0, iqcal_sel_sw, 0x2)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX5G_CFG2_OVR, 0, ovr_pad5g_tssi_ctrl_sel, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX5G_CFG2_OVR, 0, ovr_pad5g_tssi_ctrl_pu, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD5G_CFG14, 0, pad5g_tssi_ctrl_pu, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TX5G_CFG2_OVR, 0, ovr_pad5g_tssi_ctrl_range, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD5G_CFG14, 0, pad5g_tssi_ctrl_range, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, PAD5G_CFG14, 0,     pad5g_wrssi0_en, 0x0)
		RADIO_REG_LIST_EXECUTE(pi, 0);
		MOD_RADIO_REG_28NM(pi, RF, PAD5G_CFG14, 0, pad5g_tssi_ctrl_sel, for_iqcal);
		if (for_iqcal == 1) {
			MOD_RADIO_REG_28NM(pi, RF, PA5G_CFG4, 0, pa5g_pu, 0);
			MOD_RADIO_REG_28NM(pi, RF, TX5G_CFG1_OVR, 0, ovr_pa5g_pu, 1);
		}
		}

		MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_sel_ext_tssi, 0x0);
		MOD_RADIO_REG_28NM(pi, RF, IQCAL_OVR1, 0, ovr_iqcal_PU_tssi, 1);
		MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_PU_tssi, 0x1);
	} else {

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_sel_sw, 0x3); /* 0xf */
		} else {
			MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_sel_sw, 0x1); /* 0xd */
		}
		MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_sel_ext_tssi, 0x1);
		MOD_RADIO_REG_28NM(pi, RF, IQCAL_OVR1, 0, ovr_iqcal_PU_tssi, 1);
		MOD_RADIO_REG_28NM(pi, RF, IQCAL_CFG1, 0, iqcal_PU_tssi, 0x0);
	}

	RADIO_REG_LIST_START
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_OVR1, 0, ovr_iqcal_PU_iqcal, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG1, 0, iqcal_PU_iqcal, 0x0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG1, 0, iqcal_tssi_GPIO_ctrl,	0x0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG2, 0, iqcal_tssi_cm_center,	0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_IDAC, 0, iqcal_tssi_bias, 0x0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_OVR1, 0, ovr_iqcal_PU_loopback_bias, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, IQCAL_CFG5, 0, loopback_bias_pu, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TESTBUF_OVR1, 0, ovr_testbuf_sel_test_port, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TESTBUF_CFG1, 0, testbuf_sel_test_port, 0x2)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TESTBUF_OVR1, 0, ovr_testbuf_PU, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, TESTBUF_CFG1, 0, testbuf_PU, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AUXPGA_OVR1, 0, ovr_auxpga_i_pu, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AUXPGA_CFG1, 0, auxpga_i_pu, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AUXPGA_OVR1, 0, ovr_auxpga_i_sel_input, 0x1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AUXPGA_CFG1, 0, auxpga_i_sel_input, 0x2)
	RADIO_REG_LIST_EXECUTE(pi, 0);

}

void
wlc_phy_tssi_radio_setup_acphy_tiny(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal)
{
	uint8 core, pupd = 0;
	phy_ac_tssical_info_t *ti = pi->u.pi_acphy->tssicali;
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi2g(pi->tpci)) || (CHSPEC_IS5G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi5g(pi->tpci))) && PHY_IPA(pi);
	bool flaglowrangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange2g) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && ti->cfg->srom_lowpowerrange5g)) &&
		PHY_IPA(pi);
	ASSERT(TINY_RADIO(pi));

	if (BF3_TSSI_DIV_WAR(pi->u.pi_acphy) && ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* DIV_WAR is priority between DIV WAR & two range */
		flag2rangeon = 0;
	}

	/* 20691_tssi_radio_setup */
	/* # Powerdown rcal otherwise it won't let any other test point go through */
	if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID) ||
		(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) && RADIOMAJORREV(pi) == 3)) {
		/* 20691_gpaio # Powerup gpaio block, powerdown rcal,
		 * clear all test point selection
		 */
		RADIO_REG_LIST_START
			MOD_RADIO_REG_TINY_ENTRY(pi, GPAIO_SEL2, 0, gpaio_pu, 1)

			MOD_RADIO_REG_TINY_ENTRY(pi, GPAIO_SEL0, 0, gpaio_sel_0to15_port, 0x0)
			MOD_RADIO_REG_TINY_ENTRY(pi, GPAIO_SEL1, 0, gpaio_sel_16to31_port, 0x0)

			MOD_RADIO_REG_TINY_ENTRY(pi, GPAIO_SEL0, 0, gpaio_sel_0to15_port, 0x0)
			MOD_RADIO_REG_TINY_ENTRY(pi, GPAIO_SEL1, 0, gpaio_sel_16to31_port, 0x0)
		RADIO_REG_LIST_EXECUTE(pi, 0);
	}
	FOREACH_ACTV_CORE(pi, core_mask, core) {
		if (PHY_IPA(pi) || for_iqcal) {
			/* #
			 *  # INT TSSI setup
			 *  #
			 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x0);
				/* # B3: (1 = iqcal, 0 = tssi);
				 * # B2: (1 = ext-tssi, 0 = int-tssi)
				 * # B1: (1 = 5g, 0 = 2g)
				 * # B0: (1 = wo filter, 0 = w filter for ext-tssi)
				 */
				 /* # Select PA output (and not PA input) */
				MOD_RADIO_REG_TINY(pi, PA2G_CFG1, core, pa2g_tssi_ctrl_sel, 0);
				pupd = 1;
			} else {
				pupd = 0;
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x2);
				/* # Select PA output (and not PA input) */
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core,
					pa5g_tssi_ctrl_sel, 0);
				MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core,
					ovr_pa5g_tssi_ctrl_sel, 1);
			}
			if (!(flag2rangeon || flaglowrangeon)) {
				MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
					pa5g_tssi_ctrl_range, (1-pupd));
				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
					ovr_pa5g_tssi_ctrl_range, (1-pupd));
				MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core,
					pa2g_tssi_ctrl_range, pupd);
				MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST,
					core, ovr_pa2g_tssi_ctrl_range, pupd);

			} else {
				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
					ovr_pa5g_tssi_ctrl_range, pupd);
				MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST,
					core, ovr_pa2g_tssi_ctrl_range, 1-pupd);
			}

			/* # int-tssi select */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x0);

			if (for_iqcal == 1) {
				MOD_RADIO_REG_20693(pi, AUXPGA_OVR1, core,
					ovr_auxpga_i_sel_gain, 0x1);
				MOD_RADIO_REG_20693(pi, AUXPGA_OVR1, core,
					ovr_auxpga_i_sel_vmid, 0x1);
				MOD_RADIO_REG_20693(pi, AUXPGA_CFG1, core,
					auxpga_i_sel_gain, 0x3);
				phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
					AUXPGA_VMID, core), 0x9c);

				MOD_RADIO_REG_20693(pi, AUXPGA_OVR1, core,
					ovr_auxpga_i_sel_input, 0x1);
				MOD_RADIO_REG_20693(pi, AUXPGA_CFG1, core,
					auxpga_i_sel_input, 0x0);

				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
						ovr_pa5g_tssi_ctrl_range, 1);
					MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
						pa5g_tssi_ctrl_range, 0);
					MOD_RADIO_REG_20693(pi, TX5G_MISC_CFG1, core,
						pa5g_tssi_ctrl_sel, 0);
				} else {
					MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR_EAST, core,
						ovr_pa2g_tssi_ctrl_range, 0x1);
					MOD_RADIO_REG_20693(pi, TX2G_MISC_CFG1, core,
						pa2g_tssi_ctrl_range, 0);
					MOD_RADIO_REG_20693(pi, PA2G_CFG1, core,
						pa2g_tssi_ctrl_sel, 0);
				}
			}
		} else {
			/* #
			 * # EPA TSSI setup
			 * #
			 */
			/* # Enabling and Muxing per band */
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
			    MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
			} else {
				/* ### gband
				 * 47189 ACDBMR uses the same 5G eTSSI for 2G as well
				 * since both 2G and 5G FEM has a common power detector
				 * which is conencted to the 5G TSSI line
				 */

				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_sw,
					(ROUTER_4349(pi) ? 0x3 : 0x1));
			}
			/* # ext-tssi select */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1);
			if (!(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) &&
				RADIOMAJORREV(pi) == 3)) {
				MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 0x1);
				/* # power on tssi */
				MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x1);
			}
		}
		if (!(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) && RADIOMAJORREV(pi) == 3)) {
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_iqcal, 0x1);
			/* # power off iqlocal */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0);
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 1);
			MOD_RADIO_REG_TINY(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
		} else {
			/* # power on tssi/iqcal */
			MOD_RADIO_REG_TINY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x1);
		}

		if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) && RADIOMAJORREV(pi) == 3) {
			MOD_RADIO_REG_TINY(pi, TIA_CFG9, core, txbb_dac2adc, 0x0);
			MOD_RADIO_REG_TINY(pi, TIA_CFG5, core, tia_out_test, 0x0);

			MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1);
			MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1);
		}
		/* MOD_RADIO_REG_20691(pi, ADC_CFG10, 0, adc_in_test, 0xF); */
		if (!(ACMAJORREV_4(pi->pubpi->phy_rev) && (for_iqcal == 1))) {
			MOD_RADIO_REG_TINY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_sel_input, 0x1);
			MOD_RADIO_REG_TINY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2);
		}
	}
}

void
wlc_phy_tssi_radio_setup_acphy(phy_info_t *pi, uint8 core_mask, uint8 for_iqcal)
{
	uint8 core;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));

	/* 2069_gpaio(clear) to pwr up the GPAIO and clean up al lthe otehr test pins */
	if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) {
		FOREACH_ACTV_CORE(pi, core_mask, core) {
			/* first powerup the CGPAIO block */
			MOD_RADIO_REGC(pi, GE32_CGPAIO_CFG1, core, cgpaio_pu, 1);
		}
		ACPHY_REG_LIST_START
		/* turn off all test points in cgpaio block to avoid conflict,disable tp0 to tp15 */
			WRITE_RADIO_REG_ENTRY(pi, RFX_2069_GE32_CGPAIO_CFG2, 0)
			/* disable tp16 to tp31 */
			WRITE_RADIO_REG_ENTRY(pi, RFX_2069_GE32_CGPAIO_CFG3, 0)
			/* Disable muxsel0 and muxsel1 test points */
			WRITE_RADIO_REG_ENTRY(pi, RFX_2069_GE32_CGPAIO_CFG4, 0)
			/* Disable muxsel2 and muxselgpaio test points */
			WRITE_RADIO_REG_ENTRY(pi, RFX_2069_GE32_CGPAIO_CFG5, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	} else {
		ACPHY_REG_LIST_START
			/* first powerup the CGPAIO block */
			MOD_RADIO_REG_ENTRY(pi, RF2, CGPAIO_CFG1, cgpaio_pu, 1)
		/* turn off all test points in cgpaio block to avoid conflict,disable tp0 to tp15 */
			WRITE_RADIO_REG_ENTRY(pi, RF2_2069_CGPAIO_CFG2, 0)
			/* disable tp16 to tp31 */
			WRITE_RADIO_REG_ENTRY(pi, RF2_2069_CGPAIO_CFG3, 0)
			/* Disable muxsel0 and muxsel1 test points */
			WRITE_RADIO_REG_ENTRY(pi, RF2_2069_CGPAIO_CFG4, 0)
			/* Disable muxsel2 and muxselgpaio test points */
			WRITE_RADIO_REG_ENTRY(pi, RF2_2069_CGPAIO_CFG5, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}
	/* Powerdown rcal. This is one of the enable pins to AND gate in cgpaio block */
	MOD_RADIO_REG(pi, RF2, RCAL_CFG, pu, 0);

	FOREACH_ACTV_CORE(pi, core_mask, core) {

		if (for_iqcal == 0) {
			if (PHY_IPA(pi)) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x2);
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, 0x0);
					MOD_RADIO_REGC(pi, GE16_OVR21, core,
						ovr_pa5g_ctrl_tssi_sel, 0x1);
					MOD_RADIO_REGC(pi, TX5G_TSSI, core,
						pa5g_ctrl_tssi_sel, 0x0);
				} else {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x0);
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, 0x0);
					MOD_RADIO_REGC(pi, GE16_OVR21, core,
						ovr_pa2g_ctrl_tssi_sel, 0x1);
					MOD_RADIO_REGC(pi, PA2G_TSSI, core,
						pa2g_ctrl_tssi_sel, 0x0);
				}
			} else  {
				if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) > 0) &&
				    CHSPEC_IS5G(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x3);
				} else {
					MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw, 0x1);
				}
			}
		} else {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_sw,
			               (CHSPEC_IS5G(pi->radio_chanspec)) ? 0x2 : 0x0);
		}
		if (!PHY_IPA(pi)) {
			MOD_RADIO_REGC(pi, IQCAL_CFG1, core, sel_ext_tssi, for_iqcal == 0);

			switch (core) {
			case 0:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG4, cgpaio_tssi_muxsel0, 0x1);
				break;
			case 1:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG4, cgpaio_tssi_muxsel1, 0x1);
				break;
			case 2:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG5, cgpaio_tssi_muxsel2, 0x1);
				break;
			case 3:
				MOD_RADIO_REG(pi, RF2, CGPAIO_CFG5, cgpaio_tssi_muxsel2, 0x1);
				break;
			default:
				ASSERT(0);
			}
		}

		MOD_RADIO_REGC(pi, IQCAL_CFG1, core, tssi_GPIO_ctrl, 0);
		MOD_RADIO_REGC(pi, TESTBUF_CFG1, core, GPIO_EN, 0);

		/* Reg conflict with 2069 rev 16 */
		if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 0) {
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, OVR20, core, ovr_pa5g_ctrl_tssi_sel, 1);
		} else if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 1) ||
			(RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2)) {
			MOD_RADIO_REGC(pi, TX5G_TSSI, core, pa5g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa5g_ctrl_tssi_sel, 1);
			MOD_RADIO_REGC(pi, PA2G_TSSI, core, pa2g_ctrl_tssi_sel, for_iqcal);
			MOD_RADIO_REGC(pi, GE16_OVR21, core, ovr_pa2g_ctrl_tssi_sel, 1);
		}

		if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 1) {
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_vcm_ctrl, 0x0);
			/* This bit is supposed to be controlled by phy direct control line.
			 * Please check: http://jira.broadcom.com/browse/HW11ACRADIO-45
			 */
			MOD_RADIO_REGC(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2);
		}

	}
}

void
wlc_phy_tssi_radio_setup_acphy_20696(phy_info_t *pi, uint8 for_iqcal)
{
	/* ported the code from Iguana PHY_BRANCH_2_100 */
	/* save radio config before changing it */
	uint8 core;

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

	BCM_REFERENCE(phyrxchain);

	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		MOD_RADIO_REG_20696(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_RADIO_REG_20696(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
		} else {
			MOD_RADIO_REG_20696(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x5);
		}
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, LPF_REG7, core, lpf_sw_bq2_adc, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, LPF_REG7, core, lpf_sw_bq1_adc, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, LPF_REG7, core, lpf_sw_aux_adc, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, LPF_REG8, core, lpf_sw_adc_test, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG1, core, iqcal_tssi_GPIO_ctrl, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG2, core, iqcal_tssi_cm_center, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_IDAC, core, iqcal_tssi_bias, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_GAIN_RFB, core, iqcal_rfb, 0x200)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_GAIN_RIN, core, iqcal_rin, 0x800)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG5, core, wbpga_pu, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_OVR1, core, ovr_loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG5, core, loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_iqbuf, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_half, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG6, core, wbpga_cmref, 0x14)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG6, core, wbpga_bias, 0x14)

			/* Selection between auxpga or iqcal(wbpga) */
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG4, core, iqcal2adc, 0x0)
			MOD_RADIO_REG_20696_ENTRY(pi, IQCAL_CFG4, core, auxpga2adc, 0x1)

			MOD_RADIO_REG_20696_ENTRY(pi, TESTBUF_OVR1, core, ovr_testbuf_PU, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, TESTBUF_CFG1, core, testbuf_PU, 0x1)

			MOD_RADIO_REG_20696_ENTRY(pi, TESTBUF_OVR1, core,
				ovr_testbuf_sel_test_port, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, TESTBUF_CFG1, core,
				testbuf_sel_test_port, 0x2)

			MOD_RADIO_REG_20696_ENTRY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1)

			MOD_RADIO_REG_20696_ENTRY(pi, AUXPGA_OVR1, core,
				ovr_auxpga_i_sel_input, 0x1)
			MOD_RADIO_REG_20696_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}
}

void
wlc_phy_tssi_radio_setup_acphy_20698(phy_info_t *pi, uint8 for_iqcal)
{
	/* 20698_procs.tcl r708059: 20698_tssi_radio_setup */
	/* radio config is NOT saved here! */
	uint8 core;

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

	BCM_REFERENCE(phyrxchain);

	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		MOD_RADIO_REG_20698(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_RADIO_REG_20698(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
		} else {
			MOD_RADIO_REG_20698(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x5);
		}
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x0)

			/* FIXME43684: next 4 lines do not have overrides set,
			 * so seem to be ineffective.
			 * However, according to 20698_procs.tcl
			 * r631126 TPC does not work if overrides
			 * are set. This needs further attention
			 */
			MOD_RADIO_REG_20698_ENTRY(pi, LPF_REG7, core, lpf_sw_bq2_adc, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, LPF_REG7, core, lpf_sw_bq1_adc, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, LPF_REG7, core, lpf_sw_aux_adc, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, LPF_REG8, core, lpf_sw_adc_test, 0x0)

			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG1, core, iqcal_tssi_GPIO_ctrl, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG2, core, iqcal_tssi_cm_center, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_IDAC, core, iqcal_tssi_bias, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_GAIN_RFB, core, iqcal_rfb, 0x200)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_GAIN_RIN, core, iqcal_rin, 0x800)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG5, core, wbpga_pu, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG5, core, loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_OVR1, core, ovr_loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_iqbuf, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_half, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG6, core, wbpga_cmref, 0x14)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG6, core, wbpga_bias, 0x14)

			/* Selection between auxpga or iqcal(wbpga) */
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG4, core, iqcal2adc, 0x0)
			MOD_RADIO_REG_20698_ENTRY(pi, IQCAL_CFG4, core, auxpga2adc, 0x1)

			MOD_RADIO_REG_20698_ENTRY(pi, TESTBUF_CFG1, core, testbuf_PU, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, TESTBUF_OVR1, core, ovr_testbuf_PU, 0x1)

			MOD_RADIO_REG_20698_ENTRY(pi, TESTBUF_CFG1, core,
				testbuf_sel_test_port, 0x2)
			MOD_RADIO_REG_20698_ENTRY(pi, TESTBUF_OVR1, core,
				ovr_testbuf_sel_test_port, 0x1)

			MOD_RADIO_REG_20698_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1)
			MOD_RADIO_REG_20698_ENTRY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1)

			MOD_RADIO_REG_20698_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2)
			MOD_RADIO_REG_20698_ENTRY(pi, AUXPGA_OVR1, core,
				ovr_auxpga_i_sel_input, 0x1)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}
}

void
wlc_phy_tssi_radio_setup_acphy_20704(phy_info_t *pi, uint8 for_iqcal)
{
	// FIXME63178: tcl code not available yet
	/* radio config is NOT saved here! */
	uint8 core;

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

	BCM_REFERENCE(phyrxchain);

	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		MOD_RADIO_REG_20704(pi, IQCAL_OVR1, core, ovr_iqcal_PU_tssi, 1);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_RADIO_REG_20704(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x3);
		} else {
			MOD_RADIO_REG_20704(pi, IQCAL_CFG1, core, iqcal_sel_sw, 0x5);
		}
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG1, core, iqcal_sel_ext_tssi, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_tssi, 0x0)

			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG1, core, iqcal_PU_iqcal, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG1, core, iqcal_tssi_GPIO_ctrl, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG2, core, iqcal_tssi_cm_center, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_IDAC, core, iqcal_tssi_bias, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_GAIN_RFB, core, iqcal_rfb, 0x200)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_GAIN_RIN, core, iqcal_rin, 0x800)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG5, core, wbpga_pu, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG5, core, loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_OVR1, core, ovr_loopback_bias_pu, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_iqbuf, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG5, core, wbpga_cmref_half, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG6, core, wbpga_cmref, 0x14)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG6, core, wbpga_bias, 0x14)

			/* Selection between auxpga or iqcal(wbpga) */
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG4, core, iqcal2adc, 0x0)
			MOD_RADIO_REG_20704_ENTRY(pi, IQCAL_CFG4, core, auxpga2adc, 0x1)

			MOD_RADIO_REG_20704_ENTRY(pi, TESTBUF_CFG1, core, testbuf_PU, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, TESTBUF_OVR1, core, ovr_testbuf_PU, 0x1)

			MOD_RADIO_REG_20704_ENTRY(pi, TESTBUF_CFG1, core,
				testbuf_sel_test_port, 0x2)
			MOD_RADIO_REG_20704_ENTRY(pi, TESTBUF_OVR1, core,
				ovr_testbuf_sel_test_port, 0x1)

			MOD_RADIO_REG_20704_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_pu, 0x1)
			MOD_RADIO_REG_20704_ENTRY(pi, AUXPGA_OVR1, core, ovr_auxpga_i_pu, 0x1)

			MOD_RADIO_REG_20704_ENTRY(pi, AUXPGA_CFG1, core, auxpga_i_sel_input, 0x2)
			MOD_RADIO_REG_20704_ENTRY(pi, AUXPGA_OVR1, core,
				ovr_auxpga_i_sel_input, 0x1)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}
}

void
phy_ac_tssi_loopback_path_setup(phy_info_t *pi, uint8 for_iqcal)
{
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	wlc_phy_tssi_phy_setup_acphy(pi, for_iqcal);
	if (TINY_RADIO(pi)) {
		wlc_phy_tssi_radio_setup_acphy_tiny(pi, stf_shdata->hw_phyrxchain, for_iqcal);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20694_ID)) {
		wlc_phy_tssi_radio_setup_acphy_20694(pi, for_iqcal);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20695_ID)) {
		wlc_phy_tssi_radio_setup_acphy_28nm(pi, for_iqcal);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20696_ID)) {
		wlc_phy_tssi_radio_setup_acphy_20696(pi, for_iqcal);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20697_ID)) {
		wlc_phy_tssi_radio_setup_acphy_20697(pi);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20698_ID)) {
		wlc_phy_tssi_radio_setup_acphy_20698(pi, for_iqcal);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20704_ID)) {
		wlc_phy_tssi_radio_setup_acphy_20704(pi, for_iqcal);
	} else {
		wlc_phy_tssi_radio_setup_acphy(pi, stf_shdata->hw_phyrxchain, for_iqcal);
	}
}

#define PHY_MIN_IDLE_TSSI	-512 /* 10 bit format */

void
wlc_phy_txpwrctrl_set_idle_tssi_acphy(phy_info_t *pi, int16 idle_tssi, uint8 core)
{
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi2g(pi->tpci)) || (CHSPEC_IS5G(pi->radio_chanspec) &&
		phy_tpc_get_tworangetssi5g(pi->tpci))) && PHY_IPA(pi);

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (idle_tssi < PHY_MIN_IDLE_TSSI) {
			PHY_FATAL_ERROR_MESG((" %s: Invalid Idle TSSI : %d \n",
				__FUNCTION__, idle_tssi));
			PHY_FATAL_ERROR(pi, PHY_RC_IDLETSSI_INVALID);
		}
	}

	/* set idle TSSI in 2s complement format (max is 0x1ff) */
	switch (core) {
	case 0:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path0, idleTssi0, idle_tssi);
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path0, idleTssi_second0, idle_tssi);
		}
		break;
	case 1:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path1, idleTssi1, idle_tssi);
		break;
	case 2:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path2, idleTssi2, idle_tssi);
		break;
	case 3:
		MOD_PHYREG(pi, TxPwrCtrlIdleTssi_path3, idleTssi3, idle_tssi);
		break;
	}

	/* Only 4335 and 4350 has 2nd idle-tssi */
	if (((ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_2(pi->pubpi->phy_rev) ||
		ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_40(pi->pubpi->phy_rev)) &&
			BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) ||	flag2rangeon) {
		switch (core) {
		case 0:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path0,
			           idleTssi_second0, idle_tssi);
			break;
		case 1:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path1,
			           idleTssi_second1, idle_tssi);
			break;
		}
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	           ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) ||
	           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		switch (core) {
		case 0:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path0, idleTssi_second0, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path0, idleTssi_third0, idle_tssi);
			break;
		case 1:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path1, idleTssi_second1, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path1, idleTssi_third1, idle_tssi);
			break;
		case 2:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path2, idleTssi_second2, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path2, idleTssi_third2, idle_tssi);
			break;
		case 3:
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_second_path3, idleTssi_second3, idle_tssi);
			MOD_PHYREG(pi, TxPwrCtrlIdleTssi_third_path3, idleTssi_third3, idle_tssi);
			break;
		}
		if ((ACMAJORREV_47(pi->pubpi->phy_rev) &&
			(HW_ACMINORREV(pi) >= 1))) {
			switch (core) {
			case 0:
				MOD_PHYREG(pi, TxPwrCtrlIdleTssi_fourth_path0,
					idleTssi_fourth0, idle_tssi);
				break;
			case 1:
				MOD_PHYREG(pi, TxPwrCtrlIdleTssi_fourth_path1,
					idleTssi_fourth1, idle_tssi);
				break;
			case 2:
				MOD_PHYREG(pi, TxPwrCtrlIdleTssi_fourth_path2,
					idleTssi_fourth2, idle_tssi);
				break;
			case 3:
				MOD_PHYREG(pi, TxPwrCtrlIdleTssi_fourth_path3,
					idleTssi_fourth3, idle_tssi);
				break;
			}
		}
	}
}

static int8
phy_ac_tssical_get_visible_thresh(phy_type_tssical_ctx_t *ctx)
{
	phy_ac_tssical_info_t *info = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = info->pi;
	return wlc_phy_tssivisible_thresh_acphy(pi);
}

int8
wlc_phy_tssivisible_thresh_acphy(phy_info_t *pi)
{
	int8 visi_thresh_qdbm;
	uint16 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		/* J28 */
		if ((BFCTL(pi->u.pi_acphy) == 3) && (BF3_FEMCTRL_SUB(pi->u.pi_acphy) == 2)) {
			visi_thresh_qdbm = 30;  /* 7.5*4 */
		} else {
			if (IS_X52C_BOARDTYPE(pi))
				visi_thresh_qdbm = 5*4;
			else if (IS_X29C_BOARDTYPE(pi) && (channel >= 149))
				visi_thresh_qdbm = 22; /* 5.5 dBm */
			else
				visi_thresh_qdbm = 6*4;
		}
	}
	else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {

#ifdef WLC_TXCAL
		if (phy_tssical_get_olpc_threshold(pi->tssicali) != 0) {
			visi_thresh_qdbm = phy_tssical_get_olpc_threshold(pi->tssicali);
		} else
#endif /* WLC_TXCAL */

		{
			visi_thresh_qdbm = 7*4;
		}

#ifdef WLC_TXCAL
		if (phy_tssical_get_disable_olpc(pi->tssicali) == 1)
			visi_thresh_qdbm = WL_RATE_DISABLED;
#endif /* WLC_TXCAL */

	}
	else if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		if (phy_tpc_get_2g_pdrange_id(pi->tpci) == 24)
			visi_thresh_qdbm = 4*4;
		else
			visi_thresh_qdbm = 7*4;
	}
	else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		visi_thresh_qdbm = (CHSPEC_IS2G(pi->radio_chanspec) ? pi->min_txpower :
			pi->min_txpower_5g) * WLC_TXPWR_DB_FACTOR;
#if (!defined(WLOLPC) || defined(WLOLPC_DISABLED)) && defined(WLC_TXCAL)
		/* if training based OLPC is disabled
		 * but txcal based olpc idx is not valid
		 * disable OLPC
		 */
		if (!phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali)) {
			visi_thresh_qdbm = WL_RATE_DISABLED;
		}
#endif	/* (!WLOLPC || WLOLPC_DISABLED) && WLC_TXCAL */
	} else if (SROMREV(pi->sh->sromrev) == 13) {
	/* TSSI visibility thresholds for the Enterprise Wave-2 cards */
		visi_thresh_qdbm = WL_RATE_DISABLED;
	} else if (SROMREV(pi->sh->sromrev) >= 18) {
		/* Support for 11ax chips
		 * Visibility threshold may be unique for each board.
		 */

		/* SROM18 includes OLPC thresholds for 2G and 5G, check it first. */
		visi_thresh_qdbm = CHSPEC_IS2G(pi->radio_chanspec) ?
			phy_tssical_get_olpc_threshold2g(pi->tssicali) :
			phy_tssical_get_olpc_threshold5g(pi->tssicali);
		if (WL_RATE_DISABLED != visi_thresh_qdbm) {
			/* Convert dBm to qdBm */
			visi_thresh_qdbm *= WLC_TXPWR_DB_FACTOR;
			PHY_TXPWR(("WL_EAP: Using srom-ed TSSI thrsh %d qdBm\n",
				visi_thresh_qdbm));
		} else {
			/* Else hard-code default thresholds for each board. */
			/* set threshold to -128 for BCA prodcut to avoid OLPC */
			visi_thresh_qdbm = WL_RATE_DISABLED;
		}
		PHY_TXPWR(("WL_OLPC: 11ax board 0x%x, TSSI thrsh %d qdBm %s\n",
			pi->sh->boardtype, visi_thresh_qdbm,
			(WL_RATE_DISABLED == visi_thresh_qdbm) ? "Disabled" : ""));
	}
	else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
#if defined(WLC_TXCAL)
		if (phy_tssical_get_disable_olpc(pi->tssicali) == 1)
			visi_thresh_qdbm = WL_RATE_DISABLED;
		else if (phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali))
			visi_thresh_qdbm = phy_tssical_get_olpc_threshold(pi->tssicali);
		else
			visi_thresh_qdbm = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
#else
		visi_thresh_qdbm = pi->min_txpower * WLC_TXPWR_DB_FACTOR;
#endif // endif
	} else {
		PHY_ERROR(("EAP: %s: no visibility threshold for boardtype"
			" 0x%x AND rev 0x%x(%d).  Do you need to add it?\n",
			__FUNCTION__, pi->sh->boardtype, pi->pubpi->phy_rev, pi->pubpi->phy_rev));

		visi_thresh_qdbm = WL_RATE_DISABLED;
	}
	if (ACMAJORREV_3(pi->pubpi->phy_rev) ||
			ACMAJORREV_5(pi->pubpi->phy_rev))
		visi_thresh_qdbm = -128;
	return visi_thresh_qdbm;
}

#if defined(WLTEST)
int16
wlc_phy_test_tssi_acphy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs)
{
	int16 tssi = 0;
	int16 temp = 0;
	int Npt, Npt_log2, i;
	bool suspend = FALSE;
	wlc_phy_conditional_suspend(pi, &suspend);
	Npt_log2 = READ_PHYREGFLD(pi, TxPwrCtrlNnum, Npt_intg_log2);
	Npt = 1 << Npt_log2;

	switch (ctrl_type & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		for (i = 0; i < Npt; i++) {
			OSL_DELAY(10);
			temp = READ_PHYREGCE(pi, TssiVal_path, ctrl_type) & 0x3ff;
			temp -= (temp >= 512) ? 1024 : 0;
			tssi += temp;
		}
		tssi = tssi >> Npt_log2;
		break;
	default:
		tssi = -1024;
	}
	wlc_phy_conditional_resume(pi, &suspend);
	return (tssi);
}

int16
wlc_phy_test_idletssi_acphy(phy_info_t *pi, int8 ctrl_type)
{
	int16 idletssi = INVALID_IDLETSSI_VAL;
	bool suspend = FALSE;

	/* Suspend MAC if haven't done so */
	wlc_phy_conditional_suspend(pi, &suspend);

	switch (ctrl_type & 0x7) {
	case 0:
	case 1:
	case 2:
	case 3:
		idletssi = READ_PHYREGCE(pi, TxPwrCtrlIdleTssi_path, ctrl_type) & 0x3ff;
		idletssi -= (idletssi >= 512) ? 1024 : 0;
		break;
	default:
		idletssi = INVALID_IDLETSSI_VAL;
	}

	/* Resume MAC */
	wlc_phy_conditional_resume(pi, &suspend);

	return (idletssi);
}
#endif // endif

static void
wlc_phy_get_tssisens_min_acphy(phy_type_tssical_ctx_t *ctx, int8 *tssiSensMinPwr)
{
	phy_ac_tssical_info_t *info = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* These need to be in descending order and fall-thru */
	switch (PHYCORENUM(pi->pubpi->phy_corenum)) {
	case 4:
		tssiSensMinPwr[3] = READ_PHYREGFLD(pi, TxPwrCtrlCore3TSSISensLmt, tssiSensMinPwr);
	case 3:
		tssiSensMinPwr[2] = READ_PHYREGFLD(pi, TxPwrCtrlCore2TSSISensLmt, tssiSensMinPwr);
	case 2:
		tssiSensMinPwr[1] = READ_PHYREGFLD(pi, TxPwrCtrlCore1TSSISensLmt, tssiSensMinPwr);
	default:
		tssiSensMinPwr[0] = READ_PHYREGFLD(pi, TxPwrCtrlCore0TSSISensLmt, tssiSensMinPwr);
	}
}

void
wlc_phy_set_tssisens_lim_acphy(phy_info_t *pi, uint8 override)
{
	uint16 tssi_limit;
	int8 visi_thresh_qdbm = WL_RATE_DISABLED;
#if defined(PHYCAL_CACHING)
	acphy_calcache_t *cache;
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif
	if (override) {
		/* Override mode, always write tssi visible thresh */
		visi_thresh_qdbm = wlc_phy_tssivisible_thresh_acphy(pi);
	}
#if defined(WLC_TXCAL)
	else if (phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali)) {
		/* txcal based OLPC is in use, set tssi visible thresh to let OLPC work */
		visi_thresh_qdbm = wlc_phy_tssivisible_thresh_acphy(pi);
	}
#endif	/* WLC_TXCAL */
#if defined(PHYCAL_CACHING)
	else {
		if (ctx) {
			if (ctx->valid) {
				cache = &ctx->u.acphy_cache;
				if (cache->olpc_caldone) {
					/* If cal is done, write correct visibility thresh
					 * else, write -127
					 */
					visi_thresh_qdbm = wlc_phy_tssivisible_thresh_acphy(pi);
				}
			}
		}
	}
#endif // endif
	tssi_limit = (127 << 8) + (visi_thresh_qdbm & 0xFF);
	ACPHYREG_BCAST(pi, TxPwrCtrlCore0TSSISensLmt, tssi_limit);
}

#ifdef PHYCAL_CACHING
void
phy_ac_tssical_idle_save_cache(phy_ac_tssical_info_t *ti, ch_calcache_t *ctx)
{
	phy_info_t *pi = ti->pi;
	uint8 core;
	PHY_CAL(("phy_ac_tssical_idle_save_cache: 0x%x:\n", pi->radio_chanspec));
	FOREACH_CORE(pi, core) {
		/* save idle TSSI */
		ctx->u.acphy_cache.idle_tssi[core] =
			READ_PHYREGCE(pi, TxPwrCtrlIdleTssi_path, core);
	}
}
#endif /* PHYCAL_CACHING */

void
phy_ac_tssical_idle(phy_info_t *pi)
{
	/*
	 *     Idle TSSI & TSSI-to-dBm Mapping Setup
	 */
#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif
	wlc_phy_cts2self(pi, 1550);
	if ((pi->radar_percal_mask & 0x8) != 0)
		pi->u.pi_acphy->radar_cal_active = TRUE;

	/* Idle TSSI determination once right after join/up/assoc */
	wlc_phy_txpwrctrl_idle_tssi_meas_acphy(pi);

	/* done with multi-phase cal, reset phase */
	pi->first_cal_after_assoc = FALSE;

#if defined(PHYCAL_CACHING)
	if (ctx) {
		phy_ac_tssical_idle_save_cache(pi->u.pi_acphy->tssicali, ctx);
		ctx->valid = TRUE;
		phy_ac_tpc_save_cache(pi->u.pi_acphy->tpci, ctx);
	}
#endif // endif

	phy_calmgr_mphase_reset(pi->calmgri);
}

#ifdef WLC_TXCAL
uint8
wlc_phy_set_olpc_anchor_acphy(phy_info_t *pi)
{
	/* Search over the linked txcal table list */
	/* to find out the anchor power tx idx */
	txcal_pwr_tssi_lut_t *LUT_pt;
	txcal_pwr_tssi_lut_t *LUT_root;
	uint8 chan_num = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 flag_chan_found = 0;
	uint8 core;
	txcal_root_pwr_tssi_t *pi_txcal_root_pwr_tssi_tbl =
			phy_tssical_get_root_pwr_tssi_tbl(pi->tssicali);
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
	} else {
		if (CHSPEC_IS80(pi->radio_chanspec))
			LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G80;
		else if (CHSPEC_IS40(pi->radio_chanspec))
			LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G40;
		else
			LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
	}
	if (LUT_root->txcal_pwr_tssi->channel == 0) {
		if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
			/* No Txcal table is present, return */
			/* olpc_anchor_idx is a check in phy level */
			/* to verify whether the tx idx at the anchor */
			/* power is valid for current channel */
			phy_tssical_set_olpc_idx_valid(pi->tssicali, 0);
			return BCME_OK;
		} else {
			/* For 40 and 80 if no Txcal table is present */
			/* Use 20mhz txcal table */
			LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
			if (LUT_root->txcal_pwr_tssi->channel == 0) {
				phy_tssical_set_olpc_idx_valid(pi->tssicali, 0);
				return BCME_OK;
			}
		}
	}
	LUT_pt = LUT_root;
	while (LUT_pt->next_chan != 0) {
		/* Go over all the entries in the list */
		if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
			flag_chan_found = 1;
			break;
		}
		if ((LUT_pt->txcal_pwr_tssi->channel < chan_num) &&
			(LUT_pt->next_chan->txcal_pwr_tssi->channel > chan_num)) {
			flag_chan_found = 2;
			break;
		}
		LUT_pt = LUT_pt->next_chan;
	}
	if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
		/* In case only one entry is in the list */
		flag_chan_found = 1;
	}
	switch (flag_chan_found) {
	case 0:
		/* Channel not found in linked list or not between two channels */
		/* Then pick the closest one */
		if (chan_num < LUT_root->txcal_pwr_tssi->channel)
			LUT_pt = LUT_root;
		break;
	case 2:
		/* Channel is in between two channels, pick closest one as the anchor idx */
		if (ABS(chan_num - LUT_pt->txcal_pwr_tssi->channel) >=
				ABS(LUT_pt->next_chan->txcal_pwr_tssi->channel - chan_num))
			LUT_pt = LUT_pt->next_chan;
		break;
	}
	FOREACH_CORE(pi, core) {
		phy_tssical_set_olpc_anchor_idx(pi->tssicali, core,
				LUT_pt->txcal_pwr_tssi->pwr_start_idx[core]);
		/* if anchor idx is 0, then decide it is not valid */
		if (phy_tssical_get_olpc_anchor_idx(pi->tssicali, core) == 0) {
			phy_tssical_set_olpc_idx_valid(pi->tssicali, 0);
			return BCME_OK;
		}
		/* temperature recorded for tx idx at the anchor power */
		phy_tssical_set_olpc_tempsense(pi->tssicali, core,
				LUT_pt->txcal_pwr_tssi->tempsense[core]);
	}
	phy_tssical_set_olpc_idx_valid(pi->tssicali, 1);
	/* If olpc idx is valid from LUT assign pwr_start (Ptssi) to olpc_thresh and anchor */
	phy_tssical_set_olpc_threshold_val(pi->tssicali,
			(LUT_pt->txcal_pwr_tssi->pwr_start[0] >> 1));
	phy_tssical_set_olpc_anchor_threshold(pi->tssicali);
	return BCME_OK;
}

uint8
wlc_phy_estpwrlut_intpol_acphy(phy_info_t *pi, uint8 channel,
		wl_txcal_power_tssi_t *pwr_tssi_lut_ch1, wl_txcal_power_tssi_t *pwr_tssi_lut_ch2,
		uint8 bphy_tbl)
{
	uint16 estpwr1[128];
	uint16 estpwr2[128];
	uint16 estpwr[128];
	int16 est_pwr_calc, est_pwr_calc1, est_pwr_calc2, est_pwr_intpol1, est_pwr_intpol2;
	uint32 tbl_len = 128;
	uint32 tbl_offset = 0;
	uint8 core, i;
	uint8 tx_pwr_ctrl_state;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	tx_pwr_ctrl_state =  pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);

	/* Interpolate between estpwrlut */
	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		phy_tssical_generate_estpwr_lut(pwr_tssi_lut_ch1, estpwr1, core);
		phy_tssical_generate_estpwr_lut(pwr_tssi_lut_ch2, estpwr2, core);
		for (i = 0; i < 128; i++) {
			est_pwr_calc1 = estpwr1[i] > 0x7F ?
				(int16) (estpwr1[i] - 0x100) : estpwr1[i];
			est_pwr_calc2 = estpwr2[i] > 0x7F ?
				(int16) (estpwr2[i] - 0x100) : estpwr2[i];
			/* round to the nearest integer */
			est_pwr_intpol1 = 2*(channel - pwr_tssi_lut_ch1->channel)*(est_pwr_calc2 -
				est_pwr_calc1)/(pwr_tssi_lut_ch2->channel -
				pwr_tssi_lut_ch1->channel);
			est_pwr_intpol2 = (channel - pwr_tssi_lut_ch1->channel)*(est_pwr_calc2 -
					est_pwr_calc1)/(pwr_tssi_lut_ch2->channel -
				pwr_tssi_lut_ch1->channel);
			est_pwr_calc = est_pwr_calc1 + est_pwr_intpol1 - est_pwr_intpol2;
			/* Program the upper 8-bits for CCK for 43012 */
			if (ACMAJORREV_36(pi->pubpi->phy_rev))
				estpwr[i] = (uint16) (((est_pwr_calc & 0x00FF) << 8) |
						(est_pwr_calc & 0x00FF));
			else
				estpwr[i] = (uint16)(est_pwr_calc & 0xFF);
		}
		if (!bphy_tbl) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
				tbl_offset, 16, estpwr);
		} else {
			uint16 estpwr_tmp[128];
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
				tbl_offset, 16, &estpwr_tmp);
			for (i = 0; i < 128; i++) {
				estpwr[i]  = (uint16) (((estpwr[i] & 0x00FF) << 8) |
						(estpwr_tmp[i] & 0x00FF));
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
				tbl_offset, 16, estpwr);
		}

	}
	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	return BCME_OK;
}
#define TEMP_COMP_IDX_BOUND (10)
/* Based on the temperature compensation data, limiting the temperature compensated
 * idx to 10 to avoid running out of idx to unexpected value due to incorrect
 * temperature updates
*/
uint8
wlc_phy_olpc_idx_tempsense_comp_acphy(phy_info_t *pi, uint8 *iidx, uint8 core)
{
	/* This function calculates the init idx based on max tgt pwr, */
	/* table based txcal anchor power tx idx, and temperature */
	uint8 olpc_anchor = 0, olpc_anchor_idx = 0;
	int16 currtemp = 0;
	int16 olpc_tempslope = 0, olpc_tempsense = 0;
	int16 idx = 0;
	int8 chan_freq_range, offset_idx = 0;
	int8 delta_idx = 0;
	olpc_anchor = phy_tssical_get_olpc_anchor(pi->tssicali);
	olpc_tempslope = phy_tssical_get_olpc_tempslope(pi->tssicali, core);
	olpc_anchor_idx = phy_tssical_get_olpc_anchor_idx(pi->tssicali, core);
	olpc_tempsense = phy_tssical_get_olpc_tempsense(pi->tssicali, core);
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Gain table in steps of 0.25dBm */
		idx = olpc_anchor_idx - (pi->tx_power_max_per_core[core]
			- olpc_anchor + 1);
	} else {
		idx = olpc_anchor_idx - ((pi->tx_power_max_per_core[core]
			- olpc_anchor + 1) >> 1);
	}
	if (olpc_tempslope) {
		currtemp = wlc_phy_tempsense_acphy(pi);
		delta_idx = idx - (((currtemp - olpc_tempsense) * olpc_tempslope +
			512) >> 10);
		if (ABS(delta_idx) > TEMP_COMP_IDX_BOUND) {
			delta_idx = 0;
		}
		idx = idx - delta_idx;
	}
	chan_freq_range = phy_ac_chanmgr_get_chan_freq_range(pi, pi->radio_chanspec, core);
	offset_idx = phy_tssical_get_olpc_offset(pi->tssicali, chan_freq_range);
	idx = idx + offset_idx;
	if (idx < 0) {
		idx = 0;
	} else if (idx > 127) {
		idx = 127;
	}
	*iidx = (uint8) idx;
	return BCME_OK;
}

void
phy_ac_tssical_set_olpc_threshold(phy_info_t *pi)
{
	/* If olpc_thresh is present from nvram but olpc_thresh2g/5g is not,
	 * use olpc_thresh value;
	 * If olpc_thresh2g/5g is present from nvram, then use it
	 * the IOVAR olpc_thresh can still be used to override nvram value
	 */
	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
			!ACMAJORREV_33(pi->pubpi->phy_rev) &&
			!ACMAJORREV_37(pi->pubpi->phy_rev) &&
			!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		phy_tssical_set_olpc_threshold(pi->tssicali);
	 }
}

static void
phy_ac_tssical_compute_olpc_idx(phy_type_tssical_ctx_t *ctx)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	bool olpc_idx_valid_in_use = phy_tssical_get_olpc_idx_valid_in_use(pi->tssicali);
	if (phy_tssical_get_olpc_idx_in_use(pi->tssicali)) {
		wlc_phy_set_olpc_anchor_acphy(pi);
	}
	/* If txcal based olpc is in use, compute the idx before */
	/* calling wlc_phy_txpwrctrl_set_baseindex */
	wlc_phy_txcal_olpc_idx_recal_acphy(pi, olpc_idx_valid_in_use);
	/* Load tx gain table: Typically needs to be done as part of band change.
	 * because of tx gain table capping using txcal and olpc infra, this
	 * needs to be done a chanspec
	 */
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		wlc_phy_ac_gains_load(pi->u.pi_acphy->tbli);
	}
}

static uint16
phy_ac_tssical_adjusted_tssi(phy_type_tssical_ctx_t *ctx, uint8 core_num)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	return wlc_phy_adjusted_tssi_acphy(pi, core_num);
}

uint16
wlc_phy_adjusted_tssi_acphy(phy_info_t *pi, uint8 core_num)
{
	uint16 adj_tssi = 0;
	int16 tssi_OB, idletssi_OB;
	uint8 pos_slope;
	int16 tssi_reg, idletssi_reg;
	tssi_reg = READ_PHYREGCE(pi, TssiVal_path, core_num) & 0x3ff;
	if (tssi_reg >= 512)
		tssi_OB = tssi_reg - 511;
	else
		tssi_OB = tssi_reg + 512;
	idletssi_reg = READ_PHYREGCE(pi, TxPwrCtrlIdleTssi_path, core_num) & 0x3ff;
	if (idletssi_reg >= 512)
		idletssi_OB = idletssi_reg - 511;
	else
		idletssi_OB = idletssi_reg + 512;
	pos_slope = READ_PHYREGFLD(pi, TSSIMode, tssiPosSlope);
	if (pos_slope)
		adj_tssi = idletssi_OB - tssi_OB  + ((1 << 10)-1);
	else
		adj_tssi = tssi_OB - idletssi_OB  + ((1 << 10)-1);
	return adj_tssi;
}

uint8
wlc_phy_apply_pwr_tssi_tble_chan_acphy(phy_info_t *pi)
{
	txcal_pwr_tssi_lut_t *LUT_pt;
	txcal_pwr_tssi_lut_t *LUT_root;
	txcal_pwr_tssi_lut_t *LUT_root_bphy;
	txcal_root_pwr_tssi_t *pi_txcal_root_pwr_tssi_tbl =
			phy_tssical_get_root_pwr_tssi_tbl(pi->tssicali);
	LUT_root_bphy = NULL;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G;
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			LUT_root_bphy = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_2G_bphy;
		}
	}
	else if (CHSPEC_IS80(pi->radio_chanspec))
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G80;
	else if (CHSPEC_IS40(pi->radio_chanspec))
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G40;
	else
		LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
	if (LUT_root->txcal_pwr_tssi->channel == 0) {
		/* if no txcal table is present for 2G and 5G20, apply paparam */
		/* For 5G40/80, apply 20mhz txcal table */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_tssical_apply_pa_params(pi->tssicali);
			return BCME_OK;
		} else {
			if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
				phy_tssical_apply_pa_params(pi->tssicali);
				return BCME_OK;
			} else {
				LUT_root = pi_txcal_root_pwr_tssi_tbl->root_pwr_tssi_lut_5G20;
				if (LUT_root->txcal_pwr_tssi->channel == 0) {
					phy_tssical_apply_pa_params(pi->tssicali);
					return BCME_OK;
				}
			}
		}
	}
	LUT_pt = LUT_root;
	wlc_phy_populate_pwr_tssi_tble_chan_acphy(pi, LUT_pt, OFDM_TBL);
	if (ACMAJORREV_40 (pi->pubpi->phy_rev) && CHSPEC_IS2G(pi->radio_chanspec)) {
		if (LUT_root_bphy->txcal_pwr_tssi->channel == 0) {
			LUT_pt = LUT_root;
		} else {
			uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
			uint8 core;
			bool suspend = FALSE;
			LUT_pt = LUT_root_bphy;
			//suspend mac before accessing phy registers
			wlc_phy_conditional_suspend(pi, &suspend);
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {
				MOD_PHYREGCEE(pi, TxPwrCtrlTargetPwr_path, core,
					cckPwrOffset, 0);
			}
			wlc_phy_conditional_resume(pi, &suspend);
		}
		wlc_phy_populate_pwr_tssi_tble_chan_acphy(pi, LUT_pt, BPHY_TBL);
	}

	return BCME_OK;
}

void
wlc_phy_populate_pwr_tssi_tble_chan_acphy(phy_info_t *pi,
	txcal_pwr_tssi_lut_t *LUT_pt, uint8 bphy_tbl)
{
	uint8 chan_num = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 flag_chan_found = 0;
	txcal_pwr_tssi_lut_t *LUT_root = LUT_pt;

	while (LUT_pt->next_chan != 0) {
		/* Go over all the entries in the list */
		if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
			flag_chan_found = 1;
			break;
		}
		if ((LUT_pt->txcal_pwr_tssi->channel < chan_num) &&
		    (LUT_pt->next_chan->txcal_pwr_tssi->channel > chan_num)) {
			flag_chan_found = 2;
			break;
		}
		LUT_pt = LUT_pt->next_chan;
	}
	if (LUT_pt->txcal_pwr_tssi->channel == chan_num) {
		/* In case only one entry is in the list */
		flag_chan_found = 1;
	}
	switch (flag_chan_found) {
	case 0:
		/* Channel not found in linked list or not between two channels */
		/* Then pick the closest one */
		if (chan_num < LUT_root->txcal_pwr_tssi->channel)
			LUT_pt = LUT_root;
		phy_tssical_apply_pwr_tssi_tbl(pi->tssicali, LUT_pt->txcal_pwr_tssi,
			bphy_tbl);
		phy_tssical_set_txcal_status(pi->tssicali, 2);
		break;
	case 1:
		/* Channel found */
		phy_tssical_apply_pwr_tssi_tbl(pi->tssicali, LUT_pt->txcal_pwr_tssi,
			bphy_tbl);
		phy_tssical_set_txcal_status(pi->tssicali, 1);
		break;
	case 2:
		/* Channel is in between two channels, do interpolation */
		/* ---- need to verify goodness of interpolation */
		wlc_phy_estpwrlut_intpol_acphy(pi, chan_num,
			LUT_pt->txcal_pwr_tssi, LUT_pt->next_chan->txcal_pwr_tssi,
				bphy_tbl);
		phy_tssical_set_txcal_status(pi->tssicali, 2);
		break;
	}

}

static uint16
phy_ac_tssical_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	return phy_utils_read_phyreg(pi, ACPHY_TxPwrCtrlCmd(pi->pubpi.phy_rev));
}

static void
phy_ac_tssical_set_txpwrindex(phy_type_tssical_ctx_t *ctx, int16 gain_idx, uint16 save_TxPwrCtrlCmd)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint32 gidx_core[WLC_TXCORE_MAX];
	uint8 core;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(save_TxPwrCtrlCmd);
	BCM_REFERENCE(stf_shdata);

	/* Set txpwrindex of all cores to be same */
	FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
	   gidx_core[core] = (uint32) gain_idx;
	}
	wlc_phy_iovar_txpwrindex_set(pi, &gidx_core);
	/* Disable HWTXPwrCtrl */
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
}

static void
phy_ac_tssical_restore_txpwr_ctrl_cmd(phy_type_tssical_ctx_t *ctx, uint16 save_TxPwrCtrlCmd,
		uint8 txpwr_ctrl_state)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	phy_utils_write_phyreg(pi, ACPHY_TxPwrCtrlCmd(pi->pubpi.phy_rev),
		save_TxPwrCtrlCmd);
	wlc_phy_txpwrctrl_enable_acphy(pi, txpwr_ctrl_state);
}

static void
phy_ac_tssical_set_olpc_anchor(phy_type_tssical_ctx_t *ctx)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	if (phy_tssical_get_olpc_idx_in_use(pi->tssicali))
		wlc_phy_set_olpc_anchor_acphy(pi);
}

static void
phy_ac_tssical_iov_apply_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx, int int_val)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	if (pi->sh->clk) {
		if (int_val) {
			/* Use Tx cal based pwr_tssi_tbl */
			wlc_phy_apply_pwr_tssi_tble_chan_acphy(pi);
		} else {
			/* Use PA Params */
			phy_tssical_apply_pa_params(pi->tssicali);
		}
	}
	phy_tssical_set_pwr_tssi_tbl_in_use(pi->tssicali, (bool)int_val);
}

static void
phy_ac_tssical_read_est_pwr_lut(phy_type_tssical_ctx_t *ctx, void *output_buff, uint8 core)
{
	phy_ac_tssical_info_t *txcali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = txcali->pi;
	uint16 estpwr[128];
	uint8 tx_pwr_ctrl_state;
	uint32 tbl_len = 128;
	uint32 tbl_offset = 0;
	bool suspend = FALSE;
	wlc_phy_conditional_suspend(pi, &suspend);
	tx_pwr_ctrl_state =  pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core),
		tbl_len, tbl_offset, 16, estpwr);

	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	bcopy(estpwr, output_buff, 128*sizeof(uint16));
	wlc_phy_conditional_resume(pi, &suspend);
}

static void
phy_ac_tssical_apply_paparams(phy_type_tssical_ctx_t *ctx)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint32 tbl_len = 128;
	uint32 tbl_offset = 0;
	uint8 core;
	uint8 tx_pwr_ctrl_state;
	int16 a1[WLC_TXCORE_MAX];
	int16 b0[WLC_TXCORE_MAX];
	int16 b1[WLC_TXCORE_MAX];
	uint8 tssi_val;
	uint16 estpwr[128];
	bool suspend = FALSE;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	tx_pwr_ctrl_state =  pi->txpwrctrl;
	wlc_phy_conditional_suspend(pi, &suspend);
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
	wlc_phy_get_paparams_for_band_acphy(pi, a1, b0, b1);

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		for (tssi_val = 0; tssi_val < 128; tssi_val++) {
			estpwr[tssi_val] = wlc_phy_tssi2dbm_acphy(pi,
					tssi_val, a1[core], b0[core], b1[core]);
			/* Program the upper 8-bits for CCK for 43012 */
			if (ACMAJORREV_36(pi->pubpi->phy_rev))
				estpwr[tssi_val] = (int16) (estpwr[tssi_val]<< 8 |
					estpwr[tssi_val]);
			else if (BF3_TSSI_DIV_WAR(pi_ac) &&
					ACMAJORREV_40(pi->pubpi->phy_rev)) {
				uint8 core2range =
					PHYCORENUM(pi->pubpi->phy_corenum) + core;
				uint16 estpwr_cck = wlc_phy_tssi2dbm_acphy(pi,
					tssi_val, a1[core2range], b0[core2range], b1[core2range]);
				estpwr[tssi_val] |= (uint16)(estpwr_cck & 0xff) << 8;
			}

		}
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core),
			tbl_len, tbl_offset, 16, estpwr);
	}
	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	phy_tssical_set_txcal_status(pi->tssicali, 0);
	wlc_phy_conditional_resume(pi, &suspend);
}

static void
phy_ac_tssical_appy_pwr_tssi_tbl(phy_type_tssical_ctx_t *ctx, wl_txcal_power_tssi_t* txcal_pwr_tssi,
	uint8 bphy_tbl)
{
	phy_ac_tssical_info_t *tssicali = (phy_ac_tssical_info_t *)ctx;
	phy_info_t *pi = tssicali->pi;
	uint8 tx_pwr_ctrl_state =  pi->txpwrctrl;
	bool suspend = FALSE;
	uint8 core;
	uint8 tssi_idx;
	uint16 estpwr_acphy[128];
	uint32 tbl_len = 128;
	uint32 tbl_offset = 0;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	wlc_phy_conditional_suspend(pi, &suspend);
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
	wlc_phy_conditional_resume(pi, &suspend);

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		if (txcal_pwr_tssi->num_entries[core] == 0) {
			/* sanity check */
			phy_tssical_apply_pa_params(pi->tssicali);
		} else {
			phy_tssical_generate_estpwr_lut(txcal_pwr_tssi,
				estpwr_acphy, core);
			/* Program the upper 8-bits for CCK for 43012 */
			if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				for (tssi_idx = 0; tssi_idx < 128; tssi_idx++) {
					estpwr_acphy[tssi_idx] = (uint16)
						(((estpwr_acphy[tssi_idx] & 0x00FF) << 8) |
						(estpwr_acphy[tssi_idx] & 0x00FF));
				}
			}
			wlc_phy_conditional_suspend(pi, &suspend);
			if (!bphy_tbl) {
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_ESTPWRLUTS(core),
					tbl_len, tbl_offset, 16, estpwr_acphy);
			} else {
				uint16 estpwr_tmp[128];
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core), tbl_len,
					tbl_offset, 16, estpwr_tmp);

				for (tssi_idx = 0; tssi_idx < 128; tssi_idx++) {
					estpwr_acphy[tssi_idx]  =
						(uint16) (((estpwr_acphy[tssi_idx] & 0x00FF) << 8) |
							(estpwr_tmp[tssi_idx] & 0x00FF));
				}
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_ESTPWRLUTS(core),
					tbl_len, tbl_offset, 16, estpwr_acphy);
			}

			wlc_phy_conditional_resume(pi, &suspend);
		}
	}
	wlc_phy_conditional_suspend(pi, &suspend);
	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	wlc_phy_conditional_resume(pi, &suspend);
}
#endif /* WLC_TXCAL */
