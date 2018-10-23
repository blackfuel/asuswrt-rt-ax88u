/*
 * Portability
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
 * $Id: portability.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _WPS_PORTAB_
#define _WPS_PORTAB_

#include <wpstypes.h>

#ifndef __cplusplus
#include "stdio.h"
#include "stdlib.h"
#include "typedefs.h"

#ifdef true
#undef true
#endif // endif
#define true 1

#ifdef false
#undef false
#endif // endif
#define false 0

char * alloc_init(int size);
#define new(a) (a *)alloc_init(sizeof(a))
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif // endif

#include "string.h"

/* Byte swapping functions. To be implemented by application. */
uint32 WpsHtonl(uint32 intlong);
uint16 WpsHtons(uint16 intshort);
uint32 WpsHtonlPtr(uint8 * in, uint8 * out);
uint16 WpsHtonsPtr(uint8 * in, uint8 * out);
uint32 WpsNtohl(uint8* intlong);
uint16 WpsNtohs(uint8 * intshort);
void WpsSleep(uint32 seconds);
void WpsSleepMs(uint32 ms);

typedef struct {
	char ssid[SIZE_SSID_LENGTH];
	uint32 ssidLen;
	char keyMgmt[SIZE_20_BYTES+1];
	char nwKey[SIZE_64_BYTES+1];
	uint32 nwKeyLen;
	uint32 encrType;
	uint16 wepIndex;
	bool nwKeyShareable;
} WpsEnrCred;

typedef struct {
	uint8  pub_key_hash[SIZE_160_BITS];
	uint16 devPwdId;
	uint8  pin[SIZE_32_BYTES];
	int    pin_len;
} WpsOobDevPw;

typedef struct {
	WpsOobDevPw oobdevpw;
	uint8 uuid[SIZE_16_BYTES];
	char ssid[SIZE_SSID_LENGTH];
	uint8 rfband;
	uint16 apchannel;
	uint8 bssid[SIZE_6_BYTES];
} WpsCho;

bool wps_isSRPBC(IN uint8 *p_data, IN uint32 len);
bool wps_isWPSS(IN uint8 *p_data, IN uint32 len);
bool wl_is_wps_ie(uint8 **wpaie, uint8 **tlvs, uint *tlvs_len);
bool is_wps_ies(uint8* cp, uint len);
bool is_SelectReg_PBC(uint8* cp, uint len);
bool is_ConfiguredState(uint8* cp, uint len);
uint8 *wps_parse_tlvs(uint8 *tlv_buf, int buflen, uint key);
int set_mac_address(char *mac_string, char *mac_bin);
void wps_set_reg_result(uint8 val);
bool wps_isSELR(IN uint8 *p_data, IN uint32 len);

bool wps_isVersion2(uint8 *p_data, uint32 len, uint8 *version2, uint8 *macs);
bool is_wpsVersion2(uint8* cp, uint len, uint8 *version2, uint8 *macs);
bool wps_isAuthorizedMAC(IN uint8 *p_data, IN uint32 len, IN uint8 *mac);
bool is_AuthorizedMAC(uint8* cp, uint len, uint8 *mac);

#ifdef __cplusplus
}
#endif // endif

#endif /* _WPS_PORTAB_ */
