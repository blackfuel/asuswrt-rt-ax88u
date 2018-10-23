/*
 * ACPHY BT Coex module interface (to other PHY modules).
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
 * $Id: phy_ac_btcx.h 691048 2017-03-20 16:47:17Z $
 */

#ifndef _phy_ac_btcx_h_
#define _phy_ac_btcx_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_btcx.h>

/* forward declaration */
typedef struct phy_ac_btcx_info phy_ac_btcx_info_t;

typedef struct phy_ac_btcx_data {
	int32	btc_mode;
	int32	ltecx_mode;
	bool	poll_adc_WAR;
} phy_ac_btcx_data_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_btcx_info_t *phy_ac_btcx_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_btcx_info_t *cmn_info);
void phy_ac_btcx_unregister_impl(phy_ac_btcx_info_t *info);

/* inter-module data API */
phy_ac_btcx_data_t *phy_ac_btcx_get_data(phy_ac_btcx_info_t *btcxi);

void chanspec_btcx(phy_info_t *pi);
void wlc_phy_set_bt_on_core1_acphy(phy_info_t *pi, uint8 bt_fem_val, uint16 gpioen);
void wlc_phy_bt_on_gpio4_acphy(phy_info_t *pi);

#if (!defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX))
void wlc_phy_ac_femctrl_mask_on_band_change_btcx(phy_ac_btcx_info_t *info);
void phy_ac_btcx_reset_swctrl(phy_ac_btcx_info_t *info);
#endif /* !defined(WL_SISOCHIP) && defined(SWCTRL_TO_BT_IN_COEX) */

#ifdef WL_UCM
void phy_ac_btcx_ucm_set_max_siso_resp_pwr(phy_ac_btcx_info_t *btcxi, int8 *siso_resp_pwr);
#endif /* WL_UCM */

#endif /* _phy_ac_btcx_h_ */
