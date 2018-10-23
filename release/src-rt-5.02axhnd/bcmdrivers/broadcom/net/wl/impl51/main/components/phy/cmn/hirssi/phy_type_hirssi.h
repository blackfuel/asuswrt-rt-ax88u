/*
 * HiRSSI eLNA Bypass module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_hirssi.h 707224 2017-06-27 01:13:09Z $
 */

#ifndef _phy_type_hirssi_h_
#define _phy_type_hirssi_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_hirssi.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_hirssi_ctx_t;
typedef int (*phy_type_hirssi_fn_t)(phy_type_hirssi_ctx_t *ctx);
typedef int (*phy_type_hirssi_get_period_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 *period);
typedef int (*phy_type_hirssi_set_period_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 period);
typedef int (*phy_type_hirssi_get_en_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 *enable);
typedef int (*phy_type_hirssi_set_en_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 enable);
typedef int (*phy_type_hirssi_get_rssi_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 *rssi,
	phy_hirssi_t opt);
typedef int (*phy_type_hirssi_set_rssi_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 rssi,
	phy_hirssi_t opt);
typedef int (*phy_type_hirssi_get_cnt_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 *cnt,
	phy_hirssi_t opt);
typedef int (*phy_type_hirssi_set_cnt_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 cnt,
	phy_hirssi_t opt);
typedef int (*phy_type_hirssi_get_status_fn_t)(phy_type_hirssi_ctx_t *ctx, int32 *status);

typedef struct {
	/* init */
	phy_type_hirssi_fn_t init_hirssi;
	/* get period */
	phy_type_hirssi_get_period_fn_t getperiod;
	/* set period */
	phy_type_hirssi_set_period_fn_t setperiod;
	/* get enable flag */
	phy_type_hirssi_get_en_fn_t geten;
	/* set enable flag */
	phy_type_hirssi_set_en_fn_t seten;
	/* get rssi */
	phy_type_hirssi_get_rssi_fn_t getrssi;
	/* set rssi */
	phy_type_hirssi_set_rssi_fn_t setrssi;
	/* get count */
	phy_type_hirssi_get_cnt_fn_t getcnt;
	/* set count */
	phy_type_hirssi_set_cnt_fn_t setcnt;
	/* get status */
	phy_type_hirssi_get_status_fn_t getstatus;
	/* context */
	phy_type_hirssi_ctx_t *ctx;
} phy_type_hirssi_fns_t;

/*
 * Register/unregister PHY type implementation to the HIRSSI module.
 *
 * It returns BCME_XXXX.
 */
int phy_hirssi_register_impl(phy_hirssi_info_t *hirssi, phy_type_hirssi_fns_t *fns);
void phy_hirssi_unregister_impl(phy_hirssi_info_t *hirssi);

#endif /* _phy_type_hirssi_h_ */
