/*
 * Radio 20698 channel tuning header file
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
 * $Id: wlc_phytbl_20698.h 618585 2016-02-11 18:13:22Z $
 */

#ifndef _WLC_PHYTBL_20698_H_
#define _WLC_PHYTBL_20698_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"
#include "phy_ac_rxgcrs.h"

typedef struct _chan_info_radio20698_rffe_2G {
	/* 2G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
	uint8 RF0_logen_core_reg3_logen_lc_ctune;
	uint8 RFP1_logen_reg1_logen_mix_ctune;
	uint8 RF1_logen_core_reg3_logen_lc_ctune;
	uint8 RF2_logen_core_reg3_logen_lc_ctune;
	uint8 RF3_logen_core_reg3_logen_lc_ctune;
	uint8 RF0_tx2g_mix_reg4_mx2g_tune;
	uint8 RF0_tx2g_pad_reg3_pad2g_tune;
	uint8 RF0_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF0_rx2g_reg3_rx2g_mix_Cin_tune;
	uint8 RF1_tx2g_mix_reg4_mx2g_tune;
	uint8 RF1_tx2g_pad_reg3_pad2g_tune;
	uint8 RF1_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF1_rx2g_reg3_rx2g_mix_Cin_tune;
	uint8 RF2_tx2g_mix_reg4_mx2g_tune;
	uint8 RF2_tx2g_pad_reg3_pad2g_tune;
	uint8 RF2_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF2_rx2g_reg3_rx2g_mix_Cin_tune;
	uint8 RF3_tx2g_mix_reg4_mx2g_tune;
	uint8 RF3_tx2g_pad_reg3_pad2g_tune;
	uint8 RF3_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF3_rx2g_reg3_rx2g_mix_Cin_tune;
} chan_info_radio20698_rffe_2G_t;

typedef struct _chan_info_radio20698_rffe_5G {
	/* 5G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
	uint8 RF0_logen_core_reg3_logen_lc_ctune;
	uint8 RFP1_logen_reg1_logen_mix_ctune;
	uint8 RF1_logen_core_reg3_logen_lc_ctune;
	uint8 RF2_logen_core_reg3_logen_lc_ctune;
	uint8 RF3_logen_core_reg3_logen_lc_ctune;
	uint8 RF0_logen_core_reg1_logen_rccr_tune;
	uint8 RF0_tx5g_mix_reg2_mx5g_tune;
	uint8 RF0_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF0_tx5g_pad_reg3_pad5g_tune;
	uint8 RF0_rx5g_reg1_rx5g_lna_tune;
	uint8 RF0_rx5g_reg5_rx5g_mix_Cin_tune;
	uint8 RF1_logen_core_reg1_logen_rccr_tune;
	uint8 RF1_tx5g_mix_reg2_mx5g_tune;
	uint8 RF1_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF1_tx5g_pad_reg3_pad5g_tune;
	uint8 RF1_rx5g_reg1_rx5g_lna_tune;
	uint8 RF1_rx5g_reg5_rx5g_mix_Cin_tune;
	uint8 RF2_logen_core_reg1_logen_rccr_tune;
	uint8 RF2_tx5g_mix_reg2_mx5g_tune;
	uint8 RF2_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF2_tx5g_pad_reg3_pad5g_tune;
	uint8 RF2_rx5g_reg1_rx5g_lna_tune;
	uint8 RF2_rx5g_reg5_rx5g_mix_Cin_tune;
	uint8 RF3_logen_core_reg1_logen_rccr_tune;
	uint8 RF3_tx5g_mix_reg2_mx5g_tune;
	uint8 RF3_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF3_tx5g_pad_reg3_pad5g_tune;
	uint8 RF3_rx5g_reg1_rx5g_lna_tune;
	uint8 RF3_rx5g_reg5_rx5g_mix_Cin_tune;
	uint8 RFP0_xtal2_xtal_cmos_pg_ctrl0;
	uint8 RFP0_pll_lvldo1_ldo_1p0_ldo_LOGEN_vout_sel;
	uint8 RFP0_pll_hvldo3_ldo_1p8_ldo_CP_vout_sel;
	uint8 RFP0_pll_refdoubler3_RefDoublerbuf_rstrg;
	uint8 RFP0_pll_refdoubler3_RefDoublerbuf_fstrg;
	uint8 RFP0_xtal1_xtal_LDO_Vctrl;
} chan_info_radio20698_rffe_5G_t;

typedef struct _chan_info_radio20698_rffe {
	uint16 channel;
	uint16 freq;
	union {
		/* In this union, make sure the largest struct is at the top. */
		chan_info_radio20698_rffe_5G_t val_5G;
		chan_info_radio20698_rffe_2G_t val_2G;
	} u;
} chan_info_radio20698_rffe_t;

extern const chan_info_radio20698_rffe_t
	chan_tune_20698_rev0[];
extern const chan_info_radio20698_rffe_t
	chan_tune_20698_rev1[];
extern const chan_info_radio20698_rffe_t
	chan_tune_20698_rev2[];

extern const uint16 chan_tune_20698_rev0_length;
extern const uint16 chan_tune_20698_rev1_length;
extern const uint16 chan_tune_20698_rev2_length;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20698_rev0[];
extern const radio_20xx_dumpregs_t dumpregs_20698_rev1[];
extern const radio_20xx_dumpregs_t dumpregs_20698_rev2[];
#endif // endif

/* Radio referred values tables */
extern const radio_20xx_prefregs_t prefregs_20698_rev0[];
extern const radio_20xx_prefregs_t prefregs_20698_rev1[];
extern const radio_20xx_prefregs_t prefregs_20698_rev2[];

extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_20698r0)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_20698r0)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_20698r0)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_20698r0)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_20698r0)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_20698r0)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_20698r0)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_20698r0)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_20698r0)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_20698r0)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_20698r0)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_20698r0)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_20698r0)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_20698rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_20698rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_20698rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_20698rX)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_20698rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_20698rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_20698rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_20698rX)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_20698rX)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_20698rX)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_20698rX)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_20698rX)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_20698rX)[2][N_BIQ01_GAINS];

extern uint16 BCMATTACHDATA(nap_lo_th_adj_maj47)[5];
extern uint16 BCMATTACHDATA(nap_hi_th_adj_maj47)[5];

#endif	/* _WLC_PHYTBL_20698_H_ */
