/*
 * PAPD CAL module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_papdcal.h 639713 2016-05-24 18:02:57Z $
 */

#ifndef _phy_type_papdcal_h_
#define _phy_type_papdcal_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_papdcal.h>

#ifdef WLPHY_IPA_ONLY
	#define PHY_EPAPD(pi)   0
#elif defined(EPAPD_SUPPORT)
	#define PHY_EPAPD(pi)   1
#elif defined(EPAPD_SUPPORT_NVRAMCTL)
	#define PHY_EPAPD(pi)							\
			(((pi)->papdcali->data->epacal2g && CHSPEC_IS2G((pi)->radio_chanspec) && \
			(((pi)->papdcali->data->epacal2g_mask >> \
			(CHSPEC_CHANNEL(pi->radio_chanspec) - 1)) & 1)) || \
			((pi)->papdcali->data->epacal5g && CHSPEC_IS5G((pi)->radio_chanspec)))
#elif defined(DONGLEBUILD)
	#define PHY_EPAPD(pi)   0
#else
	#define PHY_EPAPD(pi)							\
			(((pi)->papdcali->data->epacal2g && CHSPEC_IS2G((pi)->radio_chanspec) && \
			(((pi)->papdcali->data->epacal2g_mask >> \
			(CHSPEC_CHANNEL(pi->radio_chanspec) - 1)) & 1)) || \
			((pi)->papdcali->data->epacal5g && CHSPEC_IS5G((pi)->radio_chanspec)))
#endif /* WLPHY_IPA_ONLY */

typedef struct phy_papdcal_priv_info phy_papdcal_priv_info_t;

typedef struct phy_papdcal_data {
	uint8	epacal2g;
	uint16	epacal2g_mask;
	uint8	epacal5g;
	/* #ifdef WFD_PHY_LL */
	uint8	wfd_ll_enable;
	uint8	wfd_ll_enable_pending;
	uint8	wfd_ll_chan_active;
	uint8	wfd_ll_chan_active_force;
	/* #endif */
} phy_papdcal_data_t;

struct phy_papdcal_info {
    phy_papdcal_priv_info_t *priv;
    phy_papdcal_data_t *data;
};

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_papdcal_ctx_t;

typedef int (*phy_type_papdcal_init_fn_t)(phy_type_papdcal_ctx_t *ctx);
typedef void (*phy_type_papdcal_epa_dpd_set_fn_t)(phy_type_papdcal_ctx_t *ctx, uint8 enab_epa_dpd,
	bool in_2g_band);
typedef int (*phy_type_papdcal_dump_fn_t)(phy_type_papdcal_ctx_t *ctx, struct bcmstrbuf *b);
typedef bool (*phy_type_papdcal_isperratedpden_fn_t) (phy_type_papdcal_ctx_t *ctx);
typedef void (*phy_type_papdcal_perratedpdset_fn_t) (phy_type_papdcal_ctx_t *ctx, bool enable);
typedef int (*phy_type_papdcal_set_uint_fn_t) (phy_type_papdcal_ctx_t *ctx, uint8 var);
typedef int (*phy_type_papdcal_get_var_fn_t) (phy_type_papdcal_ctx_t *ctx, int32 *var);
typedef int (*phy_type_papdcal_set_int_fn_t) (phy_type_papdcal_ctx_t *ctx, int8 var);
typedef void (*phy_type_papdcal_void_fn_t) (phy_type_papdcal_ctx_t *ctx);
typedef struct {
	phy_type_papdcal_epa_dpd_set_fn_t epa_dpd_set;
	phy_type_papdcal_isperratedpden_fn_t	isperratedpden;
	phy_type_papdcal_perratedpdset_fn_t	perratedpdset;
	phy_type_papdcal_get_var_fn_t get_idx0;
	phy_type_papdcal_get_var_fn_t get_idx1;
	phy_type_papdcal_set_int_fn_t set_idx;
	phy_type_papdcal_set_uint_fn_t set_skip;
	phy_type_papdcal_get_var_fn_t get_skip;
	phy_type_papdcal_void_fn_t wd_wfd_ll;
	phy_type_papdcal_set_uint_fn_t set_wfd_ll_enable;
	phy_type_papdcal_get_var_fn_t get_wfd_ll_enable;
	phy_type_papdcal_ctx_t *ctx;
} phy_type_papdcal_fns_t;

/*
 * Register/unregister PHY type implementation to the papdcal module.
 * It returns BCME_XXXX.
 */
int phy_papdcal_register_impl(phy_papdcal_info_t *cmn_info, phy_type_papdcal_fns_t *fns);
void phy_papdcal_unregister_impl(phy_papdcal_info_t *cmn_info);

#endif /* _phy_type_papdcal_h_ */
