
/*
 * RadarDetect module implementation - iovar table/handlers & registration
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
 * $Id: phy_radar_iov.c 753604 2018-03-22 08:20:27Z $
 */

#include <phy_cfg.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <bcmendian.h>

#include <phy_api.h>
#include "phy_radar_st.h"
#include <phy_radar_iov.h>

#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>
#include <phy_ac_info.h>

#ifndef ALL_NEW_PHY_MOD
#include <wlc_phy_int.h>
#endif // endif

/* id's */
enum {
	IOV_RADAR_ARGS = 1,
	IOV_RADAR_ARGS_40MHZ = 2,
	IOV_RADAR_THRS = 3,
	IOV_PHY_DFS_LP_BUFFER = 4,
	IOV_RADAR_STATUS = 5,
	IOV_CLEAR_RADAR_STATUS = 6,
	IOV_RADAR_THRS2 = 7,
	IOV_RADAR_SC_STATUS = 8,
	IOV_CLEAR_RADAR_SC_STATUS = 9,
	IOV_RADAR_SUBBAND_STATUS = 10

};

/* iovar table */
static const bcm_iovar_t phy_radar_iovt[] = {
	{"radarargs", IOV_RADAR_ARGS, (0), 0, IOVT_BUFFER, sizeof(wl_radar_args_t)},
	{"radarargs40", IOV_RADAR_ARGS_40MHZ, (0), 0, IOVT_BUFFER, sizeof(wl_radar_args_t)},
	{"radarthrs", IOV_RADAR_THRS, (IOVF_SET_UP), 0, IOVT_BUFFER, sizeof(wl_radar_thr_t)},
	{"radar_status", IOV_RADAR_STATUS, (0), 0, IOVT_BUFFER, sizeof(wl_radar_status_t)},
	{"clear_radar_status", IOV_CLEAR_RADAR_STATUS, (IOVF_SET_UP), 0, IOVT_BUFFER,
	sizeof(wl_radar_status_t)},
	{"radarthrs2", IOV_RADAR_THRS2, (IOVF_SET_UP), 0, IOVT_BUFFER, sizeof(wl_radar_thr2_t)},
	{"radar_sc_status", IOV_RADAR_SC_STATUS, (0), 0, IOVT_BUFFER, sizeof(wl_radar_status_t)},
	{"clear_radar_sc_status", IOV_CLEAR_RADAR_SC_STATUS, (IOVF_SET_UP), 0, IOVT_BUFFER,
	sizeof(wl_radar_status_t)},
	{"radar_subband_status", IOV_RADAR_SUBBAND_STATUS, (0), 0, IOVT_UINT16, 0},
#if defined(BCMDBG) || defined(WLTEST)
	{"phy_dfs_lp_buffer", IOV_PHY_DFS_LP_BUFFER, 0, 0, IOVT_UINT8, 0},
#endif // endif
	{NULL, 0, 0, 0, 0, 0}
};

#include <wlc_patch.h>

/* iovar handler */
static int
phy_radar_doiovar(void *ctx, uint32 aid, void *p, uint plen, void *a, uint alen, uint vsz,
	struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	phy_radar_info_t *ri = pi->radari;
	phy_radar_st_t *st = phy_radar_get_st(ri);
	int err = BCME_OK;
	int int_val = 0;
	bool bool_val;
	int subband_80p80 = 0, subband_160 = 0;

	/* The PHY type implemenation isn't registered */
	if (st == NULL) {
		PHY_ERROR(("%s: not supported\n", __FUNCTION__));
		return BCME_UNSUPPORTED;
	}

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	switch (aid) {
	case IOV_GVAL(IOV_RADAR_ARGS):
		bcopy(&st->rparams.radar_args, a, sizeof(wl_radar_args_t));
		break;

	case IOV_SVAL(IOV_RADAR_THRS): {
		wl_radar_thr_t radar_thr;

		/* len is check done before gets here */
		bzero(&radar_thr, sizeof(wl_radar_thr_t));
		bcopy(p, &radar_thr, sizeof(wl_radar_thr_t));
		err = phy_radar_set_thresholds(pi->radari, &radar_thr);
		break;
	}
	case IOV_SVAL(IOV_RADAR_ARGS): {
		wl_radar_args_t radarargs;

		if (!pi->sh->up) {
			err = BCME_NOTUP;
			break;
		}

		/* len is check done before gets here */
		bcopy(p, &radarargs, sizeof(wl_radar_args_t));
		if (radarargs.version != WL_RADAR_ARGS_VERSION) {
			err = BCME_VERSION;
			break;
		}
		bcopy(&radarargs, &st->rparams.radar_args, sizeof(wl_radar_args_t));
		/* apply radar inits to hardware if we are on the A/LP/NPHY */
		phy_radar_detect_enable(pi, pi->sh->radar);
		break;
	}
	case IOV_SVAL(IOV_PHY_DFS_LP_BUFFER):
		if (ISNPHY(pi) || ISACPHY(pi)) {
			pi->dfs_lp_buffer_nphy = bool_val;
			if (PHY_SUPPORT_SCANCORE(pi) ||
				PHY_SUPPORT_BW80P80(pi)) {
				pi->dfs_lp_buffer_nphy_sc = bool_val;
			}
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_RADAR_STATUS):
		if (ISNPHY(pi) || ISACPHY(pi)) {
			bcopy(&st->radar_status, a, sizeof(wl_radar_status_t));
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_CLEAR_RADAR_STATUS):
		if (ISNPHY(pi) || ISACPHY(pi)) {
			st->radar_status.detected = FALSE;
			st->radar_status.count = 0;
		} else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_GVAL(IOV_RADAR_THRS2):
		bcopy(&st->rparams.radar_thrs2, a, sizeof(wl_radar_thr2_t));
		break;

	case IOV_SVAL(IOV_RADAR_THRS2): {
		wl_radar_thr2_t radar_thr2;

		/* len is check done before gets here */
		bzero(&radar_thr2, sizeof(wl_radar_thr2_t));
		bcopy(p, &radar_thr2, sizeof(wl_radar_thr2_t));
		if (radar_thr2.version != WL_RADAR_THR_VERSION ||
			!(PHY_SUPPORT_SCANCORE(pi) ||
			PHY_SUPPORT_BW80P80(pi))) {
			err = BCME_VERSION;
			break;
		}
		if (ACMAJORREV_32(pi->pubpi->phy_rev) || ACMAJORREV_33(pi->pubpi->phy_rev) ||
			ACMAJORREV_47(pi->pubpi->phy_rev)) {
			st->rparams.radar_thrs2.thresh0_sc_20_lo =
				radar_thr2.thresh0_sc_20_lo;
			st->rparams.radar_thrs2.thresh1_sc_20_lo =
				radar_thr2.thresh1_sc_20_lo;
			st->rparams.radar_thrs2.thresh0_sc_20_hi =
				radar_thr2.thresh0_sc_20_hi;
			st->rparams.radar_thrs2.thresh1_sc_20_hi =
				radar_thr2.thresh1_sc_20_hi;
			st->rparams.radar_thrs2.thresh0_sc_40_lo =
				radar_thr2.thresh0_sc_40_lo;
			st->rparams.radar_thrs2.thresh1_sc_40_lo =
				radar_thr2.thresh1_sc_40_lo;
			st->rparams.radar_thrs2.thresh0_sc_40_hi =
				radar_thr2.thresh0_sc_40_hi;
			st->rparams.radar_thrs2.thresh1_sc_40_hi =
				radar_thr2.thresh1_sc_40_hi;
			st->rparams.radar_thrs2.thresh0_sc_80_lo =
				radar_thr2.thresh0_sc_80_lo;
			st->rparams.radar_thrs2.thresh1_sc_80_lo =
				radar_thr2.thresh1_sc_80_lo;
			st->rparams.radar_thrs2.thresh0_sc_80_hi =
				radar_thr2.thresh0_sc_80_hi;
			st->rparams.radar_thrs2.thresh1_sc_80_hi =
				radar_thr2.thresh1_sc_80_hi;
			st->rparams.radar_thrs2.thresh0_sc_160_lo =
				radar_thr2.thresh0_sc_160_lo;
			st->rparams.radar_thrs2.thresh1_sc_160_lo =
				radar_thr2.thresh1_sc_160_lo;
			st->rparams.radar_thrs2.thresh0_sc_160_hi =
				radar_thr2.thresh0_sc_160_hi;
			st->rparams.radar_thrs2.thresh1_sc_160_hi =
				radar_thr2.thresh1_sc_160_hi;
			st->rparams.radar_thrs2.fc_varth_sb =
				radar_thr2.fc_varth_sb;
			st->rparams.radar_thrs2.fc_varth_bin5_sb =
				radar_thr2.fc_varth_bin5_sb;
			st->rparams.radar_thrs2.notradar_enb =
				radar_thr2.notradar_enb;
			st->rparams.radar_thrs2.max_notradar_lp =
				radar_thr2.max_notradar_lp;
			st->rparams.radar_thrs2.max_notradar =
				radar_thr2.max_notradar;
			st->rparams.radar_thrs2.max_notradar_lp_sc =
				radar_thr2.max_notradar_lp_sc;
			st->rparams.radar_thrs2.max_notradar_sc =
				radar_thr2.max_notradar_sc;
			st->rparams.radar_thrs2.highpow_war_enb =
				radar_thr2.highpow_war_enb;
			st->rparams.radar_thrs2.highpow_sp_ratio =
				radar_thr2.highpow_sp_ratio;
			st->rparams.radar_thrs2.fm_chk_opt =
				radar_thr2.fm_chk_opt;
			st->rparams.radar_thrs2.fm_chk_pw =
				radar_thr2.fm_chk_pw;
			st->rparams.radar_thrs2.fm_var_chk_pw =
				radar_thr2.fm_var_chk_pw;
			st->rparams.radar_thrs2.fm_thresh_sp1 =
				radar_thr2.fm_thresh_sp1;
			st->rparams.radar_thrs2.fm_thresh_sp2 =
				radar_thr2.fm_thresh_sp2;
			st->rparams.radar_thrs2.fm_thresh_sp3 =
				radar_thr2.fm_thresh_sp3;
			st->rparams.radar_thrs2.fm_thresh_etsi4 =
				radar_thr2.fm_thresh_etsi4;
			st->rparams.radar_thrs2.fm_thresh_p1c =
				radar_thr2.fm_thresh_p1c;
			st->rparams.radar_thrs2.fm_tol_div =
				radar_thr2.fm_tol_div;
		}

		phy_radar_detect_enable(pi, pi->sh->radar);
		break;
	}
	case IOV_GVAL(IOV_RADAR_SC_STATUS):
		if (PHY_SUPPORT_SCANCORE(pi) ||
		    PHY_SUPPORT_BW80P80(pi)) {
			bcopy(st->radar_status_sc, a, sizeof(wl_radar_status_t));
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_RADAR_SUBBAND_STATUS):
		if (ISACPHY(pi)) {
			if (st->radar_work_lp_sc->subband_result == 0) {
				if (CHSPEC_IS160(pi->radio_chanspec) &&
					!PHY_AS_80P80(pi, pi->radio_chanspec)) {
					subband_160 = ((st->radar_lp_info.subband_result &
						0xf) << 4) + ((st->radar_lp_info.subband_result &
						0xf0) >> 4);
					bcopy(&subband_160, a, sizeof(int));
				} else {
					bcopy(&st->radar_lp_info.subband_result, a, sizeof(int));
				}
			} else {
				subband_80p80 = st->radar_lp_info.subband_result +
					(st->radar_work_lp_sc->subband_result << 4);
				bcopy(&subband_80p80, a, sizeof(int));
			}
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_RADAR_SUBBAND_STATUS):
		if (ISACPHY(pi)) {
			if (!pi->sh->up) {
				err = BCME_NOTUP;
				break;
			}

			st->radar_lp_info.subband_result = 0;
			st->radar_work_lp_sc->subband_result = 0;
		} else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_CLEAR_RADAR_SC_STATUS):
		if (PHY_SUPPORT_SCANCORE(pi) ||
		    PHY_SUPPORT_BW80P80(pi)) {
			st->radar_status_sc->detected = FALSE;
			st->radar_status_sc->count = 0;
		} else
			err = BCME_UNSUPPORTED;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* register iovar table/handlers to the system */
int
BCMATTACHFN(phy_radar_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
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

	wlc_iocv_init_iovd(phy_radar_iovt,
	                   phy_radar_pack_iov, phy_radar_unpack_iov,
	                   phy_radar_doiovar, disp_fn, patch_table, pi,
	                   &iovd);

	return wlc_iocv_register_iovt(ii, &iovd);
}
