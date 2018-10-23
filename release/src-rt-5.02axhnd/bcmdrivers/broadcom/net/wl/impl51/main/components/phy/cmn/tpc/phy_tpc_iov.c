/*
 * TxPowerControl module implementation - iovar table
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
 * $Id: phy_tpc_iov.c 691388 2017-03-22 02:17:00Z $
 */

#include <phy_tpc.h>
#include <phy_tpc_iov.h>
#include <phy_tpc_api.h>
#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>
#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_int.h>
#endif // endif
#include <phy_type_tpc.h>

/* iovar ids */
enum {
	IOV_PHY_FEM2G = 1,
	IOV_PHY_FEM5G = 2,
	IOV_PHY_SAR_LIMIT = 3,
	IOV_INITBASEIDX2G = 4,
	IOV_INITBASEIDX5G = 5,
	IOV_FCC_PWR_LIMIT_2G = 6,
	IOV_PAVARS = 7,
	IOV_PHY_VBAT_TEMPSENSE_PWRBACKOFF = 8,
	IOV_PHY_VBAT_PWRBACKOFF_THRESHOLD = 9,
	IOV_PHY_TEMPSENSE_PWRBACKOFF_THRESHOLD = 10
};

/* iovar table */
static const bcm_iovar_t phy_tpc_iovars[] = {
#ifdef FCC_PWR_LIMIT_2G
	{"fccpwrlimit2g", IOV_FCC_PWR_LIMIT_2G, 0, 0, IOVT_BOOL, 0},
#endif /* FCC_PWR_LIMIT_2G */
#ifdef WL_SARLIMIT
	{"phy_sarlimit", IOV_PHY_SAR_LIMIT, 0, 0, IOVT_UINT32, 0},
#endif /* WL_SARLIMIT */
#ifdef WLTEST
	{"fem2g", IOV_PHY_FEM2G, (IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, 0},
#ifdef BAND5G
	{"fem5g", IOV_PHY_FEM5G, (IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, 0},
#endif /* BAND5G */
#endif /* WLTEST */
#if defined(WLTEST)
	{"initbaseidx2g", IOV_INITBASEIDX2G, (IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0},
	{"initbaseidx5g", IOV_INITBASEIDX5G, (IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0},
	{"pavars", IOV_PAVARS,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, WL_PHY_PAVARS_LEN * sizeof(uint16)},
#endif // endif
#if defined(TXPWRBACKOFF) && defined(WLTEST)
	{"phy_vbat_tempsense_pwrbackoff", IOV_PHY_VBAT_TEMPSENSE_PWRBACKOFF,
	0, 0, IOVT_UINT16, 0
	},
#endif /* TXPWRBACKOFF && WLTEST */
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
phy_tpc_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	phy_info_t *pi = (phy_info_t *)ctx;
	bool bool_val = FALSE;
	int int_val = 0;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	(void)pi;
	(void)bool_val;
	(void)ret_int_ptr;

	switch (aid) {
#ifdef FCC_PWR_LIMIT_2G
	case IOV_GVAL(IOV_FCC_PWR_LIMIT_2G):
		int_val = (int32)pi->tpci->data->fcc_pwr_limit_2g;
		bcopy(&int_val, a, sizeof(int_val));
		break;
	case IOV_SVAL(IOV_FCC_PWR_LIMIT_2G):
		pi->tpci->data->fcc_pwr_limit_2g = bool_val;
		wlc_phy_fcc_pwr_limit_set((wlc_phy_t *)pi, bool_val);
		break;
#endif /* FCC_PWR_LIMIT_2G */
#ifdef WL_SARLIMIT
	case IOV_SVAL(IOV_PHY_SAR_LIMIT):
	{
		wlc_phy_sar_limit_set((wlc_phy_t*)pi, (uint32)int_val);
		break;
	}
#endif /* WL_SARLIMIT */
#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_FEM2G): {
		bcopy(pi->fem2g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM2G): {
		bcopy(p, pi->fem2g, sizeof(srom_fem_t));
		/* srom_fem2g.extpagain changed after attach time */
		phy_tpc_ipa_upd(pi->tpci);
		break;
	}
#ifdef BAND5G
	case IOV_GVAL(IOV_PHY_FEM5G): {
		bcopy(pi->fem5g, a, sizeof(srom_fem_t));
		break;
	}

	case IOV_SVAL(IOV_PHY_FEM5G): {
		bcopy(p, pi->fem5g, sizeof(srom_fem_t));
		/* srom_fem5g.extpagain changed after attach time */
		phy_tpc_ipa_upd(pi->tpci);
		break;
	}
#endif /* BAND5G */
#endif /* WLTEST */
#if defined(WLTEST)
	case IOV_GVAL(IOV_INITBASEIDX2G):
	{
		*ret_int_ptr = pi->tpci->data->cfg->initbaseidx2govrval;
		break;
	}
	case IOV_SVAL(IOV_INITBASEIDX2G):
	{
		pi->tpci->data->cfg->initbaseidx2govrval = (uint8)int_val;
		break;
	}
	case IOV_GVAL(IOV_INITBASEIDX5G):
	{
		*ret_int_ptr = pi->tpci->data->cfg->initbaseidx5govrval;
		break;
	}
	case IOV_SVAL(IOV_INITBASEIDX5G):
	{
		pi->tpci->data->cfg->initbaseidx5govrval = (uint8)int_val;
		break;
	}
	case IOV_GVAL(IOV_PAVARS):
		phy_tpc_get_pavars(pi->tpci, a, p);
		break;

	case IOV_SVAL(IOV_PAVARS):
		phy_tpc_set_pavars(pi->tpci, a, p);
		break;
#endif // endif

#if defined(TXPWRBACKOFF) && defined(WLTEST)
	case IOV_GVAL(IOV_PHY_VBAT_TEMPSENSE_PWRBACKOFF):
		err = phy_tpc_get_vt_pwrbackoff(pi->tpci, ret_int_ptr);
		break;
#endif /* TXPWRBACKOFF && WLTEST */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_tpc_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
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

	wlc_iocv_init_iovd(phy_tpc_iovars,
	                   NULL, NULL,
	                   phy_tpc_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
