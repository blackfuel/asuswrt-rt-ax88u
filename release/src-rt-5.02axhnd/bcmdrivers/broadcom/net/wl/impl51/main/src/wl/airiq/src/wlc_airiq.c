/*
 * @file
 * @brief
 *
 *  Air-IQ general
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
#include <wlc_scandb.h>
#include <wlc.h>
#include <phy_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_ac.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#include <wlc_scan.h>
#include <wlc_types.h>
#include <wlc_modesw.h>
#include <wlc_dfs.h>
#include <wlc_radioreg_20693.h>
#include <wlc_event_utils.h>
#include <wlc_event.h>
#include <wlc_vasip.h>
#include <wlc_airiq.h>

void wlc_airiq_scantimer(void *arg);

static void wlc_airiq_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data);
extern void wlc_disable_hw_beacons(wlc_info_t *wlc);

int airiq_doiovar(void *hdl, uint32 actionid, 	void *p, uint plen,
	void *arg, uint alen, uint vsize, struct wlc_if *wlcif);

#ifdef BCMDBG
static void wlc_airiq_reset_fft_counters(airiq_info_t *airiqh);

static void wlc_airiq_dump_fft_counters(airiq_info_t *airiqh, struct bcmstrbuf *b);
#endif /* BCMDBG */
void wlc_airiq_default_scan_channels(airiq_info_t *airiqh);

static const uint16 airiq_fft_latency_bins[AIRIQ_HISTOGRAM_BINCNT] =
{ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700 };
/*  */
/* phy macros */
/*  */

/*
 * Initialize airiq private context. It returns a pointer to the
 * airiq private context if succeeded. Otherwise it returns NULL.
 */
airiq_info_t *
BCMATTACHFN(wlc_airiq_attach) (wlc_info_t * wlc)
{
	airiq_info_t *airiqh;
	extern const bcm_iovar_t airiq_iovars[];

	WL_AIRIQ(("%s: attaching to wl%d\n", __FUNCTION__, wlc->pub->unit));

	/* allocate airiq private info struct */
	airiqh = MALLOC(wlc->osh, sizeof(airiq_info_t));
	if (!airiqh) {
		WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* init airiq private info struct */
	bzero(airiqh, sizeof(airiq_info_t));

	airiqh->wlc = wlc;

	/* register module */
	wlc_module_register(wlc->pub, /* pub */
			    airiq_iovars, /* iovars */
			    "airiq", /* name */
			    airiqh, /* handle */
			    airiq_doiovar, /* iovar fcn */
			    NULL, /* watchdog fcn */
			    NULL, /* up fcn */
			    NULL); /* down fcn */

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "airiq", (dump_fn_t)wlc_airiq_dump, (void*)airiqh);
#endif // endif
	/* This code is for platforms where the WL driver scanning is used */
	airiqh->scan.channel_idx = airiqh->scan.channel_cnt - 1;
	airiqh->timer = wl_init_timer((struct wl_info*)wlc->wl,
			wlc_airiq_scantimer, airiqh, "airiq_scantimer");

	wlc_airiq_default_scan_channels(airiqh);

	airiqh->scan_cpu = 1; /* by default, the first core */
	airiqh->measure_cpu = 1;
	airiqh->idle_thr = 15;
	airiqh->sirq_thr = 70;
	airiqh->fft_reduction = 2;
	airiqh->scan_scale = 3;
	airiqh->cts2self = 0; /* enable cts-to-self when scanning */
	airiqh->scanmute = 0; /* mute radio during home scans */
	airiqh->core     = 0; /* the MIMO chain (0 to PHY_CORE_MAX) used by AIRIQ */
	airiqh->lte_u_scan_configured = FALSE;
	airiqh->detector_config.detector_configured  = FALSE;
	airiqh->lte_u_aging_interval = 10*1000*1000;   //10secs
	airiqh->pkt_up_counter = 0;
	/* get 64-bit aligned pointer */
	airiqh->fft_buffer = (uint8*)(((uintptr)airiqh->__fft_buffer + 7) & (uintptr)(~7));
	ASSERT(ISALIGNED(airiqh->fft_buffer, 8));

	/* 3+1 scanning */
#ifdef WL_MODESW
	/* Register mode switch callback */
	if (WLC_MODESW_ENAB(wlc->pub)) {
		ASSERT(wlc->modesw != NULL);
		if (wlc_modesw_notif_cb_register(wlc->modesw,
				wlc_airiq_opmode_change_cb, wlc) == BCME_OK) {
			airiqh->modesw_cb_regd = TRUE;
		}
	}
#endif /* WL_MODESW */

	/* Allocate debug capture buffer */
	wlc_lte_u_create_iqbuf(airiqh);

	wlc_airiq_phy_init(airiqh);

	/* register up/down callback: needed to upgrade phy */
	if (wlc_bsscfg_updown_register(wlc, wlc_airiq_updown_cb, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		ASSERT(0);
		return NULL;
	}

	airiqh->updown_cb_regd = TRUE;

	return airiqh;
}

/* Cleanup airiq private context */
void
BCMATTACHFN(wlc_airiq_detach) (airiq_info_t * airiqh)
{

	if (!airiqh) {
		return;
	}

#if defined(WL_MODESW)
	if (WLC_MODESW_ENAB(airiqh->wlc->pub)) {
		wlc_modesw_notif_cb_unregister(airiqh->wlc->modesw,
			wlc_airiq_opmode_change_cb, airiqh->wlc);
		airiqh->modesw_cb_regd = FALSE;
	}
#endif /* WL_MODESW */

	wlc_module_unregister(airiqh->wlc->pub, "airiq", airiqh);

	// Free debug capture buffer
	wlc_lte_u_free_iqbuf(airiqh);

	MFREE(airiqh->wlc->osh, airiqh, sizeof(airiq_info_t));
}

#ifdef BCMDBG

static void
wlc_airiq_dump_fft_counters(airiq_info_t *airiqh, struct bcmstrbuf *b)
{
	uint16 k, j;

	bcm_bprintf(b, "Air-IQ FFT latencies\nChan    Tot    Avg  ");
	for (k = 0; k < AIRIQ_HISTOGRAM_BINCNT; k++) {
		bcm_bprintf(b, "%5d ", airiq_fft_latency_bins[k]);
	}
	bcm_bprintf(b, "\n============");
	for (k = 0; k < AIRIQ_HISTOGRAM_BINCNT; k++) {
		bcm_bprintf(b, "======");
	}
	bcm_bprintf(b, "\n");
	for (k = 0; k < MAXCHANNEL; k++) {
		if (airiqh->fft_log_count[k] > 0) {
			airiqh->fft_latency[k].avg =
				airiqh->fft_latency[k].sum / airiqh->fft_log_count[k];

			bcm_bprintf(b, "%3d %9d %4d ", k, airiqh->fft_log_count[k],
				airiqh->fft_latency[k].avg);
			for (j = 0; j < AIRIQ_HISTOGRAM_BINCNT; j++) {
				bcm_bprintf(b, "%5d ", airiqh->fft_latency[k].bin[j]);
			}
			bcm_bprintf(b, "\n");
		}
	}
}

static void
wlc_airiq_reset_fft_counters(airiq_info_t *airiqh)
{
	bzero(airiqh->fft_latency, sizeof(airiqh->fft_latency));
	bzero(airiqh->fft_log_count, sizeof(airiqh->fft_log_count));
}

void
wlc_airiq_log_fft(airiq_info_t *airiqh, uint16 channel, uint16 latency)
{
	uint16 k;

	airiqh->fft_log_count[channel]++;
	for (k = 0; k < AIRIQ_HISTOGRAM_BINCNT - 1; k++) {
		if (airiq_fft_latency_bins[k] > latency) {
			airiqh->fft_latency[channel].bin[k]++;
			airiqh->fft_latency[channel].sum += latency;
			return;
		}
	}
	airiqh->fft_latency[channel].bin[AIRIQ_HISTOGRAM_BINCNT - 1]++;
}

int
wlc_airiq_dump(airiq_info_t *airiqh, struct bcmstrbuf *b)
{

	wlc_airiq_phy_dump_gain(airiqh, b);
	wlc_airiq_dump_fft_counters(airiqh, b);
	wlc_airiq_reset_fft_counters(airiqh);

	return 0;
}
#endif /* BCMDBG */

static void
wlc_airiq_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data)
{
	wlc_info_t *wlc = (wlc_info_t*)ctx;
	airiq_info_t *airiqh;

#ifdef WLOFFLD
	airiq_message_header_t *hdr;
	uint8 *buffer;
	uint32 msg_size;
#endif // endif

	ASSERT(wlc);
	ASSERT(updown_data);
	if (wlc->airiq == NULL) {
		return;
	}
	airiqh = wlc->airiq;

	WL_AIRIQ(("%s:got callback from updown. interface %s\n",
		__FUNCTION__, (updown_data->up ? "up" : "down")));

	if (updown_data->up == TRUE) {
#ifdef WLOFFLD
		// Send Offload core Reboot message up to user-space SWSA
		airiqh->ol_seq_last = 0;

		// Critical section
		spin_lock_bh(&airiq.lock);

		msg_size = sizeof(airiq_message_header_t);
		buffer = MALLOCZ(airiqh->wlc->osh, msg_size);

		if (buffer) {
			hdr = (airiq_message_header_t*)buffer;

			/* Setup the data hdr */
			hdr->message_type = MESSAGE_TYPE_OL_REBOOT;
			hdr->corerev = airiqh->wlc->pub->corerev;
			hdr->unit = airiqh->wlc->pub->unit;
			hdr->size_bytes = sizeof(airiq_message_header_t);

			wl_airiq_sendup_data(airiqh, buffer,  msg_size);
			MFREE(airiqh->wlc->osh, buffer, msg_size);
		} else {
			WL_AIRIQ(("%s: Could not get bytes: queue full\n", __FUNCTION__));
		}

		spin_unlock_bh(&airiq.lock);
		// end critical section
#endif /* WLOFFLD */
		airiqh->modeswitch_state = AIRIQ_MODESW_IDLE;
		airiqh->phy_mode = PHYMODE(airiqh->wlc);

		/* Initialize PHY overrides */
		wlc_airiq_phy_init_overrides(airiqh);

		return;
	}
	/* Interface is going down */
	if (airiqh->phy_mode == PHYMODE_3x3_1x1) {
		WL_AIRIQ(("Aborting Air-IQ scan\n"));
		wlc_airiq_scan_abort(airiqh, TRUE);
	}
}

/* callback after tx fifo suspended. */
void
wlc_airiq_fifo_suspend_complete(airiq_info_t *airiqh)
{
	WL_PRINT(("%s: TXPKTPENDTOT=%d\n", __FUNCTION__,
		TXPKTPENDTOT(airiqh->wlc)));

	airiqh->tx_suspending = FALSE;
}

void
wlc_airiq_set_scan_in_progress(airiq_info_t *airiqh, bool in_progress)
{
	airiqh->wlc->scan->in_progress = in_progress;

	if (in_progress) {
		ASSERT(STAY_AWAKE(airiqh->wlc));
	} else {
		/* note: no assert needed in this path since there could be other conditions
		 * causing wake state to be set
		 */
	}
	wlc_set_wake_ctrl(airiqh->wlc);
}

int wl_airiq_sendup_data(airiq_info_t *airiqh, uint8 *data, unsigned long size)
{
	airiq_event_t *airiq_event;

	airiq_event = MALLOC(airiqh->wlc->osh, sizeof(airiq_event_t) + size);
	airiq_event->airiq_event_type = AIRIQ_EVENT_DATA;
	airiq_event->data_len = sizeof(airiq_event_t) + size;

	memcpy(airiq_event->data, data, size);

	airiq_event->pkt_counter = airiqh->pkt_up_counter++;

	wlc_mac_event(airiqh->wlc, WLC_E_AIRIQ_EVENT, NULL, WLC_E_STATUS_SUCCESS,
		0, 0, (void*)(airiq_event), airiq_event->data_len);

	MFREE(airiqh->wlc->osh, airiq_event, sizeof(airiq_event_t) + size);

	return TRUE;
}

void wl_airiq_sendup_scan_complete_alternate(airiq_info_t *airiqh, uint16 status)
{
	airiq_event_t *airiq_event;
	uint16 radio;

	radio = airiqh->wlc->pub->unit;

	/* counter increments to indicate an alternate scan-complete event */
	if (radio < SCAN_COMPLETE_RADIOS) {
		/* handle special  case for RSDB  radio 47452 */

		/* now send it up */
		airiq_event = MALLOCZ(airiqh->wlc->osh, sizeof(airiq_event_t));
		airiq_event->airiq_event_type = AIRIQ_EVENT_SCAN_COMPLETE;
		airiq_event->data_len = sizeof(airiq_event_t);
		airiq_event->scan_status[radio] = status;
		airiqh->scan_complete[radio]++;
		airiq_event->scan_complete[radio] = airiqh->scan_complete[radio];
		airiq_event->pkt_counter = airiqh->pkt_up_counter++;

		wlc_mac_event(airiqh->wlc, WLC_E_AIRIQ_EVENT, NULL, WLC_E_STATUS_SUCCESS,
			0, 0, (void*)(airiq_event), airiq_event->data_len);
		MFREE(airiqh->wlc->osh, airiq_event, sizeof(airiq_event_t));
		return;

	} else {
		WL_ERROR(("%s: invalid radio number %d > %d\n",
			__FUNCTION__, radio, SCAN_COMPLETE_RADIOS - 1));
	}
}

bool wl_lte_u_send_scan_abort_event(airiq_info_t *airiqh, int reason)
{
	lte_u_event_t *lte_u_event;

	lte_u_event = MALLOC(airiqh->wlc->osh, sizeof(lte_u_event_t) + sizeof(uint32));
	lte_u_event->lte_u_event_type = LTE_U_EVENT_SCAN_ABORT;
	lte_u_event->data_len = sizeof(lte_u_event_t) + sizeof(uint32);
	memcpy(lte_u_event->data, &reason, sizeof(uint32));

	WL_AIRIQ(("%s Sending scan abort event\n", __FUNCTION__));
	wlc_mac_event(airiqh->wlc, WLC_E_LTE_U_EVENT, NULL, WLC_E_STATUS_SUCCESS,
			0, 0, (void *)(lte_u_event), lte_u_event->data_len);
	MFREE(airiqh->wlc->osh, lte_u_event, sizeof(lte_u_event_t) + sizeof(uint32));
	return TRUE;
}

int wlc_lte_u_get_user_channel_index_from_scanchan(airiq_info_t *airiqh, int scan_channel)
{
	int i;
	int scan_channel_index = 0;

	for (i = 0; i < airiqh->scan.channel_cnt; i++) {
		if (airiqh->scan.chanspec_list[i] == scan_channel) {
			scan_channel_index = i;
		}
	}
	return airiqh->user_channel_mapping[scan_channel_index];
}

void wlc_lte_u_send_status(airiq_info_t *airiqh)
{
	int  scan_chidx = airiqh->scan.channel_idx;

	lte_u_event_t *lte_u_event;

	/* lte_u_present: LTE-U signal detected for the scan channel
	 * lte_u_active: used to mark the detection active from when
	 * lte_u_present=true --> lte_u_present=false+aging_interval
	 */
	if ((airiqh->scan.lte_scan_status[scan_chidx].lte_u_present) ||
		(airiqh->scan.lte_scan_status[scan_chidx].lte_u_active &&
		((airiqh->scan.lte_scan_status[scan_chidx].timestamp -
			airiqh->scan.lte_scan_status[scan_chidx].prevTimestamp)
			<= airiqh->lte_u_aging_interval))) {
		/* Send the mac event if LTE-U present OR
		 * Send the mac event if we are within aging interval
		 */
		WL_AIRIQ(("wl%d:[LTEU] time: 0x%x (prev 0x%x) active: %d present: %d\n",
			airiqh->wlc->pub->unit,
			airiqh->scan.lte_scan_status[scan_chidx].timestamp,
			airiqh->scan.lte_scan_status[scan_chidx].prevTimestamp,
			airiqh->scan.lte_scan_status[scan_chidx].lte_u_active,
			airiqh->scan.lte_scan_status[scan_chidx].lte_u_present));

		airiqh->scan.lte_scan_status[scan_chidx].lte_u_active = TRUE;

		if (airiqh->scan.lte_scan_status[scan_chidx].lte_u_present) {
			airiqh->scan.lte_scan_status[scan_chidx].prevTimestamp =
				airiqh->scan.lte_scan_status[scan_chidx].timestamp;
		}

		lte_u_event = MALLOC(airiqh->wlc->osh, sizeof(lte_u_event_t) +
			sizeof(lte_u_scan_status_t));
		lte_u_event->lte_u_event_type = LTE_U_EVENT_SCAN_STATUS;
		lte_u_event->data_len = sizeof(lte_u_event_t) + sizeof(lte_u_scan_status_t);
		memcpy(lte_u_event->data, &(airiqh->scan.lte_scan_status[scan_chidx]),
			sizeof(lte_u_scan_status_t));

		wlc_mac_event(airiqh->wlc, WLC_E_LTE_U_EVENT, NULL, WLC_E_STATUS_SUCCESS,
				0, 0, (void *)(lte_u_event), lte_u_event->data_len);
		MFREE(airiqh->wlc->osh, lte_u_event, sizeof(lte_u_event_t) +
			sizeof(lte_u_scan_status_t));
	} else if (airiqh->scan.lte_scan_status[scan_chidx].lte_u_active &&
			((airiqh->scan.lte_scan_status[scan_chidx].timestamp -
			airiqh->scan.lte_scan_status[scan_chidx].prevTimestamp) >
			airiqh->lte_u_aging_interval)) {
		/* Send the mac event to indicate end of detection (i.e. lte_u_active=false) */
		WL_AIRIQ(("wl%d:[LTEU] time: 0x%x (prev 0x%x) active: %d present: %d\n",
			airiqh->wlc->pub->unit,
			airiqh->scan.lte_scan_status[scan_chidx].timestamp,
			airiqh->scan.lte_scan_status[scan_chidx].prevTimestamp,
			airiqh->scan.lte_scan_status[scan_chidx].lte_u_active,
			airiqh->scan.lte_scan_status[scan_chidx].lte_u_present));
		airiqh->scan.lte_scan_status[scan_chidx].lte_u_active = FALSE;

		lte_u_event = MALLOC(airiqh->wlc->osh, sizeof(lte_u_event_t) +
			sizeof(lte_u_scan_status_t));
		lte_u_event->lte_u_event_type = LTE_U_EVENT_SCAN_STATUS;
		lte_u_event->data_len = sizeof(lte_u_event_t) + sizeof(lte_u_scan_status_t);
		memcpy(lte_u_event->data, &(airiqh->scan.lte_scan_status[scan_chidx]),
			sizeof(lte_u_scan_status_t));

		wlc_mac_event(airiqh->wlc, WLC_E_LTE_U_EVENT, NULL, WLC_E_STATUS_SUCCESS,
			0, 0, (void *)(lte_u_event), lte_u_event->data_len);
		MFREE(airiqh->wlc->osh, lte_u_event, sizeof(lte_u_event_t) +
			sizeof(lte_u_scan_status_t));
	}
}
