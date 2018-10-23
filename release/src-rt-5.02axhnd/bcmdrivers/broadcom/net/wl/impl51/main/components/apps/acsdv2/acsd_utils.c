/*
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
 * $Id: acsd_utils.c 756552 2018-04-09 20:06:37Z $
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmtimer.h>
#include <bcmendian.h>
#include <shutils.h>
#include <bcmendian.h>
#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <ctype.h> /* isprint() */
#include "acsd.h"
#include "acsd_svr.h"

bool acsd_swap = FALSE;
#ifdef ACS_DEBUG
int acsd_debug_level = ACSD_DEBUG_ERROR | ACSD_DEBUG_INFO | ACSD_DEBUG_SYSLOG;
#else
int acsd_debug_level = ACSD_DEBUG_ERROR;
#endif

static const char *
capmode2str(uint16 capability)
{
	capability &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);

	if (capability == DOT11_CAP_ESS)
		return "Managed";
	else if (capability == DOT11_CAP_IBSS)
		return "Ad Hoc";
	else
		return "<unknown>";
}

int
wl_format_ssid(char* ssid_buf, uint8* ssid, int ssid_len)
{
	int i, c;
	char *p = ssid_buf;

	if (ssid_len > 32) ssid_len = 32;

	for (i = 0; i < ssid_len; i++) {
		c = (int)ssid[i];
		if (c == '\\') {
			*p++ = '\\';
			*p++ = '\\';
		} else if (isprint((uchar)c)) {
			*p++ = (char)c;
		} else {
			p += sprintf(p, "\\x%02X", c);
		}
	}
	*p = '\0';

	return p - ssid_buf;
}

char *
wl_ether_etoa(const struct ether_addr *n)
{
	static char etoa_buf[ETHER_ADDR_LEN * 3];
	char *c = etoa_buf;
	int i;

	for (i = 0; i < ETHER_ADDR_LEN; i++) {
		if (i)
			*c++ = ':';
		c += sprintf(c, "%02X", n->octet[i] & 0xff);
	}
	return etoa_buf;
}

void
dump_rateset(uint8 *rates, uint count)
{
	uint i;
	uint r;
	bool b;

	acsddbg("[ ");
	for (i = 0; i < count; i++) {
		r = rates[i] & 0x7f;
		b = rates[i] & 0x80;
		if (r == 0)
			break;
		acsddbg("%d%s%s ", (r / 2), (r % 2)?".5":"", b?"(b)":"");
	}
	acsddbg("]");
}

void
dump_bss_info(wl_bss_info_t *bi)
{
	char ssidbuf[SSID_FMT_BUF_LEN];
	wl_bss_info_107_t *old_bi;
	int mcs_idx = 0;

	/* Convert version 107 to 109 */
	if (dtoh32(bi->version) == LEGACY_WL_BSS_INFO_VERSION) {
		old_bi = (wl_bss_info_107_t *)bi;
		bi->chanspec = CH20MHZ_CHSPEC(old_bi->channel);
		bi->ie_length = old_bi->ie_length;
		bi->ie_offset = sizeof(wl_bss_info_107_t);
	}

	wl_format_ssid(ssidbuf, bi->SSID, bi->SSID_len);

	acsddbg("SSID: \"%s\"\n", ssidbuf);

	acsddbg("Mode: %s\t", capmode2str(dtoh16(bi->capability)));
	acsddbg("RSSI: %d dBm\t", (int16)(dtoh16(bi->RSSI)));

	/*
	 * SNR has valid value in only 109 version.
	 * So print SNR for 109 version only.
	 */
	if (dtoh32(bi->version) == WL_BSS_INFO_VERSION) {
		acsddbg("SNR: %d dB\t", (int16)(dtoh16(bi->SNR)));
	}

	acsddbg("noise: %d dBm\t", bi->phy_noise);
	if (bi->flags) {
		bi->flags = dtoh16(bi->flags);
		acsddbg("Flags: ");
		if (bi->flags & WL_BSS_FLAGS_FROM_BEACON) acsddbg("FromBcn ");
		if (bi->flags & WL_BSS_FLAGS_FROM_CACHE) acsddbg("Cached ");
		if (bi->flags & WL_BSS_FLAGS_RSSI_ONCHANNEL) acsddbg("RSSI on-channel ");
		acsddbg("\t");
	}
	acsddbg("chanspec: 0x%4x (%s)\n", dtohchanspec(bi->chanspec), wf_chspec_ntoa(bi->chanspec, chanspecbuf));

	acsddbg("BSSID: %s\t", wl_ether_etoa(&bi->BSSID));

	acsddbg("Capability: ");
	bi->capability = dtoh16(bi->capability);
	if (bi->capability & DOT11_CAP_ESS) acsddbg("ESS ");
	if (bi->capability & DOT11_CAP_IBSS) acsddbg("IBSS ");
	if (bi->capability & DOT11_CAP_POLLABLE) acsddbg("Pollable ");
	if (bi->capability & DOT11_CAP_POLL_RQ) acsddbg("PollReq ");
	if (bi->capability & DOT11_CAP_PRIVACY) acsddbg("WEP ");
	if (bi->capability & DOT11_CAP_SHORT) acsddbg("ShortPre ");
	if (bi->capability & DOT11_CAP_PBCC) acsddbg("PBCC ");
	if (bi->capability & DOT11_CAP_AGILITY) acsddbg("Agility ");
	if (bi->capability & DOT11_CAP_SHORTSLOT) acsddbg("ShortSlot ");
	if (bi->capability & DOT11_CAP_CCK_OFDM) acsddbg("CCK-OFDM ");
	acsddbg("\n");

	acsddbg("Supported Rates: ");
	dump_rateset(bi->rateset.rates, dtoh32(bi->rateset.count));
	acsddbg("\n");

	if (dtoh32(bi->version) != LEGACY_WL_BSS_INFO_VERSION && bi->n_cap) {
		acsddbg("802.11N Capable:\n");
		bi->chanspec = dtohchanspec(bi->chanspec);
		acsddbg("\tChanspec: %sGHz channel %d %dMHz (0x%4x %s)\n",
			CHSPEC_IS2G(bi->chanspec)?"2.4":"5", CHSPEC_CHANNEL(bi->chanspec),
			CHSPEC_IS40(bi->chanspec) ? 40 : (CHSPEC_IS20(bi->chanspec) ? 20 : 10),
			bi->chanspec, wf_chspec_ntoa(bi->chanspec, chanspecbuf));
		acsddbg("\tControl channel: %d\n", bi->ctl_ch);
		acsddbg("\t802.11N Capabilities: ");
		if (dtoh32(bi->nbss_cap) & HT_CAP_40MHZ)
			acsddbg("40Mhz ");
		if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_20)
			acsddbg("SGI20 ");
		if (dtoh32(bi->nbss_cap) & HT_CAP_SHORT_GI_40)
			acsddbg("SGI40 ");
		if (dtoh32(bi->nbss_cap) & VHT_BI_SGI_80MHZ)
			acsddbg("SGI80 ");
		acsddbg("\n\tSupported MCS : [ ");
		for (mcs_idx = 0; mcs_idx < (MCSSET_LEN * 8); mcs_idx++)
			if (isset(bi->basic_mcs, mcs_idx))
				acsddbg("%d ", mcs_idx);
		acsddbg("]\n");
	}

	acsddbg("\n");
}

void
dump_networks(char *network_buf)
{
	wl_scan_results_t *list = (wl_scan_results_t*)network_buf;
	wl_bss_info_t *bi;
	uint i;

	if (list->count == 0)
		return;
	else if (list->version != WL_BSS_INFO_VERSION &&
	         list->version != LEGACY2_WL_BSS_INFO_VERSION &&
	         list->version != LEGACY_WL_BSS_INFO_VERSION) {
		fprintf(stderr, "Sorry, your driver has bss_info_version %d "
			"but this program supports only version %d.\n",
			list->version, WL_BSS_INFO_VERSION);
		return;
	}

	bi = list->bss_info;
	for (i = 0; i < list->count; i++, bi = (wl_bss_info_t*)((int8*)bi + dtoh32(bi->length))) {
		dump_bss_info(bi);
	}
}

char *
acsd_malloc(int bufsize)
{
	char *buf = NULL;

	buf = malloc(bufsize);

	if (!buf) {
		ACSD_ERROR("failed to allocate %d bytes\n", bufsize);
		perror("acs_malloc failed");
		exit(-1);
	}

	memset(buf, 0, bufsize);
	ACSD_DEBUG("address: %p, size %d\n", buf, bufsize);
	return buf;
}

char *
acsd_realloc(char *buf, int bufsize)
{
	buf = realloc(buf, bufsize);

	if (!buf) {
		ACSD_ERROR("failed to re-allocate %d bytes\n", bufsize);
		perror("acs_realloc failed");
		exit(-1);
	}

	ACSD_DEBUG("address:%p, size %d\n", buf, bufsize);
	return buf;
}

int
acs_safe_get_conf(char *outval, int outval_size, char *name)
{
	char *val;

	if (name == NULL || outval == NULL) {
		if (outval)
			memset(outval, 0, outval_size);
		return -1;
	}

	val = nvram_safe_get(name);
	if (!strcmp(val, ""))
		memset(outval, 0, outval_size);
	else
		snprintf(outval, outval_size, "%s", val);
	return 0;
}

/*
 *   Wrapper on snprintf returns exactly used number of bytes excluding trailing null byte.
 *   Note: More importantly, this avoids snprintf return value crossing size limit
 */
int
acs_snprintf(char *str, size_t size, const char *format, ...)
{
	int ret;
	va_list va;

	va_start(va, format);
	ret = vsnprintf(str, size, format, va);
	va_end(va);

	return (ret < size ? ret : (size - 1));
}

int
swrite(int fd, char *buf, unsigned int size)
{
	int ret = 0, len = 0;

	do {
		errno = 0;
		ret = write(fd, &buf[len], size - len);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			else
				break;
		}
		if (ret > 0)
			len += ret;
	} while (len < size);

	return ((len > 0) ? len:ret);
}

/*
 * Reads size bytes into from a given device.
 * Handles the read failures because of signals.
 */

int
sread(int fd, char *buf, unsigned int size)
{
	int ret = 0, len = 0;

	do {
		errno = 0;
		ret = read(fd, &buf[len], size - len);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			else
				break;
		}
		if (ret > 0)
			len += ret;
	} while ((len < size) && ret);

	return ((len > 0) ? len:ret);
}

void
sleep_ms(const unsigned int ms)
{
	usleep(1000*ms);
}

const char *
acs_ch_score_name(const int ch_score_index)
{
	static const char *score_names[CH_SCORE_MAX] = {
		"BSS", "busy", "interf.", "itf_adj",
		"fcs", "txpower", "bgnoise", "TOTAL",
		"CNS", "ADJ", "TXOP"
	};

	return (ch_score_index < CH_SCORE_MAX) ? score_names[ch_score_index] : "?range";
}

void
acs_dump_score(ch_score_t * score_p)
{
	int i;
	int score, weight, subtotal;

	acsddbg("Channel Score Breakdown:\n");
	acsddbg("Fact\t Score   \t Weight  \t SubTotal\t\n");

	for (i = 0; i < CH_SCORE_MAX; i++) {
		if (!score_p[i].score)
			continue;
		score = score_p[i].score;
		weight = score_p[i].weight;
		subtotal = score * weight;
		acsddbg("%s\t %8d\t %8d\t %8d\t\n", acs_ch_score_name(i), score, weight, subtotal);
	}
}

void
acs_dump_score_csv(chanspec_t chspec, ch_score_t * score_p)
{
	int i;
	int score, weight, subtotal;

	acsddbg("CSV DATA: 0x%4x\nTYPE,  ", chspec);
	for (i = 0; i < CH_SCORE_MAX; i++) {
		acsddbg("%8s, ", acs_ch_score_name(i));
	}
	acsddbg("\nSCORE, ");
	for (i = 0; i < CH_SCORE_MAX; i++) {
		acsddbg("%8d, ",score_p[i].score);
	}
	acsddbg("\nWEIGHT,");
	for (i = 0; i < CH_SCORE_MAX; i++) {
		acsddbg("%8d, ",score_p[i].weight);
	}
	acsddbg("\nSUBTOT,");
	for (i = 0; i < CH_SCORE_MAX; i++) {
		score = score_p[i].score;
		weight = score_p[i].weight;
		subtotal = score * weight;
		acsddbg("%8d, ", subtotal);
	}
	acsddbg("\n");
}
