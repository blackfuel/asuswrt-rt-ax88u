/*
 * Named dump callback registry functions
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
 * $Id: wlc_dump_reg.h 613675 2016-01-19 19:24:39Z $
 */

#ifndef _wlc_dump_reg_h_
#define _wlc_dump_reg_h_

#include <typedefs.h>
#include <osl_decl.h>
#include <bcmutils.h>

/* forward declarataion */
typedef struct wlc_dump_reg_info wlc_dump_reg_info_t;

/* create a registry with 'count' entries */
wlc_dump_reg_info_t *wlc_dump_reg_create(osl_t *osh, uint16 count);

/* destroy a registry */
void wlc_dump_reg_destroy(wlc_dump_reg_info_t *reg);

/* dump callback function of registry */
typedef int (*wlc_dump_reg_dump_fn_t)(void *ctx, struct bcmstrbuf *b);
/* dump clear callback function of registry */
typedef int (*wlc_dump_reg_clr_fn_t)(void *ctx);
#define wlc_dump_reg_fn_t wlc_dump_reg_dump_fn_t

/* add a name and its callback functions to a registry */
int wlc_dump_reg_add_fns(wlc_dump_reg_info_t *reg, const char *name,
	wlc_dump_reg_dump_fn_t dump_fn, wlc_dump_reg_clr_fn_t clr_fn, void *ctx);
#define wlc_dump_reg_add_fn(reg, name, fn, ctx) \
	wlc_dump_reg_add_fns(reg, name, fn, NULL, ctx)

/* invoke a dump callback function in a registry by name */
int wlc_dump_reg_invoke_dump_fn(wlc_dump_reg_info_t *reg, const char *name,
	void *arg);
#define wlc_dump_reg_invoke_fn(reg, name, arg) \
	wlc_dump_reg_invoke_dump_fn(reg, name, arg)

/* invoke a dump clear callback function in a registry by name */
int wlc_dump_reg_invoke_clr_fn(wlc_dump_reg_info_t *reg, const char *name);

/* dump the registered names */
int wlc_dump_reg_list(wlc_dump_reg_info_t *reg, struct bcmstrbuf *b);
/* dump the registry internals */
int wlc_dump_reg_dump(wlc_dump_reg_info_t *reg, struct bcmstrbuf *b);

#endif /* _wlc_dump_reg_h_ */
