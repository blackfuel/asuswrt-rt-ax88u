/*
 * NPHY Rx Gain Control and Carrier Sense module implementation
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
 * $Id: phy_n_rxgcrs.c 707224 2017-06-27 01:13:09Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_rxgcrs.h"
#include <phy_utils_reg.h>
#include <phy_n.h>
#include <phy_n_info.h>
#include <phy_n_rxgcrs.h>
#include <phy_calmgr.h>
#include <wlc_phyreg_n.h>
#include <phy_type_rxgcrs.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif

/* ************************ */
/* Modules used by this module */
/* ************************ */

/* module private states */
struct phy_n_rxgcrs_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_rxgcrs_info_t *cmn_info;
};

static void phy_n_rxgcrs_adjust_ed_thres(phy_type_rxgcrs_ctx_t * ctx, int32 *assert_threshold,
	bool set_threshold);
#if defined(RXDESENS_EN)
static int phy_n_rxgcrs_get_rxdesens(phy_type_rxgcrs_ctx_t *ctx, int32 *ret_int_ptr);
static int phy_n_rxgcrs_set_rxdesens(phy_type_rxgcrs_ctx_t *ctx, int32 int_val);
#endif /* defined(RXDESENS_EN) */
#ifndef ATE_BUILD
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
static int phy_n_rxgcrs_forcecal_noise(phy_type_rxgcrs_ctx_t *ctx, void *a, bool set);
#endif // endif
#endif /* !ATE_BUILD */
static void
phy_n_rxgcrs_stay_in_carriersearch(phy_type_rxgcrs_ctx_t *ctx, bool enable);
#if defined(BCMDBG)
static void
phy_n_rxgcrs_phydump_chanest(phy_type_rxgcrs_ctx_t *ctx, struct bcmstrbuf *b);
#endif // endif
#ifdef WLTEST
static void
phy_n_rxgcrs_get_chanest(phy_type_rxgcrs_ctx_t *ctx, uint16 fftk, uint16 idx,
	int16 *ch_re, int16 *ch_im);
#endif /* WLTEST */
#if defined(DBG_BCN_LOSS)
static int
phy_n_rxgcrs_dump_phycal_rx_min(phy_type_rxgcrs_ctx_t *ctx, struct bcmstrbuf *b);
#endif /* DBG_BCN_LOSS */

/* register phy type specific implementation */
phy_n_rxgcrs_info_t *
BCMATTACHFN(phy_n_rxgcrs_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_rxgcrs_info_t *cmn_info)
{
	phy_n_rxgcrs_info_t *n_info;
	phy_type_rxgcrs_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((n_info = phy_malloc(pi, sizeof(phy_n_rxgcrs_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	n_info->pi = pi;
	n_info->ni = ni;
	n_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = n_info;
	fns.adjust_ed_thres = phy_n_rxgcrs_adjust_ed_thres;
#if defined(RXDESENS_EN)
	fns.get_rxdesens = phy_n_rxgcrs_get_rxdesens;
	fns.set_rxdesens = phy_n_rxgcrs_set_rxdesens;
#endif /* defined(RXDESENS_EN) */
#ifndef ATE_BUILD
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
	fns.forcecal_noise = phy_n_rxgcrs_forcecal_noise;
#endif // endif
#endif /* !ATE_BUILD */
	fns.stay_in_carriersearch = phy_n_rxgcrs_stay_in_carriersearch;
#if defined(BCMDBG)
	fns.phydump_chanest = phy_n_rxgcrs_phydump_chanest;
#endif // endif
#ifdef WLTEST
	fns.get_chanest = phy_n_rxgcrs_get_chanest;
#endif /* WLTEST */
#if defined(DBG_BCN_LOSS)
	fns.phydump_phycal_rxmin = phy_n_rxgcrs_dump_phycal_rx_min;
#endif /* DBG_BCN_LOSS */

	if (phy_rxgcrs_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_rxgcrs_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return n_info;

	/* error handling */
fail:
	if (n_info != NULL)
		phy_mfree(pi, n_info, sizeof(phy_n_rxgcrs_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_rxgcrs_unregister_impl)(phy_n_rxgcrs_info_t *n_info)
{
	phy_info_t *pi = n_info->pi;
	phy_rxgcrs_info_t *cmn_info = n_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_rxgcrs_unregister_impl(cmn_info);

	phy_mfree(pi, n_info, sizeof(phy_n_rxgcrs_info_t));
}

static void phy_n_rxgcrs_adjust_ed_thres(phy_type_rxgcrs_ctx_t * ctx, int32 *assert_thresh_dbm,
	bool set_threshold)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	/* Set the EDCRS Assert and De-assert Threshold
	The de-assert threshold is set to 6dB lower then the assert threshold
	Accurate Formula:64*log2(round((10.^((THRESHOLD_dBm +65-30)./10).*50).*(2^9./0.4).^2))
	Simplified Accurate Formula: 64*(THRESHOLD_dBm + 75)/(10*log10(2)) + 832;
	Implemented Approximate Formula: 640000*(THRESHOLD_dBm + 75)/30103 + 832;
	*/
	int32 assert_thres_val, de_assert_thresh_val;

	if (set_threshold == TRUE) {
		assert_thres_val = (640000*(*assert_thresh_dbm + 75) + 25045696)/30103;
		de_assert_thresh_val = (640000*(*assert_thresh_dbm + 69) + 25045696)/30103;

		phy_utils_write_phyreg(pi, NPHY_ed_crs20LAssertThresh0, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LAssertThresh1, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UAssertThresh0, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UAssertThresh1, (uint16)assert_thres_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LDeassertThresh0,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20LDeassertThresh1,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UDeassertThresh0,
		                       (uint16)de_assert_thresh_val);
		phy_utils_write_phyreg(pi, NPHY_ed_crs20UDeassertThresh1,
		                       (uint16)de_assert_thresh_val);
	} else {
		assert_thres_val = phy_utils_read_phyreg(pi, NPHY_ed_crs20LAssertThresh0);
		*assert_thresh_dbm = ((((assert_thres_val - 832)*30103)) - 48000000)/640000;
	}
}

#if defined(RXDESENS_EN)
static int
phy_n_rxgcrs_get_rxdesens(phy_type_rxgcrs_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_n_rxgcrs_info_t *info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;
	uint16 regval, x1, x2, y;

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (pi_nphy->ntd_crs_adjusted == FALSE)
		*ret_int_ptr = 0;
	else {
		regval =  phy_utils_read_phyreg(pi, NPHY_Core1InitGainCodeB2057);
		x1 = (((pi_nphy->ntd_initgain>>8) & 0xf) - ((regval>>8) & 0xf)) * 3;
		x2 = (((pi_nphy->ntd_initgain>>4) & 0xf) - ((regval>>4) & 0xf)) * 3;

		regval =  phy_utils_read_phyreg(pi, NPHY_crsminpoweru0);
		y = ((regval & 0xff) - pi_nphy->ntd_crsminpwr[0]) * 3;
		y = (y >> 3) + ((y & 0x4) >> 2);
		*ret_int_ptr = x1 + x2 + y;
	}

	return BCME_OK;
}

static int
phy_n_rxgcrs_set_rxdesens(phy_type_rxgcrs_ctx_t *ctx, int32 int_val)
{
	phy_n_rxgcrs_info_t *info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = info->pi;
	if (pi->sh->interference_mode == INTERFERE_NONE) {
		return wlc_nphy_set_rxdesens((wlc_phy_t *)pi, int_val);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* defined(RXDESENS_EN) */

#ifndef ATE_BUILD
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
static int
phy_n_rxgcrs_forcecal_noise(phy_type_rxgcrs_ctx_t *ctx, void *a, bool set)
{
	phy_n_rxgcrs_info_t *rxgcrsi = (phy_n_rxgcrs_info_t *) ctx;
	phy_info_t *pi = rxgcrsi->pi;
	int err = BCME_OK;
	uint8 wait_ctr = 0;
	int val = 1;
	uint16 crsmin[4];

	if (!set) {
		crsmin[0] = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0);
		crsmin[1] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0);
		crsmin[2] = phy_utils_read_phyreg(pi, NPHY_crsminpowerl0_core1);
		crsmin[3] = phy_utils_read_phyreg(pi, NPHY_crsminpoweru0_core1);
		bcopy(crsmin, a, sizeof(uint16)*4);
	}
	else {
		phy_calmgr_mphase_reset(pi->calmgri);

		pi->cal_info->cal_phase_id = MPHASE_CAL_STATE_NOISECAL;
		pi->trigger_noisecal = TRUE;

		while (wait_ctr < 50) {
			val = ((pi->cal_info->cal_phase_id !=
			MPHASE_CAL_STATE_IDLE)? 1 : 0);

			if (val == 0) {
				err = BCME_OK;
				break;
			}
			else {
				wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_PARTIAL);
				wait_ctr++;
			}
		}

		if (wait_ctr >= 50) {
			return BCME_ERROR;
		}
	}
	return err;
}
#endif // endif
#endif /* !ATE_BUILD */

static void
phy_n_rxgcrs_stay_in_carriersearch(phy_type_rxgcrs_ctx_t *ctx, bool enable)
{
	phy_n_rxgcrs_info_t *rxgcrs_info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = rxgcrs_info->pi;
	uint16 clip_off[] = {0xffff, 0xffff};
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* MAC should be suspended before calling this function */
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));

	if (enable) {
		if (pi_nphy->nphy_deaf_count == 0) {
			pi->phy_classifier_state = wlc_phy_classifier_nphy(pi, 0, 0);
			wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK, 4);
			wlc_phy_clip_det_nphy(pi, 0, pi->phy_clip_state);
			wlc_phy_clip_det_nphy(pi, 1, clip_off);
		}

		pi_nphy->nphy_deaf_count++;

		wlc_phy_resetcca_nphy(pi);

	} else {
		ASSERT(pi_nphy->nphy_deaf_count > 0);

		pi_nphy->nphy_deaf_count--;

		if (pi_nphy->nphy_deaf_count == 0) {
			wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
			pi->phy_classifier_state);
			wlc_phy_clip_det_nphy(pi, 1, pi->phy_clip_state);
		}
	}
}

#if defined(BCMDBG)
static void
phy_n_rxgcrs_phydump_chanest(phy_type_rxgcrs_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_n_rxgcrs_info_t *rxgcrs_info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = rxgcrs_info->pi;
	uint16 num_rx, num_sts, num_tones, start_tone;
	uint16 k, r, t, fftk;
	uint32 ch;
	uint16 ch_re_ma, ch_im_ma;
	uint8  ch_re_si, ch_im_si;
	int16  ch_re, ch_im;
	int8   ch_exp;
	uint8  dump_tones;

	num_rx = (uint8)PHYCORENUM(pi->pubpi->phy_corenum);
	num_sts = 4;

	if (CHSPEC_IS40(pi->radio_chanspec)) {
		num_tones = 128;
#ifdef CHSPEC_IS80
	} else if (CHSPEC_IS80(pi->radio_chanspec)) {
		num_tones = 256;
#endif /* CHSPEC_IS80 */
	} else {
		num_tones = 64;
	}

	bcm_bprintf(b, "num_tones=%d\n", num_tones);

	/* Dump only 16 sub-carriers at a time */
	dump_tones = 16;
	/* Reset the dump counter */
	if (pi->phy_chanest_dump_ctr > (num_tones/dump_tones - 1))
		pi->phy_chanest_dump_ctr = 0;

	start_tone = pi->phy_chanest_dump_ctr * dump_tones;
	pi->phy_chanest_dump_ctr++;

	for (r = 0; r < num_rx; r++) {
		bcm_bprintf(b, "rx=%d\n", r);
		for (t = 0; t < num_sts; t++) {
			bcm_bprintf(b, "sts=%d\n", t);
			for (k = start_tone; k < (start_tone + dump_tones); k++) {
				wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_CHANEST, 1,
					t*128 + k, 32, &ch);

				/* Q11 FLP (12,12,6) */
				ch_re_ma  = ((ch >> 18) & 0x7ff);
				ch_re_si  = ((ch >> 29) & 0x001);
				ch_im_ma  = ((ch >>  6) & 0x7ff);
				ch_im_si  = ((ch >> 17) & 0x001);
				ch_exp	  = ((int8)((ch << 2) & 0xfc)) >> 2;
				ch_re = (ch_re_si > 0) ? -ch_re_ma : ch_re_ma;
				ch_im = (ch_im_si > 0) ? -ch_im_ma : ch_im_ma;
				fftk = ((k < num_tones/2) ? (k + num_tones/2) : (k - num_tones/2));

				bcm_bprintf(b, "chan(%d,%d,%d)=(%d+i*%d)*2^%d;\n",
					r+1, t+1, fftk+1, ch_re, ch_im, ch_exp);
			}
		}
	}
}
#endif // endif

#ifdef WLTEST
static void
phy_n_rxgcrs_get_chanest(phy_type_rxgcrs_ctx_t *ctx, uint16 fftk, uint16 idx,
	int16 *ch_re, int16 *ch_im)
{
	phy_n_rxgcrs_info_t *rxgcrs_info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = rxgcrs_info->pi;
	uint32 ch, tbl_id;
	uint16 ch_re_ma, ch_im_ma;
	uint8  ch_re_si, ch_im_si;

	tbl_id = NPHY_TBL_ID_CHANEST;
	wlc_phy_table_read_nphy(pi, tbl_id, 1, fftk, 32, &ch);
	ch_re_ma  = ((ch >> 14) & 0xff);
	ch_re_si  = ((ch >> 22) & 0x01);
	ch_im_ma  = ((ch >>  5) & 0xff);
	ch_im_si  = ((ch >> 13) & 0x01);
	*ch_re = (ch_re_si > 0) ? -ch_re_ma : ch_re_ma;
	*ch_im = (ch_im_si > 0) ? -ch_im_ma : ch_im_ma;
}
#endif /* WLTEST */
#if defined(DBG_BCN_LOSS)
static int
phy_n_rxgcrs_dump_phycal_rx_min(phy_type_rxgcrs_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_n_rxgcrs_info_t *rxgcrs_info = (phy_n_rxgcrs_info_t *)ctx;
	phy_info_t *pi = rxgcrs_info->pi;
	nphy_iq_comp_t rxcal_coeffs;
	int time_elapsed;
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (!pi_nphy) {
		PHY_ERROR(("wl%d: %s: NPhy null, cannot dump \n",
			pi->sh->unit, __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	time_elapsed = pi->sh->now - pi->cal_info->last_cal_time;
	if (time_elapsed < 0)
		time_elapsed = 0;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

	/* Read Rx calibration co-efficients */
	wlc_phy_rx_iq_coeffs_nphy(pi, 0, &rxcal_coeffs);

	if (pi_nphy->phyhang_avoid)
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);

	/* reg access is done, enable the mac */
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	bcm_bprintf(b, "time since last cal: %d (sec), mphase_cal_id: %d\n\n",
		time_elapsed, pi->cal_info->cal_phase_id);

	bcm_bprintf(b, "rx cal  a0=%d, b0=%d, a1=%d, b1=%d\n\n",
		rxcal_coeffs.a0, rxcal_coeffs.b0, rxcal_coeffs.a1, rxcal_coeffs.b1);

	return BCME_OK;
}
#endif /* DBG_BCN_LOSS */
