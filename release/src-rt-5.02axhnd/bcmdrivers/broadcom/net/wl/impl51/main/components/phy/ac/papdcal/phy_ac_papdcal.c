/*
 * ACPHY PAPD CAL module implementation
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
 * $Id: phy_ac_papdcal.c 735703 2017-12-12 05:40:37Z $
 */
#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_cache.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_btcx.h>
#include <phy_type_papdcal.h>
#include <phy_ac.h>
#include <phy_ac_papdcal.h>
#include <phy_ac_calmgr.h>
#include <phy_ac_info.h>
#include <phy_ac_misc.h>
#include <phy_ac_radio.h>
#include <phy_ac_tbl.h>
#include <wlc_phy_shim.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_radioreg_20695.h>
#include <qmath.h>
#include "phy_ac_papdcal_data.h"
#include <d11.h>
#include <hndd11.h>
#include <fcbs.h>
#include <phy_rstr.h>
#include <phy_stf.h>
#include <sbchipc.h>

uint16 papd_gainctrl_pga[PHY_CORE_MAX];

/* MACROS */
#define WBPAPD_REFDB_BASE	6872
#define WBPAPD_REFDB_STEP	102

/* module PARAM structure */
typedef struct {
	uint16	buf_offset_2g;
	uint16	delayfilt_gamma_2g;
	uint16	cal_refdb_2g;
	uint16  epsilon_offset_2g;
	uint8  bbmult_2g;
	uint8   rx_atten_2g;
	uint8   rx_atten_5g;
	uint8	tx_atten_2g;
	uint8	tx_atten_5g;
	uint8	papd_calidx_2g;
	uint8	papd_calidx_5g;
	uint8	papd_iter;
	uint8	papd_cal_settle;
	uint8	papd_cal_corr;
	uint8	papd_corr_shift;
	uint8	tia_mode_2g;
	uint8	cal_dac_mode;
	uint8	cal_adc_mode;
	uint32  wbcal_lutstep_dB;
	uint8   wbcal_lut_len;
	uint32  wbcal_start;
	uint32  wbcal_end;
	uint32  wbcal_scale_start;
	uint32  wbcal_scale_end;
	uint8   wbcal_gfrac_bits;
	uint8   wbcal_dcc_en;
	uint8   wbcal_dly_filt_en;
	uint8   wbcal_dccorr_ovr_en;
	uint8   wbcal_dcoffset_real;
	uint8   wbcal_dcoffset_imag;
	uint8   wbcal_twotbl_en;
	uint8   wbcal_eps_fxdpt;
	uint16  wbcal_phyreg0_val;
	uint8   eps_stop_idx;
	uint16  wbcal_const_pow_scale;
	uint32  wbcal_macbuf_offset;
	uint16  wbcal_waveform_sz;
	uint8	tia_mode_5g;
	uint16  bbmult_5g;
	uint16  epsilon_offset_5g;
	uint16	cal_refdb_5g;
	uint16	buf_offset_5g;
	uint16	delayfilt_gamma_5g;
	fcbs_tuples_t *ft;
} phy_ac_papdcal_params_t;

typedef struct _phy_ac_papdcal_radioregs {
	uint16 adc_cfg10[PHY_CORE_MAX];
	uint16 adc_ovr1[PHY_CORE_MAX];
} phy_ac_papdcal_radioregs_t;

/* module private states */
struct phy_ac_papdcal_info {
	phy_info_t			*pi;
	phy_ac_info_t		*aci;
	phy_papdcal_info_t	*cmn_info;
	phy_ac_papdcal_params_t *papd_params;
	phy_ac_papdcal_radioregs_t *papd_radioregs;
	int16	acphy_papd_epsilon_offset[PHY_CORE_MAX];
	uint16	papd_cal_time;
	uint8	papdmode;
	int8	pacalshift2g[3];
	int8	pacalshift5g[3];
	int8	pacalindex2g;
	int8	pacalindex5g[3];
	int8	pacalpwr2g;
	int8	pacalpwr5g[8];
	int8	pacalpwr5g40[8];
	int8	pacalpwr5g80[8];
	int8	parfps2g;
	int8	parfps5g;
	int8	papdbbmult2g;
	int8	papdbbmult5g;
	int8	pacalmode;
	int8	pacalopt;
	int8	patoneidx2g;
	int8	patoneidx5g[4];
	int8	pacalshift5ga0[12];
	int8	pacalshift5ga1[12];
	uint8   papdpwrctrl;
	uint8   edpdcalset;
	uint8	acphy_papd_skip;		/* skip papd calibration for IPA case */
	uint8	srom_pagc2g;			/* iPa Pa gain override */
	uint8	srom_pagc2g_ovr;
	uint8	srom_pagc5g;
	uint8	srom_pagc5g_ovr;
	int8	papd_lut0_cal_idx;		/* PAPD index for lut0 */
	int8	papd_lut1_cal_idx;		/* PAPD index for lut1 */
	int8	pacalidx_iovar;			/* force papd cal index */
	bool	_apapd;
	bool	perratedpd2g;			/* Per Rate DPD */
	bool	perratedpd5g;
/* add other variable size variables here at the end */
};
#ifdef WL_APAPD
	#define DO_PAPD_GAINCTRL 0
#else
	#define DO_PAPD_GAINCTRL 1
#endif /* WL_PAPD */

/* Analytic PAPD Support */
#ifndef DONGLEBUILD
	#ifndef WL_APAPD
		#define WL_APAPD
		#define APAPD_ENAB(papdcali)	(papdcali->_apapd)
	#endif /* WL_APAPD */
#else
	#ifdef WL_APAPD
		#define APAPD_ENAB(papdcali)	(papdcali->_apapd)
	#else
		#define APAPD_ENAB(papdcali)	(0)
	#endif /* WL_APAPD */
#endif /* !DONGLEBUILD */

static const char BCMATTACHDATA(rstr_pacalshift2g)[] = "pacalshift2g";
static const char BCMATTACHDATA(rstr_pacalshift5g)[] = "pacalshift5g";
static const char BCMATTACHDATA(rstr_pacalindex2g)[] = "pacalindex2g";
static const char BCMATTACHDATA(rstr_pacalindex5g)[] = "pacalindex5g";
static const char BCMATTACHDATA(rstr_pacalpwr2g)[] = "pacalpwr2g";
static const char BCMATTACHDATA(rstr_pacalpwr5g)[] = "pacalpwr5g";
static const char BCMATTACHDATA(rstr_pacalpwr5g40)[] = "pacalpwr5g40";
static const char BCMATTACHDATA(rstr_pacalpwr5g80)[] = "pacalpwr5g80";
static const char BCMATTACHDATA(rstr_parfps2g)[] = "parfps2g";
static const char BCMATTACHDATA(rstr_parfps5g)[] = "parfps5g";
static const char BCMATTACHDATA(rstr_papdbbmult2g)[] = "papdbbmult2g";
static const char BCMATTACHDATA(rstr_papdbbmult5g)[] = "papdbbmult5g";
static const char BCMATTACHDATA(rstr_pacalmode)[] = "pacalmode";
static const char BCMATTACHDATA(rstr_pacalopt)[] = "pacalopt";
static const char BCMATTACHDATA(rstr_patoneidx2g)[] = "patoneidx2g";
static const char BCMATTACHDATA(rstr_patoneidx5g)[] = "patoneidx5g";
static const char BCMATTACHDATA(rstr_pacalshift5ga0)[] = "pacalshift5ga0";
static const char BCMATTACHDATA(rstr_pacalshift5ga1)[] = "pacalshift5ga1";
static const char BCMATTACHDATA(rstr_papdpwrctrl)[] = "papdpwrctrl";
static const char BCMATTACHDATA(rstr_edpdcalset)[] = "edpdcalset";

/* papd params added for 43012 */
/* -- wb cal -- */
static const char BCMATTACHDATA(rstr_wb_tx_attn)[] = "wb_txattn";
static const char BCMATTACHDATA(rstr_wb_rx_attn)[] = "wb_rxattn";
static const char BCMATTACHDATA(rstr_wb_papd_calidx)[] = "wb_papdcalidx";
static const char BCMATTACHDATA(rstr_wb_eps_offset)[] = "wb_eps_offset";
static const char BCMATTACHDATA(rstr_wb_bbmult)[] = "wb_bbmult";
static const char BCMATTACHDATA(rstr_wb_calref_db)[] = "wb_calref_db";
static const char BCMATTACHDATA(rstr_wb_tia_gain_mode)[] = "wb_tia_gain_mode";
static const char BCMATTACHDATA(rstr_wb_txbuf_offset)[] = "wb_txbuf_offset";
static const char BCMATTACHDATA(rstr_wb_frac_del)[] = "wb_frac_del";
static const char BCMATTACHDATA(rstr_wb_g_frac_bits)[] = "wb_g_frac_bits";
/* -- nb cal -- */
static const char BCMATTACHDATA(rstr_nb_tx_attn)[] = "nb_txattn";
static const char BCMATTACHDATA(rstr_nb_rx_attn)[] = "nb_rxattn";
static const char BCMATTACHDATA(rstr_nb_papd_calidx)[] = "nb_papdcalidx";
static const char BCMATTACHDATA(rstr_nb_eps_offset)[] = "nb_eps_offset";
static const char BCMATTACHDATA(rstr_nb_bbmult)[] = "nb_bbmult";
static const char BCMATTACHDATA(rstr_nb_tia_gain_mode)[] = "nb_tia_gain_mode";

/* local functions */
#if defined(WLTEST) || defined(BCMDBG)
static void wlc_phy_epa_dpd_set_acphy(phy_type_papdcal_ctx_t *ctx, uint8 enab_epa_dpd,
	bool in_2g_band);
#endif /* defined(WLTEST) || defined(BCMDBG) */
static void phy_ac_papd_phy_setup(phy_info_t *pi, uint8 core);
static void phy_ac_papd_phy_setup_majorrev36(phy_info_t *pi, uint8 core);
static void phy_ac_papd_phy_setup_majorrev44(phy_info_t *pi);
static void phy_ac_papd_cal(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
                                   uint16 yrefindex, uint16 stopindex);
static void phy_ac_papd_phy_cleanup(phy_info_t *pi, uint8 core);
static void phy_ac_papd_phy_cleanup_majorrev36(phy_info_t *pi, uint8 core);
static void phy_ac_papd_phy_cleanup_majorrev44(phy_info_t *pi);
static void acphy_papd_cal_phyreg_sr(phy_info_t *pi, uint8 core, acphy_rxcal_phyregs_t *porig,
	bool sr);
static void phy_ac_papd_cal_set_tx_gain(phy_info_t *pi, uint8 core, int8 *bbmult, uint8 *calmode);
static void phy_ac_papd_cal_mode0_1(phy_info_t *pi, acphy_papdCalParams_t *calParams,
	uint8 *calmode);
static int phy_ac_wbcal_run(phy_info_t *pi, uint8 core);
static int phy_ac_papd_mac_play(phy_info_t *pi, const uint32* buf, uint16 len, bool start);
static void phy_ac_papd_cal_mode2(phy_info_t *pi, acphy_papdCalParams_t *calParams,
	uint8 papdmode);
static void phy_ac_papd_cal_mode3(phy_info_t *pi, acphy_papdCalParams_t *calParams);
static void phy_ac_papd_cal_eps_calc_tiny(phy_info_t *pi, uint8 core, int8 *bbmult);
void wlc_phy_papd_set_rfpwrlut(phy_info_t *pi);
static int phy_ac_populate_papd_params(phy_ac_papdcal_info_t *papd_info);

static void wlc_phy_txpwr_papd_cal_run_acphy(phy_info_t *pi, uint8 tx_pre_cal_pwr_ctrl_state);
static void wlc_phy_txpwr_papd_cal_run_tiny(phy_info_t *pi, uint8 tx_pre_cal_pwr_ctrl_state);
#ifdef WFD_PHY_LL
static void phy_ac_wd_wfd_ll(phy_type_papdcal_ctx_t *ctx);
static int phy_ac_papdcal_set_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, uint8 int_val);
static int phy_ac_papdcal_get_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, int32 *ret_int_ptr);
#endif /* WFD_PHY_LL */

#if (defined(WLTEST) || defined(WLPKTENG))
static bool wlc_phy_isperratedpden_acphy(phy_type_papdcal_ctx_t *ctx);
static void wlc_phy_perratedpdset_acphy(phy_type_papdcal_ctx_t *ctx, bool enable);
#endif // endif
#if defined(WLTEST)
static int phy_ac_papdcal_get_lut_idx0(phy_type_papdcal_ctx_t *ctx, int32* idx);
static int phy_ac_papdcal_get_lut_idx1(phy_type_papdcal_ctx_t *ctx, int32* idx);
static int phy_ac_papdcal_set_idx(phy_type_papdcal_ctx_t *ctx, int8 idx);
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
static int phy_ac_papdcal_set_skip(phy_type_papdcal_ctx_t *ctx, uint8 skip);
static int phy_ac_papdcal_get_skip(phy_type_papdcal_ctx_t *ctx, int32 *skip);
#endif /* !ATE_BUILD */
#endif // endif

static void phy_ac_papdcal_nvram_attach_old(phy_ac_papdcal_info_t *ac_info);

static void
BCMATTACHFN(phy_ac_papdcal_nvram_attach)(phy_info_t *pi, phy_ac_papdcal_info_t *ac_info)
{
	uint8 i;

	BCM_REFERENCE(rstr_nb_tx_attn);
	BCM_REFERENCE(rstr_nb_rx_attn);
	BCM_REFERENCE(rstr_nb_papd_calidx);
	BCM_REFERENCE(rstr_nb_eps_offset);
	BCM_REFERENCE(rstr_nb_bbmult);
	BCM_REFERENCE(rstr_nb_tia_gain_mode);

	for (i = 0; i < 3; i++) {
		ac_info->pacalshift2g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
			(pi, rstr_pacalshift2g, i, 0));
	}

	if (PHY_BAND5G_ENAB(pi)) {
		for (i = 0; i < 3; i++) {
			ac_info->pacalshift5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalshift5g, i, 0));
			ac_info->pacalindex5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalindex5g, i, -1));
		}

		/* For 4350 */
		for (i = 0; i < 12; i++) {
			ac_info->pacalshift5ga0[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalshift5ga0, i, 0));
			ac_info->pacalshift5ga1[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalshift5ga1, i, 0));
		}
	}

	ac_info->pacalindex2g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_pacalindex2g, -1));
	ac_info->pacalpwr2g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_pacalpwr2g, -99));
	ac_info->papdpwrctrl = (uint8) (PHY_GETINTVAR_DEFAULT(pi, rstr_papdpwrctrl, 0));
	ac_info->edpdcalset = (uint8) (PHY_GETINTVAR_DEFAULT(pi, rstr_edpdcalset, 0));
	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* For 4350, pacalpwr5g = lo, mi, hi, x1, lo, mi, hi, x1 */
		/*                       |    core 0     |     core 1    | */
		for (i = 0; i < 8; i++) {
			ac_info->pacalpwr5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalpwr5g, i, -99));
			ac_info->pacalpwr5g40[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalpwr5g40, i, -99));
			ac_info->pacalpwr5g80[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_pacalpwr5g80, i, -99));
		}
	} else {
		if (PHY_BAND5G_ENAB(pi)) {
			/* For 4345 and others */
			for (i = 0; i < 4; i++) {
				ac_info->pacalpwr5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
					(pi, rstr_pacalpwr5g, i, -99));
			}
		}
	}

	ac_info->parfps2g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_parfps2g, -1));
	ac_info->papdbbmult2g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_papdbbmult2g, -1));
	ac_info->pacalmode = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_pacalmode, -1));
	ac_info->pacalopt = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_pacalopt, -1));
	ac_info->patoneidx2g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_patoneidx2g, -1));

	if (PHY_BAND5G_ENAB(pi)) {
		ac_info->parfps5g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_parfps5g, -1));
		ac_info->papdbbmult5g = (int8) (PHY_GETINTVAR_DEFAULT(pi, rstr_papdbbmult5g, -1));

		for (i = 0; i < 4; i++) {
			ac_info->patoneidx5g[i] = (int8) (PHY_GETINTVAR_ARRAY_DEFAULT
				(pi, rstr_patoneidx5g, i, -1));
		}
	}
}

/* register phy type specific implementation */
phy_ac_papdcal_info_t *
BCMATTACHFN(phy_ac_papdcal_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_papdcal_info_t *cmn_info)
{
	phy_ac_papdcal_info_t *ac_info;
	phy_type_papdcal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_papdcal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if ((ac_info->papd_radioregs =
			phy_malloc(pi, sizeof(phy_ac_papdcal_radioregs_t))) == NULL) {
		PHY_ERROR(("%s: ac_txcal_radioregs_orig malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	phy_ac_papdcal_nvram_attach(pi, ac_info);

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#if defined(WLTEST) || defined(BCMDBG)
	fns.epa_dpd_set = wlc_phy_epa_dpd_set_acphy;
#endif /* defined(WLTEST) || defined(BCMDBG) */
	fns.ctx = ac_info;
#if (defined(WLTEST) || defined(WLPKTENG))
	fns.isperratedpden = wlc_phy_isperratedpden_acphy;
	fns.perratedpdset = wlc_phy_perratedpdset_acphy;
#endif // endif
#if defined(WLTEST)
	fns.get_idx0 = phy_ac_papdcal_get_lut_idx0;
	fns.get_idx1 = phy_ac_papdcal_get_lut_idx1;
	fns.set_idx = phy_ac_papdcal_set_idx;
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
	fns.set_skip = phy_ac_papdcal_set_skip;
	fns.get_skip = phy_ac_papdcal_get_skip;
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WFD_PHY_LL)
	fns.wd_wfd_ll = phy_ac_wd_wfd_ll;
	fns.set_wfd_ll_enable = phy_ac_papdcal_set_wfd_ll_enable;
	fns.get_wfd_ll_enable = phy_ac_papdcal_get_wfd_ll_enable;
#endif /* WFD_PHY_LL */

	/* Populate the PAPD Params */
	if (phy_ac_populate_papd_params(ac_info) != BCME_OK) {
		goto fail;
	}
	/* setup srom cfg */
	phy_ac_papdcal_nvram_attach_old(ac_info);

	if (phy_papdcal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_papdcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

#ifdef WL_APAPD
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		ac_info->_apapd = TRUE;
	} else {
		ac_info->_apapd = FALSE;
	}
#else
	ac_info->_apapd = FALSE;
#endif /* WL_PAPD */
	return ac_info;

	/* error handling */
fail:
	phy_ac_papdcal_unregister_impl(ac_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_papdcal_unregister_impl)(phy_ac_papdcal_info_t *ac_info)
{
	phy_papdcal_info_t *cmn_info;
	phy_info_t *pi;

	if (ac_info == NULL) {
		return;
	}

	cmn_info = ac_info->cmn_info;
	pi = ac_info->pi;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_papdcal_unregister_impl(cmn_info);

	if (ac_info->papd_params) {
		/* Free FCBS tuples memory */
		if (ac_info->papd_params->ft) {
			phy_mfree(pi, ac_info->papd_params->ft, sizeof(fcbs_tuples_t));
		}
		/* Free PAPD params */
		phy_mfree(pi, ac_info->papd_params, sizeof(phy_ac_papdcal_params_t));
	}
	if (ac_info->papd_radioregs != NULL) {
		phy_mfree(pi, ac_info->papd_radioregs, sizeof(phy_ac_papdcal_radioregs_t));
	}
	phy_mfree(pi, ac_info, sizeof(phy_ac_papdcal_info_t));
}

#ifdef WFD_PHY_LL
static void
phy_ac_wd_wfd_ll(phy_type_papdcal_ctx_t *ctx)
{
	phy_ac_papdcal_info_t *papdcali = (phy_ac_papdcal_info_t *)ctx;
	phy_papdcal_data_t *data = papdcali->cmn_info->data;

	/* Be sure there is no cal in progress to enable/disable optimization */
	if (!PHY_PERICAL_MPHASE_PENDING(papdcali->pi)) {
		if (data->wfd_ll_enable != data->wfd_ll_enable_pending) {
			data->wfd_ll_enable = data->wfd_ll_enable_pending;
			if (!data->wfd_ll_enable) {
				/* Force a watchdog CAL when disabling WFD optimization
				 * As PADP CAL has not been executed since a long time
				 * a PADP CAL is executed at the next watchdog timeout
				 */
				papdcali->pi->cal_info->last_cal_time = 0;
			}
		}
	}
}
#endif /* WFD_PHY_LL */

static int
phy_ac_populate_papd_params(phy_ac_papdcal_info_t *papd_info)
{
	phy_ac_papdcal_params_t *papd_params;
	phy_info_t *pi;

	pi = papd_info->pi;

	if ((papd_params = phy_malloc(pi, sizeof(phy_ac_papdcal_params_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if ((papd_params->ft = phy_malloc(pi, sizeof(fcbs_tuples_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* 43012 */
		papd_params->buf_offset_2g = 24;
		papd_params->buf_offset_5g = 24;
		papd_params->papd_iter = 64;
		papd_params->papd_cal_settle = 64;
		papd_params->papd_cal_corr = 64;
		papd_params->papd_corr_shift = 5;

		if (PHY_EPAPD(pi))
			papd_params->cal_dac_mode = 1; /* 360 MHz mode for Papd cal */
		else
			papd_params->cal_dac_mode = 3; /* 180 MHz mode for Papd cal */

		/* Read from NVRAM - Need to change this going forward
		 * Put here as temporary measure for 43012
		 */
		papd_params->tia_mode_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_tia_gain_mode, 0)) & TIA_GAIN_MODE_2G;
		papd_params->tia_mode_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_tia_gain_mode, 0)) & TIA_GAIN_MODE_5G) >> TIA_GAIN_MODE_5G_SHIFT;
		papd_params->epsilon_offset_2g = ((uint32)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_eps_offset, 451)) & PAPD_EPS_OFFSET_2G;
		papd_params->epsilon_offset_5g = (((uint32)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_eps_offset, 451)) & PAPD_EPS_OFFSET_5G) >> PAPD_EPS_OFFSET_5G_SHIFT;
		papd_params->bbmult_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_bbmult, 90)) & PAPD_BBMULT_2G;
		papd_params->bbmult_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_bbmult, 90)) & PAPD_BBMULT_5G) >> PAPD_BBMULT_5G_SHIFT;
		papd_params->rx_atten_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_rx_attn, 1)) & PAPD_RX_ATTN_2G;
		papd_params->rx_atten_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_rx_attn, 3)) & PAPD_RX_ATTN_5G) >> PAPD_RX_ATTN_5G_SHIFT;
		papd_params->tx_atten_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_tx_attn, 1)) & PAPD_TX_ATTN_2G;
		papd_params->tx_atten_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_tx_attn, 3)) & PAPD_TX_ATTN_5G) >> PAPD_TX_ATTN_5G_SHIFT;
		papd_params->papd_calidx_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_papd_calidx, 5)) & PAPD_CAL_IDX_2G;
		papd_params->papd_calidx_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_papd_calidx, 15)) & PAPD_CAL_IDX_5G) >> PAPD_CAL_IDX_5G_SHIFT;
		papd_params->cal_refdb_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_calref_db, 5)) & PAPD_CALREF_DB_2G;
		papd_params->cal_refdb_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
			rstr_wb_calref_db, 15)) & PAPD_CALREF_DB_5G) >> PAPD_CALREF_DB_5G_SHIFT;

		if (WBPAPD_ENAB(pi)) {
			papd_params->cal_adc_mode = 2; /* 180 MHz mode */
			papd_params->wbcal_lutstep_dB = 10240;
			papd_params->wbcal_lut_len = 64;
			papd_params->wbcal_start = 512*4;
			papd_params->wbcal_end   = 143384;
			papd_params->wbcal_scale_start = 600;
			papd_params->wbcal_scale_end   = 1624;
			papd_params->wbcal_gfrac_bits  = 11;
			papd_params->wbcal_dcc_en      = 1;
			papd_params->wbcal_dly_filt_en = 1;
			papd_params->wbcal_dccorr_ovr_en = 0;
			papd_params->wbcal_dcoffset_real = 0;
			papd_params->wbcal_dcoffset_imag = 0;
			papd_params->wbcal_twotbl_en = 0;
			papd_params->wbcal_phyreg0_val = 0xA924;
			papd_params->eps_stop_idx = 38;
			papd_params->wbcal_const_pow_scale = 710/2;
			papd_params->wbcal_macbuf_offset = 65536;
			papd_params->wbcal_waveform_sz = 4000;
			papd_params->buf_offset_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_wb_txbuf_offset, 24)) & PAPD_BUF_OFFSET_2G;
			papd_params->buf_offset_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_wb_txbuf_offset, 0x1800)) & PAPD_BUF_OFFSET_5G) >>
				PAPD_BUF_OFFSET_5G_SHIFT;
			papd_params->delayfilt_gamma_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_wb_frac_del, 90)) & PAPD_FRACDELAY_OFFSET_2G;
			papd_params->delayfilt_gamma_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_wb_frac_del, 90)) & PAPD_FRACDELAY_OFFSET_5G) >>
				PAPD_FRACDELAY_OFFSET_5G_SHIFT;
			/* By default frac bits is 11 */
			papd_params->wbcal_gfrac_bits = ((uint8)PHY_GETINTVAR_DEFAULT(pi,
				rstr_wb_g_frac_bits, 0xBB));
		} else {
			/* In case of NB-PAPD Cal */
			papd_params->cal_adc_mode = 1; /* 43 MHz mode */
			papd_params->tx_atten_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_tx_attn, 1)) & PAPD_TX_ATTN_2G;
			papd_params->tx_atten_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_tx_attn, 3)) & PAPD_TX_ATTN_5G) >> PAPD_TX_ATTN_5G_SHIFT;
			papd_params->rx_atten_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_rx_attn, 1)) & PAPD_RX_ATTN_2G;
			papd_params->rx_atten_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_rx_attn, 3)) & PAPD_RX_ATTN_5G) >> PAPD_RX_ATTN_5G_SHIFT;
			papd_params->papd_calidx_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_papd_calidx, 5)) & PAPD_CAL_IDX_2G;
			papd_params->papd_calidx_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_papd_calidx, 15)) & PAPD_CAL_IDX_5G) >>
				PAPD_CAL_IDX_5G_SHIFT;
			papd_params->epsilon_offset_2g = ((uint32)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_eps_offset, 451)) & PAPD_EPS_OFFSET_2G;
			papd_params->epsilon_offset_5g = (((uint32)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_eps_offset, 451)) & PAPD_EPS_OFFSET_5G) >>
				PAPD_EPS_OFFSET_5G_SHIFT;
			papd_params->bbmult_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_bbmult, 90)) & PAPD_BBMULT_2G;
			papd_params->bbmult_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_bbmult, 90)) & PAPD_BBMULT_5G) >> PAPD_BBMULT_5G_SHIFT;
			papd_params->tia_mode_2g = ((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_tia_gain_mode, 0)) & TIA_GAIN_MODE_2G;
			papd_params->tia_mode_5g = (((uint16)PHY_GETINTVAR_DEFAULT(pi,
				rstr_nb_tia_gain_mode, 0)) & TIA_GAIN_MODE_5G) >>
				TIA_GAIN_MODE_5G_SHIFT;
		}
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		papd_params->rx_atten_2g = 0x3;
		papd_params->rx_atten_5g = 0x3;
		papd_params->tx_atten_2g = 0x3;
		papd_params->tx_atten_5g = 0x2;
	} else {
		papd_params->rx_atten_2g = 0x3;
		papd_params->rx_atten_5g = 0x3;
		papd_params->tx_atten_2g = 0x3;
		papd_params->tx_atten_5g = 0x3;
		papd_params->papd_calidx_2g = PHY_EPAPD(pi) ? 48: 26;

	}

	/* setup ptr */
	papd_info->papd_params = papd_params;

	return (BCME_OK);

fail:
	if (papd_params != NULL)
		phy_mfree(pi, papd_params, sizeof(phy_ac_papdcal_params_t));

	return (BCME_NOMEM);

}

#if (defined(WLTEST) || defined(WLPKTENG))
static bool
wlc_phy_isperratedpden_acphy(phy_type_papdcal_ctx_t *ctx)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (CHSPEC_IS2G(pi->radio_chanspec))
		return (pi->papdcali->data->epacal2g && info->perratedpd2g);
	else
		return (pi->papdcali->data->epacal5g && info->perratedpd5g);
}

static void
wlc_phy_perratedpdset_acphy(phy_type_papdcal_ctx_t *ctx, bool enable)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* Set the new value */
	MOD_PHYREG(pi, PapdEnable0, papd_compEnb0, enable);
}
#endif // endif

#if defined(WLTEST)
static int
phy_ac_papdcal_get_lut_idx0(phy_type_papdcal_ctx_t *ctx, int32* idx)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	*idx = (int32)info->papd_lut0_cal_idx;
	return BCME_OK;
}

static int
phy_ac_papdcal_get_lut_idx1(phy_type_papdcal_ctx_t *ctx, int32* idx)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	*idx = (int32)info->papd_lut1_cal_idx;
	return BCME_OK;
}

static int phy_ac_papdcal_set_idx(phy_type_papdcal_ctx_t *ctx, int8 idx)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	info->pacalidx_iovar = idx;
	return BCME_OK;
}
#endif // endif

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
static int
phy_ac_papdcal_set_skip(phy_type_papdcal_ctx_t *ctx, uint8 skip)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	info->acphy_papd_skip = skip;
	return BCME_OK;
}

static int
phy_ac_papdcal_get_skip(phy_type_papdcal_ctx_t *ctx, int32 *skip)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	*skip = (int32)info->acphy_papd_skip;
	return BCME_OK;
}
#endif /* !ATE_BUILD */
#endif // endif

void
phy_ac_papdcal_get_gainparams(phy_ac_papdcal_info_t *papdcali, uint8 *Gainoverwrite, uint8 *PAgain)
{
	if (PHY_IPA(papdcali->pi)) {
		*Gainoverwrite = (CHSPEC_IS2G(papdcali->pi->radio_chanspec)) ?
			papdcali->srom_pagc2g_ovr :
			papdcali->srom_pagc5g_ovr;
		*PAgain = (CHSPEC_IS2G(papdcali->pi->radio_chanspec)) ?
			papdcali->srom_pagc2g :
			papdcali->srom_pagc5g;
	}
}

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
#define PAPD_GAIN_CTRL
#define ACPHY_SPINWAIT_PAPDCAL			5000000

static int8 gain_ctrl = 0; /* Use this flag for gain control using analytic PAPD */
static const int8 *rfpwrlut_ptr;
static const uint32 *macplay_wfm_ptr;

#ifdef PAPD_GAIN_CTRL
static uint16 phy_ac_papd_gain_ctrl_acradio(phy_info_t *pi, uint16 num_iter, uint8 core,
	uint16 startindex, uint16 yrefindex, uint16 stopindex);
static void phy_ac_papd_write_tx_gain(phy_info_t *pi, uint8 core,
	acphy_txgains_t *target_gain, uint16 * bbmult);
#endif /* PAPD_GAIN_CTRL */

static void phy_ac_papd_radio_lpbk_cleanup_acradio(phy_info_t *pi, uint8 core);
static void phy_ac_papd_radio_lpbk_setup_acradio(phy_info_t *pi, uint16 tx_atten,
	uint16 rx_atten, uint8 core);
static void phy_ac_get_tx_gain(phy_info_t *pi, uint8 core_no, acphy_txgains_t *target_gain);
static void phy_ac_papd_smooth(phy_info_t *pi, uint8 core,
	uint32 winsz, uint32 start, uint32 end);
static void phy_ac_papd_radio_lpbk_setup_tiny(phy_info_t *pi,
	uint16 tx_atten, uint16 rx_atten);
static void phy_ac_papd_radio_setup_28nm(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten,
		bool epapd_flag);
static void phy_ac_papd_radio_lpbk_setup_20697(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten,
		bool epapd_flag);
static void phy_ac_papd_set_wbtia_gain_bw(phy_info_t *pi, uint8 wbtia_mode);
static void phy_ac_papd_rx_gain_ctrl(phy_info_t *pi);
static void phy_ac_papd_radio_lpbk_cleanup_tiny(phy_info_t *pi);
static void phy_ac_papd_radio_lpbk_cleanup_28nm(phy_info_t *pi);
static void phy_ac_papd_radio_lpbk_cleanup_20697(phy_info_t *pi);
static void phy_ac_papd_set_tia_gain_tiny(phy_info_t *pi, uint16 gain, uint8 core);
static void phy_ac_apapd_init_seq(phy_info_t *pi, uint8 core, uint16 yrefindex);
static void phy_ac_papd_turningoff_inactivecore(phy_info_t *pi, uint8 core);
static uint8 phy_ac_papd_gain_ctrl_tiny(phy_info_t *pi, uint8 core, uint16 yrefindex);
static uint8 phy_ac_wbcal_eps_stopidx(phy_info_t *pi, uint8 core);

const uint32 *BCMRAMFN(get_wbpapd_wfm_43012)(phy_info_t *pi);

#ifdef PAPD_GAIN_CTRL
static uint16
phy_ac_papd_gain_ctrl_acradio(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
#define EPS_MAX 4095
#define EPS_MIN -4095
#define GAINCTRL_ITER 8
	bool clipping_flag = 0;
	uint8 i = 0;
	/* Change PRF to avoid radar detection */
	uint16 numidx_array[GAINCTRL_ITER] = {4, 5, 6, 7, 4, 5, 6, 7};
	uint8 PAPD_FAST_GAIN_CTRL = 0;
	uint16 pga_u = 255, pga_l = 0, pga_mid = 127;
	uint32 tempclip = 0;
	uint32 clipthresholdl = 47894000, clipthresholdu = 49034000, clipthreshold = 48462000;
	acphy_txgains_t tx_gains;
	int32 eps_re, eps_im;
	uint16 bbmult;
	uint32 eps_complex;
	uint16 numidx;
	uint8 PAgain = 0xff;
	uint8 Gainoverwrite = 0;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2};
	phy_ac_papdcal_info_t *papdi = pi->u.pi_acphy->papdcali;
	uint8 edpdcalset = papdi->edpdcalset;
	uint8 papdmode = papdi->papdmode;

	if (ACMAJORREV_1(pi->pubpi->phy_rev) ||
		(PHY_IPA(pi) && ACMAJORREV_2(pi->pubpi->phy_rev))) {
		PAPD_FAST_GAIN_CTRL = 1;
		/* 43569 */
		if (RADIOREV(pi->pubpi->radiorev) == 0x27 ||
		RADIOREV(pi->pubpi->radiorev) == 0x29 ||
		RADIOREV(pi->pubpi->radiorev) == 0x28 ||
		RADIOREV(pi->pubpi->radiorev) == 0x2C) {
				PAPD_FAST_GAIN_CTRL = 0;
		}
	}

	bbmult = 0x3f;
	tx_gains.txlpf = 0x0;
	tx_gains.txgm = 0xff;
	tx_gains.pga = 0xff;
	tx_gains.pad = 0xff;
	phy_ac_papdcal_get_gainparams(papdi, &Gainoverwrite, &PAgain);
	tx_gains.ipa = (Gainoverwrite == 0) ? 0xff : PAgain;
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		/* 4350EPAPD */
		tx_gains.txlpf = 0x0;
		tx_gains.txgm = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x67 : 0x91;
		tx_gains.pga = 0xff;
		tx_gains.pad = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xff : 0x7f;
		tx_gains.ipa = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xc0 : 0x3;
	}

	if (PAPD_FAST_GAIN_CTRL) {
		PHY_CAL(("---------- Using Fast Gain Control ------------"));
		if ((papdmode == PAPD_ANALYTIC) || (papdmode == PAPD_ANALYTIC_WO_YREF)) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 59901000;
					clipthresholdu = 61175000;
					clipthreshold = 60536000;
					/* 1.9 */
				} else {
					clipthresholdl = 47894000;
					clipthresholdu = 49034000;
					clipthreshold = 48462000;
					/* 1.7 */
				}
			}
		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				clipthresholdl = 30110000;
				clipthresholdu = 31016000;
				/* clipthreshold 1.35 */
				clipthreshold = 30562000;
			}
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 39255000;
					clipthresholdu = 40288000;
					clipthreshold = 39769000;
				} else {
					clipthresholdl = 32399000;
					clipthresholdu = 33338000;
					clipthreshold = 32867289;
					/* 1.2 */
				}
			}
		}
	} else {
		numidx = 16;
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				bbmult = 0x30;
				if (!CHSPEC_IS80(pi->radio_chanspec)) {
					clipthresholdl = 59901000;
					clipthresholdu = 61175000;
					clipthreshold = 60536000;
				}
			}
		}
	}
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			clipthresholdl = 13886000;
			clipthresholdu = 14504000;
			clipthreshold = 14193000;
			pga_u = 127;
			pga_l = 40;
			pga_mid = 84;
		} else {
			if (edpdcalset == 1) {
			    if (CHSPEC_IS20(pi->radio_chanspec)) {
				    clipthresholdl = 32399000;
				    clipthresholdu = 33338000;
				    clipthreshold = 32867000;
			    } else if (CHSPEC_IS40(pi->radio_chanspec)) {
				    clipthresholdl = 27905000;
				    clipthresholdu = 28777000;
				    clipthreshold = 28340000;
			    } else {
				    clipthresholdl = 19923000;
				    clipthresholdu = 20661000;
				    clipthreshold = 20291000;
			    }
			}
		}
	}
	/* Continuous tone throughout gain control to avoid radar detection */
	(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186, TX_TONE_IQCAL_MODE_OFF,
		FALSE);
	/* Binary search */
	for (i = 1; i <= GAINCTRL_ITER; i++) {
		/* For fast papd numidx value is changed per iteration otherwise fixed to 16 */
		if (PAPD_FAST_GAIN_CTRL) {
			numidx = numidx_array[i-1];
		}
		tx_gains.pga = pga_mid;
		phy_ac_papd_write_tx_gain(pi, core, &tx_gains, &bbmult);
		/* TODO: check start and stop indexs do we want to check more than the last index */
		phy_ac_papd_cal(pi, num_iter, core, stopindex - numidx + 1,
		    yrefindex, stopindex);
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], 1, 63, 32,
			&eps_complex, core);
		phy_papdcal_decode_epsilon(eps_complex, &eps_re, &eps_im);
		tempclip =   ((4095+eps_re)*(4095+eps_re))+  (eps_im*eps_im);
		if (tempclip >= clipthreshold)
		    clipping_flag = 1;
		else
		    clipping_flag = 0;

		if (tempclip >= clipthresholdl && tempclip <= clipthresholdu) {
		    break;
		}

		if (clipping_flag)
			pga_u = pga_mid;
		else
		    pga_l = pga_mid;

		pga_mid = (pga_u + pga_l)/ 2;
	}
	/* Stop Continuous tone after gain control to avoid radar detection */
	wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
	return tx_gains.pga;
}

static void
phy_ac_apapd_init_seq(phy_info_t *pi, uint8 core, uint16 yrefindex)
{
	uint32 cal_corr = 0x0a0, corr_shift = 0x9;
	MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 1);
	MOD_PHYREGCEE(pi, PapdEnable, core, avgPapdPowerEnb, 0);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0x0);
	cal_corr = (ROUTER_4349(pi)) ? (CHSPEC_IS2G(pi->radio_chanspec)
			? 0x050 : 0xa00) : 0xa0;
	corr_shift = (ROUTER_4349(pi)) ? (CHSPEC_IS2G(pi->radio_chanspec)
			? 0x5 : 0x9) : 0x5;
	MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, cal_corr);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, corr_shift);
	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0)
		MOD_PHYREG_ENTRY(pi, PapdIpaOffPower, papd_calIpaOffPower, 0x0)
		MOD_PHYREG_ENTRY(pi, PapdEpsilonUpdateIterations, epsilonUpdateIterations, 1)
	ACPHY_REG_LIST_EXECUTE(pi);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);
	MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);

}

static void
phy_ac_papd_turningoff_inactivecore(phy_info_t *pi, uint8 core)
{
	uint8 off_core = 0;
	off_core = (core == 0) ? 1: 0;
	MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, off_core, ovr_pa5g_pu, 1);
	MOD_RADIO_REG_TINY(pi, PA5G_CFG4, off_core, pa5g_pu, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, off_core, txrf_pwrup, 0x1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus, off_core, txrf_pwrup, 0x0);
	MOD_RADIO_REG_TINY(pi, TX_TOP_5G_OVR1, core, ovr_pa5g_pu, 1);
	MOD_RADIO_REG_TINY(pi, PA5G_CFG4, core, pa5g_pu, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, txrf_pwrup, 0x1);
	MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, txrf_pwrup, 0x1);
}

/* gain _ctrl for Tiny papd */

static uint8
phy_ac_papd_gain_ctrl_tiny(phy_info_t *pi, uint8 core, uint16 yrefindex)

{
#define EPS_MAX 4095
#define EPS_MIN -4095
#define NUM_ITER_BINARY_SEARCH 8
#define BB_MID 60
#define BB_UP 90
#define BB_LO 20
#define SCAL_IDX 60
#define TXIDX 30

	int8 i;
	bool is2g = (CHSPEC_IS2G(pi->radio_chanspec));
	int8 tt_u = 0, tt_l = 0, tt_mid = 0, coremask;
	int32 tempclip = 0;
	int32 clipthreshold = 0;
	/* analytic Gain control  with bbmult from B0 onwards */
	uint16 m[4] = {0, 0, 0, 0};
	uint16 scal_idx = SCAL_IDX;
	int16  corr_I = 0, corr_Q = 0;
	int16  Yref_I = 0, Yref_Q = 0;
	int32 y_den = 0, yref_den = 0;
	int32 yref = 0;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
	ACPHY_TBL_ID_SCALAR2};
	wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 128, 0, 32,
			acphy_papd_scaltbl_128);
	phy_ac_apapd_init_seq(pi, core, yrefindex);
	clipthreshold  = (is2g)? 13:19;
	tt_mid = BB_MID;
	tt_u = BB_UP;
	tt_l = BB_LO;
	coremask = (phy_get_phymode(pi) == PHYMODE_RSDB) ? 1 : 3;
	if ((phy_get_phymode(pi) == PHYMODE_MIMO))
		phy_ac_papd_turningoff_inactivecore(pi, core);
	wlc_phy_txpwr_by_index_acphy(pi, core + 1, TXIDX);

	(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186, TX_TONE_IQCAL_MODE_OFF,
		FALSE);

	for (i = 1; i <= NUM_ITER_BINARY_SEARCH; i++) {
		m[core] = tt_mid;
		wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], coremask);
		MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, scal_idx);
		MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, scal_idx);
		WRITE_PHYREG(pi, papdCalCorrDebugAddr, scal_idx);

		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
		OSL_DELAY(10);
		MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		if ((READ_PHYREG(pi, PapdCalStart) & 1)) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR :"
				" PAPD cal failed \n", __FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_PAPD_CAL_FAILED);
		}
		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		corr_I = (int16) (READ_PHYREGCE(pi, papdCalFirstCorr_I, core)
				& 0xffff);
		corr_Q = (int16) (READ_PHYREGCE(pi, papdCalFirstCorr_Q, core)
				& 0xffff);
		Yref_I = (int16) (READ_PHYREGCE(pi, PapdCalYref_I, core) & 0xffff);
		Yref_Q = (int16) (READ_PHYREGCE(pi, PapdCalYref_Q, core) & 0xffff);

		y_den = (int32)corr_I*(int32)corr_I +
			(int32)corr_Q*(int32)corr_Q;
		yref_den =  (int32)Yref_I*(int32)Yref_I +
			(int32)Yref_Q*(int32)Yref_Q;
		yref = yref_den*10;
		tempclip = y_den*clipthreshold;

		if (tempclip >= yref)
			tt_l = tt_mid;
		else
			tt_u = tt_mid;

		tt_mid = (tt_u+tt_l) >> 1;
	}
	wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);

	return tt_mid;
}

static void
phy_ac_papd_write_tx_gain(phy_info_t *pi, uint8 core, acphy_txgains_t *target_gain,
	uint16 *bbmult)
{
	uint8 stall_val = 0;
	uint16 curr_gains_0, curr_gains_1, curr_gains_2;
	uint16 txgain1, txgain2, lpf_gain, dac_gain;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	ACPHY_DISABLE_STALL(pi);

	txgain1  = ((target_gain->ipa & 0x00FF) | ((target_gain->pad  << 8) & 0xFF00));
	txgain2  = ((target_gain->pga & 0x00FF) | ((target_gain->txgm  << 8) & 0xFF00));
	lpf_gain = (target_gain->txlpf & 0xF0) >> 4;
	dac_gain = (target_gain->txlpf & 0x0F) >> 0;

	WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, txgain1);
	WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, txgain2);
	WRITE_PHYREGCE(pi, Dac_gain, core, dac_gain);
	MOD_PHYREGCE(pi, RfctrlCoreLpfGain, core, lpf_bq2_gain, lpf_gain);
	MOD_PHYREGCE(pi, RfctrlOverrideGains, core, txgain, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 1);
	wlc_phy_set_tx_bbmult_acphy(pi, bbmult, core);
	/* emulate RFSEQ GAIN CHANGE */
	curr_gains_0 = (target_gain->txlpf & 0xFF) | ((target_gain->ipa << 8) & 0xFF00);
	curr_gains_1 = (target_gain->pad & 0xFF) | ((target_gain->pga << 8) & 0xFF00);
	curr_gains_2 = (target_gain->txgm & 0xff);

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&curr_gains_0);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&curr_gains_1);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&curr_gains_2);

	ACPHY_ENABLE_STALL(pi, stall_val);
}
#endif /* PAPD_GAIN_CTRL */

#if defined(WLTEST) || defined(BCMDBG)
static void
wlc_phy_epa_dpd_set_acphy(phy_type_papdcal_ctx_t *ctx, uint8 enab_epa_dpd, bool in_2g_band)
{
	phy_ac_papdcal_info_t *info = (phy_ac_papdcal_info_t *)ctx;
	phy_info_t *pi = info->pi;
	bool turn_papd_on = FALSE;
	bool iovar_in_band;
	uint8 core = 0;

	if (in_2g_band) {
		pi->papdcali->data->epacal2g = enab_epa_dpd;
		turn_papd_on = (pi->papdcali->data->epacal2g == 1);
	} else {
		pi->papdcali->data->epacal5g = enab_epa_dpd;
		turn_papd_on = (pi->papdcali->data->epacal5g == 1);
	}
	iovar_in_band = ((in_2g_band &&
		(CHSPEC_IS2G(pi->radio_chanspec))) ||
		(!in_2g_band && (CHSPEC_IS5G(pi->radio_chanspec))));
	if (iovar_in_band) {
		if (!PHY_PAPDEN(pi) && !PHY_IPA(pi) && in_2g_band) {
			if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
				/* WAR for FDIQI when bq_bw = 9, 25 MHz */
				wlc_phy_radio_tiny_lpf_tx_set(pi, 2, 2, 1, 1);
			} else {
				wlc_phy_radio_tiny_lpf_tx_set(pi, 2, 2, 2, 1);
			}
		}
		if (turn_papd_on) {
			wlc_phy_cals_acphy(pi->u.pi_acphy->calmgri, PHY_PERICAL_UNDEF,
				PHY_CAL_SEARCHMODE_RESTART);
		} else {
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);
		}
	}
}
#endif /* defined(WLTEST) || defined(BCMDBG) */
static void
phy_ac_papd_radio_lpbk_cleanup_acradio(phy_info_t *pi, uint8 core)
{
	uint16 radio_rev_id;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));

/* CAL PER CORE - FOREACH_ACTV_CORE(pi, phy_stf_get_data(pi->stfi)->phyrxchain, core) */
	{
		radio_rev_id = READ_RADIO_REGC(pi, RF, REV_ID, core);

		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainI, 0);
		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainQ, 0);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_cal_pa_atten_2g, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_rx_attn, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG1, core, pwrsw_en, 1);
/*
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf2g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf2g_pwrsw_en, 1);
			}
*/

		} else {
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_cal_pa_atten_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_rx_attn, 0);

			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf5g_pwrsw_en, 0);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf5g_pwrsw_en, 0);
			}

		}
	}

/*
	ASSERT(porig->is_orig);
	porig->is_orig = FALSE;
*/

	if (ACREV_IS(pi->pubpi->phy_rev, 2)) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, rxfe_bilge_cnt, 0)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, soft_sdfeFifoReset, 1)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, soft_sdfeFifoReset, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

}

static void
phy_ac_papd_radio_lpbk_setup_acradio(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten,
 uint8 core)
{
	uint16 radio_rev_id;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));

	/* CAL PER CORE - FOREACH_ACTV_CORE(pi, phy_stf_get_data(pi->stfi)->phyrxchain, core) */
	{
		radio_rev_id = READ_RADIO_REGC(pi, RF, REV_ID, core);

		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainI, 1);
		MOD_RADIO_REGC(pi, GE16_OVR20, core, ovr_tia_GainQ, 1);
		MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 2);
		MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 2);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			if (RADIO2069REV(pi->pubpi->radiorev) == 25) {
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 0);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 0);
			}
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_cal_pa_atten_2g, tx_atten);
			MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core,
			               loopback2g_papdcal_rx_attn, rx_atten);

			/* Enable the vdd switch on mixer */
			MOD_RADIO_REGC(pi, RXRF2G_CFG1, core, pwrsw_en, 1);
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
				radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf2g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf2g_pwrsw_en, 1);
			}
			if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 4);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 4);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pa2g_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_TX, core, i_calPath_pad2g_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_cal_pu, 0);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_papdcal_pu, 1);
				MOD_RADIO_REGC(pi, TXRX2G_CAL_RX, core, loopback2g_rxiqcal_pu, 0);
				MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 1);
				MOD_RADIO_REGC(pi, RXRF2G_CFG2, core, lna2g_epapd_attn, 0);
			}
		} else {
			if (RADIO2069REV(pi->pubpi->radiorev) == 25) {
				if (CHSPEC_IS80(pi->radio_chanspec)) {
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 4);
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 4);
				} else {
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 2);
					MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 2);
				}
			}
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 1);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
			MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 0);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_cal_pa_atten_5g, tx_atten);
			MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core,
				loopback5g_papdcal_rx_attn, rx_atten);
			MOD_RADIO_REGC(pi, RXRF5G_CFG1, core, pwrsw_en, 1);
			if (radio_rev_id == 0 || radio_rev_id == 1 || radio_rev_id == 2 ||
			    radio_rev_id == 3 || radio_rev_id == 4) {
				MOD_RADIO_REGC(pi, OVR18, core, ovr_rxrf5g_pwrsw_en, 1);
			} else {
				MOD_RADIO_REGC(pi, OVR19, core, ovr_rxrf5g_pwrsw_en, 1);
			}
			if ((RADIO2069_MAJORREV(pi->pubpi->radiorev) == 2) && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainI, 7);
				MOD_RADIO_REGC(pi, TIA_CFG1, core, GainQ, 7);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pa_pu_5g, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_TX, core, i_calPath_pad_pu_5g, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_papdcal_pu, 1);
				MOD_RADIO_REGC(pi, TXRX5G_CAL_RX, core, loopback5g_rxiqcal_pu, 0);
				MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_en, 1);
				MOD_RADIO_REGC(pi, RXRF5G_CFG2, core, lna5g_epapd_attn, 0);
			}
		}
		/* #Powerdown LNA1, LNA2 */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_pwrup, 0);

		/* acphy_rfctrl_override lpf_sw rxiq_rx2 $core; */
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_aux_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_aux_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_iqcal_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_iqcal_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 1);

		/* acphy_rfctrl_override lpf_pu_dc 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_pu_dc, 1);

		/* acphy_rfctrl_override tia_DC_loop_PU 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, tia_DC_loop_PU, 1);

		/* acphy_rfctrl_override fast_nap_bias_pu 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 1);

		/* acphy_rfctrl_override rxrf_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);

		/* acphy_rfctrl_override logen_rx_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);
	}
}

static void
phy_ac_get_tx_gain(phy_info_t *pi, uint8 core_no, acphy_txgains_t *target_gain)
{
	uint16 curr_gains_0 = 0, curr_gains_1 = 0, curr_gains_2 = 0;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (pi->acphy_txpwrctrl == PHY_TPC_HW_OFF) {
		/* read current tx gain from RFSeq table and use as target_gain */

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core_no), 16,
			&curr_gains_0);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core_no), 16,
			&curr_gains_1);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core_no), 16,
			&curr_gains_2);

		/* extract gain values */
		target_gain->txlpf = (uint16) ((curr_gains_0 & 0xFF));
		target_gain->ipa  = (uint16) ((curr_gains_0 & 0xFF00) >> 8);
		target_gain->pad  = (uint16) ((curr_gains_1 & 0xFF)   >> 0);
		target_gain->pga  = (uint16) ((curr_gains_1 & 0xFF00) >> 8);
		target_gain->txgm = (uint16) ((curr_gains_2 & 0xFF)   >> 0);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_radio_lpbk_setup_tiny(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_radioregs_t *porig = (pi_ac->papdcali->papd_radioregs);
	uint8 core;

	ASSERT(TINY_RADIO(pi));

	FOREACH_CORE(pi, core) {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* PAPD loopback in g-band */

			MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				ovr_gm2g_pwrup, 1);
			MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_auxgm_pwrup, 1);

			/* AUXLNA2 Power Up */
			MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 1);
			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				/* LNA2 Power Up */
				MOD_RADIO_REG_20693(pi, LNA2G_CFG2, core, gm2g_pwrup, 0);
				MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 1);

			} else {
				/* LNA2 Power Down */
				MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_pwrup, 0);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_gm2g_auxgm_pwrup, 1);
			}

			if (!PHY_EPAPD(pi)) {
				/* Enable ipapd */
				MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core, cal2g_pa_pu, 1);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					loopback2g_papdcal_pu, 1);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
				MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core,
					cal2g_pa_atten, tx_atten);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					rf2g_papdcal_rx_attn, rx_atten);
				if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
					RADIO_REG_LIST_START
						MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST,
							core, ovr_lna2g_lna2_gain, 1)
						MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG2, core,
							lna2g_lna2_gain, 0)
						/* LNA1 Kill switch */
						MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2,
							core, ovr_lna2g_tr_rx_en, 1)
						MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core,
							lna2g_tr_rx_en, 0)
						MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2,
							core, ovr_lna2g_lna1_Rout, 1)
						MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core,
							lna2g_lna1_Rout, 0xb)
						MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2,
							core, ovr_lna2g_lna1_out_short_pu, 1)
						MOD_RADIO_REG_20693_ENTRY(pi, LNA2G_CFG1, core,
							lna2g_lna1_out_short_pu, 1)
						MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_2G_OVR_EAST2,
							core, ovr_lna2g_lna1_pu, 0)
					RADIO_REG_LIST_EXECUTE(pi, core);
				}
			} else {
				/* Enable epapd */
				RADIO_REG_LIST_START
					MOD_RADIO_REG_TINY_ENTRY(pi, TX2G_MISC_CFG1, core,
						cal2g_pa_pu, 0)
					MOD_RADIO_REG_TINY_ENTRY(pi, RXRF2G_CFG2, core,
						loopback2g_papdcal_pu, 0)
					MOD_RADIO_REG_TINY_ENTRY(pi, RXRF2G_CFG2, core,
						lna2g_epapd_en, 1)
					MOD_RADIO_REG_TINY_ENTRY(pi, LNA2G_CFG1, core,
						lna2g_lna1_bypass, 1)
					MOD_RADIO_REG_TINY_ENTRY(pi, RX_TOP_2G_OVR_NORTH, core,
						ovr_lna2g_lna1_bypass, 1)
				RADIO_REG_LIST_EXECUTE(pi, core);
				MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core,
					lna2g_epapd_attn, rx_atten);
			}

			/* #Powerdown 2G LNA1 */
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 0);
		} else {
			/* PAPD loopback in a-band */

			MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_gm5g_pwrup, 1);

			/* AUXLNA2 Power Up */
			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_5G_pwrup, 1);
				MOD_PHYREGCE(pi, RfctrlCoreRxPus, core,	rxrf_lna1_5G_pwrup, 0);
				MOD_RADIO_REG_20693(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 0);
				MOD_RADIO_REG_20693(pi, SPARE_CFG16, core, loopback5g_gm5g_pu, 1);
			} else {
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 1);
			}

			/* power down rxgm5g */
			MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0);

			if (!RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				/* powerup auxgm5g */
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_pu_auxlna2, 1);
			}

			if (!PHY_EPAPD(pi)) {
				/* Enable ipapd */
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core, cal5g_pa_pu, 1);
				MOD_RADIO_REG_TINY(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 1);
				MOD_RADIO_REG_TINY(pi, PA5G_CFG1, core, rf5g_epapd_en, 0);
				MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core,
				                    cal5g_pa_atten, tx_atten);
				MOD_RADIO_REG_TINY(pi, RXRF5G_CFG2, core,
				                    loopback5g_papdcel_rx_attn, rx_atten);
				if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
					MOD_RADIO_REG_20693(pi, LNA5G_CFG1, core,
						lna5g_lna2_gain, 0);
					MOD_RADIO_REG_20693(pi, RX_TOP_5G_OVR2, core,
					                    ovr_lna5g_lna2_gain, 1);
					MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
					                    ovr_lna5g_tr_rx_en, 1);
					if (ROUTER_4349(pi)) {
						MOD_RADIO_REG_TINY(pi, LNA5G_CFG1, core,
							lna5g_tr_rx_en, 0);
					} else {
						MOD_RADIO_REG_TINY(pi, LNA5G_CFG1, core,
							lna5g_tr_rx_en, 1);
					}
				}
			} else {
				RADIO_REG_LIST_START
					/* Enable  */
					MOD_RADIO_REG_TINY_ENTRY(pi, TX5G_MISC_CFG1, core,
						cal5g_pa_pu, 0)
					MOD_RADIO_REG_TINY_ENTRY(pi, TXRX5G_CAL_RX, core,
						loopback5g_cal_pu, 0)
					MOD_RADIO_REG_TINY_ENTRY(pi, PA5G_CFG1, core,
						rf5g_epapd_en, 1)
					MOD_RADIO_REG_TINY_ENTRY(pi, LNA5G_CFG1, core,
						lna5g_lna1_bypass, 1)
					MOD_RADIO_REG_TINY_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_lna5g_lna1_bypass, 1)
				RADIO_REG_LIST_EXECUTE(pi, core);
				MOD_RADIO_REG_TINY(pi, RXRF5G_SPARE, core,
				                    rf5g_epapd_attn, rx_atten);
			}

			/* #Powerdown 5G LNA1 */
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_5G_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_5G_pwrup, 0);
		}

		/* PHY register writes */
		/* Farrow */
		MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 1);
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_tx, 1);
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);
		} else {
			MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0,
				(CHSPEC_IS5G(pi->radio_chanspec)) ? 0 : 2);
		}

		/* #Powerup LNA2 for 20693 & power up otherwise */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_pwrup,
			(RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) ? 1 : 0);
		/* Configure the LPF switches and powerup the DC loop */
		/* MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, 0x3ff, 0x3ff);	*/
		/* MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, 0x3ff, 0x151);		*/
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_adc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_rc, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_aux_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_aux_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_iqcal_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_iqcal_bq1, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 0);

		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_dac_bq2, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq2_rc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_tia_bq1, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfSwtch, core, lpf_sw_bq1_adc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfSwtch, core, lpf_sw_bq1_adc, 1);

		/* acphy_rfctrl_override lpf_pu_dc 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_pu_dc, 1);

		/* acphy_rfctrl_override tia_DC_loop_PU 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, tia_DC_loop_PU, 1);

		/* acphy_rfctrl_override fast_nap_bias_pu 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 0);

		/* acphy_rfctrl_override rxrf_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);

		/* acphy_rfctrl_override logen_rx_pwrup 1 $core */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);

		MOD_RADIO_REG_TINY(pi, CLK_DIV_CFG1, core, dac_driver_size, 8);
		porig->adc_cfg10[core] = READ_RADIO_REG_TINY(pi, ADC_CFG10, core);
		porig->adc_ovr1[core] = READ_RADIO_REG_TINY(pi, ADC_OVR1, core);
		if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 1);
			MOD_RADIO_REG_TINY(pi, ADC_CFG18, core, adc_od_pu,
				((CHSPEC_IS20(pi->radio_chanspec)) ||
				(CHSPEC_IS40(pi->radio_chanspec))) ? 0 : 1);
		} else {
			MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 0);
		}
		MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_in_test, 1);
		MOD_RADIO_REG_TINY(pi, ADC_CFG10, core, adc_in_test, 0x0);
		/* Setting TIA gain */
		if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID))  {
			if (HW_RADIOREV(pi->pubpi->radiorev) == 5)  {
				phy_ac_papd_set_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
					?((phy_get_phymode(pi) == PHYMODE_RSDB) ? 0x5 : 0x6)
						: 0x0, core);
			} else  {
				phy_ac_papd_set_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
					? 0x3 : 0x0, core);
			}
		} else
			phy_ac_papd_set_tia_gain_tiny(pi, (CHSPEC_IS2G(pi->radio_chanspec))
				? 0x3 : 0x0, core);

	}
}

static void
phy_ac_papd_radio_setup_28nm(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten, bool epapd_flag)
{
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_params_t *params	= pi->u.pi_acphy->papdcali->papd_params;
	uint8 tia_gain_mode = 0;

	/* Initially save the registers for TIA */
	phy_ac_reg_cache_save(aci, RADIOREGS_PAPDCAL);
	if (RADIOMAJORREV(pi) >= 2)
	{
		//43012b0 making ntssi 0 during TX for cal
		// To have rx adc in normal mode
		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_CFG1_OVR, 0, ovr_afediv_rst_en, 0x1);
		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_CFG1_OVR, 0, ovr_afediv_reset, 0x1);

		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_REG5, 0, afediv_rst_en, 0x1);

		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_RADIO_REG_28NM(pi, RF, SPARE_CFG1, 0, ovr_afediv2g_adc_Ntssi, 1);
			MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_REG1, 0, afediv2g_adc_Ntssi, 0);
		} else {
			MOD_RADIO_REG_28NM(pi, RF, SPARE_CFG1, 0, ovr_afediv5g_adc_Ntssi, 1);
			MOD_RADIO_REG_28NM(pi, RF, AFEDIV5G_REG1, 0, afediv5g_adc_Ntssi,  0);
		}
		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_REG1, 0, afediv_reset, 0x1);
		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_REG1, 0, afediv_reset, 0x0);

		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_REG5, 0, afediv_rst_en, 0x0);

		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_CFG1_OVR, 0, ovr_afediv_rst_en, 0x0);
		MOD_RADIO_REG_28NM(pi, RF, AFEDIV2G_CFG1_OVR, 0, ovr_afediv_reset, 0x0);

	}

	/* Configure ADC/DAC path switches */
	MOD_RADIO_REG_20695_MULTI_6(pi, RF, TIA_CFG2_OVR, 0, ovr_tia_sw_bq2_adc, 0x1,
		ovr_tia_sw_bq2_wbcal, 0x1, ovr_tia_sw_rx_bq1, 0x1,
		ovr_tia_sw_rx_wbcal, 0x1, ovr_tia_sw_wbcal_adc, 0x1,
		ovr_tia_wbcal_pu, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_CFG2_OVR, 0, ovr_tia_sw_aux_adc, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_CFG1_OVR, 0, ovr_tia_bias_pu, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_CFG1_OVR, 0, ovr_tia_bq1_pu, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_CFG1_OVR, 0, ovr_tia_bq2_pu, 0x1);

	MOD_RADIO_REG_20695_MULTI_3(pi, RF, TIA_REG16, 0, tia_sw_aux_adc, 0x0,
		tia_sw_bq2_adc, 0x0, tia_sw_bq2_wbcal, 0x0);
	MOD_RADIO_REG_20695_MULTI_3(pi, RF, TIA_REG18, 0, tia_sw_rx_bq1, 0x0,
		tia_sw_rx_wbcal, 0x1, tia_sw_wbcal_adc, 0x1);
	/* Enable wb TIA and disable main TIA */
	MOD_RADIO_REG_28NM(pi, RF, TIA_REG19, 0, tia_wbcal_pu, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_REG7, 0, tia_bias_pu, 0x1);
	MOD_RADIO_REG_28NM(pi, RF, TIA_REG7, 0, tia_bq1_pu, 0x0);
	MOD_RADIO_REG_28NM(pi, RF, TIA_REG7, 0, tia_bq2_pu, 0x0);

	/* Currently no need to setup the DAC bandwidth as it will be set to 120 */

	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		tia_gain_mode = params->tia_mode_2g;
		MOD_RADIO_REG_28NM(pi, RF, LNA2G_REG2, 0, lna2g_auxpath_en, 0x0);
		MOD_RADIO_REG_28NM(pi, RF, RX2G_CFG1_OVR, 0, ovr_rx2g_gm_en, 0x1);
		MOD_RADIO_REG_28NM(pi, RF, RX2G_REG3, 0, rx2g_gm_en, 0);
		MOD_RADIO_REG_20695_MULTI_3(pi, RF, RX2G_REG3, 0, rx2g_gm_loopback_mainpath, 0x0,
			rx2g_gm_loopback_en, 0x1, rx2g_gm_bias_en, 0x1);
		MOD_RADIO_REG_20695_MULTI_3(pi, RF, LNA2G_REG1, 0, lna2g_tr_rx_en, 0x1,
			lna2g_lna1_out_short_pu, 0x1, lna2g_lna1_pu, 0x0);
		MOD_RADIO_REG_20695_MULTI_3(pi, RF, RX2G_CFG2_OVR, 0, ovr_lna2g_tr_rx_en, 0x1,
			ovr_lna2g_lna1_out_short_pu, 0x1, ovr_lna2g_lna1_pu, 0x1);

		/* PA Pull up done using Phy override */
		// MOD_RADIO_REG_28NM(pi, RF, PA2G_CFG1, 0, pa2g_pu, 0x1);
		// MOD_RADIO_REG_28NM(pi, RF, TX2G_CFG1_OVR, 0, ovr_pa2g_pu, 0x1);
		MOD_RADIO_REG_28NM(pi, RF, TX2G_MISC_CFG1, 0, pa2g_cal_pu, 1);
		MOD_RADIO_REG_28NM(pi, RF, PA2G_CFG1, 0, pa2g_cal_atten, tx_atten);
		MOD_RADIO_REG_28NM(pi, RF, RX2G_REG3, 0, rx2g_gm_loopback_gain, 0x3);

		if (epapd_flag) {
			/* EPAPD */
			MOD_RADIO_REG_28NM(pi, RF, RX2G_REG1, 0, rx2g_iloopback_en, 0x0);
			MOD_RADIO_REG_28NM(pi, RF, LNA2G_REG2, 0, lna2g_epapd_en, 0x1);
			/* PAPD Attenuation. Need to tune this based on results */
			MOD_RADIO_REG_28NM(pi, RF, LNA2G_REG2, 0, lna2g_epapd_attn, rx_atten);
		} else {
			/* iPAPD */
			MOD_RADIO_REG_28NM(pi, RF, RX2G_REG1, 0, rx2g_iloopback_en, 0x1);
			MOD_RADIO_REG_28NM(pi, RF, RX2G_REG1, 0, rx2g_iloopback_attn, rx_atten);
			MOD_RADIO_REG_28NM(pi, RF, LNA2G_REG2, 0, lna2g_epapd_en, 0x0);
		}

	} else {
		/* 5G Radio configuration. Yet to be filled */
		tia_gain_mode = params->tia_mode_5g;
		RADIO_REG_LIST_START
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG1, 0, rx5g_lna_tr_rx_en, 0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG1, 0, rx5g_lna_pu, 0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG1, 0, rx5g_lna_out_short, 0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG1, 0, rx5g_lna_bypass, 0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG2, 0, lna5g_auxpath_en, 0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG4, 0, rx5g_gm_pu, 0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG4, 0, rx5g_gm_loopback_en_auxpath,
					0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_REG4, 0, rx5g_gm_loopback_en_mainpath,
					0x0)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, PA5G_CFG11, 0, pa5g_cal_pu, 0x1)
			/* Override Registers */
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_CFG2_OVR, 0, ovr_rx5g_lna_tr_rx_en,
					0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_CFG2_OVR, 0, ovr_rx5g_lna_pu, 0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_CFG2_OVR, 0, ovr_rx5g_lna_out_short,
					0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_CFG2_OVR, 0, ovr_rx5g_lna_bypass, 0x1)
			MOD_RADIO_REG_28NM_ENTRY(pi, RF, RX5G_CFG1_OVR, 0, ovr_rx5g_gm_pu, 0x1)
		RADIO_REG_LIST_EXECUTE(pi, 0);

		MOD_RADIO_REG_28NM(pi, RF, PA5G_CFG11, 0, pa5g_cal_atten, tx_atten);

		if (epapd_flag) {
			/* EPAPD */
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG2, 0, rx5g_lna_epapd_en, 0x1);
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG2, 0, rx5g_lna_epapd_attn, rx_atten);
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG4, 0, rx5g_coup_loopback_en, 0);
		} else {
			/* iPAPD */
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG2, 0, rx5g_lna_epapd_en, 0x0);
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG4, 0, rx5g_coup_loopback_en, 1);
			MOD_RADIO_REG_28NM(pi, RF, RX5G_REG4, 0, rx5g_coup_loopback_attn, rx_atten);
		}

	}
	/* Set Bandwidth of WB TIA */
	phy_ac_papd_set_wbtia_gain_bw(pi, tia_gain_mode);

	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, 0, rxrf_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, 0, rxrf_pwrup, 1);

	/* acphy_rfctrl_override logen_rx_pwrup 1 $core */
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, 0, logen_rx_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, 0, logen_rx_pwrup, 1);
}

static void
phy_ac_papd_radio_lpbk_setup_20697(phy_info_t *pi, uint16 tx_atten, uint16 rx_atten,
bool epapd_flag)
{
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint8 core;

	/* Initially save the registers for TIA */
	phy_ac_reg_cache_save(aci, RADIOREGS_PAPDCAL);
	FOREACH_CORE(pi, core) {
		/* configuring radio registers per core */
		RADIO_REG_LIST_START
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG2_OVR, 1, core,
				ovr_tia_sw_bq2_adc, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG2_OVR, 1, core,
				ovr_tia_sw_aux_adc, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG2_OVR, 1, core, ovr_tia_bq1_bw, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG2_OVR, 1, core, ovr_tia_bq2_bw, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG2_OVR, 1, core,
				ovr_tia_bq2_gain, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG20, 1, core, tia_bq1_bw, 3)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG20, 1, core, tia_bq2_bw, 4)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG20, 1, core, tia_bq2_gain, 4)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG17, 1, core, tia_sw_dac_rx, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG16, 1, core, tia_sw_bq2_wbcal, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG16, 1, core, tia_sw_aux_adc, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG16, 1, core, tia_sw_adc_gpaio, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG18, 1, core, tia_sw_wbcal_balls, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG18, 1, core, tia_sw_rx_bq1, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG16, 1, core, tia_sw_bq2_adc, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG18, 1, core, tia_sw_rx_wbcal, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG16, 1, core, tia_sw_bq2_wbcal, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG18, 1, core, tia_sw_wbcal_adc, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG2_OVR, 1, core,
				ovr_lna2g_tr_rx_en, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG1, 1, core, lna2g_tr_rx_en, 0x0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG2, 1, core, lna2g_auxpath, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG1_OVR, 1, core,
				ovr_rx2g_gm_bypass, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG4, 1, core, rx2g_gm_bypass, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG2_OVR, 1, core,
				ovr_lna2g_lna1_bias_pu, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG3, 1, core,
				lna2g_lna1_bias_pu, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG2_OVR, 1, core,
				ovr_lna2g_lna1_pu, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG1, 1, core, lna2g_lna1_pu, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TX2G_MISC_CFG1, 1, core, pa2g_cal_pu, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG2, 1, core, lna2g_epapd_en, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG1, 1, core, rx2g_iloopback_en, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG3, 1, core,
				rx2g_gm_loopback_mainpath, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG5, 1, core,
				rx2g_gm_loopback_en, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG1_OVR, 1, core, ovr_rx2g_gm_en, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG3, 1, core, rx2g_gm_en, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG5, 1, core,
				rx2g_gm_loopback_gain, 2)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_CFG2_OVR, 1, core,
				ovr_lna2g_lna1_out_short_pu, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, LNA2G_REG1, 1, core,
				lna2g_lna1_out_short_pu, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, RX2G_REG3, 1, core, rx2g_gm_bias_en, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g11, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g12, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g21, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g22, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_c11, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG1, 1, core, tia_g11, 0)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG2, 1, core, tia_g12, 502)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG3, 1, core, tia_g21, 270)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG4, 1, core, tia_g22, 172)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG6, 1, core, tia_c11, 31)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g32, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g33, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g34, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_g43, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_CFG1_OVR, 1, core, ovr_tia_c42, 1)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG10, 1, core, tia_g32, 157)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG11, 1, core, tia_g33, 46)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG12, 1, core, tia_g34, 157)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG13, 1, core, tia_g43, 157)
			MOD_RADIO_REG_20697_ENTRY(pi, RF, TIA_REG9, 1, core, tia_c42, 1)
		RADIO_REG_LIST_EXECUTE(pi, core);

		MOD_RADIO_REG_20697X(pi, RF, PA2G_CFG1, 1, core, pa2g_cal_atten, tx_atten);
		MOD_RADIO_REG_20697X(pi, RF, RX2G_REG1, 1, core, rx2g_iloopback_attn, rx_atten);
		/* #Powerdown LNA1, LNA2 */
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_pwrup, 1);

		/* #Rx_pu */
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_bq2_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, lpf_bq2_pu, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_bq1_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, lpf_bq1_pu, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, lpf_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, lpf_pu, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_nrssi_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1g_pu, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, tia_DC_loop_PU, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_pu_dc, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna1_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, tia_DC_loop_bypass, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, tia_DC_loop_bypass, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 0);
	}
}

static void
phy_ac_papd_set_wbtia_gain_bw(phy_info_t *pi, uint8 wbtia_mode)
{
	/* TIA Gain codes */
	uint16 wbcal_g_array[] = {256, 3079, 2180, 4080, 3079, 2180, 1024};
	uint16 wbcal_c_array[] = {2, 18, 13, 28, 21, 15, 8};
	uint16 gval = wbcal_g_array[wbtia_mode];
	uint16 cval = wbcal_c_array[wbtia_mode];

	MOD_RADIO_REG_28NM(pi, RF, TIA_REG19, 0, tia_wbcal_g, gval);
	MOD_RADIO_REG_28NM(pi, RF, TIA_REG18, 0, tia_wbcal_c, cval);

}

static void
phy_ac_papd_radio_lpbk_cleanup_tiny(phy_info_t *pi)
{
	uint8 core;

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_radioregs_t *porig = (pi_ac->papdcali->papd_radioregs);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	FOREACH_CORE(pi, core) {
		/* # Enable the loopback path */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* # Powerup the 2G iPA attenuation on Tx side (loops back to Rx) */
			MOD_RADIO_REG_TINY(pi, TX2G_MISC_CFG1, core, cal2g_pa_pu, 0x0);
			/* # Powerup the papd loopback path on 2G Rx side */
			MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, loopback2g_papdcal_pu, 0x0);

			if (ROUTER_4349(pi)) {
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_tr_rx_en, 0);
			} else {
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_tr_rx_en, 1);
				MOD_RADIO_REG_20693(pi, LNA2G_CFG1, core,
					lna2g_tr_rx_en, 1);
			}

			MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				ovr_gm2g_pwrup, 0);
			if (!RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_NORTH, core,
					ovr_rf2g_mix1st_en, 0);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_EAST, core,
				                    ovr_gm2g_auxgm_pwrup, 1);
				MOD_RADIO_REG_TINY(pi, RX_TOP_2G_OVR_NORTH, core,
					ovr_lna2g_lna1_bypass, 0);
			}
			MOD_RADIO_REG_TINY(pi, LNA2G_CFG2, core, gm2g_auxgm_pwrup, 0);
			MOD_RADIO_REG_TINY(pi, RXRF2G_CFG2, core, lna2g_epapd_en, 0);
			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST, core,
					ovr_lna2g_lna2_gain, 0);
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_lna1_Rout, 0);
				MOD_RADIO_REG_20693(pi, RX_TOP_2G_OVR_EAST2, core,
					ovr_lna2g_lna1_out_short_pu, 0);
			}
		} else {
			/* # Powerdown the 5G iPA attenuation on Tx side (loops back to Rx) */
			MOD_RADIO_REG_TINY(pi, TX5G_MISC_CFG1, core, cal5g_pa_pu, 0x0);
			/* # Powerdown the master cal pu signal on 5G Rx side (common to papd &
			 * rxiqcal). Not needed for rc/cr rxiqcal pu.
			 */
			MOD_RADIO_REG_TINY(pi, TXRX5G_CAL_RX, core, loopback5g_cal_pu, 0x0);
			MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core, ovr_gm5g_pwrup, 0);
			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				MOD_RADIO_REG_TINY(pi, LNA5G_CFG2, core, lna5g_pu_lna2, 0);
			} else {
				MOD_RADIO_REG_TINY(pi, RX_TOP_5G_OVR, core,
				                    ovr_lna5g_pu_auxlna2, 1);
			}
			RADIO_REG_LIST_START
				MOD_RADIO_REG_TINY_ENTRY(pi, LNA5G_CFG2, core, lna5g_pu_auxlna2, 0)
				MOD_RADIO_REG_TINY_ENTRY(pi, RX_TOP_5G_OVR, core,
					ovr_lna5g_lna1_bypass, 0)
				MOD_RADIO_REG_TINY_ENTRY(pi, PA5G_CFG1, core, rf5g_epapd_en, 0)
			RADIO_REG_LIST_EXECUTE(pi, core);

			if (RADIOID_IS(pi->pubpi->radioid, BCM20693_ID)) {
				RADIO_REG_LIST_START
					MOD_RADIO_REG_20693_ENTRY(pi, SPARE_CFG16, core,
						loopback5g_gm5g_pu, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_lna5g_lna1_pu, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR2, core,
						ovr_lna5g_lna2_gain, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_mix5g_en, 1)
					MOD_RADIO_REG_20693_ENTRY(pi, RX_TOP_5G_OVR, core,
						ovr_lna5g_tr_rx_en, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, TX_TOP_5G_OVR1, core,
						ovr_pa5g_pu, 0)
					MOD_RADIO_REG_20693_ENTRY(pi, PA5G_CFG4, core, pa5g_pu, 0)
				RADIO_REG_LIST_EXECUTE(pi, core);
			}
		}
		MOD_RADIO_REG_TINY(pi, ADC_OVR1, core, ovr_adc_od_pu, 0);
	}
	MOD_PHYREG(pi, RxSdFeConfig1, farrow_rshift_force, 0);
	MOD_PHYREG(pi, RxSdFeConfig6, rx_farrow_rshift_0, 0);

	FOREACH_CORE(pi, core) {
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_CFG10, core),
		                         porig->adc_cfg10[core]);
		phy_utils_write_radioreg(pi, RADIO_REG(pi, ADC_OVR1, core),
		                         porig->adc_ovr1[core]);
	}
}

static void
phy_ac_papd_radio_lpbk_cleanup_28nm(phy_info_t *pi)
{
	/* restore radio config back */
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_reg_cache_restore(aci, RADIOREGS_PAPDCAL);
}

static void
phy_ac_papd_radio_lpbk_cleanup_20697(phy_info_t *pi)
{
	/* restore radio config back */
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_reg_cache_restore(aci, RADIOREGS_PAPDCAL);
}

static void
phy_ac_papd_set_tia_gain_tiny(phy_info_t *pi, uint16 gain, uint8 core)
{
	/* clear bits 0-5 of the RX_BB_2G_OVR_EAST radio register */
	phy_utils_write_radioreg(pi, RADIO_REG(pi, RX_BB_2G_OVR_EAST, core),
		(READ_RADIO_REG_TINY(pi, RX_BB_2G_OVR_EAST, core) & 0xffc0));
	MOD_PHYREGCE(pi, RfctrlOverrideGains, core, rxgain, 1);
	MOD_PHYREGCE(pi, RfctrlCoreRXGAIN1, core, rxrf_tia_gain, gain);
}

static void
wlc_phy_txpwr_papd_cal_run_acphy(phy_info_t *pi, uint8 tx_pre_cal_pwr_ctrl_state)
{
	bool suspend = TRUE, suspend_flag = FALSE;
	uint8 tx_pwr_ctrl_state;
	uint8 core;
	int16  tx_atten = 0, rx_atten = 0, i;
	uint32 eps_init_val = 0x0;
	int16 pgagain_offset = 0;
	int16 epsilonoffset = 0;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2, ACPHY_TBL_ID_SCALAR3};
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};
#ifdef PAPD_GAIN_CTRL
	acphy_txgains_t tx_gains;
	uint8 PAgain = 0xff;
	uint16 bbmult = 0x3f;
	uint8 Gainoverwrite = 0;
#endif /* PAPD_GAIN_CTRL */
#ifdef BCMDBG
	int16 epsregval;
#endif /* BCMDBG */
	int16 pgag = 0, eps_offset = 0;
	uint16 numIterLMS_papd = 0x10, startindex = 5, yrefindex = 5, stopindex = 63;
	acphy_txgains_t txgains[4] = {{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}};
	int8 tx_idx = 20, tx_idx_pwr[PHY_CORE_MAX] = {0};
	txgain_setting_t txgain_settings;
	phy_ac_papdcal_info_t *papdi = pi->u.pi_acphy->papdcali;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (papdi->acphy_papd_skip == 1)
		return;
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		numIterLMS_papd = 0x10;
#ifdef PAPD_GAIN_CTRL
		bbmult = 0x30;
#endif /* PAPD_GAIN_CTRL */
	}
#ifdef PAPD_GAIN_CTRL
	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		numIterLMS_papd = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x20 : 0x40;
	}
	tx_gains.txlpf = 0x0;
	tx_gains.txgm = 0xff;
	tx_gains.pga = 0xff;
	tx_gains.pad = 0xff;
	phy_ac_papdcal_get_gainparams(papdi, &Gainoverwrite, &PAgain);
	tx_gains.ipa = (Gainoverwrite == 0) ? 0xff : PAgain;
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
		tx_gains.txlpf = 0x0;
		tx_gains.txgm = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0x67 : 0x91;
		tx_gains.pga = 0xff;
		tx_gains.pad = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xff : 0x7f;
		tx_gains.ipa = (CHSPEC_IS2G(pi->radio_chanspec)) ? 0xc0 : 0x3;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			startindex = 0;
			yrefindex = 0;
		}
	}
#endif /* PAPD_GAIN_CTRL */

	/* skip cal if phy is muted */
	if (PHY_MUTED(pi) || ISSIM_ENAB(pi->sh->sih))
		return;

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		printf("MAC was not suspended before calling wlc_phy_txpwr_papd_cal_run_acphy!\n");
		suspend_flag = TRUE;
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}
	/* Disable BT as it affects PAPD CAL */
	wlc_btcx_override_enable(pi);
	/* Disable CRS */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Turn off the txpowercontrol and save the txindex */
	tx_pwr_ctrl_state = pi->acphy_txpwrctrl;
	wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);

	/* Make it work for both 4335 and 4360 */
	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		/* initialize scaler table */
		wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 64, 0, 32,
			acphy_papd_scaltbl);
	/* Fill up epsilon table with eps_init_val = 0 */
		for (i = 0; i < 64; i++) {
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], 1, i, 32,
				&eps_init_val, core);
		}
	}

	/* Call power control before radio/phy set up to not mess with papd setup */
	if (PHY_EPAPD(pi) && (papdi->papdpwrctrl)) {
		FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
			tx_idx_pwr[core] = wlc_phy_tone_pwrctrl(pi, 20, core);
		}
	}

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		/* acphy_papd_cal_phy_setup */
		phy_ac_papd_phy_setup(pi, core);

		/* 2069_papd_cal_setup */
		phy_ac_papd_radio_lpbk_setup_acradio(pi, tx_atten, rx_atten, core);
		pi->u.pi_acphy->txpwrindex[core] = 16;

		MOD_PHYREGCEE(pi, PapdCalShifts, core, papdYrefOverride, 0x0);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0x0);
		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core); /* Specify the core */
#ifdef PAPD_GAIN_CTRL
		gain_ctrl = 1;
		/* This is needed to put analytic cal, if enabled, in gain control mode */
		if (PHY_EPAPD(pi) && (papdi->papdpwrctrl)) {
			if (tx_idx_pwr[core] != -1)
				tx_idx = tx_idx_pwr[core];
			wlc_phy_get_txgain_settings_by_index_acphy(
				pi, &txgain_settings, tx_idx);
			papd_gainctrl_pga[core] =  (txgain_settings.rad_gain_mi >> 8) & 0xff;
			tx_gains.pga = papd_gainctrl_pga[core];
			wlc_phy_txpwr_by_index_acphy(pi, (1 << core), tx_idx);
		} else {
			papd_gainctrl_pga[core] = phy_ac_papd_gain_ctrl_acradio(pi,
				numIterLMS_papd, core, startindex, yrefindex, stopindex);
			tx_gains.pga = papd_gainctrl_pga[core];
			phy_ac_papd_write_tx_gain(pi, core, &tx_gains, &bbmult);
		}
		papd_gainctrl_pga[core] = phy_ac_papd_gain_ctrl_acradio(pi, numIterLMS_papd,
			core, startindex, yrefindex, stopindex);
		gain_ctrl = 0; /* Set analytic cal, if enabled, to normal mode */

#endif  /* PAPD_GAIN_CTRL */

		phy_ac_papd_cal(pi, numIterLMS_papd, core, startindex, yrefindex, stopindex);

		for (i = 0; i < startindex; i++) {
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], 1, i, 32,
				&eps_init_val, core);
		}

		phy_ac_get_tx_gain(pi, core, &(txgains[core]));
		papd_gainctrl_pga[core] = txgains[core].pga;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pgag = txgains[core].pga;
			pgagain_offset = *(rfpwrlut_ptr + pgag);
			eps_offset = -4;

		} else {
			pgag = txgains[core].pga;
			pgagain_offset = *(rfpwrlut_ptr + pgag);
			eps_offset = 0;
		}

		/* papd_index_shift in tcl-- needs to be taken care of */
		epsilonoffset = -66 + pgagain_offset + eps_offset;

		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 8);
		if (tx_pre_cal_pwr_ctrl_state == PHY_TPC_HW_OFF) {
			MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
				epsilonoffset);
		} else {
			MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, 0);
		}
#ifdef BCMDBG
		epsregval = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonOffset);
		PHY_PAPD((" ------------------ \n"));
		PHY_PAPD(("read epsilonTableAdjust In PAPD is %d\n", epsregval));
		PHY_PAPD((" ------------------ \n"));
#endif /* BCMDBG */

		/* save this value, in case some other pro re-init phyregs */
		papdi->acphy_papd_epsilon_offset[core] = epsilonoffset;

		/* acradio papd_cal_cleanup */
		phy_ac_papd_radio_lpbk_cleanup_acradio(pi, core);

	/* acphy_papd_cal_phy_cleanup */
		phy_ac_papd_phy_cleanup(pi, core);
		/* acphy_rfctrl_override txgain off all */
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN1, core, 0);
		WRITE_PHYREGCE(pi, RfctrlCoreTXGAIN2, core, 0);
		WRITE_PHYREGCE(pi, Dac_gain, core, 0);
		MOD_PHYREGCE(pi, RfctrlCoreLpfGain, core, lpf_bq2_gain, 0);

		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, txgain, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideGains, core, lpf_bq2_gain, 0);

		/* acphy_papd_stats-- Some debug related thing. Not done here */
		/* acphy_papd on {0} or {0 1 2} */
	}

	/* restore tx pwr index to original power index */
	/* wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF); */
	wlc_phy_txpwrctrl_enable_acphy(pi, tx_pwr_ctrl_state);
	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 0);
	}
	wlc_phy_papd_set_rfpwrlut(pi);
	/* Disabling BTCX Override */
	wlc_phy_btcx_override_disable(pi);
	/* Enable CRS */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	/* If the mac was suspended from this function, enable it */
	if (suspend_flag)
		wlapi_enable_mac(pi->sh->physhim);
}

/* Dump the PAPD LUT (eps table) to PHY_CAL trace */
#if defined(BCMDBG)
void
wlc_phy_papd_dump_eps_trace_acphy(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 core, j;
	uint32 eps_table[ACPHY_PAPD_EPS_TBL_SIZE];
	int32 eps_re, eps_im;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	FOREACH_ACTV_CORE(pi, stf_shdata->phytxchain, core) {
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core],
			ACPHY_PAPD_EPS_TBL_SIZE, 0, 32, eps_table, core);

		PHY_CAL(("core %d\n", core));
		bcm_bprintf(b, "  PAPD Epsilon Table  Real Image CORE %d \n", core);
		for (j = 0; j < ACPHY_PAPD_EPS_TBL_SIZE; j++) {
			phy_papdcal_decode_epsilon(eps_table[j], &eps_re, &eps_im);
			PHY_CAL(("{%d %d} ", eps_re, eps_im));
			bcm_bprintf(b, "{%d %d}\n ", eps_re, eps_im);
		}
		PHY_CAL(("\n"));
	}
	PHY_CAL(("\n"));
}
#endif // endif

static void
phy_ac_papd_smooth(phy_info_t *pi, uint8 core, uint32 winsz, uint32 start, uint32 end)
{
	uint32 *buf, *src, *dst, sz;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2};

	PHY_CAL(("Smoothing papd cal on core: %d\n", core));

	sz = end - start + 1;
	ASSERT(end > start);
	ASSERT(end < ACPHY_PAPD_EPS_TBL_SIZE);

	/* Allocate storage for both source & destination tables */
	buf = phy_malloc_fatal(pi, 2 * sizeof(*buf) * ACPHY_PAPD_EPS_TBL_SIZE);

	/* Setup source & destination pointers */
	src = buf;
	dst = buf + ACPHY_PAPD_EPS_TBL_SIZE;

	/* Read original table */
	wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core], ACPHY_PAPD_EPS_TBL_SIZE,
		0, 32, src, core);

	/* Average coeffs across window */
	do {
		uint32 win_start, win_end;
		int32 nAvr, eps_r, eps_i, eps_real, eps_imag;

		win_start = end - MIN(end, (winsz >> 1));
		win_end = MIN(ACPHY_PAPD_EPS_TBL_SIZE - 1, end + (winsz >> 1));
		nAvr = win_end - win_start + 1;
		eps_real = 0;
		eps_imag = 0;

		do {
			phy_papdcal_decode_epsilon(src[win_end], &eps_r, &eps_i);
			eps_real += eps_r;
			eps_imag += eps_i;
		} while (win_end-- != win_start);

		eps_real /= nAvr;
		eps_imag /= nAvr;
		dst[end] = ((uint32)eps_imag << 13) | ((uint32)eps_real & 0x1fff);
	} while (end-- != start);

	/* Write updated table */
	wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_ids[core], sz, start, 32, dst, core);

	/* Free allocated buffer */
	phy_mfree(pi, buf, 2 * sizeof(*buf) * ACPHY_PAPD_EPS_TBL_SIZE);
}

static void
phy_ac_papd_rx_gain_ctrl(phy_info_t *pi)
{

	int8 gain_ctrl_done = 0;
	int16 clip_det_sel, wlc_clip_cnt_regval;
	uint16 bbmult_orig[PHY_CORE_MAX];
	uint8 core;
	int16 SAVE_rx_bb_2g_ovr1, SAVE_tia_cfg12_rssi_pwrup, SAVE_tia_cfg12_wrssi3_ref_high_sel;
	int16 SAVE_tia_cfg12_wrssi3_ref_mid_sel, SAVE_tia_cfg13_wrssi3_ref_low_sel;
	uint8 rx_attn = 0, tx_attn = 0, max_attn = 0;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	uint8 phyrxchain;

	BCM_REFERENCE(phyrxchain);

	ASSERT(ACMAJORREV_3(pi->pubpi->phy_rev));
	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM20691_ID));

	if (PHY_EPAPD(pi)) {
		max_attn = 15;
	} else {
		max_attn = 3;
	}

	clip_det_sel = READ_PHYREGFLD(pi, RxSdFeConfig6, rx_farrow_rshift_0);
	/* tcl has a loop for this */
	/* return from Deaf */
	if (phy_ac_rxgcrs_get_deaf_count(pi_ac->rxgcrsi) > 0) {
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	}

	/* start tone with full scale at DAC */
	phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
	FOREACH_ACTV_CORE(pi, phyrxchain, core) {
		wlc_phy_get_tx_bbmult_acphy(pi, &(bbmult_orig[core]), core);
		(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_2MHz, 450,
			TX_TONE_IQCAL_MODE_OFF, FALSE);
		wlc_phy_set_tx_bbmult_acphy(pi, &bbmult_orig[core], core);
	}

	/* save register states */
	if (ACREV_IS(pi->pubpi->phy_rev, 4)) {
		SAVE_rx_bb_2g_ovr1 = READ_RADIO_REGFLD_20691(pi, RX_BB_2G_OVR1, 0,
		                                             ovr_tia_offset_rssi_pwrup);
	} else {
		SAVE_rx_bb_2g_ovr1 = READ_RADIO_REGFLD_20691(pi, RX_BB_2G_OVR_EAST, 0,
		                                             ovr_tia_offset_rssi_pwrup);
	}
	SAVE_tia_cfg12_rssi_pwrup = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0, rssi_pwrup);
	SAVE_tia_cfg12_wrssi3_ref_high_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                             wrssi3_ref_high_sel);
	SAVE_tia_cfg12_wrssi3_ref_mid_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                            wrssi3_ref_mid_sel);
	SAVE_tia_cfg13_wrssi3_ref_low_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG13, 0,
	                                                            wrssi3_ref_low_sel);

	/* Enable rssi clip detectors */
	if (ACREV_IS(pi->pubpi->phy_rev, 4)) {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR1, 0, ovr_tia_offset_rssi_pwrup, 1);
	} else {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR_EAST, 0, ovr_tia_offset_rssi_pwrup, 1);
	}
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, rssi_pwrup, 1);

	if (clip_det_sel == 3) {
		MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, 4);
	} else {
		MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, 0);
	}
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_mid_sel, 0);
	MOD_RADIO_REG_20691(pi, TIA_CFG13, 0, wrssi3_ref_low_sel, 0);

	for (rx_attn = 0; rx_attn < max_attn; rx_attn++) {
		while (tx_attn <= max_attn) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				MOD_RADIO_REG_20691(pi, TX2G_MISC_CFG1, 0, cal2g_pa_atten, tx_attn);
				MOD_RADIO_REG_20691(pi, RXRF2G_CFG2, 0,
				                    rf2g_papdcal_rx_attn, rx_attn);
			} else {
				MOD_RADIO_REG_20691(pi, TX5G_MISC_CFG1, 0, cal5g_pa_atten, tx_attn);
				MOD_RADIO_REG_20691(pi, RXRF5G_CFG2, 0,
				                    loopback5g_papdcel_rx_attn, rx_attn);
			}

			if (clip_det_sel == 3) {
				clip_det_sel = clip_det_sel - 1;
			}

			if (clip_det_sel+1 == 1) {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt1);
			} else if (clip_det_sel+1 == 2) {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt2);
			} else {
				wlc_clip_cnt_regval = READ_PHYREG(pi, W3ClipCnt3);
			}

			if (wlc_clip_cnt_regval == 0) {
				gain_ctrl_done = 1;
				break;
			}

			if (tx_attn == max_attn) {
				break;
			}
			tx_attn = tx_attn + 1;
		}
		if ((gain_ctrl_done == 1) || (rx_attn == 3)) {
			break;
		}
	}

	/* Restor Registers */
	if (ACREV_IS(pi->pubpi->phy_rev, 4)) {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR1, 0,
		                    ovr_tia_offset_rssi_pwrup, SAVE_rx_bb_2g_ovr1);
	} else {
		MOD_RADIO_REG_20691(pi, RX_BB_2G_OVR_EAST, 0,
		                    ovr_tia_offset_rssi_pwrup, SAVE_rx_bb_2g_ovr1);
	}

	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, rssi_pwrup, SAVE_tia_cfg12_rssi_pwrup);
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0,
	                    wrssi3_ref_high_sel, SAVE_tia_cfg12_wrssi3_ref_high_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0,
	                    wrssi3_ref_mid_sel, SAVE_tia_cfg12_wrssi3_ref_mid_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG13, 0,
	                    wrssi3_ref_low_sel, SAVE_tia_cfg13_wrssi3_ref_low_sel);

	/* Switch off test tone */
	wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);

	/* beDeaf */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
}

static void
BCMATTACHFN(phy_ac_papdcal_nvram_attach_old)(phy_ac_papdcal_info_t *papdcal_info)
{
	phy_info_t *pi = papdcal_info->pi;

	pi->papdcali->data->epacal2g = (uint8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_epacal2g, 0));
	pi->papdcali->data->epacal5g = (uint8) (PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_epacal5g, 0));
	pi->papdcali->data->epacal2g_mask = (uint16) (PHY_GETINTVAR_DEFAULT_SLICE(pi,
		rstr_epacal2g_mask, 0x3fff));

	/* Default value for forced papd cal index */
	papdcal_info->pacalidx_iovar = -1;
	papdcal_info->papdmode = (uint8)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_papdmode, PAPD_LMS);

	if ((PHY_GETVAR_SLICE(pi, rstr_pagc2g)) != NULL) {
		papdcal_info->srom_pagc2g = (uint8)PHY_GETINTVAR_SLICE(pi, rstr_pagc2g);
		papdcal_info->srom_pagc2g_ovr = 0x1;
	} else {
		papdcal_info->srom_pagc2g = 0xff;
		papdcal_info->srom_pagc2g_ovr = 0x0;
	}

	if ((PHY_GETVAR_SLICE(pi, rstr_pagc5g)) != NULL) {
		papdcal_info->srom_pagc5g = (uint8)PHY_GETINTVAR_SLICE(pi, rstr_pagc5g);
		papdcal_info->srom_pagc5g_ovr = 0x1;
	} else {
		papdcal_info->srom_pagc5g = 0xff;
		papdcal_info->srom_pagc5g_ovr = 0x0;
	}

#if (defined(WLTEST) || defined(WLPKTENG))
	/* Read the per rate dpd enable param */
	papdcal_info->perratedpd2g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_perratedpd2g, 0);
	papdcal_info->perratedpd5g = (bool)PHY_GETINTVAR_DEFAULT_SLICE(pi, rstr_perratedpd5g, 0);
#endif // endif
}

/* ***************************** */
/*		External Definitions			*/
/* ***************************** */

void
wlc_phy_papd_set_rfpwrlut(phy_info_t *pi)
{
	int16 epsilonoffset, pga_gain;
#ifdef BCMDBG
	int16 epsregval;
#endif /* BCMDBG */
	uint8 idx, core = 0;
	txgain_setting_t txgain_settings;
	uint16 channel = 5180;
	int8 epscaldelta = 0, eps_offset = 0, delta = 0;
	uint32 rfpwrlut_table_ids[] = { ACPHY_TBL_ID_RFPWRLUTS0,
		ACPHY_TBL_ID_RFPWRLUTS1, ACPHY_TBL_ID_RFPWRLUTS2};
	uint8 subbandidx = 0;
	phy_ac_papdcal_info_t *papdi = pi->u.pi_acphy->papdcali;
	uint8 papdmode = papdi->papdmode;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	PHY_PAPD((" ------- In RFPWRLUT ------- \n"));

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
			/* 4354A0 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rfpwrlut_ptr = pga_gain_array_2g_4354;
				if ((RADIOREV(pi->pubpi->radiorev) == 0x27 ||
				     RADIOREV(pi->pubpi->radiorev) == 0x29 ||
				     RADIOREV(pi->pubpi->radiorev) == 0x28 ||
				     RADIOREV(pi->pubpi->radiorev) == 0x2C) &&
				    (PHY_XTAL_IS40M(pi))) {
					eps_offset = 18; /* 43569 */
				} else {
					eps_offset = 19;
				}
			} else {
				if (RADIOID(pi->pubpi->radioid) == BCM2069_ID &&
				    RADIOREV(pi->pubpi->radiorev) == 0x28 &&
				    PHY_XTAL_IS40M(pi)) {
					/* 43566/43567/43569/43570 A0 */
					rfpwrlut_ptr = pga_gain_array_5g_435x_radiorev40;
					eps_offset = 3;
				} else if (RADIOID(pi->pubpi->radioid) == BCM2069_ID &&
				           RADIOREV(pi->pubpi->radiorev) == 0x2C &&
				           PHY_XTAL_IS40M(pi)) {
					channel = CHSPEC_CHANNEL(pi->radio_chanspec);

					/* 43567/43570 A2 */
					if (CST4350_IFC_MODE(pi->sh->sih->chipst) ==
					    CST4350_IFC_MODE_PCIE) {
						rfpwrlut_ptr = pga_gain_array_5g_435x_radiorev40;
						if (CHSPEC_IS40(pi->radio_chanspec)) {
							eps_offset = 3;

						} else {
							eps_offset = 4;
						}
					} else {
						/* 43566/43569 A2 */
						rfpwrlut_ptr = pga_gain_array_5g_435x_radiorev44;
						if (CHSPEC_IS80(pi->radio_chanspec)) {
							if (core == 0) {
								eps_offset = 4;
							} else {
								eps_offset = 5;
							}
						} else {
							if (core == 0) {
								eps_offset = 3;
							} else {
								eps_offset = 4;
							}
						}
					}
				} else {
					rfpwrlut_ptr = pga_gain_array_5g_4354;
					if (core == 0)
						eps_offset = 2;
					else if (core == 1)
						eps_offset = 3;
				}
			}
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				/* 4350EPAPD */
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					rfpwrlut_ptr = pga_gain_array_2g_epapd[core];
					if (core == 0)
						eps_offset = -3;
					else
						eps_offset = -6;
				} else {
					channel = CHSPEC_CHANNEL(pi->radio_chanspec);
					channel = 5000 + 5 * channel;
					eps_offset = 3;
					/* pacalshift5gaX=lo,mi,hi,x1,lo,mi,hi,x1,lo,mi,hi,x1
					 *                |    20mhz |   40mhz   |   80mhz   |
					 */
					if (channel >= 5180 && channel <= 5320) {
						rfpwrlut_ptr = pga_gain_array_5g_epapd_0;
						subbandidx = 0;
					} else if (channel >= 5500 && channel <= 5620) {
						rfpwrlut_ptr = pga_gain_array_5g_epapd_1;
						subbandidx = 1;
					} else if (channel >= 5630 && channel <= 5700) {
						rfpwrlut_ptr = pga_gain_array_5g_epapd_2;
						subbandidx = 2;
					} else if (channel >= 5710 && channel <= 5825) {
						rfpwrlut_ptr = pga_gain_array_5g_epapd_3;
						subbandidx = 3;
					}
					if (CHSPEC_IS20(pi->radio_chanspec))
						eps_offset += (core == 0) ?
							papdi->pacalshift5ga0[0 + subbandidx] :
							papdi->pacalshift5ga1[0 + subbandidx];
					else if (CHSPEC_IS40(pi->radio_chanspec))
						eps_offset += (core == 0) ?
							papdi->pacalshift5ga0[4 + subbandidx] :
							papdi->pacalshift5ga1[4 + subbandidx];
					else if (CHSPEC_IS80(pi->radio_chanspec))
						eps_offset += (core == 0) ?
							papdi->pacalshift5ga0[8 + subbandidx] :
							papdi->pacalshift5ga1[8 + subbandidx];
				}
			}
		} else if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			/* 4335-4339 */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				rfpwrlut_ptr  =  pga_gain_array_2g;
				if (pi->sh->chippkg == BCM4335_FCBGA_PKG_ID) {
					delta = 1;
					eps_offset = -3;
				} else {
					if ((papdmode == PAPD_ANALYTIC) ||
						(papdmode == PAPD_ANALYTIC_WO_YREF)) {
						eps_offset = -3;
					} else {
						eps_offset = 0;
					}
				}
			} else {
				channel = CHSPEC_CHANNEL(pi->radio_chanspec);
				channel = 5000 + 5 * channel;

				if (channel >= 5180 && channel <= 5320) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_0;
				} else if (channel >= 5500 && channel <= 5620) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_1;
				} else if (channel >= 5630 && channel <= 5700) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_2;
				} else if (channel >= 5710 && channel <= 5825) {
				    rfpwrlut_ptr  =  pga_gain_array_5g_3;
				}

				if (pi->sh->chippkg == BCM4335_FCBGA_PKG_ID) {
					if (CHSPEC_IS80(pi->radio_chanspec)) {
						eps_offset = 0;
						if (channel >= 5710 && channel <= 5825)
							eps_offset = -1;
					} else {
						eps_offset = -1;
					}
				} else {
					if ((papdmode == PAPD_ANALYTIC) ||
						(papdmode == PAPD_ANALYTIC_WO_YREF)) {
						eps_offset = -5;
						if (channel >= 5710 && channel <= 5825)
							eps_offset = -6;
					} else {
						if (CHSPEC_IS80(pi->radio_chanspec)) {
							eps_offset = -1;
							if (channel == 5775)
								eps_offset = -2;
						} else {
							eps_offset = -2;
						}
					}
				}
			}
		}
		for (idx = 0; idx < 128; idx++)
		{
			wlc_phy_get_txgain_settings_by_index_acphy(pi, &txgain_settings, idx);
			pga_gain =  (txgain_settings.rad_gain_mi >> 8) & 0xff;
			if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 133);
				} else {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 207);
				}
			} else {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 167);
				} else {
					epscaldelta = (int8)*(rfpwrlut_ptr+papd_gainctrl_pga[core])
						- (int8)*(rfpwrlut_ptr + 71);
				}
			}
			epsilonoffset = (-66 + (int8)*(rfpwrlut_ptr + pga_gain)
				- epscaldelta + eps_offset + delta) << 1;

			wlc_phy_table_write_acphy(pi, rfpwrlut_table_ids[core], 1, idx,
				16, &epsilonoffset);
		}
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, 0);
#ifdef BCMDBG
		epsregval = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonOffset);
		PHY_PAPD((" ------------------ \n"));
		PHY_PAPD(("read epsilonTableAdjust In RF PowerLUT is %d\n", epsregval));
		PHY_PAPD((" ------------------ \n"));
#endif /* BCMDBG */
	}
}

void
wlc_phy_papd_set_rfpwrlut_phymaj_rev36(phy_info_t *pi)
{
	int16 epsilonoffset  = 470;
	int16 rfpwrlutval = 0;
	uint8 idx;
	uint8 core = 0;
#ifdef BCMDBG
	int16 epsregval;
#endif /* BCMDBG */
	const uint16 *gaintblptr = wlc_phy_get_txgain_tbl_20695(pi);
	uint8 bbmult;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	PHY_PAPD((" ------- In RFPWRLUT ------- \n"));
	/* Point to the epsilon offset array */

	FOREACH_ACTV_CORE(pi, stf_shdata->phyrxchain, core) {
		for (idx = 0; idx < 128; idx++)
		{
			bbmult = (gaintblptr[idx*3] & 0x00ff);
			/* An offset of 37 is added to prevent value from going too negative */
			rfpwrlutval = 37 - idx + lut_log20_bbmult[bbmult];
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFPWRLUTS0, 1, idx,
				16, &rfpwrlutval);
		}

		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, epsilonoffset);
#ifdef BCMDBG
		epsregval = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonOffset);
		PHY_PAPD((" ------------------ \n"));
		PHY_PAPD(("read epsilonTableAdjust In RF PowerLUT is %d\n", epsregval));
		PHY_PAPD((" ------------------ \n"));
#endif /* BCMDBG */
	}
}

void
wlc_phy_papd_set_rfpwrlut_tiny(phy_info_t *pi)
{
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_info_t *papdcal_info = aci->papdcali;
	int16 radiogainqdb;
	uint8 idx;
	uint16 txgain[1], bbmult;
	int16 temp, temp1, temp2, qQ, qQ1, qQ2, shift;
	uint8 scale_factor = 1;
	int8 papd_rf_pwr_scale = 32; /* Q5 format */
	int32 val = 0;
	uint8 tx_gain_tbl_id, core;
	uint8 rfpwrlut_table_ids[] = { ACPHY_TBL_ID_RFPWRLUTS0,
		ACPHY_TBL_ID_RFPWRLUTS1, ACPHY_TBL_ID_RFPWRLUTS2};

	if (CHSPEC_IS2G(pi->radio_chanspec) && (papdcal_info->parfps2g != -1)) {
		papd_rf_pwr_scale = papdcal_info->parfps2g;
	} else if (CHSPEC_IS5G(pi->radio_chanspec) && (papdcal_info->parfps5g != -1)) {
		papd_rf_pwr_scale = papdcal_info->parfps5g;
	}

	/* acphy_beDeaf??? */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	FOREACH_CORE(pi, core) {
		for (idx = 0; idx < 128; idx++)  {
			tx_gain_tbl_id = wlc_phy_get_tbl_id_gainctrlbbmultluts(pi, core);
			wlc_phy_table_read_acphy(pi, tx_gain_tbl_id, 1, idx, 48, &txgain);
			bbmult = (txgain[0] & 0xff);

			qm_log10((int32)(bbmult), 0, &temp1, &qQ1);
			qm_log10((int32)(1<<6), 0, &temp2, &qQ2);

			if (qQ1 < qQ2) {
				temp2 = qm_shr16(temp2, qQ2-qQ1);
				qQ = qQ1;
			} else {
				temp1 = qm_shr16(temp1, qQ1-qQ2);
				qQ = qQ2;
			}
			temp = qm_sub16(temp1, temp2);

			if (qQ >= 4)
				shift = qQ-4;
			else
				shift = 4-qQ;

			val = ((((idx*papd_rf_pwr_scale/32) << shift) + (5*temp) +
				(1<<(scale_factor+shift-3)))>>(scale_factor+shift-2));

			radiogainqdb = -(val)/2;

			/* No need of iteration delays for 4349 family */
			if (!ACMAJORREV_4(pi->pubpi->phy_rev)) {
				/* adding 10us of delay as table_write is throwing assert */
				OSL_DELAY(10);
			}
			wlc_phy_table_write_acphy(pi, rfpwrlut_table_ids[core], 1, idx,
				16, &radiogainqdb);
		}
	}
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	wlapi_enable_mac(pi->sh->physhim);
}

static void
phy_ac_papd_phy_cleanup(phy_info_t *pi, uint8 core)
{

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 stall_val;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(porig->is_orig);
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* Will hit assert for second core onwards without below check */
		if (core == PHYCORENUM((pi)->pubpi->phy_corenum)-1)
			porig->is_orig = FALSE;
	} else {
		porig->is_orig = FALSE;
	}

	WRITE_PHYREG(pi, RfseqCoreActv2059, porig->RfseqCoreActv2059);

	WRITE_PHYREG(pi, RfctrlCoreGlobalPus, porig->RfctrlCoreGlobalPus);
	WRITE_PHYREG(pi, RfctrlOverrideGlobalPus, porig->RfctrlOverrideGlobalPus);

/* CAL PER CORE - FOREACH_ACTV_CORE(pi, phy_stf_get_data(pi->stfi)->phyrxchain, core) */
	{

		/* Are the following three statements redundant?
			wlc_phy_txpwr_by_index_acphy would overwrite on the following anyway
		*/
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core + 0]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core + 3]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core + 6]);
		wlc_phy_txpwr_by_index_acphy(pi, (1 << core), porig->txpwridx[core]);
		wlc_phy_set_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);

		/* Restore papd cal phy registers */
		acphy_papd_cal_phyreg_sr(pi, core, porig, 0);

	}
	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);
	if (!TINY_RADIO(pi)) {
		WRITE_PHYREG(pi, lbFarrowCtrl, porig->lbFarrowCtrl);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_phy_cleanup_majorrev36(phy_info_t *pi, uint8 core)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 stall_val;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(porig->is_orig);
	porig->is_orig = FALSE;

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 0);
	}
	/* Restore the PHY registers modified during setup */
	phy_ac_reg_cache_restore(pi_ac, PHYREGS_PAPDCAL);

	/* Restore the gains */
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core + 0]);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core + 3]);
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core + 6]);
	wlc_phy_txpwr_by_index_acphy(pi, (1 << core), porig->txpwridx[core]);
	wlc_phy_set_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);

	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_phy_cleanup_majorrev44(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 stall_val, core;

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(porig->is_orig);
	porig->is_orig = FALSE;

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 0);
	}
	/* Restore the PHY registers modified during setup */
	phy_ac_reg_cache_restore(pi_ac, PHYREGS_PAPDCAL);
	FOREACH_CORE(pi, core) {
		/* Restore the gains */
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&porig->rfseq_txgain[core + 0]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&porig->rfseq_txgain[core + 3]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&porig->rfseq_txgain[core + 6]);
		wlc_phy_txpwr_by_index_acphy(pi, (1 << core), porig->txpwridx[core]);
		wlc_phy_set_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);
	}

	wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RESET2RX);

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_cal(phy_info_t *pi, uint16 num_iter, uint8 core, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
	int8 bbmult = 64;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2};

	acphy_papdCalParams_t calParams = {core, startindex, stopindex, yrefindex,
		epsilon_table_ids[core], num_iter};
	phy_ac_papdcal_info_t *papdcali = (phy_ac_papdcal_info_t *)pi->u.pi_acphy->papdcali;
	phy_ac_papdcal_params_t *params	= papdcali->papd_params;
	uint8 papdmode = papdcali->papdmode, calmode = 0;

	PHY_PAPD(("\n\nPAPD Main Cal .. "));

	/* Setting tx gain for all tx cores */
	phy_ac_papd_cal_set_tx_gain(pi, core, &bbmult, &calmode);

	/* Selecting PAPD cal mode */
	if (((papdmode == PAPD_ANALYTIC) || (papdmode == PAPD_ANALYTIC_WO_YREF)) &&
		(ACMAJORREV_3(pi->pubpi->phy_rev))) {
		/* TCL PAPDCAL Mode 2 - Analytic Cal */
		phy_ac_papd_cal_mode2(pi, &calParams, papdmode);
	} else if (APAPD_ENAB(papdcali)) {
		/* TCL PAPDCAL Mode 3 - Analytic Cal */
		phy_ac_papd_cal_mode3(pi, &calParams);
	} else if (WBPAPD_ENAB(pi)) {
		phy_ac_wbcal_run(pi, core);
	} else {
		/* TCL PAPDCAL Mode 0/1 - LMS Cal */
		phy_ac_papd_cal_mode0_1(pi, &calParams, &calmode);
	}

	/* Calculate epsilon offset, for tiny radio */
	if (TINY_RADIO(pi))
		phy_ac_papd_cal_eps_calc_tiny(pi, core, &bbmult);
	else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		uint16 eps_offset;
		eps_offset = (CHSPEC_IS5G(pi->radio_chanspec)) ?
			params->epsilon_offset_5g: params->epsilon_offset_2g;
		/* In case of 43012 set epsilon offset to predetermined number */
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
				eps_offset);
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		uint16 epsoffset;
		epsoffset = 460;
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
			epsoffset);
	}
}

/* PAPD Functions */
static void
phy_ac_papd_phy_setup(phy_info_t *pi, uint8 core)
{

	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint16 sdadc_config = 0;
	uint8 bw_idx;
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	if (CHSPEC_IS80(pi->radio_chanspec)) {
		bw_idx = 2;
		sdadc_config = sdadc_cfg80;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		bw_idx = 1;
		if (pi->sdadc_config_override)
			sdadc_config = sdadc_cfg40hs;
		else
			sdadc_config = sdadc_cfg40;
	} else {
		bw_idx = 0;
		sdadc_config = sdadc_cfg20;
	}

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(!porig->is_orig);
	if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* Will hit assert for second core onwards without below check */
		if (core == PHYCORENUM((pi)->pubpi->phy_corenum)-1)
			porig->is_orig = TRUE;
	} else {
		porig->is_orig = TRUE;
	}
	/* 4335 phy_rev is 7. For phy_rev 2, following things are required. */
	if (ACREV_IS(pi->pubpi->phy_rev, 2)) {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, rxfe_bilge_cnt, 4)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, soft_sdfeFifoReset, 1)
			MOD_PHYREG_ENTRY(pi, RxFeCtrl1, soft_sdfeFifoReset, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	porig->RfctrlOverrideGlobalPus = READ_PHYREG(pi, RfctrlOverrideGlobalPus);
	porig->RfctrlCoreGlobalPus = READ_PHYREG(pi, RfctrlCoreGlobalPus);

	/* CAL PER CORE - FOREACH_ACTV_CORE(pi, phy_stf_get_data(pi->stfi)->phyrxchain, core) */
	{
		porig->txpwridx[core] = pi->u.pi_acphy->txpwrindex[core];

		/* Save papd cal phy registers */
		acphy_papd_cal_phyreg_sr(pi, core, porig, 1);

		wlc_phy_get_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core+0]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core+3]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core+6]);

	}

	/* RfseqCoreActv2059 is core indepedant and will be modified below hence backing
	   up during core 0
	 */
	if (core == 0)
		porig->RfseqCoreActv2059 = READ_PHYREG(pi, RfseqCoreActv2059);

	/* MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, phy_stf_get_data(pi->stfi)->phyrxchain); */
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, (ACMAJORREV_4(pi->pubpi->phy_rev) ? 7 : 1 << core));
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, (ACMAJORREV_4(pi->pubpi->phy_rev) ? 7 :
		~(1 << core)));
	if (!TINY_RADIO(pi)) {
		porig->lbFarrowCtrl = READ_PHYREG(pi, lbFarrowCtrl);
		MOD_PHYREG(pi, lbFarrowCtrl, lb_decimator_output_shift, 2);
	}

/* CAL PER CORE - FOREACH_ACTV_CORE(pi, phy_stf_get_data(pi->stfi)->phyrxchain, core) */
	{

		/* acphy_rfctrlintc_override  ext_pa 1  $core */
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_2g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);

		/* acphy_rfctrlintc_override  ext_lna_pu 0  $core */
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_2g_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_5g_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_lna, 1);

		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_tx_pu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_rx_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_tr_sw, 1);

		/* Setting the SD-ADC related stuff */
		/* acphy_rfctrl_override iqadc 1 $core */
		wlc_phy_txcal_phy_setup_acphy_core_sd_adc(pi, core, sdadc_config);
		/* acphy_rfctrl_override pa_pwrup 1 $core; */
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, pa_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 1);
		/* acphy_rfctrl_override lpf_nrssi_pwrup 0 $core */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_nrssi_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup, 1);
		/* acphy_rfctrl_override wb1_pu 0 $core */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1g_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu, 1);
		} else {
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1a_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1a_pu, 1);
		}

		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_bq1_bw, 1);
		MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq1_bw, bw_idx);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_bq2_bw, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_rc_bw, 1);
		if ((RADIOREV(pi->pubpi->radiorev) == 17) ||
			(RADIOREV(pi->pubpi->radiorev) == 23) ||
		(RADIOREV(pi->pubpi->radiorev) == 25)) {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq2_bw, 7);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, lpf_rc_bw, 7);
		} else {
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT, core, lpf_bq2_bw, bw_idx + 5);
			MOD_PHYREGCE(pi, RfctrlCoreRCDACBuf, core, lpf_rc_bw, bw_idx + 5);
		}

		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);

		if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev) ||
			ACMAJORREV_4(pi->pubpi->phy_rev)) {
			MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
			MOD_PHYREGCE(pi, forceFront, core, freqCor, 1);
		}
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_phy_setup_majorrev36(phy_info_t *pi, uint8 core)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(!porig->is_orig);
	porig->is_orig = TRUE;
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 1);
	}

	/* Save the PHY override registers for papd cal */
	phy_ac_reg_cache_save(pi_ac, PHYREGS_PAPDCAL);

	wlc_phy_get_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
			&porig->rfseq_txgain[core+0]);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
			&porig->rfseq_txgain[core+3]);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
			&porig->rfseq_txgain[core+6]);

	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, 1 << core);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, ~(1 << core));

	/* Band Specific register settings */
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_2g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_2g_pu, 0);
		/* wb1_pu */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1g_pu, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu, 1);
	} else {
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_5g_pu, 0);
		/* wb1_pu */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1a_pu, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1a_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_5G_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_5G_pwrup, 1);
	}

	MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);
	MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_lna, 1);
	MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_tx_pu, 1);
	MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_rx_pu, 0);
	MOD_PHYREGCE(pi, RfctrlIntc, core, override_tr_sw, 1);

	/* Power up Rx chain */
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
	/* Power up LOGEN */
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
	/* TIA Bias PU */
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
	/* Turn off LPF nrssi */
	MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_nrssi_pwrup, 0);
	MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup, 1);

	/* Power up ADC. Need to only set the LSB bit for both i and q */
	MOD_PHYREGCE(pi, RfctrlCoreAfeCfg1, core, afe_iqadc_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideAfeCfg, core, afe_iqadc_pwrup, 1);

	/* acphy_rfctrl_override pa_pwrup 1 $core; */
	MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, pa_pwrup, 1);
	MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 1);

	/* No need to set LPF Bq1, Bq2 and RC bandwidth */

	/* Ensure DAC Bandwidth is at 120 */

	/* Enable FreqEst and FreqCorr */
	MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
	MOD_PHYREGCE(pi, forceFront, core, freqCor, 1);

	/* Enable PAPD Comp during PAPD Cal */
	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);

	ACPHY_ENABLE_STALL(pi, stall_val);
}

static void
phy_ac_papd_phy_setup_majorrev44(phy_info_t *pi)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_rxcal_phyregs_t *porig = (pi_ac->ac_rxcal_phyregs_orig);
	uint8 core;
	uint8 stall_val;
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT(!porig->is_orig);
	porig->is_orig = TRUE;
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_rfseq_mode_set(pi, 1);
	}

	/* Save the PHY override registers for papd cal */
	phy_ac_reg_cache_save(pi_ac, PHYREGS_PAPDCAL);
	MOD_PHYREG(pi, RfseqCoreActv2059, EnTx, 15);
	MOD_PHYREG(pi, RfseqCoreActv2059, DisRx, 0);
	FOREACH_CORE(pi, core) {
		wlc_phy_get_tx_bbmult_acphy(pi, &(porig->bbmult[core]), core);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x100 + core), 16,
				&porig->rfseq_txgain[core+0]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x103 + core), 16,
				&porig->rfseq_txgain[core+3]);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, (0x106 + core), 16,
				&porig->rfseq_txgain[core+6]);

		/* Band Specific register settings */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_2g_papu, 1);
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 0);
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_2g_pu, 0);
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_5g_pu, 0);
			/* wb1_pu */
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1g_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1g_pu, 1);
		} else {
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_5g_papu, 1);
			MOD_PHYREGCE(pi, RfctrlIntc, core, ext_lna_5g_pu, 0);
			/* wb1_pu */
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rssi_wb1a_pu, 0);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rssi_wb1a_pu, 1);
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_5G_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_5G_pwrup, 1);
		}

		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_pa, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_ext_lna, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_tx_pu, 1);
		MOD_PHYREGCE(pi, RfctrlIntc, core, tr_sw_rx_pu, 0);
		MOD_PHYREGCE(pi, RfctrlIntc, core, override_tr_sw, 1);

		/* Power up Rx chain */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		/* Power up LOGEN */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, logen_rx_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, logen_rx_pwrup, 1);
		/* TIA Bias PU */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, fast_nap_bias_pu, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, fast_nap_bias_pu, 1);
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_pwrup, 1);
		/* Turn off LPF nrssi */
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, lpf_nrssi_pwrup, 0);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_nrssi_pwrup, 1);

		/* acphy_rfctrl_override pa_pwrup 1 $core; */
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, pa_pwrup, 1);
		MOD_PHYREGCE(pi, RfctrlOverrideTxPus, core, pa_pwrup, 1);

		/* No need to set LPF Bq1, Bq2 and RC bandwidth */

		/* Ensure DAC Bandwidth is at 120 */

		/* Enable FreqEst and FreqCorr */
		MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
		MOD_PHYREGCE(pi, forceFront, core, freqCor, 1);

		/* Enable PAPD Comp during PAPD Cal */
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

/* Run PAPD calibration for Tiny */
static void
wlc_phy_txpwr_papd_cal_run_tiny(phy_info_t *pi,	uint8 tx_pre_cal_pwr_ctrl_state)
{
	uint8 j;
	uint8 core = 0;
	uint32 initvalue = 0;
	bool suspend = TRUE;
	uint8 yref = 5, start = 5;
	uint8 tx_atten = 3, rx_atten = 3;
	uint16 numiter = 128;
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2};
	uint8 eps_table_ids[] = {ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2, ACPHY_TBL_ID_EPSILON3};

	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_params_t *params	= aci->papdcali->papd_params;

	uint8  orig_dac_mode = phy_ac_radio_get_data(aci->radioi)->ulp_tx_mode;
	uint8  orig_adc_mode = phy_ac_radio_get_data(aci->radioi)->ulp_adc_mode;
	BCM_REFERENCE(aci);

	if (!ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* Setup DAC, ADC and Farrow */
		phy_ac_radio_set_modes(aci->radioi, params->cal_dac_mode, params->cal_adc_mode);
	}

#ifdef ATE_BUILD
	printf("===> Running PAPD cal\n");
#endif /* ATE_BUILD */

	/* PHY may be muted, e.g. for DFS/CAC, so must not transmit anything */
	if (PHY_MUTED(pi)) {
		PHY_CAL(("wl%d: %s: PHY muted - no PAPD cal\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (!ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* ADC cal is removed for 180mz ADC mode in 43012 */
		/* ADC cal coeff for 40mhz mode should be good for 180mhz as well */
		wlc_phy_dac_rate_mode_acphy(pi, 1);
	}

	FOREACH_CORE(pi, core) {
		/* clear eps table  */
		for (j = 0; j < 64; j++) {
			wlc_phy_table_write_acphy_dac_war(pi, eps_table_ids[core], 1, j, 32,
				&initvalue, core);
		}
		/* initialize scalar table */
		if (APAPD_ENAB(aci->papdcali))
			wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 128, 0, 32,
				acphy_papd_scaltbl_128);
		else
			wlc_phy_table_write_acphy(pi, scalar_table_ids[core], 64, 0, 32,
				acphy_papd_scaltbl);

		/* acphy_papd_cal_phy_setup */
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			phy_ac_papd_phy_setup_majorrev36(pi, core);
			/* Set number of iterations given in params */
			numiter = params->papd_iter;
		} else {
			if (!ACMAJORREV_44(pi->pubpi->phy_rev)) {
				phy_ac_papd_phy_setup(pi, core);
			}
		}
	}

	if (ACMAJORREV_44(pi->pubpi->phy_rev))
		phy_ac_papd_phy_setup_majorrev44(pi);
	/* Values of Tx and Rx atten set in phy_ac_populate_papd_params */
	tx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ?
		params->tx_atten_2g : params->tx_atten_5g;
	rx_atten = (CHSPEC_IS2G(pi->radio_chanspec) == 1) ?
		params->rx_atten_2g : params->rx_atten_5g;

	/* 20691/20693_loopback_papd 0 $tx_atten $rx_atten */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		if (PHY_EPAPD(pi)) {
			phy_ac_papd_radio_setup_28nm(pi, tx_atten, rx_atten, 1);
		} else {
			phy_ac_papd_radio_setup_28nm(pi, tx_atten, rx_atten, 0);
		}
	} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* currently hardcoding tx and rx atten */
		tx_atten = 3;
		rx_atten = 0;
		phy_ac_papd_radio_lpbk_setup_20697(pi, tx_atten, rx_atten, 0);
	} else {
		phy_ac_papd_radio_lpbk_setup_tiny(pi, tx_atten, rx_atten);
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev) && pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
		/* In low rate TSSI mode, adc running low,
		 * use overrideds to configure ADC to normal mode
		 */
		wlc_phy_radio20697_afe_div_ratio(pi, 1);
	}

	if (CHSPEC_IS2G(pi->radio_chanspec) && ACREV_GE(pi->pubpi->phy_rev, 4) &&
		(!PHY_EPAPD(pi)) && (!ACMAJORREV_4(pi->pubpi->phy_rev)) &&
		(!ACMAJORREV_36(pi->pubpi->phy_rev)) && (!ACMAJORREV_44(pi->pubpi->phy_rev))) {
		phy_ac_papd_rx_gain_ctrl(pi);
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev))
		wlc_dcc_fsm_reset(pi);

	/* suspend mac if haven't done so */
	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		printf("MAC was not suspended before calling wlc_phy_txpwr_papd_cal_run_acphy!\n");
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	/* Disable BT as it affects PAPD CAL */
	wlc_btcx_override_enable(pi);

	/* Disable CRS */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	FOREACH_CORE(pi, core) {
		phy_ac_papd_cal(pi, numiter, core, start, yref, 63);

		/* eps scalar */
		MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 8);

		if (!WBPAPD_ENAB(pi)) {
			/* Write the LUT table with 0's for index idx=0 upto
			 * idx=start
			 */
		for (j = 0; j < start; j++) {
			wlc_phy_table_write_acphy_dac_war(pi, eps_table_ids[core], 1, j, 32,
				&initvalue, core);
		}
	}
	}

	/* acradio papd_cal_cleanup */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_papd_radio_lpbk_cleanup_28nm(pi);
	} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		phy_ac_papd_radio_lpbk_cleanup_20697(pi);
	} else {
		phy_ac_papd_radio_lpbk_cleanup_tiny(pi);
	}

	/* phy clean up */
	FOREACH_CORE(pi, core) {
		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			phy_ac_papd_phy_cleanup_majorrev36(pi, core);
		} else {
			if (!ACMAJORREV_44(pi->pubpi->phy_rev)) {
				phy_ac_papd_phy_cleanup(pi, core);
			}
		}
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
		phy_ac_papd_phy_cleanup_majorrev44(pi);
	}

	/* Enable CRS */
	phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	/* Enable BT */
	wlc_phy_btcx_override_disable(pi);

	/* acphy_papd on */
	FOREACH_CORE(pi, core) {
		MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_mcs_dependent_comp_enable,
				(CHSPEC_IS2G(pi->radio_chanspec) &&
				CHSPEC_IS20(pi->radio_chanspec)) ? 1 : 0);
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compCckEnb, 1);
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			/* Currently enabling the papd comp for CCK in 43012 */
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compCckEnb, 1);
		} else {
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compCckEnb, 0);
		}
		MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 0);
	}
	if (ACMAJORREV_44(pi->pubpi->phy_rev) && pi->u.pi_acphy->sromi->srom_low_adc_rate_en) {
		/* Remove afe_div overrides */
		wlc_phy_radio20697_afe_div_ratio(pi, 0);
	}
	/* Restore the original DAC mode back */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_radio_set_modes(aci->radioi, orig_dac_mode, orig_adc_mode);
		wlc_phy_dac_rate_mode_acphy(pi, 1);
	}

#ifdef ATE_BUILD
		printf("===> Finished PAPD cal\n");
#endif /* ATE_BUILD */

}

void
wlc_phy_do_papd_cal_acphy(phy_info_t *pi)
{

	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_info_t *papdcal_info = aci->papdcali;
	uint8 tx_pwr_ctrl_state = pi->txpwrctrl;
	int8 tx_idx = 0;
	uint8 band;
	uint8 bands[NUM_CHANS_IN_CHAN_BONDING];

	if (PHY_AS_80P80(pi, pi->radio_chanspec)) {
		phy_ac_chanmgr_get_chan_freq_range_80p80(pi, 0, bands);
		band = bands[0];
		PHY_INFORM(("wl%d: %s: FIXME for 80P80\n", pi->sh->unit, __FUNCTION__));
	} else {
		/* Get current subband information */
		band = phy_ac_chanmgr_get_chan_freq_range(pi, 0, PRIMARY_FREQ_SEGMENT);
	}

	if (!PHY_PAPDEN(pi)) {
		uint8 core = 0;

		FOREACH_CORE(pi, core)
			MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);
		return;
	}

	PHY_PAPD(("PAPD : PHY_IPA(pi) = %d", PHY_IPA(pi)));
	ASSERT(papdcal_info != NULL);

	if (ACMAJORREV_36(pi->pubpi->phy_rev) || ACMAJORREV_44(pi->pubpi->phy_rev)) {
		/* TX Index is set inside the Cal function for 43012 */
		wlc_phy_txpwr_papd_cal_run_tiny(pi, tx_pwr_ctrl_state);

	} else if (TINY_RADIO(pi)) {

		if (!ACMAJORREV_4(pi->pubpi->phy_rev)) {
			int8 tx_idx_pwr;

			/* use for phy_pacalidx0 and phy_pacalidx1 iovar */
			papdcal_info->papd_lut0_cal_idx = -1;
			papdcal_info->papd_lut1_cal_idx = -1;

			/* 4th priority: default cal index */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				tx_idx = PHY_EPAPD(pi) ? 48: 26;
			} else {
				tx_idx = PHY_EPAPD(pi) ?
					((band <= WL_CHAN_FREQ_RANGE_5G_BAND1) ? 56 : 64) : 30;
			}

			/* 3rd priority: pacalindex from nvram */
			if (CHSPEC_IS2G(pi->radio_chanspec) &&
					papdcal_info->pacalindex2g != -1) {
				tx_idx = papdcal_info->pacalindex2g;
			} else {
				if ((band <= WL_CHAN_FREQ_RANGE_5G_BAND1) &&
						(papdcal_info->pacalindex5g[0] != -1)) {
					tx_idx = papdcal_info->pacalindex5g[0];
				} else if ((band == WL_CHAN_FREQ_RANGE_5G_BAND2) &&
						(papdcal_info->pacalindex5g[1] != -1)) {
					tx_idx = papdcal_info->pacalindex5g[1];
				} else if ((band == WL_CHAN_FREQ_RANGE_5G_BAND3) &&
						(papdcal_info->pacalindex5g[2] != -1)) {
					tx_idx = papdcal_info->pacalindex5g[2];
				}
			}

			/* 2nd priority: pacalpwr from nvram */
			tx_idx_pwr = wlc_phy_tone_pwrctrl(pi, 96, 0);
			if (tx_idx_pwr != -1)
				tx_idx = tx_idx_pwr;

			/* 1st priority: force cal index through iovar */
			if (papdcal_info->pacalidx_iovar != -1)
				tx_idx = papdcal_info->pacalidx_iovar;

			papdcal_info->papd_lut0_cal_idx = tx_idx;

		} else {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				tx_idx = PHY_EPAPD(pi) ? 48: 26;
			} else {
				tx_idx = PHY_EPAPD(pi) ?
					((band <= WL_CHAN_FREQ_RANGE_5G_BAND1) ? 36 : 44): 22;
			}
		}

		wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);

		wlc_phy_cals_mac_susp_en_other_cr(pi, TRUE);
		wlc_phy_txpwr_papd_cal_run_tiny(pi, tx_pwr_ctrl_state);
		wlc_phy_cals_mac_susp_en_other_cr(pi, FALSE);

	} else {
		wlc_phy_txpwr_papd_cal_run_acphy(pi, tx_pwr_ctrl_state);
	}
}

void
wlc_phy_get_papd_cal_pwr_acphy(phy_info_t *pi, int8 *targetpwr, int8 *returned_tx_idx, uint8 core)
{
	if (PHY_PAPDEN(pi)) {
		phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
		phy_ac_papdcal_info_t *papdcal_info = aci->papdcali;
		uint8 band;
		uint8 pacalpwr_idx;
		uint8 core_freq_segment_map;
		int8 tx_idx = -1;

		ASSERT(papdcal_info != NULL);

		/* core_freq_segment_map is only required for 80P80 mode.
		For other modes, it is ignored
		*/
		core_freq_segment_map =
			phy_ac_chanmgr_get_data(aci->chanmgri)->core_freq_mapping[core];

		/* Get current subband information */
		band = phy_ac_chanmgr_get_chan_freq_range(pi, 0, core_freq_segment_map);

		switch (band) {
		case WL_CHAN_FREQ_RANGE_2G:
			*targetpwr = papdcal_info->pacalpwr2g;
			tx_idx = papdcal_info->patoneidx2g;
			break;
		case WL_CHAN_FREQ_RANGE_5G_BAND0:
		case WL_CHAN_FREQ_RANGE_5G_BAND1:
		case WL_CHAN_FREQ_RANGE_5G_BAND2:
		case WL_CHAN_FREQ_RANGE_5G_BAND3:
			pacalpwr_idx = band - 1;
			*targetpwr = papdcal_info->pacalpwr5g[core * 4 + pacalpwr_idx];
			if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
					*targetpwr =
						papdcal_info->pacalpwr5g40[core * 4 + pacalpwr_idx];
				} else if (CHSPEC_IS80(pi->radio_chanspec)) {
					*targetpwr =
						papdcal_info->pacalpwr5g80[core * 4 + pacalpwr_idx];
				}
			}
			tx_idx = papdcal_info->patoneidx5g[pacalpwr_idx];
			break;
		default:
			;
		}

		if (tx_idx != -1) {
			*returned_tx_idx = tx_idx;
		}
	}
}

/* TCL PAPDCAL Mode 0/1 - LMS Cal */
void
phy_ac_papd_cal_mode0_1(phy_info_t *pi, acphy_papdCalParams_t *calParams,  uint8 *calmode)
{

	uint8 core = calParams->core, epsilon_table_id = calParams->epsilon_table_id;
	uint16 m[4] = {0, 0, 0, 0};
	uint16 startindex = calParams->startindex, stopindex = calParams->stopindex;
	uint16 yrefindex = calParams->yrefindex, num_iter = calParams->num_iter;
	int8 coremask = 1 << core;
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	phy_ac_papdcal_info_t *papdi = pi->u.pi_acphy->papdcali;
	uint8 edpdcalset = papdi->edpdcalset;
	BCM_REFERENCE(aci);

	ASSERT(aci->papdcali != NULL);

	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 1);

	if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		if (core == 0) {
			MOD_PHYREGCEE(pi, PapdEnable, 1, papd_compEnb, 0);
			MOD_PHYREGCEE(pi, PapdCalShifts, 1, papd_calEnb, 0);
		} else if (core == 1) {
			MOD_PHYREGCEE(pi, PapdEnable, 0, papd_compEnb, 0);
			MOD_PHYREGCEE(pi, PapdCalShifts, 0, papd_calEnb, 0);
		}
	}
	if (TINY_RADIO(pi)) {
		if ((CHSPEC_IS80(pi->radio_chanspec)) && (PHY_EPAPD(pi))) {
			ACPHY_REG_LIST_START
				MOD_PHYREG_ENTRY(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, 256)
				MOD_PHYREG_ENTRY(pi, PapdCalSettle, papd_calSettleTime,
					0x80)
				MOD_PHYREG_ENTRY(pi, PapdCalCorrelate, papd_calCorrTime,
					0x100)
			ACPHY_REG_LIST_EXECUTE(pi)
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x7);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x9);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x9);
		} else {
			MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations,
					(CHSPEC_IS2G(pi->radio_chanspec))?32:16);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I,
					(CHSPEC_IS2G(pi->radio_chanspec))?0x9:0x7);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q,
					(CHSPEC_IS2G(pi->radio_chanspec))?0x9:0x7);

				if (CHSPEC_IS80(pi->radio_chanspec)) {
					MOD_PHYREG(pi, PapdCalCorrelate,
						papd_calCorrTime, 0x80);
					MOD_PHYREG(pi, PapdCalSettle,
						papd_calSettleTime,
						(CHSPEC_IS2G(pi->radio_chanspec))
						?0x80:0x80);
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
						papdCorrShift, 0x6);

				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
						(CHSPEC_IS2G(pi->radio_chanspec))
						?0x40:0x80);
					MOD_PHYREG(pi, PapdCalSettle,
						papd_calSettleTime,
						(CHSPEC_IS2G(pi->radio_chanspec))
						?0x40:0x40);
					MOD_PHYREGCEE(pi, PapdCalShifts,
						core, papdCorrShift,
						(CHSPEC_IS2G(pi->radio_chanspec))?0x4:0x6);
				} else {
					MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
						(CHSPEC_IS2G(pi->radio_chanspec))
						?0x20:0x40);
					MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime,
						(CHSPEC_IS2G(pi->radio_chanspec))
						?0x20:0x20);
					MOD_PHYREGCEE(pi, PapdCalShifts,
						core, papdCorrShift,
						(CHSPEC_IS2G(pi->radio_chanspec))?0x3:0x5);
				}

			} else {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
				MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, num_iter);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
			}
			if (aci->papdcali->pacalopt == 1 || aci->papdcali->pacalopt == 2) {
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x100);
				MOD_PHYREG(pi, PapdCalShifts0, papdCorrShift0, 0x7);
			}
		}
	} else {
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			/* For One cores */
			MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
			        epsilonUpdateIterations, num_iter);
			/* setup LMS convergence related params */
			if (CHSPEC_IS80(pi->radio_chanspec)) {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x40);
			} else {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x20);
			}
			MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x8);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x8);
			} else {
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0x9);
				MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0x9);
			}
		} else if (ACMAJORREV_2(pi->pubpi->phy_rev) ||
		           ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* For 4350 */
			MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
			        epsilonUpdateIterations, num_iter);
			/* setup LMS convergence related params */
			if (CHSPEC_IS80(pi->radio_chanspec)) {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x100);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x40);
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x80);
			} else {
				MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x20);
				MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
			}
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (PHY_EPAPD(pi)) {
					if (edpdcalset == 2) {
						/* For MSC 5G FEM */
						if ((CHSPEC_IS20(pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x20);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x100);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x6);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0x8);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0x8);
						} else if ((CHSPEC_IS40
							    (pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x20);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x100);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x6);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0x8);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0x8);
						} else {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x80);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x100);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x4);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0xa);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0xa);
						}
					} else if (edpdcalset == 1) {
						/* For Triquint 5G FEM */
						if ((CHSPEC_IS20(pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x20);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x40);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x4);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0xa);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0xa);
						} else if ((CHSPEC_IS40
							    (pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x40);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x80);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x5);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0xa);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0xa);
						} else {
							MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
							    epsilonUpdateIterations, 0x80);
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x40);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x80);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdCorrShift, 0x7);
							MOD_PHYREG(pi, PapdIpaOffCorr,
							    papd_calIpaOffCorr, 0x40);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_I, 0x9);
							MOD_PHYREGCEE(pi, PapdCalShifts,
							    core, papdLambda_Q, 0x9);
						}
					} else if (edpdcalset == 0) {
						/* For Skyworks 5G FEM */
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdCorrShift, 0x4);
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_I, 0x8);
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_Q, 0x8);
						if ((CHSPEC_IS20(pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x20);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x40);
						} else if ((CHSPEC_IS40
							    (pi->radio_chanspec))) {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x40);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x80);
						} else {
							MOD_PHYREG(pi, PapdCalSettle,
							    papd_calSettleTime, 0x80);
							MOD_PHYREG(pi, PapdCalCorrelate,
							    papd_calCorrTime, 0x100);
						}
					}
				} else {
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
					    papdLambda_I, 0x7);
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
					    papdLambda_Q, 0x7);
				}
			} else {
				if (PHY_EPAPD(pi)) {
					MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					    epsilonUpdateIterations, num_iter);
					MOD_PHYREG(pi, PapdCalSettle,
					    papd_calSettleTime, 0x80);
					MOD_PHYREG(pi, PapdCalCorrelate,
					    papd_calCorrTime, 0x40);
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
					    papdCorrShift, 0x4);
					MOD_PHYREG(pi, PapdIpaOffCorr,
					    papd_calIpaOffCorr, 0x0);
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
					    papdLambda_I, 0x8);
					MOD_PHYREGCEE(pi, PapdCalShifts, core,
					    papdLambda_Q, 0x8);
				} else {
					if (PHY_IPA(pi) &&
					    !ACMAJORREV_5(pi->pubpi->phy_rev)) {
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_I, 0x8);
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_Q, 0x8);
					} else {
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_I, 0x9);
						MOD_PHYREGCEE(pi, PapdCalShifts, core,
						    papdLambda_Q, 0x9);
					}
				}
			}
		} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			phy_ac_papdcal_params_t *params	= pi->u.pi_acphy->papdcali->papd_params;

			MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, num_iter);
			MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime,
					params->papd_cal_settle);
			MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
					params->papd_cal_corr);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift,
					params->papd_corr_shift);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
		} else if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
					epsilonUpdateIterations, 64);
			MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime,
					64);
			MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime,
					64);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift,
					4);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
			MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0);
			MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, 5);
			MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 0x8);
			MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
		} else {
			MOD_PHYREG(pi, PapdEpsilonUpdateIterations,
				epsilonUpdateIterations, num_iter);
			MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
			MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x40);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, 0x4);
			MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x0);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
			MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
		}
	}
	if (!ACMAJORREV_44(pi->pubpi->phy_rev))
		MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);

	/* use s2.10 PAPD epsilon fixed point format */
	MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
	if (*calmode == 0) {
		/* Run the PAPD automatic machine on all indices */
		/* setup iter, Yref, start and end address */
		MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, startindex);
		MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, stopindex);
		if (ACMAJORREV_44(pi->pubpi->phy_rev)) {
			if (core == 0) {
				m[0] = 90;
				wlc_phy_txpwr_by_index_acphy(pi, coremask, 0);
				wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], 3);
			} else {
				m[0] = 0;
				m[1] = 90;
				wlc_phy_txpwr_by_index_acphy(pi, coremask, 0);
				wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], 3);
			}
		}
		if (gain_ctrl == 0) {
			(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186,
				TX_TONE_IQCAL_MODE_OFF, FALSE);
		}
		OSL_DELAY(100);
		/* start PAPD calibration */
		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
		MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		if ((READ_PHYREG(pi, PapdCalStart) & 1)) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR :"
				" PAPD cal failed \n", __FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_PAPD_CAL_FAILED);
		}
		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		if (gain_ctrl == 0) {
			wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
		}
	} else { /* cal mode 1, hardware cal runs single index at a time */
		uint16 mtable_idx;
		uint32 eps_pre, eps, eps_next;
		int32 epspre_r, epspre_i, eps_r, eps_i, epsnext_r, epsnext_i;
		/* run single index of PAPD table only */
		/* (mode 1 which has been used for debug of 4335 PAPD) */
		/* start the tone */
		if (gain_ctrl == 0) {
			(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186,
				TX_TONE_IQCAL_MODE_OFF, FALSE);
		}
		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
		OSL_DELAY(600);

		for (mtable_idx = startindex; mtable_idx <= stopindex; mtable_idx++) {
			MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);

			MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit,
			           (mtable_idx == startindex) ? 0x0 : 0x1);

			MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, mtable_idx);
			MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, mtable_idx);

			/* start PAPD calibration */
			OSL_DELAY(10);
			MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
			SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
			if ((READ_PHYREG(pi, PapdCalStart) & 1)) {
				PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR :"
					" PAPD cal failed \n", __FUNCTION__));
				PHY_FATAL_ERROR(pi, PHY_RC_PAPD_CAL_FAILED);
			}
			SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
			OSL_DELAY(10);

			/* predict the next epsilon point */
			wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_id, 1,
				mtable_idx-1, 32, &eps_pre, core);
			wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_id, 1,
				mtable_idx, 32, &eps, core);

			if (aci->papdcali->pacalopt == 2) {
				eps_next = eps;
			} else {
				/* linear extrapolation of prev 2 points to set
				starting point for next eps
				*/

				phy_papdcal_decode_epsilon(eps_pre, &epspre_r, &epspre_i);
				phy_papdcal_decode_epsilon(eps, &eps_r, &eps_i);
				if (mtable_idx == startindex) {
					epsnext_r = eps_r;
					epsnext_i = eps_i;
				} else {
					epsnext_r = 2*eps_r-epspre_r;
					epsnext_i = 2*eps_i-epspre_i;
				}
				if (epsnext_r >= 4095) {
					epsnext_r = 4095;
				}
				if (epsnext_r <= -4095) {
					epsnext_r = -4095;
				}
				if (epsnext_r < 0) {
					epsnext_r = 8192+epsnext_r;
				}
				if (epsnext_i >= 4095) {
					epsnext_i = 4095;
				}
				if (epsnext_i <= -4095) {
					epsnext_i = -4095;
				}
				if (epsnext_i < 0) {
					epsnext_i = 8192+epsnext_i;
				}
				eps_next = ((uint32)epsnext_i << 13) |
					((uint32)epsnext_r & 0x1fff);
			}

			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_id,
				1, mtable_idx+1, 32, &eps_next, core);

			PHY_PAPD(("\n We are in %u M table iteration", mtable_idx));
		}
		if (gain_ctrl == 0) {
			wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
		}
	}
	if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi) &&
	    CHSPEC_IS2G(pi->radio_chanspec)) {
		phy_ac_papd_smooth(pi, core, 5, 0, 32);
	}
}

/* WBPAPD core function */
static int
phy_ac_wbcal_run(phy_info_t *pi, uint8 core)
{
	struct write_regs {
		uint16 reg_addr;
		uint16 reg_val;
	};

	phy_iq_comp_t iqcoeffs;
	int i;
	int err = BCME_OK;
	int initPval = 0x7E49;
	int initEpsVal = 0;
	uint8 stall_val = 0;
	uint8 stop_idx = 63;

	phy_ac_papdcal_params_t *params	= pi->u.pi_acphy->papdcali->papd_params;
	const uint16 wbcal_phyreg00_val = (params->wbcal_phyreg0_val);

	/* Set wbcal specific variables from param */
	uint16 lut_step_lo = (uint16) (params->wbcal_lutstep_dB & 0xFFFF);
	uint8 lut_step_hi  = (uint8) ((params->wbcal_lutstep_dB >> 16) & 0xF);
	uint16 start_lo = (uint16)(params->wbcal_start & 0xFFFF);
	uint8 start_hi  = (uint8)((params->wbcal_start >> 16) & 0x3F);
	uint16 stop_lo = (uint16)(params->wbcal_end & 0xFFFF);
	uint8 stop_hi  = (uint8)((params->wbcal_end >> 16) & 0xF);
	uint16 scale_start_lo = (uint16)(params->wbcal_scale_start & 0xFFFF);
	uint8 scale_start_hi  = (uint8)((params->wbcal_scale_start >> 16) & 0xF);
	uint16 scale_stop_lo = (uint16)(params->wbcal_scale_end & 0xFFFF);
	uint8 scale_stop_hi  = (uint8)((params->wbcal_scale_end >> 16) & 0x3F);
	uint8 scale_hi = ((scale_start_hi <<4) | (scale_stop_hi));
	uint16 bbmult = CHSPEC_IS2G(pi->radio_chanspec) ?
			params->bbmult_2g : params->bbmult_5g;
	uint16 epsilon_offset = CHSPEC_IS2G(pi->radio_chanspec) ?
			params->epsilon_offset_2g : params->epsilon_offset_5g;
	uint16 ref_dB = CHSPEC_IS2G(pi->radio_chanspec) ?
		params->cal_refdb_2g : params->cal_refdb_5g;
	uint16 delayfilt_gamma =  CHSPEC_IS2G(pi->radio_chanspec) ?
		params->delayfilt_gamma_2g : params->delayfilt_gamma_5g;
	uint16 bufoffset =  CHSPEC_IS2G(pi->radio_chanspec) ?
		params->buf_offset_2g : params->buf_offset_5g;
	// ref_dB = WBPAPD_REFDB_BASE  + ((ref_dB) * WBPAPD_REFDB_STEP);
	uint8 wbcal_gfrac_bits = CHSPEC_IS2G(pi->radio_chanspec) ?
		(params->wbcal_gfrac_bits & 0x0F) :
		((params->wbcal_gfrac_bits >> 4) & 0x0F);
	uint8 papd_calidx = CHSPEC_IS2G(pi->radio_chanspec) ?
		params->papd_calidx_2g : params->papd_calidx_5g;

	struct write_regs reg_list [WBPAPD_REGLIST_SIZE] = {
		{ ACPHY_REG(pi, wbcal_ctl_50), WBPAPD_REFDB_BASE  + ((ref_dB)*WBPAPD_REFDB_STEP) },
		{ ACPHY_REG(pi, wbcal_ctl_60), lut_step_lo},  /* LUT Step dB */
		{ ACPHY_REG(pi, wbcal_ctl_70), lut_step_hi}, /* LUT step dB hi */
		{ ACPHY_REG(pi, wbcal_ctl_80), params->wbcal_lut_len}, /* LUT LEN */
		{ ACPHY_REG(pi, wbcal_ctl_E0), start_lo}, /* START Sample */
		{ ACPHY_REG(pi, wbcal_ctl_F0), start_hi},
		{ ACPHY_REG(pi, wbcal_ctl_100), stop_lo}, /* STOP Sample */
		{ ACPHY_REG(pi, wbcal_ctl_110), stop_hi},
		{ ACPHY_REG(pi, wbcal_ctl_2d0), scale_start_lo}, /* scale start lo */
		{ ACPHY_REG(pi, wbcal_ctl_2e0), scale_stop_lo},   /* scale stop lo */
		{ ACPHY_REG(pi, wbcal_ctl_2f0), scale_hi}   /* scale start and stop hi */
	};

	MOD_PHYREG(pi, wbcal_ctl_210, wb_mem_access_sel, 0);
	for (i = 0; i < 128; i++) {
		wlc_phy_table_write_acphy_dac_war(pi, ACPHY_TBL_ID_WBCAL_PTBL0,
				1, i, 32, &initPval, 0);
		wlc_phy_table_write_acphy_dac_war(pi, ACPHY_TBL_ID_EPSILON0,
				1, i, 32, &initEpsVal, 0);
	}
	MOD_PHYREG(pi, wbcal_ctl_210, wb_mem_access_sel, 1);

	/* Read RX IQ Cal coeffs */
	iqcoeffs.a = 0;
	iqcoeffs.b = 0;
	wlc_phy_rx_iq_comp_acphy(pi, 0, &(iqcoeffs), core);
	WRITE_PHYREG(pi, wbcal_ctl_120, iqcoeffs.a);
	WRITE_PHYREG(pi, wbcal_ctl_130, iqcoeffs.b);

	for (i = 0; i < WBPAPD_REGLIST_SIZE; i++) {
		phy_utils_write_phyreg(pi, reg_list[i].reg_addr, reg_list[i].reg_val);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, wbcal_ctl_00, wbcal_on, 0)
		MOD_PHYREG_ENTRY(pi, wbcal_ctl_00, wbcal_trig, 0)
		MOD_PHYREG_ENTRY(pi, wbcal_ctl_220, wbcal_dc_accum_wait_lo, 12) /* dc_accum_wait */
	ACPHY_REG_LIST_EXECUTE(pi);

	/* G Frac bits */
	MOD_PHYREG(pi, wbcal_ctl_210, wbpapd_cal_g_frac_bits, wbcal_gfrac_bits);
	MOD_PHYREG(pi, wbcal_ctl_220, wbpapd_cal_dcc_en, params->wbcal_dcc_en);  /* DCC Enable */
	/* Delay Filter */
	MOD_PHYREG(pi, wbcal_ctl_220, wbpapd_delay_filter_en, params->wbcal_dly_filt_en);
	MOD_PHYREG(pi, wbcal_ctl_230, wbpapd_cal_dc_corr_override_en, params->wbcal_dccorr_ovr_en);
	MOD_PHYREG(pi, wbcal_ctl_240, wbpapd_cal_dc_offset_value_real,
			params->wbcal_dcoffset_real);
	MOD_PHYREG(pi, wbcal_ctl_240, wbpapd_cal_dc_offset_value_imag,
			params->wbcal_dcoffset_imag);
	/* 2 tbl enble */
	MOD_PHYREG(pi, wbcal_ctl_250, wbpapd_two_table_enable, params->wbcal_twotbl_en);
	MOD_PHYREG(pi, wbcal_ctl_1B0, wbpapd_cal_const_pow_scale, params->wbcal_const_pow_scale);

	WRITE_PHYREG(pi, wbcal_ctl_00, wbcal_phyreg00_val);

	MOD_PHYREG(pi, wbcal_ctl_10, wbcal_txbuf_offset, bufoffset);
	MOD_PHYREG(pi, wbcal_ctl_220, wbpapd_filter_delay_gamma, delayfilt_gamma);

	/* Turn off papd before WBCAL */
	// comp_enable = READ_PHYREGFLDCEE(pi, PapdEnable, core, papd_compEnb);
	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0);

	/* Set index and bbmult */
	wlc_phy_txpwr_by_index_acphy(pi, (uint8)(1 << core), papd_calidx);
	wlc_phy_set_tx_bbmult_acphy(pi, &bbmult, core);

	/* disable Stalls  before mac play and restore it later */
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 0x1);
	MOD_PHYREG(pi, sampleCmd, stop, 0x1);

	/* Transmit packet from MAC FIFO */
	/* sizeof gives sizeof array in bytes and we want it in words */
	macplay_wfm_ptr = get_wbpapd_wfm_43012(pi);

	/* Enable WBCAL */
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_en, 1);
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_on, 1);
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_trig, 1);

	err = phy_ac_papd_mac_play(pi, macplay_wfm_ptr, params->wbcal_waveform_sz, ON);
	if (err != BCME_OK) {
		PHY_ERROR(("%s: MAC-PLAY START FAILED\n", __FUNCTION__));
		goto fail;
	}

	/* Wait for 300 microseconds */
	OSL_DELAY(750);

	/* ToDo: Stop playback from MAC FIFO */
	err = phy_ac_papd_mac_play(pi, NULL, 0, OFF);
	if (err != BCME_OK) {
		PHY_ERROR(("%s: MAC-PLAY STOP FAILED\n", __FUNCTION__));
		goto fail;
	}
	ACPHY_ENABLE_STALL(pi, stall_val);

	/* Stop WBCAL */
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_on, 0);
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_trig, 0);
	MOD_PHYREG(pi, wbcal_ctl_00, wbcal_en, 0);

	/* Calcullate the stop index */
	stop_idx = phy_ac_wbcal_eps_stopidx(pi, core);

	/* Set comp to same precisison as Cal: 11 bits */
	if (wbcal_gfrac_bits == 11)
		params->wbcal_eps_fxdpt = 0;
	else if (wbcal_gfrac_bits == 10)
		params->wbcal_eps_fxdpt = 2;
	else
		params->wbcal_eps_fxdpt = 1; /* 12 bit case */

	MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, params->wbcal_eps_fxdpt);
	MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset, epsilon_offset);
	MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonScalar, 8);
	MOD_PHYREGCEE(pi, PapdPolarSaturation0, core, stop_index, stop_idx);

	/* Restore the value of PAPD COMP enable bit */
	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 0x1);

fail:
	return err;
}

static uint8
phy_ac_wbcal_eps_stopidx(phy_info_t *pi, uint8 core)
{
	uint8 stop_idx = ACPHY_PAPD_EPS_TBL_SIZE - 1;
	uint32 eps_table[ACPHY_PAPD_EPS_TBL_SIZE];
	uint8 idx;
	uint8 epsilon_table_ids[] = { ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1,
		ACPHY_TBL_ID_EPSILON2};

	wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core],
		ACPHY_PAPD_EPS_TBL_SIZE, 0, 32, eps_table, core);
	for (idx = stop_idx; idx > 0; idx--) {
		if (eps_table[idx] > 0) {
			break;
		}
	}
	stop_idx = idx;
	return stop_idx;
}

static int
phy_ac_papd_mac_play(phy_info_t *pi, const uint32* buf, uint16 len, bool start)
{
	phy_ac_papdcal_params_t *params = pi->u.pi_acphy->papdcali->papd_params;
	uint32 startidx = params->wbcal_macbuf_offset;

	if (start) {
		if (ACMAJORREV_36(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
#ifdef USE_FCBS_ROM
			int32 fcbs_val;
			/* In case of 43012 B0 and later */
			/* Get Data pointer to FCBS ROM or RAM */
			fcbs_val = fcbs_rom_metadata(FCBSROM_PAPD_WAVEFORM, params->ft);

			if (fcbs_val == FCBS_SEQ_IN_RAM) {
				/* if FCBS Sequence in RAM write sequence to BM */
				ASSERT(params->ft->dat_ptr != NULL);
				wlapi_bmac_write_template_ram(pi->sh->physhim, startidx,
					len*sizeof(uint32), params->ft->dat_ptr);
			} else if (fcbs_val == FCBS_SEQ_IN_ROM) {
				/* If FCBS sequence in ROM modify start index to play
				 * Note that the offset is from 96k so adding 96K
				 */
				startidx = (uint32)(params->ft->dat_ptr) + (96*1024);
			} else {
				/* In case of error */
				PHY_ERROR(("%s: WBPAPD Waveform not found \n", __FUNCTION__));
				ASSERT(0);
			}
#else
		if (ACMINORREV_1(pi)) {
			/* ROM address in 43012B0 is hardcoded here without enabling USE_FCBS_ROM */
			startidx = WPAPD_MAC_PLAY_ROM_ADDR_43012B0;
		} else {
			/* Load the waveform into MAC buffer */
			ASSERT(buf != NULL);
			/* multiply len by 4 since function expects length in bytes */
			wlapi_bmac_write_template_ram(pi->sh->physhim, startidx,
					len*sizeof(uint32), buf);
		}
#endif /* USE_FCBS_ROM */
		} else {
			/* Load the waveform into MAC buffer */
			ASSERT(buf != NULL);
			/* multiply len by 4 since function expects length in bytes */
			wlapi_bmac_write_template_ram(pi->sh->physhim, startidx,
					len*sizeof(uint32), buf);
		}
		W_REG(pi->sh->osh, D11_SCP_STRTPTR(pi), startidx/4);
		W_REG(pi->sh->osh, D11_SCP_STOPPTR(pi), (startidx/4)+len-1);

		/* Play the Waveform */
		phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
				ACPHY_macbasedDACPlay_macBasedDACPlayEn_MASK(phy_rev));

		if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
			if (CHSPEC_IS80(pi->radio_chanspec)) {
				phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x3 <<
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x2 <<
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
			} else {
				phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x1 <<
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
			}
		} else {
			/* In case of 43012 */
			/*
			phy_utils_mod_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
				ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev),
				0x2);
			*/
			/* Set the MAC mode to 0x2 and enable the MAC play */
			phy_utils_write_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev), 0x5);
		}

		PHY_TRACE(("Starting MAC based Sample Play"));
		wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);

		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
			uint16 SampleCollectPlayCtrl =
				R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
			SampleCollectPlayCtrl |= (1 << SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT);
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
					SampleCollectPlayCtrl);
		} else {
			uint16 phy_ctl;
			phy_ctl = (1 << PHYCTRL_SAMPLEPLAYSTART_SHIFT)
				| (1 << PHYCTRL_MACPHYFORCEGATEDCLKSON_SHIFT);
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
		}
	} else {
		/* To stop playback */
		bool mac_sample_play_on = 0;
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
			uint16 SampleCollectPlayCtrl =
				R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
			mac_sample_play_on =
				((SampleCollectPlayCtrl >>
				  SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT) & 1);

			if (mac_sample_play_on) {
				/* Make the 9th bit zero */
				SampleCollectPlayCtrl &= 0xFDFF;
				W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
					SampleCollectPlayCtrl);

			}
		} else {
			uint16 phy_ctl;
			phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));

			if (phy_ctl & 0x800) {
				W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi),
					(phy_ctl & 0xf7ff));
			}

		}
		/* Stop MAC play  by setting macBasedDACPlayEn to 0 */
		phy_utils_write_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev), 0x4);
	}  /* End of start */

	return BCME_OK;
}

/* TCL PAPDCAL Mode 2 - Analytic Cal */
void
phy_ac_papd_cal_mode2(phy_info_t *pi, acphy_papdCalParams_t *calParams, uint8 papdmode)
{
	uint8 core = calParams->core, epsilon_table_id = calParams->epsilon_table_id;
	uint16 startindex = calParams->startindex, stopindex = calParams->stopindex;
	uint16 yrefindex = calParams->yrefindex;
	bool   corr_sat = 0;
	uint8  corr_shift = 0x4;
	uint16 index;
	int32  mag_corr_papd_index;
	uint32 dst[64];
	uint32 dst_tmp = 0;
	uint32 dst_limit = 0;
	uint16 next_index_interp = startindex;
	uint16 next_index_write = startindex;
	uint16 next_index_interp_init = startindex;
	int16  abs_c, eps_re_curr = 0, eps_im_curr = 0,
	       eps_re_prev = 0, eps_im_prev = 0,
	       eps_re_interp = 0, eps_im_interp = 0;
	int16  corr_papd_index_I = 0, corr_papd_index_Q = 0,
	       corr_papd_index_I_yref = 0, corr_papd_index_Q_yref = 0;
	uint16 vin_prev = 0, vin_curr = 0, vin_p[64];
	/* When startindex == yrefindex, the hw cal for the startindex takes long time */
	if (startindex == yrefindex)
		startindex ++;
	/* dst_limit to reduce variation on AMAM/AMPM for last few entries */
	if (CHSPEC_IS5G(pi->radio_chanspec)) {
		if (CHSPEC_IS20(pi->radio_chanspec)) {
			dst_limit = 54358000; /* 1.8 */
		} else if (CHSPEC_IS40(pi->radio_chanspec)) {
			dst_limit = 54358000; /* 1.8 */
		} else {
			dst_limit = 42928704; /* 1.6 */
		}
	} else {
		dst_limit = 54358000; /* 1.8 */
	}
	for (index = 0; index <= 63; index++) {
		vin_p[index] = (uint16)(acphy_papd_scaltbl[index] & 0xffff);
	}
	MOD_PHYREGCEE(pi, PapdEnable, core, papd_compEnb, 1);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papd_calEnb, 1);
	MOD_PHYREG(pi, PapdEpsilonUpdateIterations, epsilonUpdateIterations, 1);
	MOD_PHYREG(pi, PapdCalSettle, papd_calSettleTime, 0x80);
	MOD_PHYREG(pi, PapdCalCorrelate, papd_calCorrTime, 0x140);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papdCorrShift, corr_shift);
	MOD_PHYREG(pi, PapdIpaOffCorr, papd_calIpaOffCorr, 0x3ff);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref, 0x1);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdEpsilonInit, 0x0);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_I, 0xa);
	MOD_PHYREGCEE(pi, PapdCalShifts, core, papdLambda_Q, 0xa);
	MOD_PHYREG(pi, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);
	MOD_PHYREGCE(pi, EpsilonOverrideI_, core, epsilonFixedPoint, 0x1);
	/* Looping over yrefindex + (startindex -> stopindex); */
	if (gain_ctrl == 0) {
		(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186,
			TX_TONE_IQCAL_MODE_OFF, FALSE);
	}
	for (index = startindex; index <= stopindex+1; corr_sat? index : index++) {
		if (corr_sat) {
			MOD_PHYREGCEE(pi, PapdCalShifts, core,
				papdCorrShift, ++corr_shift);
		}

		if ((index == yrefindex) || ((index >= startindex) &&
			(index <= stopindex))) {
			MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, index);
			MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, index);
			MOD_PHYREG(pi, PapdCalYrefEpsilon, papdInitYref,
				(papdmode == PAPD_ANALYTIC_WO_YREF) ?
				((index == startindex) ? 0x1 : 0x0) : 0x1);
			/*
			   printf("PAPD_ANALYTIC_WO_YREF= %d,YREF_INIT=%d \n",
			   papdmode, (papdmode == PAPD_ANALYTIC_WO_YREF)
			   ? ((index == startindex) ? 0x1 : 0x0) : 0x1);
			   */
			WRITE_PHYREG(pi, papdCalCorrDebugAddr, index);
			MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
			MOD_PHYREG(pi, PapdCalStart, papdStart, 1);
		}

		if (((yrefindex == startindex) && (index > startindex+1)) ||
			((yrefindex != startindex) && (index > startindex))) {
			/* 1+eps = c = Yref/Y = Yref*conj(Y)/abs(Y)^2 */
			mag_corr_papd_index =
				(int32)corr_papd_index_I*
				(int32)corr_papd_index_I +
				(int32)corr_papd_index_Q*
				(int32)corr_papd_index_Q;
			eps_re_prev = eps_re_curr;
			eps_im_prev = eps_im_curr;
			if (mag_corr_papd_index >> 12) {
				eps_re_curr = (((int32)corr_papd_index_I*
						(int32)corr_papd_index_I_yref +
						(int32)corr_papd_index_Q*
						(int32)corr_papd_index_Q_yref) /
						(mag_corr_papd_index >> 12)) - 4096;
				eps_im_curr = ((int32)corr_papd_index_Q*
					(int32)corr_papd_index_I_yref -
					(int32)corr_papd_index_I*
					(int32)corr_papd_index_Q_yref) /
					(mag_corr_papd_index >> 12);
			} else {
				eps_re_curr = 0;
				eps_im_curr = 0;
			}
			abs_c = (uint16)math_sqrt_int_32(
				((int32)4096 + (int32)eps_re_curr)*
				((int32)4096 + (int32)eps_re_curr) +
				((int32)eps_im_curr*(int32)eps_im_curr));
			vin_prev = vin_curr;
			if (abs_c) {
				vin_curr = (uint16)(((uint32)vin_p[index-1]<<12)/abs_c);
			} else {
				vin_curr = vin_p[index-1];
			}
			eps_re_curr = (eps_re_curr > 4095) ? 4095 :
				((eps_re_curr < -4096) ? -4096: eps_re_curr);
			eps_im_curr = (eps_im_curr > 4095) ? 4095 :
				((eps_im_curr < -4096) ? -4096: eps_im_curr);
		}
		if (gain_ctrl == 0) {
			/* computing the first index to interpolate for based on
			 * the first vin computed
			 */
			if (((yrefindex != startindex) && (index == startindex+1)) ||
				((yrefindex == startindex) && (index == startindex+2))) {
				while ((next_index_interp_init <= 63) &&
					(vin_curr > vin_p[next_index_interp_init])) {
					dst[next_index_interp_init++] = 0;
				}
				next_index_interp = next_index_interp_init;
				next_index_write = next_index_interp_init;
			}
			while ((next_index_interp <= 63) &&
				(index > startindex+1) &&
				(vin_curr >  vin_p[next_index_interp]) &&
				(vin_prev <= vin_p[next_index_interp]) &&
				(vin_curr != vin_prev)) {
				eps_re_interp =
					(int16)(((int32)eps_re_curr*(int32)
						(vin_p[next_index_interp] - vin_prev) +
						(int32)eps_re_prev*(int32)
						(vin_curr - vin_p[next_index_interp]))/
						(int16)(vin_curr - vin_prev));
				eps_im_interp =
					(int16)(((int32)eps_im_curr*(int32)
						(vin_p[next_index_interp] - vin_prev) +
						(int32)eps_im_prev*(int32)
						(vin_curr - vin_p[next_index_interp]))/
						(int16)(vin_curr - vin_prev));
				dst[next_index_interp] =
					(uint32)((((int32)eps_im_interp & 0x1fff) << 13) |
					((int32)eps_re_interp & 0x1fff));
				next_index_interp++;
			}
			while ((next_index_write < index) &&
				(next_index_write < next_index_interp) &&
				(next_index_write <= 63)) {
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id, 1, next_index_write,
					32, &dst[next_index_write], core);
				next_index_write++;
			}
		}
		if (index <= stopindex) {
			SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
			if ((READ_PHYREG(pi, PapdCalStart) & 1)) {
				PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR :"
					" PAPD cal failed \n", __FUNCTION__));
				PHY_FATAL_ERROR(pi, PHY_RC_PAPD_CAL_FAILED);
			}
			corr_papd_index_I = READ_PHYREG(pi, papdCalFirstCorr_I0);
			corr_papd_index_Q = READ_PHYREG(pi, papdCalFirstCorr_Q0);
			corr_papd_index_I_yref = READ_PHYREG(pi, PapdCalYref_I0);
			corr_papd_index_Q_yref = READ_PHYREG(pi, PapdCalYref_Q0);
		}
		corr_sat = ((index == startindex) &&
			((ABS(corr_papd_index_I_yref) > 32000) ||
			(ABS(corr_papd_index_Q_yref) > 32000)));
	}
	if (gain_ctrl == 0) {
		wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
	}
	/* To cover gctrl only for (the case where startindex <
	 * stopindex is already covered)
	 */
	if (gain_ctrl == 1) {
		dst[stopindex] =
			(uint32)((((int32)eps_im_curr & 0x1fff) << 13) |
				((int32)eps_re_curr & 0x1fff));
		wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_id, 1,
			stopindex, 32, &dst[stopindex], core);
	} else {
		if ((next_index_interp_init > startindex) &&
			(next_index_interp_init <= 63)) {
			wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_id,
				next_index_interp_init - startindex, startindex, 32,
				&dst[startindex], core);
		}
		/* Fill remaining indices with last result */
		for (index = next_index_interp; index <= stopindex; index++) {
			dst_tmp =
				(uint32)((((int32)eps_im_interp & 0x1fff) << 13) |
				((int32)eps_re_interp & 0x1fff));
			if ((uint32)(((int32)4096 + (int32)eps_re_interp)*
				((int32)4096 + (int32)eps_re_interp) +
				((int32)eps_im_interp*(int32)eps_im_interp)) > dst_limit) {
				wlc_phy_table_read_acphy_dac_war(pi,
					epsilon_table_id, 1, next_index_write - 2,
					32, &dst[index], core);
			} else {
				dst[index] = dst_tmp;
			}
		}
		if (stopindex >= next_index_write) {
			if ((uint32)(((int32)4096 + (int32)eps_re_interp)*
				((int32)4096 + (int32)eps_re_interp) +
				((int32)eps_im_interp*(int32)eps_im_interp)) > dst_limit) {
				wlc_phy_table_read_acphy_dac_war(pi,
					epsilon_table_id, 1,
					next_index_write - 2, 32,
					&dst[next_index_write-1], core);
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id,
					stopindex-next_index_write+2, next_index_write-1,
					32, &dst[next_index_write-1], core);
			} else {
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id,
					stopindex-next_index_write+1,
					next_index_write, 32,
					&dst[next_index_write], core);
			}
		}
		for (index = 0; index < startindex; index++) {
			dst[index] = 0x0;
		}
		wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_id,
				startindex, 0, 32, dst, core);
	}
	if (gain_ctrl == 0)
		phy_ac_papd_smooth(pi, 0, 5, 0, 32);
}

/* scal_tbl_map_shift = -1.0 -0.75 -0.50 -0.25 0.00
 * +0.25  +0.50  +0.75  +1.00 +1.25
 * tbl_map_scale = 0.891 0.9173 0.9441 0.9716 1.0000
 * 1.0292 1.0593 1.0902 1.1220 1.1548
 * tbl_map_scale = 3649 3757 3867 3980 4096
 * 4216 4339 4465 4596 4730
 * tbl_map_scale can be tuned to reduce IFS dependence
 *
 * int16  scal_tbl_map_shift = 1;
 * int16 tbl_map_scale_2G = 4596;
 * int16 tbl_map_scale_5G_20[4] = {4596, 4596, 4596, 4596};
 * int16 tbl_map_scale_5G_40[4] = {4596, 4596, 4596, 4596};
 * int16 tbl_map_scale_5G_80[4] = {4596, 4596, 4596, 4596};
 * int16 tbl_map_scale_2G_formin1 = 3649;
 * int16 tbl_map_scale_2G_forp75 = 4465;
 * int16 tbl_map_scale_2G_forminp75 = 3757;
 * int16 tbl_map_scale_5G_20_forp75[4] = {4465, 4465, 4465, 4465};
 * int16 tbl_map_scale_5G_40_forp75[4] = {4465, 4465, 4465, 4465};
 * int16 tbl_map_scale_5G_80_forp75[4] = {4465, 4465, 4465, 4465};
 * just made scal_tbl_map_shift *100, since float is not accepted
 * scal_tbl_map_shift 100 = 1 , -100 = -1, 75 = 0.75
 */

/* TCL PAPDCAL Mode 3 - Analytic Cal */
void
phy_ac_papd_cal_mode3(phy_info_t *pi, acphy_papdCalParams_t *calParams)
{
	uint8 core = calParams->core, epsilon_table_id = calParams->epsilon_table_id;
	uint16 startindex = calParams->startindex;
	uint16 yrefindex = calParams->yrefindex;
	uint16 corr_end = 79;
	uint16 idx;
	int16  corr_I = 0, corr_Q = 0;
	int16  Yref_I = 0, Yref_Q = 0;
	int32  eps_den = 0;
	uint16  abs_c = 0;
	int16  eps_I_curr = 0, eps_Q_curr = 0;
	int16  tbl_map_scale = 1;

	/* fixed-point fractional bits shift */
	uint16 fixpt_shift_bits = 12;
	/* eps interpolation */
	int16  eps_I_prev = 0, eps_Q_prev = 0;
	int16  eps_I_interp_curr = 0, eps_Q_interp_curr = 0;
	uint16 scal_tbl_interp = 0, scal_tbl_map_prev = 0, scal_tbl_map_curr = 0;
	uint16 next_idx_interp_init = 0, next_idx_interp = 0;
	uint32 dst;

	uint16 scal_tbl[APAPD_ARRAYSIZE], scal_tbl_map[APAPD_ARRAYSIZE];
	int16 eps_I[APAPD_ARRAYSIZE], eps_Q[APAPD_ARRAYSIZE], eps_I_interp[APAPD_ARRAYSIZE],
	      eps_Q_interp[APAPD_ARRAYSIZE];

	/* Changing scal tbl map shift to reduce EVM dependance on IFS */
	if ((ROUTER_4349(pi)) && (CHSPEC_IS5G(pi->radio_chanspec)))
		tbl_map_scale = 4596;
	else
		tbl_map_scale = 4096;

	for (idx = startindex; idx <= corr_end; idx++) {
		scal_tbl[idx-startindex] = (uint16)(acphy_papd_scaltbl_128[idx]
				& 0xffff);
		eps_I[idx-startindex] = 0;
		eps_Q[idx-startindex] = 0;
	}

	phy_ac_apapd_init_seq(pi, core, yrefindex);
	(void)wlc_phy_tx_tone_acphy(pi, ACPHY_IQCAL_TONEFREQ_1MHz, 186, TX_TONE_IQCAL_MODE_OFF,
		FALSE);

	for (idx = 0; idx <= corr_end; idx++) {
		MOD_PHYREG(pi, PapdCalAddress, papdStartAddr, idx);
		MOD_PHYREG(pi, PapdCalAddress, papdEndAddr, idx);
		WRITE_PHYREG(pi, papdCalCorrDebugAddr, idx);

		MOD_PHYREG(pi, PapdCalCoreSel, papdCoreSel, core);
		OSL_DELAY(10);
		MOD_PHYREG(pi, PapdCalStart, papdStart, 1);

		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		if ((READ_PHYREG(pi, PapdCalStart) & 1)) {
			PHY_FATAL_ERROR_MESG((" %s: SPINWAIT ERROR :"
				" PAPD cal failed \n", __FUNCTION__));
			PHY_FATAL_ERROR(pi, PHY_RC_PAPD_CAL_FAILED);
		}
		SPINWAIT(READ_PHYREG(pi, PapdCalStart), ACPHY_SPINWAIT_PAPDCAL);
		if (idx >= startindex) {
			corr_I = (int16) (READ_PHYREGCE(pi, papdCalFirstCorr_I, core)
					& 0xffff);
			corr_Q = (int16) (READ_PHYREGCE(pi, papdCalFirstCorr_Q, core)
					& 0xffff);
			Yref_I = (int16) (READ_PHYREGCE(pi, PapdCalYref_I, core) & 0xffff);
			Yref_Q = (int16) (READ_PHYREGCE(pi, PapdCalYref_Q, core) & 0xffff);

			/* 1+eps = c = Yref/Y = Yref*conj(Y)/abs(Y)^2 */
			eps_den = (int32)corr_I*(int32)corr_I +
				(int32)corr_Q*(int32)corr_Q;
			/* prevent divide-by-zero: if (eps_den >> 12) */
			if (eps_den >> 12) {
				eps_I_curr = (((int32)corr_I*(int32)Yref_I +
					(int32)corr_Q*(int32)Yref_Q)/(eps_den >> 12))
					- 4096;
				eps_Q_curr =  ((int32)corr_Q*(int32)Yref_I -
					(int32)corr_I*(int32)Yref_Q)/(eps_den >> 12);
			} else {
				eps_I_curr = 0;
				eps_Q_curr = 0;
			}

			eps_I[idx-startindex] = eps_I_curr;
			eps_Q[idx-startindex] = eps_Q_curr;

			abs_c = (uint16)math_sqrt_int_32(
					(uint32)(((int32)4096 + (int32)eps_I_curr) *
						((int32)4096 + (int32)eps_I_curr) +
						((int32)eps_Q_curr*(int32)eps_Q_curr)));
			/* prevent divide-by-zero */
			abs_c = (abs_c == 0)? 1:abs_c;
			scal_tbl_map[idx-startindex] = (uint16)
				((((uint32)scal_tbl[idx-startindex])<<12)/abs_c);

			/* shift PA curve to better fit PA in normal operation */
			scal_tbl_map[idx-startindex] = (uint16)
				(((uint32)scal_tbl_map[idx-startindex] * tbl_map_scale)
				 >> fixpt_shift_bits);
		}
	}
	wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);

	/* next_idx_interp */
	while ((next_idx_interp_init <= corr_end - startindex) &&
			(scal_tbl[next_idx_interp_init] < scal_tbl_map[0])) {
		eps_I_interp[next_idx_interp_init] = 0;
		eps_Q_interp[next_idx_interp_init] = 0;
		next_idx_interp_init++;
	}
	next_idx_interp = next_idx_interp_init;

	for (idx = 0; idx < corr_end-startindex; idx++) {
		scal_tbl_map_prev = scal_tbl_map[idx];
		scal_tbl_map_curr = scal_tbl_map[idx+1];
		eps_I_prev = eps_I[idx];
		eps_Q_prev = eps_Q[idx];
		eps_I_curr = eps_I[idx+1];
		eps_Q_curr = eps_Q[idx+1];
		while ((next_idx_interp <= corr_end-startindex) &&
				(scal_tbl[next_idx_interp] >= scal_tbl_map_prev) &&
				(scal_tbl[next_idx_interp] < scal_tbl_map_curr)) {
			scal_tbl_interp = scal_tbl[next_idx_interp];
			eps_I_interp_curr = (int16)
				(((int32)eps_I_prev *
				  (int32)(scal_tbl_map_curr - scal_tbl_interp) +
				  (int32)eps_I_curr *
				  (int32)(scal_tbl_interp - scal_tbl_map_prev))/
				 (int16)(scal_tbl_map_curr - scal_tbl_map_prev));
			eps_Q_interp_curr = (int16)
				(((int32)eps_Q_prev *
				  (int32)(scal_tbl_map_curr - scal_tbl_interp) +
				  (int32)eps_Q_curr *
				  (int32)(scal_tbl_interp - scal_tbl_map_prev))/
				 (int16)(scal_tbl_map_curr - scal_tbl_map_prev));
			eps_I_interp[next_idx_interp - next_idx_interp_init] =
				(eps_I_interp_curr >= 4095)?
				4095 : (eps_I_interp_curr <= -4096)?
				-4096 : eps_I_interp_curr;
			eps_Q_interp[next_idx_interp - next_idx_interp_init] =
				(eps_Q_interp_curr >= 4095)?
				4095 : (eps_Q_interp_curr <= -4096)?
				-4096 : eps_Q_interp_curr;
			next_idx_interp++;
		}
	}

	/* write table */
	if (next_idx_interp_init != next_idx_interp) {
		for (idx = 0; idx <= 63; idx++) {
			if (idx < startindex + next_idx_interp_init) {
				dst = 0x0;
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id, 1, idx, 32, &dst, core);
			} else if (idx < (next_idx_interp + startindex)) {
				dst = (uint32)((((uint32)eps_Q_interp
					[idx-startindex-next_idx_interp_init]
					& 0x1fff) << 13) | ((uint32)eps_I_interp
					[idx-startindex-next_idx_interp_init]
					& 0x1fff));
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id, 1, idx, 32, &dst, core);
			} else {
					dst = (uint32)((((uint32)eps_Q_interp
						[next_idx_interp-next_idx_interp_init-1]
						& 0x1fff) << 13) | ((uint32)eps_I_interp
						[next_idx_interp-next_idx_interp_init-1]
						& 0x1fff));
				wlc_phy_table_write_acphy_dac_war(pi,
					epsilon_table_id, 1, idx, 32, &dst, core);
			}
		}
	} else {
		dst = (uint32)
			((((int32)eps_Q[corr_end-startindex] & 0x1fff) << 13) |
			((int32)eps_I[corr_end-startindex] & 0x1fff));
		wlc_phy_table_write_acphy_dac_war(pi, epsilon_table_id,
				1, 63, 32, &dst, core);

	}
}

void
phy_ac_papd_cal_eps_calc_tiny(phy_info_t *pi, uint8 core, int8 *bbmult)
{
	bool is2g = (CHSPEC_IS2G(pi->radio_chanspec)),
	     is80M = (CHSPEC_IS80(pi->radio_chanspec));
	uint8 scalar_table_ids[] = { ACPHY_TBL_ID_SCALAR0, ACPHY_TBL_ID_SCALAR1,
		ACPHY_TBL_ID_SCALAR2};
	uint32 scalartblval, papdmult, epsilonscalartemp;
	int8 k;
	int16 cal_tone_mag = 186;
	int16 temp, temp1, qQ1, lut_shift, epsilonoffsettemp, dac_rf_offset;
	int32 dig_gain_dB;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	phy_ac_papdcal_info_t *papdcali = (phy_ac_papdcal_info_t *)pi->u.pi_acphy->papdcali;

	ASSERT(papdcali != NULL);

	wlc_phy_table_read_acphy(pi, scalar_table_ids[core], 1, 0, 32, &scalartblval);
	papdmult = scalartblval & 0x1fff;

	temp = ((*bbmult)*cal_tone_mag*papdmult*1000)/(64*1024*100);
	qm_log10((int32)(temp), 0, &temp1, &qQ1);
	dig_gain_dB = ((20*temp1) - (60 << qQ1)) >> qQ1;

	lut_shift = -2;
	if (ACREV_GE(pi->pubpi->phy_rev, 4)) {
		if (APAPD_ENAB(papdcali)) {
			if (is2g) {
				lut_shift = -2;
			} else {
				lut_shift = (ACMINORREV_2(pi))? -1 :
					(is80M) ? -3 : (channel < 100)? -2 : -3;
			}
			if (ROUTER_4349(pi)) {
				lut_shift = (is2g) ? -2 :
					(is80M) ? 1 : -2;
			}
		} else {
			if (CHSPEC_IS5G(pi->radio_chanspec) && (!PHY_EPAPD(pi)) &&
				ACMAJORREV_4(pi->pubpi->phy_rev)) {
				lut_shift = (IS20MHZ(pi)) ? 0 : 2;
			}
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (IS20MHZ(pi)) {
					lut_shift += papdcali->pacalshift5g[0];
				} else if (IS40MHZ(pi)) {
					lut_shift += papdcali->pacalshift5g[1];
				} else {
					lut_shift += papdcali->pacalshift5g[2];
				}
			} else {
				if (IS20MHZ(pi)) {
					lut_shift += papdcali->pacalshift2g[0];
				} else if (IS40MHZ(pi)) {
					lut_shift += papdcali->pacalshift2g[1];
				} else {
					lut_shift += papdcali->pacalshift2g[2];
				}
			}
		}
	}
	k = -80;
	dac_rf_offset = READ_PHYREGFLDCEE(pi, PapdEnable, core, gain_dac_rf_reg);
	if (dac_rf_offset >= 256) {
		dac_rf_offset = dac_rf_offset - 512;
	}
	epsilonscalartemp = READ_PHYREGFLDCEE(pi, EpsilonTableAdjust, core, epsilonScalar);
	epsilonoffsettemp = k - 2*dig_gain_dB + lut_shift -
		dac_rf_offset*epsilonscalartemp/16;
	if (epsilonoffsettemp < 0) {
		epsilonoffsettemp = 512 + epsilonoffsettemp;
	}
	MOD_PHYREGCEE(pi, EpsilonTableAdjust, core, epsilonOffset,
		epsilonoffsettemp);
}

void
phy_ac_papd_cal_set_tx_gain(phy_info_t *pi, uint8 core, int8 *bbmult, uint8 *calmode)
{
	bool is2g = (CHSPEC_IS2G(pi->radio_chanspec)),
		is80M = (CHSPEC_IS80(pi->radio_chanspec));
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	int8 tx_idx = 40;
	int8 coremask = 1 << core;
	uint16 m[4] = {0, 0, 0, 0};
	phy_info_acphy_t *aci = (phy_info_acphy_t *)pi->u.pi_acphy;
	BCM_REFERENCE(aci);

	ASSERT(aci->papdcali != NULL);

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		phy_ac_papdcal_params_t *params	= pi->u.pi_acphy->papdcali->papd_params;
		*calmode = 0;
		*bbmult = (CHSPEC_IS2G(pi->radio_chanspec)) ?
			(int8)params->bbmult_2g: (int8) params->bbmult_5g;

		/* Currently not reading tx index from NVRAM for PHY Major rev 36 */
		tx_idx = (CHSPEC_IS2G(pi->radio_chanspec)) ? params->papd_calidx_2g :
			params->papd_calidx_5g;

		/* 1st priority: force cal index through iovar */
		if (aci->papdcali->pacalidx_iovar != -1)
			tx_idx = aci->papdcali->pacalidx_iovar;

		/* Set the TX index */
		aci->papdcali->papd_lut0_cal_idx = tx_idx;
		wlc_phy_txpwr_by_index_acphy(pi, 1, tx_idx);

	} else if (TINY_RADIO(pi))  {
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			*calmode = 0;
			if (ROUTER_4349(pi)) {
				*bbmult = (is2g)? 90 : (is80M) ?
					((channel < 100) ? 55 : 65) :
					((core == 0) ? 60 : 50);
				tx_idx = (is2g)? 24 : 40;
				if ((channel == 38) || (channel == 151) || (channel == 42) ||
					(channel == 106) || (channel == 159)) {
					tx_idx = 65;
				}
			} else {

				*bbmult = phy_ac_papd_gain_ctrl_tiny(pi,
						core, 5);
				tx_idx = (is2g) ? 24 : 30;
			}
			if ((phy_get_phymode(pi) == PHYMODE_MIMO))
				phy_ac_papd_turningoff_inactivecore(pi, core);

			/* Logic added to 'turn' off tone on inactive core */
			/* Need to turn off PA on the inactive core as the next step */
			wlc_phy_txpwr_by_index_acphy(pi, core + 1, tx_idx);
			coremask = (phy_get_phymode(pi) == PHYMODE_RSDB) ? 1 : 3;

		} else {
			*calmode = 1; /* Run single index of PAPD table only */

			*bbmult = (is2g) ? 64 : 60;
			if (PHY_EPAPD(pi))
				*bbmult = (is2g) ? 100 : 75;
			if ((is2g) && (aci->papdcali->papdbbmult2g != -1)) {
				*bbmult = aci->papdcali->papdbbmult2g;
			} else if (CHSPEC_IS5G(pi->radio_chanspec) &&
					(aci->papdcali->papdbbmult5g != -1)) {
				*bbmult = aci->papdcali->papdbbmult5g;
			}
			if (aci->papdcali->pacalmode != -1) {
				*calmode = aci->papdcali->pacalmode;
			}
		}
	}  else {
		*calmode = 0;  /* Run the PAPD automatic machine on all indices */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			*bbmult = 0x3f;
			if (RADIOMAJORREV(pi) == 2 && PHY_EPAPD(pi)) {
				*bbmult = (core == 0) ? 100 : 110;
			}
		} else
			*bbmult = 0x30;
	}

	m[core] = *bbmult;

	/* Setting appropriate bbmult for all tx cores */
	wlc_phy_ipa_set_bbmult_acphy(pi, &m[0], &m[1], &m[2], &m[3], coremask);
}

/* Function to save and restore papd cal PHY registers
 * sr = 1 => SAVE
 * sr = 0 => RESTORE
 */
void
acphy_papd_cal_phyreg_sr(phy_info_t *pi, uint8 core, acphy_rxcal_phyregs_t *porig, bool sr)
{
	struct addr_val {
		uint16 source;
		uint16 * destination;
	} reg_data[] = {
		{ACPHY_RfctrlOverrideTxPus0(pi->pubpi->phy_rev), &porig->RfctrlOverrideTxPus[core]},
		{ACPHY_RfctrlOverrideRxPus0(pi->pubpi->phy_rev), &porig->RfctrlOverrideRxPus[core]},
		{ACPHY_RfctrlOverrideGains0(pi->pubpi->phy_rev), &porig->RfctrlOverrideGains[core]},
		{ACPHY_RfctrlOverrideLpfCT0(pi->pubpi->phy_rev), &porig->RfctrlOverrideLpfCT[core]},
		{ACPHY_RfctrlOverrideLpfSwtch0(pi->pubpi->phy_rev),
		&porig->RfctrlOverrideLpfSwtch[core]},
		{ACPHY_RfctrlOverrideAfeCfg0(pi->pubpi->phy_rev),
		&porig->RfctrlOverrideAfeCfg[core]},
		{ACPHY_RfctrlOverrideLowPwrCfg0(pi->pubpi->phy_rev),
		&porig->RfctrlOverrideLowPwrCfg[core]},
		{ACPHY_RfctrlOverrideAuxTssi0(pi->pubpi->phy_rev),
		&porig->RfctrlOverrideAuxTssi[core]},
		{ACPHY_RfctrlCoreTxPus0(pi->pubpi->phy_rev), &porig->RfctrlCoreTxPus[core]},
		{ACPHY_RfctrlCoreRxPus0(pi->pubpi->phy_rev), &porig->RfctrlCoreRxPus[core]},
		{ACPHY_RfctrlCoreTXGAIN10(pi->pubpi->phy_rev), &porig->RfctrlCoreTXGAIN1[core]},
		{ACPHY_RfctrlCoreTXGAIN20(pi->pubpi->phy_rev), &porig->RfctrlCoreTXGAIN2[core]},
		{ACPHY_RfctrlCoreRXGAIN10(pi->pubpi->phy_rev), &porig->RfctrlCoreRXGAIN1[core]},
		{ACPHY_RfctrlCoreRXGAIN20(pi->pubpi->phy_rev), &porig->RfctrlCoreRXGAIN2[core]},
		{ACPHY_RfctrlCoreLpfGain0(pi->pubpi->phy_rev), &porig->RfctrlCoreLpfGain[core]},
		{ACPHY_RfctrlCoreLpfCT0(pi->pubpi->phy_rev), &porig->RfctrlCoreLpfCT[core]},
		{ACPHY_RfctrlCoreLpfGmult0(pi->pubpi->phy_rev), &porig->RfctrlCoreLpfGmult[core]},
		{ACPHY_RfctrlCoreRCDACBuf0(pi->pubpi->phy_rev), &porig->RfctrlCoreRCDACBuf[core]},
		{ACPHY_RfctrlCoreLpfSwtch0(pi->pubpi->phy_rev), &porig->RfctrlCoreLpfSwtch[core]},
		{ACPHY_RfctrlCoreAfeCfg10(pi->pubpi->phy_rev), &porig->RfctrlCoreAfeCfg1[core]},
		{ACPHY_RfctrlCoreAfeCfg20(pi->pubpi->phy_rev), &porig->RfctrlCoreAfeCfg2[core]},
		{ACPHY_RfctrlCoreLowPwr0(pi->pubpi->phy_rev), &porig->RfctrlCoreLowPwr[core]},
		{ACPHY_RfctrlCoreAuxTssi10(pi->pubpi->phy_rev), &porig->RfctrlCoreAuxTssi1[core]},
		{ACPHY_RfctrlCoreAuxTssi20(pi->pubpi->phy_rev), &porig->RfctrlCoreAuxTssi2[core]},
		{ACPHY_Dac_gain0(pi->pubpi->phy_rev), &porig->Dac_gain[core]},
		{ACPHY_RfctrlIntc0(pi->pubpi->phy_rev), &porig->RfctrlIntc[core]},
		{ACPHY_PapdEnable0(pi->pubpi->phy_rev), &porig->PapdEnable[core]},
		{ACPHY_forceFront0(pi->pubpi->phy_rev), &porig->forceFront[core]},
		{ 0, 0}
	};
	struct addr_val * addrp;

	for (addrp = reg_data; addrp->source; addrp++) {
		if (sr) {
			*addrp->destination = phy_utils_read_phyreg(pi, addrp->source +
				(core * PHY_REG_BANK_CORE1_OFFSET));
		} else {
			phy_utils_write_phyreg(pi, addrp->source + (core *
				PHY_REG_BANK_CORE1_OFFSET), *addrp->destination);
		}
	}
}

const uint32 *
BCMRAMFN(get_wbpapd_wfm_43012)(phy_info_t *pi)
{
	phy_ac_papdcal_params_t *params	= pi->u.pi_acphy->papdcali->papd_params;

	if (ACMAJORREV_36(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
#ifdef USE_FCBS_ROM
		params->wbcal_waveform_sz = SZ_WBPAPD_WAVEFORM;
		return NULL;
#else
		if (ACMINORREV_1(pi)) {
			/* in 43012 B0 use ROM address even when USE_FCBS_ROM is */
			/* disabled as enabling USE_FCBS_ROM abandons some other code */
			params->wbcal_waveform_sz = SZ_WBPAPD_WAVEFORM;
			return NULL;
		} else {
			params->wbcal_waveform_sz = ARRAYSIZE(acphy_wbpapd_waveform);
			return (const uint32 *)&acphy_wbpapd_waveform[0];
		}
#endif /* USE_FCBS_ROM */
	} else {
		/* In case of 43012a0 there was no PAPD data in FCBS ROM */
		params->wbcal_waveform_sz = ARRAYSIZE(acphy_wbpapd_waveform);
		return (const uint32 *)&acphy_wbpapd_waveform[0];
	}
}

void
phy_ac_papdcal_cal_init(phy_info_t *pi)
{
	/* If single phase cal send out CTS to self to ensure assoc/join */
	uint8 phase_id = pi->cal_info->cal_phase_id;
	uint16 cal_exec_time = 29000;
	if (phase_id == PHY_CAL_PHASE_IDLE) {
		if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
			if (PHY_PAPDEN(pi)) {
				if (pi->u.pi_acphy->papdcali->papdmode == PAPD_ANALYTIC_WO_YREF) {
					pi->u.pi_acphy->papdcali->papd_cal_time = 5000;
				} else if (pi->u.pi_acphy->papdcali->papdmode == PAPD_ANALYTIC) {
					pi->u.pi_acphy->papdcali->papd_cal_time = 8000;
				} else {
					pi->u.pi_acphy->papdcali->papd_cal_time = 12000;
				}
				cal_exec_time += pi->u.pi_acphy->papdcali->papd_cal_time;
			}
		}
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			if (pi->sh->up) {
				wlc_phy_cts2self(pi, cal_exec_time);
			}
		} else {
			wlc_phy_cts2self(pi, cal_exec_time);
		}
	}
}

#ifdef PHYCAL_CACHING
void
phy_ac_papdcal_save_cache(phy_ac_papdcal_info_t *papdcali, ch_calcache_t *ctx)
{
	phy_info_t *pi = papdcali->pi;
	acphy_ram_calcache_t *ctx_ac = ctx->u_ram_cache.acphy_ram_calcache;
	uint32 *epsilon_cache;
	uint16 *epstbl_offset_cache;
	uint32 epsilon_table_ids[] =
		{ACPHY_TBL_ID_EPSILON0, ACPHY_TBL_ID_EPSILON1, ACPHY_TBL_ID_EPSILON2};
	uint32 rfpwrlut_table_ids[] =
		{ACPHY_TBL_ID_RFPWRLUTS0, ACPHY_TBL_ID_RFPWRLUTS1, ACPHY_TBL_ID_RFPWRLUTS2};
	uint8 core;

	epsilon_cache = ctx->u.acphy_cache.papd_eps;
	epstbl_offset_cache = ctx->u.acphy_cache.eps_offset_cache;
	/* save the calibration to cache */
	FOREACH_CORE(pi, core) {
		/* save PAPD epsilon offsets */
		ctx_ac->epsilon_offset[core] = READ_PHYREGFLDCEE(pi,
			EpsilonTableAdjust, core, epsilonOffset);
		ctx_ac->papd_comp_en[core] = READ_PHYREGFLDCEE(pi, PapdEnable,
			core, papd_compEnb);
		wlc_phy_table_read_acphy_dac_war(pi, epsilon_table_ids[core],
			ACPHY_PAPD_EPS_TBL_SIZE, 0, 32, epsilon_cache, core);
		epsilon_cache += ACPHY_PAPD_EPS_TBL_SIZE;
		wlc_phy_table_read_acphy(pi, rfpwrlut_table_ids[core],
			ACPHY_PAPD_RFPWRLUT_TBL_SIZE, 0, 16, epstbl_offset_cache);
		epstbl_offset_cache += ACPHY_PAPD_RFPWRLUT_TBL_SIZE;
	}
}
#endif /* PHYCAL_CACHING */

void
phy_ac_papdcal(phy_info_t *pi)
{
#ifdef PHYCAL_CACHING
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif /* PHYCAL_CACHING */
	wlc_phy_cals_mac_susp_en_other_cr(pi, TRUE);
	if (ACMAJORREV_1(pi->pubpi->phy_rev) &&
		ACMINORREV_2(pi) && PHY_PAPDEN(pi)) {
		if ((pi->radar_percal_mask & 0x20) != 0) {
			pi->u.pi_acphy->radar_cal_active = TRUE;
		}
	}

#ifdef WFD_PHY_LL
	if (pi->papdcali->data->wfd_ll_enable) {
		/* skip the PAPD calibration */
		wlc_phy_cts2self(pi, 0);
		pi->cal_info->cal_phase_id++;
		return;
	}
#endif // endif
	if (PHY_PAPDEN(pi)) {
		wlc_phy_cts2self(pi, pi->u.pi_acphy->papdcali->papd_cal_time);
		wlc_phy_do_papd_cal_acphy(pi);
#ifdef PHYCAL_CACHING
		if (ctx) {
			phy_ac_papdcal_save_cache(pi->u.pi_acphy->papdcali, ctx);
		}
#endif /* PHYCAL_CACHING */
	} else {
		/* To make phyreg_enter & mac_suspend in sync for PAPD_EN=0 */
		wlc_phy_cts2self(pi, 0);
	}
	wlc_phy_cals_mac_susp_en_other_cr(pi, FALSE);

	/* move on */
	pi->cal_info->cal_phase_id++;
}

#if defined(WFD_PHY_LL)
static int
phy_ac_papdcal_set_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, uint8 int_val)
{
	phy_ac_papdcal_info_t *papdcali = (phy_ac_papdcal_info_t *)ctx;
	phy_papdcal_data_t *data = papdcali->cmn_info->data;

	/* Force the channel to be active */
	data->wfd_ll_chan_active_force = (int_val == 2) ? TRUE : FALSE;
	data->wfd_ll_enable_pending = int_val;
	if (!PHY_PERICAL_MPHASE_PENDING(papdcali->pi)) {
		/* Apply it since there is no CAL in progress */
		data->wfd_ll_enable = int_val;
		if (!int_val) {
			/* Force a watchdog CAL when disabling WFD optimization
			 * As PADP CAL has not been executed since a long time
			 * a PADP CAL is executed at the next watchdog timeout
			 */
			 papdcali->pi->cal_info->last_cal_time = 0;
		}
	}
	return BCME_OK;
}

static int
phy_ac_papdcal_get_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_ac_papdcal_info_t *papdcali = (phy_ac_papdcal_info_t *)ctx;
	*ret_int_ptr = papdcali->cmn_info->data->wfd_ll_enable;
	return BCME_OK;
}
#endif /* WFD_PHY_LL */
