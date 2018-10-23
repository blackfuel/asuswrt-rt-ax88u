/*
 *      acs_cfg.c
 *
 *      This module will retrieve acs configurations from nvram, if params are
 *      not set it will retrieve default values.
 *
 *      Copyright 2018 Broadcom
 *
 *      This program is the proprietary software of Broadcom and/or
 *      its licensors, and may only be used, duplicated, modified or distributed
 *      pursuant to the terms and conditions of a separate, written license
 *      agreement executed between you and Broadcom (an "Authorized License").
 *      Except as set forth in an Authorized License, Broadcom grants no license
 *      (express or implied), right to use, or waiver of any kind with respect to
 *      the Software, and Broadcom expressly reserves all rights in and to the
 *      Software and all intellectual property rights therein.  IF YOU HAVE NO
 *      AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *      WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *      THE SOFTWARE.
 *
 *      Except as expressly set forth in the Authorized License,
 *
 *      1. This program, including its structure, sequence and organization,
 *      constitutes the valuable trade secrets of Broadcom, and you shall use
 *      all reasonable efforts to protect the confidentiality thereof, and to
 *      use this information only in connection with your use of Broadcom
 *      integrated circuit products.
 *
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *      "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *      REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *      OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *      DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *      NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *      ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *      CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *      OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *      BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *      SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *      IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *      IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *      ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *      OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *      NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: acs_cfg.c 764927 2018-06-11 06:02:47Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"
static const char *station_key_fmtstr = "toa-sta-%d";	/* format string, passed to snprintf() */

#define MAX_KEY_LEN 16				/* the expanded key format string must fit within */
#define ACS_VIDEO_STA_TYPE "video"		/* video sta type */

#define ACS_DFLT_FLAGS ACS_FLAGS_LASTUSED_CHK

extern int bcm_ether_atoe(const char *p, struct ether_addr *ea);

#ifdef DEBUG
extern void acs_dump_policy(acs_policy_t *a_pol);
extern void acs_dump_config_extra(acs_chaninfo_t *c_info);
#endif

/*
 * Function to set a channel table by parsing a list consisting
 * of a comma-separated channel numbers.
 */
int
acs_set_chan_table(char *channel_list, chanspec_t *chspec_list,
                      unsigned int vector_size)
{
	int chan_index;
	int channel;
	int chan_count = 0;
	char *chan_str;
	char *delim = ",";
	char chan_buf[ACS_MAX_VECTOR_LEN + 2];
	int list_length;

	/*
	* NULL list means no channels are set. Return without
	* modifying the vector.
	*/
	if (channel_list == NULL)
		return 0;

	/*
	* A non-null list means that we must set the vector.
	*  Clear it first.
	* Then parse a list of <chan>,<chan>,...<chan>
	*/
	memset(chan_buf, 0, sizeof(chan_buf));
	list_length = strlen(channel_list);
	list_length = MIN(list_length, ACS_MAX_VECTOR_LEN);
	strncpy(chan_buf, channel_list, list_length);
	strncat(chan_buf, ",", list_length);

	chan_str = strtok(chan_buf, delim);

	for (chan_index = 0; chan_index < vector_size; chan_index++)
	{
		if (chan_str == NULL)
			break;
		channel = strtoul(chan_str, NULL, 16);
		if (channel == 0)
			break;
		chspec_list[chan_count++] = channel;
		chan_str = strtok(NULL, delim);
	}
	return chan_count;
}

static int
acs_toa_load_station(acs_chaninfo_t *c_info, const char *keyfmt, int stain)
{
	char keybuf[MAX_KEY_LEN];
	char *tokens, *sta_type;
	int index = c_info->video_sta_idx;
	char ea[ACS_STA_EA_LEN];

	if (acs_snprintf(keybuf, sizeof(keybuf), keyfmt, stain) >= sizeof(keybuf)) {
		ACSD_ERROR("key buffer too small\n");
		return BCME_ERROR;
	}

	tokens = nvram_get(keybuf);
	if (!tokens) {
		ACSD_INFO("No toa NVRAM params set\n");
		return BCME_ERROR;
	}

	strncpy(ea, tokens, ACS_STA_EA_LEN);
	ea[ACS_STA_EA_LEN -1] = '\0';
	sta_type = strstr(tokens, ACS_VIDEO_STA_TYPE);
	if (sta_type) {
		c_info->acs_toa_enable = TRUE;
		if (index >= ACS_MAX_VIDEO_STAS) {
			ACSD_ERROR("MAX VIDEO STAs exceeded\n");
			return BCME_ERROR;
		}

		strncpy(c_info->vid_sta[index].vid_sta_mac, ea,
				ACS_STA_EA_LEN);
		c_info->vid_sta[index].vid_sta_mac[ACS_STA_EA_LEN -1] = '\0';
		if (!bcm_ether_atoe(c_info->vid_sta[index].vid_sta_mac,
			&c_info->vid_sta[index].ea)) {
			ACSD_ERROR("toa video sta ether addr NOT proper\n");
			return BCME_ERROR;
		}
		c_info->video_sta_idx++;
		ACSD_INFO("VIDEOSTA %s\n", c_info->vid_sta[index].vid_sta_mac);
	}

	return BCME_OK;
}

static void
acs_bgdfs_acs_toa_retrieve_config(acs_chaninfo_t *c_info, char * prefix)
{
	/* retrieve toa related configuration from nvram */
	int stain, ret = BCME_OK;

	c_info->acs_toa_enable = FALSE;
	c_info->video_sta_idx = 0;
	ACSD_INFO("retrieve TOA config from nvram ...\n");

	/* Load station specific settings */
	for (stain = 1; stain <= ACS_MAX_VIDEO_STAS; ++stain) {
		ret = acs_toa_load_station(c_info, station_key_fmtstr, stain);
		if (ret != BCME_OK)
			return;
	}
}

static void
acs_retrieve_config_bgdfs(acs_bgdfs_info_t *acs_bgdfs, char * prefix)
{
	char conf_word[128], tmp[100];

	if (acs_bgdfs == NULL) {
		ACSD_ERROR("acs_bgdfs is NULL");
		return;
	}

	/* acs_bgdfs_ahead */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_ahead", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_ahead is set. Retrieve default.\n");
		acs_bgdfs->ahead = ACS_BGDFS_AHEAD;
	} else {
		char *endptr = NULL;
		acs_bgdfs->ahead = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_ahead: 0x%x\n", acs_bgdfs->ahead);
	}

	/* acs_bgdfs_idle_interval */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_idle_interval", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_idle_interval is set. Retrieve default.\n");
		acs_bgdfs->idle_interval = ACS_BGDFS_IDLE_INTERVAL;
	} else {
		char *endptr = NULL;
		acs_bgdfs->idle_interval = strtoul(conf_word, &endptr, 0);
		if (acs_bgdfs->idle_interval < ACS_TRAFFIC_INFO_UPDATE_INTERVAL(acs_bgdfs)) {
			acs_bgdfs->idle_interval = ACS_TRAFFIC_INFO_UPDATE_INTERVAL(acs_bgdfs);
		}
		ACSD_INFO("acs_bgdfs_idle_interval: 0x%x\n", acs_bgdfs->idle_interval);
	}

	/* acs_bgdfs_idle_frames_thld */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_idle_frames_thld", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_idle_frames_thld is set. Retrieve default.\n");
		acs_bgdfs->idle_frames_thld = ACS_BGDFS_IDLE_FRAMES_THLD;
	} else {
		char *endptr = NULL;
		acs_bgdfs->idle_frames_thld = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_idle_frames_thld: 0x%x\n", acs_bgdfs->idle_frames_thld);
	}

	/* acs_bgdfs_avoid_on_far_sta */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_avoid_on_far_sta", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_avoid_on_far_sta is set. Retrieve default.\n");
		acs_bgdfs->bgdfs_avoid_on_far_sta = ACS_BGDFS_AVOID_ON_FAR_STA;
	} else {
		char *endptr = NULL;
		acs_bgdfs->bgdfs_avoid_on_far_sta = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_avoid_on_far_sta: 0x%x\n", acs_bgdfs->bgdfs_avoid_on_far_sta);
	}

	/* acs_bgdfs_fallback_blocking_cac */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_fallback_blocking_cac", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_fallback_blocking_cac set. Get default.\n");
		acs_bgdfs->fallback_blocking_cac = ACS_BGDFS_FALLBACK_BLOCKING_CAC;
	} else {
		char *endptr = NULL;
		acs_bgdfs->fallback_blocking_cac = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_fallback_blocking_cac: 0x%x\n",
				acs_bgdfs->fallback_blocking_cac);
	}

	/* acs_bgdfs_txblank_threshold */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_txblank_threshold", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_txblank_threshold set. Get default.\n");
		acs_bgdfs->txblank_th = ACS_BGDFS_TX_LOADING;
	} else {
		char *endptr = NULL;
		acs_bgdfs->txblank_th = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_txblank_threshold: 0x%x\n", acs_bgdfs->txblank_th);
	}
}

void
acs_retrieve_config(acs_chaninfo_t *c_info, char *prefix)
{
	/* retrieve policy related configuration from nvram */
	char conf_word[128], conf_var[16], tmp[200];
	char *next, *str;
	int i = 0, val;
	acs_policy_index index;
	acs_policy_t *a_pol = &c_info->acs_policy;
	uint32 flags;
	uint8 chan_count, band;
	int acs_bgdfs_enab = 0;
	bool bgdfs_cap = FALSE;

	/* the current layout of config */
	ACSD_INFO("retrieve config from nvram ...\n");

	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_scan_entry_expire", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_scan_entry_expire set. Retrieve default.\n");
		c_info->acs_scan_entry_expire = ACS_CI_SCAN_EXPIRE;
	}
	else {
		char *endptr = NULL;
		c_info->acs_scan_entry_expire = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_scan_entry_expire: 0x%x\n", c_info->acs_scan_entry_expire);
	}

	/* acs_bgdfs_enab */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_bgdfs_enab", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_bgdfs_enab is set. Retrieve default.\n");
		acs_bgdfs_enab = ACS_BGDFS_ENAB;
	}
	else {
		char *endptr = NULL;
		acs_bgdfs_enab = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_bgdfs_enab: 0x%x\n", acs_bgdfs_enab);
	}

	bgdfs_cap = acs_bgdfs_capable(c_info);

	if (acs_bgdfs_enab && bgdfs_cap) {
		/* allocate core data structure for bgdfs */
		c_info->acs_bgdfs =
			(acs_bgdfs_info_t *)acsd_malloc(sizeof(*(c_info->acs_bgdfs)));

		acs_retrieve_config_bgdfs(c_info->acs_bgdfs, prefix);
	}

	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_boot_only", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_boot_only is set. Retrieve default. \n");
		c_info->acs_boot_only = ACS_BOOT_ONLY_DEFAULT;
	} else {
		char *endptr = NULL;
		c_info->acs_boot_only = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_boot_only: 0x%x\n", c_info->acs_boot_only);
	}

	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_ignore_txfail", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_DEBUG("Retrieve default. \n");
		c_info->ignore_txfail = ACS_IGNORE_TXFAIL;
	} else {
		char *endptr = NULL;
		c_info->ignore_txfail = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_ignore_txfail: 0x%x\n", c_info->ignore_txfail);
	}

	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_flags", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs flag set. Retrieve default.\n");
		flags = ACS_DFLT_FLAGS;
	}
	else {
		char *endptr = NULL;
		flags = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs flags: 0x%x\n", flags);
	}

	acs_safe_get_conf(conf_word, sizeof(conf_word),
		strcat_r(prefix, "acs_pol", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs policy set. Retrieve default.\n");

		acs_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "acs_pol_idx", tmp));

		wl_ioctl(c_info->name, WLC_GET_BAND, &band, sizeof(band));

		band = dtoh32(band);

		if (!strcmp(conf_word, "")) {
			if (BAND_5G(band)) {
				index = ACS_POLICY_DEFAULT5G;
				ACSD_PRINT("Selecting 5g band ACS policy\n");
			} else {
				index = ACS_POLICY_DEFAULT2G;
				ACSD_PRINT("Selecting 2g band ACS policy\n");
			}
		} else {
			index = atoi(conf_word);
		}
		acs_default_policy(a_pol, index);

	} else {

		index = ACS_POLICY_USER;
		memset(a_pol, 0, sizeof(*a_pol));	/* Initialise policy values to all zeroes */
		foreach(conf_var, conf_word, next) {
			val = atoi(conf_var);
			ACSD_DEBUG("i: %d conf_var: %s val: %d\n", i, conf_var, val);

			if (i == 0)
				a_pol->bgnoise_thres = val;
			else if (i == 1)
				a_pol->intf_threshold = val;
			else {
				if ((i - 2) >= CH_SCORE_MAX) {
					ACSD_ERROR("Ignoring excess values in %sacs_pol=\"%s\"\n",
						prefix, conf_word);
					break; /* Prevent overwriting innocent memory */
				}
				a_pol->acs_weight[i - 2] = val;
				ACSD_DEBUG("weight No. %d, value: %d\n", i-2, val);
			}
			i++;
		}
		a_pol->chan_selector = acs_pick_chanspec;
	}

	if ((str = nvram_get(strcat_r(prefix, "acs_txdelay_period", tmp))) == NULL)
		c_info->acs_txdelay_period = ACS_TXDELAY_PERIOD;
	else
		c_info->acs_txdelay_period = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_txdelay_cnt", tmp))) == NULL)
		c_info->acs_txdelay_cnt = ACS_TXDELAY_CNT;
	else
		c_info->acs_txdelay_cnt = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_txdelay_ratio", tmp))) == NULL)
		c_info->acs_txdelay_ratio = ACS_TXDELAY_RATIO;
	else
		c_info->acs_txdelay_ratio = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_far_sta_rssi", tmp))) == NULL)
		c_info->acs_far_sta_rssi = ACS_FAR_STA_RSSI;
	else
		c_info->acs_far_sta_rssi = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_nofcs_least_rssi", tmp))) == NULL)
		c_info->acs_nofcs_least_rssi = ACS_NOFCS_LEAST_RSSI;
	else
		c_info->acs_nofcs_least_rssi = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_scan_chanim_stats", tmp))) == NULL)
		c_info->acs_scan_chanim_stats = ACS_SCAN_CHANIM_STATS;
	else
		c_info->acs_scan_chanim_stats = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_ci_scan_chanim_stats", tmp))) == NULL)
		c_info->acs_ci_scan_chanim_stats = ACS_CI_SCAN_CHANIM_STATS;
	else
		c_info->acs_ci_scan_chanim_stats = atoi(str);

	memset(&c_info->pref_chans, 0, sizeof(acs_conf_chspec_t));
	if ((str = nvram_get(strcat_r(prefix, "acs_pref_chans", tmp))) == NULL)	{
		c_info->pref_chans.count = 0;
	} else {
		chan_count = acs_set_chan_table(str, c_info->pref_chans.clist, ACS_MAX_LIST_LEN);
		c_info->pref_chans.count = chan_count;
	}
	memset(&c_info->excl_chans, 0, sizeof(acs_conf_chspec_t));
	if ((str = nvram_get(strcat_r(prefix, "acs_excl_chans", tmp))) == NULL)	{
		c_info->excl_chans.count = 0;
	} else {
		chan_count = acs_set_chan_table(str, c_info->excl_chans.clist, ACS_MAX_LIST_LEN);
		c_info->excl_chans.count = chan_count;
	}

	if ((str = nvram_get(strcat_r(prefix, "acs_dfs", tmp))) == NULL)
		c_info->acs_dfs = ACS_DFS_ENABLED;
	else
		c_info->acs_dfs = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_chan_dwell_time", tmp))) == NULL)
		c_info->acs_chan_dwell_time = ACS_CHAN_DWELL_TIME;
	else
		c_info->acs_chan_dwell_time = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_chan_flop_period", tmp))) == NULL)
		c_info->acs_chan_flop_period = ACS_CHAN_FLOP_PERIOD;
	else
		c_info->acs_chan_flop_period = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_tx_idle_cnt", tmp))) == NULL)
		c_info->acs_tx_idle_cnt = ACS_TX_IDLE_CNT;
	else
		c_info->acs_tx_idle_cnt = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_ci_scan_timeout", tmp))) == NULL)
		c_info->acs_ci_scan_timeout = ACS_CI_SCAN_TIMEOUT;
	else
		c_info->acs_ci_scan_timeout = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_cs_scan_timer", tmp))) == NULL)
		c_info->acs_cs_scan_timer = ACS_DFLT_CS_SCAN_TIMER;
	else
		c_info->acs_cs_scan_timer = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "acs_ci_scan_timer", tmp))) == NULL)
		c_info->acs_ci_scan_timer = ACS_DFLT_CI_SCAN_TIMER;
	else
		c_info->acs_ci_scan_timer = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "intfer_period", tmp))) == NULL)
		c_info->intfparams.period = ACS_INTFER_SAMPLE_PERIOD;
	else
		c_info->intfparams.period = atoi(str);

	if ((str = nvram_get(strcat_r(prefix, "intfer_cnt", tmp))) == NULL)
		c_info->intfparams.cnt = ACS_INTFER_SAMPLE_COUNT;
	else
		c_info->intfparams.cnt = atoi(str);

	c_info->intfparams.thld_setting = ACSD_INTFER_THLD_SETTING;

	if ((str = nvram_get(strcat_r(prefix, "intfer_txfail", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD].txfail_thresh =
			ACS_INTFER_TXFAIL_THRESH;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD].txfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_tcptxfail", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD].tcptxfail_thresh =
			ACS_INTFER_TCPTXFAIL_THRESH;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD].tcptxfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_txfail_hi", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD_HI].txfail_thresh =
			ACS_INTFER_TXFAIL_THRESH_HI;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD_HI].txfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_tcptxfail_hi", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD_HI].tcptxfail_thresh =
			ACS_INTFER_TCPTXFAIL_THRESH_HI;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_80_THLD_HI].tcptxfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_txfail_160", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD].txfail_thresh =
			ACS_INTFER_TXFAIL_THRESH_160;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD].txfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_tcptxfail_160", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD].tcptxfail_thresh =
			ACS_INTFER_TCPTXFAIL_THRESH_160;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD].tcptxfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_txfail_160_hi", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD_HI].txfail_thresh =
			ACS_INTFER_TXFAIL_THRESH_160_HI;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD_HI].txfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "intfer_tcptxfail_160_hi", tmp))) == NULL) {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD_HI].tcptxfail_thresh =
			ACS_INTFER_TCPTXFAIL_THRESH_160_HI;
	} else {
		c_info->intfparams.acs_txfail_thresholds[ACSD_INTFER_PARAMS_160_THLD_HI].tcptxfail_thresh =
			strtoul(str, NULL, 0);
	}

	if ((str = nvram_get(strcat_r(prefix, "acs_inttrf_thresh", tmp))) == NULL) {
		c_info->trf_params.thresh = ACS_INTFER_TRF_THRESH;
	} else {
		c_info->trf_params.thresh = atoi(str);
	}

	if ((str = nvram_get(strcat_r(prefix, "acs_inttrf_numsecs", tmp))) == NULL) {
		c_info->trf_params.num_secs = ACS_INTFER_TRF_NUMSECS;
	} else {
		c_info->trf_params.num_secs = atoi(str);
	}

	if (nvram_match(strcat_r(prefix, "dcs_csa_unicast", tmp), "1"))
		c_info->acs_dcs_csa = CSA_UNICAST_ACTION_FRAME;
	else
		c_info->acs_dcs_csa = CSA_BROADCAST_ACTION_FRAME;

	if ((str = nvram_get(strcat_r(prefix, "txop_weight", tmp))) == NULL)
		c_info->txop_weight = ACS_DEFAULT_TXOP_WEIGHT;
	else
		c_info->txop_weight = strtol(str, NULL, 0);

	if ((str = nvram_get(strcat_r(prefix, "acs_dfs_reentry", tmp))) == NULL) {
		c_info->dfs_reentry = ACS_DFS_REENTRY_EN;
	} else {
		c_info->dfs_reentry = strtol(str, NULL, 0);
	}

#ifdef ACSD_SEGMENT_CHANIM
	if ((str = nvram_get(strcat_r(prefix, "acs_segment_chanim", tmp))) == NULL) {
		c_info->segment_chanim = ACSD_SEGMENT_CHANIM_DEFAULT;
	} else {
		c_info->segment_chanim = (bool) strtol(str, NULL, 0);
	}
	if ((str = nvram_get(strcat_r(prefix, "acs_chanim_num_segments", tmp))) == NULL) {
		c_info->num_seg = ACSD_NUM_SEG_DEFAULT;
	} else {
		int tmp = strtol(str, NULL, 0);
		if (tmp < ACSD_NUM_SEG_MIN || tmp > ACSD_NUM_SEG_MAX) {
			c_info->num_seg = ACSD_NUM_SEG_DEFAULT;
		} else {
			c_info->num_seg = tmp;
		}
	}
#endif /* ACSD_SEGMENT_CHANIM */

	if ((str = nvram_get(strcat_r(prefix, "mode", tmp))) == NULL) {
		c_info->wet_enabled = FALSE;
	} else {
		c_info->wet_enabled = strcmp(str, "wet") ? FALSE : TRUE;
	}

	acs_bgdfs_acs_toa_retrieve_config(c_info, prefix);

	/* Customer req
	 * bootup on nondfs channel if below nvram is set.
	 */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "acs_start_on_nondfs", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_start_on_nondfs set,Retrieve default.ifname:%s\n", c_info->name);
		c_info->acs_start_on_nondfs = ACS_START_ON_NONDFS;
	} else {
		char *endptr = NULL;
		c_info->acs_start_on_nondfs = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_start_on_nondfs: 0x%x ifname: %s\n", c_info->acs_start_on_nondfs,
			c_info->name);
	}

	/* Customer Knob #1
	 * Preference for DFS and Non-DFS channels
	 */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "acs_cs_dfs_pref", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_cs_dfs_pref set. Use val of acs_dfs instead\n");
		c_info->acs_cs_dfs_pref = c_info->acs_dfs;
	}
	else {
		char *endptr = NULL;
		c_info->acs_cs_dfs_pref = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_cs_dfs_pref: 0x%x\n", c_info->acs_cs_dfs_pref);
	}

	/* Customer Knob #2
	 * Preference for channel power
	 */
	acs_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "acs_cs_high_pwr_pref", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No acs_cs_high_pwr_pref set. Disabled by default.\n");
		c_info->acs_cs_high_pwr_pref = 0;
	}
	else {
		char *endptr = NULL;
		c_info->acs_cs_high_pwr_pref = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs_cs_high_pwr_pref: 0x%x\n", c_info->acs_cs_high_pwr_pref);
	}

	/* allocate core data structure for escan */
	c_info->acs_escan =
		(acs_escaninfo_t *)acsd_malloc(sizeof(*(c_info->acs_escan)));

	acs_safe_get_conf(conf_word, sizeof(conf_word),
			strcat_r(prefix, "acs_use_escan", tmp));

	if (!strcmp(conf_word, "")) {
		ACSD_INFO("No escan config set. use defaults\n");
		c_info->acs_escan->acs_use_escan = ACS_ESCAN_DEFAULT;
	}
	else {
		char *endptr = NULL;
		c_info->acs_escan->acs_use_escan = strtoul(conf_word, &endptr, 0);
		ACSD_DEBUG("acs escan enable: %d\n", c_info->acs_escan->acs_use_escan);
	}

	c_info->flags = flags;
	c_info->policy_index = index;
#ifdef DEBUG
	acs_dump_policy(a_pol);
	acs_dump_config_extra(c_info);
#endif // endif
}
