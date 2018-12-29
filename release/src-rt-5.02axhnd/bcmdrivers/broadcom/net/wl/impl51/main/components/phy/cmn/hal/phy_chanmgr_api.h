/*
 * Channel Manager module public interface (to MAC driver).
 *
 * Controls radio chanspec and manages related operating chanpsec context.
 *
 * Operating chanspec context:
 *
 * A Operating chanspec context is a collection of s/w properties
 * associated with a radio chanspec.
 *
 * Current operating chanspec context:
 *
 * The current operating chanspec context is the operating chanspec context
 * whose associated chanspec is used as the current radio chanspec, and
 * whose s/w properties are applied to the corresponding h/w if any.
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
 * $Id: phy_chanmgr_api.h 766514 2018-08-04 06:18:03Z $
 */

#ifndef _phy_chanmgr_api_h_
#define _phy_chanmgr_api_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>
#include <phy_api.h>

/* add the feature to take out the BW RESET duriny the BW change  0: disable 1: enable */
#define BW_RESET 1

#ifdef PHYCAL_CACHING
/*
 * Create/Destroy an operating chanspec context for radio 'chanspec'.
 */
int phy_chanmgr_create_ctx(phy_info_t *pi, chanspec_t chanspec);
void phy_chanmgr_destroy_ctx(phy_info_t *pi, chanspec_t chanspec);

/*
 * Set the operating chanspec context associated with the 'chanspec'
 * as the current operating chanspec context, and set the 'chanspec'
 * as the current radio chanspec.
 */
int phy_chanmgr_set_oper(phy_info_t *pi, chanspec_t chanspec);

/*
 * Set the radio chanspec to 'chanspec', and unset the current
 * operating chanspec context if any.
 */
int phy_chanmgr_set(phy_info_t *pi, chanspec_t chanspec);
#endif /* PHYCAL_CACHING */

void wlc_phy_chanspec_set(wlc_phy_t *ppi, chanspec_t chanspec);
void wlc_phy_chanspec_radio_set(wlc_phy_t *ppi, chanspec_t newch);
bool wlc_phy_is_txbfcal(wlc_phy_t *ppi);
bool wlc_phy_is_smth_en(wlc_phy_t *ppi);

/* band specific init */
int phy_chanmgr_bsinit(phy_info_t *pi, chanspec_t chanspec, bool forced);
/* band width init */
int phy_chanmgr_bwinit(phy_info_t *pi, chanspec_t chanspec);

void phy_chanmgr_tdcs_enable_160m(phy_info_t *pi, bool set_val);

/*     VSDB, RVSDB Module related definitions         */
uint8 phy_chanmgr_vsdb_sr_attach_module(wlc_phy_t *ppi, chanspec_t chan0, chanspec_t chan1);
int phy_chanmgr_vsdb_sr_detach_module(wlc_phy_t *pi);
uint8 phy_chanmgr_vsdb_sr_set_chanspec(wlc_phy_t *pi, chanspec_t chanspec, uint8 * last_chan_saved);
int phy_chanmgr_vsdb_force_chans(wlc_phy_t *pi, uint16 * chans, uint8 set);
#endif /* _phy_chanmgr_api_h_ */
