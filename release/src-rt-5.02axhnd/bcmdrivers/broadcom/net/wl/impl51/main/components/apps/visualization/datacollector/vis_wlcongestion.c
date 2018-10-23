/*
 * chanim statistics for visualization tool
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
 * $Id: vis_wlcongestion.c 734571 2017-12-05 12:50:36Z $
 */
#ifdef WIN32
#include <windows.h>
#endif // endif

#if !defined(TARGETOS_nucleus)
#define CLMDOWNLOAD
#endif // endif

#if defined(__NetBSD__)
#include <typedefs.h>
#endif // endif

/* Because IL_BIGENDIAN was removed there are few warnings that need
 * to be fixed. Windows was not compiled earlier with IL_BIGENDIAN.
 * Hence these warnings were not seen earlier.
 * For now ignore the following warnings
 */
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4761)
#endif // endif

#include <bcmendian.h>
#include "vis_common.h"
#if defined(WLPFN) && defined(linux)
#ifndef TARGETENV_android
#include <unistd.h>
#endif // endif
#endif /* WLPFN */

#ifdef WLEXTLOG
#include <wlc_extlog_idstr.h>
#endif // endif

#include <inttypes.h>
#include <errno.h>

#if defined SERDOWNLOAD || defined CLMDOWNLOAD
#include <sys/stat.h>
#include <trxhdr.h>
#ifdef SERDOWNLOAD
#include <usbrdl.h>
#endif // endif
#include <stdio.h>
#include <errno.h>

#ifndef WIN32
#include <fcntl.h>
#endif /* WIN32 */
#endif /* SERDOWNLOAD || defined CLMDOWNLOAD */

#include "vis_wl.h"
#include "vis_wlcongestion.h"

#define ACS_CHANIM_BUF_LEN (2*1024)
#define ACS_CHANIM_BUF_SMLEN	512

extern long g_timestamp;

static int
wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	err = wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen, FALSE);

	return (err);
}

/* Gets chanim stats for current channel */
static char*
wl_get_curr_chanim_stats(void *wl)
{
	wl_chanim_stats_t param;
	char *curr_data_buf;
	int buflen = ACS_CHANIM_BUF_SMLEN;

	curr_data_buf = (char*)malloc(sizeof(curr_data_buf) * buflen);
	if (curr_data_buf == NULL) {
		VIS_CHANIM("Failed to allocate curr_data_buf of %d bytes\n", buflen);
		return NULL;
	}
	memset(curr_data_buf, 0, buflen);

	param.buflen = htod32(buflen);
	param.count = htod32(WL_CHANIM_COUNT_ONE);
	if (wl_iovar_getbuf(wl, "chanim_stats", &param, sizeof(wl_chanim_stats_t),
		curr_data_buf, buflen) < 0) {
		VIS_CHANIM("failed to get chanim results\n");
		free(curr_data_buf);
		return NULL;
	}

	return curr_data_buf;
}

/* Function to verify the chanim stats data */
static void
wl_check_chanim_stats_list(wl_chanim_stats_t *list)
{
	if (list->buflen == 0) {
		list->version = 0;
		list->count = 0;
	} else if (list->version != WL_CHANIM_STATS_VERSION) {
		VIS_CHANIM("Sorry, your driver has wl_chanim_stats version %d "
			"but this program supports only version %d.\n",
				list->version, WL_CHANIM_STATS_VERSION);
		list->buflen = 0;
		list->count = 0;
	}
}

/* Gets wifi, non wifi  interference */
congestion_list_t*
wl_get_chanim_stats(void *wl)
{
	int err;
	wl_chanim_stats_t *list, *current = NULL;
	wl_chanim_stats_t param;
	chanim_stats_t *stats;
	int i, idle;
	int count;
	int buflen = ACS_CHANIM_BUF_LEN;
	char *data_buf, *curr_data_buf = NULL;
	congestion_list_t *congestionout = NULL;
	uint32 curr_channel = 0, channel;
	bool curr_present = FALSE;

	data_buf = (char*)malloc(sizeof(data_buf) * buflen);
	if (data_buf == NULL) {
		VIS_CHANIM("Failed to allocate data_buf of %d bytes\n", buflen);
		return NULL;
	}
	memset(data_buf, 0, buflen);

	param.buflen = htod32(buflen);
	param.count = htod32(WL_CHANIM_COUNT_ALL);

	if ((err = wl_iovar_getbuf(wl, "chanim_stats", &param, sizeof(wl_chanim_stats_t),
		data_buf, buflen)) < 0) {
		VIS_CHANIM("failed to get chanim results\n");
		free(data_buf);
		return NULL;
	}

	list = (wl_chanim_stats_t*)data_buf;

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);
	wl_check_chanim_stats_list(list);

	count = dtoh32(list->count);

	congestionout = (congestion_list_t*)malloc(sizeof(congestion_list_t) +
		(count * sizeof(congestion_t)));
	if (congestionout == NULL) {
		VIS_CHANIM("Failed to allocate congestionout buffer of size : %d\n", count);
		free(data_buf);
		return NULL;
	}
	congestionout->timestamp = g_timestamp;
	congestionout->length = 0;

	/* Fetch the chanim stats for current channel */
	curr_data_buf = wl_get_curr_chanim_stats(wl);
	if (curr_data_buf) {
		current = (wl_chanim_stats_t*)curr_data_buf;
		current->buflen = dtoh32(current->buflen);
		current->version = dtoh32(current->version);
		current->count = dtoh32(current->count);
		wl_check_chanim_stats_list(current);
		curr_channel = wf_chspec_ctlchan(current->stats->chanspec);
	}

	VIS_CHANIM("Count is : %d\n", (count == 0) ? current->count : count);
	VIS_CHANIM("chanspec   tx   inbss   obss   nocat   nopkt   doze     txop     "
		"goodtx  badtx   glitch   badplcp  knoise  idle\n");

	/*
	 * Fill the chanim stats for current channel.
	 */
	if (current != NULL && current->count == 1) {
		stats = current->stats;
		congestionout->congest[0].channel = curr_channel;
		congestionout->congest[0].tx = stats->ccastats[0];
		congestionout->congest[0].inbss = stats->ccastats[1];
		congestionout->congest[0].obss = stats->ccastats[2];
		congestionout->congest[0].nocat = stats->ccastats[3];
		congestionout->congest[0].nopkt = stats->ccastats[4];
		congestionout->congest[0].doze = stats->ccastats[5];
		congestionout->congest[0].txop = stats->ccastats[6];
		congestionout->congest[0].goodtx = stats->ccastats[7];
		congestionout->congest[0].badtx = stats->ccastats[8];
		congestionout->congest[0].glitchcnt = dtoh32(stats->glitchcnt);
		congestionout->congest[0].badplcp = dtoh32(stats->badplcp);
		congestionout->congest[0].knoise = stats->bgnoise;
		/* As idle is calculated using formula idle = 100 - busy
		 * where busy = inbss+goodtx+badtx+obss+nocat+nopkt.
		 * So, instead of chan_idle use the computed idle
		 */
		idle = 100 - (congestionout->congest[0].inbss +
			congestionout->congest[0].obss +
			congestionout->congest[0].goodtx +
			congestionout->congest[0].badtx +
			congestionout->congest[0].nocat +
			congestionout->congest[0].nopkt);

		congestionout->congest[0].chan_idle = idle;
		congestionout->length++;
		curr_present = TRUE;
		VIS_CHANIM("%4d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			congestionout->congest[0].channel, congestionout->congest[0].tx,
			congestionout->congest[0].inbss, congestionout->congest[0].obss,
			congestionout->congest[0].nocat, congestionout->congest[0].nopkt,
			congestionout->congest[0].doze, congestionout->congest[0].txop,
			congestionout->congest[0].goodtx, congestionout->congest[0].badtx,
			congestionout->congest[0].glitchcnt, congestionout->congest[0].badplcp,
			congestionout->congest[0].knoise, congestionout->congest[0].chan_idle);
	}

	for (i = 0; i < count; i++) {
		stats = &list->stats[i];
		idle = 0;
		channel = CHSPEC_CHANNEL(stats->chanspec);
		/* Skip the already stored chanim stats for current channel */
		if (curr_present && (current != NULL) && (channel == curr_channel)) {
			continue;
		}
		congestionout->congest[congestionout->length].channel = channel;
		congestionout->congest[congestionout->length].tx = stats->ccastats[0];
		congestionout->congest[congestionout->length].inbss = stats->ccastats[1];
		congestionout->congest[congestionout->length].obss = stats->ccastats[2];
		congestionout->congest[congestionout->length].nocat = stats->ccastats[3];
		congestionout->congest[congestionout->length].nopkt = stats->ccastats[4];
		congestionout->congest[congestionout->length].doze = stats->ccastats[5];
		congestionout->congest[congestionout->length].txop = stats->ccastats[6];
		congestionout->congest[congestionout->length].goodtx = stats->ccastats[7];
		congestionout->congest[congestionout->length].badtx = stats->ccastats[8];
		congestionout->congest[congestionout->length].glitchcnt = dtoh32(stats->glitchcnt);
		congestionout->congest[congestionout->length].badplcp = dtoh32(stats->badplcp);
		congestionout->congest[congestionout->length].knoise = stats->bgnoise;
		/* As idle is calculated using formula idle = 100 - busy
		 * where busy = inbss+goodtx+badtx+obss+nocat+nopkt.
		 * So, instead of chan_idle use the computed idle
		 */
		idle = 100 - (congestionout->congest[congestionout->length].inbss +
			congestionout->congest[congestionout->length].obss +
			congestionout->congest[congestionout->length].goodtx +
			congestionout->congest[congestionout->length].badtx +
			congestionout->congest[congestionout->length].nocat +
			congestionout->congest[congestionout->length].nopkt);

		congestionout->congest[congestionout->length].chan_idle = idle;

		VIS_CHANIM("%4d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
			congestionout->congest[congestionout->length].channel,
			congestionout->congest[congestionout->length].tx,
			congestionout->congest[congestionout->length].inbss,
			congestionout->congest[congestionout->length].obss,
			congestionout->congest[congestionout->length].nocat,
			congestionout->congest[congestionout->length].nopkt,
			congestionout->congest[congestionout->length].doze,
			congestionout->congest[congestionout->length].txop,
			congestionout->congest[congestionout->length].goodtx,
			congestionout->congest[congestionout->length].badtx,
			congestionout->congest[congestionout->length].glitchcnt,
			congestionout->congest[congestionout->length].badplcp,
			congestionout->congest[congestionout->length].knoise,
			congestionout->congest[congestionout->length].chan_idle);

		congestionout->length++;
	}

	if (curr_data_buf) {
		free(curr_data_buf);
	}
	free(data_buf);
	return (congestionout);
}
