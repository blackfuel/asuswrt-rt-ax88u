/*
 * Radio 20694 channel tuning header file
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
 * $Id: wlc_phytbl_20694.h 691143 2017-03-21 00:21:29Z $
 */

#ifndef  _wlc_phytbl_20694_h_
#define  _wlc_phytbl_20694_h_
#include "phy_ac_rxgcrs.h"

#define NUM_ROWS_CHAN_TUNING_20694 (101)

typedef struct _chan_info_radio20694 {
	uint8 channel;
	uint8 vco_buf;
	uint8 lo_gen;
	uint8 rx_lna;
	uint8 tx_mix;
	uint8 tx_pad;
	uint8 rsvd1;
	uint8 rsvd2;
	uint8 rsvd3;
	uint8 rsvd4;
	uint8 rsvd5;
	uint8 rsvd6;
	uint8 rsvd7;
	uint8 rsvd8;
	uint16 freq;
} chan_info_radio20694_t;

typedef struct _chan_info_radio20694_rffe {
uint8 channel;
uint16 freq;
uint8 vco_buf;
uint16 RF0_logen2g_reg2_logen2g_x2_ctune_2G;
uint16 RF0_logen2g_reg2_logen2g_buf_ctune_2G;
uint16 RF0_lna2g_reg1_lna2g_lna1_freq_tune_2G;
uint16 RF0_txmix2g_cfg5_mx2g_tune_2G;
uint16 RF0_pa2g_cfg2_pad2g_tune_2G;
uint16 RF1_logen2g_reg2_logen2g_x2_ctune_2G;
uint16 RF1_logen2g_reg2_logen2g_buf_ctune_2G;
uint16 pa5g_cfg4; /* Unused. Keep for ROM compatibility. */
uint16 RF1_lna2g_reg1_lna2g_lna1_freq_tune_2G;
uint16 RF1_txmix2g_cfg5_mx2g_tune_2G;
uint16 RF1_pa2g_cfg2_pad2g_tune_2G;
uint16 RF0_logen2g_reg4_logen2g_lobuf2g2_ctune_2G_AUX;
uint16 RF0_logen5g_reg5_logen5g_x3_ctune_5G;
uint16 RF0_logen5g_reg8_logen5g_buf_ctune_5G;
uint16 RF0_logen2g_reg5_logen5g_mimobuf_ctune_5G;
uint16 RF0_rx5g_reg1_rx5g_lna_tune_5G;
uint16 RF0_txmix5g_cfg6_mx5g_tune_5G;
uint16 RF0_pa5g_cfg4_pad5g_tune_5G;
uint16 RF0_pa5g_cfg4_pa5g_tune_5G;
uint16 RF1_logen5g_reg5_logen5g_x3_ctune_5G;
uint16 RF1_logen5g_reg8_logen5g_buf_ctune_5G;
uint16 RF1_logen2g_reg5_logen5g_mimobuf_ctune_5G;
uint16 RF1_rx5g_reg1_rx5g_lna_tune_5G;
uint16 RF1_txmix5g_cfg6_mx5g_tune_5G;
uint16 RF1_pa5g_cfg4_pad5g_tune_5G;
uint16 RF1_pa5g_cfg4_pa5g_tune_5G;
uint16 RF0_logen5g_reg6_logen5g_buf_ctune_c0_5G_AUX;
uint16 RF0_logen5g_reg6_logen5g_buf_ctune_c1_5G_AUX;
} chan_info_radio20694_rffe_t;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20694_rev36[];
extern const radio_20xx_dumpregs_t dumpregs_20694_rev5[];
extern const radio_20xx_dumpregs_t dumpregs_20694_rev11[];
#endif // endif

extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev5_fcbu_e_MAIN[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev5_fcbu_e_AUX[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev8_MAIN[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev9_AUX[NUM_ROWS_CHAN_TUNING_20694];

extern const chan_info_radio20694_t chan_tune_20694_rev361[NUM_ROWS_CHAN_TUNING_20694];
extern radio_20xx_prefregs_t prefregs_20694_rev36_wlbga[];
extern radio_20xx_prefregs_t prefregs_20694_rev5_main[];
extern radio_20xx_prefregs_t prefregs_20694_rev5_aux[];
extern const chan_info_radio20694_rffe_t chan_tune_20694_rev36_wlbga[NUM_ROWS_CHAN_TUNING_20694];
extern radio_20xx_prefregs_t prefregs_20694_rev8_main[];
extern radio_20xx_prefregs_t prefregs_20694_rev9_aux[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_main[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_aux[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_2g_aux[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_5g_aux[];

extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_maj40)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_maj40)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_maj40)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_maj40)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_maj40)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_maj40)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_maj40)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_maj40)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_maj40)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_maj40)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_maj40)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_maj40)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_maj40)[2][N_BIQ01_GAINS];

extern uint16 BCMATTACHDATA(nap_lo_th_adj_maj40)[5];
extern uint16 BCMATTACHDATA(nap_hi_th_adj_maj40)[5];
#endif /* _WLC_PHYTBL_20694_H_ */
