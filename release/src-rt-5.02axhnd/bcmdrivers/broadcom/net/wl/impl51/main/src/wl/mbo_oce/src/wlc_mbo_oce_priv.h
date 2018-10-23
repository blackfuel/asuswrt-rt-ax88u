/*
 * MBO-OCE Private declarations/definitions for
 * Broadcom 802.11abgn Networking Device Driver
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 *
 */

/**
 */

#ifndef _wlc_mbo_oce_priv_h_
#define _wlc_mbo_oce_priv_h_

typedef void *wlc_mbo_oce_ie_build_hndl_t;
typedef void *wlc_mbo_oce_ie_parse_hndl_t;

/*
 * - 'cbparm' points to the user supplied calc_len/build parameters
 *   structure from the wlc_iem_calc_len()/wlc_iem_build_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'tag' is that the callback was registered for.
 * - 'buf' = points to the buffer where callback writes the attributes.
 *    buf = NULL; if called for  getting attribute length.
 * - 'buf_len' = Length of the buffer.
 *    buf_len = 0; if called for getting attribute length.
 */
typedef struct {
	wlc_iem_cbparm_t *cbparm;  /* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;  /* Frame type */
	uint8 tag;  /* IE tag */
	uint8 *buf; /* IE buffer pointer to put attributes */
	uint buf_len; /* buffer length */
} wlc_mbo_oce_attr_build_data_t;

/* This same callback will be called during allocating buffer
 * to know the length of attributes it is going to write,
 * as well as for actual writing of attributes into the buffer.
 * during length calculation: 'buf' = NULL and 'buf_len' = 0.
 * during writing attribute into IE: 'buf' = pointer to buffer and
 * 'buf_len' = available buffer length.
 */
typedef int
(*wlc_mbo_oce_attr_build_fn_t)(void *ctx, wlc_mbo_oce_attr_build_data_t *data);

/*
 * - 'pparm' points to the parse callback parameters structure from the
 *   wlc_iem_parse_frame() caller.
 * - 'cfg' is the pointer to bsscfg for which the call is issued.
 * - 'ft' is the frame type FC_XXXX as defined in 802.11.h (see also
 *   (WLC_IEM_FC_SCAN_XXXX in wlc_iem_mgmt.h)
 * - 'ie' = points to the begining of MBO+OCE attributes
 * - 'ie_len' = Length of only MBO+OCE attributes without MBO_OCE header.
 */
typedef struct {
	wlc_iem_pparm_t *pparm; /* Callback parameters */
	wlc_bsscfg_t *cfg;
	uint16 ft;  /* Frame type */
	uint8 *ie; /* IE pointer */
	uint ie_len; /* buffer length */
} wlc_mbo_oce_attr_parse_data_t;

typedef int
(*wlc_mbo_oce_attr_parse_fn_t)(void *ctx, wlc_mbo_oce_attr_parse_data_t *data);

typedef struct wlc_mbo_oce_ie_build_data {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_build_fn_t build_fn;
} wlc_mbo_oce_ie_build_data_t;

typedef struct wlc_mbo_oce_ie_parse_data {
	void *ctx;
	uint16 fstbmp;
	wlc_mbo_oce_attr_parse_fn_t parse_fn;
} wlc_mbo_oce_ie_parse_data_t;

wlc_mbo_oce_ie_build_hndl_t
wlc_mbo_oce_register_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_data_t* build_data);
wlc_mbo_oce_ie_parse_hndl_t
wlc_mbo_oce_register_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_data_t *parse_data);
int
wlc_mbo_oce_unregister_ie_build_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_build_hndl_t hndl);
int
wlc_mbo_oce_unregister_ie_parse_cb(wlc_mbo_oce_info_t *mbo_oce,
	wlc_mbo_oce_ie_parse_hndl_t hndl);

#endif	/* _wlc_mbo_oce_priv_h_ */
