/*
 * ACPHY VCO CAL module interface (to other PHY modules).
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
 * $Id: phy_ac_vcocal.h 742511 2018-01-22 14:14:24Z $
 */

#ifndef _phy_ac_vcocal_h_
#define _phy_ac_vcocal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_vcocal.h>

/* vco calcode cache address for ucode pm mode */
#define VCOCAL_MAINCAP_OFFSET_20693   14
#define VCOCAL_SECONDCAP_OFFSET_20693 18
#define VCOCAL_AUXCAP0_OFFSET_20693   22
#define VCOCAL_AUXCAP1_OFFSET_20693   26

/* forward declaration */
typedef struct phy_ac_vcocal_info phy_ac_vcocal_info_t;
typedef struct _acphy_vcocal_radregs_t {
	bool is_orig;
	uint16 clk_div_ovr1;
	uint16 clk_div_cfg1;
} acphy_vcocal_radregs_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_vcocal_info_t *phy_ac_vcocal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_vcocal_info_t *mi);
void phy_ac_vcocal_unregister_impl(phy_ac_vcocal_info_t *info);

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
extern void wlc_phy_radio_tiny_vcocal(phy_info_t *pi);
extern void wlc_phy_radio_tiny_vcocal_wave2(phy_info_t *pi, uint8 vcocal_mode, uint8 pll_mode,
	uint8 coupling_mode, bool cache_calcode);
extern void wlc_phy_radio_tiny_afe_resynch(phy_info_t *pi, uint8 mode);
extern void wlc_phy_radio2069_vcocal(phy_info_t *pi);
extern int wlc_phy_radio2069x_vcocal_isdone(phy_info_t *pi, bool set_delay, bool cache_calcode);
extern void wlc_phy_28nm_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode);
extern void wlc_phy_28nm_radio_vcocal_isdone(phy_info_t *pi, bool set_delay);
extern void wlc_phy_20694_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode);
extern void wlc_phy_20696_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode);
extern void wlc_phy_20697_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode,
	uint8 pll_num);
extern void wlc_phy_20698_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode,
	uint8 logen_mode);
extern void wlc_phy_20704_radio_vcocal(phy_info_t *pi, uint8 cal_mode, uint8 coupling_mode);
extern void wlc_phy_radio20694_vcocal_isdone(phy_info_t *pi, bool set_delay, bool cache_calcode);
extern void wlc_phy_radio20696_vcocal_isdone(phy_info_t *pi, bool set_delay, bool cache_calcode);
extern void wlc_phy_radio20698_vcocal_isdone(phy_info_t *pi, bool set_delay);
extern void wlc_phy_radio20704_vcocal_isdone(phy_info_t *pi, bool set_delay);
extern int32 wlc_phy_radio20697_vcocal_isdone(phy_info_t *pi, bool set_delay, bool cache_calcode,
	uint8 pll_num);
extern void wlc_phy_get_radio20697_vcocal_codes(phy_info_t *pi, uint16 *maincap, uint16* secondcap,
	uint16* auxcap);
/* 20695 vco cal */
#define VCO_CAL_MODE_20695              0
#define VCO_CAL_COUPLING_MODE_20695     0
/* 20696 vco cal */
#define VCO_CAL_MODE_20696              0
#define VCO_CAL_COUPLING_MODE_20696     0
/* 20698 vco cal */
#define VCO_CAL_MODE_20698              0
#define VCO_CAL_COUPLING_MODE_20698     1
/* 20704 vco cal */
#define VCO_CAL_MODE_20704              0
#define VCO_CAL_COUPLING_MODE_20704     1

void phy_ac_vcocal(phy_info_t *pi);

#endif /* _phy_ac_vcocal_h_ */
