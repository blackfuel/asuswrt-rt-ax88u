/*
 * Radio 20704 table definition header file
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
 * $Id$
 */

#ifndef _WLC_PHYTBL_20704_H_
#define _WLC_PHYTBL_20704_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"
#include "phy_ac_rxgcrs.h"

typedef struct _chan_info_radio20704_rffe_2G {
	/* 63178FIXME: tuning registers not yet available: using stub */
	/* 2G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
} chan_info_radio20704_rffe_2G_t;

typedef struct _chan_info_radio20704_rffe_5G {
	/* 63178FIXME: tuning registers not yet available: using stub */
	/* 5G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
} chan_info_radio20704_rffe_5G_t;

typedef struct _chan_info_radio20704_rffe {
	uint16 channel;
	uint16 freq;
	union {
		/* In this union, make sure the largest struct is at the top. */
		chan_info_radio20704_rffe_5G_t val_5G;
		chan_info_radio20704_rffe_2G_t val_2G;
	} u;
} chan_info_radio20704_rffe_t;

extern const chan_info_radio20704_rffe_t
	chan_tune_20704_rev0[];

extern const uint16 chan_tune_20704_rev0_length;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20704_rev0[];
#endif // endif

/* Radio referred values tables */
extern const radio_20xx_prefregs_t prefregs_20704_rev0[];

extern int8 BCMATTACHDATA(lna12_gain_tbl_2g_20704rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gain_tbl_5g_20704rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_2g_20704rX)[2][N_LNA12_GAINS];
extern int8 BCMATTACHDATA(lna12_gainbits_tbl_5g_20704rX)[2][N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_2g_20704rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_rout_map_5g_20704rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_2g_20704rX)[N_LNA12_GAINS];
extern uint8 BCMATTACHDATA(lna1_gain_map_5g_20704rX)[N_LNA12_GAINS];
extern int8 BCMATTACHDATA(gainlimit_tbl_20704rX)[RXGAIN_CONF_ELEMENTS][MAX_RX_GAINS_PER_ELEM];
extern int8 BCMATTACHDATA(tia_gain_tbl_20704rX)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(tia_gainbits_tbl_20704rX)[N_TIA_GAINS];
extern int8 BCMATTACHDATA(biq01_gain_tbl_20704rX)[2][N_BIQ01_GAINS];
extern int8 BCMATTACHDATA(biq01_gainbits_tbl_20704rX)[2][N_BIQ01_GAINS];

extern uint16 BCMATTACHDATA(nap_lo_th_adj_maj51)[5];
extern uint16 BCMATTACHDATA(nap_hi_th_adj_maj51)[5];

#endif	/* _WLC_PHYTBL_20704_H_ */
