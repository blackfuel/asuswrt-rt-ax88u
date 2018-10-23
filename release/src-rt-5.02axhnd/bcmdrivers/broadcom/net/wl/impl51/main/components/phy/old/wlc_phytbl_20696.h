/*
 * Radio 20696 channel tuning header file
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
 * $Id: wlc_phytbl_20694.h 618585 2016-02-11 18:13:22Z $
 */

#ifndef _WLC_PHYTBL_20696_H_
#define _WLC_PHYTBL_20696_H_

#include <wlc_cfg.h>
#include <typedefs.h>

#include "wlc_phy_int.h"

typedef struct _chan_info_radio20696_rffe_2G {
	/* 2G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
	uint8 RF0_logen_core_reg3_logen_lc_ctune;
	uint8 RF0_rxadc_reg2_rxadc_ti_ctune_Q;
	uint8 RF0_rxadc_reg2_rxadc_ti_ctune_I;
	uint8 RF0_tx2g_mix_reg4_mx2g_tune;
	uint8 RF0_tx2g_pad_reg3_pad2g_tune;
	uint8 RF0_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF1_logen_core_reg3_logen_lc_ctune;
	uint8 RF1_tx2g_mix_reg4_mx2g_tune;
	uint8 RF1_tx2g_pad_reg3_pad2g_tune;
	uint8 RF1_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF2_logen_core_reg3_logen_lc_ctune;
	uint8 RF2_tx2g_mix_reg4_mx2g_tune;
	uint8 RF2_tx2g_pad_reg3_pad2g_tune;
	uint8 RF2_lna2g_reg1_lna2g_lna1_freq_tune;
	uint8 RF3_logen_core_reg3_logen_lc_ctune;
	uint8 RF3_tx2g_mix_reg4_mx2g_tune;
	uint8 RF3_tx2g_pad_reg3_pad2g_tune;
	uint8 RF3_lna2g_reg1_lna2g_lna1_freq_tune;
} chan_info_radio20696_rffe_2G_t;

typedef struct _chan_info_radio20696_rffe_5G {
	/* 5G tuning data */
	uint8 RFP0_logen_reg1_logen_mix_ctune;
	uint8 RF0_logen_core_reg3_logen_lc_ctune;
	uint8 RF0_rxadc_reg2_rxadc_ti_ctune_Q;
	uint8 RF0_rxadc_reg2_rxadc_ti_ctune_I;
	uint8 RF0_tx5g_mix_reg2_mx5g_tune;
	uint8 RF0_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF0_tx5g_pad_reg3_pad5g_tune;
	uint8 RF0_rx5g_reg1_rx5g_lna_tune;
	uint8 RF1_logen_core_reg3_logen_lc_ctune;
	uint8 RF1_tx5g_mix_reg2_mx5g_tune;
	uint8 RF1_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF1_tx5g_pad_reg3_pad5g_tune;
	uint8 RF1_rx5g_reg1_rx5g_lna_tune;
	uint8 RF2_logen_core_reg3_logen_lc_ctune;
	uint8 RF2_tx5g_mix_reg2_mx5g_tune;
	uint8 RF2_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF2_tx5g_pad_reg3_pad5g_tune;
	uint8 RF2_rx5g_reg1_rx5g_lna_tune;
	uint8 RF3_logen_core_reg3_logen_lc_ctune;
	uint8 RF3_tx5g_mix_reg2_mx5g_tune;
	uint8 RF3_tx5g_pa_reg4_tx5g_pa_tune;
	uint8 RF3_tx5g_pad_reg3_pad5g_tune;
	uint8 RF3_rx5g_reg1_rx5g_lna_tune;
} chan_info_radio20696_rffe_5G_t;

typedef struct _chan_info_radio20696_rffe {
	uint16 channel;
	uint16 freq;
	union {
		/* In this union, make sure the largest struct is at the top. */
		chan_info_radio20696_rffe_5G_t val_5G;
		chan_info_radio20696_rffe_2G_t val_2G;
	} u;
} chan_info_radio20696_rffe_t;

extern const chan_info_radio20696_rffe_t
	chan_tune_20696_rev0[];

extern const uint16 chan_tune_20696_rev0_length;

#if defined(BCMDBG)
#if defined(DBG_PHY_IOV)
extern radio_20xx_dumpregs_t dumpregs_20696_rev0[];

#endif // endif
#endif // endif

/* Radio referred values tables */
extern radio_20xx_prefregs_t prefregs_20696_rev0[];

#endif	/* _WLC_PHYTBL_20696_H_ */
