/*
 * WPS header
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
 * $Id: wpsheaders.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _WPS_HEADERS_
#define _WPS_HEADERS_

#include <wpstypes.h>

#include <wpstlvbase.h>

/* Include the following until we figure out where to put the beacons */
#include <reg_prototlv.h>

/* WSC 2.0, deprecated and set to hardcode 0x10 for backwords compatibility resons */
#define WPS_VERSION                0x10	/* Do not change it anymore */

/* WSC 2.0 */
#define WPS_VERSION2               0x20
#define WPS_SETTING_DELAY_TIME_ROUTER	10 /* seconds for embedded router */
#define WPS_SETTING_DELAY_TIME_LINUX	10 /* seconds for Host Linux STA */

/* Beacon Info */
typedef struct {
	CTlvVersion version;
	CTlvScState scState;
	CTlvAPSetupLocked apSetupLocked;
	CTlvSelRegistrar selRegistrar;
	CTlvDevicePwdId pwdId;
	CTlvSelRegCfgMethods selRegConfigMethods;
	CTlvUuid uuid;
	CTlvRfBand rfBand;

	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2; /* C: WSC 2.0 */
	CSubTlvAuthorizedMACs authorizedMACs; /* C: WSC 2.0 */
} WpsBeaconIE;

/* Probe Request Info */
typedef struct {
	CTlvVersion version;
	CTlvReqType reqType;
	CTlvConfigMethods confMethods;
	CTlvUuid uuid;
	CTlvPrimDeviceType primDevType;
	CTlvRfBand rfBand;
	CTlvAssocState assocState;
	CTlvConfigError confErr;
	CTlvDevicePwdId pwdId;
	CTlvManufacturer manufacturer; /* C: WSC 2.0 */
	CTlvModelName modelName; /* C: WSC 2.0 */
	CTlvModelNumber modelNumber; /* C: WSC 2.0 */
	CTlvDeviceName deviceName; /* C: WSC 2.0 */
	CTlvReqDeviceType reqDevType; /* O: WSC 2.0 */
	CTlvPortableDevice portableDevice;

	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2; /* C: WSC 2.0 */
	CSubTlvReqToEnr reqToEnr; /* O: WSC 2.0 */
} WpsProbreqIE;

/* Probe Response Info */
typedef struct {
	CTlvVersion version;
	CTlvScState scState;
	CTlvAPSetupLocked apSetupLocked;
	CTlvSelRegistrar selRegistrar;
	CTlvDevicePwdId pwdId;
	CTlvSelRegCfgMethods selRegConfigMethods;
	CTlvRespType respType;
	CTlvUuid uuid;
	CTlvManufacturer manuf;
	CTlvModelName modelName;
	CTlvModelNumber modelNumber;
	CTlvSerialNum serialNumber;
	CTlvPrimDeviceType primDevType;
	CTlvDeviceName devName;
	CTlvConfigMethods confMethods;
	CTlvRfBand rfBand;

	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2; /* C: WSC 2.0 */
	CSubTlvAuthorizedMACs authorizedMACs; /* C: WSC 2.0 */
} WpsProbrspIE;

#ifdef WPS_NFC_DEVICE
typedef struct {
	CTlvOobDevPwd OobDevPw;
	CTlvUuid uuid;
	CTlvSsid ssid;
	CTlvRfBand rfband;
	CTlvApChannel apchannel;
	CTlvMacAddr bssid;
	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2;
} WpsChoMsg;
#endif /* WPS_NFC_DEVICE */

#endif /* _WPS_HEADERS_ */
