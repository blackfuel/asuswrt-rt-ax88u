/*
 * wl server declarations
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
 * $Id: wlu_server_shared.h 674525 2016-12-09 04:17:19Z $
 */

#ifndef _wlu_server_shared_h
#define _wlu_server_shared_h

	extern int wl_ioctl(void *wl, int cmd, void *buf, int len, bool set);

	extern int dhd_ioctl(void *dhd, int cmd, void *buf, int len, bool set);

#ifdef RWLASD
/* streams' buffers */
BYTE *xcCmdBuf = NULL, *parmsVal = NULL;
BYTE *trafficBuf = NULL, *respBuf = NULL;
struct timeval *toutvalp = NULL;
#endif // endif

#define POLLING_TIME      			200
#define DONGLE_TX_FRAME_SIZE   		1024
#define MESSAGE_LENGTH				1024
#define MAX_SHELL_FILE_LENGTH       50
#define MAX_IOVAR				10000
int remote_type = NO_REMOTE;
rem_ioctl_t *g_rem_ptr;

extern int wl_ioctl(void *wl, int cmd, void *buf, int len, bool set);

/* Function prototypes from shellpoc_linux.c/shell_ce.c */
extern int rwl_create_dir(void);
extern int remote_shell_execute(char *buf_ptr, void *wl);
extern int remote_shell_get_resp(char* shell_fname, void *wl);
extern void rwl_wifi_find_server_response(void *wl, dot11_action_wifi_vendor_specific_t *rec_frame);
extern dot11_action_wifi_vendor_specific_t *rwl_wifi_allocate_actionframe(void);

/* Common code for serial and wifi */
#if defined(RWL_DONGLE) || defined(RWL_WIFI) || defined(RWL_SERIAL)
typedef struct rem_packet {
	rem_ioctl_t rem_cdc;
	uchar message[MESSAGE_LENGTH];
} rem_packet_t;
#define REMOTE_PACKET_SIZE sizeof(rem_packet_t)

rem_packet_t *g_rem_pkt_ptr;
rem_packet_t g_rem_pkt;
#endif // endif

static struct ether_addr rwlea;

static union {
	uchar bufdata[WLC_IOCTL_MAXLEN];
	uint32 alignme;
} bufstruct_wlu;
static uchar* rwl_buf = (uchar*) &bufstruct_wlu.bufdata;
extern int need_speedy_response;

#endif /* _wlu_server_shared_h_ */
