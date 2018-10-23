/*
 * TOF module public interface (to MAC driver).
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
 * $Id: phy_tof_api.h 688501 2017-03-06 20:58:21Z $
 */
#ifndef _phy_tof_api_h_
#define _phy_tof_api_h_

#include <typedefs.h>
#include <phy_api.h>

/* forward declaration */
typedef struct phy_tof_info phy_tof_info_t;

/*  TOF info type mask */
typedef int16 wlc_phy_tof_info_type_t;

enum {
	WLC_PHY_TOF_INFO_TYPE_NONE		= 0x0000,
	WLC_PHY_TOF_INFO_TYPE_FRAME_TYPE	= 0x0001, /* TOF frame type */
	WLC_PHY_TOF_INFO_TYPE_FRAME_BW		= 0x0002, /* Frame BW */
	WLC_PHY_TOF_INFO_TYPE_CFO		= 0x0004, /* CFO */
	WLC_PHY_TOF_INFO_TYPE_RSSI		= 0x0008, /* RSSI */
	WLC_PHY_TOF_INFO_TYPE_SNR		= 0x0010, /* SNR */
	WLC_PHY_TOF_INFO_TYPE_BITFLIPS		= 0x0020, /* No of Bit flips */
	WLC_PHY_TOF_INFO_TYPE_PHYERROR		= 0x0040, /* phy errors -- 1040 */
	WLC_PHY_TOF_INFO_TYPE_ALL		= 0xffff
};

typedef struct wlc_phy_tof_info {
	wlc_phy_tof_info_type_t	info_mask;
	int			frame_type;
	int			frame_bw;
	int			cfo;
	wl_proxd_rssi_t		rssi;
	wl_proxd_snr_t		snr;
	wl_proxd_bitflips_t	bitflips;
	wl_proxd_phy_error_t tof_phy_error;
} wlc_phy_tof_info_t;

#define WL_PROXD_SEQ
#define WL_PROXD_BW_MASK	0x7
#define WL_PROXD_SEQEN		0x80
#define WL_PROXD_80M_40M	1
#define WL_PROXD_80M_20M	2
#define WL_PROXD_40M_20M	3
#define WL_PROXD_RATE_VHT	0
#define WL_PROXD_RATE_6M	1
#define WL_PROXD_RATE_LEGACY	2
#define WL_PROXD_RATE_MCS_0	3
#define WL_PROXD_RATE_MCS	4
/* #define TOF_DBG */
/* #define TOF_DBG_SEQ */
/* #define TOF_SEQ_40_IN_40MHz */
/* #define TOF_SEQ_20_IN_80MHz */
/* #define TOF_SEQ_20MHz_BW */
#define TOF_SEQ_20MHz_BW_512IFFT
/* #define TOF_SEQ_40MHz_BW */

#define TOF_DEBUG_TIME2
#define TOF_PROFILE_BUF_SIZE 15
#define TOF_P_IDX(x) (x < TOF_PROFILE_BUF_SIZE) ? x : 0

#define TOF_CLASSIFIER_MASK			0x7	/* last 3 bits of the register */
#define TOF_CLASSIFIER_BPHY_OFF_OFDM_ON		6
#define TOF_CLASSIFIER_BPHY_ON_OFDM_ON		7

typedef struct profile_buf
{
	int8 event;
	int8 token;
	int8 follow_token;
	uint32 ts;
} tof_pbuf_t;

typedef struct wlc_phy_tof_secure_2_0 {
	uint16 start_seq_time;
	uint16 delta_time_tx2rx;
} wlc_phy_tof_secure_2_0_t;

int phy_tof_seq_upd_dly(wlc_phy_t *ppi, bool tx, uint8 core, bool mac_suspend);
int phy_tof_seq_params(wlc_phy_t *ppi, bool assign_buffer);
int phy_tof_set_ri_rr(wlc_phy_t *ppi, const uint8* ri_rr, const uint16 len,
	const uint8 core, const bool isInitiator, const bool isSecure,
	wlc_phy_tof_secure_2_0_t  tof_sec_params);
int phy_tof_seq_params_get_set(wlc_phy_t *ppi, uint8 *delays, bool set, bool tx,
	int size);
int phy_tof_dbg(wlc_phy_t *ppi, int arg); /* DEBUG */
void phy_tof_setup_ack_core(wlc_phy_t *ppi, int core);
uint8 phy_tof_num_cores(wlc_phy_t *ppi);
void phy_tof_core_select(wlc_phy_t *ppi, const uint32 gdv_th, const int32 gdmm_th,
		const int8 rssi_th, const int8 delta_rssi_th, uint8* core,
		uint8 core_mask);
int wlc_phy_tof_calc_snr_bitflips(wlc_phy_t *ppi, void *In,
	wl_proxd_bitflips_t *bit_flips, wl_proxd_snr_t *snr);
int phy_tof_chan_freq_response(wlc_phy_t *ppi,
	int len, int nbits, int32* Hr, int32* Hi, uint32* Hraw, const bool single_core,
	uint8 num_sts, bool collect_offset);
void phy_tof_cmd(wlc_phy_t *ppi, bool seq, int emu_delay);

#ifdef WL_PROXD_SEQ
int wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool tx, bool hw_adj, bool seq_en,
	int core, int emu_delay);
int wlc_phy_tof_info(wlc_phy_t *ppi, wlc_phy_tof_info_t *tof_info,
	wlc_phy_tof_info_type_t tof_info_mask, uint8 core);
int wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip,
	uint32 *ktp, uint8 seq_en);
int wlc_phy_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 rspecidx, uint32 *kip,
	uint32 *ktp, uint8 seq_en);
#else
int wlc_phy_tof(wlc_phy_t *ppi, bool enter, bool hw_adj);
int wlc_phy_tof_info(wlc_phy_t *ppi, int* p_frame_type, int* p_frame_bw, int8* p_rssi);
int wlc_phy_tof_kvalue(wlc_phy_t *ppi, chanspec_t chanspec, uint32 *kip, uint32 *ktp);
#endif /* WL_PROXD_SEQ */

void phy_tof_init_gdmm_th(wlc_phy_t *ppi, int32 *gdmm_th);
void phy_tof_init_gdv_th(wlc_phy_t *ppi, uint32 *gdv_th);

int wlc_phy_chan_mag_sqr_impulse_response(wlc_phy_t *ppi, int frame_type,
	int len, int offset, int nbits, int32* h, int* pgd, uint32* hraw, uint16 tof_shm_ptr);
int wlc_phy_seq_ts(wlc_phy_t *ppi, int n, void* p_buffer, int tx, int cfo, int adj,
	void* pparams, int32* p_ts, int32* p_seq_len, uint32* p_raw, uint8* ri_rr,
	const uint8 smooth_win_en);

#endif /* _phy_tof_api_h_ */
