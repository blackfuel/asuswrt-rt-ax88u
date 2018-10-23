/*
 * WPS device infomation
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
 * $Id: wps_devinfo.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef __WPS_DEVICE_INFO_H__
#define __WPS_DEVICE_INFO_H__

#include <wps_dh.h>

/* data structure to hold Enrollee and Registrar information */
typedef struct {
	uint8   version;
	uint8   uuid[SIZE_16_BYTES];
	uint8   macAddr[SIZE_6_BYTES];
	char    deviceName[SIZE_32_BYTES+1];
	uint16  primDeviceCategory;
	uint32  primDeviceOui;
	uint16  primDeviceSubCategory;
	uint16  authTypeFlags;
	uint16  encrTypeFlags;
	uint8   connTypeFlags;
	uint16  configMethods;
	uint8   scState;
	bool    selRegistrar;
	char    manufacturer[SIZE_64_BYTES+1];
	char    modelName[SIZE_32_BYTES+1];
	char    modelNumber[SIZE_32_BYTES+1];
	char    serialNumber[SIZE_32_BYTES+1];
	uint8   rfBand;
	uint32  osVersion;
	uint32  featureId;
	uint16  assocState;
	uint16  devPwdId;
	uint16  configError;
	bool    b_ap;
	char    ssid[SIZE_SSID_LENGTH];
	char    keyMgmt[SIZE_20_BYTES+1];
	char    nwKey[SIZE_64_BYTES+1];
	uint16  auth;
	uint16  wep;
	uint16  wepKeyIdx;
	uint16  crypto;
	uint16  reqDeviceCategory;
	uint32  reqDeviceOui;
	uint16  reqDeviceSubCategory;
	uint8   version2;
	uint8   settingsDelayTime;
	bool    b_reqToEnroll;
	bool    b_nwKeyShareable;
#ifdef WFA_WPS_20_TESTBED
	char    dummy_ssid[SIZE_SSID_LENGTH];
	bool    b_zpadding;
	bool    b_zlength;
	bool    b_mca;
	int     nattr_len;
	char    nattr_tlv[SIZE_128_BYTES];
#endif /* WFA_WPS_20_TESTBED */
	bool	b_oob_m2;
	int     authorizedMacs_len;
	char    authorizedMacs[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM];

	uint8   transport_uuid[SIZE_16_BYTES]; /* Used for WCN-NET VP */

	char    *pairing_auth_str;

	/* Run time data */
	int     sc_mode;
	bool    configap;
	char    pin[SIZE_64_BYTES+1]; /* String format */
	int     flags;
	DH      *DHSecret;
	uint8   pre_nonce[SIZE_128_BITS];
	uint8   pre_privkey[SIZE_PUB_KEY];
	uint8   peerMacAddr[SIZE_6_BYTES];
	uint8   pbc_uuids[SIZE_16_BYTES * 2];
	uint8   pub_key_hash[SIZE_160_BITS];
	uint16  apchannel;

	void    *mp_tlvEsM7Ap;
	void    *mp_tlvEsM7Sta;
	void    *mp_tlvEsM8Ap;
	void    *mp_tlvEsM8Sta;
#ifdef SECONDARY_DEVICE_TYPE
		uint16 secDeviceCategory;
		uint16 secDeviceSubCategory;
		uint32 secDeviceOui;
#endif // endif

} DevInfo;

typedef enum {
	WPS_WL_AKM_NONE = 0,
	WPS_WL_AKM_PSK,
	WPS_WL_AKM_PSK2,
	WPS_WL_AKM_BOTH
} WPS_WL_AKM_E;

#define DEVINFO_FLAG_PRE_NONCE		0x1
#define DEVINFO_FLAG_PRE_PRIV_KEY	0x2

/* Functions */
DevInfo *devinfo_new();
void devinfo_delete(DevInfo *dev_inf);

uint16 devinfo_getKeyMgmtType(DevInfo *inf);

#endif /* __WPS_DEVICE_INFO_H__ */
