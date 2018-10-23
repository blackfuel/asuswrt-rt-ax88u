/*
 * ACPHY Sample Collect module interface (to other PHY modules).
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
 * $Id: phy_ac_samp.h 655756 2016-08-23 12:40:40Z $
 */

#ifndef _phy_ac_samp_h_
#define _phy_ac_samp_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_samp.h>
#include <phy_type_samp.h>

/* Macros for sample play */
#ifdef IQPLAY_DEBUG
#define SAMPLE_COLLECT_PLAY_CTRL_PLAY_MODE_SHIFT	10
#define START_IDX_ADDR	65536
#define AXI_BASE_ADDR	0xE8000000U
#define MAIN_AUX_CORE_ADDR_OFFSET	0x800000U
#define BM_BASE_OFFFSET_ADDR	0x00400000U
#endif /* IQPLAY_DEBUG */

/* forward declaration */
typedef struct phy_ac_samp_info phy_ac_samp_info_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_samp_info_t *phy_ac_samp_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_samp_info_t *cmn_info);
void phy_ac_samp_unregister_impl(phy_ac_samp_info_t *ac_info);

/* ************************************** */
/* ACPHY sample collect intermodule api's */
/* ************************************** */
#ifdef WL_PROXDETECT
void acphy_set_sc_startptr(phy_info_t *pi, uint32 start_idx);
void acphy_set_sc_stopptr(phy_info_t *pi, uint32 stop_idx);
#endif /* WL_PROXDETECT */
#ifdef SAMPLE_COLLECT
void phy_ac_lpf_hpc_override(phy_ac_info_t *aci, bool setup);
#endif /* SAMPLE_COLLECT */
#endif /* _phy_ac_samp_h_ */
