/*
 * ACPHY Channel Manager module implementation
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
 * $Id: phy_ac_chanmgr.c 768567 2018-10-17 21:00:23Z $
 */

#include <wlc_cfg.h>
#if (ACCONF != 0) || (ACCONF2 != 0)
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <qmath.h>
#include "phy_type_chanmgr.h"
#include <phy_chanmgr_api.h>
#include <phy_ac.h>
#include <phy_ac_chanmgr.h>
#include <phy_ac_antdiv.h>
#include <phy_ac_tpc.h>
#include <phy_papdcal.h>

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
#include <wlc_phy_shim.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phytbl_20691.h>
#include <wlc_phytbl_20693.h>
#include <wlc_phytbl_20694.h>
#include <wlc_phytbl_20696.h>
#include <wlc_phytbl_20697.h>
#include <wlc_phytbl_20698.h>
#include <wlc_phytbl_20704.h>
#include <wlc_phytbl_ac.h>
#include <phy_btcx.h>
#include <phy_tpc.h>
#include <phy_ac_dccal.h>
#include <phy_ac_noise.h>
#include <phy_ac_hirssi.h>
#include <phy_rxgcrs_api.h>
#include <phy_rxiqcal.h>
#include <phy_txiqlocal.h>
#include <phy_ac_rxiqcal.h>
#include <phy_ac_txiqlocal.h>
#include <phy_rxgcrs.h>
#include <hndpmu.h>
#include <sbchipc.h>
#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_ac_info.h>
#include <phy_ocl_api.h>
#include <phy_stf.h>
#include <phy_radar_st.h>
#include <phy_smc_api.h>
#include <phy_vasip_api.h>

#ifdef WL_ETMODE
#include <phy_ac_et.h>
#endif /* WL_ETMODE */
#include "phy_ac_nap.h"

#ifdef WL_DSI
#include "phy_ac_dsi.h"
#endif /* WL_DSI */

#include <phy_rstr.h>

#ifdef WLC_SW_DIVERSITY
#include "phy_ac_antdiv.h"
#endif // endif

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif
#include <bcmdevs.h>

#ifdef WL_DSI
#include "fcbs.h"
#endif /* WL_DSI */

#ifdef WL_AIR_IQ
enum {
	lna2g_cfg1 = 0,
	lna2g_cfg2,
	lna5g_cfg1,
	lna5g_cfg2,
	lna5g_cfg3,
	lna5g_cfg4,
	logen2g_rccr,
	logen5g_epapd,
	logen5g_rccr,
	rx_bb_2g_ovr_east,
	rxiqmix_cfg1,
	rxmix2g_cfg1,
	rxrf2g_cfg2,
	rx_top_2g_ovr_east2,
	rx_top_2g_ovr_east,
	rx_top_5g_ovr,
	tia_cfg1,
	tia_cfg5,
	tia_cfg6,
	tia_cfg7,
	tia_cfg8,
	lo2g_logen0_cfg1,
	lo2g_logen1_cfg1,
	lo2g_vco_drv_cfg1,
	lo5g_core0_cfg1,
	lo5g_core1_cfg1,
	lo5g_core2_cfg1,
	REGS_CNT_20693_3PLUS1
};
typedef struct {
	int8	save_count;
	uint16	regs[REGS_CNT_20693_3PLUS1];
} phy_ac_chanmgr_20693_3plus1_regs_t;
#endif /* WL_AIR_IQ */
#ifdef PHYWAR_43012_HW43012_211_RF_SW_CTRL
static void phy_ac_WAR_43012_rf_sw_ctrl_pinmux(phy_info_t *pi);
#endif /* PHYWAR_43012_HW43012_211_RF_SW_CTRL */

#define RFSEQEXT_TBL_SZ_PER_CORE_28NM 11

/* module private states */
typedef struct phy_ac_chanmgr_config_info {
	uint8	srom_tssisleep_en; /* TSSI sleep enable */
	uint8	srom_txnoBW80ClkSwitch; /* 5G Tx BW80 AFE CLK switch */
	uint8	vlinpwr2g_from_nvram;
	uint8	vlinpwr5g_from_nvram;
	int8	srom_papdwar; /* papd war enable and threshold */
	bool	srom_paprdis; /* papr disable */
	bool	srom_nonbf_logen_mode_en;	/* to enable non-bf logen mode  */
} phy_ac_chanmgr_config_info_t;

struct phy_ac_chanmgr_info {
	phy_info_t		*pi;
	phy_ac_info_t		*aci;
	phy_chanmgr_info_t	*cmn_info;
	phy_ac_chanmgr_data_t *data; /* shared data */
	phy_ac_chanmgr_config_info_t *cfg; /* configuration params */
	chan_info_tx_farrow(*tx_farrow)[ACPHY_NUM_CHANS];
	chan_info_rx_farrow(*rx_farrow)[ACPHY_NUM_CHANS];
	uint8	acphy_cck_dig_filt_type;
	uint8	chsm_en, ml_en;
	uint8	use_fast_adc_20_40;
	uint8	acphy_enable_smth;
	uint8	acphy_smth_dump_mode;
	int8	tdcs_160_en;
	int8    chanup_ovrd;
	bool	vco_12GHz;
	bool	FifoReset; /* flag to hold FifoReset val */
	chanspec_t	radio_chanspec_sc;	/* 3x3_1x1 mode, setting */
	uint16	prev_pmu_ulbmode;
	/* lesi_perband: used for NVRAM based LESI enable/disable
	* 1st entry: 2G, 2nd entry: 5G
	*/
	int8 lesi_perband[2];

	bool veloce_farrow_db;
	/* add other variable size variables here at the end */
#ifdef WL_AIR_IQ
	phy_ac_chanmgr_20693_3plus1_regs_t regs_save_20693_3plus1;
#endif // endif
};

typedef struct {
	phy_ac_chanmgr_info_t info;
	phy_ac_chanmgr_data_t data;
	phy_ac_chanmgr_config_info_t cfg;
} phy_ac_chanmgr_mem_t;

#define N_BITS_RX_FARR (24)

#define ZEROS_TBL_SZ	64		/* in 32-bit DWORDs */

/* 20693 Radio functions */

/* local functions */
static void phy_ac_chanmgr_write_tx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, int sc_mode);
static void phy_ac_chanmgr_chanspec_set(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static void phy_ac_chanmgr_upd_interf_mode(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static void phy_ac_chanmgr_preempt(phy_type_chanmgr_ctx_t *ctx, bool enable_preempt,
    bool EnablePostRxFilter_Proc);
static int phy_ac_chanmgr_get_chanspec_bandrange(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static bool phy_ac_chanmgr_is_txbfcal(phy_type_chanmgr_ctx_t *ctx);
static bool phy_ac_chanmgr_is_smth_en(phy_type_chanmgr_ctx_t *ctx);
static void wlc_tiny_setup_coarse_dcc(phy_info_t *pi);
#if defined(WLTEST)
static int phy_ac_chanmgr_get_smth(phy_type_chanmgr_ctx_t *ctx, int32* ret_int_ptr);
static int phy_ac_chanmgr_set_smth(phy_type_chanmgr_ctx_t *ctx, int8 int_val);
#endif /* defined(WLTEST) */
static void phy_ac_chanmgr_tdcs_enable_160m(phy_info_t *pi, bool set_val);
static void phy_ac_chanmgr_set_spexp_matrix(phy_info_t *pi);
static void phy_ac_chanmgr_dccal_force(phy_info_t *pi);

static void femctrl_clb_majrev_ge40(phy_info_t *pi, int band_is_2g, int slice);
static void wlc_phy_low_pwr_logen_setting(phy_info_t *pi, bool mimo_bf_enable);
static void wlc_phy_bphymrc_hwconfig(phy_info_t *pi);
static const char BCMATTACHDATA(rstr_VlinPwr2g_cD)[]                  = "VlinPwr2g_c%d";
static const char BCMATTACHDATA(rstr_VlinPwr5g_cD)[]                  = "VlinPwr5g_c%d";
static const char BCMATTACHDATA(rstr_Vlinmask2g_cD)[]                 = "Vlinmask2g_c%d";
static const char BCMATTACHDATA(rstr_Vlinmask5g_cD)[]                 = "Vlinmask5g_c%d";
static const char BCMATTACHDATA(rstr_AvVmid_cD)[]                     = "AvVmid_c%d";

/* function to read femctrl params from nvram */
static void BCMATTACHFN(phy_ac_chanmgr_nvram_attach)(phy_ac_chanmgr_info_t *ac_info);
static void phy_ac_chanmgr_std_params_attach(phy_ac_chanmgr_info_t *ac_info);
static int phy_ac_chanmgr_init_chanspec(phy_type_chanmgr_ctx_t *ctx);
static int WLBANDINITFN(phy_ac_chanmgr_bsinit)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec,
	bool forced);
static int WLBANDINITFN(phy_ac_chanmgr_bwinit)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static bool BCMATTACHFN(phy_ac_chanmgr_attach_farrow)(phy_ac_chanmgr_info_t *chanmgri);
static bool BCMATTACHFN(phy_ac_chanmgr_attach_paprr)(phy_ac_chanmgr_info_t *ac_info);

const uint16 *BCMRAMFN(get_rfseq_majrev44)(phy_info_t *pi, phy_ac_rfseq_tbl_id_t tbl_id);

static void wlc_phy_set_rfseqext_tbl_majrev40(phy_info_t *pi);
static void wlc_phy_set_rfseqext_tbl_majrev44(phy_info_t *pi);
static void wlc_phy_set_rfseqext_tbl_majrev47(phy_info_t *pi);
static void wlc_phy_set_papdlut_dynradioregtbl_majrev_ge40(phy_info_t *pi);
static void wlc_phy_set_analog_lpf(phy_info_t *pi, uint8 *tx_bq1, uint8 *tx_bq2,
                                   uint8 *rx_bq1, uint8 *rx_bq2);

#ifdef WL_AIR_IQ
static void wlc_phy_radio20693_setup_logen_3plus1(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc);
static void wlc_phy_radio20693_save_3plus1(phy_info_t *pi, uint8 core, uint8 restore);
static void wlc_phy_save_regs_3plus1(phy_info_t *pi, int32 set_val);
#endif // endif
/* register phy type specific implementation */
phy_ac_chanmgr_info_t *
BCMATTACHFN(phy_ac_chanmgr_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_chanmgr_info_t *cmn_info)
{
	phy_ac_chanmgr_info_t *ac_info;
	phy_type_chanmgr_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_chanmgr_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;
	ac_info->data = &(((phy_ac_chanmgr_mem_t *) ac_info)->data);
	ac_info->cfg = &(((phy_ac_chanmgr_mem_t *) ac_info)->cfg);
	ac_info->radio_chanspec_sc = pi->radio_chanspec;

	if (!TINY_RADIO(pi) && !phy_ac_chanmgr_attach_farrow(ac_info)) {
		PHY_ERROR(("%s: phy_ac_chanmgr_attach_farrow failed\n", __FUNCTION__));
		goto fail;
	}
	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) && !phy_ac_chanmgr_attach_paprr(ac_info)) {
		PHY_ERROR(("%s: phy_ac_chanmgr_attach_paprr failed\n", __FUNCTION__));
		goto fail;
	}
	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.init_chanspec = phy_ac_chanmgr_init_chanspec;
	fns.is_txbfcal = phy_ac_chanmgr_is_txbfcal;
	fns.is_smth_en = phy_ac_chanmgr_is_smth_en;
	fns.get_bandrange = phy_ac_chanmgr_get_chanspec_bandrange;
	fns.chanspec_set = phy_ac_chanmgr_chanspec_set;
	fns.interfmode_upd = phy_ac_chanmgr_upd_interf_mode;
	fns.preempt = phy_ac_chanmgr_preempt;
	fns.tdcs_enable_160m = phy_ac_chanmgr_tdcs_enable_160m;
	fns.dccal_force = phy_ac_chanmgr_dccal_force;
#if defined(WLTEST)
	fns.get_smth = phy_ac_chanmgr_get_smth;
	fns.set_smth = phy_ac_chanmgr_set_smth;
#endif /* defined(WLTEST) */
	fns.bsinit = phy_ac_chanmgr_bsinit;
	fns.bwinit = phy_ac_chanmgr_bwinit;
	fns.ctx = ac_info;

	/* Read VLIN signal parameters from NVRAM */
	if (!TINY_RADIO(pi))
		wlc_phy_nvram_vlin_params_read(pi);
	/* Read AVVMID signal from NVARM */
	wlc_phy_nvram_avvmid_read(pi);

	/* set up srom cfg */
	phy_ac_chanmgr_nvram_attach(ac_info);
	phy_ac_chanmgr_std_params_attach(ac_info);

	if (ACMAJORREV_44(pi->pubpi->phy_rev) || ACMAJORREV_46(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (ISSIM_ENAB(pi->sh->sih))
			pi->u.pi_acphy->sromi->srom_low_adc_rate_en = 0;
		else {
			pi->u.pi_acphy->sromi->srom_low_adc_rate_en =
			    (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
			                                       rstr_low_adc_rate_en, 1);
			if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en == 1) {
					pi->u.pi_acphy->sromi->srom_low_adc_rate_en = 1;
				} else {
					pi->u.pi_acphy->sromi->srom_low_adc_rate_en = 0;
				}
			}
		}
	} else {
		pi->u.pi_acphy->sromi->srom_low_adc_rate_en = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
			rstr_low_adc_rate_en, 0);
	}
	if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en >= 2) {
		PHY_ERROR(("%s: ERROR: wrong low_adc_rate_en value (%d)\n", __FUNCTION__,
			pi->u.pi_acphy->sromi->srom_low_adc_rate_en));
		goto fail;
	}

	if (phy_chanmgr_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}
#ifdef WL_AIR_IQ
	ac_info->regs_save_20693_3plus1.save_count = 0;
#endif // endif
	return ac_info;

	/* error handling */
fail:

	phy_ac_chanmgr_unregister_impl(ac_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_chanmgr_unregister_impl)(phy_ac_chanmgr_info_t *ac_info)
{
	phy_info_t *pi;
	phy_chanmgr_info_t *cmn_info;
	int num_bw;
#ifdef ACPHY_1X1_ONLY
	num_bw = 1;
#else
	num_bw = ACPHY_NUM_BW;
#endif // endif

	if (ac_info == NULL)
		return;

	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_chanmgr_unregister_impl(cmn_info);

	if (ac_info->tx_farrow != NULL) {
		phy_mfree(pi, ac_info->tx_farrow,
			num_bw * sizeof(*(ac_info->tx_farrow)));
	}
	if (ac_info->rx_farrow != NULL) {
		phy_mfree(pi, ac_info->rx_farrow,
			num_bw * sizeof(*(ac_info->rx_farrow)));
	}

	phy_mfree(pi, ac_info, sizeof(phy_ac_chanmgr_mem_t));
}

static void
BCMATTACHFN(phy_ac_chanmgr_std_params_attach)(phy_ac_chanmgr_info_t *chanmgri)
{
	phy_info_t *pi = chanmgri->pi;
	BCM_REFERENCE(pi);
	chanmgri->data->curr_band2g = CHSPEC_IS2G(chanmgri->pi->radio_chanspec);
	chanmgri->aci->curr_bw = CHSPEC_BW(chanmgri->pi->radio_chanspec);
	chanmgri->data->fast_adc_en = 0;
	chanmgri->use_fast_adc_20_40 = 0;
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
		ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		if (chanmgri->chsm_en) {
			chanmgri->acphy_enable_smth = SMTH_ENABLE;
		} else {
			chanmgri->acphy_enable_smth = SMTH_DISABLE;
		}
	} else if ((ACMAJORREV_32(pi->pubpi->phy_rev) ||
	            ACMAJORREV_33(pi->pubpi->phy_rev) ||
	            ACMAJORREV_37(pi->pubpi->phy_rev) ||
	            ACMAJORREV_47(pi->pubpi->phy_rev))) {
		chanmgri->acphy_enable_smth = 0;

		if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi))) {
			/* channel smoothing is not supported in 80p80 */
			if (!PHY_AS_80P80(pi, pi->radio_chanspec))
				chanmgri->acphy_enable_smth = 1;
		} else {
			chanmgri->acphy_enable_smth = 0;
		}
	}
	chanmgri->acphy_smth_dump_mode = SMTH_NODUMP;
	chanmgri->tdcs_160_en = 0; /* Disable 160M TDCS by default */
	chanmgri->chanup_ovrd = -1; /* Auto */
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349A0 uses 12Ghz VCO */
		chanmgri->vco_12GHz = TRUE;
	} else {
		/* Disable 12Ghz VCO for all other chips */
		chanmgri->vco_12GHz = FALSE;
	}

	chanmgri->veloce_farrow_db = FALSE;
	if (ISSIM_ENAB(pi->sh->sih)) {
		/* Only for QT/Veloce */
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			/* 43684 veloce db uses this bit to signify that its a farrow db */
			chanmgri->veloce_farrow_db =  READ_PHYREGFLD(pi,
			                                             spur_can_fll_enable_p0, pi_en);
		}
	}
}

static int
WLBANDINITFN(phy_ac_chanmgr_init_chanspec)(phy_type_chanmgr_ctx_t *ctx)
{
	phy_ac_chanmgr_info_t *chanmgri = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_acphy_t *pi_ac = chanmgri->aci;
	phy_info_t *pi = chanmgri->pi;
#ifdef WL_AIR_IQ
	uint16 phymode = phy_get_phymode(chanmgri->pi);
#endif /* WL_AIR_IQ */

	chanmgri->data->both_txchain_rxchain_eq_1 = FALSE;

	/* indicate chanspec control flow to follow init path */
	mboolset(pi_ac->CCTrace, CALLED_ON_INIT);
#ifdef WL_AIR_IQ
	/* After a reinit, must reset phymode */
	if (phymode == PHYMODE_BGDFS) {
		phy_set_phymode(pi, PHYMODE_MIMO);
	}
#endif /* WL_AIR_IQ */
	wlc_phy_chanspec_set_acphy(pi, pi->radio_chanspec);
	mboolclr(pi_ac->CCTrace, CALLED_ON_INIT);
	chanmgri->data->init_done = TRUE;

	if (!(ACMAJORREV_4(pi->pubpi->phy_rev))) {
		if (TINY_RADIO(pi) || (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev) &&
		    (pi->u.pi_acphy->sromi->srom_low_adc_rate_en))) {
			phy_ac_rfseq_mode_set(pi, 1);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_47(pi->pubpi->phy_rev)) {
			/* Disable core2core sync (and enable later)
			 * as AFE overrides below can mess with it
			 */
			phy_ac_chanmgr_core2core_sync_setup(chanmgri, FALSE);
			/* In low rate TSSI mode, adc running low,
			 * use overrideds to configure ADC to normal mode
			*/
			wlc_phy_low_rate_adc_enable_acphy(pi, FALSE);
			wlc_phy_radio20698_afe_div_ratio(pi, 1, 0, 0);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_51(pi->pubpi->phy_rev)) {
			/* In low rate TSSI mode, adc running low,
			 * use overrideds to configure ADC to normal mode
			*/
			wlc_phy_low_rate_adc_enable_acphy(pi, FALSE);
			wlc_phy_radio20704_afe_div_ratio(pi, 1);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_40(pi->pubpi->phy_rev)) {
			/* In low rate TSSI mode, adc running low,
			 * use overrideds to configure ADC to normal mode
			*/
			wlc_phy_low_rate_adc_enable_acphy(pi, FALSE);
			wlc_phy_radio20694_afe_div_ratio(pi, 1, 0);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			/* In low rate TSSI mode, adc running low,
			 * use overrideds to configure ADC to normal mode
			*/
			wlc_phy_low_rate_adc_enable_acphy(pi, FALSE);
			wlc_phy_radio20697_afe_div_ratio(pi, 1);
		}
		wlc_phy_txpwrctrl_idle_tssi_meas_acphy(pi);
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_47(pi->pubpi->phy_rev)) {
			/* Remove afe_div overrides */
			wlc_phy_radio20698_afe_div_ratio(pi, 0, 0, 0);
			wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
			/* Enable core2core sync back */
			phy_ac_chanmgr_core2core_sync_setup(chanmgri, TRUE);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_51(pi->pubpi->phy_rev)) {
			/* Remove afe_div overrides */
			wlc_phy_radio20704_afe_div_ratio(pi, 0);
			wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_40(pi->pubpi->phy_rev)) {
			/* Remove afe_div overrides */
			wlc_phy_radio20694_afe_div_ratio(pi, 0, 0);
			wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
		}
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
			ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			/* Remove afe_div overrides */
			wlc_phy_radio20697_afe_div_ratio(pi, 0);
			wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
		}
		if (TINY_RADIO(pi) || (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev) &&
			(pi->u.pi_acphy->sromi->srom_low_adc_rate_en))) {
			phy_ac_rfseq_mode_set(pi, 0);
		}
	}

	/* cmn/chanspec_set calls this, but cmn/chanspec_set is NOT called during init
	   and so if channel is not changed after init, it could lead to preempt not being called
	*/
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		phy_ac_chanmgr_preempt(ctx,
		                       ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0),
		                       FALSE);
	}

	return BCME_OK;
}

/* band specific init */
static int
WLBANDINITFN(phy_ac_chanmgr_bsinit)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec, bool forced)
{
	phy_ac_chanmgr_info_t *chanmgri = (phy_ac_chanmgr_info_t *)ctx;
	if (forced) {
		return phy_init(chanmgri->pi, chanspec);
	} else {
		return BCME_UNSUPPORTED;
	}
}

/* band width init */
static int
WLBANDINITFN(phy_ac_chanmgr_bwinit)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	return BCME_UNSUPPORTED;
}

/* Internal data api between ac modules */
phy_ac_chanmgr_data_t *
phy_ac_chanmgr_get_data(phy_ac_chanmgr_info_t *ci)
{
	return ci->data;
}

void
phy_ac_chanmgr_set_both_txchain_rxchain(phy_ac_chanmgr_info_t *ci, uint8 rxchain, uint8 txchain)
{
	ci->data->both_txchain_rxchain_eq_1 = ((rxchain == 1) && (txchain == 1)) ? TRUE : FALSE;
}

#ifdef WL_PROXDETECT
void
phy_ac_chanmgr_save_smoothing(phy_ac_chanmgr_info_t *ci, uint8 *enable, uint8 *dump_mode)
{
	*enable = ci->acphy_enable_smth;
	*dump_mode = ci->acphy_smth_dump_mode;
}
#endif /* WL_PROXDETECT */
/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#define TXMAC_IFHOLDOFF_DEFAULT		0x12	/* 9.0us */
#define TXMAC_MACDELAY_DEFAULT		0x2a8	/* 8.5us */
#define TXDELAY_BW80		3	/* 3.0us */

#define TXMAC_IFHOLDOFF_43012A0		0x1a	/* 13.0us */
#define TXMAC_MACDELAY_43012A0		0x3e8	/* 12.5us */

#define TXMAC_IFHOLDOFF_43684B0		0x11	/* 8.5us */
#define TXMAC_IFHOLDOFF_63178		0x11	/* 8.5us */

#define ACPHY_VCO_2P5V	1
#define ACPHY_VCO_1P35V	0

#define WLC_TINY_GI_MULT_P12		4096U
#define WLC_TINY_GI_MULT_TWEAK_P12	4096U
#define WLC_TINY_GI_MULT			WLC_TINY_GI_MULT_P12

#define MAX_VALID_EDTHRESH		(-15) /* max level of valid rssi */

typedef struct _chan_info_common {
	uint16 chan;		/* channel number */
	uint16 freq;		/* in Mhz */
} chan_info_common_t;

static const uint16 qt_rfseq_val1[] = {0x8b5, 0x8b5, 0x8b5, 0x8b5};
static const uint16 qt_rfseq_val2[] = {0x0, 0x0, 0x0, 0x0};
static const uint16 rfseq_reset2rx_dly[] = {12, 2, 2, 4, 4, 6, 1, 4, 1, 2, 1, 1, 1, 1, 1, 1};
static const uint16 rfseq_updl_lpf_hpc_ml[] = {0x0aaa, 0x0aaa};
static const uint16 rfseq_updl_tia_hpc_ml[] = {0x0222, 0x0222};
static const uint16 rfseq_reset2rx_cmd[] = {0x4, 0x3, 0x6, 0x5, 0x2, 0x1, 0x8,
            0x2a, 0x2b, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};

uint16 const rfseq_rx2tx_cmd[] =
{0x0, 0x1, 0x2, 0x8, 0x5, 0x0, 0x6, 0x3, 0xf, 0x4, 0x0, 0x35, 0xf, 0x0, 0x36, 0x1f};
static uint16 rfseq_rx2tx_dly_epa1_20[] =
	{0x8, 0x6, 0x4, 0x4, 0x6, 0x2, 0x10, 60, 0x2, 0x5, 0x1, 0x4, 0xe4, 0xfa, 0x2, 0x1};
static uint16 rfseq_rx2tx_dly_epa1_40[] =
	{0x8, 0x6, 0x4, 0x4, 0x6, 0x2, 0x10, 30, 0x2, 0xd, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
static uint16 rfseq_rx2tx_dly_epa1_80[] =
	{0x8, 0x6, 0x4, 0x4, 0x6, 0x2, 0x10, 20, 0x2, 0x17, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};

static uint16 rfseq_rx2tx_cmd_noafeclkswitch[] =
        {0x0, 0x1, 0x5, 0x8, 0x2, 0x3d, 0x6, 0x3, 0xf, 0x4, 0x3e, 0x35, 0xf, 0x0, 0x36, 0x1f};
static uint16 rfseq_rx2tx_cmd_noafeclkswitch_dly[] =
        {0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x10, 0x26, 0x2, 0x5, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
static uint16 rfseq_rx2tx_cmd_afeclkswitch[] =
        {0x0, 0x3d, 0x3e, 0x1, 0x5, 0x8, 0x2, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x0, 0x36, 0x1f};
static uint16 rfseq_rx2tx_cmd_afeclkswitch_dly[] =
        {0x2, 0x8, 0x1, 0xd, 0x6, 0x4, 0x4, 0x10, 0x24, 0x2, 0x5, 0x4, 0xfa, 0xfa, 0x2, 0x1};

static uint16 rfseq_tx2rx_cmd_noafeclkswitch[] =
  {0x00, 0x04, 0x03, 0x06, 0xb3, 0x02, 0x3d, 0x05, 0xb3, 0x01, 0x08, 0x2a, 0x3e, 0x0f, 0x2b, 0x1f};
static uint16 rfseq_tx2rx_dly_noafeclkswitch[] =
  {0x02, 0x01, 0x07, 0x04, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x0a, 0x0a, 0x04, 0x02, 0x01};

static uint16 rfseq_rx2tx_cmd_withtssisleep[] =
{0x0000, 0x0001, 0x0005, 0x0008, 0x0002, 0x0006, 0x0003, 0x000f, 0x0004, 0x0035,
0x000f, 0x0000, 0x0000, 0x0036, 0x0080, 0x001f};
static uint16 rfseq_rx2tx_dly_withtssisleep[] =
{0x0008, 0x0006, 0x0006, 0x0004, 0x0006, 0x0010, 0x0026, 0x0002, 0x0006, 0x0004,
0x00ff, 0x00ff, 0x00a8, 0x0004, 0x0001, 0x0001};
static uint16 rfseq_rx2tx_cmd_rev15_ipa[] =
        {0x0, 0x1, 0x5, 0x8, 0x2, 0x6, 0x35, 0x3, 0xf, 0x4, 0x0f, 0x0, 0x0, 0x36, 0x00, 0x1f};
static uint16 rfseq_rx2tx_cmd_rev15_ipa_withtssisleep[] =
        {0x0, 0x1, 0x5, 0x8, 0x2, 0x6, 0x35, 0x3, 0xf, 0x4, 0x0f, 0x0, 0x0, 0x36, 0x80, 0x1f};
static uint16 rfseq_rx2tx_dly_rev15_ipa20[] =
	{0x8, 0x6, 0x6, 0x4, 0x6, 0x10, 40, 0x26, 0x2, 0x6, 0xff, 0xff, 0x56, 0x4, 0x1, 0x1};
static uint16 rfseq_rx2tx_dly_rev15_ipa40[] =
	{0x8, 0x6, 0x6, 0x4, 0x6, 0x10, 16, 0x26, 0x2, 0x6, 0xff, 0xff, 0x6e, 0x4, 0x1, 0x1};

static const uint16 rfseq_tx2rx_cmd[] =
{0x4, 0x3, 0x6, 0x5, 0x0, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x0, 0xf, 0x2b, 0x1f, 0x1f, 0x1f};
static uint16 rfseq_tx2rx_dly[] =
  {0x08, 0x4, 0x2, 0x2, 0x1, 0x3, 0x4, 0x6, 0x04, 0xa, 0x4, 0x2, 0x01, 0x01, 0x01, 0x1};

static const uint16 rf_updh_cmd_clamp[] = {0x2a, 0x07, 0x0a, 0x00, 0x08, 0x2b, 0x1f, 0x1f};
static const uint16 rf_updh_dly_clamp[] = {0x01, 0x02, 0x02, 0x02, 0x10, 0x01, 0x01, 0x01};
static const uint16 rf_updl_cmd_clamp[] = {0x2a, 0x07, 0x08, 0x0c, 0x0e, 0x2b, 0x1f, 0x1f};
static const uint16 rf_updl_dly_clamp[] = {0x01, 0x06, 0x12, 0x08, 0x10, 0x01, 0x01, 0x01};
static const uint16 rf_updu_cmd_clamp[] = {0x2a, 0x07, 0x08, 0x0e, 0x2b, 0x1f, 0x1f, 0x1f};
static const uint16 rf_updu_dly_clamp[] = {0x01, 0x06, 0x1e, 0x1c, 0x01, 0x01, 0x01, 0x01};

static const uint16 rf_updh_cmd_adcrst[] = {0x07, 0x0a, 0x00, 0x08, 0xb0, 0xb1, 0x1f, 0x1f};
static uint16 rf_updh_dly_adcrst[] = {0x02, 0x02, 0x02, 0x01, 0x0a, 0x01, 0x01, 0x01};
static const uint16 rf_updl_cmd_adcrst[] = {0x07, 0x08, 0x0c, 0x0e, 0xb0, 0xb2, 0x1f, 0x1f};
static uint16 rf_updl_dly_adcrst[] = {0x06, 0x12, 0x08, 0x01, 0x0a, 0x01, 0x01, 0x01};
static const uint16 rf_updu_cmd_adcrst[] = {0x07, 0x08, 0x0e, 0xb0, 0xb1, 0x1f, 0x1f, 0x1f};
static const uint16 rf_updu_dly_adcrst[] = {0x06, 0x1e, 0x1c, 0x0a, 0x01, 0x01, 0x01, 0x01};
static const uint16 rf_updl_dly_dvga[] =  {0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01};

static const uint32 clip2_tbl[6] =
	{0xaa98766, 0xaa98766, 0xaa99877, 0xaaaa988, 0xaaaaaaa, 0xaaaaaaa};
static const uint32 clip2_tbl_maj47[6] =
	{0xaa98643, 0xaaa9844, 0xaaaa966, 0xaaaa996, 0xaaaa996, 0xaaaaa99};
static const uint32 clip2_tbl_maj47_rX[6] =
	{0xaaa6433, 0xaaa7544, 0xaaa9766, 0xaaaa876, 0xaaaa997, 0xaaaaa99};
static const uint8 avvmid_set[25][5][6] = {{{2, 1, 2,   107, 150, 110},  /* pdet_id = 0 */
			       {2, 2, 1,   157, 153, 160},
			       {2, 2, 1,   157, 153, 161},
			       {2, 2, 0,   157, 153, 186},
			       {2, 2, 0,   157, 153, 187}},
			       {{1, 0, 1,   159, 174, 161},  /* pdet_id = 1 */
			       {1, 0, 1,   160, 185, 156},
			       {1, 0, 1,   163, 185, 162},
			       {1, 0, 1,   169, 187, 167},
			       {1, 0, 1,   152, 188, 160}},
			       {{1, 1, 1,   159, 166, 166},  /* pdet_id = 2 */
			       {2, 2, 4,   140, 151, 100},
			       {2, 2, 3,   143, 153, 116},
			       {2, 2, 2,   143, 153, 140},
			       {2, 2, 2,   145, 160, 154}},
			       {{1, 1, 2,   130, 131, 106},  /* pdet_id = 3 */
			       {1, 1, 2,   130, 131, 106},
			       {1, 1, 2,   128, 127, 97},
			       {0, 1, 3,   159, 137, 75},
			       {0, 0, 3,   164, 162, 76}},
			       {{1, 1, 1,   156, 160, 158},  /* pdet_id = 4 */
			       {1, 1, 1,   156, 160, 158},
			       {1, 1, 1,   156, 160, 158},
			       {1, 1, 1,   156, 160, 158},
			       {1, 1, 1,   156, 160, 158}},
			       {{2, 2, 2,   104, 108, 106},  /* pdet_id = 5 */
			       {2, 2, 2,   104, 108, 106},
			       {2, 2, 2,   104, 108, 106},
			       {2, 2, 2,   104, 108, 106},
			       {2, 2, 2,   104, 108, 106}},
			       {{2, 0, 2,   102, 170, 104},  /* pdet_id = 6 */
			       {3, 4, 3,    82, 102,  82},
			       {1, 3, 1,   134, 122, 136},
			       {1, 3, 1,   134, 124, 136},
			       {2, 3, 2,   104, 122, 108}},
			       {{0, 0, 0,   180, 180, 180},  /* pdet_id = 7 */
			       {0, 0, 0,   180, 180, 180},
			       {0, 0, 0,   180, 180, 180},
			       {0, 0, 0,   180, 180, 180},
			       {0, 0, 0,   180, 180, 180}},
			       {{2, 1, 2,   102, 138, 104},  /* pdet_id = 8 */
			       {3, 5, 3,    82, 100,  82},
			       {1, 4, 1,   134, 116, 136},
			       {1, 3, 1,   134, 136, 136},
			       {2, 3, 2,   104, 136, 108}},
			       {{3, 2, 3,    90, 106,  86},  /* pdet_id = 9 */
			       {3, 1, 3,    90, 158,  90},
			       {2, 1, 2,   114, 158, 112},
			       {2, 1, 1,   116, 158, 142},
			       {2, 1, 1,   116, 158, 142}},
			       {{2, 2, 2,   152, 156, 156},  /* pdet_id = 10 */
			       {2, 2, 2,   152, 156, 156},
			       {2, 2, 2,   152, 156, 156},
			       {2, 2, 2,   152, 156, 156},
			       {2, 2, 2,   152, 156, 156}},
			       {{1, 1, 1,   134, 134, 134},  /* pdet_id = 11 */
			       {1, 1, 1,   136, 136, 136},
			       {1, 1, 1,   136, 136, 136},
			       {1, 1, 1,   136, 136, 136},
			       {1, 1, 1,   136, 136, 136}},
			       {{3, 3, 3,    90,  92,  86},  /* pdet_id = 12 */
			       {3, 3, 3,    90,  86,  90},
			       {2, 3, 2,   114,  86, 112},
			       {2, 2, 1,   116, 109, 142},
			       {2, 2, 1,   116, 110, 142}},
			       {{2, 2, 2,   112, 114, 112},  /* pdet_id = 13 */
			       {2, 2, 2,   114, 114, 114},
			       {2, 2, 2,   114, 114, 114},
			       {2, 2, 2,   113, 114, 112},
			       {2, 2, 2,   113, 114, 112}},
			       {{1, 1, 1,   134, 134, 134},  /* pdet_id = 14 */
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168}},
			       {{0, 0, 0,   172, 172, 172},  /* pdet_id = 15 */
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168}},
			       {{3, 2, 3,    90, 106,  86},  /* pdet_id = 16 */
			       {3, 0, 3,    90, 186,  90},
			       {2, 0, 2,   114, 186, 112},
			       {2, 0, 1,   116, 186, 142},
			       {2, 0, 1,   116, 186, 142}},
			       {{4, 4, 4,   50,  45,  50},  /* pdet_id = 17 */
			       {3, 3, 3,    82,  82, 82},
			       {3, 3, 3,    82,  82, 82},
			       {3, 3, 3,    82,  82, 82},
			       {3, 3, 3,    82,  82, 82}},
			       {{5, 5, 5,   61,  61,  61},  /* pdet_id = 18 */
			       {2, 2, 2,   122, 122, 122},
			       {2, 2, 2,   122, 122, 122},
			       {2, 2, 2,   122, 122, 122},
			       {2, 2, 2,   122, 122, 122}},
			       {{2, 2, 2,  152, 156, 156},  /* pdet_id = 19 */
			       {1, 1, 1,   165, 165, 165},
			       {1, 1, 1,   160, 160, 160},
			       {1, 1, 1,   152, 150, 160},
			       {1, 1, 1,   152, 150, 160}},
                       {{3, 3, 3,  108, 108, 108},  /* pdet_id = 20 */
			       {1, 1, 1,   160, 160, 160},
			       {1, 1, 1,   160, 160, 160},
			       {1, 1, 1,   160, 160, 160},
			       {1, 1, 1,   160, 160, 160}},
                       {{2, 2, 2,  110, 110, 110},  /* pdet_id = 21 */
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168},
			       {0, 0, 0,   168, 168, 168}},
			       {{6, 6, 6,   40,  40,  40},  /* pdet_id = 22 */
			       {2, 2, 1,   115, 115, 142},
			       {1, 2, 1,   142, 115, 142},
			       {1, 1, 1,   142, 142, 142},
			       {1, 1, 1,   142, 142, 142}},
			       {{1, 1, 1,  156, 160, 158},  /* pdet_id = 23 */
			       {6, 6, 6,    47,  45,  48},
			       {1, 1, 1,   147, 146, 148},
			       {1, 1, 1,   146, 146, 152},
			       {1, 1, 1,   146, 146, 152}},
			       {{2, 2, 2,   120, 120, 120}, /* pdet_id =24 */
			       {2, 2, 2,   120, 120, 120},
			       {2, 2, 2,   120, 120, 120},
			       {2, 2, 2,   120, 120, 120},
			       {2, 2, 2,   120, 120, 120}}
};

static const uint8 avvmid_set1[16][5][2] = {
	{{1, 154}, {0, 168}, {0, 168}, {0, 168}, {0, 168}},  /* pdet_id = 0 */
	{{1, 145}, {1, 145}, {1, 145}, {1, 145}, {1, 145}},  /* pdet_id = 1 WLBGA */
	{{6,  76}, {1, 160}, {6,  76}, {6,  76}, {6,  76}},  /* pdet_id = 2 */
	{{1, 156}, {1, 152}, {1, 152}, {1, 152}, {1, 152}},  /* pdet_id = 3 */
	{{1, 152}, {1, 152}, {1, 152}, {1, 152}, {1, 152}},  /* pdet_id = 4 WLCSP */
	{{3, 100}, {3,  75}, {3,  75}, {3,  75}, {3,  75}},  /* pdet_id = 5 WLCSP TM */
	{{1, 152}, {0, 166}, {0, 166}, {0, 166}, {0, 166}},  /* pdet_id = 6 WLCSP HK */
	{{1, 145}, {3, 120}, {3, 120}, {3, 120}, {3, 125}},  /* pdet_id = 7 WLiPA */
	{{1, 145}, {1, 155}, {1, 155}, {1, 155}, {1, 155}},  /* pdet_id = 8 WLBGA C0 */
	{{1, 135}, {1, 165}, {1, 165}, {1, 165}, {1, 165}}   /* pdet_id = 9 WLBGA RR FEM */
};
static const uint8 avvmid_set2[16][5][4] = {
	{
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145}},  /* pdet_id = 0 */
	{
		{3, 3, 100, 100},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145}},  /* pdet_id = 1 */
	{
		{4, 4,  95,  95},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145}},  /* pdet_id = 2 */
	{
		{1, 1, 145, 145},
		{3, 3,  90,  90},
		{3, 3,  92,  92},
		{2, 3, 110,  90},
		{2, 3, 110,  93}},  /* pdet_id = 3 */
	{
		{2, 2, 140, 140},
		{2, 2, 145, 145},
		{2, 2, 145, 145},
		{2, 2, 145, 145},
		{2, 2, 145, 145}},  /* pdet_id = 4 */
	{
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{1, 1, 145, 145},
		{2, 2, 110, 110}},  /* pdet_id = 5 */
	{
		{3, 3, 98, 98},
		{2, 1, 122, 150},
		{2, 1, 122, 150},
		{2, 2, 122, 122},
		{2, 2, 122, 122}}  /* pdet_id = 6 */
};

static const uint8 avvmid_set3[16][5][2] = {
	{{1, 115}, {2, 90}, {2, 90}, {2, 90}, {2, 90}},  /* pdet_id = 0 4345 TC */
	{{0, 131}, {0, 134}, {0, 134}, {0, 134}, {0, 134}},  /* pdet_id = 1 4345TC FCBGA EPA */
	{{4, 132}, {4, 127}, {4, 127}, {4, 127}, {4, 127}},  /* pdet_id = 2 4345A0 fcbusol */
	{{0, 150}, {2, 97}, {2, 97}, {2, 97}, {2, 97}},  /* pdet_id = 3 4345A0 fcpagb ipa */
};
static const uint8 avvmid_set4[1][5][4] = {
	{
		{2, 2, 130, 130},
		{2, 2, 130, 130},
		{2, 2, 130, 130},
		{2, 2, 130, 130},
		{2, 2, 130, 130}},  /* pdet_id = 0 */
};

static const uint8 avvmid_set32[6][5][8] = {
	{
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148}},  /* pdet_id = 0 */
	{
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148},
		{1, 1, 1, 1, 148, 148, 148, 148}},  /* pdet_id = 1 */
	{
		{2, 2, 2, 2, 110, 110, 110, 110},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116}},  /* pdet_id = 2 */
	{
		{1, 1, 1, 1, 154, 154, 154, 156},
		{2, 2, 2, 2, 136, 136, 136, 136},
		{2, 2, 2, 2, 136, 136, 136, 136},
		{2, 2, 2, 2, 136, 136, 136, 136},
		{2, 2, 2, 2, 136, 136, 136, 136}},  /* pdet_id = 3 */
	{
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128}},  /* pdet_id = 4 */
	{
		{2, 2, 2, 2, 110, 110, 110, 110},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116},
		{2, 2, 2, 2, 116, 116, 116, 116}},  /* pdet_id = 5 */
};

static const uint8 avvmid_set47[10][5][8] = {
	{
	/* http://confluence.broadcom.com/pages/viewpage.action?pageId=431104865 */
		{4, 4, 4, 4, 120, 115, 115, 115},
		{4, 4, 4, 4, 75, 70, 76, 70},
		{4, 4, 4, 4, 75, 70, 76, 70},
		{4, 4, 4, 4, 75, 70, 76, 70},
		{4, 4, 4, 4, 75, 70, 76, 70}},  /* pdet_id = 0 */
	{
		{4, 4, 4, 4, 70, 70, 70, 70},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50}},  /* pdet_id = 1 */
	{
		{4, 4, 4, 4, 82, 80, 79, 80},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155}},  /* pdet_id = 2 */
	{
		{4, 4, 4, 4, 105, 105, 105, 105},
		{4, 4, 4, 4, 72, 70, 74, 74},
		{4, 4, 4, 4, 72, 70, 74, 74},
		{4, 4, 4, 4, 74, 72, 76, 76},
		{4, 4, 4, 4, 74, 72, 76, 76}},  /* pdet_id = 3 */
	{
		{4, 4, 4, 4, 85, 85, 85, 85},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128}},  /* pdet_id = 4 */
	{
		{3, 3, 3, 3, 95, 95, 95, 95},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140}},  /* pdet_id = 5 */
	{
		{4, 4, 4, 4, 85, 85, 85, 85},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140}},  /* pdet_id = 6 */
	{
		{4, 4, 4, 4, 82, 82, 82, 82},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110}},  /* pdet_id = 7 */
	{
		{6, 6, 6, 6, 65, 60, 60, 60},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50},
		{5, 5, 5, 5, 50, 50, 50, 50}},  /* pdet_id = 8 */
	{
		{4, 4, 4, 4, 70, 70, 70, 70},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},},  /* pdet_id = 9 */
};

static const uint8 avvmid_set47_1[10][5][8] = {
	{
	/* http://confluence.broadcom.com/pages/viewpage.action?pageId=431104865 */
		{5, 5, 5, 5, 95, 95, 90, 95},
		{4, 4, 4, 4, 75, 80, 75, 82},
		{4, 4, 4, 4, 80, 80, 76, 82},
		{4, 4, 4, 4, 80, 80, 75, 83},
		{4, 4, 4, 4, 80, 80, 75, 84}},  /* pdet_id = 0 */
	{
		{4, 4, 4, 4, 70, 70, 70, 70},
		{5, 5, 5, 5, 62, 63, 63, 64},
		{5, 5, 5, 5, 62, 63, 62, 65},
		{5, 5, 5, 5, 60, 61, 61, 63},
		{5, 5, 5, 5, 61, 61, 61, 62}},  /* pdet_id = 1 */
	{
		{4, 4, 4, 4, 82, 80, 79, 80},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155},
		{2, 2, 2, 2, 155, 155, 155, 155}},  /* pdet_id = 2 */
	{
		{4, 4, 4, 4, 105, 105, 105, 105},
		{4, 4, 4, 4, 72, 70, 74, 74},
		{4, 4, 4, 4, 72, 70, 74, 74},
		{4, 4, 4, 4, 74, 72, 76, 76},
		{4, 4, 4, 4, 74, 72, 76, 76}},  /* pdet_id = 3 */
	{
		{4, 4, 4, 4, 85, 85, 85, 85},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128},
		{2, 2, 2, 2, 128, 128, 128, 128}},  /* pdet_id = 4 */
	{
		{3, 3, 3, 3, 95, 95, 95, 95},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140},
		{2, 2, 2, 2, 140, 140, 140, 140}},  /* pdet_id = 5 */
	{
		{4, 4, 4, 4, 120, 120, 120, 120},
		{3, 3, 3, 3, 98, 98, 100, 102},
		{3, 3, 3, 3, 92, 92, 94, 94},
		{3, 3, 3, 3, 94, 94, 96, 96},
		{3, 3, 3, 3, 98, 98, 100, 102}},  /* pdet_id = 6 */
	{
		{4, 4, 4, 4, 82, 82, 82, 82},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110},
		{3, 3, 3, 3, 110, 110, 110, 110}},  /* pdet_id = 7 */
	{
		{4, 4, 3, 4, 115, 120, 130, 120},
		{4, 4, 4, 4, 98, 96, 97, 98},
		{4, 4, 4, 4, 98, 95, 96, 97},
		{4, 4, 4, 4, 96, 95, 97, 97},
		{4, 4, 4, 4, 96, 95, 96, 97}},  /* pdet_id = 8 */
	{
		{4, 4, 4, 4, 70, 70, 70, 70},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},
		{2, 2, 2, 2, 125, 125, 125, 125},},  /* pdet_id = 9 */
};

static const uint8 avvmid_set47_2[1][5][8] = {
	{
	/* http://confluence.broadcom.com/pages/viewpage.action?pageId=431104865 */
		{4, 4, 4, 4, 115, 105, 112, 107},
		{4, 4, 4, 4, 75, 80, 75, 82},
		{4, 4, 4, 4, 80, 80, 76, 80},
		{4, 4, 4, 4, 80, 80, 80, 78},
		{4, 4, 4, 4, 80, 80, 75, 84}}  /* pdet_id = 0 */
};

#ifdef REGULATORY_DEBUG
static const uint32 rfseqext_2g_rev40[2][11][2] = {
	{
		{0x002A, 0x19FE8443}, {0x002A, 0x18F68423}, {0x000A, 0x18EE8003},
		{0x0028, 0x19FE8043}, {0x002A, 0x18F68023}, {0x0028, 0x18EE8443},
		{0x006A, 0x39FC8843}, {0x002A, 0x18F48423}, {0x000A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}, /* normal ADC + DACx2 mode */
	{
		{0x002E, 0x19FE8473}, {0x002E, 0x19F68473}, {0x000A, 0x18EE8003},
		{0x0028, 0x19FE8043}, {0x002A, 0x18F68023}, {0x0028, 0x18EE8443},
		{0x006A, 0x39FC8843}, {0x002A, 0x18F48423}, {0x000A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}  /* low ADC + DACx2 mode */
};

static const uint32 rfseqext_5g_rev40[2][11][2] = {
	{
		{0x012A, 0x19FE8443}, {0x012A, 0x18F68423}, {0x010A, 0x18EE8003},
		{0x012A, 0x19FE8043}, {0x012A, 0x18F68023}, {0x012A, 0x19FE8443},
		{0x016A, 0x39FC8843}, {0x012A, 0x18F48423}, {0x010A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}, /* normal ADC + DACx2 mode */
	{
		{0x012E, 0x19FE8473}, {0x012E, 0x19F68473}, {0x012E, 0x19EE8073},
		{0x012A, 0x19FE8043}, {0x012A, 0x18F68023}, {0x012A, 0x19FE8443},
		{0x016A, 0x39FC8843}, {0x012A, 0x18F48423}, {0x010A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}  /* low ADC + DACx2 mode */
};

static const uint32 rfseqext_2g_aux0_rev40[2][11][2] = {
	{
		{0x0AA, 0x19FE8443}, {0x0AA, 0x18F68423}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39FC8843}, {0x0AA, 0x18F48423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC + DACx2 mode */
	{
		{0x0AE, 0x19FE8473}, {0x0AE, 0x19F68473}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39FC8843}, {0x0AA, 0x18F48423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}  /* low ADC + DACx2 mode */
};

static const uint32 rfseqext_2g_aux1_rev40[2][11][2] = {
	{
		{0x0AA, 0x19FA8443}, {0x0AA, 0x18F28423}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39F88843}, {0x0AA, 0x18F08423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC + DACx2 mode */
	{
		{0x0AE, 0x19FE8473}, {0x0AE, 0x19F68473}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39F88843}, {0x0AA, 0x18F08423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}  /* low ADC + DACx2 mode */
};

static const uint32 rfseqext_5g_aux0_rev40[2][11][2] = {
	{
		{0x1AA, 0x19FE8443}, {0x12A, 0x18F68423}, {0x10A, 0x18EE8003},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39FC8843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC + DACx2 mode */
	{
		{0x1AE, 0x19FE8473}, {0x1AE, 0x19F68473}, {0x1AE, 0x19EE8073},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39FC8843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}   /* low ADC + DACx2 mode */
};

static const uint32 rfseqext_5g_aux1_rev40[2][11][2] = {
	{
		{0x1AA, 0x19FA8443}, {0x12A, 0x18F68423}, {0x10A, 0x18EE8003},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39F88843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC + DACx2 mode */

	{
		{0x1AE, 0x19FE8473}, {0x1AE, 0x19F68473}, {0x1AE, 0x19EE8073},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39F88843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}} /* low ADC + DACx2 mode */
};
#else
static const uint32 rfseqext_2g_rev40[2][11][2] = {
	{
		{0x006A, 0x39FE8843}, {0x002A, 0x18F68423}, {0x000A, 0x18EE8003},
		{0x0028, 0x19FE8043}, {0x002A, 0x18F68023}, {0x0028, 0x18EE8443},
		{0x006A, 0x39FC8843}, {0x002A, 0x18F48423}, {0x000A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x006E, 0x39FE8873}, {0x002E, 0x19F68473}, {0x000A, 0x18EE8003},
		{0x0028, 0x19FE8043}, {0x002A, 0x18F68023}, {0x0028, 0x18EE8443},
		{0x006A, 0x39FC8843}, {0x002A, 0x18F48423}, {0x000A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}  /* low ADC rate mode */
};
static const uint32 rfseqext_5g_rev40[2][11][2] = {
	{
		{0x016A, 0x39FE8843}, {0x012A, 0x18F68423}, {0x010A, 0x18EE8003},
		{0x012A, 0x19FE8043}, {0x012A, 0x18F68023}, {0x012A, 0x19FE8443},
		{0x016A, 0x39FC8843}, {0x012A, 0x18F48423}, {0x010A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x016E, 0x39FE8873}, {0x012E, 0x19F68473}, {0x012E, 0x19EE8073},
		{0x012A, 0x19FE8043}, {0x012A, 0x18F68023}, {0x012A, 0x19FE8443},
		{0x016A, 0x39FC8843}, {0x012A, 0x18F48423}, {0x010A, 0x18EC8003},
		{0x0000, 0x00000000}, {0x0000, 0x00000000}}  /* low ADC rate mode */
};
static const uint32 rfseqext_2g_aux0_rev40[2][11][2] = {
	{
		{0x0EA, 0x39FE8843}, {0x0AA, 0x18F68423}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39FC8843}, {0x0AA, 0x18F48423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x0EE, 0x39FE8873}, {0x0AE, 0x19F68473}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39FC8843}, {0x0AA, 0x18F48423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* low ADC rate mode */
};

static const uint32 rfseqext_2g_aux1_rev40[2][11][2] = {
	{
		{0x0EA, 0x39FA8843}, {0x0AA, 0x18F28423}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39F88843}, {0x0AA, 0x18F08423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x0EE, 0x39FE8873}, {0x0AE, 0x19F68473}, {0x00A, 0x18EE8003},
		{0x028, 0x19FE8043}, {0x02A, 0x18F68023}, {0x028, 0x18EE8443},
		{0x0EA, 0x39F88843}, {0x0AA, 0x18F08423}, {0x00A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* low ADC rate mode */
};

static const uint32 rfseqext_5g_aux0_rev40[2][11][2] = {
	{
		{0x1EA, 0x39FE8843}, {0x12A, 0x18F68423}, {0x10A, 0x18EE8003},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39FC8843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x1EE, 0x39FE8873}, {0x1AE, 0x19F68473}, {0x1AE, 0x19EE8073},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39FC8843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}  /* low ADC rate mode */
};

static const uint32 rfseqext_5g_aux1_rev40[2][11][2] = {
	{
		{0x1EA, 0x39FA8843}, {0x12A, 0x18F68423}, {0x10A, 0x18EE8003},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39F88843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}}, /* normal ADC rate mode */
	{
		{0x1EE, 0x39FE8873}, {0x1AE, 0x19F68473}, {0x1AE, 0x19EE8073},
		{0x12A, 0x19FE8043}, {0x12A, 0x18F68023}, {0x12A, 0x19FE8443},
		{0x1EA, 0x39F88843}, {0x12A, 0x18F48423}, {0x10A, 0x18EC8003},
		{0x000, 0x00000000}, {0x000, 0x00000000}} /* low ADC rate mode */
};

static const uint32 rfseqext_rev47[2][11][2] = {
	{
		{0x0074, 0x000698C3}, {0x0034, 0x00068C63}, {0x0014, 0x00068843},
		{0x0014, 0x00068843}, {0x0014, 0x00068843}, {0x0034, 0x00068C63},
		{0x0074, 0x000498C3}, {0x0034, 0x00048C63}, {0x0014, 0x00048843},
		{0x000C, 0x00048E47}, {0x000C, 0x00068E47}}, /* normal ADC rate mode */
	{
		{0x0070, 0x020698F3}, {0x0030, 0x02068CF3}, {0x0010, 0x020688F3},
		{0x0010, 0x020688F3}, {0x0010, 0x020688F3}, {0x0030, 0x02068CF3},
		{0x0074, 0x000498C3}, {0x0034, 0x00048C63}, {0x0014, 0x00048843},
		{0x000C, 0x00048E47}, {0x0008, 0x02068EF7}}  /* low ADC rate mode */
};

#endif /* REGULATORY_DEBUG */

/* Location of 20MHz bw afediv values in RFSeqExt table */
/* tx_20 tx_40 tx_80 tx_20_80 tx_40_80 tx_20_40 rx_20 rx_40 rx_80 tx_160 rx_160 */
#define RFSEQEXT_TX20 0
#define RFSEQEXT_RX20 6
#define RFSEQEXT_RX40 7
#define RFSEQEXT_RX80 8
#define RFSEQEXT_RX160 9

/* http://confluence.broadcom.com/display/WLAN/RfSeqExt+table */
/* dac_clk_x2_mode = 0 */
static uint32 rfseqext_rev44_main[2][11][2] = {
	{
		{0x001EB181, 0x060}, {0x001698C1, 0x060}, {0x000E9081, 0x060},
		{0x001E9181, 0x060}, {0x001690C1, 0x060}, {0x001E9981, 0x060},
		{0x001CB181, 0x060}, {0x001498C1, 0x060}, {0x000C9081, 0x060},
		{0x00000000, 0x000}, {0x00000000, 0x000}}, /* normal ADC rate mode */
	{
		{0x021EB0F1, 0x062}, {0x021698F1, 0x062}, {0x020E90F1, 0x062},
		{0x021E90F1, 0x062}, {0x021690F1, 0x062}, {0x021E98F1, 0x062},
		{0x001CB181, 0x060}, {0x001498C1, 0x060}, {0x000C9081, 0x060},
		{0x00000000, 0x000}, {0x00000000, 0x000}} /* low ADC rate mode */
};

/* http://confluence.broadcom.com/display/WLAN/RfSeqExt+table */
/* ulp_tx_mode  RfSeqExt_location */
/* 1 (360MHz)	5 */
/* 2 (240MHz)	2 */
/* 2 (180MHz)	2 */
/* 4 (120MHz)	0 */
/* 5 ( 90MHz)	0 */
/* Current value of ulp_tx_mode is 1 */
static const uint32 rfseqext_rev44_aux_2g[2][11][2] = {
	{
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00068E61, 0x061},
		{0x00048E61, 0x061}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}}, /* normal ADC rate mode */
	{
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00068F79, 0x0E1},
		{0x00048E61, 0x061}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}} /* low ADC rate mode */
};
static const uint32 rfseqext_rev44_aux_5g[2][11][2] = {
	{
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00068869, 0x060},
		{0x00048869, 0x060}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}}, /* normal ADC rate mode */
	{
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}, {0x00068979, 0x0E0},
		{0x00048869, 0x060}, {0x00000000, 0x000}, {0x00000000, 0x000},
		{0x00000000, 0x000}, {0x00000000, 0x000}} /* low ADC rate mode */
};

#define NUM_SUBBANDS_AV_VMID		(5)
#define NUM_VALUES_AV_VMID			(2)
#define MAX_NUM_OF_CORES_AV_VMID	(3)

static uint8
avvmid_set_maj36[][NUM_SUBBANDS_AV_VMID][MAX_NUM_OF_CORES_AV_VMID * NUM_VALUES_AV_VMID] = {
	{
	/* pdet_id = 0 */
		{5, 0, 0, 100, 0, 0},
		{3, 0, 0, 141, 0, 0},
		{3, 0, 0, 141, 0, 0},
		{3, 0, 0, 141, 0, 0},
		{3, 0, 0, 141, 0, 0}},
};
static uint8
avvmid_set_maj40[][NUM_SUBBANDS_AV_VMID][MAX_NUM_OF_CORES_AV_VMID * NUM_VALUES_AV_VMID] = {
	{
	/* pdet_id = 0 */
		{4, 4, 0, 125, 125, 0},
		{4, 4, 0, 125, 125, 0},
		{4, 4, 0, 125, 125, 0},
		{4, 4, 0, 125, 125, 0},
		{4, 4, 0, 125, 125, 0}},
};
uint8 papdluttbl[128] = {
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
};
uint dynradioregtbl[2] = {35, 63};

uint8 *BCMRAMFN(get_avvmid_set_36)(phy_info_t *pi, uint16 pdet_range_id, uint16 subband_id);

uint8 *BCMRAMFN(get_avvmid_set_40)(phy_info_t *pi, uint16 pdet_range_id, uint16 subband_id);

uint16 const rfseq_majrev3_reset2rx_dly[] = {12, 2, 2, 4, 4, 6, 1, 4, 1, 2, 1, 1, 1, 1, 1, 1};

uint16 const rfseq_rx2tx_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x10, 0x26, 0x2, 0x5, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
uint16 const tiny_rfseq_rx2tx_cmd[] =
	{0x42, 0x1, 0x2, 0x8, 0x5, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
uint16 const tiny_rfseq_rx2tx_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x10, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x1, 0x1, 0x1, 0x1};
uint16 const tiny_rfseq_rx2tx_tssi_sleep_cmd[] =
	{0x42, 0x1, 0x2, 0x8, 0x5, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x00, 0x00, 0x36, 0x1f, 0x1f};
uint16 const tiny_rfseq_rx2tx_tssi_sleep_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x10, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x88, 0x1, 0x1, 0x1};
uint16 const tiny_rfseq_tx2rx_cmd[] =
	{0x4, 0x3, 0x6, 0x5, 0x85, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x0, 0xf, 0x2b, 0x43, 0x1F};
static uint16 tiny_rfseq_tx2rx_dly[] =
	{0x8, 0x4, 0x2, 0x2, 0x1, 0x3, 0x4, 0x6, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1};

/* tiny major rev4 RF Sequences : START */

/* Reset2Rx */
/* changing RF sequencer to add DCC reset */
static const uint16 rfseq_majrev4_reset2rx_cmd[] = {0x84, 0x4, 0x3, 0x6, 0x5, 0x2, 0x1, 0x8,
	0x2a, 0x2b, 0xf, 0x0, 0x0, 0x85, 0x41, 0x1f};
uint16 const rfseq_majrev4_reset2rx_dly[] = {10, 12, 2, 2, 4, 4, 6, 1, 4, 1, 2, 10, 1, 1, 1, 1};

/* Tx2Rx */
uint16 const rfseq_majrev4_tx2rx_cmd[] =
	{0x84, 0x4, 0x3, 0x6, 0x5, 0x85, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x0, 0xf, 0x2b, 0x43, 0x1F};
uint16 const rfseq_majrev4_tx2rx_dly[] =
	{0x8, 0x8, 0x4, 0x2, 0x2, 0x1, 0x3, 0x4, 0x6, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1};

/* Rx2Tx */
/* Refer to tiny_rfseq_rx2tx_cmd */

/* Rx2Tx -- Cal */
uint16 const rfseq_majrev4_rx2tx_cal_cmd[] =
	{0x84, 0x1, 0x2, 0x8, 0x5, 0x3d, 0x85, 0x6, 0x3, 0xf, 0x4, 0x3e, 0x35, 0xf, 0x36, 0x1f};
uint16 const rfseq_majrev4_rx2tx_cal_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x12, 0x10, 0x26, 0x2, 0x5, 0x1, 0x4, 0xfa, 0x2, 0x1};
/* tiny major rev4 RF Sequences : END */

// rev32 rfseq
uint16 const rfseq_majrev32_rx2tx_cal_cmd[] =
	{0x84, 0x1, 0x2, 0x8, 0x5, 0x0, 0x85, 0x6, 0x3, 0xf, 0x4, 0x0, 0x35, 0xf, 0x36, 0x1f};
uint16 const rfseq_majrev32_rx2tx_cal_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x12, 0x10, 0x26, 0x17, 0x5, 0x1, 0x4, 0xfa, 0x2, 0x1};
uint16 const rfseq_majrev32_rx2tx_cmd[]    = {0x42, 0x01, 0x02, 0x08, 0x05, 0x06, 0x03, 0x0f,
	0x04, 0x35, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
uint16 const rfseq_majrev32_tx2rx_cmd[]    = {0x84, 0x04, 0x03, 0x06, 0x05, 0x85, 0x02, 0x01,
	0x08, 0x2a, 0x0f, 0x00, 0x0f, 0x2b, 0x43, 0x1F};
uint16 const rfseq_majrev32_reset2rx_cmd[] = {0x0, 0x04, 0x03, 0x06, 0x05, 0x02, 0x01, 0x08,
	0x2a, 0x2b, 0x0f, 0x00, 0x00, 0x0, 0x1f, 0x1f};
// change PA PU delay to 0x17 from 0x2 to resolve the LOFT issues
uint16 const rfseq_majrev32_rx2tx_dly[]    = {0x08, 0x06, 0x06, 0x04, 0x04, 0x10, 0x26, 0x17,
	0x05, 0x04, 0xFA, 0xFA, 0x01, 0x01, 0x01, 0x01};
uint16 const rfseq_majrev32_tx2rx_dly[]    = {0x08, 0x08, 0x04, 0x02, 0x02, 0x01, 0x03, 0x04,
	0x06, 0x04, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01};
uint16 const rfseq_majrev32_reset2rx_dly[] = {0x0A, 0x0C, 0x02, 0x02, 0x04, 0x04, 0x06, 0x01,
	0x04, 0x01, 0x02, 0x0A, 0x01, 0x01, 0x01, 0x01};

/* Major rev36 RF Sequences : START */
/* Reset2Rx */
static const uint16 rfseq_majrev36_reset2rx_cmd[] =
	{0x0, 0x4, 0x3, 0x6, 0x5, 0x2, 0x1, 0x8, 0x0, 0x0, 0xf, 0x3d, 0x3e, 0x0, 0xbf, 0x1f};

static const uint16 rfseq_majrev36_reset2rx_dly[] =
	{0xA, 0xC, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0xA, 0x1, 0x1, 0x01, 0x01};

/* Tx2Rx */
static const uint16 rfseq_majrev36_tx2rx_cmd[] =
	{0x85, 0x4, 0x3, 0x6, 0x5, 0x3d, 0x3e, 0x2, 0x1, 0x8, 0xbf, 0xf, 0xf, 0x0, 0x0, 0x1F};

static const uint16 rfseq_majrev36_tx2rx_dly[] =
	{0x8, 0x8, 0x4, 0x2, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0x1, 0x1, 0x1, 0x1, 0x1};

/* Rx2Tx */
const uint16 rfseq_majrev36_rx2tx_cmd[] =
   {0x85, 0x1,  0x2, 0x8, 0x5, 0x3d, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0xbe, 0x1f, 0x1f};

const uint16 rfseq_majrev36_rx2tx_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x4, 0xe, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x1, 0x1};

/* Rx2Tx -- Cal */
const uint16 rfseq_majrev36_rx2tx_cal_cmd[] =
	{0x84, 0x1, 0x2, 0x8, 0x5, 0x3d, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x85, 0x35, 0xf, 0x0, 0x1f};
const uint16 rfseq_majrev36_rx2tx_cal_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x12, 0x10, 0x26, 0x2, 0x5, 0x4, 0x4, 0xfa, 0x2, 0x1};

/* Rx2Tx -- TSSI sleep */
const uint16 rfseq_majrev36_rx2tx_tssi_sleep_cmd[] =
	{0x85, 0x1, 0x2, 0x5, 0x3d, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x0, 0x86, 0x36, 0x1f};
const uint16 rfseq_majrev36_rx2tx_tssi_sleep_dly[] =
	{0x2, 0x6, 0xa, 0x4, 0x2, 0x4, 0xe, 0x26, 0x2, 0x5, 0x4, 0xfa, 0xfa, 0x88, 0x1, 0x1};
/* Major rev36 RF Sequences : END */

/* Major rev37 RF Sequences : START */
/* Reset2Rx */
static const uint16 rfseq_majrev37_reset2rx_cmd[] =
	{0x4, 0x3, 0x6, 0x2, 0x5, 0x1, 0x8, 0x2a, 0x2b, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
static const uint16 rfseq_majrev37_reset2rx_dly[] =
	{0xc, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
/* Tx2Rx */
static const uint16 rfseq_majrev37_tx2rx_cmd[] =
	{0x4, 0x3, 0x6, 0x5, 0x3d, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x3e, 0xf, 0x2b, 0x1f, 0x1f, 0x1f};
static const uint16 rfseq_majrev37_tx2rx_dly[] =
	{0x8, 0x4, 0x2, 0x2, 0x1, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x1, 0x1, 0x1};
/* Rx2Tx */
static const uint16 rfseq_majrev37_rx2tx_cmd[] =
	{0x0, 0x1, 0x2, 0x8, 0x5, 0x3d, 0x6, 0x3, 0xf, 0x4, 0x3e, 0x35, 0xf, 0x0, 0x36, 0x1f};
static const uint16 rfseq_majrev37_rx2tx_dly[] =
	{0x8, 0x6, 0x6, 0x4, 0x4, 0x2, 0x10, 0x26, 0x2, 0x5, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
/* Major rev37 RF Sequences : END */

/* Major rev47 RF Sequences : START */
/* Bundle scheme */
static const uint16 rfseq_majrev47_bundleScheme2 = 1;
/* Reset2Rx */
static const uint16 rfseq_majrev47_reset2rx_cmd[] =
	{0x4, 0x3, 0x6, 0x2, 0x5, 0x1, 0x8, 0x2a, 0x2b, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
static const uint16 rfseq_majrev47_reset2rx_dly[] =
	{0xc, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
/* Tx2Rx */
static const uint16 rfseq_majrev47_tx2rx_cmd[] =
	{0x85, 0x4, 0x3, 0x6, 0x5, 0x3d, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x3e, 0xf, 0x2b, 0x86, 0x1f};
static const uint16 rfseq_majrev47_tx2rx_dly[] =
	{0x2, 0x8, 0x4, 0x2, 0x2, 0x1, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x2, 0x1};
/* Rx2Tx */

static const uint16 rfseq_majrev47_rx2tx_cmd[] =
	{0x0, 0x1, 0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x0, 0x36, 0x1f};
static const uint16 rfseq_majrev47_rx2tx_dly[] =
	{0x18, 0x6, 0x6, 0x4, 0x4, 0x4, 0x1, 0x10, 0x26, 0x2, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
/* Major rev47 RF Sequences : END */

/* Major rev40 RF Sequences : START */
/* Reset2Rx */
static const uint16 rfseq_majrev40_reset2rx_cmd[] =
	{0x4, 0x3, 0x6, 0x2, 0x5, 0x1, 0x8, 0x2a, 0x2b, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
static const uint16 rfseq_majrev40_reset2rx_dly[] =
	{0xc, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
/* Tx2Rx */
static const uint16 rfseq_majrev40_tx2rx_cmd[] =
	{0x85, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x3e, 0xf, 0x2b, 0x86, 0x1f};
static const uint16 rfseq_majrev40_tx2rx_dly[] =
	{0x2, 0x8, 0x4, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x2, 0x1};
static const uint16 rfseq_majrev40_tx2rx_cal_cmd[] =
	{0x4, 0x3, 0x6, 0x3d, 0x5, 0x2, 0x1, 0x8, 0x2a, 0xf, 0x3e, 0xf, 0x2b, 0x1f, 0x1f, 0x1f};
static const uint16 rfseq_majrev40_tx2rx_cal_dly[] =
	{0x8, 0x4, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x1, 0x1, 0x1};
/* Rx2Tx */
static const uint16 rfseq_majrev40_rx2tx_cmd[] =
	{0x0, 0x1, 0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x0, 0x36, 0x1f};
static const uint16 rfseq_majrev40_rx2tx_dly[] =
	{0x18, 0x6, 0x6, 0x4, 0x4, 0x4, 0x1, 0x10, 0x26, 0x2, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
/* Major rev40 RF Sequences : END */

/* Major rev44 RF Sequences for main : START */
/* Rx2Tx */
static const uint16 rfseq_majrev44_rx2tx_cmd_main[] =
	{0x0, 0x1, 0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x0, 0x0, 0x1f};
static const uint16 rfseq_majrev44_rx2tx_dly_main[] =
	{0x18, 0x6, 0x6, 0x4, 0x4, 0x4, 0x1, 0x10, 0x26, 0x2, 0x1, 0x4, 0xfa, 0xfa, 0x2, 0x1};
static const uint16 rfseq_majrev44_rx2tx_tssisleep_en_cmd_main[] =
	{0x85, 0x86, 0x1, 0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x36, 0x1f};
static const uint16 rfseq_majrev44_rx2tx_tssisleep_en_dly_main[] =
	{0x2, 0x2, 0x6, 0x6, 0x4, 0x4, 0x4, 0x1, 0x10, 0x26, 0x2, 0x1, 0x4, 0xfa, 0x2, 0x1};
/* Tx2Rx */
static const uint16 rfseq_majrev44_tx2rx_cmd_main[] =
	{0x85, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x2, 0x1, 0x8, 0x0, 0xf, 0x3e, 0xf, 0x0, 0x86, 0x1f};
static const uint16 rfseq_majrev44_tx2rx_dly_main[] =
	{0x2, 0x8, 0x4, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x2, 0x1};
static const uint16 rfseq_majrev44_tx2rx_cal_cmd_main[] =
	{0, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x2, 0x1, 0x8, 0x0, 0xf, 0x3e, 0xf, 0x0, 0, 0x1f};
static const uint16 rfseq_majrev44_tx2rx_cal_dly_main[] =
	{0x2, 0x8, 0x4, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0xa, 0x4, 0x2, 0x1, 0x2, 0x1};
/* Reset2Rx */
const uint16 rfseq_majrev44_reset2rx_cmd_main[] =
	{0x4, 0x3, 0x6, 0x2, 0x5, 0x1, 0x8, 0x0, 0x0, 0xf, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};
const uint16 rfseq_majrev44_reset2rx_dly_main[] =
	{0xc, 0x2, 0x2, 0x4, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
/* Major rev44 RF Sequences for main: END */

/* Major rev44 RF Sequences for aux : START */
/* Rx2Tx */
static const uint16 rfseq_majrev44_rx2tx_cmd_aux[] =
	{0x0, 0x1,  0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x99, 0x0, 0x1f};
static const uint16 rfseq_majrev44_rx2tx_dly_aux[] =
	{0x18, 0x6, 0x6, 0x4, 0x2, 0x4, 0x4, 0xe, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x1, 0x1};
static const uint16 rfseq_majrev44_rx2tx_tssisleep_en_cmd_aux[] =
	{0x0, 0x1,  0x2, 0x8, 0x3d, 0x5, 0x3e, 0x6, 0x3, 0xf, 0x4, 0x35, 0xf, 0x99, 0x36, 0x1f};
static const uint16 rfseq_majrev44_rx2tx_tssisleep_en_dly_aux[] =
	{0x18, 0x6, 0x6, 0x4, 0x2, 0x4, 0x4, 0xe, 0x26, 0x2, 0x5, 0x4, 0xFA, 0xFA, 0x1, 0x1};
/* Tx2Rx */
static const uint16 rfseq_majrev44_tx2rx_cmd_aux[] =
	{0x85, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x3e, 0x2, 0x1, 0x8, 0x98, 0xf, 0xf, 0x0, 0x86, 0x1F};
static const uint16 rfseq_majrev44_tx2rx_dly_aux[] =
	{0x8,  0x8, 0x4, 0x2, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0x1, 0x1, 0x1, 0x2, 0x1};
static const uint16 rfseq_majrev44_tx2rx_cal_cmd_aux[] =
	{0x0, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x3e, 0x2, 0x1, 0x8, 0x98, 0xf, 0xf, 0x0, 0, 0x1F};
static const uint16 rfseq_majrev44_tx2rx_cal_dly_aux[] =
	{0x8,  0x8, 0x4, 0x2, 0x2, 0x2, 0x2, 0x3, 0x4, 0x6, 0x4, 0x1, 0x1, 0x1, 0x2, 0x1};
/* Reset2Rx */
const uint16 rfseq_majrev44_reset2rx_cmd_aux[] =
	{0x0, 0x4, 0x3, 0x6, 0x3d, 0x5, 0x3e, 0x2, 0x1, 0x8, 0x0, 0x0, 0xf, 0x0, 0x98, 0x1f};
const uint16 rfseq_majrev44_reset2rx_dly_aux[] =
	{0xA, 0xC, 0x2, 0x2, 0xA, 0x4, 0x1, 0x4, 0x6, 0x1, 0x4, 0x1, 0x2,  0x1, 0x01, 0x01};
/* Major rev44 RF Sequences for aux: END */

/* Channel smoothing MTE filter image */
#define CHANSMTH_FLTR_LENGTH 64
static CONST uint16 acphy_Smth_tbl_4349[] = {
	0x4a5c, 0xdba7, 0x1672,
	0xb167, 0x742d, 0xa5ca,
	0x4afe, 0x4aa6, 0x14f3,
	0x4176, 0x6f25, 0xa75a,
	0x7aca, 0xeca4, 0x1e94,
	0xf177, 0x4e27, 0xa7fa,
	0x0b46, 0xcead, 0x270c,
	0x3169, 0x4f1d, 0xa70b,
	0xda4e, 0xcb35, 0x1431,
	0xd1d2, 0x572e, 0xae6b,
	0x8a4b, 0x68bc, 0x1f62,
	0x81f6, 0xc826, 0xa4bb,
	0x2add, 0x6b37, 0x1d42,
	0xcaff, 0xdd9e, 0x0c6a,
	0xd0c6, 0xecad, 0xaff9,
	0xbad8, 0xe69d, 0x173a,
	0x20d1, 0xf5b7, 0xa579,
	0x6b71, 0xdb9c, 0x156a,
	0x60d6, 0xf345, 0xa6f9,
	0x0b42, 0xc6a6, 0x1f5a,
	0xb0d4, 0xe22e, 0x9c19,
	0x4bc4, 0x5aaf, 0x1c6b,
	0xc0cc, 0xc326, 0x9c49,
	0x1cf1, 0xddb7, 0x243b,
	0xe17a, 0xe21c, 0xa75a,
	0x6a50, 0xcb35, 0x1441,
	0xb1d3, 0x5d2e, 0xae4b,
	0x8a4b, 0x67bd, 0x1f72,
	0x71f7, 0xd826, 0xa4bb,
	0xfade, 0x6b36, 0x1d42,
	0xe153, 0xcf96, 0x0fc8,
	0xf0fc, 0x6e8c, 0x1539,
	0xd1fd, 0x7d94, 0x0da9,
	0xd047, 0xc08c, 0x1578,
	0x41c9, 0x4c9d, 0x1679,
	0xe043, 0x7696, 0x1459,
	0xf2f7, 0x7faf, 0x1d1a,
	0xd0e4, 0x4c9c, 0x1c49,
	0xe37e, 0xca9c, 0x1782,
	0x31ff, 0x7ba4, 0x2f1a,
	0xd243, 0xe69d, 0x16ba,
	0x616b, 0xddae, 0x2439,
	0xdc69, 0x46ae, 0x1fb2,
	0xf0c9, 0x5a97, 0x0658,
	0xa065, 0x7f85, 0x0c99,
	0xd174, 0x4a95, 0x0508,
	0x2074, 0xce86, 0x0d38,
	0xb152, 0xea9f, 0x0f08,
	0xd078, 0xd785, 0x0d38,
	0x71e3, 0xc29c, 0x0c48,
	0xc06e, 0xd684, 0x0c88,
	0x4262, 0x42a4, 0x1439,
	0x0058, 0xc78e, 0x1658,
	0xb2c4, 0x5cb5, 0x25da,
	0x60f5, 0x5694, 0x1dd9,
	0x02c6, 0xc39d, 0x1792,
	0x61ff, 0x7ba4, 0x2f3a,
	0xf246, 0xee9d, 0x16ca,
	0xe16c, 0xdfae, 0x2469,
	0x2c4d, 0x44af, 0x1fd2,
	0x0bcd, 0x4faf, 0x1c5b,
	0x30cb, 0x7e27, 0x9c6a,
	0xec42, 0xd3b6, 0x243b,
	0x0179, 0xd81d, 0xa77a
 };
static CONST uint16 acphy_Smth_tbl_tiny[] = {
	0x5fd2,	0x16fc,	0x0ce0,
	0x60ce,	0xc501,	0xfd2f,
	0xefe0,	0x09fc,	0x09e0,
	0x90eb,	0xc802,	0xfc5f,
	0xcfed,	0x01fd,	0x0690,
	0xf0ed,	0xd903,	0xfc0f,
	0xcff7,	0xfefe,	0x037f,
	0x30d2,	0xf605,	0xfc7f,
	0xbfd8,	0x4b00,	0x0860,
	0xb052,	0xf501,	0xfe6f,
	0xbfda,	0x33ff,	0x0750,
	0x3075,	0xfb03,	0xfdaf,
	0xefe8,	0x3500,	0x0530,
	0x4fe0,	0xe8f9,	0x119f,
	0x8119,	0x94fe,	0xfe0f,
	0x5fea,	0xe6fa,	0x0e5f,
	0x1142,	0x8aff,	0xfd4f,
	0xaff1,	0xe9fb,	0x0acf,
	0x2156,	0x8d00,	0xfc8f,
	0xfff7,	0xeefc,	0x075f,
	0xa151,	0x9d01,	0xfbef,
	0x2ffb,	0xf4fe,	0x045f,
	0x612f,	0xbd03,	0xfbbf,
	0x1ffe,	0xfaff,	0x021f,
	0xe0f4,	0xe704,	0xfc5f,
	0xafd8,	0x4b00,	0x0880,
	0xa052,	0xf401,	0xfe7f,
	0xafda,	0x33ff,	0x0770,
	0x3077,	0xfa03,	0xfdaf,
	0xdfe8,	0x3500,	0x0540,
	0x30a5,	0xc5f2,	0x1f1e,
	0x51f1,	0x23ec,	0x0a5f,
	0x607c,	0x06f6,	0x167f,
	0xb236,	0xffec,	0x0ade,
	0xc049,	0x67fa,	0x0cdf,
	0x4214,	0x13f2,	0x089f,
	0x001d,	0xc0fe,	0x051f,
	0x2191,	0x68fb,	0x044f,
	0x100f,	0x12fb,	0x0ef0,
	0xe07f,	0xc2fd,	0x01cf,
	0x2021,	0xe6fa,	0x0d5f,
	0x60d5,	0xa2fe,	0x021f,
	0x4ffc,	0x22fe,	0x07b0,
	0x2125,	0x2cf0,	0x32bd,
	0xc32b,	0x02d2,	0x125f,
	0x50e8,	0xb0f4,	0x27ed,
	0xe3a0,	0xc7ce,	0x14be,
	0x40a3,	0x57f8,	0x1bee,
	0x43bd,	0xa3d1,	0x14ce,
	0x8062,	0xf9fb,	0x10ee,
	0x3370,	0xa7da,	0x11fe,
	0xe030,	0x7cfd,	0x085f,
	0x12c1,	0xe2e8,	0x0c9e,
	0x4010,	0xd1ff,	0x02ef,
	0x41d4,	0x54f7,	0x05df,
	0xf011,	0x10fa,	0x0f10,
	0xd07f,	0xc2fd,	0x01cf,
	0x1023,	0xe4fa,	0x0d7f,
	0x40d7,	0xa1fe,	0x023f,
	0x3ffd,	0x22fe,	0x07c0,
	0x3ffb,	0xf6fe,	0x044f,
	0x912a,	0xc103,	0xfbaf,
	0x2ffd,	0xfaff,	0x021f,
	0xf0f1,	0xe904,	0xfc4f,
};

/* China 40M Spur WAR */
static const uint16 resamp_cnwar_5270[] = {0x4bda, 0x0038, 0x10e0, 0x4bda, 0x0038, 0x10e0,
0xed0e, 0x0068, 0xed0e, 0x0068};
static const uint16 resamp_cnwar_5310[] = {0x0000, 0x00d8, 0x0b40, 0x0000, 0x00d8, 0x0b40,
0x6c79, 0x0045, 0x6c79, 0x0045};

/* TxEvmTbl of 43012 */
static const uint8 tx_evm_tbl_majrev36[] = {
	0x09, 0x0e, 0x11, 0x14, 0x17, 0x1a, 0x1d, 0x20, 0x09, 0x0e, 0x11, 0x14, 0x17, 0x1a,
	0x1d, 0x20, 0x22, 0x24, 0x09, 0x0e, 0x11, 0x14, 0x17, 0x1a, 0x1d, 0x20, 0x22, 0x24,
	0x09, 0x0e, 0x11, 0x14, 0x17, 0x1a, 0x1d, 0x20, 0x22, 0x24
};

/* NvAdjTbl of 43012 */
static const uint32 nv_adj_tbl_majrev36[] = {
	0x00000000, 0x00400844, 0x00300633, 0x00200422, 0x00100211, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000100, 0x00000200, 0x00000311, 0x00000422, 0x00100533, 0x00200644, 0x00300700,
	0x00400800, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00400800, 0x00300700, 0x00200644, 0x00100533, 0x00000422, 0x00000311,
	0x00000200, 0x00000100, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00100211, 0x00200422, 0x00300633,
	0x00400844,
};

/* PhaseTrackTbl_1x1 of 43012 */
static const uint32 phase_track_tbl_majrev36[] = {
	0x06af56cd, 0x059acc7b, 0x04ce6652, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819,
	0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x06af56cd, 0x059acc7b, 0x04ce6652,
	0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819,
	0x02b15819
};

/* scalarTable0 of 43012 */
static const uint32 scalar_table0_majrev36[] = {
	0x0b5e002d, 0x0ae2002f, 0x0a3b0032, 0x09a70035, 0x09220038, 0x08ab003b, 0x081f003f,
	0x07a20043, 0x07340047, 0x06d2004b, 0x067a004f, 0x06170054, 0x05bf0059, 0x0571005e,
	0x051e0064, 0x04d3006a, 0x04910070, 0x044c0077, 0x040f007e, 0x03d90085, 0x03a1008d,
	0x036f0095, 0x033d009e, 0x030b00a8, 0x02e000b2, 0x02b900bc, 0x029200c7, 0x026d00d3,
	0x024900e0, 0x022900ed, 0x020a00fb, 0x01ec010a, 0x01d20119, 0x01b7012a, 0x019e013c,
	0x0188014e, 0x01720162, 0x015d0177, 0x0149018e, 0x013701a5, 0x012601be, 0x011501d8,
	0x010601f4, 0x00f70212, 0x00e90231, 0x00dc0253, 0x00d00276, 0x00c4029b, 0x00b902c3,
	0x00af02ed, 0x00a50319, 0x009c0348, 0x0093037a, 0x008b03af, 0x008303e6, 0x007c0422,
	0x00750460, 0x006e04a3, 0x006804e9, 0x00620533, 0x005d0582, 0x005805d6, 0x0053062e,
	0x004e068c
};

/* sgiAdjustTbl of 43012 */
static const uint32 sgi_adjust_tbl_majrev36[] = {
	0x00100101, 0x10100111, 0x10110010, 0x11010111, 0x10022000, 0x01200021, 0x11202312,
	0x01110100, 0x10111010, 0x00010010, 0x11001010, 0x12111200, 0x21210111, 0x11410200,
	0x00101001, 0x10100001, 0x10011111, 0x11001011, 0x22222001, 0x01112212, 0x11112144,
	0x00110100, 0x01101101, 0x01100000, 0x10110100, 0x00020021, 0x20211022, 0x01103442,
	0x01011110, 0x01110100, 0x10111010, 0x11000000, 0x21000202, 0x21011112, 0x11001314,
	0x11001010, 0x10111110, 0x10010101, 0x02110011, 0x11101022, 0x24211201, 0x00011023,
	0x11100101, 0x10000100, 0x10010101, 0x22210110, 0x21112220, 0x13100222
};

/* per-tone reciprocity compensation coefficients
 * used for implicit beamforming calibration
 */
uint16 recip_packed_majrev32_33_37[256][3] = {{0}};

#ifdef WLSMC
static const uint32 Ndbps_ru_LUT[9][8] = {
{0x0018000C, 0x00300024, 0x00600048, 0x0078006C, 0x00A00090, 0x00C800B4, 0x00D200D2, 0x00D200D2},
{0x00660033, 0x00CC0099, 0x01980132, 0x01FE01CB, 0x02A80264, 0x035202FD, 0x037C037C, 0x037C037C},
{0x00EA0075, 0x01D4015F, 0x03A802BE, 0x0492041D, 0x0618057C, 0x079E06DB, 0x07FE07FE, 0x07FE07FE},
{0x03D401EA, 0x07A805BE, 0x0F500B7C, 0x1324113A, 0x198516F8, 0x1FE61CB6, 0x217F217F, 0x217F217F},
{0x00060003, 0x000C0009, 0x00180012, 0x001E001B, 0x00280024, 0x0032002D, 0x00320032, 0x00320032},
{0x0018000C, 0x00300024, 0x00600048, 0x0078006C, 0x00A00090, 0x00C800B4, 0x00C800C8, 0x00C800C8},
{0x003C001E, 0x0078005A, 0x00F000B4, 0x012C010E, 0x01900168, 0x01F401C2, 0x01F401F4, 0x01F401F4},
{0x00F00078, 0x01E00168, 0x03C002D0, 0x04B00438, 0x064005A0, 0x07D00708, 0x07D007D0, 0x07D007D0},
{0x01EC00F6, 0x03D802E2, 0x07B005C4, 0x099C08A6, 0x0CD00B88, 0x10040E6A, 0x10041004, 0x10041004}};

static const uint32 n_cbps_ru_LUT[3][6] = {
{0x00060003, 0x000C0006, 0x0012000C, 0x00120012, 0x00180018, 0x00180018},
{0x001A000D, 0x0033001A, 0x004D0033, 0x004D004D, 0x00660066, 0x00660066},
{0x003B001E, 0x0075003B, 0x00B00075, 0x00B000B0, 0x00EA00EA, 0x00EA00EA}};

static const uint32 dummy_rxctrl[] = {
0x20003001, 0x00000050, 0x01010000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x081a0281, 0x7fc00010, 0x0058240b, 0x440c0000};
#endif /* WLSMC */

#ifdef WLVASIP
static const uint32 bfdLut_content[] = {
0x22002000, 0x26002400, 0x30903000, 0x31b03120};

static const uint16 ibfdRptAddrLut_content = 0x2600;
#endif /* WLVASIP */

/* 43012 tables to be written on reset */
static acphytbl_info_t tbls_write_on_reset_list_majrev36[] = {
	{tx_evm_tbl_majrev36, ARRAYSIZE(tx_evm_tbl_majrev36), ACPHY_TBL_ID_TXEVMTBL, 0, 8},
	{nv_adj_tbl_majrev36, ARRAYSIZE(nv_adj_tbl_majrev36), ACPHY_TBL_ID_NVADJTBL, 0, 32},
	{phase_track_tbl_majrev36, ARRAYSIZE(phase_track_tbl_majrev36),
	ACPHY_TBL_ID_PHASETRACKTBL_1X1, 0, 32},
	{scalar_table0_majrev36, ARRAYSIZE(scalar_table0_majrev36),
	ACPHY_TBL_ID_SCALARTABLE0, 0, 32},
	{sgi_adjust_tbl_majrev36, ARRAYSIZE(sgi_adjust_tbl_majrev36),
	ACPHY_TBL_ID_SGIADJUSTTBL, 0, 32},
	{NULL, 0, 0, 0, 0}
};

static acphytbl_info_t *BCMRAMFN(phy_ac_get_tbls_write_on_reset_list)(phy_info_t *pi);
static void wlc_phy_set_noise_var_shaping_acphy(phy_info_t *pi,
	uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES],	int8 *tone_id, uint8 *core_nv);
static void phy_ac_chanmgr_papr_iir_filt_reprog(phy_info_t *pi);
static void chanspec_setup_papr(phy_info_t *pi,
	int8 papr_final_clipping, int8 papr_final_scaling);
static void wlc_phy_spurwar_nvshp_acphy(phy_info_t *pi, bool bw_chg,
	bool spurwar, bool nvshp);
static void wlc_phy_write_rx_farrow_acphy(phy_ac_chanmgr_info_t *ci, chanspec_t chanspec);

#ifndef WL_FDSS_DISABLED
static void wlc_phy_fdss_init(phy_info_t *pi);
static void wlc_phy_set_fdss_table(phy_info_t *pi);
static void wlc_phy_set_fdss_table_majrev_ge40(phy_info_t *pi);
#endif // endif

extern void wlc_phy_ulb_feature_flag_set(wlc_phy_t *pih);
static void phy_ac_lp_enable(phy_info_t *pi);
static void wlc_phy_ac_lpf_cfg(phy_info_t *pi);
static void wlc_acphy_load_4349_specific_tbls(phy_info_t *pi);
static void wlc_acphy_load_radiocrisscross_phyovr_mode(phy_info_t *pi);
static void wlc_acphy_load_logen_tbl(phy_info_t *pi);
static void wlc_phy_set_regtbl_on_band_change_acphy_20693(phy_info_t *pi);
static void wlc_phy_load_channel_smoothing_tiny(phy_info_t *pi);
static void wlc_phy_set_reg_on_reset_acphy_20693(phy_info_t *pi);
static void wlc_phy_set_tbl_on_reset_acphy(phy_info_t *pi);
static void wlc_phy_set_regtbl_on_band_change_acphy(phy_info_t *pi);
static void wlc_phy_set_regtbl_on_bw_change_acphy(phy_info_t *pi);
static void wlc_phy_set_sdadc_pd_val_per_core_acphy(phy_info_t *pi);
static void chanspec_setup_regtbl_on_chan_change(phy_info_t *pi);
static void wlc_phy_set_sfo_on_chan_change_acphy(phy_info_t *pi, uint8 ch);
static void wlc_phy_write_sfo_params_acphy(phy_info_t *pi, uint16 fc);
static void wlc_phy_write_sfo_params_acphy_wave2(phy_info_t *pi, const uint16 *val_ptr);
static void wlc_phy_write_sfo_80p80_params_acphy(phy_info_t *pi, const uint16 *val_ptr,
	const uint16 *val_ptr1);
static void wlc_phy_set_reg_on_reset_acphy_20691(phy_info_t *pi);
static void acphy_load_txv_for_spexp(phy_info_t *pi);
static void wlc_phy_cfg_energydrop_timeout(phy_info_t *pi);
static void wlc_phy_set_reg_on_bw_change_acphy(phy_info_t *pi);
static void wlc_phy_set_reg_on_bw_change_acphy_majorrev40_44(phy_info_t *pi);
static void wlc_phy_set_pdet_on_reset_acphy(phy_info_t *pi);
static void wlc_phy_set_tx_iir_coeffs(phy_info_t *pi, bool cck, uint8 filter_type);
static void wlc_phy_write_regtbl_fc3_sub0(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc3_sub1(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc3_sub2(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc3_sub3(phy_info_t *pi);
static INLINE void wlc_phy_write_regtbl_fc3(phy_info_t *pi, phy_info_acphy_t *pi_ac);
static void wlc_phy_write_regtbl_fc4_sub0(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc4_sub1(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc4_sub2(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc4_sub34(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc4_sub5(phy_info_t *pi);
static INLINE void wlc_phy_write_regtbl_fc4(phy_info_t *pi, phy_info_acphy_t *pi_ac);
static void wlc_phy_write_regtbl_fc10_sub0(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc10_sub1(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc10_sub2(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc10_sub3(phy_info_t *pi);
static void wlc_phy_write_regtbl_fc10_sub4(phy_info_t *pi);
static INLINE void wlc_phy_write_regtbl_fc10(phy_info_t *pi, phy_info_acphy_t *pi_ac);
static void wlc_phy_tx_gm_gain_boost(phy_info_t *pi);
static void wlc_phy_write_rx_farrow_pre_tiny(phy_info_t *pi, chan_info_rx_farrow *rx_farrow,
	chanspec_t chanspec);
static void wlc_phy_set_reg_on_reset_acphy(phy_info_t *pi);
static void wlc_phy_set_analog_tx_lpf(phy_info_t *pi, uint16 mode_mask, int bq0_bw, int bq1_bw,
	int rc_bw, int gmult, int gmult_rc, int core_num);
static void wlc_phy_set_tx_afe_dacbuf_cap(phy_info_t *pi, uint16 mode_mask, int dacbuf_cap,
	int dacbuf_fixed_cap, int core_num);
static void wlc_phy_set_analog_rx_lpf(phy_info_t *pi, uint8 mode_mask, int bq0_bw, int bq1_bw,
	int rc_bw, int gmult, int gmult_rc, int core_num);
#ifndef ACPHY_1X1_ONLY
static void wlc_phy_write_tx_farrow_acphy(phy_ac_chanmgr_info_t *ci, chanspec_t chanspec);
#endif // endif
static void wlc_phy_radio20693_set_reset_table_bits(phy_info_t *pi, uint16 tbl_id, uint16 offset,
	uint16 start, uint16 end, uint16 val, uint8 tblwidth);
static void wlc_acphy_dyn_papd_cfg_20693(phy_info_t *pi);
static void wlc_phy_set_bias_ipa_as_epa_acphy_20693(phy_info_t *pi, uint8 core);

static void wlc_phy_farrow_setup_28nm(phy_info_t *pi, uint16 dac_rate_mode);
static void wlc_phy_td_sfo_eco(phy_info_t *pi);
static void wlc_phy_ul_mac_aided(phy_info_t *pi);
static void wlc_phy_ul_mac_aided_timing(phy_info_t *pi);
/* chanspec handle */
typedef void (*chanspec_module_t)(phy_info_t *pi);
chanspec_module_t * BCMRAMFN(get_chanspec_module_list)(void);

/* setup */
static void chanspec_setup(phy_info_t *pi);
static void chanspec_setup_phy(phy_info_t *pi);
static void chanspec_setup_cmn(phy_info_t *pi);

/* tune */
static void chanspec_tune_phy(phy_info_t *pi);
static void chanspec_tune_txpath(phy_info_t *pi);
static void chanspec_tune_rxpath(phy_info_t *pi);

/* wars & features */
static void chanspec_fw_enab(phy_info_t *pi);

/* cleanup */
static void chanspec_cleanup(phy_info_t *pi);

/* other helper functions */
static void chanspec_phy_table_access_war(phy_info_t *pi);
static void chanspec_setup_hirssi_ucode_cap(phy_info_t *pi);
static void chanspec_sparereg_war(phy_info_t *pi);
static void chanspec_prefcbs_init(phy_info_t *pi);
static bool chanspec_papr_enable(phy_info_t *pi);

/* phy setups */
static void chanspec_setup_phy_ACMAJORREV_47_51(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_44_46(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_40(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_37(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_36(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_32(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_5(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_4(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_3(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_2(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_1(phy_info_t *pi);
static void chanspec_setup_phy_ACMAJORREV_0(phy_info_t *pi);

/* phy tunables */
static void chanspec_tune_phy_ACMAJORREV_47_51(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_44_46(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_40(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_37(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_36(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_32(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_5(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_4(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_3(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_2(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_1(phy_info_t *pi);
static void chanspec_tune_phy_ACMAJORREV_0(phy_info_t *pi);

static void chanspec_tune_phy_dccal(phy_info_t *pi, bool force);

void wlc_phy_ac_shared_ant_femctrl_master(phy_info_t *pi);
#if (defined(WL_SISOCHIP) || !defined(SWCTRL_TO_BT_IN_COEX))
static void wlc_phy_ac_femctrl_mask_on_band_change(phy_info_t *pi);
#endif // endif

#ifdef WL11ULB
static void phy_ac_chanmgr_write_sfo_ulb_params_acphy(phy_info_t *pi, int freq);
#endif /* WL11ULB */
static int phy_ac_chanmgr_switch_phymode_acphy(phy_info_t *pi, uint32 phymode);
static void wlc_phy_set_hesiga_sigb_pos(phy_info_t *pi);
static void wlc_phy_set_afediv_reset_bundles(phy_info_t *pi);

chanspec_module_t chanspec_module_list[] = {
	chanspec_setup,
	chanspec_setup_tpc,
	chanspec_setup_radio,
	chanspec_setup_phy,
	chanspec_setup_cmn,
	chanspec_noise,
	chanspec_setup_rxgcrs,
	chanspec_tune_radio,
	chanspec_tune_phy,
	chanspec_tune_txpath,
	chanspec_tune_rxpath,
	chanspec_fw_enab,
	chanspec_cleanup,
	chanspec_btcx,
	NULL
};

chanspec_module_t *
BCMRAMFN(get_chanspec_module_list)(void)
{
	return chanspec_module_list;
}

static acphytbl_info_t *
BCMRAMFN(phy_ac_get_tbls_write_on_reset_list)(phy_info_t *pi)
{
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		return tbls_write_on_reset_list_majrev36;
	} else {
		PHY_ERROR(("wl%d: %s: tables write on reset list not know for rev%d\n",
				pi->sh->unit, __FUNCTION__, pi->pubpi->phy_rev));
		return NULL;
	}
}

static void
wlc_phy_config_bias_settings_20693(phy_info_t *pi)
{
	uint8 core;
	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID));

	FOREACH_CORE(pi, core) {
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20693_ENTRY(pi, TRSW2G_CFG1, core, trsw2g_pu, 0)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR1_EAST, core, ovr_trsw2g_pu, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TRSW2G_CFG1, core, trsw2g_bias_pu, 0)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR1_EAST, core,
				ovr_trsw2g_bias_pu, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR1_EAST, core,
				ovr_mx2g_idac_bbdc, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core, ovr_mx5g_idac_bbdc, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core, ovr_pad5g_idac_pmos, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core, ovr_pad5g_idac_gm, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR3, core,
				ovr_pa5g_bias_filter_main, 1)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}
	if (ROUTER_4349(pi)) {
		/* 53573A0: 5G Tx Low Power Radio Optimizations */
		/* proc 20693_tx5g_set_bias_ipa_opt_sv */
		if (PHY_IPA(pi)) {
			bool BW40, BW80;

			FOREACH_CORE(pi, core) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					BW40 = (CHSPEC_IS40(pi->radio_chanspec));
					BW80 = (CHSPEC_IS80(pi->radio_chanspec) ||
						CHSPEC_IS8080(pi->radio_chanspec));

					RADIO_REG_LIST_START
						MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
							ovr_trsw5g_pu, 0x1)
						MOD_RADIO_REG_20693_ENTRY(pi, TRSW5G_CFG1, core,
							trsw5g_pu, 0x0)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG7, core,
							swcap_sec_gate_off_5g, 0xf)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG7, core,
							swcap_sec_sd_on_5g, 0x10)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG6, core,
							swcap_pri_pd_5g, 0x1)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG6, core,
							swcap_pri_5g, 0x0)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG6, core,
							swcap_pri_gate_off_5g, 0x0)
						MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG6, core,
							swcap_pri_sd_on_5g, 0x0)
						MOD_RADIO_REG_20693_ENTRY(pi, PA5G_CFG3, core,
							pa5g_ptat_slope_main, 0x0)
						MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core,
							ovr_pa5g_idac_incap_compen_main, 0x1)
					RADIO_REG_LIST_EXECUTE(pi, core);
					MOD_RADIO_REG_20693(pi, PA5G_INCAP, core,
						pa5g_idac_incap_compen_main,
						IS_ACR(pi) ? ((BW40 || BW80) ? 0xc : 0x3a) :
						0x16);
					MOD_RADIO_REG_20693(pi, PA5G_IDAC3, core,
						pa5g_idac_tuning_bias, 0x0);

					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
						ovr_pad5g_idac_gm, 1);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG8, core, pad5g_idac_gm,
						IS_ACR(pi) ? ((BW40 || BW80) ? 0x3f : 0x26) :
						0x1a);
					MOD_RADIO_REG_20693(pi, TXGM5G_CFG1, core,
						pad5g_idac_cascode,
						IS_ACR(pi) ? ((BW40 || BW80) ? 0xf : 0xe) :
						0xd);
					MOD_RADIO_REG_20693(pi, SPARE_CFG10, core,
						pad5g_idac_cascode2, 0x0);
					MOD_RADIO_REG_20693(pi, PA5G_INCAP, core, pad5g_idac_pmos,
						(IS_ACR(pi) && (BW40 || BW80)) ? 0xa : 0x1c);

					MOD_RADIO_REG_20693(pi, PA5G_IDAC3, core,
						pad5g_idac_tuning_bias, 0xd);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG6, core,
						mx5g_ptat_slope_cascode, 0x0);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG6, core,
						mx5g_ptat_slope_lodc, 0x2);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG4, core,
						mx5g_idac_cascode, 0xf);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG5, core,
						mx5g_idac_lodc, 0xa);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG5, core, mx5g_idac_bbdc,
						(BW40 && IS_ACR(pi)) ? 0x2 : 0xc);

					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
						ovr_pa5g_idac_main, 1);
					MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR3, core,
						ovr_pa5g_idac_cas, 1);

					if (BW80) {
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_main, (IS_ACR(pi)) ? 0x38 : 0x3a);
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_cas, (IS_ACR(pi)) ? 0x14 : 0x15);
					} else if (BW40) {
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_main, (IS_ACR(pi)) ? 0x34 : 0x28);
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_cas, (IS_ACR(pi)) ? 0x14 : 0x13);
					} else {
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_main, (IS_ACR(pi)) ? 0x20 : 0x1a);
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_cas, (IS_ACR(pi)) ? 0x12 : 0x11);
					}

					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG3, core,
						mx5g_pu_bleed, 0x0);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG1, core,
						mx5g_idac_bleed_bias, 0x0);
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG4, core,
						mx5g_idac_tuning_bias, 0xd);
				} else {
					/* 2G Tx settings */
					MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
							ovr_pa2g_idac_main, 0x1);
					MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
							ovr_pa2g_idac_cas, 0x1);
					if (CHSPEC_IS40(pi->radio_chanspec)) {
						MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
								pa2g_idac_main, 0x1e);
						MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
								pa2g_idac_cas, 0x21);
					} else {
						MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
								pa2g_idac_main, 0x19);
						MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
								pa2g_idac_cas, 0x24);
					}

					MOD_RADIO_REG_20693(pi, SPARE_CFG10, core,
							pa2g_incap_pmos_src_sel_gnd, 0x0);
					MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
							ovr_pa2g_idac_incap_compen_main, 0x1);
					MOD_RADIO_REG_20693(pi, PA2G_INCAP, core,
							pa2g_idac_incap_compen_main, 0x34);
				}
			}
		} if (!PHY_IPA(pi) && CHSPEC_IS5G(pi->radio_chanspec)) {
			/* EVM Ramp: TxBias5G & Pad5G on */
			FOREACH_CORE(pi, core) {
				RADIO_REG_LIST_START
					MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
						ovr_tx5g_bias_pu, 0x1)
					MOD_RADIO_REG_20693_ENTRY(pi, TX5G_CFG1, core,
						tx5g_bias_pu, 0x1)
					MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR3, core,
						ovr_pad5g_bias_cas_pu, 0x1)
					MOD_RADIO_REG_20693_ENTRY(pi, TX5G_CFG1, core,
						pad5g_bias_cas_pu, 0x1)
					MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core,
						ovr_pad5g_pu, 0x1)
					MOD_RADIO_REG_20693_ENTRY(pi, TX5G_CFG1, core,
						pad5g_pu, 0x1)
				RADIO_REG_LIST_EXECUTE(pi, core);
			}
		}
	}
}

static void
wlc_phy_set_noise_var_shaping_acphy(phy_info_t *pi, uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES],
                                             int8 *tone_id, uint8 *core_nv)
{
	uint8 i;

	/* Starting offset for nvshp */
	i = ACPHY_NV_NTONES_OFFSET;

	/* 4335C0 */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		if (!PHY_ILNA(pi)) {
			if (CHSPEC_IS80(pi->radio_chanspec)) {
				static const int8 tone_id_def[] = {-123, -122, -121, -120,
				                                   -119, -118, -117, -116,
				                                   -115, -114, -113, -112,
				                                    112,  113,  114,  115,
				                                    116,  117,  118,  119,
				                                    120,  121,  122,  123};
				static const uint8 noise_var_def[] = {0xF8, 0xF8, 0xF8, 0xF8,
				                                      0xF8, 0xF8, 0xF8, 0xF8,
				                                      0xFA, 0xFA, 0xFC, 0xFE,
				                                      0xFE, 0xFC, 0xFA, 0xFA,
				                                      0xF8, 0xF8, 0xF8, 0xF8,
				                                      0xF8, 0xF8, 0xF8, 0xF8};
				memcpy((tone_id + i), tone_id_def, sizeof(int8)*ACPHY_NV_NTONES);
				memcpy((noise_var[PHY_CORE_0] + i), noise_var_def,
				        sizeof(uint8)*ACPHY_NV_NTONES);
				*core_nv = 1; /* core 0 */
				PHY_INFORM(("wlc_phy_set_noise_var_shaping_acphy:"
				            "applying noise_var shaping for BW 80MHz\n"));
			}
		}
	}
}

/**
 * Whenever the transmit power is less than a certain value, lower PA power consumption can be
 * achieved by selecting lower PA linearity. The VLIN signal towards the FEM is configured to
 * either be driven by the FEM control table or by a chip internal VLIN signal.
 */
void wlc_phy_vlin_en_acphy(phy_info_t *pi)
{
	uint8 band2g_idx, core;
	uint8 stall_val;
	int16 idle_tssi[PHY_CORE_MAX];
	uint16 adj_tssi1[PHY_CORE_MAX];
	uint16 adj_tssi2[PHY_CORE_MAX], adj_tssi3[PHY_CORE_MAX];
	int16 tone_tssi1[PHY_CORE_MAX];
	int16 tone_tssi2[PHY_CORE_MAX], tone_tssi3[PHY_CORE_MAX];
	int16 a1[PHY_CORE_MAX];
	int16 b0[PHY_CORE_MAX];
	int16 b1[PHY_CORE_MAX];
	uint8 pwr1, pwr2, pwr3;
	uint8 txidx1 = 40, txidx2 = 90, txidx3;
	struct _orig_reg_vals {
		uint8 core;
		uint16 orig_OVR3;
		uint16 orig_auxpga_cfg1;
		uint16 orig_auxpga_vmid;
		uint16 orig_iqcal_cfg1;
		uint16 orig_tx5g_tssi;
		uint16 orig_pa2g_tssi;
		uint16 orig_RfctrlIntc;
		uint16 orig_RfctrlOverrideRxPus;
		uint16 orig_RfctrlCoreRxPu;
		uint16 orig_RfctrlOverrideAuxTssi;
		uint16 orig_RfctrlCoreAuxTssi1;
		} orig_reg_vals[PHY_CORE_MAX];
	uint core_count = 0;
	txgain_setting_t curr_gain1, curr_gain2, curr_gain3;
	bool init_adc_inside = FALSE;
	uint16 save_afePuCtrl, save_gpio;
	uint16 orig_use_txPwrCtrlCoefs;
	uint16 fval2g_orig, fval5g_orig, fval2g, fval5g;
	uint32 save_chipc = 0;
	uint16 save_gpioHiOutEn;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	txgain_setting_t curr_gain4;
	int16 tone_tssi4[PHY_CORE_MAX];
	uint16 adj_tssi4[PHY_CORE_MAX];
	int bbmultcomp;
	uint16 tempmuxTxVlinOnFemCtrl2;
	uint16 txidxval;
	uint16 txgaintemp1[3], txgaintemp1a[3];
	uint16 tempmuxTxVlinOnFemCtrl, globpusmask;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));
	/* prevent crs trigger */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	band2g_idx = CHSPEC_IS2G(pi->radio_chanspec);
	if (band2g_idx)	{
		pwr3 = pi_ac->chanmgri->cfg->vlinpwr2g_from_nvram;
		}
	else {
		pwr3 = pi_ac->chanmgri->cfg->vlinpwr5g_from_nvram;
		}
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* Turn off epa/ipa and unused rxrf part to prevent energy go into air */
	orig_use_txPwrCtrlCoefs = READ_PHYREGFLD(pi, TxPwrCtrlCmd,
	use_txPwrCtrlCoefs);
	FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
		/* save phy/radio regs going to be touched */
		orig_reg_vals[core_count].orig_RfctrlIntc = READ_PHYREGCE(pi,
		RfctrlIntc, core);
		orig_reg_vals[core_count].orig_RfctrlOverrideRxPus =
			READ_PHYREGCE(pi, RfctrlOverrideRxPus, core);
		orig_reg_vals[core_count].orig_RfctrlCoreRxPu =
			READ_PHYREGCE(pi, RfctrlCoreRxPus, core);
		orig_reg_vals[core_count].orig_RfctrlOverrideAuxTssi =
			READ_PHYREGCE(pi, RfctrlOverrideAuxTssi, core);
		orig_reg_vals[core_count].orig_RfctrlCoreAuxTssi1 =
			READ_PHYREGCE(pi, RfctrlCoreAuxTssi1, core);
		orig_reg_vals[core_count].orig_OVR3 = READ_RADIO_REGC(pi,
			RF, OVR3, core);
		orig_reg_vals[core_count].orig_auxpga_cfg1 =
			READ_RADIO_REGC(pi, RF, AUXPGA_CFG1, core);
		orig_reg_vals[core_count].orig_auxpga_vmid =
			READ_RADIO_REGC(pi, RF, AUXPGA_VMID, core);
		orig_reg_vals[core_count].orig_iqcal_cfg1 =
			READ_RADIO_REGC(pi, RF, IQCAL_CFG1, core);
		orig_reg_vals[core_count].orig_tx5g_tssi = READ_RADIO_REGC(pi,
			RF, TX5G_TSSI, core);
		orig_reg_vals[core_count].orig_pa2g_tssi = READ_RADIO_REGC(pi,
			RF, PA2G_TSSI, core);
		orig_reg_vals[core_count].core = core;
		/* set tssi_range = 0   (it suppose to bypass 10dB attenuation before pdet) */
		MOD_PHYREGCE(pi, RfctrlOverrideAuxTssi,  core, tssi_range, 1);
		MOD_PHYREGCE(pi, RfctrlCoreAuxTssi1,	 core, tssi_range, 0);
		/* turn off lna and other unsed rxrf components */
		WRITE_PHYREGCE(pi, RfctrlOverrideRxPus, core, 0x7CE0);
		WRITE_PHYREGCE(pi, RfctrlCoreRxPus, 	core, 0x0);
		++core_count;
		}
	ACPHY_ENABLE_STALL(pi, stall_val);
	/* tssi loopback setup */
	phy_ac_tssi_loopback_path_setup(pi, LOOPBACK_FOR_TSSICAL);

	if (!init_adc_inside) {
		wlc_phy_init_adc_read(pi, &save_afePuCtrl, &save_gpio,
			&save_chipc, &fval2g_orig, &fval5g_orig,
			&fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
		}
	wlc_phy_get_paparams_for_band_acphy(pi, a1, b0, b1);
	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		if (!init_adc_inside)
			wlc_phy_gpiosel_acphy(pi, 16+core, 1);
		/* Measure the Idle TSSI */
		wlc_phy_poll_samps_WAR_acphy(pi, idle_tssi, TRUE, TRUE, NULL,
		FALSE, init_adc_inside, core, 1);
		MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefs, 0);
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &curr_gain1, txidx1);
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi1, TRUE, FALSE,
			&curr_gain1, FALSE, init_adc_inside, core, 1);
		adj_tssi1[core] = 1024+idle_tssi[core]-tone_tssi1[core];
		adj_tssi1[core] = adj_tssi1[core] >> 3;
		pwr1 = wlc_phy_tssi2dbm_acphy(pi, adj_tssi1[core], a1[core], b0[core], b1[core]);
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &curr_gain2, txidx2);
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi2, TRUE, FALSE,
			&curr_gain2, FALSE, init_adc_inside, core, 1);
		adj_tssi2[core] = 1024+idle_tssi[core]-tone_tssi2[core];
		adj_tssi2[core] = adj_tssi2[core] >> 3;
		pwr2 = wlc_phy_tssi2dbm_acphy(pi, adj_tssi2[core], a1[core], b0[core], b1[core]);
		if (pwr2 != pwr1) {
			txidx3 = txidx1+(4*pwr3-pwr1) *(txidx2-txidx1)/(pwr2-pwr1);
		} else {
			txidx3 = txidx1;
		}
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &curr_gain3, txidx3);
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi3, TRUE, FALSE,
			&curr_gain3, FALSE, init_adc_inside, core, 1);
		adj_tssi3[core] = 1024+idle_tssi[core]-tone_tssi3[core];
		adj_tssi3[core] = adj_tssi3[core] >> 3;
		if (band2g_idx)	{
			globpusmask = 1<<(pi_ac->chanmgri->data->vlinmask2g_from_nvram);
		} else {
			globpusmask = 1<<(pi_ac->chanmgri->data->vlinmask5g_from_nvram);
		}
		tempmuxTxVlinOnFemCtrl = READ_PHYREGFLD(pi, RfctrlCoreGlobalPus,
			muxTxVlinOnFemCtrl);
		tempmuxTxVlinOnFemCtrl2 = (tempmuxTxVlinOnFemCtrl | globpusmask);
		MOD_PHYREG(pi, RfctrlCoreGlobalPus, muxTxVlinOnFemCtrl, tempmuxTxVlinOnFemCtrl2);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINCTRLBBMULTLUTS,
			1, txidx3, 48, &txgaintemp1);
		txgaintemp1a[0] = (txgaintemp1[0]|0x8000);
		txgaintemp1a[1] = txgaintemp1[1];
		txgaintemp1a[2] = txgaintemp1[2];
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINCTRLBBMULTLUTS, 1,
			txidx3, 48, txgaintemp1a);
		wlc_phy_get_txgain_settings_by_index_acphy(pi, &curr_gain4, txidx3);
		wlc_phy_poll_samps_WAR_acphy(pi, tone_tssi4, TRUE, FALSE,
			&curr_gain4, FALSE, init_adc_inside, core, 1);
		adj_tssi4[core] = 1024+idle_tssi[core]-tone_tssi4[core];
		adj_tssi4[core] = adj_tssi4[core] >> 3;
		bbmultcomp = (int)((tone_tssi3[core]-tone_tssi4[core])/6);
		pi_ac->chanmgri->data->vlin_txidx = txidx3;
		pi_ac->chanmgri->data->bbmult_comp = bbmultcomp;
		for (txidxval = txidx3; txidxval < 128; txidxval++) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINCTRLBBMULTLUTS, 1,
				txidxval, 48, &txgaintemp1);
			txgaintemp1a[0] = (txgaintemp1[0]|0x8000)+bbmultcomp;
			txgaintemp1a[1] = txgaintemp1[1];
			txgaintemp1a[2] = txgaintemp1[2];
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_GAINCTRLBBMULTLUTS, 1,
				txidxval, 48, txgaintemp1a);
			}
		if (!init_adc_inside)
			wlc_phy_restore_after_adc_read(pi, &save_afePuCtrl, &save_gpio,
			&save_chipc, &fval2g_orig, &fval5g_orig,
			&fval2g, &fval5g, &stall_val, &save_gpioHiOutEn);
		/* restore phy/radio regs */
		while (core_count > 0) {
			--core_count;
			phy_utils_write_radioreg(pi, RF_2069_OVR3(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_OVR3);
			phy_utils_write_radioreg(pi,
				RF_2069_AUXPGA_CFG1(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_auxpga_cfg1);
			phy_utils_write_radioreg(pi,
				RF_2069_AUXPGA_VMID(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_auxpga_vmid);
			phy_utils_write_radioreg(pi,
				RF_2069_IQCAL_CFG1(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_iqcal_cfg1);
			phy_utils_write_radioreg(pi,
				RF_2069_TX5G_TSSI(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_tx5g_tssi);
			phy_utils_write_radioreg(pi,
				RF_2069_PA2G_TSSI(orig_reg_vals[core_count].core),
				orig_reg_vals[core_count].orig_pa2g_tssi);
			WRITE_PHYREGCE(pi, RfctrlIntc, orig_reg_vals[core_count].core,
				orig_reg_vals[core_count].orig_RfctrlIntc);
			WRITE_PHYREGCE(pi, RfctrlOverrideRxPus,
				orig_reg_vals[core_count].core,
				orig_reg_vals[core_count].orig_RfctrlOverrideRxPus);
			WRITE_PHYREGCE(pi, RfctrlCoreRxPus, orig_reg_vals[core_count].core,
				orig_reg_vals[core_count].orig_RfctrlCoreRxPu);
			WRITE_PHYREGCE(pi, RfctrlOverrideAuxTssi,
				orig_reg_vals[core_count].core,
				orig_reg_vals[core_count].orig_RfctrlOverrideAuxTssi);
			WRITE_PHYREGCE(pi, RfctrlCoreAuxTssi1,
				orig_reg_vals[core_count].core,
				orig_reg_vals[core_count].orig_RfctrlCoreAuxTssi1);
			}
		MOD_PHYREG(pi, TxPwrCtrlCmd, use_txPwrCtrlCoefs, orig_use_txPwrCtrlCoefs);
		/* prevent crs trigger */
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		PHY_TRACE(("======= IQLOCAL PreCalGainControl : END =======\n"));
		}
}

/* customize papr shaping filters */
static void phy_ac_chanmgr_papr_iir_filt_reprog(phy_info_t *pi)
{
	uint16 k;

	// 20_in_20: [b, a] = cheby1(4, 1.7, 5/Fs*2); norm = 1.02
	// filt Fc is smaller, will be fixed in C0
	//
	// 20_40 and 20_80 are copied from 20_20 and 20_40
	// of the 4x filt designs, respectively,
	// [b, a] = cheby1(4, 1, 7/Fs*2);

	uint16 ppr_iir_phyreg_vals_rev32[][2] =
	{{ACPHY_papr_iir_20_20_group_dly(pi->pubpi.phy_rev), 4},
	 {ACPHY_papr_iir_20_20_b10(pi->pubpi.phy_rev), 127},
	 {ACPHY_papr_iir_20_20_b11(pi->pubpi.phy_rev), 254},
	 {ACPHY_papr_iir_20_20_b12(pi->pubpi.phy_rev), 127},
	 {ACPHY_papr_iir_20_20_a11(pi->pubpi.phy_rev), 341},
	 {ACPHY_papr_iir_20_20_a12(pi->pubpi.phy_rev), 109},
	 {ACPHY_papr_iir_20_20_b20(pi->pubpi.phy_rev), 126},
	 {ACPHY_papr_iir_20_20_b21(pi->pubpi.phy_rev), 252},
	 {ACPHY_papr_iir_20_20_b22(pi->pubpi.phy_rev), 126},
	 {ACPHY_papr_iir_20_20_a21(pi->pubpi.phy_rev), 318},
	 {ACPHY_papr_iir_20_20_a22(pi->pubpi.phy_rev), 82},
	 {ACPHY_papr_iir_20_40_group_dly(pi->pubpi.phy_rev), 6},
	 {ACPHY_papr_iir_20_40_b10(pi->pubpi.phy_rev), 66},
	 {ACPHY_papr_iir_20_40_b11(pi->pubpi.phy_rev), 131},
	 {ACPHY_papr_iir_20_40_b12(pi->pubpi.phy_rev), 66},
	 {ACPHY_papr_iir_20_40_a11(pi->pubpi.phy_rev), 308},
	 {ACPHY_papr_iir_20_40_a12(pi->pubpi.phy_rev), 111},
	 {ACPHY_papr_iir_20_40_b20(pi->pubpi.phy_rev), 74},
	 {ACPHY_papr_iir_20_40_b21(pi->pubpi.phy_rev), 149},
	 {ACPHY_papr_iir_20_40_b22(pi->pubpi.phy_rev), 74},
	 {ACPHY_papr_iir_20_40_a21(pi->pubpi.phy_rev), 306},
	 {ACPHY_papr_iir_20_40_a22(pi->pubpi.phy_rev), 88},
	 {ACPHY_papr_iir_20_80_group_dly(pi->pubpi.phy_rev), 6},
	 {ACPHY_papr_iir_20_80_b10(pi->pubpi.phy_rev), 17},
	 {ACPHY_papr_iir_20_80_b11(pi->pubpi.phy_rev), 35},
	 {ACPHY_papr_iir_20_80_b12(pi->pubpi.phy_rev), 17},
	 {ACPHY_papr_iir_20_80_a11(pi->pubpi.phy_rev), 274},
	 {ACPHY_papr_iir_20_80_a12(pi->pubpi.phy_rev), 119},
	 {ACPHY_papr_iir_20_80_b20(pi->pubpi.phy_rev), 20},
	 {ACPHY_papr_iir_20_80_b21(pi->pubpi.phy_rev), 40},
	 {ACPHY_papr_iir_20_80_b22(pi->pubpi.phy_rev), 20},
	 {ACPHY_papr_iir_20_80_a21(pi->pubpi.phy_rev), 280},
	 {ACPHY_papr_iir_20_80_a22(pi->pubpi.phy_rev), 106}};

	/* 20in20: cheby1(4,0.5,8/40*2) */
	uint16 ppr_iir_phyreg_vals_rev33[][2] =
	{{ACPHY_papr_iir_20_20_group_dly(pi->pubpi.phy_rev), 6},
	 {ACPHY_papr_iir_20_20_b10(pi->pubpi.phy_rev), 299},
	 {ACPHY_papr_iir_20_20_b11(pi->pubpi.phy_rev), 598},
	 {ACPHY_papr_iir_20_20_b12(pi->pubpi.phy_rev), 299},
	 {ACPHY_papr_iir_20_20_a11(pi->pubpi.phy_rev), 450},
	 {ACPHY_papr_iir_20_20_a12(pi->pubpi.phy_rev), 92},
	 {ACPHY_papr_iir_20_20_b20(pi->pubpi.phy_rev), 424},
	 {ACPHY_papr_iir_20_20_b21(pi->pubpi.phy_rev), 847},
	 {ACPHY_papr_iir_20_20_b22(pi->pubpi.phy_rev), 424},
	 {ACPHY_papr_iir_20_20_a21(pi->pubpi.phy_rev), 397},
	 {ACPHY_papr_iir_20_20_a22(pi->pubpi.phy_rev), 41},
	 {ACPHY_papr_iir_20_40_group_dly(pi->pubpi.phy_rev), 6},
	 {ACPHY_papr_iir_20_40_b10(pi->pubpi.phy_rev), 66},
	 {ACPHY_papr_iir_20_40_b11(pi->pubpi.phy_rev), 131},
	 {ACPHY_papr_iir_20_40_b12(pi->pubpi.phy_rev), 66},
	 {ACPHY_papr_iir_20_40_a11(pi->pubpi.phy_rev), 308},
	 {ACPHY_papr_iir_20_40_a12(pi->pubpi.phy_rev), 111},
	 {ACPHY_papr_iir_20_40_b20(pi->pubpi.phy_rev), 74},
	 {ACPHY_papr_iir_20_40_b21(pi->pubpi.phy_rev), 149},
	 {ACPHY_papr_iir_20_40_b22(pi->pubpi.phy_rev), 74},
	 {ACPHY_papr_iir_20_40_a21(pi->pubpi.phy_rev), 306},
	 {ACPHY_papr_iir_20_40_a22(pi->pubpi.phy_rev), 88},
	 {ACPHY_papr_iir_20_80_group_dly(pi->pubpi.phy_rev), 6},
	 {ACPHY_papr_iir_20_80_b10(pi->pubpi.phy_rev), 17},
	 {ACPHY_papr_iir_20_80_b11(pi->pubpi.phy_rev), 35},
	 {ACPHY_papr_iir_20_80_b12(pi->pubpi.phy_rev), 17},
	 {ACPHY_papr_iir_20_80_a11(pi->pubpi.phy_rev), 274},
	 {ACPHY_papr_iir_20_80_a12(pi->pubpi.phy_rev), 119},
	 {ACPHY_papr_iir_20_80_b20(pi->pubpi.phy_rev), 20},
	 {ACPHY_papr_iir_20_80_b21(pi->pubpi.phy_rev), 40},
	 {ACPHY_papr_iir_20_80_b22(pi->pubpi.phy_rev), 20},
	 {ACPHY_papr_iir_20_80_a21(pi->pubpi.phy_rev), 280},
	 {ACPHY_papr_iir_20_80_a22(pi->pubpi.phy_rev), 106}};

	if (!(ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	      ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		return;
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
		for (k = 0; k < ARRAYSIZE(ppr_iir_phyreg_vals_rev32); k++)  {
			phy_utils_write_phyreg(pi, ppr_iir_phyreg_vals_rev32[k][0],
					ppr_iir_phyreg_vals_rev32[k][1]);
		}
	} else if  (ACMAJORREV_33(pi->pubpi->phy_rev) ||
	            ACMAJORREV_37(pi->pubpi->phy_rev) ||
	            ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		for (k = 0; k < ARRAYSIZE(ppr_iir_phyreg_vals_rev33); k++)  {
			phy_utils_write_phyreg(pi, ppr_iir_phyreg_vals_rev33[k][0],
					ppr_iir_phyreg_vals_rev33[k][1]);
		}
	}
}

/* PAPRR Functions */
static void chanspec_setup_papr(phy_info_t *pi,
int8 papr_final_clipping, int8 papr_final_scaling)
{
	uint16 lowMcsGamma = 600, highMcsGamma, vhtMcsGamma_c8_c9 = 1100;
	uint16 highMcsGamma_c8_c11 = 1200;
	uint16 vhtMcsGamma_c10_c11 = 1100;
	uint32 gammaOffset_4347[4] = {0, 0, 0, 0};
	uint8 gainOffset[4] = {0, 0, 0, 0};
	uint32 gain = 128, gamma;
	uint32 gamma_4347[28];
	uint8 i, j, core, gain_4347[28];
	int16 *paprrmcsgamma = NULL;
	uint8 *paprrmcsgain = NULL;
	bool enable = chanspec_papr_enable(pi);

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		highMcsGamma = 950;
		vhtMcsGamma_c8_c9 = 950;
		lowMcsGamma = 600;
	} else {
		highMcsGamma = 1100;
		vhtMcsGamma_c8_c9 = 1100;
		vhtMcsGamma_c10_c11 = 1100;
		if ((ACMAJORREV_32(pi->pubpi->phy_rev) ||
		     ACMAJORREV_33(pi->pubpi->phy_rev) ||
		     ACMAJORREV_37(pi->pubpi->phy_rev) ||
		     ACMAJORREV_47_51(pi->pubpi->phy_rev)) &&
			(CHSPEC_IS20(pi->radio_chanspec)))
			lowMcsGamma = 700;
		else
			lowMcsGamma = 600;
	}

	if (!PHY_IPA(pi) && ACMAJORREV_2(pi->pubpi->phy_rev)) {
		vhtMcsGamma_c8_c9 = 8191;
	}

	if (enable) {
		MOD_PHYREG_4(pi, papr_ctrl, papr_blk_en, enable,
			papr_final_clipping_en, papr_final_clipping,
			papr_final_scaling_en, papr_final_scaling,
			papr_override_enable, 0);

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			FOREACH_CORE(pi, core) {
				switch (core) {
				case 0:
					MOD_PHYREG(pi, papr_gain_index_p0, papr_enable, 1);
					break;
				case 1:
					MOD_PHYREG(pi, papr_gain_index_p1, papr_enable, 1);
					break;
				case 2:
					MOD_PHYREG(pi, papr_gain_index_p2, papr_enable, 1);
					break;
				case 3:
					MOD_PHYREG(pi, papr_gain_index_p3, papr_enable, 1);
					break;
				default:
					PHY_ERROR(("%s: Max 4 cores only!\n", __FUNCTION__));
					ASSERT(0);
				}
			}
			phy_ac_chanmgr_papr_iir_filt_reprog(pi);
		}

		if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			/* Program enable/gainidx */
			gamma = 0x0; /* not used fields */
			for (i = 1; i <= 3; i++) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, i,
						32, &gamma);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, 0x40 + i,
						32, &gamma);
			}
			gamma = 0x80;
			for (i = 4; i <= 33; i++) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, i,
						32, &gamma);
			}

			/* Program gamma1/gamma */
			for (i = 0x44; i <= 0x40 + 31; i++) {
				if ((i >= 0x44 && i <= 0x47) || (i >= 0x4C && i <= 0x4E) ||
						(i >= 0x54 && i <= 0x56)) {
					gamma = (lowMcsGamma << 13) | lowMcsGamma;
				} else {
					gamma = (highMcsGamma << 13) | highMcsGamma;
					if (i >= 0x5C && i <= 0x5F) {
						gamma = (highMcsGamma_c8_c11 << 13) |
							highMcsGamma_c8_c11;
					}
				}
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, i,
						32, &gamma);
			}

			/* program GammaOffset fields */
			for (i = 0; i <= 2; i++) {
				uint16 gammaOffset[3] = {0, 100, 150};
				gamma = (gammaOffset[i] << 13) | gammaOffset[i];
				j = ((i == 2) ? 0 : (32 + i));
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, (0x40+j),
						32, &gamma);
				gamma = 0x80;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, j,
						32, &gamma);
			}
		} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			/* gain/gamma entries for different rates/bw */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (!((CHSPEC_CHANNEL(pi->radio_chanspec) == 13) ||
					(CHSPEC_CHANNEL(pi->radio_chanspec) == 1)) ||
					(pi->fdss_bandedge_2g_en == 0)) {
					paprrmcsgamma = pi->paprrmcsgamma2g;
					paprrmcsgain = pi->paprrmcsgain2g;
				} else if ((CHSPEC_CHANNEL(pi->radio_chanspec) == 13)) {
					paprrmcsgamma = pi->paprrmcsgamma2g_ch13;
					paprrmcsgain = pi->paprrmcsgain2g_ch13;
				} else {
					paprrmcsgamma = pi->paprrmcsgamma2g_ch1;
					paprrmcsgain = pi->paprrmcsgain2g_ch1;
				}
			} else {
				if (CHSPEC_IS20(pi->radio_chanspec)) {
					paprrmcsgamma = pi->paprrmcsgamma5g20;
					paprrmcsgain = pi->paprrmcsgain5g20;
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					paprrmcsgamma = pi->paprrmcsgamma5g40;
					paprrmcsgain = pi->paprrmcsgain5g40;
				} else {
					paprrmcsgamma = pi->paprrmcsgamma5g80;
					paprrmcsgain = pi->paprrmcsgain5g80;
				}
			}
			/* set gamma to default values if not explicitly specified */
			if (paprrmcsgamma[8] == -1) {
				paprrmcsgamma[8] = vhtMcsGamma_c8_c9;
			}
			if (paprrmcsgamma[9] == -1) {
				paprrmcsgamma[9] = vhtMcsGamma_c8_c9;
			}
			if (paprrmcsgamma[10] == -1) {
				 paprrmcsgamma[10] = vhtMcsGamma_c10_c11;
			}
			if (paprrmcsgamma[11] == -1) {
				 paprrmcsgamma[11] = vhtMcsGamma_c10_c11;
			}

			/* Hard code dot11ac vhtmcs8_9, vhtmcs10_11 gain/gamma values */
			gain_4347[24] = paprrmcsgain[8];
			gain_4347[25] = paprrmcsgain[9];
			gain_4347[26] = paprrmcsgain[10];
			gain_4347[27] = paprrmcsgain[11];

			gamma_4347[24] = (paprrmcsgamma[8] << 13) | paprrmcsgamma[8];
			gamma_4347[25] = (paprrmcsgamma[9] << 13) | paprrmcsgamma[9];
			gamma_4347[26] = (paprrmcsgamma[10] << 13) | paprrmcsgamma[10];
			gamma_4347[27] = (paprrmcsgamma[11] << 13) | paprrmcsgamma[11];

			for (j = 0; j < 8; j++) {
				if (paprrmcsgamma[j] == -1) {
					if (j < 5) {
						paprrmcsgamma[j] = lowMcsGamma;
					} else {
						paprrmcsgamma[j] = highMcsGamma;
					}
				}
				/* Set m0 --> m7 settings for 11ag rates */
				gain_4347[j] = paprrmcsgain[j];
				gamma_4347[j] = (paprrmcsgamma[j] << 13) | paprrmcsgamma[j];

				/* Replicate m0 --> m7 settings for 11n and 11ac rates */
				gain_4347[j+8] = gain_4347[j];
				gain_4347[j+16] = gain_4347[j];
				gamma_4347[j+8] = gamma_4347[j];
				gamma_4347[j+16] = gamma_4347[j];
			}
			/* Write gain values from offset 4 to offset 31 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 28, 4, 8, &gain_4347);
			/* Write gain offsets from offset 32 to offset 35 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 4, 32, 8, &gainOffset);
			/* Write gamma values from offset 68 to offset 95 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 28, 68, 32, &gamma_4347);
			/* Write gamma offsets from offset 96 to offset 99 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 4, 96, 32,
				&gammaOffset_4347);
		} else {
			for (j = 4; j <= 32; j++) {
				if (j <= 29) {
					/* gain entries for different rates */
					gain = 128;
				} else {
					/* gain offsets */
					gain = 0;
				}
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, j, 32, &gain);
			}
			for (j = 0x44; j <= 0x5D; j++) {
				/* tbl offset starting 0x40 is gamma table */
				if (j >= 0x5C && j <= 0x5D) {
					/* vht rate mcs8 mcs 9 256 QAM */
					gamma = (vhtMcsGamma_c8_c9 << 13) | vhtMcsGamma_c8_c9;
				} else if ((j >= 0x44 && j <= 0x47) || (j >= 0x4c && j <= 0x4e) ||
					(j >= 0x54 && j <= 0x56)) {
					/* All BPSK and QPSK rates */
					gamma = (lowMcsGamma << 13) | lowMcsGamma;
				} else {
					/* ALL 16QAM and 64QAM rates */
					gamma = (highMcsGamma << 13) | highMcsGamma;
				}
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, j, 32, &gamma);
			}
			for (i = 0, j = 0x5E; j <= 0x60; j++, i++) {
				if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
					uint16 gammaOffset[3] = {0, 100, 150};
					gamma = (gammaOffset[i] << 13) | gammaOffset[i];
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, j, 32,
						&gamma);
				} else {
					uint16 gammaOffset[3] = {0, 0, 0};
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPR, 1, j, 32,
						&gammaOffset[i]);
				}
			}
		}
	} else {
		MOD_PHYREG(pi, papr_ctrl, papr_blk_en, enable);
	}
}

static void
wlc_phy_spurwar_nvshp_acphy(phy_info_t *pi, bool bw_chg, bool spurwar, bool nvshp)
{
	uint8 i, core;
	uint8 core_nv = 0, core_sp = 0;
	uint8 noise_var[PHY_CORE_MAX][ACPHY_SPURWAR_NV_NTONES];
	int8 tone_id[ACPHY_SPURWAR_NV_NTONES];
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	/* Initialize variables */
	for (i = 0; i < ACPHY_SPURWAR_NV_NTONES; i++) {
		tone_id[i]   = 0;
		FOREACH_CORE(pi, core)
			noise_var[core][i] = 0;
	}

	/* Table reset req or not */
	if (nvshp && !bw_chg && !spurwar)
		nvshp = FALSE;

	if (spurwar || nvshp) {
		/* Reset Table */
		wlc_phy_reset_noise_var_shaping_acphy(pi);

		/* Call nvshp */
		if (nvshp)
			wlc_phy_set_noise_var_shaping_acphy(pi, noise_var, tone_id, &core_nv);

		/* Call spurwar */
		if (spurwar)
			phy_ac_spurwar(pi_ac->rxspuri, noise_var, tone_id, &core_sp);

		/* Write table
		 * If both nvshp and spurwar tries to write same tone
		 * priority lies with spurwar
		 */
		wlc_phy_noise_var_shaping_acphy(pi, core_nv, core_sp, tone_id, noise_var, 0);
	}
}

/* Set up rx2tx rfseq tables differently for cal vs. packets for tiny */
/* to avoid problems with AGC lock-up */
void
phy_ac_rfseq_mode_set(phy_info_t *pi, bool cal_mode)
{
	if (cal_mode) {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, rfseq_majrev4_rx2tx_cal_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				rfseq_majrev4_rx2tx_cal_dly);
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, rfseq_majrev32_rx2tx_cal_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				rfseq_majrev32_rx2tx_cal_dly);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, rfseq_majrev36_rx2tx_cal_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				rfseq_majrev36_rx2tx_cal_dly);
		} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
					rfseq_majrev40_tx2rx_cal_cmd);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
					rfseq_majrev40_tx2rx_cal_dly);
			}
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10,
					16, get_rfseq_majrev44(pi, TX2RX_CAL_SEQ_CMD_TBL));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80,
					16, get_rfseq_majrev44(pi, TX2RX_CAL_SEQ_DLY_TBL));
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, rfseq_rx2tx_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				rfseq_rx2tx_dly);
		}
	} else {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
				tiny_rfseq_rx2tx_tssi_sleep_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				tiny_rfseq_rx2tx_tssi_sleep_dly);
		} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
					rfseq_majrev40_tx2rx_cmd);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
					rfseq_majrev40_tx2rx_dly);
			}
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10,
				16, get_rfseq_majrev44(pi, TX2RX_SEQ_CMD_TBL));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80,
				16, get_rfseq_majrev44(pi, TX2RX_SEQ_DLY_TBL));
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
				rfseq_majrev32_rx2tx_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
				rfseq_majrev32_rx2tx_dly);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x0, 16,
				rfseq_majrev36_rx2tx_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
				rfseq_majrev36_rx2tx_dly);

			if ((CHSPEC_IS2G(pi->radio_chanspec) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x1)) ||
				(CHSPEC_IS5G(pi->radio_chanspec) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x4))) {

				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x0, 16,
					rfseq_majrev36_rx2tx_tssi_sleep_cmd);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
					rfseq_majrev36_rx2tx_tssi_sleep_dly);
			}
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
				tiny_rfseq_rx2tx_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
				tiny_rfseq_rx2tx_dly);
		}
	}
}

static void
wlc_phy_radio20693_set_reset_table_bits(phy_info_t *pi, uint16 tbl_id, uint16 offset,
	uint16 start, uint16 end, uint16 val, uint8 tblwidth)
{
	uint16 val_shift, mask;
	uint32 data[2];

	val_shift = val << start;
	mask  = ((1 << (end + 1)) - (1 << start));
	wlc_phy_table_read_acphy(pi, tbl_id, 1, offset, tblwidth, &data);

	data[0] = ((data[0] & mask) | val_shift);
	wlc_phy_table_write_acphy(pi, tbl_id, 1, offset, tblwidth, &data);
}

#ifndef ACPHY_1X1_ONLY
static void
wlc_phy_write_tx_farrow_acphy(phy_ac_chanmgr_info_t *ci, chanspec_t chanspec)
{
	uint8	ch = CHSPEC_CHANNEL(chanspec), afe_clk_num, afe_clk_den;
	uint16	a, b, lb_b = 0;
	uint32	fcw, lb_fcw, tmp_low = 0, tmp_high = 0;
	uint32  deltaphase;
	uint16  deltaphase_lo, deltaphase_hi;
	uint16  farrow_downsamp;
	phy_info_t *pi = ci->pi;
	uint32	fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ? WF_CHAN_FACTOR_2_4_G
	                                                               : WF_CHAN_FACTOR_5_G);

	if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1) {
		if (CHSPEC_BW_LE20(chanspec)) {
			if (CHSPEC_IS5G(chanspec)) {
				if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
				    !(ISSIM_ENAB(pi->sh->sih)) &&
				    (((((fc == 5180) && (pi->sh->chippkg != 2)) ||
				       ((fc >= 5200) && (fc <= 5320)) ||
				       ((fc >= 5745) && (fc <= 5825))) && !PHY_IPA(pi)))) {
					a = 10;
				} else if (((RADIOMAJORREV(pi) == 2) &&
				            ((fc == 5745) || (fc == 5765) || (fc == 5825 &&
				        !PHY_IPA(pi)))) && !(ISSIM_ENAB(pi->sh->sih))) {
					a = 18;
				} else {
					a = 16;
				}
			} else {
				if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
				    !(ISSIM_ENAB(pi->sh->sih))) {
				    phy_ac_radio_data_t *r = phy_ac_radio_get_data(ci->aci->radioi);
			        if ((r->srom_txnospurmod2g == 0) && !PHY_IPA(pi)) {
						a = 9;
					} else if (((fc != 2412) && (fc != 2467)) ||
						(pi->xtalfreq == 40000000) ||
						(ACMAJORREV_2(pi->pubpi->phy_rev) &&
						(ACMINORREV_1(pi) ||
						ACMINORREV_3(pi) ||
						ACMINORREV_5(pi)) &&
						pi->xtalfreq == 37400000 && PHY_ILNA(pi))) {
						a = 18;
					} else {
						a = 16;
					}
				} else {
					a = 16;
				}
			}
			b = 160;
		} else if (CHSPEC_IS40(chanspec)) {
			if (CHSPEC_IS5G(chanspec)) {
				if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
				       !PHY_IPA(pi) && (fc != 5190)) {
					a = 10;
				} else if ((RADIOMAJORREV(pi) == 2) &&
				       !PHY_IPA(pi) && (fc == 5190)) {
					a = 6;
				} else if (((RADIOMAJORREV(pi) == 2) &&
				     ((fc == 5755) || (fc == 5550 && pi->xtalfreq == 40000000) ||
				      (fc == 5310 && pi->xtalfreq == 37400000 && PHY_IPA(pi)))) &&
				    !(ISSIM_ENAB(pi->sh->sih))) {
					a = 9;
				} else {
					a = 8;
				}
			} else {
				if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
				    !(ISSIM_ENAB(pi->sh->sih))) {
					a = 9;
				} else {
					a = 8;
				}
			}
			b = 320;
		} else {
			a = 6;
			b = 640;
		}
	} else if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 2) {
		a = 6;
		b = 640;
		lb_b = 320;
	} else {
		a = 8;
		b = 320;
		lb_b = 320;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		afe_clk_num = 2;
		afe_clk_den = 3;
	} else {
		afe_clk_num = 3;
		afe_clk_den = 2;
		if (fc == 5290 && ACMAJORREV_2(pi->pubpi->phy_rev) &&
		    ((ACMINORREV_1(pi) && pi->sh->chippkg == 2) ||
		     ACMINORREV_3(pi)) && PHY_XTAL_IS37M4(pi)) {
			afe_clk_num = 4;
			afe_clk_den = 3;
		}
	}

	math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
		1 << 23, (fc * afe_clk_den) >> 1);
	math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);
	wlc_phy_tx_farrow_mu_setup(pi, fcw & 0xffff, (fcw & 0xff0000) >> 16, fcw & 0xffff,
		(fcw & 0xff0000) >> 16);
	/* DAC MODE 1 lbfarrow setup in rx_farrow_acphy */
	if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode != 1) {
		math_uint64_multiple_add(&tmp_high, &tmp_low, fc * afe_clk_den,
		        1 << 25, 0);
		math_uint64_divide(&lb_fcw, tmp_high, tmp_low, a * afe_clk_num * lb_b);
		deltaphase = (lb_fcw - 33554431) >> 1;
		deltaphase_lo = deltaphase & 0xffff;
		deltaphase_hi = (deltaphase >> 16) & 0xff;
		farrow_downsamp = fc * afe_clk_den / (a * afe_clk_num * lb_b);
		WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, deltaphase_lo);
		WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, deltaphase_hi);
		WRITE_PHYREG(pi, lbFarrowDriftPeriod, 5120);
		MOD_PHYREG(pi, lbFarrowCtrl, lb_farrow_downsampfactor, farrow_downsamp);
	}
}
#endif /* ACPHY_1X1_ONLY */

static void
wlc_phy_write_rx_farrow_acphy(phy_ac_chanmgr_info_t *ci, chanspec_t chanspec)
{
	uint16 deltaphase_lo, deltaphase_hi;
	uint8 ch = CHSPEC_CHANNEL(chanspec), num, den, bw, M, vco_div;
	uint32 deltaphase, farrow_in_out_ratio, fcw, tmp_low = 0, tmp_high = 0;
	uint16 drift_period, farrow_ctrl;
	uint8 farrow_outsft_reg, dec_outsft_reg, farrow_outscale_reg = 1;
	phy_info_t *pi = ci->pi;
	uint32 fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ?
	        WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);
	if (CHSPEC_IS80(chanspec)) {
		farrow_outsft_reg = 0;
		dec_outsft_reg = 0;
	} else {
		if (((ACMAJORREV_0(pi->pubpi->phy_rev)) && ((ACMINORREV_1(pi)) ||
		    (ACMINORREV_0(pi)))) || ((ACMAJORREV_1(pi->pubpi->phy_rev)) &&
		    (ACMINORREV_1(pi) || ACMINORREV_0(pi)))) {
			farrow_outsft_reg = 2;
		} else {
			farrow_outsft_reg = 0;
		}
		dec_outsft_reg = 3;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		num = 3;
		den = 2;
	} else {
		num = 2;
		den = 3;
		if (CHSPEC_IS80(chanspec) && fc == 5290 && ACMAJORREV_2(pi->pubpi->phy_rev) &&
		    ((ACMINORREV_1(pi) && pi->sh->chippkg == 2) ||
		    ACMINORREV_3(pi)) && PHY_XTAL_IS37M4(pi)) {
			num = 3;
			den = 4;
		}
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) && !(ISSIM_ENAB(pi->sh->sih))) {
			if (CHSPEC_IS40(chanspec)) {
				bw = 40;
				M = 4;
				vco_div = 18;
				drift_period = 1920;
			} else {
				phy_ac_radio_data_t *r = phy_ac_radio_get_data(ci->aci->radioi);
				if ((r->srom_txnospurmod2g == 0) && !PHY_IPA(pi)) {
					bw = 20;
					M = 8;
					vco_div = 9;
					drift_period = 2880;
				} else if ((fc != 2412 && fc != 2467) ||
					(pi->xtalfreq == 40000000) ||
					(ACMAJORREV_2(pi->pubpi->phy_rev) &&
					(ACMINORREV_1(pi) ||
					ACMINORREV_3(pi) ||
					ACMINORREV_5(pi)) &&
					pi->xtalfreq == 37400000 && PHY_ILNA(pi))) {
					bw = 20;
					M = 8;
					vco_div = 18;
					drift_period = 5760;
				} else {
					bw = 20;
					M = 8;
					vco_div = 16;
					drift_period = 5120;
				}
			}
		} else {
			bw = 20;
			M = 8;
			vco_div = 16;
			drift_period = 5120;
		}
	} else {
		if (CHSPEC_IS80(chanspec)) {
			bw = 80;
			M = 4;
			vco_div = 6;
			drift_period = 2880;
			if (fc == 5290 && (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
			    ((((RADIOMINORREV(pi) == 4) ||
			       (RADIOMINORREV(pi) == 10) ||
			       (RADIOMINORREV(pi) == 11) ||
			       (RADIOMINORREV(pi) == 13)) &&
			      (pi->sh->chippkg == 2)) ||
			     ((RADIOMINORREV(pi) == 7) ||
			     (RADIOMINORREV(pi) == 9) ||
			     (RADIOMINORREV(pi) == 8) ||
			     (RADIOMINORREV(pi) == 12))) &&
			    (pi->xtalfreq == 37400000)) {
				drift_period = 2560;
			}
		} else {
			if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) {
				if (CHSPEC_IS20(chanspec) &&
				    (((fc == 5180) && (pi->sh->chippkg != 2)) ||
				     ((fc >= 5200) && (fc <= 5320)) ||
				     ((fc >= 5745) && (fc <= 5825))) &&
				    !PHY_IPA(pi)) {
					bw = 20;
					M = 8;
					vco_div = 10;
					drift_period = 2400;
				} else if (CHSPEC_IS40(chanspec) && !PHY_IPA(pi) && (fc != 5190)) {
					bw = 20;
					M = 8;
					vco_div = 20;
					drift_period = 4800;
				} else if (CHSPEC_IS40(chanspec) && !PHY_IPA(pi) && (fc == 5190)) {
					bw = 20;
					M = 8;
					vco_div = 12;
					drift_period = 2880;
				} else if ((((fc == 5755 || (fc == 5550 &&
					pi->xtalfreq == 40000000) ||
					(fc == 5310 && pi->xtalfreq == 37400000 &&
					PHY_IPA(pi))) && (CHSPEC_IS40(chanspec))) ||
					((fc == 5745 || fc == 5765 ||
					(fc == 5825 && !PHY_IPA(pi))) &&
					(CHSPEC_IS20(chanspec)))) && !(ISSIM_ENAB(pi->sh->sih))) {
					bw = 20;
					M = 8;
					vco_div = 18;
					drift_period = 4320;
				} else {
					bw = 20;
					M = 8;
					vco_div = 16;
					drift_period = 3840;
				}
			} else {
				bw = 20;
				M = 8;
				vco_div = 16;
				drift_period = 3840;
			}
		}
	}
	math_uint64_multiple_add(&tmp_high, &tmp_low, fc * num, 1 << 25, 0);
	math_uint64_divide(&fcw, tmp_high, tmp_low, (uint32) (den * vco_div * M * bw));

	farrow_in_out_ratio = (fcw >> 25);
	deltaphase = (fcw - 33554431)>>1;
	deltaphase_lo = deltaphase & 0xffff;
	deltaphase_hi = (deltaphase >> 16) & 0xff;
	farrow_ctrl = (dec_outsft_reg & 0x3) | ((farrow_outscale_reg & 0x3) << 2) |
		((farrow_outsft_reg & 0x7) << 4) | ((farrow_in_out_ratio & 0x3) <<7);

	WRITE_PHYREG(pi, rxFarrowDeltaPhase_lo, deltaphase_lo);
	WRITE_PHYREG(pi, rxFarrowDeltaPhase_hi, deltaphase_hi);
	WRITE_PHYREG(pi, rxFarrowDriftPeriod, drift_period);
	WRITE_PHYREG(pi, rxFarrowCtrl, farrow_ctrl);
	MOD_PHYREG_3(pi, lbFarrowCtrl, lb_farrow_outShift, farrow_outsft_reg,
		lb_decimator_output_shift, dec_outsft_reg, lb_farrow_outScale, farrow_outscale_reg);
	/* Use the same settings for the loopback Farrow */
	if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1) {
		WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, deltaphase_lo);
		WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, deltaphase_hi);
		WRITE_PHYREG(pi, lbFarrowDriftPeriod, drift_period);
		MOD_PHYREG(pi, lbFarrowCtrl, lb_farrow_downsampfactor, farrow_in_out_ratio);
	}
}

static void
wlc_phy_radio20695_etdac_pwrdown(phy_info_t *pi)
{
#ifndef WL_ETMODE
	if (!ET_ENAB(pi)) {
	RADIO_REG_LIST_START
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AFE_CFG1_OVR2, 0, ovr_etdac_pu_diode, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AFE_CFG1_OVR2, 0, ovr_etdac_pu, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AFE_CFG1_OVR2, 0, ovr_etdacbuff_pu, 1)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, AFE_CFG1_OVR2, 0, ovr_etdac_reset, 1)

		MOD_RADIO_REG_28NM_ENTRY(pi, RF, ETDAC_CFG1, 0, etdac_pwrup_diode, 0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, ETDAC_CFG1, 0, etdac_pwrup, 0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, ETDAC_CFG1, 0, etdac_buf_pu, 0)
		MOD_RADIO_REG_28NM_ENTRY(pi, RF, ETDAC_CFG1, 0, etdac_reset, 0)

	RADIO_REG_LIST_EXECUTE(pi, 0);
	}
#endif // endif
}

#ifndef WL_FDSS_DISABLED
static void
wlc_phy_fdss_init(phy_info_t *pi)
{
	uint8 core;
	uint8 nbkpts = 5;
	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (pi->fdss_level_2g[core] == 6) {
					pi->fdss_interp_en = 0;
					nbkpts = 4;
				} else if ((pi->fdss_level_2g[core] == 7) ||
					(pi->fdss_level_2g[core] == 8)) {
					pi->fdss_interp_en = 0;
					nbkpts = 6;
				} else {
					pi->fdss_interp_en = 1;
				}
			} else {
				if ((pi->fdss_level_5g[core] == 9) ||
					(pi->fdss_level_5g[core] == 10) ||
					(pi->fdss_level_5g[core] == 11)) {
					pi->fdss_interp_en = 0;
				} else {
					pi->fdss_interp_en = 1;
				}
			}
		}

		MOD_PHYREGCEE(pi, txfdss_ctrl, core, txfdss_enable, 1);
		MOD_PHYREGCEE(pi, txfdss_ctrl, core, txfdss_interp_enable, pi->fdss_interp_en);
		MOD_PHYREGCEE(pi, txfdss_cfgtbl, core, txfdss_num_20M_tbl, 2);
		MOD_PHYREGCEE(pi, txfdss_cfgtbl, core, txfdss_num_40M_tbl, 2);
		MOD_PHYREGCEE(pi, txfdss_cfgbrkpt0_, core, txfdss_num_20M_breakpoints, nbkpts);
		MOD_PHYREGCEE(pi, txfdss_cfgbrkpt0_, core, txfdss_num_40M_breakpoints, nbkpts);
		MOD_PHYREGCEE(pi, txfdss_cfgbrkpt1_, core, txfdss_num_80M_breakpoints, nbkpts);
		MOD_PHYREGCEE(pi, txfdss_scaleadj_en_, core, txfdss_scale_adj_enable, 7);
	}
}

static void
wlc_phy_set_fdss_table(phy_info_t *pi)
{
	uint8 core;
	uint8 nbkpts = 5;
	uint8 *fdss_tbl = NULL;
	uint8 *bkpt_tbl_20 = NULL;
	uint8 *bkpt_tbl_40 = NULL;
	uint8 *bkpt_tbl_80 = NULL;
	uint8 val = 0;
	uint8 mcstable[71] = {16, 16, 16, 16, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		};
	uint8 mcstable_majorrev4[71] = {16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		};
	uint8 mcstable_majorrev4_53574[71] = {16, 16, 16, 16, 16, 16, 0, 0,
		16, 16, 16, 16, 16, 0, 0, 0,
		16, 16, 16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 16, 16, 16, 0, 0,
		16, 16, 16, 16, 16, 0, 0, 0,
		16, 16, 16, 16, 16, 0, 0, 0, 0, 0,
		16,
		16, 16, 16, 16, 16, 16, 0, 0,
		16, 16, 16, 16, 16, 0, 0, 0, 0, 0,
		};

	uint8 i, fdss_level[2];
	uint8 breakpoint_list_20[5] = {0, 3, 17, 48, 62};
	uint8 breakpoint_list_40[5] = {0, 6, 34, 96, 124};
	uint8 breakpoint_list_80[5] = {0, 12, 68, 192, 248};
	uint8 breakpoint_list_interp_20[2] = {47, 61};
	uint8 breakpoint_list_interp_40[2] = {97, 123};
	uint8 breakpoint_list_interp_80[2] = {191, 247};

	/* introducing new fdss table for 4359 */
	uint8 fdss_scale_level[5][5] = {{128, 128, 128, 128, 128},
		{128, 128, 128, 128, 128},
		{164, 146, 104, 146, 164}, /* Mild, meets older +1, -3 dB flatness limits */
		{180, 128, 72, 128, 180}, /* Extreme, meets older +3, -5 dB flatness limits */
		{170, 146, 85, 146, 170}  /* intermediate fdss coeff for 4359 */		};
	int16 fdss_scale_level_interp_20[5][5] = {{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{-683, -338, 0, 338, 683},
		{-2219, -512, 0, 512, 2219},
		{-683, -338, 0, 338, 683}};
	int16 fdss_scale_level_interp_40[5][5] = {{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{-341, -169, 0, 169, 341},
		{-1109, -256, 0, 256, 1109},
		{-341, -169, 0, 169, 341}};
	int16 fdss_scale_level_interp_80[5][5] = {{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{-171, -86, 0, 86, 171},
		{-555, -128, 0, 128, 555},
		{-171, -86, 0, 86, 171}};
	uint8 fdss_scale_level_adjust_20[5] = {128, 128, 132, 128, 128};
	uint8 fdss_scale_level_adjust_40[5] = {128, 128, 132, 128, 128};
	uint8 fdss_scale_level_adjust_80[5] = {128, 128, 134, 128, 128};
	uint8 fdss_scale_level_adjust_interp_20[5] = {128, 128, 132, 128, 128};
	uint8 fdss_scale_level_adjust_interp_40[5] = {128, 128, 131, 128, 128};
	uint8 fdss_scale_level_adjust_interp_80[5] = {128, 128, 134, 128, 128};

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		fdss_level[0] = pi->fdss_level_2g[0];
		if (pi->fdss_level_2g[1] ==  -1) {
			fdss_level[1] = 0;
		} else {
			fdss_level[1] = pi->fdss_level_2g[1];
		}
	} else {
		fdss_level[0] = pi->fdss_level_5g[0];
		if (pi->fdss_level_5g[1] ==  -1) {
			fdss_level[1] = 0;
		} else {
			fdss_level[1] = pi->fdss_level_5g[1];
		}
	}
	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FDSS_MCSINFOTBL(core),
				71, 0, 8, (ROUTER_4349(pi) ?
				mcstable_majorrev4_53574 : mcstable_majorrev4));
		} else {
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_MCSINFOTBL(core), 71, 0, 8, mcstable);
		}
		bkpt_tbl_20 = breakpoint_list_20;
		bkpt_tbl_40 = breakpoint_list_40;
		bkpt_tbl_80 = breakpoint_list_80;

	/* Populate breakpoint and scale tables with the scale values for each BW */
		for (i = 0; i < 2; i++) {
			fdss_tbl = fdss_scale_level[fdss_level[i]];
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts, nbkpts*i, 8,
				bkpt_tbl_20);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts, nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i, 8,
				&fdss_scale_level_adjust_20[fdss_level[i]]);

			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts,
				(2 * nbkpts) + nbkpts*i, 8,
				bkpt_tbl_40);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts,
				(2 * nbkpts) + nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i+2, 8,
				&fdss_scale_level_adjust_40[fdss_level[i]]);

			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts,
				(4 * nbkpts) + nbkpts*i, 8,
				bkpt_tbl_80);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts,
				(4 * nbkpts) + nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i+4, 8,
				&fdss_scale_level_adjust_80[fdss_level[i]]);
		}

	/* Edit  breakpoint table for interpolation case */
		if (pi->fdss_interp_en) {
			for (i = 0; i < 2; i++) {
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 3+5*i, 8, breakpoint_list_interp_20);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 5*i, 16, fdss_scale_level_interp_20[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i, 8, &fdss_scale_level_adjust_interp_20[fdss_level[i]]);

				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 13+5*i, 8, breakpoint_list_interp_40);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 10+5*i, 16, fdss_scale_level_interp_40[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i+2, 8,
					&fdss_scale_level_adjust_interp_40[fdss_level[i]]);

				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 23+5*i, 8, breakpoint_list_interp_80);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 20+5*i, 16, fdss_scale_level_interp_80[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i+4, 8,
					&fdss_scale_level_adjust_interp_80[fdss_level[i]]);
			}
		}
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
			1, 5*3*2, 8, &val);
		/* 5: table length; 3: 20MHz 40MHz and 80MHz; 2: low rate and high rate */
	}

}

static void
wlc_phy_set_fdss_table_majrev_ge40(phy_info_t *pi)
{
	uint8 core;
	uint8 nbkpts;
	uint8 *fdss_tbl = NULL;
	uint8 *bkpt_tbl_20 = NULL;
	uint8 *bkpt_tbl_40 = NULL;
	uint8 *bkpt_tbl_80 = NULL;
	uint8 mcstable[71] = {16, 16, 16, 16, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		16, 16, 16, 16, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		17,
		16, 16, 16, 17, 17, 17, 17, 17,
		16, 16, 16, 17, 17, 17, 17, 17, 17, 17,
		};
	uint8 mcstable_majorrev40[71] = {16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		0,
		16, 16, 16, 0, 0, 0, 0, 0,
		16, 16, 16, 0, 0, 0, 0, 0, 0, 0,
		};

	uint8 i, fdss_level[2];
	uint8 breakpoint_list_20[5] = {0, 3, 17, 48, 62};
	uint8 breakpoint_list_20_4347_fdss6[4] = {0, 17, 33, 48};
	uint8 breakpoint_list_20_4347_fdss7[6] = {0, 9, 17, 36, 48, 56};
	uint8 breakpoint_list_20_4347_fdss8[6] = {0, 9, 17, 36, 48, 56};
	uint8 breakpoint_list_80_4347_fdss9[5] = {0, 38, 85, 171, 219};
	uint8 breakpoint_list_40_4347_fdss9[5] = {0, 19, 44, 85, 110};
	uint8 breakpoint_list_20_4347_fdss9[5] = {0, 9, 18, 47, 56};
	uint8 breakpoint_list_40[5] = {0, 6, 34, 96, 124};
	uint8 breakpoint_list_80[5] = {0, 12, 68, 192, 248};
	uint8 breakpoint_list_interp_20[2] = {47, 61};
	uint8 breakpoint_list_interp_40[2] = {97, 123};
	uint8 breakpoint_list_interp_80[2] = {191, 247};
		/* introducing new fdss table for 4359 */
	uint8 fdss_scale_level[6][5] = {{128, 128, 128, 128, 128},
		{128, 128, 128, 128, 128},
		{164, 146, 104, 146, 164}, /* Mild, meets older +1, -3 dB flatness limits */
		{180, 128, 72, 128, 180}, /* Extreme, meets older +3, -5 dB flatness limits */
		{165, 142, 83, 142, 165},  /* intermediate fdss coeff for 4359 */
		{144, 127, 114, 127, 144} /* intermediate fdss coeff for 4355 */};
	uint8 fdss_scale_level_4347_fdss6[4] = {138, 111, 118, 138};
	uint8 fdss_scale_level_4347_fdss7[6] = {137, 124, 114, 122, 124, 137};
	uint8 fdss_scale_level_4347_fdss8[6] = {141, 128, 126, 115, 128, 141};
	uint8 fdss_scale_level_4347_fdss9[5] = {152, 123, 111, 123, 152};
	uint8 fdss_scale_level_4347_fdss10[5] = {193, 137, 102, 137, 193};
	uint8 fdss_scale_level_4347_fdss11[5] = {166, 143, 106, 143, 166};
	uint8 fdss_scale_level_4347_ch13_fdss6[4] = {138, 111, 118, 138};
	/* boost right side spectrun for channel 13 */
	uint8 fdss_scale_level_4347_ch13_fdss7[6] = {135, 128, 125, 120, 122, 135};
	/* boost fdss scaling on right side for channel 13 */
	uint8 fdss_scale_level_4347_ch13_fdss8[6] = {128, 128, 128, 122, 124, 137};
	uint8 fdss_scale_level_4347_ch1_fdss6[4] = {138, 111, 118, 138};
	/* boost left side spectrun  for channel1 */
	uint8 fdss_scale_level_4347_ch1_fdss7[6] = {133, 120, 110, 140, 133, 133};
	/* boost fdss scaling on left side for channel 1 */
	uint8 fdss_scale_level_4347_ch1_fdss8[6] = {137, 124, 114, 128, 128, 128};
	int16 fdss_scale_level_interp_20[6][5] = {{0, 0, 0, 0, 0},
		{-1700, -362, 0, 362, 1700},
		{-683, -338, 0, 338, 683},
		{-2219, -512, 0, 512, 2219},
		{-683, -338, 0, 338, 683},
		{-1700, -498, 0, 498, 1700}};
	int16 fdss_scale_level_interp_40[5][5] = {{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{-341, -169, 0, 169, 341},
		{-1109, -256, 0, 256, 1109},
		{-341, -169, 0, 169, 341}};
	int16 fdss_scale_level_interp_80[5][5] = {{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{-171, -86, 0, 86, 171},
		{-555, -128, 0, 128, 555},
		{-171, -86, 0, 86, 171}};
	uint8 fdss_scale_level_adjust_20[12] = {128, 128, 132, 128, 128, 128,
		128, 127, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_20_ch1[12] = {128, 128, 132, 128, 128, 128,
		128, 127, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_20_ch13[12] = {128, 128, 132, 128, 128, 128,
		128, 127, 128, 128, 128, 128};
	uint8* p_fdss_scale_level_adjust_20;
	uint8 fdss_scale_level_adjust_40[12] = {128, 128, 132, 128, 128, 128,
		128, 128, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_80[12] = {128, 128, 134, 128, 128, 128,
		128, 128, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_interp_20[12] = {128, 128, 132, 128, 128,
		128, 128, 128, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_interp_40[12] = {128, 128, 131, 128, 128,
		128, 128, 128, 128, 128, 128, 128};
	uint8 fdss_scale_level_adjust_interp_80[12] = {128, 128, 134, 128, 128,
		128, 128, 128, 128, 128, 128, 128};

	p_fdss_scale_level_adjust_20 = fdss_scale_level_adjust_20;

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (!((CHSPEC_CHANNEL(pi->radio_chanspec) == 13) ||
			(CHSPEC_CHANNEL(pi->radio_chanspec) == 1)) ||
		    (pi->fdss_bandedge_2g_en == 0)) {
			fdss_level[0] = pi->fdss_level_2g[0];
			if (pi->fdss_level_2g[1] ==  -1) {
				fdss_level[1] = 0;
			} else {
				fdss_level[1] = pi->fdss_level_2g[1];
			}
		} else if ((CHSPEC_CHANNEL(pi->radio_chanspec) == 13)) {
			fdss_level[0] = pi->fdss_level_2g_ch13[0];
			if (pi->fdss_level_2g_ch13[1] ==  -1) {
				fdss_level[1] = 0;
			} else {
				fdss_level[1] = pi->fdss_level_2g_ch13[1];
			}
			p_fdss_scale_level_adjust_20 = fdss_scale_level_adjust_20_ch13;
		} else {
			fdss_level[0] = pi->fdss_level_2g_ch1[0];
			if (pi->fdss_level_2g_ch1[1] ==  -1) {
				fdss_level[1] = 0;
			} else {
				fdss_level[1] = pi->fdss_level_2g_ch1[1];
			}
			p_fdss_scale_level_adjust_20 = fdss_scale_level_adjust_20_ch1;
		}
	} else {
		fdss_level[0] = pi->fdss_level_5g[0];
		if (pi->fdss_level_5g[1] ==  -1) {
			fdss_level[1] = 0;
		} else {
			fdss_level[1] = pi->fdss_level_5g[1];
		}
	}
	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			if (!PHY_IPA(pi) && !ROUTER_4349(pi)) {
				uint8 mcs_value = 16;
				int8 new_mcs_value[77] = { 0 };
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					/* Currently enabling only for 20MHz mcs0-5 for ag rates,
						mcs0-4 for n & ac rates
					*/
					for (i = 0; i < 5; i++) {
						new_mcs_value[i] = mcs_value;
						new_mcs_value[i+8] = mcs_value;
						new_mcs_value[i+16] = mcs_value;
					}
					new_mcs_value[5] = mcs_value;
				} else {
				/* Currently enabling only for mcs0-4 */
					for (i = 0; i < 5; i++) {
						new_mcs_value[i] = mcs_value;
						new_mcs_value[i+8] = mcs_value;
						new_mcs_value[i+16] = mcs_value;
					}
					new_mcs_value[5] = mcs_value;
					for (i = 0; i < 5; i++) {
						new_mcs_value[i+28] = mcs_value;
						new_mcs_value[i+36] = mcs_value;
						new_mcs_value[i+44] = mcs_value;
					}
					new_mcs_value[33] = mcs_value;
					for (i = 0; i < 5; i++) {
						new_mcs_value[i+57] = mcs_value;
						new_mcs_value[i+65] = mcs_value;
					}
					new_mcs_value[62] = mcs_value;
				}
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_MCSINFOTBL(core), 77,
					0, 8, &new_mcs_value);
			} else {
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_MCSINFOTBL(core), 77, 0,
					8, mcstable_majorrev40);
			}
		} else {
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_MCSINFOTBL(core), 77, 0, 8, mcstable);
		}
		bkpt_tbl_20 = breakpoint_list_20;
		bkpt_tbl_40 = breakpoint_list_40;
		bkpt_tbl_80 = breakpoint_list_80;

		/* Populate breakpoint and scale tables with the scale values for each BW */
		for (i = 0; i < 2; i++) {
			if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
				if (fdss_level[i] == 6) {
					nbkpts = 4;
					fdss_tbl = fdss_scale_level_4347_fdss6;
					bkpt_tbl_20 = breakpoint_list_20_4347_fdss6;
					if (pi->fdss_bandedge_2g_en) {
					  if (CHSPEC_CHANNEL(pi->radio_chanspec) == 13) {
					    fdss_tbl = fdss_scale_level_4347_ch13_fdss6;
					  } else if (CHSPEC_CHANNEL(pi->radio_chanspec) == 1) {
					    fdss_tbl = fdss_scale_level_4347_ch1_fdss6;
					  }
					}
				} else if (fdss_level[i] == 7) {
					nbkpts = 6;
					fdss_tbl = fdss_scale_level_4347_fdss7;
					if (pi->fdss_bandedge_2g_en) {
					  if (CHSPEC_CHANNEL(pi->radio_chanspec) == 13) {
					    fdss_tbl = fdss_scale_level_4347_ch13_fdss7;
					  } else if (CHSPEC_CHANNEL(pi->radio_chanspec) == 1) {
					    fdss_tbl = fdss_scale_level_4347_ch1_fdss7;
					  }
					}
					bkpt_tbl_20 = breakpoint_list_20_4347_fdss7;
				} else if (fdss_level[i] == 8) {
					nbkpts = 6;
					fdss_tbl = fdss_scale_level_4347_fdss8;
					if (pi->fdss_bandedge_2g_en) {
					  if (CHSPEC_CHANNEL(pi->radio_chanspec) == 13) {
					    fdss_tbl = fdss_scale_level_4347_ch13_fdss8;
					  } else if (CHSPEC_CHANNEL(pi->radio_chanspec) == 1) {
					    fdss_tbl = fdss_scale_level_4347_ch1_fdss8;
					  }
					}
					bkpt_tbl_20 = breakpoint_list_20_4347_fdss8;
				} else if (fdss_level[i] == 9) {
					nbkpts = 5;
					fdss_tbl = fdss_scale_level_4347_fdss9;
					bkpt_tbl_80 = breakpoint_list_80_4347_fdss9;
					bkpt_tbl_40 = breakpoint_list_40_4347_fdss9;
					bkpt_tbl_20 = breakpoint_list_20_4347_fdss9;
				} else if (fdss_level[i] == 10) {
					nbkpts = 5;
					fdss_tbl = fdss_scale_level_4347_fdss10;
				} else if (fdss_level[i] == 11) {
					nbkpts = 5;
					breakpoint_list_20[1] = 2;
					breakpoint_list_20[4] = 63;
					breakpoint_list_80[1] = 14;
					breakpoint_list_80[4] = 243;
					fdss_tbl = fdss_scale_level_4347_fdss11;
				} else {
					nbkpts = 5;
					fdss_tbl = fdss_scale_level[fdss_level[i]];
				}
			} else {
				nbkpts = 5;
				fdss_tbl = fdss_scale_level[fdss_level[i]];
			}
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts, nbkpts*i, 8,
				bkpt_tbl_20);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts, nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i, 8,
				(p_fdss_scale_level_adjust_20 + fdss_level[i]));

			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts,
				(2 * nbkpts) + nbkpts*i, 8,
				bkpt_tbl_40);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts,
				(2 * nbkpts) + nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i+2, 8,
				&fdss_scale_level_adjust_40[fdss_level[i]]);

			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core), nbkpts,
				(4 * nbkpts) + nbkpts*i, 8,
				bkpt_tbl_80);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEFACTORSTBL(core), nbkpts,
				(4 * nbkpts) + nbkpts*i, 8,
				fdss_tbl);
			wlc_phy_table_write_acphy(pi,
				ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core), 1, i+4, 8,
				&fdss_scale_level_adjust_80[fdss_level[i]]);
		}
		/* Edit  breakpoint table for interpolation case */

		if (pi->fdss_interp_en) {
			for (i = 0; i < 2; i++) {
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 3+5*i, 8, breakpoint_list_interp_20);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 5*i, 16, fdss_scale_level_interp_20[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i, 8, &fdss_scale_level_adjust_interp_20[fdss_level[i]]);

				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 13+5*i, 8, breakpoint_list_interp_40);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 10+5*i, 16, fdss_scale_level_interp_40[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i+2, 8,
					&fdss_scale_level_adjust_interp_40[fdss_level[i]]);

				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_BREAKPOINTSTBL(core),
					2, 23+5*i, 8, breakpoint_list_interp_80);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEFACTORSDELTATBL(core),
					5, 20+5*i, 16, fdss_scale_level_interp_80[fdss_level[i]]);
				wlc_phy_table_write_acphy(pi,
					ACPHY_TBL_ID_FDSS_SCALEADJUSTFACTORSTBL(core),
					1, i+4, 8,
					&fdss_scale_level_adjust_interp_80[fdss_level[i]]);
			}
		}
	}
}
#endif /* WL_FDSS_DISABLED */

static void
wlc_acphy_load_4349_specific_tbls(phy_info_t *pi)
{
	wlc_acphy_load_radiocrisscross_phyovr_mode(pi);
	wlc_acphy_load_logen_tbl(pi);
}

static void
wlc_acphy_load_radiocrisscross_phyovr_mode(phy_info_t *pi)
{
	uint8 core;
	FOREACH_CORE(pi, core) {
		WRITE_PHYREGCE(pi, AfeClkDivOverrideCtrlN, core, 0x0000);
		WRITE_PHYREGCE(pi, RfctrlAntSwLUTIdxN, core, 0x0000);
		WRITE_PHYREGCE(pi, RfctrlCoreTxPus, core,
			(READ_PHYREGCE(pi, RfctrlCoreTxPus, core) & 0x7DFF));
		WRITE_PHYREGCE(pi, RfctrlOverrideTxPus, core,
			(READ_PHYREGCE(pi, RfctrlOverrideTxPus, core) & 0xF3FF));
	}
}

static void wlc_acphy_load_logen_tbl(phy_info_t *pi)
{
	/* 4349BU */
	if (ACMAJORREV_4(pi->pubpi->phy_rev))
		return;

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		if (phy_get_phymode(pi) == PHYMODE_MIMO) {
			/* set logen mimodes pu */
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x14d, 1, 1, 0, 16);
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x15d, 1, 1, 1, 16);
			/* Set logen mimosrc pu */
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x14d, 4, 4, 1, 16);
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x15d, 4, 4, 0, 16);
		} else {
			/* set logen mimodes pu */
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x14d, 1, 1, 0, 16);
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x15d, 1, 1, 0, 16);
			/* Set logen mimosrc pu */
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x14d, 4, 4, 0, 16);
			wlc_phy_radio20693_set_reset_table_bits(pi, ACPHY_TBL_ID_RFSEQ,
				0x15d, 4, 4, 0, 16);
		}
	}
}

static void
wlc_phy_set_regtbl_on_band_change_acphy_20693(phy_info_t *pi)
{

	uint8 core = 0;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID));

	if (RADIOMAJORREV(pi) == 3) {
		wlc_phy_radio20693_sel_logen_mode(pi);
		return;
	}

	FOREACH_CORE(pi, core)
	{
		if (CHSPEC_IS2G(pi->radio_chanspec))
		{
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
				TX_TOP_2G_OVR_EAST, core), 0x0);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
				TX_TOP_2G_OVR1_EAST, core), 0x0);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
				RX_TOP_2G_OVR_EAST, core), 0x0);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
				RX_TOP_2G_OVR_EAST2, core), 0x0);
			if (PHY_IPA(pi)) {
				phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
					BG_TRIM2, core), 0x1937);
			}

			if (RADIOMAJORREV(pi) == 2) {
				MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, core,
					ovr_mix5g_lobuf_en, 0);
				MOD_RADIO_REG_20693(pi, LNA5G_CFG3, core, mix5g_lobuf_en, 0);
			}
			RADIO_REG_LIST_START
				MOD_RADIO_REG_20693_ENTRY(pi, TIA_CFG8, core,
					tia_offset_dac_biasadj, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_tr_rx_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core, lna2g_tr_rx_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_tr_rx_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG1, core, lna5g_tr_rx_en, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_auxgm_pwrup, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, LOGEN_CFG2, core, logencore_5g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, LOGEN_OVR1, core,
					ovr_logencore_5g_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX5G_CFG1, core, tx5g_bias_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
					ovr_tx5g_bias_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TXMIX5G_CFG4, core, mx5g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core, ovr_mx5g_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TXMIX5G_CFG4, core,
					mx5g_pu_lodc_loop, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core,
					ovr_mx5g_pu_lodc_loop, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA5G_CFG1, core, pa5g_bias_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
					ovr_pa5g_bias_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA5G_CFG1, core, pa5g_bias_cas_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core,
					ovr_pa5g_bias_cas_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA5G_CFG4, core, pa5g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 1)
			RADIO_REG_LIST_EXECUTE(pi, core);

			if ((RADIO20693_MAJORREV(pi->pubpi->radiorev) == 1) &&
				(RADIO20693_MINORREV(pi->pubpi->radiorev) == 1)) {
				MOD_RADIO_REG_20693(pi, TRSW5G_CFG1, core, trsw5g_pu, 0);
				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core, ovr_trsw5g_pu, 1);
			}

			RADIO_REG_LIST_START
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LOGEN5G_CFG1, core,
					logen5g_tx_enable_5g, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
					ovr_logen5g_tx_enable_5g, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LOGEN5G_CFG1, core,
					logen5g_tx_enable_5g_low_band, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR2, core,
					ovr_logen5g_tx_enable_5g_low_band, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG1, core, lna5g_lna1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_lna1_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_gm5g_pwrup, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_RSSI1, core,
					lna5g_dig_wrssi1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_dig_wrssi1_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_pu_auxlna2, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG3, core, mix5g_en, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core, ovr_mix5g_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LPF_CFG2, core,
					lpf_sel_5g_out_gm, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LPF_CFG3, core,
					lpf_sel_2g_5g_cmref_gm, 0)
			RADIO_REG_LIST_EXECUTE(pi, core);
			/* Bimodal settings */
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				RADIO_REG_LIST_START
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, RXMIX2G_CFG1, core,
						rxmix2g_pu, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
						ovr_rxdiv2g_rs, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, RXRF2G_CFG1, core,
						rxdiv2g_rs, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
						ovr_rxdiv2g_pu_bias, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, RXRF2G_CFG1, core,
						rxdiv2g_pu_bias, 1)
					/* Turn off 5g overrides */
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_mix5g_en, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG3, core, mix5g_en, 0)
				RADIO_REG_LIST_EXECUTE(pi, core);
				if (!(PHY_IPA(pi)) && (RADIO20693REV(pi->pubpi->radiorev) == 13)) {
					wlc_phy_set_bias_ipa_as_epa_acphy_20693(pi, core);
				}
			}
		}
		else
		{
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core), 0);
			phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core), 0);
			if (PHY_IPA(pi) && !(ROUTER_4349(pi))) {
				phy_utils_write_radioreg(pi, RADIO_REG_20693(pi,
					BG_TRIM2, core), 0x1737);
			}

			if (RADIOMAJORREV(pi) == 2) {
				MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, core,
					ovr_mix5g_lobuf_en, 1);
				MOD_RADIO_REG_20693(pi, LNA5G_CFG3, core, mix5g_lobuf_en, 1);
			}
			MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_tr_rx_en,
				(ROUTER_4349(pi) ? 0 : 1));
			MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core, lna5g_tr_rx_en,
				(ROUTER_4349(pi) ? 0 : 1));
			RADIO_REG_LIST_START
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_lna1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_gm5g_pwrup, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_dig_wrssi1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_pu_auxlna2, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TIA_CFG8, core,
					tia_offset_dac_biasadj, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_tr_rx_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core, lna2g_tr_rx_en, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, LOGEN_CFG2, core, logencore_2g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, LOGEN_OVR1, core,
					ovr_logencore_2g_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_auxgm_pwrup, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG2, core, gm2g_pwrup, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_pwrup, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX2G_CFG1, core, tx2g_bias_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_tx2g_bias_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TXMIX2G_CFG2, core, mx2g_bias_en, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_mx2g_bias_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA2G_CFG1, core, pa2g_bias_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_pa2g_bias_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA2G_CFG1, core, pa2g_2gtx_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_pa2g_2gtx_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, PA2G_IDAC2, core, pa2g_bias_cas_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_pa2g_bias_cas_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LOGEN2G_CFG1, core,
					logen2g_tx_pu_bias, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_logen2g_tx_pu_bias, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LOGEN2G_CFG1, core,
					logen2g_tx_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_2G_OVR_EAST, core,
					ovr_logen2g_tx_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_rxmix2g_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RXMIX2G_CFG1, core, rxmix2g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_lna2g_dig_wrssi1_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_RSSI1, core,
					lna2g_dig_wrssi1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_lna1_pu, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core, lna2g_lna1_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG3, core, mix5g_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core, ovr_mix5g_en, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, LOGEN_OVR1, core,
					ovr_logencore_5g_pu, 0)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LPF_CFG2, core,
					lpf_sel_5g_out_gm, 1)
				MOD_RADIO_REG_20693_ENTRY(pi, TX_LPF_CFG3, core,
					lpf_sel_2g_5g_cmref_gm, 1)
			RADIO_REG_LIST_EXECUTE(pi, core);
			/* Bimodal settings */
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				RADIO_REG_LIST_START
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_mix5g_en, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, LNA5G_CFG3, core, mix5g_en, 1)
					/* Turn off 2G overrides */
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2, core,
						ovr_rxmix2g_pu, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RXMIX2G_CFG1, core,
						rxmix2g_pu, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
						ovr_rxdiv2g_rs, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RXRF2G_CFG1, core,
						rxdiv2g_rs, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST, core,
						ovr_rxdiv2g_pu_bias, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RXRF2G_CFG1, core,
						rxdiv2g_pu_bias, 0)
				RADIO_REG_LIST_EXECUTE(pi, core);
				if (!ROUTER_4349(pi) && PHY_IPA(pi))
					MOD_RADIO_REG_20693(pi, TXMIX5G_CFG8, core,
						pad5g_idac_gm, 58);
			}
		} /* band */
	} /* foreach core */

	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		wlc_phy_radio20693_mimo_core1_pmu_restore_on_bandhcange(pi);
	}

	/* This is applicable only for 4349B0 variants */
	if (PHY_IPA(pi)) {
		if ((RADIO20693REV(pi->pubpi->radiorev) >= 0xE) &&
			(RADIO20693REV(pi->pubpi->radiorev) <= 0x12)) {
			FOREACH_CORE(pi, core) {
				phy_utils_write_radioreg(pi, RADIO_REG_20693(pi, BG_TRIM2, core),
					(CHSPEC_IS2G(pi->radio_chanspec)) ? 0x1937 : 0x1737);
			}
			/* minipmu_cal */
			wlc_phy_tiny_radio_minipmu_cal(pi);
		}
	}

	if (ROUTER_4349(pi) && (((pi->sh->boardrev >> 8) & 0xf) >= 0x3)) {
		FOREACH_CORE(pi, core) {
			MOD_RADIO_REG_20693(pi, BG_TRIM2, core, bg_pmu_vbgtrim, 26);
			MOD_RADIO_REG_20693(pi, PMU_CFG1, core, wlpmu_vrefadj_cbuck, 6);
		}
		/* minipmu_cal */
		wlc_phy_tiny_radio_minipmu_cal(pi);
	}
}

static void
wlc_phy_load_channel_smoothing_tiny(phy_info_t *pi)
{

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
	    /* set 64 48-bit entries */
		wlc_phy_table_write_tiny_chnsmth(pi,
			ACPHY_TBL_ID_CORE0CHANSMTH_FLTR,
			CHANSMTH_FLTR_LENGTH, 0, 48, acphy_Smth_tbl_4349);
		if (phy_get_phymode(pi) == PHYMODE_MIMO) {
			wlc_phy_table_write_tiny_chnsmth(pi,
				ACPHY_TBL_ID_CORE1CHANSMTH_FLTR,
				CHANSMTH_FLTR_LENGTH, 0, 48, acphy_Smth_tbl_4349);
		}
	} else {
	    const uint16 zero_table[3] = { 0, 0, 0 };
	    acphytbl_info_t tbl;
	    tbl.tbl_id = ACPHY_TBL_ID_CHANNELSMOOTHING_1x1;
		tbl.tbl_ptr = zero_table;
		tbl.tbl_len = 1;
		tbl.tbl_offset = 0;
		tbl.tbl_width = 48;
		/* clear 1st 128 48-bit entries */
		for (tbl.tbl_offset = 0; tbl.tbl_offset < 128; tbl.tbl_offset++) {
			wlc_phy_table_write_ext_acphy(pi, &tbl);
		}

		/* set next 64 48-bit entries */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_CHANNELSMOOTHING_1x1,
		                          CHANSMTH_FLTR_LENGTH, 128,
		                          tbl.tbl_width, acphy_Smth_tbl_tiny);

		/* clear next 64 48-bit entries */
		for (tbl.tbl_offset = 128 + (ARRAYSIZE(acphy_Smth_tbl_tiny) / 3);
		     tbl.tbl_offset < 256;
		     tbl.tbl_offset++) {
			wlc_phy_table_write_ext_acphy(pi, &tbl);
		}
	}
}
static void
wlc_phy_set_reg_on_reset_acphy_20693(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
}

static void
wlc_phy_set_spareReg_on_reset_majorrev4(phy_info_t *pi, uint8 phyrxchain)
{
	uint16 spare_reg = READ_PHYREG(pi, SpareReg);

	if (phy_get_phymode(pi) == PHYMODE_MIMO) {
		/* The targeted use case is mimo mode coremask 1 case.
		 * Below settings will turn off some of the blocks for core 1
		 * and thus resulting in current savings
		 */
		if (phyrxchain == 1) {
			/* bit #12: Limit hrp access to core0 alone. Should be
			   made 1 before m aking 1 bits 8,9,13 and should
			   be made 0 only after bits 8,9,13 are made 0.
			   Recommended value: 0x1
			 */
			WRITE_PHYREG(pi, SpareReg, (spare_reg & ~(1 << 12)));
			spare_reg = READ_PHYREG(pi, SpareReg);

			/* bit #8: Use core1 clk for second chain like rsdb except div4 clk
			   Recommended value: 0x1
			 */
			spare_reg &= ~(1 << 8);
			/* bit #9: Turn off core1 divider in phy1rx1 */
			/* Recommended value: 0x1 */
			spare_reg &= ~(1 << 9);
			/* bit #13: Use core1 clk for second chain for div4 clk */
			/* Recommended value: 0x1 */
			spare_reg &= ~(1 << 13);
		}
		/* bit #10: Turn off core1 divider in RX2 */
		/* Recommended value: 0x1 */
		spare_reg &= ~(1 << 10);
	}

	/* bit #6: Duration control of Rx2tx reset to some designs. Enable always */
	spare_reg |= (1 << 6);

	/* bit #11: Turn off RX2 during TX */
	spare_reg |= (1 << 11);

	WRITE_PHYREG(pi, SpareReg, spare_reg);
}

static void
wlc_phy_set_tssiSleep_on_reset(phy_info_t *pi)
{
	uint8 srom_tssisleep_en = pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en;

	if (ACMAJORREV_1(pi->pubpi->phy_rev) || (ACMAJORREV_2(pi->pubpi->phy_rev) && PHY_IPA(pi))) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (((CHSPEC_BW_LE20(pi->radio_chanspec)) && (srom_tssisleep_en & 0x1)) ||
			  ((CHSPEC_IS40(pi->radio_chanspec)) && (srom_tssisleep_en & 0x2))) {
				MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
			} else {
				MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
			}
		} else {
			if (((CHSPEC_BW_LE20(pi->radio_chanspec)) && (srom_tssisleep_en & 0x4)) ||
			  ((CHSPEC_IS40(pi->radio_chanspec)) && (srom_tssisleep_en & 0x8)) ||
			  ((CHSPEC_IS80(pi->radio_chanspec)) && (srom_tssisleep_en & 0x10))) {
				MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
			} else {
				MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
			}
		}
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		if ((CHSPEC_IS2G(pi->radio_chanspec) && (srom_tssisleep_en & 0x1)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) && (srom_tssisleep_en & 0x4))) {

			MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
		} else {
			MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
		}
	} else {
		MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
	}
}

static void
wlc_phy_set_mfLessAve_on_reset(phy_info_t *pi)
{
	if (ACREV_GE(pi->pubpi->phy_rev, 32)) {
		uint8 core;

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			/*  Increase bphy digigain freeze time to 3 us */
			MOD_PHYREG(pi, overideDigiGain1, cckdigigainEnCntValue, 119);
			FOREACH_CORE(pi, core) {
				MOD_PHYREGCE(pi, crsControll, core, mfLessAve, 0);
				MOD_PHYREGCE(pi, crsControlu, core, mfLessAve, 0);
				MOD_PHYREGCE(pi, crsControllSub1, core, mfLessAve, 0);
				MOD_PHYREGCE(pi, crsControluSub1, core, mfLessAve, 0);
			}
		/* Retain reset values for 43012A0 and 4347 */
		} else if (!(ACMAJORREV_36(pi->pubpi->phy_rev) ||
			ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev))) {
			FOREACH_CORE(pi, core) {
				MOD_PHYREGCE(pi, crsControlu, core, mfLessAve, 0);
				if ((wlc_phy_ac_phycap_maxbw(pi) > BW_20MHZ)) {
					MOD_PHYREGCE(pi, crsControll, core, mfLessAve, 0);
					MOD_PHYREGCE(pi, crsControllSub1, core, mfLessAve, 0);
					MOD_PHYREGCE(pi, crsControluSub1, core, mfLessAve, 0);
				}
			}
		}
	} else {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, crsControll, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControlu, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControllSub1, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControluSub1, mfLessAve, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}
}

static void
wlc_phy_set_peakThresh_on_reset(phy_info_t *pi)
{
	if (!(ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev))) {
		if (ACREV_GE(pi->pubpi->phy_rev, 32)) {
			uint8 core;

			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			    ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				FOREACH_CORE(pi, core) {
					MOD_PHYREGCE(pi, crsThreshold2l, core, peakThresh, 85);
					MOD_PHYREGCE(pi, crsThreshold2u, core, peakThresh, 85);
					MOD_PHYREGCE(pi, crsThreshold2lSub1, core, peakThresh, 85);
					MOD_PHYREGCE(pi, crsThreshold2uSub1, core, peakThresh, 85);
				}
			} else if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
				FOREACH_CORE(pi, core) {
					MOD_PHYREGCE(pi, crsThreshold2u, core, peakThresh, 85);
					if ((wlc_phy_ac_phycap_maxbw(pi) > BW_20MHZ)) {
						MOD_PHYREGCE(pi, crsThreshold2l, core,
							peakThresh, 85);
						MOD_PHYREGCE(pi, crsThreshold2lSub1, core,
							peakThresh, 85);
						MOD_PHYREGCE(pi, crsThreshold2uSub1, core,
							peakThresh, 85);
					}
				}
			}
		} else {
			ACPHY_REG_LIST_START
				MOD_PHYREG_ENTRY(pi, crsThreshold2l, peakThresh, 85)
				MOD_PHYREG_ENTRY(pi, crsThreshold2u, peakThresh, 85)
				MOD_PHYREG_ENTRY(pi, crsThreshold2lSub1, peakThresh, 85)
				MOD_PHYREG_ENTRY(pi, crsThreshold2uSub1, peakThresh, 85)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	} else {
		if (ACMAJORREV_5(pi->pubpi->phy_rev) && CHSPEC_IS20(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, crsThreshold2u, 0x2055);
			WRITE_PHYREG(pi, crsThreshold2l, 0x2055);
		} else {
			WRITE_PHYREG(pi, crsThreshold2u, 0x204d);
			WRITE_PHYREG(pi, crsThreshold2l, 0x204d);
		}
		WRITE_PHYREG(pi, crsThreshold2lSub1, 0x204d);
		WRITE_PHYREG(pi, crsThreshold2uSub1, 0x204d);
	}
}

static void
wlc_phy_set_reg_on_reset_majorrev32_33_37_47_51(phy_info_t *pi)
{
	uint8 core;
	uint16 rxbias, txbias;

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, crshighlowpowThresholdl, core, low2highpowThresh, 69);
		MOD_PHYREGCE(pi, crshighlowpowThresholdu, core, low2highpowThresh, 69);
		MOD_PHYREGCE(pi, crshighlowpowThresholdlSub1, core, low2highpowThresh, 69);
		MOD_PHYREGCE(pi, crshighlowpowThresholduSub1, core, low2highpowThresh, 69);

		// This is to save power, the setting is applicable to all chips.
		MOD_PHYREGCE(pi, forceFront, core, freqCor, 0);
		MOD_PHYREGCE(pi, forceFront, core, freqEst, 0);

		// Old DCC related rf regs used for 4349/4345
		if (RADIOID(pi->pubpi->radioid) == BCM20693_ID) {
			MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core,
			              ovr_tia_offset_comp_pwrup, 1);
			MOD_RADIO_REG_20693(pi, TIA_CFG15, core, tia_offset_comp_pwrup, 1);
		}

		/* No limit for now on max analog gain */
		MOD_PHYREGC(pi, HpFBw, core, maxAnalogGaindb, 100);
		MOD_PHYREGC(pi, DSSScckPktGain, core, dsss_cck_maxAnalogGaindb, 100);

		/* SW RSSI report calculation based on variance (DC is removed) */
		MOD_PHYREGCEE(pi, RxStatPwrOffset, core, use_gainVar_for_rssi, 1);
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, SlnaControl, inv_btcx_prisel_polarity, 1);
		MOD_PHYREG(pi, Core1TxControl, loft_comp_shift, 1);
		MOD_PHYREG(pi, Core2TxControl, loft_comp_shift, 1);
		MOD_PHYREG(pi, Core3TxControl, loft_comp_shift, 1);
		MOD_PHYREG(pi, Core4TxControl, loft_comp_shift, 1);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, Core1TxControl, loft_comp_shift, 1);
		MOD_PHYREG(pi, Core2TxControl, loft_comp_shift, 1);
	}

	/* [4365] need to reduce FSTR threshold by 12dB to resolve
	   the hump around -80dB~-90dBm
	*/
	MOD_PHYREG(pi, FSTRHiPwrTh, finestr_hiPwrSm_th, 65);
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, FSTRHiPwrTh, finestr_hiPwr_th, 57);
	} else {
		MOD_PHYREG(pi, FSTRHiPwrTh, finestr_hiPwr_th, 51);
	}

	// Enable_bcm1_proprietary_256QAM
	MOD_PHYREG(pi, miscSigCtrl, brcm_11n_256qam_support, 0x1);

	// Make this same as TCL
	// 4365 needs to disable bphypredetection,
	// otherwise pktproc stuck at bphy when(AP) 20L is receiving ACK from(STA) 20
	MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 0x0);
	} else {
		MOD_PHYREG(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 0x1);
	}
	MOD_PHYREG(pi, bOverAGParams, bOverAGlog2RhoSqrth, 0x0);

	MOD_PHYREG(pi, CRSMiscellaneousParam, crsInpHold, 1);
	//MOD_PHYREG(pi, RxStatPwrOffset0, use_gainVar_for_rssi0, 1);

	if (ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		// Temporarily disable dac_reset pulse to avoid OOB 8MHz spur
		txbias = 0x0a; rxbias = 0x2c;
	} else {
		txbias = 0x2b; rxbias = 0x28;
	}
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe8, 16, &txbias);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe7, 16, &rxbias);

	if (ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		uint16 val = 0x4;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36e, 16, &val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x37e, 16, &val);
	}

	// Tuned for ofdm PER humps
	WRITE_PHYREG(pi, HTAGCWaitCounters, 0x1020);

	// Linear filter compensation in fine timing:
	//   maps to C-model minkept settings for optimal ofdm sensitivity
	WRITE_PHYREG(pi, FSTRCtrl, 0x7aa);

	// Enable fstr auto-adjust
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, FSTRCtrl_he4, cp3p2AdjOverride, 0);
		MOD_PHYREG(pi, FSTRCtrl_he4, cp1p6AdjOverride, 0);
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr, 1);
		MOD_PHYREG(pi, RxSdFeConfig5, rx_farow_scale_value, 9);
		MOD_PHYREG(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, 1);
		MOD_PHYREG(pi, HeRxBePhsCtrl, HeLtfTrackingEn, 1);
	} else {
		// Tiny specific: reason unknown
		MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr, 0);
		// Tiny specific: tune conversion gain in 20/40
		MOD_PHYREG(pi, RxSdFeConfig5, rx_farow_scale_value, 7);
		// Tiny specific: disable DVG2 to avoid bphy resampler saturation
		// used to avoid 1mbps performance issues due to DC offset. Required
		MOD_PHYREG(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, 0);
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev) &&
	    pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
		wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
	}

	if (!ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev) &&
		!((ACMAJORREV_44_46(pi->pubpi->phy_rev)) &&
		(pi->pubpi->slice == DUALMAC_MAIN))) {
		// Tiny specific: required for 1mbps performance
		MOD_PHYREG(pi, bphyTest, dccomp, 0);

		// Tiny specific: required for 1mbps performance
		MOD_PHYREG(pi, bphyFiltBypass, bphy_tap_20in20path_from_DVGA_en, 1);
	}

	// TXBF related regs
	//  c_param_wlan_bfe_user_index for implicit TXBF
	WRITE_PHYREG(pi, BfeConfigReg1, 0x1f);
	MOD_PHYREG(pi, BfrMuConfigReg0, bfrMu_delta_snr_mode, 2);

	// PHY capability relate regs
	MOD_PHYREG(pi, RfseqCoreActv2059, DisTx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnRx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, CoreConfig, CoreMask, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, CoreConfig, NumRxCores, pi->pubpi->phy_corenum);
	MOD_PHYREG(pi, HTSigTones, support_max_nss, pi->pubpi->phy_corenum);

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		// DCC related regs
		MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 1);
		WRITE_PHYREG(pi, femctrl_override_control_reg, 0);

		// mclip related(sparereg_1_for_div_1x1 = mclip_agc_ENABLED)
		MOD_PHYREG(pi, moved_from_sparereg, sparereg_1_for_div_1x1,
		           phy_ac_rxgcrs_get_cap_mclip_agc_en(pi));
	} else {
		MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 0);
		MOD_PHYREG(pi, moved_from_sparereg, sparereg_1_for_div_1x1, 0);
	}

	if (ISSIM_ENAB(pi->sh->sih)) {
		MOD_PHYREG(pi, clip_detect_normpwr_var_mux, dont_use_clkdiv4en_for_gaindsmpen, 1);
	}

	/* Don't scale ADC_pwr with dvga, as its casuing some
	   clip-failures for 80mhz at high pwrs (> -30dBm)
	*/
	MOD_PHYREG(pi, clip_detect_normpwr_var_mux, en_clip_detect_adc, 1);

	//FIXME43684, FIXME63178
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, SyncPeakCnt, 0x107);

		MOD_PHYREG(pi, Tx11agplcpDelay, plcpdelay11ag, 0x50);
		MOD_PHYREG(pi, TxheplcpDelay, plcpdelayhe, 0x700);

		MOD_PHYREG(pi, cyclicDelayNsts1, enablePreSpatMapCDD_AXTR, 1);
		MOD_PHYREG(pi, cyclicDelayNsts1, enablePreSpatMapCDD_AXRE, 1);
		MOD_PHYREG(pi, cyclicDelayNsts1, enablePreSpatMapCDD_AXMU, 1);
		MOD_PHYREG(pi, cyclicDelayNsts1, enablePreSpatMapCDD_AXSU, 1);

		MOD_PHYREG(pi, RfBiasControl, tx_bg_pulse_val, 0);
		MOD_PHYREG(pi, RfBiasControl, rx_bg_pulse_val, 0);

		WRITE_PHYREG(pi, bfdsConfig1_spatialCoef, 0x380);
		WRITE_PHYREG(pi, bfdsConfig2_spatialCoef, 0x60a);

		WRITE_PHYREG(pi, Rx2Spare, 1);

		/* ENABLE RIFS in 436840, all DV based on RIFS ON */
		MOD_PHYREG(pi, RxControl, RIFSEnable, 1);
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) {
		MOD_PHYREG(pi, Tx11agplcpDelay, plcpdelay11ag, 0x280);

		/* Disable this bit for now (might be responsible for hangs in B0) */
		MOD_PHYREG(pi, demod_low_power, sensitivity_imp_11ax, 0);

		/* Disable lsig length check on 11ax detection path for 43684B0 (vht160 hang) */
		MOD_PHYREG(pi, HEDetectionConfig, lsig_length_min_hedetection, 0);

		MOD_PHYREG(pi, rx1misc_0, rstCmpPwrAftrVht, 0); /* Pulakesh */

		/* Disable rifs for 11ac & 11ax */
		MOD_PHYREG(pi, rx1misc_0, useRifsCntrOnlyVhtHe, 1);
	}

	// Temporary disable 11b/ag txerror check
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		MOD_PHYREG(pi, phytxerrorMaskReg3, miscmask, 0);

		/* Disable rifs for 11ac & 11ax */
		MOD_PHYREG(pi, rx1misc_0, useRifsCntrOnlyVhtHe, 1);
	}

	/* Enable last symbol pilot only phest (only for 20MHz/ru242 HE) */
	if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) {
		MOD_PHYREG(pi, DemodmodeControl, enPhestPilotOnlyForHeLastSymbol, 1);
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* Change IIR filter shape to solve FLATNESS issue for BW160 */
		WRITE_PHYREG(pi, txfilt160st0a1, 0xfa1);
		WRITE_PHYREG(pi, txfilt160st0a2, 0x34f);
		WRITE_PHYREG(pi, txfilt160st1a1, 0xe5c);
		WRITE_PHYREG(pi, txfilt160st1a2, 0x20c);
		WRITE_PHYREG(pi, txfilt160st2a1, 0xc8d);
		WRITE_PHYREG(pi, txfilt160st2a2, 0x10d);
		WRITE_PHYREG(pi, txfilt160finescale, 0xad);
	}
}

static void
wlc_phy_set_reg_on_reset_majorrev1_3(phy_info_t *pi)
{
	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, crsThreshold2l, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2u, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2lSub1, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2uSub1, peakThresh, 77)

		MOD_PHYREG_ENTRY(pi, crsacidetectThreshl, acidetectThresh, 0x80)
		MOD_PHYREG_ENTRY(pi, crsacidetectThreshlSub1, acidetectThresh, 0x80)
		MOD_PHYREG_ENTRY(pi, crsacidetectThreshu, acidetectThresh, 0x80)
		MOD_PHYREG_ENTRY(pi, crsacidetectThreshuSub1, acidetectThresh, 0x80)
		WRITE_PHYREG_ENTRY(pi, initcarrierDetLen,  0x40)
		WRITE_PHYREG_ENTRY(pi, clip1carrierDetLen, 0x5c)
	ACPHY_REG_LIST_EXECUTE(pi);

	if ((CHSPEC_IS2G(pi->radio_chanspec) && BF3_AGC_CFG_2G(pi->u.pi_acphy)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && BF3_AGC_CFG_5G(pi->u.pi_acphy))) {
		WRITE_PHYREG(pi, clip2carrierDetLen, 0x3a);
		WRITE_PHYREG(pi, defer_setClip1_CtrLen, 20);
	} else {
	        WRITE_PHYREG(pi, clip2carrierDetLen, 0x48);
		WRITE_PHYREG(pi, defer_setClip1_CtrLen, 24);
	}
	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, clip_detect_normpwr_var_mux, use_norm_var_for_clip_detect, 0)
		MOD_PHYREG_ENTRY(pi, norm_var_hyst_th_pt8us, cck_gain_pt8us_en, 1)
		MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, mf_crs_initgain_only, 1)
		/* disable bphyacidetEn as it is causing random rxper humps */
		MOD_PHYREG_ENTRY(pi, RxControl, bphyacidetEn, 0)

		WRITE_PHYREG_ENTRY(pi, RfseqCoreActv2059, 0x7717)
	ACPHY_REG_LIST_EXECUTE(pi);

	if (!ACMAJORREV_3(pi->pubpi->phy_rev) &&
		(ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, HTSigTones, 0x9ee1)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0)
			MOD_PHYREG_ENTRY(pi, bOverAGParams, bOverAGlog2RhoSqrth, 0)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 1)
		ACPHY_REG_LIST_EXECUTE(pi);
	} else {
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, HTSigTones, 0x9ee9)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0)
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 0)
			/* digigain is not proper for low power bphy signals */
			/* causes kink near sensitivity region of 11mbps */
			/* fix is to increase cckshiftbitsRefVar by 1.5dB */
			/* WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 46422) */
		ACPHY_REG_LIST_EXECUTE(pi);
		if (IS_4364_1x1(pi)) {
			/* fix is to increase cckshiftbitsRefVar by -1.5dB */
			WRITE_PHYREG(pi, cckshiftbitsRefVar, 0x4075);
		}
	}
	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, FSTRCtrl, fineStrSgiVldCntVal,  0xb)
		MOD_PHYREG_ENTRY(pi, FSTRCtrl, fineStrVldCntVal, 0xa)

		MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs9, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs8, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs7, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs6, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs5, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs4, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs3, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs2, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs1, 0x3)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs0, 0x2)
	ACPHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_phy_set_reg_on_reset_majorrev2_5(phy_info_t *pi)
{
	wlc_phy_set_lowpwr_phy_reg_rev3(pi);

	if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
		if (IS_4364_3x3(pi)) {
			/* fix is to increase cckshiftbitsRefVar by +1.5dB */
			WRITE_PHYREG(pi, cckshiftbitsRefVar, 0x5b0c);
		}
	} else if ((RADIOID(pi->pubpi->radioid) == BCM2069_ID) &&
	    (RADIOREV(pi->pubpi->radiorev) == 0x2C) &&
	    (PHY_XTAL_IS40M(pi))) {
		/* for 43569A2 use chip default setting for cckshiftbitsRefVar */
		ACPHY_REG_LIST_START
			/* Enable BPHY pre-detect */
			MOD_PHYREG_ENTRY(pi, RxControl, preDetOnlyinCS, 1)
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 1)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0)
			MOD_PHYREG_ENTRY(pi, bphyPreDetectThreshold0, ac_det_1us_min_pwr_0,
				350)
			WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 32924)
		ACPHY_REG_LIST_EXECUTE(pi);
	} else {
		ACPHY_REG_LIST_START
			/* Enable BPHY pre-detect */
			MOD_PHYREG_ENTRY(pi, RxControl, preDetOnlyinCS, 1)
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 0)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0)
			MOD_PHYREG_ENTRY(pi, bphyPreDetectThreshold0, ac_det_1us_min_pwr_0, 350)
			WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 46422)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs6, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs5, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs4, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs3, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs2, 0x7)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs1, 0x3)
		MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs0, 0x2)
	ACPHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_phy_set_reg_on_reset_majorrev36(phy_info_t *pi)
{
	uint16 rxbias, txbias;

	ACPHY_REG_LIST_START
		WRITE_PHYREG_ENTRY(pi, HTAGCWaitCounters, 0x2220)
		WRITE_PHYREG_ENTRY(pi, Core0_TargetVar_log2, 0x1c0)
		WRITE_PHYREG_ENTRY(pi, RxSdFeConfig1, 0x1)

		MOD_PHYREG_ENTRY(pi, dccal_control_10, dcoe_abort_threshold, 25)
		MOD_PHYREG_ENTRY(pi, dccal_control_10, idacc_tia_init_00, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_10, idacc_tia_init_01, 9)

		MOD_PHYREG_ENTRY(pi, dccal_control_20, idacc_tia_init_02, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_20, idacc_tia_init_03, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_20, idacc_tia_init_04, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_20, idacc_tia_init_05, 9)

		MOD_PHYREG_ENTRY(pi, dccal_control_30, idacc_tia_init_06, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_30, idacc_tia_init_07, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_30, idacc_tia_init_08, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_30, idacc_tia_init_09, 9)

		MOD_PHYREG_ENTRY(pi, dccal_control_40, idacc_tia_init_10, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_40, idacc_tia_init_11, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_40, idacc_tia_init_12, 9)
		MOD_PHYREG_ENTRY(pi, dccal_control_40, idacc_tia_init_13, 9)

		WRITE_PHYREG_ENTRY(pi, nvcfg2, 0x48f5)
		WRITE_PHYREG_ENTRY(pi, crsmfminpoweru0, 0x3131)
		WRITE_PHYREG_ENTRY(pi, crsminpoweru0, 0x2c31)
		WRITE_PHYREG_ENTRY(pi, crsminpoweru1, 0x2c2c)
		WRITE_PHYREG_ENTRY(pi, crsThreshold1u, 0xa5eb)
		WRITE_PHYREG_ENTRY(pi, crsminpoweroffset0, 0x505)
		WRITE_PHYREG_ENTRY(pi, crsmfminpoweroffset0, 0x505)
		WRITE_PHYREG_ENTRY(pi, Core0HpFBw, 0x3e9f)
		WRITE_PHYREG_ENTRY(pi, Core0DSSScckPktGain, 0x3f00)
		WRITE_PHYREG_ENTRY(pi, BfeConfigReg1, 0x8)

		MOD_PHYREG_ENTRY(pi, RfseqMode, mixer_first_mask_dis, 1)
		MOD_PHYREG_ENTRY(pi, FSTRCtrl, fineStrSgiVldCntVal, 0xa)
		MOD_PHYREG_ENTRY(pi, bOverAGParams, bOverAGlog2RhoSqrth, 0)
		MOD_PHYREG_ENTRY(pi, DcFiltAddress, dc_accum_wait, 3)
		MOD_PHYREG_ENTRY(pi, bphyTest, dccomp, 0)
		MOD_PHYREG_ENTRY(pi, bphyFiltBypass, bphy_tap_20in20path_from_DVGA_en, 1)
		MOD_PHYREG_ENTRY(pi, ultra_low_pwr, bphy_non_beacon_mode, 0x1)
		MOD_PHYREG_ENTRY(pi, forceFront0, spurcan_clk_en_slms0, 0x0)
		MOD_PHYREG_ENTRY(pi, forceFront0, spurcan_clk_en_slms1, 0x0)
		MOD_PHYREG_ENTRY(pi, forceFront0, spurcan_clk_en_fll, 0x0)
		MOD_PHYREG_ENTRY(pi, miscSigCtrl, brcm_11n_256qam_support, 0x1)
		MOD_PHYREG_ENTRY(pi, forceFront0, freqEst, 0)
		MOD_PHYREG_ENTRY(pi, forceFront0, freqCor, 0)
		MOD_PHYREG_ENTRY(pi, femctrl_override_control_reg, femctrl_override_control_reg, 0)
		MOD_PHYREG_ENTRY(pi, crsThreshold2u, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdu, low2highpowThresh, 69)
		MOD_PHYREG_ENTRY(pi, crsminpoweru2, crsminpower4, 0x2c)
		MOD_PHYREG_ENTRY(pi, crsmfminpoweru1, crsmfminpower2, 0x31)
		MOD_PHYREG_ENTRY(pi, crsmfminpoweru1, crsmfminpower3, 0x31)
		MOD_PHYREG_ENTRY(pi, crsmfminpoweru2, crsmfminpower4, 0x31)

		/* Limit TIA index to 5 during packet gain */
		MOD_PHYREG_ENTRY(pi, Core0lpfQ, maxtiagainindx, 0x5)

		/* Bphy PER improvement (SW43012-948) */
		WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 0x209c)
		WRITE_PHYREG_ENTRY(pi, Core0_BPHY_TargetVar_log2_pt8us, 479)
		MOD_PHYREG_ENTRY(pi, overideDigiGain1, cckdigigainEnCntValue, 119)

		/* NB high and low swap */
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, wrssi3_sel_20, 0x1)
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, wrssi3_sel_10, 0x2)
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, wrssi3_sel_00, 0x3)
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, nrssi_sel_20, 0x1)
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, nrssi_sel_10, 0x2)
		MOD_PHYREG_ENTRY(pi, Core0_RSSIMuxSel2, nrssi_sel_00, 0x3)

		/* Reduce clip1 gain settle counter to avoid timing hit */
		WRITE_PHYREG_ENTRY(pi, clip1gainSettleLen, 40)

		/*
			RTT : bundleDacDiodePwrupEn should be 0 for DAC PU
			to take effect from RF Bundle cmds
		*/
		MOD_PHYREG_ENTRY(pi, Logen_AfeDiv_reset_select, bundleDacDiodePwrupEn, 0)
	ACPHY_REG_LIST_EXECUTE(pi);

	if (ACMINORREV_0(pi)) {
		/* Since DC Correction saturation check is missing in 43012A0,
		     Increasing targetvar to 511 causes badfcs
		     Hence keeping targetvar as 479 only for 43012A0
		*/
		WRITE_PHYREG(pi, Core0_BPHY_TargetVar_log2_pt8us, 479);
		WRITE_PHYREG(pi, cckshiftbitsRefVar, 0x209c);
	} else {
		ACPHY_REG_LIST_START
			/* Target var 502 (ie 10db backoff) is good for11mbps hump
			 * reduction compared to 511. Ideally 12db is req as there
			 * is SAT for 10bit after DVG but 5.5senloss with this
			 */
			WRITE_PHYREG_ENTRY(pi, Core0_BPHY_TargetVar_log2_pt8us, 502)
			WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 0x409c)
			MOD_PHYREG_ENTRY(pi, Core0MinMaxGain, maxGainValue, 0x63)
			/* With Predect Enabled, reducing Warmup helps to save some */
			/* 5us (22mhz) time budget */
			MOD_PHYREG_ENTRY(pi, SyncControl, WarmupDurationM1, 1)
			/* Keeping sync wait below 5us effects 1/2mbps across boards */
			MOD_PHYREG_ENTRY(pi, OptionalModes, WaitStateTime, 5)
			WRITE_PHYREG_ENTRY(pi, CCKLMSStepSize, 0x2)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	if (!ISSIM_ENAB(pi->sh->sih)) {
		MOD_PHYREG(pi, Core1TxControl, iqSwapEnable, 1);
	}

	txbias = 0x2b;
	rxbias = 0x28;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe8, 16, &txbias);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe7, 16, &rxbias);

	/* Switch Off ETDAC */
	wlc_phy_radio20695_etdac_pwrdown(pi);

	if (ACMINORREV_0(pi)) {
		/* Force BPHY clock (SW43012-1050) */
		MOD_PHYREG(pi, dacClkCtrl, forcebphyclkon, 1);
	} else {
		uint16 spare_reg;

		/* BPHY predetect bug fix : CRDOT11ACPHY-2073
		    SpareRegB0(8) = 1

		    ADC PU control from bundle : CRDOT11ACPHY-2071
		    Enable ADC PU from bundle SpareRegB0(9) = 1

		    DCC saturation check fix : CRDOT11ACPHY-2060
		    SpareRegB0(3) = 0
		*/
		spare_reg = (READ_PHYREG(pi, SpareRegB0) & 0xfff7) | (1 << 8) | (1 << 9);
		WRITE_PHYREG(pi, SpareRegB0, spare_reg);

		/* LNA protection override logic : CRDOT11ACPHY-1546
		    SpareReg(14) = 1
		*/
		_PHY_REG_MOD(pi, ACPHY_SpareReg(pi->pubpi->phy_rev), (1 << 14), (1 << 14));
	}
}

static void
wlc_phy_set_reg_on_reset_majorrev40_44(phy_info_t *pi)
{
	uint16 temp;
	uint8 core;

	/* 4347 - forcing CoreConfig to 2x2, RTL default is 4x4 */
	WRITE_PHYREG(pi, CoreConfig, 0x43);
	WRITE_PHYREG(pi, RfseqCoreActv2059, 0x3333);

	/* extra_sym_en is always ON in Aux slice */
	/* 11n packets fail when ampdu is enabled;CRDOT11ACPHY-2066 */
	if (wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX) {
		MOD_PHYREG(pi, ldpc_encoder, extra_sym_en, 0);
	}

	if (!ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		/* disable bphy core-remap */
		MOD_PHYREG(pi, HPFBWovrdigictrl, bphyNoCoreRemap, 1);
	}

	/* extra_sym_en is always ON in Aux slice */
	/* 11n packets fail when ampdu is enabled;CRDOT11ACPHY-2066 */

	if (wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX) {
		MOD_PHYREG(pi, ldpc_encoder, extra_sym_en, 0);
	}

	temp = 0x2c;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe7, 16, &temp);
	temp = 0x0a;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe8, 16, &temp);

	temp = 0x4;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36e, 16, &temp);
	temp = 0x4;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x37e, 16, &temp);

	if (!ISSIM_ENAB(pi->sh->sih)) {
		MOD_PHYREG(pi, Core1TxControl, iqSwapEnable, 1);
		MOD_PHYREG(pi, Core2TxControl, iqSwapEnable, 1);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, FFTSoftReset, lbsdadc_clken_ovr, 0)
		MOD_PHYREG_ENTRY(pi, clip_detect_normpwr_var_mux,
			dont_use_clkdiv4en_for_gaindsmpen, 1)
		MOD_PHYREG_ENTRY(pi, clip_detect_normpwr_var_mux, use_extclkdiv4en_for_fsen, 1)
		MOD_PHYREG_ENTRY(pi, clip_detect_normpwr_var_mux, en_clip_detect_adc, 1)
		MOD_PHYREG_ENTRY(pi, SlnaControl, inv_btcx_prisel_polarity, 1)
		MOD_PHYREG_ENTRY(pi, dccal_common, dcc_method_select, 0)
		MOD_PHYREG_ENTRY(pi, radio_pu_seq, dcc_tia_dac_method_select, 1)
	ACPHY_REG_LIST_EXECUTE(pi);

	/* http://jira.broadcom.com/browse/CRDOT11ACPHY-2574 */
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, SlnaControl, EncGain_Tiny_FSMmode3_en, 0);
	}
	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, dccal_control_16, core, idact_bypass, 1);
		MOD_PHYREGCE(pi, dccal_control_16, core, dcoe_bypass, 1);
		MOD_PHYREGCE(pi, dccal_control_14, core, idacc_bypass, 1);
	}
	if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
		wlc_phy_low_rate_adc_enable_acphy(pi, TRUE);
	}
	MOD_PHYREG(pi, CRSMiscellaneousParam, crsInpHold, 1);

	MOD_PHYREG(pi, HTSigTones, support_max_nss, pi->pubpi->phy_corenum);
	//MOD_PHYREG(pi, miscSigCtrl, brcm_11n_256qam_support, 0x1);

	FOREACH_CORE(pi, core) {
		WRITE_PHYREGCE(pi, crshighlowpowThresholdl, core, 0x454b);
		WRITE_PHYREGCE(pi, crshighlowpowThresholdu, core, 0x454b);
		WRITE_PHYREGCE(pi, crshighlowpowThresholdlSub1, core, 0x454b);
		WRITE_PHYREGCE(pi, crshighlowpowThresholduSub1, core, 0x454b);
		MOD_PHYREGCEE(pi, RxStatPwrOffset, core, use_gainVar_for_rssi, 1);
	}
	ACPHY_REG_LIST_START
		/* Doppler related fix in channel update block */
		MOD_PHYREG_ENTRY(pi, ChanestCDDshift, dmd_chupd_use_mod_depend_mu, 1)
		WRITE_PHYREG_ENTRY(pi, chanupsym2, 0x050)
		WRITE_PHYREG_ENTRY(pi, mu_a_mod_ml_4, 0x4400)
		WRITE_PHYREG_ENTRY(pi, mu_a_mod_ml_5, 0x4444)
	ACPHY_REG_LIST_EXECUTE(pi);
	if (CHSPEC_IS80(pi->radio_chanspec)) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, mu_a_mod_zf_4, mu_a_mod_zf_long_4, 0)
			MOD_PHYREG_ENTRY(pi, mu_a_mod_zf_4, mu_a_mod_zf_long_5, 0)
			MOD_PHYREG_ENTRY(pi, mu_a_mod_zf_5, mu_a_mod_zf_extra_long_4, 4)
			MOD_PHYREG_ENTRY(pi, mu_a_mod_zf_5, mu_a_mod_zf_extra_long_5, 4)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, overideDigiGain1, cckdigigainEnCntValue, 119)
		MOD_PHYREG_ENTRY(pi, ADC_PreClip_Enable, small_sig_gain_indx_decr, 0)
		// Fix for 11Mbps PER hump issue in 4361B0 chips
		MOD_PHYREG_ENTRY(pi, bphyPreDetectThreshold5, log2_rho_sqr_1us_th, 0x80)
	ACPHY_REG_LIST_EXECUTE(pi);

	/* 4357B0/B1: to help reduce m4/c4s1 PER hump seen in -70 to -80 dBm RxPwr range */
	WRITE_PHYREG(pi, FSTRHiPwrTh, 0x453f);

	// Write default value of cckshiftbitsRefVar
	WRITE_PHYREG(pi, cckshiftbitsRefVar, CCK_REF_VAR_NOR);

	if (!ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
		/* Enabling bphypredetect timer and setting timeout to 25 microsecs */
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 1)
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutNegEdgEn, 0)
			WRITE_PHYREG_ENTRY(pi, bphy_pre_detection_timeout_interval, 0x3e8)
		ACPHY_REG_LIST_EXECUTE(pi);
	}
	/* Seeing LTE inband jammer degradation with low ifs.
	 * temporarily disabling it
	 */
	MOD_PHYREG(pi, RxControl, RIFSEnable, 0);

	/* Disable core2core sync for 4347 */
	phy_ac_chanmgr_core2core_sync_setup(pi->u.pi_acphy->chanmgri, FALSE);

	/* majrev40: 10*log10((1.4/2^10)^2/50)+30 = -44.27
	 * majrev44: 10*log10((1.2/2^10)^2/50)+30 = -45.61
	 */
	MOD_PHYREG(pi, RxStatPwrOffset0, rx_status_pwr_offset0,
		ACMAJORREV_40(pi->pubpi->phy_rev) ? 44 : 46);
	MOD_PHYREG(pi, RxStatPwrOffset1, rx_status_pwr_offset1,
		ACMAJORREV_40(pi->pubpi->phy_rev) ? 44 : 46);

	/* TIA gain table entry 9 onwards is applicable only for init gain */
	if (ACMAJORREV_44(pi->pubpi->phy_rev) && pi->pubpi->slice == DUALMAC_AUX) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, Core0lpfQ, maxtiagainindx, 0x8)
			MOD_PHYREG_ENTRY(pi, Core1lpfQ, maxtiagainindx, 0x8)
			/* RfseqMode mixer_1st_dis is set to 1, so mixer 1st is not enabled */
			MOD_PHYREG_ENTRY(pi, RfseqMode, mixer_first_mask_dis, 1)
		ACPHY_REG_LIST_EXECUTE(pi);
		FOREACH_CORE(pi, core) {
			MOD_PHYREGC(pi, HpFBw, core, maxAnalogGaindb, 66);
			MOD_PHYREGC(pi, DSSScckPktGain, core, dsss_cck_maxAnalogGaindb, 66);
		}
	}
}

static void
wlc_phy_set_reg_on_reset_majorrev4(phy_info_t *pi)
{
	uint16 phymode = phy_get_phymode(pi);
	uint16 rxbias, txbias;
	uint8 core;

	ACPHY_REG_LIST_START
		WRITE_PHYREG_ENTRY(pi, miscSigCtrl, 0x003)
		MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, crsInpHold, 1)
		MOD_PHYREG_ENTRY(pi, crsThreshold2l, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2u, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2lSub1, peakThresh, 77)
		MOD_PHYREG_ENTRY(pi, crsThreshold2uSub1, peakThresh, 77)

		MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdl, low2highpowThresh, 69)
		MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdu, low2highpowThresh, 69)
		MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdlSub1, low2highpowThresh, 69)
		MOD_PHYREG_ENTRY(pi, crshighlowpowThresholduSub1, low2highpowThresh, 69)

		MOD_PHYREG_ENTRY(pi, BfeConfigReg1, bfe_nvar_comp, 64)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg0, bfe_config_lut_noise_var0, 0)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg1, bfe_config_lut_noise_var1, 0)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg2, bfe_config_lut_noise_var2, 0)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg3, bfe_config_lut_noise_var3, 0)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg4, bfe_config_lut_noise_var4, 0)
		MOD_PHYREG_ENTRY(pi, BfeConfigNvarAdjustTblReg5, bfe_config_lut_noise_var5, 0)

		MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 0)
		MOD_PHYREG_ENTRY(pi, bOverAGParams, bOverAGlog2RhoSqrth, 0)
		MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 1)
		WRITE_PHYREG_ENTRY(pi, cckshiftbitsRefVar, 46422)
		MOD_PHYREG_ENTRY(pi, RxStatPwrOffset0, use_gainVar_for_rssi0, 1)
		WRITE_PHYREG_ENTRY(pi, HTAGCWaitCounters, 0x1020)
		WRITE_PHYREG_ENTRY(pi, FSTRCtrl, 0x7aa)
		MOD_PHYREG_ENTRY(pi, FFTSoftReset, lbsdadc_clken_ovr, 0)
		MOD_PHYREG_ENTRY(pi, RxSdFeConfig5, rx_farow_scale_value, 7)
		MOD_PHYREG_ENTRY(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, 0)
	ACPHY_REG_LIST_EXECUTE(pi);

	txbias = 0x2b;
	rxbias = 0x28;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe8, 16, &txbias);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe7, 16, &rxbias);

	MOD_PHYREG(pi, bphyTest, dccomp, 0);

	MOD_PHYREG(pi, RfseqCoreActv2059, DisTx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnRx, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, CoreConfig, CoreMask, pi->pubpi->phy_coremask);
	MOD_PHYREG(pi, CoreConfig, NumRxCores, pi->pubpi->phy_corenum);
	WRITE_PHYREG(pi, HTSigTones, 0x9ee9);
	MOD_PHYREG(pi, HTSigTones, support_max_nss, pi->pubpi->phy_corenum);
	MOD_PHYREG(pi, bphyFiltBypass, bphy_tap_20in20path_from_DVGA_en, 1);
	WRITE_PHYREG(pi, femctrl_override_control_reg, 0x0);

	FOREACH_CORE(pi, core) {
		MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_offset_comp_pwrup, 1);
		MOD_RADIO_REG_20693(pi, TIA_CFG15, core, tia_offset_comp_pwrup, 1);
	}

	// DCC related regs
	MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 1);

	if (phymode == PHYMODE_MIMO) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, BfeConfigReg1, bfe_config_legacyUserIndex, 0x8)
			MOD_PHYREG_ENTRY(pi, dot11acConfig, bphyPreDetTmOutEn, 1)
			MOD_PHYREG_ENTRY(pi, CRSMiscellaneousParam, crsMfMode, 1)
			WRITE_PHYREG_ENTRY(pi, fineclockgatecontrol, 0x4000)
		ACPHY_REG_LIST_EXECUTE(pi);

		/* disableML if QT and MIMO mode */
		if (ISSIM_ENAB(pi->sh->sih) || !pi->u.pi_acphy->chanmgri->ml_en) {
			MOD_PHYREG(pi, RxControl, MLenable, 0);
		}
	} else if (phymode == PHYMODE_80P80) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdl1, low2highpowThresh, 69)
			MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdu1, low2highpowThresh, 69)
			MOD_PHYREG_ENTRY(pi, crshighlowpowThresholdlSub11, low2highpowThresh, 69)
			MOD_PHYREG_ENTRY(pi, crshighlowpowThresholduSub11, low2highpowThresh, 69)
			MOD_PHYREG_ENTRY(pi, crsThreshold2u1, peakThresh, 77)

			MOD_PHYREG_ENTRY(pi, crsControll1, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControlu1, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControllSub11, mfLessAve, 0)
			MOD_PHYREG_ENTRY(pi, crsControluSub11, mfLessAve, 0)

			WRITE_PHYREG_ENTRY(pi, fineclockgatecontrol, 0x4000)
			MOD_PHYREG_ENTRY(pi, HTSigTones, support_max_nss, 0x1)
		ACPHY_REG_LIST_EXECUTE(pi);
	} else {
		WRITE_PHYREG(pi, fineclockgatecontrol, 0x0);
	}

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, forceFront, core, freqCor, 0);
		MOD_PHYREGCE(pi, forceFront, core, freqEst, 0);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq1, 1)
		MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq2, 0)

		/* RfseqMode mixer_1st_dis is set to 1, so mixer 1st is not enabled */
		MOD_PHYREG_ENTRY(pi, RfseqMode, mixer_first_mask_dis, 1)

		/* Doppler related fix in channel update block */
		MOD_PHYREG_ENTRY(pi, ChanestCDDshift, dmd_chupd_use_mod_depend_mu, 1)
		WRITE_PHYREG_ENTRY(pi, chanupsym2, 0x050)
		WRITE_PHYREG_ENTRY(pi, mu_a_mod_ml_4, 0x4400)
		WRITE_PHYREG_ENTRY(pi, mu_a_mod_ml_5, 0x4444)
		/* Doppler related fix in Bphy LMS update block */
		WRITE_PHYREG_ENTRY(pi, CCKLMSStepSize, 0x1)
	ACPHY_REG_LIST_EXECUTE(pi);

	/* BPHY packet gain settle time . Removes Humps. */
	MOD_PHYREG(pi, overideDigiGain1, cckdigigainEnCntValue, 175);
}

/* Initialize chip regs(RW) that get reset with phy_reset */
static void
wlc_phy_set_reg_on_reset_acphy(phy_info_t *pi)
{
	uint8 core;

	/* IQ Swap (revert swap happening in the radio) */
	if (!(RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) && !(ISSIM_ENAB(pi->sh->sih)) &&
	    !(ACMAJORREV_32(pi->pubpi->phy_rev) ||
	      ACMAJORREV_33(pi->pubpi->phy_rev) ||
	      ACMAJORREV_37(pi->pubpi->phy_rev) ||
	      ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		phy_utils_or_phyreg(pi, ACPHY_RxFeCtrl1(pi->pubpi->phy_rev), 7 <<
			ACPHY_RxFeCtrl1_swap_iq0_SHIFT(pi->pubpi->phy_rev));
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47(pi->pubpi->phy_rev)) {
		if (!ISSIM_ENAB(pi->sh->sih)) {
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 1);
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 1);
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, 1);
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 1);

			MOD_PHYREG(pi, Core1TxControl, iqSwapEnable, 1);
			MOD_PHYREG(pi, Core2TxControl, iqSwapEnable, 1);
			MOD_PHYREG(pi, Core3TxControl, iqSwapEnable, 1);
			MOD_PHYREG(pi, Core4TxControl, iqSwapEnable, 1);
		}
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		if (!ISSIM_ENAB(pi->sh->sih)) {
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 1);
			MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 1);
			MOD_PHYREG(pi, Core1TxControl, iqSwapEnable, 1);
			MOD_PHYREG(pi, Core2TxControl, iqSwapEnable, 1);
		}
	}

	/* Avoid underflow trigger for loopback Farrow */
	MOD_PHYREG(pi, RxFeCtrl1, en_txrx_sdfeFifoReset, 1);

	/* Needed for 4347B0 */
	if (ACMAJORREV_40(pi->pubpi->phy_rev) ||
		ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		/* WAR for CRDOT11ACPHY-2664: keep interlvr clock on */
		/* Needed for 4347B0, Fixed for phy_maj44 */
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, forceClkOn, forceInterlvClkOn, 1);
		}

		/* Needed for phy_maj44_min0 as reset value is 1. Fixed to 0 in phy_maj44_min1 */
		if (ACMAJORREV_44(pi->pubpi->phy_rev) && ACMINORREV_0(pi)) {
			MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr, 0);
		}

		MOD_PHYREG(pi, RX1_Clock_Root_Gating_Control, usePhy1ClkDivAlign, 0);
		wlapi_bmac_mhf(pi->sh->physhim, MHF4, MHF4_NOPHYHANGWAR,
				MHF4_NOPHYHANGWAR, WLC_BAND_AUTO);

		/* Disable GF detect logic, causing issue with -8dBm ACI power */
		MOD_PHYREG(pi, miscSigCtrl, force_1st_sigqual_bpsk, 1);

		/* HW4347BU-177 Update bphy predetect settings */
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, RxControl, preDetOnlyinCS, 1);
			MOD_PHYREG(pi, BphyControl8, dinonRstTglScheme, 1);
		}
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 0);
	} else {
		MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 4);
	}

#ifdef WL_NAP
	/* Enable or disable Napping feature for 43012A0 */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		if (PHY_NAP_ENAB(pi->sh->physhim)) {
			phy_ac_config_napping_28nm_ulp(pi);
		}
	}
#endif /* WL_NAP */

#ifdef WL_MU_RX
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		//FIXME43684
		MOD_PHYREG(pi, miscSigCtrl, mu_enable, 0);
		MOD_PHYREG(pi, miscSigCtrl, check_vht_siga_valid_mu, 1);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		//FIXME63178
		MOD_PHYREG(pi, miscSigCtrl, mu_enable, 0);
		MOD_PHYREG(pi, miscSigCtrl, check_vht_siga_valid_mu, 1);
	} else {
		MOD_PHYREG(pi, miscSigCtrl, mu_enable, 1);
		MOD_PHYREG(pi, miscSigCtrl, check_vht_siga_valid_mu, 0);
	}
#endif /* WL_MU_RX */

	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349 */
		phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

		wlc_phy_set_spareReg_on_reset_majorrev4(pi, stf_shdata->phyrxchain);

		MOD_PHYREG(pi, overideDigiGain1, cckdigigainEnCntValue, 0x6E);
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		/* Write 0x0 to RfseqMode to turn off both CoreActv_override */
		WRITE_PHYREG(pi, RfseqMode, 0);
	}

	if (ACMAJORREV_3(pi->pubpi->phy_rev) &&
		ACREV_GE(pi->pubpi->phy_rev, 20)) {
		  /* 4345C0 : temporarily configure rfseq lines as B1 (CRDOT11ACPHY-1004) */
		  MOD_PHYREG(pi, RxFeTesMmuxCtrl, lpf_gain_biq1_from_rfctrl, 0);
		  MOD_PHYREG(pi, AfePuCtrl, lna1_gain_bits_from_rfctrl, 0);
	}

	/* Enable 6-bit Carrier Sense Match Filter Mode for 4335C0 and 43602A0 */
	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    (ACMAJORREV_2(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ||
	    ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, CRSMiscellaneousParam, crsMfMode, 1);
	}

	/* Retain reset value for 43012A0 */
	if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* Turn on TxCRS extension.
		 * (Need to eventually make the 1.0 be native TxCRSOff (1.0us))
		 */
		WRITE_PHYREG(pi, dot11acphycrsTxExtension, 200);
	}

	/* Currently PA turns on 1us before first DAC sample. Decrease that gap to 0.5us */
	if ((ACMAJORREV_0(pi->pubpi->phy_rev)) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
	        WRITE_PHYREG(pi, TxRealFrameDelay, 146);
	}

	/* Retain reset value for 43012A0 */
	if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* This number combined with MAC RIFS results in 2.0us RIFS air time */
		WRITE_PHYREG(pi, TxRifsFrameDelay, 48);
	}

	si_core_cflags(pi->sh->sih, SICF_MPCLKE, SICF_MPCLKE);
	if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);
	}

	/* allow TSSI loopback path to turn off */
	wlc_phy_set_tssiSleep_on_reset(pi);

	/* In event of high power spurs/interference that causes crs-glitches,
	   stay in WAIT_ENERGY_DROP for 1 clk20 instead of default 1 ms.
	   This way, we get back to CARRIER_SEARCH quickly and will less likely to miss
	   actual packets. PS: this is actually one settings for ACI
	*/
	/* WRITE_PHYREG(pi, ACPHY_energydroptimeoutLen, 0x2); */

	/* Upon Reception of a High Tone/Tx Spur, the default 40MHz MF settings causes ton of
	   glitches. Set the MF settings similar to 20MHz uniformly. Provides Robustness for
	   tones (on-chip, on-platform, accidential loft coming from other devices)
	*/
	wlc_phy_set_mfLessAve_on_reset(pi);

	wlc_phy_set_peakThresh_on_reset(pi);

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* 4365 */
		wlc_phy_set_reg_on_reset_majorrev32_33_37_47_51(pi);
	} else if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* 4335 and 4345 */
		wlc_phy_set_reg_on_reset_majorrev1_3(pi);
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* 4350 and 43602 */
		wlc_phy_set_reg_on_reset_majorrev2_5(pi);
	}

	WRITE_PHYREG(pi, RfseqMode, 0);

	/* Retain reset values for 43012A0 */
	if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* Disable Viterbi cache-hit low power featre for 4360
		 * since it is hard to meet 320 MHz timing
		 */
		MOD_PHYREG(pi, ViterbiControl0, CacheHitEn, ACMAJORREV_0(pi->pubpi->phy_rev)
					? 0 : 1);
	}
	/* WAR: Enable timeout for 4347 for paydecode pktproc hang state */
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, ofdmpaydecodetimeoutlen, 0x7d0)
			/* 2000 * 25 us = 50 ms */
			WRITE_PHYREG_ENTRY(pi, cckpaydecodetimeoutlen, 0x7d0)
			/* 2000 * 25 us = 50 ms */
			WRITE_PHYREG_ENTRY(pi, nonpaydecodetimeoutlen, 0x20)
			/* 32 * 25 us = 800 us */
			WRITE_PHYREG_ENTRY(pi, timeoutEn, 0x817)
			/* ofdmpaydecodetimeoutEn = 1
			* cckpaydecodetimeoutEn = 1
			* nonpaydecodetimeoutEn = 1
			* resetCCAontimeout = 0
			* resetRxontimeout = 1
			*/
		ACPHY_REG_LIST_EXECUTE(pi);
	}
	/* and eventually crashes in reset2rx spin wait. resetcca is getting called as */
	/* part of set channel */
	if (!IS_4364_3x3(pi)) {
		/* Reset pktproc state and force RESET2RX sequence */
		wlc_phy_resetcca_acphy(pi);
	}

	/* Make TSSI to select Q-rail */
	MOD_PHYREG(pi, TSSIMode, tssiADCSel,
		(ACMAJORREV_GE40(pi->pubpi->phy_rev) &&
		pi->u.pi_acphy->sromi->srom_low_adc_rate_en) ? 0 : 1);

	/* Increase this by 10 ticks helps in getting rid of humps at high SNR, single core runs */
	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, defer_setClip2_CtrLen, 16);
	}

	MOD_PHYREG(pi, HTSigTones, support_gf, 0);

	/* Retain reset value for 43012A0 */
	if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
		/* JIRA-CRDOT11ACPHY-273: SIG errror check For number of VHT symbols calculated */
		MOD_PHYREG(pi, partialAIDCountDown, check_vht_siga_length, 1);
	}

	MOD_PHYREG(pi, DmdCtrlConfig, check_vhtsiga_rsvd_bit, 0);

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, forceFront, core, freqCor, 1);
		MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
		/* Use Gain variance for RSSI estimation */
		MOD_PHYREGCEE(pi, RxStatPwrOffset, core, use_gainVar_for_rssi, 1);
	}

#ifdef OCL
	if (TINY_RADIO(pi)) {
		if (PHY_OCL_ENAB(pi->sh->physhim)) {
			uint16 regval = 0x82c0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x15b, 16, &regval);
		}
	}
#endif // endif
	if (ROUTER_4349(pi)) {
		WRITE_PHYREG(pi, pktgainSettleLen, 0x40);
	} else {
		WRITE_PHYREG(pi, pktgainSettleLen, 0x30);
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, CoreConfig, 0x29);
		WRITE_PHYREG(pi, RfseqCoreActv2059, 0x1111);

		if (ACMAJORREV_1(pi->pubpi->phy_rev))
			wlc_phy_set_lowpwr_phy_reg(pi);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev) && (CCT_INIT(pi->u.pi_acphy))) {
		/* Low pwr settings for radio */
		phy_ac_lp_enable(pi);
	}

	/* tkip macdelay & mac holdoff */
	if ((ACMAJORREV_36(pi->pubpi->phy_rev))) {
		MOD_PHYREG(pi, TxMacIfHoldOff, holdoffval, TXMAC_IFHOLDOFF_43012A0);
		MOD_PHYREG(pi, TxMacDelay, macdelay, TXMAC_MACDELAY_43012A0);
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) {
		WRITE_PHYREG(pi, TxMacIfHoldOff, TXMAC_IFHOLDOFF_43684B0);
		WRITE_PHYREG(pi, TxMacDelay, TXMAC_MACDELAY_DEFAULT);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, TxMacIfHoldOff, TXMAC_IFHOLDOFF_63178);
		WRITE_PHYREG(pi, TxMacDelay, TXMAC_MACDELAY_DEFAULT);
	} else if (!ACMAJORREV_0(pi->pubpi->phy_rev) && !ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, TxMacIfHoldOff, TXMAC_IFHOLDOFF_DEFAULT);
		WRITE_PHYREG(pi, TxMacDelay, TXMAC_MACDELAY_DEFAULT);
	}

	/* tiny radio specific processing */
	if (TINY_RADIO(pi)) {
		if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID))
			wlc_phy_set_reg_on_reset_acphy_20691(pi);
		else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID))
			wlc_phy_set_reg_on_reset_acphy_20693(pi);
	}

	wlc_phy_mlua_adjust_acphy(pi, phy_btcx_is_btactive(pi->btcxi));
#ifndef WLC_DISABLE_ACI
	/* Setup HW_ACI block */
	if (!ACPHY_ENABLE_FCBS_HWACI(pi)) {
		if (((pi->sh->interference_mode_2G & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) ||
		    ((pi->sh->interference_mode_5G & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) ||
		    ((((pi->sh->interference_mode_2G & ACPHY_HWACI_MITIGATION) != 0) ||
		     ((pi->sh->interference_mode_5G & ACPHY_HWACI_MITIGATION) != 0)) &&
		     !(ACMAJORREV_32(pi->pubpi->phy_rev) ||
		       ACMAJORREV_33(pi->pubpi->phy_rev) ||
		       ACMAJORREV_37(pi->pubpi->phy_rev) ||
		       ACMAJORREV_47_51(pi->pubpi->phy_rev))))
			wlc_phy_hwaci_setup_acphy(pi, FALSE, TRUE);
		else
			wlc_phy_hwaci_setup_acphy(pi, FALSE, FALSE);
	}
#endif /* !WLC_DISABLE_ACI */

	/* 4335C0: Current optimization */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, FFTSoftReset, 0x2)
			WRITE_PHYREG_ENTRY(pi, fineclockgatecontrol, 0x0)
			WRITE_PHYREG_ENTRY(pi, RxFeTesMmuxCtrl, 0x60)
			MOD_PHYREG_ENTRY(pi, forceFront0, freqEst, 0)
			MOD_PHYREG_ENTRY(pi, forceFront0, freqCor, 0)
			MOD_PHYREG_ENTRY(pi, fineRxclockgatecontrol, forcedigigaingatedClksOn, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	/* 4364_1x1: Current optimization */
	if (IS_4364_1x1(pi)) {
		WRITE_PHYREG(pi, FFTSoftReset, 0x2);
	}

	/* 43602: C-Model Parameters setting */
	if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
			/* Turn ON 11n 256 QAM in 2.4G */
			WRITE_PHYREG_ENTRY(pi, miscSigCtrl, 0x203)
			WRITE_PHYREG_ENTRY(pi, HTAGCWaitCounters, 0x1028)

			/* WRITE_PHYREG_ENTRY(pi, bfeConfigReg1, 0x8) */

			WRITE_PHYREG_ENTRY(pi, crsThreshold2lSub1, 0x204d)
			WRITE_PHYREG_ENTRY(pi, crsThreshold2uSub1, 0x204d)

			/* Fine timing optimization for linear filter */
			WRITE_PHYREG_ENTRY(pi, FSTRCtrl, 0x7aa)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* Low_power settings */
		WRITE_PHYREG(pi, RxFeTesMmuxCtrl, 0x60);
		/* Commenting out this low-power feature. Seen performance hit because of it.  */
		/* FOREACH_CORE(pi, core) { */
		/* 	MOD_PHYREGCE(pi, forceFront, core, freqCor, 0); */
		/* 	MOD_PHYREGCE(pi, forceFront, core, freqEst, 0); */
		/* } */
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		/* Helps for OFDM high-end hump due to W1 clamping */
		/* WRITE_PHYREG(pi, pktgainSettleLen, 0x33); */
		/* WRITE_PHYREG(pi, defer_setClip2_CtrLen, 13); */
		WRITE_PHYREG(pi, dssscckgainSettleLen, 0x65);
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* 43012 */
		wlc_phy_set_reg_on_reset_majorrev36(pi);
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		wlc_phy_set_reg_on_reset_majorrev40_44(pi);
	}

	/* enable fix for bphy loft calibration issue CRDOT11ACPHY-378 */
	if (ACREV_GE(pi->pubpi->phy_rev, 6) &&
	   !((ACMAJORREV_44_46(pi->pubpi->phy_rev)) && (pi->pubpi->slice == DUALMAC_MAIN)))
		MOD_PHYREG(pi, bphyTest, bphyTxfiltTrunc, 0);

	ACPHY_REG_LIST_START
		/* for: http://jira.broadcom.com/browse/SWWFA-10  */
		WRITE_PHYREG_ENTRY(pi, drop20sCtrl1, 0xc07f)

		/* phyrcs20S drop threshold -110 dBm */
		WRITE_PHYREG_ENTRY(pi, drop20sCtrl2, 0x64)
	ACPHY_REG_LIST_EXECUTE(pi);

		/* phyrcs40S drop threshold -110 dBm */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, drop20sCtrl3, phycrs40SpwrTh, 0x64);
	} else {
		WRITE_PHYREG(pi, drop20sCtrl3, 0x64);
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349 */
		wlc_phy_set_reg_on_reset_majorrev4(pi);
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* phy_tx_active_cores {0 1 2 3} */
		MOD_PHYREG(pi, CoreConfig, CoreMask,   pi->pubpi->phy_coremask);
		MOD_PHYREG(pi, CoreConfig, NumRxCores, pi->pubpi->phy_corenum);

		/* RfseqMode mixer_1st_dis is set to 1, so mixer 1st is not enabled */
		MOD_PHYREG(pi, RfseqMode, mixer_first_mask_dis, 1);
	}

	/* 4360 & 43602 & 4365 & 7271 */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
	    ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* Increase timing search timeout to reduce number of glitches under 64k,
		   if glitches > 64k, they are wrapped around to a low number. 12 us --> 13 us
		*/
	        WRITE_PHYREG(pi, timingsearchtimeoutLen, 520);   /* 13 / 0.025 */
	}

	if (IS_4364_3x3(pi) && CHSPEC_IS2G(pi->radio_chanspec))
		MOD_PHYREG(pi, crsThreshold1u, autoThresh, 240);

	if (ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, Logen_AfeDiv_reset_select, rst2rx_afediv_arst_val, 0);
	}

#ifdef WL_MU_RX
	if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* MU-MIMO STA related stuff */
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			//FIXME43684, FIXME63178
			MOD_PHYREG(pi, mu_a_mumimo_ltrn_dependent,
				mu_a_mumimo_ltrn_dependent_log2_1, 0x6);
			MOD_PHYREG(pi, mu_a_mumimo_ltrn_dependent,
				mu_a_mumimo_ltrn_dependent_log2_0, 0x7);
		} else {
			MOD_PHYREG(pi, mu_a_mumimo_ltrn_dependent,
				mu_a_mumimo_ltrn_dependent_log2_1, 0x0);
			MOD_PHYREG(pi, mu_a_mumimo_ltrn_dependent,
				mu_a_mumimo_ltrn_dependent_log2_0, 0x0);
		}
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, miscSigCtrl, mu_enable, 1)
			MOD_PHYREG_ENTRY(pi, miscSigCtrl, check_vht_siga_valid_mu, 0)
			MOD_PHYREG_ENTRY(pi, MuSumThreshold4x4_0, IntSumThresh2, 0x80)
			MOD_PHYREG_ENTRY(pi, MuMimoStaCtrl_4x4,
				mumimo_4x4_dummystreamdetect_enable, 0x1)
			MOD_PHYREG_ENTRY(pi, MuMimoStaCtrl_4x4, mumimo_4x4_all_stream_equalize, 0x1)
			MOD_PHYREG_ENTRY(pi, MuMimoStaCtrl_4x4, mumimo_skip_datasc_for_phest, 0x1)
		ACPHY_REG_LIST_EXECUTE(pi);
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			//FIXME43684, FIXME63178
			MOD_PHYREG(pi, miscSigCtrl, mu_enable, 0);
			MOD_PHYREG(pi, miscSigCtrl, check_vht_siga_valid_mu, 1);
		}
	}
#endif /* WL_MU_RX */

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		wlc_phy_set_hesiga_sigb_pos(pi);

		/* http://jira.broadcom.com/browse/CRDOT11ACPHY-2982
		 * Pwrctrl clk enable is dependent on core0 adc pwr up for aux core
		 * Clock enable to Pwr ctrl clock depends on core 0 adc pwr_up for aux core.
		 * Following is the code for clock enable:
		 * txlbClk2en <= (txbeclkEn and (adc_pu(0) or phyregs2clkrst.forcetxpwrctrlclkon));
		 * Setting dacClkCtrl.forcetxpwrctrlclkon register to 1 will make sure that pwrctrl
		 *  clk is propagating.
		 * ored signal of pwr_up of all cores should be used in the logic.
		 */
		if (ACMINORREV_0(pi)) {
			if (pi->pubpi->slice == DUALMAC_AUX) {
				MOD_PHYREG(pi, dacClkCtrl, forcetxpwrctrlclkon, 0x1);
			} else {
				MOD_PHYREG(pi, dacClkCtrl, forcetxpwrctrlclkon, 0x0);
			}
		}

		/* algn248ToClkDiv4 has to be set for phy_maj44 as per PHY-RTL team
		 * This bit is used to align the clk248 datapath to clkdiv4en
		 */
		MOD_PHYREG(pi, RX1_Clock_Root_Gating_Control, algn248ToClkDiv4, 1);

		/* HW-1518: logen core-1 pu is forced to 1 (WAR for CR28NMWLANRF-354) */
		if (HW_ACMINORREV(pi) == 0 && pi->pubpi->slice == DUALMAC_MAIN) {
			MOD_PHYREG(pi, Logen_percore_pu, logen_core1_pu_val, 1);
			MOD_PHYREG(pi, Logen_percore_pu, logen_core1_pu_ovr, 1);
		}
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, phase_word_3p2_hi, upconv_11ax_forceClkOn, 1);

		/* Initialize MUMIMO related phyreg.
		 * 4366 initialize them in VASIP firmware
		 */
		WRITE_PHYREG(pi, m2v_svmpaddr0, 0x4008);
		WRITE_PHYREG(pi, m2v_svmpaddr1, 0x4010);
		WRITE_PHYREG(pi, bfdrBaseAddr, 0x1000);
		WRITE_PHYREG(pi, bfdrCqiBaseAddr, 0xe00);
		WRITE_PHYREG(pi, bfdsLogAddr, 0x4028);
		WRITE_PHYREG(pi, bfdsMlbfBaseAddr, 0x4068);
		WRITE_PHYREG(pi, vasip_wdog_timeout_H, 0x40);

		/* Enable MRC SIG QUAL for 43684B0 */
		if (!ACMINORREV_0(pi))
			MOD_PHYREG(pi, MrcSigQualControl0, enableMrcSigQual, 0x1);
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, bfdsCmdSeq, use_bfd_spexp, 1);
		MOD_PHYREG(pi, dacClkCtrl, vasipClkEn, 1);
	}

	// disable HeSigA check, enable only BW check; FWUCODE-3404
	if (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		WRITE_PHYREG(pi, Config1_MAC_aided_trig_frame, 0x3);
	}

	// change the CSD for 2SS case to [0 3 1 5]
	if (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		WRITE_PHYREG(pi, bfdsConfig3_spatialCoef, 0xC0);
		WRITE_PHYREG(pi, bfdsConfig4_spatialCoef, 0x141);
		WRITE_PHYREG(pi, bfdsConfig5_spatialCoef, 0xC0);
		WRITE_PHYREG(pi, bfdsConfig6_spatialCoef, 0x141);
	}
}

/* Initialize chip tbls(reg-based) that get reset with phy_reset */
static void
wlc_phy_set_tbl_on_reset_acphy(phy_info_t *pi)
{
	uint8 stall_val, ctr;
	phy_info_acphy_t *pi_ac;
	phy_ac_radio_data_t *rdata;
	uint16 adc_war_val = 0x20, pablowup_war_val = 120;
	uint8 core;
	uint16 gmult20, gmult40, gmult80;
	uint16 rfseq_bundle_tssisleep48[3];
	uint16 rfseq_bundle_48[3];
	const void *data, *dly;
	uint32 read_val[2], write_val[2];
	uint16 x, temp_data;
	uint8 zeros[ZEROS_TBL_SZ * 4];
	uint16 rfseq_lpf_pu_val = 0x500e, rfseq_lpf_pu_addr[] = {0x14d, 0x15d, 0x16d, 0x48d};
	uint16 rfseq_dacdiode_val = 0x49, rfseq_dacdiode_addr[] = {0x3cb, 0x3db, 0x3eb, 0x56b};

	/* uint16 AFEdiv_read_val = 0x0000; */

	bool ext_pa_ana_2g =  ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_SROM11_ANAPACTRL_2G) != 0);
	bool ext_pa_ana_5g =  ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_SROM11_ANAPACTRL_5G) != 0);

	/* DEBUG: TEST CODE FOR SS PTW70 DEBUG */
	uint32 war_val = 0x7ffffff;
	uint8 offset;
	uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	BCM_REFERENCE(phyrxchain);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	pi_ac = pi->u.pi_acphy;
	rdata = phy_ac_radio_get_data(pi_ac->radioi);
	/* Load settings related to antswctrl if not on QT */
	if (!ISSIM_ENAB(pi->sh->sih)) {
		wlc_phy_set_regtbl_on_femctrl(pi);
	}
	/* Quickturn only init */
	if (ISSIM_ENAB(pi->sh->sih)) {
		uint8 ncore_idx;
		uint16 val;
		uint16 init_gain_code_A = 0x16a, init_gain_code_B = 0x24;
		uint16 rfseq_val1 = 0x124d, rfseq_val2 = 0x62;

		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			val = 64;
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			val = 50;
			init_gain_code_A = 0x49a;
			init_gain_code_B = 0x2044;

			if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
				MOD_PHYREG(pi, DcFiltAddress, dcBypass, 1);
			}
		} else {
		   /* changing to TCL value */
		   val = 50;
		   if (CHSPEC_IS20(pi->radio_chanspec)) {
			  MOD_PHYREG(pi, DcFiltAddress, dcBypass, 1);
		   }

		}

		FOREACH_CORE(pi, ncore_idx) {
			wlc_phy_set_tx_bbmult_acphy(pi, &val, ncore_idx);
		}

		/* dummy call to satisfy compiler */
		wlc_phy_get_tx_bbmult_acphy(pi, &val, 0);

		/* on QT: force the init gain to allow noise_var not limiting 256QAM performance */
		ACPHYREG_BCAST(pi, Core0InitGainCodeA, init_gain_code_A);
		ACPHYREG_BCAST(pi, Core0InitGainCodeB, init_gain_code_B);

		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				0xf9, 16, &rfseq_val1);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
				0xf6, 16, &rfseq_val2);
		} else {
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {
				if (core == 3) {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
						0x509, 16, &qt_rfseq_val1[core]);
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
						0x506, 16, &qt_rfseq_val2[core]);
				} else {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
						0xf9 + core, 16, &qt_rfseq_val1[core]);
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
						0xf6 + core, 16, &qt_rfseq_val2[core]);
				}
			}
		}
	}

	/* Update gmult, dacbuf after radio init */
	/* Tx Filters */
	if (!(ACMAJORREV_37(pi->pubpi->phy_rev) ||
	      ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		/* 7271A0 has these not connected via RFSEQ */
		wlc_phy_set_analog_tx_lpf(pi, 0x1ff, -1, -1, -1, rdata->rccal_gmult,
		                          rdata->rccal_gmult_rc, -1);
		wlc_phy_set_tx_afe_dacbuf_cap(pi, 0x1ff, rdata->rccal_dacbuf, -1, -1);
	}

	/* Rx Filters */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* 4360 (tighten rx analog filters). Note than 80mhz filter cutoff
		   was speced at 39mhz (should have been 38.5)
		   C-model desired bw : {9, 18.5, 38.5}  @ 3dB cutoff
		   lab-desired (freq offset + 5%PVT): {9.5, 20, 41}
		   with gmult = 193 (in 2069_procs.tcl), we get {11, 23.9, 48.857}
		   Reduce bw by factor : {9.5/11, 20/23.9, 41/48.857} = {0.863, 0.837, 0.839}
		*/
		gmult20 = (rdata->rccal_gmult * 221) >> 8;     /* gmult * 0.863 */
		gmult40 = (rdata->rccal_gmult * 215) >> 8;     /* gmult * 0.839 (~ 0.837) */
		gmult80 = (rdata->rccal_gmult * 215) >> 8;     /* gmult * 0.839 */
	} else if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		/* 4335C0 (tighten rx analog filter for 80mhz only).
		   This is needed to take away
		   hump which comes because of ACI causing false clip_hi
		*/
		gmult20 = rdata->rccal_gmult;
		gmult40 = rdata->rccal_gmult;
		if (!(PHY_ILNA(pi))) {
			gmult80 = (rdata->rccal_gmult * 225) >> 8;     /* gmult * 0.879 */
		} else {
			gmult80 = rdata->rccal_gmult;
		}
	} else {
		gmult20 = rdata->rccal_gmult;
		gmult40 = rdata->rccal_gmult;
		gmult80 = rdata->rccal_gmult;
	}
	if (!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_set_analog_rx_lpf(pi, 1, -1, -1, -1, gmult20,
			rdata->rccal_gmult_rc, -1);
	}
	/* 43012: No need to progam for 40 and 80 MHz */
	if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
		wlc_phy_set_analog_rx_lpf(pi, 2, -1, -1, -1, gmult40, rdata->rccal_gmult_rc, -1);
		wlc_phy_set_analog_rx_lpf(pi, 4, -1, -1, -1, gmult80, rdata->rccal_gmult_rc, -1);
	}

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		wlc_phy_set_afediv_reset_bundles(pi);
	}

	/* Reset2rx sequence */
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		data = rfseq_majrev4_reset2rx_cmd;
		dly = rfseq_majrev4_reset2rx_dly;
	} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		data = rfseq_reset2rx_cmd;
		dly = rfseq_majrev3_reset2rx_dly;
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		data = rfseq_majrev32_reset2rx_cmd;
		dly = rfseq_majrev32_reset2rx_dly;
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		data = rfseq_majrev36_reset2rx_cmd;
		dly = rfseq_majrev36_reset2rx_dly;
	} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		data = rfseq_majrev37_reset2rx_cmd;
		dly = rfseq_majrev37_reset2rx_dly;
	} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		data = rfseq_majrev40_reset2rx_cmd;
		dly = rfseq_majrev40_reset2rx_dly;
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		data = get_rfseq_majrev44(pi, RESET2RX_SEQ_CMD_TBL);
		dly = get_rfseq_majrev44(pi,  RESET2RX_SEQ_DLY_TBL);
	} else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		data = rfseq_majrev47_reset2rx_cmd;
		dly = rfseq_majrev47_reset2rx_dly;
	} else {
		data = rfseq_reset2rx_cmd;
		dly = rfseq_reset2rx_dly;
	}
	if (data && dly) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x20, 16, data);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x90, 16, dly);
	}

	if (!(ACMAJORREV_32(pi->pubpi->phy_rev)) &&
	    !(ACMAJORREV_33(pi->pubpi->phy_rev)) &&
	    !(ACMAJORREV_GE36(pi->pubpi->phy_rev))) {
		/* during updateGainL make sure the lpf/tia hpc corner is set
			properly to optimum setting
		*/
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 2, 0x121, 16,
				rfseq_updl_lpf_hpc_ml);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 2, 0x131, 16,
				rfseq_updl_lpf_hpc_ml);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 2, 0x124, 16,
				rfseq_updl_tia_hpc_ml);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 2, 0x137, 16,
				rfseq_updl_tia_hpc_ml);
	}

	/* tx2rx/rx2tx: Remove SELECT_RFPLL_AFE_CLKDIV/RESUME as we are not in boost mode */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) || (ACMAJORREV_2(pi->pubpi->phy_rev) && PHY_IPA(pi))) {
		if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			(((CHSPEC_IS20(pi->radio_chanspec)) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x1)) ||
			((CHSPEC_IS40(pi->radio_chanspec)) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x2)))) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			(((CHSPEC_IS20(pi->radio_chanspec)) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x4)) ||
			((CHSPEC_IS40(pi->radio_chanspec)) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x8)) ||
			((CHSPEC_IS80(pi->radio_chanspec)) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x10))))) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
					16, rfseq_rx2tx_cmd_withtssisleep);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
					16, rfseq_rx2tx_dly_withtssisleep);
				MOD_PHYREG(pi, RfBiasControl, tssi_sleep_bg_pulse_val, 1);
				MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
				rfseq_bundle_tssisleep48[0] = 0x0000;
				rfseq_bundle_tssisleep48[1] = 0x20;
				rfseq_bundle_tssisleep48[2] = 0x0;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 0, 48,
					rfseq_bundle_tssisleep48);
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, rfseq_rx2tx_cmd);
		}
	} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
		                          tiny_rfseq_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
		                          tiny_rfseq_rx2tx_dly);
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
		                          tiny_rfseq_rx2tx_tssi_sleep_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 112, 16,
		                          tiny_rfseq_rx2tx_tssi_sleep_dly);
		MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00, 16,
		                          rfseq_majrev32_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
		                          rfseq_majrev32_rx2tx_dly);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		const uint16 rx_adc = 0xE0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x0, 16,
		                          rfseq_majrev36_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
		                          rfseq_majrev36_rx2tx_dly);
		if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x1)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x4))) {

			MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 1);

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x0, 16,
				rfseq_majrev36_rx2tx_tssi_sleep_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70, 16,
				rfseq_majrev36_rx2tx_tssi_sleep_dly);
		}
		/* make clkgen pu , ref pu, pu cmbuf to 1 for rx2tx mode. */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x145, 16,
			&rx_adc);
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && !(PHY_IPA(pi))) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			16, rfseq_rx2tx_cmd_noafeclkswitch);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			16, rfseq_rx2tx_cmd_noafeclkswitch_dly);
	}  else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			16, rfseq_majrev37_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			16, rfseq_majrev37_rx2tx_dly);
	}  else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			16, rfseq_majrev47_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			16, rfseq_majrev47_rx2tx_dly);
	}  else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			16, rfseq_majrev40_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			16, rfseq_majrev40_rx2tx_dly);
	}  else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x1)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) &&
			(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x4))) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, get_rfseq_majrev44(pi, RX2TX_TSSISLEEPEN_SEQ_CMD_TBL));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
				16, get_rfseq_majrev44(pi, RX2TX_TSSISLEEPEN_SEQ_DLY_TBL));
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				16, get_rfseq_majrev44(pi, RX2TX_SEQ_CMD_TBL));
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
				16, get_rfseq_majrev44(pi, RX2TX_SEQ_DLY_TBL));
		}
		if (pi->pubpi->slice == DUALMAC_AUX) {
			/*
			* RX2TX -- ENT
			* rfseq2rfctrlint.rxadc_pu(i)<= rfseqreg.rx2tx_lpf_ctl_lut_entx(i)
			* (40 downto 36)
			* RX2TX
			* rx2tx_lpf_ctl_lut_entx(1)(40:33)-15downto8|rx2tx_lpf_ctl_lut_entx(0)
			* (40:33)
			* -- 7downto0
			* for programming rxadc_pu_refbuf, adc_ref_cmbuf_pu,
			* afe_pwr_switch_pu
			*/
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x58D,
					16, &temp_data);
			temp_data = temp_data | 0x6060;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x58D,
					16, &temp_data);
			/*
			 * RX2TX -- DISTX
			 * For disabled core
			 * rfseq2rfctrlint.rxadc_pu(i) <= rfseqreg.rx2tx_lpf_ctl_lut_distx(i)
			 * (31 downto 27)
			 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36C,
					16, &temp_data);
			temp_data = temp_data | 0x3000;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36C,
					16, &temp_data);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x37C,
					16, &temp_data);
			temp_data = temp_data | 0x3000;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x37C,
					16, &temp_data);
			/*
			 * TX2RX | RESET2RX | RESETCCA -- ENRX
			 * rfseq2rfctrlint.rxadc_pu(i)<= rfseqreg.tx2rx_lpf_ctl_lut_enrx(i)
			 * (43 downto 39)
			 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x360,
					16, &temp_data);
			temp_data = temp_data | 0x00C0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x360,
					16, &temp_data);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x370,
					16, &temp_data);
			temp_data = temp_data | 0x00C0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x370,
					16, &temp_data);
			/*
			 * For disabled core -- DISRX
			 * rfseq2rfctrlint.rxadc_pu(i)<= rfseqreg.tx2rx_lpf_ctl_lut_disrx(i)
			 * (44 downto 40)
			 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x361,
					16, &temp_data);
			temp_data = temp_data | 0x00C0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x361,
					16, &temp_data);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x371,
					16, &temp_data);
			temp_data = temp_data | 0x00C0;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x371,
					16, &temp_data);
		}
	} else {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			16, rfseq_rx2tx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			16, rfseq_rx2tx_dly);
	}

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* Overwrite delay before RX_DC_LOOP_CONT */
		tiny_rfseq_tx2rx_dly[14] = 0x0020;

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          tiny_rfseq_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 128, 16,
		                          tiny_rfseq_tx2rx_dly);
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          rfseq_majrev4_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 128, 16,
		                          rfseq_majrev4_tx2rx_dly);
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          rfseq_majrev32_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
		                          rfseq_majrev32_tx2rx_dly);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          rfseq_majrev36_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
		                          rfseq_majrev36_tx2rx_dly);

		rfseq_bundle_48[0] = 0x6000;
		rfseq_bundle_48[1] = 0x0;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 4, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0000;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 5, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x4000;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 6, 48,
		                          rfseq_bundle_48);
	} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          rfseq_majrev37_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
		                          rfseq_majrev37_tx2rx_dly);
	} else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* FIXME63178: lifting with 43684 until we have to differ */
		/* set rfseq_bundle_tbl {0x01C703C 0x01C7014 0 0} */
		/* FIXME43684: for second PLL we should probably use 0x02C700E 0x02C702C
		   for 0x82 and 0x83. acphy_write_table RfseqBundle $rfseq_bundle_tbl 0
		*/

		MOD_PHYREG(pi, RfctrlCmd, bundleScheme2, rfseq_majrev47_bundleScheme2);

		rfseq_bundle_48[0] = 0x703C;
		rfseq_bundle_48[1] = 0x1c;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 0, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x7014;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 1, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0;
		rfseq_bundle_48[1] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 2, 48,
		                          rfseq_bundle_48);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 3, 48,
		                          rfseq_bundle_48);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 4, 48,
		                          rfseq_bundle_48);
		/* in low rate adc mode, add bundle cmd 0x85 to reset rxfarrow at
		 * the beginning of tx2rx sequence, add cmd 0x86 to release reset
		 * at the end of tx2rx sequence
		 */
		rfseq_bundle_48[0] = 0x400;
		rfseq_bundle_48[1] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 5, 48,
				rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 6, 48,
				rfseq_bundle_48);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
		                          rfseq_majrev47_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
		                          rfseq_majrev47_tx2rx_dly);

		bzero(zeros, sizeof(zeros));
		for (x = 0; x < 512; x += ZEROS_TBL_SZ) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVNOISESHAPINGTBL,
			                          ZEROS_TBL_SZ, x, 32, zeros);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVNOISESHAPING11AXTBL,
			                          ZEROS_TBL_SZ, x, 32, zeros);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVRXEVMSHAPINGTBL,
			                          ZEROS_TBL_SZ, x, 8, zeros);
		}

		for (x = 0; x < 256; x += ZEROS_TBL_SZ) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVADJ11AXTBL,
			                          ZEROS_TBL_SZ, x, 32, zeros);
		}

		if (RADIOREV(pi->pubpi->radiorev) == 0) {
			/* update mclip clip2 table */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL0, 6, 0x1, 32,
					clip2_tbl_maj47);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL1, 6, 0x1, 32,
					clip2_tbl_maj47);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL2, 6, 0x1, 32,
					clip2_tbl_maj47);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL3, 6, 0x1, 32,
					clip2_tbl_maj47);
		} else {
			/* update mclip clip2 table */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL0, 6, 0x1, 32,
					clip2_tbl_maj47_rX);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL1, 6, 0x1, 32,
					clip2_tbl_maj47_rX);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL2, 6, 0x1, 32,
					clip2_tbl_maj47_rX);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL3, 6, 0x1, 32,
					clip2_tbl_maj47_rX);
		}

	} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		/* set rfseq_bundle_tbl {0x01C703C 0x01C7014 0x02C703E 0x02C701C} */
		/* acphy_write_table RfseqBundle $rfseq_bundle_tbl 0 */
		rfseq_bundle_48[0] = 0x703C;
		rfseq_bundle_48[1] = 0x1c;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 0, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x7014;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 1, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x700e;
		rfseq_bundle_48[1] = 0x2c;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 2, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x702c;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 3, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x6020;
		rfseq_bundle_48[1] = 0x20;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 4, 48,
		                          rfseq_bundle_48);
		/* in low rate adc mode, add bundle cmd 0x85 to reset rxfarrow at
		 * the beginning of tx2rx sequence, add cmd 0x86 to release reset
		 * at the end of tx2rx sequence
		 */
		rfseq_bundle_48[0] = 0x400;
		rfseq_bundle_48[1] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 5, 48,
				rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 6, 48,
				rfseq_bundle_48);
		if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
					rfseq_majrev40_tx2rx_cmd);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
					rfseq_majrev40_tx2rx_dly);
		}
		/* update mclip clip2 table */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL0, 6, 0x1, 32, clip2_tbl);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL1, 6, 0x1, 32, clip2_tbl);

	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		/* in low rate adc mode, add bundle cmd 0x85 to reset rxfarrow at
		 * the beginning of tx2rx sequence, add cmd 0x86 to release reset
		 * at the end of tx2rx sequence
		 */
		/* using same commands for rx mode adc rate also as farrow reset should not harm */
		/* using bundle rfctrl_bundle_global_28nm for farrow reset */
		rfseq_bundle_48[0] = 0x400;
		rfseq_bundle_48[1] = 0x0;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 5, 48,
			rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 6, 48,
			rfseq_bundle_48);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16,
			get_rfseq_majrev44(pi, TX2RX_SEQ_CMD_TBL));
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16,
			get_rfseq_majrev44(pi, TX2RX_SEQ_DLY_TBL));
		/* update mclip clip2 table */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL0, 6, 0x1, 32, clip2_tbl);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_MCLPAGCCLIP2TBL1, 6, 0x1, 32, clip2_tbl);

	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) && !(PHY_IPA(pi))) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10,
			16, rfseq_tx2rx_cmd_noafeclkswitch);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80,
			16, rfseq_tx2rx_dly_noafeclkswitch);
	} else {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x10, 16, rfseq_tx2rx_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x80, 16, rfseq_tx2rx_dly);
	}

	/* This was to keep the adc-clock buffer powered up even if adc is powered down
	   for non-tiny radio. But for tiny radio this is not required.
	*/
	if (!(ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	      ACMAJORREV_33(pi->pubpi->phy_rev) ||
	      ACMAJORREV_GE36(pi->pubpi->phy_rev))) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c6, 16, &adc_war_val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c7, 16, &adc_war_val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3d6, 16, &adc_war_val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3d7, 16, &adc_war_val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3e6, 16, &adc_war_val);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3e7, 16, &adc_war_val);
	}

	/* do this during fem table load for 43602a0 */
	if (((CHSPEC_IS2G(pi->radio_chanspec) && ext_pa_ana_2g) ||
	    (CHSPEC_IS5G(pi->radio_chanspec) && ext_pa_ana_5g)) &&
	    !(ACMAJORREV_5(pi->pubpi->phy_rev) && ACMINORREV_0(pi))) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x80, 16, &pablowup_war_val);
	}

	/* 4360 and 43602 */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* load the txv for spatial expansion */
		acphy_load_txv_for_spexp(pi);
	}

	if ((RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) &&
	    ((RADIO2069_MAJORREV(pi->pubpi->radiorev) > 0) ||
			((RADIO2069_MINORREV(pi->pubpi->radiorev) == 16) ||
			(RADIO2069_MINORREV(pi->pubpi->radiorev) == 17)))) {
		/* 11n_20 */
		wlc_phy_set_analog_tx_lpf(pi, 0x2, -1, 5, 5, -1, -1, -1);
		/* 11ag_11ac_20 */
		wlc_phy_set_analog_tx_lpf(pi, 0x4, -1, 5, 5, -1, -1, -1);
		/* 11n_40 */
		wlc_phy_set_analog_tx_lpf(pi, 0x10, -1, 5, 5, -1, -1, -1);
		/* 11ag_11ac_40 */
		wlc_phy_set_analog_tx_lpf(pi, 0x20, -1, 5, 5, -1, -1, -1);
		/* 11n_11ag_11ac_80 */
		wlc_phy_set_analog_tx_lpf(pi, 0x80, -1, 6, 6, -1, -1, -1);
	} else if (RADIOID(pi->pubpi->radioid) == BCM20696_ID) {
		/* "11b_20" "11n_20" "11ag_11ac_20" "samp_play" */
		wlc_phy_set_analog_tx_lpf(pi, 0x107, 0, 4, 4, -1, -1, -1);
		/* "11b_40" "11n_40" "11ag_11ac_40" "samp_play" */
		wlc_phy_set_analog_tx_lpf(pi, 0x138, 0, 5, 5, -1, -1, -1);
		/* "11b_80" "11n_11ag_11ac_80" "samp_play" */
		wlc_phy_set_analog_tx_lpf(pi, 0x1c0, 0, 6, 6, -1, -1, -1);
	} else if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		/* 11b_20 11n_20 11ag_11ac_20 */
		wlc_phy_set_analog_tx_lpf(pi, 0x7, -1, 4, -1, -1, -1, -1);
		/* 11n_40 11ag_11ac_40 */
		wlc_phy_set_analog_tx_lpf(pi, 0x30, -1, 5, -1, -1, -1, -1);
		/* 11n_11ag_11ac_80 */
		wlc_phy_set_analog_tx_lpf(pi, 0x80, -1, 6, -1, -1, -1, -1);
	}

	/* tiny radio specific processing */
	if (TINY_RADIO(pi)) {
		uint16 regval;
		const uint32 NvAdjTbl[64] = { 0x000000, 0x400844, 0x300633, 0x200422,
			0x100211, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000100,
			0x000200, 0x000311, 0x000422, 0x100533, 0x200644, 0x300700,
			0x400800, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x400800, 0x300700, 0x200644, 0x100533,
			0x000422, 0x000311, 0x000200, 0x000100, 0x000000, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
			0x000000, 0x000000, 0x100211, 0x200422, 0x300633, 0x400844};

		const uint32 phasetracktbl[22] = { 0x06af56cd, 0x059acc7b,
			0x04ce6652, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819,
			0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x06af56cd,
			0x059acc7b, 0x04ce6652, 0x02b15819, 0x02b15819, 0x02b15819,
			0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819, 0x02b15819};

		/* Tiny NvAdjTbl */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVADJTBL, 64, 0, 32, NvAdjTbl);

		if (!(ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev))) {
			/* Tiny phasetrack */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PHASETRACKTBL_1X1, 22, 0, 32,
			phasetracktbl);
		}

		/* Channels Smoothing */
		if ((!ACMINORREV_0(pi) || ACMAJORREV_4(pi->pubpi->phy_rev)) &&
		   (!ACMAJORREV_32(pi->pubpi->phy_rev)) &&
		   (!ACMAJORREV_33(pi->pubpi->phy_rev)))
			wlc_phy_load_channel_smoothing_tiny(pi);

		/* program tx, rx bias reset to avoid clock stalls */
		regval = 0x2b;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe8, 16, &regval);
		regval = 0x28;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe7, 16, &regval);

		/* #Keep lpf_pu @ 0 for rx since lpf_pu controls tx lpf exclusively */
		regval = 0x82c0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x14b, 16, &regval);

		/* Magic rfseqbundle writes to make TX->Rx turnaround work */
		/* set rfseq_bundle_tbl {0x4000 0x0000 } */
		/* acphy_write_table RfseqBundle $rfseq_bundle_tbl 4 */
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			rfseq_bundle_48[0] = 0x6000;
		} else {
			rfseq_bundle_48[0] = 0x4000;
		}
		rfseq_bundle_48[1] = 0x0;
		rfseq_bundle_48[2] = 0x0;

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 4, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0000;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 5, 48,
		                          rfseq_bundle_48);

		/* set rfseq_bundle_tbl {0x3000C 0x20000 0x30034 0x20000} */
		/* acphy_write_table RfseqBundle $rfseq_bundle_tbl 0 */
		rfseq_bundle_48[0] = 0x0000;
		rfseq_bundle_48[1] = 0x2;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 0, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0034;
		rfseq_bundle_48[1] = 0x3;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 1, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x0000;
		rfseq_bundle_48[1] = 0x2;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 2, 48,
		                          rfseq_bundle_48);
		rfseq_bundle_48[0] = 0x000c;
		rfseq_bundle_48[1] = 0x3;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 3, 48,
		                          rfseq_bundle_48);
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
		uint8 txevmtbl[40] = {0x09, 0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D, 0x20, 0x09,
			0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D, 0x20, 0x22, 0x24, 0x09, 0x0E,
			0x11, 0x14, 0x17, 0x1A, 0x1D, 0x20, 0x22, 0x24, 0x09, 0x0E, 0x11,
			0x14, 0x17, 0x1A, 0x1D, 0x20, 0x22, 0x24, 0x0, 0x0};
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_TXEVMTBL, 40, 0, 8, txevmtbl);
	}

	/* 4335: Running phase track loop faster */
	/* Fix for ping issue caused by high phase imbalance */
	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		uint32 phasetracktbl_1x1[22] = { 0x6AF5700, 0x59ACC9A,
			0x4CE6666, 0x4422064, 0x4422064, 0x4422064,	0x4422064,
			0x4422064, 0x4422064, 0x4422064, 0x4422064, 0x6AF5700,
			0x59ACC9A, 0x4CE6666, 0x4422064, 0x4422064, 0x4422064,
			0x4422064, 0x4422064, 0x4422064, 0x4422064, 0x4422064};
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PHASETRACKTBL_1X1, 22, 0, 32,
		                          phasetracktbl_1x1);
	}
	/* Increase phase track loop BW to improve PER floor, */
	/*   Phase noise  seems higher. Needs further investigation */
	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		uint32 phasetracktbl[22] = { 0x6AF5700, 0x59ACC9A,
			0x4CE6666, 0x4422064, 0x4422064, 0x4422064,	0x4422064,
			0x4422064, 0x4422064, 0x4422064, 0x4422064, 0x6AF5700,
			0x59ACC9A, 0x4CE6666, 0x4422064, 0x4422064, 0x4422064,
			0x4422064, 0x4422064, 0x4422064, 0x4422064, 0x4422064};
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PHASETRACKTBL, 22, 0, 32,
		                          phasetracktbl);
	}
	/* DEBUG: TEST CODE FOR SS PTW70 DEBUG */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && BF3_PHASETRACK_MAX_ALPHABETA(pi_ac)) {
		for (offset = 0; offset < 22; offset++) {
			wlc_phy_table_write_acphy(pi, 0x1a, 1, offset, 32, &war_val);
		}
	}

	/* initialize the fixed spatial expansion matrix */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev)) {
		phy_ac_chanmgr_set_spexp_matrix(pi);
	}

	/* To save current, turn off AFEDiv for the unused core, */
	/* Below forces AFEDiv_pu_repeater2_disRX to be 0 when doing TX2RX || reset2RX */
	/* if (ACMAJORREV_2(pi->pubpi->phy_rev)) { */
	/* 	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe9, 16, &AFEdiv_read_val); */
	/* 	AFEdiv_read_val = (AFEdiv_read_val & 0xfdff); */
	/* 	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0xe9, 16, &AFEdiv_read_val); */
	/* } */

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Writing tables related to 43012 */
		acphytbl_info_t *tbl = phy_ac_get_tbls_write_on_reset_list(pi);

		while (tbl->tbl_ptr != NULL) {
			wlc_phy_table_write_acphy(pi, tbl->tbl_id, tbl->tbl_len,
					tbl->tbl_offset, tbl->tbl_width, tbl->tbl_ptr);
			tbl++;
		}

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
				&read_val);
		write_val[0] = (read_val[0] | (1<<18));
		write_val[1] = read_val[1];
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
				write_val);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
				&read_val);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
				&read_val);
		write_val[0] = (read_val[0] | (1<<18));
		write_val[1] = read_val[1];
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
				write_val);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
				&read_val);

		// Bundle cmd: 0xbe adc_en = 1 dac_en = 1
		rfseq_bundle_48[0] = 0x0;
		rfseq_bundle_48[1] = 0x0E20;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 62, 48,
				rfseq_bundle_48);

		//  Bundle cmd: 0xbf  only adc_en = 1 reset_enable = 0
		rfseq_bundle_48[0] = 0x0;
		rfseq_bundle_48[1] = 0x0400;
		rfseq_bundle_48[2] = 0x0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 63, 48,
				rfseq_bundle_48);

		if (!ACMINORREV_0(pi)) {
				uint16 rfseq_read_val;
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
						&read_val);
				write_val[0] = (read_val[0] | 0x2);
				write_val[1] = read_val[1];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
						write_val);

				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
						&read_val);
				write_val[0] = (read_val[0] & 0xfffffffd);
				write_val[1] = read_val[1];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
						write_val);

				/* Changes related to SW43012-1540 */
				/* Unset RX_CFG(0) */
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
						&read_val);
				write_val[0] = (read_val[0] & 0xfffffffe);
				write_val[1] = read_val[1];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
						write_val);

				/* Program tx2rx_lpf_ctl_lut_enrx(40:39) = 3
				    RFSeqTbl[0x360] => tx2rx_lpf_ctl_lut_enrx(0)(43:35)
				*/
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x360, 16,
						&rfseq_read_val);
				rfseq_read_val = rfseq_read_val | (3 << 4);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x360, 16,
						&rfseq_read_val);

				/* Program tx2rx_lpf_ctl_lut_disrx(41:40) = 0
				    RFSeqTbl[0x361] => tx2rx_lpf_ctl_lut_disrx(0)(44:36)
				*/
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x361, 16,
						&rfseq_read_val);
				rfseq_read_val = rfseq_read_val & 0xffcf;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x361, 16,
						&rfseq_read_val);

				/* Program rx2tx_lpf_ctl_lut_entx(37:36) = 0
				    RFSeqTbl[0x145] => rx2tx_lpf_ctl_lut_entx(0)(40:33)
				*/
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x145, 16,
						&rfseq_read_val);
				rfseq_read_val = rfseq_read_val & 0xffe7;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x145, 16,
						&rfseq_read_val);

				/* Program rx2tx_lpf_ctl_lut_distx(28:27) = 0
				    RFSeqTbl[0x36c] => rx2tx_lpf_ctl_lut_distx(0)(31:17)
				*/
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36c, 16,
						&rfseq_read_val);
				rfseq_read_val = rfseq_read_val & 0xf3ff;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x36c, 16,
						&rfseq_read_val);
		}
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		for (ctr = 0; ctr < 4; ctr++) {
			/* TX, lpf_bq2_pu=1, lpf_bq1_pu=0 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
			                          rfseq_lpf_pu_addr[ctr], 16,
			                          &rfseq_lpf_pu_val);
			/* dac_diode_pwrup = 1 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
			                          rfseq_dacdiode_addr[ctr],
			                          16, &rfseq_dacdiode_val);
		}
	}

#ifdef WLSMC
	if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_0(pi)) {
		if (pi->smci->download) {
			phy_smc_reset(pi, TRUE);
		}

	} else if ((ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) ||
		ACMAJORREV_51(pi->pubpi->phy_rev)) {

		if (pi->smci->download) {
			for (offset = 0; offset < 9; offset++) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPTINYTBL, 8,
					0x8a0 + offset*8, 32, &Ndbps_ru_LUT[offset]);
			}
			for (offset = 0; offset < 3; offset++) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPTINYTBL, 6,
					0x980 + offset*8, 32, &n_cbps_ru_LUT[offset]);
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPTINYTBL, 18,
					0, 32, &dummy_rxctrl[0]);
			phy_smc_reset(pi, FALSE);

			phy_smc_reset(pi, TRUE);

			OSL_DELAY(1);
			MOD_PHYREG(pi, mac2smc_controls, mac2smc_interrupt, 0x1);
			OSL_DELAY(6);
		}
	}
#endif /* WLSMC */

#ifdef WLVASIP
	if ACMAJORREV_47(pi->pubpi->phy_rev) {
		if (pi->vasipi->active) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFDINDEXLUT, 4,
				128, 32, &bfdLut_content[0]);

			// for implicit beamforing, steering idx 127 is used
			// which will point to svmp 0x2600
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFDINDEXLUT, 1,
				159, 32, &bfdLut_content[1]);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_IBFERPTADDRLUT, 1,
				63, 16, &ibfdRptAddrLut_content);
			/* After 43684B1, by default phy reset won't trigger vasip reset */
			if (ACMINORREV_LE(pi, 1)) {
				phy_vasip_reset_proc(pi, TRUE);
				phy_vasip_reset_proc(pi, FALSE);
			}
		}
	}
#endif /* WLVASIP */

	ACPHY_ENABLE_STALL(pi, stall_val);

}

static void
wlc_phy_set_regtbl_on_band_change_majrev_ge40(phy_info_t *pi)
{
	uint8 core, phy_coremask = pi->pubpi->phy_coremask;
	uint sicoreunit;
	int32 rd;

	PHY_INFORM(("%s: band=%d coremask = 0x%x\n", __FUNCTION__,
		(CHSPEC_IS2G(pi->radio_chanspec))?2:5, phy_coremask));

	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);

	/* RFSeqExt table programming */
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		wlc_phy_set_rfseqext_tbl_majrev40(pi);
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		wlc_phy_set_rfseqext_tbl_majrev44(pi);
	}

	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				MOD_RADIO_REG_20694(pi, RF, RX5G_REG5, core,
						rx5g_ldo_pu, 0x1);
			} else {
				MOD_RADIO_REG_20694(pi, RF, RX2G_REG4, core,
						rx2g_ldo_pu, 0x1);
			}
		}
		MOD_PHYREG(pi, dyn_radioa0, dyn_radio_ovr0, 0x1);
		MOD_PHYREG(pi, dyn_radioa1, dyn_radio_ovr1, 0x1);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			//set phyreg(dyn_radioa$core.dyn_radio_ovr_val_idac_main$core) 0x2a;
			MOD_PHYREG(pi, dyn_radioa0, dyn_radio_ovr_val_idac_main0, 0x2a);
			MOD_PHYREG(pi, dyn_radioa1, dyn_radio_ovr_val_idac_main1, 0x2a);
			//set phyreg(radio_connection_direct_pad_idac.pad_idac_gm) 0xa;
			MOD_PHYREG(pi, radio_connection_direct_pad_idac, pad_idac_gm, 0xa);
			//set phyreg(radio_connection_direct_mx_bbdc.mx5g_idac_bbdc) 0xa;
			MOD_PHYREG(pi, radio_connection_direct_mx_bbdc,
				mx5g_idac_bbdc, 0xa);
			//set phyreg(radio_connection_direct_pad_idac.pad_idac_pmos) 0x14;
			MOD_PHYREG(pi, radio_connection_direct_pad_idac,
				pad_idac_pmos, 0x14);
			//set phyreg(radio_connection_direct_mx_bbdc.mx2g_idac_bbdc) 0x14;
			MOD_PHYREG(pi, radio_connection_direct_mx_bbdc,
				mx2g_idac_bbdc, 0x14);
		} else {
			wlc_phy_set_papdlut_dynradioregtbl_majrev_ge40(pi);
		}

		if (PHY_COREMASK_SISO(phy_coremask)) {
			/* SISO */
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				PHY_INFORM(("%s: SISO 5G core= %d\n", __FUNCTION__, core));
				WRITE_PHYREGCE(pi, Dac_gain, core, 0xd670);
				WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x1f);
			} else {
				PHY_INFORM(("%s: SISO 2G core= %d\n", __FUNCTION__, core));
				WRITE_PHYREGCE(pi, Dac_gain, core, 0x7870);
				WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x3f);
			}
			if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
				WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x67f);
			}
		} else {
			/* MIMO */
			if (core == 1) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					PHY_INFORM(("wl%d %s: MIMO 5G core= %d\n",
						PI_INSTANCE(pi), __FUNCTION__, core));
					sicoreunit = wlapi_si_coreunit(pi->sh->physhim);
					WRITE_PHYREGCE(pi, Dac_gain, core, 0xc770);
					if (sicoreunit == DUALMAC_MAIN) {
						PHY_INFORM(("wl%d %s: MIMO 5G MAIN\n",
							PI_INSTANCE(pi), __FUNCTION__));
						WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core,
							0x2fb);
					} else if (sicoreunit == DUALMAC_AUX) {
						PHY_INFORM(("wl%d %s: MIMO 5G AUX\n",
							PI_INSTANCE(pi), __FUNCTION__));
						WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core,
							0x2ff);
					}
					WRITE_PHYREGCE(pi, Extra2AfeClkDivOverrideCtrl28nm,
						core, 0x20);
				} else {
					PHY_INFORM(("wl%d %s: MIMO 2G core= %d\n",
						PI_INSTANCE(pi), __FUNCTION__, core));
					WRITE_PHYREGCE(pi, Dac_gain, core, 0x6170);
					WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0xff);
					WRITE_PHYREGCE(pi, Extra2AfeClkDivOverrideCtrl28nm,
						core, 0x10);
				}
			} else if (core == 0) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					PHY_INFORM(("wl%d %s: MIMO 5G core= %d\n",
						PI_INSTANCE(pi), __FUNCTION__, core));
					WRITE_PHYREGCE(pi, Dac_gain, core, 0xc770);
					WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x77f);
					WRITE_PHYREGCE(pi, Extra2AfeClkDivOverrideCtrl28nm,
						core, 0x20);
				} else {
					PHY_INFORM(("wl%d %s: MIMO 2G core= %d\n",
						PI_INSTANCE(pi), __FUNCTION__, core));
					WRITE_PHYREGCE(pi, Dac_gain, core, 0x6170);
					WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x1ff);
					WRITE_PHYREGCE(pi, Extra2AfeClkDivOverrideCtrl28nm,
						core, 0x10);
				}
			}
			if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
				WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x67f);
			}
		}
	}
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* CRDOT11ACPHY-2555 lesi and bphy_pre_det set in aux block bphy */
		phy_ac_rxgcrs_iovar_get_lesi(pi->u.pi_acphy->rxgcrsi, &rd);
		if (rd && wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX)
			MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
		else {
			MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 1);
			if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, dot11acConfig, bphyPreDetTmOutEn, 1);
				MOD_PHYREG(pi, RxControl, preDetOnlyinCS, 1);
			}
		}
	} else {
		MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
	}

	MOD_PHYREG(pi, fineRxclockgatecontrol, EncodeGainClkEn, 1);
}

static void
wlc_phy_set_regtbl_on_band_change_acphy(phy_info_t *pi)
{
	uint8 stall_val, core;
	uint16 bq1_gain_core1 = 0x49;
	uint8 pdet_range_id;
#ifndef WLC_DISABLE_ACI
	bool hwaci_on;
#endif /* !WLC_DISABLE_ACI */
	bool w2_on;
	txcal_coeffs_t txcal_cache[PHY_CORE_MAX];
	rxcal_coeffs_t rxcal_cache[PHY_CORE_MAX];
#ifdef PHYCAL_CACHING
	ch_calcache_t *ctx;
	bool ctx_valid;
#endif /* PHYCAL_CACHING */
	uint16 rfpwrlutval0[128] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, -4, -4, -4, -4, -6, -6, -12, -12, -18, -18, -22, -22, -26, -26, -28,
		-28, -30, -30, -34, -34, -36, -36, -40, -40, -40, -40, -40, -40, -40, -40, -40,
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
	};
	uint16 rfpwrlutval1[128] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, -4, -4, -4, -4, -6, -6, -12, -12, -18, -18, -18, -18, -24, -24, -24,
		-24, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
		-40, -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40 -40
	};
	uint16 gpioout = 0;
	uint16 gpio9_mask = 0x200;
	uint16 gpio11_mask = 0x800;
	acphy_swctrlmap4_t *swctrl = pi->u.pi_acphy->sromi->swctrlmap4;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_cfg_energydrop_timeout(pi);

	if (!ACMAJORREV_47(pi->pubpi->phy_rev)) {
		wlc_phy_bphymrc_hwconfig(pi);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* write to bphy regs only when in 2g */
		WRITE_PHYREG(pi, DsssStep, 0x668);      /* gives slight improvement */

		if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
		    WRITE_PHYREG(pi, bphymrcCtrl, 0x60);    /* disable bphy mrc for now  */
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_set_rfseqext_tbl_majrev47(pi);
	} else if (ACMAJORREV_GE40(pi->pubpi->phy_rev)) {
		wlc_phy_set_regtbl_on_band_change_majrev_ge40(pi);
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, fineRxclockgatecontrol, forcedigigaingatedClksOn, 1);
		} else {
			MOD_PHYREG(pi, fineRxclockgatecontrol, forcedigigaingatedClksOn, 0);
		}

		/* 4335C0: Current optimization */
		if (ACMINORREV_2(pi)) {
			MOD_PHYREG(pi, fineRxclockgatecontrol, forcedigigaingatedClksOn, 0);
		}
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
		ACMAJORREV_2(pi->pubpi->phy_rev) || (ACMAJORREV_1(pi->pubpi->phy_rev) &&
		!(ACMINORREV_0(pi) || ACMINORREV_1(pi))) ||
		(ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev))) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, bOverAGParams, bOverAGlog2RhoSqrth, 120);
			MOD_PHYREG(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 1);
		} else {
			MOD_PHYREG(pi, bOverAGParams, bOverAGlog2RhoSqrth, 255);
			MOD_PHYREG(pi, CRSMiscellaneousParam, b_over_ag_falsedet_en, 0);
		}
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		FOREACH_CORE(pi, core) {
			/* Reduces 20in80 humps in 5G */
			WRITE_PHYREGC(pi, Clip2Threshold, core, 0xa04e);
			if (CHSPEC_IS2G(pi->radio_chanspec))
			  WRITE_PHYREGC(pi, Clip2Threshold, core, 0x804e);
		}
	}

	if (ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
	    ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, clip_detect_normpwr_var_mux, use_norm_var_for_clip_detect, 1);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, clip_detect_normpwr_var_mux, use_norm_var_for_clip_detect, 0);
	}

	/* Load tx gain table */
	wlc_phy_ac_gains_load(pi->u.pi_acphy->tbli);

	if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
		wlc_phy_tx_gm_gain_boost(pi);
	}
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		wlc_phy_set_afediv_reset_bundles(pi);
	}

	pdet_range_id = phy_tpc_get_5g_pdrange_id(pi->tpci);
	if ((pdet_range_id == 9 || pdet_range_id == 16) && !ACMAJORREV_32(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_33(pi->pubpi->phy_rev) && !ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		bq1_gain_core1 = (CHSPEC_IS5G(pi->radio_chanspec))? 0x49 : 0;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x18e, 16, &bq1_gain_core1);
	}

	if (IS_4364_1x1(pi) ||	IS_4364_3x3(pi)) {
		wlc_phy_ac_shared_ant_femctrl_master(pi);
	} else {
#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
		/* Write FEMCTRL mask to shmem ; let ucode write them to FEMCTRL register */
		wlc_phy_ac_femctrl_mask_on_band_change_btcx(pi->u.pi_acphy->btcxi);
#else
		wlc_phy_ac_femctrl_mask_on_band_change(pi);
#endif // endif
	}

	/* 20691 specific processing, if needed */
	if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID))
		wlc_phy_set_regtbl_on_band_change_acphy_20691(pi);
	else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		wlc_phy_set_regtbl_on_band_change_acphy_20693(pi);

		/* 4349 specific chspec initializations */
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			if (CCT_INIT(pi->u.pi_acphy) ||
				CCT_BAND_CHG(pi->u.pi_acphy) || CCT_BW_CHG(pi->u.pi_acphy)) {
				wlc_acphy_load_4349_specific_tbls(pi);
				wlc_acphy_dyn_papd_cfg_20693(pi);
				wlc_phy_config_bias_settings_20693(pi);
				acphy_set_lpmode(pi, ACPHY_LP_PHY_LVL_OPT);
			}
		}

	}

	/* 2g/5g band can have different aci modes */
	if (!ACPHY_ENABLE_FCBS_HWACI(pi)) {
#ifndef WLC_DISABLE_ACI
		hwaci_on = ((pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) ||
		(((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) != 0) &&
		!(ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)));
		wlc_phy_hwaci_setup_acphy(pi, hwaci_on, FALSE);
#endif /* !WLC_DISABLE_ACI */
		w2_on = ((pi->sh->interference_mode & ACPHY_ACI_W2NB_PKTGAINLMT) != 0) ||
			(((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) != 0) &&
			!(ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)));
		wlc_phy_aci_w2nb_setup_acphy(pi, w2_on);
	}

	if (PHY_PAPDEN(pi)) {

		if (!ACMAJORREV_4(pi->pubpi->phy_rev))
			OSL_DELAY(100);

		if (TINY_RADIO(pi))
		{
#ifdef PHYCAL_CACHING
			PHY_CAL(("wlc_phy_set_regtbl_on_band_change_acphy:"
				" wlc_phy_get_chanctx\n"));
			ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
			ctx_valid = (ctx != NULL) ? ctx->valid : FALSE;

			/* allow reprogramming rfpwrlut if ctx is not available or
			 * ctx is available but invalid
			 */
			if (!ctx_valid)
#endif /* PHYCAL_CACHING */
				wlc_phy_papd_set_rfpwrlut_tiny(pi);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			wlc_phy_papd_set_rfpwrlut_phymaj_rev36(pi);
		} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0, 128, 0,
				16, &rfpwrlutval0);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS1, 128, 0,
				16, &rfpwrlutval1);
		} else {
			wlc_phy_papd_set_rfpwrlut(pi);
		}
	}

	/* For 4350C0, bphy is turned off when in 5G. Need to disable the predetector. */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
		} else {
			MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, 0);
		}
	}

	/* For 4345, bphy is turned on when in 5G. Need to enable the predetector and timeout to
	 * Effectively disable and reduce average current ~1.5mA and remove 2mA x 20ms humps
	 * See http://jira.broadcom.com/browse/SWWLAN-77067
	 * Missing CRDOT11ACPHY-815
	 */
	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		uint16 enablepd = ((CHSPEC_IS5G(pi->radio_chanspec)) ? 1 : 0);
		MOD_PHYREG(pi, CRSMiscellaneousParam, bphy_pre_det_en, enablepd);
		MOD_PHYREG(pi, dot11acConfig, bphyPreDetTmOutEn, enablepd);
	}

	/* Turn ON 11n 256 QAM in 2.4G */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		bool enable = (CHSPEC_IS2G(pi->radio_chanspec) && CHSPEC_IS20(pi->radio_chanspec));

		if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
		    !ACMAJORREV_47_51(pi->pubpi->phy_rev))
			WRITE_PHYREG(pi, miscSigCtrl, enable ? 0x203 : 0x3);

		wlapi_11n_proprietary_rates_enable(pi->sh->physhim, enable);

		PHY_INFORM(("wl%d %s: 11n turbo QAM %s\n",
			PI_INSTANCE(pi), __FUNCTION__,
			enable ? "enabled" : "disabled"));

		/* Loading Tx specific radio settings  */
		if (ACMAJORREV_4(pi->pubpi->phy_rev) &&
			RADIOID_IS(pi->pubpi->radioid, BCM20693_ID))
			wlc_phy_config_bias_settings_20693(pi);
	}

	/* knoise rxgain override value initializaiton */
	/*
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wlapi_bmac_write_shm(pi->sh->physhim, M_RXGAIN_HI(pi), 0x5d7);
			wlapi_bmac_write_shm(pi->sh->physhim, M_RXGAIN_LO(pi), 0x457);
		} else {
			wlapi_bmac_write_shm(pi->sh->physhim, M_RXGAIN_HI(pi), 0x1d5);
			wlapi_bmac_write_shm(pi->sh->physhim, M_RXGAIN_LO(pi), 0x55);
		}
	}
	*/
#ifdef WL_EAP_NOISE_MEASUREMENTS
	/* knoise rxgain override value initializaiton */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_HI(pi), RxGAIN_HI_5G);
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_LO(pi), RxGAIN_LO_5G);
		} else {
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_HI(pi), RxGAIN_HI_2G);
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_RXGAIN_LO(pi), RxGAIN_LO_2G);
		}
	}
#ifdef WL_EAP_BCM43570
	else if (IS_43570(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* Compensate for 2G's low LNA2 gains */
			if (BF_ELNA_2G(pi->u.pi_acphy)) {
				wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_HI(pi), 0x28);
				wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_LO(pi), 0x9);
			}
			else {
				wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_HI(pi), 0x28);
				wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_LO(pi), 0x0);
			}
		/* Else 5G */
		} else {
			/* Compensate for 5G's eLNA-less high mixer gain */
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_HI(pi), 0x32);
			wlapi_bmac_write_shm(pi->sh->physhim, M_NCAL_LPFGAIN_LO(pi), 0x3c);
		}
	}
#endif /* WL_EAP_BCM43570 */

#endif /* WL_EAP_NOISE_MEASUREMENTS */
	/* need to zero out cal coeffs on band change */
	bzero(txcal_cache, sizeof(txcal_cache));
	bzero(rxcal_cache, sizeof(rxcal_cache));
	wlc_phy_txcal_coeffs_upd(pi, txcal_cache);
	wlc_phy_rxcal_coeffs_upd(pi, rxcal_cache);
	if (!ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMAJORREV_33(pi->pubpi->phy_rev)) {
		wlc_acphy_paldo_change(pi);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);

	/* Using different filter settings for 2G and 5G (50MHz notch) for 43012
	 This improves both the Jammer performance and AACI performance for 2G
	 while not degrading the ACI performance. For 5G use default
	 filter settings (40MHz notch)
	*/
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RfctrlOverrideLpfCT0, lpf_bq2_bw, 1);
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			MOD_PHYREG(pi, RfctrlCoreLpfCT0, lpf_bq2_bw, 0);
		} else {
			MOD_PHYREG(pi, RfctrlCoreLpfCT0, lpf_bq2_bw, 1);
		}
	}

	if (ACMAJORREV_40(pi->pubpi->phy_rev) &&
		pi->u.pi_acphy->chanmgri->cfg->srom_nonbf_logen_mode_en) {
		wlc_phy_low_pwr_logen_setting(pi, 0);
	}
	if (swctrl->bandsel_on_gpio9 && ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		gpioout = CHSPEC_IS2G(pi->radio_chanspec) ? 0 : gpio9_mask;
		si_gpioout(pi->sh->sih, gpio9_mask, gpioout, GPIO_DRV_PRIORITY);
		si_gpioouten(pi->sh->sih, gpio9_mask, gpio9_mask, GPIO_DRV_PRIORITY);
		//chip_gpio9 = gpioout
		si_gpiocontrol(pi->sh->sih, gpio9_mask, 0, GPIO_DRV_PRIORITY);
	}
	if (swctrl->bandsel_on_gpio11 && ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		gpioout = CHSPEC_IS2G(pi->radio_chanspec) ? 0 : gpio11_mask;
		si_gpioout(pi->sh->sih, gpio11_mask, gpioout, GPIO_DRV_PRIORITY);
		si_gpioouten(pi->sh->sih, gpio11_mask, gpio11_mask, GPIO_DRV_PRIORITY);
		si_gpiocontrol(pi->sh->sih, gpio11_mask, 0, GPIO_DRV_PRIORITY);
	}
}

static void
wlc_phy_set_regtbl_on_bw_change_acphy(phy_info_t *pi)
{
	int sp_tx_bw = 0;
	uint8 stall_val, core, nbclip_cnt_4360 = 15;
	uint8 rxevm20p[] = {8, 6, 4}, rxevm20n[] = {4, 6, 8};
	uint8 rxevm0[] = {0, 0, 0}, rxevm_len = 3;
	uint32 epa_turnon_time;
	uint32 NvAdjAXVal;
	uint8 lpf_txbq1[] = {0, 0, 0, 0}, lpf_txbq2[] = {4, 5, 6, 7};
	uint8 lpf_rxbq1[] = {0, 1, 2, 3}, lpf_rxbq2[] = {0, 1, 2, 3};
	uint16 x, NvAdjAXStartTone, NvAdjAXEndTone, val;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (BW_RESET == 1 && !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_set_reg_on_bw_change_acphy(pi);
	}

	if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID &&
		pi->sh->chippkg == BCM4335_FCBGA_PKG_ID)
		nbclip_cnt_4360 = 12;

	if (CHSPEC_IS80(pi->radio_chanspec) ||
		PHY_AS_80P80(pi, pi->radio_chanspec)) {
		/* 80mhz */
		if (ACMAJORREV_0(pi->pubpi->phy_rev))
			sp_tx_bw = 5;
		else
			sp_tx_bw = 6;

		nbclip_cnt_4360 *= 4;
	} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		/* true 160mhz */
		sp_tx_bw = 7;
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			nbclip_cnt_4360 *= 8;
		} else {
			nbclip_cnt_4360 *= 4;
		}
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		/* 40mhz */
		if (ACMAJORREV_0(pi->pubpi->phy_rev))
			sp_tx_bw = 4;
		else
			sp_tx_bw = 5;

		nbclip_cnt_4360 *= 2;
	} else if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
		/* 20mhz */
		if (ACMAJORREV_0(pi->pubpi->phy_rev))
			sp_tx_bw = 3;
		else if (ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
			sp_tx_bw = 4;
		} else {
			sp_tx_bw = 5;
		}
	} else {
		PHY_ERROR(("%s: No primary channel settings for bw=%d\n",
		           __FUNCTION__, CHSPEC_BW(pi->radio_chanspec)));
	}

	/* reduce NB clip CNT thresholds */
	FOREACH_CORE(pi, core) {
		if (!ACMAJORREV_1(pi->pubpi->phy_rev) ||
			(CHSPEC_IS2G(pi->radio_chanspec) && BF3_AGC_CFG_2G(pi->u.pi_acphy)) ||
			(CHSPEC_IS5G(pi->radio_chanspec) && BF3_AGC_CFG_5G(pi->u.pi_acphy))) {
			MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh,
				nbclip_cnt_4360);
		} else {
			MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh, 23);
		}
	}
	if (!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_set_analog_tx_lpf(pi, 0x100, -1, sp_tx_bw, sp_tx_bw, -1, -1, -1);
	}
	if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		/* This is necessary because in 7271 RC filter is not connected to RFSEQ */
		FOREACH_CORE(pi, core) {
			WRITE_RADIO_REG_20696(pi, LPF_GMULT_RC_BW, core, (uint16)sp_tx_bw);
		}
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		wlc_phy_radio20698_set_tx_notch(pi);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		wlc_phy_radio20704_set_tx_notch(pi);
	}

	/* change the barelyclipgainbackoff to 6 for 80Mhz due to some PER issue for 4360A0 CHIP */
	if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
	  if (CHSPEC_IS80(pi->radio_chanspec)) {
	      ACPHYREG_BCAST(pi, Core0computeGainInfo, 0xcc0);
	  } else {
	      ACPHYREG_BCAST(pi, Core0computeGainInfo, 0xc60);
	  }
	}

#ifndef WL_FDSS_DISABLED
	/* Enable FDSS */
	if (TINY_RADIO(pi) && !ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev) &&
		((CHSPEC_IS2G(pi->radio_chanspec) && (pi->fdss_level_2g[0] != -1)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && (pi->fdss_level_5g[0] != -1))))  {
		wlc_phy_fdss_init(pi);
		if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			wlc_phy_set_fdss_table_majrev_ge40(pi);
		} else {
			wlc_phy_set_fdss_table(pi);
		}
	}
#endif /* WL_FDSS_DISABLED */
	/* For 4347, set the notch filter per bandwidth */
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		FOREACH_CORE(pi, core) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				MOD_RADIO_REG_20694(pi, RF, LPF_GMULT_RC_BW, core,
					lpf_gmult_rc_bw, 0x0);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				MOD_RADIO_REG_20694(pi, RF, LPF_GMULT_RC_BW, core,
					lpf_gmult_rc_bw, 0x1);
			} else {
				MOD_RADIO_REG_20694(pi, RF, LPF_GMULT_RC_BW, core,
					lpf_gmult_rc_bw, 0x2);
			}
		}
	}
	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		wlc_phy_ac_set_20697_lpf_rc_bw(pi);
	}
	/* SWWLAN-28943 */
	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		MOD_PHYREGC(pi, computeGainInfo, 0, gainBackoffValue, 1);
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) || ACMINORREV_3(pi))) {
		FOREACH_CORE(pi, core) {
			/* Reduces 54Mbps humps */
			MOD_PHYREGC(pi, computeGainInfo, core, gainBackoffValue, 1);
			/* Reduces 20in80 humps */
			WRITE_PHYREGC(pi, Clip2Threshold, core, 0xa04e);
		}
	}

	if (IS_4364_3x3(pi)) {
		FOREACH_CORE(pi, core) {
			/* Reduces 54Mbps humps */
			MOD_PHYREGC(pi, computeGainInfo, core, gainBackoffValue, 1);
		}
	}

	/* Shape rxevm table due to hit on near DC_tones */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_2(pi->pubpi->phy_rev) ||
	    ACMAJORREV_5(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			/* Freq Bins {1 2 3} = {8 6 4} dB */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVRXEVMSHAPINGTBL,
			                          rxevm_len, 1, 8, rxevm20p);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVRXEVMSHAPINGTBL,
			                          rxevm_len, 64 - rxevm_len, 8, rxevm20n);
		} else {
			/* Reset the 20mhz entries to 0 */
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVRXEVMSHAPINGTBL,
			                          rxevm_len, 1, 8, rxevm0);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVRXEVMSHAPINGTBL,
			                          rxevm_len, 64 - rxevm_len, 8, rxevm0);
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
		BCM_REFERENCE(phyrxchain);
		FOREACH_ACTV_CORE(pi, phyrxchain, core) {
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_clamp_en, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_clamp_en, 1);
		}

		/* updategainH : increase clamp_en off delay to 16 */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x30, 16, rf_updh_cmd_clamp);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xa0, 16, rf_updh_dly_clamp);

		/* updategainL : increase clamp_en off delay to 16 */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x40, 16, rf_updl_cmd_clamp);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xb0, 16, rf_updl_dly_clamp);

		/* updategainU : increase clamp_en off delay to 16 */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x50, 16, rf_updu_cmd_clamp);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xc0, 16, rf_updu_dly_clamp);
	}

	/* [4365]resovle DVGA stuck high - htagc and gainreset during wait_energy_drop collides */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev))
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xb0, 16, rf_updl_dly_dvga);

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
		WRITE_PHYREG_ENTRY(pi, ofdmpaydecodetimeoutlen, 0x7d0)
		/* 2000 * 25 us = 50 ms */
		WRITE_PHYREG_ENTRY(pi, cckpaydecodetimeoutlen, 0x7d0)
		/* 2000 * 25 us = 50 ms */
		WRITE_PHYREG_ENTRY(pi, nonpaydecodetimeoutlen, 0x20)
		/* 32 * 25 us = 800 us */
		ACPHY_REG_LIST_EXECUTE(pi);
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, timeoutEn, resetRxontimeout, 1)
			MOD_PHYREG_ENTRY(pi, timeoutEn, resetCCAontimeout, 1)
			MOD_PHYREG_ENTRY(pi, timeoutEn, nonpaydecodetimeoutEn, 0)
			MOD_PHYREG_ENTRY(pi, timeoutEn, cckpaydecodetimeoutEn, 0)
			MOD_PHYREG_ENTRY(pi, timeoutEn, ofdmpaydecodetimeoutEn, 1)
			ACPHY_REG_LIST_EXECUTE(pi);
		} else {
			ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, timeoutEn, resetRxontimeout, 0)
			MOD_PHYREG_ENTRY(pi, timeoutEn, resetCCAontimeout, 1)
			MOD_PHYREG_ENTRY(pi, timeoutEn, nonpaydecodetimeoutEn, 0)
			MOD_PHYREG_ENTRY(pi, timeoutEn, cckpaydecodetimeoutEn, 0)
			MOD_PHYREG_ENTRY(pi, timeoutEn, ofdmpaydecodetimeoutEn, 1)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	}

	if (ACMAJORREV_1(pi->pubpi->phy_rev) || TINY_RADIO(pi) ||
			ACMAJORREV_36(pi->pubpi->phy_rev)) {
			if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, nonpaydecodetimeoutlen, 1)
					MOD_PHYREG_ENTRY(pi, timeoutEn, resetCCAontimeout, 1)
					MOD_PHYREG_ENTRY(pi, timeoutEn, nonpaydecodetimeoutEn, 1)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else {
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, nonpaydecodetimeoutlen, 32)
					MOD_PHYREG_ENTRY(pi, timeoutEn, resetCCAontimeout, 0)
					MOD_PHYREG_ENTRY(pi, timeoutEn, nonpaydecodetimeoutEn, 0)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
	}

	/* 4360, 4350. 4335 does its own stuff */
	if (!ACMAJORREV_1(pi->pubpi->phy_rev)) {
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				/* reduce clip2 len, helps with humps due to late clip2 */
				WRITE_PHYREG(pi, defer_setClip1_CtrLen, 40);
				WRITE_PHYREG(pi, defer_setClip2_CtrLen, 20);
			} else if (CHSPEC_IS160(pi->radio_chanspec)) {
				/* increase clip1 len. Needed for 20in160 cases */
				WRITE_PHYREG(pi, defer_setClip1_CtrLen, 45);
				WRITE_PHYREG(pi, defer_setClip2_CtrLen, 40);
			} else {
				/* Use default value */
				WRITE_PHYREG(pi, defer_setClip1_CtrLen, 40);
				WRITE_PHYREG(pi, defer_setClip2_CtrLen, 40);
			}

			/* Increase pktgain settling len to prevent PER spike @ ~-62dB for 160MHz */
			if (CHSPEC_IS160(pi->radio_chanspec)) {
				WRITE_PHYREG(pi, pktgainSettleLen, 0x40);
			} else {
				WRITE_PHYREG(pi, pktgainSettleLen, 0x30);
			}

		} else  {
			if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
				/* reduce clip2 len, helps with humps due to late clip2 */
				WRITE_PHYREG(pi, defer_setClip1_CtrLen, 20);
				WRITE_PHYREG(pi, defer_setClip2_CtrLen, 16);
			} else {
				/* increase clip1 len. Needed for 20in80, 40in80 cases */
				WRITE_PHYREG(pi, defer_setClip1_CtrLen, 30);
				WRITE_PHYREG(pi, defer_setClip2_CtrLen, 20);
			}
		}
	} else if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi) &&
		(!(PHY_ILNA(pi))) && pi->sh->chippkg != BCM4335_FCBGA_PKG_ID) {
		if (CHSPEC_IS80(pi->radio_chanspec)) {
			/* increase clip1 defer  len to make clip gain more accurate */
			/* decrease clip1 carrier blanking length to speedup crs */
			/* this is okay fror 80MHz as the settling is very fast for wider BW */
			ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, defer_setClip1_CtrLen, 36)
				WRITE_PHYREG_ENTRY(pi, defer_setClip2_CtrLen, 16)
				WRITE_PHYREG_ENTRY(pi, clip1carrierDetLen, 77)
				WRITE_PHYREG_ENTRY(pi, clip2carrierDetLen, 72)
			ACPHY_REG_LIST_EXECUTE(pi);
		} else {
		  /* increase defer setclip Gain by 0.1usec */
		  /* reduce clip1 carrier detect blanking by same amount */
		  /* reduce clip2 carrier detect blanking to speedup carrier detect */
		  /* this helps in cleaning the small floor in 4335C0 epa boards */
		  ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, defer_setClip1_CtrLen, 28)
			WRITE_PHYREG_ENTRY(pi, defer_setClip2_CtrLen, 16)
			WRITE_PHYREG_ENTRY(pi, clip1carrierDetLen, 87)
			WRITE_PHYREG_ENTRY(pi, clip2carrierDetLen, 62)
		  ACPHY_REG_LIST_EXECUTE(pi);
		}
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		WRITE_PHYREG(pi, crsThreshold2u, 0x204d);
		WRITE_PHYREG(pi, crsThreshold2l, 0x204d);
		WRITE_PHYREG(pi, crsThreshold2lSub1, 0x204d);
		WRITE_PHYREG(pi, crsThreshold2uSub1, 0x204d);
	}

	if (ACMAJORREV_5(pi->pubpi->phy_rev) ||
		ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* CRS. 6bit MF */
		/* BPHY pre-detect is disabled by default. No writes here. */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, crsThreshold2u, 0x2055);
			WRITE_PHYREG(pi, crsThreshold2l, 0x2055);
		} else {
			WRITE_PHYREG(pi, crsThreshold2u, 0x204d);
			WRITE_PHYREG(pi, crsThreshold2l, 0x204d);
		}
		WRITE_PHYREG(pi, crsThreshold2lSub1, 0x204d);
		WRITE_PHYREG(pi, crsThreshold2uSub1, 0x204d);
	}

	if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* Spur canceller */
		if (CHSPEC_IS20(pi->radio_chanspec))
			WRITE_PHYREG(pi, spur_can_phy_bw_mhz, 0x14);
		else if (CHSPEC_IS40(pi->radio_chanspec))
			WRITE_PHYREG(pi, spur_can_phy_bw_mhz, 0x280);
		else
			WRITE_PHYREG(pi, spur_can_phy_bw_mhz, 0x50);
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {

		uint16 phymode = phy_get_phymode(pi);

		if (phymode == PHYMODE_MIMO) {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				MOD_PHYREG(pi, CRSMiscellaneousParam, crsMfFlipCoef, 0);
				ACPHYREG_BCAST(pi, crsThreshold2u0, 0x2055);
				ACPHYREG_BCAST(pi, crsThreshold2l0, 0x2055);
			} else {
				MOD_PHYREG(pi, CRSMiscellaneousParam, crsMfFlipCoef, 1);
			}
		}
	}

	if (PHY_IPA(pi) && ACMAJORREV_2(pi->pubpi->phy_rev) &&
	    (ACMINORREV_3(pi) || ACMINORREV_5(pi))) {
		/* 4354a1_ipa, to decrease LOFT, move TSSI_CONFIG & extra delay before IPA_PU. Need
		   to move in TSSI_CONFIG, otherwise only delaying IPA_PU would delay TSSI_CONFIG
		   ;80MHz alone this change is backed out..
		*/
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			if (((CHSPEC_IS2G(pi->radio_chanspec)) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x1)) ||
				((CHSPEC_IS5G(pi->radio_chanspec)) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x4))) {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
						16, rfseq_rx2tx_cmd_rev15_ipa_withtssisleep);
			} else {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				                 16, rfseq_rx2tx_cmd_rev15_ipa);
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			                         16, rfseq_rx2tx_dly_rev15_ipa20);
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			if (((CHSPEC_IS2G(pi->radio_chanspec)) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x2)) ||
				((CHSPEC_IS5G(pi->radio_chanspec)) &&
				(pi->u.pi_acphy->chanmgri->cfg->srom_tssisleep_en & 0x8))) {
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
						16, rfseq_rx2tx_cmd_rev15_ipa_withtssisleep);
			} else {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
				                 16, rfseq_rx2tx_cmd_rev15_ipa);
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
			                          16, rfseq_rx2tx_dly_rev15_ipa40);
		}
	}

	/* R8000 - atlas has different PA turn on timing */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		epa_turnon_time = (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
		                   BFL_SROM11_EPA_TURNON_TIME) >> BFL_SROM11_EPA_TURNON_TIME_SHIFT;
		if (epa_turnon_time == 1) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
			                          16, rfseq_rx2tx_cmd);
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
				                          16, rfseq_rx2tx_dly_epa1_20);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
				                          16, rfseq_rx2tx_dly_epa1_40);
			} else {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
				                          16, rfseq_rx2tx_dly_epa1_80);
			}
		}
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		wlc_phy_ac_lpf_cfg(pi);
	}

	if (ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
			phy_ac_chanmgr_switch_phymode_acphy(pi, PHYMODE_80P80);
		} else {
			phy_ac_chanmgr_switch_phymode_acphy(pi, PHYMODE_MIMO);
		}
	}

	if ((ACMAJORREV_33(pi->pubpi->phy_rev) ||
	     ACMAJORREV_37(pi->pubpi->phy_rev) ||
	     ACMAJORREV_47(pi->pubpi->phy_rev)) &&
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
		uint32 read_val[22];
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_PHASETRACKTBL,
			22, 0x0, 32, &read_val);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PHASETRACKTBL_B,
			22, 0x0, 32, &read_val);
	}

	if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev)) {
		if (CHSPEC_IS20(pi->radio_chanspec))
			MOD_PHYREG(pi, FSTRCtrl, sgiLtrnAdjMax, 7);
		else
			MOD_PHYREG(pi, FSTRCtrl, sgiLtrnAdjMax, 3);
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACREV_IS(pi->pubpi->phy_rev, 37)) {
		/* Disable VHT proprietary MCS and 1024-QAM for phybw=20MHz */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			MOD_PHYREG(pi, HTSigTones, ldpc_proprietary_mcs_vht, 0);
			MOD_PHYREG(pi, miscSigCtrl, brcm_vht_1024qam_support, 0);
		} else {
			MOD_PHYREG(pi, HTSigTones, ldpc_proprietary_mcs_vht, 1);
			MOD_PHYREG(pi, miscSigCtrl, brcm_vht_1024qam_support, 1);
		}
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlc_phy_set_analog_lpf(pi, lpf_txbq1, lpf_txbq2, lpf_rxbq1, lpf_rxbq2);
		MOD_PHYREG(pi, bfdsConfig, phyBandwidth, CHSPEC_BW_LE20(pi->radio_chanspec) ?
		0x0 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1 : CHSPEC_IS80(pi->radio_chanspec) ?
		0x2 : 0x3);
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfePhaseSel, 1);
		}

		/* add these registers on channel bw change: no longer going to
			be running bw_reset, rather will do a phy_reset, and so
			need to determine what bw dependent registers are needed to be set
		*/
		MOD_PHYREG(pi, TssiEnRate, StrobeRateOverride, 1);
		MOD_PHYREG(pi, IqestWaitTime, waitTime,
			CHSPEC_IS20(pi->radio_chanspec) ? 20 :
			CHSPEC_IS40(pi->radio_chanspec) ? 40 :
			CHSPEC_IS80(pi->radio_chanspec) ? 40 : 40);

		/* subband Classifer (helps in eliminating flase detect due to ACPR leakage)
		   i.e 20in40/80 is not false detected as 40/80
		   (these regs get reset on a bw change)
		*/
		WRITE_PHYREG(pi, ClassifierLogAC1, 0x90a);
		MOD_PHYREG(pi, ClassifierCtrl6, logACDelta2, 9);

		/* these registers were explicitly set in phy_ac_tssical.c */
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 0x0);
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0x1);
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* Disable ChanUpd for 43684B0. HW bug for BW160 */
		if (ACMINORREV_1(pi)) {
			if (pi->u.pi_acphy->chanmgri->chanup_ovrd == -1) {
				if (CHSPEC_IS160(pi->radio_chanspec)) {
					WRITE_PHYREG(pi, mluA, 0x0004);
					WRITE_PHYREG(pi, zfuA, 0x0004);
				} else {
					WRITE_PHYREG(pi, mluA, 0x2024);
					WRITE_PHYREG(pi, zfuA, 0x2224);
				}
			} else {
				phy_ac_chanmgr_iovar_set_chanup_ovrd(pi,
				  pi->u.pi_acphy->chanmgri->chanup_ovrd);
			}
		} else {
			if (pi->u.pi_acphy->chanmgri->chanup_ovrd != -1) {
				phy_ac_chanmgr_iovar_set_chanup_ovrd(pi,
				  pi->u.pi_acphy->chanmgri->chanup_ovrd);
			}
		}

		/* 11ax has 3~5 dc gaps for phybw <= 80M,
		 * to get good enough EVM for 1028QAM
		 * fine tune EVM shaping table per phybw
		 */
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			NvAdjAXStartTone = 2;
			NvAdjAXEndTone = 10;
		} else if ((CHSPEC_IS40(pi->radio_chanspec)) ||
				(CHSPEC_IS80(pi->radio_chanspec))) {
			NvAdjAXStartTone = 3;
			NvAdjAXEndTone = 11;
		} else {
			NvAdjAXStartTone = 2;
			NvAdjAXEndTone = 11;
		}

		for (x = NvAdjAXStartTone; x < NvAdjAXEndTone; x++) {
			if (CHSPEC_IS160(pi->radio_chanspec)) {
				NvAdjAXVal = 0;
			} else {
				NvAdjAXVal = NvAdjAXEndTone-x;
				NvAdjAXVal = NvAdjAXVal + (NvAdjAXVal << 6);
			}
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVADJ11AXTBL,
				1, x, 32, &NvAdjAXVal);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVADJ11AXTBL,
				1, 256-x, 32, &NvAdjAXVal);
		}

		// Enable fft-in shift with BW utilization <= 12.5 %
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, fft_backoff_cont_0, 0x1011);
			WRITE_PHYREG(pi, fft_backoff_cont_1, 0x1100);
		} else {
			WRITE_PHYREG(pi, fft_backoff_cont_0, 0x0);
			WRITE_PHYREG(pi, fft_backoff_cont_1, 0x0);
		}

		/* Avoid humps at -40 dBm */
		val = CHSPEC_IS20(pi->radio_chanspec) ? 0x3108 : 0x2018;
		FOREACH_CORE(pi, core)
		    WRITE_PHYREGC(pi, Clip2Threshold, core, val);
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		/* Reduce the ldpc max iteration for 43684B0 and after
		   HW bug for BW160 ULOFDMA on register file overflow within LDPC decoder
		*/
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			MOD_PHYREG(pi, LDPCtermControl, max_iteration, 25);
		} else {
			MOD_PHYREG(pi, LDPCtermControl, max_iteration, 50);
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}
static void
wlc_phy_set_sfo_on_chan_change_acphy(phy_info_t *pi, uint8 ch)
{
	int fc = 0;
	const uint16 *val_ptr = NULL;
	const uint16 *val_ptr1 = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!TINY_RADIO(pi)) {

		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			const chan_info_radio20695_rffe_t *chan_info;
			fc = wlc_phy_chan2freq_20695(pi, ch, &chan_info);
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			const chan_info_radio20698_rffe_t *chan_info;
			fc = wlc_phy_chan2freq_20698(pi, ch, &chan_info);
		} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
			const chan_info_radio20704_rffe_t *chan_info;
			fc = wlc_phy_chan2freq_20704(pi, ch, &chan_info);
		} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
			const chan_info_radio20696_rffe_t *chan_info;
			fc = wlc_phy_chan2freq_20696(pi, ch, &chan_info);
		} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			const chan_info_radio20694_rffe_t *chan_info;
			fc = wlc_phy_chan2freq_20694(pi, ch, &chan_info);
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			chan_info_radio20697_rffe_t chan_info;
			fc = wlc_phy_chan2freq_20697(pi, ch, &chan_info);
		} else {
			const void *chan_info;
			fc = wlc_phy_chan2freq_acphy(pi, ch, &chan_info);
		}

		if (fc >= 0) {
			wlc_phy_write_sfo_params_acphy(pi, (uint16)fc);
		}
	} else {
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			const chan_info_radio20691_t *ci20691;

			fc = wlc_phy_chan2freq_20691(pi, ch, &ci20691);

			if (fc >= 0) {
				wlc_phy_write_sfo_params_acphy(pi, (uint16)fc);
			}
		} else if (ACMAJORREV_4(pi->pubpi->phy_rev) ||
				ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
			const chan_info_radio20693_pll_t *pll_tbl;
			const chan_info_radio20693_rffe_t *rffe_tbl;
			const chan_info_radio20693_pll_wave2_t *pll_tbl_wave2;

			if (phy_get_phymode(pi) != PHYMODE_80P80) {
				if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
					/* For First freq segment */
					uint8 chans[NUM_CHANS_IN_CHAN_BONDING];
					wf_chspec_get_80p80_channels(pi->radio_chanspec, chans);

					if (wlc_phy_chan2freq_20693(pi, chans[0], &pll_tbl,
							&rffe_tbl, &pll_tbl_wave2) >= 0) {
						val_ptr = &(pll_tbl_wave2->PHY_BW1a);
					}
					/* For second freq segment */
					if (wlc_phy_chan2freq_20693(pi, chans[1], &pll_tbl,
							&rffe_tbl, &pll_tbl_wave2) >= 0) {
						val_ptr1 = &(pll_tbl_wave2->PHY_BW1a);
					}
					if (val_ptr != NULL && val_ptr1 != NULL) {
					wlc_phy_write_sfo_80p80_params_acphy(pi, val_ptr, val_ptr1);
					} else {
						PHY_ERROR(("wl%d: %s: CFO/SFO settings fails!\n",
								pi->sh->unit, __FUNCTION__));
						ASSERT(0);
					}
				} else {
					fc = wlc_phy_chan2freq_20693(pi, ch, &pll_tbl, &rffe_tbl,
						&pll_tbl_wave2);
					if (fc >= 0) {
						if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
							ACMAJORREV_33(pi->pubpi->phy_rev)) {
							val_ptr = &(pll_tbl_wave2->PHY_BW1a);
							wlc_phy_write_sfo_params_acphy_wave2(pi,
								val_ptr);
						} else {
							wlc_phy_write_sfo_params_acphy(pi,
								(uint16)fc);
						}
#ifdef WL11ULB
						phy_ac_chanmgr_write_sfo_ulb_params_acphy(pi, fc);
#endif /* WL11ULB */
					}
				}
			} else {
				/* For First freq segment */
				ch = wf_chspec_primary80_channel(pi->radio_chanspec);
				fc = wlc_phy_chan2freq_20693(pi, ch, &pll_tbl, &rffe_tbl,
					&pll_tbl_wave2);
				if (fc >= 0) {
					if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
						ACMAJORREV_33(pi->pubpi->phy_rev)) {
						val_ptr = &(pll_tbl_wave2->PHY_BW1a);
						wlc_phy_write_sfo_params_acphy_wave2(pi, val_ptr);
					} else {
						wlc_phy_write_sfo_params_acphy(pi, (uint16)fc);
					}
				}
				/* For second freq segment */
				ch = wf_chspec_secondary80_channel(pi->radio_chanspec);
				if (wlc_phy_chan2freq_20693(pi, ch, &pll_tbl, &rffe_tbl,
					&pll_tbl_wave2) >= 0) {
					if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
						ACMAJORREV_33(pi->pubpi->phy_rev)) {
						val_ptr = &(pll_tbl_wave2->PHY_BW1a);
					} else {
						val_ptr = &(pll_tbl->PHY_BW1a);
					}
					wlc_phy_write_sfo_80p80_params_acphy(pi, val_ptr, NULL);
				}
			}
		}
	}

}

static void
wlc_phy_write_sfo_params_acphy(phy_info_t *pi, uint16 fc)
{
	uint16 phy_bw;
	uint32 tmp;
#define SFO_UNITY_FACTOR	2621440UL	/* 2.5 x 2^20 */

	/*
	 * sfo_chan_center_Ts20 = round([ fc-10e6  fc   fc+10e6] / 20e6 * 8), fc in Hz
	 *                      = round([$fc-10   $fc  $fc+10] * 0.4), $fc in MHz
	 */

	if (!TINY_RADIO(pi) && PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		/* BW1a */
		phy_bw = (((fc + 10) * 4) + 5) / 10;
		WRITE_PHYREG(pi, BW1a, phy_bw);

		/* BW3 */
		phy_bw = (((fc - 10) * 4) + 5) / 10;
		WRITE_PHYREG(pi, BW3, phy_bw);
	}

	/* BW2 */
	phy_bw = ((fc * 4) + 5) / 10;
	WRITE_PHYREG(pi, BW2, phy_bw);

	/*
	 * sfo_chan_center_factor = round(2^17 / ([fc-10e6 fc fc+10e6]/20e6)/(ten_mhz+1)), fc in Hz
	 *                        = round(2621440 ./ [$fc-10 $fc $fc+10]/($ten_mhz+1)), $fc in MHz
	 */

	if (!TINY_RADIO(pi) && PHYCORENUM(pi->pubpi->phy_corenum) > 1) {
		/* BW4 */
		tmp = fc + 10;
		phy_bw = (uint16)((SFO_UNITY_FACTOR + tmp / 2) / tmp);
		WRITE_PHYREG(pi, BW4, phy_bw);

		/* BW6 */
		tmp = fc - 10;
		phy_bw = (uint16)((SFO_UNITY_FACTOR + tmp / 2) / tmp);
		WRITE_PHYREG(pi, BW6, phy_bw);
	}

	/* BW5 */
	tmp = fc;
	phy_bw = (uint16)((SFO_UNITY_FACTOR + tmp / 2) / tmp);
	WRITE_PHYREG(pi, BW5, phy_bw);
}

static void
wlc_phy_write_sfo_params_acphy_wave2(phy_info_t *pi, const uint16 *val_ptr)
{
#ifdef WL11ULB
	if (PHY_ULB_ENAB(pi)) {
		if (CHSPEC_IS10(pi->radio_chanspec) ||
				CHSPEC_IS5(pi->radio_chanspec) ||
				CHSPEC_IS2P5(pi->radio_chanspec))
			return;
	}
#endif /* WL11ULB */

	ASSERT(val_ptr != NULL);
	if (val_ptr != NULL) {
		/* set SFO parameters */
		WRITE_PHYREG(pi, BW1a, val_ptr[0]);
		WRITE_PHYREG(pi, BW2, val_ptr[1]);
		WRITE_PHYREG(pi, BW3, val_ptr[2]);
		/* Set sfo_chan_center_factor */
		WRITE_PHYREG(pi, BW4, val_ptr[3]);
		WRITE_PHYREG(pi, BW5, val_ptr[4]);
		WRITE_PHYREG(pi, BW6, val_ptr[5]);
	}
}

static void
wlc_phy_write_sfo_80p80_params_acphy(phy_info_t *pi, const uint16 *val_ptr, const uint16 *val_ptr1)
{
	ASSERT(val_ptr != NULL);
	if (val_ptr != NULL) {
		if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ASSERT(val_ptr1 != NULL);
			/* set SFO parameters */
			WRITE_PHYREG(pi, BW1a0, val_ptr[0]);
			WRITE_PHYREG(pi, BW1a1, val_ptr[0]);
			WRITE_PHYREG(pi, BW1a2, val_ptr1[0]);
			WRITE_PHYREG(pi, BW1a3, val_ptr1[0]);
			WRITE_PHYREG(pi, BW20, val_ptr[1]);
			WRITE_PHYREG(pi, BW21, val_ptr[1]);
			WRITE_PHYREG(pi, BW22, val_ptr1[1]);
			WRITE_PHYREG(pi, BW23, val_ptr1[1]);
			WRITE_PHYREG(pi, BW30, val_ptr[2]);
			WRITE_PHYREG(pi, BW31, val_ptr[2]);
			WRITE_PHYREG(pi, BW32, val_ptr1[2]);
			WRITE_PHYREG(pi, BW33, val_ptr1[2]);
			/* Set sfo_chan_center_factor */
			WRITE_PHYREG(pi, BW40, val_ptr[3]);
			WRITE_PHYREG(pi, BW41, val_ptr[3]);
			WRITE_PHYREG(pi, BW42, val_ptr1[3]);
			WRITE_PHYREG(pi, BW43, val_ptr1[3]);
			WRITE_PHYREG(pi, BW50, val_ptr[4]);
			WRITE_PHYREG(pi, BW51, val_ptr[4]);
			WRITE_PHYREG(pi, BW52, val_ptr1[4]);
			WRITE_PHYREG(pi, BW53, val_ptr1[4]);
			WRITE_PHYREG(pi, BW60, val_ptr[5]);
			WRITE_PHYREG(pi, BW61, val_ptr[5]);
			WRITE_PHYREG(pi, BW62, val_ptr1[5]);
			WRITE_PHYREG(pi, BW63, val_ptr1[5]);
		} else {
			/* set SFO parameters */
			WRITE_PHYREG(pi, BW1a1, val_ptr[0]);
			WRITE_PHYREG(pi, BW21, val_ptr[1]);
			WRITE_PHYREG(pi, BW31, val_ptr[2]);
			/* Set sfo_chan_center_factor */
			WRITE_PHYREG(pi, BW41, val_ptr[3]);
			WRITE_PHYREG(pi, BW51, val_ptr[4]);
			WRITE_PHYREG(pi, BW61, val_ptr[5]);
		}
	}
}

#ifdef WL11ULB
static void
phy_ac_chanmgr_write_sfo_ulb_params_acphy(phy_info_t *pi, int freq)
{
	if (PHY_ULB_ENAB(pi)) {
		uint8 ulb_mode = PMU_ULB_BW_NONE;
		uint8 div = 1;

		if (CHSPEC_IS10(pi->radio_chanspec)) {
			ulb_mode = PMU_ULB_BW_10MHZ;
			div = 2;
		} else if (CHSPEC_IS5(pi->radio_chanspec)) {
			ulb_mode = PMU_ULB_BW_5MHZ;
			div = 4;
		} else if (CHSPEC_IS2P5(pi->radio_chanspec)) {
			ulb_mode = PMU_ULB_BW_2P5MHZ;
			div = 8;
		}

		if (ulb_mode == PMU_ULB_BW_NONE) {
			MOD_PHYREG(pi, PhaseTrackOffset, sfo_corr_ulb, 0x0);
		} else {
			MOD_PHYREG(pi, PhaseTrackOffset, sfo_corr_ulb, 0x1);
			WRITE_PHYREG(pi, BW8, freq * div * 4/10);
			WRITE_PHYREG(pi, BW9, (2621440 + freq*div/2) / (freq*div));
		}
	}
}
#endif /* WL11ULB */

static void
chanspec_set_primary_chan(phy_info_t * pi)
{
	if (CHSPEC_IS8080(pi->radio_chanspec) || CHSPEC_IS160(pi->radio_chanspec)) {
		/* 80P80 */
		uint16 param1 = 0;
		uint16 param2 = 0;

		switch (CHSPEC_CTL_SB(pi->radio_chanspec)) {
		case WL_CHANSPEC_CTL_SB_LLL:
			/* param1 initialized to 0 */
			/* param2 initialized to 0 */
			break;
		case WL_CHANSPEC_CTL_SB_LLU:
			/* param1 initialized to 0 */
			param2 = 1;
			break;
		case WL_CHANSPEC_CTL_SB_LUL:
			/* param1 initialized to 0 */
			param2 = 2;
			break;
		case WL_CHANSPEC_CTL_SB_LUU:
			/* param1 initialized to 0 */
			param2 = 3;
			break;
		case WL_CHANSPEC_CTL_SB_ULL:
			param1 = 1;
			/* param2 initialized to 0 */
			break;
		case WL_CHANSPEC_CTL_SB_ULU:
			param1 = 1;
			param2 = 1;
			break;
		case WL_CHANSPEC_CTL_SB_UUL:
			param1 = 1;
			param2 = 2;
			break;
		case WL_CHANSPEC_CTL_SB_UUU:
			param1 = 1;
			param2 = 3;
			break;
		default:
			PHY_ERROR(("%s: No primary channel settings for CTL_SB=%d\n",
			           __FUNCTION__, CHSPEC_CTL_SB(pi->radio_chanspec)));
			ASSERT(0);
		}
		MOD_PHYREG(pi, ClassifierCtrl_80p80, prim_sel_hi, param1);
		MOD_PHYREG(pi, ClassifierCtrl2, prim_sel, param2);

	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		/* 80mhz */
		uint16 param = 0;

		switch (CHSPEC_CTL_SB(pi->radio_chanspec)) {
		case WL_CHANSPEC_CTL_SB_LL:
			/* param initialized to 0 */
			break;
		case WL_CHANSPEC_CTL_SB_LU:
			param = 1;
			break;
		case WL_CHANSPEC_CTL_SB_UL:
			param = 2;
			break;
		case WL_CHANSPEC_CTL_SB_UU:
			param = 3;
			break;
		default:
			PHY_ERROR(("%s: No primary channel settings for CTL_SB=%d\n",
			           __FUNCTION__, CHSPEC_CTL_SB(pi->radio_chanspec)));
			ASSERT(0);
		}
		MOD_PHYREG(pi, ClassifierCtrl2, prim_sel, param);

	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		/* 40mhz */
		uint16 param;
		if (CHSPEC_SB_UPPER(pi->radio_chanspec)) {
			param = 1;
		} else {
			param = 0;
		}
		MOD_PHYREG(pi, RxControl, bphy_band_sel, param);
		MOD_PHYREG(pi, ClassifierCtrl2, prim_sel, param);

	} else if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
		/* 20mhz */
		MOD_PHYREG(pi, RxControl, bphy_band_sel, 0);
		MOD_PHYREG(pi, ClassifierCtrl2, prim_sel, 0);
	} else {
		PHY_ERROR(("%s: No primary channel settings for bw=%d\n",
		           __FUNCTION__, CHSPEC_BW(pi->radio_chanspec)));
		ASSERT(0);
	}
}

/*
 * making IIR filter gaussian like for BPHY to improve ACPR ACMajor 40
 */
static void
chanspec_make_iir_filter_gaussian_acmajor_40(phy_info_t *pi)
{
	uint8 bphy_testmode_val = (0x3F & READ_PHYREGFLD(pi, bphyTest, testMode));
	bphy_testmode_val = bphy_testmode_val |
		((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x2)  << 5);
	MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
	MOD_PHYREG(pi, bphyTest, FiltSel2,
		((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x4) >> 2));
	if ((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type == 0) ||
		(pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type == 2)) {
		wlc_phy_set_tx_iir_coeffs(pi, 1, 4);
		if (pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type == 0) {
			MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x2F);
		} else {
			MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x2A);
		}
	} else {
		wlc_phy_set_tx_iir_coeffs(pi, 1, 5);
		MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x37);
	}
}

/*
 * making IIR filter gaussian like for BPHY to improve ACPR ACMajor 36
 */
static void
chanspec_make_iir_filter_gaussian_acmajor_36(phy_info_t *pi)
{
	uint8 bphy_testmode_val = (0x3F & READ_PHYREGFLD(pi, bphyTest, testMode));
	bphy_testmode_val = bphy_testmode_val |
		((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x2)  << 5);
	MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
	MOD_PHYREG(pi, bphyTest, FiltSel2,
		((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x4) >> 2));
	/* Load filter with Gaussian shaping */
	wlc_phy_set_tx_iir_coeffs(pi, 1,
		(pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type));

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		wlc_phy_set_tx_iir_coeffs(pi, 0, pi->sromi->ofdmfilttype_2g);
	} else {
		wlc_phy_set_tx_iir_coeffs(pi, 0, pi->sromi->ofdmfilttype);
	}
	MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x3b);
}

/*
 * making IIR filter gaussian like for BPHY to improve ACPR ACMajor 44
 */
static void
chanspec_make_iir_filter_gaussian_acmajor_44(phy_info_t *pi)
{
	/* phy_maj44 main slice supports only 5g, so disabling bphy register writes. */
	if (pi->pubpi->slice == DUALMAC_AUX) {
		uint16 bphy_testmode_val;
		bphy_testmode_val =
		        (0x3F & READ_PHYREGFLD(pi, bphyTest, testMode));
		bphy_testmode_val = bphy_testmode_val |
		        ((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type &
		        0x2) << 5);
		MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
		MOD_PHYREG(pi, bphyTest, FiltSel2,
			((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type &
		        0x4) >> 2));
		if ((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type == 0) ||
			(pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type == 2)) {
			wlc_phy_set_tx_iir_coeffs(pi, 1, 4);
			if (pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type ==
			        0) {
				MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x2F);
			} else {
				MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x2A);
			}

		} else {
			wlc_phy_set_tx_iir_coeffs(pi, 1, 5);
			MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, 0x37);
		}
	}
}

/*
 * making IIR filter gaussian like for BPHY to improve ACPR ACMajor 4
 */
static void
chanspec_make_iir_filter_gaussian_acmajor_4(phy_info_t *pi, int fc)
{

	{
		uint16 bphy_testmode_val;
		uint16 param2;
		if (!PHY_IPA(pi) && !ROUTER_4349(pi)) {
			bphy_testmode_val = (0x3F & READ_PHYREGFLD(pi, bphyTest, testMode));
			param2 = 0u;
		} else {
			bphy_testmode_val = 0u;
			param2 = 1u;
		}
		MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
		MOD_PHYREG(pi, bphyTest, FiltSel2, param2);
	}

	wlc_phy_set_tx_iir_coeffs(pi, 1, pi->sromi->cckfilttype);
	wlc_phy_set_tx_iir_coeffs(pi, 0, 0); /* default setting for ofdm */

	{
		uint16 bphy_scale_val;
		if (!PHY_IPA(pi) && !ROUTER_4349(pi)) {
			bphy_scale_val = 0x4Du;
		} else if (ROUTER_4349(pi)) {
			/* bphyScale to equalize gain between cck and ofdm frames */
			bphy_scale_val = 0x23u;
		} else {
			bphy_scale_val = 0x3Bu;
		}
		MOD_PHYREG(pi, BphyControl3, bphyScale20MHz, bphy_scale_val);
	}

	if (((fc == 2412) || (fc == 2462) || (fc == 2467) ||
	        (fc == 2472)) &&
		(pi->sromi->ofdmfilttype_2g != 127)) {
		wlc_phy_set_tx_iir_coeffs(pi, 0,
		        pi->sromi->ofdmfilttype_2g);
	} else if (((fc == 5240) || (fc == 5260) || (fc == 5580) ||
	        (fc == 5660)) &&
		(pi->sromi->ofdmfilttype != 127)) {
		wlc_phy_set_tx_iir_coeffs(pi, 0, pi->sromi->ofdmfilttype);
	}
}

/*
 * making IIR filter gaussian like for BPHY to improve ACPR
 */
static void
chanspec_make_iir_filter_gaussian(phy_info_t *pi, int fc)
{
	/* making IIR filter gaussian like for BPHY to improve ACPR */

	/* set RRC filter alpha
	 FiltSel2 is 11 bit which msb, bphyTest's 6th bit is lsb
	 These 2 bits control alpha
	 bits 11 & 6    Resulting filter
	  -----------    ----------------
	      00         alpha=0.35 - default
	      01         alpha=0.75 - alternate
	      10         alpha=0.2  - for use in Japan on channel 14
	      11         no TX filter
	*/

	uint8 cck_filt_type;
	uint8 bphy_testmode_val = (0x3F & READ_PHYREGFLD(pi, bphyTest, testMode));

	if ((fc == 2484) && (!CHSPEC_IS8080(pi->radio_chanspec))) {
		MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
		MOD_PHYREG(pi, bphyTest, FiltSel2, 1);
		/* Load default filter */
		wlc_phy_set_tx_iir_coeffs(pi, 1, 0);
	} else {
		if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, bphyTest, FiltSel2, 0);
			if (PHY_IPA(pi)) {
				wlc_phy_set_tx_iir_coeffs(pi, 1, 2);
			} else {
				wlc_phy_set_tx_iir_coeffs(pi, 1, 1);
			}
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		           ACMAJORREV_33(pi->pubpi->phy_rev) ||
		           ACMAJORREV_37(pi->pubpi->phy_rev) ||
		           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			cck_filt_type = 2;
			bphy_testmode_val |= (cck_filt_type & 0x2)  << 5;
			MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
			MOD_PHYREG(pi, bphyTest, FiltSel2, 0);
			wlc_phy_set_tx_iir_coeffs(pi, 1, cck_filt_type);
			if (fc == 2472) {
				/* Narrower filter for ch13 - ofdm */
				wlc_phy_set_tx_iir_coeffs(pi, 0, 4);
			}
		} else if ACMAJORREV_3(pi->pubpi->phy_rev) {
			MOD_PHYREG(pi, bphyTest, testMode, 0);
			MOD_PHYREG(pi, bphyTest, FiltSel2, 0);
		} else if ACMAJORREV_4(pi->pubpi->phy_rev) {
			chanspec_make_iir_filter_gaussian_acmajor_4(pi, fc);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			chanspec_make_iir_filter_gaussian_acmajor_36(pi);
		} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			chanspec_make_iir_filter_gaussian_acmajor_40(pi);
		} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
			chanspec_make_iir_filter_gaussian_acmajor_44(pi);
		} else {
			bphy_testmode_val = bphy_testmode_val |
				((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x2)  << 5);
			MOD_PHYREG(pi, bphyTest, testMode, bphy_testmode_val);
			MOD_PHYREG(pi, bphyTest, FiltSel2,
				((pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x4) >> 2));
			/* Load filter with Gaussian shaping */
			wlc_phy_set_tx_iir_coeffs(pi, 1,
				(pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0x1));
		}
		if (ACMAJORREV_1(pi->pubpi->phy_rev) && PHY_IPA(pi)) {
			MOD_PHYREG(pi, bphyTest, testMode, 0);
			MOD_PHYREG(pi, bphyTest, FiltSel2, 0);
			wlc_phy_set_tx_iir_coeffs(pi, 1,
				(pi->u.pi_acphy->chanmgri->acphy_cck_dig_filt_type & 0xF));
		}
	}
}

static void
wlc_phy_farrow_setup_nontiny(phy_info_t * pi)
{
	phy_ac_radio_data_t *radio_data = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);
	uint8 use_ovr = ISSIM_ENAB(pi->sh->sih) ? 1 : 0;

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		if (pi->pubpi->slice == DUALMAC_MAIN) {
			wlc_phy_farrow_setup_28nm(pi, 1 /* DAC rate mode */ );
		} else {
			wlc_phy_farrow_setup_28nm_ulp(pi, radio_data->ulp_tx_mode,
				radio_data->ulp_adc_mode);
		}
	} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		wlc_phy_farrow_setup_28nm(pi, 1 /* DAC rate mode */ );
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		wlc_phy_farrow_setup_28nm(pi, 1 /* DAC rate mode */ );
		/* Configure AFE div */
		wlc_phy_radio20698_afe_div_ratio(pi, use_ovr, 0, 0);
	} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		wlc_phy_farrow_setup_28nm(pi, 1 /* DAC rate mode */ );
		/* Configure AFE div */
		wlc_phy_radio20704_afe_div_ratio(pi, use_ovr);
	} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		wlc_phy_farrow_setup_28nm(pi, 1 /* DAC rate mode */ );
		/* Configure AFE div */
		wlc_phy_radio20696_afe_div_ratio(pi);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		wlc_phy_farrow_setup_28nm_ulp(pi, radio_data->ulp_tx_mode,
			radio_data->ulp_adc_mode);
		/* Configure AFE div */
		wlc_phy_radio20695_afe_div_ratio(pi, radio_data->ulp_tx_mode,
			radio_data->ulp_adc_mode);
	} else {
		wlc_phy_farrow_setup_acphy(pi, pi->radio_chanspec);
	}
}

/*
 * JIRA (HW11ACRADIO-30)
 *
 * clamp_en needs to be high for ~1us for clipped pkts (80mhz)
 */
static void
war_jira_hw11acradio_30(phy_info_t * pi, int fc)
{
	uint8 core = 0;

	if ((CHSPEC_IS80(pi->radio_chanspec) ||
	     (CHSPEC_IS40(pi->radio_chanspec) && (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
	      !PHY_IPA(pi) && (fc == 5190))) &&
	    !TINY_RADIO(pi) &&
	    !ACMAJORREV_GE37(pi->pubpi->phy_rev)) {

		uint16 rfseq_bundle_adcrst48[3];
		uint16 rfseq_bundle_adcrst49[3];
		uint16 rfseq_bundle_adcrst50[3];
		uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;

		BCM_REFERENCE(phyrxchain);
		FOREACH_ACTV_CORE(pi, phyrxchain, core) {
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_clamp_en, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_clamp_en, 1);
		}

		rfseq_bundle_adcrst48[2]  = 0;
		rfseq_bundle_adcrst49[2]  = 0;
		rfseq_bundle_adcrst50[2]  = 0;
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			rfseq_bundle_adcrst48[0] = 0xef52;
			rfseq_bundle_adcrst48[1] = 0x94;
			rfseq_bundle_adcrst49[0] = 0xef42;
			rfseq_bundle_adcrst49[1] = 0x84;
			rfseq_bundle_adcrst50[0] = 0xef52;
			rfseq_bundle_adcrst50[1] = 0x84;
		} else if (! CHSPEC_IS40(pi->radio_chanspec) ||
				((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) && !PHY_IPA(pi) &&
				(fc == 5190))) {
			rfseq_bundle_adcrst48[0] = 0x0fd2;
			rfseq_bundle_adcrst48[1] = 0x96;
			rfseq_bundle_adcrst49[0] = 0x0fc2;
			rfseq_bundle_adcrst49[1] = 0x86;
			rfseq_bundle_adcrst50[0] = 0x0fd2;
			rfseq_bundle_adcrst50[1] = 0x86;
		} else {
			rfseq_bundle_adcrst48[0] = 0x4f52;
			rfseq_bundle_adcrst48[1] = 0x94;
			rfseq_bundle_adcrst49[0] = 0x4f42;
			rfseq_bundle_adcrst49[1] = 0x84;
			rfseq_bundle_adcrst50[0] = 0x4f52;
			rfseq_bundle_adcrst50[1] = 0x84;
		}

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 48, 48,
		                          rfseq_bundle_adcrst48);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 49, 48,
		                          rfseq_bundle_adcrst49);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 50, 48,
		                          rfseq_bundle_adcrst50);
		/* reduce the adc reset time from 250ns to 50ns for 43602 as it caused CSTR failure
		* when ADC clips during clip gain
		*/
		if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
			rf_updh_dly_adcrst[4] = 0x2;
			rf_updl_dly_adcrst[4] = 0x2;
		}
		/* updategainH : issue adc reset for 250ns */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x30, 16, rf_updh_cmd_adcrst);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xa0, 16, rf_updh_dly_adcrst);

		/* updategainL : issue adc reset for 250ns */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x40, 16, rf_updl_cmd_adcrst);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xb0, 16, rf_updl_dly_adcrst);

		/* updategainU : issue adc reset for 250n */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x50, 16, rf_updu_cmd_adcrst);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xc0, 16, rf_updu_dly_adcrst);
	} else {
		if (!ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMAJORREV_33(pi->pubpi->phy_rev)) {
			uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
			BCM_REFERENCE(phyrxchain);
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {
				/*
				 * 4360A0 : SD-ADC was not monotonic for 1st revision
				 * but is fixed now
				 */
				if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
					MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core,
						afe_iqadc_clamp_en, 0);
				} else {
					MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core,
						afe_iqadc_clamp_en, 1);
				}
				MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_clamp_en, 1);
			}

			if (pi->pubpi->phy_rev < 40) {
				/* updategainH : increase clamp_en off delay to 16 */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x30,
				16, rf_updh_cmd_clamp);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xa0,
				16, rf_updh_dly_clamp);

				/* updategainL : increase clamp_en off delay to 16 */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x40,
				16, rf_updl_cmd_clamp);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xb0,
				16, rf_updl_dly_clamp);

				/* updategainU : increase clamp_en off delay to 16 */
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x50,
				16, rf_updu_cmd_clamp);
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xc0,
				16, rf_updu_dly_clamp);
			}
		}
	}
}

/*
 * AFE clk and Harmonic of 40 MHz crystal causes a spur at 417 Khz
 */
static void
war_4350_spur_417mhz(phy_info_t * pi, int fc)
{
	uint32 rx_afediv_sel, tx_afediv_sel;
	uint32 read_val[2], write_val[2];
	phy_ac_radio_data_t *radio_data = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);

	/* AFE clk and Harmonic of 40 MHz crystal causes a spur at 417 Khz */
	if (CHSPEC_IS20(pi->radio_chanspec)) {
		if ((radio_data->srom_txnospurmod2g == 0) &&
			(CHSPEC_IS2G(pi->radio_chanspec)) && !PHY_IPA(pi)) {
			/* AFE divider of 4.5 */
			/* i_iqadc_adc_bias 2 */
			/* iqmode 20 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
			                         &read_val);
			rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                ~(0x3) & 0xfffff) |
				(0x2 << 14 | 0x2 << 11 | 0x3);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
			                         &read_val);
			tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                 ~(0x3) & 0xfffff) |
				(0x2 << 14 | 0x2 << 11 | 0x3);
		} else if ((CHSPEC_IS2G(pi->radio_chanspec) &&
			((fc != 2412 && fc != 2467) || (pi->xtalfreq == 40000000) ||
			(ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) ||
			ACMINORREV_3(pi) ||
			ACMINORREV_5(pi)) &&
			pi->xtalfreq == 37400000 && PHY_ILNA(pi)))) ||
			(fc == 5745) || (fc == 5765) || (fc == 5825 && !PHY_IPA(pi)) ||
			((fc == 5180) && ((((RADIOMINORREV(pi) == 4) ||
			(RADIOMINORREV(pi) == 10) ||
			(RADIOMINORREV(pi) == 11) ||
			(RADIOMINORREV(pi) == 13)) &&
			(pi->sh->chippkg == 2)) ||
			(RADIOMINORREV(pi) == 7) ||
			(RADIOMINORREV(pi) == 9) ||
			(RADIOMINORREV(pi) == 8) ||
			(RADIOMINORREV(pi) == 12)) &&
			(pi->xtalfreq == 37400000))) {
			/* if AFE divider of 8 is used for 20 MHz channel 149,153, */
			/* or any channel in 2GHz when xtalfreq=40MHz, */
			/* or any 2Ghz channel except 2467 when xtalfreq=37.4MHz */
			/* so change divider ratio to 9 */
			/* i_iqadc_adc_bias 2 */
			/* iqmode 20 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
			                         &read_val);
			rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                 ~(0x3) & 0xfffff) |
				(0x4 << 14 | 0x2 << 11 | 0x3);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
			                         &read_val);
			tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                 ~(0x3) & 0xfffff) |
				(0x4 << 14 | 0x2 << 11 | 0x3);
		} else {
			/* AFE divider of 8 */
			/* i_iqadc_adc_bias 2 */
			/* iqmode 20 */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
			                         &read_val);
			rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                 ~(0x3) & 0xfffff) |
				(0x3 << 14 | 0x2 << 11 | 0x3);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
			                         &read_val);
			tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
			                 ~(0x3) & 0xfffff) |
				(0x3 << 14 | 0x2 << 11 | 0x3);
		}
		if (!PHY_IPA(pi) && CHSPEC_IS5G(pi->radio_chanspec)) {
			if ((((fc == 5180) && (pi->sh->chippkg != 2)) ||
			     ((fc >= 5200) && (fc <= 5320)) ||
			     ((fc >= 5745) && (fc <= 5825)))) {
				/* AFE div 5 for tx/rx  (bits 13:15) */
				/* i_iqadc_adc_bias 0 (bits 11:12) for stability */
				/* iqmode 40 (bits 0:2) to fix TSSI issue */
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6,
				                         60, &read_val);
				rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
				                 ~(0x3) & 0xfffff) |
					(0x7 << 14 | 0x0 << 11 | 0x2);
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0,
				                         60, &read_val);
				tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
				                 ~(0x3) & 0xfffff) |
					(0x7 << 14 | 0x0 << 11 | 0x2);
			} else {
				/* AFE div 8 for tx/rx */
				/* i_iqadc_adc_bias 2 */
				/* iqmode 20 */
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6,
				                         60, &read_val);
				rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
				                 ~(0x3) & 0xfffff) |
					(0x3 << 14 | 0x2 << 11 | 0x3);
				wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0,
				                         60, &read_val);
				tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & ~(0x3 << 11) &
				                 ~(0x3) & 0xfffff) |
					(0x3 << 14 | 0x2 << 11 | 0x3);
			}
		}
		/* RX_SD_ADC_PU_VAL bw20 */
		write_val[0] = ((rx_afediv_sel & 0xfff) << 20) | rx_afediv_sel;
		write_val[1] = (rx_afediv_sel << 8) | (rx_afediv_sel >> 12);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 6, 60,
		                          write_val);
		/* bw20_HighspeedMode1 */
		write_val[0] = ((tx_afediv_sel & 0xfff) << 20) | tx_afediv_sel;
		write_val[1] = (tx_afediv_sel << 8) | (tx_afediv_sel >> 12);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 0, 60,
		                          write_val);
		wlc_phy_force_rfseq_noLoleakage_acphy(pi);
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		/* if AFE divider of 4 is used for 40 MHz channel 151m,
		 * so change divider ratio to 4.5
		 */
		if (CHSPEC_IS5G(pi->radio_chanspec) &&
		    !PHY_IPA(pi) && (fc != 5190)) {
			/* AFE div 5 for tx/rx */
			rx_afediv_sel = (sdadc_cfg40 & ~(0x7 << 14) & 0xfffff) |
				(0x7 << 14) | (0x3 << 17);
			tx_afediv_sel = (sdadc_cfg40 & ~(0x7 << 14) & 0xfffff) |
				(0x7 << 14);
		} else if (CHSPEC_IS5G(pi->radio_chanspec) &&
		           !PHY_IPA(pi) && (fc == 5190)) {
			/* AFE div 3 for tx/rx, bw80 mode */
			rx_afediv_sel = (sdadc_cfg80 & ~(0x7 << 14) & 0xfffff) |
				(0x0 << 14) | (0x3 << 17);
			tx_afediv_sel = (sdadc_cfg80 & ~(0x7 << 14) & 0xfffff) |
				(0x0 << 14);
		} else if ((CHSPEC_IS2G(pi->radio_chanspec)) || (fc == 5755) ||
			(fc == 5550 && pi->xtalfreq == 40000000) ||
			(fc == 5310 && pi->xtalfreq == 37400000 && PHY_IPA(pi))) {
			/* AFE div 4.5 for tx/rx */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 7, 60,
			                         &read_val);
			rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & 0xfffff) |
				(0x2 << 14);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 1, 60,
			                         &read_val);
			tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & 0xfffff) |
			        (0x2 << 14);
		} else {
			/* AFE div 4 for tx/rx */
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 7, 60,
			                         &read_val);
			rx_afediv_sel = (read_val[0] & ~(0x7 << 14) & 0xfffff) |
				(0x1 << 14);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 1, 60,
			                         &read_val);
			tx_afediv_sel = (read_val[0] & ~(0x7 << 14) & 0xfffff) |
				(0x1 << 14);
		}
		/* RX_SD_ADC_PU_VAL bw40 */
		write_val[0] = ((rx_afediv_sel & 0xfff) << 20) | rx_afediv_sel;
		write_val[1] = (rx_afediv_sel << 8) | (rx_afediv_sel >> 12);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 7, 60,
		                          write_val);
		/* bw40_HighspeedMode1 */
		write_val[0] = ((tx_afediv_sel & 0xfff) << 20) | tx_afediv_sel;
		write_val[1] = (tx_afediv_sel << 8) | (tx_afediv_sel >> 12);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 1, 60,
		                          write_val);
		wlc_phy_force_rfseq_noLoleakage_acphy(pi);
	}
}

static void
chanspec_setup_regtbl_on_chan_change(phy_info_t *pi)
{
	bool suspend = 0;
	uint8 stall_val = 0, orig_rxfectrl1 = 0;
	uint8 ch[NUM_CHANS_IN_CHAN_BONDING];
	uint8 core = 0;

	phy_ac_radio_data_t *radio_data = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);

	/* get the center freq */
	int fc = radio_data->fc;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* get the operating channels */
	chanspec_get_operating_channels(pi, ch);

	/* -ve freq means channel not found in tuning table */
	if (fc < 0) {
		return;
	}

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);

	/* Setup the Tx/Rx Farrow resampler */
	if (TINY_RADIO(pi)) {
		wlc_phy_farrow_setup_tiny(pi, pi->radio_chanspec);
	} else {
		wlc_phy_farrow_setup_nontiny(pi);
	}

	/* Load Pdet related settings */
	wlc_phy_set_pdet_on_reset_acphy(pi);

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev)) {
		suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* Disable stalls and hold FIFOs in reset */
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);

		ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	}

	/* 4350A0 radio */
	if ((RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) &&
		(RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) &&
		!(ISSIM_ENAB(pi->sh->sih))) {
		war_4350_spur_417mhz(pi, fc);
	}

	/* JIRA (HW11ACRADIO-30) - clamp_en needs to be high for ~1us for clipped pkts (80mhz) */
	war_jira_hw11acradio_30(pi, fc);

	if (ACMAJORREV_2(pi->pubpi->phy_rev) && !(PHY_IPA(pi))) {
		uint16 rfseq_bundle_adcdacoff51[3];
		/* Add AFE Power down to RFSeq */

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			rfseq_bundle_adcdacoff51[0] = 0xef72;
			rfseq_bundle_adcdacoff51[1] = 0x84;
		} else {
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				if (((fc >= 5180) && (fc <= 5320)) ||
				((fc >= 5745) && (fc <= 5825))) {
					rfseq_bundle_adcdacoff51[0] = 0x8f72;
					rfseq_bundle_adcdacoff51[1] = 0x84;
				} else {
					rfseq_bundle_adcdacoff51[0] = 0xef72;
					rfseq_bundle_adcdacoff51[1] = 0x84;
				}
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				if (fc == 5190) {
					rfseq_bundle_adcdacoff51[0] = 0x0ff2;
					rfseq_bundle_adcdacoff51[1] = 0x86;
				} else {
					rfseq_bundle_adcdacoff51[0] = 0x8f72;
					rfseq_bundle_adcdacoff51[1] = 0x84;
				}
			} else {
				rfseq_bundle_adcdacoff51[0] = 0x0ff2;
				rfseq_bundle_adcdacoff51[1] = 0x86;
			}
		}
		/* Below bundle shuts off all DACs at the beginning of TX2RX sequence */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 51, 48,
		rfseq_bundle_adcdacoff51);

		wlc_phy_set_sdadc_pd_val_per_core_acphy(pi);
	}

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);

		/* Restore FIFO reset and Stalls */
		ACPHY_ENABLE_STALL(pi, stall_val);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);
	}

	wlc_phy_set_sfo_on_chan_change_acphy(pi, ch[0]);

	/* Set the correct primary channel */
	chanspec_set_primary_chan(pi);

	/* set aci thresholds */
	wlc_phy_set_aci_regs_acphy(pi);

	/* making IIR filter gaussian like for BPHY to improve ACPR */
	chanspec_make_iir_filter_gaussian(pi, fc);

	/* if it's 2x2 or 3x3 design, populate the reciprocity compensation coeff */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
			wlc_phy_populate_recipcoeffs_acphy(pi);
	}

	#ifndef MACOSX
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev))
		wlc_phy_populate_recipcoeffs_acphy(pi);
	#endif /* MACOSX */

	/* 4354 wlipa 2GHz xtal spur war */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi) ||
		ACMINORREV_3(pi)) && PHY_ILNA(pi)) {
		ACPHY_REG_LIST_START
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL2, xtal_pu_RCCAL1, 0)
			MOD_RADIO_REG_ENTRY(pi, RFP, GE16_OVR27, ovr_xtal_outbufBBstrg, 1)
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL4, xtal_outbufBBstrg, 0)
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL4, xtal_outbufcalstrg, 0)
			MOD_RADIO_REG_ENTRY(pi, RFP, GE16_OVR27, ovr_xtal_outbufstrg, 1)
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL4, xtal_outbufstrg, 2)
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL5, xtal_sel_BT, 1)
			MOD_RADIO_REG_ENTRY(pi, RFP, PLL_XTAL5, xtal_bufstrg_BT, 2)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	if (BFCTL(pi->u.pi_acphy) == 3) {
		if (fc == 5180 || fc == 5190 || fc == 5310 ||
		    fc == 5320 || fc == 5500 || fc == 5510) {
			MOD_RADIO_REG(pi, RFP, PLL_CP4, rfpll_cp_ioff, 0xA0);
		}
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		femctrl_clb_majrev_ge40(pi,
			pi->u.pi_acphy->chanmgri->data->curr_band2g,
			wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_AUX ? 1 : 0);
	}

	/* WAR for 4365 */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		MOD_RADIO_ALLREG_20693(pi, RX_TOP_5G_OVR, ovr_lna5g_nap, 1);
		MOD_RADIO_ALLREG_20693(pi, LNA5G_CFG2, lna5g_nap, 0);
		MOD_RADIO_ALLREG_20693(pi, RX_TOP_2G_OVR_EAST, ovr_lna2g_nap, 1);
		MOD_RADIO_ALLREG_20693(pi, LNA2G_CFG2, lna2g_nap, 0);
	}

	if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* tdsfo eco is applicable only for main slice */
		if ((pi->pubpi->slice == DUALMAC_MAIN) && (HW_ACMINORREV(pi) == 0)) {
			pi->u.pi_acphy->td_sfo_corr_en = 1;
			wlc_phy_td_sfo_eco(pi);
		}
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 2)) {
		/* Enable TD-SFO for phybw = 80M or 160M */
		if (CHSPEC_IS80(pi->radio_chanspec) || CHSPEC_IS160(pi->radio_chanspec)) {
			pi->u.pi_acphy->td_sfo_corr_en = 2;
		} else {
			pi->u.pi_acphy->td_sfo_corr_en = 0;
		}
		wlc_phy_td_sfo_eco(pi);
	}

	/* 53573A0: 5G Tx Low Power Radio Optimizations */
	/* proc 20693_tx5g_set_bias_ipa_opt_sv */
	if (ROUTER_4349(pi) && CHSPEC_IS5G(pi->radio_chanspec)) {
		if (PHY_IPA(pi)) {
			FOREACH_CORE(pi, core)
			{
				bool isB0, isB0_BW40, isB0_BW80;
				bool isACR = (((pi->sh->boardrev >> 12) & 0xF) == 2);
				isB0 = (RADIO20693REV(pi->pubpi->radiorev) == 23);
				isB0_BW40 = (RADIO20693REV(pi->pubpi->radiorev) == 23) &&
					(CHSPEC_IS40(pi->radio_chanspec));
				isB0_BW80 = (RADIO20693REV(pi->pubpi->radiorev) == 23) &&
				(CHSPEC_IS80(pi->radio_chanspec) ||
				 CHSPEC_IS8080(pi->radio_chanspec));

				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core,
					ovr_trsw5g_pu, 0x1);
				MOD_RADIO_REG_20693(pi, TRSW5G_CFG1, core,
					trsw5g_pu, 0x0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG7, core,
					swcap_sec_gate_off_5g, 0xf);
				MOD_RADIO_REG_20693(pi, SPARE_CFG7, core,
					swcap_sec_sd_on_5g, 0x10);
				MOD_RADIO_REG_20693(pi, SPARE_CFG6, core,
					swcap_pri_pd_5g, 0x1);
				MOD_RADIO_REG_20693(pi, SPARE_CFG6, core,
					swcap_pri_5g, 0x0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG6, core,
					swcap_pri_gate_off_5g, 0x0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG6, core,
					swcap_pri_sd_on_5g, 0x0);

				MOD_RADIO_REG_20693(pi, PA5G_CFG3, core,
					pa5g_ptat_slope_main, 0x0);

				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
					ovr_pa5g_idac_incap_compen_main, 0x1);
				MOD_RADIO_REG_20693(pi, PA5G_INCAP, core,
					pa5g_idac_incap_compen_main,
					(isACR && (isB0_BW40 || isB0_BW80))? 0xc :
					(isACR && isB0)? 0x3a : 0x16);
				MOD_RADIO_REG_20693(pi, PA5G_IDAC3, core,
					pa5g_idac_tuning_bias, 0x0);

				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG8, core, pad5g_idac_gm,
					(isACR && (isB0_BW40 || isB0_BW80)) ? 0x3f :
					(isACR && isB0) ? 0x26 :
					(isB0) ? 0x1a : 0x18);
				MOD_RADIO_REG_20693(pi, TXGM5G_CFG1, core, pad5g_idac_cascode,
					(isACR && (isB0_BW40 || isB0_BW80)) ? 0xf :
					(isACR && isB0) ? 0xe :
					(isB0) ? 0xd : 0xb);
				MOD_RADIO_REG_20693(pi, SPARE_CFG10, core,
					pad5g_idac_cascode2, 0x0);
				MOD_RADIO_REG_20693(pi, PA5G_INCAP, core, pad5g_idac_pmos,
					(isACR && (isB0_BW40 || isB0_BW80)) ? 0xa : 0x1c);

				MOD_RADIO_REG_20693(pi, PA5G_IDAC3, core,
					pad5g_idac_tuning_bias, 0xd);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG6, core,
					mx5g_ptat_slope_cascode, 0x0);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG6, core,
					mx5g_ptat_slope_lodc, 0x2);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG4, core,
					mx5g_idac_cascode, 0xf);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG5, core,
					mx5g_idac_lodc, 0xa);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG5, core, mx5g_idac_bbdc,
					(isB0_BW40 && isACR) ? 0x2 : (isB0)? 0xc : 0x8);

				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
					ovr_pa5g_idac_main, 1);
				MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR3, core,
					ovr_pa5g_idac_cas, 1);

				if (isB0_BW80) {
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_main, (isACR) ? 0x38 : 0x3a);
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_cas, (isACR) ? 0x14 : 0x15);
				} else if (isB0_BW40) {
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_main, (isACR) ? 0x34 : 0x28);
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_cas, (isACR) ? 0x14 : 0x13);
				} else if (isB0) {
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_main, (isACR) ? 0x20 : 0x1a);
					MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
						pa5g_idac_cas, (isACR) ? 0x12 : 0x11);
				} else {
					if (CHSPEC_IS40(pi->radio_chanspec) ||
						CHSPEC_IS80(pi->radio_chanspec) ||
						CHSPEC_IS8080(pi->radio_chanspec)) {
						/* if this 'current-boosting' tuning is adopted,
						 * remember to increase PAPD cal index (65)
						 * since the RF gain is significantly boosted.
						 */
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_main, 0x2c);
						MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
							pa5g_idac_cas, 0x16);
					} else {
						/* 20Mhz */
						if (fc >= 5745) {
							MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
								pa5g_idac_main, 0x14);
							MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
								pa5g_idac_cas, 0x11);
						} else {
							MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
								pa5g_idac_main, 0x20);
							MOD_RADIO_REG_20693(pi, PA5G_IDAC1, core,
								pa5g_idac_cas, 0x14);
						}
					}
				}

				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG3, core,
					mx5g_pu_bleed, 0x0);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG1, core,
					mx5g_idac_bleed_bias, 0x0);
				MOD_RADIO_REG_20693(pi, TXMIX5G_CFG4, core,
					mx5g_idac_tuning_bias, 0xd);
			}
		}

		/* EVM Ramp: TxBias5G & Pad5G on */
		FOREACH_CORE(pi, core)
		{
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core, ovr_tx5g_bias_pu, 0x1);
			MOD_RADIO_REG_20693(pi, TX5G_CFG1, core, tx5g_bias_pu, 0x1);
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR3, core, ovr_pad5g_bias_cas_pu, 0x1);
			MOD_RADIO_REG_20693(pi, TX5G_CFG1, core, pad5g_bias_cas_pu, 0x1);
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core, ovr_pad5g_pu, 0x1);
			MOD_RADIO_REG_20693(pi, TX5G_CFG1, core, pad5g_pu, 0x1);
		}
	}

	if (ROUTER_4349(pi) && PHY_IPA(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
		FOREACH_CORE(pi, core)
		{
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
					ovr_pa2g_idac_main, 0x1);
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
					ovr_pa2g_idac_cas, 0x1);
			if (CHSPEC_IS40(pi->radio_chanspec)) {
				MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
						pa2g_idac_main, 0x1e);
				MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
						pa2g_idac_cas, 0x21);
			} else {
				MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
						pa2g_idac_main, 0x19);
				MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
						pa2g_idac_cas, 0x24);
			}

			MOD_RADIO_REG_20693(pi, SPARE_CFG10, core,
					pa2g_incap_pmos_src_sel_gnd, 0x0);
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
					ovr_pa2g_idac_incap_compen_main, 0x1);
			MOD_RADIO_REG_20693(pi, PA2G_INCAP, core,
					pa2g_idac_incap_compen_main, 0x34);
		}
	}

	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

static void
wlc_phy_set_reg_on_reset_acphy_20691(phy_info_t *pi)
{
	uint16 temp_reg;
	uint32 datapath = pi->u.pi_acphy->sromi->dot11b_opts;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM20691_ID));

	MOD_RADIO_REG_20691(pi, SPARE_CFG1, 0, spare_0, 0xfc00);
	MOD_RADIO_REG_20691(pi, SPARE_CFG2, 0, spare_1, 0x003f);

	if (ACPHY_ENABLE_FCBS_HWACI(pi))
		MOD_PHYREG(pi, FastChanSW_PLLVCOARBITR, arbitrdisable, 1);

	/* CRDOT11ACPHY-566: rx fix for dac rate mode 2 & 3 for >= rev1
	 * i.e. clear the top bit of the work_around_ctrl ACPHY register
	 */
	ACPHY_REG_LIST_START
		MOD_PHYREG_RAW_ENTRY(pi, ACPHY_work_around_ctrl(pi->pubpi->phy_rev), 0x8000, 0)

		MOD_PHYREG_ENTRY(pi, RxStatPwrOffset0, use_gainVar_for_rssi0, 1)
		MOD_PHYREG_ENTRY(pi, ForcePktAbort, dcblk_hpf_bw_en, 1)
		MOD_PHYREG_ENTRY(pi, HTAGCWaitCounters, HTAgcPktgainWait, 34)

		/* CRDOT11ACPHY-601: BPHY-20in20 Tapping via Datapath DC Filter */
		MOD_PHYREG_ENTRY(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, 0)
		MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq1, 1)
		MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq2, 0)
		MOD_PHYREG_ENTRY(pi, bphyTest, dccomp, 0)

		/* maximum drive strength */
		MOD_RADIO_REG_20691_ENTRY(pi, TIA_CFG8, 0, tia_offset_comp_drive_strength, 1)

		/* DCC FSM Defaults */
		MOD_PHYREG_ENTRY(pi, BBConfig, dcc_wakeup_restart_en, 0)
		MOD_PHYREG_ENTRY(pi, BBConfig, dcc_wakeup_restart_delay, 10)
		MOD_PHYREG_ENTRY(pi, dcc_ctrl_restart_length_grp, dcc_ctrl_restart_length, 0xffff)

		/* Set DCC FSM to run and then stop - i.e  do not idle, */
		MOD_PHYREG_ENTRY(pi, rx_tia_dc_loop_0, en_lock, 1)

		/* Correct sign of loop gain */
		MOD_PHYREG_ENTRY(pi, rx_tia_dc_loop_0, dac_sign, 1)

		/* disable DVG2 to avoid bphy resampler saturation */
		MOD_PHYREG_ENTRY(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, 0)

		/* digital-packet gain only */
		MOD_PHYREG_ENTRY(pi, singleShotAgcCtrl, singleShotPktGainElement, 96)

	ACPHY_REG_LIST_EXECUTE(pi);

	if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		/* SWWLAN-42666  : 0x454b ==> SW4345-514 : 0x424b */
		ACPHY_REG_LIST_START
			WRITE_PHYREG_ENTRY(pi, crshighlowpowThresholdl, 0x424b)
			WRITE_PHYREG_ENTRY(pi, crshighlowpowThresholdu, 0x424b)
			WRITE_PHYREG_ENTRY(pi, crshighlowpowThresholdlSub1, 0x424b)
			WRITE_PHYREG_ENTRY(pi, crshighlowpowThresholduSub1, 0x424b)
		ACPHY_REG_LIST_EXECUTE(pi);
		/*
		BPHY-20in20:Tap main-DC-Filter,
		DVGA2=ON,dvga2maxgain limit disabled(i.e. gain=12dB)
		*/
		/*
		A0 = 001000 = 0x8 (11bhpf on,adc10 off )
		B0 = 000001 = 0x1 (adc10off,dvga tap)
		B0mod = 110111 = 0x37(targetvar, adc10+6dB, 11bhpf-off, adc10 on, dcnotchtap )
		*/
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, datapath & 1);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, (datapath>>1) & 1);
		MOD_PHYREG(pi, RxSdFeConfig5, tiny_bphy20_ADC10_sel, (datapath>>2)&1);
		MOD_PHYREG(pi, bphyTest, dccomp, (datapath>>3)&1);
		if ((datapath >> 4) & 1) {
			temp_reg = READ_PHYREG(pi, work_around_ctrl) | (0x1 << 7);
			WRITE_PHYREG(pi, work_around_ctrl, temp_reg);
		}
		if ((datapath >> 5) & 1) {
			MOD_PHYREGC(pi, _BPHY_TargetVar_log2_pt8us, 0,
				bphy_targetVar_log2_pt8us, 479);
		}
		if ((datapath >> 6) & 7) {
			MOD_PHYREG(pi, DigiGainLimit0, minCckDigiGainShift, (datapath >> 6) & 7);
		}
		if ((datapath >> 9) & 0x3ff) {
			MOD_PHYREGC(pi, _BPHY_TargetVar_log2_pt8us, 0, bphy_targetVar_log2_pt8us,
				(datapath >> 9) & 0x3ff);
		}
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, overideDigiGain1, cckdigigainEnCntValue, 119)

#if defined(ATE_BUILD) && defined(ATE_HTAGC_DISABLED)
		MOD_PHYREG_ENTRY(pi, dot11acConfig, HTAgcPktgainEn, 0)
#endif // endif
	ACPHY_REG_LIST_EXECUTE(pi);

}

/*
gmult_rc (24:17), gmult(16:9), bq1_bw(8:6), rc_bw(5:3), bq0_bw(2:0)
LO: (15:0), HI (24:16)
mode_mask = bits[0:8] = 11b_20, 11n_20, 11ag_11ac_20, 11b_40, 11n_40, 11ag_11ac_40, 11b_80,
11n_11ag_11ac_80, samp_play
*/
static void
wlc_phy_set_analog_tx_lpf(phy_info_t *pi, uint16 mode_mask, int bq0_bw, int bq1_bw,
                       int rc_bw, int gmult, int gmult_rc, int core_num)
{
	uint8 ctr, core, max_modes = 9;
	uint16 addr_lo_offs[] = {0x142, 0x152, 0x162, 0x482};
	uint16 addr_hi_offs[] = {0x362, 0x372, 0x382, 0x552};
	uint16 rxlpfbw[] = {0, 0, 0, 1, 1, 1, 2, 2, 1};
	uint16 addr_lo_base, addr_hi_base, addr_lo, addr_hi;
	uint16 val_lo, val_hi;
	uint32 val;
	uint8 stall_val;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	/* This proc does not impact 4349, so return without doing anything */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_36(pi->pubpi->phy_rev)) {
		return;
	}
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* core_num = -1 ==> all cores */
	FOREACH_ACTV_CORE(pi, stf_shdata->phytxchain, core) {
		if ((core_num == -1) || (core_num == core)) {
			ASSERT(core < ARRAYSIZE(addr_lo_offs));
			addr_lo_base = addr_lo_offs[core];
			addr_hi_base = addr_hi_offs[core];
			for (ctr = 0; ctr < max_modes; ctr++) {
				if ((mode_mask >> ctr) & 1) {
					addr_lo = addr_lo_base + ctr;
					addr_hi = addr_hi_base + ctr;
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                         1, addr_lo, 16, &val_lo);
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                         1, addr_hi, 16, &val_hi);
					val = (val_hi << 16) | val_lo;
					if (ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
						/* Only bq1/bq2 (= TIA/LPF) bandwidth is controlled
						 * by direct control. The biquad control fields for
						 * Rx are in the Rx2Tx table entries and are
						 * initialized here too.
						 * For Tx bq2/bq1 control is in bit 5:2/1:0, however
						 * only bq2 is in the Tx path
						 * For Rx bq2/bq1 control is in bit 11:8/7:6
						 * Note: in this function bq2/bq1 are referred to
						 *       as bq1/bq0
						 */
						if (bq0_bw >= 0) {
							val = (val & 0x1fffffc) | (bq0_bw << 0);
						}
						if (bq1_bw >= 0) {
							val = (val & 0x1ffffc3) | (bq1_bw << 2);
						}
						val = (val & 0x1fff03f) | (rxlpfbw[ctr] << 6) |
						(rxlpfbw[ctr] << 8);
					} else {
						if (bq0_bw >= 0) {
							val = (val & 0x1fffff8) | (bq0_bw << 0);
							}
						if (rc_bw >= 0) {
							val = (val & 0x1ffffc7) | (rc_bw << 3);
						}
						if (bq1_bw >= 0) {
							val = (val & 0x1fffe3f) | (bq1_bw << 6);
						}
						if (gmult >= 0) {
							val = (val & 0x1fe01ff) | (gmult << 9);
						}
						if (gmult_rc >= 0) {
							val = (val & 0x001ffff) | (gmult_rc << 17);
						}
					}

					val_lo = val & 0xffff;
					val_hi = (val >> 16) & 0x1ff;
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                          1, addr_lo, 16, &val_lo);
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                          1, addr_hi, 16, &val_hi);
				}
			}
		}
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

/*
dacbuf_fixed_cap[5], dacbuf_cap[4:0]
mode_mask = bits[0:8] = 11b_20, 11n_20, 11ag_11ac_20, 11b_40, 11n_40, 11ag_11ac_40, 11b_80,
11n_11ag_11ac_80, samp_play
*/
static void
wlc_phy_set_tx_afe_dacbuf_cap(phy_info_t *pi, uint16 mode_mask, int dacbuf_cap,
                           int dacbuf_fixed_cap, int core_num)
{
	uint8 ctr, core, max_modes = 9;
	uint16 core_base[] = {0x3f0, 0x60, 0xd0, 0x570};
	uint8 offset[] = {0xb, 0xb, 0xc, 0xc, 0xe, 0xe, 0xf, 0xf, 0xa};
	uint8 shift[] = {0, 6, 0, 6, 0, 6, 0, 6, 0};
	uint16 addr, read_val, val;
	uint8 stall_val;
	uint8 phyrxchain;

	BCM_REFERENCE(phyrxchain);

	if (ACMAJORREV_36(pi->pubpi->phy_rev))
		return;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* core_num = -1 ==> all cores */
	phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		if ((core_num == -1) || (core_num == core)) {
			for (ctr = 0; ctr < max_modes; ctr++) {
				if ((mode_mask >> ctr) & 1) {
					addr = core_base[core] + offset[ctr];
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                         1, addr, 16, &read_val);
					val = (read_val >> shift[ctr]) & 0x3f;

					if (dacbuf_cap >= 0) {
							val = (val & 0x20) | dacbuf_cap;
					}
					if (dacbuf_fixed_cap >= 0) {
						val = (val & 0x1f) |
						        (dacbuf_fixed_cap << 5);
					}

					if (shift[ctr] == 0) {
						val = (read_val & 0xfc0) | val;
					} else {
						val = (read_val & 0x3f) | (val << 6);
					}

					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                          1, addr, 16, &val);
				}
			}
		}
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

/*
gmult_rc (24:17), rc_bw(16:14), gmult(13:6), bq1_bw(5:3), bq0_bw(2:0)
LO: (15:0), HI (24:16)
mode_mask = bits[0:2] = 20, 40, 80
*/
static void
wlc_phy_set_analog_rx_lpf(phy_info_t *pi, uint8 mode_mask, int bq0_bw, int bq1_bw,
                  int rc_bw, int gmult, int gmult_rc, int core_num)
{
	uint8 ctr, core, max_modes = 3;
	uint16 addr20_lo_offs[] = {0x140, 0x150, 0x160};
	uint16 addr20_hi_offs[] = {0x360, 0x370, 0x380};
	uint16 addr40_lo_offs[] = {0x141, 0x151, 0x161};
	uint16 addr40_hi_offs[] = {0x361, 0x371, 0x381};
	uint16 addr80_lo_offs[] = {0x441, 0x443, 0x445};
	uint16 addr80_hi_offs[] = {0x440, 0x442, 0x444};
	uint16 addr_lo, addr_hi;
	uint16 val_lo, val_hi;
	uint32 val;
	uint8 stall_val;
	uint8 phyrxchain;

	BCM_REFERENCE(phyrxchain);

	/* This proc does not impact 4349, so return without doing anything */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_GE36(pi->pubpi->phy_rev))
		return;

	ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) < 4);

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	/* core_num = -1 ==> all cores */
	phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		if ((core_num == -1) || (core_num == core)) {
			for (ctr = 0; ctr < max_modes; ctr++) {
				if ((mode_mask >> ctr) & 1) {
					if (ctr == 0) {
						addr_lo = addr20_lo_offs[core];
						addr_hi = addr20_hi_offs[core];
					}
					else if (ctr == 1) {
						addr_lo = addr40_lo_offs[core];
						addr_hi = addr40_hi_offs[core];
					} else {
						addr_lo = addr80_lo_offs[core];
						addr_hi = addr80_hi_offs[core];
					}

					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                         1, addr_lo, 16, &val_lo);
					wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
					                         1, addr_hi, 16, &val_hi);
					val = (val_hi << 16) | val_lo;

					if (bq0_bw >= 0) {
						val = (val & 0x1fffff8) | (bq0_bw << 0);
					}
					if (bq1_bw >= 0) {
						val = (val & 0x1ffffc7) | (bq1_bw << 3);
					}
					if (gmult >= 0) {
						val = (val & 0x1ffc03f) | (gmult << 6);
					}
					if (rc_bw >= 0) {
						val = (val & 0x1fe3fff) | (rc_bw << 14);
					}
					if (gmult_rc >= 0) {
						val = (val & 0x001ffff) | (gmult_rc << 17);
					}

					val_lo = val & 0xffff;
					val_hi = (val >> 16) & 0x1ff;
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
					                          addr_lo, 16, &val_lo);
					wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1,
					                          addr_hi, 16, &val_hi);
				}
			}
		}
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
wlc_phy_set_analog_lpf(phy_info_t *pi, uint8 *tx_bq1, uint8 *tx_bq2, uint8 *rx_bq1, uint8 *rx_bq2)
{
	uint8 ctr,  core, stall_val;
	uint16 addrs[] = {0x142, 0x152, 0x162, 0x482};  // cores 0-3(20/40/80 mhz)
	uint16 addr160 = 0x5c0;                             // not per core table
	uint16 val_bw[4], vals[9];

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	// loop over 20/40/80/160mhz
	for (ctr = 0; ctr < 4; ctr++)
		val_bw[ctr] = (rx_bq2[ctr] << 8) | (rx_bq1[ctr] << 6) |
		    (tx_bq2[ctr] << 2) | tx_bq1[ctr];

	/* $val20 $val20 $val20 $val40 $val40 $val40 $val80 $val80 $valsmp */
	vals[0] = vals[1] = vals[2] = val_bw[0];
	vals[3] = vals[4] = vals[5] = val_bw[1];
	vals[6] = vals[7] = val_bw[2];
	vals[8] = CHSPEC_IS160(pi->radio_chanspec) ?
	    val_bw[3] : (CHSPEC_IS80(pi->radio_chanspec) ?
	                 val_bw[2] : (CHSPEC_IS40(pi->radio_chanspec) ? val_bw[1] : val_bw[0]));
	FOREACH_CORE(pi, core) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 9, addrs[core], 16, &vals);
	}

	/* 160mhz entries are not per core */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr160,   16, &val_bw[3]);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr160+1, 16, &val_bw[3]);

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
acphy_load_txv_for_spexp(phy_info_t *pi)
{
	uint32 len = 243, offset = 1220;

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX,
	                          len, offset, 32, acphy_txv_for_spexp);
}

static void
wlc_phy_cfg_energydrop_timeout(phy_info_t *pi)
{
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		/* Fine timing mod to have more overlap(~10dB) between low and high SNR regimes
		 * change to 0x8 to prevent the radar to trigger the fine timing
		 */
		MOD_PHYREG(pi, FSTRMetricTh, hiPwr_min_metric_th, 0x8);
		/* change it to 40000 for radar detection */
		WRITE_PHYREG(pi, energydroptimeoutLen, 0x9c40);
	} else {
		/* Fine timing mod to have more overlap(~10dB) between low and high SNR regimes */
		MOD_PHYREG(pi, FSTRMetricTh, hiPwr_min_metric_th, 0xf);
		/* In event of high power spurs/interference that causes crs-glitches,
		 * stay in WAIT_ENERGY_DROP for 1 clk20 instead of default 1 ms.
		 * This way, we get back to CARRIER_SEARCH quickly and will less likely to miss
		 * actual packets. PS: this is actually one settings for ACI
		 */
		WRITE_PHYREG(pi, energydroptimeoutLen, 0x2);
	}
}

static void
wlc_phy_set_reg_on_bw_change_acphy(phy_info_t *pi)
{
	uint8 core;
	const bool chspec_is20 = CHSPEC_IS20(pi->radio_chanspec);
	const bool chspec_is40 = CHSPEC_IS40(pi->radio_chanspec);

	if ((TINY_RADIO(pi) || ACMAJORREV_36(pi->pubpi->phy_rev) || IS_28NM_RADIO(pi))) {
		MOD_PHYREG(pi, TssiEnRate, StrobeRateOverride, 1);
	}

	MOD_PHYREG(pi, TssiEnRate, StrobeRate, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	0x1 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2 :
	CHSPEC_IS80(pi->radio_chanspec) ? 0x3 : 0x4);
	MOD_PHYREG(pi, ClassifierCtrl, mac_bphy_band_sel, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	0x1 : CHSPEC_IS40(pi->radio_chanspec) ? 0x0  : 0x0);
	MOD_PHYREG(pi, RxControl, bphy_band_sel, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	0x1 : CHSPEC_IS40(pi->radio_chanspec) ? 0x0 : 0x0);
	MOD_PHYREG(pi, DcFiltAddress, dcCoef0, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	0x15 : CHSPEC_IS40(pi->radio_chanspec) ? 0xb : 0x5);
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
		ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		if (!ACMAJORREV_4(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, CRSMiscellaneousParam, crsMfFlipCoef,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x0 : 0x1);
		}

		MOD_PHYREG(pi, iqest_input_control, dc_accum_wait_vht,
			chspec_is20 ? 0xc :
			chspec_is40 ? 0x1d : 0x3b);
		MOD_PHYREG(pi, iqest_input_control, dc_accum_wait_mm,
			chspec_is20 ? 0xb :
			chspec_is40 ? 0x1b : 0x37);
		if (!ACMAJORREV_3(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, IqestWaitTime, waitTime,
				chspec_is20 ? 0x14  : 0x28);
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, lesiCrsTypRxPowerPerCore, PowerLevelPerCore,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x15b : CHSPEC_IS40(pi->radio_chanspec) ? 0x228 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x184 : 0x27e);
		MOD_PHYREG(pi, lesiCrsHighRxPowerPerCore, PowerLevelPerCore,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x76e : CHSPEC_IS40(pi->radio_chanspec) ? 0x9c1 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x551 : 0x64b);
		MOD_PHYREG(pi, lesiCrsMinRxPowerPerCore, PowerLevelPerCore,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x5 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x2c : 0x58);
		MOD_PHYREG(pi, lesiCrs1stDetThreshold_1, crsDetTh1_1Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x3b : CHSPEC_IS40(pi->radio_chanspec) ? 0x2a :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x1d : 0x15);
		MOD_PHYREG(pi, lesiCrs1stDetThreshold_1, crsDetTh1_2Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x2a : CHSPEC_IS40(pi->radio_chanspec) ? 0x1d :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x15 : 0xe);
		MOD_PHYREG(pi, lesiCrs1stDetThreshold_2, crsDetTh1_3Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x22 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x11 : 0xc);
		MOD_PHYREG(pi, lesiCrs1stDetThreshold_2, crsDetTh1_4Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x1d : CHSPEC_IS40(pi->radio_chanspec) ? 0x15 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0xe : 0xa);
		MOD_PHYREG(pi, lesiCrs2ndDetThreshold_1, crsDetTh1_1Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x3b : CHSPEC_IS40(pi->radio_chanspec) ? 0x2a :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x1d : 0x15);
		MOD_PHYREG(pi, lesiCrs2ndDetThreshold_1, crsDetTh1_2Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x2a : CHSPEC_IS40(pi->radio_chanspec) ? 0x1d :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x15 : 0xe);
		MOD_PHYREG(pi, lesiCrs2ndDetThreshold_2, crsDetTh1_3Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x22 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x11 : 0xc);
		MOD_PHYREG(pi, lesiCrs2ndDetThreshold_2, crsDetTh1_4Core,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x1d : CHSPEC_IS40(pi->radio_chanspec) ? 0x15 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0xe : 0xa);
		MOD_PHYREG(pi, lesiFstrControl3, cCrsFftInpAdj, CHSPEC_IS20(pi->radio_chanspec) ?
			0x0 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x3 : 0x7);
		MOD_PHYREG(pi, lesiFstrControl3, lCrsFftInpAdj, CHSPEC_IS20(pi->radio_chanspec) ?
			9: CHSPEC_IS40(pi->radio_chanspec) ? 0x13 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x28 : 0x51);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor0_0, subBand0Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor1_0, subBand3Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20: CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor0_1, subBand0Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20: CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor1_1, subBand3Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor0_2, subBand0Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor1_2, subBand3Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor0_3, subBand0Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, lesiFstrClassifierEqualizationFactor1_3, subBand3Factor,
			CHSPEC_IS20(pi->radio_chanspec) ?
			0x20 : CHSPEC_IS40(pi->radio_chanspec) ? 0x20 :
			CHSPEC_IS80(pi->radio_chanspec) ? 0x30 : 0x30);
		MOD_PHYREG(pi, LesiFstrFdNoisePower0, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
			175: CHSPEC_IS40(pi->radio_chanspec) ? 175 :
			CHSPEC_IS80(pi->radio_chanspec) ? 100 : 100);
		MOD_PHYREG(pi, LesiFstrFdNoisePower1, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
			175: CHSPEC_IS40(pi->radio_chanspec) ? 175 :
			CHSPEC_IS80(pi->radio_chanspec) ? 100 : 100);
		MOD_PHYREG(pi, LesiFstrFdNoisePower2, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
			175: CHSPEC_IS40(pi->radio_chanspec) ? 175 :
			CHSPEC_IS80(pi->radio_chanspec) ? 100 : 100);
		MOD_PHYREG(pi, LesiFstrFdNoisePower3, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
			175: CHSPEC_IS40(pi->radio_chanspec) ? 175 :
			CHSPEC_IS80(pi->radio_chanspec) ? 100 : 100);
		MOD_PHYREG(pi, iqest_input_control, dc_accum_wait_vht,
			CHSPEC_IS20(pi->radio_chanspec) ? 12 :
			CHSPEC_IS40(pi->radio_chanspec) ? 29 :
			CHSPEC_IS80(pi->radio_chanspec) ? 59 : 0x56);
		MOD_PHYREG(pi, iqest_input_control, dc_accum_wait_mm,
			CHSPEC_IS20(pi->radio_chanspec) ? 11 :
			CHSPEC_IS40(pi->radio_chanspec) ? 27 :
			CHSPEC_IS80(pi->radio_chanspec) ? 55 : 0x4d);
		MOD_PHYREG(pi, IqestWaitTime, waitTime,
			CHSPEC_IS20(pi->radio_chanspec) ? 20 :
			CHSPEC_IS40(pi->radio_chanspec) ? 40 :
			CHSPEC_IS80(pi->radio_chanspec) ? 40 : 40);
		if (ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, lesiFstrModeSwitchHiPower, fstrSwitchPwrHi1c,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x239 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x33d :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x211 : 0x308);
			MOD_PHYREG(pi, lesiFstrModeSwitchLoPower, fstrSwitchPwrLo1c,
				CHSPEC_IS20(pi->radio_chanspec) ? 0xec :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x19d :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x13f : 0x239);
			MOD_PHYREG(pi, lesiFstrControl4, lCrsFftOp1Adj,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x1 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x2 :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x4 : 0x8);
			MOD_PHYREG(pi, lesiFstrControl4, lCrsFftOp1Adj,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x1 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x2 :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x4 : 0x8);
			MOD_PHYREG(pi, lesiFstrControl4, delayCmbCfo,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x0 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x1 :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x1 : 0x1);
		}

		if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev)) {
			WRITE_PHYREG(pi, ACIBrwdfCoef0,
				CHSPEC_IS20(pi->radio_chanspec) ? 0xdc31 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0xc32b :
				CHSPEC_IS80(pi->radio_chanspec) ? 0xba28 : 0xb626);
			WRITE_PHYREG(pi, ACIBrwdfCoef1,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x0000 :
				CHSPEC_IS40(pi->radio_chanspec) ? 0x0000 :
				CHSPEC_IS80(pi->radio_chanspec) ? 0x00f7 : 0xfcf1);
			WRITE_PHYREG(pi, ACIBrwdfCoef2,
				CHSPEC_IS20(pi->radio_chanspec) ? 0x008d :
				CHSPEC_IS40(pi->radio_chanspec) ? 0xee80 :
				CHSPEC_IS80(pi->radio_chanspec) ? 0xe179 : 0xdd74);
		}
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		wlc_phy_set_reg_on_bw_change_acphy_majorrev40_44(pi);
	}

	if (!TINY_RADIO(pi) && !IS_28NM_RADIO(pi)) {
		MOD_PHYREG(pi, RxFilt40Num00, RxFilt40Num00, chspec_is20 ?
			0x146 : chspec_is40 ? 0x181 : 0x17a);
		MOD_PHYREG(pi, RxFilt40Num01, RxFilt40Num01, chspec_is20 ?
			0x88 : chspec_is40 ? 0x5a : 0x9e);
		MOD_PHYREG(pi, RxFilt40Num02, RxFilt40Num02, chspec_is20 ?
			0x146 : chspec_is40 ? 0x181 : 0x17a);
		MOD_PHYREG(pi, RxFilt40Den00, RxFilt40Den00, chspec_is20 ?
			0x76e : chspec_is40 ? 0x793 : 0x7ca);
		MOD_PHYREG(pi, RxFilt40Den01, RxFilt40Den01, chspec_is20 ?
			0x1a8 : chspec_is40 ? 0x1b7 : 0x1b2);
		MOD_PHYREG(pi, RxFilt40Num10, RxFilt40Num10, chspec_is20 ?
			0xa3 : chspec_is40 ? 0xc1 : 0xbd);
		MOD_PHYREG(pi, RxFilt40Num11, RxFilt40Num11, chspec_is20 ?
			0xf4 : chspec_is40 ? 0x102 : 0x114);
		MOD_PHYREG(pi, RxFilt40Num12, RxFilt40Num12, chspec_is20 ?
			0xa3 : chspec_is40 ? 0xc1 : 0xbd);
		MOD_PHYREG(pi, RxFilt40Den10, RxFilt40Den10, chspec_is20 ?
			0x684 : chspec_is40 ? 0x6c0 : 0x6d6);
		MOD_PHYREG(pi, RxFilt40Den11, RxFilt40Den11, chspec_is20 ?
			0xad : chspec_is40 ? 0xa9 : 0xa2);
		MOD_PHYREG(pi, RxStrnFilt40Num00, RxStrnFilt40Num00,
			chspec_is20 ? 0xe5 : chspec_is40 ? 0x162 : 0x16c);
		MOD_PHYREG(pi, RxStrnFilt40Num01, RxStrnFilt40Num01,
			chspec_is20 ? 0x68 : chspec_is40 ? 0x42 : 0x6f);
		MOD_PHYREG(pi, RxStrnFilt40Num02, RxStrnFilt40Num02,
			chspec_is20 ? 0xe5 : chspec_is40 ? 0x162 : 0x16c);
		MOD_PHYREG(pi, RxStrnFilt40Den00, RxStrnFilt40Den00,
			chspec_is20 ? 0x6be : chspec_is40 ? 0x75c : 0x793);
		MOD_PHYREG(pi, RxStrnFilt40Den01, RxStrnFilt40Den01,
			chspec_is20 ? 0x19e : chspec_is40 ? 0x1b3 : 0x1b2);
		MOD_PHYREG(pi, RxStrnFilt40Num10, RxStrnFilt40Num10,
			chspec_is20 ? 0x73 : chspec_is40 ? 0xb1 : 0xb6);
		MOD_PHYREG(pi, RxStrnFilt40Num11, RxStrnFilt40Num11,
			chspec_is20 ? 0xb2 : chspec_is40 ? 0xed : 0xff);
		MOD_PHYREG(pi, RxStrnFilt40Num12, RxStrnFilt40Num12,
			chspec_is20 ? 0x73 : chspec_is40 ? 0xb1 : 0xb6);
		MOD_PHYREG(pi, RxStrnFilt40Den10, RxStrnFilt40Den10,
			chspec_is20 ? 0x5fe : chspec_is40 ? 0x692 : 0x6b4);
		MOD_PHYREG(pi, RxStrnFilt40Den11, RxStrnFilt40Den11,
			chspec_is20 ? 0xcc : chspec_is40 ? 0xaf : 0xa8);
	}

	MOD_PHYREG(pi, nvcfg3, noisevar_rxevm_lim_qdb, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	0x97 : CHSPEC_IS40(pi->radio_chanspec) ? 0x8b :
	CHSPEC_IS80(pi->radio_chanspec) ? 0x97 : 0x97);
	if (CHSPEC_IS5G(pi->radio_chanspec) && !(ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
	    MOD_PHYREG(pi, RadarBlankCtrl, radarBlankingInterval,
	    CHSPEC_BW_LE20(pi->radio_chanspec) ? 0x19 :
	    CHSPEC_IS40(pi->radio_chanspec) ? 0x32 : 0x32);
	    MOD_PHYREG(pi, RadarT3BelowMin, Count, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	    0x14 : CHSPEC_IS40(pi->radio_chanspec) ? 0x28 : 0x28);
	    MOD_PHYREG(pi, RadarT3Timeout, Timeout, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	    0xc8 : CHSPEC_IS40(pi->radio_chanspec) ? 0x190 : 0x190);
	    MOD_PHYREG(pi, RadarResetBlankingDelay, Count, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	    0x19 : CHSPEC_IS40(pi->radio_chanspec) ? 0x32 : 0x32);
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* subband Classifer (helps in eliminating flase detect due to ACPR leakage)
		i.e 20in40/80 is not false detected
		as 40/80 (these regs get reset on a bw change)
		*/
		WRITE_PHYREG(pi, ClassifierLogAC1, 0x90a);
		MOD_PHYREG(pi, ClassifierCtrl6, logACDelta2, 9);
		/* EANBLE time interleaved ADC for BW160 and DISABLE for others */
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfePhaseSel, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 1);
			MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 1);
		} else {
			MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfePhaseSel, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfePhaseSel, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfePhaseSel, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfePhaseSel, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 0);
			MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 0);
		}
	} else {
		MOD_PHYREG(pi, ClassifierCtrl6, logACDelta2, CHSPEC_BW_LE20(pi->radio_chanspec) ?
		           0x13 : CHSPEC_IS40(pi->radio_chanspec) ? 0x13 : 0x9);
		MOD_PHYREG(pi, ClassifierLogAC1, logACDelta1, CHSPEC_BW_LE20(pi->radio_chanspec) ?
		           0x13 : CHSPEC_IS40(pi->radio_chanspec) ? 0x13 : 0x9);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
			    ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, bphyPreDetectThreshold6, ac_det_1us_aci_th,
				           chspec_is20 ? 0x80 : chspec_is40 ? 0x200 : 0x200);
			}
		}
	}

	FOREACH_CORE(pi, core) {
	  MOD_PHYREGC(pi, Adcclip, core, adc_clip_cnt_th, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	  0xa : CHSPEC_IS40(pi->radio_chanspec) ? 0x14 :
	  CHSPEC_IS80(pi->radio_chanspec) ? 0x14 : 0x14);
	  MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh,
	  CHSPEC_BW_LE20(pi->radio_chanspec) ?
	  0x17 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2a :
	  CHSPEC_IS80(pi->radio_chanspec) ? 0x54 : 0x54);
	  MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcW1ClipCntTh,
	  CHSPEC_BW_LE20(pi->radio_chanspec) ?
	  0xe : CHSPEC_IS40(pi->radio_chanspec) ? 0x16 :
	  CHSPEC_IS80(pi->radio_chanspec) ? 0x2c : 0x58);
	}
	if (!ACMAJORREV_0(pi->pubpi->phy_rev) &&
	    !(ACMAJORREV_2(pi->pubpi->phy_rev) && ACMINORREV_0(pi))) {
	  MOD_PHYREG(pi, CRSMiscellaneousParam, crsMfFlipCoef, CHSPEC_BW_LE20(pi->radio_chanspec) ?
	    0x0 : 0x1);
	}
	/* FIX ME : Currently setting only for 4350, Other phy revs should
	 * check with RTL folks and set accordingly
	 */
	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, FSTRCtrl, fineStrSgiVldCntVal, chspec_is20 ?
			0x9 : 0xa);
		MOD_PHYREG(pi, FSTRCtrl, fineStrVldCntVal, chspec_is20 ?
			0x9 : 0xa);
	}
}

static void
wlc_phy_set_reg_on_bw_change_acphy_majorrev40_44(phy_info_t *pi)
{
	const bool chspec_is20 = CHSPEC_IS20(pi->radio_chanspec);
	const bool chspec_is40 = CHSPEC_IS40(pi->radio_chanspec);

	WRITE_PHYREG(pi, lesiCrsTypRxPowerPerCore,
		CHSPEC_IS20(pi->radio_chanspec) ?
		0x15b : CHSPEC_IS40(pi->radio_chanspec) ? 0x228 : 0x184);
	WRITE_PHYREG(pi, lesiCrsHighRxPowerPerCore,
		CHSPEC_IS20(pi->radio_chanspec) ?
		0x76e : CHSPEC_IS40(pi->radio_chanspec) ? 0x9c1 : 0x551);
	WRITE_PHYREG(pi, lesiCrsMinRxPowerPerCore,
		CHSPEC_IS20(pi->radio_chanspec) ?
		0x5 : CHSPEC_IS40(pi->radio_chanspec) ? 0x18 : 0x2c);

	WRITE_PHYREG(pi, lesiCrs1stDetThreshold_1,
		CHSPEC_IS20(pi->radio_chanspec) ?
		0x3344 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2633 : 0x1e26);

	WRITE_PHYREG(pi, lesiCrs2ndDetThreshold_1,
		CHSPEC_IS20(pi->radio_chanspec) ?
		0x3344 : CHSPEC_IS40(pi->radio_chanspec) ? 0x2633 : 0x1e26);

	MOD_PHYREG(pi, lesiFstrControl3, cCrsFftInpAdj, CHSPEC_IS20(pi->radio_chanspec) ?
		0x1 : CHSPEC_IS40(pi->radio_chanspec) ? 0x1 : 0x3);
	MOD_PHYREG(pi, lesiFstrControl3, lCrsFftInpAdj, CHSPEC_IS20(pi->radio_chanspec) ?
		9: CHSPEC_IS40(pi->radio_chanspec) ? 19 : 40);

	MOD_PHYREG(pi, LesiFstrFdNoisePower0, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
		175: CHSPEC_IS40(pi->radio_chanspec) ? 175 : 100);
	MOD_PHYREG(pi, LesiFstrFdNoisePower1, noi_pow, CHSPEC_IS20(pi->radio_chanspec) ?
		175: CHSPEC_IS40(pi->radio_chanspec) ? 175 : 100);

	WRITE_PHYREG(pi, ACIBrwdfCoef0,
		CHSPEC_IS20(pi->radio_chanspec) ? 0xdc31 :
		CHSPEC_IS40(pi->radio_chanspec) ? 0xc32b : 0xba28);
	WRITE_PHYREG(pi, ACIBrwdfCoef1,
		CHSPEC_IS20(pi->radio_chanspec) ? 0x0000 :
		CHSPEC_IS40(pi->radio_chanspec) ? 0x0000 : 0x00f7);
	WRITE_PHYREG(pi, ACIBrwdfCoef2,
		CHSPEC_IS20(pi->radio_chanspec) ? 0x008d :
		CHSPEC_IS40(pi->radio_chanspec) ? 0xee80 : 0xe179);
	MOD_PHYREG(pi, DcFiltAddressExt, dc_s2_bnd_start,
		chspec_is20 ? 0x10 :
		chspec_is40 ? 0x20 : 0x40);
	WRITE_PHYREG(pi, DcFiltAddressExt1,
		CHSPEC_IS20(pi->radio_chanspec) ? 0x1f30 :
		CHSPEC_IS40(pi->radio_chanspec) ? 0x3f60 : 0x7fc0);
	WRITE_PHYREG(pi, LtrnOffset,
		CHSPEC_IS20(pi->radio_chanspec) ? 0x090d : 0x0d11);
	WRITE_PHYREG(pi, MrcSigQualControl3,
		CHSPEC_IS20(pi->radio_chanspec) ? 0x071c : 0x0c30);
}

/* Load pdet related Rfseq on reset */
static void
wlc_phy_set_pdet_on_reset_acphy(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint8 core, pdet_range_id, subband_idx, ant, core_freq_segment_map;
	uint16 offset, tmp_val, val_av, val_vmid;
	uint8 bands[NUM_CHANS_IN_CHAN_BONDING];
	uint8 av[4] = {0, 0, 0, 0};
	uint8 vmid[4] = {0, 0, 0, 0};
	uint8 stall_val;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
	bool flag2rangeon =
		((CHSPEC_IS2G(pi->radio_chanspec) && phy_tpc_get_tworangetssi2g(pi->tpci)) ||
		(CHSPEC_IS5G(pi->radio_chanspec) && phy_tpc_get_tworangetssi5g(pi->tpci))) &&
		PHY_IPA(pi);

	BCM_REFERENCE(stf_shdata);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		pdet_range_id = phy_tpc_get_2g_pdrange_id(pi->tpci);
	} else {
		pdet_range_id = phy_tpc_get_5g_pdrange_id(pi->tpci);
	}

	FOREACH_CORE(pi, core) {
		/* core_freq_segment_map is only required for 80P80 mode
		 For other modes, it is ignored
		*/
		if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
			phy_ac_chanmgr_get_chan_freq_range_80p80(pi, 0, bands);
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				subband_idx = (core <= 1) ? bands[0] : bands[1];
			} else {
				subband_idx = bands[0];
				ASSERT(0);
			}
		} else {
			core_freq_segment_map = pi_ac->chanmgri->data->core_freq_mapping[core];
			subband_idx = phy_ac_chanmgr_get_chan_freq_range(pi, 0,
				core_freq_segment_map);
		}
		ant = phy_get_rsdbbrd_corenum(pi, core);
		if (BF3_AVVMID_FROM_NVRAM(pi->u.pi_acphy)) {
			av[core] = pi_ac->sromi->avvmid_set_from_nvram[ant][subband_idx][0];
			vmid[core] = pi_ac->sromi->avvmid_set_from_nvram[ant][subband_idx][1];
		} else {
			if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
				/* 4360 and 43602 */
				av[core] = avvmid_set[pdet_range_id][subband_idx][ant];
				vmid[core] = avvmid_set[pdet_range_id][subband_idx][ant+3];
			} else if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
				if (core == 0) {
					av[core] = avvmid_set1[pdet_range_id][subband_idx][ant];
					vmid[core] = avvmid_set1[pdet_range_id][subband_idx][ant+1];
				}
			} else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
				av[core] = avvmid_set2[pdet_range_id][subband_idx][ant];
				vmid[core] = avvmid_set2[pdet_range_id][subband_idx][ant+2];
			} else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
				if (core == 0) {
					av[core] = avvmid_set3[pdet_range_id][subband_idx][ant];
					vmid[core] =
					        avvmid_set3[pdet_range_id][subband_idx][ant+1];
				}
			} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				av[core] = avvmid_set4[pdet_range_id][subband_idx][ant];
				vmid[core] =
				        avvmid_set4[pdet_range_id][subband_idx][ant+2];
			} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
				av[core] = avvmid_set32[pdet_range_id][subband_idx][ant];
				vmid[core] = avvmid_set32[pdet_range_id][subband_idx][ant+4];
			} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				uint8 *p_avvmid_set =
					get_avvmid_set_36(pi, pdet_range_id, subband_idx);
				av[core] = p_avvmid_set[ant];
				vmid[core] = p_avvmid_set[ant+3];
			} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
				av[core] = avvmid_set32[pdet_range_id][subband_idx][ant];
				vmid[core] = avvmid_set32[pdet_range_id][subband_idx][ant+4];
			} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				if (ACMINORREV_GE(pi, 1)) {
					av[core] =
						avvmid_set47_1[pdet_range_id][subband_idx][ant];
					vmid[core] =
						avvmid_set47_1[pdet_range_id][subband_idx][ant+4];
					if (pdet_range_id == 0 && pi->sh->boardrev >= 5123) {
					//different avvmid ofr MCM P403 or later, 0x1403 = 5123
						av[core] =
						avvmid_set47_2[pdet_range_id][subband_idx][ant];
						vmid[core] =
						avvmid_set47_2[pdet_range_id][subband_idx][ant+4];
					}
				} else {
					av[core] =
						avvmid_set47[pdet_range_id][subband_idx][ant];
					vmid[core] =
						avvmid_set47[pdet_range_id][subband_idx][ant+4];
				}
			} else if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
				av[core] = avvmid_set47[pdet_range_id][subband_idx][ant];
				vmid[core] = avvmid_set47[pdet_range_id][subband_idx][ant+4];
			} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
				uint8 *p_avvmid_set =
					get_avvmid_set_40(pi, pdet_range_id, subband_idx);
				av[core] = p_avvmid_set[ant];
				vmid[core] = p_avvmid_set[ant+3];
			}
		}
	}
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	FOREACH_ACTV_CORE(pi, stf_shdata->hw_phyrxchain, core) {
		if ((ACMAJORREV_1(pi->pubpi->phy_rev) && (core == 0)) ||
		    !(ACMAJORREV_1(pi->pubpi->phy_rev))) {
			if (core == 3)
				offset = 0x560 + 0xd;
			else
				offset = 0x3c0 + 0xd + core*0x10;
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			                         1, offset, 16, &tmp_val);
			val_av = (tmp_val & 0x1ff8) | (av[core]&0x7);
			val_vmid = (val_av & 0x7) | ((vmid[core]&0x3ff)<<3);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
			                          1, offset, 16, &val_vmid);

			if (((ACMAJORREV_1(pi->pubpi->phy_rev) ||
				ACMAJORREV_2(pi->pubpi->phy_rev) ||
				ACMAJORREV_4(pi->pubpi->phy_rev)) &&
				BF3_TSSI_DIV_WAR(pi->u.pi_acphy)) ||
				flag2rangeon) {
				offset = 0x3c0 + 0xe + core*0x10;
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ,
				                          1, offset, 16, &val_vmid);
			}
		}
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
wlc_phy_set_tx_iir_coeffs(phy_info_t *pi, bool cck, uint8 filter_type)
{
	if (cck == FALSE) {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* Default filters */
			if (filter_type == 0) {
				if (CHSPEC_IS2G(pi->radio_chanspec) &&
						!ROUTER_4349(pi) && !PHY_IPA(pi)) {
					ACPHY_REG_LIST_START
					/* Default Chebyshev ~10.5MHz cutoff */
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a1, 0x0056)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a2, 0x02fb)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a1, 0x0f3d)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a2, 0x0169)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a1, 0x0e23)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a2, 0x0068)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20finescale, 0x00E9)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else {
					ACPHY_REG_LIST_START
					/* Default Chebyshev ~10.5MHz cutoff */
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a1, 0x0056)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a2, 0x02fb)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a1, 0x0f3d)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a2, 0x0169)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a1, 0x0e23)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a2, 0x0068)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20finescale, 0x00a6)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
			} else if (filter_type == 1) {
				ACPHY_REG_LIST_START
					 /* Chebyshev ~8.8MHz cutoff (FCC -26dBr BW) */
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a1, 0x0e73)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a2, 0x033d)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a1, 0x0d5f)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a2, 0x0205)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a1, 0x0c39)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a2, 0x011e)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20finescale, 0x001a)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			if (filter_type == 4) { /* narrower filter - cheby1: 6, 0.1, 9.1/20 */
				WRITE_PHYREG(pi, txfilt20in20st0a1, 0x0f6b);
				WRITE_PHYREG(pi, txfilt20in20st0a2, 0x0339);
				WRITE_PHYREG(pi, txfilt20in20st0n,  0x0002);
				WRITE_PHYREG(pi, txfilt20in20st1a1, 0x0e29);
				WRITE_PHYREG(pi, txfilt20in20st1a2, 0x01e5);
				WRITE_PHYREG(pi, txfilt20in20st1n,  0x0003);
				WRITE_PHYREG(pi, txfilt20in20st2a1, 0x0cb2);
				WRITE_PHYREG(pi, txfilt20in20st2a2, 0x00f0);
				WRITE_PHYREG(pi, txfilt20in20st2n,  0x0003);
				WRITE_PHYREG(pi, txfilt20in20finescale, 0x0054);
			}
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			if (filter_type == 0 || filter_type == 127) {
				ACPHY_REG_LIST_START
					/* Default Chebyshev ~10.5MHz cutoff */
					/* acphy_filter('txfilt20in20',3,0.01,9.3/20) */
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a1, 0x0056)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a2, 0x02fb)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a1, 0x0f3d)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a2, 0x0169)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a1, 0x0e23)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a2, 0x0068)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20finescale, 0x00A6)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (filter_type == 1) {
				/* 2nd stage is tuned to have 3db rolloff from dc to fc bandedge */
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a1, 0x0056)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0a2, 0x02fb)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a1, 0x0f3d)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1a2, 0x0169)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a1,  -730)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2a2, 179)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20st2n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfilt20in20finescale, 0x0088)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
		}
	} else {
		/* Tx filters in PHY REV 3, PHY REV 6 and later operate at 1/2 the sampling
		 * rate of previous revs
		 */
		if ((ACMAJORREV_0(pi->pubpi->phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) ||
		    (ACMAJORREV_1(pi->pubpi->phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) ||
		    (ACMAJORREV_3(pi->pubpi->phy_rev)) || (ACMAJORREV_4(pi->pubpi->phy_rev))) {
	    if (filter_type == 0) {
			ACPHY_REG_LIST_START
				/* Default filter */
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0a94)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x0373)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0005)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0a93)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x0298)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0004)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0a52)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x021d)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0004)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x0080)
			ACPHY_REG_LIST_EXECUTE(pi);
	    } else if (filter_type == 1) {
			ACPHY_REG_LIST_START
				/* Gaussian  shaping filter */
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0b54)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x0290)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0004)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0a40)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x0290)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0005)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0a06)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x0240)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0005)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x0080)
			ACPHY_REG_LIST_EXECUTE(pi);
		} else if (filter_type == 4) {
			if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1) {
				ACPHY_REG_LIST_START
					/* Gaussian shaping filter for TINY_A0, dac_rate_mode 1 */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, -80)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 369)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, -757)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 369)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, -1007)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 256)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 120)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 2) {
				ACPHY_REG_LIST_START
					/* Gaussian shaping filter for TINY_A0, dac_rate_mode 2 */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st0a1, -1852)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st0a2, 892)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st0n, 7)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st1a1, -1890)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st1a2, 892)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st1n, 7)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st2a1, -1877)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st2a2, 860)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80st2n, 7)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in80finescale, 65)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else {
				ACPHY_REG_LIST_START
					/* Gaussian shaping filter for TINY_A0, dac_rate_mode 3 */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st0a1, -1714)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st0a2, 829)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st0n, 6)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st1a1, -1796)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st1a2, 829)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st1n, 6)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st2a1, -1790)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st2a2, 784)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40st2n, 6)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in40finescale, 54)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
	    } else if (filter_type == 5) {
			ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, -48)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 1)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 3)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, -75)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 23)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 3)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, -504)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 64)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 3)
				WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 175)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	} else if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
		ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
		ACMAJORREV_36(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			if (filter_type == 0) {
				ACPHY_REG_LIST_START
					/* Default filter */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0f6b)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x0339)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0e29)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x01e5)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0cb2)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x00f0)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x00b3)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (filter_type == 1) {
				ACPHY_REG_LIST_START
					/* Gaussian shaping filter (-0.5 dB Tx Power) */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0edb)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x01cb)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0d1d)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x0192)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0c33)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x00f3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x0076)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (filter_type == 2) {
				ACPHY_REG_LIST_START
					/* Tweaked Gaussian for 4335 iPA CCk margin */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0edb)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x01ab)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0d1d)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x0172)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0c77)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x00a9)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x0082)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (filter_type == 4) {
				/* Tweaked Gaussian for 43012 CCk margin */
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0edb)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x01ab)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0d1d)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x0172)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0c77)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x00a9)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x0082)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
		} else if (ACMAJORREV_44(pi->pubpi->phy_rev) && PHY_IPA(pi)) {
			if (filter_type == 4) {
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, -80)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 369)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, -757)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 369)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, -1007)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 256)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 3)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 76)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else {
				/* TBD */
			}
		} else {
			if (filter_type == 0) {
				ACPHY_REG_LIST_START
					/* Default filter */
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a1, 0x0f6b)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0a2, 0x0339)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st0n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a1, 0x0e29)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1a2, 0x01e5)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st1n, 0x0002)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a1, 0x0cb2)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2a2, 0x00f0)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20st2n, 0x0003)
					WRITE_PHYREG_ENTRY(pi, txfiltbphy20in20finescale, 0x00b3)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else if (filter_type == 1) {
				/* TBD */
			}
		}
	}
}

static void
wlc_phy_set_sdadc_pd_val_per_core_acphy(phy_info_t *pi)
{
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && !(PHY_IPA(pi))) {
		bool suspend;
		uint8 stall_val, orig_rxfectrl1;
		uint16 rx_sd_adc_pd_val[2] = {0x20, 0x20};
		uint16 rx_sd_adc_pd_cfg[2] = {0x00, 0x00};
		uint8 core;
		uint8 phyrxchain;

		int fc;
		const void *chan_info = NULL;
		uint8 ch = CHSPEC_CHANNEL(pi->radio_chanspec);
		phy_ac_radio_data_t *rd = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);

		BCM_REFERENCE(phyrxchain);

		suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
		/* Disable stalls and hold FIFOs in reset */
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);

		if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
			fc = wlc_phy_chan2freq_acphy(pi, ch, &chan_info);
		} else {
			const chan_info_radio20691_t *chan_info_20691;
			fc = wlc_phy_chan2freq_20691(pi, ch, &chan_info_20691);
		}

			phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (rd->srom_txnospurmod2g
						== 0) {
					rx_sd_adc_pd_val[core] = 0x3d;
					rx_sd_adc_pd_cfg[core] = 0x1a53;
				} else {
					if ((fc == 2412) || (fc == 2467)) {
						rx_sd_adc_pd_val[core] = 0x3d;
						rx_sd_adc_pd_cfg[core] = 0x1b53;
					} else {
						rx_sd_adc_pd_val[core] = 0x3d;
						rx_sd_adc_pd_cfg[core] = 0x1c53;
					}
				}
			} else {
				if (CHSPEC_IS20(pi->radio_chanspec)) {
					if (((fc >= 5180) && (fc <= 5320)) ||
					((fc >= 5745) && (fc <= 5825))) {
						rx_sd_adc_pd_val[core] = 0x3d;
						rx_sd_adc_pd_cfg[core] = 0x1f12;
					} else {
						rx_sd_adc_pd_val[core] = 0x3d;
						rx_sd_adc_pd_cfg[core] = 0x1B53;
					}
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					if (fc == 5190) {
						rx_sd_adc_pd_val[core] = 0x3f;
						rx_sd_adc_pd_cfg[core] = 0x1818;
					} else {
						rx_sd_adc_pd_val[core] = 0x3d;
						rx_sd_adc_pd_cfg[core] = 0x1f12;
					}
				} else {
					rx_sd_adc_pd_val[core] = 0x3f;
					rx_sd_adc_pd_cfg[core] = 0x1818;
				}
			}

		}

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c6, 16,
			&rx_sd_adc_pd_val[0]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c9, 16,
			&rx_sd_adc_pd_cfg[0]);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3d6, 16,
			&rx_sd_adc_pd_val[1]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3d9, 16,
			&rx_sd_adc_pd_cfg[1]);

		/* Restore FIFO reset and Stalls */
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
		ACPHY_ENABLE_STALL(pi, stall_val);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);

	}

}

static void
wlc_phy_write_regtbl_fc3_sub0(phy_info_t *pi)
{
	uint8 fectrl_mch5_c0_p200_p400[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 4, 3, 11, 2, 4, 3, 11, 0x02, 0x24, 0x03, 0x2d, 0x02, 0x24, 0x03, 0x2d};
	uint8 fectrl_mch5_c1_p200_p400[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		2, 1, 6, 14, 2, 1, 6, 14, 0x02, 0x21, 0x06, 0x2d, 0x02, 0x21, 0x06, 0x2d};
	uint8 fectrl_mch5_c2_p200_p400[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		4, 1, 6, 14, 4, 1, 6, 14, 0x04, 0x21, 0x06, 0x2b, 0x04, 0x21, 0x06, 0x2b};

	si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		0xffffff, CCTRL4360_DISCRETE_FEMCTRL_MODE);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32,	0, 8,
		fectrl_mch5_c0_p200_p400);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 32, 8,
		fectrl_mch5_c1_p200_p400);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 64, 8,
		fectrl_mch5_c2_p200_p400);

}

static void
wlc_phy_write_regtbl_fc3_sub1(phy_info_t *pi)
{
	uint8 fectrl_mch5_c0[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		8, 4, 3, 8, 8, 4, 3, 8, 0x08, 0x24, 0x03, 0x25, 0x08, 0x24, 0x03, 0x25};
	uint8 fectrl_mch5_c1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		8, 1, 6, 8, 8, 1, 6, 8, 0x08, 0x21, 0x06, 0x25, 0x08, 0x21, 0x06, 0x25};
	uint8 fectrl_mch5_c2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		8, 1, 6, 8, 8, 1, 6, 8, 0x08, 0x21, 0x06, 0x23, 0x08, 0x21, 0x06, 0x23};

	/* P500+ */
	si_corereg(pi->sh->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
		0xffffff, CCTRL4360_DISCRETE_FEMCTRL_MODE);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32,	0, 8,
		fectrl_mch5_c0);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 32, 8,
		fectrl_mch5_c1);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 64, 8,
		fectrl_mch5_c2);
}

static void
wlc_phy_write_regtbl_fc3_sub2(phy_info_t *pi)
{
	uint8 fectrl_j28[] =  {2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23,
		0x25, 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25};

	/* J28 */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32,	0, 8,
		fectrl_j28);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 32, 8,
		fectrl_j28);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 64, 8,
		fectrl_j28);
}

static void
wlc_phy_write_regtbl_fc3_sub3(phy_info_t *pi)
{
	uint8 fectrl3_sub3_c0[] = {2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23,
		0x25, 2, 4, 3, 2, 2, 4, 3, 2, 0x22, 0x24, 0x23, 0x25, 0x22, 0x24, 0x23, 0x25};
	uint8 fectrl3_sub3_c1[] = {2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26,
		0x25, 2, 1, 6, 2, 2, 1, 6, 2, 0x22, 0x21, 0x26, 0x25, 0x22, 0x21, 0x26, 0x25};
	uint8 fectrl3_sub3_c2[] = {4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26,
		0x23, 4, 1, 6, 4, 4, 1, 6, 4, 0x24, 0x21, 0x26, 0x23, 0x24, 0x21, 0x26, 0x23};

	/* MCH2 */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32,	0, 8,
		fectrl3_sub3_c0);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 32, 8,
		fectrl3_sub3_c1);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 32, 64, 8,
		fectrl3_sub3_c2);
}

static INLINE void
wlc_phy_write_regtbl_fc3(phy_info_t *pi, phy_info_acphy_t *pi_ac)
{
	switch (BF3_FEMCTRL_SUB(pi_ac)) {
		case 0:
			wlc_phy_write_regtbl_fc3_sub0(pi);
		break;
		case 1:
			wlc_phy_write_regtbl_fc3_sub1(pi);
		break;
		case 2:
			wlc_phy_write_regtbl_fc3_sub2(pi);
		break;
		case 3:
			wlc_phy_write_regtbl_fc3_sub3(pi);
		break;
	}
}

static void
wlc_phy_write_regtbl_fc4_sub0(phy_info_t *pi)
{
	uint16 fectrl_zeroval[] = {0};
	uint16 kk, fem_idx = 0;
	sparse_array_entry_t fectrl_fcbga_epa_elna[] =
		{{2, 264}, {3, 8}, {9, 32}, {18, 5}, {19, 4}, {25, 128}, {130, 64}, {192, 64}};

	for (kk = 0; kk < 256; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_fcbga_epa_elna) &&
			kk == fectrl_fcbga_epa_elna[fem_idx].idx) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				&(fectrl_fcbga_epa_elna[fem_idx].val));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc4_sub1(phy_info_t *pi)
{
	uint16 fectrl_zeroval[] = {0};
	uint16 kk, fem_idx = 0;
	sparse_array_entry_t fectrl_wlbga_epa_elna[] =
	{{2, 3}, {3, 1}, {9, 256}, {18, 20}, {19, 16}, {25, 8}, {66, 3}, {67, 1},
	{73, 256}, {82, 20}, {83, 16}, {89, 8}, {128, 3}, {129, 1}, {130, 3}, {131, 1},
	{132, 1}, {133, 1}, {134, 1}, {135, 1}, {136, 3}, {137, 1}, {138, 3}, {139, 1},
	{140, 1}, {141, 1}, {142, 1}, {143, 1}, {160, 3}, {161, 1}, {162, 3}, {163, 1},
	{164, 1}, {165, 1}, {166, 1}, {167, 1}, {168, 3}, {169, 1}, {170, 3}, {171, 1},
	{172, 1}, {173, 1}, {174, 1}, {175, 1}, {192, 128}, {193, 128}, {196, 128}, {197, 128},
	{200, 128}, {201, 128}, {204, 128}, {205, 128}, {224, 128}, {225, 128}, {228, 128},
	{229, 128}, {232, 128}, {233, 128}, {236, 128}, {237, 128} };
	for (kk = 0; kk < 256; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_wlbga_epa_elna) &&
			kk == fectrl_wlbga_epa_elna[fem_idx].idx) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				&(fectrl_wlbga_epa_elna[fem_idx].val));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc4_sub2(phy_info_t *pi)
{
	uint16 fectrl_zeroval[] = {0};
	uint16 kk, fem_idx = 0;
	sparse_array_entry_t fectrl_fchm_epa_elna[] =
	{{2, 280}, {3, 24}, {9, 48}, {18, 21}, {19, 20}, {25, 144}, {34, 776}, {35, 520},
	{41, 544}, {50, 517}, {51, 516}, {57, 640}, {66, 280}, {67, 24}, {73, 48}, {82, 21},
	{83, 20}, {89, 144}, {98, 776}, {99, 520}, {105, 544}, {114, 517}, {115, 516}, {121, 640},
	{128, 280}, {129, 24}, {130, 280}, {131, 24}, {132, 24}, {133, 24}, {134, 24}, {135, 24},
	{136, 280}, {137, 24}, {138, 280}, {139, 24}, {140, 24}, {141, 24}, {142, 24}, {143, 24},
	{160, 776}, {161, 520}, {162, 776}, {163, 520}, {164, 520}, {165, 520}, {166, 520},
	{167, 520}, {168, 776}, {169, 520}, {170, 776}, {171, 520}, {172, 520}, {173, 520},
	{174, 520}, {175, 520},	{192, 16}, {193, 16}, {196, 16}, {197, 16}, {200, 16}, {201, 16},
	{204, 16}, {205, 16}, {224, 512}, {225, 512}, {228, 512}, {229, 512}, {232, 512},
	{233, 512}, {236, 512}, {237, 512}};
	for (kk = 0; kk < 256; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_fchm_epa_elna) &&
			kk == fectrl_fchm_epa_elna[fem_idx].idx) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				&(fectrl_fchm_epa_elna[fem_idx].val));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc4_sub34(phy_info_t *pi)
{
	uint16 fectrl_zeroval[] = {0};
	uint16 kk, fem_idx = 0;

	sparse_array_entry_t fectrl_wlcsp_epa_elna[] =
		{{2, 34}, {3, 2}, {9, 1}, {18, 80}, {19, 16}, {25, 8}, {66, 34}, {67, 2},
		{73, 1}, {82, 80}, {83, 16}, {89, 8}, {128, 34}, {129, 2}, {130, 34}, {131, 2},
		{132, 2}, {133, 2}, {134, 2}, {135, 2}, {136, 34}, {137, 2}, {138, 34}, {139, 2},
		{140, 2}, {141, 2}, {142, 2}, {143, 2}, {160, 34}, {161, 2}, {162, 34}, {163, 2},
		{164, 2}, {165, 2}, {166, 2}, {167, 2}, {168, 34}, {169, 2}, {170, 34}, {171, 2},
		{172, 2}, {173, 2}, {174, 2}, {175, 2}, {192, 4}, {193, 4}, {196, 4}, {197, 4},
		{200, 4}, {201, 4}, {204, 4}, {205, 4}, {224, 4}, {225, 4}, {228, 4}, {229, 4},
		{232, 4}, {233, 4}, {236, 4}, {237, 4} };
	for (kk = 0; kk < 256; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_wlcsp_epa_elna) &&
			kk == fectrl_wlcsp_epa_elna[fem_idx].idx) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				&(fectrl_wlcsp_epa_elna[fem_idx].val));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc4_sub5(phy_info_t *pi)
{
	uint16 fectrl_zeroval[] = {0};
	uint16 kk, fem_idx = 0;

	sparse_array_entry_t fectrl_fp_dpdt_epa_elna[] =
		{{2, 280}, {3, 24}, {9, 48}, {18, 21}, {19, 20}, {25, 144}, {34, 776},
		{35, 520}, {41, 544}, {50, 517}, {51, 516}, {57, 640}, {130, 80},
		{192, 80}};

	for (kk = 0; kk < 256; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_fp_dpdt_epa_elna) &&
			kk == fectrl_fp_dpdt_epa_elna[fem_idx].idx) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				&(fectrl_fp_dpdt_epa_elna[fem_idx].val));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static INLINE void
wlc_phy_write_regtbl_fc4(phy_info_t *pi, phy_info_acphy_t *pi_ac)
{
	switch (BF3_FEMCTRL_SUB(pi_ac)) {
		case 0:
			wlc_phy_write_regtbl_fc4_sub0(pi);
		break;
		case 1:
			wlc_phy_write_regtbl_fc4_sub1(pi);
		break;
		case 2:
			wlc_phy_write_regtbl_fc4_sub2(pi);
		break;
		case 3:
		case 4:
			wlc_phy_write_regtbl_fc4_sub34(pi);
		break;
		case 5:
			wlc_phy_write_regtbl_fc4_sub5(pi);
		break;
	}
}

static void
wlc_phy_write_regtbl_fc10_sub0(phy_info_t *pi)
{
	uint16 fectrl_fcbga_epa_elna_idx[] = {2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
		129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		192, 193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
	uint16 fectrl_fcbga_epa_elna_val[] = {96, 32, 8, 6, 2, 1, 96, 32, 8, 6, 2, 96, 32,
		96, 32, 32, 32, 32, 32, 96, 32, 96, 32, 32, 32, 32, 32, 128, 128, 128, 128,
		128, 128, 128, 128, 134, 130, 5, 4, 8, 48, 32, 64 };
	uint16 fectrl_zeroval[] = {0};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < 320; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_fcbga_epa_elna_idx) &&
			kk == fectrl_fcbga_epa_elna_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(fectrl_fcbga_epa_elna_val[fem_idx]));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc10_sub1(phy_info_t *pi)
{
	uint16 fectrl_wlbga_epa_elna_idx[] = {2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
		129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		192, 193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
	uint16 fectrl_wlbga_epa_elna_val[] = {48, 32, 8, 6, 2, 1, 48, 32, 8, 6, 2, 48, 32,
	        48, 32, 32, 32, 32, 32, 48, 32, 48, 32, 32, 32, 32, 32, 128, 128, 128, 128,
		128, 128, 128, 128, 134, 130, 48, 32, 8, 6, 2, 1};
	uint16 fectrl_zeroval[] = {0};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < 320; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_wlbga_epa_elna_idx) &&
			kk == fectrl_wlbga_epa_elna_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(fectrl_wlbga_epa_elna_val[fem_idx]));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc10_sub2(phy_info_t *pi)
{
	uint16 fectrl_wlbga_ipa_ilna_idx[] = {2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
		129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		192, 193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
	uint16 fectrl_wlbga_ipa_ilna_val[] = {48, 32, 8, 6, 2, 1, 48, 32, 8, 6, 2, 48, 32,
	        48, 32, 32, 32, 32, 32, 48, 32, 48, 32, 32, 32, 32, 32, 128, 128, 128, 128,
		128, 128, 128, 128, 134, 130, 48, 32, 8, 6, 2, 1};
	uint16 fectrl_zeroval[] = {0};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < 320; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_wlbga_ipa_ilna_idx) &&
			kk == fectrl_wlbga_ipa_ilna_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(fectrl_wlbga_ipa_ilna_val[fem_idx]));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc10_sub3(phy_info_t *pi)
{
	uint16 fectrl_43556usb_epa_elna_idx[] = {2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128,
		129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 192,
		193, 196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 274, 275, 281};
	uint16 fectrl_43556usb_epa_elna_val[] = {96, 32, 8, 6, 2, 1, 96, 32, 8, 6, 2, 96, 32, 96,
		32, 32, 32, 32, 32, 96, 32, 96, 32, 32, 32, 32, 32, 128, 128, 128, 128, 128, 128,
		128, 128, 134, 130, 5, 4, 8, 48, 32, 64};
	uint16 fectrl_zeroval[] = {0};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < 320; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_43556usb_epa_elna_idx) &&
			kk == fectrl_43556usb_epa_elna_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(fectrl_43556usb_epa_elna_val[fem_idx]));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static void
wlc_phy_write_regtbl_fc10_sub4(phy_info_t *pi)
{
	uint16 fectrl_fcbga_ipa_ilna_idx[] = {2, 3, 9, 18, 19, 25, 66, 67, 73, 82, 83, 128, 129,
		130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 192, 193,
		196, 197, 200, 201, 204, 205, 210, 211, 258, 259, 265, 265, 274, 275, 281, 281};
	uint16 fectrl_fcbga_ipa_ilna_val[] = {128, 32, 32, 8, 1, 1, 128, 32, 32, 8, 8, 128, 32,
		128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 128, 32, 64, 64, 64, 64,
		64, 64, 64, 64, 72, 72, 4, 8, 8, 8, 64, 16, 16, 16};
	uint16 fectrl_zeroval[] = {0};
	uint kk, fem_idx = 0;
	for (kk = 0; kk < 320; kk++) {
		if (fem_idx < ARRAYSIZE(fectrl_fcbga_ipa_ilna_idx) &&
			kk == fectrl_fcbga_ipa_ilna_idx[fem_idx]) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
			&(fectrl_fcbga_ipa_ilna_val[fem_idx]));
			fem_idx++;
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FEMCTRLLUT, 1, kk, 16,
				fectrl_zeroval);
		}
	}
}

static INLINE void
wlc_phy_write_regtbl_fc10(phy_info_t *pi, phy_info_acphy_t *pi_ac)
{
	switch (BF3_FEMCTRL_SUB(pi_ac)) {
	case 0:
	        wlc_phy_write_regtbl_fc10_sub0(pi);
	        break;
	case 1:
	        wlc_phy_write_regtbl_fc10_sub1(pi);
	        break;
	case 2:
	        wlc_phy_write_regtbl_fc10_sub2(pi);
	        break;
	case 3:
	        wlc_phy_write_regtbl_fc10_sub3(pi);
	        break;
	case 4:
	        wlc_phy_write_regtbl_fc10_sub4(pi);
	        break;
	}
}

static void
wlc_phy_tx_gm_gain_boost(phy_info_t *pi)
{
	uint8 core;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
		BCM_REFERENCE(phyrxchain);
		FOREACH_ACTV_CORE(pi, phyrxchain, core) {
			MOD_RADIO_REGC(pi, TXGM_CFG1, core, gc_res, 0x1);
		}
	} else {
		if (BF_SROM11_GAINBOOSTA01(pi->u.pi_acphy)) {
			/* Boost A0/1 radio gain */
			FOREACH_CORE(pi, core) {
				MOD_RADIO_REGC(pi, TXMIX5G_CFG1, core, gainboost, 0x6);
				MOD_RADIO_REGC(pi, PGA5G_CFG1, core, gainboost, 0x6);
			}
		}
		if (RADIO2069REV(pi->pubpi->radiorev) <= 3) {
			/* Boost A2 radio gain */
			core = 2;
			MOD_RADIO_REGC(pi, TXMIX5G_CFG1, core, gainboost, 0x6);
			MOD_RADIO_REGC(pi, PGA5G_CFG1, core, gainboost, 0x6);
		}
	}
}

static void
wlc_phy_write_rx_farrow_pre_tiny(phy_info_t *pi, chan_info_rx_farrow *rx_farrow,
	chanspec_t chanspec)
{
	uint16 deltaphase_lo, deltaphase_hi;
	uint16 drift_period, farrow_ctrl;

#ifdef ACPHY_1X1_ONLY
	uint8 channel = CHSPEC_CHANNEL(chanspec);
	uint32 deltaphase;

	if (channel <= 14) {
		if (CHSPEC_IS20(chanspec))
			drift_period = 5120; /* 40x32x4 */
		else if (CHSPEC_IS40(chanspec))
			drift_period = 5120; /* 40x32x4 */
		else
			drift_period = 1280; /* 160x4x2 */
	} else {
		if (CHSPEC_IS20(chanspec))
			drift_period = 3840; /* 40x24x4 */
		else if (CHSPEC_IS40(chanspec))
			drift_period = 3840; /* 40x24x4 */
		else
			drift_period = 2880; /* 160x9x2 */
	}

	if (CHSPEC_IS80(chanspec)) {
		deltaphase = rx_farrow->deltaphase_80;
		farrow_ctrl = rx_farrow->farrow_ctrl_80;
	} else {
		deltaphase = rx_farrow->deltaphase_20_40;
		farrow_ctrl = rx_farrow->farrow_ctrl_20_40;
	}
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && !(ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		farrow_ctrl = (farrow_ctrl &
			~ACPHY_rxFarrowCtrl_rx_farrow_outShift_MASK(pi->pubpi->phy_rev));
	}
	deltaphase_lo = deltaphase & 0xffff;
	deltaphase_hi = (deltaphase >> 16) & 0xff;

#else  /* ACPHY_1X1_ONLY */
	UNUSED_PARAMETER(chanspec);

	/* Setup the Rx Farrow */
	deltaphase_lo = rx_farrow->deltaphase_lo;
	deltaphase_hi = rx_farrow->deltaphase_hi;
	drift_period = rx_farrow->drift_period;
	farrow_ctrl = rx_farrow->farrow_ctrl;

#endif  /* ACPHY_1X1_ONLY */
	/* Setup the Rx Farrow */
	WRITE_PHYREG(pi, rxFarrowDeltaPhase_lo, deltaphase_lo);
	WRITE_PHYREG(pi, rxFarrowDeltaPhase_hi, deltaphase_hi);
	WRITE_PHYREG(pi, rxFarrowDriftPeriod, drift_period);
	WRITE_PHYREG(pi, rxFarrowCtrl, farrow_ctrl);

	/* Use the same settings for the loopback Farrow */
	WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, deltaphase_lo);
	WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, deltaphase_hi);
	WRITE_PHYREG(pi, lbFarrowDriftPeriod, drift_period);
	WRITE_PHYREG(pi, lbFarrowCtrl, farrow_ctrl);
}

#ifdef WL11ULB
/* Function to set/reset 5/10MHz mode (cor. TCL proc is ulb_mode in chipc.tcl) */
/* ulb_mode: 0 - reset to normal mode
 * ulb_mode: 1 - 10MHz mode
 * ulb_mode: 2 - 5MHz mode
 */
void
wlc_phy_ulb_mode(phy_info_t *pi, uint8 ulb_mode)
{
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if ((ACMAJORREV_4(pi->pubpi->phy_rev) && (ACMINORREV_2(pi))) ||
	    ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		uint16 dac_rate_mode;
		uint8 orig_soft_sdfeFifoReset_val, orig_disable_stalls_val;
		uint8 orig_forceAfeClocksOff_val;
		uint16 orig_sdfeClkGatingCtrl_val;
		si_t *sih = pi->sh->sih;
		phy_ac_chanmgr_info_t *chanmgri = pi->u.pi_acphy->chanmgri;

		/* Set the PLL */
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			if (chanmgri->prev_pmu_ulbmode != ulb_mode) {
				if (!pi->sh->up) {
					si_pmu_set_ulbmode(sih, pi->sh->osh, ulb_mode);
					chanmgri->prev_pmu_ulbmode = ulb_mode;
				} else {
					PHY_ERROR(("wl%d: %s: si_pmu_set_ulbmode() was bypassed\n",
							pi->sh->unit,	__FUNCTION__));
				}
			}
		} else {
			si_pmu_set_ulbmode(sih, pi->sh->osh, ulb_mode);
		}

		/* Set the filters */
		if (!ISSIM_ENAB(pi->sh->sih)) {
			if (ulb_mode == PMU_ULB_BW_NONE) {
				/* normal mode */
				MOD_PHYREG(pi, UlbCtrl, useFirAB, 0x0);
				MOD_PHYREG(pi, UlbCtrl, useFirBbypA, 0x0);
			} else if (ulb_mode == PMU_ULB_BW_10MHZ) {
				/* 10MHz mode */
				MOD_PHYREG(pi, UlbCtrl, useFirAB, 0x0);
				MOD_PHYREG(pi, UlbCtrl, useFirBbypA, 0x1);
			} else {
				/* 5MHz mode */
				MOD_PHYREG(pi, UlbCtrl, useFirAB, 0x1);
				MOD_PHYREG(pi, UlbCtrl, useFirBbypA, 0x0);
			}

			/* To avoid Rx stalling */
			if (ulb_mode != PMU_ULB_BW_NONE) {
				/* ulb mode */
				MOD_PHYREG(pi, sdfeClkGatingCtrl, txlbclkmode_ovr, 0x1);
				MOD_PHYREG(pi, sdfeClkGatingCtrl, txlbclkmode_ovr_value, 0x1);
			} else {
				/* normal mode */
				MOD_PHYREG(pi, sdfeClkGatingCtrl, txlbclkmode_ovr, 0x1);
				MOD_PHYREG(pi, sdfeClkGatingCtrl, txlbclkmode_ovr_value, 0x0);
				MOD_PHYREG(pi, sdfeClkGatingCtrl, txlbclkmode_ovr, 0x0);
			}
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			    ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				if (ulb_mode != PMU_ULB_BW_NONE) {
					MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr, 0x1);
					MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr_value, 0x0);
				} else {
					MOD_PHYREG(pi, FFTSoftReset, lbsdadc_clken_ovr, 0x0);
				}
			}
		}

		/* Set the dac rate mode */
		if (ulb_mode == PMU_ULB_BW_NONE) {
			/* normal mode */
			dac_rate_mode = 1;
		} else if (ulb_mode == PMU_ULB_BW_10MHZ) {
			/* 10MHz mode */
			dac_rate_mode = 3;
		} else {
			/* 5MHz mode */
			dac_rate_mode = 2;
		}

		/* Store AFE clocks to PHY */
		orig_soft_sdfeFifoReset_val = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
		orig_disable_stalls_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		orig_sdfeClkGatingCtrl_val = READ_PHYREG(pi, sdfeClkGatingCtrl);
		orig_forceAfeClocksOff_val = READ_PHYREGFLD(pi, fineclockgatecontrol,
			forceAfeClocksOff);

		/* Stall AFE clocks to PHY */
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0x1);
		MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 0x1);
		WRITE_PHYREG(pi, sdfeClkGatingCtrl, 0xE);
		MOD_PHYREG(pi, fineclockgatecontrol, forceAfeClocksOff, 0x1);

		/* Tx phy toggle */
		si_core_cflags(sih, SICF_DAC, dac_rate_mode << 8);

		/* Restore AFE clocks to PHY */
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_soft_sdfeFifoReset_val);
		MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, orig_disable_stalls_val);
		WRITE_PHYREG(pi, sdfeClkGatingCtrl, orig_sdfeClkGatingCtrl_val);
		MOD_PHYREG(pi, fineclockgatecontrol, forceAfeClocksOff, orig_forceAfeClocksOff_val);
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Set the PLL */
		si_pmu_set_ulbmode(pi->sh->sih, pi->sh->osh, ulb_mode);

		/* Set Backend clks */
		if (ulb_mode == PMU_ULB_BW_NONE) {
			/* normal mode */
			WRITE_PHYREG(pi, UlbCtrl, 0x0);
			WRITE_PHYREG(pi, UlbCtrl_tx, 0x0);
		} else if (ulb_mode == PMU_ULB_BW_10MHZ) {
			/* 10MHz mode */
			WRITE_PHYREG(pi, UlbCtrl, 0x14);
			WRITE_PHYREG(pi, UlbCtrl_tx, 0x5);
		} else if (ulb_mode == PMU_ULB_BW_5MHZ) {
			/* 5MHz mode */
			WRITE_PHYREG(pi, UlbCtrl, 0xc);
			WRITE_PHYREG(pi, UlbCtrl_tx, 0x3);
		} else {
			/* 2.5MHz mode */
			WRITE_PHYREG(pi, UlbCtrl, 0x4);
			WRITE_PHYREG(pi, UlbCtrl_tx, 0x1);
		}
		wlc_phy_resetcca_acphy(pi);
	}
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* WL11ULB */

static void
BCMATTACHFN(phy_ac_chanmgr_nvram_attach)(phy_ac_chanmgr_info_t *ac_info)
{
	uint8 i;
	uint8 csml;
#ifndef BOARD_FLAGS3
	uint32 bfl3; /* boardflags3 */
#endif // endif
	phy_info_t *pi = ac_info->pi;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	csml = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_csml, 0x11);

	ac_info->ml_en =  (csml & 0xF);
	ac_info->chsm_en =  (csml & 0xF0) >> 4;

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		/* Save LESI enable/disable flag per band from NVRAM file that will be used */
		/* during band change */
		/* 2G */
		ac_info->lesi_perband[0] = (uint8)PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_lesi_en, 0, 0);
		/* 5G */
		ac_info->lesi_perband[1] = (uint8)PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_lesi_en, 1, 0);
	} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		/* Save LESI enable/disable flag per band from NVRAM file that will be used */
		/* during band change */
		/* 2G */
		ac_info->lesi_perband[0] = (uint8)PHY_GETINTVAR_ARRAY_SLICE(pi,
			rstr_lesi_en, 0);
		/* 5G */
		ac_info->lesi_perband[1] = (uint8)PHY_GETINTVAR_ARRAY_SLICE(pi,
			rstr_lesi_en, 1);
	}
	pi->sh->bphymrc_en = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_bphymrc_en, 0);

	if ((PHY_GETVAR_SLICE(pi, rstr_cckdigfilttype)) != NULL) {
		ac_info->acphy_cck_dig_filt_type = (uint8)PHY_GETINTVAR_SLICE(pi,
			rstr_cckdigfilttype);
	} else {
		if (ACMAJORREV_1(pi->pubpi->phy_rev) &&
			ACMINORREV_2(pi) &&
			((pi->epagain2g == 2) || (pi->extpagain2g == 2)) &&
			((pi->epagain5g == 2) || (pi->extpagain5g == 2)) &&
			PHY_XTAL_IS40M(pi)) {
			/* 43162yp improving ACPR */
			ac_info->acphy_cck_dig_filt_type = 0x02;
		} else {
			/* bit0 is gaussian shaping and bit1 & 2 are for RRC alpha */
			ac_info->acphy_cck_dig_filt_type = 0x01;
		}
	}
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		/* low mcs gamma and gain values for PAPRR */
		for (i = 0; i < NUM_MCS_PAPRR_GAMMA; i++) {
			pi->paprrmcsgamma2g[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgamma2g, i, -1));
			pi->paprrmcsgain2g[i] = (uint8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgain2g, i, 128));
			pi->paprrmcsgamma5g20[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgamma5g20, i, -1));
			pi->paprrmcsgain5g20[i] = (uint8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgain5g20, i, 128));
			pi->paprrmcsgamma5g40[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgamma5g40, i, -1));
			pi->paprrmcsgain5g40[i] = (uint8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgain5g40, i, 128));
			pi->paprrmcsgamma5g80[i] = (int16) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgamma5g80, i, -1));
			pi->paprrmcsgain5g80[i] = (uint8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
				rstr_paprrmcsgain5g80, i, 128));
		}
	}
#ifndef WL_FDSS_DISABLED
	pi->fdss_bandedge_2g_en = (uint8) (PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_fdss_bandedge_2g_en, 0));
	pi->fdss_interp_en = (uint8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_fdss_interp_en, 1));
	for (i = 0; i < PHY_CORE_MAX; i++) {
		pi->fdss_level_2g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_fdss_level_2g, i, -1));
		pi->fdss_level_5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_fdss_level_5g, i, -1));
		pi->fdss_level_2g_ch13[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_fdss_level_2g_ch13, i, (pi->fdss_level_2g[i])));
		pi->fdss_level_2g_ch1[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT_SLICE(pi,
			rstr_fdss_level_2g_ch1, i, (pi->fdss_level_2g[i])));
	}
#endif /* !WL_FDSS_DISABLED */
	ac_info->cfg->srom_paprdis = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_paprdis, FALSE);
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		ac_info->cfg->srom_paprdis = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi,
			rstr_paprdis, TRUE);
	} else {
		ac_info->cfg->srom_paprdis = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi,
			rstr_paprdis, FALSE);
	}
	ac_info->cfg->srom_papdwar = (int8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_papdwar, -1);
	ac_info->cfg->srom_txnoBW80ClkSwitch = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_txnoBW80ClkSwitch, 0);
	ac_info->cfg->srom_nonbf_logen_mode_en = (uint8)PHY_GETINTVAR_DEFAULT_SLICE
		(pi, rstr_nonbf_logen_mode_en, 0);
	if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
		ac_info->cfg->srom_tssisleep_en =
			(uint)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_tssisleep_en, 7);
	} else {
		ac_info->cfg->srom_tssisleep_en =
			(uint)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_tssisleep_en, 0);
	}
#ifndef BOARD_FLAGS
	BF_SROM11_GAINBOOSTA01(pi_ac) = ((BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
		BFL_SROM11_GAINBOOSTA01) != 0);
#endif /* BOARD_FLAGS */

#ifndef BOARD_FLAGS2
	BF2_SROM11_APLL_WAR(pi_ac) = ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_SROM11_APLL_WAR) != 0);
	BF2_2G_SPUR_WAR(pi_ac) = ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_2G_SPUR_WAR) != 0);
	BF2_DAC_SPUR_IMPROVEMENT(pi_ac) = (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
		BFL2_DAC_SPUR_IMPROVEMENT) != 0;
#endif /* BOARD_FLAGS2 */

#ifndef BOARD_FLAGS3
	if ((PHY_GETVAR_SLICE(pi, rstr_boardflags3)) != NULL) {
		bfl3 = (uint32)PHY_GETINTVAR_SLICE(pi, rstr_boardflags3);
		BF3_AVVMID_FROM_NVRAM(pi_ac) = (bfl3 & BFL3_AVVMID_FROM_NVRAM)
			>> BFL3_AVVMID_FROM_NVRAM_SHIFT;
		BF3_BBPLL_SPR_MODE_DIS(pi_ac) = ((bfl3 & BFL3_BBPLL_SPR_MODE_DIS) != 0);
		BF3_PHASETRACK_MAX_ALPHABETA(pi_ac) = (bfl3 & BFL3_PHASETRACK_MAX_ALPHABETA) >>
			BFL3_PHASETRACK_MAX_ALPHABETA_SHIFT;
		BF3_ACPHY_LPMODE_2G(pi_ac) = (bfl3 & BFL3_ACPHY_LPMODE_2G) >>
			BFL3_ACPHY_LPMODE_2G_SHIFT;
		BF3_ACPHY_LPMODE_5G(pi_ac) = (bfl3 & BFL3_ACPHY_LPMODE_5G) >>
			BFL3_ACPHY_LPMODE_5G_SHIFT;
		BF3_RSDB_1x1_BOARD(pi_ac) = (bfl3 & BFL3_1X1_RSDB_ANT) >>
			BFL3_1X1_RSDB_ANT_SHIFT;
		BF3_5G_SPUR_WAR(pi_ac) = ((bfl3 & BFL3_5G_SPUR_WAR) != 0);
	} else {
		BF3_BBPLL_SPR_MODE_DIS(pi_ac) = 0;
		BF3_PHASETRACK_MAX_ALPHABETA(pi_ac) = 0;
		BF3_ACPHY_LPMODE_2G(pi_ac) = 0;
		BF3_ACPHY_LPMODE_5G(pi_ac) = 0;
		BF3_RSDB_1x1_BOARD(pi_ac) = 0;
		BF3_5G_SPUR_WAR(pi_ac) = 0;
	}
#endif /* BOARD_FLAGS3 */

	pi->sromi->ofdmfilttype = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_ofdmfilttype_5gbe, 127);
	pi->sromi->ofdmfilttype_2g = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_ofdmfilttype_2gbe, 127);

	if ((PHY_GETVAR_SLICE(pi, ed_thresh2g)) != NULL) {
		pi_ac->sromi->ed_thresh2g = (int32)PHY_GETINTVAR_SLICE(pi, ed_thresh2g);
	} else {
		pi_ac->sromi->ed_thresh2g = 0;
	}

	if ((PHY_GETVAR_SLICE(pi, ed_thresh5g)) != NULL) {
		pi_ac->sromi->ed_thresh5g = (int32)PHY_GETINTVAR_SLICE(pi, ed_thresh5g);
	} else {
		pi_ac->sromi->ed_thresh5g = 0;
	}

	if ((PHY_GETVAR_SLICE(pi, "eu_edthresh2g")) != NULL) {
		pi->srom_eu_edthresh2g = (int8)PHY_GETINTVAR_SLICE(pi, "eu_edthresh2g");
	} else {
		pi->srom_eu_edthresh2g = 0;
	}
	if ((PHY_GETVAR_SLICE(pi, "eu_edthresh5g")) != NULL) {
		pi->srom_eu_edthresh5g = (int8)PHY_GETINTVAR_SLICE(pi, "eu_edthresh5g");
	} else {
		pi->srom_eu_edthresh5g = 0;
	}

	if ((PHY_GETVAR_SLICE(pi, "hwaci_sw_mitigation")) != NULL) {
		pi_ac->sromi->hwaci_sw_mitigation =
			(int8)PHY_GETINTVAR_SLICE(pi, "hwaci_sw_mitigation");
	} else {
		pi_ac->sromi->hwaci_sw_mitigation = 0;
	}

	pi->sromi->lpflags = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_lpflags, 0);
	pi->sromi->subband5Gver =
		(uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_subband5gver, PHY_SUBBAND_4BAND);
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

void
phy_ac_chanmgr_write_rx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
                             chanspec_t chanspec_sc, int sc_mode)
{
	uint8 ch, num, den, bw, M, vco_div, core;
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING];
	uint32 fcw, fcw1, tmp_low = 0, tmp_high = 0;
	uint32 fc, fc1;
	chanspec_t chanspec_sel = chanspec;
	bool vco_12GHz = pi->u.pi_acphy->chanmgri->vco_12GHz;
	if (sc_mode == 1) {
		chanspec_sel = chanspec_sc;
	}
	bw = CHSPEC_IS20(chanspec_sel) ? PHYBW_20: CHSPEC_IS40(chanspec_sel) ? PHYBW_40 :
		(CHSPEC_IS80(chanspec_sel) || PHY_AS_80P80(pi, chanspec_sel)) ? PHYBW_80 :
		PHYBW_160;

	if (CHSPEC_IS2G(chanspec_sel)) {
		if (!vco_12GHz) {
			num = 3;
			den = 2;
		} else {
			num = 4;
			den = 1;
		}
	} else {
		if (!vco_12GHz) {
			num = 2;
			den = 3;
		} else {
			num = 2;
			den = 1;
		}
	}

	if (vco_12GHz) {
		if ((pi->u.pi_acphy->chanmgri->data->fast_adc_en) ||
			(ACMAJORREV_4(pi->pubpi->phy_rev) && CHSPEC_IS8080(chanspec))) {
			M = SIPO_DIV_FAST * PHYBW_80 / bw;
			vco_div = AFE_DIV_FAST * ADC_DIV_FAST;
		} else {
			M = SIPO_DIV_SLOW;
			vco_div = AFE_DIV_BW(bw) * ADC_DIV_SLOW;
		}
	} else {
		if (CHSPEC_IS20(chanspec_sel)) {
			M = SIPO_DIV_SLOW;
			vco_div = 6;
		} else if (CHSPEC_IS40(chanspec_sel)) {
			M = SIPO_DIV_SLOW;
			vco_div = 3;
		} else if (CHSPEC_IS80(chanspec_sel) ||
				PHY_AS_80P80(pi, chanspec_sel)) {
			M = SIPO_DIV_FAST;
			vco_div = 1;
		} else if (CHSPEC_IS160(chanspec_sel)) {
			M = SIPO_DIV_FAST;
			vco_div = 1;
			ASSERT(0);
		} else {
			M = SIPO_DIV_FAST;
			vco_div = 1;
		}
	}

	if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		const uint8 afeclkdiv_arr[] = {2, 16, 4, 8, 3, 24, 6, 12};
		const uint8 adcclkdiv_arr[] = {1, 2, 3, 6};
		const uint8 sipodiv_arr[] = {12, 8};
		const chan_info_radio20693_altclkplan_t *altclkpln = altclkpln_radio20693;
		int row;
		if (ROUTER_4349(pi)) {
			altclkpln = altclkpln_radio20693_router4349;
		}
		row = wlc_phy_radio20693_altclkpln_get_chan_row(pi);
		if ((row >= 0) && (pi->u.pi_acphy->chanmgri->data->fast_adc_en == 0)) {
			num = CHSPEC_IS2G(pi->radio_chanspec) ? 4 : 2;
			M = sipodiv_arr[altclkpln[row].sipodiv];
			den = 1;
			vco_div = afeclkdiv_arr[altclkpln[row].afeclkdiv] *
				adcclkdiv_arr[altclkpln[row].adcclkdiv];
		}
	}
	/* bits_in_mu = 24 */
	/*
	fcw = (num * phy_utils_channel2freq(ch) * (((uint32)(1<<31))/
		(den * vco_div * 2 * M * bw)))>> 7;
	*/
	if (CHSPEC_IS8080(chanspec) &&
	    !(ACMAJORREV_33(pi->pubpi->phy_rev) ||
	      ACMAJORREV_37(pi->pubpi->phy_rev) ||
	      ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		FOREACH_CORE(pi, core) {
			if (core == 0) {
				ch = wf_chspec_primary80_channel(chanspec);
				fc = wf_channel2mhz(ch, WF_CHAN_FACTOR_5_G);

				math_uint64_multiple_add(&tmp_high, &tmp_low, fc * num, 1 << 24, 0);
				math_uint64_divide(&fcw, tmp_high, tmp_low,
					(uint32) (den * vco_div * 2 * M * bw));

				PHY_INFORM(("%s: fcw 0x%0x ch %d freq %d vco_div %d bw %d\n",
					__FUNCTION__, fcw, ch, phy_utils_channel2freq(ch),
					vco_div, bw));

				MOD_PHYREG(pi, RxSdFeConfig20, fcw_value_lo, fcw & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig30, fcw_value_hi,
					(fcw >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig30, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
			} else if (core == 1) {
				ch = wf_chspec_secondary80_channel(chanspec);
				fc = wf_channel2mhz(ch, WF_CHAN_FACTOR_5_G);

				math_uint64_multiple_add(&tmp_high, &tmp_low, fc * num, 1 << 24, 0);
				math_uint64_divide(&fcw, tmp_high, tmp_low,
					(uint32) (den * vco_div * 2 * M * bw));

				PHY_INFORM(("%s: fcw 0x%0x ch %d freq %d vco_div %d bw %d\n",
					__FUNCTION__, fcw, ch, phy_utils_channel2freq(ch),
					vco_div, bw));

				MOD_PHYREG(pi, RxSdFeConfig21, fcw_value_lo, fcw & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig31, fcw_value_hi,
					(fcw >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig31, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
			}
		}
	} else {
		//ch = CHSPEC_CHANNEL(chanspec);
		//fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ? WF_CHAN_FACTOR_2_4_G
		//: WF_CHAN_FACTOR_5_G);
		if (ACMAJORREV_33(pi->pubpi->phy_rev) &&
				(PHY_AS_80P80(pi, chanspec_sel))) {
			wf_chspec_get_80p80_channels(chanspec, chans);
			fc  = wf_channel2mhz(chans[0], WF_CHAN_FACTOR_5_G);
			fc1 = wf_channel2mhz(chans[1], WF_CHAN_FACTOR_5_G);

			math_uint64_multiple_add(&tmp_high, &tmp_low, fc * num, 1 << 24, 0);
			math_uint64_divide(&fcw, tmp_high, tmp_low,
					(uint32) (den * vco_div * 2 * M * bw));

			math_uint64_multiple_add(&tmp_high, &tmp_low, fc1 * num, 1 << 24, 0);
			math_uint64_divide(&fcw1, tmp_high, tmp_low,
					(uint32) (den * vco_div * 2 * M * bw));

			PHY_INFORM(("%s: fcw0 0x%0x ch0 %d fc %d freq0 %d vco_div %d bw %d\n",
				__FUNCTION__, fcw, chans[0], fc,
				phy_utils_channel2freq((uint)chans[0]), vco_div, bw));
			PHY_INFORM(("%s: fcw1 0x%0x ch1 %d fc %d freq1 %d vco_div %d bw %d\n",
				__FUNCTION__, fcw1, chans[1], fc1,
				phy_utils_channel2freq((uint)chans[1]), vco_div, bw));
		} else {
			ch = CHSPEC_CHANNEL(chanspec_sel);
			fc = wf_channel2mhz(ch, CHSPEC_IS2G(chanspec_sel) ? WF_CHAN_FACTOR_2_4_G
				: WF_CHAN_FACTOR_5_G);

			math_uint64_multiple_add(&tmp_high, &tmp_low, fc * num, 1 << 24, 0);
			math_uint64_divide(&fcw, tmp_high, tmp_low,
				(uint32) (den * vco_div * 2 * M * bw));
			fcw1 = fcw;
			PHY_INFORM(("%s: fcw 0x%0x ch %d freq %d vco_div %d bw %d\n",
				__FUNCTION__, fcw, ch, phy_utils_channel2freq(ch), vco_div, bw));
		}

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			if (sc_mode == 1) {
				if (CHSPEC_BW(chanspec) != CHSPEC_BW(chanspec_sc)) {
					printf("NOT SUPPORT SC CORE BW != NORMAL CORE BW !!! \n");
					ASSERT(0);
				}
				MOD_PHYREG(pi, RxSdFeConfig2_path3, fcw_value_lo, fcw & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path3, fcw_value_hi,
					(fcw >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path3, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
			} else {
				MOD_PHYREG(pi, RxSdFeConfig2_path0, fcw_value_lo, fcw & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig2_path1, fcw_value_lo, fcw & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig2_path2, fcw_value_lo, fcw1 & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig2_path3, fcw_value_lo, fcw1 & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path0, fcw_value_hi,
					(fcw >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path1, fcw_value_hi,
					(fcw >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path2, fcw_value_hi,
					(fcw1 >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path3, fcw_value_hi,
					(fcw1 >> 16) & 0xffff);
				MOD_PHYREG(pi, RxSdFeConfig3_path0, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
				MOD_PHYREG(pi, RxSdFeConfig3_path1, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
				MOD_PHYREG(pi, RxSdFeConfig3_path2, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
				MOD_PHYREG(pi, RxSdFeConfig3_path3, fast_ADC_en,
					(pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
			}
		} else {
			MOD_PHYREG(pi, RxSdFeConfig2, fcw_value_lo, fcw & 0xffff);
			MOD_PHYREG(pi, RxSdFeConfig3, fcw_value_hi, (fcw >> 16) & 0xffff);
			MOD_PHYREG(pi, RxSdFeConfig3, fast_ADC_en,
			           (pi->u.pi_acphy->chanmgri->data->fast_adc_en & 0x1));
		}
	}
}

void
wlc_phy_farrow_setup_acphy(phy_info_t *pi, chanspec_t chanspec)
{
#ifdef ACPHY_1X1_ONLY
	uint32 dac_resamp_fcw;
	uint16 MuDelta_l, MuDelta_u;
	uint16 MuDeltaInit_l, MuDeltaInit_u;
#endif // endif
	uint16 channel = CHSPEC_CHANNEL(chanspec);
	const uint16 *resamp_set = NULL;
	chan_info_tx_farrow *tx_farrow = NULL;
	chan_info_rx_farrow *rx_farrow;
	int bw_idx = 0;
	int tbl_idx = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (ISSIM_ENAB(pi->sh->sih)) {
		/* Use channel 7(2g)/151(5g) settings for Quickturn */
		if (CHSPEC_IS2G(chanspec)) {
			channel = 7;
		} else {
			channel = 155;
		}
	}

	/* China 40M Spur WAR */
	if (ACMAJORREV_0(pi->pubpi->phy_rev)) {
		uint8 core;
		/* Cleanup Overrides */
		MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div_ovr, 0);
		MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div, 0x0);
		pi->sdadc_config_override = FALSE;

		FOREACH_CORE(pi, core) {
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_flashhspd, 0);
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_ctrl_flash17lvl, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_ctrl_flash17lvl, 0);
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_mode, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_mode, 0);
		}
	}

#ifdef ACPHY_1X1_ONLY
	bw_idx = 0;
#else /* ACPHY_1X1_ONLY */
	bw_idx = CHSPEC_BW_LE20(chanspec)? 0 : (CHSPEC_IS40(chanspec)? 1 : 2);
#endif /* ACPHY_1X1_ONLY */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* Compute rx farrow setup */
		wlc_phy_write_rx_farrow_acphy(pi->u.pi_acphy->chanmgri, chanspec);
	} else {
		ASSERT(pi->u.pi_acphy->chanmgri->rx_farrow != NULL);
		/* Find the Rx Farrow settings in the table for the specific b/w and channel */
		for (tbl_idx = 0; tbl_idx < ACPHY_NUM_CHANS; tbl_idx++) {
			rx_farrow = &pi->u.pi_acphy->chanmgri->rx_farrow[bw_idx][tbl_idx];
			if (rx_farrow->chan == channel) {
				wlc_phy_write_rx_farrow_pre_tiny(pi, rx_farrow, chanspec);
				break;
			}
		}

		/*
		 * No need to iterate through the Tx Farrow table, since the channels have the same
		 * order as the Rx Farrow table.
		 */

		if (tbl_idx == ACPHY_NUM_CHANS) {
			PHY_ERROR(("wl%d: %s: Failed to find Farrow settings"
				   " for bw=%d, channel=%d\n",
				   pi->sh->unit, __FUNCTION__, CHSPEC_BW(chanspec), channel));
			return;
		}
	}

	ASSERT(pi->u.pi_acphy->chanmgri->tx_farrow != NULL);
#ifdef ACPHY_1X1_ONLY
	ASSERT(phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1);
	tx_farrow = &pi->u.pi_acphy->chanmgri->tx_farrow[0][tbl_idx];
	dac_resamp_fcw = tx_farrow->dac_resamp_fcw;

	if (CHSPEC_IS80(chanspec))
	{
		dac_resamp_fcw += (dac_resamp_fcw >> 1);
	}

	dac_resamp_fcw = (dac_resamp_fcw + 32) >> 6;

	MuDelta_l = (dac_resamp_fcw & 0xFFFF);
	MuDelta_u = (dac_resamp_fcw & 0xFF0000) >> 16;
	MuDeltaInit_l = (dac_resamp_fcw & 0xFFFF);
	MuDeltaInit_u = (dac_resamp_fcw & 0xFF0000) >> 16;

	wlc_phy_tx_farrow_mu_setup(pi, MuDelta_l, MuDelta_u, MuDeltaInit_l, MuDeltaInit_u);
#else /* ACPHY_1X1_ONLY */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* Compute tx farrow setup */
		wlc_phy_write_tx_farrow_acphy(pi->u.pi_acphy->chanmgri, chanspec);
	} else {
		tx_farrow = &pi->u.pi_acphy->chanmgri->tx_farrow[bw_idx][tbl_idx];
		wlc_phy_tx_farrow_mu_setup(pi, tx_farrow->MuDelta_l, tx_farrow->MuDelta_u,
			tx_farrow->MuDeltaInit_l, tx_farrow->MuDeltaInit_u);
	}
#endif /* ACPHY_1X1_ONLY */

	/* China 40M Spur WAR */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) &&
	    (pi->afe_override) && CHSPEC_IS40(pi->radio_chanspec)) {
		uint16 fc;
		if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14)
			fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		else
			fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));

		/* AFE Settings */
		if (fc == 5310) {
			uint8 core;
			MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div_ovr, 0x1);
			MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div, 0x0);

			FOREACH_CORE(pi, core) {
				MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd, 1);
				MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core,
				             afe_iqadc_flashhspd, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_ctrl_flash17lvl, 0);
				MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core,
				             afe_ctrl_flash17lvl, 1);
				MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_mode, 1);
				MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_mode, 1);
			}

			ACPHY_REG_LIST_START
				MOD_RADIO_REG_ENTRY(pi, RF0, AFEDIV1, afediv_main_driver_size, 8)
				MOD_RADIO_REG_ENTRY(pi, RF0, AFEDIV2, afediv_repeater1_dsize, 8)
				MOD_RADIO_REG_ENTRY(pi, RF0, AFEDIV2, afediv_repeater2_dsize, 8)
			ACPHY_REG_LIST_EXECUTE(pi);

			/* Set Override variable to pick up correct settings during cals */
			pi->sdadc_config_override = TRUE;
		} else if (fc == 5270) {
			MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div_ovr, 0x1);
			MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div, 0x2);
		}

		/* Resampler Settings */
		if (fc == 5270)
			resamp_set = resamp_cnwar_5270;
		else if (fc == 5310)
			resamp_set = resamp_cnwar_5310;

		if (resamp_set != NULL) {
			WRITE_PHYREG(pi, rxFarrowDeltaPhase_lo, resamp_set[0]);
			WRITE_PHYREG(pi, rxFarrowDeltaPhase_hi, resamp_set[1]);
			WRITE_PHYREG(pi, rxFarrowDriftPeriod, resamp_set[2]);
			WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, resamp_set[3]);
			WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, resamp_set[4]);
			WRITE_PHYREG(pi, lbFarrowDriftPeriod, resamp_set[5]);
			ACPHYREG_BCAST(pi, TxResamplerMuDelta0l, resamp_set[6]);
			ACPHYREG_BCAST(pi, TxResamplerMuDelta0u, resamp_set[7]);
			ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0l, resamp_set[8]);
			ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0u, resamp_set[9]);
		}
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev))
		MOD_PHYREG(pi, AfeClkDivOverrideCtrl, afediv_sel_div_ovr, 0x1);
}

#define QT_2G_DEFAULT_28NM 2465
#define QT_5G_DEFAULT_28NM 5807
#define VELOCE_2G_DEFAULT_28NM 2472
#define VELOCE_5G_DEFAULT_28NM 5760

void
wlc_phy_tx_farrow_setup_28nm(phy_info_t *pi, uint16 dac_rate_mode)
{
	uint16 bits_in_mu = 23;
	uint16 fc;
	uint16 dac_div_idx, bw_idx, bw;
	uint16 dac_div_ratio;
	// lut entries [2G20 2G40 2G80 2G160 5G20 5G40 5G80 5G160]
#ifdef REGULATORY_DEBUG
	uint8 p_afediv_lut[] = {48, 48, 32, 16, 96, 96, 64, 28};
#else
	uint8 p_afediv_lut[] = {96, 48, 32, 16, 192, 96, 64, 28};
#endif // endif
	uint32 dac_resamp_fcw, numerator_hi, numerator_lo;
	uint64 numerator;
	uint16 mu_delta_l, mu_delta_u;
	uint16 regval;
	phy_ac_radio_data_t *radio_data = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	bw_idx = CHSPEC_BW_LE20(pi->radio_chanspec)? 0 :
		(CHSPEC_IS40(pi->radio_chanspec)? 1 : CHSPEC_IS80(pi->radio_chanspec)? 2 : 3);
	bw = 10*(1<<(bw_idx+1));

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) && (radio_data->dac_clk_x2_mode == 1)) {
		/* Set 20MHz divider ratios to that of 40MHz when dac_clk_x2_mode = 1 */
		p_afediv_lut[0] = p_afediv_lut[1];
		p_afediv_lut[4] = p_afediv_lut[5];
	}

	if (ISSIM_ENAB(pi->sh->sih)) {
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			p_afediv_lut[7] = 32;    /* 5g 160mhz */
			fc = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			    VELOCE_2G_DEFAULT_28NM : VELOCE_5G_DEFAULT_28NM;
		} else {
			/* 4347 QT AFE CLK MODEL */
			fc = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			    QT_2G_DEFAULT_28NM : QT_5G_DEFAULT_28NM;
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		} else {
			fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		}
	}
	if (dac_rate_mode == 2) {
		dac_div_ratio = CHSPEC_IS2G(pi->radio_chanspec) ? 128 : 256;
		if (CHSPEC_IS40(pi->radio_chanspec)) dac_div_ratio >>= 1;
	}
	else {
		dac_div_idx = CHSPEC_IS2G(pi->radio_chanspec) ? bw_idx : bw_idx + 4;
		dac_div_ratio = p_afediv_lut[dac_div_idx];
	}

	/* Extra 1 bit left shift for rounding */
	numerator = ((uint64)bw << (bits_in_mu + 1)) * dac_div_ratio;
	numerator_hi = (uint32)(numerator >> 32);
	numerator_lo = (uint32)(numerator & 0xFFFFFFFF);
	math_uint64_divide(&dac_resamp_fcw, numerator_hi, numerator_lo, (uint32)fc);

	/* Add 1 and right shift by 1 to round */
	dac_resamp_fcw = (dac_resamp_fcw + 1) >> 1;

	mu_delta_l = (uint16)(dac_resamp_fcw & 0xFFFF);
	mu_delta_u = (uint16)((dac_resamp_fcw >> 16) & 0xFF);

	ACPHYREG_BCAST(pi, TxResamplerMuDelta0l, mu_delta_l);
	ACPHYREG_BCAST(pi, TxResamplerMuDelta0u, mu_delta_u);
	ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0l, mu_delta_l);
	ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0u, mu_delta_u);

	PHY_INFORM(("wl%d %s: band=%d fc=%d fcw=0x%x%x\n", PI_INSTANCE(pi), __FUNCTION__,
		(CHSPEC_IS2G(pi->radio_chanspec))?2:5, fc, mu_delta_u, mu_delta_l));

	if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en &&
	    ACMAJORREV_GE40(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, sdfeClkGatingCtrl, disableRxStallonTx, 0x1);
	}
	/* Enable the Tx resampler on all cores */
	regval = READ_PHYREG(pi, TxResamplerEnable0);
	regval |= (1 << ACPHY_TxResamplerEnable0_enable_tx_SHIFT(pi->pubpi->phy_rev));
	if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		/* SWWLAN-117514 - Fix for Tx stall issue. */
		/* These bits need to be set unless core2coresync is enabled */
		regval |=
		(1 << ACPHY_TxResamplerEnable0_tx_fifo_txresetEn_SHIFT(pi->pubpi->phy_rev)) |
		(1 << ACPHY_TxResamplerEnable0_txfe_baseband_enfree_SHIFT(pi->pubpi->phy_rev));
	}
	ACPHYREG_BCAST(pi, TxResamplerEnable0,  regval);
}

void
wlc_phy_rx_farrow_setup_28nm(phy_info_t *pi, chanspec_t chanspec_sc, uint8 sc_mode)
{
	uint16 fc, afe_div_ratio, fcw_hi, fcw_lo;
	uint16 drift_period, bw_idx, bw;
	uint16 afe_div_idx;
	uint32 fcw;
	uint32 numerator_hi, numerator_lo;
	uint64 numerator;
	uint8 p_afediv_lut[] = {96, 48, 32, 16, 192, 96, 64, 32};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (sc_mode == 0) {
		bw_idx = CHSPEC_BW_LE20(pi->radio_chanspec) ? 0 :
			CHSPEC_IS40(pi->radio_chanspec) ? 1 :
			CHSPEC_IS80(pi->radio_chanspec) ? 2 : 3;
		bw = 10*(1<<(bw_idx+1));

		if (ISSIM_ENAB(pi->sh->sih)) {
			if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				fc = (CHSPEC_IS2G(pi->radio_chanspec)) ?
				VELOCE_2G_DEFAULT_28NM : VELOCE_5G_DEFAULT_28NM;
			} else {
				/* 4347 QT AFE CLK MODEL */
				fc = (CHSPEC_IS2G(pi->radio_chanspec)) ?
				QT_2G_DEFAULT_28NM : QT_5G_DEFAULT_28NM;
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
			} else {
				fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
			}
		}
	} else {
		bw_idx = CHSPEC_BW_LE20(chanspec_sc)? 0 : CHSPEC_IS40(chanspec_sc)? 1 :
		CHSPEC_IS80(chanspec_sc)? 2 : 3;

		bw = 10*(1<<(bw_idx+1));
		fc = wf_channel2mhz(CHSPEC_CHANNEL(chanspec_sc), CHSPEC_IS2G(chanspec_sc) ?
			WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);
	}

	if (sc_mode == 0)
		afe_div_idx = CHSPEC_IS2G(pi->radio_chanspec) ? bw_idx : bw_idx + 4;
	else
		afe_div_idx = CHSPEC_IS2G(chanspec_sc) ? bw_idx : bw_idx + 4;

	afe_div_ratio = p_afediv_lut[afe_div_idx];

	drift_period = afe_div_ratio * bw;
	/* Extra 1 bit left shift to help in rounding */
	numerator = (uint64)(fc) << (N_BITS_RX_FARR + 1);
	numerator_hi = (uint32)(numerator >> 32);
	numerator_lo = (uint32)(numerator & 0xFFFFFFFF);
	math_uint64_divide(&fcw, numerator_hi, numerator_lo, (uint32)drift_period);

	/* Extra 1 bit left shift to help in rounding */
	fcw -= (1 << (N_BITS_RX_FARR + 1));

	/* Add 1 and right shift by 1 for rounding */
	fcw = ((fcw + 1) >> 1);

	fcw_lo = (uint16)(fcw & 0xFFFF);
	fcw_hi = (uint16)((fcw >> 16) & 0xFF);

	if (sc_mode == 0) {
		ACPHYREG_BCAST(pi, rxFarrowDeltaPhase_lo, fcw_lo);
		ACPHYREG_BCAST(pi, rxFarrowDeltaPhase_hi, fcw_hi);
		WRITE_PHYREG(pi, rxFarrowDriftPeriod, drift_period);
		ACPHYREG_BCAST(pi, rxFarrowCtrl, 0xb3);

		PHY_INFORM(("wl%d %s: band=%d fc=%d fcw=0x%x%x\n", PI_INSTANCE(pi), __FUNCTION__,
			(CHSPEC_IS2G(pi->radio_chanspec))?2:5, fc, fcw_hi, fcw_lo));

		ACPHYREG_BCAST(pi, lbFarrowDeltaPhase_lo, fcw_lo);
		ACPHYREG_BCAST(pi, lbFarrowDeltaPhase_hi, fcw_hi);
		WRITE_PHYREG(pi, lbFarrowDriftPeriod, drift_period);
		ACPHYREG_BCAST(pi, lbFarrowCtrl, 0xb3);
	} else {
		phy_utils_write_phyreg_p1c(pi, ACPHY_rxFarrowDeltaPhase_lo(pi->pubpi->phy_rev),
			fcw_lo);
		phy_utils_write_phyreg_p1c(pi, ACPHY_rxFarrowDeltaPhase_hi(pi->pubpi->phy_rev),
			fcw_hi);
		phy_utils_write_phyreg_p1c(pi, ACPHY_rxFarrowDriftPeriod(pi->pubpi->phy_rev),
			drift_period);
		phy_utils_write_phyreg_p1c(pi, ACPHY_rxFarrowCtrl(pi->pubpi->phy_rev), 0xb3);

		PHY_INFORM(("wl%d %s: band=%d fc=%d fcw=0x%x%x\n", PI_INSTANCE(pi), __FUNCTION__,
			(CHSPEC_IS2G(chanspec_sc))?2:5, fc, fcw_hi, fcw_lo));

		phy_utils_write_phyreg_p1c(pi, ACPHY_lbFarrowDeltaPhase_lo(pi->pubpi->phy_rev),
			fcw_lo);
		phy_utils_write_phyreg_p1c(pi, ACPHY_lbFarrowDeltaPhase_hi(pi->pubpi->phy_rev),
			fcw_hi);
		phy_utils_write_phyreg_p1c(pi, ACPHY_lbFarrowDriftPeriod(pi->pubpi->phy_rev),
			drift_period);
		phy_utils_write_phyreg_p1c(pi, ACPHY_lbFarrowCtrl(pi->pubpi->phy_rev), 0xb3);
	}
}

void
wlc_phy_farrow_setup_28nm(phy_info_t *pi, uint16 dac_rate_mode)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	wlc_phy_rx_farrow_setup_28nm(pi, 0, 0);
	wlc_phy_tx_farrow_setup_28nm(pi, dac_rate_mode);

	if (ISSIM_ENAB(pi->sh->sih) && (!pi_ac->chanmgri->veloce_farrow_db)) {
		/* Disable Clock Stalling */
		ACPHY_ENABLE_STALL(pi, 1);
	} else {
		/* Enable Clock Stalling */
		ACPHY_ENABLE_STALL(pi, 0);
	}
}

/* proc tx_farrow_setup_28nm_ulp */
void
wlc_phy_tx_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_tx_mode)
{
	uint16 bits_in_mu = 23;
	uint16 fc, fi = 160;
	uint16 idx = (ulp_tx_mode - 1);
	uint16 dac_div_ratio;
	uint8* p_afediv_lut;
	uint8 afediv_lut_for_fc_ge_5745[] = {16, 12, 16, 12, 16};
	uint8 afediv_lut_for_fc_ge_5400[] = {16, 12, 16, 12, 14};
	uint8 afediv_lut_for_fc_ge_5180[] = {16, 12, 16, 12, 14};
	uint8 afediv_lut_for_fc_lt_5180[] = {14, 10, 14, 10, 13};
	uint32 dac_resamp_fcw, numerator_hi, numerator_lo;
	uint64 numerator;
	uint16 mu_delta_l, mu_delta_u;
	uint16 regval;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14) {
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	} else {
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	}

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		if (fc >= 5180) {
			p_afediv_lut = afediv_lut_for_fc_ge_5180;
		} else {
			p_afediv_lut = afediv_lut_for_fc_lt_5180;
		}
	} else {
		if (fc >= 5745) {
			p_afediv_lut = afediv_lut_for_fc_ge_5745;
		} else if (fc >= 5400) {
			p_afediv_lut = afediv_lut_for_fc_ge_5400;
		} else if (fc >= 5180) {
			p_afediv_lut = afediv_lut_for_fc_ge_5180;
		} else {
			p_afediv_lut = afediv_lut_for_fc_lt_5180;
		}
	}

	dac_div_ratio = p_afediv_lut[idx];

	if (fc > 3000) {
		dac_div_ratio <<= 1;
	}

	numerator =
		(((uint64)(fi << bits_in_mu) * dac_div_ratio) + (fc >> 1));
	numerator_hi = (uint32)(numerator >> 32);
	numerator_lo = (uint32)(numerator & 0xFFFFFFFF);
	math_uint64_divide(&dac_resamp_fcw, numerator_hi, numerator_lo, (uint32)fc);

	mu_delta_l = (uint16)(dac_resamp_fcw & 0xFFFF);
	mu_delta_u = (uint16)((dac_resamp_fcw >> 16) & 0xFF);

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		ACPHYREG_BCAST(pi, TxResamplerMuDelta0l, mu_delta_l);
		ACPHYREG_BCAST(pi, TxResamplerMuDelta0u, mu_delta_u);
		ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0l, mu_delta_l);
		ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0u, mu_delta_u);
		/* Enable the Tx resampler on all cores */
		regval = READ_PHYREG(pi, TxResamplerEnable0);
		regval |= (1 < ACPHY_TxResamplerEnable0_enable_tx_SHIFT(pi->pubpi->phy_rev));
		ACPHYREG_BCAST(pi, TxResamplerEnable0,  regval);
	} else {
		WRITE_PHYREG(pi, TxResamplerMuDelta0l, mu_delta_l);
		WRITE_PHYREG(pi, TxResamplerMuDelta0u, mu_delta_u);
		WRITE_PHYREG(pi, TxResamplerMuDeltaInit0l, mu_delta_l);
		WRITE_PHYREG(pi, TxResamplerMuDeltaInit0u, mu_delta_u);
		MOD_PHYREG(pi, TxResamplerEnable0, enable_tx, 1);
	}
}

/* proc rx_farrow_setup_28nm_ulp */
void
wlc_phy_rx_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_adc_mode)
{
	uint16 fc, afe_div_ratio, fcw_hi, fcw_lo;
	uint16 drift_period, bw;
	uint32 fcw;
	uint32 numerator_hi, numerator_lo;
	uint64 numerator;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	bw = CHSPEC_BW_LE20(pi->radio_chanspec) ? PHYBW_20: CHSPEC_IS40(pi->radio_chanspec)
			? PHYBW_40 : PHYBW_80;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) > 14)
		fc = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	else
		fc = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));

	if (fc < 3000) {
		if (ulp_adc_mode) {
			afe_div_ratio = 48;
		} else {
			afe_div_ratio = 56;
		}
	} else {
		if (ulp_adc_mode) {
			afe_div_ratio = 96;
			if (fc >= 5180) {
				afe_div_ratio = 104;
			}
		} else {
			afe_div_ratio = 112;
			if (fc >= 5180) {
				afe_div_ratio = 128;
			}
		}
	}

	drift_period = afe_div_ratio * bw * 2;
	numerator = (uint64)(fc) << 24;
	numerator_hi = (uint32)(numerator >> 32);
	numerator_lo = (uint32)(numerator & 0xFFFFFFFF);
	math_uint64_divide(&fcw, numerator_hi, numerator_lo, (uint32)drift_period);

	fcw -= (1 << 24);
	fcw_lo = (uint16)(fcw & 0xFFFF);
	fcw_hi = (uint16)((fcw >> 16) & 0xFF);

	WRITE_PHYREG(pi, rxFarrowDeltaPhase_lo, fcw_lo);
	WRITE_PHYREG(pi, rxFarrowDeltaPhase_hi, fcw_hi);
	WRITE_PHYREG(pi, rxFarrowDriftPeriod, drift_period);
	WRITE_PHYREG(pi, rxFarrowCtrl, 0xbb);

	WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, fcw_lo);
	WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, fcw_hi);
	WRITE_PHYREG(pi, lbFarrowDriftPeriod, drift_period);
	WRITE_PHYREG(pi, lbFarrowCtrl, 0xbb);
}

void
wlc_phy_farrow_setup_28nm_ulp(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_mode)
{
	wlc_phy_rx_farrow_setup_28nm_ulp(pi, ulp_adc_mode);
	wlc_phy_tx_farrow_setup_28nm_ulp(pi, ulp_tx_mode);

	/* Enabling the stall since stalls are disabled by default in 28nm Chips */
	ACPHY_ENABLE_STALL(pi, 0);
}

void
wlc_phy_farrow_setup_20694(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_mode)
{
	BCM_REFERENCE(pi);
	BCM_REFERENCE(ulp_tx_mode);
	BCM_REFERENCE(ulp_adc_mode);
	/* TODO: Add 20694 specific code to be added */
}

void
wlc_phy_farrow_setup_20697(phy_info_t *pi, uint16 ulp_tx_mode,
	uint16 ulp_adc_mode)
{
	BCM_REFERENCE(pi);
	BCM_REFERENCE(ulp_tx_mode);
	BCM_REFERENCE(ulp_adc_mode);
	/* TODO:4368: Check if required and Add 20694 specific code to be added */
}

void
wlc_phy_enable_lna_dcc_comp_20691(phy_info_t *pi, bool on)
{
	uint16 sparereg = READ_PHYREG(pi, SpareReg);

	if (on)
		sparereg &= 0xfffe;
	else
		sparereg |= 0x0001;

	WRITE_PHYREG(pi, SpareReg, sparereg);
}

void
wlc_phy_set_lowpwr_phy_reg_rev3(phy_info_t *pi)
{
	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_en_alc, 0x0)
		MOD_PHYREG_ENTRY(pi, radio_rxrf_lna5g, lna5g_lna1_bias_idac, 0x8)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_tempco_dcadj_1p2, 0x9)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_vctrl_buf_ical, 0x3)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_ib_bias_opamp, 0x6)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_ib_bias_opamp_fastsettle, 0xf)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_bypass_vctrl_buf, 0x0)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_HDRM_CAL, 0x2)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_ICAL, 0x16)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_ICAL_1p2, 0xc)
		MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_USE_2p5V, 0x1)
	ACPHY_REG_LIST_EXECUTE(pi);
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && ACMINORREV_1(pi)) {
		MOD_PHYREG(pi, radio_logen2gN5g, idac_mix, 0x4);
	}
}

void
wlc_phy_set_lowpwr_phy_reg(phy_info_t *pi)
{
	/* These guys not required for tiny based phys */
	if (!TINY_RADIO(pi)) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, radio_logen2g, idac_gm, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen2g, idac_gm_2nd, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen2g, idac_qb, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen2g, idac_qb_2nd, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen2g, idac_qtx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen2gN5g, idac_itx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen2gN5g, idac_qrx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen2gN5g, idac_irx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen2gN5g, idac_buf, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen2gN5g, idac_mix, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5g, idac_div, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5g, idac_vcob, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gbufs, idac_bufb, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5g, idac_mixb, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5g, idac_load, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gbufs, idac_buf2, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gbufs, idac_bufb2, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gbufs, idac_buf1, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gbufs, idac_bufb1, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_logen5gQI, idac_qtx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen5gQI, idac_itx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen5gQI, idac_qrx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_logen5gQI, idac_irx, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcocal, vcocal_rstn, 0x1)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcocal, vcocal_force_caps, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcocal, vcocal_force_caps_val, 0x40)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_ALC_ref_ctrl, 0xd)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_bias_mode, 0x1)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_cvar_extra, 0xb)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_cvar, 0xf)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_en_alc, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_tempco_dcadj, 0xe)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_tempco, 0xb)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_cal_en, 0x1)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_cal_en_empco, 0x1)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_cap_mode, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_ib_ctrl, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_por, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_r1, lf_r1, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_r2r3, lf_r2, 0xc)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_r2r3, lf_r3, 0xc)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cm, lf_rs_cm, 0xff)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cm, lf_rf_cm, 0xc)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cSet1, lf_c1, 0x99)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cSet1, lf_c2, 0x8b)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cSet2, lf_c3, 0x8b)
			MOD_PHYREG_ENTRY(pi, radio_pll_lf_cSet2, lf_c4, 0x8f)
			MOD_PHYREG_ENTRY(pi, radio_pll_cp, cp_kpd_scale, 0x34)
			MOD_PHYREG_ENTRY(pi, radio_pll_cp, cp_ioff, 0x60)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_1p2_xtalldo1p2_lowquiescenten, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_2p5_lowpwren_VCO, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_2p5_lowquiescenten_VCO_aux, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_2p5_lowpwren_VCO_aux, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_2p5_lowquiescenten_CP, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_ldo, ldo_2p5_lowquiescenten_VCO, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna2g, lna2g_lna1_bias_idac, 0x2)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna2g, lna2g_lna2_aux_bias_idac, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna2g, lna2g_lna2_main_bias_idac, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna5g, lna5g_lna1_bias_idac, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna5g, lna5g_lna2_aux_bias_idac, 0x7)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_lna5g, lna5g_lna2_main_bias_idac, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_rxmix, rxmix2g_aux_bias_idac, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_rxmix, rxmix2g_main_bias_idac, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_rxmix, rxmix5g_gm_aux_bias_idac_i, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxrf_rxmix, rxmix5g_gm_main_bias_idac_i, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_tia, tia_DC_Ib1, 0x6)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_tia, tia_DC_Ib2, 0x6)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_tia, tia_Ib_I, 0x6)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_tia, tia_Ib_Q, 0x6)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_bias12, lpf_bias_level1, 0x4)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_bias12, lpf_bias_level2, 0x8)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_bias34, lpf_bias_level3, 0x10)
			MOD_PHYREG_ENTRY(pi, radio_rxbb_bias34, lpf_bias_level4, 0x20)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_tempco_dcadj_1p2, 0x9)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_vctrl_buf_ical, 0x3)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_ib_bias_opamp, 0x6)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet4, vco_ib_bias_opamp_fastsettle, 0xf)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_bypass_vctrl_buf, 0x0)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_HDRM_CAL, 0x2)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet2, vco_ICAL, 0x16)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet3, vco_ICAL_1p2, 0xc)
			MOD_PHYREG_ENTRY(pi, radio_pll_vcoSet1, vco_USE_2p5V, 0x1)
		ACPHY_REG_LIST_EXECUTE(pi);
	}
}

/** Tx implicit beamforming. Ingress and outgress channels are assumed to have reprocity. */
void
wlc_phy_populate_recipcoeffs_acphy(phy_info_t *pi)
{
	uint16 start_words[][3] = {
		{0x005B, 0x0000, 0x0000},
		{0x8250, 0x0000, 0x0000},
		{0xC338, 0x0000, 0x0000},
		{0x4527, 0x0001, 0x0000},
		{0xA6A1, 0x0001, 0x0000},
		{0x081B, 0x0002, 0x0000},
		{0x8A18, 0x0002, 0x0000},
		{0x2C96, 0x0003, 0x0000},
		{0x8E17, 0x0003, 0x0000},
		{0x101B, 0x0004, 0x0000},
		{0x0020, 0x0000, 0x0000},
		{0x0020, 0x0000, 0x0000}
	};

	uint16 packed_word[6] = {0, 0, 0, 0, 0, 0};
	uint16 zero_word[3] = {0, 0, 0};

	uint16 ang_tmp = 0, ang_tmp1 = 0;
	uint16 subband_idx, k, i;
	uint16 theta[2];
	uint32 packed;
	uint16 nwords_start = 12, nwords_pad = 4, nwords_recip;
	uint8  stall_val;
	uint8 bands[NUM_CHANS_IN_CHAN_BONDING];

	bool is_caled = wlc_phy_is_txbfcal((wlc_phy_t *)pi);

	if (phy_stf_get_data(pi->stfi)->hw_phytxchain <= 1 || !(is_caled)) {
		return;
	}

	/* 1. obtain angles from SROM */
	if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_ac_chanmgr_get_chan_freq_range_80p80(pi, 0, bands);
		subband_idx = bands[0];
	} else {
		subband_idx = phy_ac_chanmgr_get_chan_freq_range(pi, 0, PRIMARY_FREQ_SEGMENT);
	}

	switch (subband_idx) {
	case WL_CHAN_FREQ_RANGE_2G:
#ifdef WLTXBF_2G_DISABLED
		ang_tmp = 0; ang_tmp1 = 0;
#else
		ang_tmp = pi->u.pi_acphy->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal2gcore3;
		}
#endif /* WLTXBF_2G_DISABLED */
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
		ang_tmp = pi->u.pi_acphy->sromi->rpcal5gb0;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal5gb0core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
		ang_tmp = pi->u.pi_acphy->sromi->rpcal5gb1;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal5gb1core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
		ang_tmp = pi->u.pi_acphy->sromi->rpcal5gb2;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal5gb2core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		ang_tmp = pi->u.pi_acphy->sromi->rpcal5gb3;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal5gb3core3;
		}
		break;
	default:
		ang_tmp = pi->u.pi_acphy->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			ang_tmp1 = pi->sromi->rpcal2gcore3;
		}
		break;
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		ang_tmp1 = ((ang_tmp >> 8) & 0xff) + ((ang_tmp1 & 0x00ff) << 8);
		ang_tmp  = (ang_tmp & 0xff) << 8;
	}

	/* 2. generate packed word */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {

		math_cmplx_angle_to_phasor_lut(ang_tmp, packed_word);

		/* printf("reciprocity packed_word: %x%x%x\n",
		packed_word[2], packed_word[1], packed_word[0]);
		*/

	} else if ((ACMAJORREV_2(pi->pubpi->phy_rev)) || (ACMAJORREV_4(pi->pubpi->phy_rev)) ||
		(ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev))) {
		theta[0] = (uint8) (ang_tmp & 0xFF);
		theta[1] = (uint8) ((ang_tmp >> 8) & 0xFF);
		/* printf("---- theta1 = %d, theta2 = %d\n", theta[0], theta[1]); */

		/* every 4 tones are packed into 1 word */
		packed = (theta[0] | (theta[0] << 8) | (theta[0] << 16) | (theta[0] << 24));

		/* printf("reciprocity packedWideWord: %x\n", packed); */
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	           ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) ||
	           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (!PHY_AS_80P80(pi, pi->radio_chanspec)) {
			math_cmplx_angle_to_phasor_lut(ang_tmp, &(packed_word[0]));
			math_cmplx_angle_to_phasor_lut(ang_tmp1, &(packed_word[3]));
		}
	}

	/* Disable stalls and hold FIFOs in reset */
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	/* 3. write to table */
	/* 4360 and 43602 */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		nwords_recip = 64 + 128 + 256;

		for (k = 0; k < nwords_start; k++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFECONFIG,
			1, k, 48, start_words[k]);
		}

		for (k = 0; k < nwords_recip; k++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFECONFIG,
			1, nwords_start + k, 48, packed_word);
		}

		for (k = 0; k < nwords_pad; k++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFECONFIG,
			1, nwords_start + nwords_recip + k, 48, zero_word);
		}
	} else if ((ACMAJORREV_2(pi->pubpi->phy_rev)) || (ACMAJORREV_4(pi->pubpi->phy_rev)) ||
		(ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev))) {
		/* 4 tones are packed into one word */
		nwords_recip = (256 >> 2);

		for (k = 0; k < nwords_recip; k++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFECONFIG2X2TBL,
			1, k, 32, &packed);
		}
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	           ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) ||
	           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* total 512 or 1024 words, and partitioned into 4 mem banks */
		nwords_recip = ACMAJORREV_47_51(pi->pubpi->phy_rev)? (1024 >> 2): (512 >> 2);

		for (k = 0; k < nwords_recip; k++) {
			if (k % 2 == 0) {
				recip_packed_majrev32_33_37[k][0] = packed_word[0];
				recip_packed_majrev32_33_37[k][1] = packed_word[1];
				recip_packed_majrev32_33_37[k][2] = packed_word[2];
			} else {
				recip_packed_majrev32_33_37[k][0] = packed_word[3];
				recip_packed_majrev32_33_37[k][1] = packed_word[4];
				recip_packed_majrev32_33_37[k][2] = packed_word[5];
			}
		}

		for (i = 0; i < 4; i++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFECONFIG,
			nwords_recip, i*nwords_recip, 48, recip_packed_majrev32_33_37);
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

/* get the two 80p80 complex freq. chanspec must be 80p80 or 160 */
void
phy_ac_chanmgr_get_chan_freq_range_80p80(phy_info_t *pi, chanspec_t chanspec, uint8 *freq)
{
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING];

	if (CHSPEC_CHANNEL(chanspec) == 0) {
		chanspec = pi->radio_chanspec;
	}

	ASSERT(PHY_AS_80P80(pi, chanspec) || CHSPEC_IS160(chanspec));

	wf_chspec_get_80p80_channels(chanspec, chans);
	PHY_TRACE(("wl%d: %s: chan1=%d chan2=%d\n", pi->sh->unit,
			__FUNCTION__, chans[0], chans[1]));

	freq[0] = phy_ac_chanmgr_chan2freq_range(pi, chanspec, chans[0]);
	freq[1] = phy_ac_chanmgr_chan2freq_range(pi, chanspec, chans[1]);

}

/* get the complex freq. if chan==0, use default radio channel */
uint8
phy_ac_chanmgr_get_chan_freq_range(phy_info_t *pi, chanspec_t chanspec, uint8 core_segment_mapping)
{
	uint8 channel = CHSPEC_CHANNEL(chanspec);

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

		if (channel == 0) {
			channel = CHSPEC_CHANNEL(pi->radio_chanspec);
		}

		if (PHY_AS_80P80(pi, chanspec)) {
			PHY_INFORM(("wl%d: FIXME %s\n", pi->sh->unit, __FUNCTION__));
			channel -= 8;
		}
	} else {
		if (phy_get_phymode(pi) != PHYMODE_80P80) {
			if (channel == 0)
				channel = CHSPEC_CHANNEL(pi->radio_chanspec);
		} else {
			if (channel == 0)
				chanspec = pi->radio_chanspec;

			if (CHSPEC_BW(chanspec) == WL_CHANSPEC_BW_8080) {
				if (PRIMARY_FREQ_SEGMENT == core_segment_mapping)
					channel = wf_chspec_primary80_channel(chanspec);

				if (SECONDARY_FREQ_SEGMENT == core_segment_mapping)
					channel = wf_chspec_secondary80_channel(chanspec);
			} else {
				channel = CHSPEC_CHANNEL(chanspec);
			}
		}
	}

	return phy_ac_chanmgr_chan2freq_range(pi, chanspec, channel);

}

/* Internal function to get frequency for given channel
 * if channel==0, use default channel
 */
static int
phy_ac_chanmgr_chan2freq(phy_info_t *pi, uint8 channel)
{
	int freq = 0;

	if (channel == 0) {
		channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	}

	if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
		const void *chan_info;
		freq = wlc_phy_chan2freq_acphy(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20691_ID)) {
		const chan_info_radio20691_t *chan_info;
		freq = wlc_phy_chan2freq_20691(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		const chan_info_radio20693_pll_t *chan_info_pll;
		const chan_info_radio20693_rffe_t *chan_info_rffe;
		const chan_info_radio20693_pll_wave2_t *chan_info_pll_wave2;
		freq = wlc_phy_chan2freq_20693(pi, channel, &chan_info_pll, &chan_info_rffe,
			&chan_info_pll_wave2);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20694_ID)) {
		const chan_info_radio20694_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20694(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20695_ID)) {
		const chan_info_radio20695_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20695(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20696_ID)) {
		const chan_info_radio20696_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20696(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20697_ID)) {
		chan_info_radio20697_rffe_t chan_info;
		freq = wlc_phy_chan2freq_20697(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20698_ID)) {
		const chan_info_radio20698_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20698(pi, channel, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20704_ID)) {
		const chan_info_radio20704_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20704(pi, channel, &chan_info);
	} else {
		ASSERT(0);
		freq = -1;
	}

	return freq;
}

/* Get the subband index (WL_CHAN_FREQ_RANGE_[25]G_*) for the given channel
 * according to the SROMREV<12 definitions, i.e. 4 subbands in 5G
 * if chan==0, use default radio channel
 */
uint8
phy_ac_chanmgr_chan2freq_range(phy_info_t *pi, chanspec_t chanspec, uint8 channel)
{
	int freq = phy_ac_chanmgr_chan2freq(pi, channel);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (channel <= CH_MAX_2G_CHANNEL || freq < 0) {
		return WL_CHAN_FREQ_RANGE_2G;
	} else if ((pi->sromi->subband5Gver == PHY_MAXNUM_5GSUBBANDS) ||
	           (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)) {
		if ((freq >= PHY_SUBBAND_4BAND_BAND0) &&
		    (freq < PHY_SUBBAND_4BAND_BAND1))
			return WL_CHAN_FREQ_RANGE_5G_BAND0;
		else if ((freq >= PHY_SUBBAND_4BAND_BAND1) &&
			(freq < PHY_SUBBAND_4BAND_BAND2))
			return WL_CHAN_FREQ_RANGE_5G_BAND1;
		else if ((freq >= PHY_SUBBAND_4BAND_BAND2) &&
			(freq < PHY_SUBBAND_4BAND_BAND3))
			return WL_CHAN_FREQ_RANGE_5G_BAND2;
		else
			return WL_CHAN_FREQ_RANGE_5G_BAND3;
	} else if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_EMBDDED) {
		if ((freq >= EMBEDDED_LOW_5G_CHAN) && (freq < EMBEDDED_MID_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GL;
		} else if ((freq >= EMBEDDED_MID_5G_CHAN) &&
		           (freq < EMBEDDED_HIGH_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GM;
		} else {
			return WL_CHAN_FREQ_RANGE_5GH;
		}
	} else if (pi->sromi->subband5Gver == PHY_SUBBAND_3BAND_HIGHPWR) {
		if ((freq >= HIGHPWR_LOW_5G_CHAN) && (freq < HIGHPWR_MID_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GL;
		} else if ((freq >= HIGHPWR_MID_5G_CHAN) && (freq < HIGHPWR_HIGH_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GM;
		} else {
			return WL_CHAN_FREQ_RANGE_5GH;
		}
	} else { /* Default PPR Subband subband5Gver = 7 */
		if ((freq >= JAPAN_LOW_5G_CHAN) && (freq < JAPAN_MID_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GL;
		} else if ((freq >= JAPAN_MID_5G_CHAN) && (freq < JAPAN_HIGH_5G_CHAN)) {
			return WL_CHAN_FREQ_RANGE_5GM;
		} else {
			return WL_CHAN_FREQ_RANGE_5GH;
		}
	}
}

/* get the complex freq for 80p80 channels. if chan==0, use default radio channel */
void
phy_ac_chanmgr_get_chan_freq_range_80p80_srom12(phy_info_t *pi, chanspec_t chanspec, uint8 *freq)
{
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING];

	ASSERT(SROMREV(pi->sh->sromrev) >= 12);

	if (CHSPEC_CHANNEL(chanspec) == 0) {
		chanspec = pi->radio_chanspec;
	}

	ASSERT(PHY_AS_80P80(pi, chanspec) || CHSPEC_IS160(chanspec));

	wf_chspec_get_80p80_channels(chanspec, chans);
	PHY_TRACE(("wl%d: %s: chan1=%d chan2=%d\n", pi->sh->unit,
			__FUNCTION__, chans[0], chans[1]));

	freq[0] = phy_ac_chanmgr_chan2freq_range_srom12(pi, chanspec, chans[0]);
	freq[1] = phy_ac_chanmgr_chan2freq_range_srom12(pi, chanspec, chans[1]);
}

/* get the complex freq. if chan==0, use default radio channel */
uint8
phy_ac_chanmgr_get_chan_freq_range_srom12(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 channel = CHSPEC_CHANNEL(chanspec);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(SROMREV(pi->sh->sromrev) >= 12);

	if (channel == 0) {
		channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	}

	return phy_ac_chanmgr_chan2freq_range_srom12(pi, chanspec, channel);
}

/* Get the subband index (WL_CHAN_FREQ_RANGE_[25]G_*) for the given channel
 * according to the SROMREV>=12 definitions, i.e. subband5gver=5 or 5 band in 5GHz
 * if chan==0, use default radio channel
 */
uint8
phy_ac_chanmgr_chan2freq_range_srom12(phy_info_t *pi, chanspec_t chanspec, uint8 channel)
{
	int freq = phy_ac_chanmgr_chan2freq(pi, channel);

	ASSERT(SROMREV(pi->sh->sromrev) >= 12);

	if (CHSPEC_IS5G(chanspec)) {
		ASSERT(pi->sromi->subband5Gver == PHY_MAXNUM_5GSUBBANDS);
	}

	if (channel <= CH_MAX_2G_CHANNEL || freq < PHY_MAXNUM_5GSUBBANDS_BAND0) {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_2G_40;
		else
			return WL_CHAN_FREQ_RANGE_2G;
	} else if ((freq >= PHY_MAXNUM_5GSUBBANDS_BAND0) &&
	           (freq < PHY_MAXNUM_5GSUBBANDS_BAND1)) {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND0_40;
		else if (CHSPEC_IS80(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND0_80;
		else if (CHSPEC_IS8080(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND0_80;
		else if (CHSPEC_IS160(chanspec) && PHY_AS_80P80(pi, chanspec)) {
			return WL_CHAN_FREQ_RANGE_5G_BAND0_80;
		} else
			return WL_CHAN_FREQ_RANGE_5G_BAND0;
	} else if ((freq >= PHY_MAXNUM_5GSUBBANDS_BAND1) &&
	           (freq < PHY_MAXNUM_5GSUBBANDS_BAND2)) {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND1_40;
		else if (CHSPEC_IS80(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND1_80;
		else if (CHSPEC_IS8080(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND1_80;
		else if (CHSPEC_IS160(chanspec)) {
			if (PHY_AS_80P80(pi, chanspec)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND1_80;
			} else {
				if (freq == PHY_MAXNUM_5GSUBBANDS_BAND1) {
					return WL_CHAN_FREQ_RANGE_5G_BAND0_160;
				} else {
					return WL_CHAN_FREQ_RANGE_5G_BAND1_160;
				}
			}
		} else
			return WL_CHAN_FREQ_RANGE_5G_BAND1;
	} else if ((freq >= PHY_MAXNUM_5GSUBBANDS_BAND2) &&
	           (freq < PHY_MAXNUM_5GSUBBANDS_BAND3)) {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND2_40;
		else if (CHSPEC_IS80(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND2_80;
		else if (CHSPEC_IS8080(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND2_80;
		else if (CHSPEC_IS160(chanspec)) {
			if (PHY_AS_80P80(pi, chanspec)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND2_80;
			} else {
				return WL_CHAN_FREQ_RANGE_5G_BAND2_160;
			}
		} else
			return WL_CHAN_FREQ_RANGE_5G_BAND2;
	} else if ((freq >= PHY_MAXNUM_5GSUBBANDS_BAND3) &&
	           (freq < PHY_MAXNUM_5GSUBBANDS_BAND4)) {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND3_40;
		else if (CHSPEC_IS80(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND3_80;
		else if (CHSPEC_IS8080(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND3_80;
		else if (PHY_AS_80P80(pi, chanspec)) {
			//no 160 channel in band3
			return WL_CHAN_FREQ_RANGE_5G_BAND3;
		} else
			return WL_CHAN_FREQ_RANGE_5G_BAND3;
	} else {
		if (CHSPEC_IS40(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND4_40;
		else if (CHSPEC_IS80(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND4_80;
		else if (CHSPEC_IS8080(chanspec))
			return WL_CHAN_FREQ_RANGE_5G_BAND4_80;
		else if (CHSPEC_IS160(chanspec)) {
			if (PHY_AS_80P80(pi, chanspec)) {
				return WL_CHAN_FREQ_RANGE_5G_BAND4_80;
			} else {
				return WL_CHAN_FREQ_RANGE_5G_BAND4_160;
			}
		} else
			return WL_CHAN_FREQ_RANGE_5G_BAND4;
	}
}

static bool
phy_ac_chanmgr_is_txbfcal(phy_type_chanmgr_ctx_t *ctx)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8  subband_idx;
	uint8  chans[NUM_CHANS_IN_CHAN_BONDING];
	uint16 rpcal_val, rpcal_val1 = 0;
	bool   is_caled;

#ifdef MACOSX
	if (ACMAJORREV_0(pi->pubpi->phy_rev)|| ACMAJORREV_5(pi->pubpi->phy_rev))
		return FALSE;
#endif /* MACOSX */

	if (ACMAJORREV_33(pi->pubpi->phy_rev) && PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_ac_chanmgr_get_chan_freq_range_80p80(pi, pi->radio_chanspec, chans);
		subband_idx = chans[0];
		PHY_INFORM(("wl%d: %s: FIXME for 80P80\n", pi->sh->unit, __FUNCTION__));
	} else {
		subband_idx = phy_ac_chanmgr_get_chan_freq_range(pi, 0, PRIMARY_FREQ_SEGMENT);
	}

	switch (subband_idx) {
	case WL_CHAN_FREQ_RANGE_2G:
		rpcal_val = pi->u.pi_acphy->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal2gcore3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND0:
		rpcal_val = pi->u.pi_acphy->sromi->rpcal5gb0;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb0core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND1:
		rpcal_val = pi->u.pi_acphy->sromi->rpcal5gb1;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb1core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND2:
		rpcal_val = pi->u.pi_acphy->sromi->rpcal5gb2;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb2core3;
		}
		break;
	case WL_CHAN_FREQ_RANGE_5G_BAND3:
		rpcal_val = pi->u.pi_acphy->sromi->rpcal5gb3;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal5gb3core3;
		}
		break;
	default:
		PHY_ERROR(("wl%d: %s: Invalid chan_freq_range %d\n",
		           pi->sh->unit, __FUNCTION__, subband_idx));
		rpcal_val = pi->u.pi_acphy->sromi->rpcal2g;
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			rpcal_val1 = pi->sromi->rpcal2gcore3;
		}
		break;
	}

	is_caled = (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_37(pi->pubpi->phy_rev) ||
			ACMAJORREV_47_51(pi->pubpi->phy_rev)) ?
	        !(rpcal_val == 0 && rpcal_val1 == 0) : (rpcal_val != 0);

	return is_caled;
}

void
wlc_phy_smth(phy_info_t *pi, int8 enable_smth, int8 dump_mode)
{
#ifdef WL_PROXDETECT
	if (phy_ac_tof_forced_smth(pi->u.pi_acphy->tofi))
		return;
#endif // endif

	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
	    ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_GE36(pi->pubpi->phy_rev)) {
		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
		uint16 SmthReg0, SmthReg1;

		ACPHY_REG_LIST_START
			/* Set the SigB to the default values */
			MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs9, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs8, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs7, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs6, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs5, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs4, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs3, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs2, 0x7)
			MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs1, 0x3)
			MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs0, 0x2)
		ACPHY_REG_LIST_EXECUTE(pi);

		pi_ac->chanmgri->acphy_smth_dump_mode = SMTH_NODUMP;

		switch (enable_smth) {
		case SMTH_DISABLE:
			/* Disable Smoothing and Enable SigB */
			SmthReg0 = READ_PHYREG(pi, chnsmCtrl0) & 0xFFFE;
			SmthReg1 = READ_PHYREG(pi, chnsmCtrl1);
			break;
		case SMTH_ENABLE:
			/* Enable Smoothing With all modes ON */
			/* This is the default config in which Smth is enabled for */
			/* SISO pkts and HT TxBF case. Use SigB for VHT-TxBF */
			SmthReg0 = 0x33F;
			SmthReg1 = 0x2C0;
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				SmthReg0 |=
					ACPHY_chnsmCtrl0_mte_pilot_enable_MASK(pi->pubpi->phy_rev);
			}
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x37f;
			  SmthReg1 = 0x5eC0;
			}
			pi_ac->chanmgri->acphy_smth_dump_mode = dump_mode;
			switch (dump_mode)
			{
			case SMTH_FREQDUMP:
			/* Enable Freq-domain dumping (Raw Channel Estimates) */
			SmthReg0 &= ~(
				ACPHY_chnsmCtrl0_nw_whiten_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_group_delay_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_mte_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_window_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev));
			SmthReg1 &= ~(
				ACPHY_chnsmCtrl1_ifft_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev));
			break;

			case SMTH_FREQDUMP_AFTER_NW:
			/* Enable Freq-domain dumping (After NW Filtering) */
			SmthReg0 &= ~(
				ACPHY_chnsmCtrl0_group_delay_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_mte_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_window_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev));
			SmthReg1 &= ~(
				ACPHY_chnsmCtrl1_ifft_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev));
			break;

			case SMTH_FREQDUMP_AFTER_GD:
			/* Enable Freq-domain dumping (After GD Compensation) */
			SmthReg0 &= ~(
				ACPHY_chnsmCtrl0_mte_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_window_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev));
			SmthReg1 &= ~(
				ACPHY_chnsmCtrl1_ifft_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev));
			break;

			case SMTH_FREQDUMP_AFTER_MTE:
			/* Enable Freq-domain dumping (After MTE) */
			SmthReg0 &= ~(
				ACPHY_chnsmCtrl0_window_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev));
			SmthReg1 &= ~(
				ACPHY_chnsmCtrl1_ifft_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev));
			break;

			case SMTH_TIMEDUMP_AFTER_IFFT:
			/* Enable Time-domain dumping (After IFFT) */
			SmthReg0 &= ~(
				ACPHY_chnsmCtrl0_window_enable_MASK(pi->pubpi->phy_rev) |
				ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev));
			SmthReg1 &= ~ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev);
			break;

			case SMTH_TIMEDUMP_AFTER_WIN:
				/* Enable Time-domain dumping (After Windowing) */
			SmthReg0 &= ~ACPHY_chnsmCtrl0_fft_enable_MASK(pi->pubpi->phy_rev);
			SmthReg1 &= ~ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev);
			break;

			case SMTH_FREQDUMP_AFTER_FFT:
			/* Enable Freq-domain dumping (After FFT) */
			SmthReg1 &= ~ACPHY_chnsmCtrl1_output_enable_MASK(pi->pubpi->phy_rev);
			break;
			}
			break;
		case SMTH_ENABLE_NO_NW:
			/* Enable Smoothing With all modes ON Except NW Filter */
			SmthReg0 = 0x337;
			SmthReg1 = 0x2C0;
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x377;
			  SmthReg1 = 0x5eC0;
			}
			break;
		case SMTH_ENABLE_NO_NW_GD:
			/* Enable Smoothing With all modes ON Except NW and GD  */
			SmthReg0 = 0x327;
			SmthReg1 = 0x2C0;
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x367;
			  SmthReg1 = 0x5eC0;
			}
			break;
		case SMTH_ENABLE_NO_NW_GD_MTE:
			/* Enable Smoothing With all modes ON Except NW, GD and  MTE */
			SmthReg0 = 0x307;
			SmthReg1 = 0x2C0;
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x307;
			  SmthReg1 = 0x5eC0;
			}
			break;
		case DISABLE_SIGB_AND_SMTH:
			/* Disable Smoothing and SigB */
			SmthReg0 = 0x33E;
			SmthReg1 = 0x0C0;
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x37E;
			  SmthReg1 = 0x0C0;
			}
			ACPHY_REG_LIST_START
				MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs9, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb2, mu_sigbmcs8, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs7, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs6, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs5, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb1, mu_sigbmcs4, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs3, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs2, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs1, 0x0)
				MOD_PHYREG_ENTRY(pi, musigb0, mu_sigbmcs0, 0x0)
			ACPHY_REG_LIST_EXECUTE(pi);
			break;
		case SMTH_FOR_TXBF:
			/* Enable Smoothing for TxBF using Smth for HT and VHT */
			SmthReg0 = 0x33F;
			SmthReg1 = 0x6C0;
			if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
			    ACMAJORREV_37(pi->pubpi->phy_rev) ||
			    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			  SmthReg0 = 0x37F;
			  SmthReg1 = 0x5EC0;
			}
			break;
		default:
			PHY_ERROR(("wl%d: %s: Unrecognized smoothing mode: %d\n",
			          pi->sh->unit, __FUNCTION__, enable_smth));
			return;
		}
		WRITE_PHYREG(pi, chnsmCtrl0, SmthReg0);
		WRITE_PHYREG(pi, chnsmCtrl1, SmthReg1);
		pi_ac->chanmgri->acphy_enable_smth = enable_smth;

		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			/* 4349 specific setting */
			if (enable_smth == SMTH_ENABLE) {
				/* output_enable_new = 0x0 no output
				 * output_enable_new = 0x1 only legacy channel is smoothed
				 * output_enable_new = 0x2 only HT/VHT channel is smoothed
				 * output_enable_new = 0x3 both legacy and HT/VHT are smoothed
				 */
				/* 0x2 since TXBF doesn't work if legacy smoothing is enabled */
				MOD_PHYREG(pi, chnsmCtrl1, output_enable_new, 0x2);
			} else {
			    MOD_PHYREG(pi, chnsmCtrl1, output_enable_new, 0x0);
			}
			if ((phy_get_phymode(pi) == PHYMODE_MIMO) &&
					(phy_stf_get_data(pi->stfi)->phyrxchain == 0x3)) {
				MOD_PHYREG(pi, chnsmCtrl1, disable_2rx_nvar_calc, 0x0);
			} else {
				MOD_PHYREG(pi, chnsmCtrl1, disable_2rx_nvar_calc, 0x1);
			}
			MOD_PHYREG(pi, nvcfg3, unity_gain_for_2x2_coremask2, 0x1);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev) ||
			ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, chnsmCtrl0, mte_pilot_enable, 0x1);
			MOD_PHYREG(pi, chnsmCtrl1, ignore_VHT_txbf_bit, 0x1);
			if (enable_smth == SMTH_ENABLE) {
				/* output_enable_new = 0x0 no output
				 * output_enable_new = 0x1 only legacy channel is smoothed
				 * output_enable_new = 0x2 only HT/VHT channel is smoothed
				 * output_enable_new = 0x3 both legacy and HT/VHT are smoothed
				 */
				MOD_PHYREG(pi, chnsmCtrl1, output_enable_new, 0x3);
			} else {
				MOD_PHYREG(pi, chnsmCtrl1, output_enable_new, 0x0);
			}
		}

		/* set the Tiny specific filter slopes for channel smoothing */
		if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
			ACPHY_REG_LIST_START
				MOD_PHYREG_ENTRY(pi, chnsmCtrl5, filter_slope_20MHz, 0x2)
				MOD_PHYREG_ENTRY(pi, chnsmCtrl6, filter_slope_40MHz, 0x2)
				MOD_PHYREG_ENTRY(pi, chnsmCtrl6, filter_slope_80MHz, 0x1)
			ACPHY_REG_LIST_EXECUTE(pi);
		}

		if (ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, nvcfg3, unity_gain_for_2x2_coremask2, 0x1);
		}
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			MOD_PHYREG(pi, chnsmCtrl0, chan_smooth_enable, 0);
			MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 0);
		} else {
			MOD_PHYREG(pi, chnsmCtrl0, chan_smooth_enable, 1);
			MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 0);
		}
	}
}

static bool
phy_ac_chanmgr_is_smth_en(phy_type_chanmgr_ctx_t *ctx)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	bool is_smth_en;

	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
	    ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_GE36(pi->pubpi->phy_rev)) {
		is_smth_en = (pi->u.pi_acphy->chanmgri->acphy_enable_smth == 1) ? TRUE : FALSE;
		return is_smth_en;
	} else {
		return FALSE;
	}
}

void
wlc_phy_ac_shared_ant_femctrl_master(phy_info_t *pi)
{
	bool band5g = CHSPEC_IS5G(pi->radio_chanspec);
	if (CHIPID(pi->sh->chip) == BCM4364_CHIP_ID) {
		  #define SLICE_3x3_MASTER_2G 0xf8
		  #define SLICE_3x3_MASTER_5G 0x307
		  if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		    /* make 3x3 the master */
		    if (band5g) {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					(0x7 << 29), ((SLICE_3x3_MASTER_5G & 0x7) << 29));
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					(0x7f << 0), (((SLICE_3x3_MASTER_5G >>3) & 0x7f) << 0));
		    } else {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					(0x7 << 29), ((SLICE_3x3_MASTER_2G & 0x7) << 29));
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					(0x7f << 0), (((SLICE_3x3_MASTER_2G >>3) & 0x7f) << 0));
		    }
		  } else if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
		    /* make 1x1 the master */
		    if (band5g) {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					(0x7 << 29), ((SLICE_3x3_MASTER_2G & 0x7) << 29));
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					(0x7f << 0), (((SLICE_3x3_MASTER_2G >>3) & 0x7f) << 0));
		    } else {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					(0x7 << 29), ((SLICE_3x3_MASTER_5G & 0x7) << 29));
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					(0x7f << 0), (((SLICE_3x3_MASTER_5G >>3) & 0x7f) << 0));
		    }
		  }
		if (phy_get_master(pi) == 0) {
			/* make 3x3 the master */
			si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
				(0x7 << 29), (0x7 << 29));
			si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
				(0x7f << 0), (0x7f << 0));
		} else if (phy_get_master(pi) == 1) {
			si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
				(0x7 << 29), (0 << 29));
			si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
				(0x7f << 0), (0 << 0));
		}
	} else {
		PHY_ERROR(("%s: Multi Slice femctrl not supported\n", __FUNCTION__));
	}
}

#if (defined(WL_SISOCHIP) || !defined(SWCTRL_TO_BT_IN_COEX))
static void
wlc_phy_ac_femctrl_mask_on_band_change(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	if (!ACMAJORREV_0(pi->pubpi->phy_rev)) {
		/* When WLAN is in 5G, WLAN table should control the FEM lines */
		/* and BT should not have any access permissions */
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			/* disable BT Fem control table accesses */
			MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x0);
			if (!ACPHY_FEMCTRL_ACTIVE(pi)) {
				if (ACMAJORREV_4(pi->pubpi->phy_rev) &&
					!BF3_RSDB_1x1_BOARD(pi_ac))  {
					if (phy_get_phymode(pi) == PHYMODE_MIMO) {
						/* writes to both cores */
						MOD_PHYREG(pi, FemCtrl, femCtrlMask, 0x3ff);
						/* now write to only core0 */
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 1);
						MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							pi_ac->sromi->femctrlmask_5g);
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 0);
					} else if (phy_get_phymode(pi) == PHYMODE_RSDB &&
						phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1) {
						MOD_PHYREG(pi, FemCtrl, femCtrlMask, 0x3ff);
					} else {
						MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							pi_ac->sromi->femctrlmask_5g);
					}
				} else {
					MOD_PHYREG(pi, FemCtrl, femCtrlMask,
						pi_ac->sromi->femctrlmask_5g);
				}
			} else {
				if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
					if (BFCTL(pi_ac) == 4) {
						if (BF3_FEMCTRL_SUB(pi_ac) == 1) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x23c);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 2) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x297);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 3) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x058);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 4) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x058);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 6) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0xe);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 7) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x2e);
						} else {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x3ff);
						}
					}
				} else if (ACMAJORREV_2(pi->pubpi->phy_rev) ||
				           ACMAJORREV_5(pi->pubpi->phy_rev)) {
					if (BFCTL(pi_ac) == 10) {
						if (BF3_FEMCTRL_SUB(pi_ac) == 0) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x317);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 1) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x347);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 2) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x303);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 3) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x307);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 4) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x309);
						} else if (BF3_FEMCTRL_SUB(pi_ac) == 5) {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x3c7);
						} else {
							MOD_PHYREG(pi, FemCtrl, femCtrlMask,
							           0x3ff);
						}
					} else if (pi->u.pi_acphy->sromi->femctrl == 2) {
					  if (pi->u.pi_acphy->sromi->femctrl_sub == 5)
						si_pmu_switch_off_PARLDO(pi->sh->sih, pi->sh->osh);
					  if (pi->u.pi_acphy->sromi->femctrl_sub == 6)
						si_pmu_switch_on_PARLDO(pi->sh->sih, pi->sh->osh);
					} else {
						MOD_PHYREG(pi, FemCtrl, femCtrlMask, 0x3ff);
					}
				} else if (TINY_RADIO(pi)) {
					MOD_PHYREG(pi, FemCtrl, femCtrlMask, 0x3ff);
				}
			}
			if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
				phy_utils_write_radioreg(pi, RF_2069_TXGM_LOFT_SCALE(0), 0x0);
				if (BF2_DAC_SPUR_IMPROVEMENT(pi_ac) == 1) {
					phy_utils_write_radioreg(pi, RFX_2069_ADC_CFG5, 0x83e3);
				}
			}
		} else { /* When WLAN is in 2G, BT controls should be allowed to go through */
			/* BT should also be able to control FEM Control Table */
			if ((!(CHIPID(pi->sh->chip) == BCM43602_CHIP_ID ||
				CHIPID(pi->sh->chip) == BCM43462_CHIP_ID)) &&
				BF_SROM11_BTCOEX(pi_ac)) {
				if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
					ACMAJORREV_33(pi->pubpi->phy_rev)) {
					if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
						BFL2_BT_SHARE_ANT0) {
						MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x1);
					} else {
						MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x0);
					}
				} else {
					MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x1);
				}
			}
			if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, FemCtrl, femCtrlMask, pi_ac->sromi->femctrlmask_2g);
			} else {
				MOD_PHYREG(pi, FemCtrl, femCtrlMask, 0x3ff);
			}

			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				uint8 DLNA_BTFLAG;
				DLNA_BTFLAG = (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) &
					0x00400000) >> 22;
				MOD_PHYREG(pi, FemCtrl, femCtrlMask,
					pi_ac->sromi->femctrlmask_2g);
				if (DLNA_BTFLAG == 0) {
					MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x0);
				} else {
					if (BF3_RSDB_1x1_BOARD(pi_ac)) {
						MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x1);
					} else {
						if (phy_get_phymode(pi) == PHYMODE_MIMO) {
						/* writes to both cores */
						MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x0);
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 1);
						/* writes to only core0 */
						MOD_PHYREG(pi, FemCtrl, enBtSignalsToFEMLut, 0x1);
						wlapi_exclusive_reg_access_core0(
							pi->sh->physhim, 0);
						} else if (phy_get_phymode(pi) == PHYMODE_RSDB)  {
							if (phy_get_current_core(pi) == 0) {
								MOD_PHYREG(pi, FemCtrl,
									enBtSignalsToFEMLut, 0x1);
							} else {
								MOD_PHYREG(pi, FemCtrl,
									enBtSignalsToFEMLut, 0x0);
							}
						}
					}
				}
			}

			if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
				phy_utils_write_radioreg(pi, RF_2069_TXGM_LOFT_SCALE(0), 0xa);
				if (BF2_DAC_SPUR_IMPROVEMENT(pi_ac) == 1) {
					phy_utils_write_radioreg(pi, RFX_2069_ADC_CFG5, 0x83e0);
				}
			}
			if (ACMAJORREV_5(pi->pubpi->phy_rev) &&
				pi->u.pi_acphy->sromi->femctrl == 2) {
			    if (pi->u.pi_acphy->sromi->femctrl_sub == 5)
			        si_pmu_switch_on_PARLDO(pi->sh->sih, pi->sh->osh);
			    if (pi->u.pi_acphy->sromi->femctrl_sub == 6)
			        si_pmu_switch_off_PARLDO(pi->sh->sih, pi->sh->osh);
			}
		}
	}
}
#endif /* (defined(WL_SISOCHIP) || !defined(SWCTRL_TO_BT_IN_COEX)) */

void
phy_ac_chanmgr_core2core_sync_setup(phy_ac_chanmgr_info_t *chanmgri, bool enable)
{
	phy_info_t *pi = chanmgri->pi;
	uint8 core;
	uint8 val =  (enable ? 1 : 0);
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

	//FIXME43684, suggest to turn core2core syncup on
	if (!((ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	      ACMAJORREV_33(pi->pubpi->phy_rev) || pi->pubpi->phy_rev >= 40)) {
		return;
	}

	if (ACMAJORREV_51(pi->pubpi->phy_rev)) {
		// 63178FIXME: core2core_sync not implemented yet
		return;
	}

	PHY_TRACE(("%s: enable = %d\n", __FUNCTION__, (uint8)enable));

	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	FOREACH_CORE(pi, core) {
		if (enable) {
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, afe_iqdac_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_clamp_en, 1);
			MOD_PHYREGCE(pi, RfctrlCoreAfeCfg2, core, afe_iqadc_flashhspd, 1);
		}
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqdac_pwrup, val);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_clamp_en, val);
		MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_flashhspd, val);
	}

	/* 43684 specific settings */
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* Additional radio settings to keep DAC clocks powered on (sequence matters!) */
		if (enable) {
			/* Set RfctrlIntc to bypass mode to fix spur before CAC */
			FOREACH_CORE(pi, core) {
				WRITE_PHYREGCE(pi, RfctrlIntc, core, 0x1c00);
			}
			/* Setup */
			FOREACH_CORE(pi, core) {
				/* First overwrite dac_div_pu's then force div_resets low
				 * to avoid glitch
				 */
				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core, afediv_dac_div_pu, 1);
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_dac_div_pu, 1);

				wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);
				OSL_DELAY(10);
				wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_TX2RX);

				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core,
				                    afediv_dac_div_reset, 0);
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_dac_div_reset, 1);

				/* ensure dac outbuf always on via override
				 * (inbuf always on via override by default)
				 */
				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core,
				                    afediv_outbuf_dac_pu, 1);
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_outbuf_dac_pu, 1);
			}
			/* Restore RfctrlIntc settings */
			FOREACH_CORE(pi, core) {
				WRITE_PHYREGCE(pi, RfctrlIntc, core, 0x0);
			}
		}
		/* Additional PHY setting to avoid farrow txunderflow for core2core sync ON */
		MOD_PHYREG(pi, fineclockgatecontrol, forcetxlbClkEn, val);
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		/* Sparereg setting needed for 4365B1 */
		uint16 sparereg;
		sparereg = READ_PHYREG(pi, SpareReg);
		if (enable) {
			MOD_PHYREG(pi, SpareReg, spareReg, sparereg | 0x0040);
		} else {
			MOD_PHYREG(pi, SpareReg, spareReg, sparereg & 0xffbf);
		}
	}

	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, TxResamplerEnable, core, txfe_baseband_enfree, val);
		MOD_PHYREGCE(pi, TxResamplerEnable, core, txfetoptxfrreseten, val);
		MOD_PHYREGCE(pi, TxResamplerEnable, core, mufreeWren, val);
	}

	MOD_PHYREG(pi, dacClkCtrl, txcore2corefrclken, val);
	MOD_PHYREG(pi, dacClkCtrl, txcore2corefrdacclken, val);
	MOD_PHYREG(pi, dacClkCtrl, gateafeclocksoveren, val);

	if ((ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
		ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		/* 4365B1 & 4347 */
		MOD_PHYREG(pi, dacClkCtrl, txfarrowresetfreeen, val);
	} else if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_47(pi->pubpi->phy_rev)) {
		/* 4365C0, 43684 */
		uint8 valc =  (enable ? 0 : 1);
		MOD_PHYREG(pi, dacClkCtrl, dacpuoren, val);
		MOD_PHYREG(pi, dacClkCtrl, txframeoreden, val);
		MOD_PHYREG(pi, dacClkCtrl, endacratiochgvld, 0);
		FOREACH_CORE(pi, core) {
			/* Protect core2core sync against resetcca */
			MOD_PHYREGCE(pi, TxResamplerEnable, core, tx_fifo_resetccaEn, valc);
			if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				/* Prevent txfarrow reset at end of frame */
				MOD_PHYREGCE(pi, TxResamplerEnable, core, tx_fifo_txresetEn, valc);
			}
		}
		MOD_PHYREG(pi, dacClkCtrl, txfarrowresetfreeen, val);
		/* WAR for core2core sync (EVM jaggedness/ PER floor) */
		MOD_PHYREG(pi, bphytestcontrol, bphytestAntselect, 0xf);
	}

	if (!enable) {
		/* Resetcca when disabling core2core sync */
		uint8 stall_val;
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);
		wlc_phy_resetcca_acphy(pi);
		ACPHY_ENABLE_STALL(pi, stall_val);

		/* 43684 specific settings
		 *  - Remove radioreg overrides for keeping DAC clocks ON
		 *  - should be done after resetcca (SWWLAN-191901)
		 */
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			FOREACH_CORE(pi, core) {
				/* Remove dac outbuf overrides */
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_outbuf_dac_pu, 0);
				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core,
				                    afediv_outbuf_dac_pu, 0);

				/* Remove dac_div pu and reset overrides */
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_dac_div_pu, 0);
				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core, afediv_dac_div_pu, 0);
				MOD_RADIO_REG_20698(pi, AFEDIV_CFG1_OVR, core,
				                    ovr_afediv_dac_div_reset, 0);
				MOD_RADIO_REG_20698(pi, AFEDIV_REG0, core,
				                    afediv_dac_div_reset, 0);
			}
		}
	}

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

}

void
phy_ac_chanmgr_hwobss(phy_ac_chanmgr_info_t *chanmgri, bool enable_hwobss)
{
	phy_info_t *pi = chanmgri->pi;
	if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (enable_hwobss) {
			WRITE_PHYREG(pi, obss_control, 0x7800);
			WRITE_PHYREG(pi, drop20sCtrl1, 0x0b0); /* drop2nd needed for obss */
			WRITE_PHYREG(pi, drop20sCtrl2, 0x37f);
			WRITE_PHYREG(pi, drop20sCtrl3, 0x3340);
			/* use table based AGC for HT header */
			WRITE_PHYREG(pi, TableBasedAGCcntrlA, 0x1c84);
		} else {
		/* disable obss */
			WRITE_PHYREG(pi, obss_control, 0x7000);
		}
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && (!ACMINORREV_0(pi))) {
		if (enable_hwobss) {
			WRITE_PHYREG(pi, obss_control, 0x7800);
			WRITE_PHYREG(pi, drop20sCtrl1, 0x0b0); /* drop2nd needed for obss */
			WRITE_PHYREG(pi, drop20sCtrl2, 0x37f);
			WRITE_PHYREG(pi, drop20sCtrl3, 0x3340);
			/* use table based AGC for HT header */
			//WRITE_PHYREG(pi, TableBasedAGCcntrlA, 0x1c84);
			/* new improvements 43684B0
			 * enable exit on strong SOI
			 * but keep abort_sec_en=0 (only to be enabled along with preemption)
			 */
			WRITE_PHYREG(pi, obss_param_extra, 0x48);
		} else {
		/* disable obss */
			WRITE_PHYREG(pi, obss_control, 0x7000);
			WRITE_PHYREG(pi, obss_param_extra, 0x3f);
		}
	}
}

static void
phy_ac_chanmgr_preempt(phy_type_chanmgr_ctx_t *ctx, bool enable_preempt,
    bool EnablePostRxFilter_Proc)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_ac_noise_preempt(info->aci->noisei, enable_preempt, EnablePostRxFilter_Proc);
}

void
wlc_phy_rxcore_setstate_acphy(wlc_phy_t *pih, uint8 rxcore_bitmask, uint8 phytxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;
	uint16 rfseqCoreActv_DisRx_save;
	uint16 rfseqMode_save;
	uint8 stall_val = 0, core;
	uint8 orig_rxfectrl1 = 0;
	uint16 classifier_state = 0;
	uint16 edThreshold_save[PHY_MAX_CORES] = {0};
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	ASSERT((rxcore_bitmask > 0) && (rxcore_bitmask <= 15));
	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev) &&
		!ACMAJORREV_37(pi->pubpi->phy_rev) &&
		!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if ((stf_shdata->phyrxchain == rxcore_bitmask) && !pi->u.pi_acphy->init &&
				!CCT_INIT(pi->u.pi_acphy) &&
				!pi->u.pi_acphy->chanmgri->data->rxchain_hw_notset)
			return;
	}
	phy_stf_set_phyrxchain(pi->stfi, rxcore_bitmask);
	phy_ac_chanmgr_set_both_txchain_rxchain(pi->u.pi_acphy->chanmgri,
		rxcore_bitmask, stf_shdata->phytxchain);

	if (!pi->sh->clk) {
		pi->u.pi_acphy->chanmgri->data->rxchain_hw_notset = TRUE;
		return;
	}
	pi->u.pi_acphy->chanmgri->data->rxchain_hw_notset = FALSE;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	pi->u.pi_acphy->chanmgri->data->phyrxchain_old = READ_PHYREGFLD(pi, CoreConfig, CoreMask);

	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
#ifdef OCL
		if (PHY_OCL_ENAB(pi->sh->physhim)) {
			if ((pi->u.pi_acphy->chanmgri->data->phyrxchain_old == 3) &&
				(stf_shdata->phyrxchain != 3)) {
				phy_ocl_disable_req_set(pih, OCL_DISABLED_SISO,
				                            TRUE, WLC_OCL_REQ_RXCHAIN);
			} else if (stf_shdata->phyrxchain == 3) {
				phy_ocl_disable_req_set(pih, OCL_DISABLED_SISO,
				                            FALSE, WLC_OCL_REQ_RXCHAIN);
			}
		}
#endif /* OCL */
		if (ACMAJORREV_40(pi->pubpi->phy_rev) &&
		    pi->u.pi_acphy->chanmgri->cfg->srom_nonbf_logen_mode_en &&
		    (pi->u.pi_acphy->chanmgri->data->phyrxchain_old != stf_shdata->phyrxchain)) {
			wlc_phy_low_pwr_logen_setting(pi, 0);
		}
#ifdef WL_NAP
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev) &&
			PHY_NAP_ENAB(pi->sh->physhim)) {
				if (pi->u.pi_acphy->chanmgri->data->phyrxchain_old !=
					stf_shdata->phyrxchain)
					phy_ac_nap_update_energy_threshold(pi->u.pi_acphy->napi);
			}
#endif /* WL_NAP */
	}

	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
		ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
		/* Disable classifier */
		classifier_state = READ_PHYREG(pi, ClassifierCtrl);
		phy_rxgcrs_sel_classifier(pi, 4);

		/* Disable stalls and hold FIFOs in reset */
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
		ACPHY_DISABLE_STALL(pi);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	}

	/* Save Registers */
	rfseqCoreActv_DisRx_save = READ_PHYREGFLD(pi, RfseqCoreActv2059, DisRx);
	rfseqMode_save = READ_PHYREG(pi, RfseqMode);

	// Reset PHY. some bad state of inactive cores causes trouble in active cores.
	wlc_phy_resetcca_acphy(pi);

	FOREACH_CORE(pi, core) {
		edThreshold_save[core] = READ_PHYREGC(pi, edThreshold, core);
		WRITE_PHYREGC(pi, edThreshold, core, 0xffff);
	}
	/* delay to allow the edThreshold setting take effect */
	OSL_DELAY(10);

	/* Indicate to PHY of the Inactive Core */
	MOD_PHYREG(pi, CoreConfig, CoreMask, rxcore_bitmask);
	/* Indicate to RFSeq of the Inactive Core */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RfseqCoreActv2059, EnRx,
			stf_shdata->hw_phyrxchain);
	} else {
		MOD_PHYREG(pi, RfseqCoreActv2059, EnRx, rxcore_bitmask);
	}
	ACPHY_REG_LIST_START
		/* Make sure Rx Chain gets shut off in Rx2Tx Sequence */
		MOD_PHYREG_ENTRY(pi, RfseqCoreActv2059, DisRx, 15)
		/* Make sure Tx Chain doesn't get turned off during this function */
		MOD_PHYREG_ENTRY(pi, RfseqCoreActv2059, EnTx, 0)
		MOD_PHYREG_ENTRY(pi, RfseqMode, CoreActv_override, 1)
	ACPHY_REG_LIST_EXECUTE(pi);
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		acphy_set_lpmode(pi, ACPHY_LP_RADIO_LVL_OPT);
	}

	wlc_phy_force_rfseq_noLoleakage_acphy(pi);

	/* Make TxEn chains point to phytxchain */
	/* Needed for X51A for Assymetric TX /RX mode */
	if (ACMAJORREV_33(pi->pubpi->phy_rev))
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx,	stf_shdata->hw_phytxchain);
	else
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, phytxchain);
	/*  Restore Register */
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, rfseqCoreActv_DisRx_save);
	WRITE_PHYREG(pi, RfseqMode, rfseqMode_save);

	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
		ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
		if (pi->pubpi->phy_rev >= 33) {
			/* Reset PHY */
			wlc_phy_resetcca_acphy(pi);
		}

		/* Restore FIFO reset and Stalls */
		ACPHY_ENABLE_STALL(pi, stall_val);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);
		OSL_DELAY(1);

		/* Restore classifier */
		WRITE_PHYREG(pi, ClassifierCtrl, classifier_state);
		OSL_DELAY(1);

		if (pi->pubpi->phy_rev < 33)
			wlc_phy_resetcca_acphy(pi);
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349 Channel Smoothing related changes */
		if ((phy_get_phymode(pi) == PHYMODE_MIMO) && (stf_shdata->phyrxchain == 0x3)) {
			MOD_PHYREG(pi, chnsmCtrl1, disable_2rx_nvar_calc, 0x0);
		} else {
			MOD_PHYREG(pi, chnsmCtrl1, disable_2rx_nvar_calc, 0x1);
		}
	}
	/* Restore edThreshold */
	FOREACH_CORE(pi, core) {
		WRITE_PHYREGC(pi, edThreshold, core, edThreshold_save[core]);
	}
	wlc_phy_set_sdadc_pd_val_per_core_acphy(pi);
	/* Imposing the radio overrides due to coremask change */
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		impbf_radio_ovrs_4347(pi, 1);
	}
	/* fix to make sure ed_crs does not fire on the inactive core */
	/* RB: http://wlan-rb.sj.broadcom.com/r/105578/ */
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		if (rxcore_bitmask == 1) {
			WRITE_PHYREG(pi, ed_crsEn, 0x00f);
		}
		if (rxcore_bitmask == 2) {
			WRITE_PHYREG(pi, ed_crsEn, 0x0f0);
		}
		if (rxcore_bitmask == 3) {
			WRITE_PHYREG(pi, ed_crsEn, 0x0ff);
		}
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev) && (pi->pubpi->slice == DUALMAC_AUX)) {
		MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn,
			READ_PHYREGFLD(pi, NapCtrl, nap_en) ? pi->pubpi->phy_coremask : 0);
	}
	/* To enable TSSI clk to make it indepenant to rxcore setting */
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RxFeCtrl1, forceSdFeClkEn, phytxchain);
	}
	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_update_rxchains(wlc_phy_t *pih, uint8 *rxcore_bitmask, uint8 *txcore_bitmask,
        uint8 phyrxchain, uint8 phytxchain)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	/* Local copy of phyrxchains before overwrite */
	*rxcore_bitmask = 0;
	/* Local copy of EnTx bits from RfseqCoreActv.EnTx */
	*txcore_bitmask = 0;

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* Save and overwrite Rx chains */
		*rxcore_bitmask = stf_shdata->phyrxchain;
		*txcore_bitmask = READ_PHYREGFLD(pi, RfseqCoreActv2059, EnTx);
		phy_stf_set_phyrxchain(pi->stfi, stf_shdata->hw_phyrxchain);
		wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, stf_shdata->hw_phyrxchain,
			phytxchain);
	} else if (!PHY_COREMASK_SISO(pi->pubpi->phy_coremask) &&
		(phy_get_phymode(pi) != PHYMODE_RSDB)) {
		/* Save and overwrite Rx chains */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		*rxcore_bitmask = stf_shdata->phyrxchain;
		*txcore_bitmask = READ_PHYREGFLD(pi, RfseqCoreActv2059, EnTx);
		wlapi_enable_mac(pi->sh->physhim);
		wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, phyrxchain,
			phytxchain);
	}
}

void
wlc_phy_restore_rxchains(wlc_phy_t *pih, uint8 enRx, uint8 enTx)
{
	phy_info_t *pi = (phy_info_t*)pih;

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* Restore Rx chains */
		wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, enRx, enTx);
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, enTx);
	} else if (!PHY_COREMASK_SISO(pi->pubpi->phy_coremask) &&
		(phy_get_phymode(pi) != PHYMODE_RSDB)) {
		/* Restore Rx chains */
		wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi, enRx, enTx);
	}
}

uint8
wlc_phy_rxcore_getstate_acphy(wlc_phy_t *pih)
{
	uint16 rxen_bits;
	phy_info_t *pi = (phy_info_t*)pih;

	rxen_bits = READ_PHYREGFLD(pi, RfseqCoreActv2059, EnRx);

	ASSERT(phy_stf_get_data(pi->stfi)->phyrxchain == rxen_bits);

	return ((uint8) rxen_bits);
}

bool
wlc_phy_is_scan_chan_acphy(phy_info_t *pi)
{
	return (SCAN_RM_IN_PROGRESS(pi) &&
	        (pi->interf->curr_home_channel != CHSPEC_CHANNEL(pi->radio_chanspec)));
}

void
wlc_phy_resetcca_acphy(phy_info_t *pi)
{
	uint32 phy_ctl_reg_val = 0;
	uint16 clkgatests_reg_val = 0;
	uint8 stall_val = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* SAVE PHY_CTL value */
	phy_ctl_reg_val = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
	/* MAC should be suspended before calling this function */
	ASSERT((R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC) == 0);

	/* If MACPhy clock not enabled (Bit1), wait for 1us */
	if ((phy_ctl_reg_val & 0x2) == 0) {
		OSL_DELAY(1);
	}

	if (PHY_MAC_REV_CHECK(pi, 36)) {
		/* Save ClkGateSts register */
		clkgatests_reg_val = R_REG(pi->sh->osh, D11_CLK_GATE_STS(pi));

		/* Set ForceMacPhyClockRequest bit in ClkGateSts register : SWWLAN-101393 */
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi),
			(clkgatests_reg_val | (1 << 4)));
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, sampleCmd, enable, 1);
	}

	/* bilge count sequence fix */
	if ((ACMAJORREV_1(pi->pubpi->phy_rev) &&
	     (ACMINORREV_0(pi) || ACMINORREV_1(pi))) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
			ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

		MOD_PHYREG(pi, BBConfig, resetCCA, 1);
		OSL_DELAY(1);
		if (!TINY_RADIO(pi) && !ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, RxFeCtrl1, rxfe_bilge_cnt, 0);
			OSL_DELAY(1);
		}
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		OSL_DELAY(1);
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
		OSL_DELAY(1);
		MOD_PHYREG(pi, BBConfig, resetCCA, 0);
		OSL_DELAY(1);

		SPINWAIT(READ_PHYREGFLD(pi, RfseqStatus0, reset2rx), ACPHY_SPINWAIT_RESET2RX);
		if (READ_PHYREGFLD(pi, RfseqStatus0, reset2rx)) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : Reset to Rx failed \n",
			__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_RESET2RX_FAILED);
		}
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
	} else if (IS_4364_3x3(pi)) {
		/* Force gated clocks on */
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x6); /* set reg(PHY_CTL) 0x6 */

		/* Disable Stalls */
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);

		/* Hold FIFO's in Reset */
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		OSL_DELAY(1);

		/* Do the Reset */
		MOD_PHYREG(pi, BBConfig, resetCCA, 1);
		OSL_DELAY(1);
		MOD_PHYREG(pi, BBConfig, resetCCA, 0);

		/* Wait for reset2rx finish, which is triggered by resetcca in hw */
		SPINWAIT(READ_PHYREGFLD(pi, RfseqStatus0, reset2rx), ACPHY_SPINWAIT_RESET2RX);
		if (READ_PHYREGFLD(pi, RfseqStatus0, reset2rx)) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : Reset to Rx failed \n",
			__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_RESET2RX_FAILED);
		}

		/* Make sure pktproc came out of reset */
		SPINWAIT((READ_PHYREGFLD(pi, pktprocdebug, pktprocstate) == 0),
				ACPHY_SPINWAIT_PKTPROC_STATE);
		if (READ_PHYREGFLD(pi, pktprocdebug, pktprocstate) == 0) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR : PKTPROC was in PKTRESET \n",
			__FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_PKTPROC_RESET_FAILED);
		}

		/* Undo Stalls and SDFEFIFO Reset */
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 0);
		ACPHY_ENABLE_STALL(pi, stall_val);
		OSL_DELAY(1);

		/* Force gated clocks off */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x2); /* set reg(PHY_CTL) 0x2 */
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);
		OSL_DELAY(1);

		/* # force gated clock on */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x6); /* set reg(PHY_CTL) 0x6 */
		OSL_DELAY(1);

		MOD_PHYREG(pi, BBConfig, resetCCA, 1);
		OSL_DELAY(1);
		MOD_PHYREG(pi, BBConfig, resetCCA, 0);
		/* Add wait-time for:
		 * 1) reset2rx finish, which is triggered by resetcca in hw
		 * 2) pktproc to come out of reset
		 */
		OSL_DELAY(10);

		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x2); /* set reg(PHY_CTL) 0x2 */

		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
	} else {
		wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);

		/* # force gated clock on */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x6); /* set reg(PHY_CTL) 0x6 */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0xe); /* MacPhyResetCCA = 1 */
		OSL_DELAY(1);
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x6); /* MacPhyResetCCA = 0 */
		W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x2); /* set reg(PHY_CTL) 0x2 */

		wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);
	}

	/* wait for reset2rx finish, which is triggered by resetcca in hw */
	OSL_DELAY(2);

	/* Restore PHY_CTL register */
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl_reg_val);

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, sampleCmd, enable, 0);
	}

	if (PHY_MAC_REV_CHECK(pi, 36)) {
		/* Restore ClkGateSts register */
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi), clkgatests_reg_val);
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev)|| ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RfseqMode, Trigger_override, 0);
	}
}

/* 20693_dyn_papd_cfg */
static void
wlc_acphy_dyn_papd_cfg_20693(phy_info_t *pi)
{
	uint8 core;
	FOREACH_CORE(pi, core) {
		if (core == 0) {
			MOD_PHYREG(pi, dyn_radioa0, dyn_radio_ovr0, 0);
		} else {
			MOD_PHYREG(pi, dyn_radioa1, dyn_radio_ovr1, 0);
		}
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
				ovr_pa2g_idac_cas, 1);
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
				ovr_pa2g_idac_incap_compen_main, 1);
			MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
				ovr_pa2g_idac_main, 1);
		} else {
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR3, core,
				ovr_pa5g_idac_cas, 1);
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
				ovr_pa5g_idac_incap_compen_main, 1);
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR2, core,
				ovr_pa5g_idac_main, 1);
		}
	}
}

static void
wlc_phy_set_bias_ipa_as_epa_acphy_20693(phy_info_t *pi, uint8 core)
{
	MOD_RADIO_REG_20693(pi, SPARE_CFG2, core,
		pa2g_bias_bw_main, 0);
	MOD_RADIO_REG_20693(pi, SPARE_CFG2, core,
		pa2g_bias_bw_cas, 0);
	MOD_RADIO_REG_20693(pi, SPARE_CFG2, core,
		pa2g_bias_bw_pmos, 0);
	MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
		ovr_pa2g_idac_main, 1);
	MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
		pa2g_idac_main, 0x24);
	MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
		ovr_pa2g_idac_cas, 1);
	MOD_RADIO_REG_20693(pi, PA2G_IDAC1, core,
		pa2g_idac_cas, 0x22);
	MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
		ovr_pa2g_idac_incap_compen_main, 1);
	MOD_RADIO_REG_20693(pi, PA2G_INCAP, core,
		pa2g_idac_incap_compen_main, 0x2d);
	MOD_RADIO_REG_20693(pi, TX_TOP_2G_OVR1_EAST, core,
		ovr_mx2g_idac_bbdc, 1);
	MOD_RADIO_REG_20693(pi, TXMIX2G_CFG6, core,
		mx2g_idac_bbdc, 0x1c);
	MOD_RADIO_REG_20693(pi, TXMIX2G_CFG2, core,
		mx2g_idac_cascode, 0x13);
}

void wlc_phy_radio20693_sel_logen_mode(phy_info_t *pi)
{
	uint16 phymode = phy_get_phymode(pi);
#ifdef WL_AIR_IQ
	if (phymode == PHYMODE_BGDFS) {
		wlc_phy_radio20693_setup_logen_3plus1(pi, pi->radio_chanspec,
				pi->u.pi_acphy->chanmgri->radio_chanspec_sc);
		return;
	}
#endif /* WL_AIR_IQ */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		wlc_phy_radio20693_sel_logen_5g_mode(pi, 5);
		wlc_phy_radio20693_sel_logen_2g_mode(pi, 0);
	} else {
		wlc_phy_radio20693_sel_logen_2g_mode(pi, 2);
		if (phymode == PHYMODE_BGDFS) {
		  wlc_phy_radio20693_sel_logen_5g_mode(pi, 2);
		} else if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
		  wlc_phy_radio20693_sel_logen_5g_mode(pi, 1);
		} else if (CHSPEC_IS160(pi->radio_chanspec)) {
		  wlc_phy_radio20693_sel_logen_5g_mode(pi, 0);
		  ASSERT(0);
		} else {
		  wlc_phy_radio20693_sel_logen_5g_mode(pi, 0);
		}
	}
}

#ifdef WL_AIR_IQ
static void
wlc_phy_radio20693_set_tia(phy_info_t *pi, uint8 core, bool enable)
{
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_pwrup_resstring, 1);
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_amp1_pwrup, 1);
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_pwrup_amp2, 1);
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_amp2_bypass, 1);
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_enable_st1, 1);
	MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_offset_dac_pwrup, 1);
	MOD_RADIO_REG_20693(pi, TIA_CFG6, core, tia_pwrup_resstring, enable);
	MOD_RADIO_REG_20693(pi, TIA_CFG5, core, tia_amp1_pwrup, enable);
	MOD_RADIO_REG_20693(pi, TIA_CFG7, core, tia_pwrup_amp2, 0);
	MOD_RADIO_REG_20693(pi, TIA_CFG1, core, tia_amp2_bypass, 3);
	MOD_RADIO_REG_20693(pi, TIA_CFG1, core, tia_enable_st1, 0);
	MOD_RADIO_REG_20693(pi, TIA_CFG8, core, tia_offset_dac_pwrup, enable);
}
static void
wlc_phy_radio20693_set_rxmix_2g(phy_info_t *pi, uint8 core, bool enable)
{
	bool enable_inv = !enable;
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core, ovr_rxmix2g_lobuf_pu, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_rxmix2g_pu, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_rf2g_mix1st_en, 1);
	MOD_RADIO_REG_20693(pi, RXMIX2G_CFG1, core, rxmix2g_lobuf_pu, enable);
	MOD_RADIO_REG_20693(pi, RXMIX2G_CFG1, core, rxmix2g_pu, enable);
	MOD_RADIO_REG_20693(pi, RXMIX2G_CFG1, core, rf2g_mix1st_en, enable_inv);
	MOD_RADIO_REG_20693(pi, LOGEN2G_RCCR, core, rx2g_iqbias_short,  enable);
}
static void
wlc_phy_radio20693_set_lna2_2g(phy_info_t *pi, uint8 core, bool enable)
{
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core, ovr_gm2g_pwrup, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core, ovr_lna2g_nap, 1);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core, gm2g_pwrup, enable);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core, lna2g_nap, enable);
}
static void
wlc_phy_radio20693_set_lna1_2g(phy_info_t *pi, uint8 core, bool enable)
{
	bool enable_inv = !enable;
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_lna2g_lna1_pu, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_lna2g_tr_rx_en, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_lna2g_lna1_out_short_pu, 1);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core, lna2g_lna1_pu, enable);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core, lna2g_tr_rx_en, enable);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core, lna2g_lna1_out_short_pu, enable_inv);
}
static void
wlc_phy_radio20693_set_rx2g(phy_info_t *pi, uint8 core, bool enable)
{
	wlc_phy_radio20693_set_lna1_2g(pi, core, enable);
	wlc_phy_radio20693_set_lna2_2g(pi, core, enable);
	wlc_phy_radio20693_set_rxmix_2g(pi, core, enable);
	MOD_RADIO_REG_20693(pi, RXIQMIX_CFG1, core, rxmix2g_lobuf_pu, enable);
	MOD_RADIO_REG_20693(pi, RXIQMIX_CFG1, core, rxiq2g_coupler_pu, 0);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core, lna2g_lna1_bypass, 0);
	MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core, ovr_lna2g_lna1_bypass, 1);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core, lna2g_lna1_bypass_hiip3, 0);
	MOD_RADIO_REG_20693(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
	MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 0);
	wlc_phy_radio20693_set_tia(pi, core, enable);
}
static void
wlc_phy_radio20693_set_rxmix_5g(phy_info_t *pi, uint8 core, bool enable)
{
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_mix5g_en, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_rxmix5g_lobuf_en, 1);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG3, core, mix5g_en, enable);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG4, core, mix5g_en_lb, enable);
	MOD_RADIO_REG_20693(pi, LOGEN5G_RCCR, core, logen5g_rx_iqbias_short, enable);
	MOD_RADIO_REG_20693(pi, LOGEN5G_RCCR, core, lobuf_en,  enable);
}
static void
wlc_phy_radio20693_set_lna2_5g(phy_info_t *pi, uint8 core, bool enable)
{
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_gm5g_pwrup, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_nap, 1);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG2, core, gm5g_pwrup, enable);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG2, core, lna5g_nap, enable);
}
static void
wlc_phy_radio20693_set_lna1_5g(phy_info_t *pi, uint8 core, bool enable)
{
	bool enable_inv = !enable;
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_lna1_pu, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_tr_rx_en, 1);
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_lna1_out_short_pu, 1);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core, lna5g_lna1_pu, enable);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core, lna5g_tr_rx_en, enable);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core, lna5g_lna1_out_short_pu, enable_inv);
}
static void
wlc_phy_radio20693_set_rx5g(phy_info_t *pi, uint8 core, bool enable)
{
	wlc_phy_radio20693_set_lna1_5g(pi, core, enable);
	wlc_phy_radio20693_set_lna2_5g(pi, core, enable);
	wlc_phy_radio20693_set_rxmix_5g(pi, core, enable);
	MOD_RADIO_REG_20693(pi, LOGEN5G_EPAPD, core, loopback5g_cal_pu, 0);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core, lna5g_lna1_bypass, 0);
	MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR, core, ovr_lna5g_lna1_bypass, 1);
	MOD_RADIO_REG_20693(pi, LOGEN5G_EPAPD, core, epapd_en, 0);
	MOD_RADIO_REG_20693(pi, LNA5G_CFG2, core, auxgm_pwrup, 0);
	MOD_RADIO_REG_20693(pi, LOGEN5G_EPAPD, core, rxiq5g_coupler_pu, 0);
	wlc_phy_radio20693_set_tia(pi, core, enable);
}
static void
wlc_phy_radio20693_save_3plus1(phy_info_t *pi, uint8 core, uint8 restore)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	phy_ac_chanmgr_info_t *chnmgri = pi_ac->chanmgri;
	phy_ac_chanmgr_20693_3plus1_regs_t *r = &chnmgri->regs_save_20693_3plus1;
	uint16 ct;
	uint16 radio_regs[] = {
		RADIO_REG_20693(pi, LNA2G_CFG1, core),
		RADIO_REG_20693(pi, LNA2G_CFG2, core),
		RADIO_REG_20693(pi, LNA5G_CFG1, core),
		RADIO_REG_20693(pi, LNA5G_CFG2, core),
		RADIO_REG_20693(pi, LNA5G_CFG3, core),
		RADIO_REG_20693(pi, LNA5G_CFG4, core),
		RADIO_REG_20693(pi, LOGEN2G_RCCR, core),
		RADIO_REG_20693(pi, LOGEN5G_EPAPD, core),
		RADIO_REG_20693(pi, LOGEN5G_RCCR, core),
		RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core),
		RADIO_REG_20693(pi, RXIQMIX_CFG1, core),
		RADIO_REG_20693(pi, RXMIX2G_CFG1, core),
		RADIO_REG_20693(pi, RXRF2G_CFG2, core),
		RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core),
		RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core),
		RADIO_REG_20693(pi, RX_TOP_5G_OVR, core),
		RADIO_REG_20693(pi, TIA_CFG1, core),
		RADIO_REG_20693(pi, TIA_CFG5, core),
		RADIO_REG_20693(pi, TIA_CFG6, core),
		RADIO_REG_20693(pi, TIA_CFG7, core),
		RADIO_REG_20693(pi, TIA_CFG8, core),
		RADIO_PLLREG_20693(pi, LO2G_LOGEN0_CFG1, 0),
		RADIO_PLLREG_20693(pi, LO2G_LOGEN1_CFG1, 0),
		RADIO_PLLREG_20693(pi, LO2G_VCO_DRV_CFG1, 0),
		RADIO_PLLREG_20693(pi, LO5G_CORE0_CFG1, 0),
		RADIO_PLLREG_20693(pi, LO5G_CORE1_CFG1, 0),
		RADIO_PLLREG_20693(pi, LO5G_CORE2_CFG1, 0)
	};
	ASSERT(chnmgri);
	r = &chnmgri->regs_save_20693_3plus1;
	PHY_TRACE(("%s: saving core=%d cnt=%d.\n", __FUNCTION__, core, r->save_count));
	ASSERT(ARRAYSIZE(radio_regs) == ARRAYSIZE(r->regs));
	if (restore) {
		r->save_count--;
		for (ct = 0; ct < ARRAYSIZE(radio_regs); ct++) {
			phy_utils_write_radioreg(pi, radio_regs[ct], r->regs[ct]);
		}
	} else {
		r->save_count++;
		for (ct = 0; ct < ARRAYSIZE(radio_regs); ct++) {
			r->regs[ct] = _READ_RADIO_REG(pi, radio_regs[ct]);
		}
	}
}
static void
wlc_phy_radio20693_setup_logen_3plus1(phy_info_t *pi, chanspec_t chanspec, chanspec_t chanspec_sc)
{
	int ct;
	uint16 pll_regs_bit_vals_2g_5g[][3] = {
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN1_CFG1, 0, lo2g_logen1_pu, 0),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_main_inv_pu, 1),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN1_CFG1, 0, logen1_sel, 1),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE0_CFG1, 0, logen0_pu, 0),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE0_CFG1, 0, core0_gm_pu, 0),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE1_CFG1, 0, core1_gm_pu, 0),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE2_CFG1, 0, core2_gm_pu, 0)
	};
	uint16 pll_regs_bit_vals_5g_2g[][3] = {
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN0_CFG1, 0, lo2g_logen0_pu, 0),
	};
	if (CHSPEC_IS2G(chanspec_sc)) {
		if (CHSPEC_IS2G(chanspec)) {
			wlc_phy_radio20693_sel_logen_5g_mode(pi, 5);
			wlc_phy_radio20693_sel_logen_2g_mode(pi, 1);
		} else {
			wlc_phy_radio20693_sel_logen_5g_mode(pi, 4);
			wlc_phy_radio20693_sel_logen_2g_mode(pi, 1);
			for (ct = 0; ct < ARRAYSIZE(pll_regs_bit_vals_5g_2g); ct++) {
				phy_utils_mod_radioreg(pi, pll_regs_bit_vals_5g_2g[ct][0],
					pll_regs_bit_vals_5g_2g[ct][1],
					pll_regs_bit_vals_5g_2g[ct][2]);
			}
		}
		wlc_phy_radio20693_set_rx5g(pi, 3, FALSE);
		wlc_phy_radio20693_set_rx2g(pi, 3, TRUE);
	} else {
		if (CHSPEC_IS2G(chanspec)) {
			wlc_phy_radio20693_sel_logen_2g_mode(pi, 0);
			for (ct = 0; ct < ARRAYSIZE(pll_regs_bit_vals_2g_5g); ct++) {
				phy_utils_mod_radioreg(pi, pll_regs_bit_vals_2g_5g[ct][0],
					pll_regs_bit_vals_2g_5g[ct][1],
					pll_regs_bit_vals_2g_5g[ct][2]);
			}
		} else {
			wlc_phy_radio20693_sel_logen_2g_mode(pi, 2);
		}
		wlc_phy_radio20693_sel_logen_5g_mode(pi, 2);
		wlc_phy_radio20693_set_rx2g(pi, 3, FALSE);
		wlc_phy_radio20693_set_rx5g(pi, 3, TRUE);
	}
}
#endif /* WL_AIR_IQ */
void wlc_phy_radio20693_sel_logen_5g_mode(phy_info_t *pi, int mode)
{
	int ct;
	uint16 logen0_5g_inv_pu_val[] =	{1, 1, 1, 1, 1, 0};
	uint16 logen0_pu_val[]		  =	{1, 1, 1, 1, 1, 0};
	uint16 core0_gm_pu_val[]	  =	{1, 1, 1, 1, 1, 0};
	uint16 core1_gm_pu_val[]	  =	{1, 1, 1, 1, 1, 0};
	uint16 logen1_5g_inv_pu_val[] = {0, 1, 1, 1, 0, 0};
	uint16 logen1_pu_val[]		  =	{0, 1, 1, 1, 0, 0};
	uint16 logen1_gm_pu_val[]	  = {0, 1, 0, 1, 0, 0};
	uint16 core2_gm_pu_val[]	  =	{1, 0, 1, 0, 1, 0};
	uint16 core2_lc_pu_val[]	  =	{1, 1, 1, 0, 1, 0};
	uint16 core2_mux_sel_val[]	  =	{0, 1, 0, 0, 0, 0};
	uint16 core3_gm_pu_val[]	  =	{1, 1, 1, 1, 0, 0};
	//uint16 core3_lc_pu_val[]	  =	{1, 1, 1, 1, 0, 0};
	uint16 core3_mux_pu_val[]	  = {1, 1, 1, 1, 0, 0};
	uint16 core3_mux_sel_val[]	  =	{0, 1, 1, 1, 0, 0};
	uint16 bias0_pu_val[]         =	{1, 1, 1, 1, 1, 0};
	uint16 bias1_pu_val[]         =	{1, 1, 1, 1, 1, 0};
	uint16 bias2_pu_val[]		  =	{1, 1, 1, 1, 1, 0};
	uint16 bias3_pu_val[]		  = {1, 1, 1, 1, 1, 0};
	uint16 logen1_vco_inv_pu_val[]  = {0, 1, 1, 1, 0, 0};
	uint16 logen1_main_inv_pu_val[] = {0, 1, 1, 1, 0, 0};

	uint16 pll_regs_bit_vals[][3] = {
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen0_5g_inv_pu,
		logen0_5g_inv_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE0_CFG1, 0, logen0_pu,
		logen0_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_5g_inv_pu,
		logen1_5g_inv_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE1_CFG1, 0, logen1_pu,
		logen1_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE1_CFG1, 0, logen1_gm_pu,
		logen1_gm_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE0_CFG1, 0, core0_gm_pu,
		core0_gm_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE1_CFG1, 0, core1_gm_pu,
		core1_gm_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE2_CFG1, 0, core2_gm_pu,
		core2_gm_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE3_CFG1, 0, core3_gm_pu,
		core3_gm_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE2_CFG1, 0, core2_lc_pu,
		core2_lc_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE2_CFG1, 0, core2_mux_sel,
		core2_mux_sel_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE3_CFG1, 0, core3_lc_pu,
		core3_mux_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE3_CFG1, 0, core3_mux_sel,
		core3_mux_sel_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE0_IDAC2, 0, bias0_pu,
		bias0_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE1_IDAC2, 0, bias1_pu,
		bias1_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE2_IDAC2, 0, bias2_pu,
		bias2_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO5G_CORE3_IDAC2, 0, bias3_pu,
		bias3_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_vco_inv_pu,
		logen1_vco_inv_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_main_inv_pu,
		logen1_main_inv_pu_val[mode])
	};

	for (ct = 0; ct < ARRAYSIZE(pll_regs_bit_vals); ct++) {
		phy_utils_mod_radioreg(pi, pll_regs_bit_vals[ct][0],
		                       pll_regs_bit_vals[ct][1], pll_regs_bit_vals[ct][2]);
	}
}

void wlc_phy_radio20693_sel_logen_2g_mode(phy_info_t *pi, int mode)
{
	int ct;
	uint16 logen0_pu_val[]              = {1, 1, 0};
	uint16 logen1_pu_val[]              = {1, 1, 0};
	uint16 logen1_sel_val[]	            = {0, 1, 0};
	uint16 logen1_vco_inv_pu_val[]      = {0, 1, 0};
	uint16 logen1_main_inv_pu_val[]     = {0, 1, 0};
	uint16 logen1_div3_en_val[]         = {0, 1, 0};
	uint16 logen1_div4_en_val[]         = {0, 0, 0};
	uint16 logen1_idac_cklc_bias_val[]  = {0, 3, 0};
	uint16 logen1_idac_cklc_qb_val[]    = {0, 4, 0};

	uint16 pll_regs_bit_vals[][3] = {
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN0_CFG1, 0, lo2g_logen0_pu,
		logen0_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN1_CFG1, 0, lo2g_logen1_pu,
		logen1_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_LOGEN1_CFG1, 0, logen1_sel,
		logen1_sel_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_vco_inv_pu,
		logen1_vco_inv_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_VCO_DRV_CFG1, 0, logen1_main_inv_pu,
		logen1_main_inv_pu_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_SPARE0, 0, lo2g_1_div3_en,
		logen1_div3_en_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi, LO2G_SPARE0, 0, lo2g_1_div4_en,
		logen1_div4_en_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi,  LO2G_LOGEN1_IDAC1, 0, logen1_idac_cklc_bias,
		logen1_idac_cklc_bias_val[mode]),
		RADIO_PLLREGC_FLD_20693(pi,  LO2G_LOGEN1_IDAC1, 0, logen1_idac_cklc_qb,
		logen1_idac_cklc_qb_val[mode])
	};

	for (ct = 0; ct < ARRAYSIZE(pll_regs_bit_vals); ct++) {
			phy_utils_mod_radioreg(pi, pll_regs_bit_vals[ct][0],
			pll_regs_bit_vals[ct][1], pll_regs_bit_vals[ct][2]);
	}
}

void wlc_phy_radio20693_afe_clkdistribtion_mode(phy_info_t *pi, int mode)
{
	MOD_RADIO_PLLREG_20693(pi, AFECLK_DIV_CFG1, 0, afeclk_mode, mode);
}

void wlc_phy_radio20693_force_dacbuf_setting(phy_info_t *pi)
{
	uint8 core;

	FOREACH_CORE(pi, core) {
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20693_ENTRY(pi, TX_DAC_CFG5, core, DACbuf_fixed_cap, 0)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_BB_OVR1, core, ovr_DACbuf_fixed_cap, 1)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_DAC_CFG5, core, DACbuf_Cap, 0x6)
			MOD_RADIO_REG_20693_ENTRY(pi, TX_BB_OVR1, core, ovr_DACbuf_Cap, 1)
		RADIO_REG_LIST_EXECUTE(pi, core);
	}

}

/* Clean up chanspec */
void
chanspec_get_operating_channels(phy_info_t *pi, uint8 *ch)
{
	bool is_80p80 = FALSE;
	uint8 core;
	uint16 phymode = phy_get_phymode(pi);
	phy_ac_chanmgr_data_t *data = pi->u.pi_acphy->chanmgri->data;
	BCM_REFERENCE(is_80p80);
	for (core = 0; core < PHY_CORE_MAX; core++) {
		data->core_freq_mapping[core] = PRIMARY_FREQ_SEGMENT;
	}

	/* RSDB family has 80p80, need to handle carefully */
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		if (phymode == PHYMODE_80P80) {
			ch[0] = wf_chspec_primary80_channel(pi->radio_chanspec);
			ch[1] = wf_chspec_secondary80_channel(pi->radio_chanspec);
			data->core_freq_mapping[0] = PRIMARY_FREQ_SEGMENT;
			data->core_freq_mapping[1] = SECONDARY_FREQ_SEGMENT;
			is_80p80 = TRUE;
		} else {
			ch[0] = CHSPEC_CHANNEL(pi->radio_chanspec);
			ch[1] = ch[0];
		}
	} else {
		if (PHY_AS_80P80(pi, pi->radio_chanspec) &&
		(CHSPEC_IS160(pi->radio_chanspec) || CHSPEC_IS8080(pi->radio_chanspec))) {
			wf_chspec_get_80p80_channels(pi->radio_chanspec, ch);
		} else {
			ch[0] = CHSPEC_CHANNEL(pi->radio_chanspec);
			ch[1] = 0;
		}
	}

	PHY_INFORM(("wl%d: %s channels (%d, %d) | %s\n", PI_INSTANCE(pi), __FUNCTION__, ch[0],
		is_80p80 ? ch[1] : ch [0], is_80p80 ? "chan bonded" : "not 80p80, single chan"));
}

static void
chanspec_tune_phy_ACMAJORREV_40(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	chanspec_tune_phy_dccal(pi, FALSE);

	if (!SCAN_RM_IN_PROGRESS(pi)) {
		if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
			wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
				pi_ac->chanmgri->acphy_smth_dump_mode);

			/* Enable LESI for 4347 from the NVRAM file */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (pi_ac->chanmgri->lesi_perband[0] == 1) {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
				} else {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, FALSE, 0);
				}
			} else {
				if (pi_ac->chanmgri->lesi_perband[1] == 1) {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
				} else {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, FALSE, 0);
				}
			}
		}
	}
}

static void
chanspec_tune_phy_ACMAJORREV_44_46(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (!ISSIM_ENAB(pi->sh->sih))
		chanspec_tune_phy_dccal(pi, FALSE);

	if (!SCAN_RM_IN_PROGRESS(pi)) {
		if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
			wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
				pi_ac->chanmgri->acphy_smth_dump_mode);

			/* Enable LESI for 4347 from the NVRAM file */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				if (pi_ac->chanmgri->lesi_perband[0] == 1) {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
				} else {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, FALSE, 0);
				}
			} else {
				if (pi_ac->chanmgri->lesi_perband[1] == 1) {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
				} else {
					phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, FALSE, 0);
				}
			}
		}
#ifdef WL_NAP
		/* Enable or disable Napping feature for 4347 */
		if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			if (PHY_NAP_ENAB(pi->sh->physhim)) {
				phy_ac_config_napping_28nm(pi);
			}
		}
#endif /* WL_NAP */

	}
}

static void
chanspec_tune_phy_ACMAJORREV_37(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (CCT_INIT(pi_ac)) {
		/* Enable MRC SIG QUAL */
		MOD_PHYREG(pi, MrcSigQualControl0, enableMrcSigQual, 0x1);

		// Enable LESI for 7271
		phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
	}

	if (CCT_INIT(pi_ac)) {
		// copied below from 4366 EAGLE flow and under CCT_INIT so printf only once
		// MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 1);
		// wlc_tiny_setup_coarse_dcc(pi);
		printf("FIXME: 7271 still need to implement and setup dc calibration here\n");
	}

}

static void
chanspec_tune_phy_ACMAJORREV_47_51(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	chanspec_tune_phy_dccal(pi, FALSE);

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}

}

static void
chanspec_tune_phy_ACMAJORREV_36(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	chanspec_tune_phy_dccal(pi, TRUE);

	/* Spur Canceller */
	phy_ac_spurcan(pi_ac->rxspuri, TRUE);

	/* Disable IQ swap in QT */
	if (ISSIM_ENAB(pi->sh->sih)) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq0, 0x0)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq1, 0x0)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq2, 0x0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}
}

static void
chanspec_tune_phy_ACMAJORREV_32(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 core;
	bool elna_present = (CHSPEC_IS2G(pi->radio_chanspec)) ? BF_ELNA_2G(pi_ac) :
		BF_ELNA_5G(pi_ac);

	/* setup DCC parameters */
	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_0(pi)) {
			/* Disable the old DCC */
			MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 0);
			FOREACH_CORE(pi, core) {
				MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core,
					ovr_tia_offset_dac, 1);
			}
		} else {
			MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl, 1);
			wlc_tiny_setup_coarse_dcc(pi);
		}
	}

	phy_ac_spurcan(pi_ac->rxspuri, !elna_present);

	if ((CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) &&
		ACMAJORREV_33(pi->pubpi->phy_rev)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}

	if (ISSIM_ENAB(pi->sh->sih)) {
		ACPHY_DISABLE_STALL(pi);

		MOD_PHYREG(pi, RxFeCtrl1, swap_iq0, 0x0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq1, 0x0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq2, 0x0);
		MOD_PHYREG(pi, RxFeCtrl1, swap_iq3, 0x0);
		MOD_PHYREG(pi, Core1TxControl, iqSwapEnable, 0x0);
		MOD_PHYREG(pi, Core2TxControl, iqSwapEnable, 0x0);
		MOD_PHYREG(pi, Core3TxControl, iqSwapEnable, 0x0);
		MOD_PHYREG(pi, Core4TxControl, iqSwapEnable, 0x0);
	}

	wlc_dcc_fsm_reset(pi);

	if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
			/* Disable MRC SIG QUAL for BW80p80 for abnormal RTL behavior */
			MOD_PHYREG(pi, MrcSigQualControl0, enableMrcSigQual, 0x0);
		} else {
			/* Enable MRC SIG QUAL */
			MOD_PHYREG(pi, MrcSigQualControl0, enableMrcSigQual, 0x1);
		}
	}
}

static void
chanspec_tune_phy_ACMAJORREV_5(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
}

static void
chanspec_tune_phy_ACMAJORREV_4(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	bool elna_present = (CHSPEC_IS2G(pi->radio_chanspec)) ? BF_ELNA_2G(pi_ac) :
		BF_ELNA_5G(pi_ac);

	/* setup DCC parameters */
	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac))
		wlc_tiny_setup_coarse_dcc(pi);

	phy_ac_spurcan(pi_ac->rxspuri, !elna_present);

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}

	/* 4349A0: in quickturn, disable stalls and swap iq */
	if (ISSIM_ENAB(pi->sh->sih)) {
		ACPHY_REG_LIST_START
			ACPHY_DISABLE_STALL_ENTRY(pi)

			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq0, 0x0)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, swap_iq1, 0x0)
			MOD_PHYREG_ENTRY(pi, Core1TxControl, iqSwapEnable, 0x0)
		ACPHY_REG_LIST_EXECUTE(pi);

		if (phy_get_phymode(pi) != PHYMODE_RSDB)
			MOD_PHYREG(pi, Core2TxControl, iqSwapEnable, 0x0);
	}

	if (ROUTER_4349(pi)) {
		acphy_router_4349_nvshptbl_t
			router_4349_nvshptbl[ACPHY_NUM_SPUR_CHANS_ROUTER4349] = {
				{ 2427, 0,  16, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2437, 0,  48, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2422, 1,  32, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2427, 1,  16, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2437, 1, 112, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2442, 1,  96, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2447, 1,  80, 1, 0, RSDB_SPUR | MIMO_SPUR},
				{ 2452, 0,  18, 2, 9, RSDB_SPUR},
				{ 2457, 0,   2, 2, 9, RSDB_SPUR},
				{ 2462, 0,  50, 2, 9, RSDB_SPUR},
				{ 2467, 0,  34, 2, 9, RSDB_SPUR},
				{ 2442, 1,  50, 2, 9, RSDB_SPUR},
				{ 2447, 1,  34, 2, 9, RSDB_SPUR},
				{ 2452, 1,  18, 2, 9, RSDB_SPUR},
				{ 2457, 1,   2, 2, 9, RSDB_SPUR},
				{ 2462, 1, 114, 2, 9, RSDB_SPUR}
		};
		uint32 NvShpTbl[ACPHY_NSHAPETBL_MAX_TONES_ROUTER4349] = {0};
		uint16 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
		uint16 freq_mhz = (uint16)wf_channel2mhz(channel, WF_CHAN_FACTOR_2_4_G);
		uint8 offset = 0;
		uint8 bw = CHSPEC_IS40(pi->radio_chanspec);
		uint8 cnt, num_tones, i, spur_mode, core;

		/* Reset the entries */
		for (cnt = 0; cnt < ACPHY_NUM_SPUR_CHANS_ROUTER4349; ++cnt) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVNOISESHAPINGTBL,
				MIN(ACPHY_NSHAPETBL_MAX_TONES_ROUTER4349,
				router_4349_nvshptbl[cnt].num_tones),
				router_4349_nvshptbl[cnt].offset, 32, NvShpTbl);
		}

		for (cnt = 0; cnt < ACPHY_NUM_SPUR_CHANS_ROUTER4349; ++cnt) {
			/* If the chan is spur affected, update its NvShpTbl */
			if (freq_mhz == router_4349_nvshptbl[cnt].freq &&
				bw == router_4349_nvshptbl[cnt].bw) {
				offset = router_4349_nvshptbl[cnt].offset;
				spur_mode = router_4349_nvshptbl[cnt].spur_mode;
				num_tones = MIN(ACPHY_NSHAPETBL_MAX_TONES_ROUTER4349,
					router_4349_nvshptbl[cnt].num_tones);

				/* Update the noise variance in the NvShpTbl */
				for (i = 0; i < num_tones; ++i) {
					NvShpTbl[i] =
						router_4349_nvshptbl[cnt].nv_val;
					/* Writing the same Nv for all the cores */
					FOREACH_CORE(pi, core) {
						NvShpTbl[i] |= (NvShpTbl[i] << (8 * core));
					}
				}

				if (phy_get_phymode(pi) == PHYMODE_RSDB) {
					if (spur_mode & RSDB_SPUR) {
						wlc_phy_table_write_acphy(pi,
							ACPHY_TBL_ID_NVNOISESHAPINGTBL, num_tones,
							offset, 32, NvShpTbl);
					}
				} else if (phy_get_phymode(pi) == PHYMODE_MIMO) {
					if (spur_mode & MIMO_SPUR) {
						wlc_phy_table_write_acphy(pi,
							ACPHY_TBL_ID_NVNOISESHAPINGTBL, num_tones,
							offset, 32, NvShpTbl);
					}
				}
			}
		}

		/* JIRAs: SWWLAN-69184,SWWLAN-70731: Fix for BCC 2-stream
		 * 11n/11ac rate failures in 53573
		 */
		MOD_PHYREG(pi, DemodSoftreset, demod_reset_on_pktprocreset, 0x1);
		OSL_DELAY(10);
		MOD_PHYREG(pi, DemodSoftreset, demod_reset_on_pktprocreset, 0x0);

		if (CHSPEC_IS5G(pi->radio_chanspec) && !BF_ELNA_5G(pi->u.pi_acphy)) {
			FOREACH_CORE(pi, core) {
				MOD_PHYREGC(pi, FastAgcClipCntTh, core, fastAgcNbClipCntTh,
					CHSPEC_BW_LE20(pi->radio_chanspec) ? 0x0a :
					CHSPEC_IS40(pi->radio_chanspec) ? 0x14 : 0x28);
			}
		}
	}

	wlc_dcc_fsm_reset(pi);
}

static void
chanspec_tune_phy_ACMAJORREV_3(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* setup DCC parameters */
	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac))
		wlc_tiny_setup_coarse_dcc(pi);

	/* Spur war for 4345ilna */
	if (PHY_ILNA(pi))
		wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), TRUE, FALSE);

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}
}

static void
chanspec_tune_phy_ACMAJORREV_2(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if ((!ACMINORREV_0(pi) && !ACMINORREV_2(pi)) &&
		CHSPEC_IS2G(pi->radio_chanspec) && (BF2_2G_SPUR_WAR(pi_ac) == 1)) {
		phy_ac_dssfB(pi_ac->rxspuri, TRUE);
	}

	/* Spur war for 4350 */
	if (BF2_2G_SPUR_WAR(pi_ac) == 1)
		wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), TRUE, FALSE);
}

static void
chanspec_tune_phy_ACMAJORREV_1(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* Spur war for 4335 Ax/Bx IPA */
	if (PHY_ILNA(pi) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		if ((BF2_2G_SPUR_WAR(pi_ac) == 1) &&
			CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), TRUE, FALSE);
			MOD_RADIO_REG(pi, RFP, PLL_XTAL5, xtal_bufstrg_BT, 3);
			PHY_TRACE(("BT buffer 3 for Spur WAR; %s \n", __FUNCTION__));
		}
		if ((BF3_5G_SPUR_WAR(pi_ac) == 1) &&
				CHSPEC_IS5G(pi->radio_chanspec)) {
			wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), TRUE, FALSE);
		}
	}

	/* Spur war for 4339iLNA */
	if (PHY_ILNA(pi) && ACMINORREV_2(pi))
		wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), TRUE, FALSE);

	/* Nvshp for 4335 C0 ELNA, 80 MHz since tight filter is being used */
	if (ACMINORREV_2(pi) && (!(PHY_ILNA(pi)))) {
		if (CHSPEC_IS80(pi->radio_chanspec)) {
			wlc_phy_spurwar_nvshp_acphy(pi, CCT_BW_CHG(pi_ac), FALSE, TRUE);
		} else {
		/* Restoring default for 20/40 mhz by reseting it */
			if (CCT_BW_CHG(pi_ac))
				wlc_phy_reset_noise_var_shaping_acphy(pi);
		}
	}

	MOD_PHYREG(pi, RfseqMode, CoreActv_override, 0);

	if ((CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) && ACMINORREV_2(pi)) {
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}
}

static void
chanspec_tune_phy_ACMAJORREV_0(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* Update ucode settings based on current band/bw */
	if (CCT_INIT(pi_ac) || CCT_BAND_CHG(pi_ac) || CCT_BW_CHG(pi_ac))
		phy_ac_hirssi_set_ucode_params(pi);
}

static void
phy_ac_chanmgr_dccal_force(phy_info_t *pi)
{
	uint8 mac_suspend;

	if (!pi->sh->up) return;

	if (!ACMAJORREV_47_51(pi->pubpi->phy_rev)) return;

	mac_suspend = (R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (mac_suspend) wlapi_suspend_mac_and_wait(pi->sh->physhim);

	chanspec_tune_phy_dccal(pi, TRUE);

	if (mac_suspend) wlapi_enable_mac(pi->sh->physhim);
}

static void
chanspec_tune_phy_dccal(phy_info_t *pi, bool force)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (!SCAN_RM_IN_PROGRESS(pi) || force) {
		/* setup DCC parameters */
		if (CCT_INIT(pi_ac) || force) {
			phy_ac_dccal_init(pi);
			phy_ac_load_gmap_tbl(pi);
		} else if (CCT_BAND_CHG(pi_ac)) {
			/* Load IDAC GMAP table */
			phy_ac_load_gmap_tbl(pi);
		}

		/* Bring chip in known good state before issuing dccal,
		 as this function is called in the middle on chansepc_set
		*/
		wlc_phy_resetcca_acphy(pi);

		/* Cal DC cal in channel change */
		phy_ac_dccal(pi);
	}
}

int
wlc_phy_femctrl_clb_prio_2g_acphy(phy_info_t *pi, bool set, uint32 val)
{
	int ret = 0;

	if (set) {
		femctrl_clb_majrev_ge40(pi, 1, val);
	} else {
		ret = phy_get_femctrl_clb_prio_2g_acphy(pi);
	}
	return ret;
}

int
wlc_phy_femctrl_clb_prio_5g_acphy(phy_info_t *pi, bool set, uint32 val)
{
	int ret = 0;

	if (set) {
		femctrl_clb_majrev_ge40(pi, 0, val);
	} else {
		ret = phy_get_femctrl_clb_prio_5g_acphy(pi);
	}

	return ret;
}

static void
phy_mimo_bf_settings(phy_info_t *pi, bool mimo_bf_enable)
{
	uint8 phy_coremask = phy_stf_get_data(pi->stfi)->phyrxchain;
	uint16 ExtrAfeClkDivO_set[2] = {0x10, 0x20};
	uint8 is5g;
	uint16 plllogen_reg_val0;
	uint16 plllogen_reg_val1;
	uint8 core;
	uint sicoreunit;

	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);
	is5g = CHSPEC_IS5G(pi->radio_chanspec);

	if (mimo_bf_enable) {
		FOREACH_CORE(pi, core) {
			if (PHY_COREMASK_SISO(phy_coremask)) {
				/* SISO */
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x1f);
				} else {
					WRITE_PHYREGCE(pi, pllLogenMaskCtrl, core, 0x3f);
				}
			} else {
				/* MIMO */
				if (core == 1) {
					if (CHSPEC_IS5G(pi->radio_chanspec)) {
						if (sicoreunit == DUALMAC_MAIN) {
							WRITE_PHYREGCE(pi, pllLogenMaskCtrl,
								core, 0x2fb);
						} else if (sicoreunit == DUALMAC_AUX) {
							WRITE_PHYREGCE(pi, pllLogenMaskCtrl,
								core, 0x2ff);
						}
					} else {
						WRITE_PHYREGCE(pi, pllLogenMaskCtrl,
							core, 0xff);
					}
				} else if (core == 0) {
					if (CHSPEC_IS5G(pi->radio_chanspec)) {
						WRITE_PHYREGCE(pi, pllLogenMaskCtrl,
							core, 0x77f);
					} else {
						WRITE_PHYREGCE(pi, pllLogenMaskCtrl,
							core, 0x1ff);
					}
				}
				WRITE_PHYREGCE(pi, Extra2AfeClkDivOverrideCtrl28nm,
					core, ExtrAfeClkDivO_set[is5g]);
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			plllogen_reg_val0 = 0x1ff;
			if (sicoreunit == DUALMAC_MAIN) {
				plllogen_reg_val1 = 0xff;
			} else {
				plllogen_reg_val1 = 0x4f;
			}
		} else {
			plllogen_reg_val0 = 0x67f;
			plllogen_reg_val1 = 0x2fb;
		}
		WRITE_PHYREG(pi, pllLogenMaskCtrl0, plllogen_reg_val0);
		WRITE_PHYREG(pi, pllLogenMaskCtrl1, plllogen_reg_val1);

		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			WRITE_PHYREG(pi, Extra2AfeClkDivOverrideCtrl28nm0, 0x0);
			WRITE_PHYREG(pi, Extra2AfeClkDivOverrideCtrl28nm1, 0x0);
		} else if (RADIOMAJORREV(pi) >= 3) {
			WRITE_PHYREG(pi, Extra2AfeClkDivOverrideCtrl28nm0, (phy_coremask == 2) ?
			0x0 : 0x10);
			WRITE_PHYREG(pi, Extra2AfeClkDivOverrideCtrl28nm1, 0x0);
		}

		if (sicoreunit == DUALMAC_MAIN) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_PHYREG(pi, pllLogenMaskCtrl0,
					logen_mimo_pwrup_mask, (1 - (phy_coremask == 1)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					logen_pwrup_mask, (1 - (phy_coremask == 1)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					logen_mimo_pwrup_mask, (1 - (phy_coremask == 1)));
			} else {
				MOD_PHYREG(pi, pllLogenMaskCtrl0,
					div5g_mask, (1 - (phy_coremask == 2)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					div5g_mask, (1 - (phy_coremask == 1)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					logen_mimo_pwrup_mask, (1 - (phy_coremask == 1)));
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					logen_pwrup_mask, (1 - (phy_coremask == 1)));
			} else {
				MOD_PHYREG(pi, pllLogenMaskCtrl0,
					div5g_mask, (1 - (phy_coremask == 2)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					div5g_mask, (1 - (phy_coremask == 1)));
				MOD_PHYREG(pi, pllLogenMaskCtrl1,
					logen_mimo_pwrup_mask, (1 - (phy_coremask == 1)));
			}
		}
	}
}

static void
wlc_phy_low_pwr_logen_setting(phy_info_t *pi, bool mimo_bf_enable)
{
	uint16 div5g_mimo_bf_en = 0;
	uint16 addr_20_offset_lo[4] = {0x140, 0x150, 0x160, 0x480};
	uint16 addr_40_offset_hi[4] = {0x141, 0x151, 0x161, 0x481};
	uint16 addr_20_lo;
	uint16 addr_40_hi;
	uint16 val_20_lo;
	uint16 val_40_hi;
	uint sicoreunit;
	uint8 core;
	uint8 phy_coremask = phy_stf_get_data(pi->stfi)->phyrxchain;

	div5g_mimo_bf_en = mimo_bf_enable;
	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);

	FOREACH_CORE(pi, core) {
		addr_20_lo = addr_20_offset_lo[core];
		addr_40_hi = addr_40_offset_hi[core];

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr_20_lo,
			16, &val_20_lo);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr_40_hi,
			16, &val_40_hi);

		if (ACMAJORREV_40(pi->pubpi->phy_rev) &&
		    (RADIO20694_MAJORREV(pi->pubpi->radiorev) >= 3)) {
			val_20_lo = (val_20_lo & 0xfe7f) | (div5g_mimo_bf_en << 8) | (((core == 0)
			& ((phy_coremask == 1) | (phy_coremask == 3))) << 7);
			val_40_hi = (val_40_hi & 0xff3f) | (div5g_mimo_bf_en << 7) | (((core == 0)
			& ((phy_coremask == 1) | (phy_coremask == 3))) << 6);
		} else {
			val_20_lo = (val_20_lo & 0xfeff) | (div5g_mimo_bf_en << 8);
			val_40_hi = (val_40_hi & 0xff7f) | (div5g_mimo_bf_en << 7);

		}
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr_20_lo,
			16, &val_20_lo);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, addr_40_hi,
			16, &val_40_hi);
	}

	WRITE_PHYREG(pi, RfctrlOverrideNapPus1, 0x0);

	if ((sicoreunit == DUALMAC_MAIN) && !mimo_bf_enable) {
		MOD_PHYREG(pi, RfctrlOverrideNapPus0, div2g_pwrup, 1);
		MOD_PHYREG(pi, RfctrlCoreNapPus0, div2g_pwrup, 0);
		MOD_PHYREG(pi, RfctrlOverrideNapPus1, div2g_pwrup, 1);
		MOD_PHYREG(pi, RfctrlCoreNapPus1, div2g_pwrup, 0);
	} else {
		MOD_PHYREG(pi, RfctrlOverrideNapPus0, div2g_pwrup, 0);
		MOD_PHYREG(pi, RfctrlOverrideNapPus1, div2g_pwrup, 0);
	}
	phy_mimo_bf_settings(pi, mimo_bf_enable);

}

static void
femctrl_clb_majrev_ge40(phy_info_t *pi, int band_is_2g, int slice)
{

	uint8 core;
	int cur_val, mask;

	#define CLBMASK 0x3ff
	#define CLBCORE0_SHIFT 16
	#define CLBCORE1_SHIFT 20

	FOREACH_CORE(pi, core) {

		if (band_is_2g) {
			mask = pi->u.pi_acphy->sromi->nvram_femctrl_clb.map_2g[slice][core];
			phy_set_femctrl_clb_prio_2g_acphy(pi, slice);
		} else {
			mask = pi->u.pi_acphy->sromi->nvram_femctrl_clb.map_5g[slice][core];
			phy_set_femctrl_clb_prio_5g_acphy(pi, slice);
		}
		if (core == 0) {
			/* clb_swctrl_smask_coresel_ant0 */
			cur_val = si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09, 0, 0);
			cur_val &= (CLBMASK<<CLBCORE0_SHIFT);

			if (slice) {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					CLBMASK<<CLBCORE0_SHIFT, ~(mask<<CLBCORE0_SHIFT) & cur_val);
			} else {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_09,
					CLBMASK<<CLBCORE0_SHIFT, (mask<<CLBCORE0_SHIFT) | cur_val);
			}
		}

		if (core == 1) {
			/* clb_swctrl_smask_coresel_ant1 */
			cur_val = si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10, 0, 0);
			cur_val &= (CLBMASK<<CLBCORE1_SHIFT);

			if (slice) {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					CLBMASK<<CLBCORE1_SHIFT, ~(mask<<CLBCORE1_SHIFT) & cur_val);
			} else {
				si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10,
					CLBMASK<<CLBCORE1_SHIFT, (mask<<CLBCORE1_SHIFT) | cur_val);
			}
		}
	}

}

void
impbf_radio_ovrs_4347(phy_info_t *pi, bool ovr)
{
#ifndef WLIMPBF
	return;
#else
	uint8 core;

	if (CHSPEC_IS5G(pi->radio_chanspec) &&
	(wlapi_si_coreunit(pi->sh->physhim) == DUALMAC_MAIN)) {
		if (phy_stf_get_data(pi->stfi)->phyrxchain == 3 &&
				phy_stf_get_data(pi->stfi)->phytxchain == 3) {

			FOREACH_CORE(pi, core) {
				/* Overrides registers */
				MOD_RADIO_REG_20694(pi, RF, LOGEN_CFG1, core,
					ovr_div5g_txbuf_pu, ovr);
				MOD_RADIO_REG_20694(pi, RF, LOGEN_OVR2, core,
					ovr_div5g_bias_tx_pu, ovr);
				MOD_RADIO_REG_20694(pi, RF, LOGEN_OVR2, core,
					ovr_logen5g_bias_pu, ovr);
				MOD_RADIO_REG_20694(pi, RF, LOGEN_CFG1, core,
					ovr_logen5g_mimobuf_en, ovr);

				/* LOGEN Registers */
				MOD_RADIO_REG_20694(pi, RF, LOGEN5G_REG2, core,
					div5g_txbuf_pu, 1);
				MOD_RADIO_REG_20694(pi, RF, LOGEN5G_REG3, core,
					div5g_bias_tx_pu, 1);
				MOD_RADIO_REG_20694(pi, RF, LOGEN5G_REG8, core,
					logen5g_bias_pu, (core == 0) ? 1 : 0);
				MOD_RADIO_REG_20694(pi, RF, LOGEN5G_REG5, core,
					logen5g_mimobuf_en, (core == 0) ? 0 : 1);
			}
		} else {
			FOREACH_CORE(pi, core) {
				RADIO_REG_LIST_START
					MOD_RADIO_REG_20694_ENTRY(pi, RF, LOGEN_CFG1, core,
						ovr_div5g_txbuf_pu, 0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, LOGEN_OVR2, core,
						ovr_div5g_bias_tx_pu, 0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, LOGEN_OVR2, core,
						ovr_logen5g_bias_pu, 0)
					MOD_RADIO_REG_20694_ENTRY(pi, RF, LOGEN_CFG1, core,
						ovr_logen5g_mimobuf_en, 0)
				RADIO_REG_LIST_EXECUTE(pi, core);
			}
		}
	}
#endif /* imptxbf overrides */
}

static void
chanspec_setup_phy_ACMAJORREV_44_46(phy_info_t *pi)
{
	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	WRITE_PHYREG(pi, currentChannelSFO, CHSPEC_CHANNEL(pi->radio_chanspec));
}

static void
chanspec_setup_phy_ACMAJORREV_40(phy_info_t *pi)
{
	uint8 mac_suspend = 1;

	mac_suspend = (R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (mac_suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		impbf_radio_ovrs_4347(pi, 1);
	}
	if (mac_suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_bphymrc_hwconfig(phy_info_t *pi)
{
	if (ACPHY_bphymrcCtrl(pi->pubpi->phy_rev) != INVALID_ADDRESS) {
		if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			if (pi->sh->bphymrc_en && CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_PHYREG(pi, HPFBWovrdigictrl, bphyNoCoreRemap, 1);
				WRITE_PHYREG(pi, bphymrcCtrl, 0x242);
			} else {
				MOD_PHYREG(pi, HPFBWovrdigictrl, bphyNoCoreRemap, 0);
				WRITE_PHYREG(pi, bphymrcCtrl, 0x200);
			}
		} else {
			PHY_TRACE(("wl%d: %s: BPHY MRC settings are NOT updated\n",
				pi->sh->unit, __FUNCTION__));
		}
	}
}

static void
chanspec_setup_phy_ACMAJORREV_47_51(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		/* Initially switch LESI on, if globally enabled in pi_ac->rxgcrsi.
		 * NOTE: phy_ac_rxgcrs_lesi() will only enable LESI if the global enable
		 *       is on.
		 * After the initial setting, enabling/disabling may be controlled by desense.
		 */
		phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);

		/* enable ChanSmooth */
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}
	/* Reset the TxPwrCtrl HW during the setup */
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 1);
	OSL_DELAY(10);
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 0);

	WRITE_PHYREG(pi, currentChannelSFO, CHSPEC_CHANNEL(pi->radio_chanspec));
}

static void
chanspec_setup_phy_ACMAJORREV_37(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		/* enable ChanSmooth for 7271 */
		wlc_phy_smth(pi, pi_ac->chanmgri->acphy_enable_smth,
			pi_ac->chanmgri->acphy_smth_dump_mode);
	}
}

static void
chanspec_setup_phy_ACMAJORREV_36(phy_info_t *pi)
{
#ifdef PHYWAR_43012_HW43012_211_RF_SW_CTRL

	phy_ac_WAR_43012_rf_sw_ctrl_pinmux(pi);

#endif /* PHYWAR_43012_HW43012_211_RF_SW_CTRL */
}

static void
chanspec_setup_phy_ACMAJORREV_32(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (CCT_INIT(pi_ac))
		MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	/* Reset the TxPwrCtrl HW during the setup */
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 1);
	OSL_DELAY(10);
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 0);

	// Enable LESI for 4365
	phy_ac_rxgcrs_lesi(pi_ac->rxgcrsi, TRUE, 0);
}

static void
chanspec_setup_phy_ACMAJORREV_5(phy_info_t *pi)
{
	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);
}

static void
chanspec_setup_phy_ACMAJORREV_4(phy_info_t *pi)
{

	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	/* Reset the TxPwrCtrl HW during the setup */
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 1);
	OSL_DELAY(10);
	MOD_PHYREG(pi, TxPwrCtrlCmd, txpwrctrlReset, 0);
}

static void
chanspec_setup_phy_ACMAJORREV_3(phy_info_t *pi)
{
	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);
}

static void
chanspec_setup_phy_ACMAJORREV_2(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	if (CCT_INIT(pi_ac) && (ACMINORREV_1(pi) || ACMINORREV_3(pi)) && PHY_ILNA(pi)) {
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_06, CC_GCI_XTAL_BUFSTRG_NFC,
			(0x1 << 12));
	}
}

static void
chanspec_setup_phy_ACMAJORREV_1(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	if (CCT_INIT(pi_ac) && ACMINORREV_2(pi) && PHY_ILNA(pi)) {
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_06, CC_GCI_XTAL_BUFSTRG_NFC,
			(0x1 << 12));
	}
}

static void
chanspec_setup_phy_ACMAJORREV_0(phy_info_t *pi)
{
	/* store/clear the hirssi(shmem) info of previous channel */
	if (phy_ac_hirssi_shmem_read_clear(pi)) {
		/* Check for previous channel */
		phy_ac_hirssi_set_timer(pi);
	}
}

static int
phy_ac_chanmgr_get_chanspec_bandrange(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	if (SROMREV(pi->sh->sromrev) < 12) {
		return phy_ac_chanmgr_get_chan_freq_range(pi, chanspec, PRIMARY_FREQ_SEGMENT);
	} else {
		return phy_ac_chanmgr_get_chan_freq_range_srom12(pi, chanspec);
	}
}

static void
chanspec_setup_phy(phy_info_t *pi)
{
	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		chanspec_setup_phy_ACMAJORREV_40(pi);
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_44_46(pi);
	else if (ACMAJORREV_37(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_37(pi);
	else if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_47_51(pi);
	else if (ACMAJORREV_36(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_36(pi);
	else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_32(pi);
	else if (ACMAJORREV_5(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_5(pi);
	else if (ACMAJORREV_4(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_4(pi);
	else if (ACMAJORREV_3(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_3(pi);
	else if (ACMAJORREV_2(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_2(pi);
	else if (ACMAJORREV_1(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_1(pi);
	else if (ACMAJORREV_0(pi->pubpi->phy_rev))
		chanspec_setup_phy_ACMAJORREV_0(pi);
	else {
		PHY_ERROR(("wl%d %s: Invalid ACMAJORREV %d!\n",
			PI_INSTANCE(pi), __FUNCTION__, pi->pubpi->phy_rev));
		ASSERT(0);
	}
}

static void
chanspec_setup_cmn(phy_info_t *pi)
{
	uint8 max_rxchain;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	if (CCT_INIT(pi_ac)) {
		phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

		phy_ac_antdiv_chanspec(pi_ac->antdivi);
		wlc_phy_set_reg_on_reset_acphy(pi);
		wlc_phy_set_tbl_on_reset_acphy(pi);

		/* If any rx cores were disabled before phy_init,
		 * disable them again since phy_init enables all rx cores
		 * Also make RfseqCoreActv2059.EnTx = phytxchain except
		 * for cals where it is set to hw_phytxchain
		 */
		max_rxchain =  (1 << pi->pubpi->phy_corenum) - 1;
		if ((stf_shdata->phyrxchain != max_rxchain) ||
			(stf_shdata->hw_phytxchain != max_rxchain)) {
			wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi,
			    stf_shdata->phyrxchain,
				stf_shdata->phytxchain);
		}
	}

	/* Set up ED thresholds */
	wlc_phy_apply_default_edthresh_acphy(pi, pi->radio_chanspec);

	if (CCT_INIT(pi_ac) || CCT_BAND_CHG(pi_ac)) {
		wlc_phy_set_regtbl_on_band_change_acphy(pi);

#ifdef WL_ETMODE
		if (ET_ENAB(pi)) {
			phy_ac_et(pi);
		}
#endif /* WL_ETMODE */
	}

	if (CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac))
		wlc_phy_set_regtbl_on_bw_change_acphy(pi);

	chanspec_setup_regtbl_on_chan_change(pi);
	phy_ac_antdiv_regtbl_fc_from_nvram(pi_ac->antdivi);
	chanspec_prefcbs_init(pi);
#ifdef OCL
	if (PHY_OCL_ENAB(pi->sh->physhim)) {
		phy_ocl_disable_req_set((wlc_phy_t *)pi, OCL_DISABLED_CHANSWITCH,
		                            TRUE, WLC_OCL_REQ_CHANSWITCH);
	}
#endif /* OCL */

}

static void
chanspec_cleanup(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* Restore FIFO reset and Stalls */
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, pi_ac->chanmgri->FifoReset);

	if ((ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_51(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47(pi->pubpi->phy_rev)) {
		phy_ac_chanmgr_core2core_sync_setup(pi->u.pi_acphy->chanmgri, TRUE);
	}

	/* reset RX */
	wlc_phy_resetcca_acphy(pi);

	/* return from Deaf */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* clear Chspec Call Trace */
	CCT_CLR(pi_ac);

	/* Clear the chanest dump counter */
	pi->phy_chanest_dump_ctr = 0;
}

/* see chanspec_cleanup which restores some of the setup params */
static void
chanspec_setup(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		PHY_TRACE(("wl%d: %s chan = %d\n", pi->sh->unit, __FUNCTION__,
			CHSPEC_CHANNEL(pi->radio_chanspec)));
		PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	}

#ifdef WL11ULB
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (PHY_ULB_ENAB(pi)) {
			wlc_phy_ulb_mode(pi, PMU_ULB_BW_NONE);
		}
	}
#endif /* WL11ULB */

	/* 7271 does not follow this, see HWJUSTY-263 */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* Disable core2core sync */
		phy_ac_chanmgr_core2core_sync_setup(pi->u.pi_acphy->chanmgri, FALSE);
	}

	/* Hold FIFOs in reset before changing channels */
	pi_ac->chanmgri->FifoReset = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);

	/* update corenum and coremask state variables */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev))
		phy_ac_update_phycorestate(pi);

	/* BAND CHANGED ? */
	if (CCT_INIT(pi_ac) ||
		(pi_ac->chanmgri->data->curr_band2g != CHSPEC_IS2G(pi->radio_chanspec))) {

		if (!ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMAJORREV_33(pi->pubpi->phy_rev))
			chanspec_setup_hirssi_ucode_cap(pi);

		pi_ac->chanmgri->data->curr_band2g = CHSPEC_IS2G(pi->radio_chanspec);

		/* indicate band change to control flow */
		mboolset(pi_ac->CCTrace, CALLED_ON_BAND_CHG);
	}

	/* BW CHANGED ? */
	if (CCT_INIT(pi_ac) || (pi_ac->curr_bw != CHSPEC_BW(pi->radio_chanspec))) {
		pi_ac->curr_bw = CHSPEC_BW(pi->radio_chanspec);

		/* Set the phy BW as dictated by the chspec (also calls phy_reset) */
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(pi->radio_chanspec));

		/* bw change  do not need a phy_reset when BW_RESET == 1 */
		if (BW_RESET == 0) {
			/* indicate phy reset, follow init path to control flow */
			mboolset(pi_ac->CCTrace, CALLED_ON_INIT);
		} else {
			chanspec_sparereg_war(pi);
			/* 43684 and 63178 use phy_reset instead of bw_reset */
			if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				mboolset(pi_ac->CCTrace, CALLED_ON_INIT);
			}
		}

		OSL_DELAY(2);

		/* indicate bw change to control flow */
		mboolset(pi_ac->CCTrace, CALLED_ON_BW_CHG);
		if (PHY_AS_80P80(pi, pi_ac->curr_bw) ||
			PHY_AS_80P80(pi, pi->radio_chanspec)) {
			mboolset(pi_ac->CCTrace, CALLED_ON_BW_CHG_80P80);
		}
	}

	/* Some PHY have issue with accessing wide tables */
	chanspec_phy_table_access_war(pi);

	/* Change the band bit. Do this after phy_reset */
	if (CHSPEC_IS2G(pi->radio_chanspec))
		MOD_PHYREG(pi, ChannelControl, currentBand, 0);
	else
		MOD_PHYREG(pi, ChannelControl, currentBand, 1);
}

static void
chanspec_tune_phy(phy_info_t *pi)
{
	if (ACMAJORREV_5(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_5(pi);
	else if (ACMAJORREV_4(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_4(pi);
	else if (ACMAJORREV_3(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_3(pi);
	else if (ACMAJORREV_2(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_2(pi);
	else if (ACMAJORREV_1(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_1(pi);
	else if (ACMAJORREV_0(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_0(pi);
	else if (ACMAJORREV_40(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_40(pi);
	else if (ACMAJORREV_44_46(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_44_46(pi);
	else if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_47_51(pi);
	else if (ACMAJORREV_37(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_37(pi);
	else if (ACMAJORREV_36(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_36(pi);
	else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev))
		chanspec_tune_phy_ACMAJORREV_32(pi);
	else {
		PHY_ERROR(("wl%d %s: Invalid ACMAJORREV!\n", PI_INSTANCE(pi), __FUNCTION__));
		ASSERT(0);
	}
}

/* ******************** WARs ********************* */

static void
chanspec_phy_table_access_war(phy_info_t *pi)
{
	/* PHY table read reliability WAR.
	 * 4365:
	 * Latest DVT JIRAS: SWWLAN-154664/SWWLAN-154666/SWWLAN-154668/SWWLAN-154685
	 * exposed that '15' is a problematic setting, and further confirmed by RTL
	 * group through chipsim simulations. The newest setting '10' is obtained
	 * through experiments, i.e.
	 *  [15:12]  does not work
	 *  [11:1]   works
	 *  [0]      hang
	 *
	 * 43684A0 and 43684A1:
	 * Table read access for gainCtrlbbMultLuts0 may fail due to clock gating issue.
	 * With following values no read issues are observed.
	 *   20MHz: [8:11]; 40MHz: [5..9]; 80MHz: [3..7]; 160MHz: [3..7]
	 * Reset value of this register is 7.
	 *
	 * 43684B0: issue fixed in RTL
	 */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev) ||
			(ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_0(pi))) {
		MOD_PHYREG(pi, TableAccessCnt, TableAccessCnt_ClkIdle,
				CHSPEC_ISLE20(pi->radio_chanspec) ? 10 : 7);
	}
}

static void
chanspec_setup_hirssi_ucode_cap(phy_info_t *pi)
{
	BCM_REFERENCE(pi);
}

static void
chanspec_sparereg_war(phy_info_t *pi)
{
	if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID &&
		CHSPEC_IS80(pi->radio_chanspec)) {

		WRITE_PHYREG(pi, SpareReg, 0xfe);
		wlc_phy_resetcca_acphy(pi);
		WRITE_PHYREG(pi, SpareReg, 0xff);

	}
}

static bool
chanspec_papr_enable(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	bool enable = FALSE;
	int freq;
	uint8 ch = CHSPEC_CHANNEL(pi->radio_chanspec);

	if (!pi_ac->chanmgri->cfg->srom_paprdis) {
		if (PHY_IPA(pi) && (ACMAJORREV_1(pi->pubpi->phy_rev) ||
			(ACMAJORREV_2(pi->pubpi->phy_rev) &&
			(ACMINORREV_1(pi) || ACMINORREV_3(pi))))) {
			enable = TRUE;
		} else if (!PHY_IPA(pi) && ACMAJORREV_2(pi->pubpi->phy_rev) && (ACMINORREV_1(pi))) {
			const void *chan_info = NULL;
			freq = wlc_phy_chan2freq_acphy(pi, ch, &chan_info);
			if (freq == 2472) {
				enable = FALSE;
			} else {
				enable = TRUE;
			}
		} else if (!PHY_IPA(pi) && ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			enable = TRUE;
		}
	}
	if ((ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) &&
		CHSPEC_IS5G(pi->radio_chanspec))
		enable = TRUE;

	return enable;
}

static void
chanspec_tune_rxpath(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
		!ACMAJORREV_33(pi->pubpi->phy_rev) &&
		!ACMAJORREV_37(pi->pubpi->phy_rev) &&
		!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* DSSF for 4335C0 & 4345 */
		phy_ac_dssf(pi_ac->rxspuri, TRUE);
	}

	phy_ac_rssi_init_gain_err(pi_ac->rssii);

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* 	To keep ED run throughout packet
			This has nothing to do with DSSF
		*/
		MOD_PHYREG(pi, DSSF_C_CTRL, disableCRSCorr, 0);
	}
}

static void
chanspec_tune_txpath(phy_info_t *pi)
{
	uint8 tx_pwr_ctrl_state = PHY_TPC_HW_OFF;
	int freq = 0;
	uint8 ch = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 chans[NUM_CHANS_IN_CHAN_BONDING];
	uint8 core;
	uint32	fc = wf_channel2mhz(CHSPEC_CHANNEL(pi->radio_chanspec),
	                            CHSPEC_IS2G(pi->radio_chanspec) ? WF_CHAN_FACTOR_2_4_G
	                                                               : WF_CHAN_FACTOR_5_G);

	if (RADIOID_IS(pi->pubpi->radioid, BCM2069_ID)) {
		const void *chan_info = NULL;
		freq = wlc_phy_chan2freq_acphy(pi, ch, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		const chan_info_radio20693_pll_t *chan_info_pll;
		const chan_info_radio20693_rffe_t *chan_info_rffe;
		const chan_info_radio20693_pll_wave2_t *pll_tbl_wave2;

		if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS160(pi->radio_chanspec) || CHSPEC_IS8080(pi->radio_chanspec)) {
				wf_chspec_get_80p80_channels(pi->radio_chanspec, chans);
				PHY_INFORM(("wl%d: %s: BW 160MHz chan1 = %d, chan2 = %d\n",
					pi->sh->unit, __FUNCTION__, chans[0], chans[1]));
			} else {
				chans[0] = CHSPEC_CHANNEL(pi->radio_chanspec);
				chans[1] = 0;
				PHY_TRACE(("wl%d: %s chan = %d\n", pi->sh->unit, __FUNCTION__,
					chans[0]));
			}
			freq = wlc_phy_chan2freq_20693(pi, chans[0], &chan_info_pll,
				&chan_info_rffe, &pll_tbl_wave2);
		} else {
			freq = wlc_phy_chan2freq_20693(pi, ch, &chan_info_pll, &chan_info_rffe,
					&pll_tbl_wave2);
		}
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20694_ID)) {
		const chan_info_radio20694_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20694(pi, ch, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20696_ID)) {
		const chan_info_radio20696_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20696(pi, ch, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20697_ID)) {
		chan_info_radio20697_rffe_t chan_info;
		freq = wlc_phy_chan2freq_20697(pi, ch, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20698_ID)) {
		const chan_info_radio20698_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20698(pi, ch, &chan_info);
	} else if (RADIOID_IS(pi->pubpi->radioid, BCM20704_ID)) {
		const chan_info_radio20704_rffe_t *chan_info;
		freq = wlc_phy_chan2freq_20704(pi, ch, &chan_info);
	} else {
		const chan_info_radio20691_t *chan_info_20691;

		freq = wlc_phy_chan2freq_20691(pi, ch, &chan_info_20691);
	}
#if defined(WLC_TXCAL)
	phy_ac_tssical_set_olpc_threshold(pi);
#endif	/* WLC_TXCAL */

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev))
		MOD_PHYREG(pi, BT_SwControl, inv_btcx_prisel, 0x1);

	/* set txgain in case txpwrctrl is disabled */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
	    uint8 phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
		BCM_REFERENCE(phyrxchain);
		FOREACH_ACTV_CORE(pi, phyrxchain, core) {
			pi->u.pi_acphy->txpwrindex[core] = (fc < 5500)? 48: 64;
		}
	}
	wlc_phy_txpwr_fixpower_acphy(pi);

	/* Disable TxPwrCtrl */
	tx_pwr_ctrl_state = pi->txpwrctrl;
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);

	/* Set the TSSI visibility limits for 4360 A0/B0 */

	/* Temporary fix for UTF failure for X51 SWWLAN-93602
	 * Always set TSSI visibility threshold
	 */
	wlc_phy_set_tssisens_lim_acphy(pi, TRUE);

	/* Enable TxPwrCtrl */
	if (!((ACMAJORREV_32(pi->pubpi->phy_rev) ||
	       ACMAJORREV_33(pi->pubpi->phy_rev) ||
	       ACMAJORREV_37(pi->pubpi->phy_rev) ||
	       ACMAJORREV_47_51(pi->pubpi->phy_rev)) &&
	      ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_TXPWRCTRL_EN) == 0)))
		wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);

#ifdef WLC_TXCAL
	phy_tssical_compute_olpc_idx(pi->tssicali);
#endif /* WLC_TXCAL */
	chanspec_setup_papr(pi, 0, 0);
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (!PHY_IPA(pi)) &&
	    (pi->u.pi_acphy->chanmgri->cfg->srom_txnoBW80ClkSwitch == 0)) {
		wlc_phy_afeclkswitch_sifs_delay(pi);
		if (!(freq == 5210 || freq == 5290) && (CHSPEC_IS80(pi->radio_chanspec)))
			wlc_phy_modify_txafediv_acphy(pi, 9);
		else if (CHSPEC_IS80(pi->radio_chanspec))
			wlc_phy_modify_txafediv_acphy(pi, 6);
	}
}

static void
chanspec_prefcbs_init(phy_info_t *pi)
{
#ifdef ENABLE_FCBS
	int chanidx, chanidx_current;
	chanidx = 0;
	chanidx_current = 0;

	if (IS_FCBS(pi)) {

		chanidx_current = wlc_phy_channelindicator_obtain(pi);

		for (chanidx = 0; chanidx < MAX_FCBS_CHANS; chanidx++) {
			if ((chanidx != chanidx_current) &&
			(!(pi->phy_fcbs.initialized[chanidx]))) {

				wlc_phy_prefcbsinit_acphy(pi, chanidx);

				if (CCT_INIT(pi_ac)) {
					wlc_phy_set_reg_on_reset_acphy(pi);
					wlc_phy_set_tbl_on_reset_acphy(pi);
				}

				if (CCT_BAND_CHG(pi_ac))
					wlc_phy_set_regtbl_on_band_change_acphy(pi);

				if (CCT_BW_CHG(pi_ac))
					wlc_phy_set_regtbl_on_bw_change_acphy(pi);
			}
		}

		wlc_phy_prefcbsinit_acphy(pi, chanidx_current);
	}
#endif /* ENABLE_FCBS */
}

/* features and WARs enable */
static void
chanspec_fw_enab(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	/* min_res_mask = 0, max_res_mask = 0, clk_ctl_st = 0 */
	uint32 bbpll_parr_in[3] = {0, 0, 0};

	if (D11REV_IS(pi->sh->corerev, 48)) {
		wlapi_bmac_write_shm(pi->sh->physhim, M_PAPDOFF_MCS(pi),
		                     pi_ac->chanmgri->cfg->srom_papdwar);
	}

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* Toggle */
		chanspec_bbpll_parr(pi_ac->rxspuri, bbpll_parr_in, OFF);
		chanspec_bbpll_parr(pi_ac->rxspuri, bbpll_parr_in, ON);
	}

#if (defined(WLOLPC) && !defined(WLOLPC_DISABLED)) || defined(BCMDBG) || \
	defined(WLTEST)
	chanspec_clr_olpc_dbg_mode(pi_ac->tpci);
#endif /* ((WLOLPC) && !(WLOLPC_DISABLED)) || (BCMDBG) || (WLTEST) */

	/* Enable antenna diversity */
	if (wlc_phy_check_antdiv_enable_acphy(pi) &&
		(CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac)) &&
		pi->sh->rx_antdiv) {
		wlc_phy_antdiv_acphy(pi, pi->sh->rx_antdiv);
	}

#ifdef WLC_SW_DIVERSITY
	if (PHYSWDIV_ENAB(pi))
		wlc_phy_swdiv_antmap_init(pi->u.pi_acphy->antdivi);
#endif // endif

#ifdef WL11ULB
	if (PHY_ULB_ENAB(pi)) {
		wlc_phy_ulb_mode(pi, CHSPEC_IS10(pi->radio_chanspec) ? PMU_ULB_BW_10MHZ :
				CHSPEC_IS5(pi->radio_chanspec) ? PMU_ULB_BW_5MHZ
				: CHSPEC_IS2P5(pi->radio_chanspec) ? PMU_ULB_BW_2P5MHZ
				: PMU_ULB_BW_NONE);
	}
#endif /* WL11ULB */

	/* 4347 QT BBMULT settings */
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev) && ISSIM_ENAB(pi->sh->sih)) {
		uint8 ncore_idx;
		uint16 val = 50;

		FOREACH_CORE(pi, ncore_idx) {
			wlc_phy_set_tx_bbmult_acphy(pi, &val, ncore_idx);
		}
	}

#ifdef WL_DSI
	/* Update FCBS dynamic sequence
	   This is for DS0
	*/
	ds0_radio_seq_update(pi);
#endif /* WL_DSI */
#ifdef OCL
	if (PHY_OCL_ENAB(pi->sh->physhim)) {
		if ((CCT_INIT(pi_ac) || CCT_BW_CHG(pi_ac) || CCT_BAND_CHG(pi_ac))) {
				phy_ac_ocl_config(pi_ac->ocli);
		}
		phy_ocl_disable_req_set((wlc_phy_t *)pi, OCL_DISABLED_CHANSWITCH,
				FALSE, WLC_OCL_REQ_CHANSWITCH);
	}
#endif /* OCL */

#ifdef WL_NAP
		/* Enable or disable Napping feature for 4347
		 * This needs to be after OCL enable as the OCL
		 * info is used in the napping configuration.
	    */
		if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
			if (PHY_NAP_ENAB(pi->sh->physhim)) {
				phy_ac_config_napping_28nm(pi);
			}
		}
#endif /* WL_NAP */
}

void
wlc_phy_apply_default_edthresh_acphy(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	int32 edthresh_val = 0;

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		PHY_INFORM(("FIXME: 4347 Edthresh is bypassed for the moment\n"));
		return;
	}

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* just in case for chanspec specific control */
		UNUSED_PARAMETER(chanspec);
	}

	if (region_group ==  REGION_EU) {
		edthresh_val = CHSPEC_IS2G(pi->radio_chanspec) ?
			pi->srom_eu_edthresh2g : pi->srom_eu_edthresh5g;
	} else {
		edthresh_val = CHSPEC_IS2G(pi->radio_chanspec) ?
			pi_ac->sromi->ed_thresh2g : pi_ac->sromi->ed_thresh5g;
	}

	if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
	    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (CCT_BAND_CHG(pi_ac)) {
			if (!edthresh_val || edthresh_val > MAX_VALID_EDTHRESH) {
				wlc_phy_adjust_ed_thres(pi, &pi_ac->sromi->ed_thresh_default, TRUE);
			} else {
				wlc_phy_adjust_ed_thres(pi, &edthresh_val, TRUE);
			}
		}
	} else {
		if (!edthresh_val || edthresh_val > MAX_VALID_EDTHRESH) {
			wlc_phy_adjust_ed_thres(pi, &pi_ac->sromi->ed_thresh_default, TRUE);
		} else {
			wlc_phy_adjust_ed_thres(pi, &edthresh_val, TRUE);
		}
	}
}

void
wlc_phy_preemption_abort_during_timing_search(phy_info_t *pi, bool enable)
{
		if (enable) {
				ACPHYREG_BCAST(pi, PktAbortSupportedStates, 0x2fff);
		} else {
				ACPHYREG_BCAST(pi, PktAbortSupportedStates, 0x2ffe);
		}
}

void
wlc_phy_chanspec_set_acphy(phy_info_t *pi, chanspec_t chanspec)
{
	chanspec_module_t *module = get_chanspec_module_list();

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);
	/* sync pi->radio_chanspec with incoming chanspec */
	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);

	/* CHANSPEC DISPATCH */
	do {
		(*module)(pi);
		++module;
	} while (*module != NULL);

	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
	/* Note: !!! DO NOT ADD ANYTHING HERE !!!
	 * All changes to acphy chanspec should go to respective module list above
	 */
}

static void
phy_ac_chanmgr_chanspec_set(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlc_phy_chanspec_set_acphy(pi, chanspec);

	/* reset the phynoise_state (/ac/misc/phy_ac_misc) */
	/* RB - http://wlan-rb.sj.broadcom.com/r/105244/ */
	pi->phynoise_state = 0;
}

static void
phy_ac_chanmgr_upd_interf_mode(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	phy_ac_chanmgr_info_t *info = (phy_ac_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	if (pi->sh->interference_mode_override == TRUE) {
		pi->sh->interference_mode = CHSPEC_IS2G(chanspec) ?
		        pi->sh->interference_mode_2G_override :
		        pi->sh->interference_mode_5G_override;
	} else {
		pi->sh->interference_mode = CHSPEC_IS2G(chanspec) ?
		        pi->sh->interference_mode_2G :
		        pi->sh->interference_mode_5G;
	}
}

void
wlc_phy_ulb_feature_flag_set(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_feature_flags_t *phyflag = pi->ff;
	phyflag ->_ulb = TRUE;
}

#ifdef PHYWAR_43012_HW43012_211_RF_SW_CTRL
static void
phy_ac_WAR_43012_rf_sw_ctrl_pinmux(phy_info_t *pi)
{

	/*
	TCL Proc : WAR_43012_rf_sw_ctrl_pinmux
	*/
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardtype) ==
			BCM943012WLREF_SSID) {
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10, 0xFFFFFFFF,
				0xFFFFE000);
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_11, 0x7F,
				0x7F);
	} else {

		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10, (0x1FFF << 13),
				(0x36 << 13));
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_10, (0x3F << 26),
				(0x36 << 26));
	}
	si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_02, (0xF << 24),
			(0x3 << 24));

	si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_02, (0xF << 28),
			(0x3 << 28));

	si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_03, 0xF,
			3);

	si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_03, (0xF << 4),
			(0x3 << 4));

	WRITE_PHYREG(pi, gpioLoOutEn, 0xFFFF);
	WRITE_PHYREG(pi, gpioHiOutEn, 0x0);
	WRITE_PHYREG(pi, gpioSel, 0x3E);

	/* si_gpiocontrol(si_t *sih, uint32 mask, uint32 val, uint8 priority) */
	si_gpiocontrol(pi->sh->sih, 0xFFFF, 0xFFFF, GPIO_DRV_PRIORITY);
}
#endif /* PHYWAR_43012_HW43012_211_RF_SW_CTRL */

void
wlc_phy_modify_txafediv_acphy(phy_info_t *pi, uint16 a)
{
	uint16 rfseqExtReg_bw40 = 0x5ea;
	uint16 rfseqExtReg_bw80 = 0x7f8;
	uint8	ch = CHSPEC_CHANNEL(pi->radio_chanspec), afe_clk_num = 3, afe_clk_den = 2;
	uint16	b = 640, lb_b = 320;
	uint32	fcw, lb_fcw, tmp_low = 0, tmp_high = 0;
	uint32  deltaphase;
	uint16  deltaphase_lo, deltaphase_hi;
	uint16  farrow_downsamp;
	uint32	fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ? WF_CHAN_FACTOR_2_4_G
				    : WF_CHAN_FACTOR_5_G);
	uint32 tx_afediv_sel;
	uint32 write_val[2];
	uint8 stall_val, orig_rxfectrl1;

	if ((pi->sh->chippkg == 2) && (fc == 5290))
		return;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
		1 << 23, (fc * afe_clk_den) >> 1);
	math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);
	wlc_phy_tx_farrow_mu_setup(pi, fcw & 0xffff, (fcw & 0xff0000) >> 16, fcw & 0xffff,
		(fcw & 0xff0000) >> 16);
	math_uint64_multiple_add(&tmp_high, &tmp_low, fc * afe_clk_den,
		1 << 25, 0);
	math_uint64_divide(&lb_fcw, tmp_high, tmp_low, a * afe_clk_num * lb_b);
	deltaphase = (lb_fcw - 33554431) >> 1;
	deltaphase_lo = deltaphase & 0xffff;
	deltaphase_hi = (deltaphase >> 16) & 0xff;
	farrow_downsamp = fc * afe_clk_den / (a * afe_clk_num * lb_b);
	WRITE_PHYREG(pi, lbFarrowDeltaPhase_lo, deltaphase_lo);
	WRITE_PHYREG(pi, lbFarrowDeltaPhase_hi, deltaphase_hi);
	if (a == 9) {
		WRITE_PHYREG(pi, lbFarrowDriftPeriod, 4320);
	} else {
		WRITE_PHYREG(pi, lbFarrowDriftPeriod, 2880);
	}
	MOD_PHYREG(pi, lbFarrowCtrl, lb_farrow_outShift, 0);
	MOD_PHYREG(pi, lbFarrowCtrl, lb_decimator_output_shift, 0);
	MOD_PHYREG(pi, lbFarrowCtrl, lb_farrow_outScale, 1);
	MOD_PHYREG(pi, lbFarrowCtrl, lb_farrow_downsampfactor, farrow_downsamp);
	if (RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2 &&
	    !(ISSIM_ENAB(pi->sh->sih))) {

		/* Disable stalls and hold FIFOs in reset */
		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);

		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);

		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
		if (a == 9) {
			tx_afediv_sel = (rfseqExtReg_bw40 & ~(0x7 << 14) & 0xfffff) |
				(0x2 << 14);
		} else {
			tx_afediv_sel = (rfseqExtReg_bw80 & ~(0x7 << 14) & 0xfffff) |
				(0x0 << 14);
		}

		write_val[0] = ((tx_afediv_sel & 0xfff) << 20) | tx_afediv_sel;
		write_val[1] = (tx_afediv_sel << 8) | (tx_afediv_sel >> 12);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1, 2, 60,
			write_val);
		wlc_phy_force_rfseq_noLoleakage_acphy(pi);

		/* Restore FIFO reset and Stalls */
		ACPHY_ENABLE_STALL(pi, stall_val);
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_afeclkswitch_sifs_delay(phy_info_t *pi)
{
	uint16 sifs_rx_tx_tx = 0x3e3e, sifs_nav_tx = 0x23e;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	if (CHSPEC_IS80(pi->radio_chanspec)) {
		uint16 epaPwrupDly;

		W_REG(pi->sh->osh, D11_IFS_SIFS_RX_TX_TX(pi),
		      sifs_rx_tx_tx - ((TXDELAY_BW80 *8)<<8 | (TXDELAY_BW80 *8)));
		W_REG(pi->sh->osh, D11_IFS_SIFS_NAV_TX(pi),
		      sifs_nav_tx - (TXDELAY_BW80 *8));
		WRITE_PHYREG(pi, TxRealFrameDelay, 186 + TXDELAY_BW80 * 80);
		WRITE_PHYREG(pi, TxMacIfHoldOff, TXMAC_IFHOLDOFF_DEFAULT + TXDELAY_BW80 * 2);
		WRITE_PHYREG(pi, TxMacDelay, TXMAC_MACDELAY_DEFAULT + TXDELAY_BW80 * 80);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
		      16, rfseq_rx2tx_cmd_afeclkswitch);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
		      16, rfseq_rx2tx_cmd_afeclkswitch_dly);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x79, 16, &epaPwrupDly);
		epaPwrupDly = epaPwrupDly + TXDELAY_BW80 * 1000/25;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x79,
		      16, &epaPwrupDly);
	} else {
		W_REG(pi->sh->osh, D11_IFS_SIFS_RX_TX_TX(pi), sifs_rx_tx_tx);
		W_REG(pi->sh->osh, D11_IFS_SIFS_NAV_TX(pi), sifs_nav_tx);
		WRITE_PHYREG(pi, TxRealFrameDelay, 186);
		WRITE_PHYREG(pi, TxMacIfHoldOff, TXMAC_IFHOLDOFF_DEFAULT);
		WRITE_PHYREG(pi, TxMacDelay, TXMAC_MACDELAY_DEFAULT);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x00,
		      16, rfseq_rx2tx_cmd_noafeclkswitch);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 16, 0x70,
		      16, rfseq_rx2tx_cmd_noafeclkswitch_dly);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}
static void
wlc_phy_ac_lpf_cfg(phy_info_t *pi)
{
	uint16 data1, data2, offset;
	uint8 core;

	FOREACH_CORE(pi, core) {
		/* modify cRB_lpf_ctl_LUT_en_tx_offset to turn off LPF in tx */
		offset = 0x14d + core * 0x10;
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, offset, 16, &data1);
		/* 0x2000 is for enaling dac_rc switch for ofdm */
		data2 = (data1 | 0x2000) & 0x29FE;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, offset, 16, &data2);
		/* modify cRB_lpf_sw_en_tx_offset to bypass LPF in tx */
		offset = 0x36b + core * 0x10;
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, offset, 16, &data1);
		/* 0x40 is for enaling dac_rc switch for cck */
		data2 = (data1 | 0x40) & 0xFE53;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, offset, 16, &data2);
#ifdef REGULATORY_DEBUG
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			/* In 20MHz mode, force notch filter to be 40MHz mode in dac_clk_x2 */
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				MOD_RADIO_REG_20694(pi, RF, LPF_GMULT_RC_BW, core,
					lpf_gmult_rc_bw, 0x1);
			}
		}
#endif /* REGULATORY_DEBUG */
	}
}
static void
phy_ac_lp_enable(phy_info_t *pi)
{
	/* Enable memory standby based on lpflags */
	if ((pi->sromi->lpflags & LPFLAGS_PHY_GLOBAL_DISABLE) ||
		(pi->sromi->lpflags & LPFLAGS_PHY_LP_DISABLE)) {
		PHY_TRACE(("%s: phy lp disable\n", __FUNCTION__));
		goto exit;
	}

	if (!(pi->sromi->lpflags & LPFLAGS_PSM_PHY_CTL)) {
		AND_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x7fff);
	}

	PHY_TRACE(("%s: phy lp enable\n", __FUNCTION__));

	/* Disable xtal clocks */
	MOD_RADIO_REG_20695_MULTI_3(pi, RFP, XTAL6, 0, xtal_pu_btfmdig, 0x0,
			xtal_pu_caldrv, 0x0, xtal_pu_OFFCHIP, 0x0);
	MOD_RADIO_REG_20695_MULTI_2(pi, RFP, XTAL5, 0, xtal_pu_serdes, 0,
			xtal_pu_RCCAL, 0);
	MOD_RADIO_REG_20695_MULTI_2(pi, RFP, XTAL8, 0, xtal_pu_pfddrv_auxcore, 0,
			xtal_pu_caldrv_auxcore, 0);
	MOD_RADIO_REG_20695(pi, RFP, XTAL4, 0, xtal_pu_BT, 0);

exit:
	return;
}

uint8 *
BCMRAMFN(get_avvmid_set_36)(phy_info_t *pi, uint16 pdet_range_id, uint16 subband_id)
{
	BCM_REFERENCE(pi);
	return &avvmid_set_maj36[pdet_range_id][subband_id][0];
}

uint8 *
BCMRAMFN(get_avvmid_set_40)(phy_info_t *pi, uint16 pdet_range_id, uint16 subband_id)
{
	BCM_REFERENCE(pi);
	return &avvmid_set_maj40[pdet_range_id][subband_id][0];
}

void
wlc_phy_tx_farrow_mu_setup(phy_info_t *pi, uint16 MuDelta_l, uint16 MuDelta_u, uint16 MuDeltaInit_l,
	uint16 MuDeltaInit_u)
{
	ACPHYREG_BCAST(pi, TxResamplerMuDelta0l, MuDelta_l);
	ACPHYREG_BCAST(pi, TxResamplerMuDelta0u, MuDelta_u);
	ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0l, MuDeltaInit_l);
	ACPHYREG_BCAST(pi, TxResamplerMuDeltaInit0u, MuDeltaInit_u);
}

static void
phy_ac_chanmgr_write_tx_farrow_tiny(phy_info_t *pi, chanspec_t chanspec,
	chanspec_t chanspec_sc, int sc_mode)
{
	uint8	ch, afe_clk_num, afe_clk_den, core;
	uint16	a, b;
	uint32	fcw, tmp_low = 0, tmp_high = 0;
	uint32	fc;
	chanspec_t chanspec_sel;
	phy_ac_chanmgr_info_t *chanmgri = pi->u.pi_acphy->chanmgri;
	bool vco_12GHz_in5G = (chanmgri->vco_12GHz &&
		CHSPEC_IS5G(pi->radio_chanspec));
	bool  suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	uint8 restore_pa5g_pu[PHY_CORE_MAX];
	uint8 restore_over_pa5g_pu[PHY_CORE_MAX];
	uint8 restore_ext_5g_papu[PHY_CORE_MAX];
	uint8 restore_override_ext_pa[PHY_CORE_MAX];

	if (sc_mode == 1) {
		chanspec_sel = chanspec_sc;
	} else {
		chanspec_sel = chanspec;
	}

	fc = wf_channel2mhz(CHSPEC_CHANNEL(chanspec_sel), CHSPEC_IS2G(chanspec_sel) ?
	                    WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);

	if ((!suspend) && (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev)))
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1) {
		if (CHSPEC_IS20(chanspec_sel)) {
			if ((phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->vcodivmode & 0x1) ||
				vco_12GHz_in5G) {
				a = 16;
			} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
				if (fc <= 5320 && fc >= 5180) {
					/* dac_div_ratio = 12 */
					a = 12;
#ifdef PHY_CORE2CORESYC //WAR: core2core requires to fix dac_div_ratio
				} else if (fc <= 5825 && fc >= 5745) {
					/* dac_div_ratio = 15 */
					a = 15;
#endif // endif
				} else {
					/* dac_div_ratio = 16 */
					a = 16;
				}
			} else
				a = 18;
			b = 160;
		} else if (CHSPEC_IS40(chanspec_sel)) {
			if ((phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->vcodivmode & 0x2) ||
				vco_12GHz_in5G) {
				a = 8;
			} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
#ifdef PHY_CORE2CORESYC
				if ((fc == 5190 || fc == 5230)) {
					/* dac_div_ratio = 9 */
					a = 9;
				} else
#endif // endif
				{
					/* dac_div_ratio = 8 */
					a = 8;
				}
			} else
				a = 9;
			b = 320;
		} else if (CHSPEC_IS80(chanspec_sel) ||
				PHY_AS_80P80(pi, pi->radio_chanspec)) {
			a = 6;
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
#ifdef PHY_CORE2CORESYC
				if (fc == 5210) {
					/* dac_div_ratio = 7 */
					a = 7;
				} else
#endif // endif
				{
					/* dac_div_ratio = 6 */
					a = 6;
				}
			}
			b = 640;
		} else if (CHSPEC_IS160(chanspec_sel)) {
			a = 6;
			b = 640;
			ASSERT(0);
		} else {
			a = 6;
			b = 640;
		}

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev)) {
			if (sc_mode == 1) {
				MOD_RADIO_REG_20693(pi, CLK_DIV_OVR1, 3, ovr_afe_div_dac, 1);
				MOD_RADIO_REG_20693(pi, CLK_DIV_CFG1, 3, sel_dac_div, a);
			} else {
				MOD_RADIO_ALLREG_20693(pi, CLK_DIV_OVR1, ovr_afe_div_dac, 1);
				MOD_RADIO_ALLREG_20693(pi, CLK_DIV_CFG1, sel_dac_div, a);
			}
		}
	} else if (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 2) {
		a = 6;
		b = 640;
	} else {
		if (CHSPEC_IS80(chanspec_sel) || (CHSPEC_IS8080(chanspec_sel) &&
			!ACMAJORREV_33(pi->pubpi->phy_rev))) {
			a = 6;
			b = 640;
		} else {
			a = 8;
			b = 320;
		}
	}
	if (CHSPEC_IS2G(chanspec_sel)) {
		afe_clk_num = 2;
		afe_clk_den = 3;
	} else {
		afe_clk_num = 3;
		afe_clk_den = 2;
#if !defined(MACOSX)
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID) &&
				(CHSPEC_IS20(pi->radio_chanspec) ||
				CHSPEC_IS40(pi->radio_chanspec)) &&
				!PHY_IPA(pi) && !ROUTER_4349(pi)) {
				afe_clk_den = 3;
			}
		}
#endif /* MACOSX */
	}
	if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
		const uint8 afeclkdiv_arr[] = {2, 16, 4, 8, 3, 24, 6, 12};
		const uint8 dacclkdiv_arr[] = {6, 8, 9, 16, 18, 32, 64, 10};
		const uint8 dacdiv_arr[] = {1, 2, 3, 4};
		const chan_info_radio20693_altclkplan_t *altclkpln = altclkpln_radio20693;
		int row;
		if (ROUTER_4349(pi)) {
			altclkpln = altclkpln_radio20693_router4349;
		}
		row = wlc_phy_radio20693_altclkpln_get_chan_row(pi);

		if ((row >= 0) && (pi->u.pi_acphy->chanmgri->data->fast_adc_en == 0)) {
			a = 1;
			afe_clk_num = afeclkdiv_arr[altclkpln[row].afeclkdiv] *
				dacclkdiv_arr[altclkpln[row].dacclkdiv] *
				dacdiv_arr[altclkpln[row].dacdiv];
			afe_clk_den = CHSPEC_IS2G(pi->radio_chanspec) ? 8 : 4;
		}
	}
	/* bits_in_mu = 23 */
	if (ACMAJORREV_33(pi->pubpi->phy_rev) && PHY_AS_80P80(pi, chanspec)) {
		uint8 chans[NUM_CHANS_IN_CHAN_BONDING];

		wf_chspec_get_80p80_channels(chanspec, chans);

		FOREACH_CORE(pi, core) {
			/* core 0/1: 80L, core 2/3 80U */
			ch = (core <= 1) ? chans[0] : chans[1];
			fc = wf_channel2mhz(ch, WF_CHAN_FACTOR_5_G);
			PHY_INFORM(("%s: core=%d, fc=%d\n", __FUNCTION__, core, fc));

			math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
				1 << 23, (fc * afe_clk_den) >> 1);
			math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);

			switch (core) {
			case 0:
				WRITE_PHYREG(pi, TxResamplerMuDelta0l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta0u,
						(fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit0l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit0u,
						(fcw & 0xff0000) >> 16);
				break;
			case 1:
				WRITE_PHYREG(pi, TxResamplerMuDelta1l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta1u,
						(fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit1l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit1u,
						(fcw & 0xff0000) >> 16);
				break;
			case 2:
				WRITE_PHYREG(pi, TxResamplerMuDelta2l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta2u,
						(fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit2l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit2u,
						(fcw & 0xff0000) >> 16);
				break;
			case 3:
				WRITE_PHYREG(pi, TxResamplerMuDelta3l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta3u,
						(fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit3l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit3u,
						(fcw & 0xff0000) >> 16);
				break;
			default:
				PHY_ERROR(("wl%d: %s: Max 4 cores only!\n",
						pi->sh->unit, __FUNCTION__));
				ASSERT(0);
			}
		}
	} else if (CHSPEC_IS8080(chanspec)) {
		FOREACH_CORE(pi, core) {
			if (core == 0) {
				ch = wf_chspec_primary80_channel(chanspec);
				fc = wf_channel2mhz(ch, WF_CHAN_FACTOR_5_G);

				math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
					1 << 23, (fc * afe_clk_den) >> 1);
				math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);

				WRITE_PHYREG(pi, TxResamplerMuDelta0l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta0u, (fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit0l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit0u, (fcw & 0xff0000) >> 16);
			} else if (core == 1) {
				ch = wf_chspec_secondary80_channel(chanspec);
				fc = wf_channel2mhz(ch, WF_CHAN_FACTOR_5_G);

				math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
					1 << 23, (fc * afe_clk_den) >> 1);
				math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);

				WRITE_PHYREG(pi, TxResamplerMuDelta1l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDelta1u, (fcw & 0xff0000) >> 16);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit1l, fcw & 0xffff);
				WRITE_PHYREG(pi, TxResamplerMuDeltaInit1u, (fcw & 0xff0000) >> 16);
			}
		}
	} else {
		if (sc_mode == 1) {
			ch = CHSPEC_CHANNEL(chanspec_sel);
			fc = wf_channel2mhz(ch, CHSPEC_IS2G(chanmgri->radio_chanspec_sc) ?
				WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);
		} else {
			ch = CHSPEC_CHANNEL(chanspec_sel);
			fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ?
				WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);
		}
		//ch = CHSPEC_CHANNEL(chanspec);
		//fc = wf_channel2mhz(ch, CHSPEC_IS2G(pi->radio_chanspec) ?
		//	WF_CHAN_FACTOR_2_4_G : WF_CHAN_FACTOR_5_G);
		math_uint64_multiple_add(&tmp_high, &tmp_low, a * afe_clk_num * b,
			1 << 23, (fc * afe_clk_den) >> 1);
		math_uint64_divide(&fcw, tmp_high, tmp_low, fc * afe_clk_den);
		if (sc_mode == 1) {
			WRITE_PHYREG(pi, TxResamplerMuDelta3l, fcw & 0xffff);
			WRITE_PHYREG(pi, TxResamplerMuDelta3u, (fcw & 0xff0000) >> 16);
			WRITE_PHYREG(pi, TxResamplerMuDeltaInit3l, fcw & 0xffff);
			WRITE_PHYREG(pi, TxResamplerMuDeltaInit3u, (fcw & 0xff0000) >> 16);
		} else {
			wlc_phy_tx_farrow_mu_setup(pi, fcw & 0xffff, (fcw & 0xff0000) >> 16,
				fcw & 0xffff, (fcw & 0xff0000) >> 16);
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev)) {
		/* Disable PA during rfseq setting */
		FOREACH_CORE(pi, core) {
			restore_over_pa5g_pu[core] = READ_RADIO_REGFLD_20693(pi, TX_TOP_5G_OVR1,
					core, ovr_pa5g_pu);
			restore_pa5g_pu[core] = READ_RADIO_REGFLD_20693(pi, PA5G_CFG1,
					core, pa5g_pu);
			restore_ext_5g_papu[core] = READ_PHYREGFLDCE(pi, RfctrlIntc,
					core, ext_5g_papu);
			restore_override_ext_pa[core] = READ_PHYREGFLDCE(pi, RfctrlIntc,
					core, override_ext_pa);
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 1);
			MOD_RADIO_REG_20693(pi, PA5G_CFG1, core, pa5g_pu, 0);
			MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 0);
		}

		wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);
		wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_TX2RX);

		/* Restore PA reg value after reseq setting */
		FOREACH_CORE(pi, core) {
			MOD_RADIO_REG_20693(pi, TX_TOP_5G_OVR1,
					core, ovr_pa5g_pu, restore_over_pa5g_pu[core]);
			MOD_RADIO_REG_20693(pi, PA5G_CFG1,
					core, pa5g_pu, restore_pa5g_pu[core]);
			MOD_PHYREGCE(pi, RfctrlIntc,
					core, ext_5g_papu, restore_ext_5g_papu[core]);
			MOD_PHYREGCE(pi, RfctrlIntc,
					core, override_ext_pa, restore_override_ext_pa[core]);
		}

		wlc_phy_resetcca_acphy(pi);
		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}
}

static bool
BCMATTACHFN(phy_ac_chanmgr_attach_paprr)(phy_ac_chanmgr_info_t *ac_info)
{
	phy_info_t *pi = ac_info->pi;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if ((pi->paprrmcsgain2g = phy_malloc(pi, (sizeof(uint8) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain2g malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgain5g20 = phy_malloc(pi,
			(sizeof(uint8) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain5g20 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgain5g40 = phy_malloc(pi,
			(sizeof(uint8) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain5g40 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgain5g80 = phy_malloc(pi,
			(sizeof(uint8) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain5g80 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma2g = phy_malloc(pi, (sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma2g malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma5g20 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma5g20 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma5g40 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma5g40 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma5g80 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma5g80 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma2g_ch13 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma2g_ch13 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgamma2g_ch1 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgamma2g_ch1 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgain2g_ch13 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain2g_ch13 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	if ((pi->paprrmcsgain2g_ch1 = phy_malloc(pi,
			(sizeof(int16) * NUM_MCS_PAPRR_GAMMA))) == NULL) {
		PHY_ERROR(("%s: paprrmcsgain2g_ch1 malloc failed\n", __FUNCTION__));
		return FALSE;
	}
	return TRUE;
}
static bool
BCMATTACHFN(phy_ac_chanmgr_attach_farrow)(phy_ac_chanmgr_info_t *ci)
{
	int num_bw;
	phy_info_t *pi = ci->pi;
	const chan_info_tx_farrow(*tx_farrow) [ACPHY_NUM_CHANS];

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	ci->tx_farrow = NULL;
	ci->rx_farrow = NULL;

	/* these revs do not use farrow tables, they instead calculate them */
	if (TINY_RADIO(pi) || ACMAJORREV_36(pi->pubpi->phy_rev) ||
		ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		return TRUE;
	}

#ifdef ACPHY_1X1_ONLY
	num_bw = 1;
#else
	num_bw = ACPHY_NUM_BW;
#endif // endif

	if ((ci->tx_farrow =
	     phy_malloc(pi, num_bw * sizeof(chan_info_tx_farrow[ACPHY_NUM_CHANS]))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		           __FUNCTION__, MALLOCED(pi->sh->osh)));
		return FALSE;
	}

	if ((ci->rx_farrow =
	     phy_malloc(pi, num_bw * sizeof(chan_info_rx_farrow[ACPHY_NUM_CHANS]))) == NULL) {
		PHY_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", pi->sh->unit,
		           __FUNCTION__, MALLOCED(pi->sh->osh)));
		return FALSE;
	}

	memcpy(ci->rx_farrow, rx_farrow_tbl,
	       ACPHY_NUM_CHANS * num_bw * sizeof(chan_info_rx_farrow));

#ifdef ACPHY_1X1_ONLY
	ASSERT(phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode == 1);
	tx_farrow = tx_farrow_dac1_tbl;
#else /* ACPHY_1X1_ONLY */
	switch (phy_ac_radio_get_data(pi->u.pi_acphy->radioi)->dac_mode) {
	case 2:
		tx_farrow = tx_farrow_dac2_tbl;
		break;
	case 3:
		tx_farrow = tx_farrow_dac3_tbl;
		break;
	case 1:
	default:
		/* default to dac_mode 1 */
		tx_farrow = tx_farrow_dac1_tbl;
		break;
	}
#endif /* ACPHY_1X1_ONLY */
	memcpy(ci->tx_farrow, tx_farrow, ACPHY_NUM_CHANS * num_bw * sizeof(chan_info_tx_farrow));

	return TRUE;
}

#define TINY_GET_ADC_MODE(pi, chanspec)		\
	((CHSPEC_IS20(chanspec) || CHSPEC_IS40(chanspec)) ?	\
	(pi->u.pi_acphy->chanmgri->use_fast_adc_20_40) : 1)
void
wlc_phy_farrow_setup_tiny(phy_info_t *pi, chanspec_t chanspec)
{
	/* Setup adc mode based on BW */
	pi->u.pi_acphy->chanmgri->data->fast_adc_en = TINY_GET_ADC_MODE(pi, chanspec);

	phy_ac_chanmgr_write_tx_farrow_tiny(pi, chanspec, 0, 0);
	phy_ac_chanmgr_write_rx_farrow_tiny(pi, chanspec, 0, 0);

	/* Enable the Tx resampler on all cores */
	MOD_PHYREG(pi, TxResamplerEnable0, enable_tx, 1);
}

void
phy_ac_chanmgr_set_phymode(phy_info_t *pi, chanspec_t chanspec, chanspec_t chanspec_sc,
	uint16 phymode)
{
	uint8 ch[NUM_CHANS_IN_CHAN_BONDING];
#ifndef WL_AIR_IQ
	uint8 stall_val = 0;
#endif /* WL_AIR_IQ */
	uint16 classifier_state = 0;
	uint8 orig_rxfectrl1 = 0;
#ifndef ATE_BUILD
	uint8 rx_coremask, tx_coremask;
	uint8 bwbit = 2;
#endif /* !ATE_BUILD */
	uint8 bwidx = 0;
	uint16 gpioout = 0;
	uint16 gpio9_mask = 0x200;
	uint16 gpio11_mask = 0x800;
	acphy_swctrlmap4_t *swctrl = pi->u.pi_acphy->sromi->swctrlmap4;
	static int orig_min_fm_lp = 0, orig_st_level_time = 0, origSigFld1Decode = 0;
	phy_radar_info_t *ri = pi->radari;
	phy_radar_st_t *st = phy_radar_get_st(ri);
	bool is43684mch2 = ACMAJORREV_47(pi->pubpi->phy_rev) && CHSPEC_IS2G(chanspec) &&
		CHSPEC_IS5G(chanspec_sc) && !BF_ELNA_5G(pi->u.pi_acphy);

	/* when upgrade from 3+1, phymode is set to 0 twice */
	if (ACMAJORREV_47(pi->pubpi->phy_rev) &&
		READ_PHYREGFLD(pi, AntDivConfig2059, Trigger_override_per_core) == 0 &&
		phymode == 0) {
		goto avoid_settwice;
	}
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	/* Disable classifier */
	classifier_state = READ_PHYREG(pi, ClassifierCtrl);
	phy_rxgcrs_sel_classifier(pi, 4);

	/* Disable stalls and hold FIFOs in reset */
#ifdef WL_AIR_IQ
	// Using WRITE_PHYREG instead of MOD_PHYREG since
	// read followed by read modify write(i.e. MOD_PHYREG)
	// corrupts the value of soft_sdfeFifoReset bit in orig_rxfectrl1
	orig_rxfectrl1 = READ_PHYREG(pi, RxFeCtrl1);
	// Bit 0 is soft_sdfeFifoReset and bit 1 is disable stalls
	WRITE_PHYREG(pi, RxFeCtrl1, orig_rxfectrl1 | 0x0003);
#else
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
	ACPHY_DISABLE_STALL(pi);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
#endif /* WL_AIR_IQ */
	// Disable core2core sync and Enable it after switch
	if (ACMAJORREV_47(pi->pubpi->phy_rev))
		phy_ac_chanmgr_core2core_sync_setup(pi->u.pi_acphy->chanmgri, FALSE);
	switch (phymode) {
	case  PHYMODE_BGDFS:
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev)) { /* 4365 */
			ch[0] = CHSPEC_CHANNEL(chanspec);
			ch[1] = CHSPEC_CHANNEL(chanspec_sc);
			wlc_phy_radio20693_afe_clkdistribtion_mode(pi, 2);
			phy_ac_radio_20693_pmu_pll_config_wave2(pi, 5);
			phy_ac_radio_20693_chanspec_setup(pi, ch[0], 0, 0, 0);
			phy_ac_radio_20693_chanspec_setup(pi, ch[1], 0, 1, 0);
			wlc_phy_radio2069x_vcocal_isdone(pi, FALSE, FALSE);
#ifdef WL_AIR_IQ
			wlc_phy_radio20693_sel_logen_mode(pi);
#else
			wlc_phy_radio20693_sel_logen_5g_mode(pi, 2);
#endif /* WL_AIR_IQ */
			/* Setup adc mode based on BW */
			pi->u.pi_acphy->chanmgri->data->fast_adc_en =
				TINY_GET_ADC_MODE(pi, chanspec);
			phy_ac_chanmgr_write_tx_farrow_tiny(pi, chanspec, chanspec_sc, 1);
			phy_ac_chanmgr_write_rx_farrow_tiny(pi, chanspec, chanspec_sc, 1);
			/* Enable the Tx resampler on all cores */
			MOD_PHYREG(pi, TxResamplerEnable3, enable_tx, 1);
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) { /* 43684 */
			ch[0] = CHSPEC_CHANNEL(chanspec);
			ch[1] = CHSPEC_CHANNEL(chanspec_sc);
			wlc_phy_radio20698_powerup_RFP1(pi, 1);
			wlc_phy_radio20698_sel_logen_mode(pi, 4);
			wlc_phy_chanspec_radio20698_setup(pi, ch[1], 1, 4);
			if ((CHSPEC_IS2G(chanspec) && CHSPEC_IS5G(chanspec_sc)) ||
				(CHSPEC_IS5G(chanspec)&& CHSPEC_IS2G(chanspec_sc))) {
				/* different band */
				wlc_phy_radio20698_pu_rx_core(pi, 3, ch[1], 0);
			}
#ifndef ATE_BUILD
			/* enable p1c */
			bwbit = CHSPEC_BW_LE20(chanspec_sc)? 2 : CHSPEC_IS40(chanspec_sc)? 4 :
				CHSPEC_IS80(chanspec_sc)? 6 : 8;
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x10 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x11 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x91 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x11 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x01 + bwbit));
			MOD_PHYREG(pi, dacClkCtrl, vasipClkEn, 1);
			MOD_PHYREG(pi, dacClkCtrl, vasipAutoClkEn, 1);
#endif /* !ATE_BUILD */
			MOD_PHYREG(pi, TxResamplerEnable3, tx_stall_disable, 1);
#ifndef ATE_BUILD
			wlc_phy_rx_farrow_setup_28nm(pi, chanspec_sc, 1);
#endif /* !ATE_BUILD */
			bwidx = CHSPEC_BW_LE20(chanspec_sc)? 0 : CHSPEC_IS40(chanspec_sc)? 1 :
				CHSPEC_IS80(chanspec_sc)? 2 : 3;
			MOD_PHYREG(pi, RfctrlOverrideLpfCT3, lpf_bq1_bw, 1);
			MOD_PHYREG(pi, RfctrlOverrideLpfCT3, lpf_bq2_bw, 1);
			MOD_PHYREG(pi, RfctrlCoreLpfCT3, lpf_bq1_bw, bwidx);
			MOD_PHYREG(pi, RfctrlCoreLpfCT3, lpf_bq2_bw, bwidx);
			wlc_phy_radio20698_afe_div_ratio(pi, 1, chanspec_sc, 1);
			/* different band for 43684 MC board */
			if (swctrl->bandsel_on_gpio9) {
				gpioout = CHSPEC_IS2G(chanspec) ? 0 : gpio9_mask;
				si_gpioout(pi->sh->sih, gpio9_mask, gpioout, GPIO_DRV_PRIORITY);
				si_gpioouten(pi->sh->sih, gpio9_mask, gpio9_mask,
					GPIO_DRV_PRIORITY);
				si_gpiocontrol(pi->sh->sih, gpio9_mask, 0,
					GPIO_DRV_PRIORITY);  /* chip_gpio9=gpioout */
			}
			if (swctrl->bandsel_on_gpio11) {
				gpioout = CHSPEC_IS2G(chanspec_sc) ? 0 : gpio11_mask;
				si_gpioout(pi->sh->sih, gpio11_mask, gpioout, GPIO_DRV_PRIORITY);
				si_gpioouten(pi->sh->sih, gpio11_mask, gpio11_mask,
					GPIO_DRV_PRIORITY);
				si_gpiocontrol(pi->sh->sih, gpio11_mask, 0,
					GPIO_DRV_PRIORITY);  /* chip_gpio11=gpioout */
			}
			if (CHSPEC_IS2G(chanspec) && CHSPEC_IS5G(chanspec_sc)) {
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutputOvr, 1);
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutput, 6);
			} else if (CHSPEC_IS5G(chanspec) && CHSPEC_IS2G(chanspec_sc)) {
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutputOvr, 1);
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutput, 4);
			}
			if (CHSPEC_IS5G(chanspec) && CHSPEC_IS5G(chanspec_sc) &&
				!ACMINORREV_0(pi) && (ch[1] == 52)) {
				// small fm on 5g+5g plc CH52 due to RB149974 5g lna1 gain change
				orig_min_fm_lp = st->rparams.radar_args.min_fm_lp;
				st->rparams.radar_args.min_fm_lp = 132;
			} else if (is43684mch2) {
				orig_min_fm_lp = st->rparams.radar_args.min_fm_lp;
				orig_st_level_time = st->rparams.radar_args.st_level_time;
				// FCC-5 small fm on plc bw20/bw40
				// set pw thrsh to 610 to improve ETSI-4 detection on bw20/bw40
				if (CHSPEC_IS20(chanspec_sc)) {
					st->rparams.radar_args.min_fm_lp = 13;
					st->rparams.radar_args.st_level_time = 0x8262;
					st->chanset_elna_chk = (NC_CHAN_2GBW20 | NC_CHAN_2GBW40 |
						SC_CHAN_5GBW20 | EXT_LNA_5G_OFF);
				} else if (CHSPEC_IS40(chanspec_sc)) {
					st->rparams.radar_args.min_fm_lp = 160;
					st->rparams.radar_args.st_level_time = 0x8262;
				}
			}
			if (CHSPEC_IS80(chanspec) && (ch[0] <= WL_THRESHOLD_LO_BAND)) {
				origSigFld1Decode = READ_PHYREGFLD(pi, RadarBlankCtrl2,
					radarSigDecode1BlankEn);
				MOD_PHYREG(pi, RadarBlankCtrl2, radarSigDecode1BlankEn, 0);
			}
		} else {
			PHY_ERROR(("wl%d: %s: Unsupported PHY revision for BGDFS\n",
			         pi->sh->unit, __FUNCTION__));
			break;
		}
#ifndef ATE_BUILD
		MOD_PHYREG(pi, RfseqMode, CoreActv_override_percore, 8);
		/* Enable RX on core-3, disable Tx */
		rx_coremask = 0x8 | phy_stf_get_data(pi->stfi)->phyrxchain;
		tx_coremask = 0x7 & phy_stf_get_data(pi->stfi)->phytxchain;

		MOD_PHYREG(pi, RfseqCoreActv2059, EnRx, rx_coremask);
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, tx_coremask);
		MOD_PHYREG(pi, CoreConfig, CoreMask, tx_coremask);
#ifdef WL_AIR_IQ
		MOD_PHYREG(pi, CoreConfig, pasvRxSampCapOn, 1);
#else
		MOD_PHYREG(pi, CoreConfig, pasvRxSampCapOn, 0);
#endif /* WL_AIR_IQ */
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, AntDivConfig2059, Trigger_override_per_core, 8);
		} else {
			/* 43684: if it is set p1c bw < normal bw cannot detect radar */
			MOD_PHYREG(pi, CoreConfig, pasvRxCoreMask, 8);
		}

		if (CHSPEC_CHANNEL(chanspec_sc) <= WL_THRESHOLD_LO_BAND) {
			if (CHSPEC_IS20(chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc=
					st->rparams.radar_thrs2.thresh0_sc_20_lo;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_20_lo;
			} else if (CHSPEC_IS40(chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_40_lo;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_40_lo;
			} else if (CHSPEC_IS80(chanspec_sc) || PHY_AS_80P80(pi, chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_80_lo;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_80_lo;
			} else if (CHSPEC_IS160(chanspec_sc) && ACMAJORREV_47(pi->pubpi->phy_rev)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_160_lo;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_160_lo;
			}
		} else {
			if (CHSPEC_IS20(chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc=
					st->rparams.radar_thrs2.thresh0_sc_20_hi;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_20_hi;
			} else if (CHSPEC_IS40(chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_40_hi;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_40_hi;
			} else if (CHSPEC_IS80(chanspec_sc) || PHY_AS_80P80(pi, chanspec_sc)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_80_hi;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_80_hi;
			} else if (CHSPEC_IS160(chanspec_sc) && ACMAJORREV_47(pi->pubpi->phy_rev)) {
				st->rparams.radar_args.thresh0_sc =
					st->rparams.radar_thrs2.thresh0_sc_160_hi;
				st->rparams.radar_args.thresh1_sc =
					st->rparams.radar_thrs2.thresh1_sc_160_hi;
			}
		}
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh0_SC(pi->pubpi->phy_rev),
		(uint16)((int16)st->rparams.radar_args.thresh0_sc));
		phy_utils_write_phyreg(pi, ACPHY_RadarThresh1_SC(pi->pubpi->phy_rev),
		(uint16)((int16)st->rparams.radar_args.thresh1_sc));
#endif /* !ATE_BUILD */
		break;
	case 0:
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev)) { /* 4365 */
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG1, 1, rfpll_synth_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_CP1, 1, rfpll_cp_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG1, 1, rfpll_vco_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG1, 1, rfpll_vco_buf_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG1, 1, rfpll_monitor_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_LF6, 1, rfpll_lf_cm_pu, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_CFG1, 1, rfpll_pfd_en, 0);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_rfpll_synth_pu, 1);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_rfpll_cp_pu, 1);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_rfpll_vco_pu, 1);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_rfpll_vco_buf_pu, 1);
			MOD_RADIO_PLLREG_20693(pi, PLL_HVLDO1, 1, ldo_2p5_pu_ldo_CP, 0);
			MOD_RADIO_PLLREG_20693(pi, PLL_HVLDO1, 1, ldo_2p5_pu_ldo_VCO, 0);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_ldo_2p5_pu_ldo_CP, 1);
			MOD_RADIO_PLLREG_20693(pi, RFPLL_OVR1, 1, ovr_ldo_2p5_pu_ldo_VCO, 1);
			wlc_phy_radio20693_afe_clkdistribtion_mode(pi, 0);
#ifdef WL_AIR_IQ
			wlc_phy_radio20693_sel_logen_mode(pi);
#else
			wlc_phy_radio20693_sel_logen_5g_mode(pi, 0);
#endif /* WL_AIR_IQ */

			/* Setup adc mode based on BW */
			pi->u.pi_acphy->chanmgri->data->fast_adc_en =
				TINY_GET_ADC_MODE(pi, chanspec);
			phy_ac_chanmgr_write_tx_farrow_tiny(pi, chanspec, chanspec_sc, 0);
			phy_ac_chanmgr_write_rx_farrow_tiny(pi, chanspec, chanspec_sc, 0);
			/* Enable the Tx resampler on all cores */
			MOD_PHYREG(pi, TxResamplerEnable0, enable_tx, 1);
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) { /* 43684 */
			ch[0] = CHSPEC_CHANNEL(chanspec);
			ch[1] = CHSPEC_CHANNEL(chanspec_sc);
			wlc_phy_radio20698_powerup_RFP1(pi, 0);
			wlc_phy_radio20698_sel_logen_mode(pi, 0);
			wlc_phy_chanspec_radio20698_setup(pi, ch[0], 1, 0);

			if ((CHSPEC_IS2G(chanspec) && CHSPEC_IS5G(chanspec_sc)) ||
				(CHSPEC_IS5G(chanspec)&& CHSPEC_IS2G(chanspec_sc))) {
				/* different band */
				MOD_RADIO_REG_20698(pi, RX5G_REG1, 3, rx5g_lna_tune, 0xf);
				MOD_RADIO_REG_20698(pi, RX5G_REG5, 3, rx5g_mix_Cin_tune, 0);
				wlc_phy_radio20698_pu_rx_core(pi, 3, ch[0], 1);
			}
#ifndef ATE_BUILD
			/* reset p1c phyreg first before turn it off */
			MOD_PHYREG_p1c(pi, RadarSearchCtrl, radarEnable, 0);
			/* disable p1c */
			bwbit = CHSPEC_BW_LE20(chanspec_sc)? 2 : CHSPEC_IS40(chanspec_sc)? 4 :
				CHSPEC_IS80(chanspec_sc)? 6 : 8;
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x11 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), (0x10 + bwbit));
			W_REG(pi->sh->osh, D11_PHYPLUS1CTL(pi), 0x02);
#endif /* !ATE_BUILD */
			MOD_PHYREG(pi, TxResamplerEnable3, tx_stall_disable, 0);
#ifndef ATE_BUILD
			wlc_phy_rx_farrow_setup_28nm(pi, chanspec_sc, 0);
#endif /* !ATE_BUILD */
			MOD_PHYREG(pi, RfctrlOverrideLpfCT3, lpf_bq1_bw, 0);
			MOD_PHYREG(pi, RfctrlOverrideLpfCT3, lpf_bq2_bw, 0);
			wlc_phy_radio20698_afe_div_ratio(pi, 0, 0, 0);

			/* different band for 43684 MC board */
			if (swctrl->bandsel_on_gpio9) {
				gpioout = CHSPEC_IS2G(chanspec) ? 0 : gpio9_mask;
				si_gpioout(pi->sh->sih, gpio9_mask, gpioout, GPIO_DRV_PRIORITY);
				si_gpioouten(pi->sh->sih, gpio9_mask, gpio9_mask,
					GPIO_DRV_PRIORITY);
				si_gpiocontrol(pi->sh->sih, gpio9_mask, 0, GPIO_DRV_PRIORITY);
			}
			if (swctrl->bandsel_on_gpio11) {
				gpioout = CHSPEC_IS2G(chanspec) ? 0 : gpio11_mask;
				si_gpioout(pi->sh->sih, gpio11_mask, gpioout, GPIO_DRV_PRIORITY);
				si_gpioouten(pi->sh->sih, gpio11_mask, gpio11_mask,
					GPIO_DRV_PRIORITY);
				si_gpiocontrol(pi->sh->sih, gpio11_mask, 0, GPIO_DRV_PRIORITY);
			}
			if ((CHSPEC_IS2G(chanspec) && CHSPEC_IS5G(chanspec_sc)) ||
				(CHSPEC_IS5G(chanspec)&& CHSPEC_IS2G(chanspec_sc))) {
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutput, 0);
				MOD_PHYREG(pi, FemOutputOvrCtrl3, femCtrlOutputOvr, 0);
			}
			/* restore min_fm_lp/st_level_time/chanset_elna_chk when phymode 0 */
			if (CHSPEC_IS5G(chanspec) && CHSPEC_IS5G(chanspec_sc) &&
				!ACMINORREV_0(pi) && (ch[1] == 52) && (orig_min_fm_lp != 0)) {
				st->rparams.radar_args.min_fm_lp = orig_min_fm_lp;
			} else if (is43684mch2 && (CHSPEC_IS20(chanspec_sc) ||
				CHSPEC_IS40(chanspec_sc))) {
				if (orig_min_fm_lp != 0)
					st->rparams.radar_args.min_fm_lp = orig_min_fm_lp;
				if (orig_st_level_time != 0)
					st->rparams.radar_args.st_level_time = orig_st_level_time;
				if (CHSPEC_IS20(chanspec_sc))
					st->chanset_elna_chk = 0;
			}
			if (CHSPEC_IS80(chanspec) && (ch[0] <= WL_THRESHOLD_LO_BAND) &&
				(origSigFld1Decode != 0)) {
				MOD_PHYREG(pi, RadarBlankCtrl2, radarSigDecode1BlankEn,
					origSigFld1Decode);
			}
		} else {
			PHY_ERROR(("wl%d: %s: Unsupported PHY revision for BGDFS\n",
			         pi->sh->unit, __FUNCTION__));
			break;
		}
#ifndef ATE_BUILD
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, TxPwrCtrlCmd, txPwrCtrl_en, 0);
			wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
			wlc_phy_txpwr_by_index_acphy(pi, (1 << 3), 64);
		}
		MOD_PHYREG(pi, RfseqMode, CoreActv_override_percore, 0);
		/* Enable RX and TX on core-3 */
		rx_coremask = 0x8 | phy_stf_get_data(pi->stfi)->phyrxchain;
		tx_coremask = 0x8 | phy_stf_get_data(pi->stfi)->phytxchain;

		MOD_PHYREG(pi, RfseqCoreActv2059, EnRx, rx_coremask);
		MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, tx_coremask);
		MOD_PHYREG(pi, CoreConfig, CoreMask, tx_coremask);
		MOD_PHYREG(pi, CoreConfig, pasvRxSampCapOn, 0);
		MOD_PHYREG(pi, CoreConfig, pasvRxCoreMask, 0);
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_ON);
		}
#endif /* !ATE_BUILD */
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
			pi->sh->unit, __FUNCTION__, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(0);
	}
	/* Restore FIFO reset and Stalls */
#ifdef WL_AIR_IQ
	// Force soft_sdfeFifoReset to 0 so that FIFOs are never
	// in reset state
	WRITE_PHYREG(pi, RxFeCtrl1, orig_rxfectrl1 & 0xFFFE);
	OSL_DELAY(1);
#else
	ACPHY_ENABLE_STALL(pi, stall_val);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);
	OSL_DELAY(1);
#endif /* WL_AIR_IQ */

	/* Restore classifier */
	WRITE_PHYREG(pi, ClassifierCtrl, classifier_state);
	OSL_DELAY(1);

	/* Reset PHY */
	wlc_phy_resetcca_acphy(pi);
	/* Enable Core2Core Sync */
	if (ACMAJORREV_47(pi->pubpi->phy_rev))
		phy_ac_chanmgr_core2core_sync_setup(pi->u.pi_acphy->chanmgri, TRUE);
	wlapi_enable_mac(pi->sh->physhim);

	avoid_settwice:
	switch (phymode) {
		case  PHYMODE_BGDFS:
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, AntDivConfig2059, Trigger_override_per_core, 8);
				MOD_PHYREG(pi, RadarSearchCtrl_SC, radarEnable, 1);
				MOD_PHYREG(pi, RadarDetectConfig3_SC, scan_mode, 1);
				MOD_PHYREG(pi, RadarDetectConfig3_SC, gain_override_en, 1);
				MOD_PHYREG(pi, RadarBlankCtrl2_SC, blank_mode, 1);
			} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
#ifndef ATE_BUILD
				MOD_PHYREG(pi, AntDivConfig2059, Trigger_override_per_core, 8);
				/* disable_stalls=0 for real chip and 1 for self_test */
				MOD_PHYREG_p1c(pi, RxFeCtrl1, disable_stalls, 0);
				MOD_PHYREG_p1c(pi, RxFeCtrl1, swap_iq0, 1);
				MOD_PHYREG_p1c(pi, sarAfeCompCtrl0, sarAfePhaseSel,
					CHSPEC_IS160(chanspec_sc) ? 1 : 0);
				MOD_PHYREG_p1c(pi, RadarTx2NclksBlank, tx2nclks_BlankEn, 1);
				MOD_PHYREG_p1c(pi, RadarTx2NclksBlank, tx2nclks_BlankExtTime, 0);
				MOD_PHYREG_p1c(pi, RfseqMode, CoreActv_override_percore, 1);
				MOD_PHYREG_p1c(pi, RfseqCoreActv2059, EnRx, 1);
				MOD_PHYREG_p1c(pi, RfseqCoreActv2059, EnTx, 0);
				MOD_PHYREG_p1c(pi, CoreConfig, CoreMask, 0);
				MOD_PHYREG_p1c(pi, CoreConfig, NumRxCores, 1);
				MOD_PHYREG_p1c(pi, CoreConfig, pasvRxCoreMask, 1);
				MOD_PHYREG_p1c(pi, AntDivConfig2059, Trigger_override_per_core, 1);
				MOD_PHYREG_p1c(pi, RadarDetectConfig3, gain_override_en, 1);
				if (is43684mch2) {
					MOD_PHYREG_p1c(pi, RadarGainOverride,
						gain_override_val, 0x30);
				} else { /* set gain_override_val to 0x3d to fix A1 p1c detection */
					MOD_PHYREG_p1c(pi, RadarGainOverride,
						gain_override_val, 0x3d);
				}
				MOD_PHYREG_p1c(pi, RadarDetectConfig3, scan_mode, 1);
				MOD_PHYREG_p1c(pi, RadarSearchCtrl, radarEnable, 1);
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarBlankCtrl(pi->pubpi->phy_rev),
					CHSPEC_IS20(chanspec_sc) ? 0x8019 :
					CHSPEC_IS40(chanspec_sc) ? 0x8032 :
					CHSPEC_IS80(chanspec_sc) ? 0x8064 : 0x80c8);
				if (CHSPEC_IS2G(chanspec) && CHSPEC_IS5G(chanspec_sc)) {
					phy_utils_write_phyreg_p1c(pi,
						ACPHY_RadarBlankCtrl2(pi->pubpi->phy_rev), 0x8000);
				} else {
					phy_utils_write_phyreg_p1c(pi,
						ACPHY_RadarBlankCtrl2(pi->pubpi->phy_rev), 0xa000);
				}
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarThresh0(pi->pubpi->phy_rev),
					(uint16)((int16)st->rparams.radar_args.thresh0_sc));
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarThresh1(pi->pubpi->phy_rev),
					(uint16)((int16)st->rparams.radar_args.thresh1_sc));
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarT3BelowMin(pi->pubpi->phy_rev), 0x14);
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarT3Timeout(pi->pubpi->phy_rev),
					CHSPEC_IS20(chanspec_sc) ? 0x258 :
					CHSPEC_IS40(chanspec_sc) ? 0x4b0 :
					CHSPEC_IS80(chanspec_sc) ? 0x960 : 0x12c0);
				phy_utils_write_phyreg_p1c(pi,
					ACPHY_RadarResetBlankingDelay(pi->pubpi->phy_rev),
					CHSPEC_IS20(chanspec_sc) ? 0x19 : CHSPEC_IS40(chanspec_sc) ?
					0x32 : CHSPEC_IS80(chanspec_sc) ? 0x64 : 0xc8);
				MOD_PHYREG_p1c(pi, DcFiltAddress, dcCoef0,
					CHSPEC_IS20(chanspec_sc) ?
					0xa : CHSPEC_IS40(chanspec_sc) ? 0x5 :
					CHSPEC_IS80(chanspec_sc) ? 0x2 : 0x1);
#endif /* !ATE_BUILD */
			} else {
				PHY_ERROR(("wl%d: %s: Unsupported PHY revision for BGDFS\n",
					pi->sh->unit, __FUNCTION__));
			}
			break;
		case 0:
#ifndef ATE_BUILD
			MOD_PHYREG(pi, AntDivConfig2059, Trigger_override_per_core, 0);
			MOD_PHYREG(pi, RadarSearchCtrl_SC, radarEnable, 0);
			MOD_PHYREG(pi, RadarDetectConfig3_SC, scan_mode, 0);
			MOD_PHYREG(pi, RadarDetectConfig3_SC, gain_override_en, 0);
			MOD_PHYREG(pi, RadarBlankCtrl2_SC, blank_mode, 0);
#endif /* !ATE_BUILD */
			break;
		default:
			PHY_ERROR(("wl%d: %s: Unsupported radio revision %d\n",
			         pi->sh->unit, __FUNCTION__, RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
	}

}

void
BCMATTACHFN(wlc_phy_nvram_avvmid_read)(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint8 i, j, ant;
	uint8 core;
	char phy_var_name[20];
	/*	phy_info_acphy_t *pi_ac = pi->u.pi_acphy; */
	FOREACH_CORE(pi, core) {
		ant = phy_get_rsdbbrd_corenum(pi, core);
		(void)snprintf(phy_var_name, sizeof(phy_var_name), rstr_AvVmid_cD, ant);
		if ((PHY_GETVAR_SLICE(pi, phy_var_name)) != NULL) {
			for (i = 0; i < ACPHY_NUM_BANDS; i++) {
				for (j = 0; j < ACPHY_AVVMID_NVRAM_PARAMS; j++) {
					pi_ac->sromi->avvmid_set_from_nvram[ant][i][j] =
						(uint8) PHY_GETINTVAR_ARRAY_SLICE(pi, phy_var_name,
						(ACPHY_AVVMID_NVRAM_PARAMS*i +j));
				}
			}
#ifndef BOARD_FLAGS3
			/* If the AV VMID values are populated in the NVRAM, use those values,
			even if boadflag is not set
			*/
			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				BF3_AVVMID_FROM_NVRAM(pi_ac) = 1;
			}
#endif // endif
		}
	}
}

#if defined(WLTEST)
void wlc_phy_get_avvmid_acphy(phy_info_t *pi, int32 *ret_int_ptr, wlc_avvmid_t avvmid_type,
		uint8 *core_sub_band)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 avvmid_idx = 0;
	uint8 band_idx = core_sub_band[1];
	uint8 core = core_sub_band[0];
	avvmid_idx = (avvmid_type == AV) ? 0 : 1;
	*ret_int_ptr = (int32)(pi_ac->sromi->avvmid_set_from_nvram[core][band_idx][avvmid_idx]);
	return;
}

void wlc_phy_set_avvmid_acphy(phy_info_t *pi, uint8 *avvmid, wlc_avvmid_t avvmid_type)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 core, sub_band_idx;
	uint8 avvmid_idx = 0;
	avvmid_idx = (avvmid_type == AV) ? 0 : 1;
	core = avvmid[0];
	sub_band_idx = avvmid[1];
	pi_ac->sromi->avvmid_set_from_nvram[core][sub_band_idx][avvmid_idx] = avvmid[2];
	/* Load Pdet related settings */
	wlc_phy_set_pdet_on_reset_acphy(pi);
}
#endif // endif

void BCMATTACHFN(wlc_phy_nvram_vlin_params_read)(phy_info_t *pi)
{

	char phy_var_name2[20], phy_var_name3[20];
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 core, ant;
	FOREACH_CORE(pi, core) {
		ant = phy_get_rsdbbrd_corenum(pi, core);
		if ((!TINY_RADIO(pi)) && BF3_VLIN_EN_FROM_NVRAM(pi_ac)) {
			(void)snprintf(phy_var_name2, sizeof(phy_var_name2),
				rstr_VlinPwr2g_cD, ant);
			if ((PHY_GETVAR_SLICE(pi, phy_var_name2)) != NULL) {
				pi_ac->chanmgri->cfg->vlinpwr2g_from_nvram =
					(uint8) PHY_GETINTVAR_SLICE(pi, phy_var_name2);
			}
			(void)snprintf(phy_var_name3, sizeof(phy_var_name3),
				rstr_Vlinmask2g_cD, ant);
			if ((PHY_GETVAR_SLICE(pi, phy_var_name3)) != NULL) {
				pi_ac->chanmgri->data->vlinmask2g_from_nvram =
					(uint8) PHY_GETINTVAR_SLICE(pi, phy_var_name3);
			}

			if (PHY_BAND5G_ENAB(pi)) {
				(void)snprintf(phy_var_name2, sizeof(phy_var_name2),
					rstr_VlinPwr5g_cD, ant);
				if ((PHY_GETVAR_SLICE(pi, phy_var_name2)) != NULL) {
					pi_ac->chanmgri->cfg->vlinpwr5g_from_nvram =
						(uint8) PHY_GETINTVAR_SLICE(pi, phy_var_name2);
				}
				(void)snprintf(phy_var_name3, sizeof(phy_var_name3),
					rstr_Vlinmask5g_cD, ant);
				if ((PHY_GETVAR_SLICE(pi, phy_var_name3)) != NULL) {
					pi_ac->chanmgri->data->vlinmask5g_from_nvram =
						(uint8) PHY_GETINTVAR_SLICE(pi, phy_var_name3);
				}
			}
		}
	}
}

static void
wlc_tiny_setup_coarse_dcc(phy_info_t *pi)
{
	uint8 phybw;
	uint8 core;

	/*
	 * Settings required to use the RFSeq to trigger the coarse DCC
	 * 4345TC Not used. 20691_coarse_dcc used
	 * 4345A0 offset comparator has hysteresis and dc offset but is adequate for 5G
	 * 4365-analog DCC
	 */

	if ((!ACMAJORREV_4(pi->pubpi->phy_rev)) && (!ACMAJORREV_32(pi->pubpi->phy_rev)) &&
		(!ACMAJORREV_33(pi->pubpi->phy_rev))) {
		wlc_tiny_dc_static_WAR(pi);
	}

	/* DCC FSM Defaults */
	MOD_PHYREG(pi, BBConfig, dcc_wakeup_restart_en, 0);
	MOD_PHYREG(pi, BBConfig, dcc_wakeup_restart_delay, 10);

	/* Control via pktproc, instead of RFSEQ */
	MOD_PHYREG(pi, RfseqTrigger, en_pkt_proc_dcc_ctrl,  1);

	FOREACH_CORE(pi, core) {

		/* Disable overrides that may have been set during 2G cal */
		MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_offset_dac_pwrup, 0);
		MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_offset_dac, 0);
		if (ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
			ACMAJORREV_33(pi->pubpi->phy_rev)) {
			MOD_RADIO_REG_20693(pi, RX_BB_2G_OVR_EAST, core,
				ovr_tia_offset_comp_pwrup, 0);
		} else {
			MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_NORTH, core,
				ovr_tia_offset_comp_pwrup, 0);
		}
		MOD_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core, ovr_tia_offset_dac, 0);
		MOD_RADIO_REG_TINY(pi, TIA_CFG8, core, tia_offset_comp_drive_strength, 1);

		/* Set idac LSB to (50nA * 4) ~ 0.2uA for 2G, (50nA * 12) ~ 0.6 uA for 5G */
		/* changed biasadj to 1 as coupled d.c. in loop is very less. */
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			MOD_RADIO_REG_TINY(pi, TIA_CFG8, core, tia_offset_dac_biasadj,
			(CHSPEC_IS2G(pi->radio_chanspec)) ? 1 : 1);
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			MOD_RADIO_REG_TINY(pi, TIA_CFG8, core, tia_offset_dac_biasadj,
			(CHSPEC_IS2G(pi->radio_chanspec)) ? 12 : 4);
		} else {
			MOD_RADIO_REG_TINY(pi, TIA_CFG8, core, tia_offset_dac_biasadj,
			(CHSPEC_IS2G(pi->radio_chanspec)) ? 4 : 12);
		}
	}
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, rx_tia_dc_loop_0, dac_sign, 1);
		MOD_PHYREG(pi, rx_tia_dc_loop_0, en_lock, 1);
		if (phy_get_phymode(pi) != PHYMODE_RSDB) {
			ACPHY_REG_LIST_START
				MOD_PHYREG_ENTRY(pi, rx_tia_dc_loop_1, dac_sign, 1)
				MOD_PHYREG_ENTRY(pi, rx_tia_dc_loop_1, en_lock, 1)
				MOD_PHYREG_ENTRY(pi, rx_tia_dc_loop_1, restart_gear, 6)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	}
	/* Set minimum idle gain incase of restart */
	MOD_PHYREG(pi, rx_tia_dc_loop_0, restart_gear, 6);

	/* 4365-analog DCC: loop through the cores */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		/* Loop through cores */
		FOREACH_CORE(pi, core) {
			MOD_PHYREGCE(pi, rx_tia_dc_loop_, core, dac_sign, 1);
			MOD_PHYREGCE(pi, rx_tia_dc_loop_, core, en_lock, 1);
			MOD_PHYREGCE(pi, rx_tia_dc_loop_, core, restart_gear, 6);
		}
	}

	if (IS20MHZ(pi))
		phybw = 0;
	else if (IS40MHZ(pi))
		phybw = 1;
	else
		phybw = 2;

	/*
	 * Because FSM clock is PHY_BW dependant scale gear gain and loop count.
	 *
	 * Settings below assume:
	 *	9 DCC FSM clock cycle latency and single pole  RC filter >=2MHz ala 4345B0.
	 * (Valid also for 4345A0).
	 */
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_0, loop_gain_0, 15); /* disable */
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_1, loop_gain_1, 2 + phybw);
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_2, loop_gain_2, 4 + phybw);
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_3, loop_gain_3, 5 + phybw);
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_4, loop_gain_4, 6 + phybw);
	MOD_PHYREG(pi, rx_tia_dc_loop_gain_5, loop_gain_5, 8 + phybw);
	/* making Loop count of gear 1 because of CRDOT11ACPHY-1530 */
	MOD_PHYREG(pi, rx_tia_dc_loop_count_0, loop_count_0, 1); /* disable */
	MOD_PHYREG(pi, rx_tia_dc_loop_count_1, loop_count_1, (phybw > 1) ? 255 : (80 << phybw));
	MOD_PHYREG(pi, rx_tia_dc_loop_count_2, loop_count_2, (phybw > 1) ? 255 : (80 << phybw));
	MOD_PHYREG(pi, rx_tia_dc_loop_count_3, loop_count_3, (phybw > 1) ? 255 : (80 << phybw));
	MOD_PHYREG(pi, rx_tia_dc_loop_count_4, loop_count_4, (phybw > 1) ? 255 : (80 << phybw));
	MOD_PHYREG(pi, rx_tia_dc_loop_count_5, loop_count_5, (20 << phybw));

	if (ACMAJORREV_3(pi->pubpi->phy_rev))
		wlc_phy_enable_lna_dcc_comp_20691(pi, PHY_ILNA(pi));
}

void
wlc_phy_mlua_adjust_acphy(phy_info_t *pi, bool btactive)
{
	uint8 zfuA1, zfuA1_log2, zfuA2, zfuA2_log2;
	uint8 mluA1, mluA1_log2, mluA2, mluA2_log2;
	uint8 zfuA0 = 0, zfuA3 = 0;
	uint8 mluA0 = 0, mluA3 = 0;

	/* Disable adjust if chanup is forced to disable */
	/* With auto mode, Disable ChanUpd for 43684B0. HW bug for BW160 */
	if ((pi->u.pi_acphy->chanmgri->chanup_ovrd == 0) ||
	    ((pi->u.pi_acphy->chanmgri->chanup_ovrd == -1) && ACMAJORREV_47(pi->pubpi->phy_rev) &&
	     ACMINORREV_1(pi) && CHSPEC_IS160(pi->radio_chanspec))) {
		return;
	}

	/* Disable this for now, there is some issue with BTcoex */
	if (btactive) {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			mluA1 = 2; mluA1_log2 = 1; mluA2 = 2; mluA2_log2 = 0, mluA0 = 2, mluA3 = 2;
			zfuA1 = 2; zfuA1_log2 = 1; zfuA2 = 2; zfuA2_log2 = 1, zfuA0 = 2; zfuA3 = 2;
		} else {
			mluA1 = 2; mluA1_log2 = 1; mluA2 = 0; mluA2_log2 = 0;
			zfuA1 = 2; zfuA1_log2 = 1; zfuA2 = 2; zfuA2_log2 = 1;
		}
	} else {
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			mluA1 = 4; mluA1_log2 = 2; mluA2 = 4; mluA2_log2 = 2; mluA0 = 4; mluA3 = 2;
			zfuA1 = 4; zfuA1_log2 = 2; zfuA2 = 4; zfuA2_log2 = 2; zfuA0 = 4; zfuA3 = 2;
		} else {
			mluA1 = 4; mluA1_log2 = 2; mluA2 = 4; mluA2_log2 = 2;
			zfuA1 = 4; zfuA1_log2 = 2; zfuA2 = 4; zfuA2_log2 = 2;
			if (ACMAJORREV_2(pi->pubpi->phy_rev) &&
				(RADIOID(pi->pubpi->radioid) == BCM2069_ID) &&
				(RADIOREV(pi->pubpi->radiorev) == 0x2C) && PHY_XTAL_IS40M(pi)) {
				/* see http://confluence.broadcom.com/x/AljxEQ */
				mluA2_log2 = 3;
				mluA1_log2 = 3;
			}
		}
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			mluA1 = 2; mluA2 = 0; zfuA1 = 2; zfuA2 = 2;
		}
	}

	/* Increase Channel Update ML mu */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) && (ACMINORREV_0(pi) || ACMINORREV_1(pi))) {
		/* 4360 a0,b0 */
		MOD_PHYREG(pi, mluA, mluA1, mluA1);
		MOD_PHYREG(pi, mluA, mluA2, mluA2);
		/* zfuA register used to update channel for 256 QAM */
		MOD_PHYREG(pi, zfuA, zfuA1, zfuA1);
		MOD_PHYREG(pi, zfuA, zfuA2, zfuA2);
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* 4350 a0,b0 (log domain) */
		MOD_PHYREG(pi, mluA, mluA1, mluA1_log2);
		MOD_PHYREG(pi, mluA, mluA2, mluA2_log2);
		/* zfuA register used to update channel for 256 QAM */
		MOD_PHYREG(pi, zfuA, zfuA1, zfuA1_log2);
		MOD_PHYREG(pi, zfuA, zfuA2, zfuA2_log2);
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, mluA, mluA0, mluA0);
		MOD_PHYREG(pi, mluA, mluA1, mluA1);
		MOD_PHYREG(pi, mluA, mluA2, mluA2);
		MOD_PHYREG(pi, mluA, mluA3, mluA3);
		/* zfuA register used to update channel for 256 QAM */
		MOD_PHYREG(pi, zfuA, zfuA0, zfuA0);
		MOD_PHYREG(pi, zfuA, zfuA1, zfuA1);
		MOD_PHYREG(pi, zfuA, zfuA2, zfuA2);
		MOD_PHYREG(pi, zfuA, zfuA3, zfuA3);
	} else {
	}
}

void
phy_ac_chanmgr_cal_init(phy_info_t *pi, uint8 *enULB)
{
#ifdef WL11ULB
	/* Disable CRS min pwr cal in ULB mode
	 * For ULB mode, do crs cal at the end of the function
	 */
	if (PHY_ULB_ENAB(pi)) {
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS10(pi->radio_chanspec) ||
					CHSPEC_IS5(pi->radio_chanspec) ||
					CHSPEC_IS2P5(pi->radio_chanspec)) {
				*enULB = 1;
				/* Disable ULB mode during cals */
				wlc_phy_ulb_mode(pi, PMU_ULB_BW_NONE);
			}
		}
	}
#endif /* WL11ULB */
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (!PHY_IPA(pi)) &&
	    (CHSPEC_IS80(pi->radio_chanspec)) &&
	    (pi->u.pi_acphy->chanmgri->cfg->srom_txnoBW80ClkSwitch == 0)) {
			wlc_phy_modify_txafediv_acphy(pi, 6);
	}
}

void
phy_ac_chanmgr_cal_reset(phy_info_t *pi)
{
	uint8 ch = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint32 fc = wf_channel2mhz(ch, CHSPEC_IS5G(pi->radio_chanspec)
			    ? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	if (ACMAJORREV_2(pi->pubpi->phy_rev) && (!PHY_IPA(pi)) &&
		(pi->u.pi_acphy->chanmgri->cfg->srom_txnoBW80ClkSwitch == 0)) {
		wlc_phy_afeclkswitch_sifs_delay(pi);
		if (!(fc == 5210 || fc == 5290) && (CHSPEC_IS80(pi->radio_chanspec)))
			wlc_phy_modify_txafediv_acphy(pi, 9);
		else if (CHSPEC_IS80(pi->radio_chanspec))
			wlc_phy_modify_txafediv_acphy(pi, 6);
	}
}

#if defined(WLTEST)
static int
phy_ac_chanmgr_get_smth(phy_type_chanmgr_ctx_t *ctx, int32* ret_int_ptr)
{
	phy_ac_chanmgr_info_t *ci = (phy_ac_chanmgr_info_t *)ctx;
	if ((ACMAJORREV_1(ci->pi->pubpi->phy_rev) && ACMINORREV_2(ci->pi)) ||
	    ACMAJORREV_3(ci->pi->pubpi->phy_rev) || ACMAJORREV_4(ci->pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(ci->pi->pubpi->phy_rev) || ACMAJORREV_37(ci->pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(ci->pi->pubpi->phy_rev)) {
		*ret_int_ptr = ci->acphy_enable_smth;
		return BCME_OK;
	} else {
		PHY_ERROR(("Smth is not supported for this chip \n"));
		return BCME_UNSUPPORTED;
	}
}

static int
phy_ac_chanmgr_set_smth(phy_type_chanmgr_ctx_t *ctx, int8 int_val)
{
	phy_ac_chanmgr_info_t *ci = (phy_ac_chanmgr_info_t *)ctx;
	if ((ACMAJORREV_1(ci->pi->pubpi->phy_rev) && ACMINORREV_2(ci->pi)) ||
	    ACMAJORREV_3(ci->pi->pubpi->phy_rev) || ACMAJORREV_4(ci->pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(ci->pi->pubpi->phy_rev) || ACMAJORREV_37(ci->pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(ci->pi->pubpi->phy_rev)) {
		if (((int_val > SMTH_FOR_TXBF) || (int_val < SMTH_DISABLE)) ||
			PHY_AS_80P80(ci->pi, ci->pi->radio_chanspec)) {
			PHY_ERROR(("Smth %d is not supported \n", (uint16)int_val));
			return BCME_UNSUPPORTED;
		} else {
			wlc_phy_smth(ci->pi, int_val, SMTH_NODUMP);
			return BCME_OK;
		}
	} else {
		PHY_ERROR(("Smth is not supported for this chip \n"));
		return BCME_UNSUPPORTED;
	}
}
#endif /* defined(WLTEST) */

bool
phy_ac_chanmgr_get_val_nonbf_logen_mode(phy_ac_chanmgr_info_t *chanmgri)
{
	return chanmgri->cfg->srom_nonbf_logen_mode_en;
}

/* sets chanspec of scan core; returns error status */
int
phy_ac_chanmgr_set_val_sc_chspec(phy_ac_chanmgr_info_t *chanmgri, int32 set_val)
{
	chanmgri->radio_chanspec_sc = (chanspec_t)set_val;

	return BCME_OK;
}

/* gets scan core chanspec in pointer ret_val parameter passed; returns error status */
int
phy_ac_chanmgr_get_val_sc_chspec(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val)
{
	*ret_val = (int32)chanmgri->radio_chanspec_sc;
	return BCME_OK;
}

/* sets phymode; eg. to 3+1 or 4x4; returns error status */
int
phy_ac_chanmgr_set_val_phymode(phy_ac_chanmgr_info_t *chanmgri, int32 set_val)
{
	phy_info_t *pi = chanmgri->pi;
#ifdef WL_AIR_IQ
	if ((phy_get_phymode(pi) == PHYMODE_BGDFS && set_val != PHYMODE_BGDFS) ||
		(phy_get_phymode(pi) != PHYMODE_BGDFS && set_val == PHYMODE_BGDFS)) {
		wlc_phy_save_regs_3plus1(pi, set_val);
	}
#endif /* WL_AIR_IQ */
	phy_set_phymode(pi, (uint16) set_val);
	phy_ac_chanmgr_set_phymode(pi, pi->radio_chanspec,
			chanmgri->radio_chanspec_sc, (uint16) set_val);
	return BCME_OK;
}

/* gets phymode in pointer ret_val parameter passed; returns error status */
int
phy_ac_chanmgr_get_val_phymode(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val)
{
	phy_info_t *pi = chanmgri->pi;

	*ret_val = (int32) phy_get_phymode(pi);
	return BCME_OK;
}

static void
phy_ac_chanmgr_set_spexp_matrix(phy_info_t *pi)
{
	uint32 svmp_start_addr = 0x1000;
	uint32 svmp_m4_idx = 0x4;
	uint32 spexp_offs[3] = {  0, 680, 456+680};
	//uint32 spexp_size[3] = {680, 456, 344};
	uint32 txv_config[3][3] = {{0x1000700, 0x2004008, 0x0000055},
		{0x1000680, 0x2004008, 0x0000039},
		{0x1000480, 0x2004008, 0x000002B}};
	uint32 zeros[5] = {0, 0, 0, 0, 0};
	int16 Q43[2*12] = {591, 0, 591, 0, 591, 0, 591, 0, 591, 0, 0, -591,
		-591, 0, 0, 591, 591, 0, -591, 0, 591, 0, -591, 0};
	//int16 Q43[2*12] = {836, 0, 0, 0, 836, 0, 0, 0, 0, 0, 836, 0,
	//	0, 0, 836, 0, 591, 0, -591, 0, -591, 0, 591, 0};
	int16 Q42[2*8]  = {724, 0, 724, 0, 724, 0, 724, 0, 724, 0, 0, 724, -724, 0, 0, -724};
	int16 Q32[2*6]  = {887, 0, 0, 0, 887, 0, 512, 0, 1024, 0, -512, 0};
	int16 csd_phasor[2*16] = {1024, 0, 946, 392, 724, 724, 392, 946, 0, 1024, -392, 946,
		-724, 724, -946, 392, -1024, 0, -946, -392, -724, -724, -392, -946, 0, -1024,
		392, -946, 724, -724, 946, -392};
	int16 k, m, n, j, ncol, nrow, ntones = 56;
	int16 Qr, Qi, *Q = NULL, csd_idx[3];
	uint32 Qcsd[12], svmp_addr[2] = {0x22552200, 0x0000228E};

	// steering matrix is of size S1.14
	//  Q_4x3 = 1/sqrt(3)*[1, 1, 1; 1, -j, -1; 1, -1, 1; 1, j, -1];
	//  Q_4x2 = 1/sqrt(2)*[1, 1; 1, j; 1, -1; 1, -j];
	//  Q_3x2 = [sqrt(3/4), sqrt(1/4); 0, 1; sqrt(3/4), -sqrt(1/4)];

	// These 3 special spacial expansion matrix are stored in M4
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1,
			0x8000, 32, &svmp_m4_idx);

	for (k = 0; k < 3; k++) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 3,
			svmp_start_addr + spexp_offs[k], 32, &(txv_config[k][0]));
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 5,
			svmp_start_addr + spexp_offs[k] + 3, 32, &zeros);

		switch (k) {
		case 0:
			ncol = 3; nrow = 4; Q = Q43;
			break;
		case 1:
			ncol = 2; nrow = 4; Q = Q42;
			break;
		case 2:
			ncol = 2; nrow = 3; Q = Q32;
			break;
		}

		// initialize csd_idx to start from tone -28
		csd_idx[1] = -28;
		for (j = 0; j < ntones; j++) {
			// csd
			csd_idx[1] = (csd_idx[1]+64) & 0xf;
			csd_idx[0] = (csd_idx[1] << 1) & 0xf;
			csd_idx[2] = (csd_idx[0] + csd_idx[1]) & 0xf;
			for (m = 0; m < nrow; m++) {
				for (n = 0; n < ncol; n++) {
					if (m == 0) {
						Qr = Q[(n*nrow+m)*2];
						Qi = Q[(n*nrow+m)*2 + 1];
					} else {
						Qr = (((Q[(n*nrow+m)*2]*csd_phasor[csd_idx[m-1]*2] -
						Q[(n*nrow+m)*2 + 1]*csd_phasor[csd_idx[m-1]*2+1])
						>> 9) + 1) >> 1;
						Qi = (((Q[(n*nrow+m)*2]*csd_phasor[csd_idx[m-1]*2+1]
						+ Q[(n*nrow+m)*2+1]*csd_phasor[csd_idx[m-1]*2])
						>> 9) + 1) >> 1;
					}
					Qr = (Qr > 0)? Qr : (Qr + (1 << 12));
					Qi = (Qi > 0)? Qi : (Qi + (1 << 12));
					Qcsd[n*nrow + m] = ((Qr & 0xfff) << 4) |
						((Qi & 0xfff) << 20);
				}
			}
			csd_idx[1] += (j == ((ntones >> 1) - 1))? 2: 1;

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, ncol*nrow,
			svmp_start_addr + spexp_offs[k] + 8 + ncol*nrow*j, 32, &Qcsd);
		}
	}

	// writing txbf_user index 0x60, 0x61, 0x62 svmp address
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX, 2, 0x1030, 32, svmp_addr);
}

static int
phy_ac_chanmgr_switch_phymode_acphy(phy_info_t *pi, uint32 phymode)
{
	si_t *sih = pi->sh->sih;
	uint32 prev_phymode = (si_core_cflags(sih, 0, 0) & SICF_PHYMODE) >> SICF_PHYMODE_SHIFT;

	if (phymode != prev_phymode)
		si_core_cflags(sih, SICF_PHYMODE, phymode << SICF_PHYMODE_SHIFT);

	return BCME_OK;
}

/* report virtual core related capabilities. It only hosts 80p80 capability now */
int
phy_ac_chanmgr_get_val_phy_vcore(phy_ac_chanmgr_info_t *chanmgri, int32 *ret_val)
{
	phy_info_t *pi = chanmgri->pi;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);
	bool en_sw_txrxchain_mask =
		((pi->sh->boardflags4 & BFL4_SROM13_EN_SW_TXRXCHAIN_MASK) != 0);

	*ret_val = ((uint16)(PHY_AS_80P80_CAP(pi)) << 12);
	if (ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		*ret_val |= (pi->pubpi->phy_corenum << 8) +
			((stf_shdata->hw_phytxchain & (en_sw_txrxchain_mask ?
				pi->sromi->sw_txchain_mask:0xf)) << 4) +
			(stf_shdata->hw_phyrxchain & (en_sw_txrxchain_mask ?
				pi->sromi->sw_rxchain_mask:0xf));
	} else {
		/* 4349 and other phyrevs with 80p80/160mhz mode can add code here */
	}

	return BCME_OK;
}

static void
wlc_phy_set_hesiga_sigb_pos(phy_info_t *pi)
{
	/* phy_maj44 default RTL has HE SIGA bit field positions based on Draft 0.4 based
	Program the registers so that it is compatible with Draft 0.5 onwards
	*/
	ACPHY_REG_LIST_START
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su0, 0x0002)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su1, 0x0f08)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su2, 0x131a)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su3, 0x0315)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su4, 0x1707)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su5, 0x2321)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su6, 0x0124)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su7, 0x2a27)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su8, 0x222e)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_su9, 0x2925)

		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu0, 0x2600)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu1, 0x0b05)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu2, 0x0f1a)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu3, 0x0117)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu4, 0x0416)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu5, 0x1212)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu6, 0x2225)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu7, 0x2926)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu8, 0x2e2a)
		WRITE_PHYREG_ENTRY(pi, he_siga_position_mu9, 0x1927)

		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_0_0, 0x0f00)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_0_1, 0x0b14)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_0_2, 0x130e)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_1_0, 0x2415)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_1_1, 0x2029)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_su_1_2, 0x2823)

		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_0_0, 0x0f00)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_0_1, 0x0b14)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_0_2, 0x0013)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_1_0, 0x2415)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_1_1, 0x2029)
		WRITE_PHYREG_ENTRY(pi, he_sigb_position_mu_1_2, 0x0028)

	ACPHY_REG_LIST_EXECUTE(pi);

		MOD_PHYREG(pi, HESigSupportCtrlExt, check_he_siga_ldpc, 0);
		MOD_PHYREG(pi, HEDebugTblCtrl, he_debug_log_satus_byte_start, 10);
		MOD_PHYREG(pi, HEDebugTblCtrl, he_debug_logging_en, 1);

}

static void
wlc_phy_set_afediv_reset_bundles(phy_info_t *pi)
{
	uint16 rfseq_bundle_48[3];
	uint16 adcdiv_rxmode, adcdiv_txmode, afebypN2;

	if (pi->pubpi->slice == DUALMAC_MAIN) {
		return;
	}

	MOD_PHYREG(pi, RfctrlCmd, bundleScheme2, 1);
	afebypN2 = 0;
	//# AUX is 20mhz only
	/* As rfctrl_bundle_afe_config_28nm_1 is used for reset_en toggling. */
	/* as per bundle scheme 1, bit #7 is for bundle type, #6-#3 for */
	/* type of command and #2-#0 is for offset */
	/* direct_afediv_adc_outbuf_pu 15	1 */
	/* direct_afediv_dac_outbuf_pu 16	1 */
	/* direct_afediv_adc_en	17	1 */
	/* direct_afediv_dac_en	18	0 */
	/* direct_afediv_adc_div	26:21 */
	/* direct_afediv_adc_bypN2 35 */
	/* afediv_valid	44 */

	if (pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			 adcdiv_rxmode = 12;
			 adcdiv_txmode = 47;
		} else {
			adcdiv_rxmode = 13;
			adcdiv_txmode = 47;
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			adcdiv_rxmode = 12;
			adcdiv_txmode = 12;
		} else {
			adcdiv_rxmode = 13;
			adcdiv_txmode = 13;
		}
	}

	rfseq_bundle_48[0] = (1 << 15);
	rfseq_bundle_48[1] = (adcdiv_rxmode << 5) | (1 << 1) | (1);
	rfseq_bundle_48[2] = (0 << 12) | (afebypN2 << 3);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 24, 48,
		rfseq_bundle_48);

	rfseq_bundle_48[0] = (1 << 15);
	rfseq_bundle_48[1] = (adcdiv_txmode << 5) | (1 << 2) | (1 << 1) | (1);
	rfseq_bundle_48[2] = (0 << 12) | (afebypN2 << 3);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQBUNDLE, 1, 25, 48,
		rfseq_bundle_48);
}

const uint16 *BCMRAMFN(get_rfseq_majrev44)(phy_info_t *pi, phy_ac_rfseq_tbl_id_t tbl_id)
{
	if (pi->pubpi->slice == DUALMAC_AUX) {
		switch (tbl_id) {
			case RESET2RX_SEQ_CMD_TBL:
				return rfseq_majrev44_reset2rx_cmd_aux;
			case RESET2RX_SEQ_DLY_TBL:
				return rfseq_majrev44_reset2rx_dly_aux;
			case TX2RX_SEQ_CMD_TBL:
				return rfseq_majrev44_tx2rx_cmd_aux;
			case TX2RX_SEQ_DLY_TBL:
				return rfseq_majrev44_tx2rx_dly_aux;
			case RX2TX_SEQ_CMD_TBL:
				return rfseq_majrev44_rx2tx_cmd_aux;
			case RX2TX_SEQ_DLY_TBL:
				return rfseq_majrev44_rx2tx_dly_aux;
			case RX2TX_TSSISLEEPEN_SEQ_CMD_TBL:
				return rfseq_majrev44_rx2tx_tssisleep_en_cmd_aux;
			case RX2TX_TSSISLEEPEN_SEQ_DLY_TBL:
				return rfseq_majrev44_rx2tx_tssisleep_en_dly_aux;
			case TX2RX_CAL_SEQ_CMD_TBL:
				return rfseq_majrev44_tx2rx_cal_cmd_aux;
			case TX2RX_CAL_SEQ_DLY_TBL:
				return rfseq_majrev44_tx2rx_cal_dly_aux;
			default:
				return NULL;
		}
	} else {
		switch (tbl_id) {
			case RESET2RX_SEQ_CMD_TBL:
				return rfseq_majrev44_reset2rx_cmd_main;
			case RESET2RX_SEQ_DLY_TBL:
				return rfseq_majrev44_reset2rx_dly_main;
			case TX2RX_SEQ_CMD_TBL:
				return rfseq_majrev44_tx2rx_cmd_main;
			case TX2RX_SEQ_DLY_TBL:
				return rfseq_majrev44_tx2rx_dly_main;
			case RX2TX_SEQ_CMD_TBL:
				return rfseq_majrev44_rx2tx_cmd_main;
			case RX2TX_SEQ_DLY_TBL:
				return rfseq_majrev44_rx2tx_dly_main;
			case RX2TX_TSSISLEEPEN_SEQ_CMD_TBL:
				return rfseq_majrev44_rx2tx_tssisleep_en_cmd_main;
			case RX2TX_TSSISLEEPEN_SEQ_DLY_TBL:
				return rfseq_majrev44_rx2tx_tssisleep_en_dly_main;
			case TX2RX_CAL_SEQ_CMD_TBL:
				return rfseq_majrev44_tx2rx_cal_cmd_main;
			case TX2RX_CAL_SEQ_DLY_TBL:
				return rfseq_majrev44_tx2rx_cal_dly_main;
			default:
				return NULL;
		}
	}
}

static void wlc_phy_set_rfseqext_tbl_majrev40(phy_info_t *pi)
{
	uint8 mode = pi->u.pi_acphy->sromi->srom_low_adc_rate_en;
	uint8 core, idx;
	uint32 read_val[2], write_val[2];

	/* low_adc_rate_en=0/1 */
	if (mode >= 2) {
		PHY_ERROR(("%s: ERROR: Invalid low_adc_rate_en value (%d) failed\n",
			__FUNCTION__, mode));
		ASSERT(0);
		return;
	}

	FOREACH_CORE(pi, core) {
		/* update rfseqext table for afediv configuration */
		/* in rfseqext_[25]g_rev40 table, array index 0 is msb */
		for (idx = 0; idx < RFSEQEXT_TBL_SZ_PER_CORE_28NM; idx++) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (pi->pubpi->slice == DUALMAC_MAIN) {
					write_val[0] = rfseqext_5g_rev40[mode][idx][1];
					write_val[1] = rfseqext_5g_rev40[mode][idx][0];
				} else {
					if (core == 0) {
						write_val[0] =
						    rfseqext_5g_aux0_rev40[mode][idx][1];
						write_val[1] =
						    rfseqext_5g_aux0_rev40[mode][idx][0];
					} else if (core == 1) {
						write_val[0] =
						    rfseqext_5g_aux1_rev40[mode][idx][1];
						write_val[1] =
						    rfseqext_5g_aux1_rev40[mode][idx][0];
					}
				}
			} else {
				if (pi->pubpi->slice == DUALMAC_MAIN) {
					write_val[0] = rfseqext_2g_rev40[mode][idx][1];
					write_val[1] = rfseqext_2g_rev40[mode][idx][0];
				} else {
					if (core == 0) {
						write_val[0] =
						    rfseqext_2g_aux0_rev40[mode][idx][1];
						write_val[1] =
						    rfseqext_2g_aux0_rev40[mode][idx][0];
					} else if (core == 1) {
						write_val[0] =
						    rfseqext_2g_aux1_rev40[mode][idx][1];
						write_val[1] =
						    rfseqext_2g_aux1_rev40[mode][idx][0];
					}
				}
			}
			if ((wlc_phy_ac_phycap_maxbw(pi) > BW_20MHZ)) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				1, core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx, 60, write_val);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				1, core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx, 60, &read_val);
			} else {
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				1, core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx, 42, write_val);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				1, core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx, 42, &read_val);
			}
			PHY_INFORM(("band=%d offset=%d write=0x%08x%08x read=0x%08x%08x\n",
				(CHSPEC_IS2G(pi->radio_chanspec))?2:5,
				core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx,
				write_val[1], write_val[0], read_val[1], read_val[0]));
		}
	}
}

static void wlc_phy_set_rfseqext_tbl_majrev44(phy_info_t *pi)
{
	phy_ac_radio_data_t *radio_data = phy_ac_radio_get_data(pi->u.pi_acphy->radioi);
	uint32 *p_write_val = NULL;
	/* TCL: ocl_rfseq_from_rfseqext */
	uint32 *p_rfseq_val = NULL;
	uint32 rfseq_val = 0;
	uint8 mode = pi->u.pi_acphy->sromi->srom_low_adc_rate_en;
	uint8 core;

	/* low_adc_rate_en=0/1 */
	if (mode >= 2) {
		PHY_ERROR(("%s: ERROR: Invalid low_adc_rate_en value (%d) failed\n",
			__FUNCTION__, mode));
		ASSERT(0);
		return;
	}

	FOREACH_CORE(pi, core) {
		/* update rfseqext table for afediv configuration */
		if (pi->pubpi->slice == DUALMAC_MAIN) {
			/* Setingt the bw 20MHz DAC clock for dac_clk_x2_mode */
			if (radio_data->dac_clk_x2_mode == 1) {
				if (mode == 0) { /* low_adc_rate_en = 0 */
					rfseqext_rev44_main[0][RFSEQEXT_TX20][0] = 0x001E9981;
					rfseqext_rev44_main[0][RFSEQEXT_TX20][1] = 0x060;
					rfseqext_rev44_main[0][RFSEQEXT_RX20][0] = 0x001C9981;
					rfseqext_rev44_main[0][RFSEQEXT_RX20][1] = 0x060;
				} else if (mode == 1) { /* low_adc_rate_en = 1 */
					rfseqext_rev44_main[1][RFSEQEXT_TX20][0] = 0x021E98F1;
					rfseqext_rev44_main[1][RFSEQEXT_TX20][1] = 0x062;
					rfseqext_rev44_main[1][RFSEQEXT_RX20][0] = 0x001C9981;
					rfseqext_rev44_main[1][RFSEQEXT_RX20][1] = 0x060;
				}

			}
			p_write_val = (uint32 *)rfseqext_rev44_main[mode];
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				RFSEQEXT_TBL_SZ_PER_CORE_28NM,
				core*RFSEQEXT_TBL_SZ_PER_CORE_28NM, 60, p_write_val);

			/* TCL: ocl_rfseq_from_rfseqext */
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				p_rfseq_val = (uint32 *) &rfseqext_rev44_main[mode][RFSEQEXT_RX20];
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				p_rfseq_val = (uint32 *) &rfseqext_rev44_main[mode][RFSEQEXT_RX40];
			} else if (CHSPEC_IS80(pi->radio_chanspec)) {
				p_rfseq_val = (uint32 *) &rfseqext_rev44_main[mode][RFSEQEXT_RX80];
			} else {
				/* Other bandwidths are presently not supported */
				ASSERT(0);
			}

		} else {
			/* only ulp_tx_mode=1 table is populated */
			if (radio_data->ulp_tx_mode != 1) {
				PHY_ERROR(("%s: ERROR: Invalid ulp_tx_mode value (%d) failed\n",
					__FUNCTION__, radio_data->ulp_tx_mode));
				ASSERT(radio_data->ulp_tx_mode == 1);
				BCM_REFERENCE(radio_data);
			}

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				p_write_val = (uint32 *)rfseqext_rev44_aux_2g[mode];
				/* AUX is only 20MHz */
				p_rfseq_val = (uint32 *)&rfseqext_rev44_aux_2g[mode][RFSEQEXT_RX20];
			} else {
				p_write_val = (uint32 *)rfseqext_rev44_aux_5g[mode];
				/* AUX is only 20MHz */
				p_rfseq_val = (uint32 *)&rfseqext_rev44_aux_5g[mode][RFSEQEXT_RX20];
			}

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT,
				RFSEQEXT_TBL_SZ_PER_CORE_28NM,
				core*RFSEQEXT_TBL_SZ_PER_CORE_28NM, 60, p_write_val);

		}
		/* TCL: ocl_rfseq_from_rfseqext */
		rfseq_val = (((*p_rfseq_val >> 21) & 0x7ff) | ((*(p_rfseq_val+1)<<11)
			 & 0xF800));
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c1 + core*16, 16,
			&rfseq_val);

		rfseq_val = ((*(p_rfseq_val+1) >> 5) & 0x3fff);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c4 + core*16, 16,
			&rfseq_val);

		rfseq_val = ((*p_rfseq_val) & 0x3e);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c6 + core*16, 16,
			&rfseq_val);

		rfseq_val = ((*p_rfseq_val >> 6) & 0x7fff);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x3c9 + core*16, 16,
			&rfseq_val);
	}
}

static void wlc_phy_set_rfseqext_tbl_majrev47(phy_info_t *pi)
{
	/* TCL: ocl_rfseq_from_rfseqext */
	uint8 mode = pi->u.pi_acphy->sromi->srom_low_adc_rate_en;
	uint8 core;
	uint8 idx;
	uint32 write_val[2];

	/* low_adc_rate_en=0/1 */
	if (mode >= 2) {
		PHY_ERROR(("%s: ERROR: Invalid low_adc_rate_en value (%d) failed\n",
			__FUNCTION__, mode));
		ASSERT(0);
		return;
	}

	FOREACH_CORE(pi, core) {
		/* update rfseqext table for afediv configuration */
		if (pi->pubpi->slice == DUALMAC_MAIN) {
			/* Setingt the bw 20MHz DAC clock for dac_clk_x2_mode */
			for (idx = 0; idx < RFSEQEXT_TBL_SZ_PER_CORE_28NM; idx++) {
				write_val[0] = rfseqext_rev47[mode][idx][1];
				write_val[1] = rfseqext_rev47[mode][idx][0];
				wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQEXT, 1,
				                          core*RFSEQEXT_TBL_SZ_PER_CORE_28NM+idx,
				                          60, write_val);
			}
		}
	}
}

static void wlc_phy_td_sfo_eco(phy_info_t *pi)
{
	uint64 tdsfo_neg_freq_bias, rxfarr_ratio, bw5;
	uint32 td_sfo_be;
	uint16 rf_updh_cmd[] = {0x85, 0x2a, 0x07, 0x0a, 0x00, 0x08, 0x2b, 0x86, 0x1f};
	uint16 rf_updh_dly[] = {0x02, 0x01, 0x02, 0x02, 0x02, 0x10, 0x01, 0x02, 0x01};
	uint16 rf_updh_cmd_en_maj47[] = {0x85, 0x2a, 0x07, 0x0a, 0x00, 0x08, 0x2b, 0x86, 0x1f};
	uint16 rf_updh_dly_en_maj47[] = {0x02, 0x01, 0x02, 0x02, 0x02, 0x10, 0x01, 0x02, 0x01};
	uint16 rf_updh_cmd_dis_maj47[] = {0x07, 0x0a, 0x00, 0x08, 0x2a, 0x2b, 0x1f, 0x1f, 0x1f};
	uint16 rf_updh_dly_dis_maj47[] = {0x02, 0x02, 0x02, 0x01, 0x04, 0x01, 0x01, 0x01, 0x01};

	uint16 fcw_hi, fcw_lo;
	uint16 rx1Spare_val;

	if (ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
		MOD_PHYREG(pi, td_sfo_be_corr, td_sfo_be_enable,
			(pi->u.pi_acphy->td_sfo_corr_en != 0));
		MOD_PHYREG(pi, td_sfo_corr, td_sfo_corr_en,
			(pi->u.pi_acphy->td_sfo_corr_en != 0));
		MOD_PHYREG(pi, td_sfo_corr, td_sfo_corr_use_fineCFOatHTAGC, 0);
		MOD_PHYREG(pi, PhaseTrackOffset, tdSfoAdvRetAdj, 1);

		fcw_hi = READ_PHYREG(pi, rxFarrowDeltaPhase_hi);
		fcw_lo = READ_PHYREG(pi, rxFarrowDeltaPhase_lo);
		rxfarr_ratio = (1 << N_BITS_RX_FARR) + ((fcw_hi & 0xFF) << 16) +
			((fcw_lo & 0xFFFF));

		/* Extra 1 bit left shift to help in rounding */
		math_uint64_divide(&td_sfo_be, (uint32)1 << 7, (uint32) 0, (uint32)rxfarr_ratio);
		td_sfo_be = (td_sfo_be + 1) >> 1;
		MOD_PHYREG(pi, td_sfo_be_corr, td_sfo_be, td_sfo_be & 0x7FFF);

		/* Reduce pktgain set len to compensate extra latency from farrow reset */
		/* Only needed for phybw = 160M */
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			if (pi->u.pi_acphy->td_sfo_corr_en) {
				WRITE_PHYREG(pi, pktgainSettleLen, 0x30);
			} else {
				WRITE_PHYREG(pi, pktgainSettleLen, 0x40);
			}
		}

		/* tdsfo is causing rx badfcs after receing random number of pkts, farrow reset
		 * at the end of each pkt is a WAR. UPDATEGAINH is called at the end of each pkt,
		 * so inserting farrow reset commands in UPDATEGAINH sequence.
		 * td_sfo_corr_en == 1 ==> TDSFO SW war
		 * td_sfo_corr_en == 2 ==> TDSFO HW reset (B0 cannot work with this mode)
		 */
		if (pi->u.pi_acphy->td_sfo_corr_en == 1) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8,
				0x30, 16, rf_updh_cmd_en_maj47);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8,
				0xa0, 16, rf_updh_dly_en_maj47);
		} else {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8,
				0x30, 16, rf_updh_cmd_dis_maj47);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8,
				0xa0, 16, rf_updh_dly_dis_maj47);
		}
		/* Need TD-SFO perpkt reset if TD-SFO is enabled */
		MOD_PHYREG(pi, td_sfo_corr, td_sfo_perpktrst_en,
			pi->u.pi_acphy->td_sfo_corr_en == 2);

		/* Enable tdsfo_reset_all bit from B1 fixes */
		if (ACMINORREV_GE(pi, 2)) {
			rx1Spare_val = READ_PHYREG(pi, rx1Spare);
			rx1Spare_val = rx1Spare_val | 0x4;
			MOD_PHYREG(pi, rx1Spare, Spare, rx1Spare_val);
		}
	} else {
		MOD_PHYREG(pi, td_sfo_corr_en, td_sfo_corr_en, pi->u.pi_acphy->td_sfo_corr_en);
	}

	if (ACMAJORREV_44_46(pi->pubpi->phy_rev) && (pi->pubpi->slice == DUALMAC_MAIN) &&
		pi->u.pi_acphy->td_sfo_corr_en) {

		/* tdsfo eco is causing rx badfcs after receing random number of pkts, farrow reset
		 * at the end of each pkt is a WAR. UPDATEGAINH is called at the end of each pkt,
		 * so inserting farrow reset commands in UPDATEGAINH sequence.
		 */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0x30, 16, rf_updh_cmd);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 8, 0xa0, 16, rf_updh_dly);

		/* HW-1192
		 * BW5 = round(20.0*2^17/fc);
		 * rxfarr_ratio = fc / (afe_div_ratio * 2 * phy_bw) is in 1.24 bit format;
		 * tdsfo_neg_freq_bias = round(2^7*(rxfarr_ratio/2^24)*BW5)
		 *                     = round(rxfarr_ratio*BW5/2^17)
		 */

		fcw_hi = READ_PHYREG(pi, rxFarrowDeltaPhase_hi);
		fcw_lo = READ_PHYREG(pi, rxFarrowDeltaPhase_lo);
		rxfarr_ratio = (1 << N_BITS_RX_FARR) + ((fcw_hi & 0xFF) << 16) +
			((fcw_lo & 0xFFFF));

		bw5 = READ_PHYREG(pi, BW5);

		/* Extra 1 bit left shift to help in rounding */
		tdsfo_neg_freq_bias = ((bw5 * rxfarr_ratio) >> 16);
		/* Add 1 and right shift by 1 for rounding */
		tdsfo_neg_freq_bias = (tdsfo_neg_freq_bias + 1) >> 1;

		WRITE_PHYREG(pi, rx1Spare, tdsfo_neg_freq_bias & 0xFFFF);
		MOD_PHYREG(pi, RadarThresh1_core1, radarThd1_core1,
			(tdsfo_neg_freq_bias >> 16) & 0xFF);
	}
}

int
phy_ac_chanmgr_force_td_sfo(phy_info_t *pi, bool set, uint16 val)
{
	int ret = 0;

	if (set) {
		pi->u.pi_acphy->td_sfo_corr_en = val;
		wlc_phy_td_sfo_eco(pi);
	} else {
		ret = pi->u.pi_acphy->td_sfo_corr_en;
	}
	return ret;
}

static void
phy_ac_chanmgr_tdcs_enable_160m(phy_info_t *pi, bool set_val)
{
	uint8 ch = wf_chspec_ctlchan(pi->radio_chanspec);

	if (!pi->sh->up)
		return;
	if (!(ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)))
		return;
	if (pi->u.pi_acphy->chanmgri->tdcs_160_en == -1) {
		if (CHSPEC_IS160(pi->radio_chanspec)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			MOD_PHYREG(pi, chnsmCtrl0, chan_smooth_enable, 0);
			MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 0);
			wlc_phy_resetcca_acphy(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		return;
	} else {
		pi->u.pi_acphy->chanmgri->tdcs_160_en = set_val ? 1 : 0;
	}

	if (CHSPEC_IS160(pi->radio_chanspec)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		if (!set_val) {
			MOD_PHYREG(pi, chnsmCtrl0, chan_smooth_enable, 0);
			MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 0);
		} else {
			MOD_PHYREG(pi, chnsmCtrl0, chan_smooth_enable, 1);
			switch (ch) {
				case 36:
				case 100:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 1);
					break;
				case 40:
				case 104:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 2);
					break;
				case 44:
				case 108:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 4);
					break;
				case 48:
				case 112:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 8);
					break;
				case 52:
				case 116:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 16);
					break;
				case 56:
				case 120:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 32);
					break;
				case 60:
				case 124:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 64);
					break;
				case 64:
				case 128:
					MOD_PHYREG(pi, RxFrontEndDebug2, forceFrontClass, 128);
					break;
			}
		}
		wlc_phy_resetcca_acphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
}

int
phy_ac_chanmgr_force_tdcs_160m(phy_info_t *pi, bool set, int8 val)
{
	/* wl tdcs_160m = 1  (default, enable TDCS and set front class.)
	   wl tdcs_160m = 0  (disable TDCS and reset front class.)
	   wl tdcs_160m = -1 (always disable TDCS for 160M even when 160M STA disassoc.)
	 */
	if (set) {
		if (val != 1 && val != 0 && val != -1) {
			return 0;
		}
		pi->u.pi_acphy->chanmgri->tdcs_160_en = val;
		if (val == 1) {
			phy_ac_chanmgr_tdcs_enable_160m(pi, TRUE);
		} else if (val == 0 || val == -1) {
			phy_ac_chanmgr_tdcs_enable_160m(pi, FALSE);
		}
		return 0;
	} else {
		return pi->u.pi_acphy->chanmgri->tdcs_160_en;
	}
}

int
phy_ac_chanmgr_iovar_get_chanup_ovrd(phy_info_t *pi, int32 *ret_val)
{
	*ret_val = (int32)pi->u.pi_acphy->chanmgri->chanup_ovrd;

	return BCME_OK;
}

int
phy_ac_chanmgr_iovar_set_chanup_ovrd(phy_info_t *pi, int32 set_val)
{
	if ((set_val < -2) || (set_val > 1))
		return BCME_ERROR;

	pi->u.pi_acphy->chanmgri->chanup_ovrd = set_val;
	if (set_val == -1) {
	} else if (set_val == 0) {
		WRITE_PHYREG(pi, mluA, 0x0004);
		WRITE_PHYREG(pi, zfuA, 0x0004);
	} else if (set_val == 1) {
		WRITE_PHYREG(pi, mluA, 0x2024);
		WRITE_PHYREG(pi, zfuA, 0x2224);
	}

	return BCME_OK;
}

static void wlc_phy_ul_mac_aided(phy_info_t *pi)
{
	/* enable/disable mac-aided mode without timing forcing */
	/* Note: need uCode to set bit 8 of 'wl ucflags 3'
	 * in order to have 'has_trigger_info' in txctl
	 */
	if ((ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ||
			ACMAJORREV_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, Config0_MAC_aided_trig_frame,
			mac_aided_triggered_rx, pi->u.pi_acphy->ul_mac_aided_en);
		MOD_PHYREG(pi, Config2_MAC_ided_trig_frame,  wait_deaf_time, 0x2da);
		MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 0x190);
		MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 0x460);
		MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 0x640);
	}
}

int
phy_ac_chanmgr_enable_mac_aided(phy_info_t *pi, bool set, uint16 val)
{
	int ret = 0;
	/* printf("mac_aided_init: ul_mac_aided_en=%x, ul_mac_aided_timint_en=%x\n",
		pi->u.pi_acphy->ul_mac_aided_en, pi->u.pi_acphy->ul_mac_aided_timing_en);
		*/

	if (set) {
		/* enable/disable mac-aided mode without timing forcing */
		pi->u.pi_acphy->ul_mac_aided_en = val;
		wlc_phy_ul_mac_aided(pi);
	} else {
		ret = pi->u.pi_acphy->ul_mac_aided_en;
	}
	return ret;
}

static void wlc_phy_ul_mac_aided_timing(phy_info_t *pi)
{
	/* set the timing forcing registers for mac-aided mode */
	/* Note: Add IFS of 16 us tuned with 4375.
	 * 43684 register settings are based on current IFS (~12us).
	 * Need to redo the tunning after the IFS is fixed.
	 */

	if ((ACMAJORREV_47(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ||
			ACMAJORREV_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, Config0_MAC_aided_trig_frame, en_mac_aided_crs,
				pi->u.pi_acphy->ul_mac_aided_en);
		MOD_PHYREG(pi, Config0_MAC_aided_trig_frame, en_mac_aided_cstr,
				pi->u.pi_acphy->ul_mac_aided_en);
		MOD_PHYREG(pi, Config0_MAC_aided_trig_frame, en_mac_aided_fstr,
				pi->u.pi_acphy->ul_mac_aided_en);

		if (pi->u.pi_acphy->ul_mac_aided_timing_en == 1) {
			switch (pi->bw) {
	                // 4375 STA IFS 16 us
			case WL_CHANSPEC_BW_20:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 1030);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1750);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2230);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 920);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 1150);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1310);
			    break;
			case WL_CHANSPEC_BW_40:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 990);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1710);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2190);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 880);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 1110);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1270);
			    break;
			case WL_CHANSPEC_BW_80:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 970);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1690);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2170);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 860);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 1090);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1250);
			    break;
			case WL_CHANSPEC_BW_160:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 970);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1690);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2170);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 860);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 1090);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1250);
			    break;
			default:
			    PHY_ERROR(("%s: Unknow BW=%d\n",
			    __FUNCTION__, pi->bw));
			    ASSERT(0);
			}
		}
		if (pi->u.pi_acphy->ul_mac_aided_timing_en == 2) {
			switch (pi->bw) {
	                // 43684 STA IFS ~12 us
			case WL_CHANSPEC_BW_20:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 900);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1620);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2100);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 790);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 1020);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1180);
			    break;
			case WL_CHANSPEC_BW_40:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 820);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1540);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 2020);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 710);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 940);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1100);
			    break;
			case WL_CHANSPEC_BW_80:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 740);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1460);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 1940);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 630);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 860);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1030);
			    break;
			case WL_CHANSPEC_BW_160:
			    MOD_PHYREG(pi, Config2_MAC_ided_trig_frame, wait_deaf_time, 20);
			    MOD_PHYREG(pi, Config3_MAC_aided_trig_frame, wait_ofdm_time, 780);
			    MOD_PHYREG(pi, Config4_MAC_aided_trig_frame, wait_he_time, 1500);
			    MOD_PHYREG(pi, Config5_MAC_aided_trig_frame, wait_trig_time, 1980);
			    MOD_PHYREG(pi, Config6_MAC_aided_trig_frame, mac_aided_crs_time, 670);
			    MOD_PHYREG(pi, Config7_MAC_aided_trig_frame, mac_aided_cstr_time, 900);
			    MOD_PHYREG(pi, Config8_MAC_aided_trig_frame, mac_aided_fstr_time, 1050);
			    break;
			default:
			    PHY_ERROR(("%s: Unknow BW=%d\n",
			    __FUNCTION__, pi->bw));
			    ASSERT(0);
			}
		}
	}
}

int
phy_ac_chanmgr_enable_mac_aided_timing(phy_info_t *pi, bool set, uint16 val)
{
	int ret = 0;
	/* wl ul_mac_aided_timing 1  (IFS ~16 us)
	   wl ul_mac_aided_timing 2  (IFS ~12 us for 43684)
	 */
	if (set) {
		/* enable/disable mac-aided mode with timing forcing */
		if (val > 0) {
			pi->u.pi_acphy->ul_mac_aided_en = 1;
		} else {
			pi->u.pi_acphy->ul_mac_aided_en = 0;
		}
		wlc_phy_ul_mac_aided(pi);
		pi->u.pi_acphy->ul_mac_aided_timing_en = val;
		wlc_phy_ul_mac_aided_timing(pi);
	} else {
		ret = pi->u.pi_acphy->ul_mac_aided_timing_en;
	}
	return ret;
}

static void wlc_phy_set_papdlut_dynradioregtbl_majrev_ge40(phy_info_t *pi)
{
	MOD_PHYREG(pi, dyn_radioa0, dyn_radio_ovr0, 0x0);
	MOD_PHYREG(pi, dyn_radioa1, dyn_radio_ovr1, 0x0);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPDLUTSELECT0,
	128, 0, 8, papdluttbl);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_PAPDLUTSELECT1,
	128, 0, 8, papdluttbl);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_DYNRADIOREGTBL0,
	2, 0, 32, dynradioregtbl);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_DYNRADIOREGTBL1,
	2, 0, 32, dynradioregtbl);
	MOD_PHYREG(pi, fineclockgatecontrol, forcetxgatedClksOn,
	0x1);
	MOD_PHYREG(pi, TxPwrCtrlBaseIndex, loadPwrIndex, 0x0);
	MOD_PHYREG(pi, TxPwrCtrlBaseIndex, loadPwrIndex, 0x1);
	MOD_PHYREG(pi, TxPwrCtrlBaseIndex, loadPwrIndex, 0x0);
	MOD_PHYREG(pi, fineclockgatecontrol, forcetxgatedClksOn,
	0x0);
}
#ifdef WL_AIR_IQ
static void
wlc_phy_save_regs_3plus1(phy_info_t *pi, int32 set_val)
{
	uint8 stall_val = 0;
	uint8 orig_rxfectrl1 = 0;

	if (!(ACMAJORREV_33(pi->pubpi->phy_rev))) {
		/* only supported for REV_33 */
		return;
	}
	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* Disable stalls and hold FIFOs in reset */
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	orig_rxfectrl1 = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
	ACPHY_DISABLE_STALL(pi);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);

	if (set_val == PHYMODE_BGDFS) {
		/* Save the regs that are used in 3+1 mode */
		wlc_phy_radio20693_save_3plus1(pi, 3, 0);
	} else {
		/* Restore the 3plus1 modified regs */
		wlc_phy_radio20693_save_3plus1(pi, 3, 1);
	}
	/* Restore FIFO reset and Stalls */
	ACPHY_ENABLE_STALL(pi, stall_val);
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, orig_rxfectrl1);
	OSL_DELAY(1);

	/* Reset PHY */
	wlc_phy_resetcca_acphy(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* WL_AIR_IQ */
#endif /* (ACCONF != 0) || (ACCONF2 != 0) */
