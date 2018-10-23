/*
 * ACPHY Calibration Manager module interface (to other PHY modules).
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
 * $Id: phy_ac_calmgr.h 718240 2017-08-30 06:36:58Z $
 */

#ifndef _phy_ac_calmgr_h_
#define _phy_ac_calmgr_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_calmgr.h>

#include <phy_utils_math.h>
#include <phy_txiqlocal.h>

/* forward declaration */
typedef struct phy_ac_calmgr_info phy_ac_calmgr_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_calmgr_info_t *phy_ac_calmgr_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_calmgr_info_t *cmn_info);
void phy_ac_calmgr_unregister_impl(phy_ac_calmgr_info_t *info);

/* Inter-module data API */
bool phy_ac_calmgr_get_enIndxCap(phy_ac_calmgr_info_t *calmgri);
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
void phy_ac_calmgr_clean(phy_info_t *pi);
void phy_ac_calmgr_singleshot(phy_info_t *pi, uint8 searchmode, acphy_cal_result_t *accal);
void phy_ac_calmgr_multiphase(phy_info_t *pi, uint8 phase_id, uint8 searchmode);
void wlc_phy_cals_acphy(void *ctx, uint8 legacy_caltype, uint8 searchmode);
extern void wlc_phy_low_rate_adc_enable_acphy(phy_info_t *pi, bool enable);
void wlc_phy_set_txfgc_acphy(phy_info_t *pi, uint8 set_val, uint8 exp_val);

#endif /* _phy_ac_calmgr_h_ */
