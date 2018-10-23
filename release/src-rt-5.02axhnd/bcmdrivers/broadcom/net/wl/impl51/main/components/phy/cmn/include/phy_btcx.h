/*
 * BlueToothCoExistence module internal interface (to other PHY modules).
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
 * $Id: phy_btcx.h 691048 2017-03-20 16:47:17Z $
 */

#ifndef _phy_btcx_h_
#define _phy_btcx_h_

#include <typedefs.h>
#include <phy_api.h>
#include <wlioctl.h>

/* forward declaration */
typedef struct phy_btcx_info phy_btcx_info_t;

/* attach/detach */
phy_btcx_info_t *phy_btcx_attach(phy_info_t *pi);
void phy_btcx_detach(phy_btcx_info_t *ri);
void phy_btcx_disable_arbiter(phy_btcx_info_t *bi);
void phy_btcx_enable_arbiter(phy_btcx_info_t *bi);
void wlc_btcx_override_enable(phy_info_t *pi);
void wlc_phy_btcx_override_disable(phy_info_t *pi);
void wlc_phy_btcx_wlan_critical_enter(phy_info_t *pi);
void wlc_phy_btcx_wlan_critical_exit(phy_info_t *pi);
bool phy_btcx_is_btactive(phy_btcx_info_t *cmn_info);
int wlc_phy_iovar_set_btc_restage_rxgain(phy_btcx_info_t *btcxi, int32 set_val);
int wlc_phy_iovar_get_btc_restage_rxgain(phy_btcx_info_t *btcxi, int32 *ret_val);
#if defined(WLTEST)
int phy_btcx_get_preemptstatus(phy_info_t *pi, int32* ret_ptr);
#endif // endif
#if !defined(WLC_DISABLE_ACI)
int phy_btcx_desense_btc(phy_info_t *pi, int32 mode);
#endif /* !defined(WLC_DISABLE_ACI) */

#ifdef WL_UCM
int phy_btcx_ucm_get_desense_rxgain(phy_btcx_info_t *btcxi,
		wl_desense_restage_gain_t *desense_restage_gain);
void phy_btcx_ucm_get_siso_ack_pwr(phy_btcx_info_t *btcxi, void *a);
void phy_btcx_ucm_get_curr_siso_resp_pwr_offset(phy_btcx_info_t *btcxi, void *a);
void phy_btcx_ucm_set_txpwrcap_orig(phy_btcx_info_t *btcxi, int8 *txpwrcap);
#endif /* WL_UCM */

#endif /* _phy_btcx_h_ */
