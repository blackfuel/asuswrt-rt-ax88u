/*
 * RSSI Compute module internal interface (to PHY specific implementation).
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
 * $Id: phy_type_rssi.h 671704 2016-11-22 21:37:09Z $
 */

#ifndef _phy_type_rssi_h_
#define _phy_type_rssi_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <hndd11.h>
#include <phy_rssi.h>
#include <wlioctl.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_rssi_ctx_t;

typedef void (*phy_type_rssi_compute_fn_t)(phy_type_rssi_ctx_t *ctx, wlc_d11rxhdr_t *wrxh);
typedef void (*phy_type_rssi_init_gain_err_fn_t)(phy_type_rssi_ctx_t *ctx);
typedef int (*phy_type_rssi_dump_fn_t)(phy_type_rssi_ctx_t *ctx, struct bcmstrbuf *b);
typedef void (*phy_type_rssi_update_pkteng_rxstats_fn_t)(phy_type_rssi_ctx_t *ctx, uint8 statidx);
typedef int (*phy_type_rssi_get_pkteng_stats_fn_t)(phy_type_rssi_ctx_t *ctx, void *a, int alen,
	wl_pkteng_stats_t stats, int8 *gain_correct);
typedef int (*phy_type_rssi_gain_delta_fn_t)(phy_type_rssi_ctx_t *ctx, uint32 aid,
	int8 *deltaValues);
typedef int (*phy_type_rssi_int_fn_t)(phy_type_rssi_ctx_t *ctx, int8 *value);
typedef int8 (*phy_type_rssi_get_rssi_fn_t)(phy_type_rssi_ctx_t *ctx, uint8 core);

typedef struct {
	phy_type_rssi_compute_fn_t compute;
	phy_type_rssi_init_gain_err_fn_t init_gain_err;
	phy_type_rssi_dump_fn_t dump;
	phy_type_rssi_update_pkteng_rxstats_fn_t update_pkteng_rxstats;
	phy_type_rssi_get_pkteng_stats_fn_t get_pkteng_stats;
	phy_type_rssi_gain_delta_fn_t set_gain_delta_2g;
	phy_type_rssi_gain_delta_fn_t get_gain_delta_2g;
	phy_type_rssi_gain_delta_fn_t set_gain_delta_5g;
	phy_type_rssi_gain_delta_fn_t get_gain_delta_5g;
	phy_type_rssi_gain_delta_fn_t set_gain_delta_2gb;
	phy_type_rssi_gain_delta_fn_t get_gain_delta_2gb;
	phy_type_rssi_int_fn_t set_cal_freq_2g;
	phy_type_rssi_int_fn_t get_cal_freq_2g;
	phy_type_rssi_get_rssi_fn_t get_rssi;
	phy_type_rssi_ctx_t *ctx;
} phy_type_rssi_fns_t;

/*
 * Register/unregister PHY type implementation to the common of the RSSI Compute module.
 * It returns BCME_XXXX.
 */
int phy_rssi_register_impl(phy_rssi_info_t *ri, phy_type_rssi_fns_t *fns);
void phy_rssi_unregister_impl(phy_rssi_info_t *ri);

/*
 * Enable RSSI moving average algorithm.
 * It returns BCME_XXXX.
 */
int phy_rssi_enable_ma(phy_rssi_info_t *ri, bool enab);

#endif /* _phy_type_rssi_h_ */
