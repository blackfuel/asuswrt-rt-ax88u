/*
 * Registrar protocol TLV
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
 * $Id: reg_prototlv.h 676790 2016-12-24 17:51:50Z $
 */

#ifndef _WPS_REG_PROTO_TLV_
#define _WPS_REG_PROTO_TLV_

#ifdef __cplusplus
extern "C" {
#endif // endif

#include <wpstypes.h>

#include <wpstlvbase.h>
#include <wps.h>

typedef TlvObj_uint8
	CTlvRadioEnabled,
	CTlvReboot,
	CTlvRegistrarEstablished,
	CTlvSelRegistrar,
	CTlvPortableDevice,
	CTlvAPSetupLocked,
	CTlvKeyProvidedAuto,
	CTlv8021XEnabled;

typedef TlvObj_uint8
	CTlvConnType,
	CTlvConnTypeFlags,
	CTlvMsgType,
	CTlvNwIndex,
	CTlvNwKeyIndex,
	CTlvPowerLevel,
	CTlvPskCurrent,
	CTlvPskMax,
	CTlvRegistrarCurrent,
	CTlvRegistrarMax,
	CTlvReqType,
	CTlvRespType,
	CTlvRfBand,
	CTlvScState,
	CTlvTotNetworks,
	CTlvVersion,
	CTlvWEPTransmitKey;

typedef TlvObj_uint16
	CTlvApChannel,
	CTlvAssocState,
	CTlvAuthType,
	CTlvAuthTypeFlags,
	CTlvConfigMethods,
	CTlvConfigError,
	CTlvDevicePwdId,
	CTlvEncrType,
	CTlvEncrTypeFlags,
	CTlvPermittedCfgMethods,
	CTlvSelRegCfgMethods;

typedef TlvObj_uint32
	CTlvFeatureId,
	CTlvOsVersion,
	CTlvKeyLifetime;

typedef TlvObj_ptr
	CTlvConfUrl4, /* <= 64B */
	CTlvConfUrl6, /* <= 76B */
	CTlvDeviceName, /* <= 32B */
	CTlvIdentity, /* <= 80B */
	CTlvIdentityProof,
	CTlvManufacturer, /* <= 64B */
	CTlvModelName, /* <= 32B */
	CTlvModelNumber, /* <= 32B */
	CTlvNwKey, /* <= 64B */
	CTlvNewDeviceName, /* <= 32B */
	CTlvNewPwd, /* <= 64B */
	CTlvSerialNum, /* <= 32B */
	CTlvAppList; /* <= 512B */

typedef TlvObj_ptru
	CTlvMacAddr, /* =6B */
	CTlvAuthenticator, /* =8B */
	CTlvKeyWrapAuth, /* =8B */
	CTlvNonce, /* =16B */
	CTlvEnrolleeNonce, /* =16B */
	CTlvKeyIdentifier, /* =16B */
	CTlvRegistrarNonce, /* =16B */
	CTlvUuid, /* =16B */
	CTlvPubKeyHash, /* =16B */
	CTlvHash, /* =32B */
	CTlvInitVector, /* =32B */
	CTlvSsid, /* =32B */
	CTlvRekeyKey, /* =32B */
	CTlvPublicKey, /* =192B */
	CTlvEapId, /* TBD */
	CTlvX509CertReq,
	CTlvX509Cert,
	CTlvEapType; /* <=8B */

/* WSC 2.0, Subelements in WFA Vendor Extension */
typedef SubTlvObj_uint8
	CSubTlvVersion2,
	CSubTlvReqToEnr,
	CSubTlvSettingsDelayTime,
	CSubTlvNwKeyShareable;

typedef SubTlvObj_ptru
	CSubTlvUuid, /* =16B */
	CSubTlvAuthorizedMACs; /* <=30B */

typedef struct {
	tlvbase_s base;
	uint8 *iv; /* =16B */
	uint8 *ip_encryptedData;
	/* Internal variables, not part of actual TLV */
	uint16 encrDataLength;
} CTlvEncrSettings;

typedef struct {
	tlvbase_s base;
	uint8 *publicKeyHash; /* =20B */
	uint16 pwdId;
	uint8 *ip_devPwd; /* <= 32B */
	/* Internal variables, not part of TLV */
	uint16 devPwdLength;
} CTlvOobDevPwd;

typedef struct {
	tlvbase_s base;
	uint8 *registrarUUID; /* =16B */
	char *cp_deviceName;
} CTlvRegList; /* MAX LEN 512B ? */

typedef struct {
	tlvbase_s base;
	uint8 *vendorId;  /* 3 */
	uint8 *vendorData; /* <=1021B, <=246B for 802.11 Mgmt frames */
	/* Internal variables, not part of TLV */
	uint16 dataLength;
} CTlvVendorExt;

typedef struct {
	tlvbase_s base;

	/* Required attributes */
	CTlvNwIndex nwIndex;
	CTlvSsid ssid;
	CTlvAuthType authType;
	CTlvEncrType encrType;
	CTlvNwKeyIndex nwKeyIndex;
	CTlvNwKey nwKey;
	CTlvMacAddr macAddr;

	/* Optional attributes */
	CTlvEapType eapType;
	CTlvEapId eapIdentity; /* TBD: Define this */
	CTlvKeyLifetime keyLifetime; /* TBD: Define this */
	CTlvVendorExt vendorExt;   /* TBD: Ignore for now */
	CTlvRekeyKey rekeyKey;
	CTlvX509Cert x509Cert;
	CTlvKeyProvidedAuto keyProvidedAuto;
	CTlv8021XEnabled oneXEnabled;
	CTlvWEPTransmitKey WEPKeyIndex;

	/* WSC 2.0 */
	CSubTlvNwKeyShareable nwKeyShareable;
} CTlvCredential;

typedef struct {
	CTlvVersion version;
	CTlvScState scState;
	CTlvSelRegistrar selReg;
	CTlvDevicePwdId devPwdId;
	CTlvSelRegCfgMethods selRegCfgMethods;

	/* WSC 2.0 */
	CTlvUuid uuid_R;

	CTlvVendorExt vendorExt;
	CSubTlvVersion2 version2;
	CSubTlvAuthorizedMACs authorizedMacs;
} CTlvSsrIE;

/* Primary device type */
typedef struct {
	tlvbase_s base;
	uint16 categoryId;
	uint32 oui;
	uint16 subCategoryId;
} CTlvPrimDeviceType;

/* Secondary device type */
typedef struct {
	tlvbase_s base;
	uint16 categoryId;
	uint32 oui;
	uint16 subCategoryId;
} CTlvSecDeviceType;

/* Requested device type */
typedef struct {
	tlvbase_s base;
	uint16 categoryId;
	uint32 oui;
	uint16 subCategoryId;
} CTlvReqDeviceType;

#define tlv_dserialize(a, b, c, d, f) tlv_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define tlv_delete(a, b) tlv_delete_imp(a, b, __FUNCTION__, __LINE__)
#define tlv_allocate(a, b, c) tlv_allocate_imp(a, b, c, __FUNCTION__, __LINE__)
#define tlv_find_dserialize(a, b, c, d, f) \
	tlv_find_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)

#define subtlv_dserialize(a, b, c, d, f) \
	subtlv_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define subtlv_delete(a, b) subtlv_delete_imp(a, b, __FUNCTION__, __LINE__)
#define subtlv_allocate(a, b, c) subtlv_allocate_imp(a, b, c, __FUNCTION__, __LINE__)
#define subtlv_find_dserialize(a, b, c, d, f) \
	subtlv_find_dserialize_imp(a, b, c, d, f, __FUNCTION__, __LINE__)
#define tlv_find_vendorExtParse(a, b, c) tlv_find_vendorExtParse_imp(a, b, c)

#ifdef WFA_WPS_20_TESTBED
#define tlv_find_primDeviceTypeParse(a, b) tlv_find_primDeviceTypeParse_imp(a, b)
int tlv_find_primDeviceTypeParse_imp(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
#endif /* WFA_WPS_20_TESTBED */

tlvbase_s * tlv_new(uint16 theType);
int tlv_allocate_imp(void *v, uint16 theType, uint16 len, const char *func, int line);
int tlv_dserialize_imp(void *b, uint16 theType, BufferObj * theBuf, uint16 dataSize,
	bool allocate, const char *func, int line);
int tlv_find_dserialize_imp(void *b, uint16 theType, BufferObj * theBuf, uint16 dataSize,
	bool allocate, const char *func, int line);
void tlv_serialize(uint16 theType, BufferObj * theBuf, void *data, uint16 len);
void tlv_serialize_ve(char *pVE, uint16 theType, BufferObj * theBuf, void *data, uint16 len);

int tlv_write(void *v, BufferObj* theBuf);
int tlv_set(void *v, uint16 theType, void *val, uint16 len);
void tlv_delete_imp(void *b, bool content_only, const char *func, int line);
int tlv_encrSettingsParse(CTlvEncrSettings *tlv, BufferObj *theBuf);
void tlv_encrSettingsWrite(CTlvEncrSettings *tlv, BufferObj *theBuf);
int tlv_oobDevPwdParse(CTlvOobDevPwd *tlv, BufferObj *theBuf);
void tlv_oobDevPwdWrite(CTlvOobDevPwd *tlv, BufferObj *theBuf);
void tlv_credentialInit(CTlvCredential * tlv);
void tlv_credentialDelete(CTlvCredential *b, bool content_only);
int tlv_credentialParseAKey(CTlvCredential *tlv,  BufferObj *theBuf, bool allocate);
int tlv_credentialParse(CTlvCredential *tlv, BufferObj *theBuf, bool allocate);
void tlv_credentialWrite(CTlvCredential *tlv, BufferObj *theBuf);
int tlv_primDeviceTypeParse(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
void tlv_primDeviceTypeWrite(CTlvPrimDeviceType *tlc, BufferObj *theBuf);
void tlv_secDeviceTypeWrite(CTlvSecDeviceType *tlc, BufferObj *theBuf);
void tlv_reqDeviceTypeWrite(CTlvReqDeviceType *tlc, BufferObj *theBuf);

int tlv_vendorExt_set(CTlvVendorExt *v, uint8 *id, uint8 *data, uint16 datalen);
int tlv_vendorExtParse(CTlvVendorExt *tlv, BufferObj *theBuf);
void tlv_vendorExtWrite(CTlvVendorExt *tlv, BufferObj *theBuf);

subtlvbase_s * subtlv_new(uint8 theId);
int subtlv_allocate_imp(void *v, uint8 theId, uint8 len, const char *func, int line);
int subtlv_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
int subtlv_find_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
int subtlv_find_dserialize_imp(void *b, uint8 theId, BufferObj *theBuf, uint8 dataSize,
	bool allocate, const char *func, int line);
void subtlv_serialize(uint8 theId, BufferObj * theBuf, void *data, uint8 m_len);
int subtlv_write(void *v, BufferObj* theBuf);
int subtlv_set(void *v, uint8 theId, void *val, uint8 len);
void subtlv_delete_imp(void *b, bool content_only, const char *func, int line);
int tlv_find_vendorExtParse_imp(void *v, BufferObj *theBuf, uint8 *vendorId);

#define TLV_UINT8 1
#define TLV_UINT16 2
#define TLV_UINT32 3
#define TLV_CHAR_PTR 4
#define TLV_UINT8_PTR 5

/* special. tlv we need to identify when deleting */
#define TLV_CREDENTIAL 6

int tlv_gettype(uint16 theType);
int subtlv_getid(uint8 theId);

/* Fixed wrong usage of strncpy for tlv data */
char *tlv_strncpy(void *v, char *buf, int buflen);

#ifdef __cplusplus
}
#endif // endif

#endif /* _WPS_REG_PROTO_TLV_ */
