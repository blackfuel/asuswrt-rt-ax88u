/*
 * ACPhy Noise module implementation
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
 * $Id: phy_ac_noise.c 767447 2018-09-11 13:22:06Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include <phy_btcx.h>
#include <phy_btcx_api.h>
#include "phy_type_noise.h"
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_noise.h>
#include <phy_ac_btcx.h>
#include <phy_ac_chanmgr.h>
#include <phy_ac_rxgcrs.h>
#include <phy_rxgcrs_api.h>
#include <phy_ocl_api.h>
#include <phy_stf.h>
#include <phy_ac_ocl.h>
#include <phy_noise.h>
#include <phy_noise_api.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_radioreg_20691.h>
#include <wlc_phy_radio.h>
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_utils_status.h>
#include <phy_ac_info.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>

#define ACPHY_SROM_NOISELVL_OFFSET (-70)

typedef struct acphy_aci_params {
	/* array is indexed by chan/bw */
	uint8 chan;
	uint16 bw;
	uint64 last_updated;

	acphy_desense_values_t desense;
	int8 weakest_rssi;

	uint8 txop, jammer_cnt;

	desense_history_t bphy_hist;
	desense_history_t ofdm_hist;
	uint8 glitch_buff_idx, glitch_upd_wait;

	uint8 hwaci_setup_state, hwaci_desense_state;
	uint8 hwaci_noaci_timer;
	uint8 hwaci_sleep;
	int  hwaci_desense_state_ovr;
	bool engine_called;
} acphy_aci_params_t;

typedef struct {
	uint16 sample_time;
	uint16 energy_thresh;
	uint16 detect_thresh;
	uint16 wait_period;
	uint8 sliding_window;
	uint8 samp_cluster;
	uint8 nb_lo_th;
	uint8 w3_lo_th;
	uint8 w3_md_th;
	uint8 w3_hi_th;
	uint8 w2;
	uint16 energy_thresh_w2;
	uint8 aci_present_th;
	uint8 aci_present_select;
} acphy_hwaci_setup_t;

/* module private states */
struct phy_ac_noise_info {
	phy_info_t *pi;
	phy_ac_info_t *ac_info;
	phy_noise_info_t *cmn_info;
	phy_ac_noise_data_t *data; /* shared data */
	/* nvnoiseshapingtbl monitor */
	acphy_nshapetbl_mon_t *nshapetbl_mon;
	acphy_hwaci_state_t *hwaci_states_2g; /* Array of size ACPHY_HWACI_MAX_STATES */
	acphy_hwaci_state_t *hwaci_states_5g; /* Array of size ACPHY_HWACI_MAX_STATES */
	acphy_hwaci_state_t *hwaci_states_5g_40_80; /* Array of size ACPHY_HWACI_MAX_STATES */
	/* aci params */
	acphy_aci_params_t *aci_list2g; /* Array of size ACPHY_ACI_CHAN_LIST_SZ */
	acphy_aci_params_t *aci_list5g; /* Array of size ACPHY_ACI_CHAN_LIST_SZ */
	acphy_hwaci_defgain_settings_t *def_gains; /* Array of size PHY_CORE_MAX */
	acphy_aci_params_t	*aci;
	acphy_hwaci_setup_t	*hwaci_args;
	int		hwaci_desense_state_ovr;
	uint8	hwaci_max_states_2g, hwaci_max_states_5g;
	int8	noise_lte;
	bool	LTEJ_WAR_en;
};

typedef struct {
	phy_ac_noise_info_t info;
	phy_ac_noise_data_t data;
} phy_ac_noise_mem_t;

#ifndef WLC_DISABLE_ACI
static uint32 wlc_phy_desense_aci_get_avg_max_glitches_acphy(phy_info_t *pi, uint32 glitches[]);
static uint8 wlc_phy_desense_aci_calc_acphy(phy_info_t *pi, desense_history_t *aci_desense,
	uint8 desense, uint32 glitch_cnt, uint16 glitch_th_lo, uint16 glitch_th_hi,
	bool enable_half_dB, uint8 *desense_half_dB);
#ifdef WL_ACI_FCBS
static void wlc_phy_init_FCBS_hwaci(phy_info_t *pi);
#endif /* WL_ACI_FCBS */
static void wlc_phy_hwaci_write_table_acphy(phy_info_t *pi, uint16 table_id,
	uint16 start_off, uint8 *table_entry, bool check_5G);
static void wlc_phy_desense_aci_upd_chan_stats_acphy(phy_type_noise_ctx_t *ctx,
	chanspec_t chanspec, int8 leastRSSI);
static acphy_aci_params_t* wlc_phy_desense_aci_getset_chanidx_acphy(phy_info_t *pi,
	chanspec_t chanspec, bool create);
static void phy_ac_noise_upd_aci(phy_type_noise_ctx_t * ctx);
static void phy_ac_noise_update_aci_ma(phy_type_noise_ctx_t *ctx);
#endif /* !WLC_DISABLE_ACI */

static void phy_ac_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch);

/* local functions */
static void phy_ac_noise_attach_params(phy_ac_noise_info_t *noisei);
static void phy_ac_noise_attach_modes(phy_info_t *pi);
static void phy_ac_noise_calc(phy_type_noise_ctx_t *ctx, int8 cmplx_pwr_dbm[],
		uint8 extra_gain_1dB);
static void phy_ac_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
		uint8 extra_gain_1dB, int16 *tot_gain);
#ifndef WL_ACI_FCBS
static void phy_ac_noise_aci_mitigate(phy_type_noise_ctx_t *ctx);
#endif /* WL_ACI_FCBS */
static void phy_ac_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init);
static bool phy_ac_noise_wd(phy_wd_ctx_t *ctx);
#if defined(BCMDBG) || defined(WLTEST)
static int phy_ac_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_ac_noise_dump NULL
#endif // endif
#ifdef RADIO_HEALTH_CHECK
static phy_crash_reason_t phy_ac_noise_healthcheck_desense(phy_type_noise_ctx_t *ctx);
#endif /* RADIO_HEALTH_CHECK */

static int8 phy_ac_noise_avg(phy_type_noise_ctx_t *ctx);
static void phy_ac_noise_sample_intr(phy_type_noise_ctx_t *ctx);
static void phy_ac_noise_cb(phy_type_noise_ctx_t *ctx);
static int8 phy_ac_noise_lte_avg(phy_type_noise_ctx_t *ctx);
static int phy_ac_noise_get_srom_level(phy_type_noise_ctx_t *ctx, int32 *ret_int_ptr);
static int8 phy_ac_noise_read_shmem(phy_type_noise_ctx_t *ctx,
	uint8 *lte_on, uint8 *crs_high);
static int phy_ac_noise_reset(phy_type_noise_ctx_t *ctx);

static void
BCMATTACHFN(phy_ac_srom_read_noiselvl)(phy_info_t *pi)
{
	/* read noise levels from SROM */
	uint8 core, ant;
	char phy_var_name[20];

	if (phy_get_phymode(pi) == PHYMODE_RSDB &&
		(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0))
	{
		/* update pi[0] to hold pwrdet params for all cores */
		/* This is required for mimo operation */
		pi->pubpi->phy_corenum <<= 1;
	}

	FOREACH_CORE(pi, core) {
		/* 2G: */
		ant = phy_get_rsdbbrd_corenum(pi, core);
		(void)snprintf(phy_var_name, sizeof(phy_var_name), rstr_noiselvl2gaD, ant);
		pi->noiselvl_2g[core] = ACPHY_SROM_NOISELVL_OFFSET -
		                             ((uint8)PHY_GETINTVAR(pi, phy_var_name));

		/* 5G low: */
		(void)snprintf(phy_var_name, sizeof(phy_var_name), rstr_noiselvl5gaD, ant);
		pi->noiselvl_5gl[core] = ACPHY_SROM_NOISELVL_OFFSET -
		                              ((uint8)getintvararray(pi->vars, phy_var_name, 0));

		/* 5G mid: */
		pi->noiselvl_5gm[core] = ACPHY_SROM_NOISELVL_OFFSET -
		                              ((uint8)getintvararray(pi->vars, phy_var_name, 1));

		/* 5G high: */
		pi->noiselvl_5gh[core] = ACPHY_SROM_NOISELVL_OFFSET -
		                              ((uint8)getintvararray(pi->vars, phy_var_name, 2));

		/* 5G upper: */
		pi->noiselvl_5gu[core] = ACPHY_SROM_NOISELVL_OFFSET -
		                              ((uint8)getintvararray(pi->vars, phy_var_name, 3));
	}
	if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
		(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0))
	{
		/* update pi[0] to hold pwrdet params for all cores */
		/* This is required for mimo operation */
		pi->pubpi->phy_corenum >>= 1;
	}
}

/* register phy type specific implementation */
phy_ac_noise_info_t *
BCMATTACHFN(phy_ac_noise_register_impl)(phy_info_t *pi, phy_ac_info_t *ac_info,
	phy_noise_info_t *cmn_info)
{
	phy_ac_noise_info_t *noise_info;
	phy_type_noise_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((noise_info = phy_malloc(pi, sizeof(phy_ac_noise_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->nshapetbl_mon = phy_malloc(pi, sizeof(acphy_nshapetbl_mon_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc nshapetbl_mon failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->hwaci_states_2g =
		phy_malloc(pi, sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES)) == NULL) {
		PHY_ERROR(("%s: phy_malloc hwaci_states_2g failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->hwaci_states_5g =
		phy_malloc(pi, sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES)) == NULL) {
		PHY_ERROR(("%s: phy_malloc hwaci_states_5g failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->hwaci_states_5g_40_80 =
		phy_malloc(pi, sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES)) == NULL) {
		PHY_ERROR(("%s: phy_malloc hwaci_states_5g_40_80 failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->aci_list2g =
		phy_malloc(pi, sizeof(acphy_aci_params_t) * ACPHY_ACI_CHAN_LIST_SZ)) == NULL) {
		PHY_ERROR(("%s: phy_malloc aci_list2g failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->aci_list5g =
		phy_malloc(pi, sizeof(acphy_aci_params_t) * ACPHY_ACI_CHAN_LIST_SZ)) == NULL) {
		PHY_ERROR(("%s: phy_malloc aci_list5g failed\n", __FUNCTION__));
		goto fail;
	}
	if ((noise_info->def_gains =
		phy_malloc(pi, sizeof(acphy_hwaci_defgain_settings_t) * PHY_CORE_MAX)) == NULL) {
		PHY_ERROR(("%s: phy_malloc def_gains failed\n", __FUNCTION__));
		goto fail;
	}

#ifndef WLC_DISABLE_ACI
	if ((noise_info->hwaci_args = phy_malloc(pi, sizeof(acphy_hwaci_setup_t))) == NULL) {
		PHY_ERROR(("%s: hwaci_args malloc failed\n", __FUNCTION__));
		goto fail;
	}
#endif // endif

	noise_info->pi = pi;
	noise_info->ac_info = ac_info;
	noise_info->cmn_info = cmn_info;
	noise_info->data = &(((phy_ac_noise_mem_t *) noise_info)->data);

	phy_ac_noise_attach_modes(pi);
	phy_ac_noise_attach_params(noise_info);

#ifndef WLC_DISABLE_ACI
	/* hwaci params */
	if (!(ACPHY_ENABLE_FCBS_HWACI(pi)))
		wlc_phy_hwaci_init_acphy(noise_info);
#endif /* !WLC_DISABLE_ACI */

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_ac_noise_wd, noise_info,
		PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_ACI,
		PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.reset = phy_ac_noise_reset;
	fns.mode = phy_ac_noise_set_mode;
	fns.calc = phy_ac_noise_calc;
	fns.calc_fine = phy_ac_noise_calc_fine_resln;
#ifndef WLC_DISABLE_ACI
	fns.interf_rssi_update = wlc_phy_desense_aci_upd_chan_stats_acphy;
	fns.aci_upd = phy_ac_noise_upd_aci;
	fns.ma_upd = phy_ac_noise_update_aci_ma;
#endif // endif
#ifndef WL_ACI_FCBS
	fns.aci_mitigate = phy_ac_noise_aci_mitigate;
#endif /* !WL_ACI_FCBS */
	fns.dump = phy_ac_noise_dump;
	fns.request_noise_sample = phy_ac_noise_request_sample;
	fns.avg = phy_ac_noise_avg;
	fns.sample_intr = phy_ac_noise_sample_intr;
	fns.cb = phy_ac_noise_cb;
	fns.lte_avg = phy_ac_noise_lte_avg;
	fns.get_srom_level = phy_ac_noise_get_srom_level;
	fns.read_shm = phy_ac_noise_read_shmem;
	fns.ctx = noise_info;

#ifdef RADIO_HEALTH_CHECK
	fns.health_check_desense = phy_ac_noise_healthcheck_desense;
#endif /* RADIO_HEALTH_CHECK */

	phy_ac_srom_read_noiselvl(pi);

	if (phy_noise_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_noise_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return noise_info;

	/* error handling */
fail:
	phy_ac_noise_unregister_impl(noise_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_noise_attach_params)(phy_ac_noise_info_t *noisei)
{
	phy_info_t *pi = noisei->pi;

	pi->sh->interference_mode = INTERFERE_NONE;

	bzero(noisei->aci_list2g, ACPHY_ACI_CHAN_LIST_SZ * sizeof(acphy_aci_params_t));
	bzero(noisei->aci_list5g, ACPHY_ACI_CHAN_LIST_SZ * sizeof(acphy_aci_params_t));
	noisei->LTEJ_WAR_en = (bool)PHY_GETINTVAR_DEFAULT(pi, rstr_LTEJ_WAR_en, 1);
	noisei->aci = NULL;
}

static void
BCMATTACHFN(phy_ac_noise_attach_modes)(phy_info_t *pi)
{
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_1(pi->pubpi->phy_rev) ||
	    ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* ACPHY_ACI_GLITCHBASED_DESENSE or ACPHY_ACI_HWACI_PKTGAINLMT */
		pi->sh->interference_mode_2G = ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G = ACPHY_ACI_GLITCHBASED_DESENSE;
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		/* ADD HW_ACI and preemption for 4350/43569/43570 */
		pi->sh->interference_mode_2G |= ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G |= ACPHY_ACI_GLITCHBASED_DESENSE;
		if (PHY_XTAL_IS40M(pi)) {
			/* for 43569/43570 turn-on swaci + preemption only */
			pi->sh->interference_mode_2G |=
				ACPHY_ACI_GLITCHBASED_DESENSE | ACPHY_ACI_HWACI_PKTGAINLMT |
				ACPHY_ACI_W2NB_PKTGAINLMT | ACPHY_ACI_PREEMPTION |
				ACPHY_LPD_PREEMPTION;
			pi->sh->interference_mode_5G |=
				ACPHY_ACI_GLITCHBASED_DESENSE | ACPHY_ACI_HWACI_PKTGAINLMT |
				ACPHY_ACI_W2NB_PKTGAINLMT | ACPHY_ACI_PREEMPTION;
		}
	}
	if (ACREV_IS(pi->pubpi->phy_rev, 1) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* 4360b0 */
		pi->sh->interference_mode_2G |= ACPHY_ACI_HWACI_PKTGAINLMT |
		        ACPHY_ACI_W2NB_PKTGAINLMT;
		pi->sh->interference_mode_5G |= ACPHY_ACI_HWACI_PKTGAINLMT |
		        ACPHY_ACI_W2NB_PKTGAINLMT;
	} else if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
		/* 4360a0 */
		pi->sh->interference_mode_2G |= ACPHY_ACI_W2NB_PKTGAINLMT;
		pi->sh->interference_mode_5G |= ACPHY_ACI_W2NB_PKTGAINLMT;
	}
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		/* 4335C0 */
		pi->sh->interference_mode_2G |= ACPHY_ACI_HWACI_PKTGAINLMT |
		        ACPHY_ACI_W2NB_PKTGAINLMT | ACPHY_ACI_PREEMPTION;
		pi->sh->interference_mode_5G |= ACPHY_ACI_HWACI_PKTGAINLMT |
		        ACPHY_ACI_W2NB_PKTGAINLMT | ACPHY_ACI_PREEMPTION;
	} else if (ACMAJORREV_3(pi->pubpi->phy_rev) && CHIPREV(pi->sh->chiprev) >= 4) {
		/* 4345 B0 onwards */
		pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION;
		pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION;
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349 A0 onwards */
		pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION
			| ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION
			| ACPHY_ACI_GLITCHBASED_DESENSE;
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION |
			ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION |
			ACPHY_ACI_GLITCHBASED_DESENSE;
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		/* 4365 */
		pi->sh->interference_mode_2G = ACPHY_ACI_GLITCHBASED_DESENSE |
				ACPHY_HWACI_MITIGATION;
		pi->sh->interference_mode_5G = ACPHY_ACI_GLITCHBASED_DESENSE |
				ACPHY_HWACI_MITIGATION;

		/* Enable preemption/LPD by default for B1/C0 */
		if ((ACMAJORREV_32(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
			ACMAJORREV_33(pi->pubpi->phy_rev)) {
			pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION;
			pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION;
			//disable LPD for 5G due to impact to MU operation: need to root cause
			//pi->sh->interference_mode_2G |= ACPHY_LPD_PREEMPTION;
			//pi->sh->interference_mode_5G |= ACPHY_LPD_PREEMPTION;
		}
	} else if (ACMAJORREV_37(pi->pubpi->phy_rev)) {
		pi->sh->interference_mode_2G = ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G = ACPHY_ACI_GLITCHBASED_DESENSE;
	} else if (ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) {
		/* No premption for 43684A0/B0 */
		pi->sh->interference_mode_2G |= ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G |= ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_2G |= ACPHY_ACI_HWACI_PKTGAINLMT;
		pi->sh->interference_mode_5G |= ACPHY_ACI_HWACI_PKTGAINLMT;
		if (ACMINORREV_GE(pi, 2)) {
			pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION;
			pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION;
		}
	} else if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		if (ACMINORREV_1(pi)) {
			pi->sh->interference_mode_2G |= ACPHY_ACI_GLITCHBASED_DESENSE
				| ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION;
			pi->sh->interference_mode_5G |= ACPHY_ACI_GLITCHBASED_DESENSE
				| ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION;
		} else {
			pi->sh->interference_mode_2G |= ACPHY_ACI_GLITCHBASED_DESENSE
				| ACPHY_HWACI_MITIGATION;
			pi->sh->interference_mode_5G |= ACPHY_ACI_GLITCHBASED_DESENSE
				| ACPHY_HWACI_MITIGATION;
		}
	} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
		pi->sh->interference_mode_2G |= ACPHY_ACI_PREEMPTION;
		pi->sh->interference_mode_5G |= ACPHY_ACI_PREEMPTION;
	}
	/* Enable preemption for 4364 slices */
	if (IS_4364_1x1(pi)) {
		pi->sh->interference_mode_2G = ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION;
		pi->sh->interference_mode_5G = ACPHY_ACI_PREEMPTION;
	}

	if (IS_4364_3x3(pi)) {
		pi->sh->interference_mode_2G = ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION
			| ACPHY_ACI_GLITCHBASED_DESENSE;
		pi->sh->interference_mode_5G = ACPHY_ACI_PREEMPTION | ACPHY_HWACI_MITIGATION
			| ACPHY_ACI_GLITCHBASED_DESENSE;
	}
}

void
BCMATTACHFN(phy_ac_noise_unregister_impl)(phy_ac_noise_info_t *noise_info)
{
	phy_info_t *pi;
	phy_noise_info_t *cmn_info;

	if (noise_info == NULL) {
		return;
	}

	pi = noise_info->pi;
	cmn_info = noise_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_noise_unregister_impl(cmn_info);

#ifndef WLC_DISABLE_ACI
	if (noise_info->hwaci_args != NULL) {
		phy_mfree(pi, noise_info->hwaci_args, sizeof(acphy_hwaci_setup_t));
	}
#endif // endif

	if (noise_info->def_gains != NULL) {
		phy_mfree(pi, noise_info->def_gains,
			sizeof(acphy_hwaci_defgain_settings_t) * PHY_CORE_MAX);
	}
	if (noise_info->aci_list5g != NULL) {
		phy_mfree(pi, noise_info->aci_list5g,
			sizeof(acphy_aci_params_t) * ACPHY_ACI_CHAN_LIST_SZ);
	}
	if (noise_info->aci_list2g != NULL) {
		phy_mfree(pi, noise_info->aci_list2g,
			sizeof(acphy_aci_params_t) * ACPHY_ACI_CHAN_LIST_SZ);
	}
	if (noise_info->hwaci_states_5g_40_80 != NULL) {
		phy_mfree(pi, noise_info->hwaci_states_5g_40_80,
			sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES);
	}
	if (noise_info->hwaci_states_5g != NULL) {
		phy_mfree(pi, noise_info->hwaci_states_5g,
			sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES);
	}
	if (noise_info->hwaci_states_2g != NULL) {
		phy_mfree(pi, noise_info->hwaci_states_2g,
			sizeof(acphy_hwaci_state_t) * ACPHY_HWACI_MAX_STATES);
	}
	if (noise_info->nshapetbl_mon != NULL) {
		phy_mfree(pi, noise_info->nshapetbl_mon, sizeof(acphy_nshapetbl_mon_t));
	}
	phy_mfree(pi, noise_info, sizeof(phy_ac_noise_mem_t));
}

static int
phy_ac_noise_reset(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	BCM_REFERENCE(pi);

	PHY_TRACE(("%s\n", __FUNCTION__));

#ifndef WLC_DISABLE_ACI
	/* update interference mode before INIT process start */
	if (!(SCAN_INPROG_PHY(pi))) {
		pi->sh->interference_mode = CHSPEC_IS2G(pi->radio_chanspec) ?
			pi->sh->interference_mode_2G : pi->sh->interference_mode_5G;
	}
#endif // endif

	return BCME_OK;
}

static void
phy_ac_noise_calc(phy_type_noise_ctx_t *ctx, int8 cmplx_pwr_dbm[], uint8 extra_gain_1dB)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = noise_info->pi;
	/* init gain is fixed to -65 for acphy,
	 *-37 = 10*log10((0.4/512)*(0.4/512)*(16)*(1/50.0))
	*/
	/* knoise support, read gain value from shm, by Peyush */
	int16 assumed_gain;
	uint8 i;

	FOREACH_CORE(pi, i) {
		if (BFCTL(pi->u.pi_acphy) == 2) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_2G);
			} else {
				assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_5G);
			}
		} else {
#ifdef WL_EAP_NOISE_MEASUREMENTS
			assumed_gain = wlapi_bmac_read_shm(pi->sh->physhim,
				M_PWRIND_BLKS(pi) + 0x10);
#else
			assumed_gain = (int16)(ACPHY_NOISE_INITGAIN);
#endif // endif
		}

		assumed_gain += extra_gain_1dB;

		if (ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev)) {
			cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM_10BIT -
				assumed_gain));
		} else if ((ACMAJORREV_36(pi->pubpi->phy_rev))) {
			cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM_43012
						- assumed_gain));
		} else if ((ACMAJORREV_37(pi->pubpi->phy_rev)) ||
			(ACMAJORREV_51(pi->pubpi->phy_rev)) ||
			(ACMAJORREV_40(pi->pubpi->phy_rev))) {
			/* apply this to 4347 and 7271 (with 1.4v/12bit SAR-ADC) */
			cmplx_pwr_dbm[i] += (int16) (((ACPHY_NOISE_SAMPLEPWR_TO_QDBM_4347+2) >> 2)
						- assumed_gain);
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			cmplx_pwr_dbm[i] += (int16) (((ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ47 +
				2) >> 2) - assumed_gain);
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			/* phy_maj44 SAR-ADC is 1.2Vpp */
			cmplx_pwr_dbm[i] += (int16) (((ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ44 +
				2) >> 2) - assumed_gain);
		} else {
			cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM - assumed_gain));
		}
		noise_info->data->phy_noise_all_core[i] =  cmplx_pwr_dbm[i] + (assumed_gain);
	}
}

static void
phy_ac_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
	uint8 extra_gain_1dB, int16 *tot_gain)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = noise_info->pi;
	int16 assumed_gain;
	uint8 i;
	FOREACH_CORE(pi, i) {
		assumed_gain = tot_gain[i];
		if (assumed_gain == 0) {
			if (BFCTL(pi->u.pi_acphy) == 2) {
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_2G);
				} else {
					assumed_gain = (int16)(ACPHY_NOISE_INITGAIN_X29_5G);
				}
			} else {
				assumed_gain = (int16)(ACPHY_NOISE_INITGAIN);
			}
		}
		assumed_gain += extra_gain_1dB;

		if ((ACMAJORREV_40(pi->pubpi->phy_rev)) ||
		           (ACMAJORREV_37(pi->pubpi->phy_rev)) ||
		           (ACMAJORREV_51(pi->pubpi->phy_rev))) {
			/* apply this to 4347 and 7271 (with 1.4v/12bit SAR-ADC) */
			cmplx_pwr_dbm[i] += (int16) (ACPHY_NOISE_SAMPLEPWR_TO_QDBM_4347 -
			                             (assumed_gain << 2));
		} else if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			cmplx_pwr_dbm[i] += (int16) (ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ47 -
			                             (assumed_gain << 2));
		} else if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
			/* phy_maj44 SAR-ADC is 1.2Vpp */
			cmplx_pwr_dbm[i] += (int16) (ACPHY_NOISE_SAMPLEPWR_TO_QDBM_PHYMAJ44 -
			                             (assumed_gain << 2));
		} else {
			cmplx_pwr_dbm[i] += (int16) ((ACPHY_NOISE_SAMPLEPWR_TO_DBM -
				assumed_gain) << 2);
		}
	}
}

/* set mode */
static void
phy_ac_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = noise_info->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		if (wanted_mode == INTERFERE_NONE) {
#ifndef WLC_DISABLE_ACI
			wlc_phy_desense_aci_reset_params_acphy(pi, TRUE, TRUE, TRUE);

			/* Run noise-cal after reset of desense */
			phy_ac_rxgcrs_cal(pi->u.pi_acphy->rxgcrsi, FALSE);
#endif // endif
			wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_DISABLE, FALSE);
			phy_ac_chanmgr_hwobss(pi->u.pi_acphy->chanmgri, FALSE);
			phy_ac_noise_preempt(noise_info, FALSE, FALSE);
		} else if ((wanted_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) {
			wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_AUTO_SW, TRUE);
		}
		if ((wanted_mode & ACPHY_ACI_PREEMPTION) != 0) {
			/* Configured in Pre-Filter Processing mode */
			phy_ac_noise_preempt(noise_info, TRUE, FALSE);
		}
		if ((wanted_mode & ACPHY_HWOBSS_MITIGATION) != 0) {
			phy_ac_chanmgr_hwobss(pi->u.pi_acphy->chanmgri, TRUE);
		}
	} else {
		/* Depending on interfernece modes, do whatever needs to be done */
		if (wanted_mode == INTERFERE_NONE) {
#ifndef WLC_DISABLE_ACI
			wlc_phy_desense_aci_reset_params_acphy(pi, TRUE, TRUE, TRUE);
			if (ACPHY_ENABLE_FCBS_HWACI(pi) || ((ACMAJORREV_32(pi->pubpi->phy_rev) ||
			                                     ACMAJORREV_33(pi->pubpi->phy_rev) ||
			                                     ACMAJORREV_37(pi->pubpi->phy_rev) ||
			                                     ACMAJORREV_51(pi->pubpi->phy_rev)) &&
			                                    ((pi->sh->interference_mode &
			                                      ACPHY_HWACI_MITIGATION) != 0)))
				wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_DISABLE, FALSE);
			else
				wlc_phy_hwaci_setup_acphy(pi, FALSE, FALSE);
#endif // endif
			wlc_phy_aci_w2nb_setup_acphy(pi, FALSE);
			phy_ac_noise_preempt(noise_info, FALSE, FALSE);
			phy_ac_chanmgr_hwobss(pi->u.pi_acphy->chanmgri, FALSE);
		}

		/* Nothing needs to be done for ACPHY_ACI_GLITCHBASED_DESENSE */
#ifndef WLC_DISABLE_ACI
		/* Switch on( & init) the required rssi settings */
		if ((wanted_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) {
			if (!ACPHY_ENABLE_FCBS_HWACI(pi))
				wlc_phy_hwaci_setup_acphy(pi, TRUE, TRUE);
		} else if ((wanted_mode & ACPHY_HWACI_MITIGATION) != 0) {
			if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_47_51(pi->pubpi->phy_rev) &&
			    !ACPHY_ENABLE_FCBS_HWACI(pi))
				wlc_phy_hwaci_setup_acphy(pi, TRUE, TRUE);
			else
#ifdef WL_ACI_FCBS
				wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_AUTO_FCBS, TRUE);
#else
			wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_AUTO_SW, TRUE);
#endif /* WL_ACI_FCBS */
		}
#endif /* WLC_DISABLE_ACI */
		if (!ACPHY_ENABLE_FCBS_HWACI(pi) &&
		    (wanted_mode & ACPHY_ACI_W2NB_PKTGAINLMT) != 0) {
			wlc_phy_aci_w2nb_setup_acphy(pi, TRUE);
		}

		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_51(pi->pubpi->phy_rev)) {
#ifndef WLC_DISABLE_ACI
			if ((wanted_mode & ACPHY_HWACI_MITIGATION) == 0) {
				wlc_phy_hwaci_mitigation_enable_acphy(pi, HWACI_DISABLE, FALSE);
			}
#endif // endif
		}
		if ((wanted_mode & ACPHY_ACI_PREEMPTION) != 0)
			/* Configured in Pre-Filter Processing mode */
			phy_ac_noise_preempt(noise_info, TRUE, FALSE);
		if ((wanted_mode & ACPHY_HWOBSS_MITIGATION) != 0) {
			phy_ac_chanmgr_hwobss(pi->u.pi_acphy->chanmgri, TRUE);
		}
	}
}

/* watchdog callback */
static bool
phy_ac_noise_wd(phy_wd_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = noise_info->pi;
	phy_noise_info_t *noisei = noise_info->cmn_info;

	/* defer interference checking, scan and update if RM is progress */
	if (!SCAN_RM_IN_PROGRESS(pi)) {
		wlc_phy_aci_upd(noisei);
	}

	return TRUE;
}

#if defined(BCMDBG) || defined(WLTEST)
static int
phy_ac_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = noise_info->pi;
	uint8 bw;
	acphy_desense_values_t desense;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	int8 rssi = 0;
	uint8 jammer_cnt = 0, txop = 0, half_dB = 0;

	if (SCAN_RM_IN_PROGRESS(pi)) {
		bcm_bprintf(b, "Scanning is in progress. Can't dump aci mitigation info.\n");
		return BCME_ERROR;
	}

	if (noise_info->aci != NULL) {
		rssi = noise_info->aci->weakest_rssi;
		jammer_cnt = noise_info->aci->jammer_cnt;
		txop = noise_info->aci->txop;
	}

	bw = CHSPEC_BW_LE20(pi->radio_chanspec) ? 20 : CHSPEC_IS40(pi->radio_chanspec) ? 40 :
	    CHSPEC_IS80(pi->radio_chanspec) ? 80 : 160;

	/* Get total desense based on aci & bt */
	desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, TOTAL_DESENSE);
	half_dB = desense.ofdm_desense_extra_halfdB;

	bcm_bprintf(b, "\n");
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		bcm_bprintf(b, "*** Channel = %d(%d mhz), Weakest RSSI = %d, Associated = %d ** \n",
		            CHSPEC_CHANNEL(pi->radio_chanspec), bw, rssi,
		            !(ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)));
	} else {
		bcm_bprintf(b, "*** Channel = %d(%d mhz), Desense(mode 1) On = %d *** \n",
		            CHSPEC_CHANNEL(pi->radio_chanspec), bw, desense.on);
	}

	if (half_dB)
		bcm_bprintf(b, "OFDM desense (dB) = %d.5\n", desense.ofdm_desense);
	else
		bcm_bprintf(b, "OFDM desense (dB) = %d\n", desense.ofdm_desense);
	bcm_bprintf(b, "BPHY desense (dB) = %d\n", desense.bphy_desense);
	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		if (noise_info->aci != NULL) {
			bcm_bprintf(b, "HWACI mitigation triggered = %d\n",
					noise_info->aci->hwaci_desense_state);
		} else {
			bcm_bprintf(b, "No mitigation information available: pi_ac->aci is NULL\n");
		}
	} else {
		bcm_bprintf(b, "lna1 tbl desense (ticks) = %d\n", desense.lna1_tbl_desense);
		bcm_bprintf(b, "lna2 tbl desense (ticks) = %d\n", desense.lna2_tbl_desense);
		bcm_bprintf(b, "lna1 pktgain limit (ticks) = %d\n", desense.lna1_gainlmt_desense);
		bcm_bprintf(b, "lna2 pktgain limit (ticks) = %d\n", desense.lna2_gainlmt_desense);
		bcm_bprintf(b, "elna bypass = %d\n", desense.elna_bypass);
		bcm_bprintf(b, "forced = %d\n", desense.forced);
		bcm_bprintf(b, "jammer_cnt = %d, txop = %d\n", jammer_cnt, txop);
	}
	if (ACHWACIREV(pi)) {
		bcm_bprintf(b, "hwaci detected = %d\n", noise_info->data->hw_aci_status);
	}
	bcm_bprintf(b, "\n");

	return BCME_OK;
}
#endif // endif

/* Internal data api between ac modules */
phy_ac_noise_data_t *
phy_ac_noise_get_data(phy_ac_noise_info_t *noisei)
{
	return noisei->data;
}

void
phy_ac_noise_set_gainidx(phy_ac_noise_info_t *noisei, uint16 gainidx)
{
	noisei->data->gain_idx_forced = gainidx;
}

void
phy_ac_noise_set_trigger_crsmin_cal(phy_ac_noise_info_t *noisei, bool trigger_crsmin_cal)
{
	noisei->data->trigger_crsmin_cal = trigger_crsmin_cal;
}

void
chanspec_noise(phy_info_t *pi)
{
	pi->u.pi_acphy->noisei->aci = NULL;
	bzero((uint8 *)pi->u.pi_acphy->noisei->data->phy_noise_all_core,
	      sizeof(pi->u.pi_acphy->noisei->data->phy_noise_all_core));

	if (!wlc_phy_is_scan_chan_acphy(pi)) {
		pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
#ifndef WLC_DISABLE_ACI
		/* Get pointer to current aci channel list */
		if (!ACPHY_ENABLE_FCBS_HWACI(pi) || ACPHY_HWACI_WITH_DESENSE_ENG(pi)) {
			pi->u.pi_acphy->noisei->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
				pi->radio_chanspec, TRUE);
		}
#endif /* !WLC_DISABLE_ACI */
	}

	if (ACMAJORREV_GE37(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev)) {
		return;
	}
	if (ACHWACIREV(pi)) {
		wlc_phy_hwaci_setup_acphy(pi, TRUE, TRUE);
		pi->u.pi_acphy->noisei->data->hw_aci_status = FALSE;
#ifndef WLC_DISABLE_ACI
		wlc_phy_save_def_gain_settings_acphy(pi);
#endif // endif
	}
}

#ifndef WLC_DISABLE_ACI

/* ********************************************* */
/*				Internal Definitions					*/
/* ********************************************* */
static uint32
wlc_phy_desense_aci_get_avg_max_glitches_acphy(phy_info_t *pi, uint32 glitches[])
{
	uint8 i, j;
	uint32 max_glitch, glitch_cnt = 0;
	uint8 max_glitch_idx;
	uint32 glitches_sort[ACPHY_ACI_GLITCH_BUFFER_SZ];

	for (i = 0; i < ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
		glitches_sort[i] = glitches[i];

	/* Get 2 max from the list */
	for (j = 0; j < ACPHY_ACI_NUM_MAX_GLITCH_AVG; j++) {
		max_glitch_idx = 0;
		max_glitch = glitches_sort[0];
		for (i = 1; i <  ACPHY_ACI_GLITCH_BUFFER_SZ; i++) {
			if (glitches_sort[i] > max_glitch) {
				max_glitch_idx = i;
				max_glitch = glitches_sort[i];
			}
		}
		glitches_sort[max_glitch_idx] = 0;
		glitch_cnt += max_glitch;
		if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
			if (j == 1) {
				glitch_cnt = max_glitch;
				return glitch_cnt;
			}
		}
	}

	/* avg */
	glitch_cnt /= ACPHY_ACI_NUM_MAX_GLITCH_AVG;
	return glitch_cnt;
}

static uint8
wlc_phy_desense_aci_calc_acphy(phy_info_t *pi, desense_history_t *aci_desense, uint8 desense,
	uint32 glitch_cnt, uint16 glitch_th_lo, uint16 glitch_th_hi,
	bool enable_half_dB, uint8 *desense_half_dB)
{
	uint8 hi, lo, cnt, cnt_thresh = 0;

	hi = aci_desense->hi_glitch_dB;
	lo = aci_desense->lo_glitch_dB;
	cnt = aci_desense->no_desense_change_time_cnt;

	if (glitch_cnt > glitch_th_hi) {
		hi = desense;
		if (hi > lo) desense += ACPHY_ACI_COARSE_DESENSE_UP;
		else if (enable_half_dB) {
			/* If half dB desense is enabled, then when going up, increase by 2dBs */
			desense = MAX(desense + 2, (hi + lo) >> 1);
		} else {
			desense = MAX(desense + 1, (hi + lo) >> 1);
		}
		cnt = 0;
		if (!enable_half_dB) *desense_half_dB = 0;
	} else {
		/* Sleep for different times under different conditions */
		if (desense - hi == 1) {
			cnt_thresh = ACPHY_ACI_BORDER_GLITCH_SLEEP;
		} else {
			cnt_thresh = (glitch_cnt >= glitch_th_lo) ? ACPHY_ACI_MD_GLITCH_SLEEP :
			    ACPHY_ACI_LO_GLITCH_SLEEP;
		}

		/* Double the wait time if we are going down by 1/2 dB steps */
		if (enable_half_dB)
			cnt_thresh = (cnt_thresh << 1);

		if (cnt > cnt_thresh) {
			lo = desense;
			cnt = 0;
			if (enable_half_dB) {
				if (*desense_half_dB) {
					*desense_half_dB = 0;
				} else {
					*desense_half_dB = 1;
					desense = MAX(0, desense - 1);
				}
			} else {
				*desense_half_dB = 0;
				if (lo <= hi) desense = MAX(0, desense -
				                            ACPHY_ACI_COARSE_DESENSE_DN);
				else desense = MAX(0, MIN(desense - 1, (hi + lo) >> 1));
			}
		}
	}

	PHY_ACI(("aci_mode1, lo = %d, hi = %d, desense = %d, cnt = %d(%d)\n",  lo, hi, desense,
	         cnt, cnt_thresh));

	/* Update the values */
	aci_desense->hi_glitch_dB = hi;
	aci_desense->lo_glitch_dB = lo;
	aci_desense->no_desense_change_time_cnt = MIN(cnt + 1, 255);

	return desense;
}

static void
phy_ac_noise_upd_aci(phy_type_noise_ctx_t * ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;

	if (!(ACPHY_ENABLE_FCBS_HWACI(pi)) || ACPHY_HWACI_WITH_DESENSE_ENG(pi)) {
		if ((pi->sh->interference_mode & ACPHY_ACI_GLITCHBASED_DESENSE) != 0) {
			wlc_phy_desense_aci_engine_acphy(pi);
		}
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		if (pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT)
			wlc_phy_hwaci_engine_acphy_28nm(pi);
	} else if (!ACPHY_ENABLE_FCBS_HWACI(pi)) {
		if ((pi->sh->interference_mode & (ACPHY_ACI_HWACI_PKTGAINLMT |
		                                  ACPHY_ACI_W2NB_PKTGAINLMT)) != 0) {
			wlc_phy_hwaci_engine_acphy(pi);
		}
	}
}

static void
phy_ac_noise_update_aci_ma(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;

	int32 delta = 0;
	int32 bphy_delta = 0;
	int32 ofdm_delta = 0;
	int32 badplcp_delta = 0;
	int32 bphy_badplcp_delta = 0;
	int32 ofdm_badplcp_delta = 0;

	if ((pi->interf->cca_stats_func_called == FALSE) || pi->interf->cca_stats_mbsstime <= 0) {
		uint16 cur_glitch_cnt;
		uint16 bphy_cur_glitch_cnt = 0;
		uint16 cur_badplcp_cnt = 0;
		uint16 bphy_cur_badplcp_cnt = 0;

#ifdef WLSRVSDB
		uint8 bank_offset = 0;
		uint8 vsdb_switch_failed = 0;
		uint8 vsdb_split_cntr = 0;

		if (CHSPEC_CHANNEL(pi->radio_chanspec) == pi->srvsdb_state->sr_vsdb_channels[0]) {
			bank_offset = 0;
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->srvsdb_state->sr_vsdb_channels[1]) {
			bank_offset = 1;
		}

		/* Assume vsdb switch failed, if no switches were recorded for both the channels */
		vsdb_switch_failed = !(pi->srvsdb_state->num_chan_switch[0] &
		pi->srvsdb_state->num_chan_switch[1]);

		/* use split counter for each channel
		* if vsdb is active and vsdb switch was successfull
		*/
		/* else use last 1 sec delta counter
		* for current channel for calculations
		*/
		vsdb_split_cntr = (!vsdb_switch_failed) && (pi->srvsdb_state->srvsdb_active);

#endif /* WLSRVSDB */

		/* determine delta number of rxcrs glitches */
		cur_glitch_cnt =
			wlapi_bmac_read_shm(pi->sh->physhim, MACSTAT_ADDR(pi, MCSTOFF_RXCRSGLITCH));
		delta = cur_glitch_cnt - pi->interf->aci.pre_glitch_cnt;
		pi->interf->aci.pre_glitch_cnt = cur_glitch_cnt;

#ifdef WLSRVSDB
		if (vsdb_split_cntr) {
			delta =  pi->srvsdb_state->sum_delta_crsglitch[bank_offset];
		}
#endif /* WLSRVSDB */

		/* compute the rxbadplcp  */
		cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
			MACSTAT_ADDR(pi, MCSTOFF_RXBADPLCP));
		badplcp_delta = cur_badplcp_cnt - pi->interf->pre_badplcp_cnt;
		pi->interf->pre_badplcp_cnt = cur_badplcp_cnt;

#ifdef WLSRVSDB
		if (vsdb_split_cntr) {
			badplcp_delta = pi->srvsdb_state->sum_delta_prev_badplcp[bank_offset];
		}
#endif /* WLSRVSDB */
		/* determine delta number of bphy rx crs glitches */
		bphy_cur_glitch_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
			MACSTAT_ADDR(pi, MCSTOFF_BPHYGLITCH));
		bphy_delta = bphy_cur_glitch_cnt - pi->interf->noise.bphy_pre_glitch_cnt;
		pi->interf->noise.bphy_pre_glitch_cnt = bphy_cur_glitch_cnt;

#ifdef WLSRVSDB
		if (vsdb_split_cntr) {
			bphy_delta = pi->srvsdb_state->sum_delta_bphy_crsglitch[bank_offset];
		}
#endif /* WLSRVSDB */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			/* ofdm glitches is what we will be using */
			ofdm_delta = delta - bphy_delta;
		} else {
			ofdm_delta = delta;
		}

		/* compute bphy rxbadplcp */
		bphy_cur_badplcp_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
				MACSTAT_ADDR(pi, MCSTOFF_BPHY_BADPLCP));
		bphy_badplcp_delta = bphy_cur_badplcp_cnt -
			pi->interf->bphy_pre_badplcp_cnt;
		pi->interf->bphy_pre_badplcp_cnt = bphy_cur_badplcp_cnt;

#ifdef WLSRVSDB
		if (vsdb_split_cntr) {
			bphy_badplcp_delta =
				pi->srvsdb_state->sum_delta_prev_bphy_badplcp[bank_offset];
		}

#endif /* WLSRVSDB */

		/* ofdm bad plcps is what we will be using */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			ofdm_badplcp_delta = badplcp_delta - bphy_badplcp_delta;
		} else {
			ofdm_badplcp_delta = badplcp_delta;
		}
	} else {
		pi->interf->cca_stats_func_called = FALSE;
		/* Normalizing the statistics per second */
		delta = pi->interf->cca_stats_total_glitch * 1000 /
			pi->interf->cca_stats_mbsstime;
		bphy_delta = pi->interf->cca_stats_bphy_glitch * 1000 /
			pi->interf->cca_stats_mbsstime;
		ofdm_delta = delta - bphy_delta;
		badplcp_delta = pi->interf->cca_stats_total_badplcp * 1000 /
			pi->interf->cca_stats_mbsstime;
		bphy_badplcp_delta = pi->interf->cca_stats_bphy_badplcp * 1000 /
			pi->interf->cca_stats_mbsstime;
		ofdm_badplcp_delta = badplcp_delta - bphy_badplcp_delta;
	}

	if (delta >= 0) {
		/* evict old value */
		pi->interf->aci.ma_total -= pi->interf->aci.ma_list[pi->interf->aci.ma_index];

		/* admit new value */
		pi->interf->aci.ma_total += (uint16) delta;
		pi->interf->aci.glitch_ma_previous = pi->interf->aci.glitch_ma;
		pi->interf->aci.glitch_ma = pi->interf->aci.ma_total /
			PHY_NOISE_MA_WINDOW_SZ;

		pi->interf->aci.ma_list[pi->interf->aci.ma_index++] = (uint16) delta;

		if (pi->interf->aci.ma_index >= PHY_NOISE_MA_WINDOW_SZ) {
			pi->interf->aci.ma_index = 0;
		}
	}

	if (badplcp_delta >= 0) {
		pi->interf->badplcp_ma_total -=
			pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index];
		pi->interf->badplcp_ma_total += (uint16) badplcp_delta;
		pi->interf->badplcp_ma_previous = pi->interf->badplcp_ma;
		pi->interf->badplcp_ma =
			pi->interf->badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;

		pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index++] =
			(uint16) badplcp_delta;

		if (pi->interf->badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ) {
			pi->interf->badplcp_ma_index = 0;
		}
	}

	if ((CHSPEC_IS5G(pi->radio_chanspec) && (ofdm_delta >= 0)) ||
		(CHSPEC_IS2G(pi->radio_chanspec) && (delta >= 0) && (bphy_delta >= 0))) {
		pi->interf->noise.ofdm_ma_total -= pi->interf->noise.
				ofdm_glitch_ma_list[pi->interf->noise.ofdm_ma_index];
		pi->interf->noise.ofdm_ma_total += (uint16) ofdm_delta;
		pi->interf->noise.ofdm_glitch_ma_previous =
			pi->interf->noise.ofdm_glitch_ma;
		pi->interf->noise.ofdm_glitch_ma =
			pi->interf->noise.ofdm_ma_total / PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.ofdm_glitch_ma_list[pi->interf->noise.ofdm_ma_index++] =
			(uint16) ofdm_delta;
		if (pi->interf->noise.ofdm_ma_index >= PHY_NOISE_MA_WINDOW_SZ) {
			pi->interf->noise.ofdm_ma_index = 0;
		}
	}

	if (bphy_delta >= 0) {
		pi->interf->noise.bphy_ma_total -= pi->interf->noise.
				bphy_glitch_ma_list[pi->interf->noise.bphy_ma_index];
		pi->interf->noise.bphy_ma_total += (uint16) bphy_delta;
		pi->interf->noise.bphy_glitch_ma_previous =
			pi->interf->noise.bphy_glitch_ma;
		pi->interf->noise.bphy_glitch_ma =
			pi->interf->noise.bphy_ma_total / PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.bphy_glitch_ma_list[pi->interf->noise.bphy_ma_index++] =
			(uint16) bphy_delta;
		if (pi->interf->noise.bphy_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
			pi->interf->noise.bphy_ma_index = 0;
	}

	if ((CHSPEC_IS5G(pi->radio_chanspec) && (ofdm_badplcp_delta >= 0)) ||
		(CHSPEC_IS2G(pi->radio_chanspec) && (badplcp_delta >= 0) &&
		(bphy_badplcp_delta >= 0))) {
		pi->interf->noise.ofdm_badplcp_ma_total -= pi->interf->noise.
			ofdm_badplcp_ma_list[pi->interf->noise.ofdm_badplcp_ma_index];
		pi->interf->noise.ofdm_badplcp_ma_total += (uint16) ofdm_badplcp_delta;
		pi->interf->noise.ofdm_badplcp_ma_previous =
			pi->interf->noise.ofdm_badplcp_ma;
		pi->interf->noise.ofdm_badplcp_ma =
			pi->interf->noise.ofdm_badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.ofdm_badplcp_ma_list
			[pi->interf->noise.ofdm_badplcp_ma_index++] =
			(uint16) ofdm_badplcp_delta;
		if (pi->interf->noise.ofdm_badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
			pi->interf->noise.ofdm_badplcp_ma_index = 0;
	}

	if (bphy_badplcp_delta >= 0) {
		pi->interf->noise.bphy_badplcp_ma_total -= pi->interf->noise.
			bphy_badplcp_ma_list[pi->interf->noise.bphy_badplcp_ma_index];
		pi->interf->noise.bphy_badplcp_ma_total += (uint16) bphy_badplcp_delta;
		pi->interf->noise.bphy_badplcp_ma_previous = pi->interf->noise.
			bphy_badplcp_ma;
		pi->interf->noise.bphy_badplcp_ma =
			pi->interf->noise.bphy_badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
		pi->interf->noise.bphy_badplcp_ma_list
				[pi->interf->noise.bphy_badplcp_ma_index++] =
				(uint16) bphy_badplcp_delta;
		if (pi->interf->noise.bphy_badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ)
			pi->interf->noise.bphy_badplcp_ma_index = 0;
	}

	PHY_ACI(("phy_ac_noise_update_aci_ma: ave glitch %d, ACI is %s, delta is %d\n",
		pi->interf->aci.glitch_ma,
		(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive", delta));

#ifdef WLSRVSDB
	/* Clear out cumulatiove cntrs after 1 sec */
	/* reset both chan info becuase its a fresh start after every 1 sec */
	if (pi->srvsdb_state->srvsdb_active) {
		bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));
		bzero(pi->srvsdb_state->sum_delta_crsglitch, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_bphy_crsglitch, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_prev_badplcp, 2 * sizeof(uint32));
		bzero(pi->srvsdb_state->sum_delta_prev_bphy_badplcp, 2 * sizeof(uint32));
	}
#endif /* WLSRVSDB */
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */

/********** DESENSE (ACI, CCI, Noise - glitch based) ******** */

acphy_desense_values_t*
phy_ac_noise_get_desense(phy_ac_noise_info_t *noisei)
{
	if (noisei->aci != NULL) {
		return &noisei->aci->desense;
	} else {
		return NULL;
	}
}

uint8 phy_ac_noise_get_desense_state(phy_ac_noise_info_t *noisei)
{
	if (noisei->aci != NULL) {
		return noisei->aci->hwaci_desense_state;
	} else {
		return 0;
	}
}

int8
phy_ac_noise_get_weakest_rssi(phy_ac_noise_info_t *noisei)
{
	if (noisei->aci != NULL) {
		return noisei->aci->weakest_rssi;
	} else {
		return 0;
	}
}

#ifndef WLC_DISABLE_ACI
void
wlc_phy_hwaci_init_acphy(phy_ac_noise_info_t *noisei)
{
	phy_info_t *pi = noisei->pi;

	BCM_REFERENCE(pi);

	/* common */
	noisei->hwaci_args->energy_thresh = 0x3e8;
	noisei->hwaci_args->detect_thresh = 0x1f4;
	noisei->hwaci_args->wait_period = 0x1;
	noisei->hwaci_args->sliding_window = 0xf;
	noisei->hwaci_args->samp_cluster = 0xf;
	noisei->hwaci_args->w3_lo_th = 0x0;
	noisei->hwaci_args->w3_md_th = 0x0;
	noisei->hwaci_args->w3_hi_th = 0x0;
	noisei->hwaci_args->w2 = 4;

	/* <= ACPHY_HWACI_MAX_STATES */
	noisei->hwaci_max_states_2g = 4;
	noisei->hwaci_max_states_5g = 4;

	/* 2g */
	bzero(noisei->hwaci_states_2g, ACPHY_HWACI_MAX_STATES * sizeof(acphy_hwaci_state_t));
	/* hwaci states: 0, no ACI settings */
	noisei->hwaci_states_2g[0].energy_thresh = 0xffff;   /* don't care */
	noisei->hwaci_states_2g[0].w2_sel = 0;
	noisei->hwaci_states_2g[0].w2_thresh = 30;
	noisei->hwaci_states_2g[0].nb_thresh = 4;
	noisei->hwaci_states_2g[0].lna1_pktg_lmt = 5;
	noisei->hwaci_states_2g[0].lna2_pktg_lmt = 6;
	noisei->hwaci_states_2g[0].lna1rout_pktg_lmt = 0;
	noisei->hwaci_states_2g[0].lna2rout_pktg_lmt = 0;

	/* hwaci states: 1, check/desense for > -45dBm */
	noisei->hwaci_states_2g[1].w2_sel = 0;

	/* hwaci states: 2, check/desense for > -40dBm */
	noisei->hwaci_states_2g[2].w2_sel = 1;

	/* hwaci states: 3, check/desense for > -35dBm */
	noisei->hwaci_states_2g[3].w2_sel = 2;

	/* hwaci states: 4, check/desense */
	noisei->hwaci_states_5g[4].w2_sel = 2;

	/* 5g */
	bzero(noisei->hwaci_states_5g, ACPHY_HWACI_MAX_STATES * sizeof(acphy_hwaci_state_t));
	bzero(noisei->hwaci_states_5g_40_80, ACPHY_HWACI_MAX_STATES * sizeof(acphy_hwaci_state_t));
	/* hwaci states: 0, no ACI settings */
	noisei->hwaci_states_5g[0].energy_thresh = 0xffff;   /* don't care */
	noisei->hwaci_states_5g[0].w2_sel = 0;
	noisei->hwaci_states_5g[0].w2_thresh = 30;
	noisei->hwaci_states_5g[0].nb_thresh = 4;
	noisei->hwaci_states_5g[0].lna1_pktg_lmt = 5;
	noisei->hwaci_states_5g[0].lna2_pktg_lmt = 6;
	noisei->hwaci_states_5g[0].lna1rout_pktg_lmt = 4;

	/* hwaci states: 1, check/desense */
	noisei->hwaci_states_5g[1].w2_sel = 0;

	/* hwaci states: 2, check/desense */
	noisei->hwaci_states_5g[2].w2_sel = 1;

	/* hwaci states: 3, check/desense */
	noisei->hwaci_states_5g[3].w2_sel = 2;

	/* hwaci states: 4, check/desense */
	noisei->hwaci_states_5g[4].w2_sel = 2;

	if (RADIOID(pi->pubpi->radioid) == BCM2069_ID && (RADIOREV(pi->pubpi->radiorev) == 0x2C)) {
		/* 43569/43570: http://confluence.broadcom.com/x/HJBHEQ */
		/* hwaci params */
		noisei->hwaci_args->sample_time = 300;
		noisei->hwaci_args->nb_lo_th = 0x1;
		/* 2g */
		noisei->hwaci_states_2g[1].energy_thresh = 4000;
		noisei->hwaci_states_2g[1].w2_thresh = 30;
		noisei->hwaci_states_2g[1].nb_thresh = 4;
		noisei->hwaci_states_2g[1].lna1_pktg_lmt = 5;
		noisei->hwaci_states_2g[1].lna2_pktg_lmt = 6;
		noisei->hwaci_states_2g[1].lna1rout_pktg_lmt = 3;
		noisei->hwaci_states_2g[1].lna2rout_pktg_lmt = 3;

		noisei->hwaci_states_2g[2].energy_thresh = 11000;
		noisei->hwaci_states_2g[2].w2_thresh = 14;
		noisei->hwaci_states_2g[2].nb_thresh = 4;
		noisei->hwaci_states_2g[2].lna1_pktg_lmt = 5;
		noisei->hwaci_states_2g[2].lna2_pktg_lmt = 6;
		noisei->hwaci_states_2g[2].lna1rout_pktg_lmt = 3;
		noisei->hwaci_states_2g[2].lna2rout_pktg_lmt = 9;

		noisei->hwaci_states_2g[3].energy_thresh = 17000;
		noisei->hwaci_states_2g[3].w2_thresh = 8;
		noisei->hwaci_states_2g[3].nb_thresh = 4;
		noisei->hwaci_states_2g[3].lna1_pktg_lmt = 5;
		noisei->hwaci_states_2g[3].lna2_pktg_lmt = 6;
		noisei->hwaci_states_2g[3].lna1rout_pktg_lmt = 7;
		noisei->hwaci_states_2g[3].lna2rout_pktg_lmt = 7;

		/* 5g */
		noisei->hwaci_states_5g[1].energy_thresh = 8000;
		noisei->hwaci_states_5g[1].w2_thresh = 30;
		noisei->hwaci_states_5g[1].nb_thresh = 4;
		noisei->hwaci_states_5g[1].lna1_pktg_lmt = 5;
		noisei->hwaci_states_5g[1].lna2_pktg_lmt = 5;
		noisei->hwaci_states_5g[1].lna1rout_pktg_lmt = 2;

		noisei->hwaci_states_5g[2].energy_thresh = 18000;
		noisei->hwaci_states_5g[2].w2_thresh = 25;
		noisei->hwaci_states_5g[2].nb_thresh = 4;
		noisei->hwaci_states_5g[2].lna1_pktg_lmt = 5;
		noisei->hwaci_states_5g[2].lna2_pktg_lmt = 4;
		noisei->hwaci_states_5g[2].lna1rout_pktg_lmt = 0;

		noisei->hwaci_states_5g[3].energy_thresh = 24000;
		noisei->hwaci_states_5g[3].w2_thresh = 15;
		noisei->hwaci_states_5g[3].nb_thresh = 4;
		noisei->hwaci_states_5g[3].lna1_pktg_lmt = 4;
		noisei->hwaci_states_5g[3].lna2_pktg_lmt = 4;
		noisei->hwaci_states_5g[3].lna1rout_pktg_lmt = 0;
	}

	/* 4335C0 */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		/* hwaci params */
		noisei->hwaci_args->sample_time = 300;
		noisei->hwaci_args->nb_lo_th = 0x0;
		/* 2g */
		noisei->hwaci_states_2g[1].energy_thresh = 4000;
		noisei->hwaci_states_2g[1].w2_thresh = 30;
		noisei->hwaci_states_2g[1].nb_thresh = 10;
		noisei->hwaci_states_2g[1].lna1_pktg_lmt = 4;
		noisei->hwaci_states_2g[1].lna2_pktg_lmt = 4;
		noisei->hwaci_states_2g[1].lna1rout_pktg_lmt = 0;

		noisei->hwaci_states_2g[2].energy_thresh = 9000;
		noisei->hwaci_states_2g[2].w2_thresh = 16;
		noisei->hwaci_states_2g[2].nb_thresh = 10;
		noisei->hwaci_states_2g[2].lna1_pktg_lmt = 3;
		noisei->hwaci_states_2g[2].lna2_pktg_lmt = 4;
		noisei->hwaci_states_2g[2].lna1rout_pktg_lmt = 0;

		noisei->hwaci_states_2g[3].energy_thresh = 14000;
		noisei->hwaci_states_2g[3].w2_thresh = 2;
		noisei->hwaci_states_2g[3].nb_thresh = 10;
		noisei->hwaci_states_2g[3].lna1_pktg_lmt = 3;
		noisei->hwaci_states_2g[3].lna2_pktg_lmt = 3;
		noisei->hwaci_states_2g[3].lna1rout_pktg_lmt = 0;

		/* 5g */
		noisei->hwaci_states_5g[1].energy_thresh = 1000;
		noisei->hwaci_states_5g[1].w2_thresh = 30;
		noisei->hwaci_states_5g[1].nb_thresh = 4;
		noisei->hwaci_states_5g[1].lna1_pktg_lmt = 4;
		noisei->hwaci_states_5g[1].lna2_pktg_lmt = 4;
		noisei->hwaci_states_5g[1].lna1rout_pktg_lmt = 4;

		noisei->hwaci_states_5g[2].energy_thresh = 11000;
		noisei->hwaci_states_5g[2].w2_thresh = 25;
		noisei->hwaci_states_5g[2].nb_thresh = 4;
		noisei->hwaci_states_5g[2].lna1_pktg_lmt = 3;
		noisei->hwaci_states_5g[2].lna2_pktg_lmt = 4;
		noisei->hwaci_states_5g[2].lna1rout_pktg_lmt = 4;

		noisei->hwaci_states_5g[3].energy_thresh = 17000;
		noisei->hwaci_states_5g[3].w2_thresh = 15;
		noisei->hwaci_states_5g[3].nb_thresh = 8;
		noisei->hwaci_states_5g[3].lna1_pktg_lmt = 3;
		noisei->hwaci_states_5g[3].lna2_pktg_lmt = 3;
		noisei->hwaci_states_5g[3].lna1rout_pktg_lmt = 4;
	}

	/* 4360B0 */
	if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* hwaci params */
		noisei->hwaci_args->sample_time = 300;
		noisei->hwaci_args->nb_lo_th = 0x1;
		/* 2g */
		noisei->hwaci_states_2g[1].energy_thresh = 4000;
		noisei->hwaci_states_2g[1].w2_thresh = 30;
		noisei->hwaci_states_2g[1].nb_thresh = 4;
		noisei->hwaci_states_2g[1].lna1_pktg_lmt = 5;
		noisei->hwaci_states_2g[1].lna2_pktg_lmt = 4;
		noisei->hwaci_states_2g[1].lna1rout_pktg_lmt = 0;

		noisei->hwaci_states_2g[2].energy_thresh = 8000;
		noisei->hwaci_states_2g[2].w2_thresh = 22;
		noisei->hwaci_states_2g[2].nb_thresh = 4;
		noisei->hwaci_states_2g[2].lna1_pktg_lmt = 4;
		noisei->hwaci_states_2g[2].lna2_pktg_lmt = 4;
		noisei->hwaci_states_2g[2].lna1rout_pktg_lmt = 0;

		noisei->hwaci_states_2g[3].energy_thresh = 11000;
		noisei->hwaci_states_2g[3].w2_thresh = 10;
		noisei->hwaci_states_2g[3].nb_thresh = 4;
		noisei->hwaci_states_2g[3].lna1_pktg_lmt = 3;
		noisei->hwaci_states_2g[3].lna2_pktg_lmt = 4;
		noisei->hwaci_states_2g[3].lna1rout_pktg_lmt = 0;

		/* 5g */
		if (ACREV_IS(pi->pubpi->phy_rev, 0)) {
			/* 4360 A0 */
			noisei->hwaci_states_5g[1].energy_thresh = 1000;
			noisei->hwaci_states_5g[1].w2_thresh = 30;
			noisei->hwaci_states_5g[1].nb_thresh = 4;
			noisei->hwaci_states_5g[1].lna1_pktg_lmt = 5;
			noisei->hwaci_states_5g[1].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[1].lna1rout_pktg_lmt = 4;

			noisei->hwaci_states_5g[2].energy_thresh = 6000;
			noisei->hwaci_states_5g[2].w2_thresh = 25;
			noisei->hwaci_states_5g[2].nb_thresh = 4;
			noisei->hwaci_states_5g[2].lna1_pktg_lmt = 4;
			noisei->hwaci_states_5g[2].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[2].lna1rout_pktg_lmt = 4;

			noisei->hwaci_states_5g[3].energy_thresh = 10000;
			noisei->hwaci_states_5g[3].w2_thresh = 15;
			noisei->hwaci_states_5g[3].nb_thresh = 4;
			noisei->hwaci_states_5g[3].lna1_pktg_lmt = 3;
			noisei->hwaci_states_5g[3].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[3].lna1rout_pktg_lmt = 4;
		} else {
			/* 4360 B0, 43602 */
			noisei->hwaci_max_states_5g = 5;
			noisei->hwaci_states_5g[1].energy_thresh = 3000;
			noisei->hwaci_states_5g[1].w2_thresh = 30;
			noisei->hwaci_states_5g[1].nb_thresh = 4;
			noisei->hwaci_states_5g[1].lna1_pktg_lmt = 5;
			noisei->hwaci_states_5g[1].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[1].lna1rout_pktg_lmt = 4;

			noisei->hwaci_states_5g[2].energy_thresh = 8000;
			noisei->hwaci_states_5g[2].w2_thresh = 15;
			noisei->hwaci_states_5g[2].nb_thresh = 4;
			noisei->hwaci_states_5g[2].lna1_pktg_lmt = 5;
			noisei->hwaci_states_5g[2].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[2].lna1rout_pktg_lmt = 2;

			noisei->hwaci_states_5g[3].energy_thresh = 11000;
			noisei->hwaci_states_5g[3].w2_thresh = 8;
			noisei->hwaci_states_5g[3].nb_thresh = 4;
			noisei->hwaci_states_5g[3].lna1_pktg_lmt = 5;
			noisei->hwaci_states_5g[3].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[3].lna1rout_pktg_lmt = 0;

			noisei->hwaci_states_5g[4].energy_thresh = 12000;
			noisei->hwaci_states_5g[4].w2_thresh = 25;
			noisei->hwaci_states_5g[4].nb_thresh = 4;
			noisei->hwaci_states_5g[4].lna1_pktg_lmt = 3;
			noisei->hwaci_states_5g[4].lna2_pktg_lmt = 4;
			noisei->hwaci_states_5g[4].lna1rout_pktg_lmt = 4;
		}
	}

	if (ACHWACIREV(pi)) {
		/* HW ACI Detection SW ACI Mitigation
		 * These settings have been tested well,
		 * with only LNA back off's and using no Routs.
		 */

		/* hwaci params */
		noisei->hwaci_args->energy_thresh = 3000;
		noisei->hwaci_args->energy_thresh_w2 = 3000;
		noisei->hwaci_args->detect_thresh = 1000;
		noisei->hwaci_args->nb_lo_th = 0x1;
		noisei->hwaci_args->wait_period = 0x0;
		noisei->hwaci_args->sliding_window = 0xf;
		noisei->hwaci_args->samp_cluster = 0x5;
		noisei->hwaci_args->w3_lo_th = 0x0;
		noisei->hwaci_args->w3_md_th = 0x0;
		noisei->hwaci_args->w3_hi_th = 0x1;
		noisei->hwaci_args->w2 = 10;
		noisei->hwaci_args->aci_present_th = 0;	/* 0 = 1/8; 1 = 1/4; */
		noisei->hwaci_args->aci_present_select = 2; /* 2 = W3 || W12  */
		noisei->hwaci_desense_state_ovr = -1;
		noisei->data->gain_idx_forced  = 0xffff;

		if (((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) != 0) ||
			((pi->sh->interference_mode_2G & ACPHY_HWACI_MITIGATION) != 0) ||
			((pi->sh->interference_mode_5G& ACPHY_HWACI_MITIGATION) != 0)) {
			/* HW Swithching makes a decision in 100 ms */
			noisei->hwaci_args->sample_time = 100;
		} else if (((pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) ||
			((pi->sh->interference_mode_2G & ACPHY_ACI_HWACI_PKTGAINLMT) != 0) ||
			((pi->sh->interference_mode_5G & ACPHY_ACI_HWACI_PKTGAINLMT) != 0)) {
			/* SW Swithching makes a decision in 990 ms */
			noisei->hwaci_args->sample_time = 990;
		}
	}
}

void
wlc_phy_save_def_gain_settings_acphy(phy_info_t *pi)
{
	uint8 core;

	acphy_hwaci_defgain_settings_t *pi_ac_dgain;

	ASSERT(pi->u.pi_acphy->noisei != NULL);
	ASSERT(pi->u.pi_acphy->noisei->hwaci_args != NULL);

	pi_ac_dgain = pi->u.pi_acphy->noisei->def_gains;

	FOREACH_CORE(pi, core) {
		if (!ACMAJORREV_5(pi->pubpi->phy_rev)) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINLIMIT,
			6, 8, 8, pi_ac_dgain[core].lna1_gainlim_ofdm_def);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINLIMIT,
			6, 72, 8, pi_ac_dgain[core].lna1_gainlim_cck_def);
		} else {
			wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAINLIMIT0:
				(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1: ACPHY_TBL_ID_GAINLIMIT2,
				6, 8, 8, pi_ac_dgain[core].lna1_gainlim_ofdm_def);
			wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAINLIMIT0:
				(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1: ACPHY_TBL_ID_GAINLIMIT2,
				6, 72, 8, pi_ac_dgain[core].lna1_gainlim_cck_def);
		}

		if (!ACMAJORREV_5(pi->pubpi->phy_rev)) {
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINLIMIT,
				6, 17, 8, pi_ac_dgain[core].lna2_gainlim_ofdm_def);
			wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_GAINLIMIT,
				6, 81, 8, pi_ac_dgain[core].lna2_gainlim_cck_def);
		} else {
			wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAINLIMIT0:
				(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1: ACPHY_TBL_ID_GAINLIMIT2,
				6, 17, 8, pi_ac_dgain[core].lna2_gainlim_ofdm_def);
			wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAINLIMIT0:
				(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1: ACPHY_TBL_ID_GAINLIMIT2,
				6, 81, 8, pi_ac_dgain[core].lna2_gainlim_cck_def);
		}
		wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAIN0:
			(core == 1) ? ACPHY_TBL_ID_GAIN1: ACPHY_TBL_ID_GAIN2,
			6, 17, 8, pi_ac_dgain[core].lna2_gaindb_def);
		wlc_phy_table_read_acphy(pi, (core == 0) ? ACPHY_TBL_ID_GAINBITS0:
			(core == 1) ? ACPHY_TBL_ID_GAINBITS1: ACPHY_TBL_ID_GAINBITS2,
			6, 17, 8, pi_ac_dgain[core].lna2_gainbits_def);
	}
}

void
wlc_phy_hwaci_mitigate_acphy(phy_info_t *pi, bool aci_status)
{
	acphy_hwaci_defgain_settings_t *pi_ac_dgain;

	/* These are HW ACI Mitigation settings (to be written to FCBS table) */
	uint8 *lna1_gainlim_ofdm;
	uint8 *lna1_gainlim_cck;

	uint8 *lna2_gainlim_ofdm;
	uint8 *lna2_gaindb;
	uint8 *lna2_gainbits;

	uint8 lna1_gainlim_ofdm_updated = 0;
	uint8 lna1_gainlim_cck_updated  = 0;

	uint8 lna2_gainlim_ofdm_updated = 0;
	uint8 lna2_gaindb_updated = 0;
	uint8 lna2_gainbits_updated = 0;

	uint8 core;
	uint16 table_id_core[PHY_CORE_MAX];
	uint16 start_off_core[PHY_CORE_MAX];

	/* If any of the 10 settings (7 for 5G) are changed,
	* they will be initialized to new values here
	* If they are not being changed, they will be read
	* from the phytable, so that they are initialized.
	*/
	uint8 lna1_gainlim_ofdm_2g_aci[6] = {0xb, 0xc, 0xe, 0x20, 0x7f, 0x7f};
	uint8 lna1_gainlim_cck_2g_aci[6] = {0xb, 0xc, 0xe, 0xf, 0x7f, 0x7f};

	uint8 lna2_gaindb_2g_aci[6] = {-8, -4, -1, 2, 5, 9};
	uint8 lna2_gainbits_2g_aci[6] = {1, 2, 3, 4, 5, 6};

	uint8 lna1_gainlim_5g_aci[6] = {0xb, 0xc, 0xe, 0x20, 0x7f, 0x7f};

	uint8 lna2_gainlim_5g_aci[6] = {0x0, 0x0, 0x3, 0x3, 0x3, 0x7f};

	bool suspend;
	uint8 stall_val;

	phy_ac_noise_info_t *noisei = pi->u.pi_acphy->noisei;
	ASSERT(noisei != NULL);
	ASSERT(noisei->hwaci_args != NULL);

	pi_ac_dgain = noisei->def_gains;

	ASSERT(pi_ac_dgain);

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
	}
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	if (stall_val == 0)
		ACPHY_DISABLE_STALL(pi);

	FOREACH_CORE(pi, core) {
		/* Initialize to defaults */
		lna1_gainlim_ofdm =  pi_ac_dgain[core].lna1_gainlim_ofdm_def;
		lna1_gainlim_cck =  pi_ac_dgain[core].lna1_gainlim_cck_def;
		lna2_gainlim_ofdm = pi_ac_dgain[core].lna2_gainlim_ofdm_def;
		lna2_gaindb = pi_ac_dgain[core].lna2_gaindb_def;
		lna2_gainbits = pi_ac_dgain[core].lna2_gainbits_def;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			lna1_gainlim_ofdm =   aci_status ? lna1_gainlim_ofdm_2g_aci :
				pi_ac_dgain[core].lna1_gainlim_ofdm_def;
			lna1_gainlim_cck =    aci_status ? lna1_gainlim_cck_2g_aci :
				pi_ac_dgain[core].lna1_gainlim_cck_def;
			lna2_gaindb =         aci_status ? lna2_gaindb_2g_aci:
				pi_ac_dgain[core].lna2_gaindb_def;
			lna2_gainbits =       aci_status ? lna2_gainbits_2g_aci :
				pi_ac_dgain[core].lna2_gainbits_def;
			lna1_gainlim_ofdm_updated = 1;
			lna1_gainlim_cck_updated = 1;
			lna2_gaindb_updated = 1;
			lna2_gainbits_updated = 1;
		} else {
			lna1_gainlim_ofdm =	  aci_status ? lna1_gainlim_5g_aci :
				pi_ac_dgain[core].lna1_gainlim_ofdm_def;
			lna2_gainlim_ofdm =	  aci_status ? lna2_gainlim_5g_aci :
				pi_ac_dgain[core].lna2_gainlim_ofdm_def;
			lna1_gainlim_ofdm_updated = 1;
			lna2_gainlim_ofdm_updated = 1;
		}

		if (IS_4364_3x3(pi)) {
			/* Dont limit lna2 for 3x3 slice */
			lna2_gainlim_ofdm_updated = 0;

			/* core2 on 3x3 slice has 4db less gain till lna1,
			   so change limit index to 4 instead of 3
			 */
			if ((pi->sh->boardflags4 & BFL4_4364_HARPOON) &&
				CHSPEC_IS5G(pi->radio_chanspec) &&
				(core == 2)) {
				uint8 lna1_gainlim_5g_aci_core2[6] =
				{0xb, 0xc, 0xe, 0x20, 0x24, 0x7f};
				lna1_gainlim_ofdm = aci_status ? lna1_gainlim_5g_aci_core2 :
					pi_ac_dgain[core].lna1_gainlim_ofdm_def;
			}

			if ((pi->sh->boardflags4 & BFL4_4364_GODZILLA) &&
				CHSPEC_IS80(pi->radio_chanspec)) {
				uint8 lna1_gainlim_5g_aci_4364[6] =
				{0xb, 0xc, 0xe, 0x7f, 0x7f, 0x7f};
				lna1_gainlim_ofdm =	  aci_status ? lna1_gainlim_5g_aci_4364 :
					pi_ac_dgain[core].lna1_gainlim_ofdm_def;
			}

		}

		if (!ACMAJORREV_5(pi->pubpi->phy_rev)) {
			/* LNA1 settings */
			if (lna1_gainlim_ofdm_updated) {
				table_id_core[core] = ACPHY_TBL_ID_GAINLIMIT;
				start_off_core[core] = (core == 0) ? 8 : 0xff;
				wlc_phy_hwaci_write_table_acphy(pi, table_id_core[core],
					start_off_core[core], lna1_gainlim_ofdm, 0);
			}
			if (lna1_gainlim_cck_updated) {
				table_id_core[core] = ACPHY_TBL_ID_GAINLIMIT;
				start_off_core[core] = (core == 0) ? 72 : 0xff;
				wlc_phy_hwaci_write_table_acphy(pi, table_id_core[core],
					start_off_core[core], lna1_gainlim_cck, 1);
			}
			/* LNA2 settings */
			if (lna2_gainlim_ofdm_updated) {
				table_id_core[core] = ACPHY_TBL_ID_GAINLIMIT;
				start_off_core[core] = (core == 0) ? 17 : 0xff;
				wlc_phy_hwaci_write_table_acphy(pi, table_id_core[core],
					start_off_core[core], lna2_gainlim_ofdm, 0);
			}
			if (lna2_gaindb_updated) {
				table_id_core[core] = (core == 0) ? ACPHY_TBL_ID_GAIN0 :
					ACPHY_TBL_ID_GAIN1;
				start_off_core[core] = 17;
				wlc_phy_hwaci_write_table_acphy(pi, table_id_core[core],
					start_off_core[core], lna2_gaindb, 0);
			}
			if (lna2_gainbits_updated) {
				table_id_core[core] = (core == 0) ? ACPHY_TBL_ID_GAINBITS0 :
					ACPHY_TBL_ID_GAINBITS1;
				start_off_core[core] = 17;
				wlc_phy_hwaci_write_table_acphy(pi, table_id_core[core],
					start_off_core[core], lna2_gainbits, 0);
			}
		} else {	/* 43602 */
			/* LNA1 settings */
			if (lna1_gainlim_ofdm_updated) {
				wlc_phy_hwaci_write_table_acphy(pi, (core == 0) ?
					ACPHY_TBL_ID_GAINLIMIT0:
					(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1:
					ACPHY_TBL_ID_GAINLIMIT2,
					8, lna1_gainlim_ofdm, 0);
			}
			if (lna1_gainlim_cck_updated) {
				wlc_phy_hwaci_write_table_acphy(pi, (core == 0) ?
					ACPHY_TBL_ID_GAINLIMIT0:
					(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1:
					ACPHY_TBL_ID_GAINLIMIT2,
					72, lna1_gainlim_cck, 1);
			}
			/* LNA2 settings */
			if (lna2_gainlim_ofdm_updated) {
				wlc_phy_hwaci_write_table_acphy(pi, (core == 0) ?
					ACPHY_TBL_ID_GAINLIMIT0:
					(core == 1) ? ACPHY_TBL_ID_GAINLIMIT1:
					ACPHY_TBL_ID_GAINLIMIT2,
					17, lna2_gainlim_ofdm, 0);
			}
			if (lna2_gaindb_updated) {
				wlc_phy_hwaci_write_table_acphy(pi, (core == 0) ?
					ACPHY_TBL_ID_GAIN0:
					(core == 1) ? ACPHY_TBL_ID_GAIN1:
					ACPHY_TBL_ID_GAIN2,
					17, lna2_gaindb, 0);
			}
			if (lna2_gainbits_updated) {
				wlc_phy_hwaci_write_table_acphy(pi, (core == 0) ?
					ACPHY_TBL_ID_GAINBITS0:
					(core == 1) ? ACPHY_TBL_ID_GAINBITS1:
					ACPHY_TBL_ID_GAINBITS2,
					17, lna2_gainbits, 0);
			}
			/* tighten FSTR to prevent bphy from passing FSTR */
			if (CHSPEC_IS2G(pi->radio_chanspec) && IS43602WLCSP) {
				MOD_PHYREG(pi, FSTRMetricTh, lowPwr_min_metric_th,
					aci_status ? 0x18 : 0x20);
			}
		}
	}

	wlc_phy_switch_preemption_settings(pi, aci_status);

	/* Update INIT/CLIP-HI etc with gain limits programmed */
	if (IS_4364_3x3(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {
		if (aci_status == 0) {
			noisei->aci->desense.lna1_tbl_desense = 0;
		} else {
			noisei->aci->desense.lna1_tbl_desense = 1;
		}
		noisei->aci->desense.analog_gain_desense_ofdm = 0;
		noisei->aci->desense.analog_gain_desense_bphy = 0;
		noisei->aci->desense.lna2_tbl_desense = 0;
		noisei->aci->desense.clipgain_desense[0] = 0;
		noisei->aci->desense.clipgain_desense[1] = 0;
		noisei->aci->desense.clipgain_desense[2] = 0;
		noisei->aci->desense.clipgain_desense[3] = 0;
		/* Get total desense based on aci & bt & lte */
		wlc_phy_desense_calc_total_acphy(pi->u.pi_acphy->rxgcrsi);
		/* Update LNA1 gain table with limit values */
		wlc_phy_upd_lna1_lna2_gaintbls_acphy(pi, 1);
		/* Call gain control function to recompute INIT/CLIP-HI etc.
			based on gain limit table
		*/
		wlc_phy_rxgainctrl_gainctrl_acphy(pi);
	}

	ACPHY_ENABLE_STALL(pi, stall_val);
	if (!suspend) {
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	}
}

static void
wlc_phy_hwaci_write_table_acphy(phy_info_t *pi, uint16 table_id,
uint16 start_off, uint8 *table_entry, bool check_5G)
{
	uint8 def_values[6];
	if ((start_off == 0xff) || (CHSPEC_IS5G(pi->radio_chanspec) && check_5G))
		return;
	wlc_phy_table_read_acphy(pi, table_id,
	6, start_off, 8, def_values);
	if (!memcmp(def_values, table_entry, sizeof(uint8)*6)) {
		start_off = 0xff;
	}
	if (start_off != 0xff) {
		wlc_phy_table_write_acphy(pi, table_id,
			6, start_off, 8, table_entry);
	}

}

void
wlc_phy_hwaci_setup_acphy(phy_info_t *pi, bool on, bool init)
{
	uint8 core, on_off;
	uint8 wrssi3_ib_Refbuf;
	uint8 nbrssi_Refctrl_low;
	uint8 wrssi3_Refctrl_low, wrssi3_Refctrl_mid, wrssi3_Refctrl_high;
	uint8 wrssi3_ib_powersaving, wrssi3_ib_Refladder;
	uint16 regval;

	/* For ACI_Detect_CTRL */
	uint8 aci_detect_window_size;
	uint8 aci_rpt_det_ctr_clren;
	uint8 aci_detect_enable;
	uint8 aci_detect_clkenable;
	uint8 aci_present_th;

	/* For ACPHY_ACI_Detect_collect_interval */
	uint8 aci_detect_collect_interval;

	/* ACI_Detect_wait_period 0x552 0x553 */
	uint16 aci_detect_wait_period;

	/* Energy and detect threshold 0x554 0x555 0x556 0x557 */
	uint16 aci_detect_energy_threshold;
	uint16 aci_detect_diff_threshold;
	uint16 aci_detect_energy_threshold_w2;
	uint16 aci_detect_diff_threshold_w2;

	/* ACI_Detect_MAX_COUNT 0x558 0x559 */
	uint32 max_count;
	uint16 sample_time_period_ms;
	acphy_hwaci_setup_t hwaci;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	on_off = on ? 1 : 0;
	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) || ACHWACIREV(pi)) {
		on_off = on ? 7 : 0;
	}
	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, RfctrlCoreTxPus, core, lpf_wrssi3_pwrup, on_off);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, lpf_wrssi3_pwrup, 0x1);
	}
	if (ACHWACIREV(pi)) {
		FOREACH_CORE(pi, core) {
			MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_wrssi2_pwrup, 1);
			MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_wrssi2_pwrup, 0x1);
		}
	}

	if (!init) return;

	ASSERT(RADIOID_IS(pi->pubpi->radioid, BCM2069_ID));
	ASSERT(pi_ac->noisei->hwaci_args != NULL);

	hwaci = *pi_ac->noisei->hwaci_args;

	/* These are the thresholds of comparators */
	wrssi3_ib_Refladder = 0x0;
	wrssi3_ib_Refbuf = 0x0;
	wrssi3_ib_powersaving = 0x0;
	nbrssi_Refctrl_low = hwaci.nb_lo_th;
	wrssi3_Refctrl_low = hwaci.w3_lo_th;
	wrssi3_Refctrl_mid = hwaci.w3_md_th;
	wrssi3_Refctrl_high = hwaci.w3_hi_th;

	/* For ACI_Detect_CTRL */
	aci_detect_window_size = hwaci.sliding_window;
	aci_rpt_det_ctr_clren = 1;
	aci_detect_enable = 1;
	aci_detect_clkenable = 1;
	aci_present_th = hwaci.aci_present_th;

	/* For ACPHY_ACI_Detect_collect_interval */
	aci_detect_collect_interval = hwaci.samp_cluster;

	/* ACI_Detect_wait_period 0x552 0x553 */
	aci_detect_wait_period = hwaci.wait_period;

	/* Energy and detect threshold 0x554 0x555 0x556 0x557 */
	aci_detect_energy_threshold = hwaci.energy_thresh;
	aci_detect_diff_threshold = hwaci.detect_thresh;
	aci_detect_energy_threshold_w2 = hwaci.energy_thresh_w2;
	aci_detect_diff_threshold_w2 = hwaci.detect_thresh;

	/* hwaci energy thresholds */
	if (IS_4364_3x3(pi)) {
		if (pi->sh->boardflags4 & BFL4_4364_GODZILLA) {
			/* GODZILLA MODULES */
			aci_detect_energy_threshold = HWACI_DETECT_ENERGY_TH_GODZILLA;
			aci_detect_energy_threshold_w2 = HWACI_DETECT_ENERGY_TH_GODZILLA;
		} else {
			/* REF BOARDS & HARPOON MODULES */
			aci_detect_energy_threshold = HWACI_DETECT_ENERGY_TH_HARPOON;
			aci_detect_energy_threshold_w2 = HWACI_DETECT_ENERGY_TH_HARPOON;
		}
	}

	/* ACI_Detect_MAX_COUNT 0x558 0x559 */
	/* For 1 sec, it is = 1000 */
	sample_time_period_ms = hwaci.sample_time;

	FOREACH_CORE(pi, core) {
		/*  Configure nb_low can't touch mid, hi as auto-gainctrl is going to pick those */
		MOD_RADIO_REGC(pi, NBRSSI_CONFG, core, nbrssi_Refctrl_low, nbrssi_Refctrl_low);

		/* Configure w3 */
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core, wrssi3_ib_Refladder, wrssi3_ib_Refladder);
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core, wrssi3_ib_Refbuf, wrssi3_ib_Refbuf);
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core,
		               wrssi3_ib_powersaving, wrssi3_ib_powersaving);
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core, wrssi3_Refctrl_low, wrssi3_Refctrl_low);
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core, wrssi3_Refctrl_mid, wrssi3_Refctrl_mid);
		MOD_RADIO_REGC(pi, WRSSI3_CONFG, core, wrssi3_Refctrl_high, wrssi3_Refctrl_high);

		if (ACHWACIREV(pi)) {
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
			  MOD_RADIO_REGC(pi, LNA2G_RSSI, core, dig_wrssi2_threshold, hwaci.w2);
			} else {
			  MOD_RADIO_REGC(pi, LNA5G_RSSI, core, dig_wrssi2_threshold, hwaci.w2);
			}
		}
	}

	/* ACPHY_ACI_Detect_CTRL */
	/* Reset ACI detection block */
	MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable, 0);
	regval = READ_PHYREG(pi, ACI_Detect_CTRL);
	regval = (regval & ~ACPHY_ACI_Detect_CTRL_aci_detect_enable_MASK(pi->pubpi->phy_rev)) |
	        (aci_detect_enable <<
		 ACPHY_ACI_Detect_CTRL_aci_detect_enable_SHIFT(pi->pubpi->phy_rev));
	regval = (regval & ~ACPHY_ACI_Detect_CTRL_aci_report_ctr_clren_MASK(pi->pubpi->phy_rev)) |
	        (aci_rpt_det_ctr_clren <<
		 ACPHY_ACI_Detect_CTRL_aci_report_ctr_clren_SHIFT(pi->pubpi->phy_rev));
	regval = (regval & ~ACPHY_ACI_Detect_CTRL_aci_detected_ctr_clren_MASK(pi->pubpi->phy_rev)) |
	        (aci_rpt_det_ctr_clren <<
		 ACPHY_ACI_Detect_CTRL_aci_detected_ctr_clren_SHIFT(pi->pubpi->phy_rev));
	regval = (regval &
		~ACPHY_ACI_Detect_CTRL_aci_detect_window_size_1_MASK(pi->pubpi->phy_rev)) |
	        (aci_detect_window_size <<
		 ACPHY_ACI_Detect_CTRL_aci_detect_window_size_1_SHIFT(pi->pubpi->phy_rev));
	regval = (regval &
		~ACPHY_ACI_Detect_CTRL_aci_detect_window_size_2_MASK(pi->pubpi->phy_rev)) |
	        (aci_detect_window_size <<
		 ACPHY_ACI_Detect_CTRL_aci_detect_window_size_2_SHIFT(pi->pubpi->phy_rev));
	if (ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) {
		regval = (regval &
			~ACPHY_ACI_Detect_CTRL_aci_detect_clkenable_MASK(pi->pubpi->phy_rev)) |
		        (aci_detect_clkenable <<
			 ACPHY_ACI_Detect_CTRL_aci_detect_clkenable_SHIFT(pi->pubpi->phy_rev));
	}
	WRITE_PHYREG(pi, ACI_Detect_CTRL, regval);

	/* ACPHY_ACI_Detect_collect_interval */
	MOD_PHYREG(pi, ACI_Detect_collect_interval, aci_detect_collect_interval_1,
	           aci_detect_collect_interval);
	MOD_PHYREG(pi, ACI_Detect_collect_interval, aci_detect_collect_interval_2,
	           aci_detect_collect_interval);

	/* ACI_Detect_wait_period */
	WRITE_PHYREG(pi, ACI_Detect_wait_period_1, aci_detect_wait_period);
	WRITE_PHYREG(pi, ACI_Detect_wait_period_2, aci_detect_wait_period);

	if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		/* Energy threshold */
		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1_w3, aci_detect_energy_threshold);
		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2_w3, aci_detect_energy_threshold);

		/* Detect threshold */
		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_1_w3, aci_detect_diff_threshold);
		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_2_w3, aci_detect_diff_threshold);
	} else {
		/* Energy threshold */
		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1, aci_detect_energy_threshold);
		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2, aci_detect_energy_threshold);

		/* Detect threshold */
		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_1, aci_detect_diff_threshold);
		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_2, aci_detect_diff_threshold);
	}

	if (AC4354REV(pi) || ACHWACIREV(pi)) {
		/* Energy threshold */
		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1_w12, aci_detect_energy_threshold_w2);

		WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2_w12, aci_detect_energy_threshold_w2);

		/* Detect threshold */
		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_1_w12, aci_detect_diff_threshold_w2);

		WRITE_PHYREG(pi, ACI_Detect_detect_threshold_2_w12, aci_detect_diff_threshold_w2);
	}

	/* max count = time/0.8 us */
	max_count = sample_time_period_ms * 1000 * 10/8;
	WRITE_PHYREG(pi, ACI_Detect_MAX_COUNT_LO, (uint16) (max_count & 0xffff));
	WRITE_PHYREG(pi, ACI_Detect_MAX_COUNT_HI, (uint16) (max_count >> 16));

	/* SlnaControl bt prisel to be inv for 2G HWACI to work 4335C0 */
	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_3(pi->pubpi->phy_rev) || (IS_4364_3x3(pi))) {
		MOD_PHYREG(pi, SlnaControl, inv_btcx_prisel_polarity, 1);
	}
	/* make sure 43570 does not go through code */
	if (ACHWACIREV(pi) || (AC4354REV(pi) && !
		(RADIOID(pi->pubpi->radioid) == BCM2069_ID &&
		(RADIOREV(pi->pubpi->radiorev) == 0x2C)))) {
		if (!ACMAJORREV_5(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, ACI_Mitigation_CTRL1, aci_present_th_mit_on_w12,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL1, aci_present_th_mit_off_w12,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_present_th_mit_on_w3,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_present_th_mit_off_w3,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Detect_CTRL1, aci_present_select,
					hwaci.aci_present_select);
		} else {
			MOD_PHYREG(pi, ACI_Mitigation_CTRL2, aci_present_th_mit_on_w12,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL2, aci_present_th_mit_off_w12,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_present_th_mit_on_w3,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_present_th_mit_off_w3,
					aci_present_th);
			MOD_PHYREG(pi, ACI_Detect_CTRL1, aci_present_select,
					hwaci.aci_present_select);
		}

		if (((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) == 0)) {
			/* HW ACI Detection and SW Mitigation */
		  ACPHY_REG_LIST_START
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_enable, 1)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 1)
		    MOD_PHYREG_ENTRY(pi, ACI_Detect_report_ctr_threshold_lo, aci_report_ctr_th_lo,
		    30)
		  ACPHY_REG_LIST_EXECUTE(pi)
		} else {
			/* HW ACI Detection and HW Mitigation */
		  ACPHY_REG_LIST_START
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, ACIMitigationIndicatorBit, 0)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, ACIMitigationONShadowBit, 0)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, Disable_ChannelIndicator, 1)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_enable, 2)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_timeout_LO, aci_mitigation_timeout_lo, 0)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_timeout_HI, aci_mitigation_timeout_hi, 0)
		    MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 0)
		    MOD_PHYREG_ENTRY(pi, ACI_Detect_report_ctr_threshold_lo, aci_report_ctr_th_lo,
		    30)
		  ACPHY_REG_LIST_EXECUTE(pi);
		}
	}
}
#endif /* WLC_DISABLE_ACI */

void
wlc_phy_aci_w2nb_setup_acphy(phy_info_t *pi, bool on)
{
	uint8 core, on_off;

	on_off = on ? 1 : 0;
	FOREACH_CORE(pi, core) {
		MOD_PHYREGCE(pi, RfctrlCoreRxPus, core, rxrf_lna2_wrssi2_pwrup, on_off);
		MOD_PHYREGCE(pi, RfctrlOverrideRxPus, core, rxrf_lna2_wrssi2_pwrup, 0x1);

#ifndef WLC_DISABLE_ACI
		if (TINY_RADIO(pi) || ACMAJORREV_36(pi->pubpi->phy_rev))
			PHY_INFORM(("%s: HWACI not yet implemented for Tiny Radio chips\n",
				__FUNCTION__));
		else if (on) {
			MOD_RADIO_REGC(pi, LNA5G_RSSI, core, dig_wrssi2_threshold,
			               pi->u.pi_acphy->noisei->hwaci_args->w2);
			if (ACMAJORREV_2(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) {
				MOD_RADIO_REGC(pi, LNA2G_RSSI, core,
				dig_wrssi2_threshold, pi->u.pi_acphy->noisei->hwaci_args->w2);
			}
		}
#endif /* !WLC_DISABLE_ACI */
	}
}

typedef struct _hwaci_fcbs_phytbl_list_entry hwaci_fcbs_phytbl_list_entry;

#ifdef WL_ACI_FCBS
static uint16 hwaci_fcbs_rfregs[5];
static uint16 hwaci_fcbs_phyregs[14];
#define FCBS_1DATA_PER_ADDR  0x2000
static  hwaci_fcbs_phytbl_list_entry  hwaci_fcbs_phytbls [ ] = {
	{ ACPHY_TBL_ID_RFSEQ,       0x0f6, 1 },
	{ ACPHY_TBL_ID_RFSEQ,		 0x0f9, 1 },
	{ ACPHY_TBL_ID_GAINLIMIT,	 0x008, 7 },
	{ ACPHY_TBL_ID_GAINLIMIT,	 0x048, 7 },
	{ ACPHY_TBL_ID_GAIN0,		 0x008, 8 },
	{ ACPHY_TBL_ID_GAINBITS0,	 0x008, 8 },
	{ 0xFFFF,	 0x000, 0 }
};

static void
wlc_phy_hwaci_fcbsinit_acphy(phy_info_t *pi)
{
	hwaci_fcbs_rfregs[0] = RF0_20691_LNA2G_RSSI1(pi->pubpi->radiorev);
	hwaci_fcbs_rfregs[1] = RF0_20691_LNA5G_RSSI1(pi->pubpi->radiorev);
	hwaci_fcbs_rfregs[2] = RF0_20691_TIA_CFG13(pi->pubpi->radiorev);
	hwaci_fcbs_rfregs[3] = RF0_20691_TIA_CFG12(pi->pubpi->radiorev);
	hwaci_fcbs_rfregs[4] = 0xFFFF;

	hwaci_fcbs_phyregs[0] = ACPHY_Core0clip2GainCodeA(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[1] = ACPHY_Core0clipHiGainCodeA(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[2] = ACPHY_Core0cliploGainCodeA(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[3] = ACPHY_Core0clipmdGainCodeA(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[4] = ACPHY_Core0InitGainCodeA(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[5] = ACPHY_Core0clip2GainCodeB(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[6] = ACPHY_Core0clipHiGainCodeB(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[7] = ACPHY_Core0cliploGainCodeB(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[8] = ACPHY_Core0clipmdGainCodeB(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[9] = ACPHY_Core0InitGainCodeB(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[10] = ACPHY_Core0DSSScckPktGain(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[11] = ACPHY_Core0HpFBw(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[12] = ACPHY_Core0RssiClipMuxSel(pi->pubpi->phy_rev);
	hwaci_fcbs_phyregs[13] = 0xFFFF;
}
#endif /* WL_ACI_FCBS */

uint8
wlc_phy_disable_hwaci_fcbs_trig(phy_info_t *pi)
{
#ifdef WL_ACI_FCBS
	uint8 curr_val = READ_PHYREGFLD(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable);

	MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1);

	while ((READ_PHYREG(pi, FastChanSW_Status) & 0xe) != 0x0) {
		OSL_DELAY(5);   /* Expected time for FCBS(for HWACI) complete */
	}

	return curr_val;
#else
	return 0;
#endif /* WL_ACI_FCBS */
}

void
wlc_phy_restore_hwaci_fcbs_trig(phy_info_t *pi, uint8 trig_disable)
{
#ifdef WL_ACI_FCBS
	MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, trig_disable);
#else
	return;
#endif // endif
}

#ifndef WLC_DISABLE_ACI
/* From proc acphy_hwaci_mitigation_enable {{ enable 0 }} { */
void
wlc_phy_hwaci_mitigation_enable_acphy(phy_info_t *pi, uint8 hwaci_mode, bool init)
{
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	ASSERT(ACPHY_ENABLE_FCBS_HWACI(pi) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
		ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev) || (ACPHY_HWACI_28NM(pi)));

	/* Currently M_HWACI_STATUS register changes are not enabled in the uCode */
	/* For Major rev 44, currently bypassing this due to above uCode not present */
	if (!(ACMAJORREV_44(pi->pubpi->phy_rev) || ACMAJORREV_47_51(pi->pubpi->phy_rev))) {
		/* Reset to the High Sense mode */
		wlapi_bmac_write_shm(pi->sh->physhim, M_HWACI_STATUS(pi), 0);
	}

	if (ACMAJORREV_4(pi->pubpi->phy_rev) && CHSPEC_IS5G(pi->radio_chanspec)) {
		/* Disable HW ACI in 5G for 4349 */
		hwaci_mode = HWACI_DISABLE;
	}

	switch (hwaci_mode) {
	case HWACI_DISABLE:
		PHY_ACI(("HWACI: Disable HW-ACI-Mitigation\n"));
		wlc_phy_disable_hwaci_fcbs_trig(pi);
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, ACI_Detect_collect_interval, aci_detect_clkenable, 0);
		} else {
			MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_clkenable, 0);
		}
		MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_enable, 0);
		MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_enable, 0);
		if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_switching_disable, 1);
		} else if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
			phy_ac_noise_hwaci_mitigation(pi->u.pi_acphy->noisei, 0);
		}
		break;
	case HWACI_AUTO_FCBS:
	case HWACI_FORCED_MITON:
	case HWACI_FORCED_MITOFF:
	case HWACI_AUTO_SW:
		if (!ACPHY_HWACI_28NM(pi)) {
			PHY_ACI(("HWACI: Enable clocks, preempt \n"));
			if (init) {
				/* Disable HWACI while updating FCBS */
				wlc_phy_disable_hwaci_fcbs_trig(pi);
				if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				    ACMAJORREV_33(pi->pubpi->phy_rev) ||
				    ACMAJORREV_37(pi->pubpi->phy_rev) ||
				    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
					if ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0)
						phy_ac_noise_preempt(pi_ac->noisei, TRUE, FALSE);
				} else {
					phy_ac_noise_preempt(pi_ac->noisei, TRUE, FALSE);
				}

#ifdef WL_ACI_FCBS
				wlc_phy_hwaci_fcbsinit_acphy(pi);
				wlc_phy_init_FCBS_hwaci(pi);
#endif /* WL_ACI_FCBS */
			}
			MOD_PHYREG(pi, SlnaControl, inv_btcx_prisel_polarity, 1);
			MOD_PHYREG(pi, ACI_Mitigation_CTRL, aci_mitigation_hw_enable, 2);
		}
		break;

	default:
		break;
	}

	switch (hwaci_mode) {
	case HWACI_AUTO_FCBS:
	case HWACI_AUTO_SW:

		PHY_ACI(("HWACI: Enable clocks, preempt \n"));
		if (init) {
			PHY_ACI(("HWACI: AUTO mode\n"));
			/*
			 * From proc do_aci_setup Optimise by
			 * amalgamating writes by using WRITE_PHYREG
			 */
			if (ACPHY_HWACI_28NM(pi)) {
				wlc_phy_enable_hwaci_28nm(pi);
			} else {
				if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_mitigation_sw_enable, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_off, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_on, 5)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_mitigation_sw_enable, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_off, 2)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_on, 2)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
					ACMAJORREV_37(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_mitigation_sw_enable, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_off_1, 2)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_on_1, 2)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_mitigation_sw_enable, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_off, 5)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_on, 5)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
				if (ACMAJORREV_4(pi->pubpi->phy_rev) ||
				    ACMAJORREV_32(pi->pubpi->phy_rev) ||
				    ACMAJORREV_33(pi->pubpi->phy_rev) ||
				    ACMAJORREV_37(pi->pubpi->phy_rev)) {
					WRITE_PHYREG(pi, ACI_Detect_report_ctr_threshold_lo, 0);
				} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
					WRITE_PHYREG(pi, ACI_Detect_report_ctr_threshold_lo, 0);
					MOD_PHYREG(pi, ACI_Detect_CTRL1, aci_detect_w1w2_select, 1);
				} else {
					MOD_PHYREG(pi, ACI_Detect_report_ctr_threshold,
							aci_report_ctr_th, 0);
				}

				if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				    ACMAJORREV_33(pi->pubpi->phy_rev) ||
				    ACMAJORREV_37(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
							aci_detect_collect_interval_1, 15)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
							aci_detect_collect_interval_2, 15)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
							aci_detect_window_size_1, 15)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
							aci_detect_window_size_2, 15)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
							aci_detect_collect_interval_1, 2)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
							aci_detect_collect_interval_2, 2)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_wait_period_1,
							aci_detect_wait_period_1, 1)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_wait_period_2,
							aci_detect_wait_period_2, 0xff)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
							aci_detect_window_size_1, 8)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
							aci_detect_window_size_2, 8)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
								aci_detect_direct_output, 1)
					ACPHY_REG_LIST_EXECUTE(pi);
				}

				if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, ACI_Mitigation_status, aci_T_RSSI_select, 2);
				} else if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, ACI_Mitigation_status, aci_T_RSSI_select, 2);
					MOD_PHYREG(pi, ACI_Detect_CTRL1, aci_detect_w1w2_select, 1);
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_detect_direct_output, 1);
				} else if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
				           ACMAJORREV_37(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, ACI_Mitigation_InputSelect,
						aci_A_select_1, 0);
					MOD_PHYREG(pi, ACI_Mitigation_InputSelect,
						aci_B_select_1, 2);
					MOD_PHYREG(pi, ACI_Detect_CTRL1, aci_detect_w1w2_select, 1);
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_detect_direct_output, 1);
					MOD_PHYREG(pi, ACI_Mitigation_CTRL,
						aci_mitigation_hw_switching_disable, 0);
				} else {
					MOD_PHYREG(pi, ACI_Mitigation_status, aci_T_RSSI_select, 0);
				}
				if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
					ACPHY_REG_LIST_START
						/* 4349 eLNA boards HWACI settings */
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_input_shift, 6)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_block_shift, 4)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config1, 1300)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config2, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config3, 200)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config4, 0x004B)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config5, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config6, 0x0)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_MAX_COUNT_HI,
							aci_detect_max_count_hi, 6)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_off, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
							aci_present_th_mit_on, 5)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
					/* 43012 HWACI settings */
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_input_shift, 6)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_block_shift, 4)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config1, 0x50)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config2, 0xffff)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config3, 0x50)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config4, 0x50)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config5, 0xffff)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config6, 0x50)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_MAX_COUNT_HI,
							aci_detect_max_count_hi, 6)
					ACPHY_REG_LIST_EXECUTE(pi);
				} else if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_pwr_input_shift, 6);
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_pwr_block_shift, 4);
					WRITE_PHYREG(pi, Tiny_ACI_config1, 0x100);
					WRITE_PHYREG(pi, Tiny_ACI_config2, 0x8000);
						WRITE_PHYREG(pi, Tiny_ACI_config3, 0x300);
					WRITE_PHYREG(pi, Tiny_ACI_config4, 0x100);
				} else if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
				    ACMAJORREV_37(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_pwr_input_shift, 6);
					MOD_PHYREG(pi, ACI_Mitigation_status,
						aci_pwr_block_shift, 4);
					ACPHYREG_BCAST(pi, Tiny_ACI_config10, 0x100);
					ACPHYREG_BCAST(pi, Tiny_ACI_config20, 0x8000);
					ACPHYREG_BCAST(pi, Tiny_ACI_config30, 0x300);
					ACPHYREG_BCAST(pi, Tiny_ACI_config40, 0x100);
				} else {
					/* these are for eLNA boards only */
					if (IS_4364_1x1(pi)) {
					/* ACI detection time is modified to 100ms for 4364.
					 0x1e848 * 0.8us = 100ms.
					*/
						WRITE_PHYREG(pi, ACI_Detect_MAX_COUNT_LO, 0xe848);
						WRITE_PHYREG(pi, ACI_Detect_MAX_COUNT_HI, 1);
					}
					ACPHY_REG_LIST_START
						/* these are for eLNA boards only */
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_input_shift, 6)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_block_shift, 4)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config1,
							((CHSPEC_IS2G(pi->radio_chanspec) &&
							(IS_4364_1x1(pi))) ? 0x514 : 0x3e8))
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config2, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config3, 0x00c8)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config4, 0x0064)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config5, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config6, 0x0)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
				if (ACPHY_LO_NF_MODE_ELNA_TINY(pi) &&
				    !(ACMAJORREV_37(pi->pubpi->phy_rev))) {
					ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
							aci_detect_collect_interval_1, 1)
						MOD_PHYREG_ENTRY(pi, ACI_Detect_wait_period_1,
							aci_detect_wait_period_1, 0)
						MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
							aci_pwr_input_shift, 7)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config1, 1000)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config3, 100)
					ACPHY_REG_LIST_EXECUTE(pi);
				}

				if (ACMAJORREV_32(pi->pubpi->phy_rev)) {
					WRITE_PHYREG(pi, Tiny_ACI_config5, 0x8000);
					WRITE_PHYREG(pi, Tiny_ACI_config6, 0x300);
				} else if (ACMAJORREV_33(pi->pubpi->phy_rev) ||
					ACMAJORREV_37(pi->pubpi->phy_rev)) {
					ACPHYREG_BCAST(pi, Tiny_ACI_config50, 0x8000);
					ACPHYREG_BCAST(pi, Tiny_ACI_config60, 0x300);
				} else {
					ACPHY_REG_LIST_START
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config7, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config8, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config9, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config10, 0x0)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config11, 0x0451)
						WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config12, 0x0534)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
			}
		}
		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, ACI_Detect_collect_interval, aci_detect_clkenable, 1);
		} else {
			MOD_PHYREG(pi, ACI_Detect_CTRL, aci_detect_clkenable, 1);
		}
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL, aci_detect_enable, 1)
			MOD_PHYREG_ENTRY(pi, aci_detector_reset, aci_soft_reset, 1)
			MOD_PHYREG_ENTRY(pi, aci_detector_reset, aci_soft_reset, 0)
#ifdef WL_ACI_FCBS
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 0)
#else
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_hwtrig_disable, 1)
#endif // endif
		ACPHY_REG_LIST_EXECUTE(pi);
		break;

	case HWACI_FORCED_MITOFF:
		PHY_ACI(("HWACI: Forced NORMAL mode \n"));
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 0)
			MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL, aci_sel, 0)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 1)
		ACPHY_REG_LIST_EXECUTE(pi);
		break;

	case HWACI_FORCED_MITON:
		PHY_ACI(("HWACI: Forced ACI mode \n"));
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 0)
			MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL, aci_sel, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL, aci_mitigation_sw_enable, 1)
		ACPHY_REG_LIST_EXECUTE(pi);
		break;

	default:
		break;
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev)) {
		pi_ac->noisei->hwaci_states_2g[0].lna1_idx_min = 0;
		pi_ac->noisei->hwaci_states_2g[0].lna1_idx_max = ACPHY_4365_MAX_LNA1_IDX;
		pi_ac->noisei->hwaci_states_2g[0].lna2_idx_min = ACPHY_4365_MAX_LNA2_IDX;
		pi_ac->noisei->hwaci_states_2g[0].lna2_idx_max = ACPHY_4365_MAX_LNA2_IDX;
		pi_ac->noisei->hwaci_states_2g[0].mix_idx_min = 0;
		pi_ac->noisei->hwaci_states_2g[0].mix_idx_max = 7;
		pi_ac->noisei->hwaci_states_2g[1].lna1_idx_min = 0;
		pi_ac->noisei->hwaci_states_2g[1].lna1_idx_max = 4;
		pi_ac->noisei->hwaci_states_2g[1].lna2_idx_min = 0;
		pi_ac->noisei->hwaci_states_2g[1].lna2_idx_max = 1;
		pi_ac->noisei->hwaci_states_2g[1].mix_idx_min = 7;
		pi_ac->noisei->hwaci_states_2g[1].mix_idx_max = 7;

		pi_ac->noisei->hwaci_states_5g[0].lna1_idx_min = 0;
		pi_ac->noisei->hwaci_states_5g[0].lna1_idx_max = ACPHY_4365_MAX_LNA1_IDX;
		pi_ac->noisei->hwaci_states_5g[0].lna2_idx_min = ACPHY_4365_MAX_LNA2_IDX;
		pi_ac->noisei->hwaci_states_5g[0].lna2_idx_max = ACPHY_4365_MAX_LNA2_IDX;
		pi_ac->noisei->hwaci_states_5g[0].mix_idx_min = 0;
		pi_ac->noisei->hwaci_states_5g[0].mix_idx_max = 7;
		pi_ac->noisei->hwaci_states_5g[1].lna1_idx_min = 0;
		pi_ac->noisei->hwaci_states_5g[1].lna1_idx_max = 4;
		pi_ac->noisei->hwaci_states_5g[1].lna2_idx_min = 0;
		pi_ac->noisei->hwaci_states_5g[1].lna2_idx_max = 1;
		pi_ac->noisei->hwaci_states_5g[1].mix_idx_min = 7;
		pi_ac->noisei->hwaci_states_5g[1].mix_idx_max = 7;

		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
			pi_ac->noisei->hwaci_states_5g_40_80[0].lna1_idx_min = 0;
			pi_ac->noisei->hwaci_states_5g_40_80[0].lna1_idx_max =
				ACPHY_4365_MAX_LNA1_IDX;
			pi_ac->noisei->hwaci_states_5g_40_80[0].lna2_idx_min =
				ACPHY_4365_MAX_LNA2_IDX;
			pi_ac->noisei->hwaci_states_5g_40_80[0].lna2_idx_max =
				ACPHY_4365_MAX_LNA2_IDX;
			pi_ac->noisei->hwaci_states_5g_40_80[0].mix_idx_min = 0;
			pi_ac->noisei->hwaci_states_5g_40_80[0].mix_idx_max = 7;
			pi_ac->noisei->hwaci_states_5g_40_80[1].lna1_idx_min = 0;
			pi_ac->noisei->hwaci_states_5g_40_80[1].lna1_idx_max = 4;
			pi_ac->noisei->hwaci_states_5g_40_80[1].lna2_idx_min = 0;
			pi_ac->noisei->hwaci_states_5g_40_80[1].lna2_idx_max = 1;
			pi_ac->noisei->hwaci_states_5g_40_80[1].mix_idx_min = 4;
			pi_ac->noisei->hwaci_states_5g_40_80[1].mix_idx_max = 7;
		}
	}

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		pi_ac->noisei->hwaci_states_2g[0].lna1_pktg_lmt = ACPHY_28nm_MAX_LNA1_IDX;
		pi_ac->noisei->hwaci_states_2g[1].lna1_pktg_lmt = 3;
		pi_ac->noisei->hwaci_states_2g[2].lna1_pktg_lmt = 2;

		pi_ac->noisei->hwaci_states_5g[0].lna1_pktg_lmt = ACPHY_28nm_MAX_LNA1_IDX;
		pi_ac->noisei->hwaci_states_5g[1].lna1_pktg_lmt = 3;
		pi_ac->noisei->hwaci_states_5g[2].lna1_pktg_lmt = 2;
	}

}

void
wlc_phy_enable_hwaci_28nm(phy_info_t *pi)
{
	uint16 hostflag;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint8 core, band = CHSPEC_IS2G(pi->radio_chanspec) ? 2 : 5;

	// Set shmem bit-3 to enable HWACI interrupt in uCode
	if (pi_ac->sromi->hwaci_sw_mitigation == 1) {
		hostflag = wlapi_bmac_read_shm(pi->sh->physhim,
			M_HOST_FLAGS6(pi));
		hostflag = hostflag | HWACI_SET_SW_MITIGATION_MODE;
		wlapi_bmac_write_shm(pi->sh->physhim,
			M_HOST_FLAGS6(pi), hostflag);
	}

	ACPHY_REG_LIST_START
		MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
			aci_detect_enable, 1)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
			aci_detect_clkenable, 1)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL1,
			aci_present_select_mux_aux, 0)
		MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
			aci_mitigation_hw_enable, 2)
		MOD_PHYREG_ENTRY(pi, ACI_MITIGATION_EXTRA_CTRL,
			aci_mitigation_state_muxsel, 0)
		MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
			aci_mitigation_hwtrig_disable, 1)
		MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
			aci_detect_direct_output, 0)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
			aci_detect_window_size_1, 8)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL,
			aci_detect_window_size_2, 8)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
			aci_detect_collect_interval_1, 2)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_collect_interval,
			aci_detect_collect_interval_2, 2)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_wait_period_1,
			aci_detect_wait_period_1, 1)
		MOD_PHYREG_ENTRY(pi, ACI_Detect_wait_period_2,
			aci_detect_wait_period_2, 1)
		// ACI detection time: 0x1e848 * 0.8us = 100ms.
		WRITE_PHYREG_ENTRY(pi, ACI_Detect_MAX_COUNT_LO, 0xe848)
		WRITE_PHYREG_ENTRY(pi, ACI_Detect_MAX_COUNT_HI, 1)
		MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
			aci_pwr_input_shift, 6)
		MOD_PHYREG_ENTRY(pi, ACI_Mitigation_status,
			aci_pwr_block_shift, 3)
	ACPHY_REG_LIST_EXECUTE(pi);

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		ACPHY_REG_LIST_START
			// 0 for cond1, 1 for cond2, 2 for cond1|cond2  3 for cond1&cond2
			MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL1,
				aci_present_select, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_mitigation_sw_enable, 1)
			// mitigation is not brought up yet, use normal gain table
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_mitigation_hw_switching_disable, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_present_th_mit_on_1, 5)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_present_th_mit_off_1, 5)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_A_select_1, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_B_select_1, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_A_select_2, 0)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_B_select_2, 2)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_present_th_mit_on_2, 5)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_present_th_mit_off_2, 5)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_BT_CTRL,
				aci_detect_collect_disable_on_bt_priority, 0)
		ACPHY_REG_LIST_EXECUTE(pi);
	} else {
		ACPHY_REG_LIST_START
			MOD_PHYREG_ENTRY(pi, ACI_Detect_CTRL1,
				aci_present_select, 2)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_mitigation_sw_enable, 0)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_mitigation_hw_switching_disable, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_present_th_mit_on_1, 4)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_CTRL,
				aci_present_th_mit_off_1, 4)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_A_select_1, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_A_select_2, 1)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_B_select_1, 2)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_B_select_2, 2)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_present_th_mit_on_2, 4)
			MOD_PHYREG_ENTRY(pi, ACI_Mitigation_InputSelect,
				aci_present_th_mit_off_2, 4)
		ACPHY_REG_LIST_EXECUTE(pi);
	}

	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		FOREACH_CORE(pi, core) {
			/* 1st Detector - disbaled for now */
			/* Thresholds */
			WRITE_PHYREGCE(pi, Tiny_ACI_config1, core, HWACI_DET_TH1_0);
			WRITE_PHYREGCE(pi, Tiny_ACI_config2, core, HWACI_DET_TH1_1);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config3, core, HWACI_DET_TH1_2);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config4, core, HWACI_DET_TH1_0);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config5, core, HWACI_DET_TH1_1);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config6, core, HWACI_DET_TH1_2);

			/* Shifts */
		    WRITE_PHYREGCE(pi, Tiny_ACI_config13, core, HWACI_A_SHIFT1);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config14, core, HWACI_A_SHIFT1);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config15, core, HWACI_B_SHIFT1);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config16, core, HWACI_B_SHIFT1);

			/* Signs */
		    WRITE_PHYREGCE(pi, Tiny_ACI_config17, core, HWACI_A_SIGN1);
			WRITE_PHYREGCE(pi, Tiny_ACI_config18, core, HWACI_B_SIGN1);

			/* 2nd Detector : W1 - (ADC/2) */
			/* Thresholds */
			WRITE_PHYREGCE(pi, Tiny_ACI_config7, core, (band == 2) ?
			               HWACI_DET_TH2_0_2G_REV47 : HWACI_DET_TH2_0_5G_REV47);
			WRITE_PHYREGCE(pi, Tiny_ACI_config8, core, (band == 2) ?
			               HWACI_DET_TH2_1_2G_REV47 : HWACI_DET_TH2_1_5G_REV47);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config9, core, (band == 2) ?
		               HWACI_DET_TH2_2_2G_REV47 : HWACI_DET_TH2_2_5G_REV47);
			WRITE_PHYREGCE(pi, Tiny_ACI_config10, core, (band == 2) ?
			               HWACI_DET_TH2_0_2G_REV47 : HWACI_DET_TH2_0_5G_REV47);
			WRITE_PHYREGCE(pi, Tiny_ACI_config11, core, (band == 2) ?
			               HWACI_DET_TH2_1_2G_REV47 : HWACI_DET_TH2_1_5G_REV47);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config12, core, (band == 2) ?
		               HWACI_DET_TH2_2_2G_REV47 : HWACI_DET_TH2_2_5G_REV47);

			/* Shifts */
		    WRITE_PHYREGCE(pi, Tiny_ACI_config19, core, (band == 2) ?
		               HWACI_A_SHIFT2_2G_REV47 : HWACI_A_SHIFT2_5G_REV47);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config20, core, (band == 2) ?
		               HWACI_A_SHIFT2_2G_REV47 : HWACI_A_SHIFT2_5G_REV47);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config21, core, (band == 2) ?
		               HWACI_B_SHIFT2_2G_REV47 : HWACI_B_SHIFT2_5G_REV47);
		    WRITE_PHYREGCE(pi, Tiny_ACI_config22, core, (band == 2) ?
		               HWACI_B_SHIFT2_2G_REV47 : HWACI_B_SHIFT2_5G_REV47);

			/* Signs */
		    WRITE_PHYREGCE(pi, Tiny_ACI_config23, core, HWACI_A_SIGN2_REV47);
			WRITE_PHYREGCE(pi, Tiny_ACI_config24, core, HWACI_B_SIGN2_REV47);

		}
	} else {
		if (ACMAJORREV_40(pi->pubpi->phy_rev) &&
		    (RADIOREV(pi->pubpi->radiorev) == 8 ||
		     RADIOREV_AUX(pi->pubpi->radiorev) == 9)) {
			ACPHY_REG_LIST_START
			        // Thresholds for 1st condition
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config10, HWACI_DET_TH1_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config20, HWACI_DET_TH2_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config30, HWACI_DET_TH3_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config40, HWACI_DET_TH1_ACI_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config50, HWACI_DET_TH2_ACI_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config60, HWACI_DET_TH3_ACI_SET2)

			        // Thresholds for 2nd condition
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config70, HWACI_DET_TH1_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config80, HWACI_DET_TH2_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config90, HWACI_DET_TH3_NOR_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config100, HWACI_DET_TH1_ACI_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config110, HWACI_DET_TH2_ACI_SET2)
			        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config120, HWACI_DET_TH3_ACI_SET2)
			ACPHY_REG_LIST_EXECUTE(pi);
		}

		ACPHY_REG_LIST_START
		        // Shifts for 1st condition
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config130, 0x0300)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config140, 0x0300)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config150, 0x0)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config160, 0x0)
		        // Signs for 1st condition
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config170, 0x0451)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config180, 0x0d34)

		        // Shifts for 2nd condition
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config190, 0x0300)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config200, 0x0300)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config210, 0x0)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config220, 0x0)
		        // Signs for 2nd condition
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config230, 0x0451)
		        WRITE_PHYREG_ENTRY(pi, Tiny_ACI_config240, 0x0d34)
		ACPHY_REG_LIST_EXECUTE(pi);
	}
}

#ifdef WL_ACI_FCBS

/*  From proc init_fastcsTbl_for_aci_detector {} { */
static void
wlc_phy_init_FCBS_hwaci(phy_info_t *pi)
{

/*
 * Initialise FCBS table with ACI mitigation settings.
 * These are generated by running AGC config code
 * and then copying values to FCBS.
 *
 */

	uint16 val;
	uint16 *ptr_val;
	int i;

	uint16 fastchswTableIndex, startoffset;

	uint16 *reg_list_ptr;
	hwaci_fcbs_phytbl_list_entry  *tbl_list_ptr;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;

	const uint16 fcbs_table_delim = 0xffff;
	uint16 tia_cfg12_wrssi3_ref_high_sel;
	uint16 tia_cfg12_wrssi3_ref_mid_sel;
	uint16 tia_cfg13_wrssi3_ref_low_sel;
	acphy_desense_values_t curr_desense;

	ASSERT(ACPHY_ENABLE_FCBS_HWACI(pi));

	ptr_val = &val;

	fastchswTableIndex = startoffset =
		READ_PHYREGFLD(pi, ACI_Mitigation_CTRL1, ACIMitigation_iniraddr);

	curr_desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, CURR_DESENSE);
	curr_desense.analog_gain_desense_ofdm = 0;
	curr_desense.analog_gain_desense_bphy = 0;
	curr_desense.lna1_tbl_desense = 0;
	curr_desense.clipgain_desense[0] = 0;
	curr_desense.clipgain_desense[1] = 0;
	curr_desense.clipgain_desense[2] = 0;
	curr_desense.clipgain_desense[3] = 0;
	phy_ac_rxgcrs_set_desense(pi_ac->rxgcrsi, &curr_desense, CURR_DESENSE);

	wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(pi, 0, ACPHY_TBL_ID_GAIN0,
		ACPHY_TBL_ID_GAINBITS0);
	wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi, 0);

/* First write the Normal values, reg addresses, tbl id and delimiters */
	for (reg_list_ptr = hwaci_fcbs_rfregs; *reg_list_ptr != 0xFFFF;  reg_list_ptr++) {
		val = phy_utils_read_radioreg(pi, *reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
			16, reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+1,
			16, ptr_val);
		fastchswTableIndex += 3;
	}

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
		16, &fcbs_table_delim);
	fastchswTableIndex += 1;

	for (reg_list_ptr = hwaci_fcbs_phyregs; *reg_list_ptr != 0xFFFF;  reg_list_ptr++) {
		val = phy_utils_read_phyreg(pi, *reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
			16, reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+1,
			16, ptr_val);
		fastchswTableIndex += 3;
	}

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
		16, &fcbs_table_delim);
	fastchswTableIndex += 1;

	for (tbl_list_ptr = hwaci_fcbs_phytbls; tbl_list_ptr->tbl_id != 0xFFFF;  tbl_list_ptr++) {
		uint16 tbl_id = tbl_list_ptr->tbl_id | FCBS_1DATA_PER_ADDR;

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
			16, &tbl_id);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+1,
			16, &tbl_list_ptr->tbl_offset);
		/* calc end offset */
		val = tbl_list_ptr->tbl_offset + tbl_list_ptr->num_entries - 1;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+2,
			16, ptr_val);

		fastchswTableIndex += 3; /* point to first normal entry */
		for (i = 0; i < tbl_list_ptr->num_entries;  i++) {
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id, 1,
				tbl_list_ptr->tbl_offset + i, 16, ptr_val);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1,
				fastchswTableIndex, 16, ptr_val);
/* set to next normal entry OR table id  OR delimiter */
			fastchswTableIndex += 2;
		}
	}

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
		16, &fcbs_table_delim);

/* Write the ACI values */

	fastchswTableIndex = startoffset;

	/* save TIA wrssi3 ref values */
	tia_cfg12_wrssi3_ref_high_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                        wrssi3_ref_high_sel);
	tia_cfg12_wrssi3_ref_mid_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG12, 0,
	                                                       wrssi3_ref_mid_sel);
	tia_cfg13_wrssi3_ref_low_sel = READ_RADIO_REGFLD_20691(pi, TIA_CFG13, 0,
	                                                       wrssi3_ref_low_sel);

	curr_desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, CURR_DESENSE);
	curr_desense.analog_gain_desense_ofdm = HWACI_OFDM_DESENSE;
	curr_desense.analog_gain_desense_bphy = HWACI_BPHY_DESENSE;
	curr_desense.lna1_tbl_desense = HWACI_LNA1_DESENSE;
	curr_desense.clipgain_desense[0] = HWACI_CLIP_INIT_DESENSE;
	curr_desense.clipgain_desense[1] = HWACI_CLIP_HIGH_DESENSE;
	curr_desense.clipgain_desense[2] = HWACI_CLIP_MED_DESENSE;
	curr_desense.clipgain_desense[3] = HWACI_CLIP_LO_DESENSE;
	phy_ac_rxgcrs_set_desense(pi_ac->rxgcrsi, &curr_desense, CURR_DESENSE);

	wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(pi, 0, ACPHY_TBL_ID_GAIN0,
		ACPHY_TBL_ID_GAINBITS0);
	wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi, 0);

	RADIO_REG_LIST_START
		MOD_RADIO_REG_20691_ENTRY(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, 0)
		MOD_RADIO_REG_20691_ENTRY(pi, TIA_CFG12, 0, wrssi3_ref_mid_sel, 0)
		MOD_RADIO_REG_20691_ENTRY(pi, TIA_CFG13, 0, wrssi3_ref_low_sel, 0)
	RADIO_REG_LIST_EXECUTE(pi, 0);

	for (reg_list_ptr = hwaci_fcbs_rfregs; *reg_list_ptr != 0xFFFF;  reg_list_ptr++) {
		val = phy_utils_read_radioreg(pi, *reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+2,
			16, ptr_val);
		fastchswTableIndex += 3;
	}

	fastchswTableIndex += 1; /* skip delimiter */

	for (reg_list_ptr = hwaci_fcbs_phyregs; *reg_list_ptr != 0xFFFF;  reg_list_ptr++) {
		val = phy_utils_read_phyreg(pi, *reg_list_ptr);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex+2,
			16, ptr_val);
		fastchswTableIndex += 3;
	}

	fastchswTableIndex += 1;  /* skip delimiter */

	for (tbl_list_ptr = hwaci_fcbs_phytbls; tbl_list_ptr->tbl_id != 0xFFFF;  tbl_list_ptr++) {
		fastchswTableIndex += 4;   /* point to first aci entry */
		for (i = 0; i < tbl_list_ptr->num_entries;  i++) {
			wlc_phy_table_read_acphy(pi, tbl_list_ptr->tbl_id,  1,
				tbl_list_ptr->tbl_offset + i, 16, ptr_val);
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1,
				fastchswTableIndex,	16, ptr_val);
			/* point to next aci entry, next table, or delimiter */
			fastchswTableIndex += 2;
		}
		fastchswTableIndex -= 1;  /* back up one to point to Table ID entry */
	}

	fastchswTableIndex += 1; /* skip delimiter */
	while ((fastchswTableIndex) < 800) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_FASTCHSWITCH, 1, fastchswTableIndex,
			16, &fcbs_table_delim);
		fastchswTableIndex += 1;
	}

	curr_desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, CURR_DESENSE);
	curr_desense.analog_gain_desense_ofdm = 0;
	curr_desense.analog_gain_desense_bphy = 0;
	curr_desense.lna1_tbl_desense = 0;
	curr_desense.clipgain_desense[0] = 0;
	curr_desense.clipgain_desense[1] = 0;
	curr_desense.clipgain_desense[2] = 0;
	curr_desense.clipgain_desense[3] = 0;
	phy_ac_rxgcrs_set_desense(pi_ac->rxgcrsi, &curr_desense, CURR_DESENSE);

	/* restore TIA wrssi3 ref values */
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_high_sel, tia_cfg12_wrssi3_ref_high_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG12, 0, wrssi3_ref_mid_sel, tia_cfg12_wrssi3_ref_mid_sel);
	MOD_RADIO_REG_20691(pi, TIA_CFG13, 0, wrssi3_ref_low_sel, tia_cfg13_wrssi3_ref_low_sel);

	wlc_phy_rxgainctrl_set_gaintbls_acphy_tiny(pi, 0, ACPHY_TBL_ID_GAIN0,
		ACPHY_TBL_ID_GAINBITS0);
	wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi, 0);

}
#endif /* WL_ACI_FCBS */
#endif /* WLC_DISABLE_ACI */

/* Update chan stats offline, i.e. we might not be on this channel currently */

static acphy_aci_params_t*
wlc_phy_desense_aci_getset_chanidx_acphy(phy_info_t *pi, chanspec_t chanspec, bool create)
{
	uint8 idx, oldest_idx;
	uint64 oldest_time;
	acphy_aci_params_t *ret = NULL;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_aci_params_t *aci_list;

	aci_list = CHSPEC_IS2G(chanspec) ? pi_ac->noisei->aci_list2g : pi_ac->noisei->aci_list5g;

	/* Find if this chan/bw already exists */
	for (idx = 0; idx < ACPHY_ACI_CHAN_LIST_SZ; idx++) {
		if ((aci_list[idx].chan == CHSPEC_CHANNEL(chanspec)) &&
		    (aci_list[idx].bw == CHSPEC_BW(chanspec))) {
			ret = &aci_list[idx];
			PHY_ACI(("aci_debug. *** old_chan. idx = %d, chan = %d, bw = %d\n",
			         idx, ret->chan, ret->bw));
			break;
		}
	}

	/* If doesn't exist & don't want to create one */
	if ((ret == NULL) && !create) return ret;

	if (ret == NULL) {
		/* Chan/BW does not exist on in the list of ACI channels.
		   Create a new one (based on oldest timestamp)
		*/
		oldest_idx = 0; oldest_time = aci_list[oldest_idx].last_updated;
		for (idx = 1; idx < ACPHY_ACI_CHAN_LIST_SZ; idx++) {
			if (aci_list[idx].last_updated < oldest_time) {
				oldest_time = aci_list[idx].last_updated;
				oldest_idx = idx;
			}
		}

		/* Clear the new aciinfo data */
		ret = &aci_list[oldest_idx];
		bzero(ret, sizeof(acphy_aci_params_t));
		ret->chan =  CHSPEC_CHANNEL(pi->radio_chanspec);
		ret->bw = pi->bw;
		ret->glitch_upd_wait = 2;
		PHY_ACI(("aci_debug, *** new_chan = %d %d, idx = %d\n",
		         CHSPEC_CHANNEL(pi->radio_chanspec), pi->bw, oldest_idx));
	}

	/* Only if the request came for creation */
	if (create) {
		ret->last_updated = phy_utils_get_time_usec(pi);
	}

	return ret;
}

void
wlc_phy_desense_aci_engine_acphy(phy_info_t *pi)
{

	uint8 ma_idx, badplcp_idx, i, glitch_idx;
	bool call_mitigation = FALSE;
	uint32 ofdm_glitches, bphy_glitches;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	uint32 avg_glitch_ofdm, avg_glitch_bphy;
	uint8 new_bphy_desense, new_ofdm_desense, new_ofdm_desense_extra_halfdB = 0;
	acphy_desense_values_t *desense;
	acphy_desense_values_t zero_desense;
	acphy_aci_params_t *aci;
	bool enable_half_dB, jammer_present = FALSE;
	uint8 max_rssi, zero = 0;
	/* use tiny threshold for 43684 also */
	bool is_tiny = ACMAJORREV_4(pi->pubpi->phy_rev) || ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_47(pi->pubpi->phy_rev);
	bool lesi_cap = phy_ac_rxgcrs_get_cap_lesi(pi);
	uint8 max_lesi_desense = lesi_cap ? ACPHY_ACI_MAX_LESI_DESENSE_DB : 0;

	if (wlc_phy_is_scan_chan_acphy(pi)) return;

	if (pi_ac->noisei->aci == NULL) {
		pi_ac->noisei->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
			pi->radio_chanspec, TRUE);
	}

	aci = pi_ac->noisei->aci;
	desense = &aci->desense;

	if (aci->glitch_upd_wait > 0) {
		aci->glitch_upd_wait--;
		return;
	}

	/* bphy glitches/badplcp */
	ma_idx = (pi->interf->noise.bphy_ma_index == 0) ? PHY_NOISE_MA_WINDOW_SZ - 1 :
	        pi->interf->noise.bphy_ma_index - 1;
	badplcp_idx = (pi->interf->noise.bphy_badplcp_ma_index == 0) ? PHY_NOISE_MA_WINDOW_SZ - 1 :
	        pi->interf->noise.bphy_badplcp_ma_index - 1;
	bphy_glitches =  pi->interf->noise.bphy_glitch_ma_list[ma_idx] +
	        (2 * pi->interf->noise.bphy_badplcp_ma_list[badplcp_idx]);
	PHY_ACI(("aci_mode1, bphy(glitch, badplcp) = %d %d \n",
	         pi->interf->noise.bphy_glitch_ma_list[ma_idx],
	         pi->interf->noise.bphy_badplcp_ma_list[badplcp_idx]));

	/* Ofdm glitches/badplcp */
	ma_idx = (pi->interf->noise.ofdm_ma_index == 0) ? PHY_NOISE_MA_WINDOW_SZ - 1 :
	        pi->interf->noise.ofdm_ma_index - 1;
	badplcp_idx = (pi->interf->noise.ofdm_badplcp_ma_index == 0) ? PHY_NOISE_MA_WINDOW_SZ - 1 :
	        pi->interf->noise.ofdm_badplcp_ma_index - 1;
	ofdm_glitches =  pi->interf->noise.ofdm_glitch_ma_list[ma_idx] +
	        (2 * pi->interf->noise.ofdm_badplcp_ma_list[badplcp_idx]);
	PHY_ACI(("aci_mode1, ofdm(glitch, badplcp) = %d %d \n",
	         pi->interf->noise.ofdm_glitch_ma_list[ma_idx],
	         pi->interf->noise.ofdm_badplcp_ma_list[badplcp_idx]));

	/* Update glitch history */
	glitch_idx = aci->glitch_buff_idx;
	aci->ofdm_hist.glitches[glitch_idx] = ofdm_glitches;
	aci->bphy_hist.glitches[glitch_idx] = bphy_glitches;
	aci->glitch_buff_idx = (glitch_idx + 1) % ACPHY_ACI_GLITCH_BUFFER_SZ;

	PHY_ACI(("aci_mode1, bphy idx = %d, glitch + (2 * badplcp) = ", glitch_idx));
	for (i = 0; i < ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
		PHY_ACI(("%d ", aci->bphy_hist.glitches[i]));
	PHY_ACI(("\n"));

	PHY_ACI(("aci_mode1, ofdm idx = %d, glitch + (2 * badplcp) = ", glitch_idx));
	for (i = 0; i < ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
		PHY_ACI(("%d ", aci->ofdm_hist.glitches[i]));
	PHY_ACI(("\n"));

	/* Find AVG of Max glitches in last N seconds */
	avg_glitch_bphy =
	        wlc_phy_desense_aci_get_avg_max_glitches_acphy(pi, aci->bphy_hist.glitches);
	avg_glitch_ofdm =
	        wlc_phy_desense_aci_get_avg_max_glitches_acphy(pi, aci->ofdm_hist.glitches);

	PHY_ACI(("aci_mode1, max {bphy, ofdm} = {%d %d}, rssi = %d, aci_on = %d\n",
	         avg_glitch_bphy, avg_glitch_ofdm, aci->weakest_rssi, desense->on));

	/* **** JAMMER DETECTION (0 reserved for no txop info) *** */
	if ((aci->txop > 0) && (aci->txop < 6) && (avg_glitch_ofdm > 10000)) {
		jammer_present = TRUE;
		aci->jammer_cnt = ACPHY_JAMMER_SLEEP;
	} else if (aci->jammer_cnt > 0) {
		jammer_present = TRUE;
		aci->jammer_cnt = (avg_glitch_ofdm > ACPHY_ACI_OFDM_HI_GLITCH_THRESH) ?
		    ACPHY_JAMMER_SLEEP : aci->jammer_cnt - 1;
	}

	/* Don't need to do anything is interference mitigation is off & glitches < thresh */
	/* Using different MACROS for 4349
	 */
	if (is_tiny) {
		if (!(desense->on || (avg_glitch_bphy > ACPHY_ACI_BPHY_HI_GLITCH_THRESH) ||
			(avg_glitch_ofdm > ACPHY_ACI_OFDM_HI_GLITCH_THRESH_TINY)))
			return;
	} else {
		if (!(desense->on || (avg_glitch_bphy > ACPHY_ACI_BPHY_HI_GLITCH_THRESH) ||
			(avg_glitch_ofdm > ACPHY_ACI_OFDM_HI_GLITCH_THRESH)))
			return;
	}

	new_bphy_desense = wlc_phy_desense_aci_calc_acphy(pi, &aci->bphy_hist,
	                                                  desense->bphy_desense,
	                                                  avg_glitch_bphy,
	                                                  ACPHY_ACI_BPHY_LO_GLITCH_THRESH,
	                                                  ACPHY_ACI_BPHY_HI_GLITCH_THRESH,
	                                                  FALSE, &zero);

	new_ofdm_desense_extra_halfdB = desense->ofdm_desense_extra_halfdB;
	enable_half_dB = (jammer_present ||
	                  (lesi_cap && (desense->ofdm_desense <= max_lesi_desense)));
	if (is_tiny) {
		new_ofdm_desense = wlc_phy_desense_aci_calc_acphy(pi, &aci->ofdm_hist,
			desense->ofdm_desense,
			avg_glitch_ofdm,
			ACPHY_ACI_OFDM_LO_GLITCH_THRESH_TINY,
			ACPHY_ACI_OFDM_HI_GLITCH_THRESH_TINY,
			enable_half_dB,
			&new_ofdm_desense_extra_halfdB);
	} else {
		new_ofdm_desense = wlc_phy_desense_aci_calc_acphy(pi, &aci->ofdm_hist,
			desense->ofdm_desense,
			avg_glitch_ofdm,
			ACPHY_ACI_OFDM_LO_GLITCH_THRESH,
			ACPHY_ACI_OFDM_HI_GLITCH_THRESH,
			enable_half_dB,
			&new_ofdm_desense_extra_halfdB);
	}

	/* Limit desnese */
	new_bphy_desense = MIN(new_bphy_desense, ACPHY_ACI_MAX_DESENSE_BPHY_DB);
	new_ofdm_desense = MIN(new_ofdm_desense, ACPHY_ACI_MAX_DESENSE_OFDM_DB);

	/* BPHY */
	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi))
		/* if no-assoc, then reduce gracefully */
		new_bphy_desense = MIN(new_bphy_desense, MAX(0, desense->bphy_desense - 1));
	else
		new_bphy_desense = MIN(new_bphy_desense, MAX(0, aci->weakest_rssi + 85));

	/* OFDM */
	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) {
		/* if no-assoc & no jammer, then reduce gracefully until max_lesi,
		   but if jammer present, don't limit
		*/
		if (!jammer_present)
			new_ofdm_desense = MIN(new_ofdm_desense,
			                       MAX(max_lesi_desense, desense->ofdm_desense - 1));
	} else {
		/* more aggressive if jammer present (too many glitches) */
		max_rssi = jammer_present ? 90 : 80;
		/* Always allow LESI to desense (upto lesi off), irrespective of RSSI  */
		new_ofdm_desense = MIN(new_ofdm_desense, max_lesi_desense +
		                       MAX(0, aci->weakest_rssi + max_rssi));
	}

	PHY_ACI(("aci_mode1, old desense = {%d %d}, new = {%d %d}\n",
	         desense->bphy_desense,
	         desense->ofdm_desense,
	         new_bphy_desense, new_ofdm_desense));

	if (new_bphy_desense != desense->bphy_desense) {
		call_mitigation = TRUE;
		desense->bphy_desense = new_bphy_desense;

		/* Clear old glitch history when desnese changed */
		for (i = 0; i <  ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
			aci->bphy_hist.glitches[i] = ACPHY_ACI_BPHY_LO_GLITCH_THRESH;
	}

	if ((new_ofdm_desense != desense->ofdm_desense) ||
		(new_ofdm_desense_extra_halfdB != desense->ofdm_desense_extra_halfdB)) {
		call_mitigation = TRUE;
		desense->ofdm_desense = new_ofdm_desense;
		desense->ofdm_desense_extra_halfdB = new_ofdm_desense_extra_halfdB;

		/* Clear old glitch history when desnese changed */
		if (is_tiny) {
			for (i = 0; i <  ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
				aci->ofdm_hist.glitches[i] = ACPHY_ACI_OFDM_LO_GLITCH_THRESH_TINY;
		} else {
			for (i = 0; i <  ACPHY_ACI_GLITCH_BUFFER_SZ; i++)
				aci->ofdm_hist.glitches[i] = ACPHY_ACI_OFDM_LO_GLITCH_THRESH;
		}
	}

	desense->on = FALSE;
	zero_desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, ZERO_DESENSE);
	desense->on = (memcmp(&zero_desense, desense, sizeof(acphy_desense_values_t)) != 0);

	if (call_mitigation) {
		PHY_ACI(("aci_mode1 : desense = %d %d\n",
		         desense->bphy_desense, desense->ofdm_desense));
		wlc_phy_desense_apply_acphy(pi, TRUE);

		/* After gain change, it takes a while for updated glitches to show up */
		aci->glitch_upd_wait = ACPHY_ACI_WAIT_POST_MITIGATION;
	}
}

#endif /* WLC_DISABLE_ACI */

#ifndef WL_ACI_FCBS
static void
phy_ac_noise_aci_mitigate(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	info->data->hw_aci_status = wlapi_bmac_read_shm(pi->sh->physhim, M_HWACI_STATUS(pi)) & 1;

#ifndef WLC_DISABLE_ACI
	if ((ACMAJORREV_5(pi->pubpi->phy_rev) &&
		(phy_ac_btcx_get_data(info->ac_info->btcxi)->btc_mode == 0)) ||
		!ACMAJORREV_5(pi->pubpi->phy_rev)) {
		if ((info->data->gain_idx_forced == 0xffff) &&
			(info->hwaci_desense_state_ovr == -1)) {
			if (info->aci == NULL) {
				info->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
					pi->radio_chanspec, TRUE);
			}
			wlc_phy_hwaci_mitigate_acphy(pi, info->data->hw_aci_status);
		}
	}

	/*
	 * Currently this feature is enabled in ucode for CoreRevIds 47 and 48. In the
	 * driver it is functional only for MajorRev3 (4345b1) and not for 4350c2
	 * (Phyrev 14).
	 */
	if ((ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
		ACMAJORREV_36(pi->pubpi->phy_rev) || ACPHY_HWACI_28NM(pi)) &&
		(pi->sh->interference_mode & ACPHY_HWACI_MITIGATION)) {
		if (info->aci == NULL) {
			info->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
				pi->radio_chanspec, TRUE);
		}

		if (info->data->hw_aci_status == 0) {
			info->aci->desense.analog_gain_desense_ofdm = 0;
			info->aci->desense.analog_gain_desense_bphy = 0;
			info->aci->desense.lna1_tbl_desense = 0;
			info->aci->desense.lna2_tbl_desense = 0;
			info->aci->desense.clipgain_desense[0] = 0;
			info->aci->desense.clipgain_desense[1] = 0;
			info->aci->desense.clipgain_desense[2] = 0;
			info->aci->desense.clipgain_desense[3] = 0;
			wlc_phy_desense_apply_acphy(pi, TRUE);
		} else {
			if (ACPHY_HWACI_28NM(pi)) {
				info->aci->desense.analog_gain_desense_ofdm = 0;
				info->aci->desense.analog_gain_desense_bphy = 0;
			} else {
				info->aci->desense.analog_gain_desense_ofdm =
					HWACI_OFDM_DESENSE;
				info->aci->desense.analog_gain_desense_bphy =
					HWACI_BPHY_DESENSE;
			}
			info->aci->desense.lna1_tbl_desense = HWACI_LNA1_DESENSE;
			if (PHY_ILNA(pi))
				info->aci->desense.lna2_tbl_desense = HWACI_LNA2_DESENSE;
			else
				info->aci->desense.lna2_tbl_desense = 0;
			if (ACPHY_HWACI_28NM(pi))
				info->aci->desense.clipgain_desense[0] = 0;
			else
				info->aci->desense.clipgain_desense[0] =
					HWACI_CLIP_INIT_DESENSE;
			info->aci->desense.clipgain_desense[1] = HWACI_CLIP_HIGH_DESENSE;
			info->aci->desense.clipgain_desense[2] = HWACI_CLIP_MED_DESENSE;
			info->aci->desense.clipgain_desense[3] = HWACI_CLIP_LO_DESENSE;
			wlc_phy_desense_apply_acphy(pi, TRUE);
		}
	}
#endif /* WLC_DISABLE_ACI */
}
#endif /* !WL_ACI_FCBS */

#define PHY_NOISE_MAX_IDLETIME		30

static void
phy_ac_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	bool wait_for_intr = TRUE;

	PHY_NONE(("phy_ac_noise_request_sample: state %d reason %d, channel %d\n",
		pi->phynoise_state, reason, ch));

	/* This is needed to make sure that the crsmin cal happens
	*   even if sampling is in progress
	*/
	noise_info->data->trigger_crsmin_cal = FALSE;
	if (reason == PHY_NOISE_STATE_CRSMINCAL && sampling_in_progress) {
		noise_info->data->trigger_crsmin_cal = TRUE;
	}

	/* since polling is atomic, sampling_in_progress equals to interrupt sampling ongoing
	 *  In these collision cases, always yield and wait interrupt to finish, where the results
	 *  maybe be sharable if channel matches in common callback progressing.
	 */
	if (sampling_in_progress) {
		return;
	}

	switch (reason) {
	case PHY_NOISE_SAMPLE_MON:

		pi->phynoise_chan_watchdog = ch;
		pi->phynoise_state |= PHY_NOISE_STATE_MON;

		break;

	case PHY_NOISE_SAMPLE_EXTERNAL:

		pi->phynoise_state |= PHY_NOISE_STATE_EXTERNAL;
		break;

	case PHY_NOISE_SAMPLE_CRSMINCAL:

		pi->phynoise_state |= PHY_NOISE_STATE_CRSMINCAL;
		break;

	default:
		ASSERT(0);
		break;
	}

	/* start test, save the timestamp to recover in case ucode gets stuck */
	pi->phynoise_now = pi->sh->now;

	/* Fixed noise, don't need to do the real measurement */
	if (pi->phy_fixed_noise) {

		/* all other PHY */
		noise_dbm = PHY_NOISE_FIXED_VAL;
		wait_for_intr = FALSE;
		goto done;
	}

	if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL) ||
		(reason == PHY_NOISE_SAMPLE_CRSMINCAL)) {

		/* If noise mmt disabled due to LPAS active, do not initiate one */
		if (pi->phynoise_disable) {
			return;
		}

		/* Do not do noise mmt if in PM State */
		if (phy_noise_pmstate_get(pi) &&
			!wlapi_phy_awdl_is_on(pi->sh->physhim)) {
			/* Make sure that you do it every PHY_NOISE_MAX_IDLETIME atleast */
			if ((pi->sh->now - pi->phynoise_lastmmttime)
					< PHY_NOISE_MAX_IDLETIME)
				return;
		}
		/* Note current noise request time */
		pi->phynoise_lastmmttime = pi->sh->now;

		/* ucode assumes these shmems start with 0
		 * and ucode will not touch them in case of sampling expires
		 */
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP0(pi), 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP1(pi), 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP2(pi), 0);
		wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP3(pi), 0);

		if (D11REV_GE(pi->sh->corerev, 40)) {
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP4(pi), 0);
			wlapi_bmac_write_shm(pi->sh->physhim, M_PWRIND_MAP5(pi), 0);
		}

		W_REG(pi->sh->osh, D11_MACCOMMAND(pi), MCMD_BG_NOISE);

	} else {

		/* polling mode */
		phy_iq_est_t est[PHY_CORE_MAX];
		uint32 cmplx_pwr[PHY_CORE_MAX];
		int8 noise_dbm_ant[PHY_CORE_MAX];
		uint16 log_num_samps, num_samps, classif_state = 0;
		uint8 wait_time = 32;
		uint8 wait_crs = 0;
		uint8 i;

		bzero((uint8 *)est, sizeof(est));
		bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
		bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

		/* choose num_samps to be some power of 2 */
		log_num_samps = PHY_NOISE_SAMPLE_LOG_NUM_NPHY;
		num_samps = 1 << log_num_samps;

		/* suspend MAC, get classifier settings, turn it off
		 * get IQ power measurements
		 * restore classifier settings and reenable MAC ASAP
		*/
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		classif_state = READ_PHYREG(pi, ClassifierCtrl);
		phy_rxgcrs_sel_classifier(pi, 0);
		wlc_phy_rx_iq_est_acphy(pi, est, num_samps, wait_time, wait_crs, FALSE);
		WRITE_PHYREG(pi, ClassifierCtrl, classif_state);

		wlapi_enable_mac(pi->sh->physhim);

		/* sum I and Q powers for each core, average over num_samps */
		FOREACH_CORE(pi, i)
			cmplx_pwr[i] = (est[i].i_pwr + est[i].q_pwr) >> log_num_samps;

		/* pi->phy_noise_win per antenna is updated inside */
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
		wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);

		wait_for_intr = FALSE;
	}

done:
	/* if no interrupt scheduled, populate noise results now */
	if (!wait_for_intr) {
		phy_noise_invoke_callbacks(noisei, ch, noise_dbm);
	}
	return;
}

void
wlc_phy_reset_noise_var_shaping_acphy(phy_info_t *pi)
{
	uint8 i;
	uint32 zeroval = 0;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_nshapetbl_mon_t* nshapetbl_mon = pi_ac->noisei->nshapetbl_mon;
	uint8* offset = nshapetbl_mon->offset;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	/* Do table writes only if table has been modified */
	if (nshapetbl_mon->mod_flag) {

		/* Reset only already-modified entries */
		for (i = 0; i < ACPHY_SPURWAR_NV_NTONES; i++) {
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVNOISESHAPINGTBL,
			   1, offset[i], 32, &zeroval);
			PHY_INFORM(("wlc_phy_reset_noise_var_shaping_acphy: offset %d; val 0x00\n",
			            offset[i]));
		}

		/* Invalidate the monitor upon reseting the nvar shaping table */
		nshapetbl_mon->mod_flag = 0;
	}
	ACPHY_ENABLE_STALL(pi, stall_val);
}

void
wlc_phy_noise_var_shaping_acphy(phy_info_t *pi, uint8 core_nv, uint8 core_sp, int8 *tone_id,
                                        uint8 noise_var[][ACPHY_SPURWAR_NV_NTONES], uint8 reset)
{
	uint8 i, core;
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	acphy_nshapetbl_mon_t* nshapetbl_mon = pi_ac->noisei->nshapetbl_mon;
	uint8* offset = nshapetbl_mon->offset;
	uint32 tbllen;
	uint32 nvar;
	uint8 stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	if (CHSPEC_IS80(pi->radio_chanspec) || CHSPEC_IS160(pi->radio_chanspec)) {
		/* 80mhz */
		tbllen = 256;
	} else if (CHSPEC_IS40(pi->radio_chanspec)) {
		/* 40mhz */
		tbllen = 128;
	} else {
		/* 20mhz */
		tbllen = 64;
	}

	/* total tones should be equal to (nvshp + sp) tones */
	ASSERT(ACPHY_SPURWAR_NV_NTONES == ACPHY_NV_NTONES + ACPHY_SPURWAR_NTONES);

	for (i = 0; i < ACPHY_SPURWAR_NV_NTONES; i++) {
		nvar = 0;
		/* Wrap around up to tbllen */
		offset[i] = (tone_id[i] >= 0)? tone_id[i] : (tbllen + tone_id[i]);
		/* Using separate core value to have flexibility
		 * of doing nvshp & spurwar on different cores
		 * for multiple core chips without increasing
		 * number of table writes
		 */
		FOREACH_CORE(pi, core) {
			if (i < ACPHY_NV_NTONES) {
				nvar |= (core_nv & (0x1 << core))? ((noise_var[core][i] << 8*core) &
					(0xFF << 8*core)): 0x0; /* nvshp tones */
			} else {
				nvar |= (core_sp & (0x1 << core))? ((noise_var[core][i] << 8*core) &
					(0xFF << 8*core)): 0x0; /* spurwar tones */
			}
		}

		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_NVNOISESHAPINGTBL,
		                          1, offset[i], 32, &nvar);

		PHY_INFORM(("wlc_phy_reset_noise_var_shaping_acphy: offset %d; val 0x%x\n",
		            offset[i], nvar));
	}
	/* activate monitor flag */
	nshapetbl_mon->mod_flag = 1;
	ACPHY_ENABLE_STALL(pi, stall_val);
}

#ifndef WLC_DISABLE_ACI

void
wlc_phy_hwaci_override_acphy(phy_info_t *pi, int state)
{
	if (state == 0 || state == -1) {
		pi->u.pi_acphy->noisei->data->hw_aci_status = FALSE;
	} else if (state == 1) {
		pi->u.pi_acphy->noisei->data->hw_aci_status = TRUE;
	}

	if ((pi->sh->interference_mode & ACPHY_HWACI_MITIGATION) != 0) {
		/* This part of function is for HW Mitigation */
		if (state == 0) {
			if (TINY_RADIO(pi)) {
				/* Force Normal gain table */
				wlc_phy_hwaci_mitigation_enable_acphy(pi, 2, TRUE);
			} else {
				ACPHY_REG_LIST_START
					/* Make sure HWACI never triggers. */
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_1_w3,
						0xfffe)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_2_w3,
						0xfffe)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_1_w12,
						0xfffe)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_2_w12,
						0xfffe)
				ACPHY_REG_LIST_EXECUTE(pi);
				wlc_phy_hwaci_mitigate_acphy(pi, FALSE);
			}
		} else if (state == 1) {
			if (TINY_RADIO(pi)) {
				/* Force ACI gain table */
				wlc_phy_hwaci_mitigation_enable_acphy(pi, 3, TRUE);
			} else {
				ACPHY_REG_LIST_START
					/* Make sure HWACI is ALWAYS detected. */
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_1_w3,
						0x1)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_2_w3,
						0x1)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_1_w12,
						0x1)
					WRITE_PHYREG_ENTRY(pi, ACI_Detect_energy_threshold_2_w12,
						0x1)
				ACPHY_REG_LIST_EXECUTE(pi);
				wlc_phy_hwaci_mitigate_acphy(pi, TRUE);
			}
		} else if (state == -1) {
			/* Default Mode */
			if (TINY_RADIO(pi)) {
				wlc_phy_hwaci_mitigation_enable_acphy(pi, 5, TRUE);
			} else {
				wlc_phy_hwaci_setup_acphy(pi, TRUE, TRUE);
				wlc_phy_hwaci_mitigate_acphy(pi, 0);
			}
		}

	} else if ((pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT) && (state >= 0)) {
		/* This part of function is for SW Mitigation */
		int lna1_idx, lna2_idx;

		phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
		acphy_aci_params_t *aci;
		acphy_desense_values_t *desense;
		acphy_desense_values_t total_desense;
		uint8 band = CHSPEC_IS2G(pi->radio_chanspec) ? 2 : 5;
		acphy_hwaci_state_t  *hwaci_states;

		/* Get current channels ACI structure */
		if (pi_ac->noisei->aci == NULL)
			pi_ac->noisei->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
				pi->radio_chanspec, TRUE);
		aci = pi_ac->noisei->aci;

		desense = &aci->desense;

		pi_ac->noisei->aci->hwaci_desense_state = (uint8)state;

		/* Update Desense settings */
		ASSERT(pi_ac->noisei->hwaci_args != NULL);
		hwaci_states = (band == 2)
			? pi_ac->noisei->hwaci_states_2g : pi_ac->noisei->hwaci_states_5g;
		lna1_idx = hwaci_states[state].lna1_pktg_lmt;
		lna2_idx = hwaci_states[state].lna2_pktg_lmt;

		desense->lna1_gainlmt_desense = MAX(0, 5 - lna1_idx);
		desense->lna2_gainlmt_desense = MAX(0, 6 - lna2_idx);
		total_desense = phy_ac_rxgcrs_get_desense(pi_ac->rxgcrsi, TOTAL_DESENSE);
		total_desense.lna1_gainlmt_desense = desense->lna1_gainlmt_desense;
		total_desense.lna2_gainlmt_desense = desense->lna2_gainlmt_desense;
		phy_ac_rxgcrs_set_desense(pi_ac->rxgcrsi, &total_desense, TOTAL_DESENSE);

		/* Update the tables. If other things are updated may need to call gainctrl */
		wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(pi, 1);
		wlc_phy_upd_lna1_lna2_gainlimittbls_acphy(pi, 2);
	}
}

void
phy_ac_noise_hwaci_mitigation(phy_ac_noise_info_t *ni, int8 desense_state)
{
	phy_info_t *pi = ni->pi;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_aci_params_t *aci;
	acphy_desense_values_t *desense;
	uint8 band = CHSPEC_IS2G(pi->radio_chanspec) ? 2 : 5;
	uint8 prev_desense_state;
	bool hwaci_present = FALSE, aci_present;

	acphy_hwaci_state_t  *hwaci_states = (band == 2) ? ni->hwaci_states_2g :
	        ((ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) ?
			((CHSPEC_IS20(pi->radio_chanspec)) ? ni->hwaci_states_5g :
			ni->hwaci_states_5g_40_80) : ni->hwaci_states_5g);

	if (wlc_phy_is_scan_chan_acphy(pi)) return;

	/* Get current channels ACI structure */
	if (ni->aci == NULL) {
		ni->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi, pi->radio_chanspec, TRUE);
	}
	aci = ni->aci;
	desense = &aci->desense;

	PHY_ACI(("desense_state = %d, hwaci_sleep = %d\n", aci->hwaci_desense_state,
		aci->hwaci_sleep));

	prev_desense_state = aci->hwaci_desense_state;
	if (desense_state != -1) {
		hwaci_present = desense_state;
	} else {
		/* HW ACI Output */
		hwaci_present = ni->data->hw_aci_status;
	}

	aci_present = hwaci_present;
	aci->hwaci_desense_state = hwaci_present;

	if ((desense_state != -1) || (prev_desense_state != aci->hwaci_desense_state)) {
		/* Suspend mac before accessing phyregs */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* Update Desense settings */
		desense->lna1_idx_min = hwaci_states[aci_present].lna1_idx_min;
		desense->lna1_idx_max = hwaci_states[aci_present].lna1_idx_max;
		desense->lna2_idx_min = hwaci_states[aci_present].lna2_idx_min;
		desense->lna2_idx_max = hwaci_states[aci_present].lna2_idx_max;
		desense->mix_idx_min = hwaci_states[aci_present].mix_idx_min;
		desense->mix_idx_max = hwaci_states[aci_present].mix_idx_max;

		/* Update the tables. If other things are updated may need to call gainctrl */
		wlc_phy_desense_calc_total_acphy(pi_ac->rxgcrsi); /* Update in total desense */
		wlc_phy_upd_lna1_lna2_gains_acphy(pi);
		if (TINY_RADIO(pi)) {
			/* No support for 28nm radio's (yet) */
			phy_ac_rxgcrs_upd_mix_gains(pi_ac->rxgcrsi);
		}

		/* Inform rate control to slow down if mitigation is on */
		wlc_phy_aci_updsts_acphy(pi);

		if (TINY_RADIO(pi)) {
			wlc_phy_rxgainctrl_gainctrl_acphy_tiny(pi, 0);
		}

		PHY_ACI(("DESENSE ST CHG. state = %d --- LNA1_minmax = (%d, %d), "
				"LNA2_minmax = (%d, %d), TIA_minmax = (%d, %d)\n",
				aci->hwaci_desense_state, desense->lna1_idx_min,
				desense->lna1_idx_max, desense->lna2_idx_min,
				desense->lna2_idx_max, desense->mix_idx_min,
				desense->mix_idx_max));

		wlapi_enable_mac(pi->sh->physhim);
	}
}

void
wlc_phy_hwaci_engine_acphy(phy_info_t *pi)
{
	uint32 hw_detect, hw_report, sw_detect, sw_report;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_aci_params_t *aci;
	acphy_desense_values_t *desense;
	uint8 band = CHSPEC_IS2G(pi->radio_chanspec) ? 2 : 5;
	uint8 core, core_ctr, lna1_idx, lna2_idx, lna1_rout;
	uint16 thresh;
	uint8 state, prev_setup_state, prev_desense_state;
	bool hwaci_present = FALSE, w2aci_present = FALSE, aci_present;
	uint8 w2sel, w2thresh, w2[3], nb, max_states;
	bool hwaci, w2aci;
	acphy_hwaci_state_t  *hwaci_states = (band == 2) ? pi_ac->noisei->hwaci_states_2g :
	        pi_ac->noisei->hwaci_states_5g;
	uint8 shft = CHSPEC_BW_LE20(pi->radio_chanspec) ? 0 :
	        (CHSPEC_IS40(pi->radio_chanspec) ? 1 : 2);

	ASSERT(!ACPHY_ENABLE_FCBS_HWACI(pi));

	if (wlc_phy_is_scan_chan_acphy(pi)) return;

	hwaci = (pi->sh->interference_mode & ACPHY_ACI_HWACI_PKTGAINLMT) != 0;
	w2aci = (pi->sh->interference_mode & ACPHY_ACI_W2NB_PKTGAINLMT) != 0;
	if (!(hwaci | w2aci)) return;

	/* Get current channels ACI structure */
	if (pi_ac->noisei->aci == NULL) {
		pi_ac->noisei->aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi,
			pi->radio_chanspec, TRUE);
	}
	aci = pi_ac->noisei->aci;
	desense = &aci->desense;

	if (aci->hwaci_sleep > 0) {
		aci->hwaci_sleep--;
		return;
	}

	/* Don't use highest desense level based on SOI RSSi */
	max_states = (band == 2) ? pi_ac->noisei->hwaci_max_states_2g :
		pi_ac->noisei->hwaci_max_states_5g;
	if (aci->weakest_rssi < -75)
		max_states = MAX(1, max_states - 1);

	aci->hwaci_noaci_timer = MAX(0, aci->hwaci_noaci_timer - 1);
	state = aci->hwaci_setup_state;

	PHY_ACI(("aci_mode2_4. setup_state = %d, timer = %d\n", state, aci->hwaci_noaci_timer));

	/* Suspend mac before accessing phyregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* Disable BT as it affects rssi counters used for w2aci */
	wlc_btcx_override_enable(pi);

	if (hwaci) {
		if (ACHWACIREV(pi)) {
			hwaci_present = READ_PHYREGFLD(pi, ACI_Mitigation_status,
					aci_present_status);
			PHY_ACI(("aci_mode2_4(hwaci). state {setup, desense} = {%d, %d}, "
					"aci_present = %d\n",
					aci->hwaci_setup_state, aci->hwaci_desense_state,
					hwaci_present));
		} else {
			uint8 phyrxchain;
			/* HW ACI Output */
			hw_detect = 0; hw_report = 0; sw_detect = 0; sw_report = 0; core_ctr = 0;
			phyrxchain = phy_stf_get_data(pi->stfi)->phyrxchain;
			BCM_REFERENCE(phyrxchain);
			FOREACH_ACTV_CORE(pi, phyrxchain, core) {
				if (core == 0) {
					if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
						hw_detect +=
							READ_PHYREG(pi,
							ACI_Detect_aci_detected_ctr_w30);
						sw_detect +=
							READ_PHYREG(pi,
							ACI_Detect_sw_aci_detected_ctr_w30);
					} else {
						hw_detect += READ_PHYREG(pi,
							ACI_Detect_aci_detected_ctr0);
						sw_detect += READ_PHYREG(pi,
							ACI_Detect_sw_aci_detected_ctr0);
					}
					hw_report += READ_PHYREG(pi, ACI_Detect_aci_report_ctr0);
					sw_report += READ_PHYREG(pi, ACI_Detect_sw_aci_report_ctr0);
					core_ctr++;
				}
				if (core == 1) {
					if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
						hw_detect +=
							READ_PHYREG(pi,
							ACI_Detect_aci_detected_ctr_w31);
						sw_detect +=
							READ_PHYREG(pi,
							ACI_Detect_sw_aci_detected_ctr_w31);
					} else {
						hw_detect += READ_PHYREG(pi,
							ACI_Detect_aci_detected_ctr1);
						sw_detect +=
							READ_PHYREG(pi,
							ACI_Detect_sw_aci_detected_ctr1);
					}
					hw_report += READ_PHYREG(pi, ACI_Detect_aci_report_ctr1);
					sw_report += READ_PHYREG(pi, ACI_Detect_sw_aci_report_ctr1);
					core_ctr++;
				}
				if (core == 2) {
					hw_detect += READ_PHYREG(pi, ACI_Detect_aci_detected_ctr2);
					hw_report += READ_PHYREG(pi, ACI_Detect_aci_report_ctr2);
					sw_detect += READ_PHYREG(pi,
							ACI_Detect_sw_aci_detected_ctr2);
					sw_report += READ_PHYREG(pi, ACI_Detect_sw_aci_report_ctr2);
					core_ctr++;
				}
			}

			/* Average over cores */
			if (core_ctr > 1) {
				hw_detect /= core_ctr;
				hw_report /= core_ctr;
				sw_detect /= core_ctr;
				sw_report /= core_ctr;
			}

			if (sw_report < 200) {
				/* Try to think of doing something here */
				if (hw_report < 200) {
					/* invalid value, ignore */
					sw_detect = 0;
				} else {
					sw_detect = hw_detect;
					sw_report = hw_report;
				}
			}

			/* Here, if the ratio of report/detect < 2 declare ACI */
			hwaci_present = (sw_detect > 0) & (sw_report < 4 * sw_detect);

			PHY_ACI(("aci_mode2_4(hwaci). hw_detect = %d, hw_report = %d, "
					 "sw_detect = %d, sw_report = %d,"
					 "state {setup, desense} = {%d, %d}, aci_present = %d\n",
					 hw_detect, hw_report, sw_detect, sw_report,
					 aci->hwaci_setup_state, aci->hwaci_desense_state,
					 hwaci_present));
		}
	}

	/* w2, nb pair */
	if (w2aci && !ACHWACIREV(pi)) {
		nb = READ_PHYREGFLD(pi, NbClipCnt1, NbClipCntAccum1_i) >> shft;
		w2[0] = READ_PHYREGFLD(pi, W2W1ClipCnt1, W2ClipCntAccum1) >> shft;
		w2[1] = READ_PHYREGFLD(pi, W2W1ClipCnt2, W2ClipCntAccum2) >> shft;
		w2[2] = READ_PHYREGFLD(pi, W2W1ClipCnt3, W2ClipCntAccum3) >> shft;

		w2sel = hwaci_states[state].w2_sel;
		w2thresh = hwaci_states[state].w2_thresh;
		w2aci_present = (nb <= hwaci_states[state].nb_thresh);
		if (w2sel == 0) {
			/* lo */
			w2aci_present &= (w2[0] >= w2thresh) ||
			        (w2[1] > 0) || (w2[2] > 0);
		} else if (w2sel == 1) {
			/* md */
			w2aci_present &= (w2[1] >= w2thresh) || (w2[2] > 0);
		} else {
			/* hi */
			w2aci_present &= (w2[2] >= w2thresh);
		}

		PHY_ACI(("aci_mode2_4(w2nb). w2 = {%d %d %d}, nb_lo = %d, w2aci = %d\n",
		         w2[0], w2[1], w2[2], nb, w2aci_present));
	}

	aci_present = hwaci_present | w2aci_present;

	/* ***** HWACI State Machine & Apply Settings ******* */
	prev_setup_state = aci->hwaci_setup_state;
	prev_desense_state = aci->hwaci_desense_state;
	if (aci->hwaci_setup_state == 0) {
		/* Coming here for the first time */
		aci->hwaci_setup_state = 1;
	} else {
		if (aci_present) {
			/* ACI found */
			aci->hwaci_noaci_timer = ACPHY_HWACI_NOACI_WAIT_TIME;
			aci->hwaci_desense_state = aci->hwaci_setup_state;
			aci->hwaci_setup_state++;
		} else if (aci->hwaci_noaci_timer == 0) {
			/* No ACI found */
			aci->hwaci_setup_state = MAX(0, aci->hwaci_setup_state - 1);
			if (aci->hwaci_setup_state < aci->hwaci_desense_state) {
				aci->hwaci_desense_state = aci->hwaci_setup_state;
			}
		}
	}

	/* keep setup state between 1 & max. 0 is noACI state, for setup use atleast state 1 */
	aci->hwaci_setup_state = MAX(1, MIN(aci->hwaci_setup_state, max_states - 1));
	if (ASSOC_INPROG_PHY(pi) || PUB_NOT_ASSOC(pi)) {
		aci->hwaci_setup_state = 1;
		aci->hwaci_desense_state = 0;
	}

	/* Update new settings & desense */
	if (prev_setup_state != aci->hwaci_setup_state) {
		state = aci->hwaci_setup_state;
		aci->hwaci_noaci_timer = ACPHY_HWACI_NOACI_WAIT_TIME;
		aci->hwaci_sleep = ACPHY_HWACI_SLEEP_TIME;           /* let hwaci get refreshed */

	if (AC4354REV(pi) && ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0) &&
	!(PHY_ILNA(pi))) {
		wlc_phy_switch_preemption_settings(pi, aci->hwaci_setup_state);
	}
	/* Update HWACI settings */
	if (hwaci) {
		thresh = hwaci_states[state].energy_thresh;

		if (AC4354REV(pi) || ACHWACIREV(pi)) {
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1_w3, thresh);
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2_w3, thresh);
			thresh = hwaci_states[state].energy_thresh_w2;
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1_w12, thresh);
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2_w12, thresh);
		} else {
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_1, thresh);
			WRITE_PHYREG(pi, ACI_Detect_energy_threshold_2, thresh);
		}
	}

	PHY_ACI(("aci_mode2_4. SETUP STATE CHANGED. state = {%d --> %d}\n",
	         prev_setup_state, aci->hwaci_setup_state));
	}

	if (prev_desense_state != aci->hwaci_desense_state) {
		state = aci->hwaci_desense_state;
		/* Update Desense settings */
		lna1_idx = hwaci_states[state].lna1_pktg_lmt;
		lna2_idx = hwaci_states[state].lna2_pktg_lmt;
		lna1_rout = hwaci_states[state].lna1rout_pktg_lmt;
		desense->lna1_gainlmt_desense = MAX(0, 5 - lna1_idx);
		desense->lna2_gainlmt_desense = MAX(0, 6 - lna2_idx);

		if (band == 5)
			desense->lna1rout_gainlmt_desense = MAX(0, 4 - lna1_rout);
		else
			desense->lna1rout_gainlmt_desense = lna1_rout;

		/* Update the tables. If other things are updated may need to call gainctrl */
		wlc_phy_desense_calc_total_acphy(pi_ac->rxgcrsi); /* Update in total desense */
		wlc_phy_upd_lna1_lna2_gains_acphy(pi);

		/* Inform rate contorl to slow down is mitigation is on */
		wlc_phy_aci_updsts_acphy(pi);

		PHY_ACI(("aci_mode2_4. DESENSE ST CHG. state = %d --> %d, lna1 = %d, lna2 = %d\n",
		         prev_desense_state, aci->hwaci_desense_state, lna1_idx, lna2_idx));
	}

	/* Disabling BTCX Override */
	wlc_phy_btcx_override_disable(pi);

	wlapi_enable_mac(pi->sh->physhim);
}

void
wlc_phy_hwaci_engine_acphy_28nm(phy_info_t *pi)
{
	uint8 hwaci_present = 0, prev_desense_state, desense_state = 0;
	uint8 lna1_idx;
	acphy_desense_values_t *desense;
	uint8 band = CHSPEC_IS2G(pi->radio_chanspec) ? 2 : 5;
	phy_info_acphy_t *pi_ac = pi->u.pi_acphy;
	acphy_aci_params_t *aci = pi_ac->noisei->aci;
	acphy_hwaci_state_t  *hwaci_states = (band == 2) ? pi_ac->noisei->hwaci_states_2g :
	    pi_ac->noisei->hwaci_states_5g;

	if (wlc_phy_is_scan_chan_acphy(pi)) return;

	/* Get current channels ACI structure */
	if (aci == NULL) {
		aci = wlc_phy_desense_aci_getset_chanidx_acphy(pi, pi->radio_chanspec, TRUE);
	}

	desense = &aci->desense;
	prev_desense_state = aci->hwaci_desense_state;

	hwaci_present = READ_PHYREGFLD(pi, ACI_Mitigation_status, aci_present_status);

	if (hwaci_present) {
		/* ACI found */
		aci->hwaci_noaci_timer = ACPHY_HWACI_NOACI_WAIT_TIME;
	} else if (aci->hwaci_noaci_timer > 0) {
		/* No ACI found */
		aci->hwaci_noaci_timer --;
	}

	if (aci->hwaci_noaci_timer > 0) {
		if ((aci->weakest_rssi == 0) || (aci->weakest_rssi < -75))
			desense_state = 1;
		else
			desense_state = 2;
	} else {
		desense_state = 0;
	}

	if (prev_desense_state != desense_state) {
		aci->hwaci_desense_state = desense_state;
		lna1_idx = hwaci_states[desense_state].lna1_pktg_lmt;
		desense->lna1_gainlmt_desense = MAX(0, ACPHY_28nm_MAX_LNA1_IDX - lna1_idx);
		PHY_ACI(("%s, STATE CHANGED, mitigation = %d, max lna1 pktgain = %d\n",
		         __FUNCTION__, desense_state, lna1_idx));

		/* Suspend mac before accessing phyregs */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

		/* Update the tables. If other things are updated may need to call gainctrl */
		wlc_phy_desense_calc_total_acphy(pi_ac->rxgcrsi); /* Update in total desense */
		wlc_phy_upd_lna1_lna2_gains_acphy(pi);

		wlapi_enable_mac(pi->sh->physhim);
	}
}

#endif /* WLC_DISABLE_ACI */

void
wlc_phy_set_aci_regs_acphy(phy_info_t *pi)
{
	uint16 aci_th;
	uint8 core;

	if (ACMAJORREV_40(pi->pubpi->phy_rev) || ACMAJORREV_44(pi->pubpi->phy_rev)) {
		PHY_INFORM(("FIXME: 4347A0 Bypass set_aci_regs for the moment\n"));
		return;
	}
	if (ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_51(pi->pubpi->phy_rev)) {
		PHY_INFORM(("FIXME: Bypass set_aci_regs for the moment\n"));
		return;
	}

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		aci_th = 0x17f;
	} else if (ACMAJORREV_0(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev) ||
	           (ACMAJORREV_2(pi->pubpi->phy_rev) &&
	            (RADIOID(pi->pubpi->radioid) == BCM2069_ID) &&
	            (RADIOREV(pi->pubpi->radiorev) == 0x2C) && PHY_XTAL_IS40M(pi)) ||
	           ACMAJORREV_5(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
	           ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) || ACMAJORREV_51(pi->pubpi->phy_rev)) {
		/* Disable aci_absent, as it casues issues SOI pkts
		   to be missed when ACI is playing, eg channel 36 & 40, pwr = -40 dBm
		*/
		aci_th = 0x1ff;
	} else {
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			aci_th = 0xbf;	/* 5GHz, 40/20MHz BW */
		} else {
			aci_th = (ACMAJORREV_1(pi->pubpi->phy_rev) ||
			          ACMAJORREV_36(pi->pubpi->phy_rev)) ? 0x80 : 0xff;
		}
	}

	if (CHSPEC_IS8080(pi->radio_chanspec) || ACMAJORREV_GE37(pi->pubpi->phy_rev) ||
		ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		FOREACH_CORE(pi, core) {
			WRITE_PHYREGCE(pi, crsacidetectThreshl, core, aci_th);
			WRITE_PHYREGCE(pi, crsacidetectThreshu, core, aci_th);
			WRITE_PHYREGCE(pi, crsacidetectThreshlSub1, core, aci_th);
			WRITE_PHYREGCE(pi, crsacidetectThreshuSub1, core, aci_th);

			if ((wlc_phy_ac_phycap_maxbw(pi) > BW_80MHZ)) {
				WRITE_PHYREGCE(pi, crsacidetectThresh_4_, core, aci_th);
				WRITE_PHYREGCE(pi, crsacidetectThresh_5_, core, aci_th);
				WRITE_PHYREGCE(pi, crsacidetectThresh_6_, core, aci_th);
				WRITE_PHYREGCE(pi, crsacidetectThresh_7_, core, aci_th);
			}
		}
	} else {
		WRITE_PHYREG(pi, crsacidetectThreshl, aci_th);
		WRITE_PHYREG(pi, crsacidetectThreshu, aci_th);
		WRITE_PHYREG(pi, crsacidetectThreshlSub1, aci_th);
		WRITE_PHYREG(pi, crsacidetectThreshuSub1, aci_th);
	}

	/* Enable below code after analysis */
	if (ACMAJORREV_47(pi->pubpi->phy_rev)) return;

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		MOD_PHYREG(pi, RxControl, bphyacidetEn, 0);
		/* CRDOT11ACPHY-280 : enabled bphy aci det is causing hangs */
		if (CHSPEC_IS2G(pi->radio_chanspec) && CHSPEC_IS20(pi->radio_chanspec)) {
			ACPHY_REG_LIST_START
				/* Enable bphy ACI Detection HW */
				WRITE_PHYREG_ENTRY(pi, bphyaciThresh0, 0)
				WRITE_PHYREG_ENTRY(pi, bphyaciThresh1, 0)
				WRITE_PHYREG_ENTRY(pi, bphyaciThresh2, 0)
				WRITE_PHYREG_ENTRY(pi, bphyaciThresh3, 0x9F)
				WRITE_PHYREG_ENTRY(pi, bphyaciPwrThresh0, 0)
				WRITE_PHYREG_ENTRY(pi, bphyaciPwrThresh1, 0)
				WRITE_PHYREG_ENTRY(pi, bphyaciPwrThresh2, 0)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	}
}

#ifndef WLC_DISABLE_ACI
void
wlc_phy_desense_aci_reset_params_acphy(phy_info_t *pi, bool call_gainctrl, bool all2g, bool all5g)
{
	phy_ac_noise_info_t *noisei = pi->u.pi_acphy->noisei;

	if (all2g)
		bzero(noisei->aci_list2g,
			ACPHY_ACI_CHAN_LIST_SZ * sizeof(acphy_aci_params_t));
	if (all5g)
		bzero(noisei->aci_list5g,
			ACPHY_ACI_CHAN_LIST_SZ * sizeof(acphy_aci_params_t));

	if (all2g || all5g) {
		noisei->aci = NULL;
	} else if (noisei->aci != NULL) {
		bzero(&noisei->aci->bphy_hist, sizeof(desense_history_t));
		bzero(&noisei->aci->ofdm_hist, sizeof(desense_history_t));
		noisei->aci->glitch_buff_idx = 0;
		noisei->aci->glitch_upd_wait = 1;
		noisei->aci->jammer_cnt = 0;
		bzero(&noisei->aci->desense, sizeof(acphy_desense_values_t));
	}

	/* Call gainctrl to reset all the phy regs */
	if (call_gainctrl)
		wlc_phy_desense_apply_acphy(pi, TRUE);
}

void
wlc_phy_desense_aci_upd_txop_acphy(phy_info_t *pi, chanspec_t chanspec, uint8 txop)
{
	acphy_aci_params_t *aci;

	aci = (acphy_aci_params_t *)
	    wlc_phy_desense_aci_getset_chanidx_acphy(pi, chanspec, FALSE);

	/* not found in phy list of channels */
	if (aci == NULL) return;

	/* reserve 0 for no update of txop */
	aci->txop = MAX(1, txop);
}

/* Update chan stats offline, i.e. we might not be on this channel currently */
static void
wlc_phy_desense_aci_upd_chan_stats_acphy(phy_type_noise_ctx_t *ctx, chanspec_t chanspec, int8 rssi)
{
	phy_ac_noise_info_t *info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	acphy_aci_params_t *aci;

	aci = (acphy_aci_params_t *)
	        wlc_phy_desense_aci_getset_chanidx_acphy(pi, chanspec, FALSE);

	if (aci == NULL) return;   /* not found in phy list of channels */

	aci->weakest_rssi = rssi;
}

void
wlc_phy_aci_updsts_acphy(phy_info_t *pi)
{
	phy_ac_noise_info_t *noisei = pi->u.pi_acphy->noisei;
	acphy_aci_params_t *aci;
	uint32 phy_mode = 0;

	if (noisei->aci != NULL) {
		aci = noisei->aci;
		if (aci->desense.on || aci->hwaci_desense_state > 0)
			phy_mode = PHY_MODE_ACI;
	}

	wlapi_high_update_phy_mode(pi->sh->physhim, phy_mode);
}
#endif /* !WLC_DISABLE_ACI */

void
wlc_phy_switch_preemption_settings(phy_info_t *pi, uint8 state)
{
	/* 4354A0/A1:dynamic preemption settings based on hwaci state:
	   Aggressive settings for no_aci or aci < -40 dbm
	   & conservative settings for aci > -40 dbm
	 */
	if (!AC4354REV(pi) && !ACHWACIREV(pi)) return;

	if (IS_4364_3x3(pi)) return;

	if ((AC4354REV(pi) && ((state == 0) || (state == 1))) ||
	    (ACHWACIREV(pi) && (state == 0))) {
		/* no aci or aci < -40 dbm : Aggressive settings */
		ACPHYREG_BCAST(pi, PREMPT_per_pkt_en0, 0x31);
			if (CHSPEC_IS20(pi->radio_chanspec)) {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x24);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x48);
			} else {
				 ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0xa0);
			}
	} else {
	/* state is 2 or 3:aci > -40 dbm */
		ACPHYREG_BCAST(pi, PREMPT_per_pkt_en0, 0x21);
			if (CHSPEC_IS20(pi->radio_chanspec)) {
				ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x2d);
			} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x64);
				} else {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x48);
				}
			} else {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0xdc);
			}
	}
}

	static aci_reg_list_entry g_aci_reg_list_acphy[100];

static aci_reg_list_entry *
BCMRAMFN(phy_ac_noise_get_aci_reg_list)(void)
{
	return g_aci_reg_list_acphy;
}

static aci_tbl_list_entry_at_init g_aci_tbl_list_acphy [ ] =
	{
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 0, 7, 7, 1},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 8, 7, 7, 1},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 16, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 24, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 32, 7, 7, 1},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 40, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 48, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 56, 7, 7, 1},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 64, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 72, 7, 7, 0},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 80, 7, 7, 1},
		{ACPHY_TBL_ID_LNAROUTLUTACI, ACPHY_TBL_ID_LNAROUT, 88, 7, 7, 1},
		{ACPHY_TBL_ID_GAINACI0, ACPHY_TBL_ID_GAIN0, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINACI0, ACPHY_TBL_ID_GAIN0, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINACI1, ACPHY_TBL_ID_GAIN1, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINACI1, ACPHY_TBL_ID_GAIN1, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINACI2, ACPHY_TBL_ID_GAIN2, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINACI2, ACPHY_TBL_ID_GAIN2, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINACI3, ACPHY_TBL_ID_GAIN3, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINACI3, ACPHY_TBL_ID_GAIN3, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINBITSACI0, ACPHY_TBL_ID_GAINBITS0, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINBITSACI0, ACPHY_TBL_ID_GAINBITS0, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINBITSACI1, ACPHY_TBL_ID_GAINBITS1, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINBITSACI1, ACPHY_TBL_ID_GAINBITS1, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINBITSACI2, ACPHY_TBL_ID_GAINBITS2, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINBITSACI2, ACPHY_TBL_ID_GAINBITS2, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINBITSACI3, ACPHY_TBL_ID_GAINBITS3, 0, 70, 8, 1},
		{ACPHY_TBL_ID_GAINBITSACI3, ACPHY_TBL_ID_GAINBITS3, 70, 49, 8, 0},
		{ACPHY_TBL_ID_GAINLIMITACI0, ACPHY_TBL_ID_GAINLIMIT0, 0, 105, 8, 0},
		{ACPHY_TBL_ID_GAINLIMITACI1, ACPHY_TBL_ID_GAINLIMIT1, 0, 105, 8, 0},
		{ACPHY_TBL_ID_GAINLIMITACI2, ACPHY_TBL_ID_GAINLIMIT2, 0, 105, 8, 0},
		{ACPHY_TBL_ID_GAINLIMITACI3, ACPHY_TBL_ID_GAINLIMIT3, 0, 105, 8, 0},
		{ACPHY_TBL_ID_RSSICLIPGAINACI0, ACPHY_TBL_ID_RSSICLIPGAIN0, 0, 22, 24, 0},
		{ACPHY_TBL_ID_RSSICLIPGAINACI1, ACPHY_TBL_ID_RSSICLIPGAIN1, 0, 22, 24, 0},
		{ACPHY_TBL_ID_RSSICLIPGAINACI2, ACPHY_TBL_ID_RSSICLIPGAIN2, 0, 22, 24, 0},
		{ACPHY_TBL_ID_RSSICLIPGAINACI3, ACPHY_TBL_ID_RSSICLIPGAIN3, 0, 22, 24, 0},
		{ACPHY_TBL_ID_MCLPAGCCLIP2TBLACI0, ACPHY_TBL_ID_MCLPAGCCLIP2TBL0, 0, 9, 28, 0},
		{ACPHY_TBL_ID_MCLPAGCCLIP2TBLACI1, ACPHY_TBL_ID_MCLPAGCCLIP2TBL1, 0, 9, 28, 0},
		{ACPHY_TBL_ID_MCLPAGCCLIP2TBLACI2, ACPHY_TBL_ID_MCLPAGCCLIP2TBL2, 0, 9, 28, 0},
		{ACPHY_TBL_ID_MCLPAGCCLIP2TBLACI3, ACPHY_TBL_ID_MCLPAGCCLIP2TBL3, 0, 9, 28, 0},
		{0xFFFF, 0, 0, 0, 0, 0}
	};

static aci_tbl_list_entry_at_init *
BCMRAMFN(phy_ac_noise_get_aci_tbl_list)(void)
{
	return g_aci_tbl_list_acphy;
}

void
phy_ac_noise_hwaci_switching_regs_tbls_list_init(phy_info_t *pi)
{
	phy_ac_noise_data_t *data = pi->u.pi_acphy->noisei->data;
	aci_reg_list_entry *aci_reg_list_acphy = phy_ac_noise_get_aci_reg_list();

	aci_reg_list_acphy[0].regaddraci = ACPHY_Core0Adcclip_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[0].regaddr = ACPHY_Core0Adcclip(pi->pubpi->phy_rev);
	aci_reg_list_acphy[1].regaddraci = ACPHY_Core0FastAgcClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[1].regaddr = ACPHY_Core0FastAgcClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[2].regaddraci = ACPHY_Core0RssiClipMuxSel_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[2].regaddr = ACPHY_Core0RssiClipMuxSel(pi->pubpi->phy_rev);
	aci_reg_list_acphy[3].regaddraci = ACPHY_Core0InitGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[3].regaddr = ACPHY_Core0InitGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[4].regaddraci = ACPHY_Core0InitGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[4].regaddr = ACPHY_Core0InitGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[5].regaddraci = ACPHY_Core0clipHiGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[5].regaddr = ACPHY_Core0clipHiGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[6].regaddraci = ACPHY_Core0clipHiGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[6].regaddr = ACPHY_Core0clipHiGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[7].regaddraci = ACPHY_Core0clipmdGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[7].regaddr = ACPHY_Core0clipmdGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[8].regaddraci = ACPHY_Core0clipmdGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[8].regaddr = ACPHY_Core0clipmdGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[9].regaddraci = ACPHY_Core0cliploGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[9].regaddr = ACPHY_Core0cliploGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[10].regaddraci = ACPHY_Core0cliploGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[10].regaddr = ACPHY_Core0cliploGainCodeB(pi->pubpi.phy_rev);
	aci_reg_list_acphy[11].regaddraci = ACPHY_Core0clip2GainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[11].regaddr = ACPHY_Core0clip2GainCodeA(pi->pubpi.phy_rev);
	aci_reg_list_acphy[12].regaddraci = ACPHY_Core0clip2GainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[12].regaddr = ACPHY_Core0clip2GainCodeB(pi->pubpi.phy_rev);
	aci_reg_list_acphy[13].regaddraci = ACPHY_Core0clipGainCodeB_ilnaP_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[13].regaddr = ACPHY_Core0clipGainCodeB_ilnaP(pi->pubpi->phy_rev);
	aci_reg_list_acphy[14].regaddraci = ACPHY_Core0DSSScckPktGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[14].regaddr = ACPHY_Core0DSSScckPktGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[15].regaddraci = ACPHY_Core0HpFBw_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[15].regaddr = ACPHY_Core0HpFBw(pi->pubpi->phy_rev);
	aci_reg_list_acphy[16].regaddraci = ACPHY_Core0mClpAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[16].regaddr = ACPHY_Core0mClpAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[17].regaddraci = ACPHY_Core0mClpAgcW3ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[17].regaddr = ACPHY_Core0mClpAgcW3ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[18].regaddraci = ACPHY_Core0mClpAgcNbClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[18].regaddr = ACPHY_Core0mClpAgcNbClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[19].regaddraci = ACPHY_Core0mClpAgcMDClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[19].regaddr = ACPHY_Core0mClpAgcMDClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[20].regaddraci = ACPHY_Core0mClpAgcMDClipCntTh2_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[20].regaddr = ACPHY_Core0mClpAgcMDClipCntTh2(pi->pubpi->phy_rev);
	aci_reg_list_acphy[21].regaddraci = ACPHY_Core0SsAgcNbClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[21].regaddr = ACPHY_Core0SsAgcNbClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[22].regaddraci = ACPHY_Core0SsAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[22].regaddr = ACPHY_Core0SsAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[23].regaddraci = ACPHY_Core0_BiQuad_MaxGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[23].regaddr = ACPHY_Core0_BiQuad_MaxGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[24].regaddraci = ACPHY_Core1Adcclip_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[24].regaddr = ACPHY_Core1Adcclip(pi->pubpi->phy_rev);
	aci_reg_list_acphy[25].regaddraci = ACPHY_Core1FastAgcClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[25].regaddr = ACPHY_Core1FastAgcClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[26].regaddraci = ACPHY_Core1RssiClipMuxSel_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[26].regaddr = ACPHY_Core1RssiClipMuxSel(pi->pubpi->phy_rev);
	aci_reg_list_acphy[27].regaddraci = ACPHY_Core1InitGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[27].regaddr = ACPHY_Core1InitGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[28].regaddraci = ACPHY_Core1InitGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[28].regaddr = ACPHY_Core1InitGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[29].regaddraci = ACPHY_Core1clipHiGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[29].regaddr = ACPHY_Core1clipHiGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[30].regaddraci = ACPHY_Core1clipHiGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[30].regaddr = ACPHY_Core1clipHiGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[31].regaddraci = ACPHY_Core1clipmdGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[31].regaddr = ACPHY_Core1clipmdGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[32].regaddraci = ACPHY_Core1clipmdGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[32].regaddr = ACPHY_Core1clipmdGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[33].regaddraci = ACPHY_Core1cliploGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[33].regaddr = ACPHY_Core1cliploGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[34].regaddraci = ACPHY_Core1cliploGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[34].regaddr = ACPHY_Core1cliploGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[35].regaddraci = ACPHY_Core1clip2GainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[35].regaddr = ACPHY_Core1clip2GainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[36].regaddraci = ACPHY_Core1clip2GainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[36].regaddr = ACPHY_Core1clip2GainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[37].regaddraci = ACPHY_Core1clipGainCodeB_ilnaP_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[37].regaddr = ACPHY_Core1clipGainCodeB_ilnaP(pi->pubpi->phy_rev);
	aci_reg_list_acphy[38].regaddraci = ACPHY_Core1DSSScckPktGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[38].regaddr = ACPHY_Core1DSSScckPktGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[39].regaddraci = ACPHY_Core1HpFBw_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[39].regaddr = ACPHY_Core1HpFBw(pi->pubpi->phy_rev);
	aci_reg_list_acphy[40].regaddraci = ACPHY_Core1mClpAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[40].regaddr = ACPHY_Core1mClpAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[41].regaddraci = ACPHY_Core1mClpAgcW3ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[41].regaddr = ACPHY_Core1mClpAgcW3ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[42].regaddraci = ACPHY_Core1mClpAgcNbClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[42].regaddr = ACPHY_Core1mClpAgcNbClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[43].regaddraci = ACPHY_Core1mClpAgcMDClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[43].regaddr = ACPHY_Core1mClpAgcMDClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[44].regaddraci = ACPHY_Core1mClpAgcMDClipCntTh2_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[44].regaddr = ACPHY_Core1mClpAgcMDClipCntTh2(pi->pubpi->phy_rev);
	aci_reg_list_acphy[45].regaddraci = ACPHY_Core1SsAgcNbClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[45].regaddr = ACPHY_Core1SsAgcNbClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[46].regaddraci = ACPHY_Core1SsAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[46].regaddr = ACPHY_Core1SsAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[47].regaddraci = ACPHY_Core1_BiQuad_MaxGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[47].regaddr = ACPHY_Core1_BiQuad_MaxGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[48].regaddraci = ACPHY_Core2Adcclip_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[48].regaddr = ACPHY_Core2Adcclip(pi->pubpi->phy_rev);
	aci_reg_list_acphy[49].regaddraci = ACPHY_Core2FastAgcClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[49].regaddr = ACPHY_Core2FastAgcClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[50].regaddraci = ACPHY_Core2RssiClipMuxSel_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[50].regaddr = ACPHY_Core2RssiClipMuxSel(pi->pubpi->phy_rev);
	aci_reg_list_acphy[51].regaddraci = ACPHY_Core2InitGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[51].regaddr = ACPHY_Core2InitGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[52].regaddraci = ACPHY_Core2InitGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[52].regaddr = ACPHY_Core2InitGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[53].regaddraci = ACPHY_Core2clipHiGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[53].regaddr = ACPHY_Core2clipHiGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[54].regaddraci = ACPHY_Core2clipHiGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[54].regaddr = ACPHY_Core2clipHiGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[55].regaddraci = ACPHY_Core2clipmdGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[55].regaddr = ACPHY_Core2clipmdGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[56].regaddraci = ACPHY_Core2clipmdGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[56].regaddr = ACPHY_Core2clipmdGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[57].regaddraci = ACPHY_Core2cliploGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[57].regaddr = ACPHY_Core2cliploGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[58].regaddraci = ACPHY_Core2cliploGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[58].regaddr = ACPHY_Core2cliploGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[59].regaddraci = ACPHY_Core2clip2GainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[59].regaddr = ACPHY_Core2clip2GainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[60].regaddraci = ACPHY_Core2clip2GainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[60].regaddr = ACPHY_Core2clip2GainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[61].regaddraci = ACPHY_Core2clipGainCodeB_ilnaP_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[61].regaddr = ACPHY_Core2clipGainCodeB_ilnaP(pi->pubpi->phy_rev);
	aci_reg_list_acphy[62].regaddraci = ACPHY_Core2DSSScckPktGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[62].regaddr = ACPHY_Core2DSSScckPktGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[63].regaddraci = ACPHY_Core2HpFBw_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[63].regaddr = ACPHY_Core2HpFBw(pi->pubpi->phy_rev);
	aci_reg_list_acphy[64].regaddraci = ACPHY_Core2mClpAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[64].regaddr = ACPHY_Core2mClpAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[65].regaddraci = ACPHY_Core2mClpAgcW3ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[65].regaddr = ACPHY_Core2mClpAgcW3ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[66].regaddraci = ACPHY_Core2mClpAgcNbClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[66].regaddr = ACPHY_Core2mClpAgcNbClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[67].regaddraci = ACPHY_Core2mClpAgcMDClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[67].regaddr = ACPHY_Core2mClpAgcMDClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[68].regaddraci = ACPHY_Core2mClpAgcMDClipCntTh2_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[68].regaddr = ACPHY_Core2mClpAgcMDClipCntTh2(pi->pubpi->phy_rev);
	aci_reg_list_acphy[69].regaddraci = ACPHY_Core2SsAgcNbClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[69].regaddr = ACPHY_Core2SsAgcNbClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[70].regaddraci = ACPHY_Core2SsAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[70].regaddr = ACPHY_Core2SsAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[71].regaddraci = ACPHY_Core2_BiQuad_MaxGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[71].regaddr = ACPHY_Core2_BiQuad_MaxGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[72].regaddraci = ACPHY_Core3Adcclip_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[72].regaddr = ACPHY_Core3Adcclip(pi->pubpi->phy_rev);
	aci_reg_list_acphy[73].regaddraci = ACPHY_Core3FastAgcClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[73].regaddr = ACPHY_Core3FastAgcClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[74].regaddraci = ACPHY_Core3RssiClipMuxSel_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[74].regaddr = ACPHY_Core3RssiClipMuxSel(pi->pubpi->phy_rev);
	aci_reg_list_acphy[75].regaddraci = ACPHY_Core3InitGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[75].regaddr = ACPHY_Core3InitGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[76].regaddraci = ACPHY_Core3InitGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[76].regaddr = ACPHY_Core3InitGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[77].regaddraci = ACPHY_Core3clipHiGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[77].regaddr = ACPHY_Core3clipHiGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[78].regaddraci = ACPHY_Core3clipHiGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[78].regaddr = ACPHY_Core3clipHiGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[79].regaddraci = ACPHY_Core3clipmdGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[79].regaddr = ACPHY_Core3clipmdGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[80].regaddraci = ACPHY_Core3clipmdGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[80].regaddr = ACPHY_Core3clipmdGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[81].regaddraci = ACPHY_Core3cliploGainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[81].regaddr = ACPHY_Core3cliploGainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[82].regaddraci = ACPHY_Core3cliploGainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[82].regaddr = ACPHY_Core3cliploGainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[83].regaddraci = ACPHY_Core3clip2GainCodeA_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[83].regaddr = ACPHY_Core3clip2GainCodeA(pi->pubpi->phy_rev);
	aci_reg_list_acphy[84].regaddraci = ACPHY_Core3clip2GainCodeB_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[84].regaddr = ACPHY_Core3clip2GainCodeB(pi->pubpi->phy_rev);
	aci_reg_list_acphy[85].regaddraci = ACPHY_Core3clipGainCodeB_ilnaP_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[85].regaddr = ACPHY_Core3clipGainCodeB_ilnaP(pi->pubpi->phy_rev);
	aci_reg_list_acphy[86].regaddraci = ACPHY_Core3DSSScckPktGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[86].regaddr = ACPHY_Core3DSSScckPktGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[87].regaddraci = ACPHY_Core3HpFBw_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[87].regaddr = ACPHY_Core3HpFBw(pi->pubpi->phy_rev);
	aci_reg_list_acphy[88].regaddraci = ACPHY_Core3mClpAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[88].regaddr = ACPHY_Core3mClpAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[89].regaddraci = ACPHY_Core3mClpAgcW3ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[89].regaddr = ACPHY_Core3mClpAgcW3ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[90].regaddraci = ACPHY_Core3mClpAgcNbClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[90].regaddr = ACPHY_Core3mClpAgcNbClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[91].regaddraci = ACPHY_Core3mClpAgcMDClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[91].regaddr = ACPHY_Core3mClpAgcMDClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[92].regaddraci = ACPHY_Core3mClpAgcMDClipCntTh2_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[92].regaddr = ACPHY_Core3mClpAgcMDClipCntTh2(pi->pubpi->phy_rev);
	aci_reg_list_acphy[93].regaddraci = ACPHY_Core3SsAgcNbClipCntTh1_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[93].regaddr = ACPHY_Core3SsAgcNbClipCntTh1(pi->pubpi->phy_rev);
	aci_reg_list_acphy[94].regaddraci = ACPHY_Core3SsAgcW1ClipCntTh_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[94].regaddr = ACPHY_Core3SsAgcW1ClipCntTh(pi->pubpi->phy_rev);
	aci_reg_list_acphy[95].regaddraci = ACPHY_Core3_BiQuad_MaxGain_aci(pi->pubpi->phy_rev);
	aci_reg_list_acphy[95].regaddr = ACPHY_Core3_BiQuad_MaxGain(pi->pubpi->phy_rev);
	aci_reg_list_acphy[96].regaddr = 0xFFFF;

	data->hwaci_phyreg_list = aci_reg_list_acphy;
	data->hwaci_phytbl_list = phy_ac_noise_get_aci_tbl_list();
}

#ifdef RADIO_HEALTH_CHECK
/* Call this function after cals in watchdog */

#define PHY_FORCEFAIL_DESENSE_VALUE	255

static phy_crash_reason_t
phy_ac_noise_healthcheck_desense(phy_type_noise_ctx_t *ctx)
{
	acphy_aci_params_t *aci = NULL;
	acphy_desense_values_t *desense;
	int8 allowed_weakest_rssi, max_weakest_rssi = PHY_INVALID_RSSI;

#ifndef WLC_DISABLE_ACI
	phy_ac_noise_info_t *info = (phy_ac_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	aci = (acphy_aci_params_t *)
		wlc_phy_desense_aci_getset_chanidx_acphy(pi, pi->radio_chanspec, FALSE);
#endif /* !WLC_DISABLE_ACI */

	if (aci == NULL) {
		return PHY_RC_NONE;
	}
	desense = &aci->desense;
	if (desense->forced) {
		return PHY_RC_NONE;
	}
	if (aci->weakest_rssi < 0) {
		max_weakest_rssi = MAX(aci->weakest_rssi, max_weakest_rssi);
	}
	if (max_weakest_rssi == PHY_INVALID_RSSI) {
		return PHY_RC_NONE;
	}
	allowed_weakest_rssi = max_weakest_rssi +  PHY_AC_OFFSET_WEAKEST_RSSI;
	if (((desense->ofdm_desense) &&
		(((desense->ofdm_desense - PHY_AC_DESENSE_WEAKEST_RSSI_OFDM) >
		allowed_weakest_rssi))) ||
		((desense->bphy_desense) &&
		(((desense->bphy_desense - PHY_AC_DESENSE_WEAKEST_RSSI_BPHY) >
		allowed_weakest_rssi)))) {
		PHY_FATAL_ERROR_MESG(("\n %s : desense->ofdm_desense : %d "
			" desense->bphy_desense : %d "
			" max_weakest_rssi: %d allowed_weakest_rssi: %d\n",
			__FUNCTION__, desense->ofdm_desense,
			desense->bphy_desense, max_weakest_rssi, allowed_weakest_rssi));
		if ((aci->desense.ofdm_desense == PHY_FORCEFAIL_DESENSE_VALUE) &&
			(aci->desense.bphy_desense == PHY_FORCEFAIL_DESENSE_VALUE)) {
			return (PHY_RC_DESENSE_LIMITS);
		}
	}
	return PHY_RC_NONE;
}
int phy_ac_noise_force_fail_desense(phy_ac_noise_info_t *noisei)
{
	acphy_aci_params_t *aci = NULL;
#ifndef WLC_DISABLE_ACI
	phy_info_t *pi = noisei->pi;
	aci = (acphy_aci_params_t *)
			wlc_phy_desense_aci_getset_chanidx_acphy(pi,
			pi->radio_chanspec, FALSE);
#endif /* !WLC_DISABLE_ACI */
	if (aci == NULL)
		return BCME_UNSUPPORTED;
	/* Max possible desense value */
	aci->desense.ofdm_desense = PHY_FORCEFAIL_DESENSE_VALUE;
	aci->desense.bphy_desense = PHY_FORCEFAIL_DESENSE_VALUE;
	aci->weakest_rssi = -PHY_AC_DESENSE_WEAKEST_RSSI_OFDM;
	return BCME_OK;
}
#endif /* RADIO_HEALTH_CHECK */

static int8
phy_ac_noise_avg(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;

	int tot = 0;
	int i = 0;
	int j = 0;

	for (i = 0; i < MA_WINDOW_SZ; i++) {
		if (pi->sh->phy_noise_window[i] != 0) {
			tot += pi->sh->phy_noise_window[i];
			++j;
		}
	}
	if (j) {
		tot /= j;
	}

	return (int8)tot;
}

/* ucode finished phy noise measurement and raised interrupt */
static void
phy_ac_noise_sample_intr(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	uint16 jssi_aux;
	uint8 channel = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	uint8 lte_on, crs_high;

	/* copy of the M_CURCHANNEL, just channel number plus 2G/5G flag */
	jssi_aux = wlapi_bmac_read_shm(pi->sh->physhim, M_JSSI_AUX(pi));
	channel = jssi_aux & D11_CURCHANNEL_MAX;
	noise_dbm = phy_noise_read_shmem(noisei, &lte_on, &crs_high);
	PHY_INFORM(("%s: noise_dBm = %d, lte_on=%d, crs_high=%d\n",
		__FUNCTION__, noise_dbm, lte_on, crs_high));

	if (lte_on) {
		noise_info->noise_lte = noise_dbm;
	} else {
		phy_noise_invoke_callbacks(noisei, channel, noise_dbm);
	}
}

static void
phy_ac_noise_cb(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;

	if (pi->phynoise_state & PHY_NOISE_STATE_CRSMINCAL) {
		pi->phynoise_state &= ~PHY_NOISE_STATE_CRSMINCAL;
	}
}

/* preemption table for 4365 post Rx filter detection */
static void phy_ac_noise_preempt_postfilter_reg_tbl(phy_ac_noise_info_t *ni, bool enable)
{
	phy_info_t *pi = ni->pi;
	uint16 k, preempt_en = 0;
	uint16 ppr_preempt_phyreg_vals[][2] = {
		{ACPHY_PREMPT_ofdm_nominal_clip_th0(pi->pubpi->phy_rev), 0xae00},
		{ACPHY_PREMPT_cck_nominal_clip_th0(pi->pubpi->phy_rev), 0x4d00},
		{ACPHY_PREMPT_ofdm_large_gain_mismatch_th0(pi->pubpi->phy_rev), 0x1e},
		{ACPHY_PREMPT_cck_large_gain_mismatch_th0(pi->pubpi->phy_rev), 0x10},
		{ACPHY_PREMPT_ofdm_low_power_mismatch_th0(pi->pubpi->phy_rev), 0x1a},
		{ACPHY_PREMPT_cck_low_power_mismatch_th0(pi->pubpi->phy_rev), 0x1c},
		{ACPHY_PREMPT_ofdm_max_gain_mismatch_pkt_rcv_th0(pi->pubpi->phy_rev), 0x1e},
		{ACPHY_PREMPT_cck_max_gain_mismatch_pkt_rcv_th0(pi->pubpi->phy_rev), 0xa}};

	/* for 4365C0, set hi RSSI(low Rx gain) thresold, around -22 ~ -24 dBm */
	uint16 ppr_preempt_phyreg_vals33[][2] = {
		{ACPHY_PREMPT_ofdm_nominal_clip_th_hipwr_xtra_bits0(pi->pubpi->phy_rev), 0x2400},
		/* maximize clip detect thresholds for hi RSSI region to eliminate self-aborts */
		{ACPHY_PREMPT_ofdm_nominal_clip_th_hipwr0(pi->pubpi->phy_rev), 0xffff},
		{ACPHY_PREMPT_cck_nominal_clip_th_hipwr0(pi->pubpi->phy_rev), 0xffff},
		{ACPHY_PREMPT_ofdm_nominal_clip_th0(pi->pubpi->phy_rev), 0x5300},
		{ACPHY_PREMPT_cck_nominal_clip_th0(pi->pubpi->phy_rev), 0x3e00}};

	uint16 ppr_preempt_phyreg_vals47[][2] = {
		{ACPHY_PREMPT_ofdm_nominal_clip_th_hipwr_xtra_bits0(pi->pubpi->phy_rev), 0x2f00},
		/* maximize clip detect thresholds for hi RSSI region to eliminate self-aborts */
		{ACPHY_PREMPT_ofdm_nominal_clip_th_hipwr0(pi->pubpi->phy_rev), 0xffff},
		{ACPHY_PREMPT_cck_nominal_clip_th_hipwr0(pi->pubpi->phy_rev), 0xffff},
		{ACPHY_PREMPT_ofdm_nominal_clip_th0(pi->pubpi->phy_rev), 0x4c00},
		{ACPHY_PREMPT_cck_nominal_clip_th0(pi->pubpi->phy_rev), 0x6000},
		{ACPHY_PREMPT_ofdm_max_gain_mismatch_pkt_rcv_th0(pi->pubpi->phy_rev), 0x12},
		{ACPHY_PREMPT_cck_max_gain_mismatch_pkt_rcv_th0(pi->pubpi->phy_rev), 0xe}};

	if (enable) {
		for (k = 0; k < ARRAYSIZE(ppr_preempt_phyreg_vals); k++) {
			/* using broadcast address to take care of all 4 tones */
			phy_utils_write_phyreg(pi,
			(ppr_preempt_phyreg_vals[k][0] | ACPHY_REG_BROADCAST(pi)),
			ppr_preempt_phyreg_vals[k][1]);
		}

		if (ACMAJORREV_33(pi->pubpi->phy_rev)) {
			for (k = 0; k < ARRAYSIZE(ppr_preempt_phyreg_vals33); k++) {
				phy_utils_write_phyreg(pi, (ppr_preempt_phyreg_vals33[k][0] |
				ACPHY_REG_BROADCAST(pi)),
				ppr_preempt_phyreg_vals33[k][1]);
			}
		}

		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			for (k = 0; k < ARRAYSIZE(ppr_preempt_phyreg_vals47); k++) {
				phy_utils_write_phyreg(pi, (ppr_preempt_phyreg_vals47[k][0] |
				ACPHY_REG_BROADCAST(pi)),
				ppr_preempt_phyreg_vals47[k][1]);
			}
		}

		if ((pi->sh->interference_mode & ACPHY_LPD_PREEMPTION) != 0) {
			preempt_en = 0x1b;
		} else {
			preempt_en = 0x19;
		}
		if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
			if ((pi->sh->interference_mode & ACPHY_LPD_PREEMPTION) != 0) {
				preempt_en = 0x1f;
			} else {
				preempt_en = 0x1d;
			}
		}
	}

	ACPHYREG_BCAST(pi, PREMPT_per_pkt_en0, preempt_en);
	//wlapi_bmac_write_shm(pi->sh->physhim, M_PHYPREEMPT_VAL(pi), preempt_en);
}

void
phy_ac_noise_preempt(phy_ac_noise_info_t *ni, bool enable_preempt, bool EnablePostRxFilter_Proc)
{
	uint8 core;
	uint8 stall_val;
	phy_info_t *pi = ni->pi;

	if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
		return;
	}

	if (CHSPEC_IS2G(pi->radio_chanspec) && IS_4350(pi) &&
	    (wlapi_bmac_btc_mode_get(pi->sh->physhim) == WL_BTC_HYBRID)) {
		/* override flag ... never enable */
		enable_preempt &= !(phy_btcx_is_btactive(pi->btcxi));
	}

	/* Update SW copy of preemption setting */
	ni->data->current_preemption_status = enable_preempt;
	ni->data->pktabortctl = 0;

	if ((ACMAJORREV_1(pi->pubpi->phy_rev) && ACMINORREV_2(pi)) ||
	    ACMAJORREV_3(pi->pubpi->phy_rev)) {
		if (enable_preempt) {
			if (ACMAJORREV_3(pi->pubpi->phy_rev)) {
				/* disable clip condition for norm power when preemption on */
				_PHY_REG_MOD(pi, ACPHY_MLDisableMcs(pi->pubpi->phy_rev),
				             0x0001, 0x0001);
				MOD_PHYREG(pi, Core0_BPHY_TargetVar_log2_pt8us,
				           bphy_targetVar_log2_pt8us, 469);
				WRITE_PHYREG(pi, PktAbortSupportedStates,
				             (ACREV_GE(pi->pubpi->phy_rev, 13)) ? 0x2bbf : 0x2bb7);
				/* Enable pre rxfilt power & disable sssgc post abort */
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
				wlc_phy_deaf_acphy(pi, TRUE);
				WRITE_PHYREG(pi, PktAbortCounterClr, 0x08);
				wlc_phy_deaf_acphy(pi, FALSE);
				wlapi_enable_mac(pi->sh->physhim);
			} else {
				WRITE_PHYREG(pi, PktAbortSupportedStates, 0x2bb7);
				WRITE_PHYREG(pi, SpareReg, 0x3f);
			}
			ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x21)
				WRITE_PHYREG_ENTRY(pi, PktAbortCtrl, 0x1841)
				WRITE_PHYREG_ENTRY(pi, BphyAbortExitCtrl, 0x3840)
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 1)
				WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0, 0xffff)
				WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_large_gain_mismatch_th0, 0x1f)
				WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_th0, 0xffff)
				WRITE_PHYREG_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0, 0x1f)
			ACPHY_REG_LIST_EXECUTE(pi);
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (CHSPEC_IS80(pi->radio_chanspec)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0xb0);
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x50);
				} else {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
				}

				if ((ACMAJORREV_3(pi->pubpi->phy_rev)) &&
					(ni->LTEJ_WAR_en == TRUE)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_large_gain_mismatch_th0, 9);
				}
			} else {
				WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
				WRITE_PHYREG(pi, PREMPT_cck_nominal_clip_cnt_th0, 0x38);
			}
		} else {
			ACPHY_REG_LIST_START
				/* disable Preempt */
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 0)
				MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 0)
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x0)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev) || ACMAJORREV_5(pi->pubpi->phy_rev)) {
		/* Enable Preemption */
		if (enable_preempt) {
			WRITE_PHYREG(pi, PktAbortCtrl, 0x1041);
			WRITE_PHYREG(pi, RxMacifMode, 0x0a00);

			if ((ACMAJORREV_2(pi->pubpi->phy_rev) && ACMINORREV_4(pi)) ||
					ACMAJORREV_5(pi->pubpi->phy_rev)) {
			  ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, PktAbortSupportedStates, 0x2fff)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0, 0x9)
				MOD_PHYREG_ENTRY(pi, miscSigCtrl, force_1st_sigqual_bpsk, 1)
			  ACPHY_REG_LIST_EXECUTE(pi);
			} else {
			  ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, PktAbortSupportedStates,
				       ((AC4354REV(pi)) ? 0x2ab7 : 0x2ff7))
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0, 0x1f)
			  ACPHY_REG_LIST_EXECUTE(pi);
			}
			if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
				ACPHYREG_BCAST(pi, Core0cckcomputeGainInfo, 0x6);
				wlc_phy_preemption_abort_during_timing_search(pi,
					(phy_ac_btcx_get_data(ni->ac_info->btcxi)->btc_mode == 0) ||
					(phy_ac_btcx_get_data(ni->ac_info->btcxi)->btc_mode == 2));
			}
			if (PHY_ILNA(pi)) {
				/* 43569/43570 iLNA */
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, BphyAbortExitCtrl, 0x3840)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_per_pkt_en0, 0x21)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0,
					0xffff)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_th0,
					0xffff)
					ACPHYREG_BCAST_ENTRY(pi,
					PREMPT_ofdm_large_gain_mismatch_th0, 0x1f)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_cnt_th0,
					0x30)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else {
				ACPHY_REG_LIST_START
					WRITE_PHYREG_ENTRY(pi, BphyAbortExitCtrl, 0x3840)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_per_pkt_en0, 0x21)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0,
					0xffff)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_th0,
					0xffff)
					ACPHYREG_BCAST_ENTRY(pi,
					PREMPT_ofdm_nominal_clip_th_xtra_bits0, 0x3)
					ACPHYREG_BCAST_ENTRY(pi,
					PREMPT_ofdm_large_gain_mismatch_th0, 0x1f)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_cnt_th0,
					0x30)
				ACPHY_REG_LIST_EXECUTE(pi);
				if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
					ACPHYREG_BCAST(pi, PREMPT_cck_nominal_clip_cnt_th0,
					0x32);
				}
			}
			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (PHY_ILNA(pi)) {
					if (CHSPEC_IS20(pi->radio_chanspec)) {
						//per_pkt_en0 |= clip_detect_cond2_enable;
						ACPHY_REG_LIST_START
							MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							clip_detect_cond2_enable, 1)
							ACPHYREG_BCAST_ENTRY(pi,
							PREMPT_ofdm_large_gain_mismatch_th0, 0xf)
							ACPHYREG_BCAST_ENTRY(pi,
							PREMPT_ofdm_nominal_clip_cnt_th0, 0x30)
						ACPHY_REG_LIST_EXECUTE(pi);
					} else if (CHSPEC_IS40(pi->radio_chanspec)) {
						ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0,
						0x48);
					} else {
						ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0,
						0xa0);
					}
				} else {
					ACPHYREG_BCAST(pi,
					PREMPT_per_pkt_en0, 0x31);
					ACPHYREG_BCAST(pi, PREMPT_ofdm_large_gain_mismatch_th0,
						(IS_4364_3x3(pi) ? 0xf : 0x09));
					if (CHSPEC_IS20(pi->radio_chanspec)) {
						ACPHYREG_BCAST(pi,
						PREMPT_ofdm_nominal_clip_cnt_th0, 0x24);
					} else if (CHSPEC_IS40(pi->radio_chanspec)) {
						ACPHYREG_BCAST(pi,
						PREMPT_ofdm_nominal_clip_cnt_th0, 0x48);
					} else {
						ACPHYREG_BCAST(pi,
						PREMPT_ofdm_nominal_clip_cnt_th0, 0xa0);
					}
				}
			} else {
				if (CHSPEC_IS20(pi->radio_chanspec)) {

					if (PHY_ILNA(pi)) {
					  ACPHY_REG_LIST_START
						MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
						clip_detect_cond2_enable, 1)
					    ACPHYREG_BCAST_ENTRY(pi,
					      PREMPT_ofdm_nominal_clip_cnt_th0, 0x30)
					    ACPHYREG_BCAST_ENTRY(pi,
					      PREMPT_ofdm_large_gain_mismatch_th0, 0xf)
					  ACPHY_REG_LIST_EXECUTE(pi);
					} else {
						ACPHY_REG_LIST_START
							ACPHYREG_BCAST_ENTRY(pi,
							PREMPT_per_pkt_en0, 0x31)
							ACPHYREG_BCAST_ENTRY(pi,
							PREMPT_ofdm_nominal_clip_cnt_th0, 0x24)
							ACPHYREG_BCAST_ENTRY(pi,
							PREMPT_ofdm_large_gain_mismatch_th0, 0x9)
						ACPHY_REG_LIST_EXECUTE(pi);
					}
				} else {
					ACPHYREG_BCAST(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x48);
				}
			}
			if ((pi->sh->interference_mode & ACPHY_LPD_PREEMPTION) != 0) {
				/* enable low power detect preemption and set thresholds */
				ACPHY_REG_LIST_START
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, low_power_enable,
					1)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_low_power_mismatch_th0,
					0x1f)
					ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_low_power_mismatch_th0,
					0x1f)
				ACPHY_REG_LIST_EXECUTE(pi);
			}
			/* Enable Preemption */
		} else {
			ACPHY_REG_LIST_START
				/* disable Preempt */
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 0)
				MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 0)
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x0)
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en1, 0x0)
				MOD_PHYREG_ENTRY(pi, miscSigCtrl, force_1st_sigqual_bpsk, 0)
			ACPHY_REG_LIST_EXECUTE(pi);
		    if (ACMINORREV_2(pi))
			    WRITE_PHYREG(pi, PREMPT_per_pkt_en2, 0);
		}

	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* 4349 preemption settings */
		if (enable_preempt) {
			if (phy_get_phymode(pi) != PHYMODE_RSDB) {
				WRITE_PHYREG(pi, PktAbortCtrl, 0x1041);
			} else {
				WRITE_PHYREG(pi, PktAbortCtrl, 0x1841);
			}
			FOREACH_CORE(pi, core) {
				MOD_PHYREGC(pi, _BPHY_TargetVar_log2_pt8us,
					core, bphy_targetVar_log2_pt8us, 479);
			}
			ACPHY_REG_LIST_START
				WRITE_PHYREG_ENTRY(pi, RxMacifMode, 0x0a00)
				WRITE_PHYREG_ENTRY(pi, PktAbortSupportedStates, 0x2bbf)
				WRITE_PHYREG_ENTRY(pi, BphyAbortExitCtrl, 0x3840)
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x21)
				WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0, 0xffff)
				WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_large_gain_mismatch_th0, 0x1f)
				WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_th0, 0xffff)
				WRITE_PHYREG_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0, 0x1f)
				WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_cnt_th0, 0x38)
			ACPHY_REG_LIST_EXECUTE(pi);
			if (!ACMAJORREV_36(pi->pubpi->phy_rev)) {
				if (phy_get_phymode(pi) != PHYMODE_RSDB) {
					ACPHY_REG_LIST_START
						WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en1, 0x21)
						WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_nominal_clip_th1,
							0xffff)
						WRITE_PHYREG_ENTRY(pi,
							PREMPT_ofdm_large_gain_mismatch_th1, 0x1f)
						WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_th1,
							0xffff)
						WRITE_PHYREG_ENTRY(pi,
							PREMPT_cck_large_gain_mismatch_th1, 0x1f)
						WRITE_PHYREG_ENTRY(pi,
							PREMPT_cck_nominal_clip_cnt_th1, 0x38)
					ACPHY_REG_LIST_EXECUTE(pi);
				}
			}

			if (CHSPEC_IS5G(pi->radio_chanspec)) {
				if (CHSPEC_BW_LE20(pi->radio_chanspec)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
					if (phy_get_phymode(pi) != PHYMODE_RSDB)
						WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th1,
							0x28);
				} else if (CHSPEC_IS40(pi->radio_chanspec)) {
				    WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x50);
			        if (phy_get_phymode(pi) != PHYMODE_RSDB)
						WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th1,
							0x50);
				} else {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0xb0);
			        if (phy_get_phymode(pi) != PHYMODE_RSDB)
						WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th1,
							0xb0);
				}
			} else {
				WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
				if (phy_get_phymode(pi) != PHYMODE_RSDB)
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th1,
						0x28);
				if ((CHSPEC_IS40(pi->radio_chanspec)) &&
				    (phy_get_phymode(pi) != PHYMODE_RSDB)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x50);
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th1, 0x50);
			    }
			}
			/* 4349B0: Disable SSAGC after pktabort & Power post RxFilter */
			MOD_PHYREG(pi, PktAbortCounterClr, ssagc_pktabrt_enable, 0);
			MOD_PHYREG(pi, PktAbortCounterClr,
				mux_post_rxfilt_power_for_abrt, 0);
#ifdef OCL
			if (PHY_OCL_ENAB(pi->sh->physhim)) {
				phy_ac_ocl_data_t *ocl_data =
						phy_ac_ocl_get_data(ni->ac_info->ocli);
				if (!ocl_data->ocl_disable_reqs) {
					if (phy_stf_get_data(pi->stfi)->phyrxchain == 3) {
						/* Disable preemption on sleeping core */
						if (ocl_data->ocl_coremask == 1) {
							WRITE_PHYREG(pi, PREMPT_per_pkt_en1, 0);
						} else {
							WRITE_PHYREG(pi, PREMPT_per_pkt_en0, 0);
						}
					}
				}
			}
#endif // endif

		} else {
			ACPHY_REG_LIST_START
				/* Disable Preempt */
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 0)
				MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 0)
				WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x0)
			ACPHY_REG_LIST_EXECUTE(pi);
			if (phy_get_phymode(pi) != PHYMODE_RSDB)
				WRITE_PHYREG(pi, PREMPT_per_pkt_en1, 0x0);
		}
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* 43012 preemption settings */
		if (enable_preempt) {
			if (EnablePostRxFilter_Proc) {
				ACPHY_REG_LIST_START
					/* If Post Rx Filter Processing is enabled */
					MOD_PHYREG_ENTRY(pi, PktAbortCounterClr,
							mux_post_rxfilt_power_for_abrt, 1)
					MOD_PHYREG_ENTRY(pi, PktAbortCounterClr,
							ssagc_pktabrt_enable, 0)
					MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 1)
					WRITE_PHYREG_ENTRY(pi, PktAbortSupportedStates, 0x2bbf)
					/* Mode1: Nominal gain clip detect */
					/*	Cond1: Norm Pwr > Nominal clip threshold */
					/*	Cond2: Gain Needed < Large gain mismatch thres */
					/*	Cond3: ADC clip count threshold - > Not impltd */
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							clip_detect_enable, 1)
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							clip_detect_cond1_enable, 1)
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							clip_detect_cond2_enable, 1)
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							clip_detect_cond3_enable, 0)
					/*	Con1 threshold - I2 +Q2 value (max of 65535) -
						Moving average value over 0.8us
					*/
					WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0, 43000)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_th0, 43000)
					/*	Cond2 threshold - Gain Mismatch */
					WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_large_gain_mismatch_th0,
							20)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0,
							30)

					/* Mode2: Low Pwr Detect */
					/*	Gain needed > Low power mismatch threshold */
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, low_power_enable,
							0)
					WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_low_power_mismatch_th0,
							28)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_low_power_mismatch_th0,
							28)

					/* Mode3: Pkt Rx Pwr Variations */
					/*	Gain Needed > max_gain_mismatch_pkt_rcv_th */
					/*	Gain needed < min_gain_mismatch_pkt_rcv_th */
					/* These variations observed over consecutive
					'n' intervals then abort
					*/
					MOD_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0,
							pwr_variation_enable, 1)
					WRITE_PHYREG_ENTRY(pi,
							PREMPT_ofdm_max_gain_mismatch_pkt_rcv_th0,
							28)
					WRITE_PHYREG_ENTRY(pi,
							PREMPT_ofdm_min_gain_mismatch_pkt_rcv_th0,
							8)
					WRITE_PHYREG_ENTRY(pi,
							PREMPT_cck_max_gain_mismatch_pkt_rcv_th0,
							28)
					WRITE_PHYREG_ENTRY(pi,
							PREMPT_cck_min_gain_mismatch_pkt_rcv_th0,
							8)
					/* Actually value to be set is 32, but since the
					polling clock fixed at 10MHz, using scaled value
					*/
					MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PwrVariationThCCK, 4)
					MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PwrVariationThOFDM, 4)

					MOD_PHYREG_ENTRY(pi, Core0_BPHY_TargetVar_log2_pt8us,
							bphy_targetVar_log2_pt8us, 479)
					/* Exit classifier state ona pre-emption of Bphy */
					MOD_PHYREG_ENTRY(pi, BphyAbortExitCtrl, CckStExitonAbrtEn,
							0)
					MOD_PHYREG_ENTRY(pi, BphyAbortExitCtrl, bphyAbrtTimeOutCtr,
							32)
					MOD_PHYREG_ENTRY(pi, BphyAbortExitCtrl, bphyIdleOnAbortEn,
							1)
					MOD_PHYREG_ENTRY(pi, BphyAbortExitCtrl,
							bphyIdleForPreDetDis, 1)
					/* Packet Abort enable */
					WRITE_PHYREG_ENTRY(pi, RxMacifMode, 0xa00)
					MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 1)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else {
				/* If ADC samples are used for processing */
				ACPHY_REG_LIST_START
					MOD_PHYREG_ENTRY(pi, Core0_BPHY_TargetVar_log2_pt8us,
							bphy_targetVar_log2_pt8us, 479)
					MOD_PHYREG_ENTRY(pi, PktAbortCounterClr,
							mux_post_rxfilt_power_for_abrt, 0)
					WRITE_PHYREG_ENTRY(pi, PktAbortCtrl, 0x1841)
					WRITE_PHYREG_ENTRY(pi, RxMacifMode, 0x0a00)
					WRITE_PHYREG_ENTRY(pi, BphyAbortExitCtrl, 0x3840)
					WRITE_PHYREG_ENTRY(pi, PREMPT_per_pkt_en0, 0x21)
					WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0, 0xffff)
					WRITE_PHYREG_ENTRY(pi, PREMPT_ofdm_large_gain_mismatch_th0,
							0x1f)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_th0, 0xffff)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0,
							0x1f)
					WRITE_PHYREG_ENTRY(pi, PREMPT_cck_nominal_clip_cnt_th0,
							0x38)
				ACPHY_REG_LIST_EXECUTE(pi);
				if (CHSPEC_IS5G(pi->radio_chanspec)) {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
				} else {
					WRITE_PHYREG(pi, PREMPT_ofdm_nominal_clip_cnt_th0, 0x28);
				}
			}
		} else {
			/* Disable Preempt */
			MOD_PHYREG(pi, RxMacifMode, AbortStatusEn, 0);
			MOD_PHYREG(pi, PktAbortCtrl, PktAbortEn, 0);
			WRITE_PHYREG(pi, PREMPT_per_pkt_en0, 0x0);
		}
	} else if ((ACMAJORREV_32(pi->pubpi->phy_rev) && !ACMINORREV_0(pi)) ||
	           ACMAJORREV_33(pi->pubpi->phy_rev) || ACMAJORREV_37(pi->pubpi->phy_rev)) {
		/* 4365B0 PREEMPTion SETTINGs */
		if (enable_preempt) {
			WRITE_PHYREG(pi, PktAbortCtrl, 0xf041);
			FOREACH_CORE(pi, core) {
				MOD_PHYREGC(pi, _BPHY_TargetVar_log2_pt8us,
					core, bphy_targetVar_log2_pt8us, 479);
			}
			WRITE_PHYREG(pi, RxMacifMode, 0x0a00);

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				WRITE_PHYREG(pi, PktAbortSupportedStates, 0x2a3f);
			} else {
				WRITE_PHYREG(pi, PktAbortSupportedStates, 0x2bbf);
			}
			if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				WRITE_PHYREG(pi, PktAbortSupportedStates, 0x2fff);
			}

			WRITE_PHYREG(pi, BphyAbortExitCtrl, 0x3840);
			WRITE_PHYREG(pi, PktAbortCounterClr, 0x18);
			/* Reduces -42 dbm humps in jammer */
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				FOREACH_CORE(pi, core) {
					WRITE_PHYREGC(pi, Clip2Threshold, core, 0xb8b8);
				}
			}
			stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
			ACPHY_DISABLE_STALL(pi);
			phy_ac_noise_preempt_postfilter_reg_tbl(ni, TRUE);
			ACPHY_ENABLE_STALL(pi, stall_val);

		} else {
			/* Disable Preempt */
			MOD_PHYREG(pi, RxMacifMode, AbortStatusEn, 0);
			MOD_PHYREG(pi, PktAbortCtrl, PktAbortEn, 0);
			phy_ac_noise_preempt_postfilter_reg_tbl(ni, FALSE);
		}
	} else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		/* 43684 PREEMPTiON SETTINGs */
		if (enable_preempt) {
			WRITE_PHYREG(pi, PktAbortCtrl, 0xf041);
			FOREACH_CORE(pi, core) {
				MOD_PHYREGC(pi, _BPHY_TargetVar_log2_pt8us,
					core, bphy_targetVar_log2_pt8us, 479);
			}
			WRITE_PHYREG(pi, RxMacifMode, 0x0a00);
			WRITE_PHYREG(pi, PktAbortSupportedStates, 0x2fff);
			WRITE_PHYREG(pi, BphyAbortExitCtrl, 0x3840);
			WRITE_PHYREG(pi, PktAbortCounterClr, 0x18);

			stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
			ACPHY_DISABLE_STALL(pi);
			phy_ac_noise_preempt_postfilter_reg_tbl(ni, TRUE);
			ACPHY_ENABLE_STALL(pi, stall_val);
		} else {
			/* Disable Preempt */
			MOD_PHYREG(pi, RxMacifMode, AbortStatusEn, 0);
			MOD_PHYREG(pi, PktAbortCtrl, PktAbortEn, 0);
			phy_ac_noise_preempt_postfilter_reg_tbl(ni, FALSE);
		}
	} else if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		if (enable_preempt) {
			/* 4347A0 Preemption settings */
			ACPHY_REG_LIST_START
				/* General preemption registers */
				MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 1)
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 1)
				WRITE_PHYREG_ENTRY(pi, PktAbortSupportedStates, 0x20bf)
				/* Enable timeout */
				WRITE_PHYREG_ENTRY(pi, timeoutEn, 0x0817)
			ACPHY_REG_LIST_EXECUTE(pi);

			if (ACMAJORREV_44_46(pi->pubpi->phy_rev)) {
				/* CRDOT11ACPHY-1792: Over-rides and new logic for gain cmd choice
				 * for rx pwr variation preemption
				 * pwrVarAbrtGainCmd=0 ==> setting the gain to "init gain" after
				 * pkt abort
				 */
				WRITE_PHYREG(pi, PktAbortCounterClr, 0x558);

				/* CRDOT11ACPHY-2232: disabling init gain transition during
				 * WAIT_ENERGY_DROP state
				 */
				MOD_PHYREG(pi, clip_detect_normpwr_var_mux,
					disable_init_gain_in_wed, 1);
			} else {
				WRITE_PHYREG(pi, PktAbortCounterClr, 0x1D58);
			}

			if (ACMAJORREV_40(pi->pubpi->phy_rev) ||
				(ACMAJORREV_44(pi->pubpi->phy_rev) &&
				pi->pubpi->slice == DUALMAC_AUX)) {
				/* Bphy related setting */
				WRITE_PHYREG(pi, BphyAbortExitCtrl, 0x3840);
			}

			ACPHY_REG_LIST_START
				/* Thresholds */
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_per_pkt_en0, 0x99)
				ACPHYREG_BCAST_ENTRY(pi,
					PREMPT_ofdm_nominal_clip_th_hipwr_xtra_bits0, 0x1403)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_low_power_mismatch_th0, 24)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_low_power_mismatch_th0, 24)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_nominal_clip_th0, 28000)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_th0, 40000)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_nominal_clip_th_xtra_bits0, 0)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_large_gain_mismatch_th0, 12)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_large_gain_mismatch_th0, 15)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_ofdm_nominal_clip_th_hipwr0, 65535)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_cck_nominal_clip_th_hipwr0, 65535)
			ACPHY_REG_LIST_EXECUTE(pi);
		} else {
			ACPHY_REG_LIST_START
				/* Disable Preempt */
				MOD_PHYREG_ENTRY(pi, RxMacifMode, AbortStatusEn, 0)
				MOD_PHYREG_ENTRY(pi, PktAbortCtrl, PktAbortEn, 0)
				ACPHYREG_BCAST_ENTRY(pi, PREMPT_per_pkt_en0, 0)
			ACPHY_REG_LIST_EXECUTE(pi);
		}
	}

	if (D11REV_GE(pi->sh->corerev, 47)) {
		ni->data->pktabortctl = READ_PHYREG(pi, PktAbortCtrl);

		/* Don't call this unless this war is ready/tested */
		//phy_btcx_set_mode((wlc_phy_t *) pi, wlapi_bmac_btc_mode_get(pi->sh->physhim));
	}
}

static int8
phy_ac_noise_lte_avg(phy_type_noise_ctx_t *ctx)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	int tot = 0;

	tot = noise_info->noise_lte;
	noise_info->noise_lte = 0;
	return (int8)tot;
}

static int
phy_ac_noise_get_srom_level(phy_type_noise_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_ac_noise_info_t *noisei = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = noisei->pi;
	int8 noiselvl[PHY_CORE_MAX] = {0};
	uint8 core, core_freq_segment_map;
	uint16 channel;
	uint32 pkd_noise = 0;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	FOREACH_CORE(pi, core) {

		/* For 80P80, retrieve Primary/Secondary based on the mapping */
		if (CHSPEC_IS8080(pi->radio_chanspec)) {
			core_freq_segment_map = phy_ac_chanmgr_get_data
				(noisei->ac_info->chanmgri)->core_freq_mapping[core];
			if (PRIMARY_FREQ_SEGMENT == core_freq_segment_map) {
				channel = wf_chspec_primary80_channel(pi->radio_chanspec);
			} else if (SECONDARY_FREQ_SEGMENT == core_freq_segment_map) {
				channel = wf_chspec_secondary80_channel(pi->radio_chanspec);
			}
		}

		if (channel <= 14) {
			/* 2G */
			noiselvl[core] = pi->noiselvl_2g[core];
		} else {
#ifdef BAND5G
			/* 5G */
			if (channel <= 48) {
				/* 5G-low: channels 36 through 48 */
				noiselvl[core] = pi->noiselvl_5gl[core];
			} else if (channel <= 64) {
				/* 5G-mid: channels 52 through 64 */
				noiselvl[core] = pi->noiselvl_5gm[core];
			} else if (channel <= 128) {
				/* 5G-high: channels 100 through 128 */
				noiselvl[core] = pi->noiselvl_5gh[core];
			} else {
				/* 5G-upper: channels 132 and above */
				noiselvl[core] = pi->noiselvl_5gu[core];
			}
#endif /* BAND5G */
		}
	}

	for (core = PHYCORENUM(pi->pubpi->phy_corenum); core >= 1; core--) {
		pkd_noise = (pkd_noise << 8) | (uint8)(noiselvl[core-1]);
	}
	*ret_int_ptr = pkd_noise;

	return BCME_OK;
}

static int8
phy_ac_noise_read_shmem(phy_type_noise_ctx_t *ctx, uint8 *lte_on, uint8 *crs_high)
{
	phy_ac_noise_info_t *noise_info = (phy_ac_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	uint16 lo, hi;
	uint32 cmplx_pwr_tot = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	uint8 core;
	uint8 noise_during_crs = 0;
	uint8 noise_during_lte = 0;

	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

	if (pi->cal_info->phy_forcecal_request && ISACPHY(pi)) {
		/* Remove Rx gain override
		    RxGain was overriden when requesting noise measurement
		*/
		pi->cal_info->phy_forcecal_request = FALSE;
		if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			phy_ac_ovr_rxgain_noise_sample_request(pi->u.pi_acphy->rxgcrsi, FALSE);
		}
	}

	if (phy_noise_abort_shmem_read(noisei)) {
		return -1;
	}

	if (ISACPHY(pi)) {
		/* Restore the default value . Required if noise cal is triggered from cals */
		pi->cal_info->ignore_crs_status = FALSE;
	}

	/* read SHM, reuse old corerev PWRIND since we are tight on SHM pool */
	FOREACH_CORE(pi, core) {
		lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(pi, 2*core));
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(pi, 2*core+1));

		noise_during_crs =
			(wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP1(pi)) >> 15) & 0x1;
		noise_during_lte =
			(wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP1(pi)) >> 14) & 0x1;
		if (core == 0) {
			hi = hi & ~(3<<14); /* Clear bit 14 and 15 */
		}
		cmplx_pwr[core] = (hi << 16) + lo;
	}
#ifdef OCL
	if (PHY_OCL_ENAB(pi->sh->physhim)) {
		uint8 active_cm;
		bool ocl_en;
		phy_ocl_status_get((wlc_phy_t *)pi, NULL, &active_cm, &ocl_en);
		/* copying active core noise to inactive core when ocl_en=1 */
		if (ocl_en)
			cmplx_pwr[!(active_cm >> 1)] = cmplx_pwr[(active_cm >> 1)];
	}
#endif // endif
	FOREACH_CORE(pi, core) {
		cmplx_pwr_tot += cmplx_pwr[core];
		if (cmplx_pwr[core] == 0) {
			noise_dbm_ant[core] = PHY_NOISE_FIXED_VAL_NPHY;
		} else {
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev)) {
				cmplx_pwr[core] >>= PHY_NOISE_SAMPLE_LOG_NUM_UCODE;
			} else {
				/* Compute rounded average complex power */
				cmplx_pwr[core] = (cmplx_pwr[core] >>
					PHY_NOISE_SAMPLE_LOG_NUM_UCODE) +
					((cmplx_pwr[core] >>
					(PHY_NOISE_SAMPLE_LOG_NUM_UCODE - 1)) & 0x1);
			}
		}
	}

	if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev)) {
		/* FIX ME: this is temp WAR for the 4th core crsmin cal */
		cmplx_pwr[3] = cmplx_pwr[1];
	}

	if (cmplx_pwr_tot == 0)
		PHY_INFORM(("wlc_phy_noise_sample_nphy_compute: timeout in ucode\n"));
	else {
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
		/* Putting check here to see if crsmin cal needs to be triggered */
		if ((pi->phynoise_state == PHY_NOISE_STATE_CRSMINCAL) ||
			noise_info->data->trigger_crsmin_cal) {
			(void) phy_ac_rxgcrs_min_pwr_cal(pi->u.pi_acphy->rxgcrsi, PHY_CRS_RUN_AUTO);
		}
	}

	if ((!noise_during_lte) && (!noise_during_crs)) {
		wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);
	} else {
		noise_dbm = noise_dbm_ant[0];
	}

	/* legacy noise is the max of antennas */
	*lte_on = noise_during_lte;
	*crs_high = noise_during_crs;
	return noise_dbm;

}
