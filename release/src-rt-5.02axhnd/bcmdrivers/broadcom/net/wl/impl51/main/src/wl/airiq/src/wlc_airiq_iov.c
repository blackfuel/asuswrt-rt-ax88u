/*
 * @file
 * @brief
 *
 *  Air-IQ IOVAR
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

#include <wlc_airiq.h>
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
#include <wlc_tx.h>
#include <wlc_scandb.h>
#include <wlc.h>
#include <phy_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phyreg_ac.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#include <wlc_scan.h>
#include <wlc_types.h>
#include <wlc_airiq.h>
#include <wlc_modesw.h>
#include <wlc_dfs.h>
#include <wlc_radioreg_20693.h>

#include <phy_dbg.h>
#include <phy_mem.h>
#include <bcm_math.h>
#include <phy_btcx.h>
#include <phy_calmgr.h>
#include <phy_type_txiqlocal.h>
#include <phy_ac.h>
#include <phy_ac_txiqlocal.h>
#include <wlc_radioreg_20691.h>
#include <wlc_radioreg_20693.h>
#include <wlc_radioreg_20694.h>
#include <wlc_radioreg_20695.h>
#include <wlc_radioreg_20696.h>
#include <wlc_radioreg_20697.h>
#include <wlc_radioreg_20698.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <wlc_phy_radio.h>
#include <phy_ac_info.h>
#include <phy_ac_radio.h>
#include <phy_rxgcrs_api.h>
#include <phy_ac_tof.h>
#include <phy_rstr.h>
#include <phy_utils_var.h>
#include <bcmdevs.h>
#include <phy_stf.h>

/*  */
/* AIRIQ IOVars */
/*  */
enum {
	/* scan control */
	IOV_AIRIQ_SCANNING               = 0x01,
	IOV_AIRIQ_SCAN                   = 0x02,
	IOV_AIRIQ_SCAN_ABORT             = 0x03,
	IOV_AIRIQ_HOME_SCAN              = 0x04,
	IOV_AIRIQ_CORE                   = 0x05,
	/* gain/radio settings */
	IOV_AIRIQ_GAIN_TABLE_24GHZ       = 0x10,
	IOV_AIRIQ_GAIN_TABLE_5GHZ        = 0x11,
	IOV_AIRIQ_GAIN                   = 0x12,
	/* CPU load optimization/tuning */
	IOV_AIRIQ_SCAN_CPU               = 0x20,
	IOV_AIRIQ_MEASURE_CPU            = 0x21,
	IOV_AIRIQ_FFT_REDUCTION          = 0x22,
	IOV_AIRIQ_SCAN_SCALE             = 0x23,
	IOV_AIRIQ_SIRQ_THR               = 0x24,
	IOV_AIRIQ_IDLE_THR               = 0x25,
	/* Wi-Fi traffic coexistence tuning */
	IOV_AIRIQ_CTS2SELF               = 0x40,
	IOV_AIRIQ_SCANMUTE               = 0x41,
	IOV_AIRIQ_MANGAIN                = 0x50,
#ifdef WLOFFLD
	/* Air-IQ Offload */
	IOV_AIRIQ_OFFLOAD                = 0x60,
	IOV_AIRIQ_OFFLOAD_CMD            = 0x61,
#endif // endif
#ifdef BCMDBG
	/* Debug */
	IOV_AIRIQ_TXTONE                 = 0x80,
	IOV_AIRIQ_3PLUS1                 = 0x81,
#endif /* BCMDBG */
	IOV_LTE_U_SCAN_START             = 0x90,
	IOV_LTE_U_SCAN_ABORT             = 0x91,
	IOV_LTE_U_SCAN_CONFIG            = 0x92,
	IOV_LTE_U_DETECTOR_CONFIG        = 0x93,
	IOV_LTE_U_DEBUG_CAPTURE          = 0x94,
	IOV_LTE_U_AGING_INTERVAL         = 0x95
};

const bcm_iovar_t airiq_iovars[] = {
	{ "airiq_scanning",
	IOV_AIRIQ_SCANNING, (0), (0), IOVT_UINT16, 0
	},
	{ "airiq_scan",
	IOV_AIRIQ_SCAN,	(0), (0), IOVT_BUFFER, sizeof(airiq_config_t)
	},
	{ "airiq_home_scan",
	IOV_AIRIQ_HOME_SCAN, (0), (0), IOVT_UINT32, 0
	},
	{ "airiq_scan_abort",
	IOV_AIRIQ_SCAN_ABORT, (0), (0), IOVT_UINT16, 0
	},
	{ "airiq_gain_table_24ghz",
	IOV_AIRIQ_GAIN_TABLE_24GHZ,	(0), (0), IOVT_BUFFER, sizeof(airiq_gain_table_t)
	},
	{ "airiq_gain_table_5ghz",
	IOV_AIRIQ_GAIN_TABLE_5GHZ, (0), (0), IOVT_BUFFER, sizeof(airiq_gain_table_t)
	},
	{ "airiq_gain",
	IOV_AIRIQ_GAIN,	(0), (0), IOVT_BUFFER, sizeof(airiq_gain_t)
	},
	{ "airiq_scan_cpu",
	IOV_AIRIQ_SCAN_CPU,	(0), (0), IOVT_INT32, 0
	},
	{ "airiq_measure_cpu",
	IOV_AIRIQ_MEASURE_CPU, (0), (0), IOVT_UINT32, 0
	},
	{ "airiq_fft_reduction",
	IOV_AIRIQ_FFT_REDUCTION, (0), (0), IOVT_UINT32, 0
	},
	{ "airiq_scan_scale",
	IOV_AIRIQ_SCAN_SCALE, (0), (0), IOVT_UINT32, 0
	},
	{ "airiq_sirq_thr",
	IOV_AIRIQ_SIRQ_THR,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_idle_thr",
	IOV_AIRIQ_IDLE_THR,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_cts2self",
	IOV_AIRIQ_CTS2SELF,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_scanmute",
	IOV_AIRIQ_SCANMUTE,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_core",
	IOV_AIRIQ_CORE,	(0), (0), IOVT_UINT32, 0
	},
#ifdef WLOFFLD
	{ "airiq_offload",
	IOV_AIRIQ_OFFLOAD, (0), (0), IOVT_UINT32, 0
	},
	{ "airiq_ol_cmd",
	IOV_AIRIQ_OFFLOAD_CMD, (0), (0), IOVT_BUFFER, 0
	},
#endif /* WLOFFLD */
#ifdef BCMDBG
#ifdef WLOFFLD
	{ "airiq_olt_len",
	IOV_AIRIQ_OFFLOAD_TPUT_LEN,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_olt_cnt",
	IOV_AIRIQ_OFFLOAD_TPUT_CNT,	(0), (0), IOVT_UINT32, 0
	},
	{ "airiq_ol_test",
	IOV_AIRIQ_OFFLOAD_TPUT_TEST, (0), (0), IOVT_UINT32, 0
	},
#endif /* WLOFFLD */
#endif /* BCMDBG */
	{ "airiq_mangain",
	IOV_AIRIQ_MANGAIN, (0), (0), IOVT_UINT32, 0
	},
#ifdef BCMDBG
	{ "airiq_txtone",
	IOV_AIRIQ_TXTONE, (0), (0), IOVT_INT32, 0
	},
	{ "airiq_3plus1",
	IOV_AIRIQ_3PLUS1, (0), (0), IOVT_UINT32, 0
	},
#endif // endif
	{ "lte_u_scan_config",
	IOV_LTE_U_SCAN_CONFIG, (0), (0), IOVT_BUFFER, sizeof(airiq_config_t)
	},
	{ "lte_u_detector_config",
	IOV_LTE_U_DETECTOR_CONFIG, (0), (0), IOVT_BUFFER, sizeof(lte_u_detector_config_t)
	},
	{ "lte_u_scan_start",
	IOV_LTE_U_SCAN_START, (0), (0), IOVT_UINT16, 0
	},
	{ "lte_u_scan_abort",
	IOV_LTE_U_SCAN_ABORT, (0),  (0),IOVT_UINT16, 0
	},
	{ "lte_u_debug_capture",
	IOV_LTE_U_DEBUG_CAPTURE, (0), (0), IOVT_UINT32, 0
	},
	{ "lte_u_aging_interval",
	IOV_LTE_U_AGING_INTERVAL, (0), (0), IOVT_UINT32, 0 },
	{ NULL, 0, 0, 0, 0 }
};

/* Handling AIRIQ related iovars */
int
airiq_doiovar(void *hdl, uint32 actionid, 	void *p, uint plen,
	void *arg, uint alen, uint vsize, struct wlc_if *wlcif)
{
	airiq_info_t *airiqh = (airiq_info_t*)hdl;
	int err = 0;
	int32 int_val = 0;

	uint32 *ret_int_ptr = (uint32*)arg;
	phy_info_t *pi;

	if (!airiqh) {
		WL_ERROR(("%s: null airiq handle\n", __FUNCTION__));
		return BCME_ERROR;
	}
	if (!airiqh->wlc) {
		WL_ERROR(("%s: null wlc\n", __FUNCTION__));
		return BCME_ERROR;
	}
	if (!airiqh->wlc->band) {
		WL_ERROR(("%s: null band\n", __FUNCTION__));
		return BCME_ERROR;
	}
	if (!WLC_PI(airiqh->wlc)) {
		WL_ERROR(("%s: null hwpi\n", __FUNCTION__));
		return BCME_ERROR;
	}
#ifdef DONGLEBUILD
	if (plen >= MAXPKTDATABUFSZ) {
		WL_ERROR(("%s: iovar attachment too large\n", __FUNCTION__));
		return BCME_BUFTOOLONG;
	}
#endif // endif
	pi = (phy_info_t*)WLC_PI(airiqh->wlc);

	if (plen >= (int)sizeof(int_val)) {
		bcopy(p, &int_val, sizeof(int_val));
	}

	switch (actionid) {
	case IOV_GVAL(IOV_AIRIQ_SCANNING):
		WL_AIRIQ(("%s: airiq scanning query: %d\n", __FUNCTION__, airiqh->scan_enable));
		*ret_int_ptr = airiqh->scan_enable;
		break;
	case IOV_GVAL(IOV_AIRIQ_GAIN_TABLE_24GHZ):
	{
		airiq_gain_table_t *gt = (airiq_gain_table_t*)arg;
		memcpy(gt, &airiqh->gaintbl_24ghz, sizeof(airiq_gain_table_t));
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_GAIN_TABLE_5GHZ):
	{
		airiq_gain_table_t *gt = (airiq_gain_table_t*)arg;
		memcpy(gt, &airiqh->gaintbl_5ghz, sizeof(airiq_gain_table_t));
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_GAIN):
	{
		airiq_gain_t *g = (airiq_gain_t*)arg;
		err = airiq_get_gain(airiqh, pi, g);
		break;
	}
	case IOV_SVAL(IOV_AIRIQ_GAIN):
	{
		// if offload enabled, gain control is performed by
		// offload engine.
#ifdef WLOFFLD
		if (WLOFFLD_AIRIQ_ENAB(airiqh->wlc->pub)) {
			WL_AIRIQ(("airiq_gain: ignoring iovar gain control from applicaton\n"));

		} else
#endif /* WLOFFLD */
		{
			airiq_gain_t *g = (airiq_gain_t*)arg;
			err = airiq_set_gain(airiqh, g);
		}
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_SCAN):
		err = airiq_get_scan_config(airiqh, (airiq_config_t*)arg, alen);
		break;
	case IOV_SVAL(IOV_AIRIQ_SCAN):
	{
		airiq_config_t *sc = (airiq_config_t*)arg;
		WL_AIRIQ(("%s: scan.\n", __FUNCTION__));

		err = airiq_update_scan_config(airiqh, sc);

		if (err != BCME_OK) {
			break;
		}
		if (!chanspec_list_valid(airiqh, sc)) {
			return BCME_UNSUPPORTED;
		}
		if (sc->start) {
			if (SCAN_IN_PROGRESS(airiqh->wlc->scan)) {
				WL_AIRIQ(("%s: Cannot start scan: WL scan in progress\n",
					__FUNCTION__));
				err = BCME_BUSY;
#if defined(WLDFS) && (defined(RSDB_DFS_SCAN) || defined(BGDFS))
			} else if (wlc_dfs_scan_in_progress(airiqh->wlc->dfs)) {
				WL_AIRIQ(("%s: Cannot start scan: DFS scan in progress\n",
					__FUNCTION__));
				err = BCME_BUSY;
#endif /* WLDFS && (RSDB_DFS_SCAN || BGDFS) */
			} else {
				// now start the scan.
				airiqh->scan_type = SCAN_TYPE_AIRIQ;
				err = wlc_airiq_start_scan(airiqh, sc->sweep_cnt, 0);
			}
		}
		break;
	}
	case IOV_SVAL(IOV_AIRIQ_HOME_SCAN):
	{
		WL_AIRIQ(("%s: scan.\n", __FUNCTION__));

		// now start the scan.
		airiqh->scan.capture_count[0] = (uint32)int_val;
		airiqh->scan.dwell_interval_ms[0] = (uint32)int_val / 10;
		airiqh->scan.capture_interval_us[0] = 100;
		airiqh->scan.chanspec_list[0] = airiqh->wlc->home_chanspec;
		airiqh->scan.channel_cnt = 1;
		airiqh->scan.channel_idx = 0;
		airiqh->scan_type = SCAN_TYPE_AIRIQ;
		err = wlc_airiq_start_scan(airiqh, 1, (uint32)int_val);

		break;
	}
	case IOV_SVAL(IOV_AIRIQ_SCAN_ABORT):
		WL_AIRIQ(("%s: scan abort.\n", __FUNCTION__));
		/* Decode scan settings into args list */
		err = wlc_airiq_scan_abort(airiqh, TRUE);
		break;
	case IOV_GVAL(IOV_AIRIQ_SCAN_CPU):
		*ret_int_ptr = airiqh->scan_cpu;
		err = 0;
		break;
	case IOV_SVAL(IOV_AIRIQ_SCAN_CPU):
		if ((int)int_val < 2) {
			airiqh->scan_cpu = (int)int_val;
			err = 0;
		} else {
			err = BCME_BADARG;
		}
		break;
	case IOV_SVAL(IOV_AIRIQ_MEASURE_CPU):
		airiqh->measure_cpu = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_MEASURE_CPU):
		*ret_int_ptr = airiqh->measure_cpu;
		break;
	case IOV_SVAL(IOV_AIRIQ_FFT_REDUCTION):
		airiqh->fft_reduction = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_FFT_REDUCTION):
		*ret_int_ptr = airiqh->fft_reduction;
		break;
	case IOV_SVAL(IOV_AIRIQ_SCAN_SCALE):
		airiqh->scan_scale = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_SCAN_SCALE):
		*ret_int_ptr = airiqh->scan_scale;
		break;
	case IOV_SVAL(IOV_AIRIQ_SIRQ_THR):
		airiqh->sirq_thr = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_SIRQ_THR):
		*ret_int_ptr = airiqh->sirq_thr;
		break;
	case IOV_SVAL(IOV_AIRIQ_IDLE_THR):
		airiqh->idle_thr = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_IDLE_THR):
		*ret_int_ptr = airiqh->idle_thr;
		break;
	case IOV_SVAL(IOV_AIRIQ_CTS2SELF):
		airiqh->cts2self = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_CTS2SELF):
		*ret_int_ptr = airiqh->cts2self;
		break;
	case IOV_SVAL(IOV_AIRIQ_SCANMUTE):
		airiqh->scanmute = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_SCANMUTE):
		*ret_int_ptr = airiqh->scanmute;
		break;
	case IOV_SVAL(IOV_AIRIQ_MANGAIN):
		airiqh->mangain = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_MANGAIN):
		*ret_int_ptr = airiqh->mangain;
		break;
	case IOV_SVAL(IOV_AIRIQ_CORE):
		if ((uint32)int_val < PHY_CORE_MAX) {
			airiqh->core = (uint32)int_val;
		} else {
			err = BCME_UNSUPPORTED;
		}
		break;
	case IOV_GVAL(IOV_AIRIQ_CORE):
		*ret_int_ptr = airiqh->core;
		break;
#ifdef WLOFFLD
	case IOV_SVAL(IOV_AIRIQ_OFFLOAD_CMD):
	{
		airiq_ol_cmd_t *olcmd = (airiq_ol_cmd_t*)arg;

		if (!WLOFFLD_AIRIQ_ENAB(airiqh->wlc->pub)) {
			WL_ERROR(("wl%d: Set IOV_AIRIQ_OFFLOAD_CMD disabled: 0x%x length=%d\n",
				WLCWLUNIT(airiqh->wlc), olcmd->command, olcmd->size));
			return BCME_UNSUPPORTED;
		}

		if (airiqh->pending_olcmd.command_status != AIRIQ_OL_CMD_IDLE) {
			WL_ERROR(("Air-IQ offload cmd: squashing ongoing cmd w/ status = %d.\n",
				airiqh->pending_olcmd.command_status));
		}

		if (airiqh->olcmd_buffer) {
			MFREE(airiqh->wlc->osh, airiqh->olcmd_buffer, airiqh->olcmd_buffer_size);
			airiqh->olcmd_buffer = NULL;
			airiqh->olcmd_buffer_size = 0;
		}

		WL_AIRIQ(("wl%d: Sending Air-IQ offload command: 0x%x length=%d\n",
			WLCWLUNIT(airiqh->wlc), olcmd->command, olcmd->size));

		err = wlc_airiq_ol_cmd(airiqh->wlc, olcmd);
		if (err == BCME_OK) {
			airiqh->pending_olcmd.command_status = AIRIQ_OL_CMD_PENDING;
		} else {
			airiqh->pending_olcmd.command_status = AIRIQ_OL_CMD_ABORT;
		}
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_OFFLOAD_CMD):
	{

		airiq_ol_cmd_t *olcmd = (airiq_ol_cmd_t*)arg;

		olcmd->command_status = airiqh->pending_olcmd.command_status;

		WL_AIRIQ(("Air-IQ Get OLCMD status=%d alen=%d size=%d bufsz=%d\n",
			olcmd->command_status, alen, airiqh->pending_olcmd.size,
			airiqh->olcmd_buffer_size));

		if (!WLOFFLD_AIRIQ_ENAB(airiqh->wlc->pub)) {
			WL_ERROR(("wl%d: Get OLCMD (disabled) 0x%x length=%d\n",
				WLCWLUNIT(airiqh->wlc), olcmd->command, olcmd->size));
			return BCME_UNSUPPORTED;
		}

		if (airiqh->pending_olcmd.command_status != AIRIQ_OL_CMD_COMPLETE) {
			break;
		}

		memcpy(olcmd, &airiqh->pending_olcmd, sizeof(airiq_ol_cmd_t));

		if (airiqh->olcmd_buffer_size > 0) {
			if (alen >= sizeof(airiq_ol_cmd_t) + airiqh->olcmd_buffer_size) {
				memcpy(olcmd + 1, airiqh->olcmd_buffer, airiqh->olcmd_buffer_size);
			} else {
				WL_ERROR(("Air-IQ OLCMD (short): alen=%d olcmdsz=%d bufsz=%d\n",
					alen, (int)sizeof(airiq_ol_cmd_t),
					airiqh->olcmd_buffer_size));
				err = BCME_BUFTOOSHORT;
			}

			if (airiqh->olcmd_buffer) {
				MFREE(airiqh->wlc->osh, airiqh->olcmd_buffer,
				    airiqh->olcmd_buffer_size);
				airiqh->olcmd_buffer = NULL;
				airiqh->olcmd_buffer_size = 0;
			}
		}

		airiqh->pending_olcmd.command_status = AIRIQ_OL_CMD_IDLE;
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_OFFLOAD):
		/* TODO: query A7 fw? */
		*ret_int_ptr = WLOFFLD_AIRIQ_ENAB(airiqh->wlc->pub);
		break;
#endif /* WLOFFLD */
#ifdef BCMDBG
#ifdef WLOFFLD
	case IOV_SVAL(IOV_AIRIQ_OFFLOAD_TPUT_LEN):
		airiqh->offload_tput_len = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_OFFLOAD_TPUT_LEN):
		*ret_int_ptr = airiqh->offload_tput_len;
		break;
	case IOV_SVAL(IOV_AIRIQ_OFFLOAD_TPUT_CNT):
		airiqh->offload_tput_cnt = (uint32)int_val;
		break;
	case IOV_GVAL(IOV_AIRIQ_OFFLOAD_TPUT_CNT):
		*ret_int_ptr = airiqh->offload_tput_cnt;
		break;
	case IOV_SVAL(IOV_AIRIQ_OFFLOAD_TPUT_TEST):
	{
		uint32 dummy_tsf_high;

		/* reset time and byte counters */
		airiqh->offload_tput_bytes = 0;
		wlc_read_tsf(airiqh->wlc, &airiqh->offload_tput_start_tsf, &dummy_tsf_high);

		err = wlc_airiq_ol_tput_test(airiqh->wlc, airiqh->offload_tput_len,
			airiqh->offload_tput_cnt);
	}
	break;
#endif /* WLOFFLD */
	case IOV_SVAL(IOV_AIRIQ_TXTONE):
	{
		if (int_val == 0) {
			wlc_phy_stopplayback_acphy(pi,STOPPLAYBACK_W_CCA_RESET);
		} else {
			phy_iq_est_t loopback_rx_iq[PHY_CORE_MAX];
			int32 freq_khz = int_val / 2;
			WL_AIRIQ(("Turned on TX tone at %d kHz offset\n", int_val));

			wlc_phy_tx_tone_acphy(pi, freq_khz, ACPHY_RXCAL_TONEAMP, 0, FALSE);

			wlc_phy_rx_iq_est_acphy(pi, loopback_rx_iq, 0x3000, 32, 0, TRUE);
		}
		break;
	}
	case IOV_GVAL(IOV_AIRIQ_3PLUS1):
		if (airiqh->phy_mode != PHYMODE_3x3_1x1) {
			*ret_int_ptr = -1;
		} else {
			int32 radio_chanspec_sc;
			phy_ac_chanmgr_get_val_sc_chspec(PHY_AC_CHANMGR(pi), &radio_chanspec_sc);
			*ret_int_ptr = CHSPEC_CHANNEL(radio_chanspec_sc);
		}
		break;
	case IOV_SVAL(IOV_AIRIQ_3PLUS1):
	{
		/* when the phy is downgraded, the +1 chanspec is set to scan.chanspec_list[0] */
		airiqh->scan.chanspec_list[0] = CHSPEC_BW(pi->radio_chanspec) | int_val;
		if (int_val < 20) {
			airiqh->scan.chanspec_list[0] |= WL_CHANSPEC_BAND_2G;
		} else {
			airiqh->scan.chanspec_list[0] |= WL_CHANSPEC_BAND_5G;
		}

		airiqh->scan.scan_start = 0; /* do not begin scanning */

		/* Place the radio into the airiq 3+1 mode with the given chanspec on the +1 */
		err = wlc_airiq_3p1_scan_prep(airiqh);

		if (err == BCME_BUSY) {
			/* Must reconfigure and wait for callback */
			WL_AIRIQ(("%s: preparing for 3+1 scan. downgrade\n", __FUNCTION__));
#if defined(WL_MODESW)
			wlc_airiq_modeswitch_state_upd(airiqh, AIRIQ_MODESW_DOWNGRADE_IN_PROGRESS);
#endif // endif
			airiqh->phy_mode = PHYMODE_3x3_1x1;
			return 0;
		} else if (err != BCME_OK) {
			WL_ERROR(("%s: error starting 3+1 scan, couldn't downgrade (%d)\n",
				__FUNCTION__, err));
			return err;
		}
		/* phymode is already set */
		wlc_airiq_3p1_downgrade_phy(airiqh, airiqh->scan.chanspec_list[0]);

		break;
	}
#endif /* BCMDBG */
	case IOV_GVAL(IOV_LTE_U_SCAN_CONFIG):
		err = lte_u_get_scan_config(airiqh, (airiq_config_t*)arg, alen);
		break;
	case IOV_SVAL(IOV_LTE_U_SCAN_CONFIG):
	{
		airiq_config_t *sc = (airiq_config_t*)arg;
		WL_AIRIQ(("%s: scan config.\n", __FUNCTION__));

		if (! lte_u_chanspec_list_valid(airiqh, sc)) {
			WL_AIRIQ(("%s: scan config chanspec is not valid .\n", __FUNCTION__));
			return BCME_UNSUPPORTED;
		}
		err = lte_u_update_scan_config(airiqh, sc);
		break;

	}
	case IOV_GVAL(IOV_LTE_U_DETECTOR_CONFIG):
		err = lte_u_get_detector_config(airiqh, (lte_u_detector_config_t*)arg, alen);
		break;
	case IOV_SVAL(IOV_LTE_U_DETECTOR_CONFIG):
	{
		lte_u_detector_config_t *dc = (lte_u_detector_config_t*)arg;
		WL_AIRIQ(("%s: detector config.\n", __FUNCTION__));

		err = lte_u_update_detector_config(airiqh, dc);
		break;
	}

	case IOV_SVAL(IOV_LTE_U_SCAN_START):
	{
		if (! airiqh->lte_u_scan_configured) {
			return BCME_ERROR;
		}
		if (SCAN_IN_PROGRESS(airiqh->wlc->scan)) {
			WL_AIRIQ(("%s: Cannot start scan: WL scan in progress\n", __FUNCTION__));
			err = BCME_BUSY;
#if defined(WLDFS) && (defined(RSDB_DFS_SCAN) || defined(BGDFS))
		} else if (wlc_dfs_scan_in_progress(airiqh->wlc->dfs)) {
			WL_AIRIQ(("%s: Cannot start scan: DFS scan in progress\n", __FUNCTION__));
			err = BCME_BUSY;
#endif /* WLDFS && (RSDB_DFS_SCAN || BGDFS) */
		} else {
			// now start the scan.
			airiqh->scan_type = SCAN_TYPE_LTE_U;
			err = wlc_airiq_start_scan(airiqh, 1, 0);
		}
		break;
	}
	case IOV_SVAL(IOV_LTE_U_SCAN_ABORT):
		WL_AIRIQ(("%s: scan abort.\n", __FUNCTION__));
		/* Decode scan settings into args list */
		err = wlc_lte_u_scan_abort(airiqh, TRUE);
		break;
	case IOV_SVAL(IOV_LTE_U_DEBUG_CAPTURE):
		airiqh->debug_capture = (bool)int_val;
		lte_u_set_debug_capture(airiqh);
		break;
	case IOV_GVAL(IOV_LTE_U_DEBUG_CAPTURE):
		*ret_int_ptr = airiqh->debug_capture;
		break;
	case IOV_SVAL(IOV_LTE_U_AGING_INTERVAL):
		airiqh->lte_u_aging_interval  = (uint32)int_val; //lte_u_aging_interval in seconds
		break;
	case IOV_GVAL(IOV_LTE_U_AGING_INTERVAL):
		*ret_int_ptr = airiqh->lte_u_aging_interval;
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}
