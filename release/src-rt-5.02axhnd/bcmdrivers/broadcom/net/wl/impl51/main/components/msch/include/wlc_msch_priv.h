/*
 * private header file for multi-channel scheduler
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
 * $Id: wlc_msch_priv.h 711170 2017-07-17 07:28:46Z $
 */

#ifndef _wlc_msch_priv_h_
#define _wlc_msch_priv_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_mschutil.h>
#include <wlc_msch_profiler.h>

/* macOS Kernel framework defines LIST_ENTRY too. Undef it to let MSCH use its own def */
#if defined(LIST_ENTRY)
#undef LIST_ENTRY
#endif // endif

#define LIST_ENTRY(ptr, type, member) ((type *)((char *)(ptr) - OFFSETOF(type, member)))

#define safe_dec(val, dec) val = (val > (dec)) ? (val - (dec)) : 0

#define MSCH_START_FIXED(type) ((type == MSCH_RT_BOTH_FIXED) || (type == MSCH_RT_DUR_FLEX))
#define MSCH_BOTH_FIXED(type)	(type == MSCH_RT_BOTH_FIXED)
#define MSCH_START_FLEX(type)	(type == MSCH_RT_START_FLEX)
#define MSCH_BOTH_FLEX(type)	(type == MSCH_RT_BOTH_FLEX)

#define MSCH_MAGIC_NUMBER		(0xC0DEED0C) /* C0DE ED0C */
#define MSCH_PROCESSING_DELAY	(2000) /* General delay to account code exec */
#define MSCH_MAX_PREEMPT_DURATION (30000) /* Premption is allowed only for short req */

/*
 *	Memory pool limits for following free lists used in msch:
 *	msch_list_elem_t free_req_hdl_list; 56 bytes each
 *	msch_list_elem_t free_req_entity_list;  192 bytes each
 *	msch_list_elem_t free_chan_ctxt_list;  72 bytes each
 *	msch_list_elem_t free_chanspec_list;  64 bytes each
 *	Total size of memory pool will be:
 *	56 * 2 + 10 * 72 + 10 * 192 + 10 * 64 = 3392 bytes
 */
#define MSCH_MAX_FREE_TIMESLOTS	2
#define MSCH_MAX_FREE_CHAN_CTXT 10
#define MSCH_MAX_FREE_REQ_ENTITY 10
#define MSCH_MAX_FREE_REQ_HDL 10

/* forward declaration */
struct msch_req_entity;
struct msch_chan_ctxt;
struct msch_timeslot;

/* Channel context structure per chanspec (control channel) */
typedef struct msch_chan_ctxt {
	msch_list_elem_t link;	/* link to wlc_msch_info->chan_ctxt_list */
	chanspec_t chanspec;	/* chanspec with same ctl chan, but widest band */
	msch_list_elem_t req_entity_list;	/* queue in priority order (high -> low) */
	msch_list_elem_t bf_link;		/* Both flex channel context (VSDB usecase) */
	uint64 onchan_time;
	uint64 actual_onchan_dur;
	uint64 pend_onchan_dur;
	msch_list_elem_t bf_entity_list;	/* queue in priority order (high -> low) */
	bool	bf_sch_pending;			/* true for bf scheduled */
	uint8	bf_skipped_count;		/* counter for skipped slots */
} msch_chan_ctxt_t;

/* Channel switch timer states */
typedef enum _msch_timer_state {
	MSCH_TIMER_STATE_CHN_SW  = 1,
	MSCH_TIMER_STATE_ONCHAN_FIRE  = 2,
	MSCH_TIMER_STATE_OFF_CHN_PREP  = 3,
	MSCH_TIMER_STATE_OFF_CHN_DONE  = 4,
	MSCH_TIMER_STATE_TS_COMPLETE = 5
} _msch_timer_state_t;

/* Requested slot Callback states
 * req->pend_slot/cur_slot->flags
 */
#define MSCH_RC_FLAGS_ONCHAN_FIRE		(1 << 0)
#define MSCH_RC_FLAGS_START_FIRE_DONE		(1 << 1)
#define MSCH_RC_FLAGS_END_FIRE_DONE		(1 << 2)
#define MSCH_RC_FLAGS_ONFIRE_DONE		(1 << 3)
#define MSCH_RC_FLAGS_SPLIT_SLOT_START		(1 << 4)
#define MSCH_RC_FLAGS_SPLIT_SLOT_END		(1 << 5)
#define MSCH_RC_FLAGS_PRE_ONFIRE_DONE		(1 << 6)

/* Request Handle flags */
#define MSCH_REQ_HDL_FLAGS_NEW_REQ		(1 << 0) /* req_start callback */
#define MSCH_REQ_HDL_FLAGS_CLBK_PENDING	(1 << 1) /* clbk for req_handle is pending */

/* Request entity flags */
#define MSCH_ENTITY_FLAG_MULTI_INSTANCE	(1 << 0)
/* flag to indentify repeating channel in a channel sequence */

/* MSCH return codes */
typedef enum {
	MSCH_OK	= 0,
	MSCH_FAIL = 1,
	MSCH_TIMESLOT_REMOVED = 2,
	MSCH_SCHD_NEXT_SLOT = 3,
	MSCH_ALREADY_SCHD = 4,
	MSCH_REQ_DUR_AVAIL = 5,
	MSCH_NO_SLOT = 6,
	MSCH_TS_CANCELLED = 7
} msch_ret_type_t;

/* MSCH state flags (msch_info->flags) */
#define	MSCH_STATE_IN_TIEMR_CTXT	0x1
#define MSCH_STATE_SCHD_PENDING		0x2

/* MSCH timeslot flags */
#define MSCH_TS_FLAG_DONT_DESTROY		0x1 /* TS cannot be destroyed */

/*
 * This structure holds the SLOT_START and SLOT_END
 * times
 */
typedef struct msch_req_timing {
	msch_list_elem_t link;		/* link to msch_info->req_timing_list */
	uint32 flags;
	uint64 pre_start_time;
	uint64 start_time;
	uint64 end_time;
	struct msch_timeslot *timeslot; /* point to timeslot */
} msch_req_timing_t;

/* Requested entity per chanspec requested */
typedef struct msch_req_entity {
	msch_list_elem_t req_hdl_link;		/* link to wlc_msch_reqhandle->req_entity_list */
	msch_list_elem_t chan_ctxt_link;	/* link to chan_ctxt->req_entity_list */

	/* MSCH_RT_BOTH_FIXED/MSCH_RT_DUR_FLEX: link to retry_periodic_list
	*  MSCH_RT_START_FLEX: link to wlc_msch_info->msch_start_flex_list
	*  MSCH_RT_BOTH_FLEX: link to chan_ctxt->bf_entity_list
	*/
	msch_list_elem_t rt_specific_link;
	msch_list_elem_t start_fixed_link;	/* TODO replace this with rt_specific_link */
	msch_list_elem_t both_flex_list;	/* TODO replace with rt_specific_link */
	msch_req_timing_t cur_slot;		/* start/end time info for current timeslot */
	msch_req_timing_t pend_slot;

	msch_chan_ctxt_t *chan_ctxt;		/* point to chan_ctxt */
	wlc_msch_req_handle_t *req_hdl;		/* point to reqhandle */

	uint64 last_serv_time;		/* used to calc home_away_time */
	/* original request parameter */
	chanspec_t chanspec;			/* actual chanspec requested */
	uint8 priority;				/* requested priority */
	uint16 onchan_chn_idx;		/* idx of channel which is in cur_ts */
	uint16 cur_chn_idx;		/* current pending instance channel in channel seq */
	uint32 flags;
	uint64 actual_start_time;	/* actual start time before tweaking to schd */
					/* valid only for Start Fixed use case */
	uint64 curts_fire_time;
	uint32 chsw_time;
	bool from_2g;
} msch_req_entity_t;

typedef struct msch_timeslot {
	uint32 timeslot_id;		/* unique id represent this time slot */
	uint64 pre_start_time;
	uint64 end_time;
	uint64 sch_dur;
	uint64 fire_time;		/* The next fire time for chn switch timer */
	msch_chan_ctxt_t *chan_ctxt;	/* channel for the timeslot */
	uint8 state;
	uint16 ts_flags;
	msch_list_elem_t timeslot_list_link;
} msch_timeslot_t;

/* Handle per timeslot request (1 or more channels) */
struct wlc_msch_req_handle {
	msch_list_elem_t link;		/* link to wlc_msch_info->msch_req_hdl_list */
	wlc_msch_callback cb_func;	/* callback function */
	void *cb_ctxt;			/* callback context to be passed back */
	wlc_msch_req_param_t *req_param; /* original request parameter */
	uint32 chan_cnt;
	msch_list_elem_t req_entity_list; /* list of entity (one/chanspec) */
	uint32 flags;
	chanspec_t *chn_list;
	uint16 chan_idx;
	uint16 last_chan_idx;	/* last valid chn excluding free slots */
	uint64 req_time;
	uint32 chansw_reason;
};

struct wlc_msch_info {
	uint32	magic_num;
	wlc_info_t *wlc; /* pointer to main wlc structure */

	/* free memory pool */
	msch_list_elem_t free_req_hdl_list;
	msch_list_elem_t free_req_entity_list;
	msch_list_elem_t free_chan_ctxt_list;
	msch_list_elem_t free_chanspec_list;

	/* global list */
	msch_list_elem_t msch_req_hdl_list;
	msch_list_elem_t msch_chan_ctxt_list;
	msch_list_elem_t msch_req_timing_list;
	msch_list_elem_t msch_start_fixed_list;	/* MSCH_RT_BOTH_FIXED, MSCH_RT_DUR_FLEX */
	msch_list_elem_t msch_both_flex_req_entity_list;
	msch_list_elem_t msch_start_flex_list;	/* MSCH_RT_START_FLEX */
	msch_list_elem_t msch_both_flex_list;	/* MSCH_RT_BOTH_FLEX, queue chan_ctxt */
	uint32 flex_list_cnt;

	msch_timeslot_t * cur_msch_timeslot;
	msch_timeslot_t * next_timeslot;

	/* timer */
	wlc_hrt_to_t *chsw_timer;
	msch_timeslot_t *cur_armed_timeslot;
	wlc_hrt_to_t *slotskip_timer;
	uint32 slotskip_flag;		/* flag for slot skip timer */

	/* state flags */
	uint32	flags;
	/* timeslot id */
	uint32 ts_id;

	uint32 service_interval;
	uint32 max_lo_prio_interval;

	uint32 us_per_1024ticks;
	uint32 time_h;
	uint32 time_l;
	uint32 pm_notif_dur_us;
	wlc_msch_profiler_info_t *profiler;
	msch_list_elem_t free_timeslot_list;
};
#endif /* _wlc_msch_priv_h_ */
