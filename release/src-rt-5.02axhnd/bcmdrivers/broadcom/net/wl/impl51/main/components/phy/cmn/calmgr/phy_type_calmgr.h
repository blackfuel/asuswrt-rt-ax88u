/*
 * Calibration Management module internal interface (to PHY specific implementation).
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
 * $Id: phy_type_calmgr.h 659967 2016-09-16 19:35:14Z $
 */

#ifndef _phy_type_calmgr_h_
#define _phy_type_calmgr_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_calmgr.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_calmgr_ctx_t;
typedef int (*phy_type_calmgr_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef bool (*phy_type_calmgr_bool_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef int (*phy_type_calmgr_prepare_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef void (*phy_type_calmgr_cleanup_fn_t)(phy_type_calmgr_ctx_t *ctx);
typedef void (*phy_type_calmgr_cals_fn_t)(phy_type_calmgr_ctx_t *ctx,
	uint8 legacy_caltype, uint8 searchmode);
typedef void (*phy_type_calmgr_add_timer_fn_t)(phy_type_calmgr_ctx_t *ctx, uint delay_val);
typedef void (*phy_type_calmgr_set_override_fn_t)(phy_type_calmgr_ctx_t *ctx, uint8 override);
typedef int (*phy_type_calmgr_enable_initcal_fn_t)(phy_type_calmgr_ctx_t *ctx, bool initcal);
typedef struct {
	phy_type_calmgr_fn_t init;
	phy_type_calmgr_bool_fn_t wd;
	phy_type_calmgr_enable_initcal_fn_t enable_initcal;
	phy_type_calmgr_prepare_fn_t prepare;
	phy_type_calmgr_cleanup_fn_t cleanup;
	phy_type_calmgr_cals_fn_t cals;
	phy_type_calmgr_add_timer_fn_t add_timer_special;
	phy_type_calmgr_set_override_fn_t set_override;
	phy_type_calmgr_ctx_t *ctx;
} phy_type_calmgr_fns_t;

/*
 * Register/unregister PHY type implementation to the common layer.
 * It returns BCME_XXXX.
 */
int phy_calmgr_register_impl(phy_calmgr_info_t *ci, phy_type_calmgr_fns_t *fns);
void phy_calmgr_unregister_impl(phy_calmgr_info_t *ci);

#endif /* _phy_type_calmgr_h_ */
