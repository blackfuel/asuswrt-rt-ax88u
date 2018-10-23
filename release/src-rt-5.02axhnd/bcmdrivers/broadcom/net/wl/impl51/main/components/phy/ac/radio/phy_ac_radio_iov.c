/*
 * ACPHY RADIO control module implementation - iovar handlers & registration
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
 * $Id: $
 */

#include <phy_ac_radio.h>
#include <phy_ac_info.h>
#include <phy_ac_radio_iov.h>

#include <wlc_iocv_reg.h>
#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_int.h>
#endif // endif
#include <phy_ac_info.h>

/* iovar ids */
enum {
	IOV_LP_MODE = 1,
	IOV_LP_VCO_2G = 2,
	IOV_RADIO_PD = 3
};

static const bcm_iovar_t phy_ac_radio_iovars[] = {
	{"lp_mode", IOV_LP_MODE, (IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0},
	{"lp_vco_2g", IOV_LP_VCO_2G, (IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0},
#if defined(WLTEST)
	{"radio_pd", IOV_RADIO_PD, (IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0},
#endif /* WLTEST */
	{NULL, 0, 0, 0, 0, 0}
};

#include <wlc_patch.h>

static int
phy_ac_radio_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int err = BCME_OK;
	int int_val = 0;
	int32 *ret_int_ptr = (int32 *)a;
	phy_ac_radio_info_t *radioi = pi->u.pi_acphy->radioi;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (aid) {
	case IOV_SVAL(IOV_LP_MODE):
	{
		if ((int_val > 3) || (int_val < 1)) {
			PHY_ERROR(("LP MODE %d is not supported \n", (uint16)int_val));
			err = BCME_RANGE;
		} else {
			wlc_phy_lp_mode(radioi, (int8) int_val);
		}
		break;
	}
	case IOV_GVAL(IOV_LP_MODE):
	{
		*ret_int_ptr = phy_ac_radio_get_data(radioi)->acphy_lp_status;
		break;
	}
	case IOV_SVAL(IOV_LP_VCO_2G):
	{
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			if ((int_val != 0) && (int_val != 1)) {
				PHY_ERROR(("LP MODE %d is not supported \n", (uint16)int_val));
				err = BCME_RANGE;
			} else {
				wlc_phy_force_lpvco_2G(pi, (int8) int_val);
			}
		} else {
			PHY_ERROR(("LP MODE is not supported for this chip \n"));
			err = BCME_UNSUPPORTED;
		}
		break;
	}
	case IOV_GVAL(IOV_LP_VCO_2G):
	{
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			*ret_int_ptr = phy_ac_radio_get_data(radioi)->acphy_force_lpvco_2G;
		} else {
			PHY_ERROR(("LP MODE is not supported for this chip \n"));
			err = BCME_UNSUPPORTED;
		}
		break;
	}
#if defined(WLTEST)
	case IOV_SVAL(IOV_RADIO_PD):
	{
		err = phy_ac_radio_set_pd(radioi, (uint16)int_val);
		break;
	}
	case IOV_GVAL(IOV_RADIO_PD):
	{
		*ret_int_ptr = phy_ac_radio_get_data(radioi)->acphy_4335_radio_pd_status;
		break;
	}
#endif /* WLTEST */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_ac_radio_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;
#if defined(WLC_PATCH_IOCTL)
	wlc_iov_disp_fn_t disp_fn = IOV_PATCH_FN;
	const bcm_iovar_t *patch_table = IOV_PATCH_TBL;
#else
	wlc_iov_disp_fn_t disp_fn = NULL;
	const bcm_iovar_t* patch_table = NULL;
#endif /* WLC_PATCH_IOCTL */

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_ac_radio_iovars,
	                   NULL, NULL,
	                   phy_ac_radio_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
