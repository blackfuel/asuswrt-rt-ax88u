/*
 * WPS Registratar API
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
 * $Id: wps_apapi.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _WPS_AP_API_H_
#define _WPS_AP_API_H_

#include <typedefs.h>
#include <info.h>

typedef enum {
	SEARCH_ONLY,
	SEARCH_ENTER
} sta_lookup_mode_t;

/* ap eap state machine states */
typedef enum {
	INIT = 0,
	EAPOL_START_SENT,
	EAP_START_SEND,
	PROCESSING_PROTOCOL,
	REG_SUCCESSFUL,
	REG_FAILED,
	EAP_TIMEOUT,
	EAP_FAILURED
} ap_eap_state_t;

typedef enum {
	WPS_PROC_STATE_INIT = 0,
	WPS_PROC_STATE_PROCESSING,
	WPS_PROC_STATE_SUCCESS,
	WPS_STATUS_MSGERR,
	WPS_STATUS_TIMEOUT
} WPS_PROC_STATE_E;

#define EAP_MAX_RESEND_COUNT 5

int wps_get_mode(void *mc_dev);
int wps_processMsg(void *mc_dev, void *inBuffer, uint32 in_len, void *outBuffer, uint32 *out_len,
	TRANSPORT_TYPE m_transportType);
uint32 wpsap_start_enrollment(void *mc_dev, char *ap_pin);
uint32 wpsap_start_enrollment_devpwid(void *mc_dev, char *ap_pin, uint8 *pub_key_hash,
	uint16 devicepwid);
uint32 wpsap_start_registration(void *mc_dev, char *sta_pin);
uint32 wpsap_start_registration_devpwid(void *mc_dev, char *sta_pin, uint8 *pub_key_hash,
	uint16 devicepwid);
unsigned char * wps_get_mac_income(void *mc_dev);
unsigned char *wps_get_mac(void *mc_dev);
uint8 wps_get_version2(void *mc_dev);
bool ap_api_is_recvd_m2d(void *mc_dev);

#endif /* _WPS_AP_API_H_ */
