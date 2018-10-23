/*
 * Inband EAP
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
 * $Id: ap_eap_sm.h 742933 2018-01-24 06:52:45Z $
 */

#ifndef _AP_EAP_SM_H_
#define _AP_EAP_SM_H_

#include <eap_defs.h>
#include <ethernet.h>

typedef struct {
	char state; /* current state. */
	char sent_msg_id; /* out wps message ID */
	char recv_msg_id; /* in wps message ID */
	unsigned char sta_mac[ETHER_ADDR_LEN]; /* STA's ethernet address */
	unsigned char bssid[ETHER_ADDR_LEN]; /* incoming bssid */
	unsigned char eap_id; /* identifier of the last request  */
	char msg_to_send[WPS_EAP_DATA_MAX_LENGTH]; /* message to be sent */
	int msg_len;

	/* For fragmented packets */
	char frags_received[WPS_EAP_DATA_MAX_LENGTH]; /* buffer to store all fragmentations  */
	/*
	 * From the Length Field of the first frag.  To be set to 0 when packet is not fragmented
	 * or set to the new value when receiving the begining of the next fragmented packet.
	 */
	int total_bytes_to_recv; /* total bytes of WPS data need to receive */
	int total_received; /* total WPS data received so far */
	/*
	 * After a fragment is sent and acked, the next_frag_to_send pointer
	 * is moved along the msg_to_send buffer. It is reset to the
	 * begining of the buffer after all fragments have been sent.
	 * When this pointer is not at the begining, the receive ap_eap_sm_process_sta_msg
	 * must only accept WPS_FRAG_ACK messages.
	 */
	char *next_frag_to_send; /* pointer to the next fragment to send */
	int frag_size;

	int ap_m2d_len;
	char *ap_m2d_data;
	int flags;

	char *last_sent; /* remember last sent EAPOL packet location */
	int last_sent_len;
	int resent_count;
	unsigned long last_check_time;
	int send_count;
	void *mc_dev;
	char * (*parse_msg)(char *, int, int *);
	unsigned int (*send_data)(char *, uint32);
	int eap_frag_threshold;
	int wps_delay_deauth_ms;
} EAP_WPS_AP_STATE;

#define AP_EAP_SM_AP_M2D_READY	0x1
#define AP_EAP_SM_M1_RECVD	0x2
#define AP_EAP_SM_M2_SENT	0x4

int ap_eap_sm_process_timeout();
int ap_eap_sm_startWPSReg(unsigned char *sta_mac, unsigned char *ap_mac);
uint32 ap_eap_sm_process_sta_msg(char *msg, int msg_len,
	char *authorizedMacs, int authorizedMacs_len);
uint32 ap_eap_sm_init(void *mc_dev, char *mac_sta, char * (*parse_msg)(char *, int, int *),
	unsigned int (*send_data)(char *, uint32), int eap_frag_threshold, int wps_delay_deauth_ms);
uint32 ap_eap_sm_deinit(void);
uint32 ap_eap_sm_sendMsg(char * dataBuffer, uint32 dataLen);
uint32 ap_eap_sm_sendWPSStart(void);
unsigned char *ap_eap_sm_getMac(int mode);
int ap_eap_sm_get_msg_to_send(char **data);
void ap_eap_sm_Failure(int deauthFlag);
int ap_eap_sm_check_timer(uint32 time);
int ap_eap_sm_get_eap_state(void);
int ap_eap_sm_resend_last_sent();

#endif /* _AP_EAP_SM_H_ */
