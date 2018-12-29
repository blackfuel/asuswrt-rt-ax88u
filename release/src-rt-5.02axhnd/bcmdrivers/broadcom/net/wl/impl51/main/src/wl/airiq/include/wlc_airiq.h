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

#ifndef _wlc_airiq_h_
#define _wlc_airiq_h_

#include <wlc_cfg.h>
#include <typedefs.h>
#include <wlc_types.h>
#include <bcmdefs.h>
#include <osl.h>
#include <wl_dbg.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_modesw.h>
#include <wlc.h>
#include <wlc_phy_int.h>
#include <wlc_bsscfg.h>
#include <wlc_vasip.h>
#include <wlioctl_airiq.h>

#ifdef __AIRIQ_DBG
#define SCAN_DBG(...) printk(__VA_ARGS__)
#else
#define SCAN_DBG(...)
#endif // endif

#define AIRIQ_ENTER()    AIRIQ_DBG(("-->%s\n", __FUNCTION__))
#define AIRIQ_EXIT()     AIRIQ_DBG(("<--%s\n", __FUNCTION__))
#define AIRIQ_EXIT_ERR() AIRIQ_DBG(("<--%s ERROR\n", __FUNCTION__))

/* Backplane address */
#define SVMP_BACKPLANE_BASE 0x18400000
/* Offsets into SVMP */
#define SVMP_FFT_DATA_ADDR 0x26820

#define SVMP_STATUS_ADDR        0x26818

#define SVMP_SMPL_CTRL_ADDR     (1 + SVMP_STATUS_ADDR)
#define SVMP_SMPL_ENABLE_ADDR   (2 + SVMP_STATUS_ADDR)
#define SVMP_SMPL_ENABLE_OFFLOAD 0
#define SVMP_SMPL_ENABLE_NON_OFFLOAD 1
#define SVMP_SMPL_ENABLE_OFFLOAD_FLAG (1 << SVMP_SMPL_ENABLE_OFFLOAD)
#define SVMP_SMPL_ENABLE_NON_OFFLOAD_FLAG (1 << SVMP_SMPL_ENABLE_NON_OFFLOAD)
#define SVMP_SMPL_INTERVAL_ADDR (3 + SVMP_STATUS_ADDR)

#define SVMP_HEADER_ADDR        0x26800
#define SVMP_SMPL_SIZE_ADDR     (SVMP_HEADER_ADDR)
#define SVMP_SMPL_MODE_ADDR     (1 + SVMP_HEADER_ADDR)
#define SVMP_SMPL_CORE_ADDR     (2 + SVMP_HEADER_ADDR)
#define SVMP_SMPL_GAIN_ADDR     (3 + SVMP_HEADER_ADDR)

#define SVMP_HEADER_EXT_ADDR    0x26810
#define SVMP_SMPL_TIMESTAMP_ADDR_L   (2 + SVMP_HEADER_EXT_ADDR) // set by driver
#define SVMP_SMPL_TIMESTAMP_ADDR_H   (3 + SVMP_HEADER_EXT_ADDR) // set by driver
#define SVMP_SMPL_SEQ_NUM_ADDR       (4 + SVMP_HEADER_EXT_ADDR) // set by driver
// chanspec of +1 chain, set by driver
#define SVMP_SMPL_CHANSPEC_ADDR      (5 + SVMP_HEADER_EXT_ADDR)
// chanspec of 3x3 chain, set by driver
#define SVMP_SMPL_CHANSPEC_3X3_ADDR  (6 + SVMP_HEADER_EXT_ADDR)

/* End of offsets into SVMP */
/* Offsets into SVMP for AX chip */
#define SVMP_AX_FFT_DATA_ADDR 0x45020

#define SVMP_AX_STATUS_ADDR        0x45018

#define SVMP_AX_SMPL_CTRL_ADDR     (1 + SVMP_AX_STATUS_ADDR)
#define SVMP_AX_SMPL_ENABLE_ADDR   (2 + SVMP_AX_STATUS_ADDR)
#define SVMP_AX_SMPL_INTERVAL_ADDR (3 + SVMP_AX_STATUS_ADDR)

#define SVMP_AX_HEADER_ADDR        0x45000
#define SVMP_AX_SMPL_SIZE_ADDR     (SVMP_AX_HEADER_ADDR)
#define SVMP_AX_SMPL_MODE_ADDR     (1 + SVMP_AX_HEADER_ADDR)
#define SVMP_AX_SMPL_CORE_ADDR     (2 + SVMP_AX_HEADER_ADDR)
#define SVMP_AX_SMPL_GAIN_ADDR     (3 + SVMP_AX_HEADER_ADDR)

#define SVMP_AX_HEADER_EXT_ADDR    0x45010
#define SVMP_AX_SMPL_TIMESTAMP_ADDR_L   (2 + SVMP_AX_HEADER_EXT_ADDR) // set by driver
#define SVMP_AX_SMPL_TIMESTAMP_ADDR_H   (3 + SVMP_AX_HEADER_EXT_ADDR) // set by driver
#define SVMP_AX_SMPL_SEQ_NUM_ADDR       (4 + SVMP_AX_HEADER_EXT_ADDR) // set by driver
// chanspec of +1 chain, set by driver
#define SVMP_AX_SMPL_CHANSPEC_ADDR      (5 + SVMP_AX_HEADER_EXT_ADDR)
// chanspec of 3x3 chain, set by driver
#define SVMP_AX_SMPL_CHANSPEC_3X3_ADDR  (6 + SVMP_AX_HEADER_EXT_ADDR)

/* End of offsets into SVMP for AX chip */

#define VASIP_FFT_SIZE 1024
#define VASIP_FFT_HEADER_SIZE 64
#define VASIP_FFT_HEADER_MAGIC_NUMBER_OFFSET 32
#define VASIP_FFT_HEADER_MAGIC_NUM 0xfedcfcde
/* Need a buffer just big enough to include:
 * - the header read from VASIP,
 * - the header prepended by driver
 * - the fft data itself
 * - make length an even 64-bit multiple, add 2x uint64 (for
 *   trivial reading in multiples of uint64 with no overrun)
 *   i.e. read (bytes>>3)+1
 * - Since we use uint64's to read to/from, but alignment on
 *   64-bit boundary is not guaranteed, we offset into this
 *       to the first 64-bit boundary.
 */

#define VASIP_HW_SUPPORT 1

#define FFT_BUF_SIZE ((VASIP_FFT_SIZE + VASIP_FFT_HEADER_SIZE +	\
		       sizeof(airiq_fftdata_header_t) + 2 * sizeof(uint64)) & (~7))

//#define PHYMODE(w) (((phy_info_t*)WLC_PI(w))->cmni->phymode)

#define IS_43465MC2(devid) (devid == BCM4366_D11AC2G_ID)
#define IS_43465MC5(devid) (devid == BCM4366_D11AC5G_ID)
#define IS_43465MC(devid)  (devid == BCM4366_D11AC_ID)
#define IS_43525MC2(devid) (devid == BCM4365_D11AC2G_ID)
#define IS_43525MC5(devid) (devid == BCM4365_D11AC5G_ID)
#define IS_43525MC(devid)  (devid == BCM4365_D11AC_ID)

#define IS_43694MC2(devid) (devid == BCM43684_D11AX2G_ID)
#define IS_43694MC(devid)  (devid == BCM43684_D11AX_ID)

#define IS_WAVE2_2G(devid) (IS_43465MC2(devid) || IS_43525MC2(devid))
#define IS_WAVE2_5G(devid) (IS_43465MC5(devid) || IS_43525MC5(devid))
#define IS_WAVE2_DB(devid) (IS_43465MC(devid) || IS_43525MC(devid))

#ifdef __AIRIQ_DEBUG /* not normally defined */
#ifdef WL_AIRIQ
#undef WL_AIRIQ
#define WL_AIRIQ WL_PRINT
#endif // endif
#endif /* __AIRIQ_DEBUG */

#define AIRIQ_SCAN_P1C 4

enum {
	SCAN_TYPE_AIRIQ = 0x1,
	SCAN_TYPE_LTE_U = 0x2
};

struct _airiq_scan_config {
	int channel_idx; /* Index into the current scan */
	int channel_cnt; /* Number of channels in the current scan */
	int home_scan;
	bool scan_start;
	bool run_phycal;
	chanspec_t chanspec_list[MAXCHANNEL]; /* array of chanspec_t's */
	uint16 fft_interval_scan_us;
	uint16 fft_interval_home_us;
	uint32 capture_count_home;
	uint32 capture_count_scan;
	uint16 dwell_interval_ms[MAXCHANNEL];
	uint16 capture_interval_us[MAXCHANNEL];
	uint32 capture_count[MAXCHANNEL];
	uint8 core_config[MAXCHANNEL]; /* which core to use */
	uint32 timestamp_us; /* timestamp of the scan */
	bool bandlocked_save;
	/* array of chanspec_t's specified by user in IOVAR */
	chanspec_t user_chanspec_list[MAXCHANNEL];
	/* Number of channels in the current scan specified by user in IOVAR */
	int user_channel_cnt;
	/* scan status for each user scan channel */
	lte_u_scan_status_t lte_scan_status[MAXCHANNEL];
};

typedef struct _airiq_scan_config airiq_scan_config_t;

struct lte_u_detector_info {
	/* LTE_U detector parametrs */
	uint16 nshift;
	uint16 edlow;
	uint16 edhigh;
	uint16 edshift;
	uint16 dtlow;
	uint16 dthigh;
	uint16 dtabslow;
	uint16 dtabshigh;
	uint16 nshiftabsqlow;
	uint16 nshiftabsqhigh;
	bool  detector_configured;
};

typedef struct lte_u_detector_info lte_u_detector_info_t;

#define AIRIQ_HISTOGRAM_BINCNT 16

typedef struct {
	uint32 bin[AIRIQ_HISTOGRAM_BINCNT];
	uint32 sum;
	uint32 avg;
} airiq_fft_latency_histogram;

/* Per-WLAN interface AIRIQ private info structure */
struct airiq_info {

	/* pointer to wlc public info struct */
	bool scan_enable;
	struct wl_timer *timer;
	wlc_info_t *wlc;
	bool sa_enabled;

	uint32 fft_count;
	uint32 capture_limit;
	int32 sweep_count;

	airiq_scan_config_t scan;

	bool iq_capture_enable;
	uint32 chan_tsf;
	airiq_gain_t gain;
	airiq_gain_table_t gaintbl_24ghz;
	airiq_gain_table_t gaintbl_5ghz;

	int scan_cpu;
	uint32 measure_cpu;
	uint32 idle_thr;
	uint32 sirq_thr;
	uint32 fft_reduction;
	uint32 scan_scale;
	uint32 cts2self;
	uint32 scanmute;
	uint32 core;
	uint16 phy_mode;
	uint16 bw_3plus1;
#define AIRIQ_DESENS_CNT 16
	uint32 desens_lut_24ghz[AIRIQ_DESENS_CNT]; /* stores gain codes, steps of 2 db */
	int16 gain_lut_24ghz[AIRIQ_DESENS_CNT]; /* stores gain */
	uint32 desens_lut_5ghz[AIRIQ_DESENS_CNT]; /* stores gain codes, steps of 2 db */
	int16 gain_lut_5ghz[AIRIQ_DESENS_CNT]; /* stores gain */

	bool modesw_cb_regd;
	bool updown_cb_regd;
	bool upgrade_pending;
	uint8 modeswitch_state;

	uint32 mangain;
	rxgain_ovrd_t rxgain_ovrd[PHY_CORE_MAX];
	uint16 femctrl_ovrd;

#ifdef VASIP_HW_SUPPORT
	/* Buffer used for FFT data - not assumed to be aligned */
	uint8 __fft_buffer[FFT_BUF_SIZE];
	/* 64-bit aligned fft buffer pointer */
	uint8 *fft_buffer;
#endif /* VASIP_HW_SUPPORT */

	uint32 fft_log_count[MAXCHANNEL];
	airiq_fft_latency_histogram fft_latency[MAXCHANNEL];

	bool tx_suspending;
	uint32 scan_complete[SCAN_COMPLETE_RADIOS]; /* alternate scan complete indication */

	int  scan_type;
	bool  lte_u_scan_configured;
	lte_u_detector_info_t detector_config;
	int user_channel_mapping[MAX_CHANSPECS];
	bool lte_interrupt_received;
	uint8 *iqbuf;
	uint32 iqbuf_size;
	uint32 lte_u_aging_interval;
	bool debug_capture;
	uint32 pkt_up_counter;
	/* vasip fft timing */
	int32 start_time_mac;
	int32 vasip_time_correction;
	bool latch_vasip_start_time;
	/* vasip addresses */
	uint32 svmp_fft_data_addr;
	uint32 svmp_status_addr;
	uint32 svmp_smpl_ctrl_addr;
	uint32 svmp_smpl_enable_addr;
	uint32 svmp_smpl_interval_addr;
	uint32 svmp_header_addr;
	uint32 svmp_smpl_size_addr;
	uint32 svmp_smpl_mode_addr;
	uint32 svmp_smpl_core_addr;
	uint32 svmp_smpl_gain_addr;
	uint32 svmp_header_ext_addr;
	uint32 svmp_smpl_timestamp_addr_l;
	uint32 svmp_smpl_timestamp_addr_h;
	uint32 svmp_smpl_seq_num_addr;
	uint32 svmp_smpl_chanspec_addr;
	uint32 svmp_smpl_chanspec_3x3_addr;
};

/*
 * Initialize airiq private context.It returns a pointer to the
 * airiq private context if succeeded. Otherwise it returns NULL.
 */
airiq_info_t *wlc_airiq_attach(wlc_info_t * wlc);

/* Cleanup airiq private context */
extern void wlc_airiq_detach(airiq_info_t * airiqh);

/* wlc_airiq_capture(): Capture data for airiq analysis on this channel. */
extern int wlc_airiq_capture(airiq_info_t * airiqh, airiq_scan_config_t * scan);

/*
 * might be useful one day... tbd
 * Process frames in transmit direction
 */
extern int wlc_airiq_send_proc(airiq_info_t * airiqh, void *sdu, void **new);
/*
 * might be useful one day... tbd
 * Process frames in receive direction
 */
extern int wlc_airiq_recv_proc(airiq_info_t * airiqh, void *sdu);

int wlc_airiq_start_scan(airiq_info_t * airiqh, int sweep_cnt, int home);
int wlc_airiq_start_scan_extern(airiq_info_t * airiqh);
int wlc_airiq_scan_abort(airiq_info_t * airiqh, bool upgrade);

int wlc_airiq_update(airiq_info_t * airiqh,
	int dwell_time_ms, int scan_mode, int spectrum_analysis);

#ifdef BCMDBG
extern int wlc_airiq_dump(airiq_info_t * airiqh, struct bcmstrbuf *b);
#endif /* BCMDBG */

void wlc_airiq_fftcapture(airiq_info_t * airiqh, uint8 * fftdata, int32 len);
void wlc_airiq_start_fftcapture(airiq_info_t * airiqh);
void wlc_airiq_setup_fftcapture(airiq_info_t * airiqh);
#ifdef VASIP_HW_SUPPORT
void wlc_airiq_vasipfftcapture(airiq_info_t * airiqh);
#endif /* VASIP_HW_SUPPORT */
bool wlc_airiq_cal_in_progress(wlc_info_t * wlc);

bool wlc_airiq_scan_in_progress(wlc_info_t * wlc);
int wlc_airiq_seconds_since_last_scan(wlc_info_t * wlc);

void wlc_airiq_fifo_suspend_complete(airiq_info_t * airiqh);
bool wlc_airiq_phymode_3p1(wlc_info_t * wlc);

/* ----------------------------------------------------------------------- */
/* airiq_phy */
/* ----------------------------------------------------------------------- */
void wlc_airiq_phy_init(airiq_info_t * airiqh);
void wlc_airiq_phy_init_overrides(airiq_info_t * airiqh);
void wlc_airiq_phy_dump_gain(airiq_info_t * airiqh, struct bcmstrbuf *b);

uint32 wlc_airiq_phy_get_gaincode(uint32 gaincode_lut[],
		int16 gain_lut[],
		uint16 desens, int16 * gain);
int wlc_airiq_phy_get_gain(airiq_info_t * airiqh,
		phy_info_t * pi, airiq_gain_t * g);

int airiq_get_gain(airiq_info_t * airiqh, phy_info_t * pi, airiq_gain_t * g);
int airiq_set_gain(airiq_info_t * airiqh, airiq_gain_t * g);

void wlc_airiq_phy_enable_fft_capture(airiq_info_t * airiqh,
		uint16 fft_interval,
		uint16 desens,
		chanspec_t chanspec, uint16 fft_count);
void wlc_airiq_phy_disable_fft_capture(airiq_info_t * airiqh);
void wlc_airiq_phy_ack_vasip_fft(airiq_info_t * airiqh);
void wlc_airiq_phy_clear_svmp_status(airiq_info_t * airiqh);
/* ----------------------------------------------------------------------- */
/* airiq_capture */
/* ----------------------------------------------------------------------- */
void wlc_airiq_vasip_fft_dpc(wlc_info_t * wlc);

/* ----------------------------------------------------------------------- */
/* airiq_3p1 */
/* ----------------------------------------------------------------------- */
#ifdef VASIP_HW_SUPPORT
void wlc_airiq_scantimer_phy_mode3p1(void *arg);
void wlc_airiq_3p1_downgrade_phy(airiq_info_t * airiqh,
		chanspec_t scan_chanspec);
void wlc_airiq_3p1_upgrade_phy(airiq_info_t * airiqh);
#ifdef WL_MODESW

#define AIRIQ_MODESW_IDLE                   0 /* mode switch for airiq is idle. */
#define AIRIQ_MODESW_DOWNGRADE_IN_PROGRESS  1 /* waiting for downgrade. */
#define AIRIQ_MODESW_DOWNGRADE_FINISHED     2 /* Downgrade is done. Scan in progress. */
#define AIRIQ_MODESW_UPGRADE_IN_PROGRESS    3 /* Upgrade requested and pending state. */
#define AIRIQ_MODESW_PHY_UPGRADED           4 /* PHY Upgraded and pending final AP upgrade. */
#define AIRIQ_MODESW_STATE_CNT              5

int wlc_airiq_3p1_upgrade_wlc(wlc_info_t * wlc);
int wlc_airiq_3p1_downgrade_wlc(wlc_info_t * wlc);
void wlc_airiq_modeswitch_state_upd(airiq_info_t * airiqh, uint new_state);
void wlc_airiq_opmode_change_cb(void *ctx,
		wlc_modesw_notif_cb_data_t * notif_data);
bool wlc_airiq_phymode_3p1(wlc_info_t * wlc);
#endif /* WL_MODESW */
#endif /* VASIP_HW_SUPPORT */

/* ----------------------------------------------------------------------- */
/* airiq general */
/* ----------------------------------------------------------------------- */
void wlc_airiq_set_scan_in_progress(airiq_info_t * airiqh, bool in_progress);
void wlc_airiq_log_fft(airiq_info_t * airiqh, uint16 channel, uint16 latency);
int airiq_doiovar(void *hdl,  uint32 actionid,
		void *p, uint plen, void *arg, uint alen,
		uint vsize, struct wlc_if *wlcif);
void wl_airiq_sendup_scan_complete_alternate(airiq_info_t * airiqh,
		uint16 status);
int wl_airiq_sendup_data(airiq_info_t * airiqh, uint8 * data,
		unsigned long size);

/* ----------------------------------------------------------------------- */
/* airiq scan */
/* ----------------------------------------------------------------------- */
void wlc_airiq_scan_set_chanspec_3p1(airiq_info_t * airiqh, phy_info_t * pi,
		chanspec_t home_chanspec,
		chanspec_t scan_chanspec);
int airiq_get_scan_config(airiq_info_t * airiqh, airiq_config_t * sc, int size);
int airiq_update_scan_config(airiq_info_t * airiqh, airiq_config_t * sc);
bool chanspec_list_valid(airiq_info_t * airiqh, airiq_config_t * sc);
int wlc_airiq_3p1_scan_prep(airiq_info_t * airiqh);
int wlc_airiq_start_scan_phase2(airiq_info_t * airiqh);
void wlc_airiq_scantimer(void *arg);
void wlc_airiq_default_scan_channels(airiq_info_t * airiqh);

void wl_airiq_sendup_scan_complete_alternate(airiq_info_t * airiqh,
		uint16 status);
int wl_airiq_sendup_data(airiq_info_t * airiqh, uint8 * data,
		unsigned long size);

/* ----------------------------------------------------------------------- */
/* LTE-U Detector */
/* ----------------------------------------------------------------------- */
int lte_u_get_scan_config(airiq_info_t *airiqh, airiq_config_t *sc, int size);
bool  lte_u_chanspec_list_valid(airiq_info_t *airiqh, airiq_config_t *sc);
int lte_u_get_detector_config(airiq_info_t *airiqh, lte_u_detector_config_t *dc, int size);
int  lte_u_update_detector_config(airiq_info_t *airiqh, lte_u_detector_config_t *dc);
int lte_u_update_scan_config(airiq_info_t *airiqh, airiq_config_t *sc);
void  wlc_lte_u_phy_disable_iq_capture_shm(airiq_info_t *airiqh);
void wlc_lte_u_phy_enable_iq_capture_shm(airiq_info_t* airiqh, uint16 sample_interval,
                             uint16 desens, chanspec_t chanspec, uint16 capture_count);
int wlc_lte_u_scan_abort(airiq_info_t *airiqh, bool upgrade);
bool wl_lte_u_send_scan_abort_event(airiq_info_t *airiqh, int reason);
int wlc_lte_u_get_user_channel_index_from_scanchan(airiq_info_t *airiqh, int scan_channel);
void wlc_lte_u_send_status(airiq_info_t *airiqh);
void wlc_lte_u_detection_status(airiq_info_t *airiqh);
void wlc_lte_u_clear_svmp_status(airiq_info_t *airiqh);
void lte_u_set_debug_capture(airiq_info_t *airiqh);
bool lte_u_debug_capture_status(airiq_info_t* airiqh);
bool wlc_lte_u_create_iqbuf(airiq_info_t *airiqh);
void wlc_lte_u_free_iqbuf(airiq_info_t *airiqh);

/* Offsets into SVMP */
#define SVMP_IQ_DATA_ADDR 0x18000
#define SVMP_LTE_U_INFO_ADDR                0x27A00
#define SVMP_LTE_U_INFO_SIZE_ADDR           (SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_MODE_ADDR           (1+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_CORE_ADDR           (2+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_GAIN_ADDR           (3+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_TIMESTAMP_ADDR_L    (0x12+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_TIMESTAMP_ADDR_H    (0x13+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_SEQ_NUM_ADDR        (0x14+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_CHANSPEC_ADDR       (0x15+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_CHANSPEC_3X3_ADDR   (0x16+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_SYNC_ADDR           (0x18+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_CTRL_ADDR           (0x19+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_ENABLE_ADDR         (0x1a+SVMP_LTE_U_INFO_ADDR)
#define SVMP_LTE_U_INFO_INTERVAL_ADDR       (0x1b+SVMP_LTE_U_INFO_ADDR)

#define SVMP_LTE_U_CONFIG_ADDR             0x27A20
#define SVMP_LTE_U_CONFIG_NSHIFT_ADDR      (SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_EDLOW_ADDR       (1+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_EDHIGH_ADDR      (2+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_EDSHIFT_ADDR     (3+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_DTLOW_ADDR       (4+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_DTHIGH_ADDR      (5+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_DTABSLOW_ADDR    (6+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_DTABSHIGH_ADDR   (7+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_NSHIFTABSQLOW_ADDR  (8+SVMP_LTE_U_CONFIG_ADDR)
#define SVMP_LTE_U_CONFIG_NSHIFTABSQHIGH_ADDR (9+SVMP_LTE_U_CONFIG_ADDR)

#define SVMP_LTE_U_STATUS_ADDR                      0x296c0
#define SVMP_LTE_U_STATUS_RSSI_ADDR                 (1+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT25_PERDET_TIMESTAMP_H   (11+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT25_PERDET_TIMESTAMP_L   (12+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT29_PERDET_TIMESTAMP_H   (13+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT29_PERDET_TIMESTAMP_L   (14+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT34_PERDET_TIMESTAMP_H   (15+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT34_PERDET_TIMESTAMP_L   (0x10+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT25_THR_COUNT_ADDR       (0x11+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT29_THR_COUNT_ADDR       (0x12+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT34_THR_COUNT_ADDR       (0x13+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT25_HIT_COUNT_ADDR       (0x14+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT29_HIT_COUNT_ADDR       (0x15+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RT34_HIT_COUNT_ADDR       (0x16+SVMP_LTE_U_STATUS_ADDR)
#define SVMP_LTE_U_STATUS_RSSI_SMOOTHED_ADDR        (0x20+SVMP_LTE_U_STATUS_ADDR)

#define SVMP_LTE_U_SCRATCH_DEC_ADDR 0x285A0
#define VASIP_IQ_SIZE 256*4
#define VASIP_IQ_HEADER_SIZE 64
#define IQMSG_SIZE (VASIP_IQ_SIZE + VASIP_IQ_HEADER_SIZE)
#define VASIP_IQ_HEADER_MAGIC_NUM 0xfedcfcde

#define SVMP_LTE_U_DEBUG_ADDR 0x295A0
#define SVMP_LTE_U_DEBUG_CAPTURE    (2+SVMP_LTE_U_DEBUG_ADDR)

#define LTE_U_GAINCODE_GAIN(gaincode)	((gaincode)&0xff)
#define LTE_U_GAINCODE_DESENS(gaincode)	(0xff & ((gaincode)>>8))
#define LTE_U_GAINCODE(gain, desens)	(((gain) & 0xff) | ((desens) << 8))

#endif /* _wlc_airiq_h_ */
