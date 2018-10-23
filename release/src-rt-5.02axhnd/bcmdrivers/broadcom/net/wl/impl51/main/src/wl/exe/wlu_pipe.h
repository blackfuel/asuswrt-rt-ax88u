/*
 * OS independent declarations
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
 * $Id: wlu_pipe.h 674525 2016-12-09 04:17:19Z $
 */
#ifndef _wlu_pipe_h
#define _wlu_pipe_h

/* Function prototypes defined in wlu_pipe.c */

/* Macros to access globals */
extern char *g_rwl_device_name_serial;
#define rwl_get_serial_port_name()		(g_rwl_device_name_serial)
#define rwl_set_serial_port_name(name) 	(g_rwl_device_name_serial = name)
extern char *g_rwl_buf_mac;
#define rwl_get_wifi_mac()				(g_rwl_buf_mac)

extern int remote_CDC_tx(void *wl, uint cmd, uchar *buf, uint buf_len,
uint data_len, uint flags, int debug);
extern rem_ioctl_t *remote_CDC_rx_hdr(void *remote, int debug);
extern int remote_CDC_rx(void *wl, rem_ioctl_t *rem_ptr, uchar *readbuf, uint buflen, int debug);

extern void* rwl_open_pipe(int remote_type, char *port, int ReadTotalTimeout, int debug);
extern int rwl_close_pipe(int remote_type, void* hndle);

#ifdef RWL_SOCKET
extern int rwl_sockconnect(int SockDes, struct sockaddr *servAddr, int size);
#endif /* RWL_SOCKET */

extern int remote_CDC_DATA_wifi_rx_frag(void *wl, rem_ioctl_t *rem_ptr, uint input_len,
void *input, bool shell);
extern int remote_CDC_DATA_wifi_rx(void *wl, struct dot11_action_wifi_vendor_specific *rec_frame);
extern int
remote_CDC_wifi_tx(void *wl, uint cmd, uchar *buf, uint buf_len, uint data_len, uint flags);
extern void rwl_wifi_server_response(void *wl, struct dot11_action_wifi_vendor_specific *rec_frame);
extern int rwl_find_remote_wifi_server(void *wl, char *id);
extern int rwl_wifi_purge_actionframes(void *wl);
extern void rwl_wifi_swap_remote_type(int flag);
extern void remote_wifi_ser_init_cmds(void *wl);
#endif /* _wlu_pipe_h */
