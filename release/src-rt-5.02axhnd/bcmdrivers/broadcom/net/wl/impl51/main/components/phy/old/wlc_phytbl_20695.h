/*
 * Radio 20695 channel tuning header file
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
 * $Id: wlc_phytbl_20695.h 650058 2016-07-20 09:50:45Z $
 */

#ifndef _wlc_phytbl_20695_h_
#define _wlc_phytbl_20695_h_
#include "phy_ac_rxgcrs.h"

#define NROWS_CHAN_TUNE_20695 (78)

typedef struct _chan_info_radio20695_rffe {
	uint8 channel;
	uint16 freq;
	uint8 vco_buf; /* This is applicable for 2G only */
	uint8 lo_gen;
	uint8 rx_lna;
	uint8 tx_mix;
	uint8 tx_pad;
} chan_info_radio20695_rffe_t;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20695_rev39[];
#endif // endif

extern radio_20xx_prefregs_t prefregs_20695_rev32[];
extern radio_20xx_prefregs_t prefregs_20695_rev33[];
extern radio_20xx_prefregs_t prefregs_20695_rev37[];
extern radio_20xx_prefregs_t prefregs_20695_rev38[];
extern radio_20xx_prefregs_t prefregs_20695_rev39[];
extern radio_20xx_prefregs_t prefregs_20695_rev40[];
extern radio_20xx_prefregs_t prefregs_20695_rev41[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev32_fcbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev32_wlbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev33)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev37)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev38_fcbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev38_wlbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev39_fcbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev39_wlbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev40_fcbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev40_wlbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev41_fcbga)[];
extern chan_info_radio20695_rffe_t BCMATTACHDATA(chan_tune_20695_rev41_wlbga)[];

extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_maj36)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_maj36)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_maj36)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_maj36)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_maj36)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_maj36)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_maj36)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_maj36)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_maj36)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_maj36)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_maj36)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_maj36)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_maj36)[2][N_BIQ01_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_2g_maj36)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_5g_maj36)[N_LNA12_GAINS];
extern int16 BCMATTACHDATA(maxgain_maj36)[ACPHY_MAX_RX_GAIN_STAGES];
extern int8 BCMATTACHDATA(fast_agc_clip_gains_maj36)[4];
extern int8 BCMATTACHDATA(fast_agc_lonf_clip_gains_maj36)[4];

extern int8 BCMATTACHDATA(ssagc_clip_gains_maj36)[SSAGC_CLIP1_GAINS_CNT];
extern bool BCMATTACHDATA(ssagc_lna1byp_maj36)[SSAGC_CLIP1_GAINS_CNT];
extern uint8 BCMATTACHDATA(ssagc_clip2_tbl_maj36)[SSAGC_CLIP2_TBL_IDX_CNT];
extern uint8 BCMATTACHDATA(ssagc_rssi_thresh_2g_maj36)[SSAGC_N_RSSI_THRESH];
extern uint8 BCMATTACHDATA(ssagc_clipcnt_thresh_2g_maj36)[SSAGC_N_CLIPCNT_THRESH];
extern uint8 BCMATTACHDATA(ssagc_rssi_thresh_5g_maj36)[SSAGC_N_RSSI_THRESH];
extern uint8 BCMATTACHDATA(ssagc_clipcnt_thresh_5g_maj36)[SSAGC_N_CLIPCNT_THRESH];

extern uint16 BCMATTACHDATA(rx2nap_seq_maj36)[16];
extern uint16 BCMATTACHDATA(rx2nap_seq_dly_maj36)[16];
extern uint16 BCMATTACHDATA(nap2rx_seq_maj36)[16];
extern uint16 BCMATTACHDATA(nap2rx_seq_dly_maj36)[16];

extern uint16 BCMATTACHDATA(nap_lo_th_adj_maj36)[5];
extern uint16 BCMATTACHDATA(nap_hi_th_adj_maj36)[5];
#endif	/* _WLC_PHYTBL_20695_H_ */
