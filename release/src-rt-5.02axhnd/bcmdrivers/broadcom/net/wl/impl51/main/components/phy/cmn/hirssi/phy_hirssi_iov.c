/*
 * HiRSSI eLNA Bypass module implementation - iovar handlers & registration
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
 * $Id: phy_hirssi_iov.c 707224 2017-06-27 01:13:09Z $
 */

#include <phy_hirssi_iov.h>
#include <phy_hirssi.h>
#include <wlc_iocv_reg.h>

static const bcm_iovar_t phy_hirssi_iovars[] = {
#if defined(BCMDBG) || defined(WLTEST) || defined(PHYCAL_CHNG_CS)
	{"hirssi_period", IOV_HIRSSI_PERIOD, (IOVF_MFG), 0, IOVT_INT16, 0},
	{"hirssi_en", IOV_HIRSSI_EN, 0, 0, IOVT_UINT8, 0},
	{"hirssi_byp_rssi", IOV_HIRSSI_BYP_RSSI, (IOVF_MFG), 0, IOVT_INT8, 0},
	{"hirssi_res_rssi", IOV_HIRSSI_RES_RSSI, (IOVF_MFG), 0, IOVT_INT8, 0},
	{"hirssi_byp_w1cnt", IOV_HIRSSI_BYP_CNT, (IOVF_NTRL | IOVF_MFG), 0, IOVT_UINT16, 0},
	{"hirssi_res_w1cnt", IOV_HIRSSI_RES_CNT, (IOVF_NTRL | IOVF_MFG), 0, IOVT_UINT16, 0},
	{"hirssi_status", IOV_HIRSSI_STATUS, 0, 0, IOVT_UINT8, 0},
#endif /*  BCMDBG || WLTEST || PHYCAL_CHNG_CS */
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */

#include <wlc_patch.h>

static int
phy_hirssi_doiovar(void *ctx, uint32 aid,
		void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	BCM_REFERENCE(pi);
	BCM_REFERENCE(ret_int_ptr);

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (aid) {
#if defined(BCMDBG) || defined(WLTEST) || defined(PHYCAL_CHNG_CS)
		case IOV_GVAL(IOV_HIRSSI_PERIOD):
			err = phy_hirssi_get_period(pi, ret_int_ptr);
			break;
		case IOV_SVAL(IOV_HIRSSI_PERIOD):
			err = phy_hirssi_set_period(pi, int_val);
			break;
		case IOV_GVAL(IOV_HIRSSI_EN):
			err = phy_hirssi_get_en(pi, ret_int_ptr);
			break;
		case IOV_SVAL(IOV_HIRSSI_EN):
			err = phy_hirssi_set_en(pi, int_val);
			break;
		case IOV_GVAL(IOV_HIRSSI_BYP_RSSI):
			err = phy_hirssi_get_rssi(pi, ret_int_ptr, PHY_HIRSSI_BYP);
			break;
		case IOV_SVAL(IOV_HIRSSI_BYP_RSSI):
			err = phy_hirssi_set_rssi(pi, int_val, PHY_HIRSSI_BYP);
			break;
		case IOV_GVAL(IOV_HIRSSI_RES_RSSI):
			err = phy_hirssi_get_rssi(pi, ret_int_ptr, PHY_HIRSSI_RES);
			break;
		case IOV_SVAL(IOV_HIRSSI_RES_RSSI):
			err = phy_hirssi_set_rssi(pi, int_val, PHY_HIRSSI_RES);
			break;
		case IOV_GVAL(IOV_HIRSSI_BYP_CNT):
			err = phy_hirssi_get_cnt(pi, ret_int_ptr, PHY_HIRSSI_BYP);
			break;
		case IOV_SVAL(IOV_HIRSSI_BYP_CNT):
			err = phy_hirssi_set_cnt(pi, int_val, PHY_HIRSSI_BYP);
			break;
		case IOV_GVAL(IOV_HIRSSI_RES_CNT):
			err = phy_hirssi_get_cnt(pi, ret_int_ptr, PHY_HIRSSI_RES);
			break;
		case IOV_SVAL(IOV_HIRSSI_RES_CNT):
			err = phy_hirssi_set_cnt(pi, int_val, PHY_HIRSSI_RES);
			break;
		case IOV_GVAL(IOV_HIRSSI_STATUS):
			err = phy_hirssi_get_status(pi, ret_int_ptr);
			break;
#endif /*  BCMDBG || WLTEST || PHYCAL_CHNG_CS */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_hirssi_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;
#if defined(WLC_PATCH_IOCTL)
	wlc_iov_disp_fn_t disp_fn = IOV_PATCH_FN;
	const bcm_iovar_t* patch_table = IOV_PATCH_TBL;
#else
	wlc_iov_disp_fn_t disp_fn = NULL;
	const bcm_iovar_t* patch_table = NULL;
#endif /* WLC_PATCH_IOCTL */

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_hirssi_iovars,
	                   NULL, NULL,
	                   phy_hirssi_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
