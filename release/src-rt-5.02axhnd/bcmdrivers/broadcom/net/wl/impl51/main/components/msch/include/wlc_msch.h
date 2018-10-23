/*
 * Interface definitions for multi-channel scheduler
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
 * $Id: wlc_msch.h 711170 2017-07-17 07:28:46Z $
 */

#ifndef _wlc_msch_h_
#define _wlc_msch_h_

/*
 * The OFFCB_DUR is fixed to 5 ms, but subject to change.
 * This fixed duration is the interval from OFF_CHAN to OFF_CHAN_DONE.
 */
#define MSCH_MAX_OFFCB_DUR (5 * 1000)

/*
 * The ONCHAN_PREPARE is fixed to 5 ms, but subject to change
 * This fixed duration is the interval from OFF_CHAN_DONE to REQ_START.
 */
#define MSCH_ONCHAN_PREPARE (5 * 1000)
#define MSCH_PREPARE_DUR (MSCH_MAX_OFFCB_DUR + MSCH_ONCHAN_PREPARE)
#define MSCH_CANCELSLOT_PREPARE (5000) /* 5ms */
#define MSCH_SKIP_PRE_NOTIF_TIME (5000) /* 5 ms */
/*
 * The MIN_ONCHAN_TIME is fixed to 10 ms. The 10 ms unit available for schedule.
 */
#define MSCH_MIN_ONCHAN_TIME (10 * 1000)
/*
 * TODO: OFFCB_DUR can be changed so need to caculate dynamically and
 * handle it separately.
 */
#define MSCH_MIN_FREE_SLOT (MSCH_PREPARE_DUR + MSCH_MIN_ONCHAN_TIME)
#define MSCH_DELAY_FOR_CLBK (2 * 1000)	/* Delay for schd slots in callback */
#define MSCH_EXTRA_DELAY_FOR_MAX_AWAY_DUR (5 * 1000)

#define	MS_TO_USEC(a)	((a)*1000)
#define MSCH_TIME_UNIT_1024US  10
#define DEFAULT_HOME_TIME        45
#define DEFAULT_HOME_AWAY_TIME   100
#define DEFAULT_SCAN_PASSIVE_TIME  110
#define MSCH_INFINITE_TIME (0xFFFFFFFFFFFFFFFFull)

/* MSCH clbk types (bit masks) */

#define	MSCH_CT_REQ_START	0x1
	/*
	 * this callback function is invoked when request (multi channels) starts.
	 * PHY on requested channel
	 */

#define	MSCH_CT_ON_CHAN		0x2
	/*
	 * this callback function is invoked when switch to given chanspec.
	 * In callback function, the major work is to annouce the presence to this channel.
	 * e.g. For STA, you may need send out NULL data with PM bit clear.
	 *
	 * provide type specific info in wlc_msch_onchan_info_t.
	 * PHY on requested channel
	 */

#define	MSCH_CT_SLOT_START	0x4
	/*
	 * this callback function is invoked when requested time slot (one channel) starts.
	 * PHY on requested channel
	 */

#define	MSCH_CT_SLOT_END	0x8
	/*
	 * this callback function is invoked when requested time slot (one channel) ends.
	 * PHY channel will be changed immediately after this callback
	 */

#define	MSCH_CT_SLOT_SKIP	0x10
	/*
	 * this callback function is invoked when scheduler kick out granted time request.
	 *
	 * provide type specific info in wlc_msch_skipslot_info_t.
	 * PHY channel - Not Applicable
	 */

#define	MSCH_CT_OFF_CHAN	0x20
	/*
	 * this callback function is invoked when switch off given chanspec.
	 * In callback function, the major work is to annouce the absence to this channel.
	 * e.g. For STA, you may need send out NULL data with PM bit set.
	 * PHY on requested channel
	 */

#define MSCH_CT_OFF_CHAN_DONE	0x40
	 /* PHY channel will be changed immediately after this callback
	  */

#define	MSCH_CT_REQ_END		0x80
	/*
	 * this callback function is invoked when whole request (multi channels) ends.
	 */
#define	MSCH_CT_PARTIAL		0x100
	 /* partial slot end/slot start due to high priority pre-emptive request */

#define	MSCH_CT_PRE_ONCHAN	0x200
	/* Callback before switching to the requested channel,
	 * useful to radio on/off
	 */

#define	MSCH_CT_PRE_REQ_START	0x400
	/* Callback before switching to the requested channel when req starts
	 */
#define	MSCH_CT_SLOT_CONTINUE	0x800
	/* module is scheduled in next slot, can continue without
	 * flow control
	 */
/* MSCH clbk type end */

/* Flags used in wlc_msch_req_param_t struct */
#define MSCH_REQ_FLAGS_CHAN_CONTIGUOUS (1 << 0) /* Don't break up channels in chanspec_list */
#define MSCH_REQ_FLAGS_MERGE_CONT_SLOTS	(1 << 1)  /* No slot end if slots are continous */
#define MSCH_REQ_FLAGS_PREMTABLE	(1 << 2) /* Req can be pre-empted by PREMT_CURTS req */
#define MSCH_REQ_FLAGS_PREMT_CURTS	(1 << 3) /* Pre-empt request at the end of curts */
#define MSCH_REQ_FLAGS_PREMT_IMMEDIATE	(1 << 4) /* Pre-empt cur_ts immediately */

#define MSCH_PREEMPT_REQ(x) (((x & MSCH_REQ_FLAGS_PREMT_CURTS)||\
	(x & MSCH_REQ_FLAGS_PREMT_IMMEDIATE)))

typedef struct wlc_msch_onchan_info {
	uint32 timeslot_id;		/* unique time slot id */
	uint32 start_time_l;	/* time slot prestart time low 32bit */
	uint32 start_time_h;	/* time slot prestart time high 32bit */
	uint32 end_time_l;		/* time slot end time low 32 bit */
	uint32 end_time_h;		/* time slot end time high 32 bit */
	uint32 onchan_idx;		/* Current channel index */
	uint64 cur_chan_seq_start_time; /* start time of current sequence */
} wlc_msch_onchan_info_t;

typedef struct wlc_msch_skipslot_info {
	uint32 start_time_l;	/* time slot prestart time low 32bit  */
	uint32 start_time_h;	/* time slot prestart time high 32bit */
	uint32 end_time_l;		/* time slot end time low 32 bit       */
	uint32 end_time_h;		/* time slot end time high 32 bit    */
} wlc_msch_skipslot_info_t;

typedef struct wlc_msch_cb_info {
	uint32	type;			/* callback flag type */
	chanspec_t chanspec;		/* actual chanspec, may different with requested one */
	wlc_msch_req_handle_t *req_hdl;	/* request handle */
	void *type_specific;		/* type specific info */
} wlc_msch_cb_info_t;

typedef enum {
	MSCH_RT_BOTH_FIXED = 0,		/* both start and end time is fixed */
	MSCH_RT_START_FLEX = 1,		/* start time is flexible and duration is fixed */
	MSCH_RT_DUR_FLEX =  2,		/* start time is fixed and end time is flexible */
	MSCH_RT_BOTH_FLEX = 3,		/* Both start and duration is flexible */
} wlc_msch_req_type_t;

/* smaller value, lower priority */
typedef enum {
	MSCH_RP_DISC_BCN_FRAME = 0,     /* Low priority NAN DP can be missed */
	MSCH_DEFAULT_PRIO = 1,		/* Default priority  */
	MSCH_RP_CONNECTION = 1,		/* STA, GC, GO, SCAN */
	MSCH_RP_DTIM = 2,		/* Internal Priority for DTIM */
	MSCH_RP_FUR_AVAIL = 3,		/* Further Availability */
	MSCH_RP_SYNC_FRAME = 4,		/* AWDL or NAN */
	MSCH_RP_ASSOC_SCAN = 5,		/* ASSOC SCAN */
} wlc_msch_req_prio_t;

/* Flags used in wlc_msch_req_param_t struct */
#define MSCH_REQ_FLAGS_CHAN_CONTIGUOUS (1 << 0) /* Don't break up channels in chanspec_list */

typedef struct _wlc_msch_req_param_t {
	uint32	flags;			/* Describe various request properties */
	wlc_msch_req_type_t req_type;	/* Describe start and end time flexiblilty */
	wlc_msch_req_prio_t priority;	/* Define the request priority */
	uint32 start_time_l;		/* Requested start time in us unit, low 32bit */
	uint32 start_time_h;		/* Requested start time in us unit, high 32bit */
	uint32 duration;		/* Requested duration in us unit */
	uint32 interval;		/* Requested periodic interval in us unit,
					 * 0 means non-periodic
					 */
	union {
		uint32 dur_flex;	/* MSCH_RT_DUR_FLEX, min_dur = duration - dur_flex */
		struct {
			uint32 min_dur;		  /* min duration for traffic, home_time */
			uint32 max_away_dur;	  /* max acceptable away dur, home_away_time */

			uint32 hi_prio_time_l;
			uint32 hi_prio_time_h;	  /* high priority time point (e.g. dtim beacon) */
			uint32 hi_prio_interval; /* repeated high priority interval */
		} bf; /* MSCH_RT_BOTH_FLEX */
	} flex;
} wlc_msch_req_param_t;

/* MSCH udpate mask */
#define	MSCH_UPDATE_START_TIME		0x1
#define MSCH_UPDATE_CHANSPEC		0x2
#define MSCH_UPDATE_INTERVAL		0x4
#define	MSCH_UPDATE_END_TIME		0x8
/*
 * MSCH Call back interface declaration
 */
typedef int (*wlc_msch_callback)(void* handler_ctxt, wlc_msch_cb_info_t *cb_info);

/* Register and request time slots in multi-channel scheduler
 *
 *  msch_info - msch module context pointer
 *  chanspec_list - list of chanspec
 *  channel_num - count of chanspec list
 *  cb - call back function and context pointer
 *  req_param - parameters of requested time slot. If multiple chanspecs are specified,
 *	it is the time parameter for first chanspec. Following chanspecs' time parameter can be
 *	derived from first one.
 *  p_req_hdl -  return created request entity handle if succeed
 *
 *  return - bcm error return
 */
int wlc_msch_timeslot_register(wlc_msch_info_t *msch_info, chanspec_t* chanspec_list,
	int chanspec_cnt, wlc_msch_callback cb_func, void *cb_ctxt, wlc_msch_req_param_t *req_param,
	wlc_msch_req_handle_t **p_req_hdl);

/* Cancel and unregister scheduled time slots in multi-channel scheduler
 *
 *  msch_info - msch module context pointer
 *  p_req_hdl - pointer to request entity handle returned in register_timeslot function
 *
 *  return - bcm error return
 */
int wlc_msch_timeslot_unregister(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t **p_req_hdl);

/* Cancel one scheduled time slot in multi-channel scheduler
 *
 *  msch_info - msch module context pointer
 *  req_hdl - request entity handle returned in register_timeslot function
 *  timeslot_id - unique id for specific time slot
 *
 *  return - bcm error return
 */
#ifdef MSCH_NEW_CANCEL_API
int wlc_msch_timeslot_cancel(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t **req_hdl,
	uint32 timeslot_id);
#else
int wlc_msch_timeslot_cancel(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	uint32 timeslot_id);
#endif // endif

/* Set timeslot register reason (channel switch reason)
*
*  msch_info - msch module context pointer
*  req_hdl - request entity handle returned in register_timeslot function
*  reason - timeslot register reason
*
*  return - set error return
*/
int wlc_msch_set_chansw_reason(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	uint32 reason);

/* Returns the msch reference time in micro seconds
 *
 *  msch_info - msch module context pointer
 */
uint64 msch_current_time(wlc_msch_info_t *msch_info);

/* Returns the msch future reference time in micro seconds (_msch_current_time + offset)
 *
 *  msch_info - msch module context pointer
 */
uint64 msch_future_time(wlc_msch_info_t *msch_info, uint32 delta_in_usec);

/* Time conversion functions for tsf to pmu and pmu to tsf */
uint64 msch_tsf_to_mschtime(wlc_msch_info_t *msch_info, uint32 tsf_lo, uint32 tsf_hi,
	uint32 *pmu_lo, uint32 *pmu_hi);
void msch_mschtime_to_tsf(wlc_msch_info_t *msch_info, uint64 pmu_time, uint32 *tsf_lo,
	uint32 *tsf_hi);

/* Update pending MSCH request
 *
 *  msch_info - msch module context pointer
 *  req_hdl - handle to the pending request
 *  param - parameters to update
 *  update_mask - which of the param field to update
 *  return - bcm error return
 */
int wlc_msch_timeslot_update(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *p_req_hdl,
	wlc_msch_req_param_t *param, uint32 update_mask);

/* Query timeslot shared
 *
 *  msch_info - msch module context pointer
 *  timeslot_id - from callback info
 *  returns TRUE if the corresponding timeslot is been shared by more than 1 module
 */
bool
wlc_msch_query_ts_shared(wlc_msch_info_t *msch_info, uint32 timeslot_id);

/*
 * Queries the slot availability from start_time for duration
 * start_time 0 - returns the end time of current timeslot
 *
 * TBD : Only start time 0 is implemented
 *		 time based query not implemented
 */
uint64 wlc_msch_query_timeslot(wlc_msch_info_t *msch_info, uint64 start_time,
		uint32 duration);

/* multi-channel scheduler module interface for attach */
wlc_msch_info_t* wlc_msch_attach(wlc_info_t* wlc);
/* multi-channel scheduler module interface for  detach */
void wlc_msch_detach(wlc_msch_info_t *msch_info);

/* helper, calculates slot duration in msch callback, if duration infinite return -1 */
int32 msch_calc_slot_duration(wlc_msch_info_t *msch_info, wlc_msch_cb_info_t *cb_info);

/* Update channel bandwidth
 */
void
msch_update_bw(wlc_msch_info_t *msch_info, wlc_msch_req_handle_t *req_hdl,
	chanspec_t old_chanspec, chanspec_t chanspec);

extern int msch_timeslot_update_slot_endtime(wlc_msch_info_t *msch_info,
	wlc_msch_req_handle_t *p_req_hdl, wlc_msch_req_param_t *param);

/* Query with max allocable channel contexts based upon
 * the given free heapsize
 */
int wlc_msch_allocable_chctxts_num_get(wlc_msch_info_t *msch_info,
	uint32 free_heapsize);
/*
 * update scan_home_time and scan_home_away_time in current msch handlers
 */
void wlc_msch_update_home_away_time(wlc_msch_info_t *msch_info,
	uint32 min_dur, uint32 max_away_dur);

#endif /* _wlc_msch _h_ */
