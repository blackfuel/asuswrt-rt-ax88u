/*
 * ACPHY Rx Spur canceller module implementation
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
 * $Id: phy_ac_rxspur.c 742511 2018-01-22 14:14:24Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include "phy_type_rxspur.h"
#include <phy_ac.h>
#include <phy_ac_rxspur.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phy_shim.h>
#include <wlc_radioreg_20691.h>
#include <wlc_phy_radio.h>
#include <wlc_phytbl_ac.h>
#include <wlc_phytbl_20691.h>
#include <wlc_phyreg_ac.h>

#include <hndpmu.h>
#include <phy_utils_channel.h>
#include <phy_utils_reg.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>

/* add the feature to disable DSSF  0: disable 1: enable */
#define DSSF_ENABLE 1
#define DSSFB_ENABLE 1

/* module private structs */
struct acphy_dssf_values {
	uint16 channel;
	uint8 core;
	uint8 DSSF_gain_th0_s1;
	uint8 DSSF_gain_th1_s1;
	uint8 DSSF_gain_th2_s1;
	uint8 DSSF_gain_th0_s2;
	uint8 DSSF_gain_th1_s2;
	uint8 DSSF_gain_th2_s2;
	uint8 idepth_s1;
	uint8 idepth_s2;
	uint8 enabled_s1;
	uint8 enabled_s2;
	uint16 theta_i_s1;
	uint16 theta_q_s1;
	uint16 theta_i_s2;
	uint16 theta_q_s2;
	uint8 DSSF_C_CTRL;
	bool on;
};

struct acphy_dssfB_values {
	uint16 channel;
	uint8 core;
	uint8 DSSFB_gain_th0_s1;
	uint8 DSSFB_gain_th1_s1;
	uint8 DSSFB_gain_th2_s1;
	uint8 DSSFB_gain_th0_s2;
	uint8 DSSFB_gain_th1_s2;
	uint8 DSSFB_gain_th2_s2;
	uint8 idepth_s1;
	uint8 idepth_s2;
	uint8 enabled_s1;
	uint8 enabled_s2;
	uint16 theta_i_s1;
	uint16 theta_q_s1;
	uint16 theta_i_s2;
	uint16 theta_q_s2;
	uint8 DSSFB_C_CTRL;
	bool on;
};

struct acphy_spurcan_values {
	uint16 channel;
	uint8 core;
	uint8 bw;
	uint8 spurcan_en;
	uint8 s1_en;
	uint16 s1_omega_high;
	uint16 s1_omega_low;
	uint8 s2_en;
	uint16 s2_omega_high;
	uint16 s2_omega_low;
	uint8 s3_en;
	uint16 s3_omega_high;
	uint16 s3_omega_low;

};

/* module private states */
struct phy_ac_rxspur_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_rxspur_info_t *cmn_info;
	acphy_dssf_values_t *dssf_params;
	acphy_dssfB_values_t *dssfB_params;
	acphy_spurcan_values_t *spurcan_params;
	uint16	*spurcan_ChanList;
	int16	*spurcan_SpurFreq;
	uint8	spurcan_NoSpurs;
	uint8	acphy_spuravoid_mode;
	uint8	curr_spurmode;
	int8	acphy_spuravoid_mode_override;
};

static const uint32 acphy_spurcan_spur_freqKHz_rev12[] = {2431000, 2468400,
	5198600, 5236000, 5273400, 5310800, 5497800, 5535200, 5572600, 5647400,
	5684800, 5722200, 5759600, 5797000, 5834400};

/* In 53573 DDR freq of 392MHz introduces a spur at 2432MHz */
/* 53573-RSDB mode will have spur at 2457.778MHz when the aggressor core is in Ch-106q */
static const uint32 acphy_spurcan_spur_freqKHz_router_4349_mimo[] = {2432000};
static const uint32 acphy_spurcan_spur_freqKHz_router_4349_rsdb[] = {2432000, 2457778};

/* local functions */
static int phy_ac_rxspur_std_params(phy_ac_rxspur_info_t *info);
static void phy_ac_set_spurmode(phy_type_rxspur_ctx_t *ctx, uint16 freq);
static void phy_ac_get_spurmode(phy_ac_rxspur_info_t *rxspuri, uint16 freq);
#if defined(WLTEST)
static int phy_ac_rxspur_set_force_spurmode(phy_type_rxspur_ctx_t *ctx, int16 int_val);
static int phy_ac_rxspur_get_force_spurmode(phy_type_rxspur_ctx_t *ctx, int32 *ret_int_ptr);
#endif /* WLTEST */

/* register phy type specific implementation */
phy_ac_rxspur_info_t *
BCMATTACHFN(phy_ac_rxspur_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_rxspur_info_t *cmn_info)
{
	phy_ac_rxspur_info_t *ac_info;
	phy_type_rxspur_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_rxspur_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.set_spurmode = phy_ac_set_spurmode;
#if defined(WLTEST)
	fns.set_force_spurmode = phy_ac_rxspur_set_force_spurmode;
	fns.get_force_spurmode  = phy_ac_rxspur_get_force_spurmode;
#endif /* WLTEST */
	fns.ctx = ac_info;

	if ((ac_info->dssf_params = phy_malloc(pi, sizeof(acphy_dssf_values_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc dssf_params failed\n", __FUNCTION__));
		goto fail;
	}
	if ((ac_info->dssfB_params = phy_malloc(pi, sizeof(acphy_dssfB_values_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc dssfB_params failed\n", __FUNCTION__));
		goto fail;
	}
	if ((ac_info->spurcan_params = phy_malloc(pi, sizeof(acphy_spurcan_values_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc spurcan_params failed\n", __FUNCTION__));
		goto fail;
	}

	if (phy_ac_rxspur_std_params(ac_info) != BCME_OK) {
		PHY_ERROR(("%s: phy_ac_rxspur_std_params failed\n", __FUNCTION__));
		goto fail;
	}

	if (phy_rxspur_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxspur_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	phy_ac_rxspur_unregister_impl(ac_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_rxspur_unregister_impl)(phy_ac_rxspur_info_t *ac_info)
{
	phy_info_t *pi;
	phy_rxspur_info_t *cmn_info;

	if (ac_info == NULL) {
		return;
	}

	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_rxspur_unregister_impl(cmn_info);

	if (ac_info->spurcan_params != NULL) {
		phy_mfree(pi, ac_info->spurcan_params, sizeof(acphy_spurcan_values_t));
	}
	if (ac_info->dssfB_params != NULL) {
		phy_mfree(pi, ac_info->dssfB_params, sizeof(acphy_dssfB_values_t));
	}
	if (ac_info->dssf_params != NULL) {
		phy_mfree(pi, ac_info->dssf_params, sizeof(acphy_dssf_values_t));
	}

	phy_mfree(pi, ac_info, sizeof(phy_ac_rxspur_info_t));
}

static int
BCMATTACHFN(phy_ac_rxspur_std_params)(phy_ac_rxspur_info_t *rxspuri)
{
	phy_info_t *pi = rxspuri->pi;
	uint8 i;
	uint16 *Sp_chanlist;
	int16 *Sp_freqlist;

	rxspuri->curr_spurmode = 0;
	if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID) {
		if (pi->sh->chippkg == BCM4335_WLBGA_PKG_ID) {
			rxspuri->curr_spurmode = 8;
			rxspuri->acphy_spuravoid_mode = 8;
		} else {
			/* for WLCSP packages */
			if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
				/* for 4335 Cx Chips */
				rxspuri->curr_spurmode = 8;
				rxspuri->acphy_spuravoid_mode = 8;
			} else {
				/* for 4335 Ax/Bx Chips */
				rxspuri->curr_spurmode = 2;
				rxspuri->acphy_spuravoid_mode = 2;
			}
		}
		if (PHY_XTAL_IS52M(pi)) {
			rxspuri->curr_spurmode = 0;
			rxspuri->acphy_spuravoid_mode = 0;
		}
	}
	rxspuri->acphy_spuravoid_mode_override = 0;

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		rxspuri->spurcan_NoSpurs  = (uint8)PHY_GETINTVAR_DEFAULT(pi,
			rstr_spurcan_numspur, 0);
		if (rxspuri->spurcan_NoSpurs != 0) {
			rxspuri->spurcan_ChanList = phy_malloc(pi,
					rxspuri->spurcan_NoSpurs * sizeof(uint16));
			rxspuri->spurcan_SpurFreq = phy_malloc(pi,
					rxspuri->spurcan_NoSpurs * sizeof(int16));

			if ((rxspuri->spurcan_ChanList == NULL) ||
				(rxspuri->spurcan_SpurFreq == NULL)) {
				return BCME_NOMEM;
			}

			Sp_chanlist = rxspuri->spurcan_ChanList;
			Sp_freqlist = rxspuri->spurcan_SpurFreq;
			for (i = 0; i < rxspuri->spurcan_NoSpurs; i++) {
				Sp_chanlist[i] = (uint16)PHY_GETINTVAR_ARRAY_DEFAULT
						(pi, rstr_spurcan_chlist, i, 0);
				Sp_freqlist[i] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT
						(pi, rstr_spurcan_spfreq, i, 0);
			}
		}
	}

	return BCME_OK;
}
/* ******************************************** */
/*		Internal Definitions		*/
/* ******************************************** */

static void
phy_ac_setup_spurmode(phy_ac_rxspur_info_t *rxspuri)
{
	phy_info_t *pi = rxspuri->pi;

	si_pmu_spuravoid(pi->sh->sih, pi->sh->osh, rxspuri->acphy_spuravoid_mode);
	wlapi_switch_macfreq(pi->sh->physhim, rxspuri->acphy_spuravoid_mode);
}

static void phy_ac_spurcan_clk(phy_info_t *pi, bool enable)
{
	MOD_PHYREG(pi, forceFront0, spurcan_clk_en_slms0, enable);
	MOD_PHYREG(pi, forceFront0, spurcan_clk_en_slms1, enable);
}

static void
phy_ac_spurcan_setup(phy_ac_rxspur_info_t *rxspuri, bool enable)
{
	phy_info_t *pi = rxspuri->pi;
	acphy_spurcan_values_t *spurcan = rxspuri->spurcan_params;
	uint8 core = spurcan->core;

	if (enable) {
		phy_ac_spurcan_clk(pi, enable);
		MOD_PHYREG(pi, spur_can_phy_bw_mhz, spur_can_phy_bw_mhz, spurcan->bw);
		if (ROUTER_4349(pi)) {
			MOD_PHYREG(pi, spur_can_P_sp_min, spur_can_P_sp_min_dbm_slms, 0x8f);
		}
		MOD_PHYREGCM(pi, spur_can_p, s1_en, core, spur_can_kf_enable, 0);
		MOD_PHYREGCM(pi, spur_can_p, s1_omega_high, core,
			spur_can_omega_high, spurcan->s1_omega_high);
		MOD_PHYREGCM(pi, spur_can_p, s1_omega_low, core,
			spur_can_omega_low, spurcan->s1_omega_low);
		MOD_PHYREGCM(pi, spur_can_p, s1_en, core,
			spur_can_stage_enable, spurcan->s1_en);
		MOD_PHYREGCM(pi, spur_can_p, s2_omega_high, core,
			spur_can_omega_high, spurcan->s2_omega_high);
		MOD_PHYREGCM(pi, spur_can_p, s2_omega_low, core,
			spur_can_omega_low, spurcan->s2_omega_low);
		MOD_PHYREGCM(pi, spur_can_p, s2_en, core,
			spur_can_stage_enable, spurcan->s2_en);
		MOD_PHYREGCM(pi, spur_can_p, s3_omega_high, core,
			spur_can_omega_high, spurcan->s3_omega_high);
		MOD_PHYREGCM(pi, spur_can_p, s3_omega_low, core,
			spur_can_omega_low, spurcan->s3_omega_low);
		MOD_PHYREGCM(pi, spur_can_p, s3_en, core,
			spur_can_stage_enable, spurcan->s3_en);
		MOD_PHYREG(pi, spur_can_en, spur_can_enable,
			spurcan->spurcan_en);
	} else {
		MOD_PHYREG(pi, spur_can_en, spur_can_enable, 0);
		MOD_PHYREGCM(pi, spur_can_p, s1_en, core, spur_can_stage_enable, 0);
		MOD_PHYREGCM(pi, spur_can_p, s2_en, core, spur_can_stage_enable, 0);
		MOD_PHYREGCM(pi, spur_can_p, s3_en, core, spur_can_stage_enable, 0);
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			phy_ac_spurcan_clk(pi, 0);
		} else {
			phy_ac_spurcan_clk(pi, enable);
		}
	}
}

#ifndef WL_DSSF_DISABLED
static void
phy_ac_dssf_setup(phy_ac_rxspur_info_t *rxspuri)
{
	phy_info_t *pi = rxspuri->pi;
	acphy_dssf_values_t *dssf = rxspuri->dssf_params;

	WRITE_PHYREG(pi, DSSF_C_CTRL, dssf->DSSF_C_CTRL);
	WRITE_PHYREGCE(pi, DSSF_gain_th0_s1, dssf->core, dssf->DSSF_gain_th0_s1);
	WRITE_PHYREGCE(pi, DSSF_gain_th1_s1, dssf->core, dssf->DSSF_gain_th1_s1);
	WRITE_PHYREGCE(pi, DSSF_gain_th2_s1, dssf->core, dssf->DSSF_gain_th2_s1);
	WRITE_PHYREGCE(pi, DSSF_gain_th0_s2, dssf->core, dssf-> DSSF_gain_th0_s2);
	WRITE_PHYREGCE(pi, DSSF_gain_th1_s2, dssf->core, dssf->DSSF_gain_th1_s2);
	WRITE_PHYREGCE(pi, DSSF_gain_th2_s2, dssf->core, dssf->DSSF_gain_th2_s2);
	WRITE_PHYREGCE(pi, DSSF_exp_j_theta_i_s1, dssf->core, dssf->theta_i_s1);
	WRITE_PHYREGCE(pi, DSSF_exp_i_theta_q_s1, dssf->core, dssf->theta_q_s1);
	WRITE_PHYREGCE(pi, DSSF_exp_j_theta_i_s2, dssf->core, dssf->theta_i_s2);
	WRITE_PHYREGCE(pi, DSSF_exp_i_theta_q_s2, dssf->core, dssf->theta_q_s2);

	if (dssf->core == 0) {
		MOD_PHYREG(pi, DSSF_control_0, idepth_s1, dssf->idepth_s1);
		MOD_PHYREG(pi, DSSF_control_0, idepth_s2, dssf->idepth_s2);
		MOD_PHYREG(pi, DSSF_control_0, enabled_s1, dssf->enabled_s1);
		MOD_PHYREG(pi, DSSF_control_0, enabled_s2, dssf->enabled_s2);
	} else if (dssf->core == 1) {
		MOD_PHYREG(pi, DSSF_control_1, idepth_s1, dssf->idepth_s1);
		MOD_PHYREG(pi, DSSF_control_1, idepth_s2, dssf->idepth_s2);
		MOD_PHYREG(pi, DSSF_control_1, enabled_s1, dssf->enabled_s1);
		MOD_PHYREG(pi, DSSF_control_1, enabled_s2, dssf->enabled_s2);
	}
}
#endif /* WL_DSSF_DISABLED */

static void
phy_ac_dssfB_setup(phy_ac_rxspur_info_t *rxspuri)
{
	phy_info_t *pi = rxspuri->pi;
	acphy_dssfB_values_t *dssfB = rxspuri->dssfB_params;

	WRITE_PHYREG(pi, DSSFB_C_CTRL, dssfB->DSSFB_C_CTRL);
	WRITE_PHYREG(pi, DSSFB_gain_th0_s1,      dssfB->DSSFB_gain_th0_s1);
	WRITE_PHYREG(pi, DSSFB_gain_th1_s1,      dssfB->DSSFB_gain_th1_s1);
	WRITE_PHYREG(pi, DSSFB_gain_th2_s1,      dssfB->DSSFB_gain_th2_s1);
	WRITE_PHYREG(pi, DSSFB_gain_th0_s2,      dssfB->DSSFB_gain_th0_s2);
	WRITE_PHYREG(pi, DSSFB_gain_th1_s2,      dssfB->DSSFB_gain_th1_s2);
	WRITE_PHYREG(pi, DSSFB_gain_th2_s2,      dssfB->DSSFB_gain_th2_s2);
	WRITE_PHYREG(pi, DSSFB_exp_j_theta_i_s1, dssfB->theta_i_s1);
	WRITE_PHYREG(pi, DSSFB_exp_i_theta_q_s1, dssfB->theta_q_s1);
	WRITE_PHYREG(pi, DSSFB_exp_j_theta_i_s2, dssfB->theta_i_s2);
	WRITE_PHYREG(pi, DSSFB_exp_i_theta_q_s2, dssfB->theta_q_s2);

	MOD_PHYREG(pi, DSSFB_control, idepth_s1,  dssfB->idepth_s1);
	MOD_PHYREG(pi, DSSFB_control, idepth_s2,  dssfB->idepth_s2);
	MOD_PHYREG(pi, DSSFB_control, enabled_s1, dssfB->enabled_s1);
	MOD_PHYREG(pi, DSSFB_control, enabled_s2, dssfB->enabled_s2);
}

static bool
chanspec_bbpll_parr_enable(phy_info_t *pi)
{
	return ((pi->sh->chippkg == BCM4335_WLBGA_PKG_ID &&
		((CHIPID(pi->sh->chip) == BCM4345_CHIP_ID &&
		!CST4345_CHIPMODE_USB20D(pi->sh->sih->chipst)))) ||
		(CHIPID(pi->sh->chip) == BCM4335_CHIP_ID &&
		pi->sh->chippkg == BCM4335_FCBGA_PKG_ID));
}

/* ******************************************** */
/*		External Definitions		*/
/* ******************************************** */

void
chanspec_bbpll_parr(phy_ac_rxspur_info_t *rxspuri, uint32 *bbpll_parr_in, bool state)
{
	/* input : bbpll_parr_in
	 * 0 : min_res_mask
	 * 1 : max_res_mask
	 * 2 : clk_ctl_st
	 */

#ifdef BBPLL_PARR
	phy_info_t *pi = rxspuri->pi;
	uint32 min_res_mask = 0, max_res_mask = 0, clk_ctl_st = 0;
#endif // endif

	if (!chanspec_bbpll_parr_enable(rxspuri->pi))
		return;

#ifdef BBPLL_PARR
	min_res_mask = bbpll_parr_in[0];
	max_res_mask = bbpll_parr_in[1];
	clk_ctl_st = bbpll_parr_in[2];

	if (state == OFF) {
		/* power down BBPLL */
		phy_ac_get_spurmode(rxspuri,
			(uint16) phy_ac_radio_get_data(rxspuri->aci->radioi)->fc);
		if ((rxspuri->curr_spurmode != rxspuri->acphy_spuravoid_mode)) {
			si_pmu_pll_off_PARR(pi->sh->sih, pi->sh->osh,
				&min_res_mask, &max_res_mask, &clk_ctl_st);
		}
	} else {
		/* update and power up BBPLL */
		if (rxspuri->curr_spurmode != rxspuri->acphy_spuravoid_mode) {
			rxspuri->curr_spurmode =  rxspuri->acphy_spuravoid_mode;
			si_pmu_spuravoid_isdone(pi->sh->sih, pi->sh->osh, min_res_mask,
				max_res_mask, clk_ctl_st, rxspuri->acphy_spuravoid_mode);
			wlapi_switch_macfreq(pi->sh->physhim, rxspuri->acphy_spuravoid_mode);
		}
	}

	bbpll_parr_in[0] = min_res_mask;
	bbpll_parr_in[1] = max_res_mask;
	bbpll_parr_in[2] = clk_ctl_st;
#endif /* BBPLL_PARR */
	return;
}

void
phy_ac_spurcan(phy_ac_rxspur_info_t *rxspuri, bool enable)
{
	phy_info_t *pi = rxspuri->pi;
	phy_info_acphy_t *pi_ac = rxspuri->aci;
	acphy_spurcan_values_t *spurcan = rxspuri->spurcan_params;
	uint8 i, core;
	uint8 num_spurs = 0;
	bool enable_final;
	uint8 tbl_len = 0;
	uint16 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint32 fc_KHz, omega;
	int bw = CHSPEC_BW_LE20(pi->radio_chanspec) ?
		20000 : CHSPEC_IS40(pi->radio_chanspec) ? 40000 : 80000;
	int freq;
	int fsp, sign;
	const uint32 *spurfreq = NULL;
	uint16 *Sp_chanlist;
	int16 *Sp_freqlist;

	freq = wf_channel2mhz(channel, CHSPEC_IS5G(pi->radio_chanspec) ?
		WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	fc_KHz = freq*1000;

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		if (ROUTER_4349(pi)) {
			if (phy_get_phymode(pi) == PHYMODE_RSDB) {
				spurfreq = acphy_spurcan_spur_freqKHz_router_4349_rsdb;
				tbl_len = ARRAYSIZE(acphy_spurcan_spur_freqKHz_router_4349_rsdb);
			} else if (phy_get_phymode(pi) == PHYMODE_MIMO) {
				spurfreq = acphy_spurcan_spur_freqKHz_router_4349_mimo;
				tbl_len = ARRAYSIZE(acphy_spurcan_spur_freqKHz_router_4349_mimo);
			}
		} else {
			spurfreq = acphy_spurcan_spur_freqKHz_rev12;
			tbl_len = ARRAYSIZE(acphy_spurcan_spur_freqKHz_rev12);
		}
		num_spurs = 1;	/* Use stage2 and stage3 */

		/* Make default to 0x6E 110 later change the value if there is spur */
		MOD_PHYREG(pi, overideDigiGain1, cckdigigainEnCntValue, 0x6E);

	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		tbl_len = rxspuri->spurcan_NoSpurs;
		num_spurs = 1;	/* Use stage2 and stage3 */
		MOD_PHYREG(pi, overideDigiGain1, cckdigigainEnCntValue, 0xAF);
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		// Have to add spurs list if we plan to enable
		spurfreq = NULL;
	}
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		Sp_chanlist = (rxspuri->spurcan_ChanList);
		Sp_freqlist = (rxspuri->spurcan_SpurFreq);
		if (rxspuri->spurcan_NoSpurs != 0) {
			bzero(spurcan, sizeof(acphy_spurcan_values_t));
			spurcan->bw = bw/1000;
			for (i = 0; i < tbl_len; i++) {
				if (freq == Sp_chanlist[i]) {
					// Spur frequency in KHz
					fsp = Sp_freqlist[i];
					spurcan->spurcan_en = 1;
					num_spurs = num_spurs + 1;
					sign = (fsp > 0) - (fsp < 0);
					math_uint64_divide(&omega, sign * fsp, 0, bw);
					omega = sign * omega;
					if (num_spurs == 1) {
						spurcan->s1_en = 1;
						spurcan->s1_omega_high = omega >> 16;
						spurcan->s1_omega_low = omega & 0x0000ffff;
					} else if (num_spurs == 2) {
						spurcan->s2_en = 1;
						spurcan->s2_omega_high = omega >> 16;
						spurcan->s2_omega_low = omega & 0x0000ffff;
					} else if (num_spurs == 3) {
						spurcan->s3_en = 1;
						spurcan->s3_omega_high = omega >> 16;
						spurcan->s3_omega_low = omega & 0x0000ffff;
						break;
					}
				}
			}
			if ((num_spurs <= 1) || (!enable)) {
				enable_final = FALSE;
			} else {
				enable_final = TRUE;
			}
			phy_ac_spurcan_setup(pi_ac->rxspuri, enable_final);
			num_spurs = 1;
		}
	} else if (spurfreq != NULL) {
		FOREACH_CORE(pi, core) {
			bzero(spurcan, sizeof(acphy_spurcan_values_t));
			spurcan->channel = channel;
			spurcan->core = core;
			spurcan->bw = bw/1000;
			for (i = 0; i < tbl_len; i++) {
				fsp = spurfreq[i] - fc_KHz;

				if ((ROUTER_4349(pi) ||
					ACMAJORREV_44_46(pi->pubpi->phy_rev)) && !fsp) {
					continue;
				}

				if (((-bw/2) < fsp) && (fsp < (bw/2))) {
					spurcan->spurcan_en = 1;
					num_spurs = num_spurs + 1;
					sign = (fsp > 0) - (fsp < 0);
					math_uint64_divide(&omega, sign * fsp, 0, bw);
					omega = sign * omega;
					if (num_spurs == 1) {
						spurcan->s1_en = 1;
						spurcan->s1_omega_high = omega >> 16;
						spurcan->s1_omega_low = omega & 0x0000ffff;
					} else if (num_spurs == 2) {
						spurcan->s2_en = 1;
						spurcan->s2_omega_high = omega >> 16;
						spurcan->s2_omega_low = omega & 0x0000ffff;
					} else if (num_spurs == 3) {
						spurcan->s3_en = 1;
						spurcan->s3_omega_high = omega >> 16;
						spurcan->s3_omega_low = omega & 0x0000ffff;
						break;
					}
					if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
						MOD_PHYREG(pi, overideDigiGain1,
							cckdigigainEnCntValue, 0x82);
					}
					if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
						MOD_PHYREG(pi, overideDigiGain1,
							cckdigigainEnCntValue, 0xAF);
					}

				}
			}
			if ((num_spurs <= 1) || (!enable)) {
				enable_final = FALSE;
			} else {
				enable_final = TRUE;
			}
			phy_ac_spurcan_setup(pi_ac->rxspuri, enable_final);
			num_spurs = 1;
		}
	} else {
		// spurfreq is NULL
		FOREACH_CORE(pi, core) {
			bzero(spurcan, sizeof(acphy_spurcan_values_t));
			spurcan->channel = channel;
			spurcan->core = core;
			spurcan->bw = bw/1000;
			phy_ac_spurcan_setup(pi_ac->rxspuri, FALSE);
		}
	}
}

void
phy_ac_dssf(phy_ac_rxspur_info_t *rxspuri, bool on)
{
#ifndef WL_DSSF_DISABLED
	phy_info_t *pi = rxspuri->pi;
	phy_info_acphy_t *pi_ac = rxspuri->aci;
	acphy_dssf_values_t *dssf = rxspuri->dssf_params;

	if (!(((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
		ACMAJORREV_3(pi->pubpi->phy_rev)) &&
		PHY_ILNA(pi)) && !(ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev))) {
		return;
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_36(pi->pubpi->phy_rev) ||
		ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		return;
	}

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) &&
		!(CHSPEC_IS2G(pi->radio_chanspec) && BF2_2G_SPUR_WAR(pi_ac))) {
		return;
	}

	dssf->channel = 0;
	dssf->core = 0;

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		dssf->DSSF_gain_th0_s1 = 46;
		dssf->DSSF_gain_th1_s1 = 52;
		dssf->DSSF_gain_th2_s1 = 58;
		dssf->DSSF_gain_th0_s2 = 46;
		dssf->DSSF_gain_th1_s2 = 52;
		dssf->DSSF_gain_th2_s2 = 58;
	} else if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi) &&
	           PHY_IPA(pi) && PHY_XTAL_IS40M(pi)) {
		/* There is only 1 gain used in 5G BW80, so no waste lines here */
		if (CHSPEC_IS40(pi->radio_chanspec)) {
			dssf->DSSF_gain_th0_s1 = 60;
			dssf->DSSF_gain_th1_s1 = 66;
			dssf->DSSF_gain_th2_s1 = 72;
			dssf->DSSF_gain_th0_s2 = 60;
			dssf->DSSF_gain_th1_s2 = 66;
			dssf->DSSF_gain_th2_s2 = 72;
		} else {
			dssf->DSSF_gain_th0_s1 = 62;
			dssf->DSSF_gain_th1_s1 = 73;
			dssf->DSSF_gain_th2_s1 = 78;
			dssf->DSSF_gain_th0_s2 = 62;
			dssf->DSSF_gain_th1_s2 = 73;
			dssf->DSSF_gain_th2_s2 = 78;
		}
	} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		dssf->DSSF_gain_th0_s1 = 0;
		dssf->DSSF_gain_th1_s1 = 0;
		dssf->DSSF_gain_th2_s1 = 0;
		dssf->DSSF_gain_th0_s2 = 0;
		dssf->DSSF_gain_th1_s2 = 0;
		dssf->DSSF_gain_th2_s2 = 0;
	} else {
		dssf->DSSF_gain_th0_s1 = 68;
		dssf->DSSF_gain_th1_s1 = 74;
		dssf->DSSF_gain_th2_s1 = 80;
		dssf->DSSF_gain_th0_s2 = 68;
		dssf->DSSF_gain_th1_s2 = 74;
		dssf->DSSF_gain_th2_s2 = 80;
	}

	dssf->idepth_s1 = 2;
	dssf->idepth_s2 = 2;
	dssf->enabled_s1 = 1;
	dssf->enabled_s2 = 0;
	dssf->theta_i_s1 = 0;
	dssf->theta_q_s1 = 0;
	dssf->theta_i_s2 = 0;
	dssf->theta_q_s2 = 0;
	dssf->DSSF_C_CTRL = 0;
	dssf->on = on;

	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

	/* Reset DSSF filter */
	WRITE_PHYREG(pi, DSSF_C_CTRL, 0);

	if (on) {
		dssf->channel = CHSPEC_CHANNEL(pi->radio_chanspec);

		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			dssf->idepth_s1 = 0;
			dssf->idepth_s2 = 0;
			dssf->enabled_s1 = 1;
			dssf->enabled_s2 = 0;
			dssf->DSSF_C_CTRL = 4;
			switch (dssf->channel) {
				case 3:
					/* bw=20, fc= 2422 MHz */
					/* xtal harmonic freq =  +9 MHz */
					dssf->theta_i_s1 =  4298;
					dssf->theta_q_s1 =  1265;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 4:
					/* bw=20, fc= 2427 MHz */
					/* xtal harmonic freq =  +4 MHz */
					dssf->theta_i_s1 =  1265;
					dssf->theta_q_s1 =  3894;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 5:
					/* bw=20, fc= 2432 MHz */
					/* xtal harmonic freq =  -1 MHz */
					dssf->theta_i_s1 =  3894;
					dssf->theta_q_s1 =  6927;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 6:
					/* bw=20, fc= 2437 MHz */
					/* xtal harmonic freq =  -6 MHz */
					dssf->theta_i_s1 =  6927;
					dssf->theta_q_s1 =  4298;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 11:
					/* bw=20, fc= 2462 MHz */
					/* xtal harmonic freq =  +6.4 MHz */
					dssf->theta_i_s1 =  6449;
					dssf->theta_q_s1 =  3705;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 12:
					/* bw=20, fc= 2467 MHz */
					/* xtal harmonic freq =  +1.4 MHz */
					dssf->theta_i_s1 =  3705;
					dssf->theta_q_s1 =  1743;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				case 13:
					/* bw=20, fc= 2472 MHz */
					/* xtal harmonic freq =  -3.6 MHz */
					dssf->theta_i_s1 =  1743;
					dssf->theta_q_s1 =  4487;
					dssf->DSSF_gain_th0_s1 = 55;
					dssf->DSSF_gain_th1_s1 = 100;
					dssf->DSSF_gain_th2_s1 = 100;
					break;
				default:
				;
			}
		} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			switch (dssf->channel) {
			case 1:
				/* bw=20, fc= 2412 MHz */
				dssf->theta_i_s1 =   641; /* freq =  -4.50 (bbpll) */
				dssf->theta_q_s1 =  4147; /* freq =  -4.50 (bbpll) */
				dssf->DSSF_gain_th0_s1 = 40;
				dssf->DSSF_gain_th1_s1 = 46;
				dssf->DSSF_gain_th2_s1 = 52;
				break;
			case 4:
				/* bw=20, fc= 2427 MHz */
				dssf->theta_i_s1 =  1265; /* freq =  +4.00 (xtal) */
				dssf->theta_q_s1 =  3895; /* freq =  +4.00 (xtal) */
				break;
			case 5:
				/* bw=20, fc= 2432 MHz */
				dssf->theta_i_s1 =  3895; /* freq =  -1.00 (xtal) */
				dssf->theta_q_s1 =  6927; /* freq =  -1.00 (xtal) */
				break;
			case 6:
				/* bw=20, fc= 2437 MHz */
				dssf->theta_i_s1 =  6927; /* freq =  -6.00 (xtal) */
				dssf->theta_q_s1 =  4297; /* freq =  -6.00 (xtal) */
				dssf->DSSF_gain_th0_s1 = 49;
				dssf->DSSF_gain_th1_s1 = 55;
				dssf->DSSF_gain_th2_s1 = 61;
				break;
			case 11:
				/* bw=20, fc= 2462 MHz */
				dssf->theta_i_s1 =  6448; /* freq =  +6.40 (xtal) */
				dssf->theta_q_s1 =  3705; /* freq =  +6.40 (xtal) */
				break;
			case 12:
				/* bw=20, fc= 2467 MHz */
				dssf->theta_i_s1 =  3705; /* freq =  +1.40 (xtal) */
				dssf->theta_q_s1 =  1744; /* freq =  +1.40 (xtal) */
				break;
			case 13:
				/* bw=20, fc= 2472 MHz */
				dssf->theta_i_s1 =  1744; /* freq =  -3.60 (xtal) */
				dssf->theta_q_s1 =  4487; /* freq =  -3.60 (xtal) */
				break;
			case 40:
				/* bw=20, fc= 5200 MHz */
				dssf->theta_i_s1 =  3705; /* freq =  -1.40 (xtal) */
				dssf->theta_q_s1 =  6448; /* freq =  -1.40 (xtal) */
				break;
			case 48:
				/* bw=20, fc= 5240 MHz */
				dssf->theta_i_s1 =  1265; /* freq =  -4.00 (xtal) */
				dssf->theta_q_s1 =  4297; /* freq =  -4.00 (xtal) */
				break;
			case 56:
				/* bw=20, fc= 5280 MHz */
				dssf->theta_i_s1 =  6219; /* freq =  -6.60 (xtal) */
				dssf->theta_q_s1 =  4604; /* freq =  -6.60 (xtal) */
				break;
			case 60:
				/* bw=20, fc= 5300 MHz */
				dssf->theta_i_s1 =  1859; /* freq =  -3.50 (bbpll) */
				dssf->theta_q_s1 =  4543; /* freq =  -3.50 (bbpll) */
				dssf->DSSF_gain_th0_s1 = 43;
				dssf->DSSF_gain_th1_s1 = 49;
				dssf->DSSF_gain_th2_s1 = 55;
				break;
			case 100:
				/* bw=20, fc= 5500 MHz */
				dssf->theta_i_s1 =  3155; /* freq =  -2.20 (xtal) */
				dssf->theta_q_s1 =  5582; /* freq =  -2.20 (xtal) */
				break;
			case 108:
				/* bw=20, fc= 5540 MHz */
				dssf->theta_i_s1 =   257; /* freq =  -4.80 (xtal) */
				dssf->theta_q_s1 =  4105; /* freq =  -4.80 (xtal) */
				break;
			case 116:
				/* bw=20, fc= 5580 MHz */
				dssf->theta_i_s1 =  5389; /* freq =  -7.40 (xtal) */
				dssf->theta_q_s1 =  5207; /* freq =  -7.40 (xtal) */
				break;
			case 128:
				/* bw=20, fc= 5640 MHz */
				dssf->theta_i_s1 =  5389; /* freq =  +7.40 (xtal) */
				dssf->theta_q_s1 =  2985; /* freq =  +7.40 (xtal) */
				break;
			case 130:
				/* bw=20, fc= 5650 MHz */
				dssf->theta_i_s1 =  2803; /* freq =  -2.60 (xtal) */
				dssf->theta_q_s1 =  5207; /* freq =  -2.60 (xtal) */
				dssf->DSSF_gain_th0_s1 = 43;
				dssf->DSSF_gain_th1_s1 = 49;
				dssf->DSSF_gain_th2_s1 = 55;
				break;
			case 136:
				/* bw=20, fc= 5680 MHz */
				dssf->theta_i_s1 =   257; /* freq =  +4.80 (xtal) */
				dssf->theta_q_s1 =  4087; /* freq =  +4.80 (xtal) */
				break;
			case 144:
				/* bw=20, fc= 5720 MHz */
				dssf->theta_i_s1 =  3155; /* freq =  +2.20 (xtal) */
				dssf->theta_q_s1 =  2610; /* freq =  +2.20 (xtal) */
				break;
			case 153:
				/* bw=20, fc= 5765 MHz */
				dssf->theta_i_s1 =  7679; /* freq =  -5.40 (xtal) */
				dssf->theta_q_s1 =  4129; /* freq =  -5.40 (xtal) */
				dssf->DSSF_gain_th0_s1 = 43;
				dssf->DSSF_gain_th1_s1 = 49;
				dssf->DSSF_gain_th2_s1 = 55;
				break;
			case 161:
				/* bw=20, fc= 5805 MHz */
				dssf->theta_i_s1 =  4879; /* freq =  -8.00 (xtal) */
				dssf->theta_q_s1 =  5785; /* freq =  -8.00 (xtal) */
				break;
			case 165:
				/* bw=20, fc= 5825 MHz */
				dssf->theta_i_s2 =  4170; /* freq =  +9.40 (xtal) */
				dssf->theta_q_s2 =   767; /* freq =  +9.40 (xtal) */
				break;
			case 38:
				/* bw=40, fc= 5190 MHz */
				dssf->theta_i_s1 =   893; /* freq =  +8.60 (xtal) */
				dssf->theta_q_s1 =  3996; /* freq =  +8.60 (xtal) */
				break;
			case 46:
				/* bw=40, fc= 5230 MHz */
				dssf->theta_i_s1 =  2407; /* freq =  +6.00 (xtal) */
				dssf->theta_q_s1 =  3313; /* freq =  +6.00 (xtal) */
				break;
			case 102:
				/* bw=40, fc= 5510 MHz */
				dssf->theta_i_s1 =  6805; /* freq = -12.20 (xtal) */
				dssf->theta_q_s1 =  4339; /* freq = -12.20 (xtal) */
				break;
			case 110:
				/* bw=40, fc= 5550 MHz */
				dssf->theta_i_s1 =  5389; /* freq = -14.80 (xtal) */
				dssf->theta_q_s1 =  5207; /* freq = -14.80 (xtal) */
				break;
			case 118:
				/* bw=40, fc= 5590 MHz */
				dssf->theta_i_s1 =  4434; /* freq = -17.40 (xtal) */
				dssf->theta_q_s1 =  6566; /* freq = -17.40 (xtal) */
				break;
			case 126:
				/* bw=40, fc= 5630 MHz */
				dssf->theta_i_s1 =  4434; /* freq = +17.40 (xtal) */
				dssf->theta_q_s1 =  1626; /* freq = +17.40 (xtal) */
				break;
			case 134:
				/* bw=40, fc= 5670 MHz */
				dssf->theta_i_s1 =  5389; /* freq = +14.80 (xtal) */
				dssf->theta_q_s1 =  2985; /* freq = +14.80 (xtal) */
				break;
			case 142:
				/* bw=40, fc= 5710 MHz */
				dssf->theta_i_s1 =  6805; /* freq = +12.20 (xtal) */
				dssf->theta_q_s1 =  3853; /* freq = +12.20 (xtal) */
				break;
			case 151:
				/* bw=40, fc= 5755 MHz */
				dssf->theta_i_s1 =  3072; /* freq =  +4.60 (xtal) */
				dssf->theta_q_s1 =  2708; /* freq =  +4.60 (xtal) */
				break;
			case 159:
				/* bw=40, fc= 5795 MHz */
				dssf->theta_i_s1 =  3895; /* freq =  +2.00 (xtal) */
				dssf->theta_q_s1 =  1265; /* freq =  +2.00 (xtal) */

				dssf->theta_i_s2 =  4543; /* freq = -17.00 (bbpll) */
				dssf->theta_q_s2 =  6333; /* freq = -17.00 (bbpll) */
				break;
			case 42:
				/* bw=80, fc= 5210 MHz */
				dssf->theta_i_s1 =  2560; /* freq = -11.40 (xtal) */
				dssf->theta_q_s1 =  4996; /* freq = -11.40 (xtal) */

				dssf->theta_i_s2 =  6333; /* freq = +26.00 (xtal) */
				dssf->theta_q_s2 =  3649; /* freq = +26.00 (xtal) */
				break;
			case 58:
				/* bw=80, fc= 5290 MHz */
				dssf->theta_i_s1 =  1081; /* freq = -16.60 (xtal) */
				dssf->theta_q_s1 =  4242; /* freq = -16.60 (xtal) */

				dssf->theta_i_s2 =  7935; /* freq = +20.80 (xtal) */
				dssf->theta_q_s2 =  4087; /* freq = +20.80 (xtal) */
				break;
			case 106:
				/* bw=80, fc= 5530 MHz */
				dssf->theta_i_s1 =  4842; /* freq = -32.20 (xtal) */
				dssf->theta_q_s1 =  5837; /* freq = -32.20 (xtal) */

				dssf->theta_i_s2 =  3758; /* freq =  +5.20 (xtal) */
				dssf->theta_q_s2 =  1626; /* freq =  +5.20 (xtal) */
				break;

			case 122:
				/* bw=80, fc= 5610 MHz */
				dssf->theta_i_s1 =  4182; /* freq = -37.40 (xtal) */
				dssf->theta_q_s1 =  7362; /* freq = -37.40 (xtal) */

				dssf->theta_i_s2 =  4182; /* freq = +37.40 (xtal) */
				dssf->theta_q_s2 =   830; /* freq = +37.40 (xtal) */
				break;
			case 138:
				/* bw=80, fc= 5690 MHz */
				dssf->theta_i_s1 =  4842; /* freq = +32.20 (xtal) */
				dssf->theta_q_s1 =  2355; /* freq = +32.20 (xtal) */

				dssf->theta_i_s2 =  3758; /* freq =  -5.20 (xtal) */
				dssf->theta_q_s2 =  6566; /* freq =  -5.20 (xtal) */
				break;
			case 155:
				/* bw=80, fc= 5775 MHz */
				dssf->theta_i_s1 =  7551; /* freq = +22.00 (xtal) */
				dssf->theta_q_s1 =  4045; /* freq = +22.00 (xtal) */

				dssf->theta_i_s2 =  1447; /* freq = -15.40 (xtal) */
				dssf->theta_q_s2 =  4361; /* freq = -15.40 (xtal) */
				break;
			default:
				;
			}

			if (dssf->theta_i_s1 || dssf->theta_i_s2) {
				dssf->DSSF_C_CTRL = 4;

				if (dssf->theta_i_s2)
					dssf->enabled_s2 = 1;

				PHY_INFORM(("%s: xtal freq 37.4M, applying dssf for channel %d\n",
				           __FUNCTION__, dssf->channel));
			}
		} else if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
			if (PHY_IPA(pi) && PHY_XTAL_IS40M(pi)) {
				switch (dssf->channel) {
				/* 2G, 20Mhz && 40Mhz */
				case 1:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->idepth_s1 = 0;
					dssf->DSSF_C_CTRL = 1;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 1265; /* freq = 8 */
						dssf->theta_q_s1 = 3894; /* freq = 8 */
					} else {
						dssf->theta_i_s1 = 4880; /* freq = 8 */
						dssf->theta_q_s1 = 2406; /* freq = 8 */
					}
					break;
				case 2:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 3648; /* freq = 3 */
						dssf->theta_q_s1 = 1859; /* freq = 3 */
					} else {
						dssf->theta_i_s1 = 2406; /* freq = 3 */
						dssf->theta_q_s1 = 3312; /* freq = 3 */
					}
					break;
				case 3:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 3894; /* freq = -2 */
						dssf->theta_q_s1 = 6927; /* freq = -2 */
						dssf->theta_i_s2 = 4298; /* freq = 18 */
						dssf->theta_q_s2 = 1265; /* freq = 18 */
						dssf->enabled_s2 = 1;
					} else {
						dssf->theta_i_s1 = 3312; /* freq = -2 */
						dssf->theta_q_s1 = 5786; /* freq = -2 */
					}
					break;
				case 4:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 1859; /* freq = -7 */
						dssf->theta_q_s1 = 4544; /* freq = -7 */
						dssf->theta_i_s2 = 6333; /* freq = 13 */
						dssf->theta_q_s2 = 3648; /* freq = 13 */
						dssf->enabled_s2 = 1;
					} else {
						dssf->theta_i_s1 = 5786; /* freq = -7 */
						dssf->theta_q_s1 = 4880; /* freq = -7 */
					}
					break;
				case 5:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 6927; /* freq = -12 */
						dssf->theta_q_s1 = 4298; /* freq = -12 */
						dssf->theta_i_s2 = 1265; /* freq = 8 */
						dssf->theta_q_s2 = 3894; /* freq = 8 */
						dssf->enabled_s2 = 1;
					} else {
						dssf->theta_i_s1 = 4880; /* freq = 8 */
						dssf->theta_q_s1 = 2406; /* freq = 8 */
					}
					break;
				case 6:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 4544; /* freq = -17 */
						dssf->theta_q_s1 = 6333; /* freq = -17 */
						dssf->theta_i_s2 = 3648; /* freq = 3 */
						dssf->theta_q_s2 = 1859; /* freq = 3 */
						dssf->enabled_s2 = 1;
					} else {
						dssf->theta_i_s1 = 2406; /* freq = 3 */
						dssf->theta_q_s1 = 3312; /* freq = 3 */
					}
					break;
				case 7:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 3894; /* freq = -2 */
						dssf->theta_q_s1 = 6927; /* freq = -2 */
					} else {
						dssf->theta_i_s1 = 3312; /* freq = -2 */
						dssf->theta_q_s1 = 5786; /* freq = -2 */
					}
					break;
				case 8:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 1859; /* freq = -7 */
						dssf->theta_q_s1 = 4544; /* freq = -7 */
					} else {
						dssf->theta_i_s1 = 5786; /* freq = -7 */
						dssf->theta_q_s1 = 4880; /* freq = -7 */
					}
					break;
				case 9:
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
						dssf->DSSF_C_CTRL = 4;
						dssf->theta_i_s1 = 6927; /* freq = -12 */
						dssf->theta_q_s1 = 4298; /* freq = -12 */
					}
					break;
				case 10:
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
						dssf->DSSF_C_CTRL = 4;
						dssf->theta_i_s1 = 4544; /* freq = -17 */
						dssf->theta_q_s1 = 6333; /* freq = -17 */
					}
					break;
				case 11:
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
						dssf->DSSF_C_CTRL = 4;
						dssf->theta_i_s1 = 4298; /* freq = 18 */
						dssf->theta_q_s1 = 1265; /* freq = 18 */
						dssf->theta_i_s2 = 4298; /* freq = 18 */
						dssf->theta_q_s2 = 1265; /* freq = 18 */
						dssf->DSSF_gain_th0_s1 = 40;
						dssf->DSSF_gain_th1_s1 = 64;
						dssf->DSSF_gain_th2_s1 = 69;
						dssf->DSSF_gain_th0_s2 = 40;
						dssf->DSSF_gain_th1_s2 = 64;
						dssf->DSSF_gain_th2_s2 = 69;
						dssf->enabled_s2 = 1;
					}
					break;
				case 13:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
						dssf->DSSF_C_CTRL = 4;
					} else if (CHSPEC_IS40(pi->radio_chanspec)) {
						dssf->theta_i_s1 = 1265; /* freq = 8 */
						dssf->theta_q_s1 = 3894; /* freq = 8 */
						dssf->theta_i_s2 = 1265; /* freq = 8 */
						dssf->theta_q_s2 = 3894; /* freq = 8 */
					} else {
						dssf->theta_i_s1 = 4880; /* freq = 8 */
						dssf->theta_q_s1 = 2406; /* freq = 8 */
						dssf->theta_i_s2 = 4880; /* freq = 8 */
						dssf->theta_q_s2 = 2406; /* freq = 8 */
						dssf->DSSF_gain_th0_s1 = 60;
						dssf->DSSF_gain_th1_s1 = 68;
						dssf->DSSF_gain_th2_s1 = 74;
						dssf->DSSF_gain_th0_s2 = 60;
						dssf->DSSF_gain_th1_s2 = 68;
						dssf->DSSF_gain_th2_s2 = 74;
					}
					dssf->enabled_s2 = 1;
					break;

				/* 5G, 40Mhz */
				case 38:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 0; /* freq = -10 */
					dssf->theta_q_s1 = 4098; /* freq = -10 */
					dssf->theta_i_s2 = 0; /* freq = 10 */
					dssf->theta_q_s2 = 4094; /* freq = 10 */
					dssf->enabled_s2 = 1;
					break;
				case 62:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 0; /* freq = -10 */
					dssf->theta_q_s1 = 4098; /* freq = -10 */
					dssf->theta_i_s2 = 0; /* freq = 10 */
					dssf->theta_q_s2 = 4094; /* freq = 10 */
					dssf->enabled_s2 = 1;
					break;
				case 110:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 0; /* freq = -10 */
					dssf->theta_q_s1 = 4098; /* freq = -10 */
					dssf->theta_i_s2 = 0; /* freq = 10 */
					dssf->theta_q_s2 = 4094; /* freq = 10 */
					dssf->enabled_s2 = 1;
					break;
				case 134:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 0; /* freq = -10 */
					dssf->theta_q_s1 = 4098; /* freq = -10 */
					dssf->theta_i_s2 = 0; /* freq = 10 */
					dssf->theta_q_s2 = 4094; /* freq = 10 */
					dssf->enabled_s2 = 1;
					break;

				/* 5G, 80Mhz */
				case 122:
					PHY_INFORM((
						"%s: xtal freq 40M, applying dssf for channel %d\n",
						__FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 0; /* freq = 20 */
					dssf->theta_q_s1 = 4094; /* freq = 20 */
					dssf->theta_i_s2 = 5297; /* freq = 30 */
					dssf->theta_q_s2 = 2895; /* freq = 30 */
					dssf->DSSF_gain_th0_s1 = 63;
					dssf->DSSF_gain_th1_s1 = 69;
					dssf->DSSF_gain_th2_s1 = 75;
					dssf->DSSF_gain_th0_s2 = 63;
					dssf->DSSF_gain_th1_s2 = 69;
					dssf->DSSF_gain_th2_s2 = 75;
					dssf->enabled_s2 = 1;
					break;

				default:
					break;
				}
			} else if (PHY_ILNA(pi)) {
				switch (dssf->channel) {
				case 1:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 4880; /* freq = 8 */
					dssf->theta_q_s1 = 2406; /* freq = 8 */
					dssf->DSSF_gain_th0_s1 = 68;
					dssf->DSSF_gain_th1_s1 = 78;
					dssf->DSSF_gain_th2_s1 = 84;
					break;
				case 2:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 2406; /* freq = 3 */
					dssf->theta_q_s1 = 3312; /* freq = 3 */
					break;
				case 3:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3313; /* freq = -2 */
					dssf->theta_q_s1 = 5785; /* freq = -2 */
					dssf->DSSF_gain_th0_s1 = 68;
					dssf->DSSF_gain_th1_s1 = 74;
					dssf->DSSF_gain_th2_s1 = 84;
					break;
				case 4:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 5785; /* freq = -7 */
					dssf->theta_q_s1 = 4879; /* freq = -7 */
					dssf->theta_i_s2 = 1265; /* freq = 4 */
					dssf->theta_q_s2 = 3894; /* freq = 4 */
					dssf->enabled_s2 = 1;
					break;
				case 5:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3894; /* freq = -1 */
					dssf->theta_q_s1 = 6927; /* freq = -1 */
					dssf->DSSF_gain_th0_s1 = 72;
					dssf->DSSF_gain_th1_s1 = 78;
					dssf->DSSF_gain_th2_s1 = 84;
					break;
				case 6:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6927; /* freq = -6 */
					dssf->theta_q_s1 = 4298; /* freq = -6 */
					dssf->DSSF_gain_th0_s1 = 72;
					dssf->DSSF_gain_th1_s1 = 78;
					dssf->DSSF_gain_th2_s1 = 84;
					break;
				case 11:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6455; /* freq = 6.395 */
					dssf->theta_q_s1 = 3708; /* freq = 6.395 */
					break;
				case 12:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3708; /* freq = 1.395 */
					dssf->theta_q_s1 = 1737; /* freq = 1.395 */
					break;
				case 13:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 1737; /* freq = -3.605 */
					dssf->theta_q_s1 = 4484; /* freq = -3.605 */
					break;
				case 48:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 1265; /* freq = -4.0 */
					dssf->theta_q_s1 = 4297; /* freq = -4.0 */
					break;
				case 56:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6214; /* freq = -6.605 */
					dssf->theta_q_s1 = 4607; /* freq = -6.605 */
					break;
				case 100:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3146; /* freq = -2.211 */
					dssf->theta_q_s1 = 5571; /* freq = -2.211 */
					break;
				case 108:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 271; /* freq = -4.789 */
					dssf->theta_q_s1 = 4106; /* freq = -4.789 */
					dssf->DSSF_gain_th0_s1 = 64;
					dssf->DSSF_gain_th1_s1 = 70;
					dssf->DSSF_gain_th2_s1 = 76;
					break;
				case 116:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 5389; /* freq = -7.4 */
					dssf->theta_q_s1 = 5207; /* freq = -7.4 */
					break;
				case 128:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 5389; /* freq = 7.4 */
					dssf->theta_q_s1 = 2985; /* freq = 7.4 */
					break;
				case 136:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 271; /* freq = 4.789 */
					dssf->theta_q_s1 = 4086; /* freq = 4.789 */
					break;
				case 144:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 0;
					dssf->theta_i_s1 = 3146; /* freq = 2.211 */
					dssf->theta_q_s1 = 2621; /* freq = 2.211 */
					break;
				case 153:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 7686; /* freq = -5.395 */
					dssf->theta_q_s1 = 4129; /* freq = -5.395 */
					break;
				case 161:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 4880; /* freq = -8 */
					dssf->theta_q_s1 = 5786; /* freq = -8 */
					break;
				case 165:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 4170; /* freq = 9.395 */
					dssf->theta_q_s1 = 774; /* freq = 9.395 */
					break;
				/* 40Mhz */
				case 46:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 2406; /* freq = 6 */
					dssf->theta_q_s1 = 3312; /* freq = 6 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 54:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3526; /* freq = 3.395 */
					dssf->theta_q_s1 = 2081; /* freq = 3.395 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 62:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 4063; /* freq = 0.789 */
					dssf->theta_q_s1 = 506; /* freq = 0.789 */
					dssf->DSSF_gain_th0_s1 = 58;
					dssf->DSSF_gain_th1_s1 = 64;
					dssf->DSSF_gain_th2_s1 = 70;
					break;
				case 102:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6798; /* freq = -12.211 */
					dssf->theta_q_s1 = 4341; /* freq = -12.211 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 110:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 5394; /* freq = -14.789 */
					dssf->theta_q_s1 = 5203; /* freq = -14.789 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 134:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 5394; /* freq = 14.789 */
					dssf->theta_q_s1 = 2989; /* freq = 14.789 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 142:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6798; /* freq = 12.211 */
					dssf->theta_q_s1 = 3850; /* freq = 12.211 */
					dssf->DSSF_gain_th0_s1 = 60;
					dssf->DSSF_gain_th1_s1 = 66;
					dssf->DSSF_gain_th2_s1 = 72;
					break;
				case 151:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3069; /* freq = 4.605 */
					dssf->theta_q_s1 = 2710; /* freq = 4.605 */
					dssf->DSSF_gain_th0_s1 = 58;
					dssf->DSSF_gain_th1_s1 = 64;
					dssf->DSSF_gain_th2_s1 = 70;
					break;
				case 159:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3895; /* freq = 2 */
					dssf->theta_q_s1 = 1265; /* freq = 2 */
					dssf->DSSF_gain_th0_s1 = 58;
					dssf->DSSF_gain_th1_s1 = 64;
					dssf->DSSF_gain_th2_s1 = 70;
					break;
				/* 80Mhz */
				case 42:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 6332; /* freq = -26 */
					dssf->theta_q_s1 = 4543; /* freq = -26 */
					dssf->DSSF_gain_th0_s1 = 54;
					dssf->DSSF_gain_th1_s1 = 60;
					dssf->DSSF_gain_th2_s1 = 66;
					break;
				case 58:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 7938; /* freq = -20.789 */
					dssf->theta_q_s1 = 4104; /* freq = -20.789 */
					dssf->DSSF_gain_th0_s1 = 54;
					dssf->DSSF_gain_th1_s1 = 60;
					dssf->DSSF_gain_th2_s1 = 66;
					break;
				case 106:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3757; /* freq = -5.211 */
					dssf->theta_q_s1 = 1630; /* freq = -5.211 */
					dssf->DSSF_gain_th0_s1 = 54;
					dssf->DSSF_gain_th1_s1 = 60;
					dssf->DSSF_gain_th2_s1 = 66;
					break;
				case 138:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 3757; /* freq = 5.211 */
					dssf->theta_q_s1 = 1630; /* freq = 5.211 */
					dssf->theta_i_s2 = 4840; /* freq = -32.211 */
					dssf->theta_q_s2 = 5840; /* freq = -32.211 */
					dssf->enabled_s2 = 1;
					dssf->DSSF_gain_th0_s1 = 54;
					dssf->DSSF_gain_th1_s1 = 60;
					dssf->DSSF_gain_th2_s1 = 66;
					break;
				case 155:
					PHY_INFORM(("%s: applying dssf for channel %d\n",
					            __FUNCTION__, dssf->channel));
					dssf->DSSF_C_CTRL = 4;
					dssf->theta_i_s1 = 1448; /* freq = 15.395 */
					dssf->theta_q_s1 = 3830; /* freq = 15.396 */
					dssf->DSSF_gain_th0_s1 = 54;
					dssf->DSSF_gain_th1_s1 = 60;
					dssf->DSSF_gain_th2_s1 = 66;
					break;
				default:
					break;
				}
			}
		}
	} else {
		dssf->DSSF_C_CTRL = 0;
		dssf->enabled_s1 = 0;
		dssf->enabled_s2 = 0;
	}

	phy_ac_dssf_setup(pi_ac->rxspuri);

	wlc_phy_resetcca_acphy(pi);
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
#endif /* WL_DSSF_DISABLED */
}

void
phy_ac_dssfB(phy_ac_rxspur_info_t *rxspuri, bool on)
{
	phy_info_t *pi = rxspuri->pi;
	phy_info_acphy_t *pi_ac = rxspuri->aci;
	acphy_dssfB_values_t *dssfB = rxspuri->dssfB_params;

	if (DSSFB_ENABLE) {
		dssfB->channel = 0;
		dssfB->DSSFB_gain_th0_s1 = 61;
		dssfB->DSSFB_gain_th1_s1 = 67;
		dssfB->DSSFB_gain_th2_s1 = 73;
		dssfB->DSSFB_gain_th0_s2 = 61;
		dssfB->DSSFB_gain_th1_s2 = 67;
		dssfB->DSSFB_gain_th2_s2 = 73;

		dssfB->idepth_s1 = 0;
		dssfB->idepth_s2 = 0;
		dssfB->enabled_s1 = 0;
		dssfB->enabled_s2 = 0;
		dssfB->theta_i_s1 = 0;
		dssfB->theta_q_s1 = 0;
		dssfB->theta_i_s2 = 0;
		dssfB->theta_q_s2 = 0;
		dssfB->DSSFB_C_CTRL = 0;
		dssfB->on = on;
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);
		/* Reset DSSFB filter */
		WRITE_PHYREG(pi, DSSFB_C_CTRL, 0);
		if (on) {
			dssfB->channel = CHSPEC_CHANNEL(pi->radio_chanspec);
			if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
				(ACMINORREV_1(pi) || ACMINORREV_3(pi))) {
				if (PHY_ILNA(pi)) {
					switch (dssfB->channel) {
						/* 2G, 20Mhz && 40Mhz */
						case 4:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 3313; /* freq = 4 */
							dssfB->theta_q_s1 = 2407; /* freq = 4 */
							break;
						case 5:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 4045; /* freq = -1 */
							dssfB->theta_q_s1 = 7551; /* freq = -1 */
							break;
						case 6:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 2406; /* freq = -6 */
							dssfB->theta_q_s1 = 4879; /* freq = -6 */
							break;
						case 11:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 2406; /* freq = 6 */
							dssfB->theta_q_s1 = 3313; /* freq = 6 */
							break;
						case 12:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 4044; /* freq = 1 */
							dssfB->theta_q_s1 = 8833; /* freq = 1 */
							break;
						case 13:
							PHY_INFORM((
							"%s: applying dssfB for channel %d\n",
							__FUNCTION__, dssfB->channel));
							dssfB->idepth_s1 = 1;
							dssfB->idepth_s2 = 1;
							dssfB->enabled_s1 = 1;
							dssfB->enabled_s2 = 0;
							dssfB->DSSFB_C_CTRL = 0xc;
							dssfB->theta_i_s1 = 3313; /* freq = -4 */
							dssfB->theta_q_s1 = 5785; /* freq = -4 */
							break;
						default:
							break;
					}
				}
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
		           ((RADIOMINORREV(pi) == 4) ||
		            (RADIOMINORREV(pi) == 10) ||
		            (RADIOMINORREV(pi) == 11) ||
		            (RADIOMINORREV(pi) == 13)) &&
		           (pi->xtalfreq == 37400000)) {
			switch (dssfB->channel) {
			case 4:
				dssfB->idepth_s1 = 2;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 3313;
				dssfB->theta_q_s1 = 2407;
				break;
			case 5:
				dssfB->idepth_s1 = 2;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 4045;
				dssfB->theta_q_s1 = 7551;
				break;
			case 6:
				dssfB->idepth_s1 = 2;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 2406;
				dssfB->theta_q_s1 = 4879;
				break;
			case 11:
				dssfB->idepth_s1 = 1;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 2106;
				dssfB->theta_q_s1 = 3313;
				break;
			case 12:
				dssfB->idepth_s1 = 1;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 4044;
				dssfB->theta_q_s1 =  901;
				break;
			case 13:
				dssfB->idepth_s1 = 1;
				dssfB->enabled_s1 = 1;
				dssfB->theta_i_s1 = 3313;
				dssfB->theta_q_s1 = 6086;
				break;
			}
			if (dssfB->theta_i_s1 || dssfB->theta_i_s2) {
				dssfB->DSSFB_C_CTRL = 0xb;
				PHY_INFORM(("%s: xtal freq 37.4M, applying dssfB for channel %d\n",
				            __FUNCTION__, dssfB->channel));
			}
		}
		} else {
			dssfB->DSSFB_C_CTRL = 0;
			dssfB->enabled_s1 = 0;
			dssfB->enabled_s2 = 0;
		}

		phy_ac_dssfB_setup(pi_ac->rxspuri);
		wlc_phy_resetcca_acphy(pi);
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
	}
}

static uint8
phy_ac_spurwar_4345(phy_info_t *pi, uint32 xtal_hz, uint8 i,
	uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES], int8 *tone_id, uint8 *core_sp)
{
	uint32 dn, phy_bw;
	int32 sci, sc, fc, n, ni, df, nstp, max_sc;
	uint32 xtal = xtal_hz / 100000;
	uint16 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	fc = wf_channel2mhz(channel, CHSPEC_IS5G(pi->radio_chanspec)
		? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);

	if (CHSPEC_IS80(pi->radio_chanspec))
		phy_bw = 80;
	else if (CHSPEC_IS40(pi->radio_chanspec))
		phy_bw = 40;
	else
		phy_bw = 20;

	n  = fc * 100 / xtal;
	ni = n / 10;

	if (n - ni * 10 >= 5)
		ni++;

	/* for higher bw may have multiple harmonics */
	dn     = phy_bw * 5 / xtal;
	max_sc = 28 * phy_bw / 20;
	nstp   = ni + dn;
	ni    -= dn;

	for (; ni <= nstp; ni++) {
		df  = (xtal * ni - fc * 10);
		sc  = df * 32;
		sci = sc / 100;

		if (sci >= -max_sc && sci <= max_sc) {
			*core_sp = 1; /* core 0 */
			if (sci * 100 == sc) {
				tone_id[i + 0] = (int8) sci - 1;
				tone_id[i + 1] = (int8) sci;
				tone_id[i + 2] = (int8) sci + 1;

				noise_var[PHY_CORE_0][i + 0] = 3;
				noise_var[PHY_CORE_0][i + 1] = 9;
				noise_var[PHY_CORE_0][i + 2] = 3;
				i += 3;
			} else {
				sci -= (sci < 0);
				tone_id[i + 0] = (int8) sci;
				tone_id[i + 1] = (int8) sci + 1;

				noise_var[PHY_CORE_0][i + 0] = 9;
				noise_var[PHY_CORE_0][i + 1] = 9;

				PHY_INFORM((
				"applying spurwar (channel %d sub.ch [%d to %d] xtal_int %d)\n",
				channel, sci, sci + 1, xtal / 10));
				i += 2;
			}

			PHY_INFORM((
				"%s: applying spurwar (channel %d sub.ch [%d to %d] xtal_int %d)\n",
				__FUNCTION__, channel,
				(sci * 100 == sc) ? sci - 1 : sci, sci + 1, xtal / 10));
		}
	}

	return i;
}

void
phy_ac_spurwar(phy_ac_rxspur_info_t *rxspuri, uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES],
               int8 *tone_id, uint8 *core_sp)
{
	phy_info_t *pi = rxspuri->pi;
	uint8 i;
	uint16 channel = 0;

	/* Starting offset for spurwar */
	i = ACPHY_SPURWAR_NTONES_OFFSET;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
#define FVCO_4345 963000000
		i = phy_ac_spurwar_4345(pi, PHY_XTALFREQ(pi->xtalfreq), i, noise_var, tone_id,
				core_sp);
		i = phy_ac_spurwar_4345(pi, FVCO_4345 / 2, i, noise_var, tone_id, core_sp);
	} else if (PHY_ILNA(pi)) {
		/* Spur war for 4350 */
		if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) || ACMINORREV_3(pi))) {
			switch (channel) {
				case 4:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = 12;
					tone_id[i+1] = 13;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				case 5:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = -3;
					tone_id[i+1] = -4;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				case 6:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = -19;
					tone_id[i+1] = -20;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				case 11:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = 20;
					tone_id[i+1] = 21;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				case 12:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = 4;
					tone_id[i+1] = 5;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				case 13:
					*core_sp = 1; /* core 0 */
					tone_id[i+0] = -11;
					tone_id[i+1] = -12;
					noise_var[PHY_CORE_0][i+0] = 9;
					noise_var[PHY_CORE_0][i+1] = 9;
					PHY_INFORM(("phy_ac_spurwar: applying spurwar"
								" for channel %d\n", channel));
					break;
				default:
					break;
			}
		} else {
			*core_sp = 1; /* core 0 */
			switch (channel) {
				case 1:
					tone_id[i+0] = 1;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					break;
				case 2:
					tone_id[i+0] = -15;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					break;
				case 4:
					tone_id[i+0] = 1;
					tone_id[i+1] = -1;
					tone_id[i+2] = 12;
					tone_id[i+3] = 13;
					noise_var[PHY_CORE_0][i+0] = 0x4;
					noise_var[PHY_CORE_0][i+1] = 0x4;
					noise_var[PHY_CORE_0][i+2] = 0x2;
					noise_var[PHY_CORE_0][i+3] = 0x9;
					break;
				case 5:
					tone_id[i+0] = -3;
					tone_id[i+1] = -4;
					tone_id[i+2] = 17;
					tone_id[i+3] = 18;
					tone_id[i+4] = 25;
					tone_id[i+5] = 26;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					noise_var[PHY_CORE_0][i+1] = 0x3;
					noise_var[PHY_CORE_0][i+2] = 0x5;
					noise_var[PHY_CORE_0][i+3] = 0x2;
					noise_var[PHY_CORE_0][i+4] = 0x3;
					noise_var[PHY_CORE_0][i+5] = 0x4;
					break;
				case 6:
					tone_id[i+0] = 28;
					tone_id[i+1] = 29;
					tone_id[i+2] = -19;
					tone_id[i+3] = -20;
					tone_id[i+4] = 1;
					tone_id[i+5] = 2;
					tone_id[i+6] = 9;
					tone_id[i+7] = 10;
					noise_var[PHY_CORE_0][i+0] = 0x3;
					noise_var[PHY_CORE_0][i+1] = 0x3;
					noise_var[PHY_CORE_0][i+2] = 0x9;
					noise_var[PHY_CORE_0][i+3] = 0x3;
					noise_var[PHY_CORE_0][i+4] = 0x5;
					noise_var[PHY_CORE_0][i+5] = 0x2;
					noise_var[PHY_CORE_0][i+6] = 0x3;
					noise_var[PHY_CORE_0][i+7] = 0x4;
					break;
				case 7:
					tone_id[i+0] = 24;
					tone_id[i+1] = 25;
					tone_id[i+2] = -6;
					tone_id[i+3] = -7;
					tone_id[i+4] = -14;
					tone_id[i+5] = -15;
					noise_var[PHY_CORE_0][i+0] = 0x7;
					noise_var[PHY_CORE_0][i+1] = 0x7;
					noise_var[PHY_CORE_0][i+2] = 0x4;
					noise_var[PHY_CORE_0][i+3] = 0x3;
					noise_var[PHY_CORE_0][i+4] = 0x3;
					noise_var[PHY_CORE_0][i+5] = 0x5;
					break;
				case 8:
					tone_id[i+0] = -14;
					tone_id[i+1] = -15;
					tone_id[i+2] = -22;
					tone_id[i+3] = -23;
					tone_id[i+4] = 8;
					tone_id[i+5] = 9;
					noise_var[PHY_CORE_0][i+0] = 0x4;
					noise_var[PHY_CORE_0][i+1] = 0x2;
					noise_var[PHY_CORE_0][i+2] = 0x5;
					noise_var[PHY_CORE_0][i+3] = 0x6;
					noise_var[PHY_CORE_0][i+4] = 0x4;
					noise_var[PHY_CORE_0][i+5] = 0x9;
					break;
				case 9:
					tone_id[i+0] = -7;
					tone_id[i+1] = -8;
					tone_id[i+2] = 25;
					tone_id[i+3] = 26;
					noise_var[PHY_CORE_0][i+0] = 0x8;
					noise_var[PHY_CORE_0][i+1] = 0x3;
					noise_var[PHY_CORE_0][i+2] = 0x4;
					noise_var[PHY_CORE_0][i+3] = 0x5;
					break;
				case 10:
					tone_id[i+0] = 17;
					tone_id[i+1] = 18;
					tone_id[i+2] = -23;
					tone_id[i+3] = -24;
					tone_id[i+4] = 9;
					tone_id[i+5] = 10;
					noise_var[PHY_CORE_0][i+0] = 0x3;
					noise_var[PHY_CORE_0][i+1] = 0x4;
					noise_var[PHY_CORE_0][i+2] = 0x8;
					noise_var[PHY_CORE_0][i+3] = 0x3;
					noise_var[PHY_CORE_0][i+4] = 0x4;
					noise_var[PHY_CORE_0][i+5] = 0x5;
					break;
				case 11:
					tone_id[i+0] = 20;
					tone_id[i+1] = 21;
					tone_id[i+2] = 1;
					tone_id[i+3] = 2;
					noise_var[PHY_CORE_0][i+0] = 0x7;
					noise_var[PHY_CORE_0][i+1] = 0x6;
					noise_var[PHY_CORE_0][i+2] = 0x5;
					noise_var[PHY_CORE_0][i+3] = 0x7;
					break;
				case 12:
					tone_id[i+0] = -14;
					tone_id[i+1] = 4;
					tone_id[i+2] = 5;
					tone_id[i+3] = -22;
					tone_id[i+4] = -23;
					tone_id[i+5] = 25;
					noise_var[PHY_CORE_0][i+0] = 0x4;
					noise_var[PHY_CORE_0][i+1] = 0x8;
					noise_var[PHY_CORE_0][i+2] = 0x7;
					noise_var[PHY_CORE_0][i+3] = 0x7;
					noise_var[PHY_CORE_0][i+4] = 0x7;
					noise_var[PHY_CORE_0][i+5] = 0x5;
					break;
				case 13:
					tone_id[i+0] = -11;
					tone_id[i+1] = -12;
					tone_id[i+2] = 25;
					tone_id[i+3] = 26;
					tone_id[i+4] = 9;
					noise_var[PHY_CORE_0][i+0] = 0x6;
					noise_var[PHY_CORE_0][i+1] = 0x7;
					noise_var[PHY_CORE_0][i+2] = 0x7;
					noise_var[PHY_CORE_0][i+3] = 0x7;
					noise_var[PHY_CORE_0][i+4] = 0x4;
					break;
				case 14:
					tone_id[i+0] = 10;
					tone_id[i+1] = -4;
					tone_id[i+2] = -5;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					noise_var[PHY_CORE_0][i+1] = 0x4;
					noise_var[PHY_CORE_0][i+2] = 0x4;
					break;
				case 108:
					tone_id[i+0] = -15;
					tone_id[i+1] = -16;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					noise_var[PHY_CORE_0][i+1] = 0x9;
					break;
				case 153:
					tone_id[i+0] = -17;
					tone_id[i+1] = -18;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					noise_var[PHY_CORE_0][i+1] = 0x9;
					break;
				case 161:
					tone_id[i+0] = -25;
					tone_id[i+1] = -26;
					noise_var[PHY_CORE_0][i+0] = 0x9;
					noise_var[PHY_CORE_0][i+1] = 0x9;
					break;
				default:
					break;
			}
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
	           ((RADIOMINORREV(pi) == 4) ||
	            (RADIOMINORREV(pi) == 10) ||
	            (RADIOMINORREV(pi) == 11) ||
	            (RADIOMINORREV(pi) == 13)) &&
	           (pi->xtalfreq == 37400000)) {

		switch (channel) {
		case 4:
			*core_sp = 3;
			tone_id[i+0] = 12;
			tone_id[i+1] = 13;
			noise_var[PHY_CORE_0][i+0] = 0x3;
			noise_var[PHY_CORE_0][i+1] = 0x9;
			noise_var[PHY_CORE_1][i+0] = 0x3;
			noise_var[PHY_CORE_1][i+1] = 0x9;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		case 5:
			*core_sp = 3;
			tone_id[i+0] = -3;
			tone_id[i+1] = -4;
			noise_var[PHY_CORE_0][i+0] = 0x9;
			noise_var[PHY_CORE_0][i+1] = 0x3;
			noise_var[PHY_CORE_1][i+0] = 0x9;
			noise_var[PHY_CORE_1][i+1] = 0x3;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		case 6:
			*core_sp = 3;
			tone_id[i+0] = -19;
			tone_id[i+1] = -20;
			noise_var[PHY_CORE_0][i+0] = 0x9;
			noise_var[PHY_CORE_0][i+1] = 0x3;
			noise_var[PHY_CORE_1][i+0] = 0x9;
			noise_var[PHY_CORE_1][i+1] = 0x3;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		case 11:
			*core_sp = 3;
			tone_id[i+0] = 20;
			tone_id[i+1] = 21;
			noise_var[PHY_CORE_0][i+0] = 9;
			noise_var[PHY_CORE_0][i+1] = 3;
			noise_var[PHY_CORE_1][i+0] = 9;
			noise_var[PHY_CORE_1][i+1] = 3;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		case 12:
			*core_sp = 3;
			tone_id[i+0] = 4;
			tone_id[i+1] = 5;
			noise_var[PHY_CORE_0][i+0] = 3;
			noise_var[PHY_CORE_0][i+1] = 9;
			noise_var[PHY_CORE_1][i+0] = 3;
			noise_var[PHY_CORE_1][i+1] = 9;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		case 13:
			*core_sp = 3;
			tone_id[i+0] = -11;
			tone_id[i+1] = -12;
			noise_var[PHY_CORE_0][i+0] = 6;
			noise_var[PHY_CORE_0][i+1] = 6;
			noise_var[PHY_CORE_1][i+0] = 6;
			noise_var[PHY_CORE_1][i+1] = 6;
			PHY_INFORM(("%s: 4350 spurwar nvshp for channel %d\n",
			            __FUNCTION__, channel));
			break;
		}
	}
}

static void
phy_ac_set_spurmode(phy_type_rxspur_ctx_t *ctx, uint16 freq)
{
	phy_ac_rxspur_info_t *rxspuri = (phy_ac_rxspur_info_t *) ctx;
	phy_ac_get_spurmode(rxspuri, freq);

	if (rxspuri->curr_spurmode != rxspuri->acphy_spuravoid_mode) {
		phy_ac_setup_spurmode(rxspuri);
		rxspuri->curr_spurmode = rxspuri->acphy_spuravoid_mode;
	}
}

static void
phy_ac_get_spurmode(phy_ac_rxspur_info_t *rxspuri, uint16 freq)
{
	phy_info_t *pi = rxspuri->pi;

	if (ISSIM_ENAB(pi->sh->sih))
		return;

	if (pi->block_for_slowcal) {
		pi->blocked_freq_for_slowcal = freq;
		return;
	}

	if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID) {
		if (pi->sh->chippkg == BCM4335_WLBGA_PKG_ID)
			rxspuri->acphy_spuravoid_mode = 8;
		else {
			/* for WLCSP packages */
			if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
				/* for 4335 Cx Chips */
				if (pi->sh->chippkg == BCM4335_FCBGA_PKG_ID &&
					CHSPEC_IS2G(pi->radio_chanspec) &&
					CHSPEC_CHANNEL(pi->radio_chanspec) <= 7)
					rxspuri->acphy_spuravoid_mode = 2;
				else
					rxspuri->acphy_spuravoid_mode = 8;
			} else {
				/* for 4335 Ax/Bx Chips */
				rxspuri->acphy_spuravoid_mode = 2;
			}
		}
	} else {
		rxspuri->acphy_spuravoid_mode = 0;
	}
}

#if defined(WLTEST)
static int
phy_ac_rxspur_set_force_spurmode(phy_type_rxspur_ctx_t *ctx, int16 int_val)
{
	phy_ac_rxspur_info_t *rxspuri = (phy_ac_rxspur_info_t *) ctx;
	phy_info_t *pi = rxspuri->pi;

	uint16 freq;
	freq = (uint16)phy_utils_channel2freq(CHSPEC_CHANNEL(pi->radio_chanspec));

	if (int_val == -1) {
		rxspuri->acphy_spuravoid_mode_override = 0;
	} else {
		rxspuri->acphy_spuravoid_mode_override = 1;
	}

	if (pi->block_for_slowcal) {
		pi->blocked_freq_for_slowcal = freq;
		return BCME_OK;
	}

	switch (int_val) {
	case -1:
		PHY_TRACE(("Spurmode override is off; default spurmode restored; %s \n",
		__FUNCTION__));
		phy_ac_get_spurmode(rxspuri, freq);
		break;
	case 0:
		PHY_TRACE(("Force spurmode to 0; chanfreq %d: PLLfre:963Mhz; %s \n",
			freq, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = 0;
		break;
	case 1:
		PHY_TRACE(("Force spurmode to 1; chanfreq %d: PLLfre:960Mhz; %s \n",
			freq, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = 1;
		break;
	case 2:
		PHY_TRACE(("Force spurmode to 2; chanfreq %d: PLLfre:961Mhz; %s \n",
			freq, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = 2;
		break;
	case 3:
		PHY_TRACE(("Force spurmode to 3; chanfreq %d: PLLfre:964Mhz; %s \n",
			freq, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = 3;
		break;
	case 4:
		PHY_TRACE(("Force spurmode to 4; chanfreq %d: PLLfre:962Mhz; %s \n",
			freq, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = 4;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		PHY_TRACE(("Force spurmode to %d; chanfreq %d: PLLfre:96%dMhz; %s \n",
			int_val, freq, int_val, __FUNCTION__));
		rxspuri->acphy_spuravoid_mode = (uint8) int_val;
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported spurmode %d.\n",
			pi->sh->unit, __FUNCTION__, int_val));
		ASSERT(0);
		phy_ac_get_spurmode(rxspuri, freq);
		return BCME_ERROR;
	}
	/* Call the bbpll settings related function only if the forced */
	/* spur mode is different from the current spur mode */
	if (rxspuri->curr_spurmode != rxspuri->acphy_spuravoid_mode) {
		phy_ac_setup_spurmode(rxspuri);
		rxspuri->curr_spurmode =  rxspuri->acphy_spuravoid_mode;
	}
	return BCME_OK;
}

static int
phy_ac_rxspur_get_force_spurmode(phy_type_rxspur_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_ac_rxspur_info_t *rxspuri = (phy_ac_rxspur_info_t *) ctx;

	if (rxspuri->acphy_spuravoid_mode_override == 1) {
		*ret_int_ptr = rxspuri->acphy_spuravoid_mode;
	} else {
		*ret_int_ptr = -1;
	}
	return BCME_OK;
}
#endif /* defined(WLTEST) */
