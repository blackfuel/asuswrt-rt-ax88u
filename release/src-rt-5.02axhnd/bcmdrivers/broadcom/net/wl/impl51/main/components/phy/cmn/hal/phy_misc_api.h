/*
 * Miscellaneous PHY module public interface (to MAC driver).
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
 * $Id: phy_misc_api.h 667934 2016-10-31 21:36:53Z $
 */

#ifndef _phy_misc_api_h_
#define _phy_misc_api_h_

#include <phy_api.h>

#define PHY_HOLD_FOR_ASSOC	1	/* hold PHY activities(like cal) during association */
#define PHY_HOLD_FOR_SCAN	2	/* hold PHY activities(like cal) during scan */
#define PHY_HOLD_FOR_RM		4	/* hold PHY activities(like cal) during radio measure */
#define PHY_HOLD_FOR_PLT	8	/* hold PHY activities(like cal) during plt */
#define PHY_HOLD_FOR_MUTE		0x10
#define PHY_HOLD_FOR_NOT_ASSOC	0x20
#define PHY_HOLD_FOR_ACI_SCAN	0x40 /* hold PHY activities(like cal) during ACI scan */
#define PHY_HOLD_FOR_PKT_ENG	0x80 /* hold PHY activities(like cal) during PKTENG mode */
#define PHY_HOLD_FOR_DCS	    0x100 /* hold PHY activities(like cal) during DCS */
#define PHY_HOLD_FOR_MPC_SCAN	0x200 /* hold PHY activities(like cal) during scan in mpc 1 mode */
#define PHY_HOLD_FOR_EXCURSION	0x400 /* hold PHY activities while in excursion */
#define PHY_HOLD_FOR_TOF	0x800 /* hold PHY activities while doing wifi ranging */
#define PHY_HOLD_FOR_VSDB	0x1000 /* hold PHY cals during  VSDB */

#define PHY_MUTE_FOR_PREISM	1
#define PHY_MUTE_ALL		0xffffffff

void phy_misc_set_ldpc_override(wlc_phy_t *ppi, bool ldpc);
int phy_misc_set_lo_gain_nbcal(wlc_phy_t *ppi, bool lo_gain);
int phy_misc_set_filt_war(wlc_phy_t *ppi, bool filt_war);
bool phy_misc_get_filt_war(wlc_phy_t *ppi);
int phy_misc_tkip_rifs_war(wlc_phy_t *ppi, uint8 rifs);
/* ******************** */
void wlc_phy_set_deaf(wlc_phy_t *ppi, bool user_flag);
void wlc_phy_clear_deaf(wlc_phy_t *ppi, bool user_flag);
#ifdef ATE_BUILD
void wlc_phy_gpaio(wlc_phy_t *ppi, wl_gpaio_option_t option, int core);
#endif /* ATE_BUILD */
void wlc_phy_hold_upd(wlc_phy_t *ppi, mbool id, bool val);
void wlc_phy_mute_upd(wlc_phy_t *ppi, bool val, mbool flags);
bool wlc_phy_ismuted(wlc_phy_t *ppi);

/* WAR */
void wlc_phy_conditional_suspend(phy_info_t *pi, bool *suspend);
void wlc_phy_conditional_resume(phy_info_t *pi, bool *suspend);

uint16
wlc_phy_classifier(wlc_phy_t *ppi, uint16 mask, uint16 val);

#endif /* _phy_misc_api_h_ */
