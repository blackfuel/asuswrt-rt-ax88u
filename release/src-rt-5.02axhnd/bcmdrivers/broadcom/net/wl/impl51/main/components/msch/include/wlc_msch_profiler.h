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
 * $Id: wlc_msch_profiler.h 707078 2017-06-24 17:43:02Z $
 */

#ifndef _wlc_msch_profiler_h_
#define _wlc_msch_profiler_h_

#if (defined(EVENT_LOG_COMPILE) && defined(MSCH_EVENT_LOG)) || defined(MSCH_PROFILER) \
	|| !defined(DONGLEBUILD)

#include <wlc_msch.h>
#include <event_log.h>

/* MSCH profiler support */
#ifdef MSCH_PROFILER
#if defined(WL_ENAB_RUNTIME_CHECK) || defined(ROM_ENAB_RUNTIME_CHECK)
	#define MSCH_PROFILER_ENAB(info)	((info)->_msch_profiler)
#elif defined(MSCH_PROFILER_DISABLED)
	#define MSCH_PROFILER_ENAB(info)	(0)
#else
	#define MSCH_PROFILER_ENAB(info)	(1)
#endif /* WL_ENAB_RUNTIME_CHECK || ROM_ENAB_RUNTIME_CHECK */
#else
#define MSCH_PROFILER_ENAB(info)		(0)
#endif /* MSCH_PROFILER */

#define MSCH_ERROR_ON		0x01
#define MSCH_DEBUG_ON		0x02
#define MSCH_INFORM_ON		0x04
#define MSCH_TRACE_ON		0x08

#define MAX_PROFILE_BUFFER_SIZE		(60 * 1024)
#define MIN_PROFILE_BUFFER_SIZE		(20 * 1024)
#define DEFAULT_PROFILE_BUFFER_SIZE	(30 * 1024)

typedef struct wlc_msch_profiler_info wlc_msch_profiler_info_t;

#ifdef MSCH_TESTING
#define MAX_REGISTER_MSCH	16
typedef struct wlc_msch_test {
	wlc_msch_info_t *p_info;
	wlc_msch_req_handle_t *p_req_hdl;
} wlc_msch_test_t;
#endif /* MSCH_TESTING */

typedef struct wlc_msch_profiler {
	uint32	magic1;
	uint32	magic2;
	int	start_ptr;
	int	write_ptr;
	int	write_size;
	int	read_ptr;
	int	read_size;
	int	total_size;
	uint8  *buffer;
} wlc_msch_profiler_t;

typedef struct wlc_msch_profiler_cmn {
	wlc_msch_profiler_t *profiler;
	uint8               *edata;
	uint8  ver;
	uint8  dump_enable;
	uint8  dump_profiler;
	uint8  dump_callback;
	uint8  dump_register;
	uint8  dump_print_messages;
	uint8  dump_event_messages;
	uint8  dump_status;
	uint8  dump_event_log_messages;
	bool   event_data_flush;
	bool   event_timer_start;
	uint16 edata_len;
	msch_collect_tlv_t *ptlv;
	wlc_hrt_to_t *event_timer;
#ifdef MSCH_TESTING
	wlc_msch_test_t test[MAX_REGISTER_MSCH];
#endif /* MSCH_TESTING */
#ifndef DONGLEBUILD
	char   mschbufp[WL_MSCH_PROFILER_BUFFER_SIZE];
	bool lastMessages;
#endif /* DONGLEBUILD */
} wlc_msch_profiler_cmn_t;

struct wlc_msch_profiler_info {
	wlc_msch_profiler_cmn_t *cmn;
	wlc_info_t *wlc;
	wlc_msch_info_t *msch_info;
	uint16 msch_chanspec_alloc_cnt;
	uint16 msch_req_entity_alloc_cnt;
	uint16 msch_req_hdl_alloc_cnt;
	uint16 msch_chan_ctxt_alloc_cnt;
	uint16 msch_timeslot_alloc_cnt;
	uint16 wl_index;
	bool   _msch_profiler;
#ifndef DONGLEBUILD
	uint64 solt_start_time, req_start_time, profiler_start_time;
	uint32 solt_chanspec, req_start;
#endif /* DONGLEBUILD */
};

#define MSCH_MAGIC_1		0x4d534348
#define MSCH_MAGIC_2		0x61676963

#define MSCH_EVENT_BIT		0x01
#define MSCH_PROFILE_BIT	0x02
#define MSCH_EVENT_LOG_BIT	0x04

#define PROFILE_CNT_INCR(msch_info, type)	msch_info->profiler->type ++
#define PROFILE_CNT_DECR(msch_info, type)	msch_info->profiler->type --

extern int msch_msg_level;

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)

extern void msch_event_log0(wlc_msch_info_t *msch_info, int filter, int fmtNum);
extern void msch_event_log1(wlc_msch_info_t *msch_info, int filter, int fmtNum, uint32 t1);
extern void msch_event_log2(wlc_msch_info_t *msch_info, int filter, int fmtNum, uint32 t1,
	uint32 t2);
extern void msch_event_log3(wlc_msch_info_t *msch_info, int filter, int fmtNum, uint32 t1,
	uint32 t2, uint32 t3);
extern void msch_event_log4(wlc_msch_info_t *msch_info, int filter, int fmtNum, uint32 t1,
	uint32 t2, uint32 t3, uint32 t4);
extern void msch_event_logn(wlc_msch_info_t *msch_info, int filter, int num_args, int fmtNum, ...);

#define _MSCH_EVENT_LOG0(msch_info, filter, fmt_num) \
	msch_event_log0(msch_info, filter, fmt_num)
#define _MSCH_EVENT_LOG1(msch_info, filter, fmt_num, t1) \
	msch_event_log1(msch_info, filter, fmt_num, (uint32)(t1))
#define _MSCH_EVENT_LOG2(msch_info, filter, fmt_num, t1, t2) \
	msch_event_log2(msch_info, filter, fmt_num, (uint32)(t1), (uint32)(t2))
#define _MSCH_EVENT_LOG3(msch_info, filter, fmt_num, t1, t2, t3) \
	msch_event_log3(msch_info, filter, fmt_num, (uint32)(t1), (uint32)(t2), (uint32)(t3))
#define _MSCH_EVENT_LOG4(msch_info, filter, fmt_num, t1, t2, t3, t4) \
	msch_event_log4(msch_info, filter, fmt_num, (uint32)(t1), (uint32)(t2), (uint32)(t3), \
	(uint32)(t4))

/* The rest call the generic routine that takes a count */
#define _MSCH_EVENT_LOG5(msch_info, filter, fmt_num, ...) \
	msch_event_logn(msch_info, filter, 5, fmt_num, __VA_ARGS__)
#define _MSCH_EVENT_LOG6(msch_info, filter, fmt_num, ...) \
	msch_event_logn(msch_info, filter, 6, fmt_num, __VA_ARGS__)
#define _MSCH_EVENT_LOG7(msch_info, filter, fmt_num, ...) \
	msch_event_logn(msch_info, filter, 7, fmt_num, __VA_ARGS__)
#define _MSCH_EVENT_LOG8(msch_info, filter, fmt_num, ...) \
	msch_event_logn(msch_info, filter, 8, fmt_num, __VA_ARGS__)
#define _MSCH_EVENT_LOG9(msch_info, filter, fmt_num, ...) \
	msch_event_logn(msch_info, filter, 9, fmt_num, __VA_ARGS__)

#define _MSCH_EVENT_LOG_VA_NUM_ARGS(F, _1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) F ## N
#define _MSCH_EVENT_LOG(msch_info, filter, fmt, ...)						\
	do {											\
		static char logstr[] __attribute__ ((section(".logstrs"))) = fmt;		\
		static uint32 fmtnum __attribute__ ((section(".lognums"))) = (uint32) &logstr;	\
		if (msch_msg_level & filter) {							\
			_MSCH_EVENT_LOG_VA_NUM_ARGS(_MSCH_EVENT_LOG, ##__VA_ARGS__, 9, 8,	\
				       7, 6, 5, 4, 3, 2, 1, 0)					\
			(msch_info, filter, (int) &fmtnum , ## __VA_ARGS__);			\
		}										\
	} while (0)
#define MSCH_MESSAGE(args)	_MSCH_EVENT_LOG args

#else /* EVENT_LOG_COMPILE && DONGLEBUILD */

extern void msch_message_printf(wlc_msch_info_t *msch_info, int filter,
	const char *fmt, ...);
#define MSCH_MESSAGE(args)	msch_message_printf args

#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

extern uint32 msch_time_h(uint64 cur_time);
extern uint32 msch_time_l(uint64 cur_time);
#define _msch_display_time(cur_time)		msch_time_h(cur_time), msch_time_l(cur_time)

extern wlc_msch_profiler_info_t *wlc_msch_profiler_attach(wlc_msch_info_t *msch_info);
extern void wlc_msch_profiler_detach(wlc_msch_profiler_info_t *profiler_info);
extern void _msch_dump_profiler(wlc_msch_profiler_info_t *profiler_info);
extern void _msch_dump_register(wlc_msch_profiler_info_t *profiler_info,
	chanspec_t* chanspec_list, int chanspec_cnt, wlc_msch_req_param_t *req_param);
extern void _msch_dump_callback(wlc_msch_profiler_info_t *profiler_info,
	wlc_msch_cb_info_t *cb_info);

#ifdef MSCH_TESTING
extern void _msch_timeslot_unregister(wlc_msch_profiler_info_t *profiler_info,
	wlc_msch_req_handle_t *req_hdl);
#else /* MSCH_TESTING */
#define _msch_timeslot_unregister(profiler_info, req_hdl)
#endif /* MSCH_TESTING */

#else /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */

#define PROFILE_CNT_INCR(msch_info, type)
#define PROFILE_CNT_DECR(msch_info, type)
#define MSCH_MESSAGE(x)
#define _msch_display_time(cur_time)		-1, 0
#define wlc_msch_profiler_info_t 		void
#define wlc_msch_profiler_attach(msch_info)	NULL
#define wlc_msch_profiler_detach(profiler_info)
#define _msch_dump_profiler(profiler_info)
#define _msch_dump_register(profiler_info, chanspec_list, chanspec_cnt, req_param)
#define _msch_dump_callback(profiler_info, cb_info)
#define _msch_timeslot_unregister(profiler_info, req_hdl)

#endif /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */

#endif /* _wlc_msch_profiler_h_ */
