/*
 * WPS AP
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
 * $Id: wps_ap.h 655918 2016-08-24 07:53:47Z $
 */

#ifndef __WPS_AP_H__
#define __WPS_AP_H__

#define WPS_OVERALL_TIMEOUT	140 /* 120 + 20 for VISTA testing tolerance */
#define WPS_MSG_TIMEOUT	30

#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif // endif

typedef struct {
	int sc_mode;
	int ess_id;
	char ifname[IFNAMSIZ];
	unsigned char mac_ap[6];
	unsigned char mac_sta[6];
	unsigned char *pre_nonce;
	unsigned char *pre_privkey;

	bool config_state;

	void *mc;			/* state machine context */

	unsigned long pkt_count;
	unsigned long pkt_count_prv;

	int wps_state;			/* state machine operating state */
	unsigned long start_time;	/* workspace init time */
	unsigned long touch_time;	/* workspace latest operating time */

	int return_code;

	/* WSC 2.0 */
	bool b_wps_version2;
	bool b_reqToEnroll;
	bool b_nwKeyShareable;
	uint32 authorizedMacs_len;
	uint8 authorizedMacs[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM];
	int eap_frag_threshold;
	int wps_delay_deauth_ms;
} wpsap_wksp_t;

/*
 * implemented in wps_ap.c
 */
uint32 wpsap_osl_eapol_send_data(char *dataBuffer, uint32 dataLen);
char* wpsap_osl_eapol_parse_msg(char *msg, int msg_len, int *len);

#endif /* __WPS_AP_H__ */
