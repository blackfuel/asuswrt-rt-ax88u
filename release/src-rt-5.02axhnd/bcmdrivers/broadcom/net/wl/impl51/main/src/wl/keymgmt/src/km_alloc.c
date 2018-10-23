/*
 * Key Management Module Implementation - allocation support
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
 * $Id: km_alloc.c 523118 2014-12-26 18:53:31Z $
 */

#include "km_pvt.h"

/* Allocation takes into consideration the layout requirements for
 * lower-level - in particular the following requirements
 *		Group keys for WLC default BSS use indicies 0..3
 *		STA group keys are contiguous - follow pairwise key
 *		IBSS peer group keys are offset by WLC_KEYMGMT_IBSS_MAX_PEERS from
		pairwise key and each other
 * Key management implementation also places a few requirements.
 *		-- STA group keys and IBSS peer group keys are allocated
 *		together
 *		-- Management group keys are allocated at high indicies
 */

/* Internal interface */
static int
km_alloc_contiguous(keymgmt_t *km, wlc_key_index_t key_idx_arr[],
	size_t num_keys, bool at_end)
{
	int err = BCME_NOMEM;
	wlc_key_index_t i, j;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(num_keys > 0 && num_keys < 65536);

	if (num_keys > km->max_keys)
		goto done;

	key_idx = WLC_KEY_INDEX_INVALID;
	i = at_end ? (km->max_keys - (uint16)num_keys) : 0;
	do {
		size_t num_found = 0;
		for (j = 0; j < num_keys && num_found < num_keys; ++j) {
			if ((i+j) >= km->max_keys)
				break;
			km_pvt_key = &km->keys[i+j];
			if (km_pvt_key->flags & KM_FLAG_IDX_ALLOC) { /* already allocated */
				++j; /* to skip to entry after this */
				break;
			}
			++num_found;
		}
		if (num_found == num_keys) {
			key_idx = i;
			break;
		}

		i = at_end ? (i - 1) : (i + j);
	} while ((at_end && i > 0) || (!at_end && i < km->max_keys));

	if (key_idx != WLC_KEY_INDEX_INVALID) {
		for (j = 0; j < num_keys; ++j, ++key_idx) {
			km_pvt_key = &km->keys[key_idx];
			km_pvt_key->flags |= KM_FLAG_IDX_ALLOC;
			key_idx_arr[j] = key_idx;
			KM_LOG(("wl%d: %s: allocated key index 0x%04x\n",
				KM_UNIT(km), __FUNCTION__, key_idx));
		}
		err = BCME_OK;
	}

done:
	return err;
}

/* External interface */
int
km_alloc_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
    wlc_key_index_t key_idx_arr[], size_t num_keys)
{
	int err = BCME_OK;
	bool at_end;
	size_t i;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_LOG(("wl%d: %s: enter flags 0x%08x num_keys %d\n",
		KM_UNIT(km), __FUNCTION__, key_flags, (int)num_keys));

	at_end = key_flags & WLC_KEY_FLAG_MGMT_GROUP;
	err = km_alloc_contiguous(km, key_idx_arr, num_keys, at_end);
	if (err == BCME_OK)
		goto done;

	/* contiguous block not available; allocate non-contiguous */
	for (i = 0; i < num_keys; ++i) {
		err = km_alloc_contiguous(km, &key_idx_arr[i], 1, at_end);
		if (err != BCME_OK)
			break;
	}

	if (err != BCME_OK && i > 0)
		km_free_key_block(km, key_flags, key_idx_arr, i);
done:
	KM_LOG(("wl%d: %s: exit status %d\n", KM_UNIT(km), __FUNCTION__, err));
	return err;
}

void
km_free_key_block(keymgmt_t *km, wlc_key_flags_t key_flags,
    wlc_key_index_t key_idx_arr[], size_t num_keys)
{
	size_t i;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(KM_VALID(km));

	KM_LOG(("wl%d: %s: enter flags 0x%08x num_keys %d\n",
		KM_UNIT(km), __FUNCTION__, key_flags, (int)num_keys));

	(void)key_flags;

	for (i = 0; i < num_keys; ++i) {
		key_idx = key_idx_arr[i];
		if (key_idx == WLC_KEY_INDEX_INVALID)
			continue;

		KM_LOG(("wl%d: %s: freeing key index 0x%04x\n",
			KM_UNIT(km), __FUNCTION__, key_idx));

		km_pvt_key = &km->keys[key_idx];

		/* must have been allocated */
		KM_ASSERT((km_pvt_key->flags & KM_FLAG_IDX_ALLOC) != 0);

		if (km_pvt_key->key != NULL) {
			KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
			KM_ASSERT(KM_VALID_KEY(&km->keys[key_idx]));
			km_key_destroy(&km_pvt_key->key);
			KM_ASSERT_KEY_DESTROYED(km, key_idx);
		}

		km_pvt_key->flags = KM_FLAG_NONE;
		key_idx_arr[i] = WLC_KEY_INDEX_INVALID;
	}
}

void
km_get_alloc_key_info(wlc_keymgmt_t *km, wlc_key_index_t key_idx,
	km_alloc_key_info_t *alloc_key_info)
{
	km_pvt_key_t *km_pvt_key;
	wlc_bsscfg_t *bsscfg = NULL;
	scb_t *scb = NULL;
	km_bsscfg_t *bss_km = NULL;
	km_scb_t *scb_km = NULL;
	wlc_key_id_t key_id;

	STATIC_ASSERT(WLC_KEYMGMT_NUM_STA_GROUP_KEYS == 2);

	KM_DBG_ASSERT(KM_VALID(km) && alloc_key_info != NULL &&
		KM_VALID_KEY_IDX(km, key_idx));

	KM_LOG(("wl%d: %s: key idx 0x%04x, alloc info@%p\n", KM_UNIT(km), __FUNCTION__,
		key_idx, alloc_key_info));

	/* lookup bsscfg and scb */
	km_pvt_key = &km->keys[key_idx];
	if (km_pvt_key->flags & KM_FLAG_BSS_KEY) {
		bsscfg = km_pvt_key->u.bsscfg;
		KM_DBG_ASSERT(bsscfg != NULL);
		bss_km = KM_BSSCFG(km, bsscfg);
		if (bss_km->scb_key_idx != WLC_KEY_INDEX_INVALID) {
			key_idx = bss_km->scb_key_idx;
			KM_ASSERT(KM_VALID_KEY_IDX(km, key_idx));
			km_pvt_key = &km->keys[key_idx];
			scb = km_pvt_key->u.scb;
			KM_DBG_ASSERT(scb != NULL && bsscfg == SCB_BSSCFG(scb));
		}
	} if (km_pvt_key->flags & KM_FLAG_SCB_KEY) {
		scb = km_pvt_key->u.scb;
		KM_DBG_ASSERT(scb != NULL);
		bsscfg = SCB_BSSCFG(scb);
		KM_DBG_ASSERT(bsscfg != NULL);
		bss_km = KM_BSSCFG(km, bsscfg);
	}

	for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
		alloc_key_info->bss_info.key_idx[key_id] = WLC_KEY_INDEX_INVALID;
		alloc_key_info->bss_info.hw_idx[key_id] = WLC_KEY_INDEX_INVALID;
		if (bsscfg != NULL) {
			key_idx = bss_km->key_idx[key_id];
			alloc_key_info->bss_info.key_idx[key_id] = key_idx;
			alloc_key_info->bss_info.hw_idx[key_id] = km_get_hw_idx(km, key_idx);
		}

	}

	alloc_key_info->scb_info.key_idx = WLC_KEY_INDEX_INVALID;
	alloc_key_info->scb_info.hw_idx = WLC_KEY_INDEX_INVALID;

	if (scb != NULL) {
		scb_km = KM_SCB(km, scb);
		key_idx = scb_km->key_idx;
		alloc_key_info->scb_info.key_idx = key_idx;
		alloc_key_info->scb_info.hw_idx = km_get_hw_idx(km, key_idx);
	}

	alloc_key_info->scb_info.scb = scb;
	alloc_key_info->bss_info.bsscfg = bsscfg;
	alloc_key_info->bss_info.amt_idx = bss_km->amt_idx;
}
