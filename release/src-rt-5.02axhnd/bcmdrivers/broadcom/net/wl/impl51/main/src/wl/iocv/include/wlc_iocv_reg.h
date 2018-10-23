/*
 * IOCV module interface - ioctl/iovar table registration.
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
 * $Id: wlc_iocv_reg.h 660317 2016-09-20 05:54:54Z $
 */

#ifndef _wlc_iocv_reg_h_
#define _wlc_iocv_reg_h_

#include <typedefs.h>
#include <bcmutils.h>

#include <wlc_iocv_desc.h>
#include <wlc_iocv_types.h>

#ifdef WLC_PATCH_IOCTL
/* If we are building for NON-FULL ROM software, by default IOVAR patch
 * handling is disabled. These macros are overriden by the generated
 * IOVAR patch handler file if patching is enabled.
 * But if we are building FULL ROM software,
 * then these macros will be undef'd and redefined in 'wlc_patch.h' with global variables,
 * so that we can avoid abandoning of attach functions if patch is generated.
 */
#define IOV_PATCH_TBL NULL
#define IOV_PATCH_FN NULL
#define IOC_PATCH_FN NULL
#endif // endif

/* ==== For WLC modules use ==== */

/* register wlc iovar table & dispatcher */
int wlc_iocv_high_register_iovt(wlc_iocv_info_t *ii,
	const bcm_iovar_t *iovt, wlc_iov_disp_fn_t disp_fn,
#ifdef WLC_PATCH_IOCTL
	const bcm_iovar_t *patch_iovt, wlc_iov_disp_fn_t patch_disp_fn,
#endif // endif
	void *ctx);
/* register wlc ioctl table & dispatcher */
int wlc_iocv_high_register_ioct(wlc_iocv_info_t *ii,
	const wlc_ioctl_cmd_t *ioct, uint num_cmds, wlc_ioc_disp_fn_t disp_fn,
#ifdef WLC_PATCH_IOCTL
    wlc_ioc_disp_fn_t ioc_patch_fn,
#endif // endif
	void *ctx);

/* ioctl state validate function */
typedef int (*wlc_ioc_vld_fn_t)(void *ctx, uint32 cid,
	void *a, uint alen);

/* ==== For PHY/BMAC modules use ==== */

/* iovar table descriptor */
typedef struct {
	/* table pointer */
	const bcm_iovar_t *iovt;
	const bcm_iovar_t *patch_iovt;
	/* dispatch callback */
	wlc_iov_disp_fn_t disp_fn;
	wlc_iov_disp_fn_t patch_disp_fn;
	void *ctx;
} wlc_iovt_desc_t;

/* ioctl table descriptor */
typedef struct {
	/* table pointer */
	const wlc_ioctl_cmd_t *ioct;
	uint num_cmds;
	wlc_ioc_vld_fn_t st_vld_fn;
	/* dispatch callback */
	wlc_ioc_disp_fn_t disp_fn;
	wlc_ioc_disp_fn_t ioc_patch_fn;
	void *ctx;
} wlc_ioct_desc_t;

/* init iovar table desc */
#define _wlc_iocv_init_iovd_(\
	_iovt_, _cmdfn_, _resfn_, _dispfn_, _ptchdispfn_, _ptchtbl_, _ctx_, _iovd_) do { \
		(_iovd_)->iovt = _iovt_;				\
		(_iovd_)->patch_iovt = _ptchtbl_;			\
		(_iovd_)->disp_fn = _dispfn_;				\
		(_iovd_)->patch_disp_fn = _ptchdispfn_;			\
		(_iovd_)->ctx = _ctx_;					\
	} while (FALSE)

/* init iovar table descriptor */
#define wlc_iocv_init_iovd(iovt, cmdfn, resfn, dispfn, ptchdispfn, ptchtbl, ctx, iovd) \
	_wlc_iocv_init_iovd_(iovt, cmdfn, resfn, dispfn, ptchdispfn, ptchtbl, ctx, iovd)
/* register bmac/phy iovar table & callbacks */
int wlc_iocv_register_iovt(wlc_iocv_info_t *ii, wlc_iovt_desc_t *iovd);

/* init ioctl table desc */
#define _wlc_iocv_init_iocd_(_ioct_, _sz_, _vldfn_, _cmdfn_, _resfn_, _dispfn_, _ctx_, _iocd_) do {\
		(_iocd_)->ioct = _ioct_;				\
		(_iocd_)->num_cmds = _sz_;				\
		(_iocd_)->st_vld_fn = _vldfn_;				\
		(_iocd_)->disp_fn = _dispfn_;				\
		(_iocd_)->ctx = _ctx_;					\
	} while (FALSE)

/* init ioctl table descriptor */
#define wlc_iocv_init_iocd(ioct, sz, vldfn, cmdfn, resfn, dispfn, ctx, iocd) \
	_wlc_iocv_init_iocd_(ioct, sz, vldfn, cmdfn, resfn, dispfn, ctx, iocd)
/* register bmac/phy ioctl table & callbacks */
int wlc_iocv_register_ioct(wlc_iocv_info_t *ii, wlc_ioct_desc_t *iocd);

#endif /* wlc_iocv_reg_h_ */
