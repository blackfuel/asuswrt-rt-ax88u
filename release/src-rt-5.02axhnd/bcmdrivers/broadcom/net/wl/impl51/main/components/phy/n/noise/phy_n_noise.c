/*
 * NPHY NOISE module implementation
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
 * $Id: phy_n_noise.c 721745 2017-09-15 18:03:27Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_wd.h>
#include "phy_type_noise.h"
#include <phy_n.h>
#include <phy_n_noise.h>
#include <phy_n_info.h>
#include <wlc_phyreg_n.h>
#include <phy_ocl_api.h>
#include <phy_noise.h>
#include <phy_noise_api.h>

/* module private states */
struct phy_n_noise_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_noise_info_t *ii;
};

/* local functions */
static int phy_n_noise_attach_modes(phy_info_t *pi);
static void phy_n_noise_calc(phy_type_noise_ctx_t *ctx, int8 cmplx_pwr_dbm[], uint8 extra_gain_1dB);
static void phy_n_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[],
	uint8 extra_gain_1dB, int16 *tot_gain);
static void phy_n_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init);
static int phy_n_noise_reset(phy_type_noise_ctx_t *ctx);
static bool phy_n_noise_noise_wd(phy_wd_ctx_t *ctx);
static bool phy_n_noise_aci_wd(phy_wd_ctx_t *ctx);
#ifndef WLC_DISABLE_ACI
static void phy_n_noise_interf_rssi_update(phy_type_noise_ctx_t *ctx,
	chanspec_t chanspec, int8 leastRSSI);
static void phy_n_noise_upd_aci(phy_type_noise_ctx_t * ctx);
static void phy_n_noise_update_aci_ma(phy_type_noise_ctx_t *ctx);
static void wlc_phy_noisemode_upd(phy_info_t *pi);
static int8 wlc_phy_cmn_noisemode_glitch_chk_adj(phy_info_t *pi, uint16 total_glitch_badplcp,
	noise_thresholds_t *thresholds);
static void wlc_phy_cmn_noise_limit_desense(phy_info_t *pi);
#endif // endif
static void phy_n_noise_interf_mode_set(phy_type_noise_ctx_t *ctx, int val);
#if defined(BCMDBG)
static int phy_n_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b);
#else
#define phy_n_noise_dump NULL
#endif // endif
static void phy_n_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch);
static void phy_n_noise_sample_intr(phy_type_noise_ctx_t *ctx);

static int8 phy_n_noise_avg(phy_type_noise_ctx_t *ctx);
static int8 phy_n_noise_lte_avg(phy_type_noise_ctx_t *ctx);
static int phy_n_noise_get_srom_level(phy_type_noise_ctx_t *ctx, int32 *ret_int_ptr);
static int8 phy_n_noise_read_shmem(phy_type_noise_ctx_t *ctx,
	uint8 *lte_on, uint8 *crs_high);

/* register phy type specific implementation */
phy_n_noise_info_t *
BCMATTACHFN(phy_n_noise_register_impl)(phy_info_t *pi, phy_n_info_t *ni, phy_noise_info_t *ii)
{
	phy_n_noise_info_t *info;
	phy_type_noise_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((info = phy_malloc(pi, sizeof(phy_n_noise_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_noise_info_t));
	info->pi = pi;
	info->ni = ni;
	info->ii = ii;

	if (phy_n_noise_attach_modes(pi) != BCME_OK) {
		goto fail;
	}

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, phy_n_noise_noise_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_INTF_NOISE,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}
	if (phy_wd_add_fn(pi->wdi, phy_n_noise_aci_wd, info,
	                  PHY_WD_PRD_1TICK, PHY_WD_1TICK_NOISE_ACI,
	                  PHY_WD_FLAG_NONE) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		goto fail;
	}

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.mode = phy_n_noise_set_mode;
	fns.reset = phy_n_noise_reset;
	fns.calc = phy_n_noise_calc;
	fns.calc_fine = phy_n_noise_calc_fine_resln;
#ifndef WLC_DISABLE_ACI
	fns.interf_rssi_update = phy_n_noise_interf_rssi_update;
	fns.aci_upd = phy_n_noise_upd_aci;
	fns.ma_upd = phy_n_noise_update_aci_ma;
#endif // endif
	fns.interf_mode_set = phy_n_noise_interf_mode_set;
	fns.dump = phy_n_noise_dump;
	fns.request_noise_sample = phy_n_noise_request_sample;
	fns.avg = phy_n_noise_avg;
	fns.sample_intr = phy_n_noise_sample_intr;
	fns.lte_avg = phy_n_noise_lte_avg;
	fns.get_srom_level = phy_n_noise_get_srom_level;
	fns.read_shm = phy_n_noise_read_shmem;
	fns.ctx = info;

	phy_noise_register_impl(ii, &fns);

	return info;

	/* error handling */
fail:
	phy_n_noise_unregister_impl(info);

	return NULL;
}

static int
BCMATTACHFN(phy_n_noise_attach_modes)(phy_info_t *pi)
{
	pi->interf->aci.nphy = (nphy_aci_interference_info_t *)
		phy_malloc(pi, sizeof(*(pi->interf->aci.nphy)));

	if (pi->interf->aci.nphy == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));

		return BCME_NOMEM;
	}

	if (pi->pubpi->phy_rev == LCNXN_BASEREV + 1) {
		pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
		pi->sh->interference_mode_5G = NON_WLAN;
	} else if ((CHIPID(pi->sh->chip) == BCM43236_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43235_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43234_CHIP_ID) ||
		(CHIPID(pi->sh->chip) == BCM43238_CHIP_ID)) {
		/* assign 2G default interference mode for 4323x chips */
		pi->sh->interference_mode_2G = WLAN_AUTO_W_NOISE;
		pi->sh->interference_mode_5G = NON_WLAN;
	} else {
		pi->sh->interference_mode_2G = WLAN_AUTO;
		pi->sh->interference_mode_5G = NON_WLAN;
	}

	return BCME_OK;
}

void
BCMATTACHFN(phy_n_noise_unregister_impl)(phy_n_noise_info_t *info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	if (info != NULL) {
		phy_noise_info_t *ii = info->ii;
		phy_info_t *pi = info->pi;

		/* unregister from common */
		phy_noise_unregister_impl(ii);

		if (pi->interf->aci.nphy != NULL)
			phy_mfree(pi, pi->interf->aci.nphy, sizeof(*(pi->interf->aci.nphy)));

		phy_mfree(pi, info, sizeof(phy_n_noise_info_t));
	}
}

static void
phy_n_noise_calc(phy_type_noise_ctx_t *ctx, int8 cmplx_pwr_dbm[], uint8 extra_gain_1dB)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8 i;
	uint16 gain = 0;

	gain = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_BLKS(pi) + 0xC);

	FOREACH_CORE(pi, i) {
		if (NREV_GE(pi->pubpi->phy_rev, 3)) {
			cmplx_pwr_dbm[i] += (int8) (PHY_NOISE_OFFSETFACT_4322 - gain);
		} else if (NREV_LT(pi->pubpi->phy_rev, 3)) {
			/* assume init gain 70 dB, 128 maps to 1V so
			 * 10*log10(128^2*2/128/128/50)+30=16 dBm
			 * WARNING: if the nphy init gain is ever changed,
			 * this formula needs to be updated
			*/
			cmplx_pwr_dbm[i] += (int8)(16 - (15) * 3 - (70 + extra_gain_1dB));
		}
	}
}

static void
phy_n_noise_calc_fine_resln(phy_type_noise_ctx_t *ctx, int16 cmplx_pwr_dbm[], uint8 extra_gain_1dB,
	int16 *tot_gain)
{
	uint8 i;
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	UNUSED_PARAMETER(tot_gain);
	BCM_REFERENCE(pi);

	FOREACH_CORE(pi, i) {
		/* assume init gain 70 dB, 128 maps to 1V so
		 * 10*log10(128^2*2/128/128/50)+30=16 dBm
		 *	WARNING: if the nphy init gain is ever changed,
		 * this formula needs to be updated
		 */
		cmplx_pwr_dbm[i] += ((int16)(16 << 2) - (int16)((15 << 2)*3)
				- (int16)((70 + extra_gain_1dB) << 2));
	}
}

/* set mode */
static void
phy_n_noise_set_mode(phy_type_noise_ctx_t *ctx, int wanted_mode, bool init)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s: mode %d init %d\n", __FUNCTION__, wanted_mode, init));

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

	if (init) {
		pi->interference_mode_crs_time = 0;
		pi->crsglitch_prev = 0;
		/* clear out all the state */
		wlc_phy_noisemode_reset_nphy(pi);
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			wlc_phy_acimode_reset_nphy(pi);
		}
	}

	/* NPHY 5G, supported for NON_WLAN and INTERFERE_NONE only */
	if (CHSPEC_IS2G(pi->radio_chanspec) ||
	    (CHSPEC_IS5G(pi->radio_chanspec) &&
	     (wanted_mode == NON_WLAN || wanted_mode == INTERFERE_NONE))) {
		if (wanted_mode == INTERFERE_NONE) {	/* disable */
			switch (pi->cur_interference_mode) {
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
			case WLAN_MANUAL:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, FALSE,
						PHY_ACI_PWR_NOTPRESENT);
				}
				pi->aci_state &= ~ACI_ACTIVE;
				break;
			case NON_WLAN:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi,
						FALSE,
						PHY_ACI_PWR_NOTPRESENT);
					pi->aci_state &= ~ACI_ACTIVE;
				}
				break;
			}
		} else {	/* Enable */
			switch (wanted_mode) {
			case NON_WLAN:
			case WLAN_AUTO:
			case WLAN_AUTO_W_NOISE:
				/* fall through */
				break;
			case WLAN_MANUAL:
				if (CHSPEC_IS2G(pi->radio_chanspec)) {
					wlc_phy_acimode_set_nphy(pi, TRUE, PHY_ACI_PWR_HIGH);
				}
				break;
			}
		}
	}
}

static int
phy_n_noise_reset(phy_type_noise_ctx_t *ctx)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	PHY_TRACE(("%s\n", __FUNCTION__));

#ifndef WLC_DISABLE_ACI
	/* update interference mode before INIT process start */
	if (NREV_GE(pi->pubpi->phy_rev, 3) && !(SCAN_INPROG_PHY(pi))) {
		pi->sh->interference_mode = CHSPEC_IS2G(pi->radio_chanspec) ?
			pi->sh->interference_mode_2G : pi->sh->interference_mode_5G;
	}
#endif // endif

	ASSERT(pi->interf->aci.nphy != NULL);

	/* Reset ACI internals if not scanning and not in aci_detection */
	if (!(SCAN_INPROG_PHY(pi) ||
	      pi->interf->aci.nphy->detection_in_progress)) {
		wlc_phy_aci_sw_reset_nphy(pi);
	}

	return BCME_OK;
}

/* watchdog callback */
static bool
phy_n_noise_noise_wd(phy_wd_ctx_t *ctx)
{
	phy_n_noise_info_t *ii = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;

	if (pi->tunings[0]) {
		pi->interf->noise.nphy_noise_assoc_enter_th = pi->tunings[0];
		pi->interf->noise.nphy_noise_noassoc_enter_th = pi->tunings[0];
	}

	if (pi->tunings[2]) {
		pi->interf->noise.nphy_noise_assoc_glitch_th_dn = pi->tunings[2];
		pi->interf->noise.nphy_noise_noassoc_glitch_th_dn = pi->tunings[2];
	}

	if (pi->tunings[1]) {
		pi->interf->noise.nphy_noise_noassoc_glitch_th_up = pi->tunings[1];
		pi->interf->noise.nphy_noise_assoc_glitch_th_up = pi->tunings[1];
	}

	return TRUE;
}

static bool
phy_n_noise_aci_wd(phy_wd_ctx_t *ctx)
{
	phy_n_noise_info_t *ii = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = ii->pi;
	phy_noise_info_t *noisei = ii->ii;

	/* defer interference checking, scan and update if RM is progress */
	if (!SCAN_RM_IN_PROGRESS(pi)) {
		/* interf.scanroamtimer counts transient time coming out of scan */
		if (pi->interf->scanroamtimer != 0)
			pi->interf->scanroamtimer -= 1;

		wlc_phy_aci_upd(noisei);

	} else {
		/* in a scan/radio meas, don't update moving average when we
		 * first come out of scan or roam
		 */
		pi->interf->scanroamtimer = 2;
	}

	return TRUE;
}

#ifndef WLC_DISABLE_ACI
static void
phy_n_noise_interf_rssi_update(phy_type_noise_ctx_t *ctx, chanspec_t chanspec, int8 leastRSSI)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	if ((CHSPEC_CHANNEL(chanspec) == pi->interf->curr_home_channel)) {
		pi->u.pi_nphy->intf_rssi_avg = leastRSSI;
		pi->interf->rssi = leastRSSI;
	}
}

static void
phy_n_noise_upd_aci(phy_type_noise_ctx_t * ctx)
{
	phy_n_noise_info_t *noise_info = (phy_n_noise_info_t *) ctx;
	phy_info_t *pi = (phy_info_t *) noise_info->pi;

	bool desense_gt_4;
	int glit_plcp_sum;

	switch (pi->sh->interference_mode) {
		case NON_WLAN:
			/* NON_WLAN NPHY */
			if (NREV_GE(pi->pubpi->phy_rev, 16)) {
				/* run noise mitigation only */
				wlc_phy_noisemode_upd_nphy(pi);
			} else if (NREV_LE(pi->pubpi->phy_rev, 15)) {
				wlc_phy_noisemode_upd(pi);
			}
			break;
		case WLAN_AUTO:
			if (ASSOC_INPROG_PHY(pi))
				break;

			/* 5G band not supported yet */
			if (CHSPEC_IS5G(pi->radio_chanspec))
				break;

			if (PUB_NOT_ASSOC(pi)) {
				/* not associated:  do not run aci routines */
				break;
			}
			PHY_ACI(("Interf Mode 3, pi->interf->aci.glitch_ma = %d\n",
				pi->interf->aci.glitch_ma));

			/* Attempt to enter ACI mode if not already active */
			/* only run this code if associated */
			if (!(pi->aci_state & ACI_ACTIVE)) {
				if ((pi->sh->now  % NPHY_ACI_CHECK_PERIOD) == 0) {

					if ((pi->interf->aci.glitch_ma +
						pi->interf->badplcp_ma) >=
						pi->interf->aci.enter_thresh) {
						wlc_phy_acimode_upd_nphy(pi);
					}
				}
			} else {
				if (((pi->sh->now - pi->aci_start_time) %
					pi->aci_exit_check_period) == 0) {
					wlc_phy_acimode_upd_nphy(pi);
				}
			}
			break;
		case WLAN_AUTO_W_NOISE:
			/* 5G band not supported yet */
			if (CHSPEC_IS5G(pi->radio_chanspec))
				break;

			/* only do this for 4322 and future revs */
			if (NREV_GE(pi->pubpi->phy_rev, 16)) {
				/* Attempt to enter ACI mode if not already active */
				wlc_phy_aci_noise_upd_nphy(pi);
			} else if (NREV_LE(pi->pubpi->phy_rev, 15)) {
				PHY_ACI(("Interf Mode 4\n"));
				desense_gt_4 = (pi->interf->noise.ofdm_desense >= 4 ||
					pi->interf->noise.bphy_desense >= 4);
				glit_plcp_sum = pi->interf->aci.glitch_ma + pi->interf->badplcp_ma;
				if (!(pi->aci_state & ACI_ACTIVE)) {
					if ((pi->sh->now  % NPHY_ACI_CHECK_PERIOD) == 0) {
						if ((glit_plcp_sum >=
						    pi->interf->aci.enter_thresh) ||
						    desense_gt_4) {
							wlc_phy_acimode_upd_nphy(pi);
						}
					}
				} else {
					if (((pi->sh->now - pi->aci_start_time) %
					    pi->aci_exit_check_period) == 0) {
						wlc_phy_acimode_upd_nphy(pi);
					}
				}

				wlc_phy_noisemode_upd(pi);
			}
			break;

		default:
			break;
	}
}

static void
phy_n_noise_update_aci_ma(phy_type_noise_ctx_t *ctx)
{
	phy_n_noise_info_t *noise_info = (phy_n_noise_info_t *) ctx;
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

		/* if we aren't suppose to update yet, don't */
		if (pi->interf->scanroamtimer != 0) {
			return;
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
		if (NREV_LE(pi->pubpi->phy_rev, 15)) {
			pi->interf->aci.glitch_ma = pi->interf->aci.ma_total /
				PHY_NOISE_MA_WINDOW_SZ;
		} else {
			pi->interf->aci.glitch_ma = pi->interf->aci.ma_total / MA_WINDOW_SZ;
		}
		pi->interf->aci.ma_list[pi->interf->aci.ma_index++] = (uint16) delta;
		if (NREV_LE(pi->pubpi->phy_rev, 15)) {
			if (pi->interf->aci.ma_index >= PHY_NOISE_MA_WINDOW_SZ)
				pi->interf->aci.ma_index = 0;
		} else {
			if (pi->interf->aci.ma_index >= MA_WINDOW_SZ)
				pi->interf->aci.ma_index = 0;
		}
	}

	if (badplcp_delta >= 0) {
		pi->interf->badplcp_ma_total -=
			pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index];
		pi->interf->badplcp_ma_total += (uint16) badplcp_delta;
		pi->interf->badplcp_ma_previous = pi->interf->badplcp_ma;

		if (NREV_LE(pi->pubpi->phy_rev, 15)) {
			pi->interf->badplcp_ma =
				pi->interf->badplcp_ma_total / PHY_NOISE_MA_WINDOW_SZ;
		} else {
			pi->interf->badplcp_ma =
				pi->interf->badplcp_ma_total / MA_WINDOW_SZ;
		}

		pi->interf->badplcp_ma_list[pi->interf->badplcp_ma_index++] =
			(uint16) badplcp_delta;
		if (NREV_LE(pi->pubpi->phy_rev, 15)) {
			if (pi->interf->badplcp_ma_index >= PHY_NOISE_MA_WINDOW_SZ) {
				pi->interf->badplcp_ma_index = 0;
			}
		} else {
			if (pi->interf->badplcp_ma_index >= MA_WINDOW_SZ)
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

	if ((NREV_GE(pi->pubpi->phy_rev, 16)) &&
		(pi->sh->interference_mode == WLAN_AUTO_W_NOISE ||
		pi->sh->interference_mode == NON_WLAN)) {
		PHY_ACI(("phy_n_noise_update_aci_ma: ACI= %s, rxglitch_ma= %d,"
			" badplcp_ma= %d, ofdm_glitch_ma= %d, bphy_glitch_ma=%d,"
			" ofdm_badplcp_ma= %d, bphy_badplcp_ma=%d, crsminpwr index= %d,"
			" init gain= 0x%x, channel= %d\n",
			(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive",
			pi->interf->aci.glitch_ma,
			pi->interf->badplcp_ma,
			pi->interf->noise.ofdm_glitch_ma,
			pi->interf->noise.bphy_glitch_ma,
			pi->interf->noise.ofdm_badplcp_ma,
			pi->interf->noise.bphy_badplcp_ma,
			pi->interf->crsminpwr_index,
			pi->interf->init_gain_core1, CHSPEC_CHANNEL(pi->radio_chanspec)));
	} else {
		PHY_ACI(("phy_n_noise_update_aci_ma: ave glitch %d, ACI is %s, delta is %d\n",
		pi->interf->aci.glitch_ma,
		(pi->aci_state & ACI_ACTIVE) ? "Active" : "Inactive", delta));
	}

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

static void
wlc_phy_noisemode_upd(phy_info_t *pi)
{

	int8 bphy_desense_delta = 0, ofdm_desense_delta = 0;
	uint16 glitch_badplcp_sum;

	if (CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf->curr_home_channel)
		return;

	if (pi->interf->scanroamtimer != 0) {
		/* have not updated moving averages */
		return;
	}

	/* BPHY desense. Sets  pi->interf->noise.bphy_desense in side the func */
	/* Should be run only if associated */
	if (!PUB_NOT_ASSOC(pi) && CHSPEC_IS2G(pi->radio_chanspec)) {

		glitch_badplcp_sum = pi->interf->noise.bphy_glitch_ma +
			pi->interf->noise.bphy_badplcp_ma;

		bphy_desense_delta = wlc_phy_cmn_noisemode_glitch_chk_adj(pi, glitch_badplcp_sum,
			&pi->interf->noise.bphy_thres);

		pi->interf->noise.bphy_desense += bphy_desense_delta;
	} else {
		pi->interf->noise.bphy_desense = 0;
	}

	glitch_badplcp_sum = pi->interf->noise.ofdm_glitch_ma + pi->interf->noise.ofdm_badplcp_ma;
	/* OFDM desense. Sets  pi->interf->noise.ofdm_desense in side the func */
	ofdm_desense_delta = wlc_phy_cmn_noisemode_glitch_chk_adj(pi, glitch_badplcp_sum,
		&pi->interf->noise.ofdm_thres);
	pi->interf->noise.ofdm_desense += ofdm_desense_delta;

	wlc_phy_cmn_noise_limit_desense(pi);

	if (bphy_desense_delta || ofdm_desense_delta) {
		wlc_phy_bphy_ofdm_noise_hw_set_nphy(pi);
	}

	PHY_ACI(("PHY_CMN:  {GL_MA, BP_MA} OFDM {%d, %d},"
	         " BPHY {%d, %d}, Desense - (OFDM, BPHY) = {%d, %d} MAX_Desense {%d, %d},"
	         "channel is %d rssi = %d\n",
	         pi->interf->noise.ofdm_glitch_ma, pi->interf->noise.ofdm_badplcp_ma,
	         pi->interf->noise.bphy_glitch_ma, pi->interf->noise.bphy_badplcp_ma,
	         pi->interf->noise.ofdm_desense, pi->interf->noise.bphy_desense,
	         pi->interf->noise.max_poss_ofdm_desense, pi->interf->noise.max_poss_bphy_desense,
	         CHSPEC_CHANNEL(pi->radio_chanspec), pi->interf->rssi));

}

static int8
wlc_phy_cmn_noisemode_glitch_chk_adj(phy_info_t *pi, uint16 total_glitch_badplcp,
	noise_thresholds_t *thresholds)
{
	int8 desense_delta = 0;

	if (total_glitch_badplcp > thresholds->glitch_badplcp_low_th) {
		/* glitch count is high, could be due to inband noise */
		thresholds->high_detect_total++;
		thresholds->low_detect_total = 0;
	} else {
		/* glitch count not high */
		thresholds->high_detect_total = 0;
		thresholds->low_detect_total++;
	}

	if (thresholds->high_detect_total >= thresholds->high_detect_thresh) {
		/* we have more than glitch_th_up bphy
		 * glitches in a row. so, let's try raising the
		 * inband noise immunity
		 */
		if (total_glitch_badplcp < thresholds->glitch_badplcp_high_th) {
			/* Desense by less */
			desense_delta = thresholds->desense_lo_step;
		} else {
			/* Desense by more */
			desense_delta = thresholds->desense_hi_step;
		}
		thresholds->high_detect_total = 0;
	} else if (thresholds->low_detect_total > 0) {
		/* check to see if we can lower noise immunity */
		uint16 low_detect_total, undesense_wait, undesense_window;

		low_detect_total = thresholds->low_detect_total;
		undesense_wait = thresholds->undesense_wait;
		undesense_window = thresholds->undesense_window;

		/* Reduce the wait time to undesense if glitch count has been low for longer */
		while (undesense_wait > 1) {
			if (low_detect_total <=  undesense_window) {
				break;
			}
			low_detect_total -= undesense_window;
			/* Halve the wait time */
			undesense_wait /= 2;
		}

		if ((low_detect_total % undesense_wait) == 0) {
			/* Undesense */
			desense_delta = -1;
		}
	}
	PHY_ACI(("In %s: recomended desense = %d glitch_ma + badplcp_ma = %d,"
		"th_lo = %d, th_hi = %d \n", __FUNCTION__, desense_delta,
		total_glitch_badplcp, thresholds->glitch_badplcp_low_th,
		thresholds->glitch_badplcp_high_th));
	return desense_delta;
}

static void
wlc_phy_cmn_noise_limit_desense(phy_info_t *pi)
{
	if (pi->interf->rssi != 0) {
		int8 max_desense_rssi = PHY_NOISE_DESENSE_RSSI_MAX;
		int8 desense_margin = PHY_NOISE_DESENSE_RSSI_MARGIN;

		int8 max_desense_dBm, bphy_desense_dB, ofdm_desense_dB;

		pi->interf->noise.bphy_desense = MAX(pi->interf->noise.bphy_desense, 0);
		pi->interf->noise.ofdm_desense = MAX(pi->interf->noise.ofdm_desense, 0);

		max_desense_dBm = MIN(pi->interf->rssi, max_desense_rssi) - desense_margin;

		bphy_desense_dB = max_desense_dBm - pi->interf->bphy_min_sensitivity;
		ofdm_desense_dB = max_desense_dBm - pi->interf->ofdm_min_sensitivity;

		bphy_desense_dB = MAX(bphy_desense_dB, 0);
		ofdm_desense_dB = MAX(ofdm_desense_dB, 0);

		pi->interf->noise.max_poss_bphy_desense = bphy_desense_dB;
		pi->interf->noise.max_poss_ofdm_desense = ofdm_desense_dB;

		pi->interf->noise.bphy_desense =
			MIN(bphy_desense_dB, pi->interf->noise.bphy_desense);
		pi->interf->noise.ofdm_desense =
			MIN(ofdm_desense_dB, pi->interf->noise.ofdm_desense);
	} else {
		pi->interf->noise.max_poss_bphy_desense = 0;
		pi->interf->noise.max_poss_ofdm_desense = 0;

		pi->interf->noise.bphy_desense = 0;
		pi->interf->noise.ofdm_desense = 0;
	}
}
#endif /* WLC_DISABLE_ACI */

static void
phy_n_noise_interf_mode_set(phy_type_noise_ctx_t *ctx, int val)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	pi->sh->interference_mode_2G_override = val;
	pi->sh->interference_mode_5G_override = val;
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		/* for 2G, all values 0 thru 4 are valid */
		pi->sh->interference_mode = pi->sh->interference_mode_2G_override;
	} else {
		/* for 5G, only values 0 and 1 are valid options */
		if (val == 0 || val == 1) {
			pi->sh->interference_mode = pi->sh->interference_mode_5G_override;
		} else {
			/* default 5G interference value to 0 */
			pi->sh->interference_mode = 0;
		}
	}
}

#if defined(BCMDBG)
static int
phy_n_noise_dump(phy_type_noise_ctx_t *ctx, struct bcmstrbuf *b)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;

	return phy_noise_dump_common(pi, b);
}
#endif // endif

#define PHY_NOISE_MAX_IDLETIME		30

static void
phy_n_noise_request_sample(phy_type_noise_ctx_t *ctx, uint8 reason, uint8 ch)
{
	phy_n_noise_info_t *info = (phy_n_noise_info_t *)ctx;
	phy_info_t *pi = info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	bool sampling_in_progress = (pi->phynoise_state != 0);
	bool wait_for_intr = TRUE;

	PHY_NONE(("phy_n_noise_request_sample: state %d reason %d, channel %d\n",
		pi->phynoise_state, reason, ch));

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

	default:
		ASSERT(0);
		break;
	}

	/* start test, save the timestamp to recover in case ucode gets stuck */
	pi->phynoise_now = pi->sh->now;

	/* Fixed noise, don't need to do the real measurement */
	if (pi->phy_fixed_noise) {

		uint8 i;
		FOREACH_CORE(pi, i) {
			pi->phy_noise_win[i][pi->phy_noise_index] =
				PHY_NOISE_FIXED_VAL_NPHY;
		}
		pi->phy_noise_index = MODINC_POW2(pi->phy_noise_index,
			PHY_NOISE_WINDOW_SZ);
		/* legacy noise is the max of antennas */
		noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
		wait_for_intr = FALSE;
		goto done;
	}

	if (!pi->phynoise_polling || (reason == PHY_NOISE_SAMPLE_EXTERNAL) ||
		(reason == PHY_NOISE_SAMPLE_CRSMINCAL)) {

		/* If noise mmt disabled due to LPAS active, do not initiate one */
		if (pi->phynoise_disable)
			return;

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

		classif_state = wlc_phy_classifier_nphy(pi, 0, 0);
		wlc_phy_classifier_nphy(pi, 3, 0);
		wlc_phy_rx_iq_est_nphy(pi, est, num_samps, wait_time, wait_crs);
		wlc_phy_classifier_nphy(pi, NPHY_ClassifierCtrl_classifierSel_MASK,
			classif_state);

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

static int8
phy_n_noise_avg(phy_type_noise_ctx_t *ctx)
{
	phy_n_noise_info_t *noise_info = (phy_n_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;

	int tot = 0;
	int i = 0;

	for (i = 0; i < MA_WINDOW_SZ; i++)
		tot += pi->sh->phy_noise_window[i];

	tot /= MA_WINDOW_SZ;

	return (int8)tot;
}

/* ucode finished phy noise measurement and raised interrupt */
void
phy_n_noise_sample_intr(phy_type_noise_ctx_t *ctx)
{
	phy_n_noise_info_t *noise_info = (phy_n_noise_info_t *) ctx;
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
phy_n_noise_lte_avg(phy_type_noise_ctx_t *ctx)
{
	return 0;
}

static int
phy_n_noise_get_srom_level(phy_type_noise_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_n_noise_info_t *noisei = (phy_n_noise_info_t *) ctx;
	phy_info_t *pi = noisei->pi;
	int8 noiselvl[PHY_CORE_MAX] = {0};
	uint8 core;
	uint16 channel;
	uint32 pkd_noise = 0;

	channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	FOREACH_CORE(pi, core) {

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
phy_n_noise_read_shmem(phy_type_noise_ctx_t *ctx, uint8 *lte_on, uint8 *crs_high)
{
	phy_n_noise_info_t *noise_info = (phy_n_noise_info_t *) ctx;
	phy_info_t *pi = noise_info->pi;
	phy_noise_info_t *noisei = pi->noisei;

	uint32 cmplx_pwr[PHY_CORE_MAX];
	int8 noise_dbm_ant[PHY_CORE_MAX];
	uint16 lo, hi;
	uint32 cmplx_pwr_tot = 0;
	int8 noise_dbm = PHY_NOISE_FIXED_VAL_NPHY;
	uint8 core;

	bzero((uint8 *)cmplx_pwr, sizeof(cmplx_pwr));
	bzero((uint8 *)noise_dbm_ant, sizeof(noise_dbm_ant));

	if (phy_noise_abort_shmem_read(noisei)) {
		return -1;
	}

	/* read SHM, reuse old corerev PWRIND since we are tight on SHM pool */
	FOREACH_CORE(pi, core) {
		lo = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(pi, 2*core));
		hi = wlapi_bmac_read_shm(pi->sh->physhim, M_PWRIND_MAP(pi, 2*core+1));
		cmplx_pwr[core] = (hi << 16) + lo;
	}
#ifdef OCL
	if (PHY_OCL_ENAB(pi->sh->physhim)) {
		uint8 active_cm, ocl_en;
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
			/* Compute rounded average complex power */
			cmplx_pwr[core] = (cmplx_pwr[core] >>
				PHY_NOISE_SAMPLE_LOG_NUM_UCODE) +
				((cmplx_pwr[core] >>
				(PHY_NOISE_SAMPLE_LOG_NUM_UCODE - 1)) & 0x1);
		}
	}

	if (cmplx_pwr_tot == 0)
		PHY_INFORM(("wlc_phy_noise_sample_nphy_compute: timeout in ucode\n"));
	else {
		wlc_phy_noise_calc(pi, cmplx_pwr, noise_dbm_ant, 0);
	}

	wlc_phy_noise_save(pi, noise_dbm_ant, &noise_dbm);

	/* legacy noise is the max of antennas */
	*lte_on = 0;
	*crs_high = 0;
	return noise_dbm;
}
