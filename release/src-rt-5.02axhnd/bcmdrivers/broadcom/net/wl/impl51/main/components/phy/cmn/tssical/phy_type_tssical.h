/*
 * TSSI Cal module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_tssical.h 685812 2017-02-18 02:44:08Z $
 */

#ifndef _phy_type_tssical_h_
#define _phy_type_tssical_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_tssical.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_tssical_ctx_t;

typedef int (*phy_type_tssical_init_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef int8 (*phy_type_tssical_get_visible_thresh_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef void (*phy_type_tssical_sens_min_fn_t)(phy_type_tssical_ctx_t *ctx, int8 *tssiSensMinPwr);
typedef int (*phy_type_tssical_dump_fn_t)(phy_type_tssical_ctx_t *ctx, struct bcmstrbuf *b);

#ifdef WLC_TXCAL
typedef void (*phy_type_tssical_compute_olpc_idx_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef uint16 (*phy_type_tssical_adjusted_tssi_fn_t)(phy_type_tssical_ctx_t *ctx, uint8 core_num);
typedef uint16 (*phy_type_tssical_txpwr_ctrl_cmd_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef void (*phy_type_tssical_set_txpwrindex_fn_t)(phy_type_tssical_ctx_t *ctx, int16 gain_idx,
		uint16 save_TxPwrCtrlCmd);
typedef void (*phy_type_tssical_restore_txpwr_ctrl_cmd_fn_t)(phy_type_tssical_ctx_t *ctx,
		uint16 save_TxPwrCtrlCmd, uint8 txpwr_ctrl_state);
typedef void (*phy_type_tssical_set_olpc_anchor_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef void (*phy_type_tssical_iov_apply_pwr_tssi_tbl_fn_t)(phy_type_tssical_ctx_t *ctx,
		int int_val);
typedef void (*phy_type_tssical_read_est_pwr_lut_fn_t)(phy_type_tssical_ctx_t *ctx,
		void *output_buff, uint8 core);
typedef void (*phy_type_tssical_apply_paparams_fn_t)(phy_type_tssical_ctx_t *ctx);
typedef int32 (*phy_type_tssical_get_tssi_val_fn_t)(phy_type_tssical_ctx_t *ctx, int32 tssi_val);
typedef void (*phy_type_tssical_apply_pwr_tssi_tbl_fn_t)(phy_type_tssical_ctx_t *ctx,
		wl_txcal_power_tssi_t* tssi_tbl, uint8 bphy_tbl);
#endif /* WLC_TXCAL */

typedef struct {
	phy_type_tssical_get_visible_thresh_fn_t visible_thresh;
	phy_type_tssical_sens_min_fn_t sens_min;

#ifdef WLC_TXCAL
	/* compute olpc index */
	phy_type_tssical_compute_olpc_idx_fn_t compute_olpc_idx;
	/* adjusted tssi */
	phy_type_tssical_adjusted_tssi_fn_t adjusted_tssi;
	/* TxPwrCtrlCmd */
	phy_type_tssical_txpwr_ctrl_cmd_fn_t txpwr_ctrl_cmd;
	/* set index */
	phy_type_tssical_set_txpwrindex_fn_t set_txpwrindex;
	/* restore  TxPwrCtrlCmd */
	phy_type_tssical_restore_txpwr_ctrl_cmd_fn_t restore_txpwr_ctrl_cmd;
	/* set olpc anchor */
	phy_type_tssical_set_olpc_anchor_fn_t set_olpc_anchor;
	/* apply pwr tssi tbl */
	phy_type_tssical_iov_apply_pwr_tssi_tbl_fn_t iov_apply_pwr_tssi_tbl;
	/* read est pwr lut */
	phy_type_tssical_read_est_pwr_lut_fn_t read_est_pwr_lut;
	/* apply paparams */
	phy_type_tssical_apply_paparams_fn_t apply_paparams;
	/* get tssi val */
	phy_type_tssical_get_tssi_val_fn_t get_tssi_val;
	/* apply pwr tssi tbl */
	phy_type_tssical_apply_pwr_tssi_tbl_fn_t apply_pwr_tssi_tbl;
#endif /* WLC_TXCAL */

	phy_type_tssical_ctx_t *ctx;
} phy_type_tssical_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_tssical_register_impl(phy_tssical_info_t *cmn_info, phy_type_tssical_fns_t *fns);
void phy_tssical_unregister_impl(phy_tssical_info_t *cmn_info);

#endif /* _phy_type_tssical_h_ */
