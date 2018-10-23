/*
 * Rx Gain Control and Carrier Sense module interface (to other PHY modules).
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
 * $Id: phy_rxgcrs.h 657351 2016-08-31 23:00:22Z $
 */

#ifndef _phy_rxgcrs_h_
#define _phy_rxgcrs_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_rxgcrs_info phy_rxgcrs_info_t;

/* attach/detach */
phy_rxgcrs_info_t *phy_rxgcrs_attach(phy_info_t *pi);
void phy_rxgcrs_detach(phy_rxgcrs_info_t *cmn_info);

/* down */
int phy_rxgcrs_down(phy_rxgcrs_info_t *cmn_info);

uint8 wlc_phy_get_locale(phy_rxgcrs_info_t *info);
int wlc_phy_adjust_ed_thres(phy_info_t *pi, int32 *assert_thresh_dbm, bool set_threshold);
/* Rx desense Module */
#if defined(RXDESENS_EN)
int phy_rxgcrs_get_rxdesens(phy_info_t *pi, int32 *ret_int_ptr);
int phy_rxgcrs_set_rxdesens(phy_info_t *pi, int32 int_val);
#endif // endif
int wlc_phy_set_rx_gainindex(phy_info_t *pi, int32 gain_idx);
int wlc_phy_get_rx_gainindex(phy_info_t *pi, int32 *gain_idx);
#ifndef ATE_BUILD
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
int wlc_phy_iovar_forcecal_noise(phy_info_t *pi, void *a, bool set);
#endif // endif
#endif /* !ATE_BUILD */
void phy_rxgcrs_stay_in_carriersearch(void *ctx, bool enable);
#if defined(BCMDBG)
int phy_rxgcrs_dump_chanest(void *ctx, struct bcmstrbuf *b);
#endif // endif
#ifdef WLTEST
int phy_rxgcrs_dump_ch4rpcal(void *ctx, struct bcmstrbuf *b);
#endif /* WLTEST */
#if defined(DBG_BCN_LOSS)
int phy_rxgcrs_dump_phycal_rx_min(void *ctx, struct bcmstrbuf *b);
#endif /* DBG_BCN_LOSS */
#endif /* _phy_rxgcrs_h_ */
