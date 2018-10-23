/*
 * Radio 20697 channel tuning header file
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
 * $Id: wlc_phytbl_20697.h 672437 2016-11-28 12:30:46Z $
 */

#ifndef  _wlc_phytbl_20697_h_
#define  _wlc_phytbl_20697_h_
#include "phy_ac_rxgcrs.h"

#define NUM_ROWS_CHAN_TUNING_20697_5G (64)
#define NUM_ROWS_CHAN_TUNING_20697_2G (14)

#define IPA_20697_DAC_ATTN_VAL (3)
#define IPA_20697_MAX_MIX_GM_GC_VAL (4)

typedef struct _chan_info_radio20697_rffe_main {
	uint8 channel;
	uint16 freq;
	uint16 RFP0_logen_top_reg1_logen_mix_ctune;
	uint16 RFP0_logen_top_reg2_logen_core0_lc_ctune;
	uint16 RFP0_logen_top_reg2_logen_core1_lc_ctune;
	uint16 RF0_txmix5g_cfg6_mx5g_tune;
	uint16 RF0_pa5g_cfg4_pad5g_tune;
	uint16 RF0_pa5g_cfg4_pa5g_tune;
	uint16 RF0_rx5g_reg1_rx5g_lna_tune;
	uint16 RF1_txmix5g_cfg6_mx5g_tune;
	uint16 RF1_pa5g_cfg4_pad5g_tune;
	uint16 RF1_pa5g_cfg4_pa5g_tune;
	uint16 RF1_rx5g_reg1_rx5g_lna_tune;
} chan_info_radio20697_rffe_main_t;

typedef struct _chan_info_radio20697_rffe_aux_2g {
	uint8 channel;
	uint16 freq;
	uint16 RFP0_logen_lo0_reg2_logen_LO0_ctune;
	uint16 RFP0_logen_lo1_reg1_logen_LO1_ctune;
	uint16 RFP0_locore_reg2_logen_LOCORE_ctune;
	uint16 RFP0_logen_lo0_reg1_logen_LO0_OCL_tune;
	uint16 RFP0_logen_lo1_reg2_logen_LO1_OCL_tune;
	uint16 RF0_txmix2g_cfg5_mx2g_tune;
	uint16 RF0_pad2g_cfg2_pad2g_tune;
	uint16 RF0_lna2g_reg1_lna2g_lna1_freq_tune;
	uint16 RF1_txmix2g_cfg5_mx2g_tune;
	uint16 RF1_pad2g_cfg2_pad2g_tune;
	uint16 RF1_lna2g_reg1_lna2g_lna1_freq_tune;
} chan_info_radio20697_rffe_aux_2g_t;

typedef struct _chan_info_radio20697_rffe_aux_5g {
	uint8 channel;
	uint16 freq;
	uint16 RFP0_logen_lo0_reg2_logen_LO0_ctune;
	uint16 RFP0_logen_lo1_reg1_logen_LO1_ctune;
	uint16 RFP0_locore_reg2_logen_LOCORE_ctune;
	uint16 RFP0_logen_lo0_reg1_logen_LO0_OCL_tune;
	uint16 RFP0_logen_lo1_reg2_logen_LO1_OCL_tune;
	uint16 RF0_mx5g_cfg5_mx5g_tune;
	uint16 RF0_pa5g_cfg4_pad5g_tune;
	uint16 RF0_rx5g_reg1_rx5g_lna_tune;
	uint16 RF1_mx5g_cfg5_mx5g_tune;
	uint16 RF1_pa5g_cfg4_pad5g_tune;
	uint16 RF1_rx5g_reg1_rx5g_lna_tune;
} chan_info_radio20697_rffe_aux_5g_t;

typedef struct _chan_info_radio20697_rffe {
	const chan_info_radio20697_rffe_main_t *main_rffe;
	const chan_info_radio20697_rffe_aux_2g_t *aux_rffe_2g;
	const chan_info_radio20697_rffe_aux_5g_t *aux_rffe_5g;
} chan_info_radio20697_rffe_t;

extern const chan_info_radio20697_rffe_aux_5g_t
	chan_tune_20697_rev9_aux_5g[NUM_ROWS_CHAN_TUNING_20697_5G];
extern const chan_info_radio20697_rffe_aux_5g_t
	chan_tune_20697_rev7_aux_5g[NUM_ROWS_CHAN_TUNING_20697_5G];
extern const chan_info_radio20697_rffe_aux_2g_t
	chan_tune_20697_rev7_aux_2g[NUM_ROWS_CHAN_TUNING_20697_2G];
extern const chan_info_radio20697_rffe_aux_2g_t
	chan_tune_20697_rev9_aux_2g[NUM_ROWS_CHAN_TUNING_20697_2G];
extern const chan_info_radio20697_rffe_main_t
	chan_tune_20697_rev6_MAIN[NUM_ROWS_CHAN_TUNING_20697_5G];

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20697_rev6_main[];
extern const radio_20xx_dumpregs_t dumpregs_20697_rev7_aux[];
#endif // endif
/*
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev5_fcbu_e_MAIN[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev5_fcbu_e_AUX[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev8_MAIN[NUM_ROWS_CHAN_TUNING_20694];
extern const chan_info_radio20694_rffe_t
    chan_tune_20694_rev9_AUX[NUM_ROWS_CHAN_TUNING_20694];
*/
//extern const chan_info_radio20694_t chan_tune_20694_rev361[NUM_ROWS_CHAN_TUNING_20694];
//extern radio_20xx_prefregs_t prefregs_20694_rev36_wlbga[];
extern radio_20xx_prefregs_t prefregs_20697_rev6_main[];
extern radio_20xx_prefregs_t prefregs_20697_rev7_aux[];
extern radio_20xx_prefregs_t prefregs_20697_rev9_aux[];
//extern radio_20xx_prefregs_t prefregs_20694_rev4_main[];
/* extern radio_20xx_prefregs_t prefregs_20694_rev4_aux[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_2g_aux[];
extern radio_20xx_prefregs_t prefregs_20694_rev4_5g_aux[];
extern const chan_info_radio20694_rffe_t chan_tune_20694_rev36_wlbga[NUM_ROWS_CHAN_TUNING_20694];
extern radio_20xx_prefregs_t prefregs_20694_rev8_main[];
extern radio_20xx_prefregs_t prefregs_20694_rev9_aux[];
*/

/* Main slice */
extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_maj44_main)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_maj44_main)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_maj44_main)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_maj44_main)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_maj44_main)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_maj44_main)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_maj44_main)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_maj44_main)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_2g_maj44_main)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_5g_maj44_main)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_maj44_main)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_maj44_main)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_maj44_main)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_maj44)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_maj44_min0_aux)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_maj44)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_maj44_min0_aux)[2][N_BIQ01_GAINS];
/* Aux slice */
extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_maj44_aux)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_maj44_aux)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_maj44_aux)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_maj44_aux)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_maj44_aux)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_maj44_aux)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_maj44_aux)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_maj44_aux)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_2g_maj44_aux)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna2_gain_map_5g_maj44_aux)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_maj44_aux)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_maj44_aux)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_maj44_aux)[N_TIA_GAINS];

extern uint16 BCMATTACHDATA(nap_lo_th_adj_maj44)[5];
extern uint16 BCMATTACHDATA(nap_hi_th_adj_maj44)[5];
#endif /* _WLC_PHYTBL_20697_H_ */
