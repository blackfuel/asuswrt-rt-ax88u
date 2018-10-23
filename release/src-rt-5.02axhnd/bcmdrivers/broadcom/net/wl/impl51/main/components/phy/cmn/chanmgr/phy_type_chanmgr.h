/*
 * Channel manager interface (to PHY specific implementations).
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
 * $Id: phy_type_chanmgr.h 766664 2018-08-09 08:50:20Z $
 */

#ifndef _phy_type_chanmgr_h_
#define _phy_type_chanmgr_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_chanmgr.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_chanmgr_ctx_t;
typedef int (*phy_type_chanmgr_init_fn_t)(phy_type_chanmgr_ctx_t *ctx);
typedef bool (*phy_type_chanmgr_is_txbfcal_fn_t)(phy_type_chanmgr_ctx_t *ctx);
typedef bool (*phy_type_chanmgr_is_smth_en_fn_t)(phy_type_chanmgr_ctx_t *ctx);
typedef int (*phy_type_chanmgr_get_chanspec_bandrange_fn_t)(phy_type_chanmgr_ctx_t *ctx,
    chanspec_t chanspec);
typedef void (*phy_type_chanmgr_chanspec_set_fn_t)(phy_type_chanmgr_ctx_t *ctx,
    chanspec_t chanspec);
typedef void (*phy_type_chanmgr_upd_interf_mode_fn_t)(phy_type_chanmgr_ctx_t *ctx,
    chanspec_t chanspec);
typedef uint8 (*phy_type_set_chanspec_sr_vsdb_fn_t) (phy_type_chanmgr_ctx_t *ctx,
		chanspec_t chanspec, uint8 *last_chan_saved);
typedef void (*phy_type_chanmgr_preempt_fn_t)(phy_type_chanmgr_ctx_t *ctx, bool enable_preempt,
    bool EnablePostRxFilter_Proc);
typedef void (*phy_type_chanmgr_tdcs_enable_160m_fn_t)(phy_info_t *pi, bool set_val);
typedef int (*phy_type_chanmgr_get_fn_t)(phy_type_chanmgr_ctx_t *ctx, int32 *ret_int_ptr);
typedef int (*phy_type_chanmgr_set_fn_t)(phy_type_chanmgr_ctx_t *ctx, int8 int_val);
typedef int (*phy_type_chanmgr_dump_fn_t)(phy_type_chanmgr_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_chanmgr_bsinit_fn_t)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec,
	bool forced);
typedef int (*phy_type_chanmgr_bwinit_fn_t)(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);

typedef struct {
	/* Chanmgr Init */
	phy_type_chanmgr_init_fn_t init_chanspec;
	/* Band-specific init */
	phy_type_chanmgr_bsinit_fn_t bsinit;
	/* Band-width init */
	phy_type_chanmgr_bwinit_fn_t bwinit;
	/* TXBF cal? */
	phy_type_chanmgr_is_txbfcal_fn_t is_txbfcal;
	/* is smth enabled? */
	phy_type_chanmgr_is_smth_en_fn_t is_smth_en;
	/* get channel frequency band range */
	phy_type_chanmgr_get_chanspec_bandrange_fn_t get_bandrange;
	/* set channel specification */
	phy_type_chanmgr_chanspec_set_fn_t chanspec_set;
	/* update interference mode */
	phy_type_chanmgr_upd_interf_mode_fn_t interfmode_upd;
	/* set channel for vsdb */
	phy_type_set_chanspec_sr_vsdb_fn_t set_chanspec_sr_vsdb;
	/* pre-empt */
	phy_type_chanmgr_preempt_fn_t preempt;
	/* set TDCS for 160M */
	phy_type_chanmgr_tdcs_enable_160m_fn_t tdcs_enable_160m;
	/* set stf chain */
	phy_type_chanmgr_get_fn_t get_smth;
	/* set smth */
	phy_type_chanmgr_set_fn_t set_smth;
	/* context */
	phy_type_chanmgr_ctx_t *ctx;
} phy_type_chanmgr_fns_t;

/*
 * Register/unregister PHY type implementation to the TxPowerCtrl module.
 * It returns BCME_XXXX.
 */
int phy_chanmgr_register_impl(phy_chanmgr_info_t *cmn_info, phy_type_chanmgr_fns_t *fns);
void phy_chanmgr_unregister_impl(phy_chanmgr_info_t *cmn_info);

#endif /* _phy_type_chanmgr_h_ */
