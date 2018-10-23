/*
 * BT coex module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_btcx.h 691048 2017-03-20 16:47:17Z $
 */

#ifndef _phy_type_btcx_h_
#define _phy_type_btcx_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_btcx.h>

typedef struct phy_btcx_priv_info phy_btcx_priv_info_t;

typedef struct phy_btcx_data {
	uint16	bt_period;
	bool	bt_active;
	uint16	saved_clb_sw_ctrl_mask;
} phy_btcx_data_t;

struct phy_btcx_info {
    phy_btcx_priv_info_t *priv;
    phy_btcx_data_t *data;
};

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_btcx_ctx_t;

typedef int (*phy_type_btcx_init_fn_t)(phy_type_btcx_ctx_t *ctx);
typedef void (*phy_type_btcx_fn_t)(phy_type_btcx_ctx_t *ctx);
typedef void (*phy_type_btcx_adjust_fn_t)(phy_type_btcx_ctx_t *ctx, bool btactive);
typedef void (*phy_type_btcx_set_femctrl_fn_t)(phy_type_btcx_ctx_t *ctx, int8 state, bool set);
typedef int8 (*phy_type_btcx_get_femctrl_fn_t)(phy_type_btcx_ctx_t *ctx);
typedef int (*phy_type_btcx_dump_fn_t)(phy_type_btcx_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_btcx_get_fn_t)(phy_type_btcx_ctx_t *ctx, int32 *ret_ptr);
typedef int (*phy_type_btcx_set_fn_t)(phy_type_btcx_ctx_t *ctx, int32 int_val);
typedef int (*phy_type_btcx_mode_set_fn_t)(phy_type_btcx_ctx_t *ctx, int btc_mode);
typedef int (*phy_type_btcx_ucm_update_siso_resp_offset_fn_t)
		(phy_type_btcx_ctx_t *ctx, int8 *siso_resp_pwr, uint8 len);
typedef int (*phy_type_btcx_ucm_set_desense_rxgain_fn_t)
		(phy_type_btcx_ctx_t *ctx, uint8 *desense_val);
typedef int32 (*phy_type_btcx_ucm_get_desense_rxgain_fn_t)
		(phy_type_btcx_ctx_t *ctx);
typedef void (*phy_type_btcx_ucm_get_siso_ack_pwr_fn_t)
		(phy_type_btcx_ctx_t *ctx, void *a);
typedef void (*phy_type_btcx_ucm_get_curr_siso_resp_pwr_offset_fn_t)
		(phy_type_btcx_ctx_t *ctx, void *a);
typedef int (*phy_type_btcx_ucm_set_txpwrcaplmt_fn_t)
		(phy_type_btcx_ctx_t *ctx, int8 *tx_power_cap_in_qdbm, bool active);
typedef void (*phy_type_btcx_ucm_set_txpwrcap_orig_fn_t)
		(phy_type_btcx_ctx_t *ctx, int8 *txpwcap);
typedef void (*phy_type_btcx_ucm_update_only_siso_resp_offset_fn_t)
		(phy_type_btcx_ctx_t *ctx);

typedef struct {
	phy_type_btcx_init_fn_t init_btcx;
	phy_type_btcx_adjust_fn_t adjust;
	phy_type_btcx_set_femctrl_fn_t set_femctrl;
	phy_type_btcx_get_femctrl_fn_t get_femctrl;
	phy_type_btcx_fn_t override_enable;
	phy_type_btcx_fn_t override_disable;
	phy_type_btcx_fn_t femctrl_mask;
	phy_type_btcx_get_fn_t get_preemptstatus;
	phy_type_btcx_set_fn_t desense_ltecx;
	phy_type_btcx_set_fn_t desense_btc;
	phy_type_btcx_set_fn_t set_restage_rxgain;
	phy_type_btcx_get_fn_t get_restage_rxgain;
	phy_type_btcx_mode_set_fn_t mode_set;
	phy_type_btcx_ctx_t *ctx;
	phy_type_btcx_ucm_update_siso_resp_offset_fn_t ucm_update_siso_resp_offset;
	phy_type_btcx_ucm_set_desense_rxgain_fn_t ucm_set_desense_rxgain;
	phy_type_btcx_ucm_get_desense_rxgain_fn_t ucm_get_desense_rxgain;
	phy_type_btcx_ucm_get_siso_ack_pwr_fn_t ucm_get_siso_ack_pwr;
	phy_type_btcx_ucm_get_curr_siso_resp_pwr_offset_fn_t ucm_get_curr_siso_resp_pwr_offset;
	phy_type_btcx_ucm_set_txpwrcaplmt_fn_t ucm_set_txpwrcaplmt;
	phy_type_btcx_ucm_set_txpwrcap_orig_fn_t ucm_set_txpwrcap_orig;
	phy_type_btcx_ucm_update_only_siso_resp_offset_fn_t ucm_update_only_siso_resp_offset;
} phy_type_btcx_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_btcx_register_impl(phy_btcx_info_t *cmn_info, phy_type_btcx_fns_t *fns);
void phy_btcx_unregister_impl(phy_btcx_info_t *cmn_info);

#endif /* _phy_type_btcx_h_ */
