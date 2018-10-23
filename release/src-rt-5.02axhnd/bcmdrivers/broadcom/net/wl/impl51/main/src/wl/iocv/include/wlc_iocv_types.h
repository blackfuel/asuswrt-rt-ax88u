/*
 * IOCV module interface - basic data type declarations.
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
 * $Id: wlc_iocv_types.h 614277 2016-01-21 20:28:50Z $
 */

#ifndef _wlc_iocv_types_h_
#define _wlc_iocv_types_h_

#include <typedefs.h>

/* forward declarations */
typedef struct wlc_iocv_info wlc_iocv_info_t;

#define WLC_IOCV_NEW_IF

struct wlc_if;

/* IOVar dispatcher
 *
 * ctx - a pointer value registered with the function
 * aid - action ID, calculated by IOV_GVAL() and IOV_SVAL() based on varid.
 * p/plen - parameters and length for a get, input only.
 * a/alen - buffer and length for value to be set or retrieved, input or output.
 * vsz - value size, valid for integer type only.
 * wlcif - interface context (wlc_if pointer)
 *
 * p/a pointers may point into the same buffer.
 */
/* iovar dispatch function */
typedef int (*wlc_iov_disp_fn_t)(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsz, struct wlc_if *wlcif);
/* IOCtl dispatcher
 *
 * ctx - a pointer value registered with the function
 * cid - command/ioctl ID defined in $components/shared/devctrl_if/wlioctl_defs.h
 * a/alen - parameters and length and buffer and length for a GET command;
 *          parameters and length for a SET command
 * wlcif - interface context (wlc_if pointer)
 *
 * p/a pointers may point into the same buffer.
 */
/* ioctl dispatch function */
typedef int (*wlc_ioc_disp_fn_t)(void *ctx, uint32 cid,
	void *a, uint alen, struct wlc_if *wlcif);

#endif /* _wlc_iocv_types_h_ */
