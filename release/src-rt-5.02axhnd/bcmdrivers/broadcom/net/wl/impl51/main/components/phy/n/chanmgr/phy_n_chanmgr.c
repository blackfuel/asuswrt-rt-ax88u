/*
 * NPHY Channel Manager module implementation
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
 * $Id: phy_n_chanmgr.c 669330 2016-11-09 01:59:47Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_chanmgr.h>
#include "phy_type_chanmgr.h"
#include <phy_n.h>
#include <phy_n_chanmgr.h>
#include <bcmwifi_channels.h>

#ifndef ALL_NEW_PHY_MOD
/* < TODO: all these are going away... */
#include <wlc_phy_int.h>
/* TODO: all these are going away... > */
#endif // endif

/* module private states */
struct phy_n_chanmgr_info {
	phy_info_t *pi;
	phy_n_info_t *ni;
	phy_chanmgr_info_t *chanmgri;
};

/* Functions called using callback from Common layer */
static int phy_n_chanmgr_get_chanspec_bandrange(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static void phy_n_chanmgr_chanspec_set(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static void phy_n_chanmgr_upd_interf_mode(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec);
static uint8 phy_n_set_chanspec_sr_vsdb(phy_type_chanmgr_ctx_t *ctx,
		chanspec_t chanspec, uint8 *last_chan_saved);

/* Register/unregister NPHY specific implementation to common layer */
phy_n_chanmgr_info_t *
BCMATTACHFN(phy_n_chanmgr_register_impl)(phy_info_t *pi, phy_n_info_t *ni,
	phy_chanmgr_info_t *chanmgri)
{
	phy_n_chanmgr_info_t *info;
	phy_type_chanmgr_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage in once */
	if ((info = phy_malloc(pi, sizeof(phy_n_chanmgr_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	bzero(info, sizeof(phy_n_chanmgr_info_t));
	info->pi = pi;
	info->ni = ni;
	info->chanmgri = chanmgri;

	/* Register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.get_bandrange = phy_n_chanmgr_get_chanspec_bandrange;
	fns.chanspec_set = phy_n_chanmgr_chanspec_set;
	fns.interfmode_upd = phy_n_chanmgr_upd_interf_mode;
	fns.set_chanspec_sr_vsdb = phy_n_set_chanspec_sr_vsdb;

	fns.ctx = info;

	if (phy_chanmgr_register_impl(chanmgri, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_chanmgr_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return info;
fail:
	if (info != NULL)
		phy_mfree(pi, info, sizeof(phy_n_chanmgr_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_n_chanmgr_unregister_impl)(phy_n_chanmgr_info_t *info)
{
	phy_info_t *pi = info->pi;
	phy_chanmgr_info_t *chanmgri = info->chanmgri;

	phy_chanmgr_unregister_impl(chanmgri);

	phy_mfree(pi, info, sizeof(phy_n_chanmgr_info_t));
}

static int
phy_n_chanmgr_get_chanspec_bandrange(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	phy_n_chanmgr_info_t *info = (phy_n_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint channel = CHSPEC_CHANNEL(chanspec);
	return wlc_phy_get_chan_freq_range_nphy(pi, channel);
}

/* Set Channel Specification */
static void
phy_n_chanmgr_chanspec_set(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	phy_n_chanmgr_info_t *info = (phy_n_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;
	wlc_phy_chanspec_set_nphy(pi, chanspec);
}

static void
phy_n_chanmgr_upd_interf_mode(phy_type_chanmgr_ctx_t *ctx, chanspec_t chanspec)
{
	UNUSED_PARAMETER(ctx);
	UNUSED_PARAMETER(chanspec);
}

/**
 * Attempts to switch to the caller supplied chanspec using hardware assisted band switching. There
 * are conditions that determine whether such as hardware switch is possible, if it is not possible
 * (or there was an other problem preventing the channel switch) the function will return FALSE,
 * giving the caller an opportunity to invoke a subsequent non-SRVSDB assisted channel switch.
 *
 * Output argument 'last_chan_saved' is set to 'TRUE' when the context of the 'old' channel was
 * saved by hardware, and can even be set to 'TRUE' when the channel switch itself failed.
 *
 * Note: the *caller* should set *last_chan_saved to 'FALSE' before calling this function.
 */
static uint8
phy_n_set_chanspec_sr_vsdb(phy_type_chanmgr_ctx_t *ctx,
		chanspec_t chanspec, uint8 * last_chan_saved)
{
	uint8 switch_done = FALSE;
#ifdef WLSRVSDB
	phy_n_chanmgr_info_t *info = (phy_n_chanmgr_info_t *)ctx;
	phy_info_t *pi = info->pi;

	/* check if malloc for SW backup structures have failed */
	if (SRVSDSB_MEM_ALLOC_FAIL(pi))  {
		printf("SRVSDB : Mem alloc fail \n");
		ASSERT(0);
		goto exit;
	}

	/* While coming out of roam, its fresh start. Make status = 0 */
	if (ROAM_SRVSDB_RESET(pi)) {
		phy_reset_srvsdb_engine((wlc_phy_t*)pi);
	}

	/* reset fifo, disable stall */
	wlc_phy_srvsdb_prepare(pi, ENTER);

	/* If channel pattern matches, enter HW VSDB */
	if (sr_vsdb_switch_allowed(pi, chanspec)) {
		if (!wlc_set_chanspec_vsdb_phy(pi, chanspec)) {
			goto exit;
		}
		*last_chan_saved = TRUE;
		/*
		 * vsdb_trig_cnt tracks hardware state: the first two times the SR engine is
		 * triggered, it will save and restore (in one operation) the same context, so it
		 * will not switch context.
		 */
		if (pi->srvsdb_state->vsdb_trig_cnt > 1)
			switch_done =  TRUE;
	} else {
		switch_done = FALSE;
		goto exit;
	}

	/* to restore radio state */
	wlc_phy_resetcca_nphy(pi);

exit:
	wlc_phy_srvsdb_prepare(pi, EXIT);

	/* store present chanspec */
	pi->srvsdb_state->prev_chanspec = chanspec;
#endif /* WLSRVSDB */

	return switch_done;
}
