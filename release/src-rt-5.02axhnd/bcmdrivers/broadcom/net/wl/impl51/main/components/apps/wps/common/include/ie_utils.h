/*
 * WPS IE share utility header file
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
 * $Id: ie_utils.h 525052 2015-01-08 20:18:35Z $
 */

#ifndef _WPS_IE_UTILS_H_
#define _WPS_IE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif // endif

#include <typedefs.h>
#include <wpstypes.h>

#define IE_UTILS_BUILD_WPS_IE_FUNC_NUM	5

/* AuthorizedMacs octet structure */
typedef struct {
	int len;
	char macs[SIZE_MAC_ADDR * SIZE_AUTHORIZEDMACS_NUM];
} IE_UTILS_AUTHOMACS_OCTET;

/* Beacon parameters WPS IE structure */
typedef struct {
	uint8 version;
	uint8 scState;
	uint8 apLockdown;
	uint8 selReg;
	uint16 devPwdId;
	uint16 selRegCfgMethods;
	uint8 uuid_e[SIZE_UUID];
	uint8 rfBand;

	uint16 primDeviceCategory;
	uint32 primDeviceOui;
	uint16 primDeviceSubCategory;

	char deviceName[SIZE_32_BYTES + 1];

	/* WSC 2.0 */
	uint8 version2;
	IE_UTILS_AUTHOMACS_OCTET authorizedMacs;
} IE_UTILS_BEACON_PARAMS;

/* Probe Response parameters WPS IE structure */
typedef struct {
	uint8 version;
	uint8 scState;
	uint8 apLockdown;
	uint8 selReg;
	uint16 devPwdId;
	uint16 selRegCfgMethods;
	uint8 respType;
	uint8 uuid_e[SIZE_UUID];
	char manufacturer[SIZE_64_BYTES + 1];
	char modelName[SIZE_32_BYTES + 1];
	char modelNumber[SIZE_32_BYTES + 1];
	char serialNumber[SIZE_32_BYTES + 1];
	uint16 primDeviceCategory;
	uint32 primDeviceOui;
	uint16 primDeviceSubCategory;
	char deviceName[SIZE_32_BYTES + 1];
	uint16 configMethods;
	uint8 rfBand;

	/* WSC 2.0 */
	uint8 version2;
	IE_UTILS_AUTHOMACS_OCTET authorizedMacs;
#ifdef SECONDARY_DEVICE_TYPE
		uint16 secDeviceCategory;
		uint16 secDeviceSubCategory;
		uint32 secDeviceOui;
#endif // endif

} IE_UTILS_PROBERESP_PARAMS;

/* Associate Response parameters WPS IE structure */
typedef struct {
	uint8 version;
	uint8 respType;

	/* WSC 2.0 */
	uint8 version2;
} IE_UTILS_ASSOCRESP_PARAMS;

/* Probe Request parameters WPS IE structure */
typedef struct {
	uint8 version;
	uint8 reqType;
	uint16 configMethods;
	uint8 uuid[SIZE_UUID];
	uint16 primDeviceCategory;
	uint32 primDeviceOui;
	uint16 primDeviceSubCategory;
	uint8 rfBand;
	uint16 assocState;
	uint16 configError;
	uint16 devPwdId;

	uint16 reqDeviceCategory;
	uint32 reqDeviceOui;
	uint16 reqDeviceSubCategory;

	/* WSC 2.0 */
	uint8 version2;
	char manufacturer[SIZE_64_BYTES + 1];
	char modelName[SIZE_32_BYTES + 1];
	char modelNumber[SIZE_32_BYTES + 1];
	char deviceName[SIZE_32_BYTES + 1];
	uint8 reqToEnroll;
} IE_UTILS_PROBEREQ_PARAMS;

/* Associate Request WPS IE parameters structure */
typedef struct {
	uint8 version;
	uint8 reqType;

	/* WSC 2.0 */
	uint8 version2;
} IE_UTILS_ASSOCREQ_PARAMS;

/*
 * implemented in ie_utils.c
 */
uint32 ie_utils_build_beacon_IE(void *params, uint8 *buf, int *buflen);
uint32 ie_utils_build_proberesp_IE(void *params, uint8 *buf, int *buflen);
uint32 ie_utils_build_assocresp_IE(void *params, uint8 *buf, int *buflen);

uint32 ie_utils_build_probereq_IE(void *params, uint8 *buf, int *buflen);
uint32 ie_utils_build_assocreq_IE(void *params, uint8 *buf, int *buflen);

#ifdef __cplusplus
}
#endif // endif

#endif /* _WPS_IE_UTILS_H_ */
