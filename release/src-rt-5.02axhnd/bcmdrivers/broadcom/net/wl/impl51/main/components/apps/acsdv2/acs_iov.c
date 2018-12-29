/*
 *      acs_iov.c
 *
 *      This module will try to set/get the data from/to driver using IOVAR.
 *
 *	Copyright 2018 Broadcom
 *
 *	This program is the proprietary software of Broadcom and/or
 *	its licensors, and may only be used, duplicated, modified or distributed
 *	pursuant to the terms and conditions of a separate, written license
 *	agreement executed between you and Broadcom (an "Authorized License").
 *	Except as set forth in an Authorized License, Broadcom grants no license
 *	(express or implied), right to use, or waiver of any kind with respect to
 *	the Software, and Broadcom expressly reserves all rights in and to the
 *	Software and all intellectual property rights therein.  IF YOU HAVE NO
 *	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 *	WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 *	THE SOFTWARE.
 *
 *	Except as expressly set forth in the Authorized License,
 *
 *	1. This program, including its structure, sequence and organization,
 *	constitutes the valuable trade secrets of Broadcom, and you shall use
 *	all reasonable efforts to protect the confidentiality thereof, and to
 *	use this information only in connection with your use of Broadcom
 *	integrated circuit products.
 *
 *	2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *	"AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 *	REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 *	OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 *	DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 *	NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 *	ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 *	CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 *	OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *	3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 *	BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 *	SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 *	IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *	IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 *	ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 *	OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 *	NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *	$Id: acs_iov.c 754142 2018-03-26 06:24:48Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <assert.h>
#include <typedefs.h>

#include "acsd_svr.h"

int acs_get_perband_chanspecs(acs_chaninfo_t *c_info, chanspec_t input, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "chanspecs", &input, sizeof(chanspec_t), buf, length);
}

int acs_get_per_chan_info(acs_chaninfo_t *c_info, chanspec_t sub_chspec, char *buf, int length)
{
	return wl_iovar_getbuf(c_info->name, "per_chan_info", &sub_chspec, sizeof(chanspec_t), buf,
		length);
}

int acs_set_scanresults_minrssi(acs_chaninfo_t *c_info, int minrssi)
{
	return wl_iovar_setint(c_info->name, "scanresults_minrssi", minrssi);
}

int acs_set_escan_params(acs_chaninfo_t *c_info, wl_escan_params_t *params, int params_size)
{
	return wl_iovar_set(c_info->name, "escan", params, params_size);
}

int acs_get_chanspec(acs_chaninfo_t *c_info, int *chanspec)
{
	return wl_iovar_getint(c_info->name, "chanspec", chanspec);
}

int acs_set_chanspec(acs_chaninfo_t *c_info, chanspec_t chspec)
{
	return wl_iovar_setint(c_info->name, "chanspec", htod32(chspec));
}

int acs_get_obss_coex_info(acs_chaninfo_t *c_info, int *coex)
{
	return wl_iovar_getint(c_info->name, "obss_coex", coex);
}

int acs_get_bwcap_info(acs_chaninfo_t *c_info, acs_param_info_t *param, int param_len, char *buf,
	int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "bw_cap", param, param_len, buf, buf_len);
}

int acs_get_cap_info(acs_chaninfo_t *c_info, uint32 *param, int param_len, char *cap_buf,
	int cap_len)
{
	return wl_iovar_getbuf(c_info->name, "cap", param, param_len, cap_buf, cap_len);
}

int acs_get_dfs_forced_chspec(acs_chaninfo_t *c_info, char smbuf[WLC_IOCTL_SMLEN])
{
	wl_dfs_forced_t inp;
	int ret = 0;
	acs_rsi_t *rsi = &c_info->rs_info;

	if (BAND_2G(rsi->band_type) || !rsi->reg_11h ||
		((BAND_5G(rsi->band_type)) && (c_info->acs_dfs == ACS_DFS_DISABLED))) {
		return -1;
	}

	inp.version = DFS_PREFCHANLIST_VER;
	ret = wl_iovar_getbuf(c_info->name, "dfs_channel_forced", &inp, sizeof(wl_dfs_forced_t),
		smbuf, WLC_IOCTL_SMLEN);

	return ret;
}

int acs_set_dfs_chan_forced(acs_chaninfo_t *c_info, wl_dfs_forced_t *dfs_frcd, int dfs_frcd_len)
{
	return wl_iovar_set(c_info->name, "dfs_channel_forced", dfs_frcd, dfs_frcd_len);
}

int acs_get_chanim_stats(acs_chaninfo_t *c_info, wl_chanim_stats_t *param, int param_len,
	char *buf, int buf_len)
{
	return wl_iovar_getbuf(c_info->name, "chanim_stats", param, param_len, buf, buf_len);
}

int acs_get_dfsr_counters(char *ifname, char cntbuf[ACSD_WL_CNTBUF_SIZE])
{
	return wl_iovar_get(ifname, "counters", cntbuf, ACSD_WL_CNTBUF_SIZE);
}

/* gets (updates) bgdfs capability and status of the interface; returns bgdfs capability */
uint16
acs_bgdfs_get(acs_chaninfo_t * c_info)
{
	int ret = 0;
	acs_bgdfs_info_t *acs_bgdfs = c_info->acs_bgdfs;

	if (acs_bgdfs == NULL) {
		ACSD_ERROR("acs_bgdfs is NULL");
		return BCME_ERROR;
	}

	ret = wl_iovar_get(c_info->name, "dfs_ap_move", &acs_bgdfs->status,
		sizeof(acs_bgdfs->status));
	if (ret != BCME_OK) {
		ACSD_INFO("get dfs_ap_move returned %d.\n", ret);
		return acs_bgdfs->cap = BGDFS_CAP_UNSUPPORTED;
	}
	return acs_bgdfs->cap = BGDFS_CAP_TYPE0;
}

/* request bgdfs set; for valid values of 'arg' see help page of dfs_ap_move iovar */
int
acs_bgdfs_set(acs_chaninfo_t * c_info, int arg)
{
	int ret = 0;
	ret = wl_iovar_setint(c_info->name, "dfs_ap_move",
		(int)(htod32(arg)));
	if (arg > 0 && c_info->acs_bgdfs != NULL) {
		c_info->acs_bgdfs->last_attempted = arg;
		c_info->acs_bgdfs->last_attempted_at = (uint64) uptime();
	}
	if (ret != BCME_OK) {
		ACSD_ERROR("set dfs_ap_move %d returned %d.\n", arg, ret);
	}
	return ret;
}

/* acs_update_oper_mode read the current oper_mode and update */
int
acs_update_oper_mode(acs_chaninfo_t * c_info)
{
	int ret = BCME_ERROR, oper_mode = 0;

	if ((ret = wl_iovar_getint(c_info->name, "oper_mode", &oper_mode)) != BCME_OK) {
		ACSD_ERROR("%s read oper_mode failed with %d\n", c_info->name, ret);
		return ret;
	}
	c_info->oper_mode = (uint16) oper_mode;
	ACSD_INFO("%s read oper_mode succeeded 0x%02x\n", c_info->name, oper_mode);

	return ret;
}

/* acs_set_oper_mode set the oper_mode */
int
acs_set_oper_mode(acs_chaninfo_t * c_info, uint16 oper_mode)
{
	int ret = BCME_ERROR;

	if ((ret = wl_iovar_setint(c_info->name, "oper_mode", oper_mode)) != BCME_OK) {
		ACSD_ERROR("%s setting oper_mode (0x%02x) failed with %d\n", c_info->name,
			oper_mode, ret);
		return ret;
	}

	c_info->oper_mode = oper_mode;
	ACSD_INFO("%s setting oper_mode succeeded 0x%02x\n", c_info->name, oper_mode);

	return ret;
}

int acs_get_dyn160_status(char *name, int *dyn160_status)
{
	return wl_iovar_getint(name, "dyn160", dyn160_status);
}

int acs_get_phydyn_switch_status(char *name, int *phy_dyn_switch)
{
	return wl_iovar_getint(name, "phy_dyn_switch", phy_dyn_switch);
}

int acs_set_intfer_trf_thold(char *name, wl_trf_thold_t *params, int size)
{
	return wl_iovar_set(name, "trf_thold", (void *)params, size);
}

int acs_set_intfer_params(char *name, wl_intfer_params_t *params, int size)
{
	return wl_iovar_set(name, "intfer_params", (void *)params, size);
}

int acs_get_stainfo(char *name, struct ether_addr *ea, int ether_len,
	char *stabuf, int buf_len)
{
	return wl_iovar_getbuf(name, "sta_info", &ea, sizeof(ea), stabuf, buf_len);
}

int acs_set_chanim_sample_period(char *name, uint sample_period)
{
	return wl_iovar_setint(name, "chanim_sample_period", sample_period);
}

int acs_set_noise_metric(char *name, uint8 knoise)
{
	return wl_iovar_setint(name, "noise_metric", knoise);
}

int acs_get_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size)
{
	return wl_iovar_get(ifname, "scb_probe", scb_probe, size);
}

int acs_set_scb_probe(char *ifname, wl_scb_probe_t *scb_probe, int size_probe)
{
	return wl_iovar_set(ifname, "scb_probe", scb_probe, size_probe);
}

uint acs_get_chanim_scb_lastused(acs_chaninfo_t* c_info)
{
	uint lastused = 0;
	int ret;

	ret = wl_iovar_getint(c_info->name, "scb_lastused", (int *)&lastused);

	if (ret < 0) {
		ACSD_ERROR("failed to get scb_lastused");
		return 0;
	}

	ACSD_DEBUG("lastused: %d\n", lastused);
	return lastused;
}

/* get country details for an interface */
int acs_get_country(acs_chaninfo_t * c_info)
{
	int ret = BCME_OK;

	ret = wl_iovar_get(c_info->name, "country", &c_info->country,
		sizeof(c_info->country));

	/* ensure null termination before logging/using */
	c_info->country.country_abbrev[WLC_CNTRY_BUF_SZ - 1] = '\0';
	c_info->country.ccode[WLC_CNTRY_BUF_SZ - 1] = '\0';
	c_info->country_is_edcrs_eu = acs_is_country_edcrs_eu(c_info->country.ccode);

	if (ret != BCME_OK) {
		ACSD_ERROR("get country on %s returned %d.\n", c_info->name, ret);
	} else {
		ACSD_INFO("get country on %s returned %d. ca=%s, cr=%d, cc=%s\n",
			c_info->name, ret,
			c_info->country.country_abbrev,
			c_info->country.rev, c_info->country.ccode);
	}

	return ret;
}

/* check if there is still associated scbs. reture value: TRUE if yes. */
bool acs_check_assoc_scb(acs_chaninfo_t * c_info)
{
	bool connected = TRUE;
	int result = 0;
	int ret = 0;

	ret = wl_iovar_getint(c_info->name, "scb_assoced", &result);
	if (ret) {
		ACSD_ERROR("failed to get scb_assoced\n");
		return connected;
	}

	connected = dtoh32(result) ? TRUE : FALSE;
	ACSD_DEBUG("connected: %d\n",  connected);

	return connected;
}

int acs_update_driver(acs_chaninfo_t * c_info)
{
	int ret = 0;
	bool param = TRUE;
	/* if we are already beaconing, after the acs scan and new chanspec selection,
	   we need to ask the driver to do some updates (beacon, probes, etc..).
	*/
	if (c_info->txop_channel_select == 0) {
		ret = wl_iovar_setint(c_info->name, "acs_update", htod32((uint)param));
		ACS_ERR(ret, "acs update failed\n");
	}

	return ret;
}

int chanim_update_state(acs_chaninfo_t *c_info, bool state)
{
	int ret;

	ret = wl_iovar_setint(c_info->name, "chanim_state", (uint)state);
	ACSD_CHANIM("set chanim_state: %d\n", state);

	if (ret < 0) {
		ACSD_ERROR("failed to set chanim_state");
	}
	return ret;
}

int dcs_handle_request(char* ifname, wl_bcmdcs_data_t *dcs_data,
	uint8 mode, uint8 count, uint8 csa_mode)
{
	wl_chan_switch_t csa;
	int err = ACSD_OK;

	ACSD_INFO("ifname: %s, reason: %d, chanspec: 0x%4x (%s), csa:%x\n",
		ifname, dcs_data->reason, dcs_data->chspec, wf_chspec_ntoa(dcs_data->chspec, chanspecbuf), csa_mode);

	csa.mode = mode;
	csa.count = count;
	csa.chspec = dcs_data->chspec;
	csa.reg = 0;
	csa.frame_type = csa_mode;

	err = wl_iovar_set(ifname, "csa", &csa, sizeof(wl_chan_switch_t));

	return err;
}
