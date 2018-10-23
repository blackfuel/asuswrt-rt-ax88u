
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
* $Id: wlc_msch_profiler.c 707078 2017-06-24 17:43:02Z $
*
*/
#if (defined(EVENT_LOG_COMPILE) && defined(MSCH_EVENT_LOG)) || defined(MSCH_PROFILER) \
	|| !defined(DONGLEBUILD)

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_hrt.h>
#include <hndpmu.h>

#include <wlc_msch.h>
#include <wlc_mschutil.h>
#include <wlc_msch_priv.h>
#include <wlc_event_utils.h>

#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <machine/stdarg.h>
#else
#include <stdarg.h>
#endif /* __FreeBSD__ */

#define MSCH_NAME "msch"
#define US_PRE_SEC		1000000

#define MSCH_EVENT_LOG_WAIT_TIME	(10 * 1000)

/* IOVAR declarations */
enum {
	IOV_MSCH_REQ,
	IOV_MSCH_UNREQ,
	IOV_MSCH_EVENT,
	IOV_MSCH_COLLECT,
	IOV_MSCH_DUMP,
	IOV_MSCH_EVENT_LOG,
	IOV_MSCH_LAST
};

/* Iovars */
static const bcm_iovar_t  wlc_msch_iovars[] = {
#ifdef MSCH_TESTING
	{"msch_req", IOV_MSCH_REQ, 0, 0, IOVT_BUFFER, 0},
	{"msch_unreq", IOV_MSCH_UNREQ, 0, 0, IOVT_INT32, 0},
#endif /* MSCH_TESTING */
	{"msch_event", IOV_MSCH_EVENT, 0, 0, IOVT_INT32, 0},
	{"msch_collect", IOV_MSCH_COLLECT, 0, 0, IOVT_BUFFER, 0},
	{"msch_dump", IOV_MSCH_DUMP, 0, 0, IOVT_BUFFER, 0},
	{"msch_event_log", IOV_MSCH_EVENT_LOG, 0, 0, IOVT_INT32, 0},
	{NULL, 0, 0, 0, 0, 0}
};

#define MSCH_TESTING_DEBUG(x)	printf x

#define EVENT_LOG_MSCH_BLOCK_SIZE	1024

#undef ROUNDUP
#define	ROUNDUP(x)		(((x) + 3) & (~0x03))
#ifdef MSCH_PROFILER
static int
_msch_read_profiler_tlv(wlc_msch_profiler_info_t *profiler_info, uint8 *buf, int len);
#endif /* MSCH_PROFILER */
static void
_msch_write_profiler_tlv(wlc_msch_profiler_info_t *profiler_info, uint type, int len);
#ifdef MSCH_TESTING
static int
_msch_tesing_callback(void *ctx, wlc_msch_cb_info_t *cb_info);
#endif /* MSCH_TESTING */
static int32
wlc_msch_doiovar_get(wlc_msch_profiler_info_t *profiler_info, uint8 enable_bit,
	uint8 log_messages);
static int
wlc_msch_doiovar_set(wlc_msch_profiler_info_t *profiler_info, uint8 enable_bit,
	uint8 *log_messages, int32 int_val);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

int msch_msg_level = MSCH_ERROR_ON;

static bool
_msch_find_entry(msch_list_elem_t* list, void* entry)
{
	while ((list = msch_list_iterate(list))) {
		if ((void *)list == entry)
			return TRUE;
	}
	return FALSE;
}

#ifdef MSCH_PROFILER
static int
_msch_read_profiler_tlv(wlc_msch_profiler_info_t *profiler_info, uint8 *buf, int len)
{
	wlc_msch_profiler_t *profiler;
	uint8 *pbuf;
	msch_collect_tlv_t *p;
	int size, copysize;

	ASSERT(profiler_info);

	profiler = profiler_info->cmn->profiler;
	if (!profiler || profiler->read_size >= profiler->write_size)
		return BCME_ERROR;

	pbuf = profiler->buffer + profiler->read_ptr;
	p = (msch_collect_tlv_t *)pbuf;
	size = WL_MSCH_PROFILE_HEAD_SIZE + (int)(p->size);

	if (size > len)
		return BCME_BUFTOOSHORT;

	profiler->read_size += ROUNDUP(size);
	if (profiler->read_size > profiler->write_size)
		return BCME_ERROR;

	copysize = profiler->total_size - profiler->read_ptr;
	if (copysize > size)
		copysize = size;

	bcopy(pbuf, buf, copysize);
	if (copysize < size) {
		size -= copysize;
		bcopy(profiler->buffer, buf + copysize, size);
		profiler->read_ptr = ROUNDUP(size);
	} else {
		profiler->read_ptr += ROUNDUP(size);
		if (profiler->read_ptr == profiler->total_size) {
			profiler->read_ptr = 0;
		}
	}

	return BCME_OK;
}
#endif /* MSCH_PROFILER */

#ifndef DONGLEBUILD
#define MSCH_EVENTS_PRINT(mschbufp) \
	do { \
		printf("%s", mschbufp); \
	} while (0)

#define MSCH_EVENTS_SPPRINT(mschbufp, space) \
	do { \
		if (space > 0) { \
			int ii; \
			for (ii = 0; ii < space; ii++) mschbufp[ii] = ' '; \
			mschbufp[space] = '\0'; \
			MSCH_EVENTS_PRINT(mschbufp); \
		} \
	} while (0)

#define MSCH_EVENTS_PRINTF(mschbufp, fmt) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_PRINTF1(mschbufp, fmt, a) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt, \
			(a)); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_PRINTF2(mschbufp, fmt, a, b) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt, \
			(a), (b)); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_PRINTF3(mschbufp, fmt, a, b, c) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt, \
			(a), (b), (c)); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_PRINTF4(mschbufp, fmt, a, b, c, d) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt, \
			(a), (b), (c), (d)); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_PRINTF5(mschbufp, fmt, a, b, c, d, e) \
	do { \
		snprintf(mschbufp, WL_MSCH_PROFILER_BUFFER_SIZE, fmt, \
			(a), (b), (c), (d), (e)); \
		MSCH_EVENTS_PRINT(mschbufp); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF(mschbufp, space, fmt) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF(mschbufp, fmt); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF1(mschbufp, space, fmt, a) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF1(mschbufp, fmt, (a)); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF2(mschbufp, space, fmt, a, b) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF2(mschbufp, fmt, (a), (b)); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF3(mschbufp, space, fmt, a, b, c) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF3(mschbufp, fmt, (a), (b), (c)); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF4(mschbufp, space, fmt, a, b, c, d) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF4(mschbufp, fmt, (a), (b), (c), (d)); \
	} while (0)

#define MSCH_EVENTS_SPPRINTF5(mschbufp, space, fmt, a, b, c, d, e) \
	do { \
		MSCH_EVENTS_SPPRINT(mschbufp, space); \
		MSCH_EVENTS_PRINTF5(mschbufp, fmt, (a), (b), (c), (d), (e)); \
	} while (0)

static void wl_msch_us_to_sec(uint32 time_h, uint32 time_l, uint32 *sec, uint32 *remain)
{
	uint64 cur_time = ((uint64)(ntoh32(time_h)) << 32) | ntoh32(time_l);
	uint64 r, u = 0;

	r = cur_time;
	while (time_h != 0) {
		u += (uint64)((0xffffffff / US_PRE_SEC)) * time_h;
		r = cur_time - u * US_PRE_SEC;
		time_h = (uint32)(r >> 32);
	}

	*sec = (uint32)(u + ((uint32)(r) / US_PRE_SEC));
	*remain = (uint32)(r) % US_PRE_SEC;
}

static char *wl_msch_display_time(uint32 time_h, uint32 time_l)
{
	static char display_time[32];
	uint32 s, ss;

	if (time_h == 0xffffffff && time_l == 0xffffffff) {
		snprintf(display_time, sizeof(display_time), "-1");
	} else {
		wl_msch_us_to_sec(time_h, time_l, &s, &ss);
		snprintf(display_time, sizeof(display_time), "%d.%06d", s, ss);
	}
	return display_time;
}

static void
wl_msch_chanspec_list(char *mschbufp, int sp, char *data, uint16 ptr, uint16 chanspec_cnt)
{
	int i, cnt = (int)ntoh16(chanspec_cnt);
	uint16 *chanspec_list = (uint16 *)(data + ntoh16(ptr));
	char buf[CHANSPEC_STR_LEN];
	chanspec_t c;

	MSCH_EVENTS_SPPRINTF(mschbufp, sp, "<chanspec_list>:");
	for (i = 0; i < cnt; i++) {
		c = (chanspec_t)ntoh16(chanspec_list[i]);
		MSCH_EVENTS_PRINTF1(mschbufp, " %s", wf_chspec_ntoa(c, buf));
	}
	MSCH_EVENTS_PRINTF(mschbufp, "\n");
}

static void
wl_msch_elem_list(char *mschbufp, int sp, char *title, char *data, uint16 ptr, uint16 list_cnt)
{
	int i, cnt = (int)ntoh16(list_cnt);
	uint32 *list = (uint32 *)(data + ntoh16(ptr));

	MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "%s_list: ", title);
	for (i = 0; i < cnt; i++) {
		MSCH_EVENTS_PRINTF1(mschbufp, "0x%08x->", ntoh32(list[i]));
	}
	MSCH_EVENTS_PRINTF(mschbufp, "null\n");
}

static void
wl_msch_req_param_profiler_data(char *mschbufp, int sp, char *data, uint16 ptr)
{
	int sn = sp + 4;
	msch_req_param_profiler_event_data_t *p =
		(msch_req_param_profiler_event_data_t *)(data + ntoh16(ptr));
	uint32 type, flags;

	MSCH_EVENTS_SPPRINTF(mschbufp, sp, "<request parameters>\n");
	MSCH_EVENTS_SPPRINTF(mschbufp, sn, "req_type: ");

	type = p->req_type;
	if (type < 4) {
		char *req_type[] = {"fixed", "start-flexible", "duration-flexible",
			"both-flexible"};
		MSCH_EVENTS_PRINTF1(mschbufp, "%s", req_type[type]);
	}
	else
		MSCH_EVENTS_PRINTF1(mschbufp, "unknown(%d)", type);

	flags = ntoh16(p->flags);
	if (flags & WL_MSCH_REQ_FLAGS_CHAN_CONTIGUOUS)
		MSCH_EVENTS_PRINTF(mschbufp, ", CHAN_CONTIGUOUS");
	if (flags & WL_MSCH_REQ_FLAGS_MERGE_CONT_SLOTS)
		MSCH_EVENTS_PRINTF(mschbufp, ", MERGE_CONT_SLOTS");
	if (flags & WL_MSCH_REQ_FLAGS_PREMTABLE)
		MSCH_EVENTS_PRINTF(mschbufp, ", PREMTABLE");
	if (flags & WL_MSCH_REQ_FLAGS_PREMT_CURTS)
		MSCH_EVENTS_PRINTF(mschbufp, ", PREMT_CURTS");
	if (flags & WL_MSCH_REQ_FLAGS_PREMT_IMMEDIATE)
		MSCH_EVENTS_PRINTF(mschbufp, ", PREMT_IMMEDIATE");
	MSCH_EVENTS_PRINTF1(mschbufp, ", priority: %d\n", p->priority);

	MSCH_EVENTS_SPPRINTF3(mschbufp, sn,
		"start-time: %s, duration: %d(us), interval: %d(us)\n",
		wl_msch_display_time(p->start_time_h, p->start_time_l),
		ntoh32(p->duration), ntoh32(p->interval));

	if (type == WL_MSCH_RT_DUR_FLEX)
		MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "dur_flex: %d(us)\n",
			ntoh32(p->flex.dur_flex));
	else if (type == WL_MSCH_RT_BOTH_FLEX) {
		MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "min_dur: %d(us), max_away_dur: %d(us)\n",
			ntoh32(p->flex.bf.min_dur), ntoh32(p->flex.bf.max_away_dur));

		MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "hi_prio_time: %s, hi_prio_interval: %d(us)\n",
			wl_msch_display_time(p->flex.bf.hi_prio_time_h,
			p->flex.bf.hi_prio_time_l),
			ntoh32(p->flex.bf.hi_prio_interval));
	}
}

static void
wl_msch_timeslot_profiler_data(char *mschbufp, int sp, char *title, char *data, uint16 ptr,
	bool empty)
{
	int s, sn = sp + 4;
	msch_timeslot_profiler_event_data_t *p =
		(msch_timeslot_profiler_event_data_t *)(data + ntoh16(ptr));
	char *state[] = {"NONE", "CHN_SW", "ONCHAN_FIRE", "OFF_CHN_PREP",
		"OFF_CHN_DONE", "TS_COMPLETE"};

	MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "<%s timeslot>: ", title);
	if (empty) {
		MSCH_EVENTS_PRINTF(mschbufp, " null\n");
		return;
	}
	else
		MSCH_EVENTS_PRINTF1(mschbufp, "0x%08x\n", ntoh32(p->p_timeslot));

	s = (int)(ntoh32(p->state));
	if (s > 5) s = 0;

	MSCH_EVENTS_SPPRINTF4(mschbufp, sn, "id: %d, state[%d]: %s, chan_ctxt: [0x%08x]\n",
		ntoh32(p->timeslot_id), ntoh32(p->state), state[s], ntoh32(p->p_chan_ctxt));

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "fire_time: %s",
		wl_msch_display_time(p->fire_time_h, p->fire_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", pre_start_time: %s",
		wl_msch_display_time(p->pre_start_time_h, p->pre_start_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", end_time: %s",
		wl_msch_display_time(p->end_time_h, p->end_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", sch_dur: %s\n",
		wl_msch_display_time(p->sch_dur_h, p->sch_dur_l));
}

static void
wl_msch_req_timing_profiler_data(char *mschbufp, int sp, char *title, char *data, uint16 ptr,
	bool empty)
{
	int sn = sp + 4;
	msch_req_timing_profiler_event_data_t *p =
		(msch_req_timing_profiler_event_data_t *)(data + ntoh16(ptr));
	uint32 type;

	MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "<%s req_timing>: ", title);
	if (empty) {
		MSCH_EVENTS_PRINTF(mschbufp, " null\n");
		return;
	}
	else
		MSCH_EVENTS_PRINTF3(mschbufp, "0x%08x (prev 0x%08x, next 0x%08x)\n",
			ntoh32(p->p_req_timing), ntoh32(p->p_prev), ntoh32(p->p_next));

	MSCH_EVENTS_SPPRINTF(mschbufp, sn, "flags:");
	type = ntoh16(p->flags);
	if ((type & 0x7f) == 0)
		MSCH_EVENTS_PRINTF(mschbufp, " NONE");
	else {
		if (type & WL_MSCH_RC_FLAGS_ONCHAN_FIRE)
			MSCH_EVENTS_PRINTF(mschbufp, " ONCHAN_FIRE");
		if (type & WL_MSCH_RC_FLAGS_START_FIRE_DONE)
			MSCH_EVENTS_PRINTF(mschbufp, " START_FIRE");
		if (type & WL_MSCH_RC_FLAGS_END_FIRE_DONE)
			MSCH_EVENTS_PRINTF(mschbufp, " END_FIRE");
		if (type & WL_MSCH_RC_FLAGS_ONFIRE_DONE)
			MSCH_EVENTS_PRINTF(mschbufp, " ONFIRE_DONE");
		if (type & WL_MSCH_RC_FLAGS_SPLIT_SLOT_START)
			MSCH_EVENTS_PRINTF(mschbufp, " SPLIT_SLOT_START");
		if (type & WL_MSCH_RC_FLAGS_SPLIT_SLOT_END)
			MSCH_EVENTS_PRINTF(mschbufp, " SPLIT_SLOT_END");
		if (type & WL_MSCH_RC_FLAGS_PRE_ONFIRE_DONE)
			MSCH_EVENTS_PRINTF(mschbufp, " PRE_ONFIRE_DONE");
	}
	MSCH_EVENTS_PRINTF(mschbufp, "\n");

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "pre_start_time: %s",
		wl_msch_display_time(p->pre_start_time_h, p->pre_start_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", start_time: %s",
		wl_msch_display_time(p->start_time_h, p->start_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", end_time: %s\n",
		wl_msch_display_time(p->end_time_h, p->end_time_l));

	if (p->p_timeslot && (p->timeslot_ptr == 0))
		MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "<%s timeslot>: 0x%08x\n",
			title, ntoh32(p->p_timeslot));
	else
		wl_msch_timeslot_profiler_data(mschbufp, sn, title, data, p->timeslot_ptr,
			(p->timeslot_ptr == 0));
}

static void
wl_msch_chan_ctxt_profiler_data(char *mschbufp, int sp, char *data, uint16 ptr, bool empty)
{
	int sn = sp + 4;
	msch_chan_ctxt_profiler_event_data_t *p =
		(msch_chan_ctxt_profiler_event_data_t *)(data + ntoh16(ptr));
	chanspec_t c;
	char buf[CHANSPEC_STR_LEN];

	MSCH_EVENTS_SPPRINTF(mschbufp, sp, "<chan_ctxt>: ");
	if (empty) {
		MSCH_EVENTS_PRINTF(mschbufp, " null\n");
		return;
	}
	else
		MSCH_EVENTS_PRINTF3(mschbufp, "0x%08x (prev 0x%08x, next 0x%08x)\n",
			ntoh32(p->p_chan_ctxt), ntoh32(p->p_prev), ntoh32(p->p_next));

	c = (chanspec_t)ntoh16(p->chanspec);
	MSCH_EVENTS_SPPRINTF3(mschbufp, sn, "channel: %s, bf_sch_pending: %s, bf_skipped: %d\n",
		wf_chspec_ntoa(c, buf), p->bf_sch_pending? "TRUE" : "FALSE",
		ntoh32(p->bf_skipped_count));
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "bf_link: prev 0x%08x, next 0x%08x\n",
		ntoh32(p->bf_link_prev), ntoh32(p->bf_link_next));

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "onchan_time: %s",
		wl_msch_display_time(p->onchan_time_h, p->onchan_time_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", actual_onchan_dur: %s",
		wl_msch_display_time(p->actual_onchan_dur_h, p->actual_onchan_dur_l));

	MSCH_EVENTS_PRINTF1(mschbufp, ", pend_onchan_dur: %s\n",
		wl_msch_display_time(p->pend_onchan_dur_h, p->pend_onchan_dur_l));

	wl_msch_elem_list(mschbufp, sn, "req_entity", data, p->req_entity_list_ptr,
		p->req_entity_list_cnt);
	wl_msch_elem_list(mschbufp, sn, "bf_entity", data, p->bf_entity_list_ptr,
		p->bf_entity_list_cnt);
}

static void
wl_msch_req_entity_profiler_data(char *mschbufp, int sp, char *data, uint16 ptr, bool empty)
{
	int sn = sp + 4;
	msch_req_entity_profiler_event_data_t *p =
		(msch_req_entity_profiler_event_data_t *)(data + ntoh16(ptr));
	char buf[CHANSPEC_STR_LEN];
	chanspec_t c;
	uint32 flags;

	MSCH_EVENTS_SPPRINTF(mschbufp, sp, "<req_entity>: ");
	if (empty) {
		MSCH_EVENTS_PRINTF(mschbufp, " null\n");
		return;
	}
	else
		MSCH_EVENTS_PRINTF3(mschbufp, "0x%08x (prev 0x%08x, next 0x%08x)\n",
			ntoh32(p->p_req_entity), ntoh32(p->req_hdl_link_prev),
			ntoh32(p->req_hdl_link_next));

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "req_hdl: [0x%08x]\n", ntoh32(p->p_req_hdl));
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "chan_ctxt_link: prev 0x%08x, next 0x%08x\n",
		ntoh32(p->chan_ctxt_link_prev), ntoh32(p->chan_ctxt_link_next));
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "rt_specific_link: prev 0x%08x, next 0x%08x\n",
		ntoh32(p->rt_specific_link_prev), ntoh32(p->rt_specific_link_next));
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "start_fixed_link: prev 0x%08x, next 0x%08x\n",
		ntoh32(p->start_fixed_link_prev), ntoh32(p->start_fixed_link_next));
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "both_flex_list: prev 0x%08x, next 0x%08x\n",
		ntoh32(p->both_flex_list_prev), ntoh32(p->both_flex_list_next));

	c = (chanspec_t)ntoh16(p->chanspec);
	MSCH_EVENTS_SPPRINTF4(mschbufp, sn, "channel: %s, onchan Id %d, current chan Id %d, "
		"priority %d", wf_chspec_ntoa(c, buf), ntoh16(p->onchan_chn_idx),
		ntoh16(p->cur_chn_idx), ntoh16(p->priority));
	flags = ntoh32(p->flags);
	if (flags & WL_MSCH_ENTITY_FLAG_MULTI_INSTANCE)
		MSCH_EVENTS_PRINTF(mschbufp, "MULTI_INSTANCE\n");
	else
		MSCH_EVENTS_PRINTF(mschbufp, "\n");

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "actual_start_time: %s, ",
		wl_msch_display_time(p->actual_start_time_h, p->actual_start_time_l));
	MSCH_EVENTS_PRINTF1(mschbufp, "curts_fire_time: %s, ",
		wl_msch_display_time(p->curts_fire_time_h, p->curts_fire_time_l));
	MSCH_EVENTS_PRINTF1(mschbufp, "bf_last_serv_time: %s\n",
		wl_msch_display_time(p->bf_last_serv_time_h, p->bf_last_serv_time_l));

	wl_msch_req_timing_profiler_data(mschbufp, sn, "current", data, p->cur_slot_ptr,
		(p->cur_slot_ptr == 0));
	wl_msch_req_timing_profiler_data(mschbufp, sn, "pending", data, p->pend_slot_ptr,
		(p->pend_slot_ptr == 0));

	if (p->p_chan_ctxt && (p->chan_ctxt_ptr == 0))
		MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "<chan_ctxt>: 0x%08x\n",
			ntoh32(p->p_chan_ctxt));
	else
		wl_msch_chan_ctxt_profiler_data(mschbufp, sn, data, p->chan_ctxt_ptr,
			(p->chan_ctxt_ptr == 0));
}

static void
wl_msch_req_handle_profiler_data(char *mschbufp, int sp, char *data, uint16 ptr, bool empty)
{
	int sn = sp + 4;
	msch_req_handle_profiler_event_data_t *p =
		(msch_req_handle_profiler_event_data_t *)(data + ntoh16(ptr));
	uint32 flags;

	MSCH_EVENTS_SPPRINTF(mschbufp, sp, "<req_handle>: ");
	if (empty) {
		MSCH_EVENTS_PRINTF(mschbufp, " null\n");
		return;
	}
	else
		MSCH_EVENTS_PRINTF3(mschbufp, "0x%08x (prev 0x%08x, next 0x%08x)\n",
			ntoh32(p->p_req_handle), ntoh32(p->p_prev), ntoh32(p->p_next));

	wl_msch_elem_list(mschbufp, sn, "req_entity", data, p->req_entity_list_ptr,
		p->req_entity_list_cnt);
	MSCH_EVENTS_SPPRINTF2(mschbufp, sn, "cb_func: [0x%08x], cb_func: [0x%08x]",
		ntoh32(p->cb_func), ntoh32(p->cb_ctxt));
	flags = ntoh32(p->flags);
	if (flags & WL_MSCH_REQ_HDL_FLAGS_NEW_REQ)
		MSCH_EVENTS_PRINTF(mschbufp, ", NEW_REQ");
	MSCH_EVENTS_PRINTF(mschbufp, "\n");

	MSCH_EVENTS_SPPRINTF1(mschbufp, sn, "req_time: %s\n",
		wl_msch_display_time(p->req_time_h, p->req_time_l));

	wl_msch_req_param_profiler_data(mschbufp, sn, data, p->req_param_ptr);

	MSCH_EVENTS_SPPRINTF3(mschbufp, sn, "chan_cnt: %d, chan idx %d, last chan idx %d\n",
		ntoh16(p->chan_cnt), ntoh16(p->chan_idx), ntoh16(p->last_chan_idx));
	wl_msch_chanspec_list(mschbufp, sn, data, p->chanspec_list, p->chanspec_cnt);
}

static void
wl_msch_profiler_profiler_data(char *mschbufp, int sp, char *data, uint16 ptr)
{
	msch_profiler_profiler_event_data_t *p =
		(msch_profiler_profiler_event_data_t *)(data + ntoh16(ptr));
	uint32 flags;

	MSCH_EVENTS_SPPRINTF4(mschbufp, sp, "free list: req_hdl 0x%08x, req_entity 0x%08x,"
		" chan_ctxt 0x%08x, chanspec 0x%08x\n",
		ntoh32(p->free_req_hdl_list), ntoh32(p->free_req_entity_list),
		ntoh32(p->free_chan_ctxt_list), ntoh32(p->free_chanspec_list));

	MSCH_EVENTS_SPPRINTF5(mschbufp, sp, "alloc count: chanspec %d, req_entity %d, req_hdl %d, "
		"chan_ctxt %d, timeslot %d\n",
		ntoh16(p->msch_chanspec_alloc_cnt), ntoh16(p->msch_req_entity_alloc_cnt),
		ntoh16(p->msch_req_hdl_alloc_cnt), ntoh16(p->msch_chan_ctxt_alloc_cnt),
		ntoh16(p->msch_timeslot_alloc_cnt));

	wl_msch_elem_list(mschbufp, sp, "req_hdl", data, p->msch_req_hdl_list_ptr,
		p->msch_req_hdl_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "chan_ctxt", data, p->msch_chan_ctxt_list_ptr,
		p->msch_chan_ctxt_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "req_timing", data, p->msch_req_timing_list_ptr,
		p->msch_req_timing_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "start_fixed", data, p->msch_start_fixed_list_ptr,
		p->msch_start_fixed_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "both_flex_req_entity", data,
		p->msch_both_flex_req_entity_list_ptr,
		p->msch_both_flex_req_entity_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "start_flex", data, p->msch_start_flex_list_ptr,
		p->msch_start_flex_list_cnt);
	wl_msch_elem_list(mschbufp, sp, "both_flex", data, p->msch_both_flex_list_ptr,
		p->msch_both_flex_list_cnt);

	if (p->p_cur_msch_timeslot && (p->cur_msch_timeslot_ptr == 0))
		MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "<cur_msch timeslot>: 0x%08x\n",
			ntoh32(p->p_cur_msch_timeslot));
	else
		wl_msch_timeslot_profiler_data(mschbufp, sp, "cur_msch", data,
			p->cur_msch_timeslot_ptr, (p->cur_msch_timeslot_ptr == 0));

	if (p->p_next_timeslot && (p->next_timeslot_ptr == 0))
		MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "<next timeslot>: 0x%08x\n",
			ntoh32(p->p_next_timeslot));
	else
		wl_msch_timeslot_profiler_data(mschbufp, sp, "next", data,
			p->next_timeslot_ptr, (p->next_timeslot_ptr == 0));

	MSCH_EVENTS_SPPRINTF1(mschbufp, sp, "ts_id: %d, ", ntoh32(p->ts_id));
	flags = ntoh32(p->flags);
	if (flags & WL_MSCH_STATE_IN_TIEMR_CTXT)
		MSCH_EVENTS_PRINTF(mschbufp, "IN_TIEMR_CTXT, ");
	if (flags & WL_MSCH_STATE_SCHD_PENDING)
		MSCH_EVENTS_PRINTF(mschbufp, "SCHD_PENDING, ");
	MSCH_EVENTS_PRINTF2(mschbufp, "slotskip_flags: %d, cur_armed_timeslot: 0x%08x\n",
		ntoh32(p->slotskip_flag), ntoh32(p->cur_armed_timeslot));
	MSCH_EVENTS_SPPRINTF3(mschbufp, sp, "flex_list_cnt: %d, service_interval: %d, "
		"max_lo_prio_interval: %d\n",
		ntoh16(p->flex_list_cnt), ntoh32(p->service_interval),
		ntoh32(p->max_lo_prio_interval));
}

static void _msch_dump_data(wlc_msch_profiler_info_t *profiler_info, char *data, int type)
{
	char *mschbufp = profiler_info->cmn->mschbufp;
	uint64 t = 0, tt = 0;
	uint32 s = 0, ss = 0;
	uint16 wlc_index;

	wlc_index = (uint16)profiler_info->wlc->pub->unit;
	type &= WL_MSCH_PROFILER_TYPE_MASK;

	if (type <= WL_MSCH_PROFILER_PROFILE_END) {
		msch_profiler_event_data_t *pevent = (msch_profiler_event_data_t *)data;
		tt = ((uint64)(ntoh32(pevent->time_hi)) << 32) | ntoh32(pevent->time_lo);
		wl_msch_us_to_sec(pevent->time_hi, pevent->time_lo, &s, &ss);
	}

	if (profiler_info->cmn->lastMessages && (type != WL_MSCH_PROFILER_MESSAGE)) {
		MSCH_EVENTS_PRINTF(mschbufp, "\n");
		profiler_info->cmn->lastMessages = FALSE;
	}

	switch (type) {
	case WL_MSCH_PROFILER_START:
		MSCH_EVENTS_PRINTF2(mschbufp, "\n%06d.%06d START\n", s, ss);
		break;

	case WL_MSCH_PROFILER_EXIT:
		MSCH_EVENTS_PRINTF2(mschbufp, "\n%06d.%06d EXIT\n", s, ss);
		break;

	case WL_MSCH_PROFILER_REQ:
	{
		msch_req_profiler_event_data_t *p = (msch_req_profiler_event_data_t *)data;
		MSCH_EVENTS_PRINTF(mschbufp, "\n===============================\n");
		MSCH_EVENTS_PRINTF3(mschbufp, "%06d.%06d [wl%d] REGISTER:\n", s, ss, wlc_index);
		wl_msch_req_param_profiler_data(mschbufp, 4, data, p->req_param_ptr);
		wl_msch_chanspec_list(mschbufp, 4, data, p->chanspec_ptr, p->chanspec_cnt);
		MSCH_EVENTS_PRINTF(mschbufp, "===============================\n\n");
	}
		break;

	case WL_MSCH_PROFILER_CALLBACK:
	{
		msch_callback_profiler_event_data_t *p =
			(msch_callback_profiler_event_data_t *)data;
		char buf[CHANSPEC_STR_LEN];
		chanspec_t chanspec;
		uint16 cbtype;

		MSCH_EVENTS_PRINTF3(mschbufp, "%06d.%06d [wl%d] CALLBACK: ", s, ss, wlc_index);
		chanspec = (chanspec_t)ntoh16(p->chanspec);
		MSCH_EVENTS_PRINTF2(mschbufp, "req_hdl[0x%08x], channel %s --",
			ntoh32(p->p_req_hdl), wf_chspec_ntoa(chanspec, buf));

		cbtype = ntoh16(p->type);
		if (cbtype & WL_MSCH_CT_ON_CHAN)
			MSCH_EVENTS_PRINTF(mschbufp, " ON_CHAN");
		if (cbtype & WL_MSCH_CT_OFF_CHAN)
			MSCH_EVENTS_PRINTF(mschbufp, " OFF_CHAN");
		if (cbtype & WL_MSCH_CT_REQ_START)
			MSCH_EVENTS_PRINTF(mschbufp, " REQ_START");
		if (cbtype & WL_MSCH_CT_REQ_END)
			MSCH_EVENTS_PRINTF(mschbufp, " REQ_END");
		if (cbtype & WL_MSCH_CT_SLOT_START)
			MSCH_EVENTS_PRINTF(mschbufp, " SLOT_START");
		if (cbtype & WL_MSCH_CT_SLOT_SKIP)
			MSCH_EVENTS_PRINTF(mschbufp, " SLOT_SKIP");
		if (cbtype & WL_MSCH_CT_SLOT_END)
			MSCH_EVENTS_PRINTF(mschbufp, " SLOT_END");
		if (cbtype & WL_MSCH_CT_OFF_CHAN_DONE)
			MSCH_EVENTS_PRINTF(mschbufp, " OFF_CHAN_DONE");
		if (cbtype & WL_MSCH_CT_PARTIAL)
			MSCH_EVENTS_PRINTF(mschbufp, " PARTIAL");
		if (cbtype & WL_MSCH_CT_PRE_ONCHAN)
			MSCH_EVENTS_PRINTF(mschbufp, " PRE_ONCHAN");
		if (cbtype & WL_MSCH_CT_PRE_REQ_START)
			MSCH_EVENTS_PRINTF(mschbufp, " PRE_REQ_START");

		if (cbtype & (WL_MSCH_CT_ON_CHAN | WL_MSCH_CT_SLOT_SKIP)) {
			MSCH_EVENTS_PRINTF(mschbufp, "\n    ");
			if (cbtype & WL_MSCH_CT_ON_CHAN) {
				MSCH_EVENTS_PRINTF3(mschbufp, "ID %d onchan idx %d seq_start %s ",
					ntoh32(p->timeslot_id), ntoh32(p->onchan_idx),
					wl_msch_display_time(p->cur_chan_seq_start_time_h,
					p->cur_chan_seq_start_time_l));
			}
			t = ((uint64)(ntoh32(p->start_time_h)) << 32) |
				ntoh32(p->start_time_l);
			MSCH_EVENTS_PRINTF1(mschbufp, "start %s ",
				wl_msch_display_time(p->start_time_h,
				p->start_time_l));
			tt = ((uint64)(ntoh32(p->end_time_h)) << 32) | ntoh32(p->end_time_l);
			MSCH_EVENTS_PRINTF2(mschbufp, "end %s duration %d",
				wl_msch_display_time(p->end_time_h, p->end_time_l),
				(p->end_time_h == 0xffffffff && p->end_time_l == 0xffffffff)?
				-1 : (int)(tt - t));
		}

		if (cbtype & WL_MSCH_CT_REQ_START) {
			profiler_info->req_start = 1;
			profiler_info->req_start_time = tt;
		} else if (cbtype & WL_MSCH_CT_REQ_END) {
			if (profiler_info->req_start) {
				MSCH_EVENTS_PRINTF1(mschbufp, " : REQ duration %d",
					(uint32)(tt - profiler_info->req_start_time));
				profiler_info->req_start = 0;
			}
		}

		if (cbtype & WL_MSCH_CT_SLOT_START) {
			profiler_info->solt_chanspec = p->chanspec;
			profiler_info->solt_start_time = tt;
		} else if (cbtype & WL_MSCH_CT_SLOT_END) {
			if (p->chanspec == profiler_info->solt_chanspec) {
				MSCH_EVENTS_PRINTF1(mschbufp, " : SLOT duration %d",
					(uint32)(tt - profiler_info->solt_start_time));
				profiler_info->solt_chanspec = 0;
			}
		}
		MSCH_EVENTS_PRINTF(mschbufp, "\n");
	}
		break;

	case WL_MSCH_PROFILER_MESSAGE:
	{
		msch_message_profiler_event_data_t *p = (msch_message_profiler_event_data_t *)data;
		MSCH_EVENTS_PRINTF4(mschbufp, "%06d.%06d [wl%d]: %s", s, ss, wlc_index, p->message);
		profiler_info->cmn->lastMessages = TRUE;
		break;
	}

	case WL_MSCH_PROFILER_PROFILE_START:
		profiler_info->profiler_start_time = tt;
		MSCH_EVENTS_PRINTF(mschbufp, "-------------------------------\n");
		MSCH_EVENTS_PRINTF3(mschbufp, "%06d.%06d [wl%d] PROFILE DATA:\n", s, ss, wlc_index);
		wl_msch_profiler_profiler_data(mschbufp, 4, data, 0);
		break;

	case WL_MSCH_PROFILER_PROFILE_END:
		MSCH_EVENTS_PRINTF4(mschbufp, "%06d.%06d [wl%d] PROFILE END: take time %d\n", s, ss,
			wlc_index, (uint32)(tt - profiler_info->profiler_start_time));
		MSCH_EVENTS_PRINTF(mschbufp, "-------------------------------\n\n");
		break;

	case WL_MSCH_PROFILER_REQ_HANDLE:
		wl_msch_req_handle_profiler_data(mschbufp, 4, data, 0, FALSE);
		break;

	case WL_MSCH_PROFILER_REQ_ENTITY:
		wl_msch_req_entity_profiler_data(mschbufp, 4, data, 0, FALSE);
		break;

	case WL_MSCH_PROFILER_CHAN_CTXT:
		wl_msch_chan_ctxt_profiler_data(mschbufp, 4, data, 0, FALSE);
		break;

	case WL_MSCH_PROFILER_REQ_TIMING:
		wl_msch_req_timing_profiler_data(mschbufp, 4, "msch", data, 0, FALSE);
		break;

	default:
		MSCH_EVENTS_PRINTF2(mschbufp, "[wl%d] ERROR: unsupported EVENT reason code:%d; ",
			wlc_index, type);
		break;
	}
}
#endif /* !DONGLEBUILD */

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
static void
_msch_event_data_flush(wlc_msch_profiler_info_t *profiler_info)
{
	wlc_msch_profiler_cmn_t *cmn = profiler_info->cmn;

	if (cmn->event_timer_start) {
		cmn->event_timer_start = FALSE;
		wlc_hrt_del_timeout(cmn->event_timer);
	}

	if (cmn->edata_len > 0 && (cmn->dump_enable & MSCH_EVENT_BIT)) {
		wlc_info_t *wlc = profiler_info->wlc;
		wlc_bsscfg_t *bsscfg = wlc->cfg;
		if (bsscfg) {
			uint type = profiler_info->wl_index | WL_MSCH_PROFILER_EVENT_LOG;
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_MSCH, &bsscfg->BSSID,
				WLC_E_STATUS_SUCCESS, type, 0, cmn->edata, cmn->edata_len);
		}
	}
	cmn->edata_len = 0;
	cmn->event_data_flush = FALSE;
}

static void
_msch_event_data_timer(void *arg)
{
	wlc_msch_profiler_info_t *profiler_info = (wlc_msch_profiler_info_t *)arg;

	profiler_info->cmn->event_timer_start = FALSE;
	_msch_event_data_flush(profiler_info);
}
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

static void
_msch_write_profiler_tlv(wlc_msch_profiler_info_t *profiler_info, uint type, int len)
{
	wlc_msch_profiler_cmn_t *cmn;
#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
	bool is_even_log = (type == WL_MSCH_PROFILER_EVENT_LOG);
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

	ASSERT(profiler_info);

	cmn = profiler_info->cmn;
	ASSERT(cmn->edata);

#ifdef MSCH_PROFILER
	if (MSCH_PROFILER_ENAB(profiler_info) &&
		(cmn->dump_status & MSCH_PROFILE_BIT)) {
		wlc_msch_profiler_t *profiler = cmn->profiler;
		uint8 *edata = cmn->edata;
		uint8 *pbuf;
		msch_collect_tlv_t *p;
		int size, copysize;

		ASSERT(profiler);

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
		if (is_even_log) {
			edata += cmn->edata_len;
		}
#endif /* EVENT_LOG_COMPILE */

		size = WL_MSCH_PROFILE_HEAD_SIZE + len;
		profiler->write_size += ROUNDUP(size);

		while (profiler->write_size > profiler->total_size) {
			pbuf = profiler->buffer + profiler->start_ptr;
			p = (msch_collect_tlv_t *)pbuf;
			copysize = ROUNDUP(WL_MSCH_PROFILE_HEAD_SIZE + (int)(p->size));
			profiler->write_size -= copysize;
			profiler->start_ptr += copysize;
			if (profiler->start_ptr >= profiler->total_size) {
				profiler->start_ptr -= profiler->total_size;
			}
		}

		pbuf = profiler->buffer + profiler->write_ptr;
		p = (msch_collect_tlv_t *)pbuf;

		p->type = (uint16)(type | profiler_info->wl_index);
		p->size = (uint16)len;

		copysize = profiler->total_size - profiler->write_ptr - WL_MSCH_PROFILE_HEAD_SIZE;
		if (copysize > len)
			copysize = len;

		if (copysize > 0)
			bcopy(edata, p->value, copysize);
		if (copysize < len) {
			size = len - copysize;
			bcopy(edata + copysize, profiler->buffer, size);
			profiler->write_ptr = ROUNDUP(size);
		} else {
			profiler->write_ptr += ROUNDUP(size);
			if (profiler->write_ptr == profiler->total_size) {
				profiler->write_ptr = 0;
			}
		}
	}
#endif /* MSCH_PROFILER */

#ifdef DONGLEBUILD
#ifdef EVENT_LOG_COMPILE
	if (cmn->dump_status & MSCH_EVENT_LOG_BIT) {
		if (is_even_log) {
			EVENT_LOG_BUFFER(EVENT_LOG_TAG_MSCHPROFILE, cmn->edata +
				cmn->edata_len, len);
		} else {
			msch_collect_tlv_t *ptlv = cmn->ptlv;
			ptlv->type = hton16(type | profiler_info->wl_index);
			ptlv->size = hton16(len);
			EVENT_LOG_BUFFER(EVENT_LOG_TAG_MSCHPROFILE_TLV, (uint8 *)ptlv,
				len + WL_MSCH_PROFILE_HEAD_SIZE);
		}
	}
#endif /* EVENT_LOG_COMPILE */
	if (cmn->dump_status & MSCH_EVENT_BIT) {
#ifdef EVENT_LOG_COMPILE
		if (is_even_log) {
			cmn->edata_len += len;
			if (cmn->event_data_flush) {
				_msch_event_data_flush(profiler_info);
			} else if (!cmn->event_timer_start) {
				cmn->event_timer_start = TRUE;
				wlc_hrt_add_timeout(cmn->event_timer, MSCH_EVENT_LOG_WAIT_TIME,
					_msch_event_data_timer, (void *)profiler_info);
			}
			return;
		}
#else /* EVENT_LOG_COMPILE */
		if (MSCH_PROFILER_ENAB(profiler_info))
#endif /* EVENT_LOG_COMPILE */
		{
			wlc_info_t *wlc = profiler_info->wlc;
			wlc_bsscfg_t *bsscfg = wlc->cfg;
			if (bsscfg) {
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_MSCH, &bsscfg->BSSID,
					WLC_E_STATUS_SUCCESS, (type | profiler_info->wl_index),
						0, cmn->edata, len);
			}
		}
	}
#else /* DONGLEBUILD */
	if (cmn->dump_status & (MSCH_EVENT_LOG_BIT | MSCH_EVENT_BIT)) {
		_msch_dump_data(profiler_info, (char*)cmn->edata, type);
	}
#endif /* DONGLEBUILD */
}

static uint16
_msch_chanspec_list(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr, uint16 *list_cnt,
	chanspec_t *chanspec_list, int chanspec_cnt)
{
	uint16 i, p = *ptr, cnt = (uint16)chanspec_cnt;
	uint16 *list;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);
	ASSERT(chanspec_list || chanspec_cnt == 0);

	*list_cnt = hton16(cnt);
	if (cnt == 0)
		return 0;

	list = (uint16 *)(profiler_info->cmn->edata + p);
	for (i = 0; i < cnt; i++) {
		list[i] = hton16((uint16)chanspec_list[i]);
	}

	*ptr = (uint16)(p + ROUNDUP(cnt * sizeof(uint16)));
	return hton16(p);
}

static uint16
_msch_elem_list_link(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr, uint16 *list_cnt,
	msch_list_elem_t *msch_list, int entry_offset)
{
	uint16 cnt = 0, p = *ptr;
	uint32 entry, *list;
	msch_list_elem_t *elem;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);

	list = (uint32 *)(profiler_info->cmn->edata + p);
	elem = msch_list;
	while ((elem = msch_list_iterate(elem))) {
		entry = (uint32)((uintptr)elem) - entry_offset;
		list[cnt] = hton32(entry);
		cnt++;
	}

	*list_cnt = hton16(cnt);
	if (cnt == 0)
		return 0;

	*ptr = (uint16)(p + ROUNDUP(cnt * sizeof(uint32)));
	return hton16(p);
}

static uint16
_msch_req_param_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	wlc_msch_req_param_t *req_param)
{
	msch_req_param_profiler_event_data_t *edata;
	uint16 p = *ptr;
	int len = OFFSETOF(msch_req_param_profiler_event_data_t, flex);

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);
	ASSERT(req_param);

	edata = (msch_req_param_profiler_event_data_t *)(profiler_info->cmn->edata + p);

	edata->flags = hton16((uint16)req_param->flags);
	edata->req_type = (uint8)req_param->req_type;
	edata->priority = (uint8)req_param->priority;

	edata->start_time_l = hton32(req_param->start_time_l);
	edata->start_time_h = hton32(req_param->start_time_h);
	edata->duration = hton32(req_param->duration);
	edata->interval = hton32(req_param->interval);

	if (req_param->req_type == MSCH_RT_DUR_FLEX) {
		edata->flex.dur_flex = hton32(req_param->flex.dur_flex);
		len += sizeof(uint32);
	} else if (MSCH_BOTH_FLEX(req_param->req_type)) {
		edata->flex.bf.min_dur = hton32(req_param->flex.bf.min_dur);
		edata->flex.bf.max_away_dur = hton32(req_param->flex.bf.max_away_dur);
		edata->flex.bf.hi_prio_time_l = hton32(req_param->flex.bf.hi_prio_time_l);
		edata->flex.bf.hi_prio_time_h = hton32(req_param->flex.bf.hi_prio_time_h);
		edata->flex.bf.hi_prio_interval = hton32(req_param->flex.bf.hi_prio_interval);
		len = sizeof(msch_req_param_profiler_event_data_t);
	}

	*ptr = (uint16)(p + ROUNDUP(len));

	return hton16(p);
}

static uint16
_msch_timeslot_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	msch_timeslot_t *timeslot)
{
	msch_timeslot_profiler_event_data_t *edata;
	uint16 p = *ptr;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);

	if (!timeslot)
		return 0;

	edata = (msch_timeslot_profiler_event_data_t *)(profiler_info->cmn->edata + p);
	*ptr = (uint16)(p + ROUNDUP(sizeof(msch_timeslot_profiler_event_data_t)));

	edata->p_timeslot = hton32((uint32)((uintptr)timeslot));
	edata->timeslot_id = hton32(timeslot->timeslot_id);

	edata->pre_start_time_h = hton32((uint32)(timeslot->pre_start_time >> 32));
	edata->pre_start_time_l = hton32((uint32)(timeslot->pre_start_time));
	edata->end_time_h = hton32((uint32)(timeslot->end_time >> 32));
	edata->end_time_l = hton32((uint32)(timeslot->end_time));
	edata->sch_dur_h = hton32((uint32)(timeslot->sch_dur >> 32));
	edata->sch_dur_l = hton32((uint32)(timeslot->sch_dur));
	edata->fire_time_h = hton32((uint32)(timeslot->fire_time >> 32));
	edata->fire_time_l = hton32((uint32)(timeslot->fire_time));
	edata->p_chan_ctxt = hton32((uint32)((uintptr)timeslot->chan_ctxt));
	edata->state = hton32((uint32)timeslot->state);

	return hton16(p);
}

static uint16
_msch_req_timing_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	msch_req_timing_t *req_timing, bool allowEmpty)
{
	msch_req_timing_profiler_event_data_t *edata;
	uint16 p = *ptr;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);

	if (!req_timing || !(allowEmpty || req_timing->flags ||
		req_timing->pre_start_time || req_timing->start_time ||
		req_timing->end_time || req_timing->timeslot ||
		req_timing->link.prev != NULL))
		return 0;

	edata = (msch_req_timing_profiler_event_data_t *)(profiler_info->cmn->edata + p);
	*ptr = (uint16)(p + ROUNDUP(sizeof(msch_req_timing_profiler_event_data_t)));

	edata->p_req_timing = hton32((uint32)((uintptr)req_timing));
	edata->p_prev = hton32((uint32)((uintptr)req_timing->link.prev));
	edata->p_next = hton32((uint32)((uintptr)req_timing->link.next));

	edata->flags = hton16((uint16)req_timing->flags);

	edata->pre_start_time_h = hton32((uint32)(req_timing->pre_start_time >> 32));
	edata->pre_start_time_l = hton32((uint32)(req_timing->pre_start_time));
	edata->start_time_h = hton32((uint32)(req_timing->start_time >> 32));
	edata->start_time_l = hton32((uint32)(req_timing->start_time));
	edata->end_time_h = hton32((uint32)(req_timing->end_time >> 32));
	edata->end_time_l = hton32((uint32)(req_timing->end_time));

	edata->p_timeslot = hton32((uint32)((uintptr)req_timing->timeslot));
	if (req_timing->timeslot) {
		edata->timeslot_ptr = _msch_timeslot_profiler_data(profiler_info, ptr,
			req_timing->timeslot);
	}
	else
		edata->timeslot_ptr = 0;

	return hton16(p);
}

static uint16
_msch_chan_ctxt_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	msch_chan_ctxt_t *chan_ctxt)
{
	msch_chan_ctxt_profiler_event_data_t *edata;
	uint16 p = *ptr;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);

	if (!chan_ctxt)
		return 0;

	edata = (msch_chan_ctxt_profiler_event_data_t *)(profiler_info->cmn->edata + p);
	*ptr = (uint16)(p + ROUNDUP(sizeof(msch_chan_ctxt_profiler_event_data_t)));

	edata->p_chan_ctxt = hton32((uint32)((uintptr)chan_ctxt));
	edata->p_prev = hton32((uint32)((uintptr)chan_ctxt->link.prev));
	edata->p_next = hton32((uint32)((uintptr)chan_ctxt->link.next));

	edata->chanspec = hton16((uint16)chan_ctxt->chanspec);
	edata->bf_sch_pending = hton16((uint16)chan_ctxt->bf_sch_pending);
	edata->bf_skipped_count = hton32((uint32)chan_ctxt->bf_skipped_count);

	edata->bf_link_prev = (uint32)((uintptr)chan_ctxt->bf_link.prev);
	if (edata->bf_link_prev) {
		edata->bf_link_prev -= OFFSETOF(msch_chan_ctxt_t, bf_link);
		edata->bf_link_prev = hton32(edata->bf_link_prev);
	}

	edata->bf_link_next = (uint32)((uintptr)chan_ctxt->bf_link.next);
	if (edata->bf_link_next) {
		edata->bf_link_next -= OFFSETOF(msch_chan_ctxt_t, bf_link);
		edata->bf_link_next = hton32(edata->bf_link_next);
	}

	edata->onchan_time_h = hton32((uint32)(chan_ctxt->onchan_time >> 32));
	edata->onchan_time_l = hton32((uint32)(chan_ctxt->onchan_time));
	edata->actual_onchan_dur_h = hton32((uint32)(chan_ctxt->actual_onchan_dur >> 32));
	edata->actual_onchan_dur_l = hton32((uint32)(chan_ctxt->actual_onchan_dur));
	edata->pend_onchan_dur_h = hton32((uint32)(chan_ctxt->pend_onchan_dur >> 32));
	edata->pend_onchan_dur_l = hton32((uint32)(chan_ctxt->pend_onchan_dur));

	edata->req_entity_list_ptr = _msch_elem_list_link(profiler_info, ptr,
		&edata->req_entity_list_cnt, &chan_ctxt->req_entity_list,
		OFFSETOF(msch_req_entity_t, chan_ctxt_link));
	edata->bf_entity_list_ptr = _msch_elem_list_link(profiler_info, ptr,
		&edata->bf_entity_list_cnt, &chan_ctxt->bf_entity_list,
		OFFSETOF(msch_req_entity_t, rt_specific_link));

	return hton16(p);
}

static uint16
_msch_req_entity_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	msch_req_entity_t *req_entity)
{
	wlc_msch_info_t *msch_info;
	msch_req_entity_profiler_event_data_t *edata;
	uint16 p = *ptr;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);
	ASSERT(req_entity);

	msch_info = profiler_info->msch_info;
	edata = (msch_req_entity_profiler_event_data_t *)(profiler_info->cmn->edata + p);
	*ptr = (uint16)(p + ROUNDUP(sizeof(msch_req_entity_profiler_event_data_t)));

	edata->p_req_entity = hton32((uint32)((uintptr)req_entity));
	edata->req_hdl_link_prev = hton32((uint32)((uintptr)req_entity->req_hdl_link.prev));
	edata->req_hdl_link_next = hton32((uint32)((uintptr)req_entity->req_hdl_link.next));

	edata->chan_ctxt_link_prev = (uint32)((uintptr)req_entity->chan_ctxt_link.prev);
	if (edata->chan_ctxt_link_prev) {
		edata->chan_ctxt_link_prev -= OFFSETOF(msch_req_entity_t, chan_ctxt_link);
		edata->chan_ctxt_link_prev = hton32(edata->chan_ctxt_link_prev);
	}

	edata->chan_ctxt_link_next = (uint32)((uintptr)req_entity->chan_ctxt_link.next);
	if (edata->chan_ctxt_link_next) {
		edata->chan_ctxt_link_next -= OFFSETOF(msch_req_entity_t, chan_ctxt_link);
		edata->chan_ctxt_link_next = hton32(edata->chan_ctxt_link_next);
	}

	edata->rt_specific_link_prev = (uint32)((uintptr)req_entity->rt_specific_link.prev);
	if (edata->rt_specific_link_prev) {
		edata->rt_specific_link_prev -= OFFSETOF(msch_req_entity_t, rt_specific_link);
		edata->rt_specific_link_prev = hton32(edata->rt_specific_link_prev);
	}

	edata->rt_specific_link_next = (uint32)((uintptr)req_entity->rt_specific_link.next);
	if (edata->rt_specific_link_next) {
		edata->rt_specific_link_next -= OFFSETOF(msch_req_entity_t, rt_specific_link);
		edata->rt_specific_link_next = hton32(edata->rt_specific_link_next);
	}

	edata->start_fixed_link_prev = (uint32)((uintptr)req_entity->start_fixed_link.prev);
	if (edata->start_fixed_link_prev) {
		edata->start_fixed_link_prev -= OFFSETOF(msch_req_entity_t, start_fixed_link);
		edata->start_fixed_link_prev = hton32(edata->start_fixed_link_prev);
	}

	edata->start_fixed_link_next = (uint32)((uintptr)req_entity->start_fixed_link.next);
	if (edata->start_fixed_link_next) {
		edata->start_fixed_link_next -= OFFSETOF(msch_req_entity_t, start_fixed_link);
		edata->start_fixed_link_next = hton32(edata->start_fixed_link_next);
	}

	edata->both_flex_list_prev = (uint32)((uintptr)req_entity->both_flex_list.prev);
	if (edata->both_flex_list_prev) {
		edata->both_flex_list_prev -= OFFSETOF(msch_req_entity_t, both_flex_list);
		edata->both_flex_list_prev = hton32(edata->both_flex_list_prev);
	}

	edata->both_flex_list_next = (uint32)((uintptr)req_entity->both_flex_list.next);
	if (edata->both_flex_list_next) {
		edata->both_flex_list_next -= OFFSETOF(msch_req_entity_t, both_flex_list);
		edata->both_flex_list_next = hton32(edata->both_flex_list_next);
	}

	edata->chanspec = hton16((uint16)req_entity->chanspec);
	edata->priority = hton16((uint16)req_entity->priority);

	edata->cur_slot_ptr = _msch_req_timing_profiler_data(profiler_info, ptr,
		&req_entity->cur_slot, FALSE);
	edata->pend_slot_ptr = _msch_req_timing_profiler_data(profiler_info, ptr,
		&req_entity->pend_slot, FALSE);

	if (req_entity->chan_ctxt &&
		!_msch_find_entry(&msch_info->msch_chan_ctxt_list, req_entity->chan_ctxt)) {
		edata->chan_ctxt_ptr = _msch_chan_ctxt_profiler_data(profiler_info, ptr,
			req_entity->chan_ctxt);
	}
	else
		edata->chan_ctxt_ptr = 0;

	edata->p_chan_ctxt = hton32((uint32)((uintptr)req_entity->chan_ctxt));
	edata->p_req_hdl = hton32((uint32)((uintptr)req_entity->req_hdl));

	edata->bf_last_serv_time_h = hton32((uint32)(req_entity->last_serv_time >> 32));
	edata->bf_last_serv_time_l = hton32((uint32)(req_entity->last_serv_time));

	edata->onchan_chn_idx = hton16(req_entity->onchan_chn_idx);
	edata->cur_chn_idx = hton16(req_entity->cur_chn_idx);
	edata->flags = hton32(req_entity->flags);

	edata->actual_start_time_h = hton32((uint32)(req_entity->actual_start_time >> 32));
	edata->actual_start_time_l = hton32((uint32)(req_entity->actual_start_time));

	edata->curts_fire_time_h = hton32((uint32)(req_entity->curts_fire_time >> 32));
	edata->curts_fire_time_l = hton32((uint32)(req_entity->curts_fire_time));

	return hton16(p);
}

static uint16
_msch_req_handle_profiler_data(wlc_msch_profiler_info_t *profiler_info, uint16 *ptr,
	wlc_msch_req_handle_t *req_handle)
{
	msch_req_handle_profiler_event_data_t *edata;
	uint16 p = *ptr;

	ASSERT(profiler_info);
	ASSERT(profiler_info->cmn->edata);
	ASSERT(req_handle);

	edata = (msch_req_handle_profiler_event_data_t *)(profiler_info->cmn->edata + p);
	*ptr = (uint16)(p + ROUNDUP(sizeof(msch_req_handle_profiler_event_data_t)));

	edata->p_req_handle = hton32((uint32)((uintptr)req_handle));
	edata->p_prev = hton32((uint32)((uintptr)req_handle->link.prev));
	edata->p_next = hton32((uint32)((uintptr)req_handle->link.next));
	edata->cb_func = hton32((uint32)((uintptr)req_handle->cb_func));
	edata->cb_ctxt = hton32((uint32)((uintptr)req_handle->cb_ctxt));

	edata->req_param_ptr = _msch_req_param_profiler_data(profiler_info, ptr,
		req_handle->req_param);
	edata->req_entity_list_ptr = _msch_elem_list_link(profiler_info, ptr,
		&edata->req_entity_list_cnt, &req_handle->req_entity_list,
		OFFSETOF(msch_req_entity_t, req_hdl_link));

	edata->chan_cnt = hton16((uint16)req_handle->chan_cnt);
	edata->flags = hton32((uint32)req_handle->flags);

	edata->chanspec_list = _msch_chanspec_list(profiler_info, ptr,
		&edata->chanspec_cnt, req_handle->chn_list,
		req_handle->chn_list? req_handle->chan_cnt : 0);
	edata->chan_idx = hton16(req_handle->chan_idx);
	edata->last_chan_idx = hton16(req_handle->last_chan_idx);

	edata->req_time_h = hton32((uint32)(req_handle->req_time >> 32));
	edata->req_time_l = hton32((uint32)(req_handle->req_time));

	return hton16(p);
}

void
_msch_dump_profiler(wlc_msch_profiler_info_t *profiler_info)
{
	wlc_msch_profiler_cmn_t *cmn;
	ASSERT(profiler_info);

	cmn = profiler_info->cmn;
	cmn->dump_status = (cmn->dump_enable & cmn->dump_profiler);

	if (cmn->dump_status) {
		wlc_msch_info_t *msch_info = profiler_info->msch_info;
		uint64 cur_time = msch_current_time(msch_info);
		msch_list_elem_t *elem;
		msch_profiler_profiler_event_data_t *edata =
			(msch_profiler_profiler_event_data_t *)cmn->edata;
		uint16 len = ROUNDUP(sizeof(msch_profiler_profiler_event_data_t));

		ASSERT(cmn->edata);

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
		_msch_event_data_flush(profiler_info);
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));

		edata->free_req_hdl_list = hton32((uint32)
			((uintptr)msch_info->free_req_hdl_list.next));
		edata->free_req_entity_list = hton32((uint32)
			((uintptr)msch_info->free_req_entity_list.next));
		edata->free_chan_ctxt_list = hton32((uint32)
			((uintptr)msch_info->free_chan_ctxt_list.next));
		edata->free_chanspec_list = hton32((uint32)
			((uintptr)msch_info->free_chanspec_list.next));

		edata->p_cur_msch_timeslot = hton32((uint32)
			((uintptr)msch_info->cur_msch_timeslot));
		if (msch_info->cur_msch_timeslot) {
			edata->cur_msch_timeslot_ptr = _msch_timeslot_profiler_data(
				profiler_info, &len,
				msch_info->cur_msch_timeslot);
		}
		else
			edata->cur_msch_timeslot_ptr = 0;

		edata->p_next_timeslot = hton32((uint32)
			((uintptr)msch_info->next_timeslot));
		if (msch_info->next_timeslot) {
			edata->next_timeslot_ptr = _msch_timeslot_profiler_data(profiler_info,
				&len, msch_info->next_timeslot);
		}
		else
			edata->next_timeslot_ptr = 0;

		edata->cur_armed_timeslot = hton32((uint32)(uintptr)
			(msch_info->cur_armed_timeslot));

		edata->slotskip_flag = hton32(msch_info->slotskip_flag);
		edata->flags = hton32(msch_info->flags);
		edata->ts_id = hton32(msch_info->ts_id);
		edata->service_interval = hton32(msch_info->service_interval);
		edata->max_lo_prio_interval = hton32(msch_info->max_lo_prio_interval);
		edata->flex_list_cnt = hton16((uint16)msch_info->flex_list_cnt);

		edata->msch_chanspec_alloc_cnt =
			hton16(profiler_info->msch_chanspec_alloc_cnt);
		edata->msch_req_entity_alloc_cnt =
			hton16(profiler_info->msch_req_entity_alloc_cnt);
		edata->msch_req_hdl_alloc_cnt =
			hton16(profiler_info->msch_req_hdl_alloc_cnt);
		edata->msch_chan_ctxt_alloc_cnt =
			hton16(profiler_info->msch_chan_ctxt_alloc_cnt);
		edata->msch_timeslot_alloc_cnt =
			hton16(profiler_info->msch_timeslot_alloc_cnt);

		edata->msch_req_hdl_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_req_hdl_list_cnt, &msch_info->msch_req_hdl_list,
			OFFSETOF(wlc_msch_req_handle_t, link));
		edata->msch_chan_ctxt_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_chan_ctxt_list_cnt, &msch_info->msch_chan_ctxt_list,
			OFFSETOF(msch_chan_ctxt_t, link));
		edata->msch_req_timing_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_req_timing_list_cnt, &msch_info->msch_req_timing_list,
			OFFSETOF(msch_req_timing_t, link));
		edata->msch_start_fixed_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_start_fixed_list_cnt,
			&msch_info->msch_start_fixed_list,
			OFFSETOF(msch_req_entity_t, start_fixed_link));
		edata->msch_both_flex_req_entity_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_both_flex_req_entity_list_cnt,
			&msch_info->msch_both_flex_req_entity_list,
			OFFSETOF(msch_req_entity_t, both_flex_list));
		edata->msch_start_flex_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_start_flex_list_cnt, &msch_info->msch_start_flex_list,
			OFFSETOF(msch_req_entity_t, chan_ctxt_link));
		edata->msch_both_flex_list_ptr =
			_msch_elem_list_link(profiler_info, &len,
			&edata->msch_both_flex_list_cnt, &msch_info->msch_both_flex_list,
			OFFSETOF(msch_chan_ctxt_t, bf_link));

		_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_PROFILE_START, len);

		elem = &msch_info->msch_req_timing_list;
		while ((elem = msch_list_iterate(elem))) {
			msch_req_timing_t *req_timing;
			req_timing = LIST_ENTRY(elem, msch_req_timing_t, link);
			len = 0;
			_msch_req_timing_profiler_data(profiler_info, &len, req_timing, TRUE);
			_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_REQ_TIMING, len);
		}

		elem = &msch_info->msch_chan_ctxt_list;
		while ((elem = msch_list_iterate(elem))) {
			msch_chan_ctxt_t *chan_ctxt;
			chan_ctxt = LIST_ENTRY(elem, msch_chan_ctxt_t, link);
			len = 0;
			_msch_chan_ctxt_profiler_data(profiler_info, &len, chan_ctxt);
			_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_CHAN_CTXT, len);
		}

		elem = &msch_info->msch_req_hdl_list;
		while ((elem = msch_list_iterate(elem))) {
			msch_list_elem_t *elem_entity;
			wlc_msch_req_handle_t *req_handle;
			req_handle = LIST_ENTRY(elem, wlc_msch_req_handle_t, link);
			len = 0;
			_msch_req_handle_profiler_data(profiler_info, &len, req_handle);
			_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_REQ_HANDLE, len);

			elem_entity = &req_handle->req_entity_list;
			while ((elem_entity = msch_list_iterate(elem_entity))) {
				msch_req_entity_t *req_entity;
				req_entity = LIST_ENTRY(elem_entity, msch_req_entity_t,
					req_hdl_link);
				len = 0;
				_msch_req_entity_profiler_data(profiler_info,
					&len, req_entity);
				_msch_write_profiler_tlv(profiler_info,
					WL_MSCH_PROFILER_REQ_ENTITY, len);
			}
		}

		cur_time = msch_current_time(msch_info);
		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));

		_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_PROFILE_END,
			sizeof(msch_profiler_event_data_t));
	}
}

void
_msch_dump_register(wlc_msch_profiler_info_t *profiler_info, chanspec_t* chanspec_list,
	int chanspec_cnt, wlc_msch_req_param_t *req_param)
{
	wlc_msch_profiler_cmn_t *cmn;
	ASSERT(profiler_info);

	cmn = profiler_info->cmn;
	cmn->dump_status = (cmn->dump_enable & cmn->dump_register);

	if (cmn->dump_status) {
		wlc_msch_info_t *msch_info = profiler_info->msch_info;
		uint64 cur_time = msch_current_time(msch_info);
		msch_req_profiler_event_data_t *edata =
			(msch_req_profiler_event_data_t *)cmn->edata;
		uint16 len = ROUNDUP(sizeof(msch_req_profiler_event_data_t));

		ASSERT(cmn->edata);

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
		_msch_event_data_flush(profiler_info);
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));

		edata->chanspec_ptr = _msch_chanspec_list(profiler_info, &len,
			&edata->chanspec_cnt, chanspec_list, chanspec_cnt);
		edata->req_param_ptr = _msch_req_param_profiler_data(profiler_info, &len,
			req_param);

		_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_REQ, len);
	}
}

void
_msch_dump_callback(wlc_msch_profiler_info_t *profiler_info, wlc_msch_cb_info_t *cb_info)
{
	wlc_msch_profiler_cmn_t *cmn;
	ASSERT(profiler_info);

	cmn = profiler_info->cmn;
	cmn->dump_status = (cmn->dump_enable & cmn->dump_callback);

	if (cmn->dump_status) {
		wlc_msch_info_t *msch_info = profiler_info->msch_info;
		uint64 cur_time = msch_current_time(msch_info);
		msch_callback_profiler_event_data_t *edata =
			(msch_callback_profiler_event_data_t *)cmn->edata;
		uint16 len = ROUNDUP(sizeof(msch_callback_profiler_event_data_t));

		ASSERT(cmn->edata);

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
		_msch_event_data_flush(profiler_info);
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));
		edata->type = hton16((uint16)cb_info->type);
		edata->chanspec = hton16((uint16)cb_info->chanspec);
		edata->p_req_hdl = hton32((uint32)((uintptr)cb_info->req_hdl));

		if (cb_info->type & MSCH_CT_ON_CHAN) {
			wlc_msch_onchan_info_t *onchan =
				(wlc_msch_onchan_info_t *)cb_info->type_specific;
			ASSERT(onchan);
			edata->timeslot_id = hton32(onchan->timeslot_id);
			edata->onchan_idx = hton32(onchan->onchan_idx);
			edata->start_time_l = hton32(onchan->start_time_l);
			edata->start_time_h = hton32(onchan->start_time_h);
			edata->end_time_l = hton32(onchan->end_time_l);
			edata->end_time_h = hton32(onchan->end_time_h);
			edata->cur_chan_seq_start_time_l =
				hton32((uint32)(onchan->cur_chan_seq_start_time >> 32));
			edata->cur_chan_seq_start_time_h =
				hton32((uint32)(onchan->cur_chan_seq_start_time));
		} else if (cb_info->type & MSCH_CT_SLOT_SKIP) {
			wlc_msch_skipslot_info_t *skipslot =
				(wlc_msch_skipslot_info_t *)cb_info->type_specific;
			ASSERT(skipslot);
			edata->start_time_l = hton32(skipslot->start_time_l);
			edata->start_time_h = hton32(skipslot->start_time_h);
			edata->end_time_l = hton32(skipslot->end_time_l);
			edata->end_time_h = hton32(skipslot->end_time_h);
		}

		_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_CALLBACK, len);
	}
}

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)

static msch_event_log_profiler_event_data_t *
_msch_get_event_log_data(wlc_msch_info_t *msch_info, int filter, int num_args, int fmt_num)
{
	wlc_msch_profiler_info_t *profiler_info;
	wlc_msch_profiler_cmn_t *cmn;
	msch_event_log_profiler_event_data_t *edata = NULL;

	if (!msch_info || !msch_info->profiler || !msch_info->profiler->cmn)
		return NULL;

	profiler_info = msch_info->profiler;
	cmn = profiler_info->cmn;

	cmn->dump_status = 0;
	if (cmn->dump_event_log_messages & filter) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_EVENT_LOG_BIT);
	}
	if (cmn->dump_event_messages & filter) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_EVENT_BIT);
	}
#ifdef MSCH_PROFILER
	if (MSCH_PROFILER_ENAB(profiler_info) &&
		(cmn->dump_print_messages & filter)) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_PROFILE_BIT);
	}
#endif /* MSCH_PROFILER */

	if (cmn->dump_status) {
		uint64 cur_time = msch_current_time(msch_info);

		ASSERT(cmn->edata);

		if (cmn->edata_len > WL_MSCH_PROFILER_BUFFER_SIZE -
			sizeof(msch_event_log_profiler_event_data_t)) {
			_msch_event_data_flush(profiler_info);
		}

		edata = (msch_event_log_profiler_event_data_t *)(cmn->edata + cmn->edata_len);
		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));
		edata->hdr.tag = (uint8)profiler_info->wlc->pub->unit;
		edata->hdr.count = (uint8)num_args;
		edata->hdr.fmt_num = hton16((uint16)fmt_num);

		if (filter & MSCH_ERROR_ON) {
			cmn->event_data_flush = TRUE;
		}
	}

	return edata;
}

void
msch_event_log0(wlc_msch_info_t *msch_info, int filter, int fmt_num)
{
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x\n", EVENT_LOG_TAG_MSCHPROFILE, fmt_num);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, 0, fmt_num))) {
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE);
	}
}

void
msch_event_log1(wlc_msch_info_t *msch_info, int filter, int fmt_num, uint32 t1)
{
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x %x\n", EVENT_LOG_TAG_MSCHPROFILE,
			fmt_num, t1);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, 1, fmt_num))) {
		edata->data[0] = t1;
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE + sizeof(uint32));
	}
}

void
msch_event_log2(wlc_msch_info_t *msch_info, int filter, int fmt_num, uint32 t1, uint32 t2)
{
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x %x %x\n", EVENT_LOG_TAG_MSCHPROFILE,
			fmt_num, t1, t2);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, 2, fmt_num))) {
		edata->data[0] = t1;
		edata->data[1] = t2;
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE + 2 * sizeof(uint32));
	}
}

void
msch_event_log3(wlc_msch_info_t *msch_info, int filter, int fmt_num, uint32 t1, uint32 t2,
	uint32 t3)
{
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x %x %x %x\n", EVENT_LOG_TAG_MSCHPROFILE,
			fmt_num, t1, t2, t3);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, 3, fmt_num))) {
		edata->data[0] = t1;
		edata->data[1] = t2;
		edata->data[2] = t3;
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE + 3 * sizeof(uint32));
	}
}

void
msch_event_log4(wlc_msch_info_t *msch_info, int filter, int fmt_num, uint32 t1, uint32 t2,
	uint32 t3, uint32 t4)
{
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x %x %x %x %x\n", EVENT_LOG_TAG_MSCHPROFILE,
			fmt_num, t1, t2, t3, t4);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, 4, fmt_num))) {
		edata->data[0] = t1;
		edata->data[1] = t2;
		edata->data[2] = t3;
		edata->data[3] = t4;
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE + 4 * sizeof(uint32));
	}
}

void
msch_event_logn(wlc_msch_info_t *msch_info, int filter, int num_args, int fmt_num, ...)
{
	va_list ap;
	int i;
	msch_event_log_profiler_event_data_t *edata;

	if (filter & MSCH_ERROR_ON) {
		printf("EL: %x %x", EVENT_LOG_TAG_MSCHPROFILE, fmt_num);
		va_start(ap, fmt_num);
		for (i = num_args; i != 0; i--) {
			printf(" %x", va_arg(ap, uint32));
		}
		printf("\n");
		va_end(ap);
	}

	if ((edata = _msch_get_event_log_data(msch_info, filter, num_args, fmt_num))) {
		va_start(ap, fmt_num);
		for (i = 0; i < num_args; i++) {
			edata->data[i] = va_arg(ap, uint32);
		}
		va_end(ap);
		_msch_write_profiler_tlv(msch_info->profiler, WL_MSCH_PROFILER_EVENT_LOG,
			WL_MSCH_EVENT_LOG_HEAD_SIZE + num_args * sizeof(uint32));
	}
}

#else /* EVENT_LOG_COMPILE && DONGLEBUILD */

void
msch_message_printf(wlc_msch_info_t *msch_info, int filter, const char *fmt, ...)
{
	wlc_msch_profiler_info_t *profiler_info;
	wlc_msch_profiler_cmn_t *cmn;
	va_list ap;
	int len;

	if (filter & MSCH_ERROR_ON) {
		char buffer[128];
		va_start(ap, fmt);
		len = vsnprintf(buffer, 127, fmt, ap);
		va_end(ap);
		buffer[len] = '\0';
		printf("%s", buffer);
	}

	if (!msch_info || !msch_info->profiler || !msch_info->profiler->cmn)
		return;

	profiler_info = msch_info->profiler;
	cmn = profiler_info->cmn;

	cmn->dump_status = 0;
	if (cmn->dump_event_log_messages & filter) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_EVENT_LOG_BIT);
	}
	if (cmn->dump_event_messages & filter) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_EVENT_BIT);
	}
#ifdef MSCH_PROFILER
	if (MSCH_PROFILER_ENAB(profiler_info) &&
		(cmn->dump_print_messages & filter)) {
		cmn->dump_status |= (cmn->dump_enable & MSCH_PROFILE_BIT);
	}
#endif /* MSCH_PROFILER */

	if (cmn->dump_status) {
		uint64 cur_time = msch_current_time(msch_info);
		msch_message_profiler_event_data_t *edata =
			(msch_message_profiler_event_data_t *)cmn->edata;

		ASSERT(cmn->edata);

		edata->time_hi = hton32((uint32)(cur_time >> 32));
		edata->time_lo = hton32((uint32)(cur_time));

		len = WL_MSCH_PROFILER_BUFFER_SIZE - sizeof(msch_message_profiler_event_data_t);

		va_start(ap, fmt);
		len = vsnprintf(edata->message, len, fmt, ap);
		va_end(ap);

		edata->message[len] = '\0';
		len++;

		len += sizeof(msch_profiler_event_data_t);

		_msch_write_profiler_tlv(profiler_info, WL_MSCH_PROFILER_MESSAGE, len);
	}
}
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */

uint32
msch_time_h(uint64 cur_time)
{
	uint32 h = (uint32)(cur_time >> 32);
	uint64 r = cur_time, u = 0;

	if (cur_time == MSCH_INFINITE_TIME) {
		return -1;
	}
	while (h != 0) {
		u += (uint64)((0xffffffff / US_PRE_SEC)) * h;
		r = cur_time - u * US_PRE_SEC;
		h = (uint32)(r >> 32);
	}
	return (uint32)(u + ((uint32)(r) / US_PRE_SEC));
}

uint32
msch_time_l(uint64 cur_time)
{
	uint32 h = (uint32)(cur_time >> 32);
	uint64 r = cur_time, u = 0;

	if (cur_time == MSCH_INFINITE_TIME) {
		return 0;
	}
	while (h != 0) {
		u += (uint64)((0xffffffff / US_PRE_SEC)) * h;
		r = cur_time - u * US_PRE_SEC;
		h = (uint32)(r >> 32);
	}
	return ((uint32)(r) % US_PRE_SEC);
}

#ifdef MSCH_TESTING
static int
_msch_tesing_callback(void *ctx, wlc_msch_cb_info_t *cb_info)
{
	wlc_msch_info_t *msch_info = (wlc_msch_info_t *)ctx;
	uint64 cur_time = msch_current_time(msch_info);

#ifdef MSCH_TESTING_DEBUG
	ASSERT((cb_info->type & 0xFFFF));
	MSCH_TESTING_DEBUG(("wl%d: callback: %d.%06d: type 0x%02x chan 0x%04x(%d)\n",
		msch_info->wlc->pub->unit, _msch_display_time(cur_time),
		cb_info->type, cb_info->chanspec, (cb_info->chanspec & 0xff)));
#endif /* MSCH_TESTING_DEBUG */

	return 0;
}

void _msch_timeslot_unregister(wlc_msch_profiler_info_t *profiler_info,
	wlc_msch_req_handle_t *req_hdl)
{
	wlc_msch_profiler_cmn_t *cmn = profiler_info->cmn;
	int i;
	for (i = 0; i < MAX_REGISTER_MSCH; i++) {
		if (cmn->test[i].p_req_hdl == req_hdl) {
			cmn->test[i].p_req_hdl = NULL;
			break;
		}
	}
}
#endif /* MSCH_TESTING */

static int32
wlc_msch_doiovar_get(wlc_msch_profiler_info_t *profiler_info, uint8 enable_bit,
	uint8 log_messages)
{
	wlc_msch_profiler_cmn_t *cmn = profiler_info->cmn;
	return (((cmn->dump_enable & enable_bit)? WL_MSCH_CMD_ENABLE_BIT : 0) |
		((cmn->dump_profiler & enable_bit)? WL_MSCH_CMD_PROFILE_BIT : 0) |
		((cmn->dump_callback & enable_bit)? WL_MSCH_CMD_CALLBACK_BIT : 0) |
		((cmn->dump_register & enable_bit)?WL_MSCH_CMD_REGISTER_BIT : 0) |
		((log_messages & MSCH_ERROR_ON)? WL_MSCH_CMD_ERROR_BIT : 0) |
		((log_messages & MSCH_DEBUG_ON)? WL_MSCH_CMD_DEBUG_BIT : 0) |
		((log_messages & MSCH_INFORM_ON)? WL_MSCH_CMD_INFOM_BIT : 0) |
		((log_messages & MSCH_TRACE_ON)? WL_MSCH_CMD_TRACE_BIT : 0));
}

static int
wlc_msch_doiovar_set(wlc_msch_profiler_info_t *profiler_info, uint8 enable_bit,
	uint8 *log_messages, int32 int_val)
{
	wlc_msch_profiler_cmn_t *cmn = profiler_info->cmn;
	wlc_info_t *wlc = profiler_info->wlc;
	int err = BCME_OK;
#ifdef WLRSDB
	wlc_info_t *wlc_temp;
	int idx;
#endif /* WLRSDB */

	cmn->ver = (int_val & WL_MSCH_CMD_VER_MASK) >> WL_MSCH_CMD_VER_SHIFT;

#ifdef WLRSDB
	FOREACH_WLC(wlc->cmn, idx, wlc_temp) {
		wlc_msch_info_t *msch_info = wlc_temp->msch_info;
		if (msch_info) {
			wlc_msch_profiler_info_t *profiler = msch_info->profiler;
			profiler->wl_index = 0;
			if (cmn->ver >= WL_MSCH_PROFILER_RSDB_VER) {
				profiler->wl_index |= (wlc_temp->pub->unit <<
					WL_MSCH_PROFILER_WLINDEX_SHIFT);
			}
			if (cmn->ver >= WL_MSCH_PROFILER_REPORT_VER) {
				profiler->wl_index |= (cmn->ver <<
					WL_MSCH_PROFILER_VER_SHIFT);
			}
		}
	}
#else
	profiler_info->wl_index = 0;
	if (cmn->ver >= WL_MSCH_PROFILER_RSDB_VER) {
		profiler_info->wl_index |= (wlc->pub->unit << WL_MSCH_PROFILER_WLINDEX_SHIFT);
	}
	if (cmn->ver >= WL_MSCH_PROFILER_REPORT_VER) {
		profiler_info->wl_index |= (cmn->ver << WL_MSCH_PROFILER_VER_SHIFT);
	}
#endif /* WLRSDB */

	if (int_val & WL_MSCH_CMD_PROFILE_BIT)
		cmn->dump_profiler |= enable_bit;
	else
		cmn->dump_profiler &= ~enable_bit;

	if (int_val & WL_MSCH_CMD_CALLBACK_BIT)
		cmn->dump_callback |= enable_bit;
	else
		cmn->dump_callback &= ~enable_bit;

	if (int_val & WL_MSCH_CMD_REGISTER_BIT)
		cmn->dump_register |= enable_bit;
	else
		cmn->dump_register &= ~enable_bit;

	if (int_val & WL_MSCH_CMD_ERROR_BIT)
		*log_messages |= MSCH_ERROR_ON;
	else
		*log_messages &= ~MSCH_ERROR_ON;

	if (int_val & WL_MSCH_CMD_DEBUG_BIT)
		*log_messages |= MSCH_DEBUG_ON;
	else
		*log_messages &= ~MSCH_DEBUG_ON;

	if (int_val & WL_MSCH_CMD_INFOM_BIT)
		*log_messages |= MSCH_INFORM_ON;
	else
		*log_messages &= ~MSCH_INFORM_ON;

	if (int_val & WL_MSCH_CMD_TRACE_BIT)
		*log_messages |= MSCH_TRACE_ON;
	else
		*log_messages &= ~MSCH_TRACE_ON;

	if (!cmn->edata) {
		cmn->ptlv = MALLOC(wlc->osh, WL_MSCH_PROFILER_BUFFER_SIZE +
			WL_MSCH_PROFILE_HEAD_SIZE);
		if (cmn->ptlv) {
			cmn->edata = (uint8 *)cmn->ptlv->value;
		} else {
			err = BCME_NOMEM;
		}
	}

	return err;
}

static int
wlc_msch_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len, void *arg, uint a_len, uint val_size, struct wlc_if *wlcif)
{
	wlc_msch_profiler_info_t *profiler_info = (wlc_msch_profiler_info_t *)ctx;
	wlc_msch_profiler_cmn_t *cmn;
	wlc_msch_info_t *msch_info;
	wlc_info_t *wlc;
	int32 int_val = 0;
	bool bool_val = FALSE;
	int32 *ret_int_ptr;
	int err = BCME_UNSUPPORTED;

	ASSERT(profiler_info != NULL);
	ASSERT(profiler_info->wlc != NULL);

	cmn = profiler_info->cmn;

	msch_info = profiler_info->msch_info;
	BCM_REFERENCE(msch_info);
	wlc = profiler_info->wlc;
	BCM_REFERENCE(wlc);

	/* convenience int and bool vals for first 4 bytes of buffer */
	if (p_len >= (int)sizeof(int_val)) {
		bcopy(params, &int_val, sizeof(int_val));
	}
	bool_val = (int_val != 0) ? TRUE : FALSE;

	BCM_REFERENCE(int_val);
	BCM_REFERENCE(bool_val);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	/* Process IOVARS */
	switch (actionid) {
#ifdef MSCH_TESTING
	case IOV_SVAL(IOV_MSCH_REQ):
	{
		msch_register_params_t *req = (msch_register_params_t *)params;
		wlc_msch_req_param_t req_param;
		uint64 s, t = msch_current_time(msch_info);
		int id = (int)(req->id);

		if (id >= MAX_REGISTER_MSCH || (id != 0 && cmn->test[id].p_req_hdl)) {
			err = BCME_BADARG;
			break;
		}

#ifdef WLRSDB
		if (RSDB_ENAB(wlc->pub)) {
			if (req->wlc_index >= MAX_RSDB_MAC_NUM) {
				WL_ERROR(("wl%d: Invalid WLC Index %d \n",
					wlc->pub->unit, req->wlc_index));
				err = BCME_BADARG;
				break;
			}

			wlc = wlc->cmn->wlc[req->wlc_index];
			msch_info = wlc->msch_info;
		}
#endif /* WLRSDB */

		s = t + MS_TO_USEC(req->start_time);

		req_param.flags = (uint32)req->flags;
		req_param.req_type = (wlc_msch_req_type_t)req->req_type;
		req_param.priority = (uint8)req->priority;
		req_param.start_time_l = (uint32)s;
		req_param.start_time_h = (uint32)(s >> 32);
		req_param.duration = MS_TO_USEC(req->duration);
		req_param.interval = MS_TO_USEC(req->interval);

		if (req_param.req_type == MSCH_RT_DUR_FLEX) {
			req_param.flex.dur_flex = MS_TO_USEC(req->dur_flex);
		} else if (MSCH_BOTH_FLEX(req_param.req_type)) {
			req_param.flex.bf.min_dur = MS_TO_USEC(req->min_dur);
			req_param.flex.bf.max_away_dur = MS_TO_USEC(req->max_away_dur);

			s = t + MS_TO_USEC(req->hi_prio_time);
			req_param.flex.bf.hi_prio_time_l = (uint32)s;
			req_param.flex.bf.hi_prio_time_h = (uint32)(s >> 32);
			req_param.flex.bf.hi_prio_interval = MS_TO_USEC(req->hi_prio_interval);
		}

		cmn->test[id].p_info = msch_info;
		if ((err = wlc_msch_timeslot_register(msch_info, (chanspec_t*)req->chanspec_list,
			(int)req->chanspec_cnt, _msch_tesing_callback,
			(void *)msch_info, &req_param, &cmn->test[id].p_req_hdl)) != BCME_OK) {
			WL_ERROR(("%s request failed error %d \r\n", __FUNCTION__, err));
		} else {
			wlc_msch_set_chansw_reason(msch_info, cmn->test[id].p_req_hdl,
				CHANSW_IOVAR);
		}
		err = BCME_OK;
	}
	break;

	case IOV_SVAL(IOV_MSCH_UNREQ):
		if (int_val >= MAX_REGISTER_MSCH || int_val == 0 ||
			!cmn->test[int_val].p_req_hdl) {
			err = BCME_BADARG;
			break;
		}

		wlc_msch_timeslot_unregister(cmn->test[int_val].p_info,
			&cmn->test[int_val].p_req_hdl);
		cmn->test[int_val].p_req_hdl = NULL;
		err = BCME_OK;
		break;
#endif /* MSCH_TESTING */

#if defined(EVENT_LOG_COMPILE) || !defined(DONGLEBUILD)
	case IOV_GVAL(IOV_MSCH_EVENT_LOG):
		*ret_int_ptr = wlc_msch_doiovar_get(profiler_info, MSCH_EVENT_LOG_BIT,
			cmn->dump_event_log_messages);
		err = BCME_OK;
		break;

	case IOV_SVAL(IOV_MSCH_EVENT_LOG):
		if (int_val & WL_MSCH_CMD_ENABLE_BIT) {
			if ((err = wlc_msch_doiovar_set(profiler_info, MSCH_EVENT_LOG_BIT,
				&cmn->dump_event_log_messages, int_val)) == BCME_OK) {
				msch_msg_level = ((cmn->dump_enable & MSCH_PROFILE_BIT)?
					cmn->dump_print_messages : 0) |
					((cmn->dump_enable & MSCH_EVENT_BIT)?
					cmn->dump_event_messages : 0) |
					cmn->dump_event_log_messages | MSCH_ERROR_ON;
#ifdef EVENT_LOG_COMPILE
				event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE,
					EVENT_LOG_SET_MSCH_PROFILER,
					EVENT_LOG_TAG_FLAG_LOG);
				event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE_TLV,
					EVENT_LOG_SET_MSCH_PROFILER,
					EVENT_LOG_TAG_FLAG_LOG);
#endif /* EVENT_LOG_COMPILE */
				cmn->dump_enable |= MSCH_EVENT_LOG_BIT;
			} else {
				int_val &= ~WL_MSCH_CMD_ENABLE_BIT;
			}
		} else {
			err = BCME_OK;
		}

		if (!(int_val & WL_MSCH_CMD_ENABLE_BIT)) {
#ifdef EVENT_LOG_COMPILE
			event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE,
				EVENT_LOG_SET_MSCH_PROFILER,
				EVENT_LOG_TAG_FLAG_NONE);
			event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE_TLV,
				EVENT_LOG_SET_MSCH_PROFILER,
				EVENT_LOG_TAG_FLAG_NONE);
#endif /* EVENT_LOG_COMPILE */
			cmn->dump_enable &= ~MSCH_EVENT_LOG_BIT;
			if (cmn->edata && !cmn->dump_enable) {
				MFREE(wlc->osh, cmn->ptlv, WL_MSCH_PROFILER_BUFFER_SIZE +
					WL_MSCH_PROFILE_HEAD_SIZE);
				cmn->edata = NULL;
			}
		}
		break;
#endif /* EVENT_LOG_COMPILE || !DONGLEBUILD */

#if defined(EVENT_LOG_COMPILE) || defined(MSCH_PROFILER)
	case IOV_GVAL(IOV_MSCH_EVENT):
#ifndef EVENT_LOG_COMPILE
	if (MSCH_PROFILER_ENAB(profiler_info))
#endif /* EVENT_LOG_COMPILE */
	{
		*ret_int_ptr = wlc_msch_doiovar_get(profiler_info, MSCH_EVENT_BIT,
			cmn->dump_event_messages);
		err = BCME_OK;
	}
	break;

	case IOV_SVAL(IOV_MSCH_EVENT):
#ifndef EVENT_LOG_COMPILE
	if (MSCH_PROFILER_ENAB(profiler_info))
#endif /* EVENT_LOG_COMPILE */
	{
		if (int_val & WL_MSCH_CMD_ENABLE_BIT) {
			if ((err = wlc_msch_doiovar_set(profiler_info, MSCH_EVENT_BIT,
				&cmn->dump_event_messages, int_val)) == BCME_OK) {
				msch_msg_level = ((cmn->dump_enable & MSCH_PROFILE_BIT)?
					cmn->dump_print_messages : 0) |
					((cmn->dump_enable & MSCH_EVENT_LOG_BIT)?
					cmn->dump_event_log_messages : 0) |
					cmn->dump_event_messages | MSCH_ERROR_ON;
				cmn->dump_enable |= MSCH_EVENT_BIT;
			} else {
				int_val &= ~WL_MSCH_CMD_ENABLE_BIT;
			}
		} else {
			err = BCME_OK;
		}

		if (cmn->edata) {
			uint64 cur_time = msch_current_time(msch_info);
			msch_start_profiler_event_data_t *edata;

			edata = (msch_start_profiler_event_data_t *)cmn->edata;
			edata->time_hi = hton32((uint32)(cur_time >> 32));
			edata->time_lo = hton32((uint32)(cur_time));
			edata->status = hton32((uint32)int_val);

			cmn->dump_status = MSCH_EVENT_BIT;
			_msch_write_profiler_tlv(profiler_info, (int_val & WL_MSCH_CMD_ENABLE_BIT)?
				WL_MSCH_PROFILER_START : WL_MSCH_PROFILER_EXIT,
				sizeof(msch_start_profiler_event_data_t));
		}

		if (!(int_val & WL_MSCH_CMD_ENABLE_BIT)) {
			cmn->dump_enable &= ~MSCH_EVENT_BIT;
			if (cmn->edata && !cmn->dump_enable) {
				MFREE(wlc->osh, cmn->ptlv, WL_MSCH_PROFILER_BUFFER_SIZE +
					WL_MSCH_PROFILE_HEAD_SIZE);
				cmn->edata = NULL;
			}
		}
	}
	break;
#endif /* EVENT_LOG_COMPILE || MSCH_PROFILER */

#ifdef MSCH_PROFILER
	case IOV_GVAL(IOV_MSCH_COLLECT):
	if (MSCH_PROFILER_ENAB(profiler_info)) {
		*ret_int_ptr = wlc_msch_doiovar_get(profiler_info, MSCH_PROFILE_BIT,
			cmn->dump_print_messages);
		err = BCME_OK;
	}
	break;

	case IOV_SVAL(IOV_MSCH_COLLECT):
	if (MSCH_PROFILER_ENAB(profiler_info)) {
		if (int_val & WL_MSCH_CMD_ENABLE_BIT) {
			if ((err = wlc_msch_doiovar_set(profiler_info, MSCH_PROFILE_BIT,
				&cmn->dump_print_messages, int_val)) == BCME_OK) {
				if (!cmn->profiler) {
					int size = ((int_val & WL_MSCH_CMD_SIZE_MASK) >>
						WL_MSCH_CMD_SIZE_SHIFT) * 1024;

					if (size == 0)
						size = DEFAULT_PROFILE_BUFFER_SIZE;
					else if (size > MAX_PROFILE_BUFFER_SIZE)
						size = MAX_PROFILE_BUFFER_SIZE;
					else if (size < MIN_PROFILE_BUFFER_SIZE)
						size = MIN_PROFILE_BUFFER_SIZE;

					cmn->profiler = MALLOC(wlc->osh,
						sizeof(wlc_msch_profiler_t) + size);
					if (cmn->profiler) {
						wlc_msch_profiler_t *profiler = cmn->profiler;
						bzero(profiler, sizeof(wlc_msch_profiler_t));
						profiler->magic1 = MSCH_MAGIC_1;
						profiler->magic2 = MSCH_MAGIC_2;
						profiler->total_size = size;
						profiler->buffer = (uint8 *)&profiler[1];
					} else {
						err = BCME_NOMEM;
						int_val &= ~WL_MSCH_CMD_ENABLE_BIT;
					}
				}

				if (cmn->profiler) {
					msch_msg_level = ((cmn->dump_enable & MSCH_EVENT_BIT)?
						cmn->dump_event_messages : 0) |
						((cmn->dump_enable & MSCH_EVENT_LOG_BIT)?
						cmn->dump_event_log_messages : 0) |
						cmn->dump_print_messages | MSCH_ERROR_ON;
					cmn->dump_enable |= MSCH_PROFILE_BIT;
				}
			} else {
				int_val &= ~WL_MSCH_CMD_ENABLE_BIT;
			}
		} else {
			err = BCME_OK;
		}

		if (!(int_val & WL_MSCH_CMD_ENABLE_BIT)) {
			cmn->dump_enable &= ~MSCH_PROFILE_BIT;
			if (cmn->profiler) {
				MFREE(wlc->osh, cmn->profiler,
					sizeof(wlc_msch_profiler_t) +
					cmn->profiler->total_size);
				cmn->profiler = NULL;
			}
			if (cmn->edata && !cmn->dump_enable) {
				MFREE(wlc->osh, cmn->ptlv, WL_MSCH_PROFILER_BUFFER_SIZE +
					WL_MSCH_PROFILE_HEAD_SIZE);
				cmn->edata = NULL;
			}
		}
	}
	break;

	case IOV_GVAL(IOV_MSCH_DUMP):
	if (MSCH_PROFILER_ENAB(profiler_info)) {
		if (cmn->profiler) {
			if (bool_val) {
				wlc_msch_profiler_t *profiler = cmn->profiler;

				if (!(cmn->dump_enable & MSCH_PROFILE_BIT)) {
					err = BCME_NOTREADY;
					break;
				}

				cmn->dump_enable &= ~MSCH_PROFILE_BIT;
				profiler->read_ptr = profiler->start_ptr;
				profiler->read_size = 0;
			}
			err = _msch_read_profiler_tlv(profiler_info, (uint8 *)arg, a_len);
			if (err != BCME_OK) {
				cmn->dump_enable |= MSCH_PROFILE_BIT;
			}
		} else
			err = BCME_NOTREADY;
	}
	break;
#endif /* MSCH_PROFILER */

	default:
		break;
	}

	return err;
}

void
BCMATTACHFN(wlc_msch_profiler_detach)(wlc_msch_profiler_info_t *profiler_info)
{
	wlc_info_t *wlc;
	wlc_msch_profiler_cmn_t *cmn;

	if (!profiler_info) {
		return;
	}

	wlc = profiler_info->wlc;
	cmn = profiler_info->cmn;

	if (cmn != NULL) {
		if (obj_registry_unref(wlc->objr, OBJR_MSCH_PROFILER_INFO) == 0) {
			obj_registry_set(wlc->objr, OBJR_MSCH_PROFILER_INFO, NULL);

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
			if (cmn->event_timer) {
				if (cmn->event_timer_start) {
					wlc_hrt_del_timeout(cmn->event_timer);
				}
				wlc_hrt_free_timeout(cmn->event_timer);
				cmn->event_timer = NULL;
			}
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */
#ifdef MSCH_PROFILER
			if (MSCH_PROFILER_ENAB(profiler_info)) {
				/* deallocate memory to avoid memory leak */
				if (cmn->profiler) {
					MFREE(wlc->osh, cmn->profiler,
						sizeof(wlc_msch_profiler_t) +
						cmn->profiler->total_size);
				}
			}
#endif /* MSCH_PROFILER */
			if (cmn->edata) {
				MFREE(wlc->osh, cmn->ptlv, WL_MSCH_PROFILER_BUFFER_SIZE +
					WL_MSCH_PROFILE_HEAD_SIZE);
			}

			MFREE(wlc->osh, cmn, sizeof(wlc_msch_profiler_cmn_t));
			cmn = NULL;
		}
	}

	/* sanity */
	wlc_module_unregister(wlc->pub, MSCH_NAME, profiler_info);

	MFREE(wlc->osh, profiler_info, sizeof(wlc_msch_profiler_info_t));
}

wlc_msch_profiler_info_t *
BCMATTACHFN(wlc_msch_profiler_attach)(wlc_msch_info_t *msch_info)
{
	wlc_info_t *wlc = msch_info->wlc;
	wlc_msch_profiler_info_t *profiler_info = NULL;
	wlc_msch_profiler_cmn_t *cmn;
	int err;

	profiler_info = MALLOCZ(wlc->osh, sizeof(wlc_msch_profiler_info_t));
	if (!profiler_info) {
		return NULL;
	}

	profiler_info->wlc = wlc;
	profiler_info->msch_info = msch_info;

	/* module shared states */
	cmn = (wlc_msch_profiler_cmn_t *)obj_registry_get(wlc->objr, OBJR_MSCH_PROFILER_INFO);
	if (cmn == NULL) {
		if ((cmn = MALLOCZ(wlc->osh, sizeof(wlc_msch_profiler_cmn_t))) == NULL)
			goto fail;
		obj_registry_set(wlc->objr, OBJR_MSCH_PROFILER_INFO, cmn);

		cmn->ver = WL_MSCH_PROFILER_VER;
		cmn->dump_profiler = MSCH_PROFILE_BIT;
		cmn->dump_callback = MSCH_EVENT_LOG_BIT | MSCH_EVENT_BIT | MSCH_PROFILE_BIT;
		cmn->dump_register = MSCH_EVENT_LOG_BIT | MSCH_EVENT_BIT | MSCH_PROFILE_BIT;
		cmn->dump_print_messages = MSCH_ERROR_ON | MSCH_DEBUG_ON | MSCH_TRACE_ON;
		cmn->dump_event_messages = MSCH_ERROR_ON | MSCH_DEBUG_ON;
		cmn->dump_event_log_messages = MSCH_ERROR_ON | MSCH_DEBUG_ON;

#if defined(EVENT_LOG_COMPILE) && defined(DONGLEBUILD)
		cmn->ptlv = MALLOC_NOPERSIST(wlc->osh, WL_MSCH_PROFILER_BUFFER_SIZE +
			WL_MSCH_PROFILE_HEAD_SIZE);
		if (!cmn->ptlv) {
			goto fail;
		}

		cmn->ver = WL_MSCH_PROFILER_REPORT_VER;
		cmn->edata = (uint8 *)cmn->ptlv->value;

		msch_msg_level = MSCH_ERROR_ON | MSCH_DEBUG_ON;

		/* Allocate 2 blocks of 1K each for MSCH logging */
		err = event_log_set_init(wlc->osh, EVENT_LOG_SET_MSCH_PROFILER,
			EVENT_LOG_MSCH_BLOCK_SIZE);

		if (err != BCME_OK) {
			goto fail;
		}

		err = event_log_set_expand(wlc->osh, EVENT_LOG_SET_MSCH_PROFILER,
			EVENT_LOG_MSCH_BLOCK_SIZE);

		if (err != BCME_OK) {
			goto fail;
		}

		event_log_set_destination_set(EVENT_LOG_SET_MSCH_PROFILER,
			SET_DESTINATION_HOST);

		event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE,
			EVENT_LOG_SET_MSCH_PROFILER,
			EVENT_LOG_TAG_FLAG_LOG);
		event_log_tag_start(EVENT_LOG_TAG_MSCHPROFILE_TLV,
			EVENT_LOG_SET_MSCH_PROFILER,
			EVENT_LOG_TAG_FLAG_LOG);
		cmn->dump_enable = MSCH_EVENT_LOG_BIT;

		cmn->event_timer = wlc_hrt_alloc_timeout(wlc->hrti);
		if (!cmn->event_timer) {
			WL_ERROR(("wl%d: wlc_msch_profiler_attach: wlc_hrt_alloc_timeout failed\n",
				wlc->pub->unit));
			goto fail;
		}
#endif /* EVENT_LOG_COMPILE && DONGLEBUILD */
	}
	(void)obj_registry_ref(wlc->objr, OBJR_MSCH_PROFILER_INFO);
	profiler_info->cmn = cmn;

	profiler_info->wl_index = (wlc->pub->unit << WL_MSCH_PROFILER_WLINDEX_SHIFT) |
		(cmn->ver << WL_MSCH_PROFILER_VER_SHIFT);

#if defined(MSCH_PROFILER) && !defined(MSCH_PROFILER_DISABLED)
	profiler_info->_msch_profiler = TRUE;
#else
	profiler_info->_msch_profiler = FALSE;
#endif /* MSCH_PROFILER && !MSCH_PROFILER_DISABLED */

	/* register module */
	err = wlc_module_register(wlc->pub, wlc_msch_iovars, MSCH_NAME, (void *)profiler_info,
		wlc_msch_doiovar, NULL, NULL, NULL);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed with status %d\n",
			wlc->pub->unit, __FUNCTION__, err));
		goto fail;
	}

	return profiler_info;

fail:
	wlc_msch_profiler_detach(profiler_info);

	return NULL;
}
#endif /* (EVENT_LOG_COMPILE && MSCH_EVENT_LOG) || MSCH_PROFILER || !DONGLEBUILD */
