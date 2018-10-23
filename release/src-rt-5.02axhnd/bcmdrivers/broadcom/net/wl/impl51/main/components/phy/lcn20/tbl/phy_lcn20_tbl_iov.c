/*
 * LCN20PHY PHYTblInit module implementation - iovar handlers & registration
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
 * $Id: phy_lcn20_tbl_iov.c 707224 2017-06-27 01:13:09Z $
 */

#include <typedefs.h>
#include <phy_api.h>
#include <phy_tbl_iov.h>
#include <phy_lcn20_tbl_iov.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>

#ifndef ALL_NEW_PHY_MOD
#include <wlc_phyreg_lcn20.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
#endif // endif

#include <phy_utils_reg.h>

static const bcm_iovar_t phy_lcn20_tbl_iovars[] = {
#if defined(WLTEST) || defined(DBG_PHY_IOV)
	{"phytable", IOV_PHYTABLE, IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG, 0, IOVT_BUFFER, 4*4},
#endif // endif
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
phy_lcn20_tbl_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	phytbl_info_t tab2;
	int err = BCME_OK;

	BCM_REFERENCE(tab2);
	BCM_REFERENCE(pi);

	switch (aid) {
#if defined(WLTEST)
	case IOV_GVAL(IOV_PHYTABLE):
		tab2.tbl_len = 1;
		tab2.tbl_id = *(uint32 *)p;
		tab2.tbl_offset = *((uint32 *)p + 1);
		tab2.tbl_width = *((uint32 *)p + 2);
		tab2.tbl_ptr = (uint32 *)a;
		wlc_lcn20phy_read_table(pi, &tab2);
		break;

	case IOV_SVAL(IOV_PHYTABLE):
		tab2.tbl_len = 1;
		tab2.tbl_id = *(uint32 *)p;
		tab2.tbl_offset = *((uint32 *)p + 1);
		tab2.tbl_width = *((uint32 *)p + 2);
		tab2.tbl_ptr = (uint32 *)p + 3;
		wlc_lcn20phy_write_table(pi, &tab2);
		break;
#endif // endif

	default:
#if defined(WLTEST)
		err = BCME_UNSUPPORTED;
#else
		err = BCME_OK;
#endif // endif
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_lcn20_tbl_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;
#if defined(WLC_PATCH_IOCTL)
	wlc_iov_disp_fn_t disp_fn = IOV_PATCH_FN;
	const bcm_iovar_t* patch_table = IOV_PATCH_TBL;
#else
	wlc_iov_disp_fn_t disp_fn = NULL;
	bcm_iovar_t* patch_table = NULL;
#endif /* WLC_PATCH_IOCTL */

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_lcn20_tbl_iovars,
	                   NULL, NULL,
	                   phy_lcn20_tbl_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
