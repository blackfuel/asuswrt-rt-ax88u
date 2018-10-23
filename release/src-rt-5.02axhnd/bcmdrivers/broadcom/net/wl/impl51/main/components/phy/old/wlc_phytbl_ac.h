/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THIS IS A GENERATED FILE - DO NOT EDIT
 * Generated on Tue Aug 16 16:22:54 PDT 2011
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
 * All Rights Reserved.
 *
 * $Id: wlc_phytbl_ac.h 742511 2018-01-22 14:14:24Z $
 */
/* FILE-CSTYLED */
#ifndef _wlc_phytbl_ac_h_
#define _wlc_phytbl_ac_h_

#include <phy_ac_info.h>

#define NUM_ROWS_CHAN_TUNING 77
#define NUM_ALTCLKPLN_CHANS 5
#define TXGAIN_TABLES_LEN 384

extern CONST acphytbl_info_t acphytbl_info_rev0[];
extern CONST uint32 acphytbl_info_sz_rev0;
extern CONST acphytbl_info_t acphytbl_info_rev2[];
extern CONST uint32 acphytbl_info_sz_rev2;
extern CONST acphytbl_info_t acphytbl_info_rev6[];
extern CONST uint32 acphytbl_info_sz_rev6;
extern CONST acphytbl_info_t acphytbl_info_rev3[];
extern CONST uint32 acphytbl_info_sz_rev3;
extern CONST acphytbl_info_t acphytbl_info_rev9[];
extern CONST uint32 acphytbl_info_sz_rev9;
extern CONST acphytbl_info_t acphytbl_info_rev12[];
extern CONST uint32 acphytbl_info_sz_rev12;
extern CONST acphytbl_info_t acphytbl_info_rev32[];
extern CONST uint32 acphytbl_info_sz_rev32;
extern CONST acphytbl_info_t acphytbl_info_rev33[];
extern CONST uint32 acphytbl_info_sz_rev33;
extern CONST acphytbl_info_t acphytbl_info_rev36[];
extern CONST uint32 acphytbl_info_sz_rev36;
extern CONST acphytbl_info_t acphytbl_info_rev40[];
extern CONST uint32 acphytbl_info_sz_rev40;
extern CONST acphytbl_info_t acphytbl_info_maxbw20_rev40[];
extern CONST uint32 acphytbl_info_sz_maxbw20_rev40;
extern CONST acphytbl_info_t acphytbl_info_rev44[];
extern CONST uint32 acphytbl_info_sz_rev44;
extern CONST acphytbl_info_t acphytbl_info_maxbw20_rev44[];
extern CONST uint32 acphytbl_info_sz_maxbw20_rev44;
extern CONST acphytbl_info_t acphytbl_info_rev47[];
extern CONST uint32 acphytbl_info_sz_rev47;
extern CONST acphytbl_info_t acphytbl_info_rev47_1[];
extern CONST uint32 acphytbl_info_sz_rev47_1;
extern CONST acphytbl_info_t acphytbl_info_rev51[];
extern CONST uint32 acphytbl_info_sz_rev51;

extern CONST acphytbl_info_t acphyzerotbl_info_rev0[];
extern CONST uint32 acphyzerotbl_info_cnt_rev0;
extern CONST acphytbl_info_t acphyzerotbl_info_rev2[];
extern CONST uint32 acphyzerotbl_info_cnt_rev2;
extern CONST acphytbl_info_t acphyzerotbl_info_rev6[];
extern CONST uint32 acphyzerotbl_info_cnt_rev6;
extern CONST acphytbl_info_t acphyzerotbl_info_rev3[];
extern CONST uint32 acphyzerotbl_info_cnt_rev3;
extern CONST acphytbl_info_t acphyzerotbl_info_rev9[];
extern CONST uint32 acphyzerotbl_info_cnt_rev9;
extern CONST acphytbl_info_t acphyzerotbl_info_rev12[];
extern CONST uint32 acphyzerotbl_info_cnt_rev12;
extern CONST acphytbl_info_t acphyzerotbl_delta_MIMO_80P80_info_rev12[];
extern CONST uint32 acphyzerotbl_delta_MIMO_80P80_info_cnt_rev12;
extern CONST acphytbl_info_t acphyzerotbl_info_rev32[];
extern CONST uint32 acphyzerotbl_info_cnt_rev32;
extern CONST acphytbl_info_t acphyzerotbl_info_rev36[];
extern CONST uint32 acphyzerotbl_info_cnt_rev36;
extern CONST acphytbl_info_t acphyzerotbl_info_rev40[];
extern CONST uint32 acphyzerotbl_info_cnt_rev40;
extern CONST acphytbl_info_t acphyzerotbl_info_maxbw20_rev40[];
extern CONST uint32 acphyzerotbl_info_cnt_maxbw20_rev40;
extern CONST acphytbl_info_t acphyzerotbl_info_rev44[];
extern CONST uint32 acphyzerotbl_info_cnt_rev44;
extern CONST acphytbl_info_t acphyzerotbl_info_maxbw20_rev44[];
extern CONST uint32 acphyzerotbl_info_cnt_maxbw20_rev44;
extern CONST acphytbl_info_t acphyzerotbl_info_rev47[];
extern CONST uint32 acphyzerotbl_info_cnt_rev47;
extern CONST acphytbl_info_t acphyzerotbl_info_rev47_1[];
extern CONST uint32 acphyzerotbl_info_cnt_rev47_1;
extern CONST acphytbl_info_t acphyzerotbl_info_rev51[];
extern CONST uint32 acphyzerotbl_info_cnt_rev51;

extern CONST chan_info_radio2069_t chan_tuning_2069rev3[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069_t chan_tuning_2069rev4[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069_t chan_tuning_2069rev7[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069_t chan_tuning_2069rev64[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069_t chan_tuning_2069rev66[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_16_17[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_16_17_40[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_18[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_18_40[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_GE16_lp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_23_2Glp_5Gnonlp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_GE16_2Glp_5Gnonlp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_GE16_40_lp[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev33_37[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev33_37_40[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev36[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev36_40[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev39[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE32_t) chan_tuning_2069_rev39_40[NUM_ROWS_CHAN_TUNING];

extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_GE_18_40MHz_lp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE16_t chan_tuning_2069rev_GE_18_lp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE25_t chan_tuning_2069rev_GE_25[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE25_t chan_tuning_2069rev_GE_25_40MHz_lp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE25_t chan_tuning_2069rev_GE_25_lp[NUM_ROWS_CHAN_TUNING];
extern CONST chan_info_radio2069revGE25_t chan_tuning_2069rev_GE_25_40MHz[NUM_ROWS_CHAN_TUNING];
extern CONST BCMATTACHDATA(chan_info_radio2069revGE25_52MHz_t) chan_tuning_2069rev_GE_25_52MHz[NUM_ROWS_CHAN_TUNING];

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern CONST radio_20xx_dumpregs_t dumpregs_20693_rsdb[];
extern CONST radio_20xx_dumpregs_t dumpregs_20693_mimo[];
extern CONST radio_20xx_dumpregs_t dumpregs_20693_80p80[];
extern CONST radio_20xx_dumpregs_t dumpregs_20693_rev32[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev0[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev16[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev17[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev25[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev32[];
extern CONST radio_20xx_dumpregs_t dumpregs_2069_rev64[];
#endif // endif
extern CONST radio_20xx_prefregs_t prefregs_2069_rev3[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev4[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev16[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev17[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev18[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev23[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev24[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev25[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev26[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev33_37[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev36[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev39[];
extern CONST radio_20xx_prefregs_t prefregs_2069_rev64[];
extern CONST uint16 ovr_regs_2069_rev2[];
extern CONST uint16 ovr_regs_2069_rev16[];
extern CONST uint16 ovr_regs_2069_rev32[];
#ifndef ACPHY_1X1_ONLY
extern CONST chan_info_rx_farrow BCMATTACHDATA(rx_farrow_tbl)[ACPHY_NUM_BW][ACPHY_NUM_CHANS];
extern CONST chan_info_tx_farrow BCMATTACHDATA(tx_farrow_dac1_tbl)[ACPHY_NUM_BW][ACPHY_NUM_CHANS];
extern CONST chan_info_tx_farrow BCMATTACHDATA(tx_farrow_dac2_tbl)[ACPHY_NUM_BW][ACPHY_NUM_CHANS];
extern CONST chan_info_tx_farrow BCMATTACHDATA(tx_farrow_dac3_tbl)[ACPHY_NUM_BW][ACPHY_NUM_CHANS];
#else /* ACPHY_1X1_ONLY */
extern CONST chan_info_rx_farrow BCMATTACHDATA(rx_farrow_tbl[1][ACPHY_NUM_CHANS]);
extern CONST chan_info_tx_farrow BCMATTACHDATA(tx_farrow_dac1_tbl[1][ACPHY_NUM_CHANS]);
#endif /* ACPHY_1X1_ONLY */
extern CONST uint16 acphy_txgain_epa_2g_2069rev0[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev0[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev0[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev0[];

extern CONST uint16 acphy_txgain_ipa_2g_2069rev16[];

extern uint16 acphy_txgain_epa_2g_2069rev17[];
extern uint16 acphy_txgain_epa_5g_2069rev17[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev17[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev17[];
extern CONST uint16 acphy_txgain_epa_2g_2069rev18[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev18[];

extern CONST uint16 acphy_txgain_epa_2g_2069rev4[];
extern CONST uint16 acphy_txgain_epa_2g_2069rev4_id1[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev4[];
extern CONST uint16 acphy_txgain_epa_2g_2069rev16[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev16[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev18[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev16[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev25[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev25[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev18[];
extern CONST uint16 acphy_txgain_epa_2g_2069rev33_35_36_37[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev33_35_36[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev37_38[];
extern CONST uint16 acphy_txgain_epa_2g_2069rev34[];
extern CONST uint16 acphy_txgain_epa_5g_2069rev34[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev33_37[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev33_37[];
extern CONST uint16 acphy_txgain_ipa_2g_2069rev39[];
extern CONST uint16 acphy_txgain_ipa_5g_2069rev39[];
extern CONST uint32 acphy_txv_for_spexp[];
extern uint16 BCMATTACHDATA(acphy_txgain_epa_2g_2069rev64)[TXGAIN_TABLES_LEN];
extern uint16 BCMATTACHDATA(acphy_txgain_epa_5g_2069rev64)[TXGAIN_TABLES_LEN];
extern uint16 BCMATTACHDATA(acphy_txgain_epa_5g_2069rev64_A0_SPUR_WAR)[TXGAIN_TABLES_LEN];

/* RFSEQ table ID's */
typedef enum {
	RESET2RX_SEQ_CMD_TBL,
	RESET2RX_SEQ_DLY_TBL,
	TX2RX_SEQ_CMD_TBL,
	TX2RX_SEQ_DLY_TBL,
	RX2TX_SEQ_CMD_TBL,
	RX2TX_SEQ_DLY_TBL,
	RX2TX_TSSISLEEPEN_SEQ_CMD_TBL,
	RX2TX_TSSISLEEPEN_SEQ_DLY_TBL,
	TX2RX_CAL_SEQ_CMD_TBL,
	TX2RX_CAL_SEQ_DLY_TBL,
} phy_ac_rfseq_tbl_id_t;

#endif /* _wlc_phytbl_ac_h_ */
