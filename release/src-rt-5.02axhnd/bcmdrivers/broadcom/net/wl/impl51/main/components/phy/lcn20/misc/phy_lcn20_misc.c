/*
 * LCN20PHY Miscellaneous modules implementation
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
 * $Id: phy_lcn20_misc.c 671526 2016-11-22 08:37:30Z $
 */

#include <typedefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_btcx.h>
#include <phy_misc.h>
#include <phy_type_misc.h>
#include <phy_lcn20_misc.h>
#include <phy_utils_reg.h>
#include <wlc_radioreg_20692.h>
#include <phy_calmgr_api.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
#include <wlc_phyreg_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

#define RADIO_REG_2069X(pi, id, regnm, core)	RF##core##_##id##_##regnm(pi->pubpi->radiorev)
#define RADIO_REG_20692(pi, regnm, core)	RADIO_REG_2069X(pi, 20692, regnm, 0)
#define _MOD_RADIO_REG(pi, reg, mask, val)	phy_utils_mod_radioreg(pi, reg, mask, val)
#define MOD_RADIO_REG_2069X(pi, id, regnm, core, fldname, value) \
		_MOD_RADIO_REG(pi, \
			RADIO_REG_##id(pi, regnm, core), \
			RF_##id##_##regnm##_##fldname##_MASK(pi->pubpi->radiorev), \
			((value) << RF_##id##_##regnm##_##fldname##_SHIFT(pi->pubpi->radiorev)))

#define MOD_RADIO_REG_20692(pi, regnm, core, fldname, value) \
		MOD_RADIO_REG_2069X(pi, 20692, regnm, core, fldname, value)

/* module private states */
struct phy_lcn20_misc_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_misc_info_t *cmn_info;
};

#ifdef PHY_DUMP_BINARY
/* AUTOGENRATED by the tool : phyreg.py
 * These values cannot be in const memory since
 * the const area might be over-written in case of
 * crash dump
 */
phyradregs_list_t dot11lcn20phy_regs_rev1[] = {
	{0x000,  {0x3, 0xf, 0x66, 0xbb}},
	{0x027,  {0x7f, 0x6, 0x76, 0x1}},
	{0x046,  {0x3, 0x80, 0x32, 0xff}},
	{0x068,  {0x0, 0x0, 0x7, 0x3f}},
	{0x304,  {0x60, 0x7f, 0xf0, 0x1f}},
	{0x323,  {0x0, 0x0, 0x0, 0x1f}},
	{0x401,  {0x66, 0x1, 0xbf, 0xd9}},
	{0x420,  {0x7f, 0xc5, 0xd2, 0xff}},
	{0x440,  {0x7f, 0xce, 0xff, 0xbf}},
	{0x45f,  {0x7f, 0xff, 0xed, 0xff}},
	{0x47e,  {0x7d, 0xfc, 0x1f, 0xff}},
	{0x49d,  {0x7f, 0x3a, 0xff, 0xff}},
	{0x4bc,  {0x78, 0xf0, 0x0, 0x1}},
	{0x4db,  {0x40, 0xb7, 0x7f, 0xff}},
	{0x4fa,  {0x3, 0x1d, 0xff, 0x9f}},
	{0x524,  {0x43, 0xff, 0x20, 0xf}},
	{0x543,  {0x0, 0x3, 0xfd, 0x11}},
	{0x564,  {0x0, 0x33, 0xf0, 0x3}},
	{0x594,  {0x70, 0x0, 0x7, 0xff}},
	{0x5b3,  {0x48, 0x7f, 0xff, 0x7f}},
	{0x5d2,  {0x4f, 0xff, 0xc0, 0x3f}},
	{0x5f1,  {0xf, 0xff, 0xff, 0xff}},
	{0x62a,  {0x7f, 0xff, 0x7d, 0xbf}},
	{0x649,  {0x80, 0x0, 0x0, 0x39}},
	{0x687,  {0x7c, 0x4f, 0xc0, 0x37}},
	{0x6a6,  {0x70, 0x4, 0x0, 0xff}},
	{0x6c5,  {0xb, 0xff, 0xfb, 0xf9}},
	{0x775,  {0x0, 0xd, 0x0, 0x1}},
	{0x7c3,  {0x6f, 0xff, 0xe0, 0x1}},
	{0x7e2,  {0x41, 0x90, 0x1f, 0xff}},
	{0x801,  {0x0, 0x7f, 0x81, 0xfd}},
	{0x821,  {0x1, 0xfd, 0x81, 0xff}},
	{0x840,  {0x3, 0xff, 0x3, 0xff}},
	{0x860,  {0x0, 0x0, 0x3f, 0xff}},
	{0x880,  {0x3, 0xff, 0xff, 0xff}},
	{0x900,  {0x40, 0x7, 0xff, 0xff}},
	{0x91f,  {0x7e, 0xe, 0x7, 0xff}},
	{0x93e,  {0xf, 0xfc, 0x3f, 0xff}},
	{0x960,  {0x7, 0xff, 0xff, 0xe3}},
	{0x97f,  {0x7, 0xfe, 0x7, 0xff}},
	{0x9a0,  {0x0, 0x1f, 0xf7, 0xff}},
	{0x9cd,  {0x0, 0xf, 0xff, 0xff}},
	{0x9f4,  {0x1f, 0xff, 0x10, 0x79}},
	{0xa13,  {0x40, 0xef, 0xff, 0xff}},
	{0xa32,  {0x7f, 0xbf, 0xff, 0xa7}},
	{0xa51,  {0x80, 0x0, 0x0, 0x20}},
	{0xa7f,  {0xf, 0xff, 0xff, 0xc7}},
	{0xaa2,  {0x7f, 0x20, 0x60, 0x1}},
	{0xac1,  {0x80, 0x0, 0x0, 0x23}},
	{0xaed,  {0x7f, 0xf8, 0x1, 0xff}},
	{0xb0c,  {0x80, 0x0, 0x0, 0x22}},
	{0xbe0,  {0x0, 0x0, 0x0, 0xf}},
};
#endif /* PHY_DUMP_BINARY */

/*  Local Functions */
#if defined(BCMDBG) || defined(WLTEST)
static void phy_lcn20_init_test(phy_type_misc_ctx_t *ctx, bool encals);
#if (defined(LCN20CONF) && (LCN20CONF != 0))
static int phy_lcn20_misc_test_freq_accuracy(phy_type_misc_ctx_t *ctx, int channel);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
#endif /* defined(BCMDBG) || defined(WLTEST) */
#ifdef PHY_DUMP_BINARY
static int phy_lcn20_misc_getlistandsize(phy_type_misc_ctx_t *ctx, phyradregs_list_t **phyreglist,
	uint16 *phyreglist_sz);
#endif // endif
static uint32 phy_lcn20_rx_iq_est(phy_type_misc_ctx_t *ctx, uint8 samples, uint8 antsel,
	uint8 resolution, uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                                uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type);
static void phy_lcn20_iovar_tx_tone(phy_type_misc_ctx_t *ctx, int32 int_val);

static void phy_lcn20_iovar_txlo_tone(phy_type_misc_ctx_t *ctx);
static int phy_lcn20_iovar_get_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 *ret_int_ptr,
	int32 int_val, int err);
static int phy_lcn20_iovar_set_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 int_val, int err);
#if (defined(LCN20CONF) && (LCN20CONF != 0))
static void phy_lcn20_misc_deaf_mode(phy_type_misc_ctx_t *ctx, bool user_flag);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */

#ifdef ATE_BUILD
static void wlc_phy_gpaio_lcn20phy(phy_type_misc_ctx_t *ctx, wl_gpaio_option_t option, int core);
#endif // endif

static void phy_update_rxldpc_lcn20phy(phy_type_misc_ctx_t *ctx, bool ldpc);
static int phy_lcn20_misc_set_lo_gain_nbcal(phy_type_misc_ctx_t *ctx, bool lo_gain);

/* register phy type specific implementation */
phy_lcn20_misc_info_t *
BCMATTACHFN(phy_lcn20_misc_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_misc_info_t *cmn_info)
{
	phy_lcn20_misc_info_t *lcn20_info;
	phy_type_misc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((lcn20_info = phy_malloc(pi, sizeof(phy_lcn20_misc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	lcn20_info->pi = pi;
	lcn20_info->lcn20i = lcn20i;
	lcn20_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = lcn20_info;
#if defined(BCMDBG) || defined(WLTEST)
	fns.phy_type_misc_test_init = phy_lcn20_init_test;
#endif /* defined(BCMDBG) || defined(WLTEST) */
	fns.phy_type_misc_rx_iq_est = phy_lcn20_rx_iq_est;
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	fns.phy_type_misc_set_deaf = phy_lcn20_misc_deaf_mode;
	fns.phy_type_misc_clear_deaf = phy_lcn20_misc_deaf_mode;
#if defined(BCMDBG) || defined(WLTEST)
	fns.phy_type_misc_test_freq_accuracy = phy_lcn20_misc_test_freq_accuracy;
#endif /* defined(BCMDBG) || defined(WLTEST) */
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
#ifdef PHY_DUMP_BINARY
	fns.phy_type_misc_getlistandsize = phy_lcn20_misc_getlistandsize;
#endif // endif
	fns.phy_type_misc_iovar_tx_tone = phy_lcn20_iovar_tx_tone;
	fns.phy_type_misc_iovar_txlo_tone = phy_lcn20_iovar_txlo_tone;
	fns.phy_type_misc_iovar_get_rx_iq_est = phy_lcn20_iovar_get_rx_iq_est;
	fns.phy_type_misc_iovar_set_rx_iq_est = phy_lcn20_iovar_set_rx_iq_est;
#ifdef ATE_BUILD
	fns.gpaioconfig = wlc_phy_gpaio_lcn20phy;
#endif // endif
	fns.set_ldpc_override = phy_update_rxldpc_lcn20phy;
	fns.set_lo_gain_nbcal = phy_lcn20_misc_set_lo_gain_nbcal;

	if (phy_misc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_misc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return lcn20_info;

	/* error handling */
fail:
	if (lcn20_info != NULL)
		phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_misc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_misc_unregister_impl)(phy_lcn20_misc_info_t *lcn20_info)
{
	phy_info_t *pi;
	phy_misc_info_t *cmn_info;

	ASSERT(lcn20_info);
	pi = lcn20_info->pi;
	cmn_info = lcn20_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_misc_unregister_impl(cmn_info);

	phy_mfree(pi, lcn20_info, sizeof(phy_lcn20_misc_info_t));
}

/* ********************************************** */
/* Function table registred function */
/* ********************************************** */
#if defined(BCMDBG) || defined(WLTEST)
static void
phy_lcn20_init_test(phy_type_misc_ctx_t *ctx, bool encals)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);
	/* Disable tx power control */
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);
	/* Recalibrate for this channel */
	wlc_lcn20phy_calib_modes(pi, PHY_FULLCAL);
}

#if (defined(LCN20CONF) && (LCN20CONF != 0))
static int
phy_lcn20_misc_test_freq_accuracy(phy_type_misc_ctx_t *ctx, int channel)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	if (channel == 0) {
		wlc_lcn20phy_stop_tx_tone(pi);
		wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_HW);
	} else {
		wlc_lcn20phy_set_tx_pwr_by_index(pi, 55);
		wlc_lcn20phy_start_tx_tone(pi, 0, 112, 0, TRUE, 0);
	}

	return BCME_OK;
}
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
#endif /* defined(BCMDBG) || defined(WLTEST) */

static uint32 phy_lcn20_rx_iq_est(phy_type_misc_ctx_t *ctx, uint8 samples, uint8 antsel,
	uint8 resolution, uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                                uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type) {
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_iq_est_t est[PHY_CORE_MAX];
	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	int16	tot_gain[PHY_CORE_MAX];
	int16 noise_dBm_ant_fine[PHY_CORE_MAX];
#if defined(WLTEST)
	uint16 log_num_samps;
	uint16 num_samps;
	uint8 wait_time = 32;
	uint8 i, extra_gain_1dB = 0;
	uint32 result = 0;
#endif /* #if defined(WLTEST) */
	bool sampling_in_progress = (pi->phynoise_state != 0);
	uint16 crsmin_pwr[PHY_CORE_MAX];

	if (sampling_in_progress) {
		PHY_ERROR(("%s: sampling_in_progress\n", __FUNCTION__));

		return 0;
	}

	/* Extra INITgain is supported only for HTPHY currently */
	if (extra_gain_3dB > 0) {
		extra_gain_3dB = 0;
		PHY_ERROR(("%s: Extra INITgain not supported for this phy.\n",
		           __FUNCTION__));
	}

	pi->phynoise_state |= PHY_NOISE_STATE_MON;
#if defined(WLTEST)
	/* choose num_samps to be some power of 2 */
	log_num_samps = samples;
	num_samps = 1 << log_num_samps;
#endif /* #if defined(WLTEST) */

	bzero((uint8 *)est, sizeof(est));
	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));
	bzero((uint16 *)crsmin_pwr, sizeof(crsmin_pwr));
	bzero((uint16 *)noise_dBm_ant_fine, sizeof(noise_dBm_ant_fine));
	bzero((int16 *)tot_gain, sizeof(tot_gain));

	/* get IQ power measurements */
#if defined(WLTEST)
	wlc_lcn20phy_rx_power(pi, num_samps, wait_time, wait_for_crs, est, tot_gain);
	tot_gain[0] = tot_gain[0] - LCN20PHY_RX_IQ_EST_INP_4bit_SHIFT_TO_QDB;

	/* sum I and Q powers for each core, average over num_samps with rounding */
	ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) <= PHY_CORE_MAX);
	FOREACH_CORE(pi, i) {
		cmplx_pwr[i] = ((est[i].i_pwr + est[i].q_pwr) +
			(1U << (log_num_samps-1))) >> log_num_samps;
	}

	/* convert in 1dB gain for gain adjustment */
	extra_gain_1dB = 3 * extra_gain_3dB;

	if (resolution == 0) {
		/* pi->phy_noise_win per antenna is updated inside */
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, extra_gain_1dB);

		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;

		for (i = PHYCORENUM(pi->pubpi->phy_corenum); i >= 1; i--)
			result = (result << 8) | (noise_dbm_ant[i-1] & 0xff);

		return result;
	}
	else if (resolution == 1) {
		/* Reports power in finer resolution than 1 dB (currently 0.25 dB) */
		int16 noisefloor;

		wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, crsmin_pwr, noise_dBm_ant_fine,
		                              extra_gain_1dB, tot_gain);

		if ((gain_correct == 1) || (gain_correct == 2) || gain_correct == 3) {
			int16 gainerr[PHY_CORE_MAX];
			int16 gain_err_temp_adj;
			wlc_phy_get_rxgainerr_phy(pi, gainerr);

			wlc_phy_upd_gain_wrt_temp_phy(pi, &gain_err_temp_adj);

				FOREACH_CORE(pi, i) {
				/* gainerr is in 0.5dB units;
				 * need to convert to 0.25dB units
				 */
			    if (gain_correct == 1) {
			    gainerr[i] = gainerr[i] << 1;
				/* Apply gain correction */
				noise_dBm_ant_fine[i] -= gainerr[i];
				}
				noise_dBm_ant_fine[i] += gain_err_temp_adj;
			}
		}

		noisefloor = (CHSPEC_IS40(pi->radio_chanspec))?
			4*HTPHY_NOISE_FLOOR_40M : 4*HTPHY_NOISE_FLOOR_20M;

		FOREACH_CORE(pi, i) {
		if (noise_dBm_ant_fine[i] < noisefloor) {
					noise_dBm_ant_fine[i] = noisefloor;
			}
		}

		for (i = PHYCORENUM(pi->pubpi->phy_corenum); i >= 1; i--) {
			result = (result << 10) | (noise_dBm_ant_fine[i-1] & 0x3ff);
		}
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
		return result;
	}
#endif /* #if defined(WLTEST) */

	pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	return 0;
}

static void phy_lcn20_iovar_tx_tone(phy_type_misc_ctx_t *ctx, int32 int_val)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	pi->phy_tx_tone_freq = (int32) int_val;

	if (pi->phy_tx_tone_freq == 0) {
		wlc_lcn20phy_stop_tx_tone(pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else {
		pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_lcn20phy_set_tx_tone_and_gain_idx(pi);
	}

}

static void phy_lcn20_iovar_txlo_tone(phy_type_misc_ctx_t *ctx)
{

	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	pi->phy_tx_tone_freq = 0;
}

static int phy_lcn20_iovar_get_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 *ret_int_ptr,
	int32 int_val, int err)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	bool suspend;

	if (!pi->sh->up) {
		err = BCME_NOTUP;
		return err;
	}

	/* make sure bt-prisel is on WLAN side */
	wlc_phy_btcx_wlan_critical_enter(pi);

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
	}

	phy_utils_phyreg_enter(pi);

	/* get IQ power measurements */
	*ret_int_ptr = wlc_phy_rx_iq_est(pi, pi->phy_rxiq_samps, pi->phy_rxiq_antsel,
	                                 pi->phy_rxiq_resln, pi->phy_rxiq_lpfhpc,
	                                 pi->phy_rxiq_diglpf,
	                                 pi->phy_rxiq_gain_correct,
	                                 pi->phy_rxiq_extra_gain_3dB, 0,
	                                 pi->phy_rxiq_force_gain_type);

	phy_utils_phyreg_exit(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
	wlc_phy_btcx_wlan_critical_exit(pi);

	return err;
}

static int phy_lcn20_iovar_set_rx_iq_est(phy_type_misc_ctx_t *ctx, int32 int_val, int err)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 index, index_valid;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	uint8 samples, antenna, resolution, lpf_hpc, dig_lpf;
	uint8 gain_correct, extra_gain_3dB, force_gain_type;

	extra_gain_3dB = (int_val >> 28) & 0xf;
	gain_correct = (int_val >> 24) & 0xf;
	lpf_hpc = (int_val >> 20) & 0x3;
	dig_lpf = (int_val >> 22) & 0x3;
	resolution = (int_val >> 16) & 0xf;
	samples = (int_val >> 8) & 0xff;
	antenna = 0;
	force_gain_type = (int_val >> 4) & 0xf;
	index = (int_val >> 0) & 0x7F;
	index_valid = (int_val >> 7) & 0x1;
	if (index_valid) {
		switch (index) {
			case 1:
			{
				pi_lcn20->rxpath_index = 33;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
			case 2:
			{
				pi_lcn20->rxpath_index = 29;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
			case 3:
			{
				pi_lcn20->rxpath_index = 24;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
			case 4:
			{
				pi_lcn20->rxpath_index = 25;
				pi_lcn20->rxpath_elna = 1;
				break;
			}
			case 5:
			{
				pi_lcn20->rxpath_index = 20;
				pi_lcn20->rxpath_elna = 1;
				break;
			}
			case 6:
			{
				pi_lcn20->rxpath_index = 69;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
			case 7:
			{
				pi_lcn20->rxpath_index = 64;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
			case 8:
			{
				pi_lcn20->rxpath_index = 64;
				pi_lcn20->rxpath_elna = 1;
				break;
			}
			case 9:
			{
				pi_lcn20->rxpath_index = 58;
				pi_lcn20->rxpath_elna = 1;
				break;
			}
			default:
			{
				pi_lcn20->rxpath_index = 0xFF;
				pi_lcn20->rxpath_elna = 0;
				break;
			}
		}
	}
	else
		pi_lcn20->rxpath_index = 0xFF;

	if (gain_correct > 4) {
		err = BCME_RANGE;
		return err;
	}

	if ((lpf_hpc != 0) && (lpf_hpc != 1)) {
		err = BCME_RANGE;
		return err;
	}
	if (dig_lpf > 2) {
		err = BCME_RANGE;
		return err;
	}

	if ((resolution != 0) && (resolution != 1)) {
		err = BCME_RANGE;
		return err;
	}

	if (samples < 10 || samples > 15) {
		err = BCME_RANGE;
		return err;
	}

	/* Limit max number of samples to 2^14 since Lcnphy RXIQ Estimator
	 * takes too much and variable time for more than that.
	*/
	pi->phy_rxiq_samps = samples;
	pi->phy_rxiq_antsel = antenna;
	pi->phy_rxiq_resln = resolution;
	pi->phy_rxiq_lpfhpc = lpf_hpc;
	pi->phy_rxiq_diglpf = dig_lpf;
	pi->phy_rxiq_gain_correct = gain_correct;
	pi->phy_rxiq_extra_gain_3dB = extra_gain_3dB;
	pi->phy_rxiq_force_gain_type = force_gain_type;

	return err;
}

#if (defined(LCN20CONF) && (LCN20CONF != 0))
static void
phy_lcn20_misc_deaf_mode(phy_type_misc_ctx_t *ctx, bool user_flag)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlc_lcn20phy_deaf_mode(pi, user_flag);
}
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */

#ifdef PHY_DUMP_BINARY
/* The function is forced to RAM since it accesses non-const tables */
static int BCMRAMFN(phy_lcn20_misc_getlistandsize)(phy_type_misc_ctx_t *ctx,
                    phyradregs_list_t **phyreglist, uint16 *phyreglist_sz)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	BCM_REFERENCE(pi);
	if (LCN20REV_IS(pi->pubpi->phy_rev, 1)) {
		*phyreglist = (phyradregs_list_t *) &dot11lcn20phy_regs_rev1[0];
		*phyreglist_sz = sizeof(dot11lcn20phy_regs_rev1);
	} else {
		PHY_INFORM(("%s: wl%d: unsupported LCN20 phy rev %d\n",
			__FUNCTION__,  pi->sh->unit,  pi->pubpi->phy_rev));
		return BCME_UNSUPPORTED;
	}

	return BCME_OK;
}
#endif /* PHY_DUMP_BINARY */

#ifdef ATE_BUILD
static void
wlc_phy_gpaio_lcn20phy(phy_type_misc_ctx_t *ctx, wl_gpaio_option_t option, int core)
{
	phy_lcn20_misc_info_t *info = (phy_lcn20_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	/* To bring out various radio test signals on gpaio. */

	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL2, 0, wl_cgpaio_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RCAL_CFG1, 0, wl_rcal_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x0);
	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL1, 0, wl_cgpaio_sel_16to31_port, 0x0);

	switch (option) {
		case (GPAIO_PMU_AFELDO): {
			/* Connect port 14 */
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x4000);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x1);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG1, 0, wl_ana_mux, 0x0);
			break;
		}
		case (GPAIO_PMU_TXLDO): {
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x4000);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x1);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG1, 0, wl_ana_mux, 0x1);
			break;
		}
		case (GPAIO_PMU_VCOLDO): {
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x4000);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x1);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG1, 0, wl_ana_mux, 0x2);
			break;
		}
		case GPAIO_PMU_LNALDO: {
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x4000);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x1);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG1, 0, wl_ana_mux, 0x3);
			break;
		}
		case GPAIO_PMU_ADCLDO: {
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x4000);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x1);
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG1, 0, wl_ana_mux, 0x7);
			break;
		}
		case GPAIO_ICTAT_CAL: {
			/* Connect port 27 */
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL1, 0, wl_cgpaio_sel_16to31_port, 0x800);
			break;
		}
		case GPAIO_PMU_CLEAR: {
			MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_tsten, 0x0);
			MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL2, 0, wl_cgpaio_pu, 0);
			break;
		}
		default:
			break;
	}
}
#endif /* ATE_BUILD */

/* enable/disable ldpc_support bit when the ldpc_cap is changed */
static void
phy_update_rxldpc_lcn20phy(phy_type_misc_ctx_t *ctx, bool ldpc)
{
	phy_lcn20_misc_info_t *misc_info = (phy_lcn20_misc_info_t *) ctx;
	phy_info_t *pi = misc_info->pi;

	phy_info_lcn20phy_t *pi_lcn20 = (phy_info_lcn20phy_t *)pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (ldpc != pi_lcn20->lcn20_rxldpc_override) {
		pi_lcn20->lcn20_rxldpc_override = ldpc;

		PHY_REG_MOD(pi, LCN20PHY, LDPCControl, ldpc_support, (ldpc) ? 1 : 0);
	}

	/* SWWLAN-54874 : 43430 LDPC latency issue WAR */
	if (D11REV_IS(pi->sh->corerev, 39))
		PHY_REG_MOD(pi, LCN20PHY, TxMacIfHoldOff, holdoffval, (ldpc) ? 24 : 20);
}

/* Pass CLM flag info to PHY */
int
phy_lcn20_misc_set_lo_gain_nbcal(phy_type_misc_ctx_t *ctx, bool lo_gain)
{
	phy_lcn20_misc_info_t *misc_info = (phy_lcn20_misc_info_t *) ctx;
	misc_info->lcn20i->logain_NBcal = lo_gain;
	return BCME_OK;
}
