/*
 * Link Power Control module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_lpc.h 666101 2016-10-19 23:52:54Z $
 */

#ifndef _phy_type_lpc_h_
#define _phy_type_lpc_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_lpc.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_lpc_ctx_t;

typedef int (*phy_type_lpc_init_fn_t)(phy_type_lpc_ctx_t *ctx);
typedef int (*phy_type_lpc_dump_fn_t)(phy_type_lpc_ctx_t *ctx, struct bcmstrbuf *b);
typedef uint8 (*phy_type_lpc_getminidx_fn_t)(void);
typedef void (*phy_type_lpc_setmode_fn_t)(phy_type_lpc_ctx_t *ctx, bool enable);
typedef uint8 (*phy_type_lpc_getpwros_fn_t)(uint8 index);
typedef uint8 (*phy_type_lpc_gettxcpwrval_fn_t)(uint16 phytxctrlword);
typedef void (*phy_type_lpc_settxcpwrval_fn_t)(uint16 *phytxctrlword, uint8 txcpwrval);
typedef uint8 (*phy_type_lpc_calcpwroffset_fn_t) (uint8 total_offset, uint8 rate_offset);
typedef uint8 (*phy_type_lpc_getpwridx_fn_t) (uint8 pwr_offset);
typedef uint8 * (*phy_type_lpc_getpwrlevelptr_fn_t) (void);

typedef struct {
	phy_type_lpc_ctx_t *ctx;

	phy_type_lpc_getminidx_fn_t		getminidx;
	phy_type_lpc_getpwros_fn_t		getpwros;
	phy_type_lpc_gettxcpwrval_fn_t		gettxcpwrval;
	phy_type_lpc_settxcpwrval_fn_t		settxcpwrval;
	phy_type_lpc_setmode_fn_t		setmode;
#ifdef WL_LPC_DEBUG
	phy_type_lpc_getpwrlevelptr_fn_t	getpwrlevelptr;
#endif // endif

} phy_type_lpc_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_lpc_register_impl(phy_lpc_info_t *cmn_info, phy_type_lpc_fns_t *fns);
void phy_lpc_unregister_impl(phy_lpc_info_t *cmn_info);

#endif /* _phy_type_lpc_h_ */
