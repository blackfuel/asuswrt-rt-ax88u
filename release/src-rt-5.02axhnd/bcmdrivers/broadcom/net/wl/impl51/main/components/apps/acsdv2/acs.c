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
 * $Id: acs.c 764927 2018-06-11 06:02:47Z $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmendian.h>

#include <bcmwifi_channels.h>
#include <wlioctl.h>
#include <wlioctl_utils.h>
#include <wlutils.h>
#include <shutils.h>
#include <ethernet.h>

#include "acsd_svr.h"

#define PREFIX_LEN 32

/* some channel bounds */
#define ACS_CS_MIN_2G_CHAN	1	/* min channel # in 2G band */
#define ACS_CS_MIN_5G_CHAN	36	/* min channel # in 5G band */
#define ACS_CS_MAX_5G_CHAN	MAXCHANNEL	/* max channel # in 5G band */

/* possible min channel # in the band */
#define ACS_CS_MIN_CHAN(band)	((band == WLC_BAND_5G) ? ACS_CS_MIN_5G_CHAN : \
			(band == WLC_BAND_2G) ? ACS_CS_MIN_2G_CHAN : 0)
/* possible max channel # in the band */
#define ACS_CS_MAX_CHAN(band)	((band == WLC_BAND_5G) ? ACS_CS_MAX_5G_CHAN : \
			(band == WLC_BAND_2G) ? ACS_CS_MAX_2G_CHAN : 0)

/* Need 13, strlen("per_chan_info"), +4, sizeof(uint32). Rounded to 20. */
#define ACS_PER_CHAN_INFO_BUF_LEN 20

#define ACS_DFLT_FLAGS ACS_FLAGS_LASTUSED_CHK

acs_policy_t predefined_policy[ACS_POLICY_MAX] = {
/* threshld    Channel score weigths values                                      chan */
/* bgn  itf  {  BSS  BUSY  INTF I-ADJ   FCS TXPWR NOISE TOTAL   CNS   ADJ TXOP}  pick */
/* --- ----   ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----   func */
{ -65,  40, {   10,    1,    1,    0,    0,    0,    1,    1,    1,    0,  10}, NULL}, /* 0=DEF2G */
{   0, 100, {   10,    0,    1,    1,    0,   10,    1,    1,    1,    1,  10}, NULL}, /* 1=DEF5G */
{ -55,  45, { -200,    0, -100,  -50,    0,    0,  -50,    0,    1,    0,  -1}, NULL}, /* 2=CUST */
};

acs_info_t *acs_info;

/* get traffic information of the interface */
static int acs_get_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info);

/* get traffic information about TOAd video STAs (if any) */
static int acs_get_video_sta_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info);

/* To check whether bss is enabled for particaular interface or not */
static int
acs_check_bss_is_enabled(char *name, acs_chaninfo_t **c_info_ptr, char *prefix);

#ifdef DEBUG
void
acs_dump_config_extra(acs_chaninfo_t *c_info)
{
	uint8 intf_thld_setting = c_info->intfparams.thld_setting;

	ACSD_INFO("acs_dump_config_extra:\n");
	ACSD_INFO("\t acs_txdelay_period: %d\n", c_info->acs_txdelay_period);
	ACSD_INFO("\t acs_txdelay_cnt: %d\n", c_info->acs_txdelay_cnt);
	ACSD_INFO("\t acs_txdelay_ratio: %d\n", c_info->acs_txdelay_ratio);
	ACSD_INFO("\t acs_dfs: %d\n", c_info->acs_dfs);
	ACSD_INFO("\t acs_far_sta_rssi: %d\n", c_info->acs_far_sta_rssi);
	ACSD_INFO("\t acs_nofcs_least_rssi: %d\n", c_info->acs_nofcs_least_rssi);
	ACSD_INFO("\t acs_chan_dwell_time: %d\n", c_info->acs_chan_dwell_time);
	ACSD_INFO("\t acs_chan_flop_period: %d\n", c_info->acs_chan_flop_period);
	ACSD_INFO("\t acs_tx_idle_cnt: %d\n", c_info->acs_tx_idle_cnt);
	ACSD_INFO("\t acs_cs_scan_timer: %d\n", c_info->acs_cs_scan_timer);
	ACSD_INFO("\t acs_ci_scan_timeout: %d\n", c_info->acs_ci_scan_timeout);
	ACSD_INFO("\t acs_ci_scan_timer: %d\n", c_info->acs_ci_scan_timer);
	ACSD_INFO("\t acs_scan_chanim_stats: %d\n", c_info->acs_scan_chanim_stats);
	ACSD_INFO("\t acs_ci_scan_chanim_stats: %d\n", c_info->acs_ci_scan_chanim_stats);
	ACSD_INFO("\t tcptxfail:%d\n",
		c_info->intfparams.acs_txfail_thresholds[intf_thld_setting].tcptxfail_thresh);
	ACSD_INFO("\t txfail:%d\n",
		c_info->intfparams.acs_txfail_thresholds[intf_thld_setting].txfail_thresh);
	ACSD_INFO("\t txop_weight: %d\n", c_info->txop_weight);

}
#endif /* DEBUG */

#ifdef ACS_DEBUG
static void
acs_dump_map(void)
{
	int i;
	ifname_idx_map_t* cur_map;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		cur_map = &acs_info->acs_ifmap[i];
		if (cur_map->in_use) {
			ACSD_PRINT("i: %d, name: %s, idx: %d, in_use: %d\n",
				i, cur_map->name, cur_map->idx, cur_map->in_use);
		}
	}
}
#endif /* ACS_DEBUG */

static void
acs_add_map(char *name)
{
	int i;
	ifname_idx_map_t* cur_map = acs_info->acs_ifmap;
	size_t length = strlen(name);

	ACSD_DEBUG("add map entry for ifname: %s\n", name);

	if (length >= sizeof(cur_map->name)) {
		ACSD_ERROR("Interface Name Length Exceeded\n");
	} else {
		for (i = 0; i < ACS_MAX_IF_NUM; cur_map++, i++) {
			if (!cur_map->in_use) {
				memcpy(cur_map->name, name, length + 1);
				cur_map->idx = i;
				cur_map->in_use = TRUE;
				break;
			}
		}
	}
#ifdef ACS_DEBUG
	acs_dump_map();
#endif // endif
}

int
acs_idx_from_map(char *name)
{
	int i;
	ifname_idx_map_t *cur_map;

#ifdef ACS_DEBUG
	acs_dump_map();
#endif // endif
	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		cur_map = &acs_info->acs_ifmap[i];
		if (cur_map->in_use && !strcmp(name, cur_map->name)) {
			ACSD_DEBUG("name: %s, cur_map->name: %s idx: %d\n",
				name, cur_map->name, cur_map->idx);
			return cur_map->idx;
		}
	}
	ACSD_ERROR("cannot find the mapped entry for ifname: %s\n", name);
	return -1;
}

/* radio setting information needed from the driver */
static int
acs_get_rs_info(acs_chaninfo_t * c_info, char* prefix)
{
	int ret = 0;
	char tmp[100];
	int band, pref_chspec = 0, coex;
	acs_rsi_t *rsi = &c_info->rs_info;
	char *str;
	char data_buf[100];
	acs_param_info_t param;
	/*
	 * Check if the user set the "chanspec" nvram. If not, check if
	 * the "channel" nvram is set for backward compatibility.
	 */
	if ((str = nvram_get(strcat_r(prefix, "chanspec", tmp))) == NULL) {
		str = nvram_get(strcat_r(prefix, "channel", tmp));
	}

	if (str && strcmp(str, "0")) {
		ret = acs_get_chanspec(c_info, &pref_chspec);
		ACS_ERR(ret, "failed to get chanspec");

		rsi->pref_chspec = dtoh32(pref_chspec);
	}

	ret = acs_get_obss_coex_info(c_info, &coex);
	ACS_ERR(ret, "failed to get obss_coex");

	rsi->coex_enb = dtoh32(coex);
	ACSD_INFO("coex_enb: %d\n",  rsi->coex_enb);

	ret = wl_ioctl(c_info->name, WLC_GET_BAND, &band, sizeof(band));
	ACS_ERR(ret, "failed to get band info");

	rsi->band_type = dtoh32(band);
	ACSD_INFO("band_type: %d\n",  rsi->band_type);

	memset(&param, 0, sizeof(param));
	param.band = band;

	ret = acs_get_bwcap_info(c_info, &param, sizeof(param),	data_buf, sizeof(data_buf));
	ACS_ERR(ret, "failed to get bw_cap");

	rsi->bw_cap = *((uint32 *)data_buf);
	ACSD_INFO("bw_cap: %d\n",  rsi->bw_cap);

	return ret;
}

/*
 * acs_pick_chanspec_default2g() - default policy function to pick a chanspec to switch to.
 *
 * c_info:	pointer to the acs_chaninfo_t for this interface.
 * bw:		bandwidth to chose from
 *
 * Returned value:
 *	The returned value is the most preferred valid chanspec from the candidate array.
 *
 * This function picks the most preferred chanspec according to the default policy.
 */
static chanspec_t
acs_pick_chanspec_default2g(acs_chaninfo_t* c_info, int bw)
{
	return acs_pick_chanspec_common(c_info, bw, CH_SCORE_TOTAL);
}

/*
 * acs_pick_chanspec_default5g() - default policy function to pick a chanspec to switch to.
 *
 * c_info:	pointer to the acs_chaninfo_t for this interface.
 * bw:		bandwidth to chose from
 *
 * Returned value:
 *	The returned value is the most preferred valid chanspec from the candidate array.
 *
 * This function picks the most preferred chanspec according to the default policy.
 */
static chanspec_t
acs_pick_chanspec_default5g(acs_chaninfo_t* c_info, int bw)
{
	return acs_pick_chanspec_common(c_info, bw, CH_SCORE_TOTAL);
}

void
acs_default_policy(acs_policy_t *a_pol, acs_policy_index index)
{

	memcpy(a_pol, &predefined_policy[index], sizeof(acs_policy_t));

	if (index == ACS_POLICY_DEFAULT2G) {
		a_pol->chan_selector = acs_pick_chanspec_default2g;
	} else if (index == ACS_POLICY_DEFAULT5G) {
		a_pol->chan_selector = acs_pick_chanspec_default5g;
	} else {
		a_pol->chan_selector = acs_pick_chanspec;
	}
}

#ifdef DEBUG
void
acs_dump_policy(acs_policy_t *a_pol)
{
	acsddbg("ACS Policy:\n");
	acsddbg("Bg Noise threshold: %d\n", a_pol->bgnoise_thres);
	acsddbg("Interference threshold: %d\n", a_pol->intf_threshold);
	acsddbg("Channel Scoring Weights: \n");
	acsddbg("\t BSS: %d\n", a_pol->acs_weight[CH_SCORE_BSS]);
	acsddbg("\t BUSY: %d\n", a_pol->acs_weight[CH_SCORE_BUSY]);
	acsddbg("\t INTF: %d\n", a_pol->acs_weight[CH_SCORE_INTF]);
	acsddbg("\t INTFADJ: %d\n", a_pol->acs_weight[CH_SCORE_INTFADJ]);
	acsddbg("\t FCS: %d\n", a_pol->acs_weight[CH_SCORE_FCS]);
	acsddbg("\t TXPWR: %d\n", a_pol->acs_weight[CH_SCORE_TXPWR]);
	acsddbg("\t BGNOISE: %d\n", a_pol->acs_weight[CH_SCORE_BGNOISE]);
	acsddbg("\t CNS: %d\n", a_pol->acs_weight[CH_SCORE_CNS]);
	acsddbg("\t TXOP: %d\n", a_pol->acs_weight[CH_SCORE_TXOP]);

}
#endif /* DEBUG */

/* look for str in capability (wl cap) and return true if found */
bool
acs_check_cap(acs_chaninfo_t *c_info, char *str)
{
	char data_buf[WLC_IOCTL_MAXLEN];
	uint32 ret, param = 0;

	if (str == NULL || strlen(str) >= WLC_IOCTL_SMLEN) {
		ACSD_ERROR("%s invalid needle to look for in cap\n", c_info->name);
		return FALSE;
	}

	ret = acs_get_cap_info(c_info, &param, sizeof(param), data_buf, sizeof(data_buf));

	if (ret != BCME_OK) {
		ACSD_ERROR("%s Error %d in getting cap\n", c_info->name, ret);
		return FALSE;
	}

	data_buf[WLC_IOCTL_MAXLEN - 1] = '\0';
	if (strstr(data_buf, str) == NULL) {
		ACSD_INFO("%s '%s' not found in cap\n", c_info->name, str);
		return FALSE;
	} else {
		ACSD_INFO("%s '%s' found in cap\n", c_info->name, str);
		return TRUE;
	}
}

static int
acs_start(char *name, acs_chaninfo_t *c_info)
{
	int unit;
	char prefix[PREFIX_LEN], tmp[100];
	acs_rsi_t* rsi;
	int ret = 0;

	ACSD_INFO("acs_start for interface %s\n", name);

	ret = wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
	acs_snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	c_info->unit = unit;

	/* check radio */
	if (nvram_match(strcat_r(prefix, "radio", tmp), "0")) {
		ACSD_INFO("ifname %s: radio is off\n", name);
		c_info->mode = ACS_MODE_DISABLE;
		goto acs_start_done;
	}

	acs_retrieve_config(c_info, prefix);

	if ((ret = acs_get_country(c_info)) != BCME_OK)
		ACSD_ERROR("Failed to get country info\n");

	rsi = &c_info->rs_info;
	acs_get_rs_info(c_info, prefix);

	if (rsi->pref_chspec == 0) {
		c_info->mode = ACS_MODE_SELECT;
	}
	else if (rsi->coex_enb &&
		nvram_match(strcat_r(prefix, "nmode", tmp), "-1")) {
		c_info->mode = ACS_MODE_COEXCHECK;
	}
	else
		c_info->mode = ACS_MODE_MONITOR; /* default mode */

	if ((c_info->mode == ACS_MODE_SELECT) && BAND_5G(rsi->band_type) &&
		(nvram_match(strcat_r(prefix, "reg_mode", tmp), "h") ||
		nvram_match(strcat_r(prefix, "reg_mode", tmp), "strict_h"))) {
		rsi->reg_11h = TRUE;
	}

	ret = acsd_chanim_init(c_info);
	if (ret < 0)
		ACS_FREE(c_info);
	ACS_ERR(ret, "chanim init failed\n");

	c_info->trf_thold = acs_check_cap(c_info, ACS_CAP_STRING_TRF_THOLD);

	if (AUTOCHANNEL(c_info) && BAND_5G(rsi->band_type)) {
		if (c_info->trf_thold) {
			acs_intfer_config_trfthold(c_info, prefix);
		} else {
			acs_intfer_config_txfail(c_info);
		}
	}

	/* Do not even allocate a DFS Reentry context on 2.4GHz which does not have DFS channels */
	/* or if 802.11h spectrum management is not enabled. */
	if (BAND_2G(c_info->rs_info.band_type) || (rsi->reg_11h == FALSE)) {
		ACSD_DFSR("DFS Reentry disabled %s\n", (BAND_2G(c_info->rs_info.band_type)) ?
			"on 2.4GHz band" : "as 802.11h is not enabled");
		c_info->acs_dfs = ACS_DFS_DISABLED;
	} else {
		ACS_DFSR_CTX(c_info) = acs_dfsr_init(prefix,
			(BAND_5G(rsi->band_type) && (c_info->acs_dfs == ACS_DFS_REENTRY)),
				c_info->acs_bgdfs);
		ret = (ACS_DFSR_CTX(c_info) == NULL) ? -1 : 0;
		ACS_ERR(ret, "Failed to allocate DFS Reentry context\n");
	}

	/* When acsd starts, retrieve current traffic stats since boot */
	acs_get_initial_traffic_stats(c_info);

	if (!AUTOCHANNEL(c_info) && !COEXCHECK(c_info))
		goto acs_start_done;

	c_info->dyn160_cap = acs_check_cap(c_info, ACS_CAP_STRING_DYN160);
	if (c_info->dyn160_cap) {
		acs_update_dyn160_status(c_info);
	}

	ret = acs_run_cs_scan(c_info);
	ACS_ERR(ret, "cs scan failed\n");

	ACS_FREE(c_info->acs_bss_info_q);

#ifdef ACSD_SEGMENT_CHANIM
	acs_segment_allocate(c_info);
#endif /* ACSD_SEGMENT_CHANIM */

	ret = acs_request_data(c_info);
	ACS_ERR(ret, "request data failed\n");

acs_start_done:
	return ret;
}

static int
acs_check_bss_is_enabled(char *name, acs_chaninfo_t **c_info_ptr, char *prefix)
{
	int index, ret;
	char buf[32] = { 0 }, *bss_check;

	if (strlen(name) >= sizeof((*c_info_ptr)->name)) {
		ACSD_ERROR("Interface Name Length Exceeded\n");
		return BCME_STRLEN;
	}

	if (prefix == NULL || prefix[0] == '\0') {
		strcat_r(name, "_bss_enabled", buf);
	} else {
		strcat_r(prefix, "_bss_enabled", buf);
	}

	bss_check = nvram_safe_get(buf);
	if (atoi(bss_check) != 1) {
		/* this interface is disabled */
		ACSD_INFO("interface is disabled %s\n", name);
		return BCME_DISABLED;
	}

	acs_add_map(name);
	index = acs_idx_from_map(name);

	if (index < 0) {
		ret = ACSD_FAIL;
		ACS_ERR(ret, "Mapped entry not present for interface");
	}

	/* allocate core data structure for this interface */
	*c_info_ptr = acs_info->chan_info[index] =
		(acs_chaninfo_t*)acsd_malloc(sizeof(acs_chaninfo_t));
	strncpy((*c_info_ptr)->name, name, sizeof((*c_info_ptr)->name));
	(*c_info_ptr)->name[sizeof((*c_info_ptr)->name) - 1] = '\0';
	ACSD_INFO("bss enabled for name :%s\n", (*c_info_ptr)->name);
	return BCME_OK;
}

/*
 * Returns the channel info of the chspec passed (by combining per_chan_info of each 20MHz subband)
 */
uint32
acs_channel_info(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	char resbuf[ACS_PER_CHAN_INFO_BUF_LEN];
	int ret;
	uint8 sub_channel;
	chanspec_t sub_chspec;
	uint32 chinfo = 0, max_inactive = 0, sub_chinfo;

	FOREACH_20_SB(chspec, sub_channel) {
		sub_chspec = (uint16) sub_channel;
		ret = acs_get_per_chan_info(c_info, sub_chspec, resbuf, ACS_PER_CHAN_INFO_BUF_LEN);
		if (ret != BCME_OK) {
			ACSD_ERROR("%s Failed to get channel (0x%02x) info: %d\n",
				c_info->name, sub_chspec, ret);
			return 0;
		}

		sub_chinfo = dtoh32(*(uint32 *)resbuf);
		ACSD_DFSR("%s: sub_chspec 0x%04x info %08x (%s, %d minutes)\n",
			c_info->name, sub_chspec, sub_chinfo,
			(sub_chinfo & WL_CHAN_INACTIVE) ? "inactive" :
			((sub_chinfo & WL_CHAN_PASSIVE) ? "passive" : "active"),
			GET_INACT_TIME(sub_chinfo));

		/* combine subband chinfo (except inactive time) using bitwise OR */
		chinfo |= ((~INACT_TIME_MASK) & sub_chinfo);
		/* compute maximum inactive time amongst each subband */
		if (max_inactive < GET_INACT_TIME(sub_chinfo)) {
			max_inactive = GET_INACT_TIME(sub_chinfo);
		}
	}
	/* merge maximum inactive time computed into the combined chinfo */
	chinfo |= max_inactive << INACT_TIME_OFFSET;

	ACSD_DFSR("%s: chanspec 0x%04x (%s) info %08x (%s, %d minutes)\n",
		c_info->name, chspec, wf_chspec_ntoa(chspec, chanspecbuf), chinfo,
		(chinfo & WL_CHAN_INACTIVE) ? "inactive" :
		((chinfo & WL_CHAN_PASSIVE) ? "passive" : "active"),
		GET_INACT_TIME(chinfo));

	return chinfo;
}

bool
acsd_is_lp_chan(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	UNUSED_PARAMETER(c_info);

	/* Need to check with real txpwr */
	if (wf_chspec_ctlchan(chspec) <= LOW_POWER_CHANNEL_MAX) {
		/* <= 80MHz & primary of 160/80p80 */
		return TRUE;
	} else if (CHSPEC_BW_GT(chspec, WL_CHANSPEC_BW_80)) {
		return (wf_chspec_secondary80_channel(chspec) <= LOW_POWER_CHANNEL_MAX);
	}

	return FALSE;
}

bool acs_check_for_nondfs_chan(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t *candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
	for (i = 0; i < c_info->c_count[bw]; i++) {
		if (!candi[i].is_dfs) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

/* check for availability of high power channel present in the list of
 * valid channels to select
 */
bool acs_check_for_hp_chan(acs_chaninfo_t *c_info, int bw)
{
	int i;
	ch_candidate_t* candi;
	bool ret = FALSE;
	candi = c_info->candidate[bw];
#if 0
	acs_rsi_t *rsi = &c_info->rs_info;

	if (WL_BW_CAP_160MHZ(rsi->bw_cap))
#endif
		return FALSE;

	for (i = 0; i < c_info->c_count[bw]; i++) {
		if ((!candi[i].valid) || (candi[i].is_dfs)) {
			continue;
		}

		if (!acsd_is_lp_chan(c_info, candi[i].chspec)) {
			ret = TRUE;
			break;
		}
	}
	return ret;

}

/* check for overlap between the passed channel arguments */
bool acs_check_for_overlap_5g(chanspec_t cur_chspec, chanspec_t candi_chspec)
{
	uint8 channel1, channel2;

	FOREACH_20_SB(candi_chspec, channel1) {
		FOREACH_20_SB(cur_chspec, channel2) {
			if (channel1 == channel2) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

/* for 160M/80p80M/80M/40 bw chanspec,select control chan
 * with max AP number for neighbor friendliness
 *
 * For 80p80 - adjust ctrl chan within primary 80Mhz
 */
chanspec_t
acs_adjust_ctrl_chan(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	chanspec_t selected = chspec;
	acs_chan_bssinfo_t* bss_info = c_info->ch_bssinfo;
	uint8 i, j, max_sb, ch, channel;
	uint8 ctrl_sb[8] = {0}, num_sb[8] = {0};
	uint8 selected_sb, last_chan_idx = 0;
	acs_conf_chspec_t *excl_chans = &(c_info->excl_chans);

	if (nvram_match("acs_ctrl_chan_adjust", "0"))
		return selected;

	if (CHSPEC_ISLE20(selected))
		return selected;

	if (CHSPEC_IS160(selected)) {
		max_sb = 8;
	} else if (CHSPEC_IS80(selected) ||
		CHSPEC_IS8080(selected)) {
		max_sb = 4;
	} else {
		max_sb = 2;
	}

	i = 0;
	/* calulate no. APs for all 20M side bands */
	FOREACH_20_SB(selected, channel) {
		ctrl_sb[i] = channel;

		for (j = last_chan_idx; j < c_info->scan_chspec_list.count; j++) {
			ch = (int)bss_info[j].channel;

			if (ch == ctrl_sb[i]) {
				last_chan_idx = j;
				num_sb[i] = bss_info[j].nCtrl;
				ACSD_INFO("sb:%d channel = %d num_sb = %d\n", i,
						channel, num_sb[i]);
				break;
			}
		}
		i++;
	}

	/* when dyn160 is enabled with DFS on FCC, control ch of 50o must be ULL or higher */
	if (max_sb == 8 && c_info->dyn160_enabled && ACS_11H(c_info) &&
		CHSPEC_IS160(selected) &&
		!acs_is_country_edcrs_eu(c_info->country.ccode) &&
		CHSPEC_CHANNEL(selected) == ACS_DYN160_CENTER_CH) {
		i = 4;
		selected_sb = WL_CHANSPEC_CTL_SB_ULL >> WL_CHANSPEC_CTL_SB_SHIFT;
	} else {
		i = 0;
		selected_sb = (selected & WL_CHANSPEC_CTL_SB_MASK) >>
			WL_CHANSPEC_CTL_SB_SHIFT;
	}

	/* find which valid sideband has max no. APs */
	for (; i < max_sb; i++) {
		bool excl = FALSE;
		selected &= ~(WL_CHANSPEC_CTL_SB_MASK);
		selected |= (i << WL_CHANSPEC_CTL_SB_SHIFT);

		if (excl_chans && excl_chans->count) {
			for (j = 0; j < excl_chans->count; j++) {
				if (selected == excl_chans->clist[j]) {
					excl = TRUE;
					break;
				}
			}
		}

		if (!excl && num_sb[i] > num_sb[selected_sb]) {
			selected_sb = i;
			ACSD_INFO("selected sb so far = %d n_sbs = %d\n",
					selected_sb, num_sb[selected_sb]);
		}
	}

	ACSD_INFO("selected sb: %d\n", selected_sb);
	selected &=  ~(WL_CHANSPEC_CTL_SB_MASK);
	selected |= (selected_sb << WL_CHANSPEC_CTL_SB_SHIFT);
	ACSD_INFO("Final selected chanspec: 0x%4x (%s)\n", selected, wf_chspec_ntoa(selected, chanspecbuf));
	return selected;
}

/*
 * acs_get_txduration - get the overall tx duration
 * c_info - pointer to acs_chaninfo_t for an interface
 * Returns TRUE if tx duration is more than the txblanking threshold
 * Returns FALSE otherwise
 */
bool
acs_get_txduration(acs_chaninfo_t * c_info)
{
	int ret = 0;
	char *data_buf;
	wl_chanim_stats_t *list;
	wl_chanim_stats_t param;
	chanim_stats_t * stats;
	int buflen = ACS_CHANIM_BUF_LEN;
	uint32 count = WL_CHANIM_COUNT_ONE;
	uint8 tx_duration;

	data_buf = acsd_malloc(ACS_CHANIM_BUF_LEN);
	list = (wl_chanim_stats_t *) data_buf;

	param.buflen = htod32(buflen);
	param.count = htod32(count);

	ret = acs_get_chanim_stats(c_info, &param, sizeof(wl_chanim_stats_t), data_buf, buflen);
	if (ret < 0) {
		ACS_FREE(data_buf);
		ACSD_ERROR("failed to get chanim results");
		return FALSE;
	}

	list->buflen = dtoh32(list->buflen);
	list->version = dtoh32(list->version);
	list->count = dtoh32(list->count);

	stats = list->stats;
	stats->chanspec = htod16(stats->chanspec);
	tx_duration = stats->ccastats[CCASTATS_TXDUR];
	ACSD_INFO("chspec 0x%4x (%s) tx_duration %d txblank_th %d\n", stats->chanspec, wf_chspec_ntoa(stats->chanspec, chanspecbuf),
			tx_duration, c_info->acs_bgdfs->txblank_th);
	ACS_FREE(data_buf);
	return (tx_duration > c_info->acs_bgdfs->txblank_th) ? TRUE : FALSE;
}

static void
acs_init_info(acs_info_t ** acs_info_p)
{
	acs_info = (acs_info_t*)acsd_malloc(sizeof(acs_info_t));

	*acs_info_p = acs_info;
}

void
acs_init_run(acs_info_t ** acs_info_p)
{
	char name[16], *next, prefix[PREFIX_LEN], name_enab_if[32] = { 0 }, *vifname, *vif_next;
	acs_chaninfo_t * c_info;
	int ret = 0;
	acs_init_info(acs_info_p);

	foreach(name, nvram_safe_get("acs_ifnames"), next) {
		c_info = NULL;
		osifname_to_nvifname(name, prefix, sizeof(prefix));
		if (acs_check_bss_is_enabled(name, &c_info, prefix) != BCME_OK) {
			strcat(prefix, "_vifs");
			vifname = nvram_safe_get(prefix);
			foreach(name_enab_if, vifname, vif_next) {
				if (acs_check_bss_is_enabled(name_enab_if,
					&c_info, NULL) == BCME_OK) {
					break;
				}
			}
		}
		memset(name, 0, sizeof(name));
		if (c_info != NULL) {
			memcpy(name, c_info->name, strlen(c_info->name) + 1);
		} else {
			continue;
		}
		ret = acs_start(name, c_info);

		if (ret) {
			ACSD_ERROR("acs_start failed for ifname: %s\n", name);
			break;
		}

		if ((AUTOCHANNEL(c_info) || COEXCHECK(c_info)) && 
			!(c_info->wet_enabled && acs_check_assoc_scb(c_info))) {
			/* First call to pick the chanspec for exit DFS chan */
			c_info->switch_reason = APCS_INIT;

			/* call to pick up init cahnspec */
			acs_select_chspec(c_info);
			/* Other APP can request to change the channel via acsd, in that
			 * case proper reason will be provided by requesting APP, For ACSD
			 * USE_ACSD_DEF_METHOD: ACSD's own default method to set channel
			 */
			acs_set_chspec(c_info, TRUE, ACSD_USE_DEF_METHOD);

			ret = acs_update_driver(c_info);
			if (ret)
				ACSD_ERROR("update driver failed\n");

			ACSD_DEBUG("ifname %s - mode: %s\n", name,
			   AUTOCHANNEL(c_info)? "SELECT" :
			   COEXCHECK(c_info)? "COEXCHECK" :
			   ACS11H(c_info)? "11H" : "MONITOR");

			chanim_upd_acs_record(c_info->chanim_info,
				c_info->selected_chspec, APCS_INIT);
		}

		if (c_info->acs_boot_only) {
			c_info->mode = ACS_MODE_DISABLE;
		}
	}
}

/*
 * acs_is_country_edcrs_eu - takes country_code string and
 * returns true if it is a EDCRS_EU country.
 * EDCRS_EU countries follow harmonized ETSI regulations.
 */
bool
acs_is_country_edcrs_eu(char * country_code)
{

	static const char cc_list[][WLC_CNTRY_BUF_SZ] = {
		"AL", "DZ", "AD", "AT",    "AZ", "BE", "BJ", "BT",
		"BA", "BW", "IO", "BG",    "CD", "CI", "HR", "CY",
		"CZ", "DK", "EE", "FO",    "FI", "FR", "GE", "DE",
		"GH", "GI", "GR", "GL",    "GG", "GN", "HU", "IS",
		"IE", "IL", "IT", "JE",    "JO", "KE", "KW", "LV",
		"LB", "LI", "LT", "LU",    "MK", "MG", "MW", "MT",
		"IM", "MU", "MC", "MN",    "ME", "MA", "NL", "NG",
		"NO", "OM", "PK", "PN",    "PL", "PT", "QA", "RO",
		"RW", "SM", "SA", "SN",    "RS", "SK", "SI", "ZA",
		"ES", "SZ", "SE", "CH",    "TH", "TN", "TR", "AE",
		"UG", "GB", "VA", "ZW",    "E0"
	};
	int i, len = sizeof(cc_list)/sizeof(cc_list[0]);

	if (country_code == NULL || strlen(country_code) >= WLC_CNTRY_BUF_SZ) {
		ACSD_ERROR("country null or malformed\n");
		return FALSE;
	}

	for (i = 0; i < len; ++i) {
		if (strncmp(cc_list[i], country_code, WLC_CNTRY_BUF_SZ) == 0) {
			ACSD_INFO("country %s is an EDCRS_EU country (@%d/%d)\n",
					country_code, i, len);
			return TRUE;
		}
	}

	ACSD_INFO("country %s is not an EDCRS_EU country (%d)\n", country_code, len);
	return FALSE;
}

int
acs_update_status(acs_chaninfo_t * c_info)
{
	int ret = 0;
	int cur_chspec;

	ret = wl_iovar_getint(c_info->name, "chanspec", &cur_chspec);
	ACS_ERR(ret, "acs get chanspec failed\n");

	/* return if the channel hasn't changed */
	if ((chanspec_t)dtoh32(cur_chspec) == c_info->cur_chspec) {
		return ret;
	}

	/* To add a acs_record when finding out channel change isn't made by ACSD */
	c_info->cur_chspec = (chanspec_t)dtoh32(cur_chspec);
	c_info->cur_is_dfs = acs_is_dfs_chanspec(c_info, cur_chspec);
	c_info->cur_is_dfs_weather = acs_is_dfs_weather_chanspec(c_info, cur_chspec);
	c_info->is160_bwcap = WL_BW_CAP_160MHZ((c_info->rs_info).bw_cap);

	if (c_info->selected_chspec != c_info->cur_chspec) {
		chanim_upd_acs_record(c_info->chanim_info, c_info->cur_chspec, APCS_NONACSD);
	}

	acs_dfsr_chanspec_update(ACS_DFSR_CTX(c_info), c_info->cur_chspec,
			__FUNCTION__, c_info->name);

	if (acs_get_country(c_info) == BCME_OK) {
		c_info->country_is_edcrs_eu = acs_is_country_edcrs_eu(c_info->country.ccode);
	} else {
		c_info->country_is_edcrs_eu = FALSE;
	}

	ACSD_INFO("%s: chanspec: 0x%4x (%s) is160_bwcap %d is160_upgradable %d, is160_downgradable %d\n",
		c_info->name, c_info->cur_chspec, wf_chspec_ntoa(c_info->cur_chspec, chanspecbuf), c_info->is160_bwcap,
		c_info->is160_upgradable, c_info->is160_downgradable);

	return ret;
}

/**
 * check dyn160_enabled through iovar and if enabled, update phy_dyn_switch score
 */
int
acs_update_dyn160_status(acs_chaninfo_t * c_info)
{
	int ret = 0;
	int dyn160_enabled, phy_dyn_switch;

	/* fetch `wl dyn160` */
	ret = acs_get_dyn160_status(c_info->name, &dyn160_enabled);
	ACS_ERR(ret, "acs get dyn160 failed\n");

	c_info->dyn160_enabled = (dyn160_enabled != 0);

	if (!c_info->dyn160_enabled) {
		c_info->phy_dyn_switch = 0;
		return BCME_OK;
	}

	/* if dyn160 is enabled fetch metric `wl phy_dyn_switch` */
	ret = acs_get_phydyn_switch_status(c_info->name, &phy_dyn_switch);
	ACS_ERR(ret, "acs get phy_dyn_switch failed\n");

	c_info->phy_dyn_switch = (uint8) (phy_dyn_switch & 0xFFu);

	(void) acs_update_oper_mode(c_info);

	c_info->is160_upgradable = c_info->is160_bwcap && !CHSPEC_IS160(c_info->cur_chspec) &&
		!c_info->is_mu_active && c_info->phy_dyn_switch != 1;
	c_info->is160_downgradable = c_info->is160_bwcap && CHSPEC_IS160(c_info->cur_chspec) &&
		c_info->phy_dyn_switch == 1;

	ACSD_INFO("%s phy_dyn_switch: %d is160_upgradable %d is160_downgradable %d \n",
		c_info->name, c_info->phy_dyn_switch, c_info->is160_upgradable,
		c_info->is160_downgradable);

	return BCME_OK;
}

int acs_tx_idle_check(acs_chaninfo_t *c_info)
{
	uint timer = c_info->acs_cs_scan_timer;
	time_t now = uptime();
	char cntbuf[ACSD_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	int full_scan = 0;
	int ret = 0;
	uint32 acs_txframe;

	/* Check for idle period "acs_cs_scan_timer" */
	if ((now - c_info->timestamp_tx_idle) < timer)
		return full_scan;

	ACSD_INFO("acs_tx_idle: now %u(%u)\n", (uint)now, c_info->timestamp_tx_idle);

	/* Check wl transmit activity and trigger full scan if it is idle */
	ret = acs_get_dfsr_counters(c_info->name, cntbuf);
	if (ret < 0) {
		ACSD_ERROR("wl counters failed (%d)\n", ret);
		return full_scan;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	/* As we need only wlc layer ctrs here, no need to input corerev.  */
	ret = wl_cntbuf_to_xtlv_format(NULL, cntbuf, ACSD_WL_CNTBUF_SIZE, 0);
	if (ret < 0) {
		ACSD_ERROR("wl_cntbuf_to_xtlv_format failed (%d)\n", ret);
		return full_scan;
	}

	if (!(wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntbuf))) {
		ACSD_ERROR("wlc_cnt NULL\n");
		return full_scan;
	}

	ACSD_INFO("acs_tx_idle: txframe %d(%d)\n", wlc_cnt->txframe, c_info->acs_txframe);

	if (wlc_cnt->txframe > c_info->acs_txframe)
		acs_txframe = wlc_cnt->txframe - c_info->acs_txframe;
	else
		acs_txframe = wlc_cnt->txframe + ((uint32)0xFFFFFFFF - c_info->acs_txframe);

	if (acs_txframe < (c_info->acs_tx_idle_cnt * (now - c_info->timestamp_tx_idle))) {
		ACSD_INFO("acs_tx_idle fullscan: %d\n", c_info->acs_txframe);
		full_scan = 1;
	}

	c_info->acs_txframe = wlc_cnt->txframe;
	c_info->timestamp_tx_idle = now;
	return full_scan;
}

void
acs_cleanup(acs_info_t ** acs_info_p)
{
	int i;

	if (!*acs_info_p)
		return;

	for (i = 0; i < ACS_MAX_IF_NUM; i++) {
		acs_chaninfo_t* c_info = (*acs_info_p)->chan_info[i];

		ACS_FREE(c_info->scan_results);

		if (c_info->acs_escan->acs_use_escan)
			acs_escan_free(c_info->acs_escan->escan_bss_head);

		acs_cleanup_scan_entry(c_info);
		ACS_FREE(c_info->ch_bssinfo);
		ACS_FREE(c_info->chanim_stats);
		ACS_FREE(c_info->scan_chspec_list.chspec_list);
		ACS_FREE(c_info->candidate[ACS_BW_20]);
		ACS_FREE(c_info->candidate[ACS_BW_40]);
		ACS_FREE(c_info->chanim_info);
		ACS_FREE(c_info->acs_bgdfs);

		acs_dfsr_exit(ACS_DFSR_CTX(c_info));

		ACS_FREE(c_info);
	}
	ACS_FREE(acs_info);
	*acs_info_p = NULL;
}

/* set intfer trigger params */
int acs_intfer_config_txfail(acs_chaninfo_t *c_info)
{
	wl_intfer_params_t params;
	acs_intfer_params_t *intfer = &(c_info->intfparams);
	int err = 0;
	uint8 thld_setting = ACSD_INTFER_PARAMS_80_THLD;

	ACSD_INFO("%s@%d\n", __FUNCTION__, __LINE__);

	/*
	 * When running 80MBW high chan, and far STA exists
	 * we will use the high threshold for txfail trigger
	 */
	if (!acsd_is_lp_chan(c_info, c_info->cur_chspec) &&
			(c_info->sta_status & ACS_STA_EXIST_FAR)) {
		if (CHSPEC_IS80(c_info->cur_chspec)) {
			thld_setting = ACSD_INTFER_PARAMS_80_THLD_HI;
		} else if (CHSPEC_BW_GE(c_info->cur_chspec,
					WL_CHANSPEC_BW_160)) {
			thld_setting = ACSD_INTFER_PARAMS_160_THLD_HI;
		}
	} else if (CHSPEC_BW_GE(c_info->cur_chspec, WL_CHANSPEC_BW_160)) {
		thld_setting = ACSD_INTFER_PARAMS_160_THLD;
	}

	if (thld_setting == intfer->thld_setting) {
		ACSD_5G("Same Setting intfer[%d].\n", thld_setting);
		return err;
	}
	intfer->thld_setting = thld_setting;

	params.version = INTFER_VERSION;
	params.period = intfer->period;
	params.cnt = intfer->cnt;
	params.txfail_thresh =
		intfer->acs_txfail_thresholds[intfer->thld_setting].txfail_thresh;
	params.tcptxfail_thresh =
		intfer->acs_txfail_thresholds[intfer->thld_setting].tcptxfail_thresh;

	err = acs_set_intfer_params(c_info->name, &params, sizeof(wl_intfer_params_t));

	if (err < 0) {
		ACSD_ERROR("intfer_params error! ret code: %d\n", err);
	}

	ACSD_5G("Setting intfer[%d]: cnt:%d period:%d tcptxfail:%d txfail:%d\n",
			thld_setting, params.period, params.cnt,
			params.tcptxfail_thresh, params.txfail_thresh);

	return err;
}

/* set intfer trigger params based on type,AC and numsecs*/
int acs_intfer_config_trfthold(acs_chaninfo_t *c_info, char *prefix)
{
	wl_trf_thold_t trfdata;
	acs_intfer_trf_thold_t *intfer = &(c_info->trf_params);
	int err = 0;
	char *str_ac = NULL;
	char tmp[32];

	ACSD_INFO("%s@%d\n", __FUNCTION__, __LINE__);
	memset(&trfdata, 0, sizeof(wl_trf_thold_t));
	/* Setting txfail configuration
	*/
	str_ac = nvram_safe_get(strcat_r(prefix, "acs_access_category_en", tmp));

	trfdata.ap = ACS_AP_CFG;
	trfdata.thresh = intfer->thresh;
	trfdata.num_secs = intfer->num_secs;

	if (strstr(str_ac, "0")) {
		ACSD_5G("setting txfail config for BE\n");
		trfdata.type = ACS_AC_BE;
		err = acs_set_intfer_trf_thold(c_info->name, &trfdata,
				sizeof(wl_trf_thold_t));

		ACS_ERR(err, "intfer_params error! ret code\n");

		ACSD_5G("Setting intfer traffic info: type:%d thresh:%d num_secs:%d\n",
				trfdata.type, trfdata.thresh, trfdata.num_secs);
	}
	if (strstr(str_ac, "1")) {
		ACSD_5G("setting txfail config for BK\n");
		trfdata.type = ACS_AC_BK;
		err = acs_set_intfer_trf_thold(c_info->name, &trfdata,
				sizeof(wl_trf_thold_t));

		ACS_ERR(err, "intfer_params error! ret code\n");

		ACSD_5G("Setting intfer traffic info: type:%d thresh:%d num_secs:%d\n",
				trfdata.type, trfdata.thresh, trfdata.num_secs);
	}
	if (strstr(str_ac, "2")) {
		ACSD_5G("setting txfail config for VI\n");
		trfdata.type = ACS_AC_VI;
		err = acs_set_intfer_trf_thold(c_info->name, &trfdata,
				sizeof(wl_trf_thold_t));

		ACS_ERR(err, "intfer_params error! ret code\n");

		ACSD_5G("Setting intfer traffic info: type:%d thresh:%d num_secs:%d\n",
				trfdata.type, trfdata.thresh, trfdata.num_secs);
	}
	if (strstr(str_ac, "3")) {
		ACSD_5G("setting txfail config for VO\n");
		trfdata.type = ACS_AC_VO;
		err = acs_set_intfer_trf_thold(c_info->name, &trfdata,
				sizeof(wl_trf_thold_t));

		ACS_ERR(err, "intfer_params error! ret code\n");

		ACSD_5G("Setting intfer traffic info: type:%d thresh:%d num_secs:%d\n",
				trfdata.type, trfdata.thresh, trfdata.num_secs);
	}
	ACSD_5G("setting txfail config for TO\n");
	trfdata.type = ACS_AC_TO;
	err = acs_set_intfer_trf_thold(c_info->name, &trfdata,
			sizeof(wl_trf_thold_t));

	ACS_ERR(err, "intfer_params error! ret code\n");

	ACSD_INFO("Setting intfer traffic info: type:%d thresh:%d num_secs:%d\n",
			trfdata.type, trfdata.thresh, trfdata.num_secs);
	return err;
}

int acs_update_assoc_info(acs_chaninfo_t *c_info)
{
	struct maclist *list;
	acs_assoclist_t *acs_assoclist;
	int ret = 0, cnt, size;

	/* reset assoc STA staus */
	c_info->sta_status = ACS_STA_NONE;

	ACSD_INFO("%s@%d\n", __FUNCTION__, __LINE__);

	/* read assoclist */
	list = (struct maclist *)acsd_malloc(ACSD_BUFSIZE_4K);
	memset(list, 0, ACSD_BUFSIZE_4K);
	ACSD_INFO("WLC_GET_ASSOCLIST\n");
	list->count = htod32((ACSD_BUFSIZE_4K - sizeof(int)) / ETHER_ADDR_LEN);
	ret = wl_ioctl(c_info->name, WLC_GET_ASSOCLIST, list, ACSD_BUFSIZE_4K);
	if (ret < 0) {
		ACSD_ERROR("WLC_GET_ASSOCLIST failure\n");
		ACS_FREE(list);
		return ret;
	}

	ACS_FREE(c_info->acs_assoclist);
	list->count = dtoh32(list->count);
	if (list->count <= 0) {
		ACS_FREE(list);
		return ret;
	}

	size = sizeof(acs_assoclist_t) + (list->count)* sizeof(acs_sta_info_t);
	acs_assoclist = (acs_assoclist_t *)acsd_malloc(size);

	c_info->acs_assoclist = acs_assoclist;
	acs_assoclist->count = list->count;

	for (cnt = 0; cnt < list->count; cnt++) {
		scb_val_t scb_val;

		memset(&scb_val, 0, sizeof(scb_val));
		memcpy(&scb_val.ea, &list->ea[cnt], ETHER_ADDR_LEN);

		ret = wl_ioctl(c_info->name, WLC_GET_RSSI, &scb_val, sizeof(scb_val));

		if (ret < 0) {
			ACSD_ERROR("Err: reading intf:%s STA:"MACF" RSSI\n",
					c_info->name, ETHER_TO_MACF(list->ea[cnt]));
			ACS_FREE(c_info->acs_assoclist);
			break;
		}

		acs_assoclist->sta_info[cnt].rssi = dtoh32(scb_val.val);
		ether_copy(&(list->ea[cnt]), &(acs_assoclist->sta_info[cnt].ea));
		ACSD_INFO("%s@%d sta_info sta:"MACF" rssi:%d [%d]\n",
				__FUNCTION__, __LINE__,
				ETHER_TO_MACF(list->ea[cnt]), dtoh32(scb_val.val),
				c_info->acs_far_sta_rssi);

		if (acs_assoclist->sta_info[cnt].rssi < c_info->acs_far_sta_rssi)
			c_info->sta_status |= ACS_STA_EXIST_FAR;
		else
			c_info->sta_status |= ACS_STA_EXIST_CLOSE;

		ACSD_INFO("%s@%d sta_status:0x%x\n", __FUNCTION__, __LINE__, c_info->sta_status);
	}
	ACS_FREE(list);

	if (!ret) {
		/* check to see if we need to update intfer params */
		if (!c_info->trf_thold && (c_info->sta_status & ACS_STA_EXIST_FAR)) {
			acs_intfer_config_txfail(c_info);
		}
	}

	return ret;
}

/*
 * Check to see if we need goto hi power chan
 * (1) if exit from DFS chan, we goto hi power chan
 */
int
acsd_hi_chan_check(acs_chaninfo_t *c_info)
{
	bool is_dfs = c_info->cur_is_dfs;
	int bw = CHSPEC_BW(c_info->cur_chspec);

	if (bw <= WL_CHANSPEC_BW_40 || !is_dfs) {
		ACSD_5G("Not running in 80MBW , Chanspec:0x%4x (%s)\n",
				c_info->cur_chspec, wf_chspec_ntoa(c_info->cur_chspec, chanspecbuf));
		return FALSE;
	}

	ACSD_5G("running in 80Mbw DFS Chanspec:0x%4x (%s)\n",
			c_info->cur_chspec, wf_chspec_ntoa(c_info->cur_chspec, chanspecbuf));
	return TRUE;
}

/*
 *  check if need to switch chan:
 * (1) if run in hi-chan, all STA are far, DFS-reentry is disabled,
 *  chan switch is needed
 */
bool
acsd_need_chan_switch(acs_chaninfo_t *c_info)
{
	acs_assoclist_t *acs_assoclist = c_info->acs_assoclist;
	bool dfsr_disable = (c_info->acs_dfs != ACS_DFS_REENTRY);
	bool is_dfs = c_info->cur_is_dfs;
	int bw = CHSPEC_BW(c_info->cur_chspec);

	ACSD_5G("sta_status:0x%x chanspec:0x%4x (%s) acs_dfs:%d acs_assoclist:%p is_dfs:%d\n",
			c_info->sta_status, c_info->cur_chspec, wf_chspec_ntoa(c_info->cur_chspec, chanspecbuf),
			c_info->acs_dfs, c_info->acs_assoclist, is_dfs);

	if ((bw > WL_CHANSPEC_BW_40) &&
			!acsd_is_lp_chan(c_info, c_info->cur_chspec) &&
			acs_assoclist &&
			(c_info->sta_status & ACS_STA_EXIST_FAR) &&
			dfsr_disable &&
			!is_dfs) {
		ACSD_5G("No chan switch is needed.\n");
		return FALSE;
	}
	return TRUE;
}

/* get traffic information of the interface */
static int
acs_get_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info)
{
	char cntbuf[ACSD_WL_CNTBUF_SIZE];
	wl_cnt_info_t *cntinfo;
	const wl_cnt_wlc_t *wlc_cnt;
	int ret = BCME_OK;

	if (acs_get_dfsr_counters(c_info->name, cntbuf) < 0) {
		ACSD_DFSR("Failed to fetch interface counters for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	cntinfo = (wl_cnt_info_t *)cntbuf;
	cntinfo->version = dtoh16(cntinfo->version);
	cntinfo->datalen = dtoh16(cntinfo->datalen);
	/* Translate traditional (ver <= 10) counters struct to new xtlv type struct */
	if (wl_cntbuf_to_xtlv_format(NULL, cntbuf, ACSD_WL_CNTBUF_SIZE, 0)
		!= BCME_OK) {
		ACSD_DFSR("wl_cntbuf_to_xtlv_format failed for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	if ((wlc_cnt = GET_WLCCNT_FROM_CNTBUF(cntbuf)) == NULL) {
		ACSD_DFSR("GET_WLCCNT_FROM_CNTBUF NULL for '%s'\n", c_info->name);
		ret = BCME_ERROR;
		goto exit;
	}

	t_info->timestamp = uptime();
	t_info->txbyte = wlc_cnt->txbyte;
	t_info->rxbyte = wlc_cnt->rxbyte;
	t_info->txframe = wlc_cnt->txframe;
	t_info->rxframe = wlc_cnt->rxframe;
exit:
	return ret;
}

/* get traffic information about TOAd video STAs (if any) */
static int
acs_get_video_sta_traffic_info(acs_chaninfo_t * c_info, acs_traffic_info_t *t_info)
{
	char stabuf[ACS_MAX_STA_INFO_BUF];
	sta_info_v6_t *sta;
	int i, ret = BCME_OK;
	int index = c_info->video_sta_idx;

	struct ether_addr ea;
	acs_traffic_info_t total;

	memset(&total, 0, sizeof(acs_traffic_info_t));
	/* Consolidate the traffic info of all video stas */
	for (i = 0; i < index; i++) {
		memset(stabuf, 0, sizeof(stabuf));
		memcpy(&ea, &c_info->vid_sta[i].ea, sizeof(ea));
		if (acs_get_stainfo(c_info->name, &ea, sizeof(ea), stabuf,
				ACS_MAX_STA_INFO_BUF) < 0) {
			ACSD_ERROR("sta_info for %s failed\n", c_info->vid_sta[i].vid_sta_mac);
			return BCME_ERROR;
		}
		sta = (sta_info_v6_t *)stabuf;
		total.txbyte = total.txbyte + dtoh64(sta->tx_tot_bytes);
		total.rxbyte = total.rxbyte + dtoh64(sta->rx_tot_bytes);
		total.txframe = total.txframe + dtoh32(sta->tx_tot_pkts);
		total.rxframe = total.rxframe + dtoh32(sta->rx_tot_pkts);
	}
	t_info->timestamp = uptime();
	t_info->txbyte = total.txbyte;
	t_info->rxbyte = total.rxbyte;
	t_info->txframe = total.txframe;
	t_info->rxframe = total.rxframe;
	return ret;
}

/*
 * acs_get_initial_traffic_stats - retrieve and store traffic activity info when acsd starts
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
acs_get_initial_traffic_stats(acs_chaninfo_t *c_info)
{
	acs_activity_info_t *acs_act = &c_info->acs_activity;
	acs_traffic_info_t *t_prev = &acs_act->prev_bss_traffic;
	acs_traffic_info_t t_curr;
	int ret;

	if (!c_info->acs_toa_enable) {
		if ((ret = acs_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("Failed to get traffic information\n");
			return ret;
		}
	} else {
		if ((ret = acs_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("Failed to get video sta traffic information\n");
			return ret;
		}
	}

	t_prev->txframe = t_curr.txframe;
	t_prev->rxframe = t_curr.rxframe;

	return BCME_OK;
}

/*
 * acs_activity_update - updates traffic activity information
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise
 */
int
acs_activity_update(acs_chaninfo_t * c_info)
{
	acs_activity_info_t *acs_act = &c_info->acs_activity;
	time_t now = uptime();
	acs_traffic_info_t t_curr;
	acs_traffic_info_t *t_prev = &acs_act->prev_bss_traffic;
	acs_traffic_info_t *t_accu_diff = &acs_act->accu_diff_bss_traffic;
	acs_traffic_info_t *t_prev_diff = &acs_act->prev_diff_bss_traffic;
	uint32 total_frames; /* total tx and rx frames on link */
	int ret;

	if (!c_info->acs_toa_enable) {
		if ((ret = acs_get_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("Failed to get traffic information\n");
			return ret;
		}
	} else {
		if ((ret = acs_get_video_sta_traffic_info(c_info, &t_curr)) != BCME_OK) {
			ACSD_ERROR("Failed to get video sta traffic information\n");
			return ret;
		}
	}

	/* update delta between current and previous fetched */
	t_prev_diff->timestamp = now - t_prev->timestamp;
	t_prev_diff->txbyte = DELTA_FRAMES((t_prev->txbyte), (t_curr.txbyte));
	t_prev_diff->rxbyte = DELTA_FRAMES((t_prev->rxbyte), (t_curr.rxbyte));
	t_prev_diff->txframe = DELTA_FRAMES((t_prev->txframe), (t_curr.txframe));
	t_prev_diff->rxframe = DELTA_FRAMES((t_prev->rxframe), (t_curr.rxframe));

	/* add delta (calculated above) to accumulated deltas */
	t_accu_diff->timestamp += t_prev_diff->timestamp;
	t_accu_diff->txbyte += t_prev_diff->txbyte;
	t_accu_diff->rxbyte += t_prev_diff->rxbyte;
	t_accu_diff->txframe += t_prev_diff->txframe;
	t_accu_diff->rxframe += t_prev_diff->rxframe;

	acs_act->num_accumulated++;

	total_frames =  t_prev_diff->txframe + t_prev_diff->rxframe;

	acs_bgdfs_sw_add(ACS_DFSR_CTX(c_info), now, total_frames);

	/* save current in t_prev (previous) to help with next time delta calculation */
	memcpy(t_prev, &t_curr, sizeof(*t_prev));

	return BCME_OK;
}

/* acs_get_recent_timestamp - gets timestamp of the recent most acs record
 * (or returns zero when record isn't found)
 * c_info - pointer to acs_chaninfo_t for an interface
 * chspec - channel spec to find in acs record
 *
 * Returns timestamp if acs record is found (or zero)
 */
uint64
acs_get_recent_timestamp(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	uint64 timestamp = 0;
	int i;
	chanim_info_t * ch_info = c_info->chanim_info;

	for (i = CHANIM_ACS_RECORD - 1; i >= 0; i--) {
		if (chspec == ch_info->record[i].selected_chspc) {
			if (ch_info->record[i].timestamp > timestamp) {
				timestamp = (uint64) ch_info->record[i].timestamp;
			}
		}
	}

	return timestamp;
}

void
acs_process_cmd(acs_chaninfo_t * c_info, chanspec_t chspec, int dfs_ap_move)
{
	int ret = 0;
	wl_chan_change_reason_t reason;

	reason = (wl_chan_change_reason_t)dfs_ap_move;

	c_info->selected_chspec = chspec;
	c_info->cur_chspec = chspec;
	acs_set_chspec(c_info, FALSE, dfs_ap_move);

	/* No need to update the driver for reason DFS_AP_MOVE stop and
	 * stunt operation as it can stop running dfs cac state machine
	 * for stunt operation.
	 */
	if ((reason != WL_CHAN_REASON_DFS_AP_MOVE_RADAR_FOUND) &&
		(reason != WL_CHAN_REASON_DFS_AP_MOVE_STUNT) &&
		(reason != WL_CHAN_REASON_DFS_AP_MOVE_ABORTED)) {

		ret = acs_update_driver(c_info);

		if (ret)
			ACSD_ERROR("update driver failed\n");
	}
	ACSD_DEBUG("ifname %s - mode: %s\n", c_info->name,
		AUTOCHANNEL(c_info)? "SELECT" :
		COEXCHECK(c_info)? "COEXCHECK" :
		ACS11H(c_info)? "11H" : "MONITOR");

	chanim_upd_acs_record(c_info->chanim_info,
		c_info->selected_chspec, APCS_IOCTL);
}

/*
 * acs_upgrade_to160 - upgrade to 160Mhz BW
 *
 * c_info - pointer to acs_chaninfo_t for an interface
 *
 * Returns BCME_OK when successful; error status otherwise.
 */
int
acs_upgrade_to160(acs_chaninfo_t * c_info)
{
	int ret;
	if ((ret = acs_bgdfs_choose_channel(c_info, TRUE, TRUE)) != BCME_OK) {
		ACSD_ERROR("%s Picking a 160Mhz channel failed\n", c_info->name);
		return BCME_ERROR;
	}
	return BCME_OK;
}
