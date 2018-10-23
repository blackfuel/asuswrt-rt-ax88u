/*
 * CHanSPEC manipulation module implementation.
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
 * $Id: phy_chanmgr.c 766664 2018-08-09 08:50:20Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_init.h>
#include "phy_type_chanmgr.h"
#include <phy_chanmgr_notif.h>
#include <phy_chanmgr_notif_priv.h>
#include <phy_chanmgr_api.h>
#include <phy_chanmgr.h>
#include <phy_tpc.h>
#include <sbchipc.h>
#include <saverestore.h>

#define CHANNEL_UNDEFINED       0xEF

/* forward declaration */
typedef struct phy_chanmgr_mem phy_chanmgr_mem_t;

/* module private states */
struct phy_chanmgr_info {
	phy_info_t				*pi;		/* PHY info ptr */
	phy_type_chanmgr_fns_t	*fns;		/* PHY specific function ptrs */
	phy_chanmgr_mem_t		*mem;		/* Memory layout ptr */
	uint16	home_chanspec;				/* holds operating/home chanspec */
	uint16	prev_chanspec;				/* holds previous chanspec */
};

/* module private states memory layout */
struct phy_chanmgr_mem {
	phy_chanmgr_info_t		info;
	phy_type_chanmgr_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */
static int phy_chanmgr_set_bw(phy_init_ctx_t *ctx);
static int phy_chanmgr_init_chanspec(phy_init_ctx_t *ctx);

/* attach/detach */
phy_chanmgr_info_t *
BCMATTACHFN(phy_chanmgr_attach)(phy_info_t *pi)
{
	phy_chanmgr_mem_t	*mem = NULL;
	phy_chanmgr_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_chanmgr_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	/* Initialize infra params */
	cmn_info = &(mem->info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize chanmgr params */

	/* register init fns */
	if (phy_init_add_init_fn(pi->initi, phy_chanmgr_set_bw,
		cmn_info, PHY_INIT_CHBW) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn set_bw failed\n", __FUNCTION__));
		goto fail;
	}

	if (phy_init_add_init_fn(pi->initi, phy_chanmgr_init_chanspec,
		cmn_info, PHY_INIT_CHSPEC) != BCME_OK) {
		PHY_ERROR(("%s: phy_init_add_init_fn init_chanspec failed\n", __FUNCTION__));
		goto fail;
	}

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_chanmgr_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_chanmgr_detach)(phy_chanmgr_info_t *cmn_info)
{
	phy_chanmgr_mem_t *mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	if (!cmn_info)
		return;

	/* Memory associated with cmn_info must be cleaned up. */
	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null chanmgr module\n", __FUNCTION__));
		return;
	}
	phy_mfree(cmn_info->pi, mem, sizeof(phy_chanmgr_mem_t));
}

/* init bandwidth in h/w */
static int
WLBANDINITFN(phy_chanmgr_set_bw)(phy_init_ctx_t *ctx)
{
	phy_chanmgr_info_t *ti = (phy_chanmgr_info_t *)ctx;
	phy_info_t *pi = ti->pi;

	PHY_TRACE(("%s\n", "phy_chanmgr_set_bw"));

	/* sanitize bw here to avoid later mess. wlc_set_bw will invoke phy_reset,
	 *  but phy_init recursion is avoided by using init_in_progress
	 */
	if (CHSPEC_BW(pi->radio_chanspec) != pi->bw)
		wlapi_bmac_bw_set(pi->sh->physhim, CHSPEC_BW(pi->radio_chanspec));

	if (wlapi_bmac_bw_check(pi->sh->physhim) == BCME_BADCHAN) {
		PHY_ERROR(("PANIC! CHANSPEC_BW doesn't match with si_core_cflags BW bits\n"));
		ASSERT(0);
	}

	return BCME_OK;
}

/* Channel Manager Channel Specification Initialization */
static int
WLBANDINITFN(phy_chanmgr_init_chanspec)(phy_init_ctx_t *ctx)
{
	phy_chanmgr_info_t *chanmgri = (phy_chanmgr_info_t *)ctx;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;

	PHY_TRACE(("%s\n", "phy_chanmgr_init_chanspec"));

	if (fns->init_chanspec != NULL) {
		return (fns->init_chanspec)(fns->ctx);
	}
	return BCME_OK;
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_chanmgr_register_impl)(phy_chanmgr_info_t *ti, phy_type_chanmgr_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*ti->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_chanmgr_unregister_impl)(phy_chanmgr_info_t *ti)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* band specific init */
int
WLBANDINITFN(phy_chanmgr_bsinit)(phy_info_t *pi, chanspec_t chanspec, bool forced)
{
	/* if chanswitch path, skip phy_init for D11REV > 40 */
	phy_type_chanmgr_fns_t *fns = pi->chanmgri->fns;
	if (fns->bsinit != NULL) {
		return (fns->bsinit)(fns->ctx, chanspec, forced);
	} else {
		return phy_init(pi, chanspec);
	}
}

/* band width init */
int
WLBANDINITFN(phy_chanmgr_bwinit)(phy_info_t *pi, chanspec_t chanspec)
{
	phy_type_chanmgr_fns_t *fns = pi->chanmgri->fns;
	if (fns->bwinit != NULL) {
		return (fns->bwinit)(fns->ctx, chanspec);
	} else {
		return phy_init(pi, chanspec);
	}
}

#if defined(PHYCAL_CACHING)
/*
 * Create/Destroy an operating chanspec context for 'chanspec'.
 */
int
phy_chanmgr_create_ctx(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("phy_chanmgr_create_ctx: chanspec 0x%x\n", chanspec));

	data.event = PHY_CHANMGR_NOTIF_OPCHCTX_OPEN;
	data.new = chanspec;

	return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, TRUE);
}

void
phy_chanmgr_destroy_ctx(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("phy_chanmgr_destroy_ctx: chanspec 0x%x\n", chanspec));

	data.event = PHY_CHANMGR_NOTIF_OPCHCTX_CLOSE;
	data.new = chanspec;

	(void)phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, FALSE);
}
#endif /* PHYCAL_CACHING */

bool
wlc_phy_is_txbfcal(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;

	if (fns->is_txbfcal != NULL) {
		return (fns->is_txbfcal)(fns->ctx);
	} else {
		return FALSE;
	}
}

bool
wlc_phy_is_smth_en(wlc_phy_t *ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;

	if ((fns->is_smth_en != NULL) && ISACPHY(pi)) {
		return (fns->is_smth_en)(fns->ctx);
	} else {
		return FALSE;
	}
}

int
wlc_phy_chanspec_bandrange_get(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;
	int range = -1;

	if (fns->get_bandrange != NULL) {
		range = (fns->get_bandrange)(fns->ctx, chanspec);
	} else
		ASSERT(0);

	return range;
}

#if defined(PHYCAL_CACHING)
/*
 * Use the operating chanspec context associated with the 'chanspec' as
 * the current operating chanspec context.
 *
 * This function changes the current radio chanspec and applies
 * s/w properties to related h/w.
 */
int
phy_chanmgr_set_oper(phy_info_t *pi, chanspec_t chanspec)
{
	/* This API is called or must be called after wlc_phy_chanspec_set */
	ASSERT(pi->radio_chanspec == chanspec);
	if (chanspec != pi->chanmgri->prev_chanspec) {
		phy_chanmgr_notif_data_t data;
		PHY_TRACE(("phy_chanmgr_set_oper: 0x%x\n", chanspec));

		data.event = PHY_CHANMGR_NOTIF_OPCH_CHG;
		data.new = chanspec;
		data.old = pi->chanmgri->prev_chanspec;

		return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, TRUE);
	} else {
		return BCME_OK; /* Channel has not changed; do nothing */
	}
}

/*
 * Set the radio chanspec to the 'chanspec' and invalidate the current
 * operating chanspec context if any.
 *
 * This function only changes the current radio chanspec.
 */
int
phy_chanmgr_set(phy_info_t *pi, chanspec_t chanspec)
{
	phy_chanmgr_notif_data_t data;

	PHY_TRACE(("phy_chanmgr_set: 0x%x\n", chanspec));

	data.event = PHY_CHANMGR_NOTIF_CH_CHG;
	data.new = chanspec;
	data.old = pi->radio_chanspec;

	return phy_chanmgr_notif_signal(pi->chanmgr_notifi, &data, FALSE);
}
#endif /* PHYCAL_CACHING */

/*
 * Update the radio chanspec.
 */
void
wlc_phy_chanspec_radio_set(wlc_phy_t *ppi, chanspec_t newch)
{
	phy_info_t *pi = (phy_info_t*)ppi;
#ifdef PREASSOC_PWRCTRL
	phy_preassoc_pwrctrl_upd(pi, newch);
#endif /* PREASSOC_PWRCTRL */
	/* update prev_chanspec with current chanspec */
	pi->chanmgri->prev_chanspec = pi->radio_chanspec;
	/* update radio_chanspec */
	pi->radio_chanspec = newch;
}

/*
 * Set the chanspec.
 */

void
wlc_phy_chanspec_set(wlc_phy_t *ppi, chanspec_t chanspec)
{
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;

	if (!SCAN_RM_IN_PROGRESS(pi) &&	(chanmgri->home_chanspec != chanspec))
		chanmgri->home_chanspec = chanspec;

	PHY_TRACE(("wl%d: wlc_phy_chanspec_set: %x\n", pi->sh->unit, chanspec));

	ASSERT(!wf_chspec_malformed(chanspec));

	PHY_CHANLOG(pi, __FUNCTION__, TS_ENTER, 0);

#ifdef WLSRVSDB
	pi->srvsdb_state->prev_chanspec = chanspec;
#endif /* WLSRVSDB */

	/* Update ucode channel value */
	phy_chanmgr_set_shm(pi, chanspec);

#if defined(AP) && defined(RADAR)
	/* indicate first time radar detection */
	if (pi->radari != NULL)
		phy_radar_first_indicator_set(pi);
#endif // endif
	/* Update interference mode for ACPHY, as now init is not called on band/bw change */
	if ((!SCAN_RM_IN_PROGRESS(pi)) && (fns->interfmode_upd != NULL)) {
		fns->interfmode_upd(fns->ctx, chanspec);
	}

	ASSERT(pi->cal_info);

	if (fns->chanspec_set != NULL) {
#ifdef ENABLE_FCBS
		if (IS_FCBS(pi)) {
			int chanidx;

			if (wlc_phy_is_fcbs_chan(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
				wlc_phy_fcbs((wlc_phy_t*)pi, chanidx, 1);
				/* Need to set this to indicate that we have switched to new chan
				 * not needed for SW-based FCBS ???
				 */
				pi->phy_fcbs->FCBS_INPROG = 1;
				wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);
			} else {
				/* The conditioning here matches that below for FCBS init. Hence, if
				 * there is a pending FCBS init on this channel and if HW_FCBS, then
				 * we want to setup the channel-indicator bit, etc. appropriately
				 * before we call phy_specific chanspec_set
				 */
				if (wlc_phy_is_fcbs_pending(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
						phy_fcbs_preinit(pi, chanidx);
				}

				(fns->chanspec_set)(fns->ctx, chanspec);

				/* Now that we are on new channel, check for a pending request */
				if (wlc_phy_is_fcbs_pending(pi, chanspec, &chanidx) &&
				!(SCAN_INPROG_PHY(pi) || RM_INPROG_PHY(pi) || PLT_INPROG_PHY(pi))) {
					wlc_phy_fcbs_init((wlc_phy_t*)pi, chanidx);
				} else {
					chanidx = wlc_phy_channelindicator_obtain(pi);
					pi->phy_fcbs->initialized[chanidx] = FALSE;
				}
			}
		} else {
			(fns->chanspec_set)(fns->ctx, chanspec);
		}
#else
		(fns->chanspec_set)(fns->ctx, chanspec);
#endif /* ENABLE_FCBS */
	}
	if (fns->preempt != NULL) {
		(fns->preempt)(fns->ctx, ((pi->sh->interference_mode & ACPHY_ACI_PREEMPTION) != 0),
			FALSE);
	}

	wlapi_update_bt_chanspec(pi->sh->physhim, chanspec,
		SCAN_INPROG_PHY(pi), RM_INPROG_PHY(pi));
	PHY_CHANLOG(pi, __FUNCTION__, TS_EXIT, 0);
}

#if defined(WLTEST)
int
phy_chanmgr_get_smth(phy_info_t *pi, int32* ret_int_ptr)
{
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;
	if (fns->get_smth != NULL) {
		return (fns->get_smth)(fns->ctx, ret_int_ptr);
	} else {
		return BCME_UNSUPPORTED;
	}
}

int
phy_chanmgr_set_smth(phy_info_t *pi, int8 int_val)
{
	phy_chanmgr_info_t *chanmgri = pi->chanmgri;
	phy_type_chanmgr_fns_t *fns = chanmgri->fns;
	if (fns->set_smth != NULL) {
		return (fns->set_smth)(fns->ctx, int_val);
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif /* defined(WLTEST) */

void
phy_chanmgr_tdcs_enable_160m(phy_info_t *pi, bool set_val)
{
	phy_type_chanmgr_fns_t *fns = pi->chanmgri->fns;

	if (fns->tdcs_enable_160m != NULL)
		(fns->tdcs_enable_160m)(pi, set_val);
}

/* **************************************** */
/*     VSDB, RVSDB Module related definitions         */
/* **************************************** */
/* ***               Internal functions  (PHY use)    *** */

#ifdef WLSRVSDB
void
phy_sr_vsdb_reset(wlc_phy_t * ppi)
{
	phy_info_t *pi = (phy_info_t*)ppi;

	pi->srvsdb_state->swbkp_snapshot_valid[0] = 0;
	pi->srvsdb_state->swbkp_snapshot_valid[1] = 0;
	pi->srvsdb_state->sr_vsdb_bank_valid[0] = FALSE;
	pi->srvsdb_state->sr_vsdb_bank_valid[1] = FALSE;
	pi->srvsdb_state->prev_chanspec = CHANNEL_UNDEFINED;
	pi->srvsdb_state->vsdb_trig_cnt = 0;
}

void
phy_reset_srvsdb_engine(wlc_phy_t *ppi)
{
	/** reset SR VSDB hw */
	uint origidx;
	chipcregs_t *cc;
	si_t *sih;
	phy_info_t *pi = (phy_info_t*)ppi;

	sih = (si_t*)pi->sh->sih;

	origidx = si_coreidx(sih);

	/* setcore to chipcmn */
	cc = si_setcoreidx(sih, SI_CC_IDX);

	/* Reset 2nd bit */
	sr_chipcontrol(sih, 0x4, 0x0);
	/* Set 2nd bit */
	sr_chipcontrol(sih, 0x4, 0x4);

	/* Set core orig core */
	si_setcoreidx(sih, origidx);

}
#endif /* WLSRVSDB */

/* **************************************** */
/* ***                External functions (HAL)         *** */
/* **************************************** */
#ifdef WLSRVSDB
int
phy_chanmgr_vsdb_force_chans(wlc_phy_t *ppi, uint16* vsdb_chans, uint8 set)
{
	uint16 chans[2];
	phy_info_t *pi = (phy_info_t*)ppi;

	if (set) {
		pi->srvsdb_state->force_vsdb = 1;

		bcopy(vsdb_chans, chans, 2*sizeof(uint16));

		phy_chanmgr_vsdb_sr_attach_module(ppi, chans[0], chans[1]);

		pi->srvsdb_state->prev_chanspec = chans[0];
		pi->srvsdb_state->force_vsdb_chans[0] = chans[0];
		pi->srvsdb_state->force_vsdb_chans[1] = chans[1];

	} else {
		phy_chanmgr_vsdb_sr_detach_module(ppi);

		/* Reset force vsdb chans */
		pi->srvsdb_state->force_vsdb_chans[0] = 0;
		pi->srvsdb_state->force_vsdb_chans[1] = 0;
		pi->srvsdb_state->force_vsdb = 0;
	}

	return BCME_OK;
}

int
phy_chanmgr_vsdb_sr_detach_module(wlc_phy_t *ppi)
{
	uint8 i;
	phy_info_t *pi = (phy_info_t*)ppi;

	/* Disable the flags */
	phy_sr_vsdb_reset(ppi);

	for (i = 0; i < SR_MEMORY_BANK; i++) {
		if (pi->vsdb_bkp[i] != NULL) {
			if (pi->vsdb_bkp[i]->pi_nphy != NULL) {
				phy_mfree(pi, pi->vsdb_bkp[i]->pi_nphy, sizeof(phy_info_nphy_t));
				pi->vsdb_bkp[i]->pi_nphy = NULL;
			}

			if (pi->vsdb_bkp[i]->tx_power_offset != NULL)
				ppr_delete(pi->sh->osh, pi->vsdb_bkp[i]->tx_power_offset);
			phy_mfree(pi, pi->vsdb_bkp[i], sizeof(vsdb_backup_t));
			pi->vsdb_bkp[i] = NULL;
			PHY_INFORM(("de allocate %d of mem \n", (sizeof(vsdb_backup_t) +
				sizeof(phy_info_nphy_t))));
		}
	}
	pi->srvsdb_state->sr_vsdb_channels[0] = 0;
	pi->srvsdb_state->sr_vsdb_channels[1] = 0;
	pi->srvsdb_state->srvsdb_active = 0;

	pi->srvsdb_state->acimode_noisemode_reset_done[0] = FALSE;
	pi->srvsdb_state->acimode_noisemode_reset_done[1] = FALSE;

	/* srvsdb switch status */
	pi->srvsdb_state->switch_successful = FALSE;

	/* Timers */
	bzero(pi->srvsdb_state->prev_timer, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_timer, 2 * sizeof(uint32));

	/* counter for no of switch iterations */
	bzero(pi->srvsdb_state->num_chan_switch, 2 * sizeof(uint8));

	/* crsglitch */
	bzero(pi->srvsdb_state->prev_crsglitch_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_crsglitch, 2 * sizeof(uint32));
	/* bphy_crsglitch */
	bzero(pi->srvsdb_state->prev_bphy_rxcrsglitch_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_bphy_crsglitch, 2 * sizeof(uint32));
	/* badplcp */
	bzero(pi->srvsdb_state->prev_badplcp_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_prev_badplcp, 2 * sizeof(uint32));
	/* bphy_badplcp */
	bzero(pi->srvsdb_state->prev_bphy_badplcp_cnt, 2 * sizeof(uint32));
	bzero(pi->srvsdb_state->sum_delta_prev_bphy_badplcp, 2 * sizeof(uint32));

	return BCME_OK;
}

/**
 * Despite the 'attach' in its name: is not meant to be called in the 'attach' phase.
 * Returns TRUE on success. Caller supplied arguments chan0 and chan1 may not reside in the same
 * band.
 */
uint8
phy_chanmgr_vsdb_sr_attach_module(wlc_phy_t *ppi, chanspec_t chan0, chanspec_t chan1)
{

	uint8 i;
	phy_info_t *pi = (phy_info_t*)ppi;

	/* Detach allready existing structire */
	phy_chanmgr_vsdb_sr_detach_module(ppi);

	/* reset srvsdb enigne */
	phy_reset_srvsdb_engine(ppi);

	/* Alloc mem for sw backup structures */
	for (i = 0; i < SR_MEMORY_BANK; i++) {
		pi->vsdb_bkp[i] = phy_malloc_fatal(pi, sizeof(vsdb_backup_t));
		pi->vsdb_bkp[i]->pi_nphy = phy_malloc_fatal(pi, sizeof(phy_info_nphy_t));
		PHY_INFORM(("allocate %d of mem \n", (sizeof(vsdb_backup_t) +
			sizeof(phy_info_nphy_t))));
	}

	pi->srvsdb_state->sr_vsdb_channels[0] = CHSPEC_CHANNEL(chan0);
	pi->srvsdb_state->sr_vsdb_channels[1] = CHSPEC_CHANNEL(chan1);
	pi->srvsdb_state->srvsdb_active = 1;

	return TRUE;
}
#endif /* WLSRVSDB */

/**
 * Reduce channel switch time by attempting to use hardware acceleration.
 */
uint8
phy_chanmgr_vsdb_sr_set_chanspec(wlc_phy_t *ppi, chanspec_t chanspec, uint8 *last_chan_saved)
{
	uint8 switched = FALSE;
#ifdef WLSRVSDB
	phy_info_t *pi = (phy_info_t*)ppi;
	phy_type_chanmgr_fns_t *fns = pi->chanmgri->fns;

	if (fns->set_chanspec_sr_vsdb) {
		switched = (fns->set_chanspec_sr_vsdb)(fns->ctx, chanspec, last_chan_saved);
	}
#endif /* WLSRVSDB */

	return switched;
}

int
phy_chanmgr_set_shm(phy_info_t *pi, chanspec_t chanspec)
{
	uint16 curchannel;

	/* Update ucode channel value */
	if (D11REV_LT(pi->sh->corerev, 40)) {
		/* d11 rev < 40: compose a channel info value */
		curchannel = CHSPEC_CHANNEL(chanspec);
#ifdef BAND5G
		if (CHSPEC_IS5G(chanspec))
			curchannel |= D11_CURCHANNEL_5G;
#endif /* BAND5G */
		if (CHSPEC_IS40(chanspec))
			curchannel |= D11_CURCHANNEL_40;
	} else {
		/* d11 rev >= 40: store the chanspec */
		curchannel = chanspec;
	}

	PHY_TRACE(("wl%d: %s: M_CURCHANNEL %x\n", pi->sh->unit, __FUNCTION__, curchannel));
	wlapi_bmac_write_shm(pi->sh->physhim, M_CURCHANNEL(pi), curchannel);

	return BCME_OK;
}

uint16
phy_chanmgr_get_home_chanspec(phy_chanmgr_info_t *chanmgri)
{
	return chanmgri->home_chanspec;
}
