/*
 * PAPD CAL module implementation - iovar handlers & registration
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
 * $Id: phy_papdcal_iov.c 680443 2017-01-20 00:16:59Z $
 */

#include <phy_papdcal_iov.h>
#include <phy_papdcal.h>
#include <phy_type_papdcal.h>
#include <wlc_phyreg_ac.h>
#include <phy_utils_reg.h>
#include <wlc_iocv_reg.h>
#include <wlc_phy_int.h>

static const bcm_iovar_t phy_papdcal_iovars[] = {
#if defined(WLTEST) || defined(BCMDBG)
	{"phy_enable_epa_dpd_2g", IOV_PHY_ENABLE_EPA_DPD_2G, IOVF_SET_UP, 0, IOVT_INT8, 0},
	{"phy_enable_epa_dpd_5g", IOV_PHY_ENABLE_EPA_DPD_5G, IOVF_SET_UP, 0, IOVT_INT8, 0},
	{"phy_epacal2gmask", IOV_PHY_EPACAL2GMASK, 0, 0, IOVT_INT16, 0},
#endif /* defined(WLTEST) || defined(BCMDBG) */

#if defined(WLTEST)
	{"phy_pacalidx0", IOV_PHY_PACALIDX0, (IOVF_GET_UP | IOVF_MFG), 0, IOVT_UINT32, 0},
	{"phy_pacalidx1", IOV_PHY_PACALIDX1, (IOVF_GET_UP | IOVF_MFG), 0, IOVT_UINT32, 0},
	{"phy_pacalidx", IOV_PHY_PACALIDX, (IOVF_GET_UP | IOVF_MFG), 0, IOVT_UINT32, 0},
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || \
	defined(ATE_BUILD)
	{"phy_papd_en_war", IOV_PAPD_EN_WAR, (IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0},
#ifndef ATE_BUILD
	{"phy_skippapd", IOV_PHY_SKIPPAPD, (IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_UINT8, 0},
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WFD_PHY_LL)
	{"phy_wfd_ll_enable", IOV_PHY_WFD_LL_ENABLE, 0, 0, IOVT_UINT8, 0},
#endif /* WFD_PHY_LL */
	{NULL, 0, 0, 0, 0, 0}
};

#include <wlc_patch.h>

static int
phy_papdcal_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	BCM_REFERENCE(*pi);
	BCM_REFERENCE(*ret_int_ptr);

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	switch (aid) {
#if defined(WLTEST) || defined(BCMDBG)
		case IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_2G):
		case IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_5G):
		{
			if ((int_val < 0) || (int_val > 1)) {
				err = BCME_RANGE;
				PHY_ERROR(("Value out of range\n"));
				break;
			}
			phy_papdcal_epa_dpd_set(pi, (uint8)int_val,
				(aid == IOV_SVAL(IOV_PHY_ENABLE_EPA_DPD_2G)));
			break;
		}
		case IOV_GVAL(IOV_PHY_EPACAL2GMASK): {
			*ret_int_ptr = (uint32)pi->papdcali->data->epacal2g_mask;
			break;
		}

		case IOV_SVAL(IOV_PHY_EPACAL2GMASK): {
			pi->papdcali->data->epacal2g_mask = (uint16)int_val;
			break;
		}
#endif /* defined(WLTEST) || defined(BCMDBG) */

#if defined(WLTEST)
		case IOV_GVAL(IOV_PHY_PACALIDX0):
			err = phy_papdcal_get_lut_idx0(pi, ret_int_ptr);
			break;

		case IOV_GVAL(IOV_PHY_PACALIDX1):
			err = phy_papdcal_get_lut_idx1(pi, ret_int_ptr);
			break;

		case IOV_SVAL(IOV_PHY_PACALIDX):
			err = phy_papdcal_set_idx(pi, (int8)int_val);
			break;
#endif // endif
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || \
	defined(ATE_BUILD)
		case IOV_SVAL(IOV_PAPD_EN_WAR):
			wlapi_bmac_write_shm(pi->sh->physhim, M_PAPDOFF_MCS(pi), (uint16)int_val);
			break;

		case IOV_GVAL(IOV_PAPD_EN_WAR):
			*ret_int_ptr = wlapi_bmac_read_shm(pi->sh->physhim, M_PAPDOFF_MCS(pi));
			break;
#ifndef ATE_BUILD
		case IOV_SVAL(IOV_PHY_SKIPPAPD):
			if ((int_val != 0) && (int_val != 1)) {
				err = BCME_RANGE;
				break;
			}
			err = phy_papdcal_set_skip(pi, (uint8)int_val);
			break;

		case IOV_GVAL(IOV_PHY_SKIPPAPD):
			err = phy_papdcal_get_skip(pi, ret_int_ptr);
			break;
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WFD_PHY_LL)
		case IOV_SVAL(IOV_PHY_WFD_LL_ENABLE):
			if ((int_val < 0) || (int_val > 2)) {
				err = BCME_RANGE;
			} else {
				err = phy_papdcal_set_wfd_ll_enable(pi->papdcali, (uint8) int_val);
			}
			break;

		case IOV_GVAL(IOV_PHY_WFD_LL_ENABLE):
			err = phy_papdcal_get_wfd_ll_enable(pi->papdcali, ret_int_ptr);
			break;
#endif /* WFD_PHY_LL */
		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_papdcal_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
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

	wlc_iocv_init_iovd(phy_papdcal_iovars,
	                   NULL, NULL,
	                   phy_papdcal_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
