/*
 * PHYTableInit module internal interface (to PHY specific implementation).
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
 * $Id: phy_type_tbl.h 720902 2017-09-12 17:20:17Z $
 */

#ifndef _phy_type_tbl_h_
#define _phy_type_tbl_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_tbl.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_tbl_ctx_t;

typedef int (*phy_type_tbl_init_fn_t)(phy_type_tbl_ctx_t *ctx);
typedef int (*phy_type_tbl_down_fn_t)(phy_type_tbl_ctx_t *ctx);
typedef bool (*phy_type_tbl_dump_tblfltr_fn_t)(phy_type_tbl_ctx_t *ctx, phy_table_info_t *ti);
typedef bool (*phy_type_tbl_dump_addrfltr_fn_t)(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr);
typedef void (*phy_type_tbl_read_tbl_fn_t)(phy_type_tbl_ctx_t *ctx,
	phy_table_info_t *ti, uint addr, uint16 *val, uint16 *qval);
typedef int (*phy_type_tbl_dump_fn_t)(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_tbl_dump_txv0_fn_t)(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);
typedef int (*phy_type_tbl_dump_axmacphyif_fn_t)(phy_type_tbl_ctx_t *ctx, struct bcmstrbuf *b);

typedef struct {
	phy_type_tbl_init_fn_t init;
	phy_type_tbl_down_fn_t down;
	phy_type_tbl_dump_tblfltr_fn_t tblfltr;
	phy_type_tbl_dump_addrfltr_fn_t addrfltr;
	phy_type_tbl_read_tbl_fn_t readtbl;
	phy_type_tbl_dump_fn_t dump;
	phy_type_tbl_ctx_t *ctx;
	phy_type_tbl_dump_txv0_fn_t dump_txv0;
	phy_type_tbl_dump_axmacphyif_fn_t dump_axmacphyif;
} phy_type_tbl_fns_t;

/*
 * Register/unregister PHY type implementation to the PHY Init module.
 * It returns BCME_XXXX.
 */
int phy_tbl_register_impl(phy_tbl_info_t *ii, phy_type_tbl_fns_t *fns);
void phy_tbl_unregister_impl(phy_tbl_info_t *ii);

/* Dump specified table to buffer */
void phy_tbl_do_dumptbl(phy_tbl_info_t *info, phy_table_info_t *ti, struct bcmstrbuf *b);

#endif /* _phy_type_tbl_h_ */
