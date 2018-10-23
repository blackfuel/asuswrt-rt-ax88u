/*
 * WPS API
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
 * $Id: wpsapi.h 593315 2015-10-16 05:11:31Z $
 */

#ifndef _WPSAPI_
#define _WPSAPI_

#ifdef __cplusplus
extern "C" {
#endif // endif

#include <reg_protomsg.h>
#include <wps_devinfo.h>

typedef struct {
	void *bcmwps;

	RegSM *mp_regSM;
	EnrSM *mp_enrSM;

	bool mb_initialized;
	bool mb_stackStarted;

	DevInfo *dev_info;

	bool b_UpnpDevGetDeviceInfo;
} WPSAPI_T;

void * wps_init(void *bcmwps, DevInfo *ap_devinfo);
uint32 wps_deinit(WPSAPI_T *g_mc);

uint32 wps_ProcessBeaconIE(char *ssid, uint8 *macAddr, uint8 *p_data, uint32 len);
uint32 wps_ProcessProbeReqIE(uint8 *macAddr, uint8 *p_data, uint32 len);
uint32 wps_ProcessProbeRespIE(uint8 *macAddr, uint8 *p_data, uint32 len);

int wps_getenrState(void *mc_dev);
int wps_getregState(void *mc_dev);

uint32 wps_sendStartMessage(void *bcmdev, TRANSPORT_TYPE trType);
int wps_get_upnpDevSSR(WPSAPI_T *g_mc, void *p_cbData, uint32 length, CTlvSsrIE *ssrmsg);
int wps_upnpDevSSR(WPSAPI_T *g_mc, CTlvSsrIE *ssrmsg);

int wps_getProcessStates();
void wps_setProcessStates(int state);
void wps_setStaDevName(char *str);
void wps_setPinFailInfo(uint8 *mac, char *name, char *state);
uint32 wps_createM8StaPskKey(void *g_mc);

#ifdef __cplusplus
}
#endif // endif

#endif /* _WPSAPI_ */
