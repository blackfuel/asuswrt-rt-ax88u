/*
 * Linux Visualization System common utility function implementation
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
 * $Id: vis_shared_util.c 555336 2015-05-08 09:52:04Z $
 */

#include "vis_shared_util.h"

/* generic IOVAR handler
 * Any New IOVAR added will have one entry here
 */
iovar_handler_t iovar_handler[] = {
	{ IOVAR_NAME_CHANNELTSTATS, "Channel Statistics", IOVAR_NAME_CHANNELTSTATS,
	GRAPH_TYPE_BAR, 0,
	"Congestion(Wi-Fi)", "Channels", "Percentage", 1},

	{ IOVAR_NAME_AMPDUTXWITHOUTSGI, "AMPDU Statistics", IOVAR_NAME_AMPDUTXWITHOUTSGI,
	GRAPH_TYPE_BAR, 0,
	"AMPDU Tx MCS Without SGI", "Tx MCS Rates", "% Packets", 0},

	{ IOVAR_NAME_CHANIM, "Chanim Statistics", IOVAR_NAME_CHANIM,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	IOVAR_NAME_CHANIM, "Time", "Count", 1},

	{ IOVAR_NAME_RXCRSGLITCH, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Rx CRS Glitches", "Time", "Count", 1},

	{ IOVAR_NAME_BADPLCP, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Bad PLCP", "Time", "Count", 1},

	{ IOVAR_NAME_BADFCS, "Glitch Counter Statistics", IOVAR_NAME_RXCRSGLITCH,
	GRAPH_TYPE_LINE_AGAINST_TIME, 0,
	"Bad FCS", "Time", "Count", 1},

	{ IOVAR_NAME_PACKETREQUESTED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETREQUESTED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETSTORED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETSTORED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETDROPPED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETDROPPED, "Time", "Count", 1},

	{ IOVAR_NAME_PACKETRETRIED, "Packet Queue Statistics", IOVAR_NAME_PACKETREQUESTED,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PACKETRETRIED, "Time", "Count", 1},

	{ IOVAR_NAME_QUEUEUTILIZATION, "Packet Queue Statistics", IOVAR_NAME_QUEUEUTILIZATION,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_QUEUEUTILIZATION, "Time", "Count", 1},

	{ IOVAR_NAME_QUEUELENGTH, "Packet Queue Statistics", IOVAR_NAME_QUEUEUTILIZATION,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_QUEUELENGTH, "Time", "Count", 1},

	{ IOVAR_NAME_DATATHROUGHPUT, "Packet Queue Statistics", IOVAR_NAME_DATATHROUGHPUT,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	"Data Throughput", "Time", "Mbits/s", 1},

	{ IOVAR_NAME_PHYSICALRATE, "Packet Queue Statistics", IOVAR_NAME_DATATHROUGHPUT,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	"Physical Rate", "Time", "Mbits/s", 1},

	{ IOVAR_NAME_RTSFAIL, "Packet Queue Statistics", IOVAR_NAME_RTSFAIL,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_RTSFAIL, "Time", "Count", 1},

	{ IOVAR_NAME_RTRYDROP, "Packet Queue Statistics", IOVAR_NAME_RTSFAIL,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_RTRYDROP, "Time", "Count", 1},

	{ IOVAR_NAME_PSRETRY, "Packet Queue Statistics", IOVAR_NAME_PSRETRY,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_PSRETRY, "Time", "Count", 1},

	{ IOVAR_NAME_ACKED, "Packet Queue Statistics", IOVAR_NAME_PSRETRY,
	GRAPH_TYPE_LINE_AGAINST_TIME, 1,
	IOVAR_NAME_ACKED, "Time", "Count", 1},
};

/* Gets the iovar structure handle from the iovar name */
iovar_handler_t*
find_iovar(const char *name)
{
	iovar_handler_t *tmpiovar;

	for (tmpiovar = iovar_handler; tmpiovar->name && strcmp(tmpiovar->name, name); tmpiovar++);
	if (tmpiovar->name == NULL) {
		tmpiovar = NULL;
	}

	return tmpiovar;
}

/* Get the number of iovar's */
int
get_iovar_count()
{
	return (sizeof(iovar_handler)/sizeof(iovar_handler_t));
}

/* get IOVAR handle given the index */
iovar_handler_t*
get_iovar_handler(int idx)
{
	iovar_handler_t		*tmpiovar;
	int			i = 0;

	for (tmpiovar = iovar_handler; tmpiovar->name && i != idx; tmpiovar++, i++);
	if (tmpiovar->name == NULL) {
		tmpiovar = NULL;
	}

	return tmpiovar;
}
