/*
 * PATCH routines common hdr file
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
 * $Id: wlc_iocv_patch.h 680488 2017-01-20 05:25:18Z $
 */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations (since the generated
 * source file may reference private constants, types, variables, and functions); it also must be
 * included before wlc_module_register() call e.g. before xxxx_attach() function in MAC layer;
 * or it also must be included before wlc_iocv_register_iovt() or wlc_iocv_register_ioct() in
 * other layer.
 */

#ifndef _wlc_iocv_patch_h_
#define _wlc_iocv_patch_h_

#include <typedefs.h>
#include <bcmutils.h>

#include <wlc_iocv_types.h>

#ifdef WLC_PATCH_IOCTL

struct wlc_if;

int wlc_ioctl_patchmod(void *ctx, uint32 cmd, void *arg, uint len, struct wlc_if *wlcif);

/* Defaultly all patches will be defined to 'NULL', But this will be overridden by
 * actual patch generated during ROM OFFLOAD build from respective modules patch files.
 */
#define ROM_AUTO_PATCH_DOIOCTL NULL
#define ROM_AUTO_PATCH_DOIOVAR NULL
#define ROM_AUTO_PATCH_IOVARS NULL

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file. It must be
 * included after the prototypes above. The name of the included source file (WLC_PATCH_IOCTL_FILE)
 * is defined by the build environment.
 */
#if defined(WLC_PATCH_IOCTL_FILE)
	#include WLC_PATCH_IOCTL_FILE
#endif // endif

#define PATCH_IOCTL_FUNC_EXP(X)     X##_ioc_patch_func
#define PATCH_IOCTL_FUNC(X)         PATCH_IOCTL_FUNC_EXP(X)

#define PATCH_IOVAR_FUNC_EXP(X)     X##_patch_func
#define PATCH_IOVAR_FUNC(X)         PATCH_IOVAR_FUNC_EXP(X)

#define PATCH_IOVAR_TABLE_EXP(X)    X##_patch_table
#define PATCH_IOVAR_TABLE(X)        PATCH_IOVAR_TABLE_EXP(X)

/* For FULL ROM SW, redefine iovar patch macros with global patch pointers,
 * initialized with new patch table/func.
 * For NON-FULL ROM SW, redefine iovar patch macros with new patch table/func.
 */
#undef IOV_PATCH_TBL
#undef IOV_PATCH_FN
#undef IOC_PATCH_FN
#if defined(ROM_AUTO_IOCTL_PATCH_GLOBAL_PTRS)
	#define IOC_PATCH_FN	PATCH_IOCTL_FUNC(__FILENAME_NOEXTN__)
	#define IOV_PATCH_FN	PATCH_IOVAR_FUNC(__FILENAME_NOEXTN__)
	#define IOV_PATCH_TBL	PATCH_IOVAR_TABLE(__FILENAME_NOEXTN__)

	wlc_ioc_disp_fn_t IOC_PATCH_FN = ROM_AUTO_PATCH_DOIOCTL;
	wlc_iov_disp_fn_t IOV_PATCH_FN = ROM_AUTO_PATCH_DOIOVAR;
	bcm_iovar_t *IOV_PATCH_TBL = (bcm_iovar_t *)ROM_AUTO_PATCH_IOVARS;
#else /* !ROM_AUTO_IOCTL_PATCH_GLOBAL_PTRS */
	#define IOC_PATCH_FN	ROM_AUTO_PATCH_DOIOCTL
	#define IOV_PATCH_FN	ROM_AUTO_PATCH_DOIOVAR
	#define IOV_PATCH_TBL	ROM_AUTO_PATCH_IOVARS
#endif /* ROM_AUTO_IOCTL_PATCH_GLOBAL_PTRS */

#endif /* WLC_PATCH_IOCTL */

#endif /* _wlc_iocv_patch_h_ */
