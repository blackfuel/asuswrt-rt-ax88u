/*
 * RSSICompute module implementation - iovar handlers & registration
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

#include <phy_rssi_iov.h>
#include <phy_rssi.h>
#include <wlc_iocv_reg.h>
#include <wlc_phy_int.h>

static const bcm_iovar_t phy_rssi_iovars[] = {
	{"phy_rssi_gain_delta_2g", IOV_PHY_RSSI_GAIN_DELTA_2G,
	(0), 0, IOVT_BUFFER, 18*sizeof(int8)},
#ifdef WLTEST
	{"phy_rssi_gain_delta_2gh", IOV_PHY_RSSI_GAIN_DELTA_2GH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_BUFFER, 18*sizeof(int8)},
	{"phy_rssi_gain_delta_2ghh", IOV_PHY_RSSI_GAIN_DELTA_2GHH,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_BUFFER, 18*sizeof(int8)},
#endif /* WLTEST */
	{"rssi_cal_freq_grp_2g", IOV_PHY_RSSI_CAL_FREQ_GRP_2G,
	(0), 0, IOVT_BUFFER, 7*sizeof(int8)},
	{"phy_rssi_gain_delta_2gb0", IOV_PHY_RSSI_GAIN_DELTA_2GB0,
	(0), 0, IOVT_BUFFER, sizeof(int8)},
	{"phy_rssi_gain_delta_2gb1", IOV_PHY_RSSI_GAIN_DELTA_2GB1,
	(0), 0, IOVT_BUFFER, sizeof(int8)},
	{"phy_rssi_gain_delta_2gb2", IOV_PHY_RSSI_GAIN_DELTA_2GB2,
	(0), 0, IOVT_BUFFER, sizeof(int8)},
	{"phy_rssi_gain_delta_2gb3", IOV_PHY_RSSI_GAIN_DELTA_2GB3,
	(0), 0, IOVT_BUFFER, sizeof(int8)},
	{"phy_rssi_gain_delta_2gb4", IOV_PHY_RSSI_GAIN_DELTA_2GB4,
	(0), 0, IOVT_BUFFER, sizeof(int8)},
	{"phy_rssi_gain_delta_5gl", IOV_PHY_RSSI_GAIN_DELTA_5GL,
	(0), 0, IOVT_BUFFER, 6*sizeof(int8)},
	{"phy_rssi_gain_delta_5gml", IOV_PHY_RSSI_GAIN_DELTA_5GML,
	(0), 0, IOVT_BUFFER, 6*sizeof(int8)},
	{"phy_rssi_gain_delta_5gmu", IOV_PHY_RSSI_GAIN_DELTA_5GMU,
	(0), 0, IOVT_BUFFER, 6*sizeof(int8)},
	{"phy_rssi_gain_delta_5gh", IOV_PHY_RSSI_GAIN_DELTA_5GH,
	(0), 0, IOVT_BUFFER, 6*sizeof(int8)},
#if defined(WLTEST)
	{"pkteng_stats", IOV_PKTENG_STATS,
	(IOVF_GET_UP | IOVF_MFG), 0, IOVT_BUFFER, sizeof(wl_pkteng_stats_t)},
#endif // endif
	{NULL, 0, 0, 0, 0, 0}
};

#include <wlc_patch.h>

static int
phy_rssi_doiovar(void *ctx, uint32 aid,
	void *p, uint plen, void *a, uint alen, uint vsize, struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	int err = BCME_OK;
	int8 *setValues = p, *getValues = a;

	switch (aid) {
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2G):
#ifdef WLTEST
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GHH):
#endif /* WLTEST */
			err = phy_rssi_set_gain_delta_2g(pi->rssii, aid, setValues);
			break;

		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2G):
#ifdef WLTEST
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GH):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GHH):
#endif /* WLTEST */
			err = phy_rssi_get_gain_delta_2g(pi->rssii, aid, getValues);
			break;

		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_5GH):
			err = phy_rssi_set_gain_delta_5g(pi->rssii, aid, setValues);
			break;

		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GL):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GML):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GMU):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_5GH):
			err = phy_rssi_get_gain_delta_5g(pi->rssii, aid, getValues);
			break;
#if defined(WLTEST)
		case IOV_GVAL(IOV_PKTENG_STATS):
			err = wlc_phy_pkteng_stats_get(pi->rssii, a, alen, getValues);
			break;
#endif // endif

		case IOV_SVAL(IOV_PHY_RSSI_CAL_FREQ_GRP_2G):
			err = phy_rssi_set_cal_freq_2g(pi->rssii, setValues);
			break;

		case IOV_GVAL(IOV_PHY_RSSI_CAL_FREQ_GRP_2G):
			err = phy_rssi_get_cal_freq_2g(pi->rssii, getValues);
			break;

		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3):
		case IOV_SVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB4):
			err = phy_rssi_set_gain_delta_2gb(pi->rssii, aid, setValues);
			break;

		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB0):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB1):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB2):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB3):
		case IOV_GVAL(IOV_PHY_RSSI_GAIN_DELTA_2GB4):
			err = phy_rssi_get_gain_delta_2gb(pi->rssii, aid, getValues);
			break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* register iovar table to the system */
int
BCMATTACHFN(phy_rssi_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
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

	wlc_iocv_init_iovd(phy_rssi_iovars,
	                   NULL, NULL,
	                   phy_rssi_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
