/*
 * Decode functions which provides decoding of Hotspot2.0 ANQP packets
 * as defined in Hotspot2.0 specification.
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
 * $Id:$
 */

#include "802.11.h"
#include "bcmutils.h"
#ifdef BCMDBG
#include "wlioctl_defs.h"
#endif // endif
#include "wl_dbg.h"
#include "bcm_decode_hspot_anqp.h"

/* print decoded hotspot ANQP */
void bcm_decode_hspot_anqp_print(bcm_decode_hspot_anqp_t *hspot)
{
	WL_P2PO(("decoded hotspot ANQP frame:\n"));

	if (hspot->queryListBuffer) {
		WL_PRPKT("   query list",
			hspot->queryListBuffer, hspot->queryListLength);
	}
	if (hspot->capabilityListBuffer) {
		WL_PRPKT("   capability list",
			hspot->capabilityListBuffer, hspot->capabilityListLength);
	}
	if (hspot->operatorFriendlyNameBuffer) {
		WL_PRPKT("   operator friendly name",
			hspot->operatorFriendlyNameBuffer,
			hspot->operatorFriendlyNameLength);
	}
	if (hspot->wanMetricsBuffer) {
		WL_PRPKT("   wan metrics",
			hspot->wanMetricsBuffer, hspot->wanMetricsLength);
	}
	if (hspot->connectionCapabilityBuffer) {
		WL_PRPKT("   connection capability",
			hspot->connectionCapabilityBuffer,
			hspot->connectionCapabilityLength);
	}
	if (hspot->naiHomeRealmQueryBuffer) {
		WL_PRPKT("   NAI home realm query",
			hspot->naiHomeRealmQueryBuffer,
			hspot->naiHomeRealmQueryLength);
	}
	if (hspot->opClassIndicationBuffer) {
		WL_PRPKT("   operating class indication",
			hspot->opClassIndicationBuffer,
			hspot->opClassIndicationLength);
	}
	if (hspot->onlineSignupProvidersBuffer) {
		WL_PRPKT("   online sign-up providers",
			hspot->onlineSignupProvidersBuffer,
			hspot->onlineSignupProvidersLength);
	}
	if (hspot->anonymousNaiBuffer) {
		WL_PRPKT("   anonymous NAI",
			hspot->anonymousNaiBuffer,
			hspot->anonymousNaiLength);
	}
	if (hspot->iconRequestBuffer) {
		WL_PRPKT("   icon request",
			hspot->iconRequestBuffer,
			hspot->iconRequestLength);
	}
	if (hspot->iconBinaryFileBuffer) {
		WL_PRPKT("   icon binary file",
			hspot->iconBinaryFileBuffer,
			hspot->iconBinaryFileLength);
	}
}

/* decode hotspot ANQP frame */
int bcm_decode_hspot_anqp(bcm_decode_t *pkt, int isReset, bcm_decode_hspot_anqp_t *hspot)
{
	int nextIeOffset = 0;
	int ieCount = 0;

	WL_PRPKT("packet for hotspot ANQP decoding",
		bcm_decode_buf(pkt), bcm_decode_buf_length(pkt));

	if (isReset)
		memset(hspot, 0, sizeof(*hspot));

	while (nextIeOffset < bcm_decode_buf_length(pkt)) {
		uint16 infoId;
		uint16 length;
		uint8 oui[WFA_OUI_LEN];
		uint8 type;
		uint8 subtype;
		uint8 reserved;
		int dataLength;
		uint8 *dataPtr;

		bcm_decode_offset_set(pkt, nextIeOffset);
		WL_P2PO(("decoding offset 0x%x\n", bcm_decode_offset(pkt)));

		/* minimum ID and length */
		if (bcm_decode_remaining(pkt) < 4) {
			WL_P2PO(("ID and length too short\n"));
			break;
		}

		(void)bcm_decode_le16(pkt, &infoId);
		(void)bcm_decode_le16(pkt, &length);

		/* check length */
		if (length > bcm_decode_remaining(pkt)) {
			WL_P2PO(("length exceeds packet %d > %d\n",
				length, bcm_decode_remaining(pkt)));
			break;
		}
		nextIeOffset = bcm_decode_offset(pkt) + length;

		/* check ID */
		if (infoId != ANQP_ID_VENDOR_SPECIFIC_LIST) {
			WL_P2PO(("invalid ID 0x%04x\n", infoId));
			continue;
		}

		if (length < HSPOT_LENGTH_OVERHEAD) {
			WL_P2PO(("length too short %d < %d\n",
				length, HSPOT_LENGTH_OVERHEAD));
			continue;
		}

		/* check OUI */
		if (!bcm_decode_bytes(pkt, WFA_OUI_LEN, oui) ||
			memcmp(oui, WFA_OUI, WFA_OUI_LEN) != 0)
			continue;

		/* check type */
		if (!bcm_decode_byte(pkt, &type) || type != HSPOT_ANQP_OUI_TYPE)
			continue;

		if (!bcm_decode_byte(pkt, &subtype))
			continue;

		if (!bcm_decode_byte(pkt, &reserved))
			continue;

		/* remaining data */
		dataLength = length - HSPOT_LENGTH_OVERHEAD;
		dataPtr = bcm_decode_current_ptr(pkt);

		switch (subtype)
		{
		case HSPOT_SUBTYPE_QUERY_LIST:
			hspot->queryListLength = dataLength;
			hspot->queryListBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_CAPABILITY_LIST:
			hspot->capabilityListLength = dataLength;
			hspot->capabilityListBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME:
			hspot->operatorFriendlyNameLength = dataLength;
			hspot->operatorFriendlyNameBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_WAN_METRICS:
			hspot->wanMetricsLength = dataLength;
			hspot->wanMetricsBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_CONNECTION_CAPABILITY:
			hspot->connectionCapabilityLength = dataLength;
			hspot->connectionCapabilityBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY:
			hspot->naiHomeRealmQueryLength = dataLength;
			hspot->naiHomeRealmQueryBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION:
			hspot->opClassIndicationLength = dataLength;
			hspot->opClassIndicationBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS:
			hspot->onlineSignupProvidersLength = dataLength;
			hspot->onlineSignupProvidersBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_ANONYMOUS_NAI:
			hspot->anonymousNaiLength = dataLength;
			hspot->anonymousNaiBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_ICON_REQUEST:
			hspot->iconRequestLength = dataLength;
			hspot->iconRequestBuffer = dataPtr;
			break;
		case HSPOT_SUBTYPE_ICON_BINARY_FILE:
			hspot->iconBinaryFileLength = dataLength;
			hspot->iconBinaryFileBuffer = dataPtr;
			break;
		default:
			WL_P2PO(("invalid subtype %d\n", subtype));
			continue;
			break;
		}

		/* count IEs decoded */
		ieCount++;
	}

	return ieCount;
}

/* decode query list */
int bcm_decode_hspot_anqp_query_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_query_list_t *queryList)
{
	int count, i;

	WL_PRPKT("packet for hotspot query list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(queryList, 0, sizeof(*queryList));

	count =  bcm_decode_remaining(pkt);
	for (i = 0; i < count; i++) {
		if (i >= BCM_DECODE_ANQP_MAX_LIST_SIZE) {
			WL_ERROR(("truncating query list to %d\n",
				BCM_DECODE_ANQP_MAX_LIST_SIZE));
			return FALSE;
		}
		(void)bcm_decode_byte(pkt, &queryList->queryId[queryList->queryLen++]);
	}

	return TRUE;
}

static char *idToStr(int id)
{
	char *str;

	switch (id) {
	case HSPOT_SUBTYPE_QUERY_LIST:
		str = "HSPOT_SUBTYPE_QUERY_LIST";
		break;
	case HSPOT_SUBTYPE_CAPABILITY_LIST:
		str = "HSPOT_SUBTYPE_CAPABILITY_LIST";
		break;
	case HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME:
		str = "HSPOT_SUBTYPE_OPERATOR_FRIENDLY_NAME";
		break;
	case HSPOT_SUBTYPE_WAN_METRICS:
		str = "HSPOT_SUBTYPE_WAN_METRICS";
		break;
	case HSPOT_SUBTYPE_CONNECTION_CAPABILITY:
		str = "HSPOT_SUBTYPE_CONNECTION_CAPABILITY";
		break;
	case HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY:
		str = "HSPOT_SUBTYPE_NAI_HOME_REALM_QUERY";
		break;
	case HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION:
		str = "HSPOT_SUBTYPE_OPERATING_CLASS_INDICATION";
		break;
	case HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS:
		str = "HSPOT_SUBTYPE_ONLINE_SIGNUP_PROVIDERS";
		break;
	case HSPOT_SUBTYPE_ANONYMOUS_NAI:
		str = "HSPOT_SUBTYPE_ANONYMOUS_NAI";
		break;
	case HSPOT_SUBTYPE_ICON_REQUEST:
		str = "HSPOT_SUBTYPE_ICON_REQUEST";
		break;
	case HSPOT_SUBTYPE_ICON_BINARY_FILE:
		str = "HSPOT_SUBTYPE_ICON_BINARY_FILE";
		break;
	default:
		str = "unknown";
		break;
	}

	return str;
}

/* print decoded query list */
void bcm_decode_hspot_anqp_query_list_print(bcm_decode_hspot_anqp_query_list_t *queryList)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP query list:\n"));
	WL_PRINT(("query count = %d\n", queryList->queryLen));

	for (i = 0; i < queryList->queryLen; i++) {
		char *queryStr = idToStr(queryList->queryId[i]);
		WL_PRINT(("   %s (%d)\n", queryStr, queryList->queryId[i]));
	}
}

/* decode capability list */
int bcm_decode_hspot_anqp_capability_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_capability_list_t *capList)
{
	int count, i;

	WL_PRPKT("packet for hotspot capability list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(capList, 0, sizeof(*capList));

	count =  bcm_decode_remaining(pkt);
	for (i = 0; i < count; i++) {
		if (i >= BCM_DECODE_ANQP_MAX_LIST_SIZE) {
			WL_ERROR(("truncating capability list to %d\n",
				BCM_DECODE_ANQP_MAX_LIST_SIZE));
			return FALSE;
		}
		(void)bcm_decode_byte(pkt, &capList->capId[capList->capLen++]);
	}

	return TRUE;
}

/* print decoded capability list */
void bcm_decode_hspot_anqp_capability_list_print(bcm_decode_hspot_anqp_capability_list_t *capList)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP capability list:\n"));
	WL_PRINT(("capability count = %d\n", capList->capLen));

	for (i = 0; i < capList->capLen; i++) {
		char *capStr = idToStr(capList->capId[i]);
		WL_PRINT(("   %s (%d)\n", capStr, capList->capId[i]));
	}
}

/* decode operator name duple */
static int pktDecodeHspotAnqpOperatorNameDuple(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_name_duple_t *duple)
{
	uint8 len;

	if (!bcm_decode_byte(pkt, &len)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, VENUE_LANGUAGE_CODE_SIZE, (uint8 *)duple->lang)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	duple->lang[VENUE_LANGUAGE_CODE_SIZE] = 0;
	duple->langLen = strlen(duple->lang);

	if (len - VENUE_LANGUAGE_CODE_SIZE > bcm_decode_remaining(pkt)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, len - VENUE_LANGUAGE_CODE_SIZE, (uint8 *)duple->name)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	duple->name[len - VENUE_LANGUAGE_CODE_SIZE] = 0;
	duple->nameLen = strlen(duple->name);

	return TRUE;
}

/* decode operator friendly name */
int bcm_decode_hspot_anqp_operator_friendly_name(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_operator_friendly_name_t *op)
{
	WL_PRPKT("packet for hotspot operator friendly name decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(op, 0, sizeof(*op));

	while (bcm_decode_remaining(pkt) > 0 &&
		op->numName < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) {
		if (!pktDecodeHspotAnqpOperatorNameDuple(pkt,
			&op->duple[op->numName])) {
			return FALSE;
		}
		else {
			op->numName++;
		}
	}

	return TRUE;
}

/* print decoded operator friendly name */
void bcm_decode_hspot_anqp_operator_friendly_name_print(
	bcm_decode_hspot_anqp_operator_friendly_name_t *op)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP operator friendly name:\n"));
	for (i = 0; i < op->numName; i++) {
		WL_PRINT(("   language: %s\n", op->duple[i].lang));
		WL_PRUSR("   operator name",
			(uint8 *)op->duple[i].name, op->duple[i].nameLen);
	}
}

/* decode WAN metrics */
int bcm_decode_hspot_anqp_wan_metrics(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_wan_metrics_t *wanMetrics)
{
	uint8 wanInfo;

	WL_PRPKT("packet for hotspot WAN metrics decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(wanMetrics, 0, sizeof(*wanMetrics));

	if (!bcm_decode_byte(pkt, &wanInfo)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	wanMetrics->linkStatus =
		(wanInfo & HSPOT_WAN_LINK_STATUS_MASK) >> HSPOT_WAN_LINK_STATUS_SHIFT;
	wanMetrics->symmetricLink =
		(wanInfo & HSPOT_WAN_SYMMETRIC_LINK_MASK) >> HSPOT_WAN_SYMMETRIC_LINK_SHIFT;
	wanMetrics->atCapacity =
		(wanInfo & HSPOT_WAN_AT_CAPACITY_MASK) >> HSPOT_WAN_AT_CAPACITY_SHIFT;

	if (!bcm_decode_le32(pkt, &wanMetrics->dlinkSpeed)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_le32(pkt, &wanMetrics->ulinkSpeed)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &wanMetrics->dlinkLoad)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &wanMetrics->ulinkLoad)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_le16(pkt, &wanMetrics->lmd)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	return TRUE;
}

/* print decoded WAN metrics */
void bcm_decode_hspot_anqp_wan_metrics_print(bcm_decode_hspot_anqp_wan_metrics_t *wanMetrics)
{
	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP WAN metrics:\n"));
	WL_PRINT(("   link status              : %d\n", wanMetrics->linkStatus));
	WL_PRINT(("   symmetric link           : %d\n", wanMetrics->symmetricLink));
	WL_PRINT(("   at capacity              : %d\n", wanMetrics->atCapacity));
	WL_PRINT(("   downlink speed           : %d\n", wanMetrics->dlinkSpeed));
	WL_PRINT(("   up speed                 : %d\n", wanMetrics->ulinkSpeed));
	WL_PRINT(("   downlink load            : %d\n", wanMetrics->dlinkLoad));
	WL_PRINT(("   up load                  : %d\n", wanMetrics->ulinkLoad));
	WL_PRINT(("   load measurement duration: %d\n", wanMetrics->lmd));
}

/* decode protocol port tuple */
static int pktDecodeHspotAnqpProtoPortTuple(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_proto_port_t *protoPort)
{

	if (!bcm_decode_byte(pkt, &protoPort->ipProtocol)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_le16(pkt, &protoPort->portNumber)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &protoPort->status)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	return TRUE;
}

/* decode connection capability */
int bcm_decode_hspot_anqp_connection_capability(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_connection_capability_t *cap)
{
	WL_PRPKT("packet for hotspot connection capability decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(cap, 0, sizeof(*cap));

	while (bcm_decode_remaining(pkt) > 0 &&
		cap->numConnectCap < BCM_DECODE_HSPOT_ANQP_MAX_CONNECTION_CAPABILITY) {
		if (!pktDecodeHspotAnqpProtoPortTuple(pkt,
			&cap->tuple[cap->numConnectCap])) {
			return FALSE;
		}
		else {
			cap->numConnectCap++;
		}
	}

	return TRUE;
}

/* print decoded connection capability */
void bcm_decode_hspot_anqp_connection_capability_print(
	bcm_decode_hspot_anqp_connection_capability_t *cap)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP connection capability:\n"));
	WL_PRINT(("   count = %d\n", cap->numConnectCap));
	for (i = 0; i < cap->numConnectCap; i++) {
		WL_PRINT(("   IP protocol = %3d   port = %4d   status = %s\n",
			cap->tuple[i].ipProtocol, cap->tuple[i].portNumber,
			cap->tuple[i].status == 0 ? "closed" :
			cap->tuple[i].status == 1 ? "open" :
			cap->tuple[i].status == 2 ? "unknown" :
			"reserved"));
	}
}

static int pktDecodeHspotAnqpNaiHomeRealmQueryData(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_nai_home_realm_data_t *data)
{

	if (!bcm_decode_byte(pkt, &data->encoding)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &data->nameLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (data->nameLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (data->nameLen && !bcm_decode_bytes(pkt, data->nameLen, (uint8 *)data->name)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	data->name[data->nameLen] = 0;

	return TRUE;
}

/* decode home realm query */
int bcm_decode_hspot_anqp_nai_home_realm_query(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_nai_home_realm_query_t *realm)
{
	int i;
	WL_PRPKT("packet for hotspot NAI home realm decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(realm, 0, sizeof(*realm));

	if (!bcm_decode_byte(pkt, &realm->count)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (realm->count > BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM) {
		WL_ERROR(("home realm count %d > %d\n", realm->count,
			BCM_DECODE_HSPOT_ANQP_MAX_HOME_REALM));
		return FALSE;
	}

	for (i = 0; i < realm->count; i++) {
		if (!pktDecodeHspotAnqpNaiHomeRealmQueryData(pkt,
			&realm->data[i])) {
			return FALSE;
		}
	}

	return TRUE;
}

/* print decoded home realm query */
void bcm_decode_hspot_anqp_nai_home_realm_query_print(
	bcm_decode_hspot_anqp_nai_home_realm_query_t *realm)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot ANQP home realm query:\n"));
	WL_PRINT(("   count = %d\n", realm->count));
	for (i = 0; i < realm->count; i++) {
		WL_PRINT(("   realm = %s\n", realm->data[i].name));
	}
}

/* decode operating class indication */
int bcm_decode_hspot_anqp_operating_class_indication(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_operating_class_indication_t *opClassList)
{
	int count;

	WL_PRPKT("packet for hotspot operating class indication decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(opClassList, 0, sizeof(*opClassList));

	count = bcm_decode_remaining(pkt);
	if (count > BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE) {
		WL_ERROR(("operating class indication list size is too large %d\n",
			BCM_DECODE_HSPOT_ANQP_MAX_OPCLASS_LIST_SIZE));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, count, (uint8 *)opClassList->opClass)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}

	opClassList->opClassLen = count;

	return TRUE;
}

/* print decoded operating class indication */
void bcm_decode_hspot_anqp_operating_class_indication_print(
	bcm_decode_hspot_anqp_operating_class_indication_t *opClassList)
{
	int i;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot operating class indication list:\n"));
	WL_PRUSR("operating class",
		opClassList->opClass, opClassList->opClassLen);

	for (i = 0; i < opClassList->opClassLen; i++)
		WL_PRINT(("   operating class %i: %d\n", i, opClassList->opClass[i]));
}

/* decode icon metadata */
static int pktDecodeHspotAnqpIconMetadata(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_icon_metadata_t *icon)
{

	if (!bcm_decode_le16(pkt, &icon->width)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_le16(pkt, &icon->height)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, VENUE_LANGUAGE_CODE_SIZE, (uint8 *)icon->lang)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	icon->lang[VENUE_LANGUAGE_CODE_SIZE] = 0;
	if (!bcm_decode_byte(pkt, &icon->typeLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (icon->typeLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("type length exceeds packet %d > %d\n",
			icon->typeLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (icon->typeLength > BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH) {
		WL_ERROR(("type exceeds buffer %d > %d\n",
			icon->typeLength, BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, icon->typeLength, (uint8 *)icon->type)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	icon->type[icon->typeLength] = 0;

	if (!bcm_decode_byte(pkt, &icon->filenameLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (icon->filenameLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("filename length exceeds packet %d > %d\n",
			icon->filenameLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (icon->filenameLength > BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH) {
		WL_ERROR(("filename exceeds buffer %d > %d\n",
			icon->filenameLength, BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, icon->filenameLength, (uint8 *)icon->filename)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	icon->filename[icon->filenameLength] = 0;

	return TRUE;
}

/* decode OSU provider list */
int bcm_decode_hspot_anqp_osu_provider_list(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_osu_provider_list_t *list)
{
	int len;

	WL_PRPKT("packet for hotspot OSU provider list decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(list, 0, sizeof(*list));

	/* decode non-transmitted profile */
	if (!bcm_decode_le16(pkt, &list->nonTransProfileLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (list->nonTransProfileLength > BCM_DECODE_HSPOT_ANQP_MAX_NON_TRANS_PROFILE_LENGTH) {
		WL_ERROR(("length exceeds maximum %d > %d\n",
			list->nonTransProfileLength,
			BCM_DECODE_HSPOT_ANQP_MAX_NON_TRANS_PROFILE_LENGTH));
		return FALSE;
	}
	if (list->nonTransProfileLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			list->nonTransProfileLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (list->nonTransProfileLength > 0 &&
		!bcm_decode_bytes(pkt, list->nonTransProfileLength, list->nonTransProfile)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}

	/* decode legacy OSU SSID */
	if (!bcm_decode_byte(pkt, &list->legacyOsuSsidLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	len = list->legacyOsuSsidLength;
	if (len > BCM_DECODE_HSPOT_ANQP_MAX_LEGACY_OSU_SSID_LENGTH) {
		WL_ERROR(("length exceeds maximum %d > %d\n",
			list->legacyOsuSsidLength,
			BCM_DECODE_HSPOT_ANQP_MAX_LEGACY_OSU_SSID_LENGTH));
		return FALSE;
	}
	if (list->legacyOsuSsidLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("length exceeds packet %d > %d\n",
			list->legacyOsuSsidLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (list->legacyOsuSsidLength > 0 &&
		!bcm_decode_bytes(pkt, list->legacyOsuSsidLength, list->legacyOsuSsid)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	list->legacyOsuSsid[list->legacyOsuSsidLength] = 0;

	while (bcm_decode_remaining(pkt) > 0 &&
		list->osuProviderCount < BCM_DECODE_HSPOT_ANQP_MAX_OSU_PROVIDER) {
		bcm_decode_hspot_anqp_osu_provider_t *osu =
			&list->osuProvider[list->osuProviderCount];
		uint16 osuLength;
		uint16 nameLength;
		int remNameLength;
		uint16 iconLength;
		int remIconLength;
		uint16 descLength;
		int remDescLength;

		/* decode OSU provider length */
		if (!bcm_decode_le16(pkt, &osuLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (osuLength > bcm_decode_remaining(pkt)) {
			WL_ERROR(("OSU length exceeds packet %d > %d\n",
				osuLength, bcm_decode_remaining(pkt)));
			return FALSE;
		}

		/* decode OSU friendly name */
		if (!bcm_decode_le16(pkt, &nameLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (nameLength > bcm_decode_remaining(pkt)) {
			WL_ERROR(("name length exceeds packet %d > %d\n",
				nameLength, bcm_decode_remaining(pkt)));
			return FALSE;
		}
		remNameLength = nameLength;
		while (remNameLength > 0 &&
			osu->name.numName < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) {
			int startOffset = bcm_decode_offset(pkt);

			if (!pktDecodeHspotAnqpOperatorNameDuple(pkt,
				&osu->name.duple[osu->name.numName])) {
				return FALSE;
			}
			else {
				osu->name.numName++;
			}

			/* update remaining name length */
			remNameLength -= bcm_decode_offset(pkt) - startOffset;
		}

		/* decode OSU server URI */
		if (!bcm_decode_byte(pkt, &osu->uriLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (osu->uriLength > BCM_DECODE_HSPOT_ANQP_MAX_URI_LENGTH) {
			WL_ERROR(("URI exceeds buffer %d > %d\n",
				osu->uriLength, BCM_DECODE_HSPOT_ANQP_MAX_URI_LENGTH));
			return FALSE;
		}
		if (!bcm_decode_bytes(pkt, osu->uriLength, (uint8 *)osu->uri)) {
			WL_ERROR(("bcm_decode_bytes failed"));
			return FALSE;
		}
		osu->uri[osu->uriLength] = 0;

		/* decode OSU method */
		if (!bcm_decode_byte(pkt, &osu->methodLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (osu->methodLength > BCM_DECODE_HSPOT_ANQP_MAX_METHOD_LENGTH) {
			WL_ERROR(("method exceeds buffer %d > %d\n",
				osu->methodLength, BCM_DECODE_HSPOT_ANQP_MAX_METHOD_LENGTH));
			return FALSE;
		}
		if (!bcm_decode_bytes(pkt, osu->methodLength, (uint8 *)osu->method)) {
			WL_ERROR(("bcm_decode_bytes failed"));
			return FALSE;
		}

		/* decode icon metadata */
		if (!bcm_decode_le16(pkt, &iconLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		remIconLength = iconLength;
		while (remIconLength > 0 &&
			osu->iconMetadataCount < BCM_DECODE_HSPOT_ANQP_MAX_ICON_METADATA_LENGTH) {
			int startOffset = bcm_decode_offset(pkt);

			if (!pktDecodeHspotAnqpIconMetadata(pkt,
				&osu->iconMetadata[osu->iconMetadataCount])) {
				return FALSE;
			}
			else {
				osu->iconMetadataCount++;
			}

			/* update remaining name length */
			remIconLength -= bcm_decode_offset(pkt) - startOffset;
		}

		/* decode OSU NAI */
		if (!bcm_decode_byte(pkt, &osu->naiLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (osu->naiLength > BCM_DECODE_HSPOT_ANQP_MAX_NAI_LENGTH) {
			WL_ERROR(("NAI exceeds buffer %d > %d\n",
				osu->naiLength, BCM_DECODE_HSPOT_ANQP_MAX_NAI_LENGTH));
			return FALSE;
		}
		if (osu->naiLength > 0 &&
			!bcm_decode_bytes(pkt, osu->naiLength, (uint8 *)osu->nai)) {
			WL_ERROR(("bcm_decode_bytes failed"));
			return FALSE;
		}
		osu->nai[osu->naiLength] = 0;

		/* decode OSU service description */
		if (!bcm_decode_le16(pkt, &descLength)) {
			WL_ERROR(("decode error\n"));
			return FALSE;
		}
		if (descLength > bcm_decode_remaining(pkt)) {
			WL_ERROR(("name length exceeds packet %d > %d\n",
				descLength, bcm_decode_remaining(pkt)));
			return FALSE;
		}
		remDescLength = descLength;
		while (remDescLength > 0 &&
			osu->desc.numName < BCM_DECODE_HSPOT_ANQP_MAX_OPERATOR_NAME) {
			int startOffset = bcm_decode_offset(pkt);

			if (!pktDecodeHspotAnqpOperatorNameDuple(pkt,
				&osu->desc.duple[osu->desc.numName])) {
				return FALSE;
			}
			else {
				osu->desc.numName++;
			}

			/* update remaining name length */
			remDescLength -= bcm_decode_offset(pkt) - startOffset;
		}

		list->osuProviderCount++;
	}

	return TRUE;
}

/* print decoded OSU provider list */
void bcm_decode_hspot_anqp_osu_provider_list_print(
	bcm_decode_hspot_anqp_osu_provider_list_t *list)
{
	int i, j;

	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot OSU provider list:\n"));
	WL_PRUSR("OSU nontransmitted profile",
		list->nonTransProfile, list->nonTransProfileLength);
	WL_PRINT(("legacy OSU SSID = %s\n", list->legacyOsuSsid));
	WL_PRINT(("OSU provider count = %d\n", list->osuProviderCount));

	for (i = 0; i < list->osuProviderCount; i++) {
		bcm_decode_hspot_anqp_osu_provider_t *osu = &list->osuProvider[i];

		WL_PRINT(("\n"));
		for (j = 0; j < osu->name.numName; j++) {
			bcm_decode_hspot_anqp_name_duple_t *d = &osu->name.duple[j];
			WL_PRINT(("   language: %s\n", d->lang));
			WL_PRINT(("   provider name: %s\n", d->name));
		}
		WL_PRINT(("   URI: %s\n", osu->uri));
		for (j = 0; j < osu->methodLength; j++) {
			uint8 method = osu->method[j];
			WL_PRINT(("   method: %s\n",
				method == HSPOT_OSU_METHOD_OMA_DM ? "OMA-DM" :
				method == HSPOT_OSU_METHOD_SOAP_XML ? "SOAP-XML" :
				"invalid"));
		}
		WL_PRINT(("   icons: %d\n", osu->iconMetadataCount));
		for (j = 0; j < osu->iconMetadataCount; j++) {
			bcm_decode_hspot_anqp_icon_metadata_t *icon = &osu->iconMetadata[j];
			WL_PRINT(("      width=%d height=%d type=%s filename=%s\n",
				icon->width, icon->height, icon->type, icon->filename));
		}
		WL_PRINT(("   NAI: %s\n", osu->nai));
		for (j = 0; j < osu->desc.numName; j++) {
			bcm_decode_hspot_anqp_name_duple_t *d = &osu->desc.duple[j];
			WL_PRINT(("   language: %s\n", d->lang));
			WL_PRINT(("   description: %s\n", d->name));
		}
	}
}

/* search decoded OSU provider list for specified provider */
bcm_decode_hspot_anqp_osu_provider_t *bcm_decode_hspot_anqp_find_osu_provider(
	bcm_decode_hspot_anqp_osu_provider_list_t *list, int length, char *provider)
{
	int i;

	for (i = 0; i < list->osuProviderCount; i++) {
		bcm_decode_hspot_anqp_osu_provider_t *p = &list->osuProvider[i];
		int j;

		for (j = 0; j < p->name.numName; j++) {
			bcm_decode_hspot_anqp_name_duple_t *d = &p->name.duple[j];
			if (d->nameLen == length &&
				memcmp(d->name, provider, d->nameLen) == 0) {
				return p;
			}
		}
	}

	return 0;
}

/* decode anonymous NAI */
int bcm_decode_hspot_anqp_anonymous_nai(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_anonymous_nai_t *anonymous)
{
	WL_PRPKT("packet for hotspot anonymous NAI decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(anonymous, 0, sizeof(*anonymous));

	if (!bcm_decode_le16(pkt, &anonymous->naiLen)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (anonymous->naiLen > bcm_decode_remaining(pkt)) {
		WL_ERROR(("anonymous NAI length exceeds packet %d > %d\n",
			anonymous->naiLen, bcm_decode_remaining(pkt)));
		return FALSE;
	}

	if (anonymous->naiLen > BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE) {
		WL_ERROR(("anonymous NAI exceeds buffer %d > %d\n",
			anonymous->naiLen, BCM_DECODE_HSPOT_ANQP_MAX_NAI_SIZE));
		return FALSE;
	}

	if (!bcm_decode_bytes(pkt, anonymous->naiLen, (uint8 *)anonymous->nai)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}

	anonymous->nai[anonymous->naiLen] = 0;

	return TRUE;
}

/* print decoded anonymous NAI */
void bcm_decode_hspot_anqp_anonymous_nai_print(
	bcm_decode_hspot_anqp_anonymous_nai_t *anonymous)
{
	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot anonymous NAI:\n"));
	WL_PRINT(("   %s\n", anonymous->nai));
}

/* decode icon request */
int bcm_decode_hspot_anqp_icon_request(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_icon_request_t *request)
{
	WL_PRPKT("packet for hotspot icon request decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(request, 0, sizeof(*request));

	request->filenameLength = bcm_decode_remaining(pkt);
	if (request->filenameLength > BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH) {
		WL_ERROR(("icon filename exceeds buffer %d > %d\n",
			request->filenameLength, BCM_DECODE_HSPOT_ANQP_MAX_ICON_FILENAME_LENGTH));
		return FALSE;
	}
	if (!bcm_decode_bytes(pkt, request->filenameLength, (uint8 *)request->filename)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	request->filename[request->filenameLength] = 0;

	return TRUE;

}

/* print icon request */
void bcm_decode_hspot_anqp_icon_request_print(
	bcm_decode_hspot_anqp_icon_request_t *request)
{
	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded icon request:\n"));
	WL_PRINT(("   %s\n", request->filename));
}

/* decode icon binary file */
int bcm_decode_hspot_anqp_icon_binary_file(bcm_decode_t *pkt,
	bcm_decode_hspot_anqp_icon_binary_file_t *icon)
{
	WL_PRPKT("packet for hotspot icon binary file decoding",
		bcm_decode_current_ptr(pkt), bcm_decode_remaining(pkt));

	memset(icon, 0, sizeof(*icon));

	if (!bcm_decode_byte(pkt, &icon->status)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}

	if (!bcm_decode_byte(pkt, &icon->typeLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (icon->typeLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("icon type length exceeds packet %d > %d\n",
			icon->typeLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (icon->typeLength > BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH) {
		WL_ERROR(("icon type exceeds buffer %d > %d\n",
			icon->typeLength, BCM_DECODE_HSPOT_ANQP_MAX_ICON_TYPE_LENGTH));
		return FALSE;
	}
	if (icon->typeLength > 0 &&
		!bcm_decode_bytes(pkt, icon->typeLength, (uint8 *)icon->type)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}
	icon->type[icon->typeLength] = 0;

	if (!bcm_decode_le16(pkt, &icon->binaryLength)) {
		WL_ERROR(("decode error\n"));
		return FALSE;
	}
	if (icon->binaryLength > bcm_decode_remaining(pkt)) {
		WL_ERROR(("icon binary length exceeds packet %d > %d\n",
			icon->binaryLength, bcm_decode_remaining(pkt)));
		return FALSE;
	}
	if (icon->binaryLength > 0 &&
		!bcm_decode_bytes(pkt, icon->binaryLength, (uint8 *)icon->binary)) {
		WL_ERROR(("bcm_decode_bytes failed"));
		return FALSE;
	}

	return TRUE;
}

/* print decoded icon binary file */
void bcm_decode_hspot_anqp_icon_binary_file_print(bcm_decode_hspot_anqp_icon_binary_file_t *icon)
{
	WL_PRINT(("----------------------------------------\n"));
	WL_PRINT(("decoded hotspot icon binary file:\n"));
	WL_PRINT(("   status: %s\n",
		icon->status == HSPOT_ICON_STATUS_SUCCESS ? "success" :
		icon->status == HSPOT_ICON_STATUS_FILE_NOT_FOUND ? "file not found" :
		icon->status == HSPOT_ICON_STATUS_UNSPECIFIED_FILE_ERROR ?
		"unspecified file error" : "invalid"));
	WL_PRUSR("   type",
		(uint8 *)icon->type, icon->typeLength);
	WL_PRUSR("   binary",
		(uint8 *)icon->binary, icon->binaryLength);
}
