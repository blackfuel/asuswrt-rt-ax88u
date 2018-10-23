/*
 * ACPHY Napping module implementation
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
 * $Id$
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_nap.h"
#include <phy_utils_var.h>
#include <phy_ac.h>
#include <phy_ac_nap.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#include <wlc_phyreg_ac.h>
#include <phy_rstr.h>
#include <wlioctl.h>
#include <phy_misc_api.h>
#include <phy_wd.h>
/* Napping parameters */
typedef struct {
	uint16 nap2rx_seq[16];
	uint16 nap2rx_seq_dly[16];
	uint16 rx2nap_seq[16];
	uint16 rx2nap_seq_dly[16];
	uint16 nap_lo_th_adj[5];
	uint16 nap_hi_th_adj[5];
	uint16 nap_wait_in_cs_len;
	uint16 nap_len;
	uint16 nap2cs_wait_in_reset;
	uint16 pktprocResetLen;
	uint8 ed_thresh_cal_start_stage;
} phy_ac_nap_params_t;

/* module private states */
struct phy_ac_nap_info {
	phy_info_t		*pi;
	phy_ac_info_t		*aci;
	phy_nap_info_t		*cmn_info;
	phy_ac_nap_params_t	*nap_params;
	uint16	nap_disable_reqs;
	/* Parameter to enable Napping feature */
	uint8	nap_en;
	int8	nap_energy_threshold;
	int16	last_nap_threshold_adj_temp;
	uint32	nap_disable_dur;
	uint32	nap_disable_last;
};

/* Napping related table ID's */
typedef enum {
	RX2NAP_SEQ_TBL,
	RX2NAP_SEQ_DLY_TBL,
	NAP2RX_SEQ_TBL,
	NAP2RX_SEQ_DLY_TBL,
	NAP_LO_TH_ADJ_TBL,
	NAP_HI_TH_ADJ_TBL
} phy_ac_nap_param_tbl_t;

#define BPHY_PREDETECTTH_NAP_EN 0x1ff
#define BPHY_PREDETECTTH_NAP_DIS 0x15f
#define DIGITAL_NAPPING_EN 1

/* Local functions */
static int BCMATTACHFN(phy_ac_populate_nap_params)(phy_ac_nap_info_t *nap_info);
static void* BCMRAMFN(phy_ac_get_nap_param_tbl)(phy_info_t *pi,
		phy_ac_nap_param_tbl_t tbl_id);

#ifdef WL_NAP
static void phy_ac_set_nap_params(phy_info_t *pi);
static void phy_ac_reset_nap_params(phy_info_t *pi);
static void phy_ac_nap_enable_majrev44(phy_info_t *pi, bool enable);
static void phy_ac_load_nap_sequencer_28nm_ulp(phy_info_t *pi);
static uint8 phy_ac_nap_ed_thresh_scale(phy_info_t *pi, uint32 *lo_thresh, uint32 *hi_thresh,
		uint8 strt_stage);
static void phy_ac_compute_nap_ed_scale_factor(phy_info_t *pi, uint32 thresh, uint8 *scale_conf,
		uint8 *scale_amt);
static int8 phy_ac_nap_get_energy_threshold(phy_type_nap_ctx_t *ctx);
static void phy_ac_nap_set_energy_threshold(phy_type_nap_ctx_t *ctx, int8 energy_threshold);
static bool phy_ac_nap_wd(phy_wd_ctx_t *ctx);
#endif /* WL_NAP */

/* local functions */
static void phy_ac_nap_nvram_attach(phy_ac_nap_info_t *nap_info);
#ifdef WL_NAP
static void phy_ac_nap_get_status(phy_type_nap_ctx_t *ctx,
		uint16 *bitmap, uint8 *hwstatus, uint32 *ms);
static void phy_ac_nap_set_disable_req(phy_type_nap_ctx_t *ctx, uint16 req,
		bool disable, bool agc_reconfig, uint8 req_id);
#endif /* WL_NAP */

/* register phy type specific implementation */
phy_ac_nap_info_t *
BCMATTACHFN(phy_ac_nap_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_nap_info_t *cmn_info)
{
	phy_ac_nap_info_t *nap_info;
	phy_type_nap_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((nap_info = phy_malloc(pi, sizeof(phy_ac_nap_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	nap_info->pi = pi;
	nap_info->aci = aci;
	nap_info->cmn_info = cmn_info;
#ifdef WL_NAP
	nap_info->nap_energy_threshold = NAP_ENERGY_THRESHOLD_DBM;
#endif /* WL_NAP */

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#ifdef WL_NAP
	fns.get_status = phy_ac_nap_get_status;
	fns.set_disable_req = phy_ac_nap_set_disable_req;
	fns.get_energy_threshold = phy_ac_nap_get_energy_threshold;
	fns.set_energy_threshold = phy_ac_nap_set_energy_threshold;
	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_ac_nap_wd, nap_info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_AC_NAP,
		PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		}
#endif /* WL_NAP */
	fns.ctx = nap_info;

	/* Populate nap params */
	if (phy_ac_populate_nap_params(nap_info) != BCME_OK) {
		goto fail;
	}

	/* Read srom params from nvram */
	phy_ac_nap_nvram_attach(nap_info);

	phy_set_feature_flag(pi, PHY_FEATURE_NAP_IDX, TRUE);

	if (phy_nap_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_nap_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return nap_info;

fail:
	if ((nap_info != NULL) && (nap_info->nap_params != NULL)) {
		phy_mfree(pi, nap_info->nap_params, sizeof(phy_ac_nap_params_t));
	}

	/* error handling */
	if (nap_info != NULL) {
		phy_mfree(pi, nap_info, sizeof(phy_ac_nap_info_t));
	}

	return NULL;
}

void
BCMATTACHFN(phy_ac_nap_unregister_impl)(phy_ac_nap_info_t *nap_info)
{
	phy_info_t *pi = nap_info->pi;
	phy_nap_info_t *cmn_info = nap_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_nap_unregister_impl(cmn_info);

	if (nap_info->nap_params) {
		phy_mfree(pi, nap_info->nap_params, sizeof(phy_ac_nap_params_t));
	}

	phy_mfree(pi, nap_info, sizeof(phy_ac_nap_info_t));
}

bool
phy_ac_nap_is_enabled(phy_ac_nap_info_t *nap_info)
{
	if (nap_info != NULL) {
		return nap_info->nap_en;
	} else {
		return FALSE;
	}
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */

static int
BCMATTACHFN(phy_ac_populate_nap_params)(phy_ac_nap_info_t *nap_info)
{
	phy_ac_nap_params_t *nap_params;
	phy_info_t *pi;

	pi = nap_info->pi;

	if ((nap_params = phy_malloc(pi, sizeof(phy_ac_nap_params_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Copy NAP2RX and RX2NAP seq */
		memcpy(nap_params->rx2nap_seq,
				phy_ac_get_nap_param_tbl(pi, RX2NAP_SEQ_TBL),
				sizeof(uint16) * 16);
		memcpy(nap_params->rx2nap_seq_dly,
				phy_ac_get_nap_param_tbl(pi, RX2NAP_SEQ_DLY_TBL),
				sizeof(uint16) * 16);
		memcpy(nap_params->nap2rx_seq,
				phy_ac_get_nap_param_tbl(pi, NAP2RX_SEQ_TBL),
				sizeof(uint16) * 16);
		memcpy(nap_params->nap2rx_seq_dly,
				phy_ac_get_nap_param_tbl(pi, NAP2RX_SEQ_DLY_TBL),
				sizeof(uint16) * 16);

		/* Nap ED threshold computation adjust factors */
		memcpy(nap_params->nap_lo_th_adj,
				phy_ac_get_nap_param_tbl(pi, NAP_LO_TH_ADJ_TBL),
				sizeof(uint16) * 5);
		memcpy(nap_params->nap_hi_th_adj,
				phy_ac_get_nap_param_tbl(pi, NAP_HI_TH_ADJ_TBL),
				sizeof(uint16) * 5);

		nap_params->nap_wait_in_cs_len = 10;
		nap_params->nap_len = 64;
		nap_params->nap2cs_wait_in_reset = 2;
		nap_params->pktprocResetLen = 112;

		/* Start stage to do ED threshold cal */
		nap_params->ed_thresh_cal_start_stage = 5;
	} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		/* Nap ED threshold computation adjust factors */
		memcpy(nap_params->nap_lo_th_adj,
				phy_ac_get_nap_param_tbl(pi, NAP_LO_TH_ADJ_TBL),
				sizeof(uint16) * 5);
		memcpy(nap_params->nap_hi_th_adj,
				phy_ac_get_nap_param_tbl(pi, NAP_HI_TH_ADJ_TBL),
				sizeof(uint16) * 5);

		nap_params->nap_wait_in_cs_len = 20;
		nap_params->nap_len = 16;
		nap_params->nap2cs_wait_in_reset = 2;
		nap_params->pktprocResetLen = 112;

		/* Start stage to do ED threshold cal */
		nap_params->ed_thresh_cal_start_stage = 5;
	}

	/* setup ptr */
	nap_info->nap_params = nap_params;

	return (BCME_OK);

fail:
	if (nap_params != NULL) {
		phy_mfree(pi, nap_params, sizeof(phy_ac_nap_params_t));
	}

	return (BCME_NOMEM);
}

static void *
BCMATTACHFN(phy_ac_get_nap_param_tbl)(phy_info_t *pi, phy_ac_nap_param_tbl_t tbl_id)
{
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		switch (tbl_id) {
			case RX2NAP_SEQ_TBL:
				return rx2nap_seq_maj36;

			case RX2NAP_SEQ_DLY_TBL:
				return rx2nap_seq_dly_maj36;

			case NAP2RX_SEQ_TBL:
				return nap2rx_seq_maj36;

			case NAP2RX_SEQ_DLY_TBL:
				return nap2rx_seq_dly_maj36;

			case NAP_LO_TH_ADJ_TBL:
				return nap_lo_th_adj_maj36;

			case NAP_HI_TH_ADJ_TBL:
				return nap_hi_th_adj_maj36;

			default:
				return NULL;
		}
	} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		switch (tbl_id) {
			case NAP_LO_TH_ADJ_TBL:
				return nap_lo_th_adj_maj40;

			case NAP_HI_TH_ADJ_TBL:
				return nap_hi_th_adj_maj40;

			default:
				return NULL;
		}
	} else {
		return NULL;
	}
}

static void
BCMATTACHFN(phy_ac_nap_nvram_attach)(phy_ac_nap_info_t *nap_info)
{
	nap_info->nap_en = (uint8)PHY_GETINTVAR_DEFAULT(nap_info->pi, rstr_nap_en, 0x0);

	/* nvram to control init ocl_disable state */
	if (nap_info->nap_en) {
		nap_info->nap_disable_reqs = 0;
	} else {
		nap_info->nap_disable_reqs = NAP_DISABLED_HOST;
	}

	nap_info->nap_disable_dur = 0;
	nap_info->nap_disable_last = OSL_SYSUPTIME();
}
/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

#ifdef WL_NAP
/* proc acphy_load_nap_sequencer_28nm_ulp */
static void
phy_ac_load_nap_sequencer_28nm_ulp(phy_info_t *pi)
{
	/* Bundle commands to toggle afe_rst_clk, afediv_adc_en, afediv_dac_en, afediv_rst_en */
	uint16 rfseq_bundle_48[3] = {0x0, 0x1000, 0x0};
	uint16 rfseq_bundle_49[3] = {0x0, 0x1100, 0x0};
	uint16 rfseq_bundle_50[3] = {0x0, 0x1000, 0x0};
	uint16 rfseq_bundle_51[3] = {0x0, 0x0, 0x0};
	uint16 rfseq_bundle_52[3] = {0x0, 0x1400, 0x0};
	uint16 rfseq_bundle_53[3] = {0x0, 0x1500, 0x0};
	uint16 rfseq_bundle_54[3] = {0x0, 0x1400, 0x0};
	uint16 rfseq_bundle_55[3] = {0x0, 0x0400, 0x0};
	uint16 temp_var1, temp_var2;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	phy_ac_nap_params_t *nap_params = pi_ac->napi->nap_params;

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 48, 48, rfseq_bundle_48);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 49, 48, rfseq_bundle_49);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 50, 48, rfseq_bundle_50);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 51, 48, rfseq_bundle_51);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 52, 48, rfseq_bundle_52);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 53, 48, rfseq_bundle_53);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 54, 48, rfseq_bundle_54);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 55, 48, rfseq_bundle_55);

	/* To avoid pulsing of oclS_set_bias_reset1 (0xe4) and oclW_set_bias_reset1 (0xe2) */
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe4, 16, &temp_var1);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe2, 16, &temp_var2);

	temp_var1 = temp_var1 & 0x77;
	temp_var2 = temp_var2 & 0x77;

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe4, 16, &temp_var1);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe2, 16, &temp_var2);

	if (!ACMINORREV_0(pi)) {
		/* Changes related to SW43012-1540 */
		/* Program oclW_rx_lpf_ctl_lut_enrx1(55:54) = 3
		    RFSeqTbl[0x1AA] => oclW_rx_lpf_ctl_lut_enrx1(0)(58:50)
		*/
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1aa, 16, &temp_var1);
		temp_var1 = temp_var1 | (3 << 4);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1aa, 16, &temp_var1);

		/* Program oclS_rx_lpf_ctl_lut_disrx1(55:54) = 0
		    RFSeqTbl[0x1A8] => oclS_rx_lpf_ctl_lut_disrx1(0)(58:50)
		*/
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a8, 16, &temp_var1);
		temp_var1 = temp_var1 & 0xffcf;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x1a8, 16, &temp_var1);
	}

	/* Load RX2NAP sequence and delay values */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x400, 16,
			nap_params->rx2nap_seq);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x420, 16,
			nap_params->rx2nap_seq_dly);

	/* Load NAP2RX sequence and delay values */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x410, 16,
			nap_params->nap2rx_seq);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x430, 16,
			nap_params->nap2rx_seq_dly);
}

/* proc acphy_enable_napping_28nm_ulp */
void
phy_ac_config_napping_28nm_ulp(phy_info_t *pi)
{
	/* Load napping related RFSeq entries */
	phy_ac_load_nap_sequencer_28nm_ulp(pi);

	/* Enable napping */
	phy_ac_nap_enable(pi, !pi->u.pi_acphy->napi->nap_disable_reqs, FALSE);
}

void
phy_ac_config_napping_28nm(phy_info_t *pi)
{
	phy_ac_set_nap_params(pi);
	phy_ac_nap_enable(pi, !pi->u.pi_acphy->napi->nap_disable_reqs, FALSE);
}

static void
phy_ac_nap_set_energy_threshold(phy_type_nap_ctx_t *ctx, int8 energy_threshold)
{
	phy_ac_nap_info_t *info = (phy_ac_nap_info_t *)ctx;
	if (info->nap_energy_threshold != energy_threshold) {
		info->nap_energy_threshold = energy_threshold;
		phy_ac_nap_update_energy_threshold(info);
	}
}

static bool
phy_ac_nap_wd(phy_wd_ctx_t *ctx)
{
	phy_ac_nap_info_t *info = (phy_ac_nap_info_t *)ctx;
	phy_info_t	*pi			= info->pi;

	if (ABS(info->last_nap_threshold_adj_temp -
		phy_ac_temp_get(pi->u.pi_acphy->tempi)) <
		NAP_THRESHOLD_ADJ_TEMP_THRESHOLD) {
			return TRUE;
		}

	if (!SCAN_RM_IN_PROGRESS(pi)) {
		phy_ac_nap_update_energy_threshold(info);
	}

	return TRUE;

}
void
phy_ac_nap_update_energy_threshold(phy_ac_nap_info_t *nap_info)
{
	/* This table records the napping threshold for the second stage,
		nap energy threshold -90dBm, for initG from 60dB to 70dB in 1dB
		step, the formula to get the threshold will be initG+60, capped
		at [0 10]. Note this table can be extended to accomodate wider
		range. For different nap energy threshold, the index to this table
		will be initG - 60 + energy_threshold_dBm + 90.
		This constant are defined in phy_ac_nap.h as
		#define NAP_THRESHOLD_TBL_REF_ENERGY		(-90)
		#define NAP_THRESHOLD_TBL_REF_GAIN			60
		#define NAP_THRESHOLD_TBL_LENGTH			10
	*/
	uint16	p8us_energy_ref = NAP_THRESHOLD_TBL_REF_THRESHOLD;
	rxgain_t rxgain[PHY_CORE_MAX];
	int16 gain_db[PHY_CORE_MAX];
	int8 tmp_index = -127, shift, residue, index = -127;
	uint16 level, subband_adj;
	uint8 core;
	int8 energy_threshold_dBm;
	bool suspend	= FALSE;

	phy_info_t *pi			= (phy_info_t *)nap_info->pi;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_rssi_data_t *rssi_data = phy_ac_rssi_get_data(pi_ac->rssii);

	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	BCM_REFERENCE(phyrxchain);

	energy_threshold_dBm = pi_ac->napi->nap_energy_threshold;

	if (!ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		return;
	}

	wlc_phy_conditional_suspend(pi, &suspend);
	/* get the nominal gain for initG */
	wlc_phy_get_rxgain_acphy(pi, rxgain, gain_db, 1);
	if ((rssi_data->rssi_cal_rev == TRUE) &&
	    (rssi_data->rxgaincal_rssical == TRUE)) {
		int8 subband_idx, bw_idx, ant;
		int16 rssi_gain_delta_qdBm[PHY_CORE_MAX], rssi_temp_delta_qdBm;
		uint8 core_freq_segment_map;
		acphy_rssioffset_t *pi_ac_rssioffset =
		  &pi_ac->sromi->rssioffset;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			bw_idx = (CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		} else {
			bw_idx = (CHSPEC_IS80(pi->radio_chanspec) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) ? 2 :
				(CHSPEC_IS160(pi->radio_chanspec)) ? 3 :
				(CHSPEC_IS40(pi->radio_chanspec)) ? 1 : 0;
		}
		wlc_phy_upd_gain_wrt_gain_cal_temp_phy(pi,
				&rssi_temp_delta_qdBm);
		pi_ac->napi->last_nap_threshold_adj_temp =
				phy_ac_temp_get(pi_ac->tempi);
		/* Apply nvram based offset: */
		FOREACH_CORE(pi, core) {
			/* core_freq_segment_map is only required for
			80P80 mode. For other modes, it is ignored
			*/
			core_freq_segment_map = phy_ac_chanmgr_get_data
				(pi_ac->chanmgri)->core_freq_mapping[core];
			ant = phy_get_rsdbbrd_corenum(pi, core);
			subband_idx =
				wlc_phy_rssi_get_chan_freq_range_acphy(pi,
					core_freq_segment_map, core);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
			  rssi_gain_delta_qdBm[core] =
			    pi_ac_rssioffset
			    ->rssi_corr_gain_delta_2g_sub[ant]
			    [ACPHY_GAIN_DELTA_ELNA_ON][bw_idx][subband_idx];
			} else {
			    rssi_gain_delta_qdBm[core] =
			      pi_ac_rssioffset
			      ->rssi_corr_gain_delta_5g_sub
			      [ant][ACPHY_GAIN_DELTA_ELNA_ON][bw_idx]
			      [subband_idx];
			}
			gain_db[core] -= (rssi_gain_delta_qdBm[core] +
					rssi_temp_delta_qdBm + 2) >> 2;
		}
	}

	/* In case multiple cores pick different indexs, the higher
	threshold will be applied as the higher gain core will dominate
	the CRS detection.
	*/
	index = -127;
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
#ifdef OCL
		if (PHY_OCL_ENAB(pi->sh->physhim)) {
			phy_ac_ocl_data_t *ocl_data = phy_ac_ocl_get_data(pi_ac->ocli);
			if (ocl_data->ocl_disable_reqs) {
				tmp_index = gain_db[core] - NAP_THRESHOLD_TBL_REF_GAIN +
						energy_threshold_dBm - NAP_THRESHOLD_TBL_REF_ENERGY;
				index = index > tmp_index ? index : tmp_index;
			} else {
				if (ocl_data->ocl_coremask & (1 << core)) {
					tmp_index = gain_db[core] - NAP_THRESHOLD_TBL_REF_GAIN +
							energy_threshold_dBm -
							NAP_THRESHOLD_TBL_REF_ENERGY;
					index = index > tmp_index ? index : tmp_index;
				}
			}
		} else {
			tmp_index = gain_db[core] - NAP_THRESHOLD_TBL_REF_GAIN +
				energy_threshold_dBm - NAP_THRESHOLD_TBL_REF_ENERGY;
			index = index > tmp_index ? index : tmp_index;
		}
#else
		tmp_index = gain_db[core] - NAP_THRESHOLD_TBL_REF_GAIN +
				energy_threshold_dBm - NAP_THRESHOLD_TBL_REF_ENERGY;
		index = index > tmp_index ? index : tmp_index;
#endif /* ifdef OCL */
	}

	index = MIN(index, NAP_THRESHOLD_INDEX_MAX);
	/* the nap energy threshold is deduced from a reference point 3540,
		which corresponding to 64 norminal gain to reach -90dBm sensitivity.
		For higher threshold, 1dB higher is done by multiplying 1.25, 2dB high
		by 1.5 approxmitely, 3dB higher is double. Larger threshold is done by
		first shift multiple 3dB, then do the residue (1 or 2 dB).
		Similar idea is used for lower threshold. This avoid the use of table,
		achieves wider range.
		The error is about 0.03 and 0.25dB for residue of 1 and 2 respectively.
		Higher accuracy can be achived by create a short table of three entry
		{3540, 4457, 5610} and do not rely on approximation.
	*/
	shift		  = index / (int8)3;
	residue		  = index % (int8)3;

	residue = residue < 0 ?	residue + 3 : residue;

	p8us_energy_ref = index >= 0 ? p8us_energy_ref << shift :
			p8us_energy_ref >> (ABS(shift) + (residue != 0));
	level = residue > 0 ? (p8us_energy_ref + (p8us_energy_ref >> (3 - residue))) :
			p8us_energy_ref;

	WRITE_PHYREG(pi, nap_ed_thld_lo_2, level);
	WRITE_PHYREG(pi, nap_ed_thld_hi_2, level);

	subband_adj = level >> 3;
	if (CHSPEC_IS80(pi->radio_chanspec)) {
		WRITE_PHYREG(pi, nap_ed_thld_lo_2, level - subband_adj);
		WRITE_PHYREG(pi, nap_ed_thld_hi_2, level - subband_adj);
		WRITE_PHYREG(pi, nap_pwr_comp_20u, -1 * subband_adj);
		WRITE_PHYREG(pi, nap_pwr_comp_20lsub, -1 * subband_adj);
		WRITE_PHYREG(pi, nap_pwr_comp_20usub, subband_adj);
		WRITE_PHYREG(pi, nap_pwr_comp_20l, subband_adj);
	}
	wlc_phy_conditional_resume(pi, &suspend);
}

/* proc acphy_set_nap_params */
static void
phy_ac_set_nap_params(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	phy_ac_nap_params_t *nap_params = pi_ac->napi->nap_params;
	uint16 spare_reg;
	bool digital_nap = DIGITAL_NAPPING_EN;

	/* Napping related counters */
	MOD_PHYREG(pi, nap_len, nap_len, nap_params->nap_len);
	MOD_PHYREG(pi, nap2cs_wait_in_reset_len, nap2cs_wait_in_reset,
			nap_params->nap2cs_wait_in_reset);
	WRITE_PHYREG(pi, nap_wait_in_cs_len, nap_params->nap_wait_in_cs_len);

	/* SW43012-1024 : Fix for tx turn around delay when napping enabled */
	WRITE_PHYREG(pi, pktprocResetLen, nap_params->pktprocResetLen);

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
			/* 4347 related settings */
			WRITE_PHYREG_ENTRY(pi, nap_len, 64)
			MOD_PHYREG_ENTRY(pi, RxFeTesMmuxCtrl, resetsdFeInNonActvSt, 1)
			MOD_PHYREG_ENTRY(pi, RxFeTesMmuxCtrl, use_fr_reset, 1)
			MOD_PHYREG_ENTRY(pi, advnapCtrl, nap_root_gating_en, 1)

			/* hard-code 4347 thresholds temporarily
				single stage decision for now, which bypass stage 1
				by setting lo_1 to 0 and hi_1 to 0x7fff, and set lo_2
				and hi_2 to be equal to force a decision. Temparorily
				set the threshold to corresponding to -90dBm signal power
				for BW20 case.
				Assuming 68dB init gain, this will translate in a napping
				threshold of 10802 for stage 2.
			*/
			WRITE_PHYREG_ENTRY(pi, nap_ed_thld_lo_1, 0x0)
			WRITE_PHYREG_ENTRY(pi, nap_ed_thld_hi_1, 0x7FFF)
		ACPHY_REG_LIST_EXECUTE(pi);
		/* 0x8 for digital nap, 0xb for regular nap */
		WRITE_PHYREG(pi, nap_rfctrl_prog_bits, digital_nap ? 0x8 : 0xb);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* Do CRS short reset in 2G as WAR */
			WRITE_PHYREG(pi, NapCtrl, 0x023b);
			MOD_PHYREG(pi, rx1misc_0, crsAutoShortRst, 1);

			/* bphy detection takes longer time, set the timeout to 16us */
			WRITE_PHYREG(pi, nap_wake_event_early_timeout_len, 0x280);
			WRITE_PHYREG(pi, nap_wake_event_timeout_len, 0x280);
		} else {
			/* Do regular nap_reset in 5G */
			WRITE_PHYREG(pi, NapCtrl, 0x023f);
			MOD_PHYREG(pi, rx1misc_0, crsAutoShortRst, 0);

			WRITE_PHYREG(pi, nap_wake_event_early_timeout_len, 0x140);
			WRITE_PHYREG(pi, nap_wake_event_timeout_len, 0x140);
		}

		if (CHSPEC_IS80(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, nap_wait_in_cs_len, digital_nap ? 0x14 : 0x20);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, nap_wait_in_cs_len, digital_nap ? 0x14 : 0x20);
		} else {
			WRITE_PHYREG(pi, nap_wait_in_cs_len, digital_nap ? 0x14 : 0x23);
		}
		phy_ac_nap_update_energy_threshold(pi_ac->napi);
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev) && (!ACMINORREV_0(pi))) {
		/* Fix to pktproc stuck in nap state on nap disable : CRDOT11ACPHY-2102
		    SpareRegB0(7) = 0
		*/
		spare_reg = READ_PHYREG(pi, SpareRegB0) & 0xff7f;
		WRITE_PHYREG(pi, SpareRegB0, spare_reg);

		/* Fix to nap tx issue : CRDOT11ACPHY-2234
		    SpareReg(11) = 1
		*/
		spare_reg = READ_PHYREG(pi, SpareReg) | (1 << 11);
		WRITE_PHYREG(pi, SpareReg, spare_reg);
	}
}

static void
phy_ac_reset_nap_params(phy_info_t *pi)
{
	ACPHY_REG_LIST_START
		WRITE_PHYREG_ENTRY(pi, NapCtrl, 0)
		WRITE_PHYREG_ENTRY(pi, nap_rfctrl_prog_bits, 0)
		MOD_PHYREG_ENTRY(pi, RxFeCtrl1, use_fr_reset, 0)
		MOD_PHYREG_ENTRY(pi, RfBiasControl, ocls_bg_pulse_val, 1)
		MOD_PHYREG_ENTRY(pi, RfBiasControl, oclw_bg_pulse_val, 1)
		WRITE_PHYREG_ENTRY(pi, pktprocResetLen, 112)
	ACPHY_REG_LIST_EXECUTE(pi);
}

static void
phy_ac_nap_enable_majrev44(phy_info_t *pi, bool enable)
{
	bool digital_nap = DIGITAL_NAPPING_EN;

	if (!ACMAJORREV_44(pi->pubpi->phy_rev)) {
		return;
	}
	/* Have increase predetect th to reduce false wakes from predetect */
	MOD_PHYREG(pi, bphyPreDetectThreshold0, ac_det_1us_min_pwr_0,
		enable ? BPHY_PREDETECTTH_NAP_EN : BPHY_PREDETECTTH_NAP_DIS);
	MOD_PHYREG(pi, RxSdFeConfig1, forceStallEninNAP, !enable);
	if (digital_nap) {
		if (pi->pubpi->slice == DUALMAC_AUX)  {
			MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn,
				enable ? pi->pubpi->phy_coremask : 0);
		}
	} else {
		if (pi->pubpi->slice == DUALMAC_MAIN)  {
			MOD_PHYREG(pi, phy_workaround_ctrl, enable, enable);
		} else {
			MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn,
				enable ? pi->pubpi->phy_coremask : 0);
			MOD_PHYREG(pi, PHY1_Div_Clock_Root_Gating_Control,
			phy1_tree1_main_divider_root_gating_force_on, enable);
		}
	}
}

void
phy_ac_nap_enable(phy_info_t *pi, bool enable, bool agc_reconfig)
{
	bool suspend = FALSE;

	/* Suspend MAC if haven't done so */
	wlc_phy_conditional_suspend(pi, &suspend);
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, NapCtrl, nap_en, enable);
		MOD_PHYREG(pi, NapCtrl, napCrsRstAccDis, enable);
	} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* WARs for phy_maj44. Different sequence for enabling and disabling */
		if (enable) {
			phy_ac_nap_enable_majrev44(pi, TRUE);
			MOD_PHYREG(pi, NapCtrl, nap_en, TRUE);
			MOD_PHYREG(pi, NapCtrl, napCrsRstAccDis, TRUE);
		} else {
			MOD_PHYREG(pi, NapCtrl, nap_en, FALSE);
			MOD_PHYREG(pi, NapCtrl, napCrsRstAccDis, FALSE);
			phy_ac_nap_enable_majrev44(pi, FALSE);
		}
	} else {
		if (enable) {
			/* Set napping related params */
			phy_ac_set_nap_params(pi);
		} else {
			/* Reset napping related params */
			phy_ac_reset_nap_params(pi);
		}

		/* Indicate Ucode Napping feature enabled/disabled using host flag */
		(enable == TRUE) ? wlapi_mhf(pi->sh->physhim, MHF4, MHF4_NAPPING_ENABLE,
				MHF4_NAPPING_ENABLE, WLC_BAND_ALL) :
				wlapi_mhf(pi->sh->physhim, MHF4,
				MHF4_NAPPING_ENABLE, 0, WLC_BAND_ALL);

		/* Reconfigure AGC parameters */
		if (agc_reconfig) {
			if (enable) {
				/* Configure SSAGC */
				if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
					phy_ac_agc_config(pi, SINGLE_SHOT_AGC);
				} else {
					phy_ac_agc_config(pi, FAST_AGC);
				}
			} else {
				/* Configure FastAGC */
				phy_ac_agc_config(pi, FAST_AGC);
			}
		}
	}

	/* Resume MAC */
	wlc_phy_conditional_resume(pi, &suspend);
}

void
phy_ac_nap_ed_thresh_cal(phy_info_t *pi, int8 *cmplx_pwr_dBm)
{
	uint32 lo_thresh[] = {0x0, 0x0, 0x0, 0x0, 0x0};
	uint32 hi_thresh[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff};
	uint8 i, j, core;
	uint16 stage_sig_energy, stage_noise_energy, min_sig_energy, max_noise_energy;
	uint16 adj_fact;
	int8 max_cmplx_pwr;
	int8 tbl_idx, tbl_sz;

	/* Obtained from TCL acphy_nap_ed_thresh_energy_compute proc
	   0.4 noise energy = 10^((iq_dbm - 33)/10)*2330168888.8885
	*/
	uint16 p4us_noise_energy_tbl[] = {233, 293, 369, 465, 585, 737, 928, 1168, 1470, 1851,
			2330, 2934, 3693, 4649, 5853, 7369, 9277, 11679, 14702, 18509, 23302
	};

	/* Obtained from TCL acphy_nap_ed_thresh_energy_compute proc
	   0.4 sig energy = 10^((iq_dbm + sig_pwr_del - 33)/10)*2330168888.8885
	*/
	uint16 p4us_sig_energy_tbl[] = {434, 546, 688, 866, 1090, 1372, 1727, 2175, 2738, 3447,
			4339, 5462, 6877, 8657, 10899, 13721, 17274, 21746, 27377, 34466, 43390
	};

	/* Register to configure LOW thresholds */
	uint16 lo_thresh_reg_addr[] = {
		ACPHY_REG(pi, nap_ed_thld_lo_1),
		ACPHY_REG(pi, nap_ed_thld_lo_2),
		ACPHY_REG(pi, nap_ed_thld_lo_3),
		ACPHY_REG(pi, nap_ed_thld_lo_4),
		ACPHY_REG(pi, nap_ed_thld_lo_5)
	};

	/* Register to configure HI thresholds */
	uint16 hi_thresh_reg_addr[] = {
		ACPHY_REG(pi, nap_ed_thld_hi_1),
		ACPHY_REG(pi, nap_ed_thld_hi_2),
		ACPHY_REG(pi, nap_ed_thld_hi_3),
		ACPHY_REG(pi, nap_ed_thld_hi_4),
		ACPHY_REG(pi, nap_ed_thld_hi_5)
	};

	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	phy_ac_nap_params_t *nap_params = pi_ac->napi->nap_params;

	/* Find the maximum power seen across cores */
	max_cmplx_pwr = -100;
	FOREACH_CORE(pi, core) {
		/* DVGA gain is already accounted when computing power dBm */
		if (cmplx_pwr_dBm[core] > max_cmplx_pwr) {
			max_cmplx_pwr = cmplx_pwr_dBm[core];
		}
	}

	/* Find the index to LUT, first entry in table is for -37 dBm */
	tbl_idx = max_cmplx_pwr + 37;

	/* Out of bound */
	tbl_sz = ARRAYSIZE(p4us_sig_energy_tbl);
	if ((tbl_idx < 0) || (tbl_idx > (tbl_sz - 1))) {
		tbl_idx = (tbl_idx < 0) ? 0 : (tbl_sz - 1);
	}

	/* Compute ED thresholds (low and high) */
	for (i = nap_params->ed_thresh_cal_start_stage; i <= 5; i++) {
		j = i - 1;
		stage_sig_energy = p4us_sig_energy_tbl[tbl_idx] * i;
		stage_noise_energy = p4us_noise_energy_tbl[tbl_idx] * i;

		adj_fact = (stage_sig_energy * nap_params->nap_lo_th_adj[j]) / 100;
		min_sig_energy = stage_sig_energy - adj_fact;

		adj_fact = (stage_noise_energy * nap_params->nap_hi_th_adj[j]) / 100;
		max_noise_energy = stage_noise_energy + adj_fact;

		lo_thresh[j] = min_sig_energy;
		hi_thresh[j] = max_noise_energy;
	}

	/* since we are touching phy regs mac has to be suspended */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (ACMAJORREV_36(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		uint8 scale_conf;
		uint16 spare_reg;

		/* Nap ED threshold scaling : CRDOT11ACPHY-2200 */
		scale_conf = phy_ac_nap_ed_thresh_scale(pi, lo_thresh, hi_thresh,
				nap_params->ed_thresh_cal_start_stage);

		/* Configure scale config : SpareReg(13:12) */
		spare_reg = (READ_PHYREG(pi, SpareReg) & 0xcfff) | (scale_conf << 12);
		WRITE_PHYREG(pi, SpareReg, spare_reg);
	}

	/* Write thresholds to registers */
	for (i = 0; i < 5; i++) {
		phy_utils_write_phyreg(pi, lo_thresh_reg_addr[i], (uint16)lo_thresh[i]);
		phy_utils_write_phyreg(pi, hi_thresh_reg_addr[i], (uint16)hi_thresh[i]);
	}

	/* resume mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_ac_nap_get_status(phy_type_nap_ctx_t *ctx, uint16 *bitmap, uint8 *hwstatus, uint32 *ms)
{
	phy_ac_nap_info_t *napi = (phy_ac_nap_info_t *)ctx;
	phy_info_t *pi = napi->pi;

	if (bitmap != NULL) {
		*bitmap = napi->nap_disable_reqs;
	}

	if (hwstatus != NULL) {
		if (pi->sh->clk) {
			bool suspend = FALSE;
			/* Suspend MAC if haven't done so */
			wlc_phy_conditional_suspend(pi, &suspend);
			*hwstatus = (READ_PHYREGFLD(pi, NapCtrl, nap_en) == 1) ? NAP_HWCFG : 0;
			/* Resume MAC */
			wlc_phy_conditional_resume(pi, &suspend);
		} else {
			*hwstatus = NAP_NOCLK;
		}
	}

	if (ms != NULL) {
		*ms = napi->nap_disable_reqs ?
			napi->nap_disable_dur + OSL_SYSUPTIME() - napi->nap_disable_last :
			napi->nap_disable_dur;
	}
}

static void
phy_ac_nap_set_disable_req(phy_type_nap_ctx_t *ctx, uint16 req,
	bool disable, bool agc_reconfig, uint8 req_id)
{
	phy_ac_nap_info_t *napi = (phy_ac_nap_info_t *)ctx;
	phy_info_t *pi = napi->pi;
	uint16 bitmap = napi->nap_disable_reqs;
	bool nap_reconfig;

	if (disable) {
		napi->nap_disable_reqs |= req;
	} else {
		napi->nap_disable_reqs &= ~req;
	}
	nap_reconfig = !napi->nap_disable_reqs != !bitmap;

	if (nap_reconfig) {
		if (napi->nap_disable_reqs) {
			napi->nap_disable_last = OSL_SYSUPTIME();
		} else {
			napi->nap_disable_dur += OSL_SYSUPTIME() - napi->nap_disable_last;
		}
	}

	if (pi->sh->clk && (nap_reconfig || agc_reconfig)) {
		phy_ac_nap_enable(pi, !napi->nap_disable_reqs, agc_reconfig);
	}
}

static uint8
phy_ac_nap_ed_thresh_scale(phy_info_t *pi, uint32 *lo_thresh, uint32 *hi_thresh, uint8 strt_stage)
{
	uint8 i;
	uint8 scale_conf_max = 0, scale_amt_max = 0;
	uint8 scale_conf, scale_amt;

	/* Find the max scale required */
	for (i = (strt_stage - 1); i < 5; i++) {
		phy_ac_compute_nap_ed_scale_factor(pi, hi_thresh[i], &scale_conf, &scale_amt);

		if (scale_conf > scale_conf_max) {
			scale_conf_max = scale_conf;
			scale_amt_max = scale_amt;
		}

		phy_ac_compute_nap_ed_scale_factor(pi, lo_thresh[i], &scale_conf, &scale_amt);

		if (scale_conf > scale_conf_max) {
			scale_conf_max = scale_conf;
			scale_amt_max = scale_amt;
		}
	}

	/* Do the scaling */
	for (i = (strt_stage - 1); i < 5; i++) {
		hi_thresh[i] = hi_thresh[i] >> scale_amt_max;
		lo_thresh[i] = lo_thresh[i] >> scale_amt_max;
	}

	return scale_conf_max;
}

static void
phy_ac_compute_nap_ed_scale_factor(phy_info_t *pi, uint32 thresh, uint8 *scale_conf,
		uint8 *scale_amt)
{
	uint32 th_13bits = ((2 << 12) - 2);
	uint32 th_15bits = ((2 << 14) - 2);
	uint32 th_18bits = ((2 << 17) - 2);
	uint32 th_21bits = ((2 << 20) - 2);

	if (thresh <= th_13bits) {
		*scale_conf = 0;
		*scale_amt = 0;
	} else if ((thresh > th_13bits) && (thresh <= th_15bits)) {
		*scale_conf = 1;
		*scale_amt = 2;
	} else if ((thresh > th_15bits) && (thresh <= th_18bits)) {
		*scale_conf = 2;
		*scale_amt = 5;
	} else if ((thresh > th_18bits) && (thresh <= th_21bits)) {
		*scale_conf = 3;
		*scale_amt = 8;
	} else {
		PHY_ERROR(("%s: Threshold %d is more than scalable value\n", __FUNCTION__, thresh));
		ROMMABLE_ASSERT(0);
	}
}

static int8
phy_ac_nap_get_energy_threshold(phy_type_nap_ctx_t *ctx)
{
	phy_ac_nap_info_t *info = (phy_ac_nap_info_t *)ctx;
	return info->nap_energy_threshold;

}
#endif /* WL_NAP */
