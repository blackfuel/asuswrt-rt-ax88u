/*
 * ACPHY ANTennaDIVersity module interface (to other PHY modules).
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
 * $Id: phy_ac_antdiv.h 653304 2016-08-06 03:05:00Z $
 */

#ifndef _phy_ac_antdiv_h_
#define _phy_ac_antdiv_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_antdiv.h>
#include <phy_type_antdiv.h>

#define ANTDIV_RFSWCTRLPIN_UNDEFINED	255
#ifdef WLC_SW_DIVERSITY
#define ANTDIV_ANTMAP_NOCHANGE	(0xFF)	/* antmap is not specified */
#define ACPHY_PHYCORE_ANY	(0xFF)	/* any phycore is fine */
#endif /* WLC_SW_DIVERSITY */

/* forward declaration */
typedef struct phy_ac_antdiv_info phy_ac_antdiv_info_t;

/* register/unregister phy type specific implementation */
phy_ac_antdiv_info_t *phy_ac_antdiv_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_antdiv_info_t *di);
extern void phy_ac_antdiv_unregister_impl(phy_ac_antdiv_info_t *di);
extern bool wlc_phy_check_antdiv_enable_acphy(phy_info_t *pi);
extern void wlc_phy_antdiv_acphy(phy_info_t *pi, uint8 val);

#ifdef WLC_SW_DIVERSITY
void wlc_phy_swdiv_antmap_init(phy_type_antdiv_ctx_t *ctx);
void wlc_phy_set_femctrl_control_reg(phy_type_antdiv_ctx_t *ctx);
bool phy_ac_swdiv_is_supported(phy_type_antdiv_ctx_t *ctx, uint8 core, bool inanyband);
uint8 phy_ac_swdiv_get_rxant_bycoreidx(phy_type_antdiv_ctx_t *ctx, uint core);
#ifdef WLC_TXPWRCAP
void phy_ac_swdiv_txpwrcap_shmem_set(phy_type_antdiv_ctx_t *ctx,
	uint core, uint8 cap_tx, uint8 cap_rx, uint16 txantmap, uint16 rxantmap);
#endif /* WLC_TXPWRCAP */
#endif /* WLC_SW_DIVERSITY */
void wlc_phy_write_regtbl_fc_from_nvram(phy_info_t *pi);
void phy_ac_antdiv_chanspec(phy_ac_antdiv_info_t * antdivi);
void phy_ac_antdiv_regtbl_fc_from_nvram(phy_ac_antdiv_info_t *antdivi);

#endif /* _phy_ac_antdiv_h_ */
