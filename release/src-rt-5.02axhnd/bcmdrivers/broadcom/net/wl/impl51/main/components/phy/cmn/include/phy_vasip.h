/*
 * VASIP module interface (to other PHY modules).
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
 * $Id: phy_vasip.h 766999 2018-08-24 01:14:37Z $
 */

#ifndef _phy_vasip_h_
#define _phy_vasip_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_api.h>

typedef void phy_type_vasip_ctx_t;
typedef uint8 (*phy_type_vasip_get_ver_fn_t)(phy_type_vasip_ctx_t *ctx);
typedef void (*phy_type_vasip_reset_proc_fn_t)(phy_type_vasip_ctx_t *ctx, int reset);
typedef void (*phy_type_vasip_set_clk_t)(phy_type_vasip_ctx_t *ctx, bool val);
typedef void (*phy_type_vasip_write_bin_t)(phy_type_vasip_ctx_t *ctx, const uint32 vasip_code[],
		const uint nbytes);
#ifdef VASIP_SPECTRUM_ANALYSIS
typedef void (*phy_type_vasip_write_spectrum_tbl_t)(phy_type_vasip_ctx_t *ctx,
		const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl);
#endif // endif
typedef void (*phy_type_vasip_write_svmp_t)(phy_type_vasip_ctx_t *ctx,
		uint32 offset, uint16 val);
typedef void (*phy_type_vasip_read_svmp_t)(phy_type_vasip_ctx_t *ctx,
		const uint32 offset, uint16 *val);
typedef void (*phy_type_vasip_write_svmp_blk_t)(phy_type_vasip_ctx_t *ctx,
		uint32 offset, uint16 len, uint16 *val);
typedef void (*phy_type_vasip_read_svmp_blk_t)(phy_type_vasip_ctx_t *ctx,
		uint32 offset, uint16 len, uint16 *val);
#if defined(BCMDBG)
typedef void (*phy_type_vasip_dump_bfd_status_t)(phy_type_vasip_ctx_t *ctx,
		struct bcmstrbuf *b);
typedef void (*phy_type_vasip_dump_svmp_t)(phy_type_vasip_ctx_t *ctx,
		struct bcmstrbuf *b);
#endif // endif
typedef struct phy_vasip_info phy_vasip_info_t;
typedef struct phy_vasip_mem phy_vasip_mem_t;
typedef struct {
	phy_type_vasip_ctx_t *ctx;
	phy_type_vasip_get_ver_fn_t get_ver;
	phy_type_vasip_reset_proc_fn_t reset_proc;
	phy_type_vasip_set_clk_t set_clk;
	phy_type_vasip_write_bin_t write_bin;
#ifdef VASIP_SPECTRUM_ANALYSIS
	phy_type_vasip_write_spectrum_tbl_t write_spectrum_tbl;
#endif // endif
	phy_type_vasip_write_svmp_t	write_svmp;
	phy_type_vasip_read_svmp_t	read_svmp;
	phy_type_vasip_write_svmp_blk_t	write_svmp_blk;
	phy_type_vasip_read_svmp_blk_t	read_svmp_blk;
#if defined(BCMDBG)
	phy_type_vasip_dump_bfd_status_t dump_bfd_status;
	phy_type_vasip_dump_svmp_t dump_svmp;
#endif // endif
} phy_type_vasip_fns_t;

/* module private states */
struct phy_vasip_info {
	phy_info_t		*pi;	/* PHY info ptr */
	phy_type_vasip_fns_t	*fns;	/* PHY specific function ptrs */
	bool active;
	phy_vasip_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_vasip_mem {
	phy_vasip_info_t cmn_info;
	phy_type_vasip_fns_t fns;
/* add other variable size variables here at the end */
};

/* attach/detach */
phy_vasip_info_t *phy_vasip_attach(phy_info_t *pi);
void phy_vasip_detach(phy_vasip_info_t *cmn_info);

/*
 * Register/unregister PHY type implementation to the vasip module.
 * It returns BCME_XXXX.
 */
int phy_vasip_register_impl(phy_vasip_info_t *mi, phy_type_vasip_fns_t *fns);
void phy_vasip_unregister_impl(phy_vasip_info_t *cmn_info);

#endif /* _phy_vasip_h_ */
