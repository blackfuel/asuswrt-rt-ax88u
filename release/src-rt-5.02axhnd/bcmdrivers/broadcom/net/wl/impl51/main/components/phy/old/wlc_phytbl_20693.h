/*
 * Declarations for Broadcom PHY core tables,
 * Networking Adapter Device Driver.
 *
 * THE CONTENTS OF THIS FILE IS TEMPORARY.
 * Eventually it'll be auto-generated.
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
 * $Id: wlc_phytbl_20693.h 648596 2016-07-13 01:16:40Z $
 */

#ifndef _WLC_PHYTBL_20693_H_
#define _WLC_PHYTBL_20693_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"

#define NUM_ROWS_CHAN_TUNING 77
#define NUM_ALTCLKPLN_CHANS 5
#define NUM_ALTCLKPLN_CHANS_ROUTER4349 1

/*
 * Channel Info table for the 20693 (4349).
 */

typedef struct _chan_info_radio20693_pll {
	uint8 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	/* other stuff */
	uint16 pll_vcocal1;
	uint16 pll_vcocal11;
	uint16 pll_vcocal12;
	uint16 pll_frct2;
	uint16 pll_frct3;
	uint16 pll_hvldo1;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf7;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 spare_cfg1;
	uint16 spare_cfg14;
	uint16 spare_cfg13;
	uint16 txmix2g_cfg5;
	uint16 txmix5g_cfg6;
	uint16 pa5g_cfg4;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio20693_pll_t;

typedef struct _chan_info_radio20693_pll_wave2 {
	uint8 chan;            /* channel number */
	uint16 freq;            /* in Mhz */
	/* other stuff */
	uint16 wl_xtal_cfg3;
	uint16 pll_vcocal18;
	uint16 pll_vcocal3;
	uint16 pll_vcocal4;
	uint16 pll_vcocal7;
	uint16 pll_vcocal8;
	uint16 pll_vcocal20;
	uint16 pll_vcocal1;
	uint16 pll_vcocal12;
	uint16 pll_vcocal13;
	uint16 pll_vcocal10;
	uint16 pll_vcocal11;
	uint16 pll_vcocal19;
	uint16 pll_vcocal6;
	uint16 pll_vcocal9;
	uint16 pll_vcocal17;
	uint16 pll_vcocal15;
	uint16 pll_vcocal2;
	uint16 pll_vcocal24;
	uint16 pll_vcocal26;
	uint16 pll_vcocal25;
	uint16 pll_vcocal21;
	uint16 pll_vcocal22;
	uint16 pll_vcocal23;
	uint16 pll_vco7;
	uint16 pll_frct2;
	uint16 pll_frct3;
	uint16 pll_vco2;
	uint16 pll_vco6;
	uint16 pll_vco5;
	uint16 pll_vco4;
	uint16 pll_lf4;
	uint16 pll_lf5;
	uint16 pll_lf7;
	uint16 pll_lf2;
	uint16 pll_lf3;
	uint16 pll_cp4;
	uint16 pll_vcocal5;
	uint16 lo2g_logen0_drv;
	uint16 lo2g_vco_drv_cfg2;
	uint16 lo2g_logen1_drv;
	uint16 lo5g_core0_cfg1;
	uint16 lo5g_core1_cfg1;
	uint16 lo5g_core2_cfg1;
	uint16 lo5g_core3_cfg1;
	uint16 lna2g_tune;
	uint16 logen2g_rccr;
	uint16 txmix2g_cfg5;
	uint16 pa2g_cfg2;
	uint16 tx_logen2g_cfg1;
	uint16 lna5g_tune;
	uint16 logen5g_rccr;
	uint16 tx5g_tune;
	uint16 tx_logen5g_cfg1;
	uint16 PHY_BW1a;
	uint16 PHY_BW2;
	uint16 PHY_BW3;
	uint16 PHY_BW4;
	uint16 PHY_BW5;
	uint16 PHY_BW6;
} chan_info_radio20693_pll_wave2_t;

/* Rev5,7 & 6,8 */
typedef struct _chan_info_radio20693_rffe {
	uint16 lna2g_tune;
	uint16 lna5g_tune;
	uint16 pa2g_cfg2;
} chan_info_radio20693_rffe_t;

typedef struct _chan_info_radio20693_altclkplan {
	uint8 channel;
	uint8 bw;
	uint8 afeclkdiv;
	uint8 adcclkdiv;
	uint8 sipodiv;
	uint8 dacclkdiv;
	uint8 dacdiv;
} chan_info_radio20693_altclkplan_t;
#if defined(BCMDBG)
#if defined(DBG_PHY_IOV)
extern const radio_20xx_dumpregs_t dumpregs_20693_rev5[];

#endif // endif
#endif // endif

/* Radio referred values tables */
extern const radio_20xx_prefregs_t prefregs_20693_rev5[];
extern const radio_20xx_prefregs_t prefregs_20693_rev6[];
extern const radio_20xx_prefregs_t prefregs_20693_rev10[];
extern const radio_20xx_prefregs_t prefregs_20693_rev13[];
extern const radio_20xx_prefregs_t prefregs_20693_rev14[];
extern const radio_20xx_prefregs_t prefregs_20693_rev15_37p4MHz[];
extern const radio_20xx_prefregs_t prefregs_20693_rev15_40MHz[];
extern const radio_20xx_prefregs_t prefregs_20693_rev18[];
extern const radio_20xx_prefregs_t prefregs_20693_rev19[];
extern const radio_20xx_prefregs_t prefregs_20693_rev23_40MHz[];
extern const radio_20xx_prefregs_t prefregs_20693_rev25[];
extern const radio_20xx_prefregs_t prefregs_20693_rev32[];

/* Radio tuning values tables */
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev5_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev5_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev6_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev6_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev10_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev10_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev13_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev13_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev14_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev14_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev15_rffe_37p4MHz[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev15_pll_37p4MHz[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev15_rffe_40MHz[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev15_pll_40MHz[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev18_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev18_pll[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev32_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_wave2_t
	chan_tuning_20693_rev32_pll_part1[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_wave2_t
	chan_tuning_20693_rev32_pll_part2[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_wave2_t
	chan_tuning_20693_rev33_pll_part1[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_wave2_t
	chan_tuning_20693_rev33_pll_part2[NUM_ROWS_CHAN_TUNING];

extern const chan_info_radio20693_altclkplan_t altclkpln_radio20693[NUM_ALTCLKPLN_CHANS];
extern const chan_info_radio20693_altclkplan_t
	altclkpln_radio20693_router4349[NUM_ALTCLKPLN_CHANS_ROUTER4349];

extern const chan_info_radio20693_rffe_t chan_tuning_20693_rev25_rffe[NUM_ROWS_CHAN_TUNING];
extern const chan_info_radio20693_pll_t chan_tuning_20693_rev25_pll[NUM_ROWS_CHAN_TUNING];

/* For 2g ipa only, to be removed after code addition */
extern uint16 acphy_radiogainqdb_20693_majrev3[128];

#endif	/* _WLC_PHYTBL_20693_H_ */
