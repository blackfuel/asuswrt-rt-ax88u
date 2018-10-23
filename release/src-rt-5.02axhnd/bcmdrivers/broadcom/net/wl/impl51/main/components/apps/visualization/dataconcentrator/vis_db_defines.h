/*
 * Visualization system data concentrator database defines header
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
 * $Id: vis_db_defines.h 672681 2016-11-29 11:26:01Z $
 */
#ifndef _VIS_DB_DEFINES_H_
#define _VIS_DB_DEFINES_H_

/* Database folder Name */
#define DB_FOLDER_NAME	"/tmp/db"

/* Database file Name */
#define DB_NAME			"visdata.db"

/* Default Database Tables */
#define TABLE_CONFIG		"config"
#define TABLE_DUTDETAILS	"dutdetails"
#define TABLE_SCAN			"scanresult"
#define TABLE_ASSOCSTA		"assocsta"
#define TABLE_CHANNELSTATS	"ChannelStats"
#define TABLE_AMPDUSTATS	"AMPDUStats"
#define TABLE_GRAPHS		"graphs"
#define TABLE_DUTSETTINGS	"dutsettings"
#define TABLE_RRM_STA_STATS	"rrmstastats"
#define TABLE_RRM_STA_ADV_STATS	"rrmstaadvstats"
#define MAX_DEFAULT_TABLES	10

/* Config table defines */
#define COL_CONFIG_ROWID		"rowID"
#define COL_CONFIG_INTERVAL		"interval"
#define COL_CONFIG_DBSIZE		"dbsize"
#define COL_CONFIG_ISSTART		"isstart"
#define COL_CONFIG_GATEWAY_IP	"gatewayip"
#define COL_CONFIG_ISOVERWRITEDB	"isoverwrtdb"
#define COL_CONFIG_ISAUTOSTART		"autostart"
#define COL_CONFIG_WEEKDAYS			"weekdays"
#define COL_CONFIG_FROMTM			"fromtm"
#define COL_CONFIG_TOTM				"totm"
/* Config Table defines ends */

/* DUT Details table defines */
#define COL_DUT_ROWID			"rowID"
#define COL_DUT_ISAP			"IsAP"
#define COL_DUT_BSSID			"BSSID"
#define COL_DUT_SSID			"SSID"
#define COL_DUT_MAC				"MAC"
#define COL_DUT_CHANNEL			"Channel"
#define COL_DUT_BANDWIDTH		"bandwidth"
#define COL_DUT_RSSI			"RSSI"
#define COL_DUT_CONTROLCH		"ControlCH"
#define COL_DUT_SNR			"SNR"
#define COL_DUT_NOISE			"Noise"
#define COL_DUT_BAND		"band"
#define COL_DUT_SPEED			"Speed"
#define COL_DUT_STANDARDS		"Standards"
#define COL_DUT_MCASTRSN		"MCastRSN"
#define COL_DUT_UCASTRSN		"UCastRSN"
#define COL_DUT_AKMRSN			"AKMRSN"
#define COL_DUT_ISENABLED		"enabled"
#define COL_DUT_ERRINFO			"errinfo"
/* DUT Details table defines ends */

/* SCAN Details table defines */
#define COL_SCAN_ROWID			"rowID"
#define COL_SCAN_TIME			"Timestamp"
#define COL_SCAN_SSID			"SSID"
#define COL_SCAN_BSSID			"BSSID"
#define COL_SCAN_RSSI			"RSSI"
#define COL_SCAN_CHANNEL		"Channel"
#define COL_SCAN_CONTROLCH		"ControlCH"
#define COL_SCAN_SNR			"SNR"
#define COL_SCAN_NOISE			"Noise"
#define COL_SCAN_BANDWIDTH		"Bandwidth"
#define COL_SCAN_SPEED			"Speed"
#define COL_SCAN_STANDARDS		"Standards"
#define COL_SCAN_MCASTRSN		"MCastRSN"
#define COL_SCAN_UCASTRSN		"UCastRSN"
#define COL_SCAN_AKMRSN			"AKMRSN"
/* SCAN Details table defines ends */

/* Associated STA Details table defines */
#define COL_ASSOC_ROWID			"rowID"
#define COL_ASSOC_TIME			"TimeStamp"
#define COL_ASSOC_MAC			"MAC"
#define COL_ASSOC_RSSI			"RSSI"
#define COL_ASSOC_PHYRATE		"PhyRate"
/* Associated STA Details table defines ends */

/* Channel Stats Details table defines */
#define COL_CHSTATS_ROWID			"rowID"
#define COL_CHSTATS_TIME			"TimeStamp"
#define COL_CHSTATS_CHANNEL			"Channel"
#define COL_CHSTATS_TX		"tx"
#define COL_CHSTATS_INBSS	"inbss"
#define COL_CHSTATS_OBSS	"obss"
#define COL_CHSTATS_NOCAT	"nocat"
#define COL_CHSTATS_NOPKT	"nopkt"
#define COL_CHSTATS_DOZE	"doze"
#define COL_CHSTATS_TXOP	"txop"
#define COL_CHSTATS_GOODTX	"goodtx"
#define COL_CHSTATS_BADTX	"badtx"
#define COL_CHSTATS_GLITCH	"glitch"
#define COL_CHSTATS_BADPLCP	"plcp"
#define COL_CHSTATS_KNOISE	"noise"
#define COL_CHSTATS_IDLE	"idle"

/* Channel Stats Details table defines ends */

/* Graphs table defines */
#define COL_GRAPH_ROWID			"rowID"
#define COL_GRAPH_TAB			"tabheading"
#define COL_GRAPH_NAME			"Name"
#define COL_GRAPH_HEADING		"Heading"
#define COL_GRAPH_PLOTWITH		"PlotWith"
#define COL_GRAPH_TYPE			"Type"
#define COL_GRAPH_PERSTA		"PerSTA"
#define COL_GRAPH_BARHEADING	"BarHeading"
#define COL_GRAPH_XAXISNAME		"XAxisName"
#define COL_GRAPH_YAXISNAME		"YAxisName"
#define COL_GRAPH_TABLE			"TableName"
#define COL_GRAPH_XCOLUMN		"XSSColumn"
#define COL_GRAPH_YCOLUMN		"YColumn"
#define COL_GRAPH_ENABLE		"Enable"
/* Graphs table defines ends */

/* Common Graph tables column names define starts */
#define COL_CGRAPH_ROWID		"rowID"
#define COL_CGRAPH_GRAPHROWID	"GraphrowID"
#define COL_CGRAPH_MAC			"MAC"
#define COL_CGRAPH_TIME			"TimeStamp"
#define COL_CGRAPH_XAXIS		"XAxis"
#define COL_CGRAPH_YAXIS		"YAxis"
/* Common Graph tables column names define ends */

/* AMPDU Stats table defines */
#define COL_AMPDU_ROWID				"rowID"
#define COL_AMPDU_TIME				"TimeStamp"
#define COL_AMPDU_MCS				"MCS"
#define COL_AMPDU_TXMCS				"TXMCS"
#define COL_AMPDU_TXMCSPERCENT		"TXMCSPERCENT"
#define COL_AMPDU_TXMCSSGI			"TXMCSSGI"
#define COL_AMPDU_TXMCSSGIPERCENT	"TXMCSSGIPERCENT"
#define COL_AMPDU_RXMCS				"RXMCS"
#define COL_AMPDU_RXMCSPERCENT		"RXMCSPERCENT"
#define COL_AMPDU_RXMCSSGI			"RXMCSSGI"
#define COL_AMPDU_RXMCSSGIPERCENT	"RXMCSSGIPERCENT"
#define COL_AMPDU_TXVHT				"TXVHT"
#define COL_AMPDU_TXVHTPER			"TXVHTPER"
#define COL_AMPDU_TXVHTPERCENT		"TXVHTPERCENT"
#define COL_AMPDU_TXVHTSGI			"TXVHTSGI"
#define COL_AMPDU_TXVHTSGIPER		"TXVHTSGIPER"
#define COL_AMPDU_TXVHTSGIPERCENT	"TXVHTSGIPERCENT"
#define COL_AMPDU_RXVHT				"RXVHT"
#define COL_AMPDU_RXVHTPER			"RXVHTPER"
#define COL_AMPDU_RXVHTPERCENT		"RXVHTPERCENT"
#define COL_AMPDU_RXVHTSGI			"RXVHTSGI"
#define COL_AMPDU_RXVHTSGIPER		"RXVHTSGIPER"
#define COL_AMPDU_RXVHTSGIPERCENT	"RXVHTSGIPERCENT"
#define COL_AMPDU_MPDUDENS			"MPDUDENS"
#define COL_AMPDU_MPDUDENSPERCENT	"MPDUDENSPERCENT"
/* AMPDU Stats table defines ends */

/* DUT settings table defines starts */
#define COL_DUTSET_ROWID	"rowID"
#define COL_DUTSET_SCAN		"scan"

/* RRM STA Side statistics table */
#define COL_RRMSTATS_ROWID	"rowID"
#define COL_RRMSTATS_MAC	"MAC"
#define COL_RRMSTATS_TIME	"TimeStamp"
#define COL_RRMSTATS_GENERATION	"generation"
#define COL_RRMSTATS_TXSTREAM	"txstream"
#define COL_RRMSTATS_RXSTREAM	"rxstream"
#define COL_RRMSTATS_RATE	"rate"
#define COL_RRMSTATS_IDLE	"idle"

/* RRM Advanced STA side statistics table */
#define COL_RRMADVSTATS_ROWID		"rowID"
#define COL_RRMADVSTATS_MAC		"MAC"
#define COL_RRMADVSTATS_TIME		"TimeStamp"
#define COL_RRMADVSTATS_TXOP		"txop"
#define COL_RRMADVSTATS_PKTREQ		"pktreq"
#define COL_RRMADVSTATS_PKTDROP		"pktdrop"
#define COL_RRMADVSTATS_PKTSTORED	"pktstored"
#define COL_RRMADVSTATS_PKTRETRIED	"pktretried"
#define COL_RRMADVSTATS_PKTACKED	"pktacked"

#endif /* _VIS_DB_DEFINES_H_ */
