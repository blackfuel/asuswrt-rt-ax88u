/*
 * Rx Gain Control and Carrier Sense module internal interface
 * (to PHY specific implementations).
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
 * $Id: phy_type_rxgcrs.h 658925 2016-09-11 16:42:42Z $
 */

#ifndef _phy_type_rxgcrs_h_
#define _phy_type_rxgcrs_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_rxgcrs.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_rxgcrs_ctx_t;

typedef int (*phy_type_rxgcrs_init_fn_t)(phy_type_rxgcrs_ctx_t *ctx);
typedef int (*phy_type_rxgcrs_dump_fn_t)(phy_type_rxgcrs_ctx_t *ctx, struct bcmstrbuf *b);
typedef void (*phy_type_rxgcrs_locale_eu_fn_t)(phy_type_rxgcrs_ctx_t *ctx, uint8 region_group);
typedef void (*phy_type_rxgcrs_adjust_ed_thres_fn_t)(phy_type_rxgcrs_ctx_t *pi,
	int32 *assert_thresh_dbm, bool set_threshold);
typedef int (*phy_type_rxgcrs_get_fn_t)(phy_type_rxgcrs_ctx_t *ctx, int32 *ret_int_ptr);
typedef int (*phy_type_rxgcrs_set_fn_t)(phy_type_rxgcrs_ctx_t *ctx, int32 int_val);
typedef int (*phy_type_rxgcrs_forcecal_noise_fn_t)(phy_type_rxgcrs_ctx_t *ctx, void *a, bool set);
typedef uint16 (*phy_type_rxgcrs_sel_classifier_fn_t)(phy_type_rxgcrs_ctx_t *ctx, uint16 val);
typedef void (*phy_type_rxgcrs_stay_in_carriersearch_fn_t)(phy_type_rxgcrs_ctx_t *ctx,
	bool enable);
typedef void (*phy_type_rxgcrs_phydump_chanest_fn_t)(phy_type_rxgcrs_ctx_t *ctx,
	struct bcmstrbuf *b);
typedef void (*phy_type_rxgcrs_get_chanest_fn_t)(phy_type_rxgcrs_ctx_t *ctx, uint16 fftk,
	uint16 idx, int16 *ch_re, int16 *ch_im);
typedef int (*phy_type_rxgcrs_phydump_phycal_rx_min_fn_t)(phy_type_rxgcrs_ctx_t *ctx,
	struct bcmstrbuf *b);

typedef struct {
	phy_type_rxgcrs_ctx_t *ctx;
	phy_type_rxgcrs_locale_eu_fn_t set_locale;
	phy_type_rxgcrs_adjust_ed_thres_fn_t adjust_ed_thres;
	phy_type_rxgcrs_init_fn_t init_rxgcrs;
	phy_type_rxgcrs_get_fn_t get_rxdesens;
	phy_type_rxgcrs_set_fn_t set_rxdesens;
	phy_type_rxgcrs_get_fn_t get_rxgainindex;
	phy_type_rxgcrs_set_fn_t set_rxgainindex;
	phy_type_rxgcrs_forcecal_noise_fn_t forcecal_noise;
	phy_type_rxgcrs_sel_classifier_fn_t sel_classifier;
	phy_type_rxgcrs_stay_in_carriersearch_fn_t stay_in_carriersearch;
	phy_type_rxgcrs_phydump_chanest_fn_t	phydump_chanest;
	phy_type_rxgcrs_get_chanest_fn_t	get_chanest;
	phy_type_rxgcrs_phydump_phycal_rx_min_fn_t	phydump_phycal_rxmin;
} phy_type_rxgcrs_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_rxgcrs_register_impl(phy_rxgcrs_info_t *mi, phy_type_rxgcrs_fns_t *fns);
void phy_rxgcrs_unregister_impl(phy_rxgcrs_info_t *cmn_info);

#endif /* _phy_type_rxgcrs_h_ */
