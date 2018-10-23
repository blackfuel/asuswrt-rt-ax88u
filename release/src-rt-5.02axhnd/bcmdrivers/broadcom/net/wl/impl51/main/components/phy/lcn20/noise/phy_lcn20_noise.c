/*
 * lcn20PHY Noise module implementation
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
 * $Id: phy_lcn20_noise.c 721745 2017-09-15 18:03:27Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include "phy_type_noise.h"
#include <phy_lcn20.h>
#include <phy_lcn20_noise.h>
#include <phy_noise_api.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_lcn20_noise_info {
	phy_info_t *pi;
	phy_lcn20_info_t *lcn20i;
	phy_noise_info_t *ii;
};

/* local functions */
static void phy_lcn20_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
	uint8 extra_gain_1dB, int16 *tot_gain);
static void phy_lcn20_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init);
static bool phy_lcn20_noise_aci_wd(phy_wd_ctx_t *ctx);
#if defined(BCMDBG)
static int phy_lcn20_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_lcn20_noise_dump NULL
#endif // endif

#ifndef WLC_DISABLE_ACI
static void phy_lcn20_noise_upd_aci(phy_type_noise_ctx_t *ctx);
static void phy_lcn20_noise_update_aci_ma(phy_type_noise_ctx_t *ctx);
#endif // endif

static void phy_lcn20_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch);
static int8 phy_lcn20_noise_avg(phy_type_noise_ctx_t *ctx);
static void phy_lcn20_noise_sample_intr(phy_type_noise_ctx_t *ctx);
static int8 phy_lcn20_noise_lte_avg(phy_type_noise_ctx_t *ctx);
static int8 phy_lcn20_noise_read_shmem(phy_type_noise_ctx_t *ctx,
	uint8 *lte_on, uint8 *crs_high);

/* register phy type specific implementation */
phy_lcn20_noise_info_t *
BCMATTACHFN(phy_lcn20_noise_register_impl)(phy_info_t *pi, phy_lcn20_info_t *lcn20i,
	phy_noise_info_t *ii)
{
	phy_lcn20_noise_info_t *info;
	phy_type_noise_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_lcn20_noise_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_lcn20_noise_info_t));
	info->pi = pi;
	info->lcn20i = lcn20i;
	info->ii = ii;

	/* Init value for interference mode */
	if ((CHIPID(pi->sh->chip) == BCM43018_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43430_CHIP_ID)) {
		pi->sh->interference_mode = pi->sh->interference_mode_2G = WLAN_AUTO;
	}

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_lcn20_noise_aci_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_ACI,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.calc_fine = phy_lcn20_noise_calc_fine_resln;
	fns.mode = phy_lcn20_noise_set_mode;
	fns.dump = phy_lcn20_noise_dump;
	fns.avg = phy_lcn20_noise_avg;
	fns.sample_intr = phy_lcn20_noise_sample_intr;
	fns.lte_avg = phy_lcn20_noise_lte_avg;
	fns.read_shm = phy_lcn20_noise_read_shmem;
	fns.ctx = info;

#ifndef WLC_DISABLE_ACI
	fns.aci_upd = phy_lcn20_noise_upd_aci;
	fns.ma_upd = phy_lcn20_noise_update_aci_ma;
#endif // endif

	fns.request_noise_sample = phy_lcn20_noise_request_sample;

	phy_noise_register_impl(ii, &fns);

	return info;

	/* error handling */
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_lcn20_noise_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_lcn20_noise_unregister_impl)(phy_lcn20_noise_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_noise_info_t *ii = info->ii;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_noise_unregister_impl(ii);

	phy_mfree(pi, info, sizeof(phy_lcn20_noise_info_t));
}

static void
phy_lcn20_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
	uint8 extra_gain_1dB, int16 *tot_gain)
{
	phy_lcn20_noise_info_t *info = (phy_lcn20_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 i;
	BCM_REFERENCE(pi);

	FOREACH_CORE(pi, i) {
		/* Convert to analog input power at ADC and then
		 * backoff applied gain to get antenna-input referred power.
		 * NOTE: all the calculations below assume 0.25dB format.
		 */
		cmplx_pwr_dbm[i] += LCN20PHY_NOISE_PWR_TO_DBM;
		cmplx_pwr_dbm[i] -= tot_gain[i];
	}
}

/* set mode */
static void
phy_lcn20_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init)
{
	phy_lcn20_noise_info_t *info = (phy_lcn20_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;
		if (wlc_lcn20phy_acimode_valid(pi, wanted_mode)) {
			wlc_lcn20phy_aci_init(pi);
		}
	}
	if (wlc_lcn20phy_acimode_valid(pi, wanted_mode)) {
		wlc_lcn20phy_aci_modes(pi, wanted_mode);
	}
}

/* watchdog callback */
static bool
phy_lcn20_noise_aci_wd(phy_wd_ctx_t *ctx)
{
	BCM_REFERENCE(ctx);
	return TRUE;
}

#if defined(BCMDBG)
static int
phy_lcn20_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_lcn20_noise_info_t *info = (phy_lcn20_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_noise_dump_common(pi, b);
}
#endif // endif

#ifndef WLC_DISABLE_ACI
static void
phy_lcn20_noise_upd_aci(phy_type_noise_ctx_t * ctx)
{
}

static void
phy_lcn20_noise_update_aci_ma(phy_type_noise_ctx_t *ctx)
{
	phy_lcn20_noise_info_t *noise_info = (phy_lcn20_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;

	int32 delta = 0;

	if ((pi->interf->cca_stats_func_called == FALSE) || pi->interf->cca_stats_mbsstime <= 0) {
		uint16 cur_glitch_cnt;

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
		cur_glitch_cnt = wlapi_bmac_read_shm(pi->sh->physhim,
			MACSTAT_ADDR(pi, MCSTOFF_RXCRSGLITCH));
		delta = cur_glitch_cnt - pi->interf->aci.pre_glitch_cnt;
		pi->interf->aci.pre_glitch_cnt = cur_glitch_cnt;

#ifdef WLSRVSDB
		if (vsdb_split_cntr) {
			delta =  pi->srvsdb_state->sum_delta_crsglitch[bank_offset];
		}
#endif /* WLSRVSDB */
	} else {
		pi->interf->cca_stats_func_called = FALSE;
		/* Normalizing the statistics per second */
		delta = pi->interf->cca_stats_total_glitch * 1000 /
			pi->interf->cca_stats_mbsstime;
	}

	if (delta >= 0) {
		/* evict old value */
		pi->interf->aci.ma_total -= pi->interf->aci.ma_list[pi->interf->aci.ma_index];

		/* admit new value */
		pi->interf->aci.ma_total += (uint16) delta;
		pi->interf->aci.glitch_ma_previous = pi->interf->aci.glitch_ma;
		pi->interf->aci.glitch_ma = pi->interf->aci.ma_total / MA_WINDOW_SZ;
		pi->interf->aci.ma_list[pi->interf->aci.ma_index++] = (uint16) delta;
		if (pi->interf->aci.ma_index >= MA_WINDOW_SZ) {
			pi->interf->aci.ma_index = 0;
		}
	}

	PHY_ACI(("phy_lcn20_noise_update_aci_ma: ave glitch %d, ACI is %s, delta is %d\n",
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
#endif /* WLC_DISABLE_ACI */

#define PHY_NOISE_MAX_IDLETIME		30

static void
phy_lcn20_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch)
{
	phy_lcn20_noise_info_t *noise_info = (phy_lcn20_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	bool wait_for_intr = TRUE;

	PHY_NONE(("phy_lcn20_noise_request_sample: state %d reason %d, channel %d\n",
		pi->phynoise_state, reason, ch));

	/* since polling is atomic, sampling_in_progress equals to interrupt sampling ongoing
	 *  In these collision cases, always yield and wait interrupt to finish, where the results
	 *  maybe be sharable if channel matches in common callback progressing.
	 */
	if (sampling_in_progress)
		return;

	switch (reason) {
	case PHY_NOISE_SAMPLE_MON:

		pi->phynoise_chan_watchdog = ch;
		pi->phynoise_state |= PHY_NOISE_STATE_MON;

		break;

	case PHY_NOISE_SAMPLE_EXTERNAL:

		pi->phynoise_state |= PHY_NOISE_STATE_EXTERNAL;
		break;

	case PHY_NOISE_SAMPLE_CRSMINCAL:

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

	/* LCN20 don't have noise cal yet */
	wait_for_intr = FALSE;

done:
	/* if no interrupt scheduled, populate noise results now */
	if (!wait_for_intr) {
		phy_noise_invoke_callbacks(noisei, ch, noise_dbm);
	}
	return;
}

static int8
phy_lcn20_noise_avg(phy_type_noise_ctx_t *ctx)
{
	phy_lcn20_noise_info_t *noise_info = (phy_lcn20_noise_info_t *) ctx;
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

/* Not implemented for lcn20 phy. Not used in lcn20. */
void
phy_lcn20_noise_sample_intr(phy_type_noise_ctx_t *ctx)
{
	phy_lcn20_noise_info_t *noise_info = (phy_lcn20_noise_info_t *) ctx;
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

	/* rssi dbm computed, invoke all callbacks */
	phy_noise_invoke_callbacks(noisei, channel, noise_dbm);
}

static int8
phy_lcn20_noise_lte_avg(phy_type_noise_ctx_t *ctx)
{
	return 0;
}

static int8
phy_lcn20_noise_read_shmem(phy_type_noise_ctx_t *ctx, uint8 *lte_on, uint8 *crs_high)
{
	phy_lcn20_noise_info_t *noise_info = (phy_lcn20_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int8 noise_dbm[PHY_CORE_MAX];
	int8 noise_dBm_ucode = 0;
	uint16 lcn20phyregs_shm_addr;
	uint32 noise_iq_ucode = 0xFFFF;

	*lte_on = 0;
	*crs_high = 0;
	noise_dbm[0] = PHY_NOISE_INVALID;

	if (pi_lcn20->noise_iqest_en) {
		lcn20phyregs_shm_addr =
			2 * wlapi_bmac_read_shm(pi->sh->physhim, M_LCN40PHYREGS_PTR(pi));

		noise_iq_ucode = wlapi_bmac_read_shm(pi->sh->physhim,
			lcn20phyregs_shm_addr + M_NOISE_IQPWR(pi));
		*crs_high = (uint8)wlapi_bmac_read_shm(pi->sh->physhim,
			lcn20phyregs_shm_addr + M_NOISE_LTE_ON(pi));

		if (noise_iq_ucode) {
			wlc_phy_noise_calc(pi, &noise_iq_ucode, &noise_dBm_ucode, 0);
			noise_dbm[0] = noise_dBm_ucode + pi_lcn20->noise_iqest_gain_adj_2g;
		}
		PHY_INFORM(("noise_iq=%d, noise_dBm_ucode=%d, noise_dBm=%d, crs=%d\n",
			noise_iq_ucode, noise_dBm_ucode, noise_dbm[0], *crs_high));
	}

	wlc_phy_noise_save(pi, noise_dbm, noise_dbm);
	return noise_dbm[0];
}
