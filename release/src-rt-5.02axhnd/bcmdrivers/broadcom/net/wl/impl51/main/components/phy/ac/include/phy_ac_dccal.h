/*
 * ACPHY dccal module interface (to other PHY modules).
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
 * $Id: phy_ac_dccal.h 754220 2018-03-26 18:37:38Z $
 */

#ifndef _phy_ac_dccal_h_
#define _phy_ac_dccal_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_dccal.h>
#include <wlc_radioreg_20695.h>
#include <wlc_radioreg_20694.h>
#include <wlc_radioreg_20697.h>
#include <wlc_radioreg_20698.h>

/* forward declaration */
typedef struct phy_ac_dccal_info phy_ac_dccal_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_dccal_info_t *phy_ac_dccal_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_dccal_info_t *ani);
void phy_ac_dccal_unregister_impl(phy_ac_dccal_info_t *info);

void wlc_dcc_fsm_restart(phy_info_t *pi);
int wlc_phy_tiny_static_dc_offset_cal(phy_info_t *pi);
void wlc_dcc_fsm_reset(phy_info_t *pi);
void phy_ac_dccal(phy_info_t *pi);
void phy_ac_dccal_2steps(phy_info_t *pi);
void phy_ac_dccal_dcoe_only(phy_info_t *pi);
void phy_ac_dccal_idacc_only(phy_info_t *pi);
void phy_ac_dccal_init(phy_info_t *pi);
void phy_ac_load_gmap_tbl(phy_info_t *pi);

/* DCC with digcorr */
void phy_ax_dccal_digcorr_init(phy_info_t *pi);
void phy_ax_dccal_digcorr_dcoe(phy_info_t *pi);
void phy_ax_dccal_digcorr_idacc(phy_info_t *pi);

/* Analog DCC sw-war related functions */
void acphy_dcc_idac_set(phy_info_t *pi, int16 dac, int ch, int core);
void acphy_search_idac_iq_est(phy_info_t *pi, uint8 core,
                                     int16 dac_min, int16 dac_max, int8 dac_step);
int acphy_analog_dc_cal_war(phy_info_t *pi);

#endif /* _phy_ac_dccal_h_ */
