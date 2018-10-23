/*
 * NOISEmeasure module implementation.
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
 * $Id: phy_noise.c 753957 2018-03-23 11:56:39Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_init.h>
#include <phy_ac_info.h>
#include "phy_type_noise.h"
#include <phy_noise.h>
#include <phy_noise_api.h>
#include <phy_ac_noise.h>
#include <wlc_phyreg_n.h>

/* module private states */
struct phy_noise_info {
	phy_info_t *pi;
	phy_type_noise_fns_t *fns;
	int bandtype;
};

/* module private states memory layout */
typedef struct {
	phy_noise_info_t info;
	phy_type_noise_fns_t fns;
/* add other variable size variables here at the end */
} phy_noise_mem_t;

/* local function declaration */
static int phy_noise_reset(phy_init_ctx_t *ctx);
static int phy_noise_init(phy_init_ctx_t *ctx);
#if defined(BCMDBG)
static int phy_noise_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
#if defined(BCMDBG) || defined(WLTEST)
static int phy_aci_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static bool phy_noise_start_wd(phy_wd_ctx_t *ctx);
static bool phy_noise_stop_wd(phy_wd_ctx_t *ctx);
static bool phy_noise_reset_wd(phy_wd_ctx_t *ctx);

/* attach/detach */
phy_noise_info_t *
BCMATTACHFN(phy_noise_attach)(phy_info_t *pi, int bandtype)
{
	phy_noise_info_t *info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((info = phy_malloc(pi, sizeof(phy_noise_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	info->pi = pi;
	info->fns = &((phy_noise_mem_t *)info)->fns;

	/* Initialize noise params */
	info->bandtype = bandtype;

	pi->interf->curr_home_channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	pi->sh->interference_mode_override = FALSE;
	pi->sh->interference_mode_2G = INTERFERE_NONE;
	pi->sh->interference_mode_5G = INTERFERE_NONE;

	/* Default to 60. Each PHY specific attach should initialize it
	 * to PHY/chip specific.
	 */
	pi->aci_exit_check_period = 60;
	pi->aci_enter_check_period = 16;
	pi->aci_state = 0;

	pi->interf->scanroamtimer = 0;
	pi->interf->rssi_index = 0;
	pi->interf->rssi = 0;

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_noise_start_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_START,
	                  PHY_WD_FLAG_SCAN_SKIP|PHY_WD_FLAG_PLT_SKIP) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_noise_stop_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_STOP,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_noise_reset_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_RESET,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register reset fn */
	if (phy_init_add_init_fn(pi->initi, phy_noise_reset, info, PHY_INIT_NOISERST)
			!= BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register init fn */
	if (phy_init_add_init_fn(pi->initi, phy_noise_init, info, PHY_INIT_NOISE) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn failed\n", __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG)
	/* register dump callback */
	phy_dbg_add_dump_fn(pi, "phynoise", phy_noise_dump, info);
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
	phy_dbg_add_dump_fn(pi, "phyaci", phy_aci_dump, info);
#endif // endif

	return info;

	/* error */
fail:
	phy_noise_detach(info);
	return NULL;
}

void
BCMATTACHFN(phy_noise_detach)(phy_noise_info_t *info)
{
	phy_info_t *pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info == NULL) {
		PHY_INFORM(("%s: null noise module\n", __FUNCTION__));
		return;
	}

	pi = info->pi;

	phy_mfree(pi, info, sizeof(phy_noise_mem_t));
}

/* watchdog callbacks */
static bool
phy_noise_start_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_info_t *pi = nxi->pi;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx_noise = (phy_type_noise_ctx_t *) fns->ctx;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* update phy noise moving average only if no scan or rm in progress */
	ASSERT(fns->request_noise_sample);
	fns->request_noise_sample(ctx_noise, PHY_NOISE_SAMPLE_MON,
		CHSPEC_CHANNEL(pi->radio_chanspec));

	return TRUE;
}

static bool
phy_noise_stop_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = nxi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->stop != NULL)
		(fns->stop)(fns->ctx);

	return TRUE;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_noise_register_impl)(phy_noise_info_t *nxi, phy_type_noise_fns_t *fns)
{
	phy_info_t *pi = nxi->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	*nxi->fns = *fns;

	/* N.B.: This is the signal of module attach completion. */
	/* Summarize the PHY type setting... */
	if (BAND_2G(nxi->bandtype)) {
		pi->sh->interference_mode =
			pi->sh->interference_mode_2G;
	} else {
		pi->sh->interference_mode =
			pi->sh->interference_mode_5G;
	}

	return BCME_OK;
}

void
BCMATTACHFN(phy_noise_unregister_impl)(phy_noise_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* change mode */
int
phy_noise_set_mode(phy_noise_info_t *ii, int wanted_mode, bool init)
{
	phy_type_noise_fns_t *fns = ii->fns;
	phy_info_t *pi = ii->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

	if (fns->mode != NULL)
		(fns->mode)(fns->ctx, wanted_mode, init);

	pi->cur_interference_mode = wanted_mode;
	return BCME_OK;
}

static bool
phy_noise_reset_wd(phy_wd_ctx_t *ctx)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_info_t *pi = nxi->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* reset phynoise state if ucode interrupt doesn't arrive for so long */
	if (pi->phynoise_state && (pi->sh->now - pi->phynoise_now) > 5) {
		PHY_TMP(("wlc_phy_watchdog: ucode phy noise sampling overdue\n"));

		pi->phynoise_state = 0;
	}

	return TRUE;
}

int
phy_noise_bss_init(phy_noise_info_t *noisei, int noise)
{
	phy_info_t *pi = noisei->pi;
	uint i,	core;

	/* watchdog idle phy noise */
	for (i = 0; i < MA_WINDOW_SZ; i++) {
		pi->sh->phy_noise_window[i] = (int8)(noise & 0xff);
	}
	pi->sh->phy_noise_index = 0;

	if ((pi->sh->interference_mode == WLAN_AUTO) &&
	     (pi->aci_state & ACI_ACTIVE)) {
		/* Reset the clock to check again after the moving average buffer has filled
		 */
		pi->aci_start_time = pi->sh->now + MA_WINDOW_SZ;
	}

	for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
		FOREACH_CORE(pi, core)
			pi->phy_noise_win[core][i] = PHY_NOISE_FIXED_VAL_NPHY;
	}
	pi->phy_noise_index = 0;

	return BCME_OK;
}

/* noise calculation */
void
wlc_phy_noise_calc(phy_info_t *pi, uint32 *cmplx_pwr, int8 *pwr_ant, uint8 extra_gain_1dB)
{
	int8 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	bzero((uint8 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) <= PHY_CORE_MAX);

#ifdef WL_EAP_NOISE_MEASUREMENTS
	PHY_INFORM(("--> RXGAIN: %d\n", wlapi_bmac_read_shm(pi->sh->physhim,
		M_PWRIND_BLKS(pi)+0x10)));
#else
	PHY_INFORM(("--> RXGAIN: %d\n", wlapi_bmac_read_shm(pi->sh->physhim,
		M_PWRIND_BLKS(pi)+0xC)));
#endif /* WL_EAP_NOISE_MEASUREMENTS */

	math_cmplx_computedB(cmplx_pwr, cmplx_pwr_dbm, PHYCORENUM(pi->pubpi->phy_corenum));

	if (fns->calc != NULL)
		(fns->calc)(fns->ctx, cmplx_pwr_dbm, extra_gain_1dB);
	FOREACH_CORE(pi, i) {
		pwr_ant[i] = cmplx_pwr_dbm[i];
		PHY_INFORM(("wlc_phy_noise_calc_phy: pwr_ant[%d] = %d\n", i, pwr_ant[i]));
	}

	PHY_INFORM(("%s: samples %d ant %d\n", __FUNCTION__, pi->phy_rxiq_samps,
		pi->phy_rxiq_antsel));
}

void
wlc_phy_noise_calc_fine_resln(phy_info_t *pi, uint32 *cmplx_pwr, uint16 *crsmin_pwr,
		int16 *pwr_ant, uint8 extra_gain_1dB, int16 *tot_gain)
{
	int16 cmplx_pwr_dbm[PHY_CORE_MAX];
	uint8 i;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;

	/* lookup table for computing the dB contribution from the first
	 * 4 bits after MSB (most significant NONZERO bit) in cmplx_pwr[core]
	 * (entries in multiples of 0.25dB):
	 */
	uint8 dB_LUT[] = {0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 8, 9,
		10, 10, 11, 11};
	uint8 LUT_correction[] = {13, 12, 12, 13, 16, 20, 25,
		5, 12, 19, 2, 11, 20, 5, 15, 1};

	bzero((uint16 *)cmplx_pwr_dbm, sizeof(cmplx_pwr_dbm));
	ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) <= PHY_CORE_MAX);

	/* Convert sample-power to dB scale: */
	FOREACH_CORE(pi, i) {
		uint8 shift_ct, lsb, msb_loc;
		uint8 msb2345 = 0x0;
		uint32 tmp;
		tmp = cmplx_pwr[i];
		shift_ct = msb_loc = 0;
		while (tmp != 0) {
			tmp = tmp >> 1;
			shift_ct++;
			lsb = (uint8)(tmp & 1);
			if (lsb == 1)
				msb_loc = shift_ct;
		}

		/* Store first 4 bits after MSB: */
		if (msb_loc <= 4) {
			msb2345 = (cmplx_pwr[i] << (4-msb_loc)) & 0xf;
		} else {
			/* Need to first round cmplx_pwr to 5 MSBs: */
			tmp = cmplx_pwr[i] + (1U << (msb_loc-5));
			/* Check if MSB has shifted in the process: */
			if (tmp & (1U << (msb_loc+1))) {
				msb_loc++;
			}
			msb2345 = (tmp >> (msb_loc-4)) & 0xf;
		}

		/* Power in 0.25 dB steps: */
		cmplx_pwr_dbm[i] = ((3*msb_loc) << 2) + dB_LUT[msb2345];

		/* Apply a possible +0.25dB (1 step) correction depending
		 * on MSB location in cmplx_pwr[core]:
		 */
		cmplx_pwr_dbm[i] += (int16)((msb_loc >= LUT_correction[msb2345]) ? 1 : 0);
		/* crsmin_pwr = 8*log2(16*cmplx_pwr)
		   = 32 + 8*(10*log10(cmplx_pwr))/(10*log10(2))
		   = 32 + (cmplx_pwr_db * round(8/(10*log10(2)) * 2^10)) >> (10+2),
		   where additional >>2 is to account for quarter dB resolution
		*/
		crsmin_pwr[i] = 32 + ((cmplx_pwr_dbm[i] * 2721) >> 12);
	}

	if (fns->calc_fine != NULL)
		(fns->calc_fine)(fns->ctx, cmplx_pwr_dbm, extra_gain_1dB, tot_gain);
	FOREACH_CORE(pi, i) {
		pwr_ant[i] = cmplx_pwr_dbm[i];
		PHY_INFORM(("In %s: pwr_ant[%d] = %d\n", __FUNCTION__, i, pwr_ant[i]));
	}
}

/* driver up/init processing */
static int
WLBANDINITFN(phy_noise_init)(phy_init_ctx_t *ctx)
{
	phy_noise_info_t *ii = (phy_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

	    /* do not reinitialize interference mode, could be scanning */
	if (SCAN_INPROG_PHY(pi))
		return BCME_OK;

	/* initialize interference algorithms */
	if (pi->sh->interference_mode_override == TRUE) {
		/* keep the same values */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode = pi->sh->interference_mode_2G_override;
		} else {
			/* for 5G, only values 0 and 1 are valid options */
			if (pi->sh->interference_mode_5G_override == 0 ||
			    pi->sh->interference_mode_5G_override == 1) {
				pi->sh->interference_mode = pi->sh->interference_mode_5G_override;
			} else {
				/* used invalid value. so default to 0 */
				pi->sh->interference_mode = 0;
			}
		}
	} else {
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode = pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode = pi->sh->interference_mode_5G;
		}
	}

	return 	phy_noise_set_mode(ii, pi->sh->interference_mode, FALSE);
}

/* Reset */
static int
WLBANDINITFN(phy_noise_reset)(phy_init_ctx_t *ctx)
{
	phy_noise_info_t *ii = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = ii->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* Reset aci on band-change */
	if (fns->reset != NULL) {
		 return (fns->reset)(fns->ctx);
	} else {
		return BCME_OK;
	}
}

#ifndef WLC_DISABLE_ACI
void
wlc_phy_interf_rssi_update(wlc_phy_t *pih, chanspec_t chanspec, int8 leastRSSI)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_type_noise_fns_t *fns = pi->noisei->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->interf_rssi_update != NULL) {
		(fns->interf_rssi_update)(fns->ctx, chanspec, leastRSSI);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
	}
}

void
wlc_phy_aci_upd(phy_noise_info_t *ii)
{
	phy_type_noise_fns_t *fns = ii->fns;
	phy_type_noise_ctx_t *ctx = fns->ctx;

#ifdef WLSRVSDB
	uint8 offset = 0;
	uint8 vsdb_switch_failed = 0;
	phy_info_t *pi = ii->pi;

	if (pi->srvsdb_state->srvsdb_active) {
		/* Assume vsdb switch failure, if no chan switches were recorded in last 1 sec */
		vsdb_switch_failed = !(pi->srvsdb_state->num_chan_switch[0] &
			pi->srvsdb_state->num_chan_switch[1]);
		if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->srvsdb_state->sr_vsdb_channels[0]) {
			offset = 0;
		} else if (CHSPEC_CHANNEL(pi->radio_chanspec) ==
			pi->srvsdb_state->sr_vsdb_channels[1]) {
			offset = 1;
		}

		/* return if vsdb switching was active and time spent in current channel */
		/* is less than 1 sec */
		/* If vsdb switching had failed, it could be in a  deadlock */
		/* situation because of noise/aci */
		/* So continue with aci mitigation even though delta timers show less than 1 sec */

		if ((pi->srvsdb_state->sum_delta_timer[offset] < (1000 * 1000)) &&
			!vsdb_switch_failed) {

			bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));
			return;
		}
		PHY_INFORM(("Enter ACI mitigation for chan %x  since %d ms of time has expired\n",
			pi->srvsdb_state->sr_vsdb_channels[offset],
			pi->srvsdb_state->sum_delta_timer[offset]/1000));

		/* reset the timers after an effective 1 sec duration in the channel */
		bzero(pi->srvsdb_state->sum_delta_timer, 2 * sizeof(uint32));

		/* If enetering aci mitigation scheme, we need a save of */
		/* previous pi structure while doing switch */
		pi->srvsdb_state->swbkp_snapshot_valid[offset] = 0;
	}
#endif /* WLSRVSDB */

	ASSERT(fns->ma_upd != NULL);
	fns->ma_upd(ctx);

	/* Phy specific call */
	ASSERT(fns->aci_upd != NULL);
	fns->aci_upd(ctx);
}

#endif /* Compiling out ACI code */

/* Implements core functionality of WLC_SET_INTERFERENCE_OVERRIDE_MODE */
int
wlc_phy_set_interference_override_mode(phy_info_t* pi, int val)
{
	int bcmerror = 0;
	phy_type_noise_fns_t *fns = pi->noisei->fns;
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (val == INTERFERE_OVRRIDE_OFF) {
		/* this is a reset */
		pi->sh->interference_mode_override = FALSE;
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			pi->sh->interference_mode = pi->sh->interference_mode_2G;
		} else {
			pi->sh->interference_mode = pi->sh->interference_mode_5G;
		}
	} else {
		pi->sh->interference_mode_override = TRUE;
		/* push to sw state */
		if (fns->interf_mode_set != NULL) {
			(fns->interf_mode_set)(fns->ctx, val);
		} else {
			pi->sh->interference_mode_2G_override = val;
			pi->sh->interference_mode_5G_override = val;
			pi->sh->interference_mode = val;
		}
	}

	if (!pi->sh->up)
		return BCME_NOTUP;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
	/* turn interference mode to off before entering another mode */
	if (val != INTERFERE_NONE)
		phy_noise_set_mode(pi->noisei, INTERFERE_NONE, TRUE);

	if (phy_noise_set_mode(pi->noisei, pi->sh->interference_mode, TRUE) != BCME_OK)
		bcmerror = BCME_BADOPTION;
#endif // endif

	wlapi_enable_mac(pi->sh->physhim);
	return bcmerror;
}

/* ucode detected ACI, apply mitigation settings */
void
wlc_phy_hwaci_mitigate_intr(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->aci_mitigate != NULL)
		(fns->aci_mitigate)(fns->ctx);
}

#if defined(BCMDBG)
/* common dump functions for non-ac phy */
int
phy_noise_dump_common(phy_info_t *pi, struct bcmstrbuf *b)
{
	int channel, indx;
	int val;
	char *ptr;

	val = pi->sh->interference_mode;
	if (pi->aci_state & ACI_ACTIVE)
		val |= AUTO_ACTIVE;

	if (val & AUTO_ACTIVE)
		bcm_bprintf(b, "ACI is Active\n");
	else {
		bcm_bprintf(b, "ACI is Inactive\n");
		return BCME_OK;
	}

	for (channel = 0; channel < ACI_LAST_CHAN; channel++) {
		bcm_bprintf(b, "Channel %d : ", channel + 1);
		for (indx = 0; indx < 50; indx++) {
			ptr = &(pi->interf->aci.rssi_buf[channel][indx]);
			if (*ptr == (char)-1)
				break;
			bcm_bprintf(b, "%d ", *ptr);
		}
		bcm_bprintf(b, "\n");
	}

	return BCME_OK;
}

/* Dump rssi values from aci scans */
static int
phy_noise_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_info_t *pi = nxi->pi;
	uint32 i, idx, antidx;
	int32 tot;

	if (!pi->sh->up)
		return BCME_NOTUP;

	bcm_bprintf(b, "History and average of latest %d noise values:\n",
		PHY_NOISE_WINDOW_SZ);

	FOREACH_CORE(pi, antidx) {
		tot = 0;
		bcm_bprintf(b, "Ant%d: [", antidx);

		idx = pi->phy_noise_index;
		for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
			bcm_bprintf(b, "%4d", pi->phy_noise_win[antidx][idx]);
			tot += pi->phy_noise_win[antidx][idx];
			idx = MODINC_POW2(idx, PHY_NOISE_WINDOW_SZ);
		}
		bcm_bprintf(b, "]");

		tot /= PHY_NOISE_WINDOW_SZ;
		bcm_bprintf(b, " [%4d]\n", tot);
	}

	return BCME_OK;
}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
/* Dump desense values */
static int
phy_aci_dump(void *ctx, struct bcmstrbuf *b)
{
	phy_noise_info_t *nxi = (phy_noise_info_t *)ctx;
	phy_type_noise_fns_t *fns = nxi->fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	if (fns->dump != NULL)
		(fns->dump)(fns->ctx, b);

	return BCME_OK;
}
#endif // endif

void
wlc_phy_noise_save(phy_info_t *pi, int8 *noise_dbm_ant, int8 *max_noise_dbm)
{
	uint8 i;

	FOREACH_CORE(pi, i) {
		/* save noise per core */
		pi->phy_noise_win[i][pi->phy_noise_index] = noise_dbm_ant[i];

		/* save the MAX for all cores */
		if (noise_dbm_ant[i] > *max_noise_dbm)
			*max_noise_dbm = noise_dbm_ant[i];
	}
	pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index, PHY_NOISE_WINDOW_SZ);

	pi->sh->phy_noise_window[pi->sh->phy_noise_index] = *max_noise_dbm;
	pi->sh->phy_noise_index = MODINC_POW2(pi->sh->phy_noise_index, MA_WINDOW_SZ);
}

#ifdef RADIO_HEALTH_CHECK
phy_crash_reason_t
phy_noise_healthcheck_desense(phy_noise_info_t *noisei)
{
	phy_type_noise_fns_t *fns = noisei->fns;
	if (fns->health_check_desense)
		return (fns->health_check_desense)(noisei);
	else
		return PHY_RC_NONE;
}
#endif /* RADIO_HEALTH_CHECK */

int
phy_noise_sample_request_external(wlc_phy_t *pih)
{
	uint8  channel;
	phy_info_t *pi = (phy_info_t *) pih;
	phy_noise_info_t *noise_info = pi->noisei;
	phy_type_noise_fns_t *fns = noise_info->fns;
	phy_type_noise_ctx_t *ctx = (phy_type_noise_ctx_t *) fns->ctx;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* wlc_phy_noise_sample_request(pi, PHY_NOISE_SAMPLE_EXTERNAL, channel); */
	ASSERT(fns->request_noise_sample);
	fns->request_noise_sample(ctx, PHY_NOISE_SAMPLE_EXTERNAL, channel);

	return BCME_OK;
}

int
phy_noise_sample_request_crsmincal(wlc_phy_t *pih)
{
	uint8  channel;
	phy_info_t *pi = (phy_info_t *) pih;
	phy_noise_info_t *noise_info = pi->noisei;
	phy_type_noise_fns_t *fns = noise_info->fns;
	phy_type_noise_ctx_t *ctx = (phy_type_noise_ctx_t *) fns->ctx;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* wlc_phy_noise_sample_request(pi, PHY_NOISE_SAMPLE_CRSMINCAL, channel); */
	ASSERT(fns->request_noise_sample);
	fns->request_noise_sample(ctx, PHY_NOISE_SAMPLE_CRSMINCAL, channel);

	return BCME_OK;
}

int8
phy_noise_avg(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx = fns->ctx;
	int tot = 0;

	ASSERT(fns->avg);
	tot = fns->avg(ctx);

	return (int8)tot;
}

/* ucode finished phy noise measurement and raised interrupt */
int
phy_noise_sample_intr(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *) pih;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx = (phy_type_noise_ctx_t *) fns->ctx;

	ASSERT(fns->sample_intr);
	fns->sample_intr(ctx);

	return BCME_OK;
}

void
phy_noise_invoke_callbacks(phy_noise_info_t *noisei, uint8 channel, int8 noise_dbm)
{
	phy_info_t *pi = noisei->pi;
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx = fns->ctx;

	if (!pi->phynoise_state) {
		return;
	}

	PHY_NONE(("phy_noise_invoke_callbacks: state %d noise %d channel %d\n",
		pi->phynoise_state, noise_dbm, channel));

	if (pi->phynoise_state & PHY_NOISE_STATE_MON) {
		if (pi->phynoise_chan_watchdog == channel) {
			pi->sh->phy_noise_window[pi->sh->phy_noise_index] = noise_dbm;
			pi->sh->phy_noise_index = MODINC(pi->sh->phy_noise_index, MA_WINDOW_SZ);
		}
		pi->phynoise_state &= ~PHY_NOISE_STATE_MON;
	}

	if (pi->phynoise_state & PHY_NOISE_STATE_EXTERNAL) {
		pi->phynoise_state &= ~PHY_NOISE_STATE_EXTERNAL;
		wlapi_noise_cb(pi->sh->physhim, channel, noise_dbm);
	}

	if (fns->cb) {
		/* use phy specific callback if defined. */
		fns->cb(ctx);
	}
}

int8
phy_noise_lte_avg(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	phy_noise_info_t *noisei = pi->noisei;
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx = fns->ctx;

	int tot = 0;

	ASSERT(fns->lte_avg);
	tot = fns->lte_avg(ctx);

	return (int8)tot;
}

/* Returns noise level (read from srom) for current channel */
int
phy_noise_get_srom_level(phy_info_t *pi, int32 *ret_int_ptr)
{
	phy_type_noise_fns_t *fns = pi->noisei->fns;

	if (fns->get_srom_level != NULL) {
		if (!pi->sh->up) {
			return BCME_NOTUP;
		}
		return (fns->get_srom_level)(fns->ctx, ret_int_ptr);
	} else {
		PHY_INFORM(("%s: No phy specific function\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}
}

int8
phy_noise_read_shmem(phy_noise_info_t *noisei, uint8 *lte_on, uint8 *crs_high)
{
	phy_type_noise_fns_t *fns = noisei->fns;
	phy_type_noise_ctx_t *ctx = fns->ctx;
	phy_info_t *pi = noisei->pi;
	int8 noise_dbm = 0;

	BCM_REFERENCE(pi);

	if (fns->read_shm) {
		ASSERT(PHYCORENUM(pi->pubpi->phy_corenum) <= PHY_CORE_MAX);
		noise_dbm = fns->read_shm(ctx, lte_on, crs_high);
	}
	return noise_dbm;
}

/* Retrun true if noise measurement has been interrupted */
int8
phy_noise_abort_shmem_read(phy_noise_info_t *noisei)
{
	phy_info_t *pi = noisei->pi;
	uint16 map1 = 0;
	map1 = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP1(pi));

	/* If PHYCRS was seen during noise measurement, skip this measurement */
	/* Do not skip if noise measurement is done during LTE_TX */
	if (!(map1 & 0x4000) && !(pi->cal_info->ignore_crs_status)) {
		if ((map1 & 0x8000)) {
			pi->phynoise_state = 0;
			PHY_INFORM(("%s: PHY CRS seen during noise measurement\n", __FUNCTION__));
			return 1;
		}
	}

	/* If IQ_Est time out was seen */
	if ((map1 & 0x2000)) {
		PHY_INFORM(("%s: IQ Est timeout during noise measurement\n", __FUNCTION__));
		return 1;
	}

	/* Abort not requiered */
	return 0;
}

#ifndef WLC_DISABLE_ACI
#if defined(WLTEST)
/* For NPHY and HTPHY only */
int
phy_noise_aci_args(phy_info_t *pi, wl_aci_args_t *params, bool get, int len)
{
	if (len != WL_ACI_ARGS_LEGACY_LENGTH && len != sizeof(wl_aci_args_t))
		return BCME_BUFTOOSHORT;

	if (get) {
		params->enter_aci_thresh = pi->interf->aci.enter_thresh;
		params->exit_aci_thresh = pi->interf->aci.exit_thresh;
		params->usec_spin = pi->interf->aci.usec_spintime;
		params->glitch_delay = pi->interf->aci.glitch_delay;
	} else {
		if (params->enter_aci_thresh > 0)
			pi->interf->aci.enter_thresh = params->enter_aci_thresh;
		if (params->exit_aci_thresh > 0)
			pi->interf->aci.exit_thresh = params->exit_aci_thresh;
		if (params->usec_spin > 0)
			pi->interf->aci.usec_spintime = params->usec_spin;
		if (params->glitch_delay > 0)
			pi->interf->aci.glitch_delay = params->glitch_delay;
	}

	if (len == sizeof(wl_aci_args_t)) {
		if (pi->interf->aci.nphy == NULL)
			return BCME_UNSUPPORTED;

		if (get) {
			params->nphy_adcpwr_enter_thresh =
				pi->interf->aci.nphy->adcpwr_enter_thresh;
			params->nphy_adcpwr_exit_thresh = pi->interf->aci.nphy->adcpwr_exit_thresh;
			params->nphy_repeat_ctr = pi->interf->aci.nphy->detect_repeat_ctr;
			params->nphy_num_samples = pi->interf->aci.nphy->detect_num_samples;
			params->nphy_undetect_window_sz = pi->interf->aci.nphy->undetect_window_sz;
			params->nphy_b_energy_lo_aci = pi->interf->aci.nphy->b_energy_lo_aci;
			params->nphy_b_energy_md_aci = pi->interf->aci.nphy->b_energy_md_aci;
			params->nphy_b_energy_hi_aci = pi->interf->aci.nphy->b_energy_hi_aci;

			params->nphy_noise_noassoc_glitch_th_up =
				pi->interf->noise.nphy_noise_noassoc_glitch_th_up;
			params->nphy_noise_noassoc_glitch_th_dn =
				pi->interf->noise.nphy_noise_noassoc_glitch_th_dn;
			params->nphy_noise_assoc_glitch_th_up =
				pi->interf->noise.nphy_noise_assoc_glitch_th_up;
			params->nphy_noise_assoc_glitch_th_dn =
				pi->interf->noise.nphy_noise_assoc_glitch_th_dn;
			params->nphy_noise_assoc_aci_glitch_th_up =
				pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up;
			params->nphy_noise_assoc_aci_glitch_th_dn =
				pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn;
			params->nphy_noise_assoc_enter_th =
				pi->interf->noise.nphy_noise_assoc_enter_th;
			params->nphy_noise_noassoc_enter_th =
				pi->interf->noise.nphy_noise_noassoc_enter_th;
			params->nphy_noise_assoc_rx_glitch_badplcp_enter_th=
				pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			params->nphy_noise_noassoc_crsidx_incr=
				pi->interf->noise.nphy_noise_noassoc_crsidx_incr;
			params->nphy_noise_assoc_crsidx_incr=
				pi->interf->noise.nphy_noise_assoc_crsidx_incr;
			params->nphy_noise_crsidx_decr=
				pi->interf->noise.nphy_noise_crsidx_decr;

		} else {
			pi->interf->aci.nphy->adcpwr_enter_thresh =
				params->nphy_adcpwr_enter_thresh;
			pi->interf->aci.nphy->adcpwr_exit_thresh = params->nphy_adcpwr_exit_thresh;
			pi->interf->aci.nphy->detect_repeat_ctr = params->nphy_repeat_ctr;
			pi->interf->aci.nphy->detect_num_samples = params->nphy_num_samples;
			pi->interf->aci.nphy->undetect_window_sz =
				MIN(params->nphy_undetect_window_sz,
				ACI_MAX_UNDETECT_WINDOW_SZ);
			pi->interf->aci.nphy->b_energy_lo_aci = params->nphy_b_energy_lo_aci;
			pi->interf->aci.nphy->b_energy_md_aci = params->nphy_b_energy_md_aci;
			pi->interf->aci.nphy->b_energy_hi_aci = params->nphy_b_energy_hi_aci;

			pi->interf->noise.nphy_noise_noassoc_glitch_th_up =
				params->nphy_noise_noassoc_glitch_th_up;
			pi->interf->noise.nphy_noise_noassoc_glitch_th_dn =
				params->nphy_noise_noassoc_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_glitch_th_up =
				params->nphy_noise_assoc_glitch_th_up;
			pi->interf->noise.nphy_noise_assoc_glitch_th_dn =
				params->nphy_noise_assoc_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_aci_glitch_th_up =
				params->nphy_noise_assoc_aci_glitch_th_up;
			pi->interf->noise.nphy_noise_assoc_aci_glitch_th_dn =
				params->nphy_noise_assoc_aci_glitch_th_dn;
			pi->interf->noise.nphy_noise_assoc_enter_th =
				params->nphy_noise_assoc_enter_th;
			pi->interf->noise.nphy_noise_noassoc_enter_th =
				params->nphy_noise_noassoc_enter_th;
			pi->interf->noise.nphy_noise_assoc_rx_glitch_badplcp_enter_th =
				params->nphy_noise_assoc_rx_glitch_badplcp_enter_th;
			pi->interf->noise.nphy_noise_noassoc_crsidx_incr =
				params->nphy_noise_noassoc_crsidx_incr;
			pi->interf->noise.nphy_noise_assoc_crsidx_incr =
				params->nphy_noise_assoc_crsidx_incr;
			pi->interf->noise.nphy_noise_crsidx_decr =
				params->nphy_noise_crsidx_decr;
		}
	}

	return BCME_OK;
}
#endif // endif
#endif /* Compiling out ACI code */

#ifndef WLC_DISABLE_ACI
/* %%%%%% interference */
/* update aci rx carrier sense glitch moving average */

int
phy_noise_interf_chan_stats_upd(wlc_phy_t *ppi, chanspec_t chanspec, uint32 crsglitch,
        uint32 bphy_crsglitch, uint32 badplcp, uint32 bphy_badplcp, uint8 txop, uint32 mbsstime)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	if (ISACPHY(pi))
		wlc_phy_desense_aci_upd_txop_acphy(pi, chanspec, txop);

	/* Doing interference update of chan stats here  */

	if (pi->interf->curr_home_channel == (CHSPEC_CHANNEL(chanspec))) {
		pi->interf->cca_stats_func_called = TRUE;
		pi->interf->cca_stats_total_glitch = crsglitch;
		pi->interf->cca_stats_bphy_glitch = bphy_crsglitch;
		pi->interf->cca_stats_total_badplcp = badplcp;
		pi->interf->cca_stats_bphy_badplcp = bphy_badplcp;
		pi->interf->cca_stats_mbsstime = mbsstime;
	}

	return BCME_OK;
}

#endif /* Compiling out ACI code */

int
phy_noise_sched_set(wlc_phy_t *pih, phy_bgnoise_schd_mode_t reason, bool upd)
{
	phy_info_t *pi = (phy_info_t*)pih;

	BCM_REFERENCE(reason);
	PHY_NONE(("%s: reason %d, upd %d\n", __FUNCTION__, reason, upd));

	pi->phynoise_disable = upd;

	return BCME_OK;
}

bool
phy_noise_sched_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t*)pih;

	return (pi->phynoise_disable);
}

int
phy_noise_pmstate_set(wlc_phy_t *pih, bool pmstate)
{
	phy_info_t *pi = (phy_info_t*)pih;
	pi->phynoise_pmstate = pmstate;

	return BCME_OK;
}

bool
phy_noise_pmstate_get(phy_info_t *pi)
{
	return pi->phynoise_pmstate;
}

int8
phy_noise_avg_per_antenna(wlc_phy_t *pih, int coreidx)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint8 i, idx;
	int32 tot = 0;
	int8 result = 0;
	phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

	BCM_REFERENCE(stf_shdata);

	if (!pi->sh->up)
		return 0;

	/* checking coreidx to prevent overrunning
	 * phy_noise_win array
	 */
	if (((uint)coreidx) >= PHY_CORE_MAX)
		return 0;

	IF_ACTV_CORE(pi, stf_shdata->phyrxchain, coreidx) {
		tot = 0;
		idx = pi->phy_noise_index;
		for (i = 0; i < PHY_NOISE_WINDOW_SZ; i++) {
			tot += pi->phy_noise_win[coreidx][idx];
			idx = MODINC_POW2(idx, PHY_NOISE_WINDOW_SZ);
		}

		result = (int8)(tot/PHY_NOISE_WINDOW_SZ);
	}

	return result;
}
