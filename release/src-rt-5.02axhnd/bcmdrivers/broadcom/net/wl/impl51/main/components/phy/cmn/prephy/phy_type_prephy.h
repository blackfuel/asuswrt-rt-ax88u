/*
 * Prephy module internal interface (to PHY specific implementations).
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
 * $Id: phy_type_prephy.h 657104 2016-08-31 03:30:45Z $
 */

#ifndef _phy_type_prephy_h_
#define _phy_type_prephy_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_prephy.h>

/*
 * PHY type implementation interface.
 *
 * Each PHY type implements the following functionality and registers the functions
 * via a vtbl/ftbl defined below, along with a context 'ctx' pointer.
 */
typedef void phy_type_prephy_ctx_t;

typedef void (*phy_type_prephy_vasip_ver_get_t)(prephy_info_t *pi,
	d11regs_t *regs, uint32 *vasipver);
typedef void (*phy_type_prephy_vasip_proc_reset_t)(prephy_info_t *pi,
	d11regs_t *regs, bool reset);
typedef void (*phy_type_prephy_vasip_clk_set_t)(prephy_info_t *pi,
	d11regs_t *regs, bool set);
typedef uint32 (*phy_type_prephy_caps_t)(prephy_info_t *pi, uint32 *pacaps);

typedef struct {
	phy_type_prephy_ctx_t *ctx;
	phy_type_prephy_vasip_ver_get_t	phy_type_prephy_vasip_ver_get;
	phy_type_prephy_vasip_proc_reset_t phy_type_prephy_vasip_proc_reset;
	phy_type_prephy_vasip_clk_set_t phy_type_prephy_vasip_clk_set;
	phy_type_prephy_caps_t phy_type_prephy_caps;
} phy_type_prephy_fns_t;

/*
 * Register/unregister PHY type implementation to the MultiPhaseCal module.
 * It returns BCME_XXXX.
 */
int phy_prephy_register_impl(phy_prephy_info_t *mi, phy_type_prephy_fns_t *fns);
void phy_prephy_unregister_impl(phy_prephy_info_t *cmn_info);

#endif /* _phy_type_prephy_h_ */
