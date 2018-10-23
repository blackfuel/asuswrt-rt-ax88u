/*
* Multiple channel scheduler
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
* All exported functions are at the end of the file.
*
* The file contains MSCH exported functions (end of the file)
* and scheduling algorithm for different MSCH request types
* defined in wlc_msch.h file.
*
*
* $Id: wlc_msch.c 738731 2018-01-03 10:15:14Z $
*/

/**
* @file
* @brief
* Twiki: [MultiChanScheduler]
*/

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcm_math.h>
#include <bcmendian.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_hrt.h>
#include <wlc_msch.h>
#include <wlc_mschutil.h>
#include <wlc_msch_priv.h>
#include <phy_misc_api.h>

/* Avoid ROM invalidation in IGUANA_BRANCH_13_10 */
#if defined(MATH_LIB_NOT_SUPPORTED)
#define math_div_64 bcm_uint64_div
#endif // endif

#define MSCH_ASSIGN_MAGIC_NUM(msch_info, value)	((msch_info)->magic_num = value)
#define MSCH_CHECK_MAGIC(msch_info)	((msch_info)->magic_num == MSCH_MAGIC_NUMBER)

#define MSCH_TIME_MIN_ERROR	50

#define	ONE_MS_DELAY	(1000)	/* In u seconds */
#define MAX_BOTH_FLEX_SKIP_COUNT	3

#define MSCH_SKIP_PREPARE (10 * 1000)

/* ======== function prototypes ======== */

static void _msch_arm_chsw_timer(wlc_msch_info_t *msch_info);
static bool wlc_msch_add_timeout(wlc_hrt_to_t *to, int timeout, wlc_hrt_to_cb_fn fun, void *arg);
static bool _msch_check_pending_chan_ctxt(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *ctxt);
static void _msch_chan_ctxt_destroy(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt,
	bool free2list);
static void _msch_update_chan_ctxt_bw(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt);
static void _msch_timeslot_destroy(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	bool cur_slot);
static void _msch_timeslot_free(wlc_msch_info_t *msch_info,
	msch_timeslot_t *timeslot, bool free2list);
static void _msch_timeslot_chn_clbk(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	uint32 clbk_type);
static msch_list_elem_t *msch_list_iterate_pend_callback(msch_list_elem_t *req_list);
static void msch_mark_request_clbk_pending(msch_list_elem_t *req_list);
static void _msch_create_pend_slot(wlc_msch_info_t *msch_info, msch_timeslot_t *timeslot);
static void _msch_schd_slot_start_end(wlc_msch_info_t *msch_info, msch_timeslot_t *ts);
static void _msch_update_service_interval(wlc_msch_info_t *msch_info);
static msch_ret_type_t _msch_schd_new_req(wlc_msch_info_t *msch_info,
	wlc_msch_req_handle_t *req_hdl);
static void _msch_schd(wlc_msch_info_t *msch_info);
static void _msch_prio_list_rotate(wlc_msch_info_t *msch_info,
	msch_list_elem_t *list, uint32 offset);
static msch_ret_type_t _msch_slot_skip(wlc_msch_info_t *msch_info,
	msch_req_entity_t *entity);
static void _msch_sch_fixed(wlc_msch_info_t *msch_info,
	msch_req_entity_t *entity, uint32 prep_dur, uint64 slot_start_time);
static void _msch_schd_next_chan_ctxt(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	uint32 prep_dur, uint64 slot_dur);
static void _msch_next_pend_slot(wlc_msch_info_t *msch_info,
	msch_req_entity_t *req_entity);
static void _msch_next_chn_seq_pend_slot(wlc_msch_info_t *msch_info,
	msch_req_entity_t *req_entity);
static bool _msch_check_both_flex(wlc_msch_info_t *msch_info, uint32 req_dur);
static bool _msch_req_hdl_valid(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl);
static void _msch_update_both_flex_list(wlc_msch_info_t *msch_info, uint64 slot_start,
	uint64 slot_dur);
static void _msch_req_entity_destroy(wlc_msch_info_t *msch_info, msch_req_entity_t *entity);
static uint64 _msch_get_next_slot_fire(wlc_msch_info_t *msch_info);
static void _msch_slotskip_timer(void *arg);
static uint64 _msch_check_avail_extend_slot(wlc_msch_info_t *msch_info,
	msch_req_entity_t *entity, uint64 slot_start, uint64 dur);
static bool _msch_check_bf_timeslot(wlc_msch_info_t *msch_info,  msch_timeslot_t *ts,
	bool cur_ts);
static void _msch_chan_seq_update_ts(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *hdl,
	int32 drift);
static void _msch_reduce_curts(wlc_msch_info_t *msch_info, uint64 end_time);
static void _msch_schd_remaining_cur_slots(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	wlc_msch_req_param_t *preempt_param, chanspec_t preempt_chanspec);
static bool
_msch_chn_seq_update_entity(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	msch_req_entity_t *entity, uint32 prep_time);
static bool
_msch_enity_next_ts_overlap(wlc_msch_info_t *msch_info, msch_req_entity_t *entity);
static void
_msch_chan_cont_req_update(wlc_msch_info_t * msch_info, wlc_msch_req_handle_t *req_hdl);
static bool _msch_ts_extend(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	msch_req_entity_t *entity);
static bool _msch_check_sfix_slot_skip(wlc_msch_info_t *msch_info,
	chanspec_t chspec, uint64 end_time);
static uint64 _msch_get_next_start_time(wlc_msch_info_t *msch_info);
static uint32 wlc_msch_calc_avg_chswtime(wlc_msch_info_t *msch_info, chanspec_t to);
static uint32 wlc_msch_entity_chswtime(wlc_msch_info_t *msch_info,
	msch_req_entity_t *entity, chanspec_t from);
static msch_req_entity_t* wlc_get_ts_entity(wlc_msch_info_t *msch_info, msch_timeslot_t *ts);
static void wlc_msch_timeslot_register_cleanup(wlc_msch_info_t *msch_info,
		wlc_msch_req_handle_t *req_hdl);
static void _msch_chan_ctxt_free(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt,
	bool free2list);
static void _msch_new_req_update_curts(wlc_msch_info_t *msch_info);

#ifdef BCMDBG_MSCH_GPIO
#define MSCH_GPIO_UNUSED	0
#define MSCH_GPIO_REQ_START	1
#define MSCH_GPIO_SLOT_START	10
#define MSCH_GPIO_ON_CHAN	11

static void
msch_toggle_gpio(wlc_msch_info_t *msch_info, uint8 gpio, bool onoff)
{
	uint8 val = onoff ? 1 : 0;
	si_gpioout(msch_info->wlc->pub->sih, 1 << gpio, val << gpio, GPIO_HI_PRIORITY);
}
#endif // endif

static msch_timeslot_t*
_msch_timeslot_alloc(wlc_msch_info_t *msch_info)
{
	msch_timeslot_t *timeslot = NULL;
	msch_list_elem_t *elem;
	/*	Try to allocate from timeslot_list pool
	 *	If nothing is left in the timelot_list pool,
	 *	allocate using malloc.
	 */
	elem = msch_list_remove_head(&msch_info->free_timeslot_list);
	if (elem) {
		timeslot = LIST_ENTRY(elem, msch_timeslot_t, timeslot_list_link);
		memset(timeslot, 0, sizeof(msch_timeslot_t));
	} else {
		timeslot = MALLOCZ(msch_info->wlc->osh, sizeof(msch_timeslot_t));
	}
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_timeslot_alloc: timeslot ALLOC %p\n", OSL_OBFUSCATE_BUF(timeslot)));

	if (timeslot) {
		timeslot->timeslot_id = msch_info->ts_id++;
		PROFILE_CNT_INCR(msch_info, msch_timeslot_alloc_cnt);
	}
	else {
		WL_ERROR(("timeslot alloc failed\n"));
		ASSERT(FALSE);
	}

	return timeslot;
}

/* Function to send skip notification if the next slot's both fixed
 * item is going to be skipped. Updates the pend slot time
 * to skip slot time + interval
 */
static msch_ret_type_t
_msch_slot_skip(wlc_msch_info_t *msch_info,  msch_req_entity_t *entity)
{
	wlc_msch_cb_info_t cb_info;
	wlc_msch_skipslot_info_t skipslot;
	wlc_msch_req_handle_t *hdl_before_clbk;
	wlc_msch_req_param_t *req_param;
	msch_timeslot_t *n_ts = msch_info->next_timeslot;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 start_time;
	uint32 cur_chnsw_time = 0, next_chnsw_time = 0;
	uint64 prev_pend_start_time;
	UNUSED_PARAMETER(prev_pend_start_time);
	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_slot_skip: Entry\n"));
	ASSERT(entity && entity->req_hdl);

	memset(&cb_info, 0, sizeof(cb_info));
	req_param = entity->req_hdl->req_param;
	ASSERT(MSCH_START_FIXED(req_param->req_type));

	cb_info.type = MSCH_CT_SLOT_SKIP;
	if (entity->pend_slot.flags & MSCH_RC_FLAGS_SPLIT_SLOT_START) {
		cb_info.type |= MSCH_CT_PARTIAL;
	}
	/* notify caller the cancel event */
	cb_info.chanspec = entity->chanspec;
	cb_info.req_hdl = entity->req_hdl;
	skipslot.start_time_h =
		(uint32)(entity->pend_slot.start_time >> 32);
	skipslot.start_time_l = (uint32)entity->pend_slot.start_time;
	skipslot.end_time_h = (uint32)(entity->pend_slot.end_time >> 32);
	skipslot.end_time_l = (uint32)entity->pend_slot.end_time;
	cb_info.type_specific = &skipslot;

	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_slot_skip: idx %d chan 0x%x "
		"skip start time %u\n",
		entity->cur_chn_idx, entity->chanspec,
		(uint32)entity->pend_slot.start_time));

	/* Save the previous star time for error check */
	prev_pend_start_time = entity->pend_slot.start_time;

	/* clear the pend_slot */
	memset(&entity->pend_slot, 0, sizeof(msch_req_timing_t));
	if (req_param->interval) {
		if (req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
			_msch_next_chn_seq_pend_slot(msch_info, entity);
			ASSERT(entity->pend_slot.start_time > prev_pend_start_time);
		} else {
			_msch_next_pend_slot(msch_info, entity);
		}

		start_time = entity->pend_slot.start_time;
		if (cur_ts) {
			cur_chnsw_time = wlc_msch_entity_chswtime(msch_info, entity,
				cur_ts->chan_ctxt->chanspec);
		}
		if (n_ts) {
			next_chnsw_time = wlc_msch_entity_chswtime(msch_info, entity,
				n_ts->chan_ctxt->chanspec);
		}
		/* If the newly created pending request overlaps next ts
		 * and starting after current timeslot re-schd next ts
		 */
		if ((!cur_ts || (start_time >= (cur_ts->end_time + cur_chnsw_time))) &&
			n_ts && (start_time <= (n_ts->end_time + next_chnsw_time)) &&
			!(n_ts->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
				FALSE);
			n_ts = msch_info->next_timeslot = NULL;
		}
	}
	else {
		msch_list_remove(&entity->start_fixed_link);
		cb_info.type |= MSCH_CT_REQ_END;
	}

	_msch_dump_callback(msch_info->profiler, &cb_info);

	hdl_before_clbk = entity->req_hdl;
	entity->req_hdl->cb_func(entity->req_hdl->cb_ctxt, &cb_info);

	/* clbk could internally call unregister, which could cause
	 * the elem to go NULL
	 */
	if (!_msch_req_hdl_valid(msch_info, hdl_before_clbk)) {
		return MSCH_TIMESLOT_REMOVED;
	}

	return MSCH_OK;
}

/*
 * time slot destroy, checks both channel context and timeslot
 * since they are inter related
 */
static void
_msch_timeslot_destroy(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	bool cur_slot)
{
	msch_req_entity_t *entity;
	msch_list_elem_t *elem = NULL;
	bool bf_sch_pending = FALSE;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_timeslot_destroy: Entry\n"));

	if (!ts->chan_ctxt) {
		WL_ERROR(("_msch_timeslot_destroy : no chan ctxt\n"));
		goto done;
	}

	if (msch_list_empty(&ts->chan_ctxt->req_entity_list)) {
		_msch_chan_ctxt_destroy(msch_info, ts->chan_ctxt, TRUE);
		ts->chan_ctxt = NULL;
	} else {
		_msch_update_chan_ctxt_bw(msch_info, ts->chan_ctxt);
		elem = &ts->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if (cur_slot && entity->cur_slot.timeslot == ts) {
				msch_list_remove(&entity->cur_slot.link);
				entity->cur_slot.timeslot = NULL;
			}
			if (!cur_slot && entity->pend_slot.timeslot == ts) {
				/* The slot could have been cut down for certain (chan seq)
				* use cases so set the end time to orignial end time
				*/
				if (entity->req_hdl->req_param->flags &
					MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
					entity->pend_slot.end_time = entity->actual_start_time +
						entity->req_hdl->req_param->duration;
					MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
						"_msch_timeslot_destroy: ts_d idx %d chan 0x%x "
						"New start time %u\n",
						entity->cur_chn_idx, entity->chanspec,
						(uint32)entity->pend_slot.start_time));
				}
				entity->pend_slot.timeslot = NULL;
			}

			if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type) &&
				(entity->pend_slot.timeslot || entity->cur_slot.timeslot)) {
				bf_sch_pending = TRUE;
			}
		}

		if (bf_sch_pending != ts->chan_ctxt->bf_sch_pending) {
			ts->chan_ctxt->bf_sch_pending = bf_sch_pending;
		}
	}

done:
	_msch_timeslot_free(msch_info, ts, TRUE);
	if (ts == msch_info->cur_msch_timeslot) {
		msch_info->cur_msch_timeslot = NULL;
	}
}

static void
_msch_timeslot_free(wlc_msch_info_t *msch_info, msch_timeslot_t *timeslot,
	bool free2list)
{
	msch_list_elem_t *elem;
	uint32 freelist_size;
	elem = &msch_info->free_timeslot_list;
	freelist_size = msch_list_length(elem);
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_timeslot_free: timeslot FREE %p\n", OSL_OBFUSCATE_BUF(timeslot)));
	ASSERT(msch_info && timeslot);
	if (free2list && (freelist_size < MSCH_MAX_FREE_TIMESLOTS)) {
		msch_list_add_at(&msch_info->free_timeslot_list, &timeslot->timeslot_list_link);
	}
	else {
		MFREE(msch_info->wlc->osh, timeslot, sizeof(msch_timeslot_t));
	}

	PROFILE_CNT_DECR(msch_info, msch_timeslot_alloc_cnt);
}

static msch_req_entity_t*
_msch_req_entity_alloc(wlc_msch_info_t *msch_info)
{
	msch_req_entity_t *req_entity = NULL;
	msch_list_elem_t *elem = NULL;

	ASSERT(msch_info);
	elem = msch_list_remove_head(&msch_info->free_req_entity_list);
	if (elem) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		memset(req_entity, 0, sizeof(msch_req_entity_t));
	} else {
		req_entity = MALLOCZ(msch_info->wlc->osh, sizeof(msch_req_entity_t));
	}
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_req_entity_alloc: req_entity ALLOC %p\n", OSL_OBFUSCATE_BUF(req_entity)));

	if (req_entity)
		PROFILE_CNT_INCR(msch_info, msch_req_entity_alloc_cnt);

	return req_entity;
}

static void
_msch_req_entity_free(wlc_msch_info_t *msch_info, msch_req_entity_t *req_entity,
	bool free2list)
{
	uint32 freelist_size;
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_req_entity_free: req_entity FREE %p\n", OSL_OBFUSCATE_BUF(req_entity)));
	freelist_size = msch_list_length(&msch_info->free_req_entity_list);
	ASSERT(msch_info && req_entity);
	if (free2list && (freelist_size < MSCH_MAX_FREE_REQ_ENTITY)) {
		/* put back to free list (msch_info->free_req_entity_list) */
		msch_list_add_at(&msch_info->free_req_entity_list, &req_entity->req_hdl_link);
	} else {
		MFREE(msch_info->wlc->osh, req_entity, sizeof(msch_req_entity_t));
	}

	PROFILE_CNT_DECR(msch_info, msch_req_entity_alloc_cnt);
}

static msch_chan_ctxt_t*
_msch_chan_ctxt_alloc(wlc_msch_info_t *msch_info, chanspec_t chanspec)
{
	msch_chan_ctxt_t *chan_ctxt = NULL;
	msch_list_elem_t *elem = NULL;

	ASSERT(msch_info);
	elem = &msch_info->msch_chan_ctxt_list;
	while ((elem = msch_list_iterate(elem))) {
		chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, link);

		if (WLC_CHAN_COEXIST(chanspec, chan_ctxt->chanspec)) {
			/* same ctl channel, merge */
			return chan_ctxt;
		}
	}

	elem = msch_list_remove_head(&msch_info->free_chan_ctxt_list);
	if (elem) {
		chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, link);
		memset(chan_ctxt, 0, sizeof(msch_chan_ctxt_t));
	} else {
		chan_ctxt = MALLOCZ(msch_info->wlc->osh, sizeof(msch_chan_ctxt_t));
	}
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_chan_ctxt_alloc: chan_ctxt ALLOC %p\n", OSL_OBFUSCATE_BUF(chan_ctxt)));

	if (chan_ctxt) {
		chan_ctxt->chanspec = chanspec;
		msch_list_add_at(&msch_info->msch_chan_ctxt_list, &chan_ctxt->link);
		PROFILE_CNT_INCR(msch_info, msch_chan_ctxt_alloc_cnt);
	}

	return chan_ctxt;
}

/* chan_ctx_destroy takes care of unlinking itself from all the list
 * and free's the context
 */
static void
_msch_chan_ctxt_destroy(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt,
	bool free2list)
{
	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_chan_ctxt_destroy: Entry\n"));
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_chan_ctxt_destroy: chan_ctxt FREE %p\n", OSL_OBFUSCATE_BUF(chan_ctxt)));
	ASSERT(msch_info && chan_ctxt);
	ASSERT(msch_list_empty(&chan_ctxt->req_entity_list));

	msch_list_remove(&chan_ctxt->link);
	msch_list_remove(&chan_ctxt->bf_link);
	msch_list_remove(&chan_ctxt->bf_entity_list);

	/* There is a possibilty the channel context is in both current
	 * and next timeslot, cleanup both
	 */
	if (msch_info->cur_msch_timeslot &&
		msch_info->cur_msch_timeslot->chan_ctxt == chan_ctxt) {
		msch_info->cur_msch_timeslot->chan_ctxt = NULL;
	}

	if (msch_info->next_timeslot &&
		msch_info->next_timeslot->chan_ctxt == chan_ctxt) {
		msch_info->next_timeslot->chan_ctxt = NULL;
	}

	_msch_chan_ctxt_free(msch_info, chan_ctxt, free2list);

	PROFILE_CNT_DECR(msch_info, msch_chan_ctxt_alloc_cnt);
}

static void
_msch_chan_ctxt_free(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt,
	bool free2list)
{
	uint32 freelist_size;
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_chan_ctxt_free: chan_ctxt FREE %p\n", OSL_OBFUSCATE_BUF(chan_ctxt)));
	ASSERT(msch_info && chan_ctxt);
	freelist_size = msch_list_length(&msch_info->free_chan_ctxt_list);
	if (free2list &&
			(freelist_size < MSCH_MAX_FREE_CHAN_CTXT)) {
		/* put back to free list (msch_info->free_chan_ctxt_list) */
		msch_list_add_at(&msch_info->free_chan_ctxt_list, &chan_ctxt->link);
	} else {
		MFREE(msch_info->wlc->osh, chan_ctxt, sizeof(msch_chan_ctxt_t));
	}

	PROFILE_CNT_DECR(msch_info, msch_chan_ctxt_alloc_cnt);
}

static wlc_msch_req_handle_t*
_msch_req_handle_alloc(wlc_msch_info_t *msch_info)
{
	wlc_msch_req_handle_t *req_hdl = NULL;
	msch_list_elem_t *elem = NULL;

	ASSERT(msch_info);
	elem = msch_list_remove_head(&msch_info->free_req_hdl_list);
	if (elem) {
		req_hdl = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
		memset(req_hdl, 0, sizeof(wlc_msch_req_handle_t));
	} else {
		req_hdl = MALLOCZ(msch_info->wlc->osh, sizeof(wlc_msch_req_handle_t));
	}
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_req_handle_alloc: req_hdl ALLOC %p\n", OSL_OBFUSCATE_BUF(req_hdl)));

	if (req_hdl)
		PROFILE_CNT_INCR(msch_info, msch_req_hdl_alloc_cnt);

	return req_hdl;
}

static void
_msch_req_handle_free(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	bool free2list)
{
	uint32 freelist_size;
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_req_handle_free: req_hdl FREE %p\n", OSL_OBFUSCATE_BUF(req_hdl)));
	ASSERT(msch_info && req_hdl);
	freelist_size = msch_list_length(&msch_info->free_req_hdl_list);
	if (free2list && (freelist_size < MSCH_MAX_FREE_REQ_HDL)) {
		/* put back to free list (msch_info->free_req_hdl_list) */
		msch_list_add_at(&msch_info->free_req_hdl_list, &req_hdl->link);
	} else {
		MFREE(msch_info->wlc->osh, req_hdl, sizeof(wlc_msch_req_handle_t));
	}

	PROFILE_CNT_DECR(msch_info, msch_req_hdl_alloc_cnt);
}

/* channel change function, majorly invokde wlc_set_chanspec() */
static int
_msch_chan_adopt(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt)
{
	wlc_info_t *wlc = msch_info->wlc;
	chanspec_t chanspec = chan_ctxt->chanspec;
	msch_list_elem_t *elem;
	int chansw_reason = 0;

	elem = &chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		msch_req_entity_t *entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (entity->cur_slot.timeslot) {
			chansw_reason |= (1 << entity->req_hdl->chansw_reason);
		}
	}

	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_chan_adopt Change Chanspec 0x%x, reason 0x%08x\n",
		chanspec, chansw_reason));

	/* No scan support in emulation */
	if (ISSIM_ENAB(wlc->pub->sih) && (chansw_reason & (1 << CHANSW_SCAN)))
		return BCME_OK;

	_msch_dump_profiler(msch_info->profiler);

	wlc_suspend_mac_and_wait(wlc);
	wlc_set_chanspec(wlc, chanspec, chansw_reason);
	wlc_enable_mac(wlc);
	ASSERT((WLC_BAND_PI_RADIO_CHANSPEC == chanspec));
	return BCME_OK;
}

static msch_timeslot_t*
_msch_timeslot_request(wlc_msch_info_t *msch_info,
	msch_chan_ctxt_t *chan_ctxt, wlc_msch_req_param_t *req_param, msch_req_timing_t *slot)
{
	msch_timeslot_t *timeslot = NULL;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_timeslot_request: Entry\n"));
	ASSERT(msch_info->next_timeslot == NULL);

	/* The requested slot's pre_start_time should be greater than current slot's end time */
	if (msch_info->cur_msch_timeslot &&
	    (msch_info->cur_msch_timeslot->end_time != MSCH_INFINITE_TIME) &&
	    (slot->pre_start_time < msch_info->cur_msch_timeslot->end_time)) {

		_msch_dump_profiler(msch_info->profiler);

		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_timeslot_request: req prestart time %u is less than current "
			"slot's end time %u\n", (uint32)slot->pre_start_time,
			(uint32)msch_info->cur_msch_timeslot->end_time));
		ASSERT(FALSE);
		return NULL;
	}
	/* create new time slot */
	timeslot = _msch_timeslot_alloc(msch_info);
	if (!timeslot) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_timeslot_request: msch_timeslot_alloc failed!!\n"));
		ASSERT(FALSE);
		return NULL;
	}

	timeslot->pre_start_time = slot->pre_start_time;
	timeslot->end_time = slot->end_time;
	timeslot->chan_ctxt = chan_ctxt;
	if (timeslot->end_time != MSCH_INFINITE_TIME) {
		timeslot->sch_dur = timeslot->end_time - slot->start_time;
	} else {
		timeslot->sch_dur = -1;
	}

	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_timeslot_request: TS REQ chan 0x%x type 0x%x pre_start_time %d.%06d "
		"duration %u\n", chan_ctxt->chanspec, req_param->req_type,
		_msch_display_time(timeslot->pre_start_time), (uint32)timeslot->sch_dur));
	chan_ctxt->pend_onchan_dur += timeslot->sch_dur;
	return timeslot;
}

/* Called from request unregister or cancel_timeslot
 * This function evaluates if the time slot is still
 * valid based on current pending request entity.
 * Also udpate the new end time if changed due to removed
 * request
 */
static void
_msch_timeslot_check(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	uint64 new_end_time = 0;
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint64 cur_time = msch_current_time(msch_info);
	uint64 entity_end_time = 0;
	uint32 prep_time = 0;

	ASSERT((ts == msch_info->cur_msch_timeslot));
	if (ts == msch_info->cur_msch_timeslot) {
		/* calculate new end time for remaining entities */
		elem = &ts->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if (entity->cur_slot.timeslot == ts) {
				if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type) &&
					entity->cur_slot.end_time != MSCH_INFINITE_TIME) {
					prep_time = msch_info->pm_notif_dur_us;
					entity_end_time = cur_time +  MSCH_MIN_ONCHAN_TIME;
					if (entity->cur_slot.end_time > entity_end_time) {
						entity->cur_slot.end_time = entity_end_time;
					}
				} else {
					prep_time = 0;
				}
				if ((entity->cur_slot.end_time + prep_time) >
					new_end_time) {
					new_end_time = entity->cur_slot.end_time +
						prep_time;
				}
			}
		}

		if (new_end_time) {
			if (new_end_time != ts->end_time) {
				/* no more entity or new end time, schedule next timeslot */
				if (new_end_time) {
					safe_dec(ts->chan_ctxt->pend_onchan_dur,
						ts->end_time - new_end_time);
					ts->sch_dur -= (ts->end_time - new_end_time);
				}
				ts->end_time = new_end_time;

				if (ts->fire_time == MSCH_INFINITE_TIME) {
					ts->fire_time = ts->end_time;
				}
				MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
					"_msch_timeslot_check: Updated cur ts end_time to "
					"%d.%06d!!\n", _msch_display_time(ts->end_time)));
				_msch_arm_chsw_timer(msch_info);
			}
		} else {
			if (!(msch_info->flags & MSCH_STATE_IN_TIEMR_CTXT)) {
				/* No req entity in ts delete it */
				_msch_timeslot_free(msch_info, ts, TRUE);
				msch_info->cur_msch_timeslot = NULL;
			} else if (ts->end_time > cur_time) {
				ts->end_time = cur_time;
			}
		}
	}
}

static int
_msch_req_entity_create(wlc_msch_info_t *msch_info, chanspec_t chanspec,
	wlc_msch_req_handle_t *req_hdl, uint16 chan_idx)
{
	msch_list_elem_t *elem;
	msch_chan_ctxt_t *chan_ctxt = NULL, *tmp_ctxt;
	msch_req_entity_t *req_entity = NULL;
	int ret = BCME_OK;
	uint64 cur_time = msch_current_time(msch_info);
	uint64 time_check;
	wlc_msch_req_param_t *req_param = req_hdl->req_param;
	uint64 time_gap, gap_in_intervals;
	uint32 chansw_time;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_req_entity_create: Entry\n"));

	/* find existing chan_ctxt or create new one per chanspec */
	chan_ctxt = _msch_chan_ctxt_alloc(msch_info, chanspec);
	if (!chan_ctxt) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_req_entity_create: msch_chan_ctxt_alloc failed!!\n"));
		ret = BCME_NOMEM;
		goto fail;
	}

	/* create new req_entity per chanspec */
	req_entity = _msch_req_entity_alloc(msch_info);
	if (!req_entity) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_req_entity_create: msch_req_entity_alloc failed!!\n"));
		ret = BCME_NOMEM;
		goto fail;
	}

	ASSERT(req_entity->pend_slot.timeslot == NULL);

	req_entity->chanspec = chanspec;
	req_entity->chan_ctxt = chan_ctxt;
	req_entity->req_hdl = req_hdl;
	req_entity->priority = req_hdl->req_param->priority;
	req_entity->onchan_chn_idx = req_entity->cur_chn_idx = chan_idx;

	time_check = cur_time;
	/* Move the start time to future for START_FIXED and BOTH_FLEX */
	if (!MSCH_START_FLEX(req_param->req_type)) {
		req_entity->pend_slot.start_time = ((uint64)req_param->start_time_h << 32) |
			req_param->start_time_l;
		if (req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
			req_entity->pend_slot.start_time += (chan_idx * req_param->duration);
		}
		chansw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

		/* Add extra delay so we don't skip due to processing delay  */
		time_check += MSCH_EXTRA_DELAY_FOR_MAX_AWAY_DUR;

		/* Calculate the next slot */
		if (!(req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) &&
			req_entity->req_hdl->req_param->interval &&
			((req_entity->pend_slot.start_time - chansw_time) < time_check)) {
			time_gap = time_check - (req_entity->pend_slot.start_time - chansw_time);

			gap_in_intervals = math_div_64((uint32)(time_gap >> 32),
				(uint32)time_gap, req_param->interval);
			req_entity->pend_slot.start_time +=
				((uint64)req_param->interval * (gap_in_intervals + 1));

			req_param->start_time_h =
				(uint32)(req_entity->pend_slot.start_time >> 32);
			req_param->start_time_l = (uint32)req_entity->pend_slot.start_time;
		}

		req_entity->pend_slot.pre_start_time =
			req_entity->pend_slot.start_time - chansw_time;
		req_entity->actual_start_time = req_entity->pend_slot.start_time;
		req_entity->pend_slot.end_time = req_entity->pend_slot.start_time +
			req_entity->req_hdl->req_param->duration;
	}

	/* Add the request entity on corresponding list based on its request type */
	if (MSCH_START_FIXED(req_param->req_type)) {
		/* queue in timing */
		msch_list_sorted_add(&msch_info->msch_start_fixed_list,
			&req_entity->start_fixed_link,
			OFFSETOF(msch_req_entity_t, pend_slot) -
			OFFSETOF(msch_req_entity_t, start_fixed_link) +
			OFFSETOF(msch_req_timing_t, start_time),
			sizeof(uint64), MSCH_ASCEND);
	} else if (MSCH_START_FLEX(req_param->req_type)) {
		/* queue in priority */
		msch_list_sorted_add(&msch_info->msch_start_flex_list,
			&req_entity->rt_specific_link,
			OFFSETOF(msch_req_entity_t, priority) -
			OFFSETOF(msch_req_entity_t, rt_specific_link),
			1, MSCH_DESCEND);

	} else if (MSCH_BOTH_FLEX(req_param->req_type)) {
		/* queue in priority in bf_entity_list */
		msch_list_sorted_add(&chan_ctxt->bf_entity_list,
			&req_entity->rt_specific_link,
			OFFSETOF(msch_req_entity_t, priority) -
			OFFSETOF(msch_req_entity_t, rt_specific_link),
			1, MSCH_DESCEND);

		/* queue in timing in msch_both_flex_req_entity_list */
		msch_list_sorted_add(&msch_info->msch_both_flex_req_entity_list,
			&req_entity->both_flex_list,
			OFFSETOF(msch_req_entity_t, pend_slot) -
			OFFSETOF(msch_req_entity_t, both_flex_list) +
			OFFSETOF(msch_req_timing_t, start_time),
			sizeof(uint64), MSCH_ASCEND);

		if (req_param->interval > msch_info->max_lo_prio_interval) {
			msch_info->max_lo_prio_interval = req_param->interval;
		}

		if (!msch_elem_inlist(&chan_ctxt->bf_link)) {
			msch_list_sorted_add(&msch_info->msch_both_flex_list,
				&chan_ctxt->bf_link, 0, 0, MSCH_NO_ORDER);
			msch_info->flex_list_cnt++;
			if (msch_info->flex_list_cnt > 1) {
				/* only multi bf ctxt need use service interval to schedule */
				msch_info->service_interval = msch_info->max_lo_prio_interval /
					msch_info->flex_list_cnt;
				elem = &msch_info->msch_both_flex_list;
				while ((elem = msch_list_iterate(elem))) {
					tmp_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, bf_link);
					tmp_ctxt->actual_onchan_dur = 0;
					tmp_ctxt->pend_onchan_dur = 0;
				}
			}
		}
	} else {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_req_entity_create: No MSCH reqest type found\n"));
		ret = BCME_BADARG;
		goto fail;
	}

	/* If there is a change in channel bandwidth, then udpate the context
	 * bandwidth
	 */
	if (chan_ctxt->chanspec) {
		/* pick widest band */
		if (CHSPEC_BW(chanspec) > CHSPEC_BW(chan_ctxt->chanspec)) {
			chan_ctxt->chanspec = chanspec;
		}
	} else {
		chan_ctxt->chanspec = chanspec;
	}

	/* queue req_entity to chan_ctxt->req_entity_list in priority order */
	msch_list_sorted_add(&chan_ctxt->req_entity_list, &req_entity->chan_ctxt_link,
		OFFSETOF(msch_req_entity_t, priority) -
		OFFSETOF(msch_req_entity_t, chan_ctxt_link),
		1, MSCH_DESCEND);

	/* queue req_entity to req_hdl->req_entity_list */
	msch_list_add_to_tail(&req_hdl->req_entity_list, &req_entity->req_hdl_link);
	return ret;

fail:
	if (req_entity) {
		_msch_req_entity_destroy(msch_info, req_entity);
		req_entity = NULL;
	}

	if (chan_ctxt &&
		msch_list_empty(&chan_ctxt->req_entity_list)) {
		if (msch_info->cur_msch_timeslot) {
			ASSERT(chan_ctxt != msch_info->cur_msch_timeslot->chan_ctxt);
		}
		_msch_chan_ctxt_destroy(msch_info, chan_ctxt, TRUE);
		chan_ctxt = NULL;
	}
	return ret;
}

static void
_msch_req_entity_destroy(wlc_msch_info_t *msch_info, msch_req_entity_t *req_entity)
{
	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_req_entity_destroy: Entry\n"));
	ASSERT(msch_info && req_entity);

	msch_list_remove(&req_entity->rt_specific_link);
	msch_list_remove(&req_entity->chan_ctxt_link);
	msch_list_remove(&req_entity->req_hdl_link);
	msch_list_remove(&req_entity->start_fixed_link);
	msch_list_remove(&req_entity->both_flex_list);
	msch_list_remove(&req_entity->cur_slot.link);
	msch_list_remove(&req_entity->pend_slot.link);

	_msch_req_entity_free(msch_info, req_entity, TRUE);
}

/* Check if the request entity exist for the request handle */
static msch_req_entity_t*
_msch_req_entity_available(wlc_msch_info_t * msch_info,
	wlc_msch_req_handle_t *req_hdl, chanspec_t chanspec)
{
	msch_req_entity_t *entity;
	msch_list_elem_t *elem;
	elem = &req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		if (entity->chanspec == chanspec) {
			return entity;
		}
	}
	return NULL;
}

/* Schedule a timeslot for start flex entity, prepare the
 * start time/end time and request a timeslot
 */
static msch_ret_type_t
_msch_sch_start_flex(wlc_msch_info_t *msch_info, msch_req_entity_t *entity,
	uint64 pre_start_time, uint32 prep_dur)
{
	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_sch_start_flex: Entry\n"));
	ASSERT(msch_info && entity);

	if (entity->pend_slot.timeslot) {
		return MSCH_ALREADY_SCHD;
	}

	entity->pend_slot.pre_start_time = pre_start_time;
	entity->pend_slot.start_time = pre_start_time + prep_dur;
	entity->actual_start_time = entity->pend_slot.start_time;
	entity->pend_slot.end_time = entity->pend_slot.start_time +
		entity->req_hdl->req_param->duration;
	entity->pend_slot.timeslot = _msch_timeslot_request(msch_info, entity->chan_ctxt,
		entity->req_hdl->req_param, &entity->pend_slot);

	if (entity->pend_slot.timeslot) {
		/* request succeed */
		return MSCH_OK;
	}

	return MSCH_FAIL;
}

/* Schedule a timeslot for both flex entity, prepare the
 * start time/end time and request a timeslot
 */
static msch_ret_type_t
_msch_sch_both_flex(wlc_msch_info_t *msch_info, uint64 pre_start_time,
	uint64 free_slot, msch_req_entity_t *entity, uint32 prep_dur)
{
	msch_chan_ctxt_t *chan_ctxt = NULL;
	uint64 sch_dur, time_to_next_bf = 0;
	msch_req_entity_t *next_entity = NULL;
	uint64 next_bf_pre_start = 0;
	msch_list_elem_t *elem;
	uint32 max_chsw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_sch_both_flex: Entry\n"));

	chan_ctxt = entity->chan_ctxt;
	sch_dur = (free_slot == MSCH_INFINITE_TIME) ? MSCH_INFINITE_TIME : free_slot;
	entity->pend_slot.pre_start_time = pre_start_time;
	entity->pend_slot.start_time = pre_start_time + prep_dur;

	/* Get next BF element */
	/* VSDB mode, manage slot duration based on next both flex
	 * start time (duration = next_bf_start - cur_bf_start)
	 */
	if (msch_info->flex_list_cnt > 1) {
		if (entity->both_flex_list.next) {
			next_entity = LIST_ENTRY(entity->both_flex_list.next, msch_req_entity_t,
				both_flex_list);
			next_bf_pre_start = next_entity->pend_slot.start_time - max_chsw_time;
			time_to_next_bf = (next_bf_pre_start - entity->pend_slot.start_time);
			if (sch_dur > time_to_next_bf) {
				sch_dur = time_to_next_bf;
			}
		} else if ((elem = msch_list_iterate(&msch_info->msch_both_flex_req_entity_list))) {
			next_entity = LIST_ENTRY(elem, msch_req_entity_t, both_flex_list);
			if (next_entity != entity && next_entity->req_hdl->req_param->interval) {
				next_bf_pre_start = next_entity->pend_slot.start_time +
					next_entity->req_hdl->req_param->interval -
					max_chsw_time;
				while (next_bf_pre_start < entity->pend_slot.start_time) {
					next_bf_pre_start +=
						next_entity->req_hdl->req_param->interval;
				}
				time_to_next_bf = (next_bf_pre_start -
					entity->pend_slot.start_time);
				if (sch_dur > time_to_next_bf) {
					sch_dur = time_to_next_bf;
				}
			}
		}
	}

	if (sch_dur == MSCH_INFINITE_TIME) {
		if (entity->cur_slot.timeslot &&
			(entity->cur_slot.timeslot == msch_info->cur_msch_timeslot) &&
			(msch_info->cur_msch_timeslot->end_time == MSCH_INFINITE_TIME)) {
			/* bflex is already in cur slot with end time -1 */
			return MSCH_FAIL;
		}

		/* scheduler for min duration if there is a pending
		 * chan ctxt
		 */
		if (_msch_check_pending_chan_ctxt(msch_info, chan_ctxt)) {
			/* TODO let the duration pass from higher layer ? */
			/* each req_entity could have different home time so ...? */
			/* VSDB case ? */
			entity->pend_slot.end_time =
				entity->pend_slot.start_time +
				entity->req_hdl->req_param->flex.bf.min_dur +
				msch_info->pm_notif_dur_us;
		} else {
			entity->pend_slot.end_time = -1;
		}
	} else {
		entity->pend_slot.end_time = entity->pend_slot.start_time + sch_dur;
	}

	entity->pend_slot.timeslot = _msch_timeslot_request(msch_info, chan_ctxt,
		entity->req_hdl->req_param, &entity->pend_slot);

	if (entity->pend_slot.timeslot) {
		ASSERT(!msch_info->next_timeslot);
		chan_ctxt->bf_sch_pending = TRUE;
		msch_info->next_timeslot = entity->pend_slot.timeslot;
		if (entity->pend_slot.end_time != MSCH_INFINITE_TIME) {
			/* update the end time giving room for PM notificatino */
			entity->pend_slot.end_time -= msch_info->pm_notif_dur_us;
		}

		return MSCH_OK;
	}
	return MSCH_FAIL;
}

/* Function to notify SLOT_START and SLOT_END callback notification
 * prepares the next notification if required
 * returns the next fire time
 */
static uint64
_msch_slot_start_end_notif(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	wlc_msch_cb_info_t cb_info;
	msch_req_timing_t *slot;
	msch_req_entity_t *req_entity = NULL;
	msch_list_elem_t *elem = NULL;
	wlc_msch_req_handle_t *hdl_before_clbk;
	uint64 next_fire = -1;
	uint64 cur_time = msch_current_time(msch_info);
	wlc_msch_onchan_info_t onchan;
	uint64 start_time;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_slot_start_end_notif: Entry\n"));

	/* time sanity check */
	next_fire = _msch_get_next_slot_fire(msch_info);
	if (next_fire > cur_time) {
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"_msch_slot_start_end_notif: next fire time is greater than cur_time\n"));
		return next_fire;
	}

	while ((elem = msch_list_remove_head(&msch_info->msch_req_timing_list))) {
		wlc_msch_req_param_t *req_param;
		slot = LIST_ENTRY(elem, msch_req_timing_t, link);
		req_entity = LIST_ENTRY(slot, msch_req_entity_t, cur_slot);
		req_param = req_entity->req_hdl->req_param;

		memset(&cb_info, 0, sizeof(cb_info));
		cb_info.chanspec = req_entity->chanspec;
		cb_info.req_hdl = req_entity->req_hdl;

		if (slot->flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
			slot->flags |= MSCH_RC_FLAGS_END_FIRE_DONE;
			if (ts->fire_time != slot->end_time) {
				MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
					"_msch_slot_start_end_notif: fire %d.%06d end_time "
					"%d.%06d cur_time %d.%06d\n",
					_msch_display_time(ts->fire_time),
					_msch_display_time(slot->end_time),
					_msch_display_time(cur_time)));
			}

			if (MSCH_BOTH_FLEX(req_param->req_type)) {
				cb_info.type |= MSCH_CT_OFF_CHAN;
				if (req_entity->pend_slot.timeslot &&
					(req_entity->pend_slot.timeslot->pre_start_time ==
						ts->end_time)) {
					ASSERT(req_entity->pend_slot.timeslot ==
						msch_info->next_timeslot);
					cb_info.type |= MSCH_CT_SLOT_CONTINUE;
					msch_info->next_timeslot->ts_flags |=
						MSCH_TS_FLAG_DONT_DESTROY;
				}
				req_entity->last_serv_time = cur_time;
			} else {
				cb_info.type |= MSCH_CT_SLOT_END;
			}
#ifdef BCMDBG_MSCH_GPIO
			msch_toggle_gpio(msch_info, MSCH_GPIO_SLOT_START, FALSE);
#endif // endif
			if (req_entity->cur_slot.flags & MSCH_RC_FLAGS_SPLIT_SLOT_END) {
				cb_info.type |= MSCH_CT_PARTIAL;
			}
			if (!req_param->interval) {
				req_entity->req_hdl->chan_cnt--;
				/* Send req end for the non-periodic last channel slot_end */
				if (req_entity->req_hdl->chan_cnt == 0) {
					cb_info.type |= MSCH_CT_REQ_END;
				}
			} else if ((req_param->interval == req_param->duration) &&
					MSCH_BOTH_FIXED(req_param->req_type) &&
					!msch_info->next_timeslot) {
				/* consicutive both_fixed slots
				 * Merge the next slot to cur timeslot if there is no
				 * gap between the current and the pend slot
				 * Gap will be seen if the next pend slot is skipped for
				 * any other request
				 */
				if ((req_entity->req_hdl->req_param->flags &
					MSCH_REQ_FLAGS_MERGE_CONT_SLOTS) &&
					(req_entity->pend_slot.end_time ==
					(req_entity->cur_slot.end_time + req_param->duration))) {
					/* Update the slot end time to pend slot end time */
					req_entity->cur_slot.end_time =
						req_entity->pend_slot.end_time;
					req_entity->curts_fire_time =
						req_entity->cur_slot.end_time;
					/* Add the new end time to slot end fire time */
					msch_list_sorted_add(&msch_info->msch_req_timing_list,
						&req_entity->cur_slot.link,
						(OFFSETOF(msch_req_entity_t, curts_fire_time) -
						OFFSETOF(msch_req_entity_t, cur_slot) -
						OFFSETOF(msch_req_timing_t, link)),
						sizeof(req_entity->curts_fire_time), MSCH_ASCEND);

					_msch_next_pend_slot(msch_info, req_entity);

					/* Update the cur timeslot if there is change in
					 * end time
					 */
					if (ts->end_time != MSCH_INFINITE_TIME &&
						ts->end_time < req_entity->cur_slot.end_time) {
						ts->end_time = req_entity->cur_slot.end_time;
					}

					/* Skip slot end notification since we moved the
					 * slot end to next slot
					 */
					next_fire = _msch_get_next_slot_fire(msch_info);
					if (next_fire == MSCH_INFINITE_TIME ||
						(next_fire > cur_time)) {
						/* Done with slot notification */
						return next_fire;
					}
					/* continue with next slot notification in the timming
					 * list
					 */
					continue;
				}
			}
		} else {
			start_time =
				((uint64)req_entity->req_hdl->req_param->start_time_h << 32) |
				req_entity->req_hdl->req_param->start_time_l;
			onchan.timeslot_id = ts->timeslot_id;
			onchan.end_time_h = (uint32)(slot->end_time >> 32);
			onchan.end_time_l = (uint32)slot->end_time;
			onchan.onchan_idx = req_entity->onchan_chn_idx;

			if (req_entity->req_hdl->req_param->flags &
				MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
				if (req_entity->onchan_chn_idx ==
					req_entity->req_hdl->last_chan_idx) {
					onchan.cur_chan_seq_start_time =
						start_time -
						req_entity->req_hdl->req_param->interval;
				}
					/* Get the slot start time */
				else {
					onchan.cur_chan_seq_start_time = start_time;
				}
				/* Get the slot start time */
				start_time = onchan.cur_chan_seq_start_time +
					(onchan.onchan_idx *
					req_entity->req_hdl->req_param->duration);
				onchan.start_time_h = (uint32)(start_time >> 32);
				onchan.start_time_l = (uint32)start_time;
			} else {
				onchan.start_time_h = (uint32)(slot->start_time >> 32);
				onchan.start_time_l = (uint32)slot->start_time;
			}
			cb_info.type_specific = &onchan;

			if (ts->fire_time != slot->start_time) {
				MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
					"_msch_slot_start_end_notif: fire %d.%06d start_time "
					"%d.%06d cur_time %d.%06d\n",
					_msch_display_time(ts->fire_time),
					_msch_display_time(slot->start_time),
					_msch_display_time(cur_time)));
			}

			if (req_entity->req_hdl->flags & MSCH_REQ_HDL_FLAGS_NEW_REQ) {
				req_entity->req_hdl->flags &= ~MSCH_REQ_HDL_FLAGS_NEW_REQ;
				cb_info.type |= MSCH_CT_REQ_START;
			}

			cb_info.type |= MSCH_CT_SLOT_START;
#ifdef BCMDBG_MSCH_GPIO
			msch_toggle_gpio(msch_info, MSCH_GPIO_SLOT_START, TRUE);
#endif // endif
			if (req_entity->cur_slot.flags & MSCH_RC_FLAGS_SPLIT_SLOT_START) {
				cb_info.type |= MSCH_CT_PARTIAL;
			}
			slot->flags |= MSCH_RC_FLAGS_START_FIRE_DONE;

			/* For BothFlex it is always OFF_PREP */
			if (req_entity->cur_slot.end_time != MSCH_INFINITE_TIME) {
				req_entity->curts_fire_time = req_entity->cur_slot.end_time;
				msch_list_sorted_add(&msch_info->msch_req_timing_list,
					&req_entity->cur_slot.link,
					(OFFSETOF(msch_req_entity_t, curts_fire_time) -
					OFFSETOF(msch_req_entity_t, cur_slot) -
					OFFSETOF(msch_req_timing_t, link)),
					sizeof(req_entity->curts_fire_time), MSCH_ASCEND);
			}
		}

		_msch_dump_callback(msch_info->profiler, &cb_info);

		hdl_before_clbk = req_entity->req_hdl;
		req_entity->req_hdl->cb_func(req_entity->req_hdl->cb_ctxt,
			&cb_info);

		/* possible timeslot unregister can happen in clbk,
		 * check if req_hdl is valid after clbk
		 */
		if (_msch_req_hdl_valid(msch_info, hdl_before_clbk)) {
			if (cb_info.type & MSCH_CT_SLOT_END) {
				if (cb_info.type & MSCH_CT_REQ_END) {
					wlc_msch_timeslot_unregister(msch_info,
						&req_entity->req_hdl);
				} else if (!req_entity->req_hdl->req_param->interval) {
					_msch_req_entity_destroy(msch_info, req_entity);
				} else {
					/* Req complete, unlink from timeslot */
					req_entity->cur_slot.timeslot = NULL;
				}
			}
		}

		next_fire = _msch_get_next_slot_fire(msch_info);
		if (next_fire == MSCH_INFINITE_TIME ||
			(next_fire > cur_time)) {
			/* Done with slot notification */
			return next_fire;
		}
	}
	return -1;
}

static bool
_msch_req_hdl_valid(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl)
{
	msch_list_elem_t *elem;
	wlc_msch_req_handle_t *list_hdl;
	/* Check if the req hdl is valid */
	elem = &msch_info->msch_req_hdl_list;
	while ((elem = msch_list_iterate(elem))) {
		list_hdl = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
		if (list_hdl == req_hdl) {
			/* found valid handle */
			return TRUE;
		}
	}

	return FALSE;
}

static msch_ret_type_t
_msch_pre_onchan_clbk(wlc_msch_info_t *msch_info, msch_timeslot_t *cur_ts)
{
	_msch_timeslot_chn_clbk(msch_info, cur_ts, MSCH_CT_PRE_ONCHAN);
	return MSCH_OK;
}

#define CHSW_TIMER_CUR_TS_OK		0
#define CHSW_TIMER_DONE				1
#define CHSW_TIMER_CUR_TS_NULL		2

static int
_msch_chsw_tmr_handle_cur_ts(wlc_msch_info_t *msch_info, uint64 cur_time)
{
	msch_chan_ctxt_t *cur_chan_ctxt = NULL;
	msch_list_elem_t *req_elem;
	msch_req_entity_t *entity = NULL;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	int result = CHSW_TIMER_CUR_TS_OK;

	cur_chan_ctxt = cur_ts->chan_ctxt;
	/* Handle ONCHAN, SLOT START, SLOT_END notification in
	 * ONCHAN_FIRE state
	 */
	if (cur_ts->state == MSCH_TIMER_STATE_ONCHAN_FIRE) {
		/* Check any new request piggybacked to current timeslot */
		req_elem = &cur_ts->chan_ctxt->req_entity_list;
		while ((req_elem = msch_list_iterate(req_elem))) {
			entity = LIST_ENTRY(req_elem, msch_req_entity_t, chan_ctxt_link);
			/* identify new req based on ONFIRE_DONE flag */
			if (!entity->cur_slot.timeslot ||
				entity->cur_slot.flags & MSCH_RC_FLAGS_ONFIRE_DONE) {
				continue;
			}
			_msch_timeslot_chn_clbk(msch_info, cur_ts, MSCH_CT_ON_CHAN);
			_msch_schd_slot_start_end(msch_info, cur_ts);
		}

		if (!msch_list_empty(&msch_info->msch_req_timing_list)) {
			cur_ts->fire_time = _msch_slot_start_end_notif(msch_info, cur_ts);
			if (cur_ts->fire_time != MSCH_INFINITE_TIME) {
				MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
							  "_msch_chsw_timer:"
							  "cur_ts->fire_time is not "
							  "infinite\n"));
				return CHSW_TIMER_DONE;
			}
		}
		cur_ts->fire_time = cur_ts->end_time;
		if (cur_ts->end_time != MSCH_INFINITE_TIME) {
			if (cur_ts->end_time > cur_time) {
				MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
							  "_msch_chsw_timer:"
							  "end_time is greater than "
							  "cur_time\n"));
				return CHSW_TIMER_DONE;
			}
			if (_msch_check_bf_timeslot(msch_info, cur_ts, TRUE)) {
				cur_ts->state = MSCH_TIMER_STATE_OFF_CHN_DONE;
			} else {
				cur_ts->state = MSCH_TIMER_STATE_TS_COMPLETE;
			}
		}

		/* prepare for next fire time */
		if (msch_info->next_timeslot &&
			msch_info->next_timeslot->fire_time
			<= msch_current_time(msch_info)) {
			if (_msch_check_bf_timeslot(msch_info, cur_ts, TRUE)) {
				cur_ts->state = MSCH_TIMER_STATE_OFF_CHN_DONE;
			} else {
				cur_ts->state = MSCH_TIMER_STATE_TS_COMPLETE;
			}
		}
	}

	/* Prepare for next channel, if Both flex entity exist do
	 * OFF_PREP callback
	 */
	if (cur_ts->state == MSCH_TIMER_STATE_OFF_CHN_PREP) {
		cur_chan_ctxt->actual_onchan_dur += (cur_time - cur_chan_ctxt->onchan_time);
		safe_dec(cur_chan_ctxt->pend_onchan_dur, cur_ts->sch_dur);
		_msch_timeslot_chn_clbk(msch_info, cur_ts, MSCH_CT_OFF_CHAN);

		if (msch_info->next_timeslot) {
			msch_info->next_timeslot->pre_start_time +=
				msch_info->pm_notif_dur_us;
		}

		cur_ts->fire_time = msch_current_time(msch_info) +
			msch_info->pm_notif_dur_us;
		cur_ts->state = MSCH_TIMER_STATE_OFF_CHN_DONE;
				return CHSW_TIMER_DONE;
	}

	/* OFF_DONE notification to move to next channel */
	if (cur_ts->state == MSCH_TIMER_STATE_OFF_CHN_DONE) {
		_msch_timeslot_chn_clbk(msch_info, cur_ts, MSCH_CT_OFF_CHAN_DONE);
		cur_ts->state = MSCH_TIMER_STATE_TS_COMPLETE;
	}

	/* Cur TS complete, cleanup ts, chan ctxt */
	if (cur_ts->state == MSCH_TIMER_STATE_TS_COMPLETE) {
		if (!msch_info->next_timeslot ||
				msch_info->next_timeslot->chan_ctxt != cur_chan_ctxt ||
				_msch_check_bf_timeslot(msch_info, msch_info->next_timeslot,
				FALSE)) {
			cur_chan_ctxt->bf_sch_pending = FALSE;
		}

		if (msch_list_empty(&cur_ts->chan_ctxt->req_entity_list)) {
			_msch_chan_ctxt_destroy(msch_info, cur_ts->chan_ctxt, TRUE);
			cur_ts->chan_ctxt = NULL;
			_msch_timeslot_free(msch_info, cur_ts, TRUE);
			msch_info->cur_msch_timeslot = NULL;
			result = CHSW_TIMER_CUR_TS_NULL;
		} else {
			/* here we can unlink all req entity of cur ts */
			_msch_timeslot_destroy(msch_info, cur_ts, TRUE);
			result = CHSW_TIMER_CUR_TS_NULL;
		}
	}
	return result;
}

/* MSCH timeslot state machine timer */
static void
_msch_chsw_timer(void *arg)
{
	wlc_msch_info_t *msch_info = (wlc_msch_info_t *)arg;
	msch_list_elem_t *req_elem;
	uint64 cur_time;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	wlc_info_t *wlc = msch_info->wlc;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_chsw_timer: Entry\n"));
	ASSERT(msch_info);

	cur_time = msch_current_time(msch_info);
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_chsw_timer: current_time: %d.%06d\n",
		_msch_display_time(cur_time)));

	msch_info->flags |=  MSCH_STATE_IN_TIEMR_CTXT;
	msch_info->cur_armed_timeslot = NULL;

	if (cur_ts) {
		int cur_ts_result = _msch_chsw_tmr_handle_cur_ts(msch_info, cur_time);
		if (cur_ts_result == CHSW_TIMER_DONE) {
			goto done;
		} else if (cur_ts_result == CHSW_TIMER_CUR_TS_NULL) {
			cur_ts = NULL;
		}
	}

	if (msch_info->cur_msch_timeslot) {
		goto done;
	}

	if (msch_info->next_timeslot &&
		!msch_info->next_timeslot->chan_ctxt) {
		/* invalid ts destroy it */
		_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
		msch_info->next_timeslot = NULL;
	}

	/* check next timeslot valid */
	if (!msch_info->next_timeslot) {
		/* Schedule item if one of the list is not empty */
		if (!msch_list_empty(&msch_info->msch_start_fixed_list) ||
			!msch_list_empty(&msch_info->msch_start_flex_list) ||
			!msch_list_empty(&msch_info->msch_both_flex_req_entity_list)) {
			if (!msch_info->slotskip_flag) {
				_msch_schd(msch_info);
			}
		}
		msch_info->flags &=  ~MSCH_STATE_IN_TIEMR_CTXT;
		return;
	}

	if (!msch_info->next_timeslot->fire_time ||
		msch_info->next_timeslot->fire_time == MSCH_INFINITE_TIME) {
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"_msch_chsw_timer: fire_time not set: cur_fire %d.%06d\n",
			_msch_display_time(msch_info->next_timeslot->fire_time)));
		msch_info->next_timeslot->fire_time = msch_info->next_timeslot->pre_start_time;
	}

	if (cur_time < msch_info->next_timeslot->fire_time) {
		if (msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY) {
			WL_ERROR(("chsw_timer : no cur ts, next dont destroy set\n"));
			/* todo: This needs to be checked on why we are getting into this gap
			 * between cur and next ts even on same chan ctxt
			 */
			msch_info->next_timeslot->ts_flags &= ~MSCH_TS_FLAG_DONT_DESTROY;
		}
		/* next timeslot is not due, schedule timer for fire time */
		goto done;
	}

	cur_ts = msch_info->cur_msch_timeslot = msch_info->next_timeslot;
	cur_ts->ts_flags = 0;
	msch_info->next_timeslot = NULL;

	/* request timing of current ts should be empty */
	ASSERT(msch_list_empty(&msch_info->msch_req_timing_list));

	/* Create the pending slot for periodic request */
	_msch_create_pend_slot(msch_info, cur_ts);

	if (_msch_pre_onchan_clbk(msch_info, cur_ts) == MSCH_TS_CANCELLED) {
		/* TODO pre_onchan_clbk shall set right state and fire time
		 * if cancelled
		 */
		goto done;
	}

	/* Switch to channel */
	if (cur_ts->chan_ctxt->chanspec != WLC_BAND_PI_RADIO_CHANSPEC) {
		_msch_chan_adopt(msch_info, cur_ts->chan_ctxt);
	}
	cur_ts->state = MSCH_TIMER_STATE_ONCHAN_FIRE;
	cur_ts->chan_ctxt->onchan_time = msch_current_time(msch_info);

	/* TODO add debug to capture if the callback has consumed too long
	 * cur/next schedule might need to be re evaluated if
	 * the time consumed is more than the cur_ts end_time
	 */
	_msch_timeslot_chn_clbk(msch_info, cur_ts, MSCH_CT_ON_CHAN);

	/* Create req_timing for SLOT START or END */
	_msch_schd_slot_start_end(msch_info, cur_ts);

	cur_ts->fire_time = cur_ts->end_time;
	if ((req_elem = msch_list_iterate(&msch_info->msch_req_timing_list))) {
		msch_req_timing_t *slot;
		slot = LIST_ENTRY(req_elem, msch_req_timing_t, link);
		if (slot->flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
			cur_ts->fire_time = slot->end_time;
		} else {
			cur_ts->fire_time = slot->start_time;
		}
	}
	/*
	 * Only schedule the entity when skip timer is deleted
	 * to avoid unnecessary slot skips
	 */
	if (!msch_info->slotskip_flag) {
		_msch_schd(msch_info);
	}
done:
	cur_time = msch_current_time(msch_info);
	if (cur_ts) {
		if (cur_ts->fire_time != MSCH_INFINITE_TIME) {
			msch_info->cur_armed_timeslot = cur_ts;
			MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
				"_msch_chsw_timer: fire_time: %d.%06d, diff %d\n",
				_msch_display_time(cur_ts->fire_time),
				(int)(cur_ts->fire_time - cur_time)));
			wlc_msch_add_timeout(msch_info->chsw_timer,
				(int)(cur_ts->fire_time - cur_time),
				_msch_chsw_timer, msch_info);
		}
	} else if (msch_info->next_timeslot &&
		msch_info->next_timeslot->fire_time != MSCH_INFINITE_TIME) {
		msch_info->cur_armed_timeslot = msch_info->next_timeslot;
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"_msch_chsw_timer: fire_time: %d.%06d, diff %d\n",
			_msch_display_time(msch_info->next_timeslot->fire_time),
			(int)(msch_info->next_timeslot->fire_time - cur_time)));
		wlc_msch_add_timeout(msch_info->chsw_timer,
			(int)(msch_info->next_timeslot->fire_time - cur_time),
			_msch_chsw_timer, msch_info);
	}
	msch_info->flags &=  ~MSCH_STATE_IN_TIEMR_CTXT;

	if (!msch_info->slotskip_flag &&
		(msch_info->flags & MSCH_STATE_SCHD_PENDING)) {
		if (!msch_info->next_timeslot) {
			_msch_schd(msch_info);
		}
	}
}

/* Function to create a next pending slot for contigous channel sequence.
 * schedules the next occurance of the channel with in the seq if exist
 * or schedules the first occurance of the channel in next seq (add interval)
 */
static void
_msch_next_chn_seq_pend_slot(wlc_msch_info_t *msch_info, msch_req_entity_t *req_entity)
{
	wlc_msch_req_param_t *req_param = req_entity->req_hdl->req_param;
	wlc_msch_req_handle_t *req_hdl = req_entity->req_hdl;
	bool next_chn_found = FALSE;
	uint16 idx = -1;

	msch_list_remove(&req_entity->start_fixed_link);
	req_entity->pend_slot.start_time = ((uint64)req_param->start_time_h << 32) |
		req_param->start_time_l;

	/* Last channel, update req start time */
	if (req_entity->cur_chn_idx == req_hdl->last_chan_idx) {
		wlc_uint64_add(&req_param->start_time_h,
			&req_param->start_time_l, 0, req_param->interval);
	}
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_next_chn_seq_pend_slot: req_start: %u \n",
		req_param->start_time_l));
	if (req_entity->flags & MSCH_ENTITY_FLAG_MULTI_INSTANCE) {
		/* Search for the channel index in the channel sequence list */
		idx = req_entity->cur_chn_idx + 1;
		while (idx <= req_hdl->last_chan_idx) {
			if (req_entity->chanspec == req_hdl->chn_list[idx]) {
				req_entity->pend_slot.start_time += (idx * req_param->duration);
				next_chn_found = TRUE;
				break;
			}
			idx++;
		}

		if (!next_chn_found) {
			if (!req_param->interval) {
				return;
			}

			idx = 0;
			while (idx < req_entity->cur_chn_idx) {
				if (req_entity->chanspec == req_hdl->chn_list[idx]) {
					req_entity->pend_slot.start_time +=
						(idx * req_param->duration);
					req_entity->pend_slot.start_time += req_param->interval;
					next_chn_found = TRUE;
					break;
				}
				idx++;
			}
		}
		req_entity->onchan_chn_idx = req_entity->cur_chn_idx;
		req_entity->cur_chn_idx = idx;
	} else {
		if (!req_entity->req_hdl->req_param->interval) {
			return;
		}

		req_entity->pend_slot.start_time += (req_entity->cur_chn_idx * req_param->duration);
		/* add interval */
		req_entity->pend_slot.start_time += req_param->interval;
	}

	/* Next slot with higher prio than CHAN_CONT req could have a matching
	 * req entity in the next seq, which could cause the pending slot start to
	 * be same as cur_slot since the channel seq start time is moved only
	 * after skip notification of the last channel in the current seq. In
	 * this case the pend slot should be 1 interval more than current pend
	 * start time.
	 */
	if (req_entity->cur_slot.start_time >= req_entity->pend_slot.start_time) {
		req_entity->pend_slot.start_time += req_param->interval;
	}

	req_entity->pend_slot.end_time = req_entity->pend_slot.start_time +
		req_param->duration;
	/* Start time could be tweaked for certain use cases so preserve it
	 * for re-scheduling the same slot
	 */
	req_entity->actual_start_time = req_entity->pend_slot.start_time;
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"channel_index: %d \n",
		req_entity->cur_chn_idx));
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON, "_msch_next_chn_seq_pend_slot: start %u end %u\n",
		(uint32)req_entity->pend_slot.start_time,
		(uint32)req_entity->pend_slot.end_time));
	/* add the req_entity back to the list */
	msch_list_sorted_add(&msch_info->msch_start_fixed_list,
		&req_entity->start_fixed_link,
		OFFSETOF(msch_req_entity_t, pend_slot) -
		OFFSETOF(msch_req_entity_t, start_fixed_link) +
		OFFSETOF(msch_req_timing_t, start_time),
		sizeof(uint64), MSCH_ASCEND);
}

static void
_msch_next_pend_slot(wlc_msch_info_t *msch_info, msch_req_entity_t *req_entity)
{
	uint32 chansw_time;
	uint64 cur_time = msch_current_time(msch_info);
	wlc_msch_req_param_t *req_param = req_entity->req_hdl->req_param;
	uint64 time_gap = 0;
	uint64 gap_in_intervals = 0;

	if (MSCH_START_FIXED(req_param->req_type)) {
		msch_list_remove(&req_entity->start_fixed_link);

		/* start time fixed and periodic request */
		if (req_param->interval) {
			wlc_uint64_add(&req_param->start_time_h,
				&req_param->start_time_l, 0, req_param->interval);
			req_entity->pend_slot.start_time =
				((uint64)req_param->start_time_h << 32) |
				req_param->start_time_l;
			req_entity->pend_slot.end_time = req_entity->pend_slot.start_time +
				req_param->duration;
			/* Start time could be tweaked for certain use cases so preserve it
			 * for re-scheduling the same slot
			 */
			req_entity->actual_start_time = req_entity->pend_slot.start_time;

			/* add the req_entity back to the list */
			msch_list_sorted_add(&msch_info->msch_start_fixed_list,
				&req_entity->start_fixed_link,
				OFFSETOF(msch_req_entity_t, pend_slot) -
				OFFSETOF(msch_req_entity_t, start_fixed_link) +
				OFFSETOF(msch_req_timing_t, start_time),
				sizeof(uint64), MSCH_ASCEND);
		}
	} else if (MSCH_START_FLEX(req_param->req_type)) {
		if (req_param->interval) {
			/*
			 * Put the serviced (front) item to the end of list that has
			 * the same priority so that periodic reqeusts can be serviced
			 * in order based on priority
			 */
			_msch_prio_list_rotate(msch_info,
				&req_entity->rt_specific_link,
				OFFSETOF(msch_req_entity_t, rt_specific_link));
		} else {
			msch_list_remove(&req_entity->rt_specific_link);
		}
	} else {
		msch_list_remove(&req_entity->both_flex_list);

		/* both flex, list is not priority based */
		if (req_param->interval) {
			wlc_uint64_add(&req_param->start_time_h,
				&req_param->start_time_l, 0, req_param->interval);

			req_entity->pend_slot.start_time =
				((uint64)req_param->start_time_h << 32) |
				req_param->start_time_l;

			if (req_entity->pend_slot.start_time < cur_time) {
				time_gap = (cur_time - req_entity->pend_slot.start_time);
				gap_in_intervals = math_div_64((uint32)(time_gap >> 32),
					(uint32)time_gap, req_param->interval);
				req_entity->pend_slot.start_time +=
					((uint64)req_param->interval * (gap_in_intervals + 1));
				if (!req_entity->cur_slot.timeslot &&
					!req_entity->chan_ctxt->bf_sch_pending) {
					req_entity->chan_ctxt->bf_skipped_count +=
						(uint8)gap_in_intervals;
				}
				req_param->start_time_h =
					(uint32)(req_entity->pend_slot.start_time >> 32);
				req_param->start_time_l = (uint32)req_entity->pend_slot.start_time;
			}

			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);
			req_entity->pend_slot.pre_start_time =
				req_entity->pend_slot.start_time - chansw_time;
			req_entity->pend_slot.end_time = req_entity->pend_slot.start_time +
				req_param->duration;

			msch_list_sorted_add(&msch_info->msch_both_flex_req_entity_list,
				&req_entity->both_flex_list,
				OFFSETOF(msch_req_entity_t, pend_slot) -
				OFFSETOF(msch_req_entity_t, both_flex_list) +
				OFFSETOF(msch_req_timing_t, start_time),
				sizeof(uint64), MSCH_ASCEND);
		}
	}
}

static void
_msch_create_pend_slot(wlc_msch_info_t *msch_info, msch_timeslot_t *timeslot)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity;
	uint64 prev_pend_start_time;

	UNUSED_PARAMETER(prev_pend_start_time);

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_create_pend_slot: Entry\n"));
	ASSERT(timeslot);

	/* iterate all req_entities for periodic request */
	elem = &timeslot->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (req_entity->pend_slot.timeslot != timeslot ||
			req_entity->cur_slot.timeslot != NULL) {
			continue;
		}
		req_entity->cur_slot = req_entity->pend_slot;
		/* prep time (channel switch time is valid only for one schedule
		 * reset the value
		 */
		req_entity->chsw_time = 0;
		prev_pend_start_time = req_entity->pend_slot.start_time;
		memset(&req_entity->pend_slot, 0, sizeof(msch_req_timing_t));
		if (req_entity->req_hdl->req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
			_msch_next_chn_seq_pend_slot(msch_info, req_entity);
			ASSERT(req_entity->pend_slot.start_time > prev_pend_start_time);
		} else {
			_msch_next_pend_slot(msch_info, req_entity);
		}
	}
}

/* function to do ON_CHAN and OFF_CHAN callback for all req in the timeslot */
static void
_msch_timeslot_chn_clbk(wlc_msch_info_t *msch_info, msch_timeslot_t *ts, uint32 clbk_type)
{
	msch_list_elem_t *elem, *req_entity_list;
	msch_req_entity_t *req_entity;
	wlc_msch_cb_info_t cb_info;
	wlc_msch_onchan_info_t onchan;
	msch_req_timing_t *slot;
	uint64 start_time;
	wlc_info_t *wlc = msch_info->wlc;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_timeslot_chn_clbk: Entry clbk_type 0x%x\n",
		clbk_type));
	ASSERT(ts);

	req_entity_list = &ts->chan_ctxt->req_entity_list;
	msch_mark_request_clbk_pending(req_entity_list);

	memset(&cb_info, 0, sizeof(cb_info));
	cb_info.type = clbk_type;
	while ((elem = msch_list_iterate_pend_callback(req_entity_list))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		slot = &req_entity->cur_slot;
		cb_info.req_hdl = req_entity->req_hdl;
		cb_info.chanspec = req_entity->chanspec;

		if (!req_entity->cur_slot.timeslot) {
			continue;
		}
		if (clbk_type == MSCH_CT_PRE_ONCHAN) {
			if (slot->flags & MSCH_RC_FLAGS_PRE_ONFIRE_DONE) {
				continue;
			}
			slot->flags |= MSCH_RC_FLAGS_PRE_ONFIRE_DONE;
			cb_info.type = MSCH_CT_PRE_ONCHAN;
			onchan.timeslot_id = ts->timeslot_id;
			cb_info.type_specific = NULL;
			if (req_entity->req_hdl->flags & MSCH_REQ_HDL_FLAGS_NEW_REQ) {
				cb_info.type |= MSCH_CT_PRE_REQ_START;
			}
			/* Set the current radio chanspec so user is aware of
			 * from and to chanspec
			 */
			cb_info.chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
		}

		if (cb_info.type == MSCH_CT_OFF_CHAN_DONE) {
			if (!MSCH_BOTH_FLEX(req_entity->req_hdl->req_param->req_type)) {
				continue;
			}
			if (msch_info->next_timeslot &&
				(req_entity->pend_slot.timeslot == msch_info->next_timeslot) &&
				(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
				 cb_info.type |= MSCH_CT_SLOT_CONTINUE;
			}
		}
		if (clbk_type == MSCH_CT_ON_CHAN) {
			if (slot->flags & MSCH_RC_FLAGS_ONFIRE_DONE) {
				continue;
			}

#ifdef BCMDBG_MSCH_GPIO
			msch_toggle_gpio(msch_info, MSCH_GPIO_ON_CHAN, TRUE);
#endif // endif

			slot->flags |= MSCH_RC_FLAGS_ONFIRE_DONE;
			cb_info.type = MSCH_CT_ON_CHAN;
			onchan.timeslot_id = ts->timeslot_id;
			onchan.onchan_idx = req_entity->onchan_chn_idx;
			start_time =
				((uint64)req_entity->req_hdl->req_param->start_time_h << 32) |
				req_entity->req_hdl->req_param->start_time_l;
			onchan.end_time_h = (uint32)(slot->end_time >> 32);
			onchan.end_time_l = (uint32)slot->end_time;
			if (req_entity->req_hdl->req_param->flags &
				MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
				if (req_entity->onchan_chn_idx ==
					req_entity->req_hdl->last_chan_idx) {
					onchan.cur_chan_seq_start_time =
						start_time -
						req_entity->req_hdl->req_param->interval;
				} else {
					onchan.cur_chan_seq_start_time = start_time;
				}
				/* Get the slot start time */
				start_time = onchan.cur_chan_seq_start_time +
					(onchan.onchan_idx *
					req_entity->req_hdl->req_param->duration);
				onchan.start_time_h = (uint32)(start_time >> 32);
				onchan.start_time_l = (uint32)start_time;
			} else {
				onchan.start_time_h = (uint32)(slot->start_time >> 32);
				onchan.start_time_l = (uint32)slot->start_time;
			}

			cb_info.type_specific = &onchan;

			if (slot->start_time <= msch_current_time(msch_info)) {
				cb_info.type |= MSCH_CT_SLOT_START;
				slot->flags |= MSCH_RC_FLAGS_START_FIRE_DONE;
			}
			if (req_entity->req_hdl->flags & MSCH_REQ_HDL_FLAGS_NEW_REQ) {
				req_entity->req_hdl->flags &= ~MSCH_REQ_HDL_FLAGS_NEW_REQ;
				cb_info.type |= MSCH_CT_REQ_START;
			}

			if (slot->flags & MSCH_RC_FLAGS_SPLIT_SLOT_START) {
				cb_info.type |= MSCH_CT_PARTIAL;
			}
		}
		if (cb_info.type == MSCH_CT_OFF_CHAN) {
			if (!MSCH_BOTH_FLEX(req_entity->req_hdl->req_param->req_type) ||
				(req_entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE)) {
				continue;
			}
			req_entity->cur_slot.flags |= MSCH_RC_FLAGS_END_FIRE_DONE;
			req_entity->last_serv_time = msch_current_time(msch_info);

#ifdef BCMDBG_MSCH_GPIO
			msch_toggle_gpio(msch_info, MSCH_GPIO_ON_CHAN, FALSE);
#endif // endif
		}

		_msch_dump_callback(msch_info->profiler, &cb_info);
		req_entity->req_hdl->cb_func(req_entity->req_hdl->cb_ctxt,
			&cb_info);
	}
}

/* Mark all clbks in msch element list as pending */
static void
msch_mark_request_clbk_pending(msch_list_elem_t *req_list)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity;

	elem = req_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		req_entity->req_hdl->flags |= MSCH_REQ_HDL_FLAGS_CLBK_PENDING;
	}
}

/* Return first element corresponds to pending clbk */
static msch_list_elem_t *
msch_list_iterate_pend_callback(msch_list_elem_t *req_list)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity;
	wlc_msch_req_handle_t *req_hdl;
	bool clbk_pending = FALSE;

	elem = req_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		req_hdl = req_entity->req_hdl;
		if (req_hdl->flags & MSCH_REQ_HDL_FLAGS_CLBK_PENDING) {
			req_hdl->flags &= ~MSCH_REQ_HDL_FLAGS_CLBK_PENDING;
			clbk_pending = TRUE;
			break;
		}
	}
	return (clbk_pending ? elem : NULL);
}

/* Create a list of all SLOT_STAR/SLOT_END notification for the TS */
static void
_msch_schd_slot_start_end(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd_slot_start_end: Entry\n"));
	ASSERT(ts);

	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);

		if (!req_entity->cur_slot.timeslot ||
			msch_elem_inlist(&req_entity->cur_slot.link)) {
			continue;
		}

		if (!(req_entity->cur_slot.flags & MSCH_RC_FLAGS_START_FIRE_DONE)) {
			req_entity->curts_fire_time = req_entity->cur_slot.start_time;
			msch_list_sorted_add(&msch_info->msch_req_timing_list,
				&req_entity->cur_slot.link,
				(OFFSETOF(msch_req_entity_t, curts_fire_time) -
				OFFSETOF(msch_req_entity_t, cur_slot) -
				OFFSETOF(msch_req_timing_t, link)),
				sizeof(req_entity->curts_fire_time), MSCH_ASCEND);
		} else if (!(req_entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE) &&
			req_entity->cur_slot.end_time != MSCH_INFINITE_TIME) {
				req_entity->curts_fire_time = req_entity->cur_slot.end_time;
			msch_list_sorted_add(&msch_info->msch_req_timing_list,
				&req_entity->cur_slot.link,
				(OFFSETOF(msch_req_entity_t, curts_fire_time) -
				OFFSETOF(msch_req_entity_t, cur_slot) -
				OFFSETOF(msch_req_timing_t, link)),
				sizeof(req_entity->curts_fire_time), MSCH_ASCEND);
		}
	}
}

/* Prepare the timer for cur or next timeslot */
static void
_msch_arm_chsw_timer(wlc_msch_info_t *msch_info)
{
	uint64 cur_time = 0, switch_time = 0; /* ms */
	msch_timeslot_t* timeslot = NULL;
	uint64 new_fire_time = 0;
	msch_list_elem_t *elem;
	msch_req_timing_t *slot;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_arm_chsw_timer: Entry\n"));

	/* Check if cur_timer need new fire time */
	if (msch_info->cur_msch_timeslot) {
		if ((elem = msch_list_iterate(&msch_info->msch_req_timing_list))) {
			slot = LIST_ENTRY(elem, msch_req_timing_t, link);
			if (slot->flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
				new_fire_time = slot->end_time;
			} else {
				new_fire_time = slot->start_time;
			}
			/*  newly added request to cur_ts might still not be in req_timing list
			 * make sure fire time is earliest
			 */
			if (new_fire_time != MSCH_INFINITE_TIME &&
				new_fire_time < msch_info->cur_msch_timeslot->fire_time) {
				timeslot = msch_info->cur_msch_timeslot;
				timeslot->fire_time = new_fire_time;
				goto done;
			}
		}
	}

	if (msch_info->cur_msch_timeslot &&
		msch_info->cur_msch_timeslot->fire_time != MSCH_INFINITE_TIME) {
		if (msch_info->next_timeslot &&
			msch_info->cur_msch_timeslot->end_time != MSCH_INFINITE_TIME &&
			((msch_info->cur_msch_timeslot->end_time >
			msch_info->next_timeslot->pre_start_time))) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"_msch_arm_chsw_timer:new_ts overlap cur_end %u next_start %u\n",
				(uint32)msch_info->cur_msch_timeslot->end_time,
				(uint32)msch_info->next_timeslot->pre_start_time));
			ASSERT(FALSE);
		}
		return;
	}

	if (msch_info->next_timeslot == NULL) {
		MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
			"_msch_arm_chsw_timer: no next timeslot to schedule\n"));
		return;
	}

	msch_info->next_timeslot->fire_time = msch_info->next_timeslot->pre_start_time;
	timeslot = msch_info->next_timeslot;
done:
	cur_time = msch_current_time(msch_info);
	switch_time = (timeslot->fire_time <= cur_time) ? 0 : (timeslot->fire_time - cur_time);
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_arm_chsw_timer: current time: %x-%x, switch_time: %x-%x, diff: %u\n",
		(uint32)(cur_time >> 32), (uint32)cur_time,
		(uint32)(switch_time >> 32), (uint32)switch_time,
		(uint32)(switch_time - cur_time)));

	/* arm timer */
	ASSERT(msch_info->chsw_timer != NULL);
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_arm_chsw_timer: wlc_msch_add_timeout: %d\n",
		(int)(switch_time)));
	/* Done for current timeslot, remove the timer */
	if (msch_info->cur_armed_timeslot) {
		wlc_hrt_del_timeout(msch_info->chsw_timer);
		msch_info->cur_armed_timeslot = NULL;
	}

	msch_info->cur_armed_timeslot = timeslot;
	if (msch_info->cur_msch_timeslot &&
		msch_info->cur_msch_timeslot != timeslot) {
		if (_msch_check_bf_timeslot(msch_info, msch_info->cur_msch_timeslot,
			TRUE)) {
			msch_info->cur_msch_timeslot->state = MSCH_TIMER_STATE_OFF_CHN_PREP;
		} else {
			msch_info->cur_msch_timeslot->state = MSCH_TIMER_STATE_CHN_SW;
		}
		/* Next TS valid, make sure cur_ts has end time.  */
		if (msch_info->cur_msch_timeslot->end_time == MSCH_INFINITE_TIME) {
			msch_info->cur_msch_timeslot->end_time = timeslot->pre_start_time;
		}
	}
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_arm_chsw_timer : switch_time: %d.%06d, diff %d\n",
		_msch_display_time(timeslot->fire_time), (int)(switch_time)));
	/* add the timer for the next schd task */
	wlc_msch_add_timeout(msch_info->chsw_timer, (int)(switch_time),
		_msch_chsw_timer, msch_info);
}

static void
_msch_update_chan_ctxt_bw(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *chan_ctxt)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	chanspec_t cur_chanspec = 0;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_update_chan_ctxt_bw: Entry\n"));
	ASSERT(chan_ctxt);

	elem = &chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (!cur_chanspec) {
			cur_chanspec = entity->chanspec;
		}
		if (CHSPEC_BW(entity->chanspec) > CHSPEC_BW(cur_chanspec)) {
			cur_chanspec = entity->chanspec;
		}
	}
	if (cur_chanspec && cur_chanspec != chan_ctxt->chanspec) {
		chan_ctxt->chanspec = cur_chanspec;
	}
}

static void
_msch_update_service_interval(wlc_msch_info_t *msch_info)
{
	msch_list_elem_t *elem, *elem2;
	msch_req_entity_t *entity;
	msch_chan_ctxt_t *tmp_ctxt;
	bool interval_change = FALSE;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_update_service_interval: Entry\n"));

	msch_info->max_lo_prio_interval = 0;
	elem = &msch_info->msch_both_flex_list;
	while ((elem = msch_list_iterate(elem))) {
		tmp_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, bf_link);
		elem2 = &tmp_ctxt->bf_entity_list;
		while ((elem2 = msch_list_iterate(elem2))) {
			entity = LIST_ENTRY(elem2, msch_req_entity_t, rt_specific_link);
			if (entity->req_hdl->req_param->interval >
				msch_info->max_lo_prio_interval) {
				msch_info->max_lo_prio_interval =
					entity->req_hdl->req_param->interval;
				interval_change = TRUE;
			}
		}
	}

	if (msch_info->flex_list_cnt > 1) {
		interval_change = TRUE;
		elem = &msch_info->msch_both_flex_list;
		while ((elem = msch_list_iterate(elem))) {
			tmp_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, bf_link);
			tmp_ctxt->actual_onchan_dur = 0;
			tmp_ctxt->pend_onchan_dur = 0;
		}
	}

	if (interval_change) {
		if (msch_info->max_lo_prio_interval == 0 ||
			msch_info->flex_list_cnt == 0) {
				MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
					"_msch_update_service_interval: interval: %d "
					"flex_list_cnt %d\n",
					msch_info->max_lo_prio_interval,
					msch_info->flex_list_cnt));
			return;
		}

		msch_info->service_interval = msch_info->max_lo_prio_interval /
			msch_info->flex_list_cnt;
	}
}

static void
_msch_prio_list_rotate(wlc_msch_info_t *msch_info, msch_list_elem_t *list,
		uint32 offset)
{
	msch_req_entity_t *entity, *entity2 = NULL;
	msch_list_elem_t *elem, *prev_elem;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_prio_list_rotate: Entry\n"));

	ASSERT(list);
	ASSERT(msch_elem_inlist(list));

	entity = ((msch_req_entity_t *)((char *)(list) - offset));
	elem = prev_elem = list->next;
	while (elem) {
		entity2 = ((msch_req_entity_t *)((char *)(elem) - offset));
		if (entity->priority == entity2->priority) {
			prev_elem = elem;
			elem = elem->next;
			continue;
		} else {
			break;
		}
	}

	if (entity2) {
		msch_list_remove(list);
		msch_list_add_at(prev_elem, list);
	}
}

/* piggyback a new request to current timeslot */
static void
_msch_curts_schd_new_req(wlc_msch_info_t *msch_info, msch_req_entity_t *entity)
{
	uint64 cur_time = msch_current_time(msch_info);
	if (!msch_info->cur_msch_timeslot) {
		ASSERT(0);
		return;
	}

	entity->pend_slot.timeslot = msch_info->cur_msch_timeslot;
	_msch_create_pend_slot(msch_info, msch_info->cur_msch_timeslot);
	msch_info->cur_msch_timeslot->fire_time = cur_time;
	if (msch_info->cur_armed_timeslot) {
		wlc_hrt_del_timeout(msch_info->chsw_timer);
	}
	msch_info->cur_armed_timeslot = msch_info->cur_msch_timeslot;

	/* callback should not be done on caller context,
	 * Start timer immediately to do callback
	 */
	wlc_msch_add_timeout(msch_info->chsw_timer, (int)(0),
		_msch_chsw_timer, msch_info);
}

/* Algorithm to piggyback a new request on current timeslot chan context */
static int
_msch_schd_new_piggyback_curts(wlc_msch_info_t *msch_info, msch_req_entity_t *new_entity)
{
	uint64 dur_left, curtime = msch_current_time(msch_info);
	wlc_msch_req_param_t *req_param = new_entity->req_hdl->req_param;
	msch_timeslot_t	*cur_ts = msch_info->cur_msch_timeslot;
	bool update_curts = TRUE;
	msch_req_entity_t *entity2;
	msch_list_elem_t *elem;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd_new_piggyback_curts: Entry\n"));
	ASSERT(cur_ts);

	/* TODO: Start fixed piggyback is not supported till scb txq is
	 * supported
	 */
	if (MSCH_START_FIXED(req_param->req_type)) {
		return MSCH_SCHD_NEXT_SLOT;
	}

	if (MSCH_START_FIXED(req_param->req_type)) {
		if (new_entity->pend_slot.start_time > cur_ts->end_time) {
			return MSCH_SCHD_NEXT_SLOT;
		}

		if (new_entity->pend_slot.end_time < cur_ts->end_time) {
			_msch_curts_schd_new_req(msch_info, new_entity);
			return MSCH_OK;
		} else if (!msch_info->next_timeslot ||
			(msch_info->next_timeslot &&
			msch_info->next_timeslot->pre_start_time >
				new_entity->pend_slot.end_time)) {
			cur_ts->end_time = new_entity->pend_slot.end_time;
			_msch_curts_schd_new_req(msch_info, new_entity);
			return MSCH_OK;
		}

		/* check next slot */
		if (new_entity->pend_slot.end_time > cur_ts->end_time &&
			(msch_info->next_timeslot != NULL) &&
			(new_entity->pend_slot.end_time >
				msch_info->next_timeslot->pre_start_time)) {

			/* check if we can extend the cur_ts end_time to new_req
			 * end time by cutting next slot
			 */
			elem = &msch_info->next_timeslot->chan_ctxt->req_entity_list;
			while ((elem = msch_list_iterate(elem))) {
				entity2 = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
				if (entity2->priority > new_entity->priority &&
					entity2->pend_slot.pre_start_time <
					new_entity->pend_slot.end_time) {

					/* Check if new req duration can be adjusted */
					if (req_param->req_type == MSCH_RT_DUR_FLEX &&
						(entity2->pend_slot.pre_start_time >=
						(new_entity->pend_slot.end_time -
							req_param->flex.dur_flex))) {
						new_entity->pend_slot.end_time =
							entity2->pend_slot.pre_start_time;
						_msch_curts_schd_new_req(msch_info, new_entity);
						return MSCH_OK;
					}
					update_curts = FALSE;
					break;
				}

				if (MSCH_START_FIXED(
					entity2->req_hdl->req_param->req_type)) {
					update_curts = FALSE;
					break;
				}
			}
		}

		if (update_curts &&
			!(msch_info->next_timeslot &&
			(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY))) {
			if (msch_info->next_timeslot) {
				_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
				msch_info->next_timeslot = NULL;
			}
			if (cur_ts->end_time < new_entity->pend_slot.end_time) {
				cur_ts->end_time = new_entity->pend_slot.end_time;
			}
			_msch_curts_schd_new_req(msch_info, new_entity);
		}
	}
	else {
		/* !START_FIXED */
		dur_left = (cur_ts->end_time > curtime) ?
			(cur_ts->end_time - curtime) : 0;

		if (dur_left < MSCH_MIN_FREE_SLOT) {
			return MSCH_SCHD_NEXT_SLOT;
		}

		/* since the callback happens in timer, account for the delay */
		if (MSCH_START_FLEX(req_param->req_type) &&
			req_param->duration > dur_left) {
			return MSCH_SCHD_NEXT_SLOT;
		}

		new_entity->pend_slot.pre_start_time =
			msch_current_time(msch_info);
		new_entity->pend_slot.start_time =
			new_entity->pend_slot.pre_start_time;
		if (MSCH_BOTH_FLEX(req_param->req_type)) {
			/* If new req comes from BOTH_FLEX, we need to subtract
			 * the PM extra time to sync with the general timeline
			 * for other req types
			 */
			if (cur_ts->end_time != MSCH_INFINITE_TIME) {
				new_entity->pend_slot.end_time =
					cur_ts->end_time - msch_info->pm_notif_dur_us;
			} else {
				/* piggy back 2 both flex on same channel (slot remain unlimited) */
				new_entity->pend_slot.end_time = cur_ts->end_time;
			}
		} else {
			new_entity->pend_slot.end_time =
				new_entity->pend_slot.start_time + req_param->duration;
		}
		_msch_curts_schd_new_req(msch_info, new_entity);
		return MSCH_ALREADY_SCHD;
	}
	return MSCH_OK;
}

/* New request, check if it can be piggyback to cur_ts
 * MSCH_FAIL - memory failure
 * MSCH_OK - no action needed complete the user request
 * MSCH_SCHD_NEXT_SLOT - next timeslot could be invalid so req
 * run the algorithm for next timeslot
 */
static msch_ret_type_t
_msch_schd_new_req(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl)
{
	msch_chan_ctxt_t *chan_ctxt;
	msch_req_entity_t *new_entity, *entity2;
	msch_list_elem_t *elem;
	msch_timeslot_t	*cur_ts = msch_info->cur_msch_timeslot;
	wlc_msch_req_param_t *req_param;
	wlc_info_t *wlc = msch_info->wlc;
	uint32 chansw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd_new_req: Entry\n"));
	elem = msch_list_iterate(&req_hdl->req_entity_list);
	if (!elem) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d:_msch_schd_new_req: error entity list empty \n",
			msch_info->wlc->pub->unit));
		return MSCH_FAIL;
	}
	new_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
	req_param = new_entity->req_hdl->req_param;

	if (!msch_info->cur_msch_timeslot) {
		/* No current timeslot so re-run the msch schd algo */
		goto schd_next;
	}

	chan_ctxt = cur_ts->chan_ctxt;
	/* Same channel context, check piggy back */
	if (chan_ctxt == new_entity->chan_ctxt) {
		/* Change in channel bandwidth */
		if (chan_ctxt->chanspec != WLC_BAND_PI_RADIO_CHANSPEC) {
			_msch_chan_adopt(msch_info, chan_ctxt);
		}
		if (_msch_schd_new_piggyback_curts(msch_info,
			new_entity) == MSCH_SCHD_NEXT_SLOT) {
			goto schd_next;
		}
		return MSCH_ALREADY_SCHD;
	} else if (msch_info->next_timeslot) {
		if (MSCH_START_FIXED(req_param->req_type) &&
			(new_entity->pend_slot.start_time >
			(msch_info->next_timeslot->end_time + chansw_time))) {
			return MSCH_OK;
		}

		/* check priority */
		elem = &msch_info->next_timeslot->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity2 = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if (entity2->priority > new_entity->priority) {
				return MSCH_OK;
			}
		}
	} else if (MSCH_START_FIXED(req_param->req_type)) {
		/* Make sure the start_time is after cur_ts */
		if (cur_ts->end_time != MSCH_INFINITE_TIME &&
			cur_ts->end_time >
			(new_entity->pend_slot.start_time - chansw_time)) {
			if (!req_param->interval) {
				return MSCH_FAIL;
			}
		}
	}

schd_next:
	if (msch_info->next_timeslot) {
		if (!(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
			/* re-schedule next_ts next */
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
				FALSE);
			msch_info->next_timeslot = NULL;
		} else {
			return MSCH_OK;
		}
	}

	return MSCH_SCHD_NEXT_SLOT;
}

static msch_req_entity_t*
_msch_get_entity(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl, chanspec_t chanspec)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;

	elem = &req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		if (entity->chanspec == chanspec) {
			return entity;
		}
	}
	return NULL;
}

static void
_msch_sch_fixed(wlc_msch_info_t *msch_info, msch_req_entity_t *entity, uint32 prep_dur,
	uint64 slot_start_time)
{
	wlc_msch_req_handle_t *req_hdl = entity->req_hdl;
	chanspec_t next_chan;
	uint32 chansw_time = 0;
	msch_req_entity_t *next_entity = NULL;

	if (entity->pend_slot.timeslot) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: _msch_sch_fixed: timeslot already scheduled\n",
			msch_info->wlc->pub->unit));
		ASSERT(0);
		return;
	}
	/* For contigous channels, check the next channel to cut duration of this
	 * timeslot
	 */
	if (req_hdl->req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
		if (entity->pend_slot.flags & MSCH_RC_FLAGS_SPLIT_SLOT_START) {
			/* In case of preempt slot cancel/unregistration, remaining
			 * split slot could get more time , increare the start time
			 * based on available time
			 */
			if ((slot_start_time < (entity->pend_slot.start_time + prep_dur)) &&
				((entity->pend_slot.end_time - slot_start_time) <
				entity->req_hdl->req_param->duration)) {
				entity->pend_slot.start_time = (slot_start_time + prep_dur);
				MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
					"_msch_sch_fixed: update remaing slot start_time "
					"%d.%06d\n",
					_msch_display_time(entity->pend_slot.start_time)));
			}
		}
		if (entity->cur_chn_idx == (req_hdl->chan_cnt - 1)) {
			if (req_hdl->req_param->interval >
				(req_hdl->chan_cnt * req_hdl->req_param->duration)) {
				/* Channel sequence is not back to back */
				next_chan = -1;
			} else {
				next_chan = req_hdl->chn_list[0];
			}
		} else {
			next_chan = req_hdl->chn_list[entity->cur_chn_idx + 1];
		}

		if ((next_chan != (chanspec_t)-1) &&
			!WLC_CHAN_COEXIST(entity->chan_ctxt->chanspec, next_chan)) {
			next_entity = _msch_get_entity(msch_info, req_hdl, next_chan);
			if (next_entity) {
				chansw_time = wlc_msch_entity_chswtime(msch_info, next_entity,
					entity->chan_ctxt->chanspec);
			}
			entity->pend_slot.end_time -= chansw_time;
		}
	}

	entity->pend_slot.pre_start_time =
		entity->pend_slot.start_time - prep_dur;
	msch_info->next_timeslot = _msch_timeslot_request(msch_info,
		entity->chan_ctxt, entity->req_hdl->req_param,
		&entity->pend_slot);
	entity->pend_slot.timeslot = msch_info->next_timeslot;
}

/* Get the last slot end time for the current timeslot
 * return 0 if no slots are pending or valid end time
 */
static uint64
_msch_curts_slot_endtime(wlc_msch_info_t *msch_info)
{
	uint64 end_time = 0;
	msch_list_elem_t *elem = &msch_info->msch_req_timing_list;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	msch_req_entity_t *entity;

	ASSERT(cur_ts);

	/* Get valid end time for cur_ts */
	elem = &cur_ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (entity->cur_slot.timeslot == cur_ts &&
			entity->cur_slot.end_time != MSCH_INFINITE_TIME &&
			entity->cur_slot.end_time > end_time) {
			end_time = entity->cur_slot.end_time;
		}
	}
	return end_time;
}

/* Get the next slot fire time */
static uint64
_msch_get_next_slot_fire(wlc_msch_info_t *msch_info)
{
	msch_list_elem_t *elem = NULL;
	msch_req_timing_t *slot = NULL;

	if ((elem = msch_list_iterate(&msch_info->msch_req_timing_list))) {
		slot = LIST_ENTRY(elem, msch_req_timing_t, link);

		/* check if the next slot time is due */
		if (slot->flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
			return slot->end_time;
		} else {
			return slot->start_time;
		}
	}

	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"_msch_get_next_slot_fire: no slot fire pending\n"));
	/* If element is not in the list, return max time */
	return -1;
}

static msch_req_entity_t*
_msch_get_next_sfix(wlc_msch_info_t *msch_info, uint64 slot_start, uint64 prep_dur)
{
	msch_req_entity_t *sfix_entity = NULL, *entity = NULL;
	msch_list_elem_t *elem;
	uint64 chn_prep_dur;
	msch_chan_ctxt_t *cur_chan_ctxt = NULL;
	uint8 max_prio = 0;
	uint32 chansw_time;
	wlc_info_t *wlc = msch_info->wlc;

	if (msch_info->cur_msch_timeslot) {
		cur_chan_ctxt = msch_info->cur_msch_timeslot->chan_ctxt;
	}

	elem = &msch_info->msch_start_fixed_list;
	/* Check Start Fix overlaps */
	while ((elem = msch_list_iterate(elem))) {
		sfix_entity = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);
		chn_prep_dur = prep_dur;
		/* No need of onchannel prepare time for same channel context */
		if (sfix_entity->chan_ctxt != cur_chan_ctxt) {
			chansw_time = wlc_msch_entity_chswtime(msch_info, sfix_entity,
				WLC_BAND_PI_RADIO_CHANSPEC);
			chn_prep_dur += chansw_time;
		}

		/* If cur sfix is in past move to next one
		 * slot_start_time is pre start time
		 */
		/* For split slot adjust the start time to available time */
		if (sfix_entity->pend_slot.start_time >= (slot_start + chn_prep_dur)) {
			break;
		}

		if ((sfix_entity->req_hdl->req_param->flags &
			MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) &&
			(sfix_entity->pend_slot.flags &
			MSCH_RC_FLAGS_SPLIT_SLOT_START) &&
			(sfix_entity->pend_slot.end_time >=
			(slot_start + chn_prep_dur + MSCH_MIN_ONCHAN_TIME))) {
			sfix_entity->pend_slot.start_time =
				(slot_start + chn_prep_dur);
			/* updated the start time so re add it to list */
			msch_list_remove(&sfix_entity->start_fixed_link);

			/* add the req_entity back to the list */
			msch_list_sorted_add(&msch_info->msch_start_fixed_list,
				&sfix_entity->start_fixed_link,
				OFFSETOF(msch_req_entity_t, pend_slot) -
				OFFSETOF(msch_req_entity_t, start_fixed_link) +
				OFFSETOF(msch_req_timing_t, start_time),
				sizeof(sfix_entity->pend_slot.start_time),
				MSCH_ASCEND);
			elem = &msch_info->msch_start_fixed_list;
		}
	}

	/* Check Start Fix overlaps */
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);

		/* Cur sfix entity should not overlap with
		 * next to be scheduled sfix entity
		 */
		if (sfix_entity->chan_ctxt != entity->chan_ctxt) {
			chansw_time = wlc_msch_entity_chswtime(msch_info, entity,
					sfix_entity->chan_ctxt->chanspec);
		} else {
			chansw_time	= wlc_msch_entity_chswtime(msch_info, entity, -1);
		}

		if (sfix_entity->pend_slot.end_time <
			(entity->pend_slot.start_time - chansw_time)) {
			break;
		}

		/* swithing to same channel dose not require prep time
		 * but the next sfix of different channel might require
		 * extra prep time and that could cause overlap so check
		 * next
		 */
		if ((sfix_entity->chan_ctxt == entity->chan_ctxt) &&
			(sfix_entity->pend_slot.end_time ==
			entity->pend_slot.start_time)) {
			/* mark priority to avoid scheduling lower priority */
			if ((sfix_entity->priority < entity->priority) &&
				(max_prio < entity->priority)) {
					max_prio = entity->priority;
			}
			continue;
		}

		if ((sfix_entity->priority < entity->priority) &&
			(max_prio < entity->priority)) {
			sfix_entity = entity;
		}
	}

	/* Check for contigous both fixed slots, SLOT_END takes care of merging
	 * pend slot to current slot don't schedule it as next_timeslot
	 */
	if (sfix_entity && sfix_entity->cur_slot.timeslot &&
		(sfix_entity->req_hdl->req_param->flags &
		MSCH_REQ_FLAGS_MERGE_CONT_SLOTS)) {
		sfix_entity = NULL;
	}
	return sfix_entity;
}

/* MSCH slot skip timer */
static void
_msch_slotskip_timer(void *arg)
{
	wlc_msch_info_t *msch_info = (wlc_msch_info_t *)arg;
	msch_list_elem_t *req_elem, *prev_elem;
	uint64 cur_time, slot_due, slot_end_time;
	int switch_dur;
	msch_timeslot_t *cur_ts;
	msch_req_entity_t *entity = NULL;
	msch_ret_type_t ret;
	chanspec_t last_chanspec = 0;
	uint32 chansw_time;
	wlc_info_t *wlc;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_slotskip_timer: Entry\n"));
	if (!msch_info) {
		WL_ERROR(("_msch_slotskip_timer: msch_info is NULL\n"));
		ASSERT(FALSE);
		return;
	}
	wlc = msch_info->wlc;
	msch_info->slotskip_flag = FALSE;

	cur_time = msch_current_time(msch_info);
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_slotskip_timer: current_time: %d.%06d\n",
		_msch_display_time(cur_time)));

	req_elem = &msch_info->msch_start_fixed_list;
	while ((req_elem = msch_list_iterate(req_elem))) {
		entity = LIST_ENTRY(req_elem, msch_req_entity_t, start_fixed_link);
		prev_elem = req_elem->prev;

		cur_ts = msch_info->cur_msch_timeslot;

		if (msch_info->next_timeslot) {
			slot_end_time = msch_info->next_timeslot->end_time;
			last_chanspec = msch_info->next_timeslot->chan_ctxt->chanspec;
		} else if (cur_ts) {
			slot_end_time = cur_ts->end_time;
			last_chanspec = cur_ts->chan_ctxt->chanspec;
		} else {
			/* No timeslot active, just make sure the all necessary
			 * slots are skipped based on current time
			 * This condition could happen when there is a delay in
			 * timers due to calibration or other system activities
			 */
			MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
				"Current and next time slots are NULL, check for cur time\n"));
			slot_end_time = cur_time;
			last_chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
		}

		if (entity->pend_slot.timeslot) {
			continue;
		}

		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"_msch_slotskip_timer: entity %p start time : %d.%06d, channel 0x%x\n",
			entity,
			_msch_display_time(entity->pend_slot.start_time), entity->chanspec));

		slot_due = entity->pend_slot.start_time;
		if (entity->chan_ctxt->chanspec != last_chanspec) {
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, entity->chanspec);
			slot_due -= chansw_time;
		}

		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON, "cur time : %d.%06d slot_due %u\n",
			_msch_display_time(cur_time), (uint32)slot_due));
		switch_dur = (int)(slot_due - MSCH_SKIP_PRE_NOTIF_TIME - cur_time);

		/* If the cur/next slot is unlimited, skip based on current time */
		if (slot_end_time == MSCH_INFINITE_TIME) {
			slot_end_time = cur_time;
			WL_ERROR(("slotskip > slot end time is -1, making it cur time!"));
		}

		if (slot_due >= slot_end_time) {
			/* Two entity with same start time and different channel
			 * one channel same as last chanspec and the next one
			 * on different channel. The prep times differ for both
			 * so check the next entity if we are in margin
			 */
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);
			if ((entity->chan_ctxt->chanspec == last_chanspec) &&
				((slot_due - slot_end_time) <= chansw_time)) {
				continue;
			} else {
				break;
			}
		}

		/* skip the slot if
		 * 1. entity's prestart time is less than current time
		 * 2. entity's start time is less than current/next slot end time
		 * and there is not much switch duration left
		 */
		if ((cur_time >= slot_due) || (switch_dur < 0)) {
			ret = _msch_slot_skip(msch_info, entity);
			if (ret != MSCH_TIMESLOT_REMOVED &&
				(entity->req_hdl->req_param->interval == 0)) {
				wlc_msch_timeslot_unregister(msch_info, &entity->req_hdl);
			} else if (ret == MSCH_TIMESLOT_REMOVED) {
				req_elem = prev_elem;
				continue;
			}
			/* restart from head of list */
			req_elem = &msch_info->msch_start_fixed_list;
		} else {
			/* Add time out for the current entity if no skip
			 * is needed at current stage
			 */
			msch_info->slotskip_flag = TRUE;
			wlc_msch_add_timeout(msch_info->slotskip_timer,
				switch_dur, _msch_slotskip_timer, msch_info);
			break;
		}
	}

	if (!msch_info->slotskip_flag && !msch_info->next_timeslot) {
		_msch_schd(msch_info);
	}
}

/*
 * Schedule request for next timeslot
 */
static void
_msch_schd(wlc_msch_info_t *msch_info)
{
	wlc_info_t *wlc = msch_info->wlc;
	msch_list_elem_t *elem;
	uint64 dfix_end_time = 0;
	/* Initialize slot_dur to maximum size slot */
	uint64 actual_slot_dur, slot_dur = -1;
	uint32 chansw_time = 0, prep_dur = 0;
	msch_req_entity_t *entity = NULL;
	msch_req_entity_t *sfix_entity = NULL;
	msch_req_entity_t *dfix_entity = NULL;
	msch_req_entity_t *bflex_entity = NULL;
	msch_req_timing_t *sfix_slot = NULL;
	uint8 sfix_prio, dfix_prio, bflex_prio;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 slot_start_time = msch_current_time(msch_info);
	uint64 bf_start_time, req_dur = 0, bf_min_dur = 0;
	chanspec_t cur_chspec = WLC_BAND_PI_RADIO_CHANSPEC;
	uint32 max_prep_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd: Entry\n"));

	msch_info->flags &= ~MSCH_STATE_SCHD_PENDING;
	sfix_prio = dfix_prio = bflex_prio = MSCH_RP_DISC_BCN_FRAME;

	if (msch_info->next_timeslot) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: _msch_schd: redundant call\n",
			msch_info->wlc->pub->unit));
		return;
	}

	/* Get the next slot start time */
	slot_start_time = _msch_get_next_start_time(msch_info);
	if (cur_ts && (cur_ts->end_time == MSCH_INFINITE_TIME)) {
		if (_msch_check_bf_timeslot(msch_info, cur_ts, TRUE)) {
			prep_dur += msch_info->pm_notif_dur_us;
		}
	}

	sfix_entity = _msch_get_next_sfix(msch_info, slot_start_time, prep_dur);

	if (sfix_entity) {
		sfix_slot = &sfix_entity->pend_slot;
		sfix_prio = sfix_entity->priority;
		chansw_time = wlc_msch_entity_chswtime(msch_info, sfix_entity, cur_chspec);
		if (sfix_slot->start_time > (prep_dur + slot_start_time + chansw_time)) {
			/* slot dur is the whole slot that includes pre start time */
			slot_dur = sfix_slot->start_time - slot_start_time;
		} else {
			uint64 slot_avail = prep_dur + slot_start_time + chansw_time;

			if (sfix_slot->start_time < slot_avail) {
				if (!(sfix_entity->req_hdl->req_param->flags &
						MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) ||
					!(sfix_entity->pend_slot.flags &
						MSCH_RC_FLAGS_SPLIT_SLOT_START) ||
					(sfix_slot->end_time <
						(slot_avail + MSCH_MIN_ONCHAN_TIME))) {
					if (_msch_check_sfix_slot_skip(msch_info, cur_chspec,
						slot_start_time)) {
						MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
							"_msch_schd: skip slot, start skip "
							"timer\n"));
						return;
					}
				}
			}
			slot_dur = 0;
		}
	}
	elem = msch_list_iterate(&msch_info->msch_start_flex_list);
	if (elem) {
		dfix_entity = LIST_ENTRY(elem, msch_req_entity_t, rt_specific_link);
		dfix_prio = dfix_entity->priority;
	}

	/* make sure all entity in BF list are in future time */
	_msch_update_both_flex_list(msch_info, slot_start_time, slot_dur);

	/* Get the next unscheduled bf entity */
	elem = &msch_info->msch_both_flex_req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, both_flex_list);
		if (entity->chan_ctxt->bf_sch_pending) {
			if (bflex_entity == NULL) {
				bflex_entity = entity;
			}
			continue;
		}
		bflex_entity = entity;
		bflex_prio = bflex_entity->priority;
	}

	/* SCHD START Fix as next slot
	 *  - start time due
	 *  - or cur_time + dur_fixed dur exceeds pending start_fix start time
	 */
	if (dfix_entity) {
		chansw_time = wlc_msch_entity_chswtime(msch_info, dfix_entity, cur_chspec);
		dfix_end_time = slot_start_time + prep_dur + chansw_time +
			dfix_entity->req_hdl->req_param->duration;
	}

	if (sfix_entity && ((sfix_slot->start_time <= (slot_start_time + prep_dur)) ||
		(dfix_entity && ((sfix_slot->start_time - prep_dur) < dfix_end_time)))) {
		if (sfix_prio >= dfix_prio && sfix_prio >= bflex_prio) {

			/* If the sfix slot is in current timeslot and the priority is same
			 * as dfix/bflex schedule dfix/bflex timeslot
			 */
			if (!sfix_entity->cur_slot.timeslot ||
				(sfix_prio > dfix_prio && sfix_prio > bflex_prio)) {
				if (cur_chspec != sfix_entity->chan_ctxt->chanspec) {
					chansw_time =
						wlc_msch_entity_chswtime(msch_info, sfix_entity,
						cur_chspec);
				} else {
					chansw_time = 0;
				}
				/* sfix is due, schedule it as next timeslot */
				_msch_sch_fixed(msch_info, sfix_entity, (prep_dur + chansw_time),
					slot_start_time);
				goto done;
			}
		}
	}

	/* SF done, move on to dfix or both flex */
	if (dfix_entity && dfix_prio >= bflex_prio) {
		chansw_time = 0;
		/* Add max chansw time to req_dur to accomadate chsw time after dfix schedule
		 * since we are not sure of chanspec after this dfix use max
		 */
		req_dur = dfix_entity->req_hdl->req_param->duration + prep_dur + max_prep_time;

		if (cur_chspec  != dfix_entity->chan_ctxt->chanspec) {
			chansw_time = wlc_msch_entity_chswtime(msch_info, dfix_entity, cur_chspec);
			/* Add chsw time for cur chanspec to dfix switch */
			req_dur += chansw_time;
		}

		if (_msch_check_avail_extend_slot(msch_info, dfix_entity, slot_start_time,
			req_dur) >= req_dur) {
			if (!bflex_entity || !_msch_check_both_flex(msch_info,
				dfix_entity->req_hdl->req_param->duration) ||
				dfix_prio > bflex_prio) {
				/* schedule start flex */
				if (_msch_sch_start_flex(msch_info, dfix_entity, slot_start_time,
					(prep_dur + chansw_time)) == MSCH_OK) {
					msch_info->next_timeslot = dfix_entity->pend_slot.timeslot;
					goto done;
				}
			}
		}
	}

	/* If bflex prio is greater then check if bflex slot can be extended */
	if (bflex_entity) {
		chansw_time = 0;
		if (cur_chspec != bflex_entity->chan_ctxt->chanspec) {
			chansw_time = wlc_msch_entity_chswtime(msch_info, bflex_entity, cur_chspec);
		}
		/* chsw before bflex schd + min_dur + estimate chsw afte bflex schd */
		bf_min_dur = bflex_entity->req_hdl->req_param->flex.bf.min_dur +
			prep_dur + chansw_time + msch_info->pm_notif_dur_us + max_prep_time;

		if (bflex_prio > sfix_prio) {
			if (slot_dur < bf_min_dur) {
				slot_dur = _msch_check_avail_extend_slot(msch_info, bflex_entity,
					slot_start_time, bf_min_dur);
			}
		}

		/* - We check if user requeted min_dur (e.g. 45ms) can be serviced
		 * - udpate the requirement with acceptable min slot
		 */
		bf_min_dur -= bflex_entity->req_hdl->req_param->flex.bf.min_dur;
		bf_min_dur += MSCH_MIN_ONCHAN_TIME;
	}

	if (bflex_entity && (slot_dur >= bf_min_dur)) {
		/* VSDB is in use */
		if (msch_info->flex_list_cnt > 1) {
			bf_start_time =
				(bflex_entity->pend_slot.start_time - prep_dur - chansw_time);
			/* VSDB could have updated the start time of next connection
			 * so check if it overlaps with current timeslot
			 */
			if (cur_ts && (slot_start_time > bf_start_time)) {
				_msch_reduce_curts(msch_info, bf_start_time);
			}
			else if ((slot_dur != MSCH_INFINITE_TIME) &&
				(slot_start_time < bf_start_time)) {
				if (slot_dur >= (prep_dur + chansw_time + MSCH_MIN_FREE_SLOT +
					(bflex_entity->pend_slot.start_time - slot_start_time))) {
					slot_dur -=
						(bflex_entity->pend_slot.start_time -
						slot_start_time);
				}
				else {
					/* Don't schedule BF slot */
					slot_dur = 0;
				}
			}
			slot_start_time = bf_start_time;
		}

		if (slot_dur && slot_dur != MSCH_INFINITE_TIME) {
			actual_slot_dur = slot_dur - (prep_dur + chansw_time +
				msch_info->pm_notif_dur_us + max_prep_time);
			slot_dur -= (prep_dur + chansw_time);
			if (msch_info->flex_list_cnt > 1) {
				slot_dur -= (msch_info->pm_notif_dur_us + max_prep_time);
			} else if (sfix_entity &&
				(bflex_entity->chan_ctxt->chanspec !=
					sfix_entity->chan_ctxt->chanspec)) {
				/* Schedule STA based on next sfix slot to avoid
				 * gaps in STA schedule
				 */
				slot_dur -= msch_info->pm_notif_dur_us;
				slot_dur -= wlc_msch_entity_chswtime(msch_info, sfix_entity,
					bflex_entity->chan_ctxt->chanspec);
			}

			/* If the available slot big enough to accomdate more requests then schedule
			 * minimum bf time
			 */
			if (dfix_entity && req_dur > 0 && actual_slot_dur >
				(bflex_entity->req_hdl->req_param->flex.bf.min_dur + req_dur)) {
				slot_dur = bflex_entity->req_hdl->req_param->flex.bf.min_dur;
			}
		}
		if (slot_dur && _msch_sch_both_flex(msch_info, slot_start_time,
			slot_dur, bflex_entity, (prep_dur + chansw_time)) == MSCH_OK) {
			goto done;
		}
	}

	/* Nothing is due so go back to start fix */
	if (sfix_entity) {
		chansw_time = wlc_msch_entity_chswtime(msch_info, sfix_entity, cur_chspec);
		_msch_sch_fixed(msch_info, sfix_entity, (prep_dur + chansw_time), slot_start_time);
		goto done;
	}

	return;

done:
	if (msch_info->next_timeslot) {
		uint64 next_end_time = msch_info->next_timeslot->end_time;
		if (msch_info->next_timeslot->end_time == MSCH_INFINITE_TIME) {
			slot_dur = -1;
		} else {
			slot_dur = next_end_time -
				(msch_info->next_timeslot->pre_start_time + prep_dur + chansw_time);
		}
		_msch_schd_next_chan_ctxt(msch_info, msch_info->next_timeslot,
			(prep_dur + chansw_time), slot_dur);
		if (msch_info->cur_msch_timeslot &&
			msch_info->cur_msch_timeslot->end_time == MSCH_INFINITE_TIME) {
			msch_info->next_timeslot->pre_start_time +=
				msch_info->pm_notif_dur_us;
			_msch_reduce_curts(msch_info,
				msch_info->next_timeslot->pre_start_time);
		}

		/* Use slot skip timer to schedule slot skip */
		_msch_check_sfix_slot_skip(msch_info, msch_info->next_timeslot->chan_ctxt->chanspec,
			next_end_time);
	}
	_msch_arm_chsw_timer(msch_info);
}

static bool
_msch_check_sfix_slot_skip(wlc_msch_info_t *msch_info, chanspec_t chanspec, uint64 end_time)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint32 chsw_time;
	uint32 max_chsw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	/* Get the next Start Fixed from list */
	elem = &msch_info->msch_start_fixed_list;
	while ((elem = msch_list_iterate(elem))) {
		chsw_time = 0;
		entity = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);

		if (chanspec != entity->chanspec) {
			chsw_time = wlc_msch_entity_chswtime(msch_info, entity, chanspec);
		}

		/* the entity is already scheduled */
		if (entity->pend_slot.timeslot) {
			continue;
		}

		if (entity->pend_slot.start_time >= (end_time + max_chsw_time)) {
			break;
		}

		/* The next slot time might need a skip due to prep duration
		 * so check the next entity before breaking loop
		 */
		if (entity->pend_slot.start_time >= (end_time + chsw_time)) {
			continue;
		}
		if (msch_info->slotskip_flag) {
			/* delete the timer */
			wlc_hrt_del_timeout(msch_info->slotskip_timer);
		}
		msch_info->slotskip_flag = TRUE;
		wlc_msch_add_timeout(msch_info->slotskip_timer,
			0, _msch_slotskip_timer, msch_info);
		return TRUE;
	}
	return FALSE;
}

static void
_msch_curts_add_pm_time_to_slot(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint64 new_end_time =  msch_current_time(msch_info) + MSCH_DELAY_FOR_CLBK;
	bool new_end_update = FALSE;

	/* iterate all req_entities for periodic request */
	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type) &&
			(entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE)) {
			ASSERT(entity->cur_slot.end_time != MSCH_INFINITE_TIME);
			entity->cur_slot.flags &= ~MSCH_RC_FLAGS_END_FIRE_DONE;
			entity->cur_slot.end_time = new_end_time;
			entity->curts_fire_time = entity->cur_slot.end_time;
			msch_list_sorted_add(&msch_info->msch_req_timing_list,
				&entity->cur_slot.link,
				(OFFSETOF(msch_req_entity_t, curts_fire_time) -
				OFFSETOF(msch_req_entity_t, cur_slot) -
				OFFSETOF(msch_req_timing_t, link)),
				sizeof(entity->curts_fire_time), MSCH_ASCEND);
			new_end_update = TRUE;
		}
	}
	if (new_end_update) {
		if (ts->end_time < (new_end_time + msch_info->pm_notif_dur_us)) {
			ts->end_time = new_end_time + msch_info->pm_notif_dur_us;
		}
		_msch_slot_start_end_notif(msch_info, ts);
	}
}

static bool
_msch_check_bf_timeslot(wlc_msch_info_t *msch_info,  msch_timeslot_t *ts, bool cur_ts)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;

	/* iterate all req_entities for periodic request */
	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (!MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type)) {
			continue;
		}
		if ((cur_ts && entity->cur_slot.timeslot == ts) ||
			(!cur_ts && entity->pend_slot.timeslot == ts)) {
			return TRUE;
		}
	}
	return FALSE;
}

/*  The "entity" is overlapping with current timeslot, same chan ctxt as
 * cur timeslot except end_time overflow current ts end_time.
 * Check if the current timeslot ie extendable to include the entity
 */
static bool
_msch_ts_extend(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	msch_req_entity_t *entity)
{
	msch_req_entity_t *sfix_entity;
	uint32 prep_time = 0;
	msch_list_elem_t *elem = &msch_info->msch_start_fixed_list;
	while ((elem = msch_list_iterate(elem))) {
		sfix_entity = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);
		if (sfix_entity->pend_slot.timeslot ||
			(sfix_entity->chan_ctxt == entity->chan_ctxt)) {
			continue;
		}

		prep_time = wlc_msch_calc_avg_chswtime(msch_info,
				sfix_entity->chan_ctxt->chanspec);
		if (sfix_entity->pend_slot.start_time >
			(entity->pend_slot.end_time + prep_time)) {
			break;
		}
		if (entity->req_hdl->req_param->priority <=
			sfix_entity->req_hdl->req_param->priority) {
			return FALSE;
		}
	}
	return TRUE;
}

static void
_msch_update_next_bf_endtime(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint64 bf_end_time = ts->end_time - msch_info->pm_notif_dur_us;

	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		/* It is important to update the bf end time to end of ts
		 * after piggyback to have good sta tput
		 */
		if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type) &&
			(entity->pend_slot.timeslot == ts) &&
			(entity->pend_slot.end_time != bf_end_time)) {
			MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON, "update bf end: %u\n",
				(uint32)bf_end_time));
			entity->pend_slot.end_time = bf_end_time;
		}
	}
}

static void
_msch_schd_next_chan_ctxt(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	uint32 prep_dur, uint64 slot_dur)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity, *ts_entity;
	uint64 start_time, new_end_time = ts->end_time;
	uint32 min_bf_dur = MSCH_MIN_ONCHAN_TIME + msch_info->pm_notif_dur_us;
	wlc_msch_req_handle_t *ts_hdl = NULL;
	uint64 old_end_time = ts->end_time;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd_next_chan_ctxt: Entry\n"));
	start_time = ts->pre_start_time + prep_dur;

	/* ts_entity gets the first entity in the timeslot. Before piggybacking
	 * there'll be only one entity scheduled by _msch_schd
	 */
	if ((ts_entity = wlc_get_ts_entity(msch_info, ts)) == NULL) {
		ASSERT(FALSE);
		return;
	}
	ts_hdl = ts_entity->req_hdl;

	/* iterate all req_entities for periodic request */
	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);

		/* More than 1 channel conext for single request,
		 * sharring ts might change the order of the request
		 */
		if ((entity->req_hdl == ts_hdl) ||
			entity->pend_slot.timeslot ||
			((entity->req_hdl->chan_cnt > 1) &&
			!entity->req_hdl->req_param->interval)) {
			/* Scan piggyback could change the channel sequence so don't piggyback
			 * scan use case (non periodic multi channel request)
			 */
			continue;
		}

		/* START_FIXED - schd slots which are within the ts */
		if (MSCH_START_FIXED(entity->req_hdl->req_param->req_type)) {
			/* piggyback only overlapped slot */
			if (entity->pend_slot.start_time > new_end_time) {
				continue;
			}

			if (entity->pend_slot.start_time < start_time) {
				/* Get the next slot start time */
				uint64 next_avail_time =
					_msch_get_next_start_time(msch_info);
				next_avail_time += prep_dur;
				/* If req start time is earlier then the ts start time,
				 * need to set the ts start time as the eariler one
				 */
				if (next_avail_time < entity->pend_slot.start_time) {
					ts->pre_start_time =
						(entity->pend_slot.start_time - prep_dur);
				} else {
					/* other case we don't need to consider */
					continue;
				}
			}

			/* Overlap slot check if the current ts can be extended */
			if (entity->pend_slot.end_time > new_end_time) {
				if (!_msch_ts_extend(msch_info, ts, entity)) {
					continue;
				}
				ts->end_time = entity->pend_slot.end_time;
			}
			entity->pend_slot.timeslot = ts;
			entity->pend_slot.pre_start_time = ts->pre_start_time;
			continue;
		}

		if (MSCH_START_FLEX(entity->req_hdl->req_param->req_type) &&
			entity->cur_slot.timeslot) {
			continue;
		}

		if ((MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type) &&
			(min_bf_dur <= slot_dur)) ||
			entity->req_hdl->req_param->duration <= slot_dur) {

			if (entity->chan_ctxt->bf_sch_pending &&
				entity->pend_slot.timeslot) {
				continue;
			}

			entity->pend_slot.timeslot = ts;
			entity->pend_slot.pre_start_time = ts->pre_start_time;
			entity->pend_slot.start_time = ts->pre_start_time + prep_dur;

			if (MSCH_START_FLEX(entity->req_hdl->req_param->req_type)) {
				entity->pend_slot.end_time = (entity->pend_slot.start_time +
				entity->req_hdl->req_param->duration);
			} else {
				entity->chan_ctxt->bf_sch_pending = TRUE;
				if (slot_dur != MSCH_INFINITE_TIME) {
					entity->pend_slot.end_time =
						(entity->pend_slot.start_time + slot_dur -
						msch_info->pm_notif_dur_us);
				} else {
					entity->pend_slot.end_time = -1;
				}
			}
		}
	}
	/* update the both flex if the end time is changed
	 */
	 if (old_end_time != ts->end_time) {
		ASSERT(ts->end_time != MSCH_INFINITE_TIME);
		_msch_update_next_bf_endtime(msch_info, ts);
	 }
}

/* check if the duration can be extended by skipping some
 * more start fixed slots based on requested priority
 */
static uint64
_msch_check_avail_extend_slot(wlc_msch_info_t *msch_info, msch_req_entity_t *entity,
	uint64 slot_start, uint64 dur)
{
	msch_req_entity_t *sfix_entity;
	msch_list_elem_t *elem = &msch_info->msch_start_fixed_list;
	uint8 req_prio = entity->priority;
	uint64 req_dur = dur;
	uint32 chansw_time;

	while ((elem = msch_list_iterate(elem))) {
		sfix_entity = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);

		req_dur = dur;
		if (entity->chan_ctxt == sfix_entity->chan_ctxt) {
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);
			req_dur -= chansw_time;
		}

		/* ignore slots before the slot start */
		if (slot_start > sfix_entity->pend_slot.start_time) {
			continue;
		}
		/* Got requested duration, return success */
		if (req_dur <= (sfix_entity->pend_slot.start_time - slot_start)) {
			/* For both flex provide the actual available duration */
			if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type)) {
				return (sfix_entity->pend_slot.start_time - slot_start);
			} else {
				return dur;
			}
		}

		if (sfix_entity->priority > req_prio) {
			/* The requested duration is not available return
			 * available dur
			 */
			return (sfix_entity->pend_slot.start_time - slot_start);
		}
	}
	return dur;
}

static bool
wlc_msch_add_timeout(wlc_hrt_to_t *to, int timeout, wlc_hrt_to_cb_fn fun, void *arg)
{
	if (timeout <= 0) {
		timeout = 1;
	}
	/* hrt_add_timeout timer does not support 0 */
	return wlc_hrt_add_timeout(to, timeout, fun, arg);
}

/* start_flex list valid or vsdb (more than 1 both flex chan_ctxt)
 * return chan_ctxt pending as TRUE
 */
static bool
_msch_check_pending_chan_ctxt(wlc_msch_info_t *msch_info, msch_chan_ctxt_t *cur_ctxt)
{
	msch_list_elem_t *elem;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_check_pending_chan_ctxt: Entry\n"));

	if (!msch_list_empty(&msch_info->msch_start_flex_list)) {
		/* Pending Start flex req */
		return TRUE;
	}

	elem = &msch_info->msch_both_flex_list;
	while ((elem = msch_list_iterate(elem))) {
		msch_chan_ctxt_t *chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, bf_link);
		if (chan_ctxt != cur_ctxt) {
			/* VSDB (2 both flex chan ctxt exist) */
			/* TODO  just check the bf count instead */
			return TRUE;
		}
	}
	return FALSE;
}

static void
_msch_update_both_flex_list(wlc_msch_info_t *msch_info, uint64 slot_start, uint64 slot_dur)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity = NULL, *next_entity = NULL;
	uint64 cur_time = msch_current_time(msch_info);
	uint64 next_bf_pre_start = slot_start + slot_dur;
	uint32 max_chsw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	elem = &msch_info->msch_both_flex_req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, both_flex_list);
		/* check if the request is past */
		if (req_entity->pend_slot.start_time < cur_time) {
			/* Get next BF element */
			if (elem->next) {
				next_entity = LIST_ENTRY(elem->next, msch_req_entity_t,
					both_flex_list);
				next_bf_pre_start = next_entity->pend_slot.start_time -
					max_chsw_time;
			}

			/* check if we can still give some time for the current
			 * both flex without exceeding the next both flex
			 * start
			 */
			if ((req_entity->pend_slot.end_time < slot_start) &&
				(slot_start > next_bf_pre_start) &&
				((slot_start - next_bf_pre_start) > MSCH_MIN_FREE_SLOT)) {
				break;
			}

			/* Make sure the request is in the future. */
			_msch_next_pend_slot(msch_info, req_entity);
			elem = &msch_info->msch_both_flex_req_entity_list;
			continue;
		}
	}
}

/*
 * This function checks if a both flexible is due
 * Shedules both flexible if away time exceeds max_away_dur
 * or bf_skipped_count is greater than max threshold
 */
static bool
_msch_check_both_flex(wlc_msch_info_t *msch_info, uint32 req_dur)
{
	msch_list_elem_t *elem, *elem2;
	msch_req_entity_t *req_entity = NULL;
	msch_req_entity_t *next_entity = NULL;
	uint64 cur_time = msch_current_time(msch_info);
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 min_dur = -1;
	uint32 max_chsw_time = wlc_msch_calc_avg_chswtime(msch_info, -1);

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_check_both_flex: Entry\n"));

	/*
	* If both_flex scheduled in current_ts, then allow dfix entity
	*/
	if (cur_ts && cur_ts->chan_ctxt->bf_sch_pending) {
		return FALSE;
	}

	elem = &msch_info->msch_both_flex_req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, both_flex_list);

		/* Already scheduled as cur or next timeslot */
		if (req_entity->chan_ctxt->bf_sch_pending) {
			/* Check the next entry */
			continue;
		}

		/* Condition to schedule a Both flex entity
		 *  - First time schedule or connection in VSDB mode skipped too long.
		 *  or a both flex start time due
		 */
		if (!req_entity->last_serv_time ||
			(msch_info->flex_list_cnt > 1 &&
			req_entity->chan_ctxt->bf_skipped_count > MAX_BOTH_FLEX_SKIP_COUNT)) {
			req_entity->chan_ctxt->bf_skipped_count = 0;
			return TRUE;
		}

		/* Check if other connection is due */
		if (msch_info->flex_list_cnt > 1) {
			/* Get time to next BF start */
			elem2 = elem;
			while ((elem2 = msch_list_iterate(elem2))) {
				next_entity = LIST_ENTRY(elem2, msch_req_entity_t, both_flex_list);
				if (next_entity->chan_ctxt == req_entity->chan_ctxt) {
					continue;
				}

				if (next_entity->pend_slot.start_time > cur_time) {
					min_dur = cur_time - next_entity->pend_slot.start_time;
					min_dur -= max_chsw_time;
				}
				min_dur = MIN(min_dur,
					req_entity->req_hdl->req_param->flex.bf.min_dur);
			}

			/* cur BF slot need to be skipped */
			if (cur_time >= (req_entity->pend_slot.start_time + min_dur)) {
				memset(&req_entity->pend_slot, 0, sizeof(msch_req_timing_t));
				_msch_next_pend_slot(msch_info, req_entity);

				/* list is updated so check from the begining */
				elem = &msch_info->msch_both_flex_req_entity_list;
				continue;
			}

			if (req_entity->pend_slot.start_time > (cur_time + req_dur)) {
				/* BF not due */
				return FALSE;
			}
		}

		/* reached max_away limit, schd_bf */
		if (req_entity->req_hdl->req_param->flex.bf.max_away_dur <
			((cur_time - req_entity->last_serv_time)
				+ req_dur)) {
				return TRUE;
		}
		break;
	}
	return FALSE;
}

/*
 * This function checks if there is an extra prep time required when
 * a new pre-emptive request is placed
 * e.g. In a chn seq cur channel and next channel could be same so there
 * is no channel switch time required but if there is a pre-emptive
 * request in between with a different channel then an extra prep
 * time is required
 */
static bool
_msch_check_chn_seq_extra_prep_dur(wlc_msch_info_t *msch_info, msch_req_entity_t *entity,
	chanspec_t chspec)
{
	chanspec_t next_chan = -1;
	if (entity->onchan_chn_idx == (entity->req_hdl->chan_cnt - 1)) {
		if (entity->req_hdl->req_param->interval >
			(entity->req_hdl->chan_cnt * entity->req_hdl->req_param->duration)) {
			/* Channel sequence is not back to back */
			next_chan = -1;
		} else {
			next_chan = entity->req_hdl->chn_list[0];
		}
	} else {
		next_chan = entity->req_hdl->chn_list[entity->onchan_chn_idx + 1];
	}

	if ((next_chan != (chanspec_t)-1) &&
		WLC_CHAN_COEXIST(entity->chan_ctxt->chanspec, next_chan) &&
		!WLC_CHAN_COEXIST(next_chan, chspec)) {
		return TRUE;
	}
	return FALSE;
}

/*
 * This function checks if the current timeslot request is pre-emptable and
 * if there is enough time left to support requested duration
 */
static uint64
_msch_check_curts_preemption(wlc_msch_info_t *msch_info, uint32 duration,
	chanspec_t chanspec, uint64 *curts_end_time, uint32 flags, bool *split_curts)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 ts_end_time, new_end_time = 0;
	uint64 cur_time = msch_current_time(msch_info);
	uint32 preempt_slot_prep = 0;
	uint64 preempt_start_time =  cur_time + MSCH_DELAY_FOR_CLBK;
	uint32 extra_prep_time = 0;
	uint32 chansw_time;

	if (!curts_end_time) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_check_curts_preemption: Cur ts end time is not valid\n"));
		return -1;
	}

	if (!split_curts) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"_msch_check_curts_preemption: split_curts %p Null parameters\n",
			split_curts));
		ASSERT(0);
		return -1;
	}

	/* reset the value */
	*curts_end_time = -1;

	if (cur_ts->end_time != MSCH_INFINITE_TIME) {
		ts_end_time = cur_ts->end_time;
	} else {
		ts_end_time = _msch_curts_slot_endtime(msch_info);
	}

	*split_curts = ((flags & MSCH_REQ_FLAGS_PREMT_IMMEDIATE) != 0);
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_check_curts_preemption: Cur ts end_time %d.%06d\n",
		_msch_display_time(ts_end_time)));

	/* _msch_curts_slot_endtime returns 0 when no slots are pending
	 * cur_ts->end_time is -1 when no other MSCH req pending
	 * In above conditions the cur_ts can be reduced
	 */
	if (!ts_end_time || ts_end_time == MSCH_INFINITE_TIME) {
		/* Only BF is pending so cut the slot after MIN slot duration */
		ts_end_time = cur_time + MSCH_DELAY_FOR_CLBK;
		preempt_slot_prep += msch_info->pm_notif_dur_us;
		if (!WLC_CHAN_COEXIST(chanspec, cur_ts->chan_ctxt->chanspec)) {
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info,
				chanspec);
			preempt_slot_prep += chansw_time;
		}
		/* found end time and preempt_slot_prep time */
		goto done;
	}

	ts_end_time -= duration;
	/* check if there is a channel switch */
	if (!WLC_CHAN_COEXIST(chanspec, cur_ts->chan_ctxt->chanspec)) {
		chansw_time = wlc_msch_calc_avg_chswtime(msch_info, chanspec);
		preempt_slot_prep += chansw_time;
		ts_end_time -= chansw_time;
	}

	if (_msch_check_bf_timeslot(msch_info, cur_ts, TRUE)) {
		preempt_slot_prep += msch_info->pm_notif_dur_us;
		ts_end_time -= msch_info->pm_notif_dur_us;
	}

	if (ts_end_time <= msch_current_time(msch_info)) {
		return -1;
	}

	/* Check all the pending requests in current timeslot for preemption */
	elem = &cur_ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (entity->cur_slot.timeslot &&
			(entity->cur_slot.timeslot == cur_ts)) {
			if (!(entity->req_hdl->req_param->flags &
				MSCH_REQ_FLAGS_PREMTABLE)) {
				MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
					"_msch_check_curts_preemption: preempt req fail, cur ts "
					"does not support\n"));
				return -1;
			}
			if (!extra_prep_time &&
				(entity->req_hdl->req_param->flags &
				MSCH_REQ_FLAGS_CHAN_CONTIGUOUS)) {
				if (_msch_check_chn_seq_extra_prep_dur(msch_info, entity,
					chanspec)) {
					chansw_time = wlc_msch_calc_avg_chswtime(msch_info,
						chanspec);
					extra_prep_time = chansw_time;
					ts_end_time -= extra_prep_time;
					/* kick out if extra prep is not possible */
					if (ts_end_time <= msch_current_time(msch_info)) {
						return -1;
					}
				}
			}
		}
	}

done:
	if (*split_curts) {
		/* check the possible end time with a pre-empt slot in between
		 * prep_time for preempt slot + preempt slot dur + prepare
		 * time for coming back to cur_ts + minimun required time
		 * for curt_ts
		 */
		chansw_time = wlc_msch_calc_avg_chswtime(msch_info,
			chanspec);
		new_end_time = (preempt_start_time + preempt_slot_prep +
			duration + chansw_time + MSCH_MIN_ONCHAN_TIME);
	}

	if (*split_curts && (ts_end_time > preempt_start_time) &&
		(cur_ts->end_time >= new_end_time)) {
		/*  PREEMPT req and comeback to cur slot */
		*curts_end_time = preempt_start_time;
		preempt_start_time += preempt_slot_prep;
	} else if (flags & MSCH_REQ_FLAGS_PREMT_CURTS) {
		/*  PREEMPT slot at the end of cur slot */
		*curts_end_time = ts_end_time;
		preempt_start_time = ts_end_time + preempt_slot_prep;
		/* If preempt request is scheduled at the end of cur_ts
		 * there is no enough ts remaining to re-schd clear split_curts
		 */
		*split_curts = FALSE;
	} else {
		*split_curts = FALSE;
		return -1;
	}

	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_check_curts_preemption: preempt_start_time %d.%06d "
		"curts_end_time %d.%06d\n",
		_msch_display_time(ts_end_time), _msch_display_time(*curts_end_time)));

	/* Above conditions confirm preempting cur slot is feasible */
	return preempt_start_time;
}

/*
 * Update the current timeslot with the new end time
 */
static void
_msch_reduce_curts(wlc_msch_info_t *msch_info, uint64 end_time)
{
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint64 next_fire;

	if (!cur_ts || cur_ts->end_time <= end_time) {
		return;
	}

	/* Update the end time */
	cur_ts->end_time = end_time;
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_reduce_curts: Cutting ts end time %d.%06d\n",
		_msch_display_time(msch_info->cur_msch_timeslot->end_time)));

	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"_msch_reduce_curts: Updating current ts end time to %d.%06d\n",
		_msch_display_time(end_time)));

	elem = &cur_ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if (!entity->cur_slot.timeslot) {
			continue;
		}

		if (MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type)) {
			entity->cur_slot.end_time = end_time - msch_info->pm_notif_dur_us;
		} else {
			entity->cur_slot.end_time = end_time;
		}

		if (entity->cur_slot.flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
			/* Remove the cur_slot from link */
			msch_list_remove(&entity->cur_slot.link);
			entity->curts_fire_time = entity->cur_slot.end_time;
			/* Add the slot to req timing for new end time */
			msch_list_sorted_add(&msch_info->msch_req_timing_list,
				&entity->cur_slot.link,
				(OFFSETOF(msch_req_entity_t, curts_fire_time) -
				OFFSETOF(msch_req_entity_t, cur_slot) -
				OFFSETOF(msch_req_timing_t, link)),
				sizeof(entity->curts_fire_time), MSCH_ASCEND);
		}
	}
	next_fire = _msch_get_next_slot_fire(msch_info);

	if (next_fire == MSCH_INFINITE_TIME) {
		next_fire = end_time;
	}

	if (msch_info->next_timeslot &&
		(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
		msch_info->next_timeslot->ts_flags &= ~MSCH_TS_FLAG_DONT_DESTROY;
	}

	if (cur_ts->fire_time != next_fire) {
		cur_ts->fire_time = next_fire;
		if (msch_info->cur_armed_timeslot) {
			wlc_hrt_del_timeout(msch_info->chsw_timer);
		}
		msch_info->cur_armed_timeslot = cur_ts;
		wlc_msch_add_timeout(msch_info->chsw_timer,
			(int)(cur_ts->fire_time - msch_current_time(msch_info)),
			_msch_chsw_timer, msch_info);
	}
}

static bool
_msch_enity_next_ts_overlap(wlc_msch_info_t *msch_info, msch_req_entity_t *entity)
{
	wlc_info_t *wlc = msch_info->wlc;
	uint64 start_time = entity->pend_slot.start_time;
	uint64 ts_end_time;
	msch_timeslot_t *ts = msch_info->next_timeslot;
	uint32 chansw_time;

	if (ts) {
		ts_end_time = ts->end_time;
		if (ts->chan_ctxt != entity->chan_ctxt) {
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, entity->chanspec);
			ts_end_time += chansw_time;
		}

		if (WLC_BAND_PI_RADIO_CHANSPEC != entity->chan_ctxt->chanspec) {
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info, entity->chanspec);
			start_time -= chansw_time;
		}
		if (start_time < ts_end_time) {
			return TRUE;
		}
	}
	return FALSE;
}

/* Function to check input parameters */
static int
_msch_check_param(wlc_msch_info_t *msch_info, wlc_msch_req_param_t *param)
{
	uint64 time_check;
	uint64 cur_time = msch_current_time(msch_info);

	time_check = (((uint64)param->start_time_h << 32) | param->start_time_l);

	if (param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
		if (!MSCH_BOTH_FIXED(param->req_type)) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wl%d: _msch_check_param: wrong request type for chan "
				"contigous, req failed\n", msch_info->wlc->pub->unit));
			return BCME_BADARG;
		}
		return BCME_OK;
	}

	if (MSCH_START_FIXED(param->req_type)) {
		/* Make sure non periodic fixed request are in the future */
		if (!param->interval && (time_check < (cur_time +
			wlc_msch_calc_avg_chswtime(msch_info, -1)))) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wl%d: _msch_check_param: start time not in future\n",
				msch_info->wlc->pub->unit));
			return BCME_BADARG;
		}
	}

	/* TODO check all possible input configuration */
	return BCME_OK;
}
/* MSCH Exported Functions starts here */

wlc_msch_info_t*
BCMATTACHFN(wlc_msch_attach)(wlc_info_t *wlc)
{
	wlc_msch_info_t *msch_info = NULL;

	msch_info = MALLOCZ(wlc->osh, sizeof(wlc_msch_info_t));
	if (!msch_info) {
		WL_ERROR(("wl%d: wlc_msch_attach: msch_info mallocz failed\n",
			wlc->pub->unit));
		return NULL;
	}
	MSCH_ASSIGN_MAGIC_NUM(msch_info, MSCH_MAGIC_NUMBER);
	msch_info->wlc = wlc;

	msch_info->chsw_timer = wlc_hrt_alloc_timeout(wlc->hrti);
	if (!msch_info->chsw_timer) {
		WL_ERROR(("wl%d: wlc_msch_attach: wlc_hrt_alloc_timeout failed\n",
			wlc->pub->unit));
		goto fail;
	}
	msch_info->slotskip_timer = wlc_hrt_alloc_timeout(wlc->hrti);
	if (!msch_info->slotskip_timer) {
		WL_ERROR(("wl%d: wlc_msch_attach: wlc_hrt_alloc_timeout for slot "
			"skip timer failed\n", wlc->pub->unit));
		goto fail;
	}
	/* Initialize the pm timeout value */
	msch_info->pm_notif_dur_us = MSCH_MAX_OFFCB_DUR;

#if (defined(EVENT_LOG_COMPILE) && defined(MSCH_EVENT_LOG)) || defined(MSCH_PROFILER) \
	|| !defined(DONGLEBUILD)
	msch_info->profiler = wlc_msch_profiler_attach(msch_info);
	if (msch_info->profiler == NULL) {
		WL_ERROR(("wl%d: wlc_msch_attach: wlc_msch_profiler_attach() failed\n",
			wlc->pub->unit));
		goto fail;
	}
#endif /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */

#ifdef BCMDBG_MSCH_GPIO
	WL_ERROR(("Reconfigure GPIO output for debug\n"));
	si_gci_enable_gpio(wlc->pub->sih, MSCH_GPIO_ON_CHAN, 1 << MSCH_GPIO_ON_CHAN, 0);
	si_gci_enable_gpio(wlc->pub->sih, MSCH_GPIO_SLOT_START, 1 << MSCH_GPIO_SLOT_START, 0);
#endif // endif

	return msch_info;

fail:
	MODULE_DETACH(msch_info, wlc_msch_detach);

	return NULL;
}

void
BCMATTACHFN(wlc_msch_detach)(wlc_msch_info_t *msch_info)
{
	msch_list_elem_t *elem;

	if (!msch_info) {
		WL_ERROR(("wlc_msch_detach: msch_info is NULL\n"));
		return;
	}
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	if (msch_info->chsw_timer) {
		wlc_hrt_free_timeout(msch_info->chsw_timer);
		msch_info->chsw_timer = NULL;
	}

	if (msch_info->slotskip_timer) {
		wlc_hrt_free_timeout(msch_info->slotskip_timer);
		msch_info->slotskip_timer = NULL;
	}

	/* free global list */
	while ((elem = msch_list_remove_head(&msch_info->msch_req_hdl_list))) {
		wlc_msch_req_handle_t *req_hdl = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
		wlc_msch_timeslot_unregister(msch_info, &req_hdl);
	}

	while ((elem = msch_list_remove_head(&msch_info->msch_chan_ctxt_list))) {
		msch_chan_ctxt_t *chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, link);
		_msch_chan_ctxt_free(msch_info, chan_ctxt, FALSE);
	}

	/* Free msch_timeslot list */
	while ((elem = msch_list_remove_head(&msch_info->free_timeslot_list))) {
		msch_timeslot_t *timeslot = LIST_ENTRY(elem, msch_timeslot_t, timeslot_list_link);
		_msch_timeslot_free(msch_info, timeslot, FALSE);
	}

	/* free current and next time slot */
	if (msch_info->cur_msch_timeslot) {
		_msch_timeslot_free(msch_info, msch_info->cur_msch_timeslot, FALSE);
	}
	if (msch_info->next_timeslot) {
		_msch_timeslot_free(msch_info, msch_info->next_timeslot, FALSE);
	}

	/* free memory pool */
	while ((elem = msch_list_remove_head(&msch_info->free_req_hdl_list))) {
		wlc_msch_req_handle_t *req_hdl = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
		_msch_req_handle_free(msch_info, req_hdl, FALSE);
	}

	while ((elem = msch_list_remove_head(&msch_info->free_req_entity_list))) {
		msch_req_entity_t *req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		_msch_req_entity_free(msch_info, req_entity, FALSE);
	}

	while ((elem = msch_list_remove_head(&msch_info->free_chan_ctxt_list))) {
		msch_chan_ctxt_t *chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, link);
		_msch_chan_ctxt_free(msch_info, chan_ctxt, FALSE);
	}

#if (defined(EVENT_LOG_COMPILE) && defined(MSCH_EVENT_LOG)) || defined(MSCH_PROFILER) \
	|| !defined(DONGLEBUILD)
	MODULE_DETACH(msch_info->profiler, wlc_msch_profiler_detach);
#endif /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */

	MFREE(msch_info->wlc->osh, msch_info, sizeof(wlc_msch_info_t));
}

/* When low memory state is apparent, try to get doable amount of resources
 * for the current request, and return possible num of channel
 * contexts
 */
int
wlc_msch_allocable_chctxts_num_get(wlc_msch_info_t *msch_info,
	uint32 free_heapsize)
{
	int size_per_ch_base, size_per_req_base;
	int allocable_chans = 1;
	int size_reserved, total_base;
	int entity_cnt, chctxt_cnt;

	size_per_ch_base = sizeof(msch_chan_ctxt_t) + sizeof(chanspec_t);
	size_per_req_base = sizeof(msch_req_entity_t);
	total_base = size_per_ch_base + size_per_req_base;
	size_reserved = entity_cnt = chctxt_cnt = 0;

	/* accumulate the the free list pool availability */
	if (free_heapsize <= (sizeof(wlc_msch_req_handle_t) + total_base)) {
		return allocable_chans;
	} else {
		size_reserved += free_heapsize;
		if (msch_list_empty(&msch_info->free_req_hdl_list)) {
			size_reserved -= sizeof(wlc_msch_req_handle_t);
		}
	}
	entity_cnt = msch_list_length(&msch_info->free_req_entity_list);
	chctxt_cnt = msch_list_length(&msch_info->free_chan_ctxt_list);
	size_reserved += (entity_cnt * sizeof(msch_req_entity_t));
	size_reserved += (chctxt_cnt * sizeof(msch_chan_ctxt_t));

	allocable_chans = (size_reserved > total_base) ?
		size_reserved/total_base : 1;
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"msch_allocable_chctxts_num_get: size_reserved=%d, free_heapsize=%d,"
		"allocable_chans=%d\n",
		size_reserved, free_heapsize, allocable_chans));

	return allocable_chans;
}

/* MSCH runs on usec timer which starts when the device is powered on.
 * All the time is represented in uint64 so it is almost take 1/2
 * millon years to wrap around
 */
int
wlc_msch_timeslot_register(wlc_msch_info_t *msch_info, chanspec_t* chanspec_list,
	int chanspec_cnt, wlc_msch_callback cb_func, void *cb_ctxt, wlc_msch_req_param_t *req_param,
	wlc_msch_req_handle_t **p_req_hdl)
{
	wlc_msch_req_handle_t *req_hdl = NULL;
	msch_req_entity_t *entity;
	uint16 chanspec_num = 0;
	int ret = BCME_OK;
	uint64 new_end_time = 0;
	uint64 preempt_time = -1;
	bool split_curts = FALSE;
	wlc_info_t *wlc = msch_info->wlc;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "wlc_msch_timeslot_register: Entry\n"));
	ASSERT(msch_info && chanspec_list && req_param && p_req_hdl);
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	if ((ret = _msch_check_param(msch_info, req_param)) != BCME_OK) {
		return ret;
	}
	/* check if the current slot is big and can be reduced */
	if (msch_info->cur_msch_timeslot &&
		msch_info->cur_msch_timeslot->end_time != MSCH_INFINITE_TIME) {
		_msch_new_req_update_curts(msch_info);
	}

	if (MSCH_PREEMPT_REQ(req_param->flags)) {
		if (!MSCH_START_FLEX(req_param->req_type) ||
			req_param->duration > MSCH_MAX_PREEMPT_DURATION) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wlc_msch_timeslot_register: Request not supported, type %d "
				"dur %d flags 0x%x\n",
				req_param->req_type, req_param->duration,
				req_param->flags));
			return BCME_BADARG;
		}
		if (msch_info->next_timeslot &&
			(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"ts_register: next ts is fixed due and cannot support pre-empt\n"));
			return BCME_UNSUPPORTED;
		}

		if (!msch_info->cur_msch_timeslot) {
			uint32 prep_time = MSCH_PROCESSING_DELAY;
			/* No current time slot convert the request to fixed non preempt request */
			if (WLC_BAND_PI_RADIO_CHANSPEC != chanspec_list[0]) {
				prep_time +=
					wlc_msch_calc_avg_chswtime(msch_info, chanspec_list[0]);
			}
			preempt_time = msch_current_time(msch_info) + prep_time;
			req_param->flags &=
				~(MSCH_REQ_FLAGS_PREMT_CURTS | MSCH_REQ_FLAGS_PREMT_IMMEDIATE);
			req_param->req_type = MSCH_RT_BOTH_FIXED;
		} else if ((preempt_time =
			_msch_check_curts_preemption(msch_info, req_param->duration,
			chanspec_list[0], &new_end_time, req_param->flags, &split_curts))
				== MSCH_INFINITE_TIME) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wlc_msch_timeslot_register: current timeslot not "
				"pre-emptable.\n"));
			return BCME_UNSUPPORTED;
		}

		/* Update the request param start time */
		req_param->start_time_l = (uint32)preempt_time;
		req_param->start_time_h = (uint32)(preempt_time >> 32);
	}

	if ((req_param->flags & MSCH_REQ_FLAGS_MERGE_CONT_SLOTS) &&
		(req_param->duration != req_param->interval)) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wlc_msch_timeslot_register: Merge slots set but duration (%d) and "
			"interval (%d) are different\n",
			req_param->duration, req_param->interval));
		return BCME_BADARG;
	}
#ifdef MSCH_CHANSPEC_VALID
	{
		int i = 0;
		for (i = 0; i < chanspec_cnt; i++) {
			/* validate chanspec of slots excluding free slots */
			if ((chanspec_list[i] != ((chanspec_t)-1)) &&
				!wlc_valid_chanspec_db(wlc->cmi, chanspec_list[i])) {
				ASSERT(0);
			}
		}
	}
#endif /* MSCH_CHANSPEC_VALID */

	req_hdl = _msch_req_handle_alloc(msch_info);
	if (!req_hdl) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
		"wlc_msch_timeslot_register: msch_req_handle_alloc failed!!\n"));
		return BCME_NOMEM;
	}
	req_hdl->cb_func = cb_func;
	req_hdl->cb_ctxt = cb_ctxt;
	req_hdl->req_param = MALLOCZ(msch_info->wlc->osh, sizeof(wlc_msch_req_param_t));
	req_hdl->chan_cnt = chanspec_cnt;
	req_hdl->flags |= MSCH_REQ_HDL_FLAGS_NEW_REQ;

	if (req_hdl->req_param == NULL) {
		_msch_req_handle_free(msch_info, req_hdl, TRUE);
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wlc_msch_timeslot_register: req_param alloc failed!!\n"));
		return BCME_NOMEM;
	}
	memcpy(req_hdl->req_param, req_param, sizeof(*req_hdl->req_param));

	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"chanspec_list:\n"));
	for (chanspec_num = 0; chanspec_num < chanspec_cnt; chanspec_num++) {
		MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
			"[%02d] 0x%04x(%d)\n", chanspec_num, chanspec_list[chanspec_num],
			(chanspec_list[chanspec_num] & 0xFF)));
	}

	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"flags = 0x%04x\treq_type = %d\tpriority = %d\n",
		req_hdl->req_param->flags, req_hdl->req_param->req_type,
		req_hdl->req_param->priority));
	MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
		"start_time = %x-%x\tduration = %d\tinterval = %d\n",
		req_hdl->req_param->start_time_h, req_hdl->req_param->start_time_l,
		req_hdl->req_param->duration, req_hdl->req_param->interval));

	if (req_hdl->req_param->req_type == MSCH_RT_DUR_FLEX) {
		MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
			"dur_flex = %d\n", req_hdl->req_param->flex.dur_flex));
	} else if (MSCH_BOTH_FLEX(req_hdl->req_param->req_type)) {
		MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
			"min_dur = %d\tmax_away_dur = %d\n",
			req_hdl->req_param->flex.bf.min_dur,
			req_hdl->req_param->flex.bf.max_away_dur));
		MSCH_MESSAGE((msch_info, MSCH_INFORM_ON,
			"hi_prio_time = %x-%x\thi_prio_interval = %d\n",
			req_hdl->req_param->flex.bf.hi_prio_time_h,
			req_hdl->req_param->flex.bf.hi_prio_time_l,
			req_hdl->req_param->flex.bf.hi_prio_interval));
	}

	_msch_dump_register(msch_info->profiler, chanspec_list, chanspec_cnt,
		req_hdl->req_param);

	if (req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
		ASSERT(MSCH_START_FIXED(req_param->req_type));
		ASSERT(req_param->duration > wlc_msch_calc_avg_chswtime(msch_info, 0));
		req_hdl->chn_list = MALLOCZ(msch_info->wlc->osh,
			(sizeof(chanspec_t) * chanspec_cnt));

		if (!req_hdl->chn_list) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wlc_msch_timeslot_register: chanspec list memory alloc "
				"(%d bytes) failed!!\n",
				((sizeof(chanspec_t) * chanspec_cnt))));
			goto fail;
		}
		memcpy(req_hdl->chn_list, chanspec_list, (sizeof(chanspec_t) * chanspec_cnt));
	}

	if (MSCH_PREEMPT_REQ(req_param->flags)) {
		/* kill the next timeslot
		 * no need to check for kill flag since it is taken care above
		 */
		if (msch_info->next_timeslot) {
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
			msch_info->next_timeslot = NULL;
		}
		_msch_reduce_curts(msch_info, new_end_time);

		/* splitting the current timeslot due to a pre-emptive req
		 * re-schedule remaining pend slot
		 */
		if (split_curts) {
			_msch_schd_remaining_cur_slots(msch_info, msch_info->cur_msch_timeslot,
				req_param, chanspec_list[0]);
		}
	}

	/* if still have multiple chanspecs here, it must bf CHAN_CONTIGUOUS */
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
		"wlc_msch_timeslot_register: req_entity_create, chanspec_cnt %d\n",
		chanspec_cnt));
	for (chanspec_num = 0; chanspec_num < chanspec_cnt; chanspec_num++) {
		if (chanspec_list[chanspec_num] == ((chanspec_t)-1)) {
			/* Free slot don't do anything */
			continue;
		}
		/* save the last valid chanspec excluding free slots */
		req_hdl->last_chan_idx = chanspec_num;
		if (MSCH_START_FIXED(req_hdl->req_param->req_type) &&
			(entity = _msch_req_entity_available(msch_info, req_hdl,
			chanspec_list[chanspec_num]))) {
			entity->flags |= MSCH_ENTITY_FLAG_MULTI_INSTANCE;
			continue;
		}
		ret = _msch_req_entity_create(msch_info, chanspec_list[chanspec_num],
			req_hdl, chanspec_num);
		if (ret != BCME_OK) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wlc_msch_timeslot_register: _msch_req_entity_create failed\n"));
			goto fail;
		}

		if (!(req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS)) {
			/* calculate new start time for next chanspec */
			wlc_uint64_add(&req_param->start_time_h, &req_param->start_time_l, 0,
				req_param->duration);
		}
	}

	/* queue req_hdl to msch_info->msch_reqhandle_list */
	msch_list_sorted_add(&msch_info->msch_req_hdl_list, &req_hdl->link, 0, 0, MSCH_NO_ORDER);

	_msch_dump_profiler(msch_info->profiler);

	/* One of the slot could be in the past so do changes to schedule the next
	 * possible slot in the request
	 */
	if (req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
		_msch_chan_cont_req_update(msch_info, req_hdl);
	}

	req_hdl->req_time = msch_current_time(msch_info);
	/* run next timeslot scheduling if MSCH_STATE not in Timer_CTXT
		MSCH_REQ_FLAGS_PREMT_CURTS is a special case, where we should
		schedule when preempted, otherwise check timer_ctxt and proceed.
	*/
	if (!(msch_info->flags & MSCH_STATE_IN_TIEMR_CTXT)) {
		if (MSCH_PREEMPT_REQ(req_param->flags) ||
			((ret = _msch_schd_new_req(msch_info, req_hdl)) != MSCH_ALREADY_SCHD)) {
			if (!msch_info->slotskip_flag) {
				_msch_schd(msch_info);
			}
		}
	}

	if (!msch_info->next_timeslot && !msch_info->slotskip_flag) {
		msch_info->flags |= MSCH_STATE_SCHD_PENDING;
	}
	if (ret != MSCH_FAIL) {
		*p_req_hdl = req_hdl;
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"wlc_msch_timeslot_register: create req_hdl 0x%08x\n",
			(uint32)((uintptr)req_hdl)));
		return BCME_OK;
	}
fail:
	wlc_msch_timeslot_register_cleanup(msch_info, req_hdl);
	*p_req_hdl = NULL;
	return ret;
}

/* Timeslot unregister does not do proper clean up if registration fails
 * due to part of memory allocation failure
 * This routine make sure all the cleanup is done
 */
static void
wlc_msch_timeslot_register_cleanup(wlc_msch_info_t *msch_info,
		wlc_msch_req_handle_t *req_hdl)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity = NULL;
	msch_chan_ctxt_t *chan_ctxt = NULL;
	if (!req_hdl) {
		return;
	}

	/* Free all the request entities */
	while ((elem = msch_list_remove_head(&req_hdl->req_entity_list))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		chan_ctxt = req_entity->chan_ctxt;
		_msch_req_entity_destroy(msch_info, req_entity);
		if (chan_ctxt &&
			msch_list_empty(&chan_ctxt->req_entity_list)) {
			_msch_chan_ctxt_destroy(msch_info, chan_ctxt, TRUE);
		}
	}

	/* unlink the handle */
	msch_list_remove(&req_hdl->link);
	if (req_hdl->req_param) {
		MFREE(msch_info->wlc->osh, req_hdl->req_param, sizeof(*(req_hdl->req_param)));
		req_hdl->req_param = NULL;
	}

	if (req_hdl->chn_list) {
		MFREE(msch_info->wlc->osh, req_hdl->chn_list,
			(sizeof(chanspec_t) * req_hdl->chan_cnt));
		req_hdl->chn_list = NULL;
	}

	/* free req_hdl */
	_msch_req_handle_free(msch_info, req_hdl, TRUE);
}

int wlc_msch_set_chansw_reason(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	uint32 reason)
{
	if (_msch_req_hdl_valid(msch_info, req_hdl) && reason < CHANSW_MAX_NUMBER) {
		req_hdl->chansw_reason = reason;
		return BCME_OK;
	}
	return BCME_ERROR;
}

static void
_msch_schd_remaining_cur_slots(wlc_msch_info_t *msch_info, msch_timeslot_t *ts,
	wlc_msch_req_param_t *preempt_param, chanspec_t preempt_chanspec)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	wlc_msch_req_param_t *split_slot_param;
	uint32 chansw_time;
	uint64 split_slot_start = (((uint64)preempt_param->start_time_h << 32) |
			preempt_param->start_time_l);
	uint64 seq_start = 0;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "_msch_schd_remaining_cur_slots: Entry\n"));
	split_slot_start += preempt_param->duration;
	if (!WLC_CHAN_COEXIST(preempt_chanspec, ts->chan_ctxt->chanspec)) {
		chansw_time = wlc_msch_calc_avg_chswtime(msch_info,
			preempt_chanspec);
		split_slot_start += chansw_time;
	}

	elem = &ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);

		if (!entity->cur_slot.timeslot ||
			!(entity->req_hdl->req_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) ||
			(entity->pend_slot.flags & MSCH_RC_FLAGS_SPLIT_SLOT_START)) {
			continue;
		}

		split_slot_param = entity->req_hdl->req_param;
		seq_start = (((uint64)split_slot_param->start_time_h << 32) |
			split_slot_param->start_time_l);

		msch_list_remove(&entity->start_fixed_link);

		/* Move the chan idx to the currently scheduled idx
		 * If the currently schedule channel is the last channel, move
		 * back the request param start time since scheduling the last
		 * channel again will increment start by interval.
		 */
		if ((entity->onchan_chn_idx == entity->req_hdl->last_chan_idx) &&
			(split_slot_start < seq_start)) {
			wlc_uint64_sub(&split_slot_param->start_time_h,
				&split_slot_param->start_time_l, 0, split_slot_param->interval);
			seq_start = (((uint64)split_slot_param->start_time_h << 32) |
				split_slot_param->start_time_l);
		}
		else if (entity->onchan_chn_idx == entity->req_hdl->last_chan_idx) {
			WL_ERROR(("msch schd remaining slot, last idx without dec\n"));
		}
		entity->cur_chn_idx = entity->onchan_chn_idx;
		entity->pend_slot.start_time = split_slot_start;
		entity->pend_slot.flags |= MSCH_RC_FLAGS_SPLIT_SLOT_START;
		entity->cur_slot.flags |= MSCH_RC_FLAGS_SPLIT_SLOT_END;

		/* Since the start time is not the actual start time, calculate
		 * end time from the parameters
		 * end_time = param_start_time + (idx * duration) + dur_for_cur_slot
		 */
		entity->pend_slot.end_time = seq_start +
			(entity->cur_chn_idx * split_slot_param->duration) +
			split_slot_param->duration;

		/* Update the actual start time since the new pend slot is the cur slot */
		entity->actual_start_time =
			(entity->pend_slot.end_time - split_slot_param->duration);
		if (entity->pend_slot.end_time < entity->pend_slot.start_time) {
			WL_ERROR(("Error in split slot time start %u end %u\n",
				(uint32)entity->pend_slot.start_time,
				(uint32)entity->pend_slot.end_time));
		}

		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"Schd split slot, start %u end %u duration %u\n",
			(uint32)entity->pend_slot.start_time, (uint32)entity->pend_slot.end_time,
			(uint32)(entity->pend_slot.end_time - entity->pend_slot.start_time)));

		/* add the req_entity back to the list */
		msch_list_sorted_add(&msch_info->msch_start_fixed_list,
			&entity->start_fixed_link,
			OFFSETOF(msch_req_entity_t, pend_slot) -
			OFFSETOF(msch_req_entity_t, start_fixed_link) +
			OFFSETOF(msch_req_timing_t, start_time),
			sizeof(entity->pend_slot.start_time), MSCH_ASCEND);
	}
}

int
wlc_msch_timeslot_unregister(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t **p_req_hdl)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *req_entity;
	wlc_msch_req_handle_t *req_hdl;
	msch_timeslot_t *ts = NULL;
	msch_chan_ctxt_t *chan_ctxt;
	bool timer_ctxt = TRUE;
	msch_req_timing_t *slot;
	bool unlink_bf_chan_ctxt = TRUE;
	uint64 cur_end_time = 0;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "wlc_msch_timeslot_unregister: Entry\n"));
	ASSERT(msch_info && p_req_hdl && *p_req_hdl);
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	/*
	* Save a local copy of the handle and clear the caller's handle value.
	* NOTE: Most callers do not check the return result and assume *p_req_hdl will
	* be NULL'd out.
	*/
	req_hdl = *p_req_hdl;
	*p_req_hdl = NULL;

	if (!req_hdl->req_param) {
		WL_ERROR(("wl%d: wlc_msch_timeslot_unregister: invalid req param\n",
			msch_info->wlc->pub->unit));
		return BCME_BADARG;
	}

	if (!_msch_req_hdl_valid(msch_info, req_hdl)) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: wlc_msch_timeslot_unregister: Invalid req_hdl\n",
			msch_info->wlc->pub->unit));
		*p_req_hdl = NULL;
		return BCME_OK;
	}
	timer_ctxt = (msch_info->flags & MSCH_STATE_IN_TIEMR_CTXT);

	_msch_timeslot_unregister(msch_info->profiler, req_hdl);

	if (msch_info->cur_msch_timeslot) {
		cur_end_time = msch_info->cur_msch_timeslot->end_time;
	}

	/* Non contionous req_entity are not part of req_hdl list, check it
	 * from current timeslot list
	 */
	if (!req_hdl->req_param->interval && msch_info->cur_msch_timeslot) {
		elem = &msch_info->msch_req_timing_list;
		while ((elem = msch_list_iterate(elem))) {
			slot = LIST_ENTRY(elem, msch_req_timing_t, link);
			req_entity = LIST_ENTRY(slot, msch_req_entity_t, cur_slot);
			if (req_entity->req_hdl == req_hdl) {
				elem = elem->prev;
				ts = msch_info->cur_msch_timeslot;
				chan_ctxt = req_entity->chan_ctxt;
				ASSERT(msch_info->cur_msch_timeslot ==
					req_entity->cur_slot.timeslot);
				_msch_req_entity_destroy(msch_info, req_entity);
				if (ts) {
					_msch_timeslot_check(msch_info, ts);
				}
				if (!timer_ctxt) {
					if (chan_ctxt &&
						msch_list_empty(&chan_ctxt->req_entity_list)) {
						_msch_chan_ctxt_destroy(msch_info, chan_ctxt, TRUE);
					}
					ts = NULL;
				}
			}
		}
	}

	/* revert all requested timeslot, delist from chan_ctxt, req_hdl list */
	while ((elem = msch_list_remove_head(&req_hdl->req_entity_list))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		ts = NULL;
		chan_ctxt = req_entity->chan_ctxt;
		if (req_entity->cur_slot.timeslot) {
			ASSERT(ts == NULL);
			ts = req_entity->cur_slot.timeslot;
			msch_list_remove(&req_entity->cur_slot.link);
			req_entity->cur_slot.timeslot = NULL;
		}

		if (req_entity->pend_slot.timeslot) {
			ASSERT((msch_info->next_timeslot == req_entity->pend_slot.timeslot));
			if (msch_info->next_timeslot &&
				!(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
				_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
				msch_info->next_timeslot = NULL;
			}
		}

		_msch_req_entity_destroy(msch_info, req_entity);

		/* Check the channel context entity list, if no more bf entity
		 * unlink channel context from list
		 */
		if (MSCH_BOTH_FLEX(req_hdl->req_param->req_type)) {
			elem = &chan_ctxt->bf_entity_list;
			while ((elem = msch_list_iterate(elem))) {
				req_entity = LIST_ENTRY(elem, msch_req_entity_t, rt_specific_link);
				if (MSCH_BOTH_FLEX(req_entity->req_hdl->req_param->req_type)) {
					unlink_bf_chan_ctxt = FALSE;
				}
			}
			if (unlink_bf_chan_ctxt) {
				 msch_list_remove(&chan_ctxt->bf_link);
			 }
		}

		if (ts) {
			_msch_timeslot_check(msch_info, ts);
		}

		if (!timer_ctxt) {
			if (chan_ctxt &&
				msch_list_empty(&chan_ctxt->req_entity_list)) {
				/* Free the cur_ts before destroying channel context */
				if (msch_info->cur_msch_timeslot &&
					msch_info->cur_msch_timeslot->chan_ctxt == chan_ctxt) {
					_msch_timeslot_free(msch_info,
						msch_info->cur_msch_timeslot, TRUE);
					msch_info->cur_msch_timeslot = NULL;
				}
				_msch_chan_ctxt_destroy(msch_info, chan_ctxt, TRUE);
			}
			ts = NULL;
		}
	}

	if (MSCH_BOTH_FLEX(req_hdl->req_param->req_type)) {
		msch_info->flex_list_cnt = msch_list_length(&msch_info->msch_both_flex_list);
		_msch_update_service_interval(msch_info);
	}

	/* req entiteis of hdl are cleared, make sure the next timeslot don't destroy
	 * flag is still valid or clear it
	 */
	if (msch_info->next_timeslot) {
		if ((!msch_info->next_timeslot->chan_ctxt) ||
			((msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY) &&
		 !_msch_check_bf_timeslot(msch_info, msch_info->next_timeslot, FALSE))) {
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
			msch_info->next_timeslot = NULL;
		}
	}

	msch_list_remove(&req_hdl->link);

	MFREE(msch_info->wlc->osh, req_hdl->req_param, sizeof(*(req_hdl->req_param)));
	req_hdl->req_param = NULL;

	if (req_hdl->chn_list) {
		MFREE(msch_info->wlc->osh, req_hdl->chn_list,
			(sizeof(chanspec_t) * req_hdl->chan_cnt));
		req_hdl->chn_list = NULL;
	}

	/* free req_hdl */
	_msch_req_handle_free(msch_info, req_hdl, TRUE);

	/* Current TS valid before unregister */
	if (cur_end_time && !msch_info->slotskip_flag) {
		if (!msch_info->cur_msch_timeslot ||
			(msch_info->cur_msch_timeslot->end_time != cur_end_time)) {
			if (msch_info->next_timeslot &&
				!(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
				_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
					FALSE);
				msch_info->next_timeslot = NULL;
			}
		}
	}

	/* Change in pending request (user request unregisterd)
	 * re-schedule next ts unless skip or timeslot timer active
	 * mark cur_ts end to identify change in time slot after unregister
	 */
	if (msch_info->slotskip_flag) {
		wlc_hrt_del_timeout(msch_info->slotskip_timer);
		wlc_msch_add_timeout(msch_info->slotskip_timer,
			0, _msch_slotskip_timer, msch_info);
	} else if (!msch_info->next_timeslot) {
		if (!timer_ctxt) {
			_msch_schd(msch_info);
		} else {
			msch_info->flags |= MSCH_STATE_SCHD_PENDING;
		}
	}
	return BCME_OK;
}

static int
_wlc_msch_timeslot_cancel(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t **p_req_hdl,
	uint32 timeslot_id)
{
	msch_list_elem_t *elem = NULL;
	msch_timeslot_t *timeslot = NULL;
	uint64 old_end_time = 0;
	wlc_msch_req_handle_t *req_hdl;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "wlc_msch_timeslot_cancel: Entry\n"));

	ASSERT(msch_info && p_req_hdl);
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	if ((req_hdl = *p_req_hdl) == NULL) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: wlc_msch_timeslot_cancel: req_hdl NULL\n",
			msch_info->wlc->pub->unit));
		return BCME_BADARG;
	}

	if (!_msch_req_hdl_valid(msch_info, req_hdl)) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: wlc_msch_timeslot_cancel: Invalid req_hdl\n",
			msch_info->wlc->pub->unit));
		return BCME_NOTFOUND;
	}

	elem = &req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		msch_req_entity_t *req_entity;

		req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);

		timeslot = req_entity->cur_slot.timeslot;
		if (timeslot && (timeslot->timeslot_id == timeslot_id)) {
			ASSERT((timeslot == msch_info->cur_msch_timeslot));
			old_end_time = msch_info->cur_msch_timeslot->end_time;
			if (!req_entity->req_hdl->req_param->interval) {
				req_entity->req_hdl->chan_cnt--;
				if (!req_entity->req_hdl->chan_cnt) {
					wlc_msch_timeslot_unregister(msch_info, p_req_hdl);
					/* req_hdl = NULL; */
					/* Timeslot unregister takes care of reschd,
					 * noting more to do for the req_hdl return
					*/
					return BCME_OK;
				}
				_msch_req_entity_destroy(msch_info, req_entity);
				_msch_timeslot_check(msch_info, timeslot);
			} else {
				msch_list_remove(&req_entity->cur_slot.link);
				memset(&req_entity->cur_slot, 0, sizeof(req_entity->cur_slot));
			}
			break;
		}
		timeslot = NULL;
	}

	/* the requested timeslot id doesn't exist */
	if (timeslot == NULL) {
		return BCME_NOTFOUND;
	}

	if (msch_info->slotskip_flag) {
		wlc_hrt_del_timeout(msch_info->slotskip_timer);
		wlc_msch_add_timeout(msch_info->slotskip_timer,
			0, _msch_slotskip_timer, msch_info);
		return BCME_OK;
	}

	/* change in current timeslot - destroy the next timeslot
	 * and schedule a new next timeslot
	 */
	if (old_end_time && (!msch_info->cur_msch_timeslot ||
		(old_end_time != msch_info->cur_msch_timeslot->end_time))) {
		if (msch_info->next_timeslot &&
			!(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
				FALSE);
			msch_info->next_timeslot = NULL;
			if (msch_info->flags & MSCH_STATE_IN_TIEMR_CTXT) {
				msch_info->flags |= MSCH_STATE_SCHD_PENDING;
			} else {
				_msch_schd(msch_info);
			}
		}
	}

	return BCME_OK;
}

#ifdef MSCH_NEW_CANCEL_API
int
wlc_msch_timeslot_cancel(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t **req_hdl,
	uint32 timeslot_id)
{
	return _wlc_msch_timeslot_cancel(msch_info, req_hdl, timeslot_id);
}
#else
int
wlc_msch_timeslot_cancel(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	uint32 timeslot_id)
{
	return _wlc_msch_timeslot_cancel(msch_info, &req_hdl, timeslot_id);
}
#endif // endif

/* Update the channel sequence with the drift in start time
 * shrink/extend the current timeslot if required
 */
static void
_msch_chan_seq_update_ts(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *hdl,
	int32 drift)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity = NULL, *cur_ts_entity = NULL;
	msch_req_entity_t *list_en = NULL;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 cur_ts_endtime = 0;
	uint64 cur_time = msch_current_time(msch_info);
	int32 time_to_end = 0, new_end = drift;

	if (!cur_ts || !drift) {
		return;
	}

	/* find the next sfix entity to schdule */
	elem = &msch_info->msch_start_fixed_list;
	while ((elem = msch_list_iterate(elem))) {
		list_en = LIST_ENTRY(elem, msch_req_entity_t, start_fixed_link);
		if (list_en->req_hdl == hdl) {
			entity = list_en;
			break;
		}
	}

	if (!entity || (entity->req_hdl != hdl)) {
		return;
	}

	/* Get the current timeslot entity */
	elem = &cur_ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		list_en = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if ((list_en->req_hdl == hdl) &&
			list_en->cur_slot.timeslot == cur_ts) {
			cur_ts_entity = list_en;
			break;
		}
	}

	/* new end time isn calculated as an offset of the current
	* end time from the new end time
	*/
	if (cur_ts_entity && cur_ts_entity->cur_slot.end_time > cur_time) {
		if ((cur_ts_entity->cur_slot.flags & MSCH_RC_FLAGS_START_FIRE_DONE) &&
			!(cur_ts_entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE)) {
			time_to_end = new_end =
				(int32)(cur_ts_entity->cur_slot.end_time - cur_time);
			if (drift < 0) {
				drift = (hdl->req_param->duration + drift);
			}
			new_end = time_to_end + drift;
			new_end = new_end % hdl->req_param->duration;
		} else {
			/* Remove the cur_slot from link */
			msch_list_remove(&cur_ts_entity->cur_slot.link);
				cur_ts_entity = NULL;
		}
	}
	drift = new_end - time_to_end;
	/* kill the next timeslot */
	if (msch_info->next_timeslot &&
		!(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
		_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
		msch_info->next_timeslot = NULL;
	}

	if (drift < 0) {
		bool now = FALSE;
		if (_msch_check_curts_preemption(msch_info, ABS(drift), entity->chanspec,
			&cur_ts_endtime, MSCH_REQ_FLAGS_PREMT_CURTS, &now) != MSCH_INFINITE_TIME) {
			MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
				"_msch_chan_seq_update_ts: Cutting current ts end time "
				"from %d.%06d to %d.%06d\n",
				_msch_display_time(msch_info->cur_msch_timeslot->end_time),
				_msch_display_time(cur_ts_endtime)));
			_msch_reduce_curts(msch_info, cur_ts_endtime);
		}
	} else {
		/* found the entity in cur_ts so extend the timeslot */
		if (!cur_ts_entity || (cur_ts_entity->req_hdl != hdl) ||
			(cur_ts_entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE)) {
			return;
		}

		msch_info->cur_msch_timeslot->end_time += drift;
		cur_ts_entity->cur_slot.end_time += drift;

		/* Remove the cur_slot from link */
		msch_list_remove(&cur_ts_entity->cur_slot.link);
		cur_ts_entity->curts_fire_time = cur_ts_entity->cur_slot.end_time;
		/* Add the slot to req timing for new end time */
		msch_list_sorted_add(&msch_info->msch_req_timing_list,
			&cur_ts_entity->cur_slot.link,
			(OFFSETOF(msch_req_entity_t, curts_fire_time) -
			OFFSETOF(msch_req_entity_t, cur_slot) -
			OFFSETOF(msch_req_timing_t, link)),
			sizeof(cur_ts_entity->curts_fire_time), MSCH_ASCEND);
	}
}

int
msch_timeslot_update_slot_endtime(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *p_req_hdl,
	wlc_msch_req_param_t *param) {
	msch_req_entity_t *entity = NULL;
	msch_list_elem_t *elem;
	uint64 cur_time = msch_current_time(msch_info);

	if (!p_req_hdl) {
		WL_ERROR(("%s: p_req_hdl is NULL\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (!_msch_req_hdl_valid(msch_info, p_req_hdl)) {
		WL_ERROR(("%s: req_hdl=%p\n", __FUNCTION__, p_req_hdl));
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: %s: Invalid req_hdl\n",
			msch_info->wlc->pub->unit, __FUNCTION__));
		return BCME_NOTFOUND;
	}

	if (msch_info->next_timeslot &&
		(msch_info->next_timeslot->ts_flags & MSCH_TS_FLAG_DONT_DESTROY)) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"ts_register: next timeslot is fixed, reject request\n"));
		return BCME_ERROR;
	}

	elem = &p_req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		if (entity->cur_slot.timeslot == msch_info->cur_msch_timeslot) {
			/* If current entity's slot end time is greater than the to-be extended
			 * time do nothing
			 */
			if (cur_time + param->duration < entity->cur_slot.end_time) {
				return BCME_OK;
			}

			/* If the entity's end time is fired, could not extend the
			 * ts, return error
			 */
			if (entity->cur_slot.flags & MSCH_RC_FLAGS_END_FIRE_DONE) {
				return BCME_ERROR;
			}

			/* return ERROR if no available slot can be scheduled */
			if (_msch_check_avail_extend_slot(msch_info, entity,
				cur_time, param->duration) < param->duration) {
				return BCME_ERROR;
			}
			entity->cur_slot.end_time = cur_time + param->duration;
			if (msch_info->cur_msch_timeslot->end_time < entity->cur_slot.end_time) {
				msch_info->cur_msch_timeslot->end_time = entity->cur_slot.end_time;
			}

			/* If the extended time slot end time overlaps
			 * with next time slot, destroy and re-schd
			 * next ts
			 */
			if (msch_info->next_timeslot &&
				msch_info->next_timeslot->pre_start_time <
				msch_info->cur_msch_timeslot->end_time) {
				/* kill flag checked above so destroy */
				_msch_timeslot_destroy(msch_info, msch_info->next_timeslot, FALSE);
				msch_info->next_timeslot = NULL;
			}
			/* If the entity is already fired, remove the fired entity from
			 * the list, re-calculate the fire time and added the entity with
			 * updated fire time back to the list
			 */
			if (entity->cur_slot.flags & MSCH_RC_FLAGS_START_FIRE_DONE) {
				/* Remove the cur_slot from link */
				msch_list_remove(&entity->cur_slot.link);
				entity->curts_fire_time = entity->cur_slot.end_time;
				/* Add the slot to req timing for new end time */
				msch_list_sorted_add(&msch_info->msch_req_timing_list,
					&entity->cur_slot.link,
					(OFFSETOF(msch_req_entity_t, curts_fire_time) -
					OFFSETOF(msch_req_entity_t, cur_slot) -
					OFFSETOF(msch_req_timing_t, link)),
					sizeof(entity->curts_fire_time), MSCH_ASCEND);
			}

			_msch_arm_chsw_timer(msch_info);
		}
	}
	return BCME_OK;
}

/* TODO If the routine extened in the future don't accept multiple change request at
 * the same time since error handling becomes difficult to communicate to user
 */
int
wlc_msch_timeslot_update(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *p_req_hdl,
	wlc_msch_req_param_t *param, uint32 update_mask)
{
	msch_req_entity_t *entity = NULL;
	msch_list_elem_t *elem;
	uint64 start_time, cur_start_time, cur_time;
	wlc_msch_req_type_t param_req_type;
	int32 drift = 0, cur_ts_drift = 0;
	wlc_msch_req_param_t *cur_param = NULL;
	uint32 prep_time = 0;
	uint64 seq_start, en_seq_start = 0;
	int32 drift_in_interval = 0;
	bool init_seq = TRUE;

	ASSERT(msch_info);
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "wlc_msch_timeslot_update: Entry\n"));

	ASSERT((p_req_hdl && param));

	if (!p_req_hdl || !param) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: wlc_msch_timeslot_update: req_hdl NULL\n",
			msch_info->wlc->pub->unit));
		return BCME_BADARG;
	}

	if (!_msch_req_hdl_valid(msch_info, p_req_hdl)) {
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: wlc_msch_timeslot_update: Invalid req_hdl\n",
			msch_info->wlc->pub->unit));
		return BCME_NOTFOUND;
	}

	cur_param = p_req_hdl->req_param;
	MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON, "wlc_msch_timeslot_update: start time %d.%06d\n",
		_msch_display_time(((uint64)param->start_time_h << 32) + param->start_time_l)));

	param_req_type = cur_param->req_type;
	if (!(cur_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) &&
		(update_mask & MSCH_UPDATE_INTERVAL)) {
		cur_param->interval = param->interval;
	}

	if (update_mask & MSCH_UPDATE_START_TIME) {
		/* Start time update not supported for start flexible req
		 * but both flex is supported for STA tbtt update
		 */
		if (param_req_type == MSCH_RT_START_FLEX ||
			param_req_type == MSCH_RT_DUR_FLEX) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wl%d: wlc_msch_timeslot_update: error update start time"
				"supported only for BOTH_FIX/BOTH_FLEX req\n",
				msch_info->wlc->pub->unit));
			return BCME_USAGE_ERROR;
		}

		cur_start_time = (((uint64)cur_param->start_time_h << 32) |
			cur_param->start_time_l);

		seq_start = start_time =
			(((uint64)param->start_time_h << 32) | param->start_time_l);
		cur_time = msch_current_time(msch_info);
		drift = (int)(start_time - cur_start_time);

		if (start_time == cur_start_time) {
			/* Nothing to do exit */
			return BCME_OK;
		}
		/* If the drift is positive, start adjust the drift as
		* if we are calculating from the start of previous sequence
		*/
		if ((cur_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) &&
			(drift > 0)) {
			wlc_uint64_sub(&cur_param->start_time_h, &cur_param->start_time_l, 0,
				cur_param->duration);
		}

		if (cur_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
			if ((uint32)ABS(drift) > (cur_param->interval + cur_param->duration)) {
				MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
						"wl%d: wlc_msch_timeslot_update: drift is "
						"more than interval [drift %d]\n",
						msch_info->wlc->pub->unit, drift));

				drift_in_interval = (ABS(drift))/(p_req_hdl->req_param->interval);
				drift_in_interval *= p_req_hdl->req_param->interval;

				if (drift) {
					drift -= drift_in_interval;
				} else {
					drift += drift_in_interval;
				}
			}
		} else if (start_time <= cur_time) {
			MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
				"wlc_msch_timeslot_update: start time %u is less than current "
				"time %u\n", (uint32)start_time, (uint32)cur_time));
			ASSERT(FALSE);
			return BCME_BADARG;
		}

		cur_param->start_time_l = param->start_time_l;
		cur_param->start_time_h = param->start_time_h;

		if (msch_info->cur_msch_timeslot) {
			if (_msch_check_bf_timeslot(msch_info,
				msch_info->cur_msch_timeslot, TRUE)) {
				prep_time = wlc_msch_calc_avg_chswtime(msch_info, 0);
				if (msch_info->next_timeslot &&
					msch_info->next_timeslot->ts_flags &
						MSCH_TS_FLAG_DONT_DESTROY) {
					msch_info->next_timeslot->ts_flags &=
						~MSCH_TS_FLAG_DONT_DESTROY;
					_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
						FALSE);
					msch_info->next_timeslot = NULL;
					_msch_curts_add_pm_time_to_slot(msch_info,
						msch_info->cur_msch_timeslot);
				}
			}
			if (cur_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
				/* Get the drift for current timeslot */
				cur_ts_drift = ((ABS(drift))%(p_req_hdl->req_param->duration));
				if (drift < 0) {
					cur_ts_drift = -cur_ts_drift;
				}
				_msch_chan_seq_update_ts(msch_info, p_req_hdl, cur_ts_drift);
			}
		}
		/* todo: next ts should have destroy set when there is no cur ts */
		if (msch_info->next_timeslot &&
			msch_info->next_timeslot->ts_flags &
				MSCH_TS_FLAG_DONT_DESTROY) {
			WL_ERROR(("timeslot update: no curts but next ts"
				"don't destroy set\n"));
			msch_info->next_timeslot->ts_flags &=
				~MSCH_TS_FLAG_DONT_DESTROY;
			_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
				FALSE);
			msch_info->next_timeslot = NULL;
		}

		elem = &p_req_hdl->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);

			/* Destroy the next ts before any change to timing */
			/* Remove the entity from corresponding list */
			if (MSCH_START_FIXED(param_req_type)) {
				msch_list_remove(&entity->start_fixed_link);
			} else {
				/* BF */
				msch_list_remove(&entity->both_flex_list);
			}

			if (cur_param->flags & MSCH_REQ_FLAGS_CHAN_CONTIGUOUS) {
				if (_msch_chn_seq_update_entity(msch_info,
					p_req_hdl, entity, prep_time)) {
					/* Since start time could be partial,
					 * use end time to calculate
					 * start time - this update is needed
					 * since we will use this time as a
					 * reference for the next entity.
					 */
					en_seq_start = entity->pend_slot.end_time -
						((entity->cur_chn_idx + 1) *
						p_req_hdl->req_param->duration);
					if (init_seq || (en_seq_start < seq_start)) {
						seq_start = en_seq_start;
						init_seq = FALSE;
					}
				}
			} else {
				entity->pend_slot.start_time += drift;
				entity->pend_slot.end_time += drift;
				entity->pend_slot.pre_start_time += drift;
				entity->actual_start_time += drift;
			}

			/* Destroy the next ts before any change to timing */
			if (msch_info->next_timeslot &&
				((entity->pend_slot.timeslot &&
				(entity->pend_slot.timeslot == msch_info->next_timeslot)) ||
				_msch_enity_next_ts_overlap(msch_info, entity))) {
				ASSERT(!(msch_info->next_timeslot->ts_flags &
					MSCH_TS_FLAG_DONT_DESTROY));
				_msch_timeslot_destroy(msch_info, msch_info->next_timeslot,
					FALSE);
				msch_info->next_timeslot = NULL;
			}

			MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
				"wlc_msch_timeslot_update: idx %d chan 0x%x drift %d New start "
				"time %u\n", entity->cur_chn_idx, entity->chanspec,
				drift, (uint32)entity->pend_slot.start_time));

			if (MSCH_START_FIXED(param_req_type)) {
				/* add the req_entity back to start fixed list */
				msch_list_sorted_add(&msch_info->msch_start_fixed_list,
					&entity->start_fixed_link,
					OFFSETOF(msch_req_entity_t, pend_slot) -
					OFFSETOF(msch_req_entity_t, start_fixed_link) +
					OFFSETOF(msch_req_timing_t, start_time),
					sizeof(uint64), MSCH_ASCEND);
			} else {
				/* add the req_entity back to both flex list */
				msch_list_sorted_add(
					&msch_info->msch_both_flex_req_entity_list,
					&entity->both_flex_list,
					OFFSETOF(msch_req_entity_t, pend_slot) -
					OFFSETOF(msch_req_entity_t, both_flex_list) +
					OFFSETOF(msch_req_timing_t, start_time),
					sizeof(uint64), MSCH_ASCEND);
			}
		}

		if (seq_start != start_time) {
			cur_param->start_time_l = (uint32)seq_start;
			cur_param->start_time_h = (uint32)(seq_start >> 32);
		}
		if (!msch_info->next_timeslot) {
			if (!(msch_info->flags & MSCH_STATE_IN_TIEMR_CTXT)) {
				_msch_schd(msch_info);
			}
			else {
				msch_info->flags |= MSCH_STATE_SCHD_PENDING;
			}
		}
	}

	/* update current slot's end time */
	if (update_mask & MSCH_UPDATE_END_TIME) {
		if (msch_info->cur_msch_timeslot) {
			return msch_timeslot_update_slot_endtime(msch_info, p_req_hdl, param);
		}
	}
	return BCME_OK;
}

/* Update the channel continous request to next schedulable time
 */
static void
_msch_chan_cont_req_update(wlc_msch_info_t * msch_info, wlc_msch_req_handle_t *req_hdl)
{
	msch_list_elem_t *elem;
	msch_req_entity_t *entity;
	uint32 prep_time = 0;
	uint64 seq_start, start_time;
	start_time = (((uint64)req_hdl->req_param->start_time_h << 32) |
		req_hdl->req_param->start_time_l);
	seq_start = start_time;
	if (msch_info->cur_msch_timeslot) {
		if (_msch_check_bf_timeslot(msch_info, msch_info->cur_msch_timeslot, TRUE)) {
			prep_time = wlc_msch_calc_avg_chswtime(msch_info, 0);
		}
	}

	elem = &req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);

		msch_list_remove(&entity->start_fixed_link);
		if (_msch_chn_seq_update_entity(msch_info, req_hdl, entity, prep_time)) {
			seq_start = entity->pend_slot.end_time -
				((entity->cur_chn_idx + 1) *
				req_hdl->req_param->duration);
		}
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"_msch_chan_cont_req_update: idx %d chan 0x%x New start time %u\n",
			entity->cur_chn_idx, entity->chanspec,
			(uint32)entity->pend_slot.start_time));

		/* add the req_entity back to start fixed list */
		msch_list_sorted_add(&msch_info->msch_start_fixed_list,
			&entity->start_fixed_link,
			OFFSETOF(msch_req_entity_t, pend_slot) -
			OFFSETOF(msch_req_entity_t, start_fixed_link) +
			OFFSETOF(msch_req_timing_t, start_time),
			sizeof(entity->pend_slot.start_time), MSCH_ASCEND);
	}

	if (seq_start != start_time) {
		req_hdl->req_param->start_time_l = (uint32)seq_start;
		req_hdl->req_param->start_time_h = (uint32)(seq_start >> 32);
	}
}

/*
* This function is added to handle the case where we need to
* update a drift of more than 1 slot duration.
* We rotate the current entity list till we reach the current
* slot's end time, and schedule the next entity's start time
* to be the earliest start point after the current timeslot end.
*/
static bool
_msch_chn_seq_update_entity(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	msch_req_entity_t *entity, uint32 prep_time)
{
	uint64 cur_time = msch_current_time(msch_info);
	uint64 cur_ts_end = cur_time + MSCH_DELAY_FOR_CLBK;
	uint16 idx = 0;
	wlc_info_t *wlc = msch_info->wlc;
	bool update_seq = FALSE;
	uint32 chansw_time;
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	msch_req_entity_t *list_en;
	msch_list_elem_t *elem;

	entity->cur_chn_idx = 0;
	entity->pend_slot.start_time = ((uint64)req_hdl->req_param->start_time_h << 32) |
		req_hdl->req_param->start_time_l;

	if (cur_ts) {
		/* Get the current timeslot entity */
		elem = &cur_ts->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			list_en = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if ((list_en->req_hdl == req_hdl) &&
				list_en->cur_slot.timeslot == cur_ts) {
				cur_ts_end = list_en->cur_slot.end_time;
				break;
			}
		}
	}

	if (WLC_BAND_PI_RADIO_CHANSPEC != entity->chan_ctxt->chanspec) {
		prep_time = wlc_msch_calc_avg_chswtime(msch_info,
			entity->chan_ctxt->chanspec);
	}

	/* Search for the correct channel index in the channel sequence list */
	while (idx <= req_hdl->last_chan_idx) {
		if (entity->chanspec == req_hdl->chn_list[idx]) {
			entity->pend_slot.start_time -=
				(entity->cur_chn_idx * req_hdl->req_param->duration);
			entity->cur_chn_idx = idx;
			entity->pend_slot.start_time += (idx * req_hdl->req_param->duration);
			entity->pend_slot.end_time = entity->pend_slot.start_time +
				req_hdl->req_param->duration;
			entity->actual_start_time = entity->pend_slot.start_time;
			/* If a slot can be scheduled within the time remaining
			 * before the new slot end time, schedule it
			 */
			chansw_time = wlc_msch_calc_avg_chswtime(msch_info,
					entity->chanspec);
			if (entity->pend_slot.end_time >
				(cur_ts_end + prep_time + chansw_time)) {
				break;
			}
		}
		idx++;

		/* Last channel, update req start time */
		if (idx > req_hdl->last_chan_idx) {
			/* start searching for idx begining of the next slot */
			entity->pend_slot.start_time -=
				(entity->cur_chn_idx * req_hdl->req_param->duration);
			entity->pend_slot.start_time += req_hdl->req_param->interval;
			if (entity->cur_chn_idx == (idx - 1)) {
				update_seq = TRUE;
			}
			entity->cur_chn_idx = idx = 0;
		}
	}
	return update_seq;
}

static uint64
_msch_get_next_start_time(wlc_msch_info_t *msch_info)
{
	uint64 cur_ts_endtime = 0, nextstart = 0;
	msch_timeslot_t *cur_ts;
	ASSERT(msch_info);

	nextstart = msch_current_time(msch_info);
	cur_ts = msch_info->cur_msch_timeslot;
	if (cur_ts) {
		if (cur_ts->end_time != MSCH_INFINITE_TIME) {
			nextstart = cur_ts->end_time;
		} else {
			cur_ts_endtime = _msch_curts_slot_endtime(msch_info);
			if (cur_ts_endtime) {
				nextstart = cur_ts_endtime;
			}
		}
	}
	return nextstart;
}

/* This function returns the first valid request entity with ts
 * in the timeslot list, there could be many valid entity in the
 * list
 */
msch_req_entity_t*
wlc_get_ts_entity(wlc_msch_info_t *msch_info, msch_timeslot_t *ts)
{
	msch_list_elem_t *elem = NULL;
	msch_req_entity_t *entity = NULL;

	if (ts) {
		elem = &ts->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if (entity->pend_slot.timeslot == ts ||
				entity->cur_slot.timeslot == ts) {
				return entity;
			}
		}
	}
	return NULL;
}

bool
wlc_msch_query_ts_shared(wlc_msch_info_t *msch_info, uint32 timeslot_id)
{
	msch_req_entity_t *entity = NULL;
	msch_list_elem_t *elem = NULL;
	msch_timeslot_t *timeslot = msch_info->cur_msch_timeslot;
	int req_count = 0;

	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "wlc_msch_query_ts_shared: Entry\n"));
	ASSERT(msch_info);
	ASSERT(MSCH_CHECK_MAGIC(msch_info));

	if (timeslot && timeslot->timeslot_id == timeslot_id) {
		elem = &msch_info->cur_msch_timeslot->chan_ctxt->req_entity_list;
		while ((elem = msch_list_iterate(elem))) {
			entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
			if (entity->cur_slot.timeslot == timeslot) {
				req_count++;
				if (req_count > 1) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

uint64
msch_current_time(wlc_msch_info_t *msch_info)
{
	return OSL_SYSUPTIME_US();
}

uint64
msch_future_time(wlc_msch_info_t *msch_info, uint32 delta_in_usec)
{
	return (msch_current_time(msch_info) + delta_in_usec);
}

/*
 * Queries the slot availability from start_time for duration
 * start_time 0 - returns the end time of current timeslot
 *
 * TBD : Only start time 0 is implemented
 *		 time based query not implemented
 */
uint64
wlc_msch_query_timeslot(wlc_msch_info_t *msch_info, uint64 start_time, uint32 duration)
{
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	uint64 next_start_time = 0;

	if (!start_time && cur_ts) {
		if (cur_ts->end_time != MSCH_INFINITE_TIME) {
			next_start_time = msch_info->cur_msch_timeslot->end_time;
		} else {
			/* Get cur_ts slot end time */
			next_start_time = _msch_curts_slot_endtime(msch_info);
		}

		if (next_start_time < msch_current_time(msch_info)) {
			next_start_time = msch_current_time(msch_info);
		}

		if (_msch_check_bf_timeslot(msch_info, cur_ts, TRUE)) {
			next_start_time += msch_info->pm_notif_dur_us;
		}

		/* Add extra 5ms delay for clbk processing */
		next_start_time += wlc_msch_calc_avg_chswtime(msch_info, 0);
		return next_start_time;
	}

	/* TODO not yet implemented */
	return (msch_current_time(msch_info) +
		wlc_msch_calc_avg_chswtime(msch_info, 0));
}

static uint64
_msch_tsf2mschtime_diff(wlc_msch_info_t *msch_info)
{
	wlc_info_t *wlc = msch_info->wlc;
	uint32 tsf_l, tsf_h;
	uint64 cur_time, tsfdiff;
	bool prev_awake;

	wlc_force_ht(wlc, TRUE, &prev_awake);
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	wlc_force_ht(wlc, prev_awake, NULL);
	cur_time = msch_current_time(msch_info);
	tsfdiff = cur_time - (((uint64)tsf_h << 32) | tsf_l);

	return tsfdiff;
}

uint64
msch_tsf_to_mschtime(wlc_msch_info_t *msch_info, uint32 tsf_lo, uint32 tsf_hi,
	uint32 *mschtime_lo, uint32 *mschtime_hi)
{
	uint64 msch_time = _msch_tsf2mschtime_diff(msch_info) + (((uint64)tsf_hi << 32) | tsf_lo);

	if (mschtime_hi)
		*mschtime_hi = (uint32)(msch_time >> 32);
	if (mschtime_lo)
		*mschtime_lo = (uint32)(msch_time);

	return msch_time;
}

void
msch_mschtime_to_tsf(wlc_msch_info_t *msch_info, uint64 mschtime, uint32 *tsf_lo, uint32 *tsf_hi)
{
	mschtime -= _msch_tsf2mschtime_diff(msch_info);

	if (tsf_hi)
		*tsf_hi = (uint32)(mschtime >> 32);
	if (tsf_lo)
		*tsf_lo = (uint32)mschtime;
}

int32 msch_calc_slot_duration(wlc_msch_info_t *msch_info, wlc_msch_cb_info_t *cb_info)
{
	wlc_msch_onchan_info_t * onch_info =
			(wlc_msch_onchan_info_t *)cb_info->type_specific;

	uint64 cur_time;
	uint32 ct_h, ct_l, end_h, end_l;

	end_h = onch_info->end_time_h;
	end_l = onch_info->end_time_l;

	if ((int32)end_h == -1) {
		/* infinite slot end time */
		return -1;
	}

	cur_time = msch_current_time(msch_info);
	ct_h = (uint32)(cur_time >> 32);
	ct_l = (uint32)cur_time;

	// calculate delta current time - slot end time
	wlc_uint64_sub(&end_h, &end_l, ct_h, ct_l);

	return end_l;
}

static uint32
wlc_msch_calc_avg_chswtime(wlc_msch_info_t *msch_info, chanspec_t to)
{
	return MSCH_ONCHAN_PREPARE;
}

static uint32
wlc_msch_entity_chswtime(wlc_msch_info_t *msch_info, msch_req_entity_t *entity, chanspec_t from)
{
	chanspec_t to_chanspec = entity->chan_ctxt->chanspec;

	/* Same chanspec */
	if (from == to_chanspec) {
		return 0;
	}

	/* Query the averaging module once per request pending to schedule
	 * pend_slot will be reset after this request becomes ON_CHAN
	 * if there is a change in band of source channel, update again
	 */
	if ((entity->chsw_time == 0) ||
		(entity->from_2g != CHSPEC_IS2G(from))) {
		entity->from_2g = CHSPEC_IS2G(from);
		entity->chsw_time = wlc_msch_calc_avg_chswtime(msch_info, to_chanspec);
		MSCH_MESSAGE((msch_info, MSCH_DEBUG_ON,
			"sw_time: [0x%x to 0x%x] :%d\n", from, to_chanspec,
			entity->chsw_time));
		return entity->chsw_time;
	}

	return entity->chsw_time;
}

void
msch_update_bw(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	chanspec_t old_chanspec, chanspec_t chanspec)
{
	msch_req_entity_t *req_entity;
	msch_list_elem_t *elem = NULL;
	chanspec_t ctxt_chanspec = 0;
	wlc_info_t *wlc = msch_info->wlc;
	MSCH_MESSAGE((msch_info, MSCH_TRACE_ON, "msch_update_bw: Entry\n"));

	if (!req_hdl) {
		WL_ERROR(("%s: p_req_hdl is NULL\n", __FUNCTION__));
		return;
	}

	if (!_msch_req_hdl_valid(msch_info, req_hdl)) {
		WL_ERROR(("%s: req_hdl=%p\n", __FUNCTION__, req_hdl));
		MSCH_MESSAGE((msch_info, MSCH_ERROR_ON,
			"wl%d: %s: Invalid req_hdl\n",
			msch_info->wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (req_hdl->chan_cnt > 1) {
		return;
	}

	if (old_chanspec == chanspec) {
		return;
	}

	/* Do not update if channel is different
	 */
	if (!WLC_CHAN_COEXIST(chanspec, old_chanspec)) {
		return;
	}

	elem = &req_hdl->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		req_entity = LIST_ENTRY(elem, msch_req_entity_t, req_hdl_link);
		if (req_entity->req_hdl == req_hdl) {
			req_entity->chanspec = chanspec;

			ctxt_chanspec = req_entity->chan_ctxt->chanspec;
			if (CHSPEC_BW(old_chanspec) >= CHSPEC_BW(ctxt_chanspec) ||
				CHSPEC_BW(chanspec) > CHSPEC_BW(ctxt_chanspec)) {
				req_entity->chan_ctxt->chanspec = chanspec;
				if (CHSPEC_BW(old_chanspec) >= CHSPEC_BW(ctxt_chanspec)) {
					_msch_update_chan_ctxt_bw(msch_info,
						req_entity->chan_ctxt);
				}
			}

			if (req_entity->cur_slot.timeslot &&
				WLC_BAND_PI_RADIO_CHANSPEC != req_entity->chan_ctxt->chanspec) {
				_msch_chan_adopt(msch_info, req_entity->chan_ctxt);
			}
		}
	}
}

/*
 * On new request, cut the cur_ts if it both flex slot (connection)
 * and re-schd after the new request registered
*/
static void
_msch_new_req_update_curts(wlc_msch_info_t *msch_info)
{
	uint64 cur_time = msch_current_time(msch_info);
	msch_timeslot_t *cur_ts = msch_info->cur_msch_timeslot;
	msch_list_elem_t *elem;
	msch_req_entity_t *entity = NULL;

	if (msch_info->next_timeslot &&
		(msch_info->next_timeslot->ts_flags &
		MSCH_TS_FLAG_DONT_DESTROY)) {
		return;
	}

	/* exit if there is any req other than both flex */
	elem = &cur_ts->chan_ctxt->req_entity_list;
	while ((elem = msch_list_iterate(elem))) {
		entity = LIST_ENTRY(elem, msch_req_entity_t, chan_ctxt_link);
		if ((entity->cur_slot.timeslot == cur_ts) &&
		!MSCH_BOTH_FLEX(entity->req_hdl->req_param->req_type)) {
			return;
		}
	}

	if (!entity) {
		return;
	}

	/* cut the both flex slot to re-evaluate the new req */
	if (cur_ts->end_time > (cur_time + MSCH_MIN_ONCHAN_TIME + msch_info->pm_notif_dur_us)) {
		_msch_reduce_curts(msch_info,
			(cur_time + MSCH_MIN_ONCHAN_TIME + msch_info->pm_notif_dur_us));
	}
	return;
}

/*
 * update scan_home_time and scan_home_away_time
 * set in current msch handlers
 */
void
wlc_msch_update_home_away_time(wlc_msch_info_t *msch_info, uint32 min_dur, uint32 max_away_dur)
{
	msch_list_elem_t *elem;
	wlc_msch_req_handle_t *req_hdl;

	if (max_away_dur == 0) {
		WL_ERROR(("wl%d:%s Invalid home away time configuration %d\n",
		          msch_info->wlc->pub->unit, __FUNCTION__, max_away_dur));
		return;
	}

	/* Check if the req hdl is valid */
	elem = &msch_info->msch_req_hdl_list;
	while ((elem = msch_list_iterate(elem))) {
		req_hdl = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
		if (MSCH_BOTH_FLEX(req_hdl->req_param->req_type)) {
			req_hdl->req_param->flex.bf.min_dur = min_dur;
			req_hdl->req_param->flex.bf.max_away_dur = max_away_dur;
		}
	}
}
