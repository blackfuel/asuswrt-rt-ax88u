/*
 * BlueToothCoExistence module implementation - iovar handlers & registration
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
 * $Id: phy_btcx_iov.c 691048 2017-03-20 16:47:17Z $
 */

#include <phy_btcx_iov.h>
#include <phy_btcx.h>
#include <phy_btcx_api.h>
#include <wlc_iocv_reg.h>
#include <wlc_phy_int.h>

/* iovar ids */
enum {
	IOV_PHY_BTC_RESTAGE_RXGAIN = 1,
	IOV_PHY_LTECX_MODE = 2,
	IOV_PHY_BTC_PREEMPT_STATUS = 3,
	IOV_PHY_BTCOEX_DESENSE = 4,
	IOV_PHY_BTCOEX_DESENSE_RXGAIN = 5,
	IOV_PHY_BTC_SISO_ACK_PWR = 6,
	IOV_PHY_BTC_CURR_PWROFFSET = 7
};

static const bcm_iovar_t phy_btcx_iovars[] = {
	{"phy_btc_restage_rxgain", IOV_PHY_BTC_RESTAGE_RXGAIN, IOVF_SET_UP, 0, IOVT_UINT32, 0},
#if defined(WLTEST)
	{"phy_btc_preempt_status", IOV_PHY_BTC_PREEMPT_STATUS, 0, 0, IOVT_INT8, 0},
#endif // endif
#if !defined(WLC_DISABLE_ACI) && defined(BCMDBG)
	{"phy_btcoex_desense", IOV_PHY_BTCOEX_DESENSE, IOVF_SET_UP, 0, IOVT_INT32, 0},
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMDBG) */
#ifdef WLTEST
	{"phy_btcoex_desense_rxgain", IOV_PHY_BTCOEX_DESENSE_RXGAIN,
	IOVF_SET_UP, 0, IOVT_BUFFER, sizeof(wl_desense_restage_gain_t)
	},
#endif /* WLTEST */
	{"phy_btc_siso_ack_pwr", IOV_PHY_BTC_SISO_ACK_PWR,
	0, 0, IOVT_BUFFER, 2 * sizeof(int8)
	},
	{"phy_btc_curr_pwroffset", IOV_PHY_BTC_CURR_PWROFFSET,
	0, 0, IOVT_BUFFER, 3 * sizeof(int8)
	},
	{NULL, 0, 0, 0, 0, 0}
};

#include <wlc_patch.h>

static int
phy_btcx_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	BCM_REFERENCE(pi);
	BCM_REFERENCE(ret_int_ptr);

	switch (aid) {
	case IOV_GVAL(IOV_PHY_BTC_RESTAGE_RXGAIN):
		err = wlc_phy_iovar_get_btc_restage_rxgain(pi->btcxi, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_PHY_BTC_RESTAGE_RXGAIN):
		err = wlc_phy_iovar_set_btc_restage_rxgain(pi->btcxi, int_val);
		break;
#if defined(WLTEST)
	case IOV_GVAL(IOV_PHY_BTC_PREEMPT_STATUS):
		err = phy_btcx_get_preemptstatus(pi, ret_int_ptr);
		break;
#endif // endif
#if !defined(WLC_DISABLE_ACI) && defined(BCMDBG)
	case IOV_SVAL(IOV_PHY_BTCOEX_DESENSE):
	{
		err = phy_btcx_desense_btc(pi, int_val);
		break;
	}
#endif /* !defined(WLC_DISABLE_ACI) && defined(BCMDBG) */

#ifdef WL_UCM
#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_BTCOEX_DESENSE_RXGAIN):
	{
		wl_desense_restage_gain_t desense_restage_gain;
		err = phy_btcx_ucm_get_desense_rxgain(pi->btcxi, &desense_restage_gain);
		memcpy(a, &desense_restage_gain, sizeof(wl_desense_restage_gain_t));
		break;
	}
	case IOV_SVAL(IOV_PHY_BTCOEX_DESENSE_RXGAIN):
	{
		uint band;
		uint8 num_cores;
		uint8 *desense_array;
		wl_desense_restage_gain_t desense_restage_gain;
		memcpy(&desense_restage_gain, p, sizeof(wl_desense_restage_gain_t));
		band = desense_restage_gain.band;
		num_cores = desense_restage_gain.num_cores;
		desense_array = desense_restage_gain.desense_array;
		err = phy_btcx_ucm_set_desense_rxgain((wlc_phy_t*)pi, band,
				num_cores, desense_array);
		break;
	}
#endif /* WLTEST */
	case IOV_GVAL(IOV_PHY_BTC_SISO_ACK_PWR):
	{
		phy_btcx_ucm_get_siso_ack_pwr(pi->btcxi, a);
		break;
	}
	case IOV_SVAL(IOV_PHY_BTC_SISO_ACK_PWR):
	{
		int8 *resp_pwrs = (int8 *)p;
		phy_btcx_ucm_update_siso_resp_offset((wlc_phy_t *)pi, resp_pwrs, 2);
		break;
	}
	case IOV_GVAL(IOV_PHY_BTC_CURR_PWROFFSET):
	{
		phy_btcx_ucm_get_curr_siso_resp_pwr_offset(pi->btcxi, a);
		break;
	}
#endif /* WL_UCM */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_btcx_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
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

	wlc_iocv_init_iovd(phy_btcx_iovars,
	                   NULL, NULL,
	                   phy_btcx_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
