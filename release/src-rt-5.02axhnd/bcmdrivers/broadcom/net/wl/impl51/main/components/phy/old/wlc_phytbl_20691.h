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
 * $Id: wlc_phytbl_20691.h 614873 2016-01-25 06:02:55Z $
 */

#ifndef _WLC_PHYTBL_20691_H_
#define _WLC_PHYTBL_20691_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"

/*
 * Channel Info table for the 20691 (4345).
 */

typedef struct _chan_info_radio20691 {
	uint16 chan;            /* channel number */
	uint16 freq;            /* in Mhz */

	/* other stuff */
	uint16 RF_pll_vcocal1;
	uint16 RF_pll_vcocal11;
	uint16 RF_pll_vcocal12;
	uint16 RF_pll_frct2;
	uint16 RF_pll_frct3;
	uint16 RF_pll_lf4;
	uint16 RF_pll_lf5;
	uint16 RF_pll_lf7;
	uint16 RF_pll_lf2;
	uint16 RF_pll_lf3;
	uint16 RF_logen_cfg1;
	uint16 RF_logen_cfg2;
	uint16 RF_lna2g_tune;
	uint16 RF_txmix2g_cfg5;
	uint16 RF_pa2g_cfg2;
	uint16 RF_lna5g_tune;
	uint16 RF_txmix5g_cfg6;
	uint16 RF_pa5g_cfg4;
} chan_info_radio20691_t;

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
extern const radio_20xx_dumpregs_t dumpregs_20691_rev68[];
extern const radio_20xx_dumpregs_t dumpregs_20691_rev75[];
extern const radio_20xx_dumpregs_t dumpregs_20691_rev79[];
extern const radio_20xx_dumpregs_t dumpregs_20691_rev82[];
extern const radio_20xx_dumpregs_t dumpregs_20691_rev88[];
extern const radio_20xx_dumpregs_t dumpregs_20691_rev129[];
#endif // endif

/* Radio referred values tables */
extern const radio_20xx_prefregs_t prefregs_20691_rev68[];
extern const radio_20xx_prefregs_t prefregs_20691_rev75[];
extern const radio_20xx_prefregs_t prefregs_20691_rev79[];
extern const radio_20xx_prefregs_t prefregs_20691_rev82[];
extern const radio_20xx_prefregs_t prefregs_20691_rev88[];
extern const radio_20xx_prefregs_t prefregs_20691_rev129[];

/* Radio tuning values tables */
extern const chan_info_radio20691_t chan_tuning_20691_rev68[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev75[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev79[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev82[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev88[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev129[59];
extern const chan_info_radio20691_t chan_tuning_20691_rev130[59];

#endif	/* _WLC_PHYTBL_20691_H_ */
