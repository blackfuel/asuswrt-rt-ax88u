/*
 * ACPHY Napping module interface (to other PHY modules).
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
 * $Id$
 */

#ifndef _phy_ac_nap_h_
#define _phy_ac_nap_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_nap.h>

#if defined(WL_NAP)
	#if defined(ROM_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define PHY_NAP_ENAB(physhim)		(wlapi_nap_enab_check(physhim))
	#elif defined(WL_NAP_DISABLED)
		#define PHY_NAP_ENAB(physhim)		(0)
	#else
		#define PHY_NAP_ENAB(physhim)		(wlapi_nap_enab_check(physhim))
	#endif
#else
	#define PHY_NAP_ENAB(physhim)			(0)
#endif /* WL_NAP */

/* forward declaration */
typedef struct phy_ac_nap_info phy_ac_nap_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_nap_info_t *phy_ac_nap_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_nap_info_t *cmn_info);
void phy_ac_nap_unregister_impl(phy_ac_nap_info_t *ac_info);
bool phy_ac_nap_is_enabled(phy_ac_nap_info_t *nap_info);

#ifdef WL_NAP
void phy_ac_config_napping_28nm_ulp(phy_info_t *pi);
void phy_ac_config_napping_28nm(phy_info_t *pi);
void phy_ac_nap_ed_thresh_cal(phy_info_t *pi, int8 *cmplx_pwr_dBm);
void phy_ac_nap_enable(phy_info_t *pi, bool enable, bool agc_reconfig);
void phy_ac_nap_update_energy_threshold(phy_ac_nap_info_t *nap_info);
#define NAP_ENERGY_THRESHOLD_DBM			(-90)
#define NAP_THRESHOLD_TBL_REF_ENERGY		(-90)
#define NAP_THRESHOLD_TBL_REF_GAIN			64
#define NAP_THRESHOLD_TBL_REF_THRESHOLD		3540
/* 10*log10(65535/3540) ~= 12.67 */
#define NAP_THRESHOLD_INDEX_MAX				12
#define NAP_THRESHOLD_ADJ_TEMP_THRESHOLD	20
#endif /* WL_NAP */
#endif /* _phy_ac_nap_h_ */
