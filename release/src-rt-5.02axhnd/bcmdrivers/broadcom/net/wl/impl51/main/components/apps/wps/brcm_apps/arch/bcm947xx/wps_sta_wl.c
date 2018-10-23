/*
 * Broadcom 802.11 device interface
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
 * $Id: wps_sta_wl.c 749115 2018-02-27 20:25:46Z $
 */

#include <stdio.h>

#ifdef __linux__
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>
#endif /* __linux__ */

#include <portability.h>
#include <reg_prototlv.h>
#include <wps_enrapi.h>
#include <wps_sta.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <wps_staeapsm.h>
#include <wps_enr_osl.h>
#include <wpserror.h>
#include <tutrace.h>
#include <wps_wps.h>

int tolower(int);
int wps_wl_ioctl(int cmd, void *buf, int len, bool set);
static int wl_iovar_get(char *iovar, void *bufptr, int buflen);
static int wl_iovar_getbuf(char *iovar, void *param, int paramlen, void *bufptr, int buflen);
static int wps_iovar_set(const char *iovar, void *param, int paramlen);
static int wps_iovar_setbuf(const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen);
static uint wps_iovar_mkbuf(const char *name, char *data, uint datalen,
	char *iovar_buf, uint buflen, int *perr);
static int wps_ioctl_get(int cmd, void *buf, int len);
static int wps_ioctl_set(int cmd, void *buf, int len);

#define WPS_DUMP_BUF_LEN (127 * 1024)
#define WPS_SSID_FMT_BUF_LEN 4*32+1	/* Length for SSID format string */

#define WPS_SCAN_MAX_WAIT_SEC 10
#define WPS_JOIN_MAX_WAIT_SEC 60	/*
					 * Do not define this value too short,
					 * because AP/Router may reboot after got new
					 * credential and apply it.
					 */
#define WPS_IE_BUF_LEN	VNDR_IE_MAX_LEN * 8	/* 2048 */

wps_ap_list_info_t ap_list[WPS_MAX_AP_SCAN_LIST_LEN];
static char scan_result[WPS_DUMP_BUF_LEN];
static char escan_result[WLC_IOCTL_MEDLEN];
static uint8 wps_ie_setbuf[WPS_IE_BUF_LEN];

struct escan_bss *escan_bss_head = NULL;
struct escan_bss *escan_bss_tail = NULL;
uint32_t is_scan_done = WPS_ESCAN_NOT_STARTED;

void wps_fill_aplist(wl_bss_info_t *bi, uint wps_ap_count)
{
	if ((bi->ie_length) &&
		(wps_ap_count < WPS_MAX_AP_SCAN_LIST_LEN)) {
		int sb, chan_adj = 0;

		ap_list[wps_ap_count].used = TRUE;
		memcpy(ap_list[wps_ap_count].BSSID, bi->BSSID.octet, 6);
		strncpy((char *)ap_list[wps_ap_count].ssid, (char *)bi->SSID,
			bi->SSID_len);
		ap_list[wps_ap_count].ssid[bi->SSID_len] = '\0';
		ap_list[wps_ap_count].ssidLen = bi->SSID_len;
		ap_list[wps_ap_count].ie_buf = (uint8 *)(((uint8 *)bi) +
			bi->ie_offset);
		ap_list[wps_ap_count].ie_buflen = bi->ie_length;
		if (CHSPEC_IS40(bi->chanspec)) {
			sb = CHSPEC_CTL_SB(bi->chanspec);
			if (sb == WL_CHANSPEC_CTL_SB_LOWER)
				chan_adj = -2;
			else
				chan_adj = 2;
		}

		ap_list[wps_ap_count].channel = CHSPEC_CHANNEL(bi->chanspec) +
			chan_adj;
		ap_list[wps_ap_count].band = (CHSPEC_IS2G(bi->chanspec) ?
			WPS_RFBAND_24GHZ : WPS_RFBAND_50GHZ);
		ap_list[wps_ap_count].wep = bi->capability & DOT11_CAP_PRIVACY;

	}

}

/* Get and set display aplist and wps supported aplist */
void wpssta_display_aplist_set(uint8 *aplist_diplay, bool set)
{
	static uint8 set_aplist_display = 0;
	if (!aplist_diplay)
		return;
	if (set) {
		set_aplist_display = *aplist_diplay;
	} else {
		*aplist_diplay = set_aplist_display;
	}
}

uint32
wps_eap_parse_scan_result(wl_event_msg_t* event, char* ifname)
{
	uint16_t event_type = ntohl(event->event_type);
	uint32 bi_length;
	struct escan_bss *result;
	wps_ap_list_info_t *wpsaplist;
	uint32 status;
	struct ether_addr *addr = &event->addr;
	wl_escan_result_t *escan_data;
	uint8 aplist_display = 0;
	if (event_type != WLC_E_ESCAN_RESULT) {
		return 1;
	}
	if (is_scan_done != WPS_ESCAN_INPROGRESS) {
		return 1;
	}

	escan_data = (wl_escan_result_t*) &(((char*) event)[sizeof(wl_event_msg_t)]);
	status = ntohl(event->status);

	if (status == WLC_E_STATUS_SUCCESS) {
		TUTRACE((TUTRACE_ERR, "packet received: WLC_E_STATUS_SUCCESS\n", event->ifname));
		is_scan_done = WPS_ESCAN_DONE;

		/* Display aplist and wps enabled ap list if requested */

		wpssta_display_aplist_set(&aplist_display, FALSE);
		if (aplist_display) {
			wpsaplist = create_aplist_escan();
			if (wpsaplist) {
				wpssta_display_aplist(wpsaplist);
				wps_get_aplist(wpsaplist, wpsaplist);

				TUTRACE((TUTRACE_INFO, "WPS Enabled AP list :\n"));
				wpssta_display_aplist(wpsaplist);
			}
		/* Reset the AP list display */
		aplist_display = 0;
		wpssta_display_aplist_set(&aplist_display, TRUE);
		}

		return 0;
	} else if (status == WLC_E_STATUS_PARTIAL) {
		/* If new BSS, add to the list. Otherwise, update it. */

		wl_bss_info_t *bi = &escan_data->bss_info[0];
		wl_bss_info_t *bss;
		if (!bi) {
			TUTRACE((TUTRACE_ERR, "Invalid escan bss info (NULL pointer)\n"));
			return -1;
		}
		bi_length = dtoh32(bi->length);
		if (bi_length != (dtoh32(escan_data->buflen) - WL_ESCAN_RESULTS_FIXED_SIZE)) {
			TUTRACE((TUTRACE_ERR, "Invalid bss_info length %d: ignoring\n", bi_length));
			return -1;
		}

		/* check if we've received info of same BSSID */
		for (result = escan_bss_head; result; result = result->next) {
			bss = result->bss;

#define WLC_BSS_RSSI_ON_CHANNEL 0x0002 /* Copied from wlc.h. Is there a better way to do this? */

			if (!memcmp(&bi->BSSID, &bss->BSSID, ETHER_ADDR_LEN) &&
				CHSPEC_BAND(bi->chanspec) ==
				CHSPEC_BAND(bss->chanspec) &&
				bi->SSID_len == bss->SSID_len &&
				!memcmp(bi->SSID, bss->SSID, bi->SSID_len))
				break;
		}

		if (!result) {
			/* TUTRACE((TUTRACE_ERR, "New one\n")); */
			/* New BSS. Allocate memory and save it */
			#define ESCAN_BSS_FIXED_SIZE 4
			struct escan_bss *ebss = malloc(ESCAN_BSS_FIXED_SIZE+ bi_length);

			if (!ebss) {
				perror("can't allocate memory for bss");
				return 1;
			}

			ebss->next = NULL;
			/* Copy bss info to scan buffer. */
			memcpy(&ebss->bss, bi, bi_length);
			if (escan_bss_tail) {
				escan_bss_tail->next = ebss;
			} else {
				escan_bss_head = ebss;
			}
			escan_bss_tail = ebss;
			/* list->count++; */
		} else {
			/* TUTRACE((TUTRACE_ERR, "Update\n")); */
			/* We've got this BSS. Update rssi if necessary */
			if ((bss->flags & WLC_BSS_RSSI_ON_CHANNEL) ==
				(bi->flags & WLC_BSS_RSSI_ON_CHANNEL)) {
				/* preserve max RSSI if the measurements are
				 * both on-channel or both off-channel
				 */
				bss->RSSI = (dtoh16(bss->RSSI) > dtoh16(bi->RSSI))
					? bss->RSSI : bi->RSSI;
			} else if ((bss->flags & WLC_BSS_RSSI_ON_CHANNEL) &&
				(bi->flags & WLC_BSS_RSSI_ON_CHANNEL) == 0) {
				/* preserve the on-channel rssi measurement
				 * if the new measurement is off channel
				*/
				bss->RSSI = bi->RSSI;
				bss->flags |= WLC_BSS_RSSI_ON_CHANNEL;
			}
		}
	} else {
		printf("sync_id: %d, WLC_E_STATUS %d, misc. error/abort\n",
			dtoh16(escan_data->sync_id), status);
		is_scan_done = WPS_ESCAN_DONE;
		return 1;
	}

	return 0;
}
/* Return Current wps escan state */
uint32 get_wps_escan_state()
{
	return is_scan_done;
}

uint32
wps_eap_reset_scan_result()
{

	struct escan_bss *result;

	is_scan_done = WPS_ESCAN_NOT_STARTED;
	TUTRACE((TUTRACE_ERR, "restart scan\n"));
	/* free scan results */
	result = escan_bss_head;
	while (result) {
		struct escan_bss *tmp = result->next;
		free(result);
		result = tmp;
	}
	escan_bss_head = NULL;
	escan_bss_tail = NULL;

	return 0;
}

char *
get_wps_escan_results()
{
	if (is_scan_done == WPS_ESCAN_DONE) {
		TUTRACE((TUTRACE_ERR, "Scan done.\n"));
		return (char*)escan_bss_head;
	}
	return NULL;
}

wps_ap_list_info_t *create_aplist_escan()
{
	struct escan_bss *current = escan_bss_head;
	wl_bss_info_107_t *old_bi_107;
	wl_bss_info_t *bi;

	uint wps_ap_count = 0;

	TUTRACE((TUTRACE_ERR, "start \n"));
	if (is_scan_done != WPS_ESCAN_DONE) {
		TUTRACE((TUTRACE_ERR, "Error, scan not done !\n"));
		return NULL;
	}

	if (!current)
		return NULL;
	/*
	 * Caller must prepare the scan_result before calling this function,
	 * uses do_wps_scan and get_wps_scan_results to retrieve scan_result.
	 */

	memset(ap_list, 0, sizeof(ap_list));

	for (; current; current = current->next) {

		bi = current->bss;
		/* Convert version 107 to 108 */
		if (bi->version == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi_107 = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi_107->channel);
			bi->ie_length = old_bi_107->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}
		wps_fill_aplist(bi, wps_ap_count);
		wps_ap_count++;

	}

	return ap_list;
}

int
do_wps_escan()
{
	int ret;

	wl_escan_params_t* params;
	int params_size = (WL_SCAN_PARAMS_FIXED_SIZE
		+ OFFSETOF(wl_escan_params_t, params)) +
		WL_NUMCHANNELS * sizeof(uint16);
	TUTRACE((TUTRACE_INFO, "start escan\n"));
	if (is_scan_done == WPS_ESCAN_INPROGRESS) {
		TUTRACE((TUTRACE_ERR, "Scan skip, escan In Progress\n"));
		return -1;
	}
	wps_eap_reset_scan_result();
	is_scan_done = WPS_ESCAN_INPROGRESS;

	params = (wl_escan_params_t*)malloc(params_size);
	if (params == NULL) {
		TUTRACE((TUTRACE_ERR, "Error allocating %d bytes for scan params\n", params_size));
		return -1;
	}

	memset(params, 0, params_size);
	params->params.bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->params.bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->params.scan_type = 0;
	params->params.nprobes = -1;
	params->params.active_time = -1;
	params->params.passive_time = -1;
	params->params.home_time = -1;
	params->params.channel_num = 0;

	params->version = htod32(ESCAN_REQ_VERSION);
	params->action = htod16(WL_SCAN_ACTION_START);
	srand((unsigned)time(NULL));
	params->sync_id = htod16(rand() & 0xffff);
	ret = wps_iovar_setbuf("escan", params, params_size, escan_result, sizeof(escan_result));
	TUTRACE((TUTRACE_INFO, "escan iovar result: %d\n", ret));

	#ifdef _TUDEBUGTRACE
	if (ret) {
		TUTRACE((TUTRACE_INFO, "do wps escan command failed: %d\n", ret));
	}
	#endif
	wps_escan_timeout_handler(WPS_ESCAN_STARTED);
	free(params);
	return ret;
}

/*
 * End (main) SC patch  to scan with escan
 * ******************************************************
 */
/* PBC Overlapped detection */

int
do_wps_scan()
{
	int ret;
	wl_scan_params_t* params;
	int params_size = WL_SCAN_PARAMS_FIXED_SIZE + WL_NUMCHANNELS * sizeof(uint16);

	params = (wl_scan_params_t*)malloc(params_size);
	if (params == NULL) {
		TUTRACE((TUTRACE_ERR, "Error allocating %d bytes for scan params\n", params_size));
		return -1;
	}

	memset(params, 0, params_size);
	params->bss_type = DOT11_BSSTYPE_ANY;
	memcpy(&params->bssid, &ether_bcast, ETHER_ADDR_LEN);
	params->scan_type = -1;
	params->nprobes = -1;
	params->active_time = -1;
	params->passive_time = -1;
	params->home_time = -1;
	params->channel_num = 0;

	ret = wps_ioctl_set(WLC_SCAN, params, params_size);
#ifdef _TUDEBUGTRACE
	if (ret) {
		TUTRACE((TUTRACE_INFO, "do_wps_scan : do wps scan command failed\n"));
	}
#endif // endif

	free(params);
	return ret;
}

char *
get_wps_scan_results()
{
	int ret;
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;

	list->buflen = WPS_DUMP_BUF_LEN;
	ret = wps_ioctl_get(WLC_SCAN_RESULTS, scan_result, WPS_DUMP_BUF_LEN);

	if (ret < 0)
		return NULL;

	return scan_result;
}

wps_ap_list_info_t *create_aplist()
{
	wl_scan_results_t *list = (wl_scan_results_t*)scan_result;
	wl_bss_info_t *bi;
	wl_bss_info_107_t *old_bi_107;
	uint i, wps_ap_count = 0;

	/*
	 * Caller must prepare the scan_result before calling this function,
	 * uses do_wps_scan and get_wps_scan_results to retrieve scan_result.
	 */

	memset(ap_list, 0, sizeof(ap_list));
	if (list->count == 0)
		return 0;

#ifdef LEGACY2_WL_BSS_INFO_VERSION
	if (list->version != WL_BSS_INFO_VERSION &&
		list->version != LEGACY_WL_BSS_INFO_VERSION &&
		list->version != LEGACY2_WL_BSS_INFO_VERSION) {
#else
	if (list->version != WL_BSS_INFO_VERSION &&
		list->version != LEGACY_WL_BSS_INFO_VERSION) {
#endif // endif
		TUTRACE((TUTRACE_ERR, "Sorry, your driver has bss_info_version %d "
				"but this program supports only version %d.\n",
				list->version, WL_BSS_INFO_VERSION));
		return 0;
	}
	bi = list->bss_info;
	for (i = 0; i < list->count; i++) {
		/* Convert version 107 to 108 */
		if (bi->version == LEGACY_WL_BSS_INFO_VERSION) {
			old_bi_107 = (wl_bss_info_107_t *)bi;
			bi->chanspec = CH20MHZ_CHSPEC(old_bi_107->channel);
			bi->ie_length = old_bi_107->ie_length;
			bi->ie_offset = sizeof(wl_bss_info_107_t);
		}
		wps_fill_aplist(bi, wps_ap_count);
		wps_ap_count++;
		bi = (wl_bss_info_t*)((int8*)bi + bi->length);
	}

	return ap_list;
}

#ifdef _TUDEBUGTRACE
static char *
_pktflag_name(unsigned int pktflag)
{
	if (pktflag == VNDR_IE_BEACON_FLAG)
		return "Beacon";
	else if (pktflag == VNDR_IE_PRBRSP_FLAG)
		return "Probe Resp";
	else if (pktflag == VNDR_IE_ASSOCRSP_FLAG)
		return "Assoc Resp";
	else if (pktflag == VNDR_IE_AUTHRSP_FLAG)
		return "Auth Resp";
	else if (pktflag == VNDR_IE_PRBREQ_FLAG)
		return "Probe Req";
	else if (pktflag == VNDR_IE_ASSOCREQ_FLAG)
		return "Assoc Req";
	else if (pktflag == VNDR_IE_CUSTOM_FLAG)
		return "Custom";
	else
		return "Unknown";
}
#endif /* _TUDEBUGTRACE */

static int
_del_vndr_ie(char *bufaddr, int buflen, uint32 frametype)
{
	int iebuf_len;
	int iecount, err;
	vndr_ie_setbuf_t *ie_setbuf;
#ifdef _TUDEBUGTRACE
	int i;
	int frag_len = buflen - 6;
	unsigned char *frag = (unsigned char *)(bufaddr + 6);
#endif // endif

	iebuf_len = buflen + sizeof(vndr_ie_setbuf_t) - sizeof(vndr_ie_t);
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(iebuf_len);
	if (!ie_setbuf) {
		TUTRACE((TUTRACE_ERR, "memory alloc failure\n"));
		return -1;
	}

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "del");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy(&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &frametype, sizeof(uint32));
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data, bufaddr, buflen);

#ifdef _TUDEBUGTRACE
	printf("\n_del_vndr_ie (%s, frag_len=%d)\n", _pktflag_name(frametype), frag_len);
	for (i = 0; i < frag_len; i++) {
		if (i && !(i%16))
			printf("\n");
		printf("%02x ", frag[i]);
	}
	printf("\n");
#endif // endif

	err = wps_iovar_set("vndr_ie", ie_setbuf, iebuf_len);

	free(ie_setbuf);

	return err;
}

static int
_set_vndr_ie(unsigned char *frag, int frag_len, unsigned char ouitype, unsigned int pktflag)
{
	vndr_ie_setbuf_t *ie_setbuf;
	int buflen, iecount, i;
	int err = 0;

	buflen = sizeof(vndr_ie_setbuf_t) + frag_len;
	ie_setbuf = (vndr_ie_setbuf_t *) malloc(buflen);
	if (!ie_setbuf) {
		TUTRACE((TUTRACE_ERR, "memory alloc failure\n"));
		return -1;
	}

	/* Copy the vndr_ie SET command ("add"/"del") to the buffer */
	strcpy(ie_setbuf->cmd, "add");

	/* Buffer contains only 1 IE */
	iecount = 1;
	memcpy(&ie_setbuf->vndr_ie_buffer.iecount, &iecount, sizeof(int));

	/*
	 * The packet flag bit field indicates the packets that will
	 * contain this IE
	 */
	memcpy(&ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].pktflag, &pktflag, sizeof(uint32));

	/* Now, add the IE to the buffer, +1: one byte OUI_TYPE */
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.len = (uint8) frag_len +
		VNDR_IE_MIN_LEN + 1;

	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[0] = 0x00;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[1] = 0x50;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.oui[2] = 0xf2;
	ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[0] = ouitype;

	for (i = 0; i < frag_len; i++) {
		ie_setbuf->vndr_ie_buffer.vndr_ie_list[0].vndr_ie_data.data[i+1] = frag[i];
	}

#ifdef _TUDEBUGTRACE
	printf("\n_set_vndr_ie (%s, frag_len=%d)\n", _pktflag_name(pktflag), frag_len);
	for (i = 0; i < frag_len; i++) {
		if (i && !(i%16))
			printf("\n");
		printf("%02x ", frag[i]);
	}
	printf("\n");
#endif // endif
	err = wps_iovar_set("vndr_ie", ie_setbuf, buflen);

	free(ie_setbuf);

	return err;
}

/* Parsing TLV format WPS IE */
static unsigned char *
_get_frag_wps_ie(unsigned char *p_data, int length, int *frag_len, int max_frag_len)
{
	int next_tlv_len, total_len = 0;
	uint16 type;
	unsigned char *next;

	if (!p_data || !frag_len || max_frag_len < 4)
		return NULL;

	if (length <= max_frag_len) {
		*frag_len = length;
		return p_data;
	}

	next = p_data;
	while (1) {
		type = WpsNtohs(next);
		next += 2; /* Move to L */
		next_tlv_len = WpsNtohs(next) + 4; /* Include Type and Value 4 bytes */
		next += 2; /* Move to V */
		if (next_tlv_len > max_frag_len) {
			TUTRACE((TUTRACE_ERR, "Error, there is a TLV length %d bigger than "
				"Max fragment length %d. Unable to fragment it.\n",
				next_tlv_len, max_frag_len));
			return NULL;
		}

		/* Abnormal IE check */
		if ((total_len + next_tlv_len) > length) {
			TUTRACE((TUTRACE_ERR, "Error, Abnormal WPS IE.\n"));
			*frag_len = length;
			return p_data;
		}

		/* Fragment point check */
		if ((total_len + next_tlv_len) > max_frag_len) {
			*frag_len = total_len;
			return p_data;
		}

		/* Get this TLV length */
		total_len += next_tlv_len;
		next += (next_tlv_len - 4); /* Move to next TLV */
	}

}

int
create_wps_ie(bool pbc, unsigned int pktflag)
{
	int err = 0;

	if (pktflag == VNDR_IE_PRBREQ_FLAG) {
		if (pbc)
			err = wps_build_pbc_proberq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
		else
			err = wps_build_def_proberq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
	}
	else if (pktflag == VNDR_IE_ASSOCREQ_FLAG) {
		err = wps_build_def_assocrq(wps_ie_setbuf, sizeof(wps_ie_setbuf));
	}
	else
		return -1;

	if (err == WPS_SUCCESS)
		err = 0;

	return err;
}

#ifdef WFA_WPS_20_TESTBED
static char *
_get_update_partial_ie(unsigned int pktflag, uint8 *updie_setbuf, uint8 *updie_len)
{
	char *wps_updie;
	unsigned char *src, *dest;
	unsigned char val;
	int idx, len;
	char hexstr[3];

	if (updie_setbuf == NULL)
		return NULL;

	/* Get environment variable */
	switch (pktflag) {
	case VNDR_IE_PRBREQ_FLAG:
		wps_updie = wps_get_conf("wps_prbreq");
		break;
	case VNDR_IE_ASSOCREQ_FLAG:
		wps_updie = wps_get_conf("wps_assocreq");
		break;
	default:
		TUTRACE((TUTRACE_ERR, "unknown frame type\n"));
		return NULL;
	}

	if (wps_updie == NULL)
		return NULL;

	/* reset first */
	*updie_len = 0;

	/* Ensure in 2 characters long */
	len = strlen(wps_updie);
	if (len % 2) {
		TUTRACE((TUTRACE_ERR, "Please specify all the data bytes for this IE\n"));
		return NULL;
	}
	*updie_len = (uint8) (len / 2);

	/* string to hex */
	src = (unsigned char*)wps_updie;
	dest = updie_setbuf;
	for (idx = 0; idx < len; idx++) {
		hexstr[0] = src[0];
		hexstr[1] = src[1];
		hexstr[2] = '\0';

		val = (unsigned char) strtoul(hexstr, NULL, 16);

		*dest++ = val;
		src += 2;
	}

	return updie_setbuf;
}
#endif /* WFA_WPS_20_TESTBED */

/* add probe request for enrollee */
static int
_add_wps_ie(bool pbc, unsigned int pktflag)
{
	int frag_len;
	int wps_ie_len;
	int err = 0;
	unsigned char *frag, *wps_ie;
#ifdef WFA_WPS_20_TESTBED
	char *wps_ie_frag;
	int frag_threshold;
	uint8 updie_setbuf[2048];
	uint8 updie_len = 0;
#endif /* WFA_WPS_20_TESTBED */
	int frag_max = WLC_IOCTL_SMLEN - sizeof(vndr_ie_setbuf_t) - strlen("vndr_ie") - 1;

	if (pktflag != VNDR_IE_PRBREQ_FLAG && pktflag != VNDR_IE_ASSOCREQ_FLAG) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "_add_wps_ie : unsupported pktflag 0x%x\n", pktflag));
#endif // endif
		return -1;
	}

	/* WSC 2.0 */
	if (create_wps_ie(pbc, pktflag) != 0) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "_add_wps_ie : Create WPS IE failed\n"));
#endif // endif
		return -1;
	}

	/*
	 * wps_ie_setbuf:
	 * [0] = len
	 * [1~3] = 0x00:0x50:0xF2
	 * [4] = 0x04
	 */
	wps_ie = &wps_ie_setbuf[5];
	wps_ie_len = wps_ie_setbuf[0] - 4;

	/* try removing first in case there was one left from previous call */
	rem_wps_ie(NULL, 0, pktflag);

#ifdef WFA_WPS_20_TESTBED
	/* WPS IE fragment threshold */
	wps_ie_frag = wps_get_conf("wps_ie_frag");
	if (wps_ie_frag) {
		frag_threshold = atoi(wps_ie_frag);
		/* 72 = OUI + OUITYPE + TLV, V <=64 case */
		if (frag_threshold > frag_max || frag_threshold < 72) {
			TUTRACE((TUTRACE_ERR, "NVRAM specified WPS IE fragment threshold "
				"wps_ie_frag %s is invalid.\n"));
		}
		else {
			frag_max = frag_threshold;
		}
	}

	/* Update partial WPS IE */
	if (_get_update_partial_ie(pktflag, updie_setbuf, &updie_len) != NULL) {
		if (wps_update_partial_ie(wps_ie_setbuf, sizeof(wps_ie_setbuf),
			wps_ie, (uint8)wps_ie_len, updie_setbuf, updie_len) != WPS_SUCCESS) {
			printf("Failed to update partial WPS IE in %s\n",
				(pktflag == VNDR_IE_PRBREQ_FLAG) ? "probereq" : "assocreq");
			return -1;
		}

		/* update new length */
		wps_ie_len = wps_ie_setbuf[0] - 4;
	}
#endif /* WFA_WPS_20_TESTBED */

	/* Separate a big IE to fragment IEs */
	frag = wps_ie;
	frag_len = wps_ie_len;
	while (wps_ie_len > 0) {
		if (wps_ie_len > frag_max)
			/* Find a appropriate fragment point */
			frag = _get_frag_wps_ie(frag, wps_ie_len, &frag_len, frag_max);

		if (!frag)
			return -1;

		/* Set fragment WPS IE */
		err |= _set_vndr_ie(frag, frag_len, 0x04, pktflag);

		/* Move to next */
		wps_ie_len -= frag_len;
		frag += frag_len;
		frag_len = wps_ie_len;
	}

	return (err);
}

int
add_wps_ie(unsigned char *p_data, int length, bool pbc, bool b_wps_version2)
{
	int err = 0;

	/* Add WPS IE in probe request */
	if ((err = _add_wps_ie(pbc, VNDR_IE_PRBREQ_FLAG)) != 0) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "add_wps_ie : Add WPS IE in probe request failed\n"));
#endif // endif
		return err;
	}

	/* Add WPS IE in associate request */
	if (b_wps_version2 && (err = _add_wps_ie(pbc, VNDR_IE_ASSOCREQ_FLAG)) != 0) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "add_wps_ie : Add WPS IE in associate request failed\n"));
#endif // endif
		return err;
	}

	return 0;

}

int
rem_wps_ie(unsigned char *p_data, int length, unsigned int pktflag)
{
	int i, err = 0;
	char getbuf[WPS_IE_BUF_LEN] = {0};
	vndr_ie_buf_t *iebuf;
	vndr_ie_info_t *ieinfo;
	char wps_oui[4] = {0x00, 0x50, 0xf2, 0x04};
	char *bufaddr;
	int buflen = 0;
	int found = 0;
	uint32 ieinfo_pktflag;

	if (pktflag != VNDR_IE_PRBREQ_FLAG && pktflag != VNDR_IE_ASSOCREQ_FLAG) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "rem_wps_ie : unsupported pktflag 0x%x\n", pktflag));
#endif // endif
		return -1;
	}

	/* Get all WPS IEs in probe request IE */
	if (wl_iovar_get("vndr_ie", getbuf, WPS_IE_BUF_LEN)) {
#ifdef _TUDEBUGTRACE
		TUTRACE((TUTRACE_ERR, "rem_wps_ie : No IE to remove\n"));
#endif // endif
		return -1;
	}

	iebuf = (vndr_ie_buf_t *) getbuf;
	bufaddr = (char*) iebuf->vndr_ie_list;

	/* Delete ALL specified ouitype IEs */
	for (i = 0; i < iebuf->iecount; i++) {
		ieinfo = (vndr_ie_info_t*) bufaddr;
		bcopy(bufaddr, (char*)&ieinfo_pktflag, (int) sizeof(uint32));
		if (ieinfo_pktflag == pktflag) {
			if (!memcmp(ieinfo->vndr_ie_data.oui, wps_oui, 4)) {
				found = 1;
				bufaddr = (char*) &ieinfo->vndr_ie_data;
				buflen = (int)ieinfo->vndr_ie_data.len + VNDR_IE_HDR_LEN;
				/* Delete one vendor IE */
				err |= _del_vndr_ie(bufaddr, buflen, pktflag);
			}
		}
		bufaddr = (char*)(ieinfo->vndr_ie_data.oui + ieinfo->vndr_ie_data.len);
	}

	if (!found)
		return -1;

	return (err);
}

int
join_network(char* ssid, uint32 wsec)
{
	int ret = 0;
	wlc_ssid_t ssid_t;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;

#ifdef _TUDEBUGTRACE
	TUTRACE((TUTRACE_INFO, "Joining network %s - %d\n", ssid, wsec));
#endif // endif
	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 4; /* AES */
	ssid_t.SSID_len = strlen(ssid);
	strncpy((char *)ssid_t.SSID, ssid, ssid_t.SSID_len);

	/* set infrastructure mode */
	infra = htod32(infra);
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set authentication mode */
	auth = htod32(auth);
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec mode */
	wsec = htod32(wsec);
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	wpa_auth = htod32(wpa_auth);
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	ret = wps_ioctl_set(WLC_SET_SSID, &ssid_t, sizeof(wlc_ssid_t));

	return ret;
}

int
join_network_with_bssid(char* ssid, uint32 wsec, char *bssid)
{
#if !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE)
	return (join_network(ssid, wsec));
#else
	int ret = 0;
	int auth = 0, infra = 1;
	int wpa_auth = WPA_AUTH_DISABLED;
	wl_join_params_t join_params;
	wlc_ssid_t *ssid_t = &join_params.ssid;
	wl_assoc_params_t *params_t = &join_params.params;

	TUTRACE((TUTRACE_ERR, "Joining network %s - %d\n", ssid, wsec));

	memset(&join_params, 0, sizeof(join_params));

	/*
	 * If wep bit is on,
	 * pick any WPA encryption type to allow association.
	 * Registration traffic itself will be done in clear (eapol).
	*/
	if (wsec)
		wsec = 4; /* AES */

	/* ssid */
	ssid_t->SSID_len = strlen(ssid);
	strncpy((char *)ssid_t->SSID, ssid, ssid_t->SSID_len);

	/* bssid (if any) */
	if (bssid)
		memcpy(&params_t->bssid, bssid, ETHER_ADDR_LEN);
	else
		memcpy(&params_t->bssid, &ether_bcast, ETHER_ADDR_LEN);

	/* set infrastructure mode */
	infra = htod32(infra);
	if ((ret = wps_ioctl_set(WLC_SET_INFRA, &infra, sizeof(int))) < 0)
		return ret;

	/* set authentication mode */
	auth = htod32(auth);
	if ((ret = wps_ioctl_set(WLC_SET_AUTH, &auth, sizeof(int))) < 0)
		return ret;

	/* set wsec mode */
	wsec = htod32(wsec);
	if ((ret = wps_ioctl_set(WLC_SET_WSEC, &wsec, sizeof(int))) < 0)
		return ret;

	/* set WPA_auth mode */
	wpa_auth = htod32(wpa_auth);
	if ((ret = wps_ioctl_set(WLC_SET_WPA_AUTH, &wpa_auth, sizeof(wpa_auth))) < 0)
		return ret;

	/* set ssid */
	ret = wps_ioctl_set(WLC_SET_SSID, &join_params, sizeof(wl_join_params_t));

	return ret;
#endif /* !defined(WL_ASSOC_PARAMS_FIXED_SIZE) || !defined(WL_JOIN_PARAMS_FIXED_SIZE) */
}

int
leave_network()
{
	return wps_ioctl_set(WLC_DISASSOC, NULL, 0);
}

int
wps_get_bssid(char *bssid)
{
	return wps_ioctl_get(WLC_GET_BSSID, bssid, 6);
}

int wps_get_ssid(char *ssid, int *len)
{
	int ret;
	wlc_ssid_t wlc_ssid;

	if ((ret = wps_ioctl_get(WLC_GET_SSID, &wlc_ssid, sizeof(wlc_ssid))) < 0)
		return ret;

	*len = wlc_ssid.SSID_len;
	strncpy(ssid, (char*)wlc_ssid.SSID, *len);

	return ret;
}

int
wps_get_bands(uint *band_num, uint *active_band)
{
	int ret;
	uint list[3];

	*band_num = 0;
	*active_band = 0;

	if ((ret = wps_ioctl_get(WLC_GET_BANDLIST, list, sizeof(list))) < 0) {
		return ret;
	}

	list[0] = dtoh32(list[0]);
	list[1] = dtoh32(list[1]);
	list[2] = dtoh32(list[2]);

	/* list[0] is count, followed by 'count' bands */
	if (list[0] > 2)
		list[0] = 2;
	*band_num = list[0];

	/* list[1] is current band type */
	*active_band = list[1];

	return ret;
}

int
do_wpa_psk(WpsEnrCred* credential)
{
	return -1;
}

int
wps_ioctl_get(int cmd, void *buf, int len)
{
	return wps_wl_ioctl(cmd, buf, len, FALSE);
}

int
wps_ioctl_set(int cmd, void *buf, int len)
{
	return wps_wl_ioctl(cmd, buf, len, TRUE);
}

/*
 * set named iovar given the parameter buffer
 * iovar name is converted to lower case
 */
static int
wps_iovar_set(const char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	memset(smbuf, 0, sizeof(smbuf));

	return wps_iovar_setbuf(iovar, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * set named iovar providing both parameter and i/o buffers
 * iovar name is converted to lower case
 */
static int
wps_iovar_setbuf(const char *iovar,
	void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	int iolen;

	iolen = wps_iovar_mkbuf(iovar, param, paramlen, bufptr, buflen, &err);
	if (err)
		return err;

	return wps_ioctl_set(WLC_SET_VAR, bufptr, iolen);
}

static int
wl_iovar_get(char *iovar, void *bufptr, int buflen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int ret;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (buflen > sizeof(smbuf)) {
		ret = wl_iovar_getbuf(iovar, NULL, 0, bufptr, buflen);
	} else {
		ret = wl_iovar_getbuf(iovar, NULL, 0, smbuf, sizeof(smbuf));
		if (ret == 0)
			memcpy(bufptr, smbuf, buflen);
	}

	return ret;
}

static int
wl_iovar_getbuf(char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	 /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (-1);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8*)bufptr + namelen, param, paramlen);

	err = wps_ioctl_set(WLC_GET_VAR, bufptr, buflen);

	return (err);
}

/*
 * format an iovar buffer
 * iovar name is converted to lower case
 */
static uint
wps_iovar_mkbuf(const char *name, char *data, uint datalen, char *iovar_buf, uint buflen, int *perr)
{
	uint iovar_len;
	char *p;

	iovar_len = strlen(name) + 1;

	/* check for overflow */
	if ((iovar_len + datalen) > buflen) {
		*perr = -1;
		return 0;
	}

	/* copy data to the buffer past the end of the iovar name string */
	if (datalen > 0)
		memmove(&iovar_buf[iovar_len], data, datalen);

	/* copy the name to the beginning of the buffer */
	strcpy(iovar_buf, name);

	/* wl command line automatically converts iovar names to lower case for
	 * ease of use
	 */
	p = iovar_buf;
	while (*p != '\0') {
		*p = tolower((int)*p);
		p++;
	}

	*perr = 0;
	return (iovar_len + datalen);
}
