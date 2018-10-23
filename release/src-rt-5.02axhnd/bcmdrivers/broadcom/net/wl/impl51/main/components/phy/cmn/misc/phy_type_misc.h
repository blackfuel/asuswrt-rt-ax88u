/*
 * Miscellaneous module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_misc.h 667934 2016-10-31 21:36:53Z $
 */

#ifndef _phy_type_misc_h_
#define _phy_type_misc_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_misc.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_misc_ctx_t;

typedef int (*phy_type_misc_init_fn_t)(phy_type_misc_ctx_t *ctx);
typedef int (*phy_type_misc_dump_fn_t)(phy_type_misc_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_misc_getlistandsize_fn_t)(phy_type_misc_ctx_t *ctx,
	phyradregs_list_t **phyreglist, uint16 *phyreglist_sz);
typedef void (*phy_type_misc_test_init_fn_t)(phy_type_misc_ctx_t *ctx, bool encals);
typedef void (*phy_type_misc_test_stop_fn_t)(phy_type_misc_ctx_t *ctx);
typedef uint32 (*phy_type_misc_rx_iq_est_fn_t)(phy_type_misc_ctx_t *ctx, uint8 samples,
	uint8 antsel, uint8 resolution, uint8 lpf_hpc, uint8 dig_lpf, uint8 gain_correct,
                       uint8 extra_gain_3dB, uint8 wait_for_crs, uint8 force_gain_type);
typedef void (*phy_type_misc_deaf_mode_fn_t)(phy_type_misc_ctx_t *ctx, bool user_flag);
typedef int (*phy_type_misc_test_freq_accuracy_fn_t)(phy_type_misc_ctx_t *ctx, int channel);
typedef void (*phy_type_misc_iovar_tx_tone_fn_t)(phy_type_misc_ctx_t *ctx, int32 int_val);
typedef void (*phy_type_misc_iovar_txlo_tone_fn_t)(phy_type_misc_ctx_t *ctx);
typedef int (*phy_type_misc_iovar_get_rx_iq_est_fn_t)(phy_type_misc_ctx_t *ctx,
	int32 *ret_int_ptr, int32 int_val, int err);
typedef int (*phy_type_misc_iovar_set_rx_iq_est_fn_t)(phy_type_misc_ctx_t *ctx,
	int32 int_val, int err);
typedef void (*phy_type_misc_gpaioconfig_fn_t) (phy_type_misc_ctx_t *ctx,
	wl_gpaio_option_t option, int core);
typedef void (*phy_type_misc_disable_ate_gpiosel_fn_t)(phy_type_misc_ctx_t *ctx);
typedef int (*phy_type_misc_txswctrlmapset_fn_t) (phy_type_misc_ctx_t *ctx,
	int32 int_val);
typedef void (*phy_type_misc_txswctrlmapget_fn_t) (phy_type_misc_ctx_t *ctx,
	int32 *ret_int_ptr);
typedef bool (*phy_type_misc_get_rxgainerr_fn_t)(phy_type_misc_ctx_t *ctx, int16 *gainerr);

typedef uint8 (*phy_get_bfe_ndp_recvstreams_fn_t)(phy_type_misc_ctx_t *ctx);
typedef void (*phy_ldpc_override_set_fn_t)(phy_type_misc_ctx_t *ctx, bool ldpc);
typedef void (*phy_preamble_override_set_fn_t)(phy_type_misc_ctx_t *ctx, int8 val);
typedef void (*phy_set_wfdll_chan_active_fn_t)(phy_type_misc_ctx_t *ctx, bool chan_active);
/* WARs */
typedef int (*phy_type_misc_set_bool_fn_t)(phy_type_misc_ctx_t *ctx, bool val);
typedef bool (*phy_type_misc_get_bool_fn_t)(phy_type_misc_ctx_t *ctx);
typedef int (*phy_type_misc_tkip_rifs_war_fn_t)(phy_type_misc_ctx_t *ctx, uint8 rifs);

typedef uint16 (*phy_type_misc_classifier_fn_t)(phy_type_misc_ctx_t *ctx, uint16 mask, uint16 val);

typedef struct {
	phy_type_misc_ctx_t *ctx;
	phy_type_misc_getlistandsize_fn_t phy_type_misc_getlistandsize;
	phy_type_misc_test_init_fn_t phy_type_misc_test_init;
	phy_type_misc_test_stop_fn_t phy_type_misc_test_stop;
	phy_type_misc_rx_iq_est_fn_t phy_type_misc_rx_iq_est;
	phy_type_misc_deaf_mode_fn_t phy_type_misc_set_deaf;
	phy_type_misc_deaf_mode_fn_t phy_type_misc_clear_deaf;
	phy_type_misc_test_freq_accuracy_fn_t phy_type_misc_test_freq_accuracy;
	phy_type_misc_iovar_tx_tone_fn_t phy_type_misc_iovar_tx_tone;
	phy_type_misc_iovar_txlo_tone_fn_t phy_type_misc_iovar_txlo_tone;
	phy_type_misc_iovar_get_rx_iq_est_fn_t phy_type_misc_iovar_get_rx_iq_est;
	phy_type_misc_iovar_set_rx_iq_est_fn_t phy_type_misc_iovar_set_rx_iq_est;
	phy_type_misc_gpaioconfig_fn_t	gpaioconfig;
	phy_type_misc_disable_ate_gpiosel_fn_t	disable_ate_gpiosel;
	phy_type_misc_txswctrlmapset_fn_t	txswctrlmapset;
	phy_type_misc_txswctrlmapget_fn_t	txswctrlmapget;
	phy_type_misc_get_rxgainerr_fn_t phy_type_misc_get_rxgainerr;
	phy_get_bfe_ndp_recvstreams_fn_t	get_bfe_ndp_recvstreams;
	phy_ldpc_override_set_fn_t		set_ldpc_override;
	phy_preamble_override_set_fn_t	set_preamble_override;
	phy_set_wfdll_chan_active_fn_t	set_wfdll_chan_active;
	phy_type_misc_set_bool_fn_t set_lo_gain_nbcal;
	/* WARs */
	phy_type_misc_set_bool_fn_t set_filt_war;
	phy_type_misc_get_bool_fn_t get_filt_war;
	phy_type_misc_tkip_rifs_war_fn_t tkip_rifs_war;
	phy_type_misc_classifier_fn_t phy_type_misc_classifier;
} phy_type_misc_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_misc_register_impl(phy_misc_info_t *mi, phy_type_misc_fns_t *fns);
void phy_misc_unregister_impl(phy_misc_info_t *cmn_info);

#endif /* _phy_type_misc_h_ */
