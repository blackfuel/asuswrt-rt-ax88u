/*
 *	acs_dfsr.c
 *
 *	ACSD DFS Re-entry module.
 *
 *	This module monitors channel switches over time and re-selects DFS channels if channel
 *	switches occur too frequently. Depending on the frequency, DFS channels are reentered
 *	during times where the channel is idle (deferred reentry) or immediately.
 *
 *	Copyright 2018 Broadcom
 *
 *	This program is the proprietary software of Broadcom and/or
 *	its licensors, and may only be used, duplicated, modified or distributed
 *	pursuant to the terms and conditions of a separate, written license
 *	agreement executed between you and Broadcom (an "Authorized License").
 *	Except as set forth in an Authorized License, Broadcom grants no license
 *	(express or implied), right to use, or waiver of any kind with respect to
 *	the Software, and Broadcom expressly reserves all rights in and to the
 *	Software and all intellectual property rights therein.  IF YOU HAVE NO
 *	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *	WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *	THE SOFTWARE.
 *
 *	Except as expressly set forth in the Authorized License,
 *
 *	1. This program, including its structure, sequence and organization,
 *	constitutes the valuable trade secrets of Broadcom, and you shall use
 *	all reasonable efforts to protect the confidentiality thereof, and to
 *	use this information only in connection with your use of Broadcom
 *	integrated circuit products.
 *
 *	2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *	"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *	REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *	OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *	DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *	NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *	ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *	CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *	OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *	3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *	BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *	SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *	IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *	IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *	ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *	OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *	NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id$
 */
#include "acsd_svr.h"
#include "acs_dfsr.h"

/*
 * Definition of the Sliding windows used in this module.
 */

typedef unsigned sw_counter;

typedef struct {
	unsigned seconds;
	unsigned threshold;
	uint32 interval;
} dfsr_window_params_t;

typedef struct {
	unsigned curslot;
	sw_counter slots[SW_NUM_SLOTS];
} dfsr_window_struct_t;

typedef struct {
		dfsr_window_struct_t w;
		dfsr_window_params_t p;
} dfsr_window_t;

static int acs_dfsr_counter_update(dfsr_context_t *ctx, char *if_name);

/*
 * dfsr_sw_sum() - sum up the counters of all slots of a sliding window.
 *
 * sw:	sliding window pointer.
 *
 * The return value is the sum of all slots in the sliding window.
 *
 */
sw_counter
dfsr_sw_sum(dfsr_window_t *sw)
{
	int i;
	sw_counter tot = 0;

	for (i = 0; i < SW_NUM_SLOTS; ++i)
		tot += sw->w.slots[i];

	return tot;
}

/*
 * dfsr_sw_add() - add a counter value to a sliding window.
 *
 * sw:		sliding window pointer
 * slottime:	slot time in seconds.
 * value:	value to add to the specified slot number
 *
 * This function adds a value to the current counter contents of the slot targeted by slottime.
 * It takes car of clearing any intermediate slots if the slottime wraps or increments in a non-
 * linear fashion.
 *
 */
static void
dfsr_sw_add(dfsr_window_t *sw, unsigned int slottime, sw_counter value)
{
	int i;

	i = ((sw->p.seconds/sw->p.interval)/SW_NUM_SLOTS);	/* Adjust to fixed size windows */
	slottime /= sw->p.interval;
	slottime /= (i) ? i : 1;			/* (min is SW_NUM_SLOTS) */

	if (sw->w.curslot != slottime) { /* new slot, clear counter(s) up to here */
		i = sw->w.curslot+1;
		while (i % SW_NUM_SLOTS != slottime % SW_NUM_SLOTS) {
			sw->w.slots[i % SW_NUM_SLOTS] = 0;
			i++;
		}
		sw->w.slots[slottime % SW_NUM_SLOTS] = 0;
	}
	sw->w.curslot = slottime;
	sw->w.slots[sw->w.curslot % SW_NUM_SLOTS] += value;
}

/*
 * dfsr_sw_clear() - clear the counters of a sliding window.
 *
 * sw:	the sliding window pointer.
 *
 */
void
dfsr_sw_clear(dfsr_window_t *sw)
{
	memset(sw, 0,  sizeof(sw->w));
}

enum { W_IMMEDIATE = 0, W_DEFERRED, W_ACTIVITY, W_BGDFS, W_COUNT };

/* The DFS Reentry context structure, private to this module. */
struct dfsr_context {
	dfsr_window_t		window[W_COUNT];	/* various sliding windows */
	unsigned		prev_bytes_tx;		/* counter needed for deltas */
	unsigned		prev_bytes_rx;		/* counter needed for deltas */
	unsigned		switch_count;		/* channel switch count */
	unsigned		reentry_count;		/* dfs re-entry count */
	dfsr_reentry_type_t	reentry_type;		/* none, deferred, immediate */
	chanspec_t		channel;		/* currently selected chanspec */
	bool			enabled;		/* Runtime enable/disable flag */
};

static dfsr_window_params_t default_params[W_COUNT] = {
	{ seconds:5*60, threshold:3, interval:1 },	/* Immed: > 3 switches in last 5 minutes */
	{ seconds:7*60*60*24, threshold:5, interval:1 }, /* Defer: > 5 switches in last 7 days */
	{ seconds:30, threshold:10*1024, interval:1 },	/* ch idle if< 100kB of RX/TX in last 30s */
	{ seconds:360, threshold:36000, interval:30 }	/* idle interval, idle frames thld,
							 * slot update interval
							 */
};

/*
 * dfsr_window_name() - helper function to map the window index to a human readable name.
 *
 * This function is used both for displaying the window name in debug output, and for loading
 * nvram configuration parameters.
 *
 */
static const char *
dfsr_window_name(unsigned w)
{
	switch (w) {
		case W_IMMEDIATE:	return "immediate";
		case W_DEFERRED:        return "deferred";
		case W_ACTIVITY:        return "activity";
		case W_BGDFS:		return "bgdfs";
	}
	return "?";
}

/*
 * acs_dfsr_enabled() - test whether DFS Reentry is enabled.
 *
 * ctx:		DFS Reentry context
 *
 * Returns a boolean value indicating whether DFS reentry is enabled (TRUE) or not (FALSE).
 *
 */
bool
acs_dfsr_enabled(dfsr_context_t *ctx)
{
	return (ctx && ctx->enabled);
}

/*
 * acs_dfsr_enable() - Enable or disable DFS Reentry.
 *
 * ctx:		DFS Reentry context
 * enable:	Boolean indicating whether to enable (TRUE) or disable (FALSE) DFS Reentry.
 *
 * Returns a boolean value indicating whether DFS reentry state has been set or not (=no ctx).
 *
 */

bool
acs_dfsr_enable(dfsr_context_t *ctx, bool enable)
{
	if (ctx) {
		ctx->enabled = enable;
	}
	ACSD_DFSR("(%sable), now %sabled.\n",
		(enable) ? "en" : "dis",
		(acs_dfsr_enabled(ctx)) ? "en" : "dis");

	return (ctx != NULL);
}

/*
 * acs_dfsr_load_config() - load configuration parameters from nvram.
 *
 * ctx:		DFS Reentry context
 * prefix:	nul terminated ascii string representing the nvram prefix (eg. "wl1_")
 *
 * Current defined configuration parameters are (prepend prefix as needed):
 *	acs_dfsr_immediate=<seconds> <threshold>
 *	acs_dfsr_deferred=<seconds> <threshold>
 *	acs_dfsr_activity=<seconds> <threshold>
 *
 */
static void
acs_dfsr_load_config(dfsr_context_t *ctx, char *prefix)
{
#define KEY_SIZE 32
	unsigned i;

	/* Load window thresholds only for first three windows, eg "wl1_acs_dfsr_immediate=60 3" */

	for (i = 0; i < (W_COUNT-1); ++i) {
		char key[KEY_SIZE];
		char *str;
		acs_snprintf(key, KEY_SIZE, "%sacs_dfsr_%s", prefix, dfsr_window_name(i));
		str = nvram_get(key);
		if (str) {
			unsigned sec, thr;
			if (sscanf(str, "%u %u", &sec, &thr) == 2) { /* acs_dfsr nvram settings */
				ACSD_DFSR("Found %s, setting %s window seconds=%d, threshold=%d\n",
					key, dfsr_window_name(i), sec, thr);
				ctx->window[i].p.seconds = sec;
				ctx->window[i].p.threshold = thr;
			} else {
				ACSD_ERROR("Found invalid nvram settings (%s=%s)\n", key, str);
			}
		}
	}
}

/*
 * acs_dfsr_init() - create and initialise the dfs re-entry context.
 *
 * prefix:	nvram configuration prefix string.
 * enable:	Boolean to indicate whether or not to enable dfs reentry.
 *
 * Returns the pointer to the dfs re-entry context, or NULL on error.
 *
 * Note that to disable dfs re-entry, we can simply not create the context (and return a NULL),
 * for example based on some configuration parameter. Or the caller could just not call our init
 * function and store a null context pointer, passing that to other functions (rather than check
 * each time whether or not it should be called). But in order to allow enabling and disabling at
 * runtime, ie using a debug command, we maintain a separate "enabled" flag.
 *
 */
dfsr_context_t *
acs_dfsr_init(char *prefix, bool enable, acs_bgdfs_info_t *acs_bgdfs)
{
	dfsr_context_t *ctx;

	ctx = malloc(sizeof(*ctx));
	ACSD_DFSR("DFS Re-Entry is %sabled.\n", (enable) ? "en" : "dis");
	if (ctx) {
		int i;
		memset(ctx, 0, sizeof(*ctx));
		for (i = 0; i < W_COUNT; ++i) {
			ctx->window[i].p = default_params[i];	/* structure copy */
			ACSD_DFSR("Default DFS Re-Entry %s window seconds:%u, threshold:%u\n",
				dfsr_window_name(i), ctx->window[i].p.seconds,
				ctx->window[i].p.threshold);
		}
		acs_dfsr_load_config(ctx, prefix);
		ctx->enabled = enable;

		if (acs_bgdfs != NULL) { /* W_BGDFS nvram settings */
			ctx->window[W_BGDFS].p.seconds = acs_bgdfs->idle_interval;
			ctx->window[W_BGDFS].p.threshold = acs_bgdfs->idle_frames_thld;
		}
	}

	return ctx;
}

/*
 * acs_dfsr_exit() - clean up.
 *
 * This function cleans up any private data handled by this module, and releases the context.
 */
void
acs_dfsr_exit(dfsr_context_t *ctx)
{
	if (ctx) {
		free(ctx);
	}
}

/*
 * acs_dfsr_chanspec_update() - update the chanspec, ie on a channel switch.
 *
 * ctx:	The DFS Re-entry context pointer.
 *
 * channel: The chanspec that is being switched to (or was switched to).
 *
 * caller: caller name (for debugging).
 *
 * if_name: interface name (required for counter update)
 *
 * This function is to be called when a channel switch occurs (or was noticed).
 * It will update the immediate/deferred windows and check whether any threshold was exceeded.
 *
 * Switches from channel 0 or to the same channel will be ignored.
 *
 * The return value is the dfs reentry to be performed (none, immediate, or deferred).
 *
 */
dfsr_reentry_type_t
acs_dfsr_chanspec_update(dfsr_context_t *ctx, chanspec_t channel, const char *caller, char *if_name)
{
	if (!acs_dfsr_enabled(ctx)) return DFS_REENTRY_NONE;

	ACSD_DFSR("Switch to channel 0x%4x (%s) requested by %s\n", channel, wf_chspec_ntoa(channel, chanspecbuf), caller);

	if (channel != ctx->channel && (ctx->channel != 0)) {
		time_t now = time(0);

		ACSD_DFSR("Switching to chanspec 0x%4x (%s), switch count so far %u.\n", channel, wf_chspec_ntoa(channel, chanspecbuf),
			ctx->switch_count);

		ctx->switch_count++;

		dfsr_sw_add(&ctx->window[W_IMMEDIATE], now, 1);
		dfsr_sw_add(&ctx->window[W_DEFERRED], now, 1);

		ACSD_DFSR("Immediate window %u second threshold %u, actual %u.\n",
			ctx->window[W_IMMEDIATE].p.seconds,  ctx->window[W_IMMEDIATE].p.threshold,
			dfsr_sw_sum(&ctx->window[W_IMMEDIATE]));
		ACSD_DFSR("Deferred window %u second threshold %u, actual %u.\n",
			ctx->window[W_DEFERRED].p.seconds,  ctx->window[W_DEFERRED].p.threshold,
			dfsr_sw_sum(&ctx->window[W_DEFERRED]));

		/* A zero threshold disables the check */
		if (ctx->window[W_IMMEDIATE].p.threshold &&
			(dfsr_sw_sum(&ctx->window[W_IMMEDIATE]) >
				ctx->window[W_IMMEDIATE].p.threshold)) {
			ctx->reentry_type = DFS_REENTRY_IMMEDIATE;
			ACSD_DFSR("IMMEDIATE DFS Reentry required.\n");
		} else if (ctx->window[W_DEFERRED].p.threshold &&
			(dfsr_sw_sum(&ctx->window[W_DEFERRED]) >
				ctx->window[W_DEFERRED].p.threshold)) {
			ctx->reentry_type = DFS_REENTRY_DEFERRED;
			/* update current counters */
			if (acs_dfsr_counter_update(ctx, if_name) != BCME_OK) {
				ACSD_DFSR("Failed to fetch interface counters for '%s'\n", if_name);
			}
			ACSD_DFSR("DEFERRED DFS Reentry required.\n");
		}
	}
	ctx->channel = channel;
	return ctx->reentry_type;
}

dfsr_reentry_type_t
acs_dfsr_set(dfsr_context_t *ctx, chanspec_t channel, const char *caller)
{
	 if (!acs_dfsr_enabled(ctx)) return DFS_REENTRY_NONE;

	ACSD_DFSR("Set Reentry to channel 0x%4x (%s) requested by %s\n", channel, wf_chspec_ntoa(channel, chanspecbuf), caller);

	ctx->reentry_type = DFS_REENTRY_IMMEDIATE;
	ctx->channel = channel;
	return ctx->reentry_type;
}

/*
 * acs_dfsr_counter_update - update counters in DFSR context
 *
 * returns BCME_OK when successful; error status otherwise
 */
static int
acs_dfsr_counter_update(dfsr_context_t *ctx, char *if_name)
{
	char cntbuf[ACSD_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	time_t now = time(0);

	if (acs_get_dfsr_counters(if_name, cntbuf) < 0) {
		ACSD_DFSR("Failed to fetch interface counters for '%s'\n", if_name);
		return BCME_ERROR;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	/* As we need only wlc layer ctrs here, no need to input corerev.  */
	if (wl_cntbuf_to_xtlv_format(NULL, cntbuf, ACSD_WL_CNTBUF_SIZE, 0)
		!= BCME_OK) {
		ACSD_DFSR("wl_cntbuf_to_xtlv_format failed for '%s'\n", if_name);
		return BCME_ERROR;
	}

	if ((wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntbuf)) == NULL) {
		ACSD_DFSR("GET_WLCCNT_FROM_CNTBUF NULL for '%s'\n", if_name);
		return BCME_ERROR;
	}

	ACSD_DEBUG("DFSR ACTIVITY window %u second threshold %u, actual %u, i/o %u\n",
		ctx->window[W_ACTIVITY].p.seconds,  ctx->window[W_ACTIVITY].p.threshold,
		dfsr_sw_sum(&ctx->window[W_ACTIVITY]),
		(wlc_cnt->txbyte - ctx->prev_bytes_tx) +
		(wlc_cnt->rxbyte - ctx->prev_bytes_rx));

	if (ctx->prev_bytes_tx + ctx->prev_bytes_rx) {
		dfsr_sw_add(&ctx->window[W_ACTIVITY], now,
		(wlc_cnt->txbyte - ctx->prev_bytes_tx) +
		(wlc_cnt->rxbyte - ctx->prev_bytes_rx));
	}
	ctx->prev_bytes_tx = wlc_cnt->txbyte;
	ctx->prev_bytes_rx = wlc_cnt->rxbyte;

	return BCME_OK;
}

/*
 * acs_dfsr_activity_update() - update channel activity window counts, trigger reentry if needed.
 *
 * Intended to be called periodically, this function updates the activity window counts and checks
 * whether we have gone below threshold. If so, and a deferred reentry is requested, make that
 * an immediate reentry.
 *
 * Returns the reentry type in effect.
 */
dfsr_reentry_type_t
acs_dfsr_activity_update(dfsr_context_t *ctx, char *if_name)
{
	if (!acs_dfsr_enabled(ctx)) return DFS_REENTRY_NONE;

	if (ctx->reentry_type != DFS_REENTRY_DEFERRED) {
		goto exit;
	}

	if (acs_dfsr_counter_update(ctx, if_name) != BCME_OK) {
		ACSD_DFSR("Failed to fetch interface counters for '%s'\n", if_name);
		goto exit;
	}

	if ((dfsr_sw_sum(&ctx->window[W_ACTIVITY]) <
			ctx->window[W_ACTIVITY].p.threshold)) {

		ACSD_DFSR("Channel Inactive (io=%u in the last %u seconds),"
			" requesting IMMEDIATE DFS Re-entry.\n",
			dfsr_sw_sum(&ctx->window[W_ACTIVITY]),
			ctx->window[W_ACTIVITY].p.seconds);

		ctx->reentry_type = DFS_REENTRY_IMMEDIATE;
	}
exit:
	return ctx->reentry_type;
}

/*
 * acs_dfsr_reentry_done() - indication that the DFS reentry has been performed.
 *
 * ctx:	DFS Reentry context
 *
 * This function should be called by acsd once a DFS Reentry has been done, in order to re-arm
 * the DFS reentry windows and return to initial state.
 */
void
acs_dfsr_reentry_done(dfsr_context_t *ctx)
{
	if (!acs_dfsr_enabled(ctx)) return;

	if (ctx->reentry_type == DFS_REENTRY_IMMEDIATE) {
		ctx->reentry_count++;
		ACSD_DFSR("DFS Reentry #%u done.\n", ctx->reentry_count);

		dfsr_sw_clear(&ctx->window[W_IMMEDIATE]);
		dfsr_sw_clear(&ctx->window[W_DEFERRED]);
		ctx->reentry_type = DFS_REENTRY_NONE;
	}
}

/*
 * acs_dfsr_reentry_type() - Check what the current DFS Reentry type is.
 *
 * ctx:	DFS Reentry context
 *
 * The return value is a dfsr_reentry_type_t enum indicating the DFS reentry type in effect:
 *	DFS_REENTRY_NONE:	No DFS Reentry is needed
 *	DFS_REENTRY_IMMEDIATE:	Immediate DFS Reentry is requested.
 *	DFS_REENTRY_DEFERRED:	DFS Reentry is deferred, to occur when the channel is idle.
 */
dfsr_reentry_type_t
acs_dfsr_reentry_type(dfsr_context_t *ctx)
{
	if (!acs_dfsr_enabled(ctx)) return DFS_REENTRY_NONE;
	return ctx->reentry_type;
}

/*
 * acs_dfsr_set_reentry_type() - set dfsr reentry type.
 *
 * ctx:	DFS Reentry context
 * type: DFS Rentry type
 *
 * Returns BCME_OK when successful; error status Otherwise
 */
int acs_dfsr_set_reentry_type(dfsr_context_t *ctx, int type)
{
	if (!acs_dfsr_enabled(ctx)) {
		return BCME_ERROR;
	}
	ctx->reentry_type = type;
	return BCME_OK;
}

/*
 * acs_dfsr_dump() - debug dump function to show our internals.
 *
 * ctx:		address of our context (NULL allowed).
 * buf:		address of a buffer where to return the human readable dump
 * buflen:	size of the buffer.
 *
 * This function writes debug/status information in a human readable form to the return buffer.
 */
int
acs_dfsr_dump(dfsr_context_t *ctx, char *buf, unsigned buflen)
{
	unsigned retlen;

	retlen = acs_snprintf(buf, buflen, "DFS Reentry is %sabled\n",
		(acs_dfsr_enabled(ctx)) ? "en":"dis");

	if (acs_dfsr_enabled(ctx)) {
		int i;

		retlen += acs_snprintf(buf+retlen, buflen-retlen,
			"Channel switches      : %5u\n"
			"DFS Re-Entry count    : %5u\n"
			"Selected chanspec     : 0x%4x (%s) (Channel %d)\n"
			"Last Channel TX count : %u bytes\n"
			"Last Channel RX count : %u bytes\n"
			"Re-Entry request type : %s\n",
			ctx->switch_count,
			ctx->reentry_count,
			ctx->channel,
			wf_chspec_ntoa(ctx->channel, chanspecbuf),
			CHSPEC_CHANNEL(ctx->channel),
			ctx->prev_bytes_tx,
			ctx->prev_bytes_rx,
			(ctx->reentry_type == DFS_REENTRY_IMMEDIATE) ? "IMMEDIATE" :
			(ctx->reentry_type == DFS_REENTRY_DEFERRED) ? "DEFERRED" : "NONE");

		retlen += acs_snprintf(buf+retlen, buflen-retlen, "%20s %10s %10s %10s\n",
			"Window", "Seconds", "Threshold", "Actual");

		for (i = 0; i < W_COUNT; ++i) {
			dfsr_window_t *w = &ctx->window[i];

			retlen += acs_snprintf(buf+retlen, buflen-retlen,
				"%20s %10u %10u %10u\n",
				dfsr_window_name(i), w->p.seconds, w->p.threshold, dfsr_sw_sum(w));
		}
	}
	return retlen + 1;
}

/* acs_bgdfs_sw_add() - add frame count to sliding window */
void acs_bgdfs_sw_add(dfsr_context_t *ctx, time_t now, uint32_t frames)
{
	dfsr_sw_add(&ctx->window[W_BGDFS], now, frames);
}

sw_counter acs_bgdfs_sw_sum(dfsr_context_t *ctx)
{
	return dfsr_sw_sum(&ctx->window[W_BGDFS]);
}
