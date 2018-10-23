/*
 * ACPHY Sample Collect module implementation
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
 * $Id: phy_ac_samp.c 757615 2018-04-13 23:29:22Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_btcx.h>
#include <phy_misc.h>
#include "phy_type_samp.h"
#include <phy_ac.h>
#include <phy_ac_samp.h>
#include <phy_misc_api.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_ac.h>

#include <bcmendian.h>
#include <hndpmu.h>
#include <sbchipc.h>
#include <phy_utils_reg.h>
#include <phy_ac_info.h>
#ifdef IQPLAY_DEBUG
#include "phy_ac_samp_data.h"
#endif /* IQPLAY_DEBUG */

/* Defines to use a common name for registers that have been renamed
 * over different d11mac corerevs.
 * NOTE: the macros below do not cover rev 80-127 which do not belong to BCA
 */

typedef struct acphy_lpfCT_phyregs {
	bool   is_orig;
	uint16 RfctrlOverrideLpfCT[PHY_CORE_MAX];
	uint16 RfctrlCoreLpfCT[PHY_CORE_MAX];
} acphy_lpfCT_phyregs_t;

/* module private states */
struct phy_ac_samp_info {
	phy_info_t	*pi;
	phy_ac_info_t	*aci;
	phy_samp_info_t	*cmn_info;
	acphy_lpfCT_phyregs_t *ac_lpfCT_phyregs_orig;
	uint32	pstart; 	/* sample collect fifo begins		*/
	uint32	pstop;  	/* sample collect fifo ends		*/
	uint32	pfirst; 	/* sample collect trigger begins	*/
	uint32	plast;  	/* sample collect trigger ends		*/
};

/* local function prototypes */
#ifdef SAMPLE_COLLECT
static int phy_ac_sample_data(phy_type_samp_ctx_t *ctx, wl_sampledata_t *sample_data, void *b);
static int phy_ac_sample_collect(phy_type_samp_ctx_t *ctx,
	wl_samplecollect_args_t *collect, void *buf);
static int phy_ac_sample_collect_core_sel(phy_info_t *pi, int coresel);
#endif /* SAMPLE_COLLECT */

#ifdef IQPLAY_DEBUG
static void phy_ac_samp_prep_IQplay(phy_type_samp_ctx_t *ctx);
#endif /* IQPLAY_DEBUG */

/* register phy type specific implementation */
phy_ac_samp_info_t *
BCMATTACHFN(phy_ac_samp_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_samp_info_t *cmn_info)
{
	phy_ac_samp_info_t *ac_info = NULL;
	phy_type_samp_fns_t fns;
	acphy_lpfCT_phyregs_t *ac_lpfCT_phyregs_orig = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_samp_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	if ((ac_lpfCT_phyregs_orig = phy_malloc(pi, sizeof(*ac_lpfCT_phyregs_orig))) == NULL) {
		PHY_ERROR(("%s: ac_lpfCT_phyregs_orig malloc failed\n", __FUNCTION__));
		goto fail;
	}

	ac_info->ac_lpfCT_phyregs_orig = ac_lpfCT_phyregs_orig;

	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));

	fns.ctx = ac_info;
#ifdef SAMPLE_COLLECT
	fns.samp_data = phy_ac_sample_data;
	fns.samp_collect = phy_ac_sample_collect;
#endif // endif
#ifdef IQPLAY_DEBUG
	fns.samp_play = phy_ac_samp_prep_IQplay;
#endif /* IQPLAY_DEBUG */
	if (phy_samp_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_samp_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	phy_ac_samp_unregister_impl(ac_info);
	return NULL;
}

void
BCMATTACHFN(phy_ac_samp_unregister_impl)(phy_ac_samp_info_t *ac_info)
{
	phy_info_t *pi;
	phy_samp_info_t *cmn_info;

	if (ac_info == NULL) {
		return;
	}
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_samp_unregister_impl(cmn_info);

	if (ac_info->ac_lpfCT_phyregs_orig != NULL) {
		phy_mfree(pi, ac_info->ac_lpfCT_phyregs_orig, sizeof(acphy_lpfCT_phyregs_t));
	}
	phy_mfree(pi, ac_info, sizeof(phy_ac_samp_info_t));
}

/* ******************************************** */
/*		Internal Definitions		*/
/* ******************************************** */

#if defined(SAMPLE_COLLECT) || defined(WL_PROXDETECT)
static void
phy_ac_set_startptr(phy_ac_samp_info_t *sampi, uint32 start_idx)
{
	uint16 ptr_high;
	phy_info_t *pi = sampi->pi;

	W_REG(pi->sh->osh, D11_SCP_STRTPTR(pi), (start_idx & 0xFFFF));
	ptr_high = R_REG(pi->sh->osh, D11_SMP_PTR_H(pi)) & ~0xF;
	ptr_high |= (start_idx >> 16) & 0xF;
	W_REG(pi->sh->osh, D11_SMP_PTR_H(pi), ptr_high);
}

static void
phy_ac_set_stopptr(phy_ac_samp_info_t *sampi, uint32 stop_idx)
{
	uint32 ptr_high;
	phy_info_t *pi = sampi->pi;

	W_REG(pi->sh->osh, D11_SCP_STOPPTR(pi), (stop_idx & 0xFFFF));
	ptr_high = R_REG(pi->sh->osh, D11_SMP_PTR_H(pi)) & ~0xF0;
	ptr_high |= (stop_idx >> 12) & 0xF0;
	W_REG(pi->sh->osh, D11_SMP_PTR_H(pi), ptr_high);
}
#endif /* SAMPLE_COLLECT || WL_PROXDETECT */
#ifdef WL_PROXDETECT
void acphy_set_sc_startptr(phy_info_t *pi, uint32 start_idx)
{
	phy_ac_samp_info_t samp;

	samp.pi = pi;
	phy_ac_set_startptr(&samp, start_idx);
}

void acphy_set_sc_stopptr(phy_info_t *pi, uint32 stop_idx)
{
	phy_ac_samp_info_t samp;

	samp.pi = pi;
	phy_ac_set_stopptr(&samp, stop_idx);
}

#endif /* WL_PROXDETECT */

#ifdef SAMPLE_COLLECT

static uint32
phy_ac_get_startptr(phy_ac_samp_info_t *sampi)
{
	uint32 start_ptr_low, start_ptr_high, start_ptr;
	phy_info_t *pi = sampi->pi;

	start_ptr_low = R_REG(pi->sh->osh, D11_SCP_STRTPTR(pi));
	start_ptr_high = (R_REG(pi->sh->osh, D11_SMP_PTR_H(pi))) & 0xF;
	start_ptr = ((start_ptr_high << 16) | start_ptr_low);

	return start_ptr;
}

static uint32
phy_ac_get_stopptr(phy_ac_samp_info_t *sampi)
{
	uint32 stop_ptr_low, stop_ptr_high, stop_ptr;
	phy_info_t *pi = sampi->pi;

	stop_ptr_low = R_REG(pi->sh->osh, D11_SCP_STOPPTR(pi));
	stop_ptr_high = (R_REG(pi->sh->osh, D11_SMP_PTR_H(pi)) >> 4) & 0xF;
	stop_ptr = ((stop_ptr_high << 16) | stop_ptr_low);

	return stop_ptr;
}

static uint32
phy_ac_get_currptr(phy_ac_samp_info_t *sampi)
{
	uint32 curr_ptr_low, curr_ptr_high, curr_ptr;
	phy_info_t *pi = sampi->pi;

	curr_ptr_low = R_REG(pi->sh->osh, D11_SCP_CURPTR(pi));
	curr_ptr_high = (R_REG(pi->sh->osh, D11_SCP_CURPTR_H(pi))) & 0xF;
	curr_ptr = ((curr_ptr_high << 16) | curr_ptr_low);

	return curr_ptr;
}

static bool
phy_ac_sample_collect_done(phy_ac_samp_info_t *sampi)
{
	uint32 curr_ptr  = phy_ac_get_currptr(sampi);
	uint32 stop_ptr = phy_ac_get_stopptr(sampi);

	if (curr_ptr == stop_ptr)
		return TRUE;
	else
		return FALSE;
}

static int
phy_ac_sample_collect_length(phy_ac_samp_info_t *sampi)
{
	uint32 start_ptr = phy_ac_get_startptr(sampi);
	uint32 stop_ptr = phy_ac_get_stopptr(sampi);

	return (stop_ptr - start_ptr);
}

/* channel to frequency conversion (in BMAC: use wf_channel2mhz) */
static void
phy_ac_chan2fc(phy_ac_samp_info_t *sampi, uint16 *fc)
{
	uint16 channel;
	phy_info_t *pi = sampi->pi;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* go from channel number (such as 6) to carrier freq (such as 2442) */
	if (channel >= 184 && channel <= 228)
		*fc = (channel*5 + 4000);
	else if (channel >= 32 && channel <= 180)
		*fc = (channel*5 + 5000);
	else if (channel >= 1 && channel <= 13)
		*fc = (channel*5 + 2407);
	else if (channel == 14)
		*fc = 2484;
	else
		*fc = 0xffff;
}

/* For more comments on how "words_per_us"is calculated,
 * check the d11samples.tcl file
 */
static uint16
phy_ac_words_per_us(phy_ac_samp_info_t *sampi, uint16 sd_adc_rate, uint16 mo)
{
	phy_info_t *pi = sampi->pi;

	uint16 sampRate;
	uint16 words_per_us = sd_adc_rate / 4;
	uint8 md_datapath_os = 1;
	switch (mo)
	{
		case SC_MODE_0_sd_adc:

			sampRate = sd_adc_rate;
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					words_per_us = sampRate/2;
					break;
				case WL_CHANSPEC_BW_40:
					words_per_us = sampRate/4;
					break;
				case WL_CHANSPEC_BW_80:
					words_per_us = sampRate/4;
					break;
				default:
					/* Default 20MHz */
					words_per_us = sampRate/2;
					ASSERT(0); /* should never get here */
					break;
			}
			break;

		case  SC_MODE_0_28nm_sar_adc_nti:
		case  SC_MODE_2_28nm_dcc_out:
		case  SC_MODE_3_28nm_farrow_in:
			sampRate = sd_adc_rate;
			/* 8 iq samples will be packed in 7 words (32 bit packing) */
			words_per_us = ((pi->bw == WL_CHANSPEC_BW_160) ? 2 : 1) * sampRate * 7 / 8;
			break;

		case  SC_MODE_1_28nm_sar_adc_ti:
			sampRate = sd_adc_rate;
			words_per_us = (2 * sampRate * 4 / 10) * 2;
			break;

		case  SC_MODE_4_28nm_farrow_out:
			sampRate = sd_adc_rate;
			/* 1 iq sample is packed in 1 word */
			words_per_us = sampRate;
			break;

		case SC_MODE_1_sd_adc_5bits:

			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = sd_adc_rate;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = sd_adc_rate/2;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = sd_adc_rate/2;
					break;
				default:
					/* Default 20MHz */
					sampRate = sd_adc_rate;
					ASSERT(0); /* should never get here */
					break;
			}
			words_per_us = sampRate / 2;
			break;

		case SC_MODE_2_cic0:

			sampRate  = sd_adc_rate / 2;

			words_per_us = sampRate;
			break;

		case SC_MODE_3_cic1:

			sampRate  = sd_adc_rate / 4;

			words_per_us = sampRate;

			break;
		case SC_MODE_4s_rx_farrow_1core:
		case SC_MODE_5s_iq_comp:
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 2;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 2;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80 * 2;
					break;
				case WL_CHANSPEC_BW_160:
					sampRate = 160 * 2;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20 * 2;
					ASSERT(0); /* should never get here */
					break;
			}
			if (mo == SC_MODE_5s_iq_comp)
				md_datapath_os = (ACMAJORREV_3(pi->pubpi->phy_rev) ||
					ACMAJORREV_4(pi->pubpi->phy_rev) ||
					ACMAJORREV_5(pi->pubpi->phy_rev)) ? 2 : 1;
						sampRate /= md_datapath_os;
			words_per_us = sampRate;
			break;
		case SC_MODE_6s_dc_filt:
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 2;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 2;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80 * 2;
					break;
				case WL_CHANSPEC_BW_160:
					sampRate = 160 * 2;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20 * 2;
					ASSERT(0); /* should never get here */
					break;
			}
			md_datapath_os = (ACMAJORREV_3(pi->pubpi->phy_rev) ||
					ACMAJORREV_4(pi->pubpi->phy_rev) ||
					ACMAJORREV_5(pi->pubpi->phy_rev) ||
					ACMAJORREV_32(pi->pubpi->phy_rev) ||
					ACMAJORREV_33(pi->pubpi->phy_rev) ||
					ACMAJORREV_GE37(pi->pubpi->phy_rev)) ? 2 : 1;
			sampRate /= md_datapath_os;
			words_per_us = sampRate;
			break;

		case SC_MODE_4m_rx_farrow:
		case SC_MODE_5_iq_comp:
		case SC_MODE_6_dc_filt:
		case SC_MODE_8_rssi:
		case SC_MODE_9_rssi_all:

			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 2;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 2;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20 * 2;
					ASSERT(0); /* should never get here */
					break;
			}
			if ((mo == SC_MODE_5_iq_comp) || (mo == SC_MODE_6_dc_filt))
				sampRate /= 2;

			words_per_us = sampRate * 2;
			break;

		case SC_MODE_7_rx_filt:
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80 / 2;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20;
					ASSERT(0); /* should never get here */
					break;
			}
			words_per_us = sampRate * 4;
			break;

		case SC_MODE_7s_rx_filt:
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80;
					break;
				case WL_CHANSPEC_BW_160:
					sampRate = 160;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20;
					ASSERT(0); /* should never get here */
					break;
			}
			words_per_us = sampRate;
			break;

		case SC_MODE_10_tx_farrow:
			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 4;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 4;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80 * 2;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20;
					ASSERT(0); /* should never get here */
					break;
			}
			sampRate /= md_datapath_os;
			words_per_us = sampRate;
			break;

		case SC_MODE_11_gpio:

			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 4;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 4;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20;
					ASSERT(0); /* should never get here */
					break;
			}
			words_per_us = sampRate;
			break;

		case SC_MODE_12_gpio_trans:

			switch (pi->bw) {
				case WL_CHANSPEC_BW_20:
					sampRate = 20 * 4;
					break;
				case WL_CHANSPEC_BW_40:
					sampRate = 40 * 2;
					break;
				case WL_CHANSPEC_BW_80:
					sampRate = 80 / 2;
					break;
				default:
					/* Default 20MHz */
					sampRate = 20;
					ASSERT(0); /* should never get here */
					break;
			}
			words_per_us = sampRate * 2;

			break;

		case SC_MODE_14_spect_ana:

			words_per_us = sd_adc_rate;

			break;
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		switch (mo)
		{
			case SC_MODE_4s_rx_farrow_1core:
			case SC_MODE_4m_rx_farrow:
				/* Farrow operates at twicw the BW */
				words_per_us = 40;
				break;

			case SC_MODE_5_iq_comp:
			case SC_MODE_6_dc_filt:
			case SC_MODE_7_rx_filt:
			case SC_MODE_5s_iq_comp:
			case SC_MODE_6s_dc_filt:
			case SC_MODE_7s_rx_filt:
				/* Sampling rate is at nyquist rate after rx filter */
				words_per_us = 20;
				break;
		}
	}
	return words_per_us;
}

static uint32
phy_ac_set_pmu_chipctl5(phy_ac_samp_info_t *sampi)
{
	phy_info_t *pi = sampi->pi;

	uint32 pmu_chipctReg5 = si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5, 0, 0);
	uint32 pmu_chipctReg5_orig = pmu_chipctReg5;

	pmu_chipctReg5 |= (0xffff << 16);

	if (!ACMAJORREV_3(pi->pubpi->phy_rev) && CHSPEC_IS80(pi->radio_chanspec)) {
		pmu_chipctReg5 &= 0x1036ffff;
	} else {
		/* For 20 and 40MHz Mode 0 will be active which means
		 * I and Q are packed in 32bits word
		 */
		pmu_chipctReg5 &= 0x100effff;
	}
	si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5, 0xFFFFFFFF, pmu_chipctReg5);

	return pmu_chipctReg5_orig;
}

/* Code to do the actual sample collect for timer == now.
 * Returs TRUE if okay, FALSE if timed out
 */
static bool
wlc_phy_sample_collect_now(phy_ac_samp_info_t *sampi, wl_samplecollect_args_t *collect,
		const uint32 sc_buff_start)
{
	phy_info_t *pi = sampi->pi;
	uint8 sample_capture_pointer_timeout_max_retries = 10;

	uint32 phy_ctl = 0;
	uint32 play_ctl = 0;
	uint32 timer = collect->timeout;
	uint32 prev_currptr = 0;

	phy_ac_set_stopptr(sampi, sampi->pstop);

	if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
		play_ctl = R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
	} else {
		phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
	}

	/* no trigger flags collect (turn off just to be sure) */
	wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi), 0x0);

	if (collect->mode == SC_MODE_12_gpio_trans) {
		/* For GPIO tranisition mode do continuous sample collect */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
			                                                play_ctl | (1 << 0));
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl | (1 << 4));
		}
		OSL_DELAY(1000000);
	} else {
		uint32 sample_capture_pointer_timeouts = 0;

		/* set Stop bit and Start bit (start capture) */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
			                                     play_ctl | (1 << 0) | (1 << 2));
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi),
			                                      phy_ctl | (1 << 4) | (1 << 5));
		}

		if (collect->gpio_collection == 0 && ACMAJORREV_36(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
		}
		/* wait until done */
		do {
			prev_currptr = phy_ac_get_currptr(sampi);
			OSL_DELAY(10);
			timer--;

			/* Check for a stuck sample collect pointer - SW4345-88 */
			/* Also see SW4345-718 */
			if (phy_ac_get_currptr(sampi) == prev_currptr &&
					sample_capture_pointer_timeouts <
					sample_capture_pointer_timeout_max_retries) {

				/* timeout count for ATE */
				++sample_capture_pointer_timeouts;

				PHY_ERROR(("%s: sample capture pointer got stuck at %d "
					   "- retriggering - attempt %i\n", __FUNCTION__,
					   prev_currptr, sample_capture_pointer_timeouts));

				/* Rearm the main timeout timer */
				timer = collect->timeout;

				/* clear start/stop bits */
				if (D11REV_IS(pi->sh->corerev, 50) ||
						D11REV_GE(pi->sh->corerev, 54)) {
					W_REG(pi->sh->osh,
						D11_SMP_CTRL(pi),
						play_ctl & 0xFFFA);
				} else {
					W_REG(pi->sh->osh,
						D11_PSM_PHY_CTL(pi),
						phy_ctl & 0xFFCF);
				}

				/* Restore PHY_CTL or play_ctl to its original state */
				if (D11REV_IS(pi->sh->corerev, 50) ||
					D11REV_GE(pi->sh->corerev, 54)) {
					W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
						play_ctl);
				} else {
					W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
				}

				/* reset the start and stop pointers */
				phy_ac_set_startptr(sampi, sc_buff_start);
				phy_ac_set_stopptr(sampi, sampi->pstop);

				/* Also need to kick the phy with a ResetCCA. */
				wlc_phy_resetcca_acphy(pi);

				/* set Stop bit and Start bit (start capture) */
				phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
				wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi),
					0x0);
				if (D11REV_IS(pi->sh->corerev, 50) ||
					D11REV_GE(pi->sh->corerev, 54)) {
					W_REG(pi->sh->osh,
						D11_SMP_CTRL(pi),
						phy_ctl | (1 << 0) | (1 << 2));
				} else {
					W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi),
						phy_ctl | (1 << 4) | (1 << 5));
				}
			}
		} while (!phy_ac_sample_collect_done(sampi) && timer > 0);

		if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			/* The whole register should be made 0. Else phy_sample_valid will be
			 * held high in RTL. This causes the last samples of the current capture
			 * to appear in the beginning of the next capture
			 */
			WRITE_PHYREG(pi, AdcDataCollect, 0);
		}

		/* Restore PHY_CTL to its original state, will clear start/stop bits */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi), play_ctl);
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
		}

	}

	/* set start/stop pointers for readout */
	sampi->pfirst = phy_ac_get_startptr(sampi);
	sampi->plast = phy_ac_get_currptr(sampi);

	return (timer > 0);
}

/* ******************************************** */
/*		External Definitions		*/
/* ******************************************** */

/* FIFO memory is 176kB = 45056 x 32bit */
#define FILE_HDR_LEN 20 /* words */

int
phy_ac_sample_collect(phy_type_samp_ctx_t *ctx, wl_samplecollect_args_t *collect, void *buf)
{
	uint	index, sicoreunit;
	uint32	timer, cnt, mac_ctl;
	uint32	smp_ctl_save = 0;
	uint32	szbytes, sc_buff_length, sc_buff_offs, sc_buff_start, sc_buff_end;
	uint16	sd_adc_rate, sar_adc_rate, save_gpio, fc, val, save_clkgatests = 0;
	uint8	num_cores, core, RxFeTesMmuxCtrl_rxfe_dbg_mux_sel_save, wide64bus_sel_save = 0;
	uint8	sample_rate, sample_bitwidth, support_spectrum_analyzer_mode;
	uint8   num_actv_cores = 1, bit_unpacking_mode_28nm = 3;

	int16	agc_gain[PHY_CORE_MAX];
	uint16	forceFront_save[PHY_CORE_MAX], gpioLo_save, gpioHi_save;

	uint32	mo = 0xffff, *ptr = NULL, pmu_chipctReg5_def = 0, pmu_chip_ctl_regupdate = 0;
	uint16	words_per_us = 0, fft_sampRate = 0, macBasedDACPlayEn_save = 0;
	uint16	dBsample_to_dBm_sub = 0;
	int32   pfirst_wrap_check;

	bool	downSamp = FALSE, suspend = FALSE, mac_64_bits_enable = FALSE;
	int	coreSel = 0, bitStartVal = 2;
	int     readreg = 0;

	wl_sampledata_t	*sample_data;

	phy_ac_samp_info_t *sampi = (phy_ac_samp_info_t *)ctx;
	phy_info_t *pi = sampi->pi;
	phy_ac_info_t *pi_ac = sampi->aci;

	num_cores = PHYCORENUM(pi->pubpi->phy_corenum);
	collect->gpio_collection = 0;
	sicoreunit = wlapi_si_coreunit(pi->sh->physhim);

	/* Suspend MAC */
	wlc_phy_conditional_suspend(pi, &suspend);
	phy_ac_chan2fc(sampi, &fc);
	ASSERT(fc != 0xffff);

	mac_ctl = R_REG(pi->sh->osh, D11_MACCONTROL(pi));

	/*
	 * In 4345A0, to avoid glitches in the captured samples, suspending MAC or
	 * just disabling MAC PSM were found to help.
	 */
	if (CHIPID(pi->sh->chip) == BCM4345_CHIP_ID) {
		/* disable mac psm */
		W_REG(pi->sh->osh, D11_MACCONTROL(pi), (mac_ctl & ~MCTL_PSM_RUN));
	}

	/* Initializing the agc gain */
	for (index = 0; index < PHY_CORE_MAX; index++) {
		agc_gain[index] = 0;
	}

	/* initial return info pointers */
	sample_data = (wl_sampledata_t *)buf;
	ptr = (uint32 *)(sample_data + 1);
	bzero((uint8 *)sample_data, sizeof(wl_sampledata_t));
	sample_data->version = htol16(WL_SAMPLEDATA_T_VERSION);
	sample_data->size = htol16(sizeof(wl_sampledata_t));

	/* get Buffer length length in words */
	if (ACMAJORREV_1(pi->pubpi->phy_rev) || ACMAJORREV_3(pi->pubpi->phy_rev)) {
		szbytes = 192*1024;
		sc_buff_offs = 0;
	} else if (ACMAJORREV_2(pi->pubpi->phy_rev)) {
		szbytes = 320*1024;
		sc_buff_offs = 128 * 1024 >> 2;
	} else if (ACMAJORREV_5(pi->pubpi->phy_rev)) {
		szbytes = 576*1024;
		sc_buff_offs = 192 * 1024 >> 2;
	} else if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
		/* In rev50 (4349A0), TXMAC FIFO size is 448KB with 896 locations
		  * and each location of 512 bytes.
		  * For sample capture we need 128kb (core0) + 128kb(core1),
		  * i.e. we need 256kb for SC which means 512 locations of Tx FIFO are required.
		  * Hence we need to ask MAC to use only remaining
		  * i.e. 896 - 512 = 384 locations of Tx FIFO when SC is being done.
		  */
		if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
			(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE0)) {
			/* RSDB core0 */
			szbytes = 128*1024;
			sc_buff_offs = 192 * 1024 >> 2;
		} else if ((phy_get_phymode(pi) == PHYMODE_RSDB) &&
			(phy_get_current_core(pi) == PHY_RSBD_PI_IDX_CORE1)) {
			/* RSDB core1 */
			szbytes = 128*1024;
			sc_buff_offs = 320 * 1024 >> 2;
		} else {
			/* MIMO mode and 80P80 mode */
			szbytes = 256*1024;
			sc_buff_offs = 192 * 1024 >> 2;
		}
	} else if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	           ACMAJORREV_33(pi->pubpi->phy_rev) ||
	           ACMAJORREV_37(pi->pubpi->phy_rev) ||
	           ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			szbytes = 579*1024;
			sc_buff_offs = 192 * 1024 >> 2;
	} else if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		if (sicoreunit == DUALMAC_MAIN) {
			szbytes = 256*1024;
			sc_buff_offs = 192 * 1024 >> 2;
		} else {
			szbytes = 112*1024;
			sc_buff_offs = 16 * 1024 >> 2;
		}
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
			szbytes = 88*1024;
			sc_buff_offs = 0;
	} else {
		szbytes = 512*1024;
		sc_buff_offs = 0;
	}
	sc_buff_length = szbytes >> 2;

	/* Store MAC based sample play information */
	macBasedDACPlayEn_save = READ_PHYREGFLD(pi, macbasedDACPlay, macBasedDACPlayEn);

	/* Store GPIO information */
	gpioLo_save = READ_PHYREG(pi, gpioLoOutEn);
	gpioHi_save = READ_PHYREG(pi, gpioHiOutEn);

	/* Store forceFront register as it may be modified while setting up the samode capture */
	FOREACH_CORE(pi, core) {
		forceFront_save[core] =  READ_PHYREGCE(pi, forceFront, core);
	}

	/* Store RxFeTesMmuxCtrl register */
	RxFeTesMmuxCtrl_rxfe_dbg_mux_sel_save =
		READ_PHYREGFLD(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel);

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Store SpecAnaDataCollect register */
		wide64bus_sel_save =
			READ_PHYREGFLD(pi, SpecAnaDataCollect, wide64bus_sel);

		/* Store ClkGateSts register */
		save_clkgatests = R_REG(pi->sh->osh, D11_CLK_GATE_STS(pi));
	}
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		save_clkgatests = R_REG(pi->sh->osh, D11_CLK_GATE_STS(pi));
	}

	if (macBasedDACPlayEn_save == 1) {
		/* If MAC based sample play is being used, use only half the TX FIFO for */
		/* sample capture, unless user specifies a non-zero startptr value */
		sc_buff_start = sc_buff_length >> 1;
	}
	else {
		sc_buff_start = 0;
	}
	sc_buff_start   = sc_buff_start + sc_buff_offs;
	sc_buff_end     = sc_buff_length + sc_buff_offs;

	if (fc < 5180) {
		/* 2G channel */
		switch (pi->bw) {
			case WL_CHANSPEC_BW_20:
				sd_adc_rate = fc * 3/ 32;
				sar_adc_rate = fc / 48;
				break;
			case WL_CHANSPEC_BW_40:
				sd_adc_rate = fc * 3/16;
				sar_adc_rate = fc / 24;
				break;
			default:
				/* Default 20MHz */
				sd_adc_rate = fc * 3/ 32;
				sar_adc_rate = fc / 24;
				ASSERT(0); /* should never get here */
				break;
		}
	} else {
		/* 5G channel */
		switch (pi->bw) {
			case WL_CHANSPEC_BW_20:
				sd_adc_rate = fc/24;
				sar_adc_rate = fc / 96;
				break;
			case WL_CHANSPEC_BW_40:
				sd_adc_rate = fc/12;
				sar_adc_rate = fc / 48;
				break;
			case WL_CHANSPEC_BW_80:
				sd_adc_rate = fc/9;
				sar_adc_rate = fc / 32;
				break;
			case WL_CHANSPEC_BW_160:
				sd_adc_rate = fc/9;
				sar_adc_rate = fc / 32;
				break;
			default:
				/* Default 20MHz */
				sd_adc_rate = fc/24;
				sar_adc_rate = fc / 32;
				ASSERT(0); /* should never get here */
				break;
		}
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		if (fc < 3000) {
				if (phy_ac_radio_get_data(pi_ac->radioi)->ulp_adc_mode) {
					sd_adc_rate = fc/48;
				} else {
					sd_adc_rate = fc/56;
				}
		} else {
			if (phy_ac_radio_get_data(pi_ac->radioi)->ulp_adc_mode) {
					sd_adc_rate = fc/96;
				if (fc >= 5180) {
					sd_adc_rate = fc/104;
				}
			} else {
					sd_adc_rate = fc/112;
				if (fc >= 5180) {
					sd_adc_rate = fc/128;
				}
			}
		}
	}

	WRITE_PHYREG(pi, AdcDataCollect, 0);
	WRITE_PHYREG(pi, RxFeTesMmuxCtrl, 0);

	/* 4360A0/A2, 4335A0/B0, 4345 do not have the spectrum analyzer mode */
	if (ACREV_IS(pi->pubpi->phy_rev, 0) ||
		ACREV_IS(pi->pubpi->phy_rev, 2) || ACREV_IS(pi->pubpi->phy_rev, 5) ||
		ACMAJORREV_3(pi->pubpi->phy_rev)|| ACMAJORREV_36(pi->pubpi->phy_rev)) {
		support_spectrum_analyzer_mode = 0;
	}
	else {
		support_spectrum_analyzer_mode = 1;
		if (ACREV_IS(pi->pubpi->phy_rev, 1))
			PHY_REG_WRITE(pi, ACPHY, SpectrumAnalyzerMode(1), 0);
		else /* phyrev!=0 && phyrev !=1  */
			PHY_REG_WRITE(pi, ACPHY, SpectrumAnalyzerMode(2), 0);
		if (!ACREV_IS(pi->pubpi->phy_rev, 1))
			WRITE_PHYREG(pi, SpecAnaDataCollect, 0);
	}

	switch (collect->mode)
	{
		case SC_MODE_0_sd_adc:

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, coreSel);
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 0);

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			 */
			mo = ((coreSel << 12) | (1 << 8) | 0);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us  =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_0_sd_adc);

			break;

		case  SC_MODE_0_28nm_sar_adc_nti:

			coreSel = phy_ac_sample_collect_core_sel(pi, coreSel);
			bit_unpacking_mode_28nm = 3;
			if ((coreSel < 0) || (coreSel > 2)) {
				coreSel = 0;
			}

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us = phy_ac_words_per_us(sampi,
					sar_adc_rate, SC_MODE_0_28nm_sar_adc_nti);

			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, sar_adc_legacy_bus_sel, 1);
				if (coreSel == 3) {
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 1);
					words_per_us = words_per_us * 2;
					num_actv_cores = 2;
				} else {
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 0);
					MOD_PHYREG(pi, RxFeStatus,
						saradc_packout_default_core_sel, coreSel);
					num_actv_cores = 1;
				}
			}
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 0);
			MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			 */
			mo = (((coreSel+1) << 12) | (num_actv_cores << 8) | 0);

			break;
		case SC_MODE_1_28nm_sar_adc_ti:
			//WRITE_PHYREG(pi, rx1SampCol64bitDataselect, 0x0010);
			//WRITE_PHYREG(pi, SpecAnaDataCollect, 0x2200);

			if (ACMAJORREV_47(pi->pubpi->phy_rev) || pi->bw == WL_CHANSPEC_BW_160) {
				coreSel = collect->cores;
				bit_unpacking_mode_28nm = 2;

				/*      Determine how many words need to be placed in the MAC buffer
					to capture 1us of the signal, in this sample capture mode
				*/
				words_per_us = phy_ac_words_per_us(sampi,
				        sar_adc_rate, SC_MODE_1_28nm_sar_adc_ti);

				MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);
				MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, 0);
				MOD_PHYREG(pi, SpecAnaDataCollect, rxCoreSel_1, 0);

				MOD_PHYREG(pi, SpecAnaDataCollect, saradc_data_upper_sel, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, saradc_64bits_sel, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, sar_adc_legacy_bus_sel, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, single_or_dual_core_sel, 0);
				MOD_PHYREG(pi, RxFeStatus,
					saradc_packout_default_core_sel, coreSel);
				num_actv_cores = 1;
				MOD_PHYREG(pi, rx1SampCol64bitDataselect,
					saradc_cap_sel_WideDataBus_perCore, 1);
				MOD_PHYREG(pi, rx1SampCol64bitDataselect,
					saradc_coresel0_samplecollect, coreSel);
				MOD_PHYREG(pi, rx1SampCol64bitDataselect,
					saradc_coresel1_samplecollect, coreSel);
				WRITE_PHYREG(pi, rx1Spare, READ_PHYREG(pi, rx1Spare)|0x1);
				MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 1);
				MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
			}

			/*      Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			*/
			mo = (((coreSel+1) << 12) | (num_actv_cores << 8) | 0);

			break;
		case  SC_MODE_1_sd_adc_5bits:

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, coreSel);
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 1);

			if (pi->bw == WL_CHANSPEC_BW_40 || pi->bw == WL_CHANSPEC_BW_80) {
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=1
			 */
			mo = ((coreSel << 12) | (1 << 8) | 1);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_1_sd_adc_5bits);

			break;

		case  SC_MODE_2_cic0:

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, coreSel);
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 2);

			if (pi->bw == WL_CHANSPEC_BW_40 || pi->bw == WL_CHANSPEC_BW_80) {
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=2
			 */
			mo = ((coreSel << 12) | (1 << 8) | 2);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_2_cic0);

			break;

		case  SC_MODE_2_28nm_dcc_out:

			coreSel = collect->cores;
			if (pi->bw == WL_CHANSPEC_BW_160) {
				bit_unpacking_mode_28nm = 1;
				MOD_PHYREG(pi, SpecAnaDataCollect, saradc_64bits_sel, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, sar_adc_legacy_bus_sel, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, saradc_data_upper_sel, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 1);

			} else {
				bit_unpacking_mode_28nm = 3;
				MOD_PHYREG(pi, SpecAnaDataCollect, saradc_64bits_sel, 0);
			}
			if ((coreSel < 0) || (coreSel > 2)) {
				coreSel = 0;
			}

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us  =
				phy_ac_words_per_us(sampi, sar_adc_rate, SC_MODE_2_28nm_dcc_out);

			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, sar_adc_legacy_bus_sel, 1);
				if (coreSel == 2) {
					/* both cores are active */
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 1);
					words_per_us = words_per_us * 2;
					num_actv_cores = 2;
				} else {
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 0);
					MOD_PHYREG(pi, RxFeStatus,
						saradc_packout_default_core_sel, coreSel);
					num_actv_cores = 1;
				}
			}
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 2);
			MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
			MOD_PHYREG(pi, rx1SampCol64bitDataselect,
				saradc_cap_sel_WideDataBus_perCore, 1);
			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			 */
			mo = (((coreSel+1) << 12) | (num_actv_cores << 8) | 0);

			break;

		case SC_MODE_3_cic1:

			if (pi->bw == WL_CHANSPEC_BW_80) {
				return BCME_ERROR;
			}

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, coreSel);
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 3);

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=3
			 */
			mo = ((coreSel << 12) | (1 << 8) | 3);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_3_cic1);

			break;

		case  SC_MODE_3_28nm_farrow_in:

			if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
				coreSel = collect->cores;

				MOD_PHYREG(pi, AdcDataCollect, sampSel, 3);
				MOD_PHYREG(pi, SampleCapture, en_43684b0_sample_capture, 1);
				MOD_PHYREG(pi, SampleCapture, sample_capture_en, 1);
				MOD_PHYREG(pi, SampleCapture, core_sel_0, coreSel);
				MOD_PHYREG(pi, SampleCapture, core_sel_1, 1);
				MOD_PHYREG(pi, SampleCapture, core_sel_2, 2);
				MOD_PHYREG(pi, SampleCapture, core_sel_3, 3);
				MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 1);
				MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 1);
				num_actv_cores = 1;
				words_per_us =  phy_ac_words_per_us(sampi,
					sar_adc_rate, SC_MODE_3_28nm_farrow_in);
				if (pi->bw == WL_CHANSPEC_BW_160) {
					bit_unpacking_mode_28nm = 1;
				} else {
					bit_unpacking_mode_28nm = 3;
				}

			} else {
				coreSel = collect->cores;
				if (pi->bw == WL_CHANSPEC_BW_160) {
					bit_unpacking_mode_28nm = 1;
					MOD_PHYREG(pi, SpecAnaDataCollect, saradc_64bits_sel, 1);
					MOD_PHYREG(pi, SpecAnaDataCollect,
						sar_adc_legacy_bus_sel, 1);
					MOD_PHYREG(pi, SpecAnaDataCollect,
						saradc_data_upper_sel, 1);
					MOD_PHYREG(pi, sarAfeCompCtrl0, sarAfeCompMode, 1);
					MOD_PHYREG(pi, sarAfeCompCtrl1, sarAfeCompMode, 1);
					MOD_PHYREG(pi, sarAfeCompCtrl2, sarAfeCompMode, 1);
					MOD_PHYREG(pi, sarAfeCompCtrl3, sarAfeCompMode, 1);
				} else {
					bit_unpacking_mode_28nm = 3;
					MOD_PHYREG(pi, SpecAnaDataCollect, saradc_64bits_sel, 0);
				}
				if ((coreSel < 0) || (coreSel > 2)) {
					coreSel = 0;
				}

				/*	Determine how many words need to be placed in the MAC buffer
					to capture 1us of the signal, in this sample capture mode
				 */
				words_per_us  =
				phy_ac_words_per_us(sampi, sar_adc_rate,
					SC_MODE_3_28nm_farrow_in);

				if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
					MOD_PHYREG(pi, SpecAnaDataCollect,
						sar_adc_legacy_bus_sel, 1);
					if (coreSel == 2) {
						/* both cores are active */
						MOD_PHYREG(pi, SpecAnaDataCollect,
							single_or_dual_core_sel, 1);
						words_per_us = words_per_us * 2;
						num_actv_cores = 2;
					} else {
						MOD_PHYREG(pi, SpecAnaDataCollect,
							single_or_dual_core_sel, 0);
						MOD_PHYREG(pi, RxFeStatus,
							saradc_packout_default_core_sel, coreSel);
						num_actv_cores = 1;
					}
				}
				MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 3);
				MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
			}
			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			 */
			mo = (((coreSel+1) << 12) | (num_actv_cores << 8) | 0);

			break;

		case  SC_MODE_4s_rx_farrow_1core:
			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			coreSel = phy_ac_sample_collect_core_sel(pi, coreSel);

			if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, wide64bus_sel, 1);
			}
			if (!ACMAJORREV_32(pi->pubpi->phy_rev) &&
				!ACMAJORREV_33(pi->pubpi->phy_rev) &&
				!ACMAJORREV_37(pi->pubpi->phy_rev) &&
				!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, RxFeTesMmuxCtrl, samp_coll_core_sel, coreSel);
			}
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 4);

			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_GE37(pi->pubpi->phy_rev)) {
			  MOD_PHYREG(pi, AdcDataCollect, sampSel, 4);
			  MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);
			  MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, coreSel);
			}

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=4
			 */
			mo = ((coreSel << 12) | (1 << 8) | 4);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_4s_rx_farrow_1core);

			if (CHIPID(pi->sh->chip) == BCM4335_CHIP_ID) {
				/* Incase of 4335, 4 samples are packed into 3 words
				   (32bits per word)
				 */
				words_per_us = (words_per_us * 3) >> 2;
			}

			if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
				!(ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_6(pi)) &&
				!ACMAJORREV_32(pi->pubpi->phy_rev) &&
				!ACMAJORREV_33(pi->pubpi->phy_rev) &&
				!ACMAJORREV_37(pi->pubpi->phy_rev) &&
				!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pmu_chipctReg5_def = phy_ac_set_pmu_chipctl5(sampi);
				pmu_chip_ctl_regupdate = 1;
			}
			/* Special mode for BCM43457A0/B0, BCM4345B0/B1 (CRDOT11ACPHY-590) */
			if (ACMAJORREV_3(pi->pubpi->phy_rev) || ACMAJORREV_4(pi->pubpi->phy_rev) ||
				ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, pktprocMuxEn, 1);
			}
			break;

		case SC_MODE_4m_rx_farrow:

			if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
				mac_64_bits_enable = 1;
			bitStartVal = collect->bitStart;
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				if ((bitStartVal < 0) || (bitStartVal > 3)) {
					bitStartVal = 0;
				}
				WRITE_PHYREG(pi, data_collect_crosscoresel0, 0x0000);
				WRITE_PHYREG(pi, data_collect_crosscoresel1, 0xfff0);
			} else if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				bitStartVal = 3;
			} else {
				if ((bitStartVal < 0) || (bitStartVal > 2)) {
					bitStartVal = 2;
				}
			}

			MOD_PHYREG(pi, AdcDataCollect, bitStart, bitStartVal);
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 4);
			MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, coreSel);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=4
			 */

			mo = ((coreSel << 12) | (num_cores << 8) | 4);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_4m_rx_farrow);

			break;

	        case SC_MODE_4_28nm_farrow_out:

			coreSel = collect->cores;
			bit_unpacking_mode_28nm = 6;
			if ((coreSel < 0) || (coreSel > 2)) {
				coreSel = 0;
			}

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us  =
				phy_ac_words_per_us(sampi, sar_adc_rate, SC_MODE_4_28nm_farrow_out);

			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, sar_adc_legacy_bus_sel, 1);
				if (coreSel == 2) {
					/* both cores are active */
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 1);
					words_per_us = words_per_us * 2;
					num_actv_cores = 2;
				} else {
					MOD_PHYREG(pi, SpecAnaDataCollect,
						single_or_dual_core_sel, 0);
					MOD_PHYREG(pi, RxFeStatus,
						saradc_packout_default_core_sel, coreSel);
					num_actv_cores = 1;
				}
			}
			MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, 4);
			MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]    - mode id=0
			 */
			mo = (((coreSel+1) << 12) | (num_actv_cores << 8) | 0);

			break;

		case  SC_MODE_5_iq_comp:

			if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
				mac_64_bits_enable = 1;
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				WRITE_PHYREG(pi, data_collect_crosscoresel0, 0x0000);
				WRITE_PHYREG(pi, data_collect_crosscoresel1, 0xfff0);
			}

			if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, SpecAnaDataCollect, wide64bus_sel, 1);
			}
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 5);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - always 0
				bits [11:8]   - number of cores (always 3)
				bits [7:0]	  - mode id=5
			 */
			mo = ((num_cores << 8) | 5);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_5_iq_comp);

			break;

		case  SC_MODE_5s_iq_comp:

			if (ACREV_IS(pi->pubpi->phy_rev, 0) ||
				ACREV_IS(pi->pubpi->phy_rev, 1) ||
				ACREV_IS(pi->pubpi->phy_rev, 2) ||
				ACREV_IS(pi->pubpi->phy_rev, 5)) {
				PHY_ERROR(("IQ_comp singlecore is not supported"
					" on phy rev 0, 1, 2, and 5\n"));
				/* Resume MAC */
				wlc_phy_conditional_resume(pi, &suspend);
				return BCME_UNSUPPORTED;
			}
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 5);

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			coreSel = phy_ac_sample_collect_core_sel(pi, coreSel);

			MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, coreSel);
			MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=5
			 */
			mo = ((coreSel << 12) | (1 << 8) | 5);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_5s_iq_comp);

			break;

		case  SC_MODE_6_dc_filt:
			if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
				mac_64_bits_enable = 1;
			if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
				WRITE_PHYREG(pi, data_collect_crosscoresel0, 0x0000);
				WRITE_PHYREG(pi, data_collect_crosscoresel1, 0xfff0);
			}

			MOD_PHYREG(pi, AdcDataCollect, sampSel, 6);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - always 0
				bits [11:8]   - number of cores (always 3)
				bits [7:0]	  - mode id=6
			 */
			mo = ((num_cores << 8) | 6);
			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_6_dc_filt);
			if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
					!(ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_6(pi)) &&
					!ACMAJORREV_32(pi->pubpi->phy_rev) &&
					!ACMAJORREV_33(pi->pubpi->phy_rev) &&
					!ACMAJORREV_37(pi->pubpi->phy_rev) &&
					!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pmu_chipctReg5_def = phy_ac_set_pmu_chipctl5(sampi);
				pmu_chip_ctl_regupdate = 1;
			}
			break;

		case  SC_MODE_6s_dc_filt:
			if (ACREV_IS(pi->pubpi->phy_rev, 0) ||
				ACREV_IS(pi->pubpi->phy_rev, 1) ||
				ACREV_IS(pi->pubpi->phy_rev, 2) ||
				ACREV_IS(pi->pubpi->phy_rev, 5)) {
				PHY_ERROR(("DC_filt singlecore is not supported"
					" on phy rev 0,1,2 and 5\n"));
				/* Resume MAC */
				wlc_phy_conditional_resume(pi, &suspend);
				return BCME_UNSUPPORTED;
			}
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 6);
			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			coreSel = phy_ac_sample_collect_core_sel(pi, coreSel);

			MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, coreSel);
			MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);
			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=6
			 */
			mo = ((coreSel << 12) | (1 << 8) | 6);
			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_6s_dc_filt);

			if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
					!(ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_6(pi)) &&
					!ACMAJORREV_32(pi->pubpi->phy_rev) &&
					!ACMAJORREV_33(pi->pubpi->phy_rev) &&
					!ACMAJORREV_37(pi->pubpi->phy_rev) &&
					!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pmu_chipctReg5_def = phy_ac_set_pmu_chipctl5(sampi);
				pmu_chip_ctl_regupdate = 1;
			}
			break;

		case  SC_MODE_7_rx_filt:

			if (ACMAJORREV_47_51(pi->pubpi->phy_rev))
				mac_64_bits_enable = 1;
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 7);
			if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);
			}

			if ((pi->bw == WL_CHANSPEC_BW_80) && !ACMAJORREV_32(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_33(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_37(pi->pubpi->phy_rev) &&
			    !ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - always 0
				bits [11:8]   - number of cores (always 3)
				bits [7:0]	  - mode id=7
			 */
			mo = ((num_cores << 8) | 7);
			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_7_rx_filt);

			if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
					!(ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_6(pi)) &&
					!ACMAJORREV_32(pi->pubpi->phy_rev) &&
					!ACMAJORREV_33(pi->pubpi->phy_rev) &&
					!ACMAJORREV_37(pi->pubpi->phy_rev) &&
					!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pmu_chipctReg5_def = phy_ac_set_pmu_chipctl5(sampi);
				pmu_chip_ctl_regupdate = 1;
			}
			break;

		case  SC_MODE_7s_rx_filt:
			if (ACREV_IS(pi->pubpi->phy_rev, 0) ||
				ACREV_IS(pi->pubpi->phy_rev, 1) ||
				ACREV_IS(pi->pubpi->phy_rev, 2) ||
				ACREV_IS(pi->pubpi->phy_rev, 5)) {
				PHY_ERROR(("rx_filt singlecore is not supported"
					"on phy rev 0,1,2 and 5\n"));
				/* Resume MAC */
				wlc_phy_conditional_resume(pi, &suspend);
				return BCME_UNSUPPORTED;
			}
			MOD_PHYREG(pi, AdcDataCollect, sampSel, 7);
			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			coreSel = phy_ac_sample_collect_core_sel(pi, coreSel);

			MOD_PHYREG(pi, AdcDataCollect, rxCoreSel, coreSel);
			MOD_PHYREG(pi, AdcDataCollect, rxSingleCoreMode, 1);
			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=7
			 */
			mo = ((coreSel << 12) | (1 << 8) | 7);
			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_7s_rx_filt);

			if (!ACMAJORREV_4(pi->pubpi->phy_rev) &&
					!(ACMAJORREV_3(pi->pubpi->phy_rev) && ACMINORREV_6(pi)) &&
					!ACMAJORREV_32(pi->pubpi->phy_rev) &&
					!ACMAJORREV_33(pi->pubpi->phy_rev) &&
					!ACMAJORREV_37(pi->pubpi->phy_rev) &&
					!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pmu_chipctReg5_def = phy_ac_set_pmu_chipctl5(sampi);
				pmu_chip_ctl_regupdate = 1;
			}
			break;

		case  SC_MODE_8_rssi:

			MOD_PHYREG(pi, AdcDataCollect, sampSel, 8);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - always 0
				bits [11:8]   - number of cores (always 3)
				bits [7:0]	  - mode id=8
			 */
			mo = ((num_cores << 8) | 8);
			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_8_rssi);

			break;

		case  SC_MODE_9_rssi_all:

			MOD_PHYREG(pi, AdcDataCollect, sampSel, 9);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - always 0
				bits [11:8]   - number of cores (always 3)
				bits [7:0]	  - mode id=9
			 */
			mo = ((num_cores << 8) | 9);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_9_rssi_all);
			break;

		case  SC_MODE_10_tx_farrow:

			MOD_PHYREG(pi, AdcDataCollect, sampSel, 10);

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			MOD_PHYREG(pi, AdcDataCollect, txCoreSel, coreSel);

			if (pi->bw == WL_CHANSPEC_BW_80) {
				MOD_PHYREG(pi, AdcDataCollect, downSample, 1);
				downSamp = 1;
			}

			/*	Mode
				bits [15:12]  - Tx coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=10
			 */
			mo = ((coreSel << 12) | (1 << 8) | 10);

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_10_tx_farrow);

			break;

		case  SC_MODE_11_gpio:

			save_gpio = READ_PHYREG(pi, gpioSel);

			/* Writing to lower 8 bits */
			WRITE_PHYREG(pi, gpioSel, ((save_gpio & 0xFF00)| collect->gpio_sel));

			MOD_PHYREG(pi, AdcDataCollect, gpioMode, 0);
			MOD_PHYREG(pi, AdcDataCollect, gpioSel,  1);

			/* gpioLoOutEn and gpioHiOutEn should be a non zero value.
			 * Else it will cause gpio_reset in RTL
			 */
			WRITE_PHYREG(pi, gpioLoOutEn, 0xFFFF);
			WRITE_PHYREG(pi, gpioHiOutEn, 0xFFFF);

			collect->gpio_collection = 1;

			mo = 0xFF;

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_11_gpio);

			break;

		case  SC_MODE_12_gpio_trans:

			save_gpio = READ_PHYREG(pi, gpioSel);

			/* Writing to lower 8 bits */
			WRITE_PHYREG(pi, gpioSel, ((save_gpio & 0xFF00)| collect->gpio_sel));

			WRITE_PHYREG(pi, gpioCapMaskHigh, ((collect->gpioCapMask >> 16) & 0xFFFF));
			WRITE_PHYREG(pi, gpioCapMaskLow, (collect->gpioCapMask  & 0xFFFF));

			MOD_PHYREG(pi, AdcDataCollect, gpioMode, 1);
			MOD_PHYREG(pi, AdcDataCollect, gpioSel,  1);

			/* gpioLoOutEn and gpioHiOutEn should be a non zero value
			 * Else it will cause gpio_reset in RTL
			 */
			WRITE_PHYREG(pi, gpioLoOutEn, 0xFFFF);
			WRITE_PHYREG(pi, gpioHiOutEn, 0xFFFF);

			collect->gpio_collection = 1;

			mo = 0x1FF;

			/*	Determine how many words need to be placed in the MAC buffer
				to capture 1us of the signal, in this sample capture mode
			 */
			words_per_us =
				phy_ac_words_per_us(sampi, sd_adc_rate, SC_MODE_12_gpio_trans);

			break;

		case  SC_MODE_14_spect_ana:

			if (support_spectrum_analyzer_mode == 0) {
				PHY_ERROR(("FFT is not supported on phy rev 0, 2, 4, and 5\n"));
				/* Resume MAC */
				wlc_phy_conditional_resume(pi, &suspend);
				return BCME_UNSUPPORTED;
			}

			if (ACREV_IS(pi->pubpi->phy_rev, 1)) {
				if (IS20MHZ(pi)) {
					MOD_PHYREG(pi, AdcDataCollect, specAnaModeDs, 1);
					fft_sampRate = 20;
				} else if (IS40MHZ(pi)) {
					MOD_PHYREG(pi, AdcDataCollect, specAnaModeDs, 0);
					fft_sampRate = 40;
				} else {
					MOD_PHYREG(pi, AdcDataCollect, specAnaModeDs, 3);
					fft_sampRate = 80;
				}
			} else {
				if (IS20MHZ(pi)) {
					MOD_PHYREG(pi, SpecAnaDataCollect, specAnaModeDs, 1);
					fft_sampRate = 20;
				} else if (IS40MHZ(pi)) {
					MOD_PHYREG(pi, SpecAnaDataCollect, specAnaModeDs, 0);
					fft_sampRate = 40;
				} else {
					MOD_PHYREG(pi, SpecAnaDataCollect, specAnaModeDs, 3);
					fft_sampRate = 80;
				}
			}

			coreSel = collect->cores;
			if ((coreSel < 0) || (coreSel > 3)) {
				coreSel = 0;
			}

			/* reset CCA */
			wlc_phy_resetcca_acphy(pi);

			if (ACREV_IS(pi->pubpi->phy_rev, 1)) {
				MOD_PHYREG(pi, SpectrumAnalyzerMode, saModeChan, coreSel);

				ACPHY_REG_LIST_START
					MOD_PHYREG_ENTRY(pi, fdiqi_rx_controller_bits,
						max_no_of_symbols, 0)
					MOD_PHYREG_ENTRY(pi, rxfdiqImbCompCtrl,
						calibration_notoperation, 1)

					MOD_PHYREG_ENTRY(pi, SpectrumAnalyzerMode, saModeEn, 1)
					MOD_PHYREG_ENTRY(pi, AdcDataCollect, specAnaMode, 1)
				ACPHY_REG_LIST_EXECUTE(pi);
			} else { /* phyrev >=2 */

				MOD_PHYREG(pi, SpecAnaDataCollect,
					specAnaModeChanSel, coreSel);
				MOD_PHYREG(pi, SpectrumAnalyzerMode, specAnaModeEn, 1);
				MOD_PHYREG(pi, SpecAnaDataCollect, specAnaMode, 1);
				FOREACH_CORE(pi, core) {
					MOD_PHYREGCE(pi, forceFront, core, freqEst, 1);
				}
			}

			/*	Mode
				bits [15:12]  - coreSel
				bits [11:8]   - number of cores (always 1)
				bits [7:0]	  - mode id=254
			 */
			mo = ((coreSel << 12) | (1 << 8) | 254);

			words_per_us =
				phy_ac_words_per_us(sampi, fft_sampRate, SC_MODE_14_spect_ana);
			break;

		default:
			break;
	}

	if ((collect->gpio_collection == 0) && (! ACMAJORREV_36(pi->pubpi->phy_rev))) {
		MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
	}

	/* duration(s): length sanity check and mapping from "max" to usec values */
	if (((collect->pre_dur + collect->post_dur) * words_per_us) > sc_buff_length) {
		PHY_ERROR(("wl%d: %s: Bad Duration Option\n", pi->sh->unit, __FUNCTION__));
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
		return BCME_RANGE;
	}

	/* be deaf if requested (e.g. for spur measurement) */
	if (collect->be_deaf) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	}
	wlc_btcx_override_enable(pi);

	/* perform AGC if requested */
	if (collect->agc) {
		/* Backup gain config prior to gain control --- Not beng implemented at the moment.
		   wlc_phy_agc_rxgain_config_acphy(pi, TRUE);
		   wlc_phy_agc_acphy(pi, agc_gain);
		 */
	} else {
		/* Set reported agc gains to init_gain with gain_error correction */
		int16 gainerr[PHY_CORE_MAX];

		wlc_phy_get_rxgainerr_phy(pi, gainerr);
		FOREACH_CORE(pi, core) {
			/* gainerr is in 0.5dB steps; needs to be rounded to nearest dB */
			int16 tmp = gainerr[core];
			tmp = ((tmp >= 0) ? ((tmp + 1) >> 1) : -1*((-1*tmp + 1) >> 1));
			agc_gain[core] = ACPHY_NOISE_INITGAIN + tmp;
		}
	}

	/* Apply filter settings if requested */
	/* Override the LPF high pass corners to their lowest values (0x1) */
	if (collect->filter)
		phy_ac_lpf_hpc_override(pi_ac, TRUE);

	/* set Tx-FIFO collect start pointer to sc_buff_start */
	phy_ac_set_startptr(sampi, sc_buff_start);
	sampi->pstart = sc_buff_start;

	/* Disable Mac clkgating */
	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		readreg = R_REG(pi->sh->osh, D11_PSMPowerReqStatus(pi));
		W_REG(pi->sh->osh, D11_PSMPowerReqStatus(pi),
			readreg & ~(1<<AUTO_MEM_STBY_RET_SHIFT));
		readreg = R_REG(pi->sh->osh, D11_POWERCTL(pi));
		W_REG(pi->sh->osh, D11_POWERCTL(pi),
			readreg & ~(1 << PWRCTL_ENAB_MEM_CLK_GATE_SHIFT));
		W_REG(pi->sh->osh, D11_POWERCTL(pi),
			readreg | (1 << PWRCTL_AUTO_MEM_STBYRET));
		readreg = R_REG(pi->sh->osh, D11_BMCCTL(pi));
		W_REG(pi->sh->osh, D11_BMCCTL(pi),
			readreg & ~(1 << 8));

		W_REG(pi->sh->osh, D11_CLK_GATE_REQ0(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_REQ1(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_REQ2(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ0(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ1(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ2(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_STRETCH0(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_STRETCH1(pi), 0x2000);
		W_REG(pi->sh->osh, D11_CLK_GATE_MISC(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_DIV_CTRL(pi), 0x22);
		W_REG(pi->sh->osh, D11_CLK_GATE_PHY_CLK_CTRL(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi), 0x9853);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ0(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ1(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ2(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_PHY_CLK_CTRL(pi), 0x0);
	}

	if (ACMAJORREV_36(pi->pubpi->phy_rev) || ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi), 0x0013);
	}
	PHY_TRACE(("wl%d: %s Start capture, trigger = %d\n", pi->sh->unit, __FUNCTION__,
		collect->trigger));

	timer = collect->timeout;

	PHY_TRACE(("wl%d: %s Start capture, timer = %d\n", pi->sh->unit, __FUNCTION__,
		collect->timeout));

	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		smp_ctl_save = R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
	}
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev)) {
		W_REG(pi->sh->osh, D11_SMP_CTRL(pi), 1 << 11);
	} else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		if (mo == 0x1ff) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi), 1 << 0);
		} else {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi), (1 << 0) | (1 << 2) |
				(mac_64_bits_enable << 11) | (0 << 12));
		}
	}
	if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		OSL_DELAY(20);
	}

	/* immediate trigger */
	if (collect->trigger == TRIGGER_NOW) {

		/* compute and set stop pointer */
		sampi->pstop = sc_buff_start +
			(collect->pre_dur + collect->post_dur) * words_per_us;

		if (sampi->pstop >= sc_buff_end - 1)
			sampi->pstop = sc_buff_end - 1;

		/* wlc_phy_sample_collect_now return FALSE if things timed out,
		 * later timer is checked to be zero and an error is returned
		 */
		timer = wlc_phy_sample_collect_now(sampi, collect, sc_buff_start) ? 1 : 0;

	} else { /* triggered sample collect */

		uint32 dur_1_8th_us;
		uint16 bred;
		uint32 phy_ctl = 0;
		uint32 play_ctl = 0;

		if (collect->gpio_collection == 0 && ACMAJORREV_36(pi->pubpi->phy_rev)) {
			MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 1);
		}

		/* enable mac and run psm */
		W_REG(pi->sh->osh, D11_MACCONTROL(pi), mac_ctl | MCTL_PSM_RUN | MCTL_EN_MAC);

		/* set stop pointer */
		sampi->pstop = sc_buff_end - 1;

		phy_ac_set_stopptr(sampi, (mac_64_bits_enable ? sampi->pstop :
			sampi->pstop - 1));

		/* set up post-trigger duration (expected by ucode in units of 1/8 us) */
		bred = R_REG(pi->sh->osh, D11_PSM_BRED_3(pi));
		W_REG(pi->sh->osh, D11_PSM_BRED_3(pi), bred | 0x10);
		dur_1_8th_us = collect->post_dur << 3;
		W_REG(pi->sh->osh, D11_TSF_GPT_2_CTR_L(pi), dur_1_8th_us & 0x0000FFFF);
		W_REG(pi->sh->osh, D11_TSF_GPT_2_CTR_H(pi), dur_1_8th_us >> 16);

		/* start ucode trigger-based sample collect procedure */
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi), 0x0);

		/* set Start bit (start capture) */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			play_ctl = R_REG(pi->sh->osh, D11_SMP_CTRL(pi));
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
				play_ctl | (1 << 0));
		} else {
			phy_ctl = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi));
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl | (1 << 4));
		}

		if (ISSIM_ENAB(pi->sh->sih)) {
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				OSL_DELAY(4000*(collect->pre_dur + 50));
			} else {
				OSL_DELAY(1000*(collect->pre_dur + 50));
			}
		} else {
			OSL_DELAY(collect->pre_dur + 50);
		}
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi), (int8)collect->trigger);
		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_CTL(pi), 1);

		PHY_TRACE(("wl%d: %s Wait for trigger ...\n", pi->sh->unit, __FUNCTION__));
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			/* check bit 0 and 2 which were set for capture */
			SPINWAIT(((val = R_REG(pi->sh->osh, D11_SMP_CTRL(pi))
				& 0x05) != 0), timer);
		} else {
			/* check bit 4 and 5 which were set for capture */
			SPINWAIT(((val = R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi))
				& 0x30) != 0), timer);
		}

		if (val != 0) {
			timer = 0;
			PHY_ERROR(("wl%d: %s timer expired without trigger fired.\n",
				pi->sh->unit, __FUNCTION__));
#ifdef ATE_BUILD
			printf("===> Timer expired without trigger fired\n");
#endif /* ATE BUILD */
		} else {
#ifdef ATE_BUILD
			printf("===> Trigger fired successfully\n");
#endif /* ATE BUILD */
		}

		/* set first and last pointer indices for readout */
		sampi->plast = phy_ac_get_currptr(sampi);
		pfirst_wrap_check = (sampi->plast -
			((collect->pre_dur + collect->post_dur) * words_per_us));

		sampi->pfirst = (sampi->plast -
				((collect->pre_dur + collect->post_dur) * words_per_us));
		/* Check for Wrap around */
		if ((pfirst_wrap_check < sampi->pstart) || pfirst_wrap_check <= 0) {
			sampi->pfirst = pfirst_wrap_check +
					phy_ac_sample_collect_length(sampi)+1;
		} else {
			sampi->pfirst = pfirst_wrap_check;
		}

		/* restore mac_ctl */
		W_REG(pi->sh->osh, D11_MACCONTROL(pi), mac_ctl);

		W_REG(pi->sh->osh, D11_PSM_BRED_3(pi), bred);

		/* erase trigger setup */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi),
				play_ctl & 0xFFFA);
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl & 0xFFCF);
		}

		/* Restore PHY_CTL to its original state */
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 54)) {
			W_REG(pi->sh->osh, D11_SMP_CTRL(pi), play_ctl);
		} else {
			W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), phy_ctl);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, M_SMPL_COL_BMP(pi), 0);
	}

	/* CLEAN UP: */
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
		ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		W_REG(pi->sh->osh, D11_SMP_CTRL(pi), smp_ctl_save);
	}
	wlc_phy_btcx_override_disable(pi);
	/* return from deaf if requested */
	if (collect->be_deaf) {
		phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
		wlapi_enable_mac(pi->sh->physhim);
	}

	/* revert to original gains if AGC was applied */
	if (collect->agc) {
		/* wlc_phy_agc_rxgain_config_acphy(pi, FALSE); */
	}

	/* Restore filter settings if changed */
	if (collect->filter)
		phy_ac_lpf_hpc_override(pi_ac, FALSE);

	/* turn off sample collect config in PHY & MAC */
	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* The whole register should be made 0. Else phy_sample_valid will be
		 * held high in RTL. This causes the last samples of the current capture
		 * to appear in the beginning of the next capture
		 */
		WRITE_PHYREG(pi, AdcDataCollect, 0);
	} else {
		MOD_PHYREG(pi, AdcDataCollect, adcDataCollectEn, 0);
	}

	if (support_spectrum_analyzer_mode == 1) {
		MOD_PHYREG(pi, SpectrumAnalyzerMode, specAnaModeEn, 0);
		if (!ACREV_IS(pi->pubpi->phy_rev, 1))
			MOD_PHYREG(pi, SpecAnaDataCollect, specAnaMode, 0);
		FOREACH_CORE(pi, core) {
			WRITE_PHYREGCE(pi, forceFront, core, forceFront_save[core]);
		}
	}

	/* Restore RxFeTesMmuxCtrl register */
	MOD_PHYREG(pi, RxFeTesMmuxCtrl, rxfe_dbg_mux_sel, RxFeTesMmuxCtrl_rxfe_dbg_mux_sel_save);

	if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		/* Restore SpecAnaDataCollect register */
		MOD_PHYREG(pi, SpecAnaDataCollect, wide64bus_sel, wide64bus_sel_save);

		/* Restore clkgatests register */
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi), save_clkgatests);
	}

	if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
		/* Restore clkgatests register */

		readreg = R_REG(pi->sh->osh, D11_PSMPowerReqStatus(pi));
		W_REG(pi->sh->osh, D11_PSMPowerReqStatus(pi),
				readreg | (1<<AUTO_MEM_STBY_RET_SHIFT));
		readreg = R_REG(pi->sh->osh, D11_POWERCTL(pi));
		W_REG(pi->sh->osh, D11_POWERCTL(pi),
				readreg | (1 << PWRCTL_ENAB_MEM_CLK_GATE_SHIFT));
		W_REG(pi->sh->osh, D11_POWERCTL(pi),
				readreg | (1 << PWRCTL_AUTO_MEM_STBYRET));
		readreg = R_REG(pi->sh->osh, D11_BMCCTL(pi));
		W_REG(pi->sh->osh, D11_BMCCTL(pi),
				readreg | (1 << 8));

		W_REG(pi->sh->osh, D11_CLK_GATE_REQ0(pi), 0xffff);
		W_REG(pi->sh->osh, D11_CLK_GATE_REQ1(pi), 0xffff);
		W_REG(pi->sh->osh, D11_CLK_GATE_REQ2(pi), 0x3ff);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ0(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ1(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_REQ2(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_STRETCH0(pi), 0x1010);
		W_REG(pi->sh->osh, D11_CLK_GATE_STRETCH1(pi), 0x2000);
		W_REG(pi->sh->osh, D11_CLK_GATE_MISC(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_DIV_CTRL(pi), 0x27);
		W_REG(pi->sh->osh, D11_CLK_GATE_PHY_CLK_CTRL(pi), 0x7fff);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ0(pi), 0xffff);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ1(pi), 0xff);
		W_REG(pi->sh->osh, D11_CLK_GATE_EXT_REQ2(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_UCODE_PHY_CLK_CTRL(pi), 0x0);
		W_REG(pi->sh->osh, D11_CLK_GATE_STS(pi), save_clkgatests);
	}

	/* Restore Gpio register */
	WRITE_PHYREG(pi, gpioLoOutEn, gpioLo_save);
	WRITE_PHYREG(pi, gpioHiOutEn, gpioHi_save);

	/* Note: This line is required in order to read from the TX MAC FIFO */
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), 0x2);

	/* Note: This line is needed in order to successfully restore MAC based sample play */
	/*       at the end of the proc in case it is being used */
	MOD_PHYREG(pi, macbasedDACPlay, macBasedDACPlayEn, 0);

	/* Restore acphyreg(macbasedDACPlay.macBasedDACPlayEn) to its original state */
	MOD_PHYREG(pi, macbasedDACPlay,
		macBasedDACPlayEn, macBasedDACPlayEn_save);

	/* abort if timeout ocurred */
	if (timer == 0) {
		PHY_ERROR(("wl%d: %s Error: Timeout\n", pi->sh->unit, __FUNCTION__));
		if (pmu_chip_ctl_regupdate == 1) {
			si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5,
					0xFFFFFFFF, pmu_chipctReg5_def);
		}
		/* restore gpio lo/hi out enable config */
		WRITE_PHYREG(pi, gpioLoOutEn, gpioLo_save);
		WRITE_PHYREG(pi, gpioHiOutEn, gpioHi_save);
		/* Resume MAC */
		wlc_phy_conditional_resume(pi, &suspend);
		return BCME_NOTFOUND;
	}

	PHY_TRACE(("wl%d: %s: Capture successful\n", pi->sh->unit, __FUNCTION__));
#ifdef ATE_BUILD
	printf("===> Capture successful\n");
#endif /* ATE_BUILD */

	if (sampi->pfirst > sampi->plast) {
		cnt = sampi->pstop - sampi->pfirst + 1;
		cnt += sampi->plast;
		cnt -= sampi->pstart;
	} else {
		cnt = sampi->plast - sampi->pfirst;
	}

	sample_data->tag = htol16(WL_SAMPLEDATA_HEADER_TYPE);
	sample_data->length = htol16((WL_SAMPLEDATA_HEADER_SIZE));
	sample_data->flag = 0;		/* first sequence */
	sample_data->flag |= htol32(WL_SAMPLEDATA_MORE_DATA);

	sample_rate = CHSPEC_IS40(pi->radio_chanspec) ? 80 : 40;
	sample_bitwidth = 10;
	/* Hack in conversion factor for subtracting from adc sample
	 * power to obtain true analog power in dBm:
	 */
	dBsample_to_dBm_sub = 49;

	/* store header to buf */
	ptr[0] = htol32(0xACDC2009);
	ptr[1] = htol32(0xFFFF0000 | (FILE_HDR_LEN<<8));
	ptr[2] = htol32(cnt % (phy_ac_sample_collect_length(sampi) + 1));
	ptr[3] = htol32(0xFFFF0000 | (pi->pubpi->phy_rev<<8) | pi->pubpi->phy_type);
	ptr[4] = htol32(0xFFFFFF00);
	ptr[5] = htol32(((fc / 100) << 24) | ((fc % 100) << 16) | (num_cores << 8) |
		(CHSPEC_BW_LE20(pi->radio_chanspec) ? 20 :
		(CHSPEC_IS40(pi->radio_chanspec) ? 40 :
		(CHSPEC_IS80(pi->radio_chanspec) ? 80 : 160))));
	ptr[6] = htol32((collect->gpio_sel << 24) | (((mo >> 8)  & 0xff) << 16) |
		((mo & 0xff) << 8) | collect->gpio_collection);
	ptr[7] = htol32(0xFFFF0000 | (downSamp << 8) | collect->trigger);
	if (collect->mode >= 19 && collect->mode <= 23) {
		ptr[8] = htol32(0xFFFFFF00 | bit_unpacking_mode_28nm);
	} else {
		ptr[8] = htol32(0xFFFFFFFF);
	}
	ptr[9] = htol32((collect->post_dur << 16) | collect->pre_dur);
	FOREACH_CORE(pi, core) {
		ptr[10+core] = htol32((READ_PHYREGCE(pi, Core1RxIQCompA, core)) |
			(READ_PHYREGCE(pi, Core1RxIQCompB, core) << 16));
	}
	if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
	    ACMAJORREV_33(pi->pubpi->phy_rev) ||
	    ACMAJORREV_37(pi->pubpi->phy_rev) ||
	    ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		ptr[14] = htol32(((collect->filter ? 1 : 0) << 24) |
			((collect->agc ? 1 : 0) << 16) | (sample_rate << 8) | sample_bitwidth);
		ptr[15] = htol32(((dBsample_to_dBm_sub << 16) | agc_gain[0]));
		if (num_cores > 1) {
			ptr[16] = htol32((((num_cores > 2) ? agc_gain[2] : 0xFFFF) << 16) |
				agc_gain[1]);
			ptr[17] = htol32(0xFFFF0000 | ((num_cores > 3) ? agc_gain[3] : 0xFFFF));
		} else {
			ptr[16] = htol32(0xFFFFFFFF);
			ptr[17] = htol32(0xFFFFFFFF);
		}
	} else {
		ptr[13] = htol32(((collect->filter ? 1 : 0) << 24) |
			((collect->agc ? 1 : 0) << 16) | (sample_rate << 8) | sample_bitwidth);
		ptr[14] = htol32(((dBsample_to_dBm_sub << 16) | agc_gain[0]));
		if (num_cores > 1) {
			ptr[15] = htol32((((num_cores > 2) ? agc_gain[2] : 0xFFFF) << 16) |
				agc_gain[1]);
		} else {
			ptr[15] = htol32(0xFFFFFFFF);
		}
		ptr[16] = htol32(0xFFFFFFFF);
		ptr[17] = htol32(0xFFFFFFFF);
	}
	ptr[18] = htol32(0xFFFFFFFF);
	ptr[19] = htol32(0xFFFFFFFF);
	PHY_TRACE(("wl%d: %s: pfirst 0x%x plast 0x%x pstart 0x%x pstop 0x%x\n", pi->sh->unit,
		__FUNCTION__, sampi->pfirst, sampi->plast, sampi->pstart, sampi->pstop));
	PHY_TRACE(("wl%d: %s Capture length = %d words\n", pi->sh->unit, __FUNCTION__, cnt));

	if (pmu_chip_ctl_regupdate == 1) {
		si_pmu_chipcontrol(pi->sh->sih, PMU_CHIPCTL5, 0xFFFFFFFF, pmu_chipctReg5_def);
	}

	/*
	 * Restore the original mac_ctl value.
	 * For ATE sample capture, even if mac is reenabled at this point, there is a burst of
	 * glitches at around 260us.
	 */
#ifndef ATE_BUILD
	if (CHIPID(pi->sh->chip) == BCM4345_CHIP_ID)
		W_REG(pi->sh->osh, D11_MACCONTROL(pi), mac_ctl);
#endif /* ATE_BUILD */
	if (ACMAJORREV_4(pi->pubpi->phy_rev) &&
		phy_get_phymode(pi) == PHYMODE_MIMO) {
		WRITE_PHYREG(pi, data_collect_crosscoresel0, 0x0000);
		WRITE_PHYREG(pi, data_collect_crosscoresel1, 0x0000);
	}
	/* restore gpio lo/hi out enable config */
	WRITE_PHYREG(pi, gpioLoOutEn, gpioLo_save);
	WRITE_PHYREG(pi, gpioHiOutEn, gpioHi_save);
	/* Resume MAC */
	wlc_phy_conditional_resume(pi, &suspend);
	return BCME_OK;
}

int
phy_ac_sample_data(phy_type_samp_ctx_t *ctx, wl_sampledata_t *sample_data, void *b)
{
	int	i;
	uint32	data, cnt, bufsize, seq;

	phy_ac_samp_info_t *sampi = (phy_ac_samp_info_t *)ctx;
	phy_info_t *pi = sampi->pi;

	uint8*	head = (uint8 *)b;
	uint32*	buf = (uint32 *)(head + sizeof(wl_sampledata_t));

	/* buf2 is used when sampledata is unpacked for
	 * version WL_SAMPLEDATA_T_VERSION_SPEC_AN
	 */
	int16*	buf2 = (int16 *)(head + sizeof(wl_sampledata_t));

	bufsize = ltoh32(sample_data->length) - sizeof(wl_sampledata_t);

	if (sample_data->version == (WL_SAMPLEDATA_T_VERSION_SPEC_AN)) {
		/* convert to # of (4*num_cores)--byte  words */
		bufsize = bufsize / (PHYCORENUM(pi->pubpi->phy_corenum) * 4);
	} else {
		/* convert to # of 4--byte words */
		bufsize = bufsize >> 2;
	}

	/* get the last sequence number */
	seq = ltoh32(sample_data->flag) & 0xff;

	/* Saturate sequence number to 0xff */
	seq = (seq < 0xff) ? (seq + 1) : 0xff;

	/* write back to data struct */
	sample_data->flag = htol32(seq);

	PHY_TRACE(("wl%d: %s: bufsize(words) %d flag 0x%x\n", pi->sh->unit, __FUNCTION__,
		bufsize, sample_data->flag));

	wlapi_bmac_templateptr_wreg(pi->sh->physhim, sampi->pfirst << 2);

	/* Currently only 3 cores (and collect mode 0) are supported
	 * for version WL_SAMPLEDATA_T_VERSION_SPEC_AN
	 */

	if ((sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) &&
		(PHYCORENUM(pi->pubpi->phy_corenum) != 3))  {
		/* No more data to read */
		sample_data->flag = sample_data->flag & 0xff;
		/* No bytes were read */
		sample_data->length = 0;

		bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));

		PHY_ERROR(("wl%d: %s: Number of cores != 3\n",
			pi->sh->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	/* Initialization for version WL_SAMPLEDATA_T_VERSION_SPEC_AN: */
	if ((sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN) && (seq == 1)) {
		bool capture_start = FALSE;

		/* Search for and sync to a sample with a valid 2-bit '00' alignment pattern */
		while ((!capture_start) && (sampi->pfirst != sampi->plast)) {
			data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
			/* wrap around end of fifo if necessary */
			if (sampi->pfirst == sampi->pstop) {
				wlapi_bmac_templateptr_wreg(pi->sh->physhim, sampi->pstart << 2);
				sampi->pfirst = sampi->pstart;
				PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
					pi->sh->unit, __FUNCTION__));
			} else {
				sampi->pfirst++;
			}

			/* Check for alignment pattern 0x3 in the captured word */
			if (((data >> 30) & 0x3) == 0x3) {
				/* Read and discard one 32-bit word to
				 * move to where the next sample
				 * (with alignment pattern '00') starts
				 */
				data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);

				/* wrap around end of fifo if necessary */
				if (sampi->pfirst == sampi->pstop) {
					wlapi_bmac_templateptr_wreg(pi->sh->physhim,
						sampi->pstart << 2);
					sampi->pfirst = sampi->pstart;
					PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
						pi->sh->unit, __FUNCTION__));
				} else {
					sampi->pfirst++;
				}

				if (sampi->pfirst != sampi->plast) {
					capture_start = TRUE;
				}
			}
		}

		if (capture_start == FALSE) {
			/* ERROR: No starting pattern was found! */
			/* No more data to read */
			sample_data->flag = sample_data->flag & 0xff;

			/* No bytes were read */
			sample_data->length = 0;

			bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));
			PHY_ERROR(("wl%d: %s: Starting pattern not found! \n",
				pi->sh->unit, __FUNCTION__));
			return BCME_ERROR;
		}
	}

	/* start writing samples to buffer */
	cnt = 0;

	while ((cnt < bufsize) && (sampi->pfirst != sampi->plast))
	{
		if (sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN)
		{
			/* Unpack collected samples and write to buffer */
			uint32 data1[2];
			int16 isample[PHY_CORE_MAX];
			int16 qsample[PHY_CORE_MAX];

			for (i = 0; ((i < 2) && (sampi->pfirst != sampi->plast)); i++)
			{
				data1[i] = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
				/* wrap around end of fifo if necessary */
				if (sampi->pfirst == sampi->pstop) {
					wlapi_bmac_templateptr_wreg(pi->sh->physhim,
						sampi->pstart << 2);
					sampi->pfirst = sampi->pstart;
					PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
						pi->sh->unit, __FUNCTION__));
				} else {
					sampi->pfirst++;
				}
			}

			/* Unpack samples only if two 32-bit words have
			 * been successfully read from TX FIFO
			 */
			if (i == 2) {
				/* Unpack and perform sign extension: */
				uint16 temp;

				/* Core 0: */
				temp = (uint16)(data1[0] & 0x3ff);
				isample[0] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)((data1[0] >> 10) & 0x3ff);
				qsample[0] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Core 1: */
				temp = (uint16)((data1[0] >> 20) & 0x3ff);
				isample[1] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)(data1[1] & 0x3ff);
				qsample[1] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Core 2: */
				temp = (uint16)((data1[1] >> 10) & 0x3ff);
				isample[2] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;
				temp = (uint16)((data1[1] >> 20) & 0x3ff);
				qsample[2] = (temp & 0x200) ? (int16)(temp | 0xfc00) : temp;

				/* Write to buffer in 2-byte words */
				buf2[6*cnt]     = isample[0];
				buf2[6*cnt + 1] = qsample[0];
				buf2[6*cnt + 2] = isample[1];
				buf2[6*cnt + 3] = qsample[1];
				buf2[6*cnt + 4] = isample[2];
				buf2[6*cnt + 5] = qsample[2];

				cnt++;
			}

		} else {
			/* Write collected samples as-is to buffer */
			data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
			/* write one 4-byte word */
			buf[cnt++] = htol32(data);
			/* wrap around end of fifo if necessary */
			if (sampi->pfirst == sampi->pstop) {
				wlapi_bmac_templateptr_wreg(pi->sh->physhim, sampi->pstart << 2);
				sampi->pfirst = sampi->pstart;
				PHY_TRACE(("wl%d: %s TXFIFO wrap around\n",
					pi->sh->unit, __FUNCTION__));
			} else {
				sampi->pfirst++;
			}
		}
	}

	PHY_TRACE(("wl%d: %s: Data fragment completed (pfirst %d plast %d)\n",
		pi->sh->unit, __FUNCTION__, sampi->pfirst, sampi->plast));

	if (sampi->pfirst != sampi->plast)
		sample_data->flag |= htol32(WL_SAMPLEDATA_MORE_DATA);

	/* update to # of bytes read */
	if (sample_data->version == WL_SAMPLEDATA_T_VERSION_SPEC_AN)
		sample_data->length = htol16((cnt * 4 * PHYCORENUM(pi->pubpi->phy_corenum)));
	else
		sample_data->length = htol16((cnt << 2));

	bcopy((uint8 *)sample_data, head, sizeof(wl_sampledata_t));

	PHY_TRACE(("wl%d: %s: Capture length = %d words\n", pi->sh->unit, __FUNCTION__, cnt));

	return BCME_OK;
}
static int phy_ac_sample_collect_core_sel(phy_info_t *pi, int coresel)
{
	uint16 val;
	if (ACMAJORREV_4(pi->pubpi->phy_rev) &&
		phy_get_phymode(pi) == PHYMODE_MIMO) {
		if (coresel)
			val = 0xffff;
		else
			val = 0;
		WRITE_PHYREG(pi, data_collect_crosscoresel0, val);
		WRITE_PHYREG(pi, data_collect_crosscoresel1, val);
		coresel = 0;
	} else if (ACMAJORREV_36(pi->pubpi->phy_rev)) {
		coresel = 0;
	}
	return coresel;
}

/* SETUP/CLEANUP routine for high-pass corner (HPC) of LPF
 * (arg == 1) Setup	: Save LPF config and set HPC to lowest value (0x1)
 * (arg == 0) Cleanup	: Restore HPC config
 */
void
phy_ac_lpf_hpc_override(phy_ac_info_t *aci, bool setup)
{
	phy_info_t *pi = aci->pi;
	acphy_lpfCT_phyregs_t *porig = aci->sampi->ac_lpfCT_phyregs_orig;
	uint8 core, stall_val;
	uint16 tmp_tia_hpc, tmp_lpf_dc_bw, val_tia_hpc, val_lpf_dc_bw;

	if (setup) {
		ASSERT(!porig->is_orig);
		porig->is_orig = TRUE;

		stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
		ACPHY_DISABLE_STALL(pi);

		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x122, 16, &tmp_tia_hpc);
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_RFSEQ, 1, 0x125, 16, &tmp_lpf_dc_bw);

		ACPHY_ENABLE_STALL(pi, stall_val);

		FOREACH_CORE(pi, core) {
			porig->RfctrlOverrideLpfCT[core] =
				READ_PHYREGCE(pi, RfctrlOverrideLpfCT, core);
			porig->RfctrlCoreLpfCT[core] =
				READ_PHYREGCE(pi, RfctrlCoreLpfCT, core);

			val_tia_hpc = (tmp_tia_hpc >> (core * 4)) & 0xf;
			val_lpf_dc_bw = (tmp_lpf_dc_bw >> (core * 4)) & 0xf;

			MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, tia_HPC,  1);
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT,     core, tia_HPC, val_tia_hpc);

			MOD_PHYREGCE(pi, RfctrlOverrideLpfCT, core, lpf_dc_bw, 1);
			MOD_PHYREGCE(pi, RfctrlCoreLpfCT,     core, lpf_dc_bw, val_lpf_dc_bw);
		}
	} else { /* cleanup */
		ASSERT(porig->is_orig);
		porig->is_orig = FALSE;

		FOREACH_CORE(pi, core) {
			WRITE_PHYREGCE(pi, RfctrlOverrideLpfCT, core,
				porig->RfctrlOverrideLpfCT[core]);
			WRITE_PHYREGCE(pi, RfctrlCoreLpfCT, core, porig->RfctrlCoreLpfCT[core]);
		}
	}
}
#endif /* SAMPLE_COLLECT */

#ifdef IQPLAY_DEBUG
static int
phy_ac_samp_mac_IQplay(phy_ac_samp_info_t *sampi, const uint32* buf, uint16 len, bool start)
{
	phy_info_t *pi = sampi->pi;

	if (start) {
		uint32 startidx = START_IDX_ADDR;
		if (!(ACMAJORREV_36(pi->pubpi->phy_rev) && !ACMINORREV_0(pi))) {

			/* Load the waveform into MAC buffer */
			ASSERT(buf != NULL);
			/* multiply len by 4 since function expects length in bytes */
			if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
				unsigned long axi_base = AXI_BASE_ADDR;
				if (wlapi_si_coreunit(pi->sh->physhim) == 1) {
					axi_base |= MAIN_AUX_CORE_ADDR_OFFSET;
				}

				unsigned long bm_base_offset = BM_BASE_OFFFSET_ADDR;
				unsigned long axi_bm_addr = (axi_base + bm_base_offset + startidx);

				memcpy((unsigned long *)axi_bm_addr, buf,
					len*sizeof(uint32));
			}
		}
		W_REG(pi->sh->osh, &pi->regs->u.d11acregs.SamplePlayStartPtr, startidx/4);
		W_REG(pi->sh->osh, &pi->regs->u.d11acregs.SamplePlayStopPtr, (startidx/4)+len-1);

		if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			uint32 stall_val;
			stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
			ACPHY_DISABLE_STALL(pi);
			MOD_PHYREG(pi, sampleCmd, enable, 0x1);
			ACPHY_ENABLE_STALL(pi, stall_val);
		}
		/* Play the Waveform */
		phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
			ACPHY_macbasedDACPlay_macBasedDACPlayEn_MASK(phy_rev));

		if (!(ACMAJORREV_36(pi->pubpi->phy_rev))) {
			phy_utils_or_phyreg(pi, ACPHY_macbasedDACPlay(phy_rev),
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_MASK(phy_rev) & (0x1 <<
			ACPHY_macbasedDACPlay_macBasedDACPlayMode_SHIFT(phy_rev)));
		}

		PHY_TRACE(("Starting MAC based Sample Play"));
		wlc_phy_force_rfseq_acphy(pi, ACPHY_RFSEQ_RX2TX);

		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
			uint16 SampleCollectPlayCtrl =
				R_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl);
			SampleCollectPlayCtrl |= (1 << SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT);
			W_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl,
				SampleCollectPlayCtrl);
		}
	} else {
		/* To stop playback */
		bool mac_sample_play_on = 0;
		if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
			uint16 SampleCollectPlayCtrl =
				R_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl);
			mac_sample_play_on =
				((SampleCollectPlayCtrl >>
					SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT) & 1);

			if (mac_sample_play_on) {
				/* Make the 9th bit zero */
				SampleCollectPlayCtrl &= 0xFDFF;
				W_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl,
					SampleCollectPlayCtrl);
			}
		}
		/* Stop MAC play  by setting macBasedDACPlayEn to 0 */
		if (ACMAJORREV_GE40_NE47_NE51(pi->pubpi->phy_rev)) {
			uint32 stall_val;
			stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
			ACPHY_DISABLE_STALL(pi);
			MOD_PHYREG(pi, macbasedDACPlay, macBasedDACPlayEn, 0);
			MOD_PHYREG(pi, sampleCmd, enable, 0x0);
			ACPHY_ENABLE_STALL(pi, stall_val);
		}
	} /* End of start */
	return BCME_OK;
}

static void
phy_ac_samp_prep_IQplay(phy_type_samp_ctx_t *ctx)
{
	uint8 stall_val = 0;
	uint16  wbcal_waveform_sz;
	const uint32 *macplay_wfm_ptr;
	phy_ac_samp_info_t *sampi = (phy_ac_samp_info_t *)ctx;
	phy_info_t *pi = sampi->pi;

	/* Prepare Mac and Phregs */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	/* disable Stalls  before mac play and restore it later */
	phy_ac_samp_mac_IQplay(sampi, NULL, 0, OFF);
	OSL_DELAY(2);

	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);

	MOD_PHYREG(pi, RxFeCtrl1, disable_stalls, 0x1);
	MOD_PHYREG(pi, sampleCmd, stop, 0x1);

	/* Transmit packet from MAC FIFO */
	/* sizeof gives sizeof array in bytes and we want it in words */

	wbcal_waveform_sz = ARRAYSIZE(phy_ping_waveform_11ax_test);
	macplay_wfm_ptr = (const uint32 *)&phy_ping_waveform_11ax_test[0];

	if (D11REV_IS(pi->sh->corerev, 50) || D11REV_GE(pi->sh->corerev, 53)) {
		uint16 SampleCollectPlayCtrl =
			R_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl);
		SampleCollectPlayCtrl |= (1 << SAMPLE_COLLECT_PLAY_CTRL_PLAY_MODE_SHIFT);
		W_REG(pi->sh->osh, &pi->regs->PHYREF_SampleCollectPlayCtrl,
			SampleCollectPlayCtrl);
	}

	phy_ac_samp_mac_IQplay(sampi, macplay_wfm_ptr, wbcal_waveform_sz, ON);

	ACPHY_ENABLE_STALL(pi, stall_val);

	OSL_DELAY(150);

	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* IQPLAY_DEBUG */
