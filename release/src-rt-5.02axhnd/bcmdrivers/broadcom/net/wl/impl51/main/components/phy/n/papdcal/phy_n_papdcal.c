/*
 * N PHY PAPD CAL module implementation
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
 * $Id: phy_n_papdcal.c 639713 2016-05-24 18:02:57Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_type_papdcal.h>
#include <phy_n.h>
#include <phy_n_papdcal.h>

/* module private states */
struct phy_n_papdcal_info {
	phy_info_t			*pi;
	phy_n_info_t		*aci;
	phy_papdcal_info_t	*cmn_info;
/* add other variable size variables here at the end */
};

#ifdef WFD_PHY_LL
static void phy_ac_wd_wfd_ll(phy_type_papdcal_ctx_t *ctx);
static int phy_n_papdcal_set_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, uint8 int_val);
static int phy_n_papdcal_get_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, int32 *ret_int_ptr);
#endif /* WFD_PHY_LL */

/* register phy type specific implementation */
phy_n_papdcal_info_t *
BCMATTACHFN(phy_n_papdcal_register_impl)(phy_info_t *pi, phy_n_info_t *aci,
	phy_papdcal_info_t *cmn_info)
{
	phy_n_papdcal_info_t *ac_info;
	phy_type_papdcal_fns_t fns;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_n_papdcal_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize params */
	bzero(ac_info, sizeof(phy_n_papdcal_info_t));
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
#if defined(WFD_PHY_LL)
	fns.wd_wfd_ll = phy_n_wd_wfd_ll;
	fns.set_wfd_ll_enable = phy_n_papdcal_set_wfd_ll_enable;
	fns.get_wfd_ll_enable = phy_n_papdcal_get_wfd_ll_enable;
#endif /* WFD_PHY_LL */
	fns.ctx = ac_info;

	if (phy_papdcal_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_papdcal_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_n_papdcal_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_papdcal_unregister_impl)(phy_n_papdcal_info_t *ac_info)
{
	phy_papdcal_info_t *cmn_info = ac_info->cmn_info;
	phy_info_t *pi = ac_info->pi;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_papdcal_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_n_papdcal_info_t));
}

#ifdef WFD_PHY_LL
static void
phy_n_wd_wfd_ll(phy_type_papdcal_ctx_t *ctx)
{
	phy_n_papdcal_info_t *papdcali = (phy_n_papdcal_info_t *)ctx;
	phy_papdcal_data_t *data = papdcali->cmn_info->data;

	/* Be sure there is no cal in progress to enable/disable optimization */
	if (!PHY_PERICAL_MPHASE_PENDING(papdcali->pi)) {
		if (data->wfd_ll_enable != data->wfd_ll_enable_pending) {
			data->wfd_ll_enable = data->wfd_ll_enable_pending;
			if (!data->wfd_ll_enable) {
				/* Force a watchdog CAL when disabling WFD optimization
				 * As PADP CAL has not been executed since a long time
				 * a PADP CAL is executed at the next watchdog timeout
				 */
				papdcali->pi->cal_info->last_cal_time = 0;
			}
		}
	}
}
#endif /* WFD_PHY_LL */

#if defined(WFD_PHY_LL)
static int
phy_n_papdcal_set_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, uint8 int_val)
{
	phy_n_papdcal_info_t *papdcali = (phy_n_papdcal_info_t *)ctx;
	phy_papdcal_data_t *data = papdcali->cmn_info->data;

	/* Force the channel to be active */
	data->wfd_ll_chan_active_force = (int_val == 2) ? TRUE : FALSE;
	data->wfd_ll_enable_pending = int_val;
	if (!PHY_PERICAL_MPHASE_PENDING(papdcali->pi)) {
		/* Apply it since there is no CAL in progress */
		data->wfd_ll_enable = int_val;
		if (!int_val) {
			/* Force a watchdog CAL when disabling WFD optimization
			 * As PADP CAL has not been executed since a long time
			 * a PADP CAL is executed at the next watchdog timeout
			 */
			 papdcali->pi->cal_info->last_cal_time = 0;
		}
	}
	return BCME_OK;
}

static int
phy_n_papdcal_get_wfd_ll_enable(phy_type_papdcal_ctx_t *ctx, int32 *ret_int_ptr)
{
	phy_n_papdcal_info_t *papdcali = (phy_n_papdcal_info_t *)ctx;
	*ret_int_ptr = papdcali->cmn_info->data->wfd_ll_enable;
	return BCME_OK;
}
#endif /* WFD_PHY_LL */
