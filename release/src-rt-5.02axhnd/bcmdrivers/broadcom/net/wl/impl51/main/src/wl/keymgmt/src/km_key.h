/*
 * Internal interface to wlc_key implementation
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
 * $Id: km_key.h 616104 2016-01-29 18:26:03Z $
 */

#ifndef _km_key_h_
#define _km_key_h_

#include <wlc_key.h>
#ifdef ACKSUPR_MAC_FILTER
#include <wlc_macfltr.h>
#endif /* ACKSUPR_MAC_FILTER */
#define KM_KEY_MAX_DATA_LEN 32

/* key dump types */
enum {
    KM_KEY_DUMP_NONE   		= 0x00,
    KM_KEY_DUMP_SW_KEYS     = 0x01,
    KM_KEY_DUMP_HW_KEYS     = 0x02,
    KM_KEY_DUMP_SECALGO     = 0x04,
    KM_KEY_DUMP_ALL    		= 0xff
};
typedef int km_key_dump_type_t;

int km_key_create(wlc_keymgmt_t *km, const wlc_key_info_t *key_info,
	wlc_key_t **key);
int km_key_destroy(wlc_key_t **key);

#if defined(BCMDBG)
void km_key_dump(wlc_key_t *key, struct bcmstrbuf *buf);
#endif // endif

int km_key_get_data(wlc_key_t *key, uint8* data, size_t data_size,
    size_t *data_len, wlc_key_data_type_t type, int instance, bool tx);

/* internal interfaces have additional settable key flags */
#define KM_KEY_SETTABLE_FLAGS (WLC_KEY_SETTABLE_FLAGS |\
	WLC_KEY_FLAG_TX | WLC_KEY_FLAG_RX |\
	WLC_KEY_FLAG_IBSS |\
	WLC_KEY_FLAG_IN_HW | WLC_KEY_FLAG_HW_MIC |\
	WLC_KEY_FLAG_LINUX_CRYPTO |\
	WLC_KEY_FLAG_MULTI_BAND |\
	WLC_KEY_FLAG_GTK_RESET |\
	WLC_KEY_FLAG_WAPI_HAS_PREV_KEY |\
	WLC_KEY_FLAG_NO_REPLAY_CHECK |\
	WLC_KEY_FLAG_ARM_TX_ENABLED |\
	0)

/* suppress notification for these flag changes */
#define KM_KEY_FLAGS_NOTIFY_MASK(_flags)  ((_flags) & ~(WLC_KEY_FLAG_GTK_RESET | \
	WLC_KEY_FLAG_NO_REPLAY_CHECK | \
	WLC_KEY_FLAG_ARM_TX_ENABLED | \
	0))

/* helper macros */

#define KEY_SEQ_SIZE 6 			/* key seq/PN size, except WAPI */

/* increment an seq in LE order */
#define KEY_SEQ_INCR(_seq, _sz) do {\
	size_t _i; \
	for (_i = 0; _i < (_sz); ++(_i))\
		if (++((_seq)[_i]))\
			break; \
	} while (0)

#define KEY_SEQ_DECR(_seq, _sz) do {\
	size_t _i; \
	for (_i = 0; _i < (_sz); ++(_i))\
		if (((_seq)[_i])--)\
			break; \
	} while (0)

#define KEY_SEQ_HI32(_seq) ltoh32_ua(&(_seq)[2])
#define KEY_SEQ_LO16(_seq) ltoh16_ua(&(_seq)[0])
#define KEY_SEQ_FORMAT "0x%02x%02x%02x%02x%02x%02x"
#define KEY_SEQ_FORMAT_ARG(_seq) (_seq)[5], (_seq)[4], (_seq)[3], \
	 (_seq)[2], (_seq)[1], (_seq)[0]

#define KEY_SEQ_IS_MAX(_seq) ((_seq)[0] == 0xff && (_seq)[1] == 0xff && \
		(_seq)[2] == 0xff && (_seq)[3] == 0xff && \
		(_seq)[4] == 0xff && (_seq)[5] == 0xff)

/* Cached used to optimize key lookup on rx/tx */
struct km_key_cache {
	wlc_key_t *key;
	const wlc_key_info_t *key_info;
	wlc_key_hw_index_t hw_idx;
};
typedef struct km_key_cache  km_key_cache_t;

/* prototypes */
void km_key_set_flags(wlc_key_t *key, wlc_key_flags_t flags);

/* stores input hw idx and returns previous one */
wlc_key_hw_index_t km_key_set_hw_idx(wlc_key_t *key,
	wlc_key_hw_index_t hw_idx, bool hw_mic);

/* check (rx) seq for replay */
bool km_key_seq_less(const uint8* seq1, const uint8* seq2, size_t seq_len);

/* shared key war: fixup h/w use of wep default keys for non-default bss */
int km_key_wep_rx_defkey_fixup(wlc_key_t *rx_key, uint8 *body, int body_len);

/* reset keymgmt key cache info */
void km_key_update_key_cache(wlc_key_t *key, km_key_cache_t *key_cache);

/* set the key id in a key - for dynamic wep support */
int km_key_set_key_id(wlc_key_t *key, wlc_key_id_t key_id);

#endif /* _km_key_h_ */
