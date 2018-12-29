/*
 * Key Management Module km_hw Implementation
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
 * $Id: km_hw.c 767425 2018-09-11 07:50:50Z $
 */

/* This file implements the wlc keymgmt functionality. It provides
 * hardware/memory layout dependent support
 */

#include "km_hw_pvt.h"

#ifdef PSTA
#include <wlc_psta.h>
#endif // endif
#include <wlc_ratelinkmem.h>

#define HAE(algo) CRYPTO_ALGO_##algo
static const key_algo_t km_hw_algos[] = {
	HAE(WEP1),
	HAE(WEP128),
	HAE(AES_CCM),
	HAE(AES_OCB_MSDU),
	HAE(AES_OCB_MPDU),
#ifndef LINUX_CRYPTO
	HAE(TKIP),
#endif // endif
	HAE(AES_GCM),
	HAE(AES_GCM256),
};
#undef HAE

/* public interface */
km_hw_t*
BCMATTACHFN(km_hw_attach)(wlc_info_t *wlc, keymgmt_t *km)
{
	km_hw_t *hw;
	int err = BCME_OK;
	size_t i;
	amt_idx_t amt_idx;
	size_t amt_count;
	uint corerev;

	STATIC_ASSERT(AMT_SIZE_64 >= RCMTA_SIZE);
	STATIC_ASSERT(TKIP_KEY_SIZE == 32);

	KM_DBG_ASSERT(km != NULL && wlc != NULL);
	KM_DBG_ASSERT(wlc->keymgmt == NULL || wlc->keymgmt == km);

	KM_HW_LOG(("wl%d: %s:\n",  WLCWLUNIT(wlc), __FUNCTION__));

	hw = (km_hw_t *)MALLOCZ(wlc->osh, sizeof(km_hw_t));
	if (!hw) {
		err = BCME_NOMEM;
		goto done;
	}

	hw->wlc = wlc;

	amt_idx = KM_HW_AMT_IDX_INVALID;
	amt_count = 0;
	corerev = KM_HW_COREREV(hw);

	/* support for indicies allocated for device MAC, primary BSS, MCNX/P2P,
	 * PSTA downstream (enet) TAs (WLAN MAC/RA)
	 */
	if (KM_HW_COREREV_GE40(hw)) {
		hw->max_amt_idx = AMT_SIZE(corerev);
		hw->max_idx =  AMT_SIZE(corerev);
		amt_idx = hw->max_amt_idx - 2;
#ifdef WL_RELMCAST
		if (RMC_SUPPORT(KM_HW_PUB(hw)) && RMC_ENAB(KM_HW_PUB(hw))) {
			--amt_idx;
		}
#endif // endif
	} else {
		hw->max_amt_idx = RCMTA_SIZE;
		hw->max_idx = RCMTA_SIZE;

		amt_idx = hw->max_amt_idx;
	}

#if defined(WLMCNX) && !defined(WLMCNX_DISABLED)
	amt_idx -= M_ADDR_BMP_BLK_SZ;
	amt_count += M_ADDR_BMP_BLK_SZ;
#endif /* WLMCNX */

	if (amt_count != 0) {
		hw->amt_info.mcnx_start = amt_idx;
		hw->amt_info.mcnx_count = (uint8)amt_count;
	}

	hw->max_key_size = D11_MAX_KEY_SIZE;

	/* initialize hw/algo specific processing */
	hw->impl.num_algo_entries  = ARRAYSIZE(km_hw_algos);
	hw->impl.algo_entries = MALLOCZ(wlc->osh,
		sizeof(km_hw_algo_entry_t) * hw->impl.num_algo_entries);
	if (hw->impl.algo_entries == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	for (i = 0; i < hw->impl.num_algo_entries; ++i) {
		km_hw_algo_entry_t *ae;
		ae = &hw->impl.algo_entries[i];
		ae->algo = km_hw_algos[i];
		err = km_hw_algo_init(hw, ae->algo, &ae->impl);
		KM_HW_LOG(("wl%d: %s: initialized algo %d[%s], status %d\n",
			WLCWLUNIT(wlc), __FUNCTION__,
			ae->algo, wlc_keymgmt_get_algo_name(KM_HW_KM(hw), ae->algo), err));
			/* on err hw algo will be disabled - continue */
	}
	/* HW algorithm init failure should not cause deattach, so re-initialize err to be OK */
	err = BCME_OK;

	hw->used = MALLOCZ(wlc->osh, (CEIL(hw->max_amt_idx, NBBY)));
	if (hw->used == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

done:
	if (err != BCME_OK) {
		KM_HW_ERR(("wl%d: %s: done with error %d\n",  WLCWLUNIT(wlc), __FUNCTION__, err));
		km_hw_detach(&hw);
	} else {
		/* note: reset of init is done by reset, deferred till later, since it
		 * performs shm operations
		 */
		KM_HW_LOG(("wl%d: %s: done\n",  WLCWLUNIT(wlc), __FUNCTION__));
	}

	return hw;
}

void
BCMATTACHFN(km_hw_detach)(km_hw_t **hw_in)
{
	km_hw_t *hw;
	wlc_info_t *wlc = NULL;

	if (hw_in == NULL) {
		KM_DBG_ASSERT(hw_in != NULL);
		goto done;
	}

	hw = *hw_in;
	*hw_in = NULL;

	if (hw == NULL)
		goto done;

	wlc = hw->wlc;
	KM_DBG_ASSERT(wlc != NULL);

	/* reset h/w only if it has already been initialized. */
	if (!KM_HW_NEED_INIT(hw)) {
		km_hw_reset(hw);
	}

	/* destroy hw/algo */
	if (hw->impl.algo_entries != NULL) {
		km_hw_algo_destroy_algo_entries(hw);
		MFREE(wlc->osh, hw->impl.algo_entries,
			sizeof(km_hw_algo_entry_t) * hw->impl.num_algo_entries);
	}

	if (hw->used != NULL) {
		MFREE(wlc->osh, hw->used, (CEIL(hw->max_amt_idx, NBBY)));
	}

	MFREE(wlc->osh, hw, sizeof(km_hw_t));
done:
	KM_HW_LOG(("wl%d: %s: done\n", (wlc != NULL ? WLCWLUNIT(wlc) : 0), __FUNCTION__));
}

void
BCMINITFN(km_hw_init)(km_hw_t *hw)
{
	/* initialize shm base addresses for km hw entities */
	if (KM_HW_COREREV_GE40(hw)) {
		hw->shm_info.skl_idx_base = KM_HW_SHM_ADDR_FROM_PTR(hw, M_SECKINDX_PTR(hw->wlc));
		hw->shm_info.tkip_tsc_ttak_base = KM_HW_SHM_ADDR_FROM_PTR(hw,
			M_TKIP_TTAK_PTR(hw->wlc));
	} else {
		hw->shm_info.skl_idx_base = M_SECKINDXALGO_BLK(hw->wlc);
		hw->shm_info.tkip_tsc_ttak_base = M_TKIP_TSC_TTAK(hw->wlc);
	}

	/* MAX_TSC_TTAK for d11core_rev80 will change */
	hw->shm_info.max_tsc_ttak = KM_HW_TKIP_MAX_TSC_TTAK(KM_HW_COREREV(hw));

	if (hw->wlc->machwcap & MCAP_TKIPMIC) {
		hw->shm_info.max_tkip_mic_keys = KM_HW_MAX_TKMIC_KEYS(hw);
		/* note: mic key base init by reset */
	}

}

void
km_hw_reset(km_hw_t *hw)
{
	skl_idx_t skl_idx;
	amt_idx_t amt_idx;
	int i;

	/* note: may be called during hw attach cleanup */
	if (!KM_HW_VALID(hw))
		return;

	if (!KM_HW_NEED_RESET(hw))
		return;

	KM_HW_LOG(("wl%d: %s: resetting h/w key data\n", KM_HW_UNIT(hw), __FUNCTION__));

	if (!KM_HW_KEYTAB_SUPPORTED(hw)) {
		/* rx keys in hwktab */
		hw->shm_info.key_base = KM_HW_SHM_ADDR_FROM_PTR(hw, M_SECKEYS_PTR(hw->wlc));

		/* init ptr-based shm base values for tkip and wapi */
		if (KM_HW_WLC(hw)->machwcap & MCAP_TKIPMIC) {
			hw->shm_info.tkip_mic_key_base =
				KM_HW_SHM_ADDR_FROM_PTR(hw, M_TKMICKEYS_PTR(hw->wlc));
		}

	}

	/* reset amt, skl assigments, and restore idx reservation(s) */
	memset(hw->used, 0, (CEIL(hw->max_amt_idx, NBBY)));

	if (KM_HW_COREREV_GE40(hw)) {
		km_hw_amt_reserve(hw, AMT_IDX_MAC, 1, TRUE);
		km_hw_amt_reserve(hw, AMT_IDX_BSSID, 1, TRUE);
		if (RATELINKMEM_ENAB(hw->wlc->pub)) {
			km_hw_amt_reserve(hw, AMT_IDX_RSVD_START, AMT_IDX_RSVD_SIZE, TRUE);
		}
#ifdef WL_RELMCAST
		if (RMC_SUPPORT(KM_HW_PUB(hw)) && RMC_ENAB(KM_HW_PUB(hw)))
			km_hw_amt_reserve(hw, AMT_IDX_MCAST_ADDR, 1, TRUE);
#endif // endif
	}

#ifdef WLMCNX
	if (hw->amt_info.mcnx_count != 0)
		km_hw_amt_reserve(hw, hw->amt_info.mcnx_start, hw->amt_info.mcnx_count, TRUE);
#endif // endif

#ifdef PSTA
	if (PSTA_ENAB(KM_HW_PUB(hw))) {
		km_hw_amt_reserve(hw, PSTA_TA_STRT_INDX, PSTA_RA_PRIM_INDX, TRUE);
		km_hw_amt_reserve(hw, PSTA_RA_PRIM_INDX, 1, TRUE);
	}
#endif // endif

	for (i = 0; i < hw->max_idx; ++i) {
		KM_HW_SKL_IDX(hw, i) = KM_HW_SKL_IDX_INVALID;
	}

	/* clear key blocks */
#ifdef WL_HWKTAB
	if (KM_HW_KEYTAB_SUPPORTED(hw)) {
		KM_HW_SET_HWKTAB(KM_HW_WLC(hw), 0, 0, hw->max_idx * KM_HWKTAB_SLOT_SIZE);
	} else {
#endif // endif
		wlc_set_shm(KM_HW_WLC(hw), hw->shm_info.key_base, 0,
			hw->max_idx * hw->max_key_size);
#ifdef WL_HWKTAB
	}
#endif // endif

	/* clear skl index block */
	for (skl_idx = 0; skl_idx < KM_HW_MAX_SKL_IDX(hw); ++skl_idx)
		wlc_write_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx), 0);

	/* clear amt/rcmta, we already updated used bitmask */
	for (amt_idx = 0; amt_idx < hw->max_amt_idx; ++amt_idx)
		hw_amt_update(hw, amt_idx, NULL);

	if (D11REV_LT(KM_HW_COREREV(hw), 40)) {
		/* limit h/w idx to what is configured  <= RCMTA_SIZE or AMT_SIZE */
		W_REG(KM_HW_OSH(hw), D11_rcmta_size(KM_HW_WLC(hw)), hw->max_amt_idx);
	}

	if (!KM_HW_KEYTAB_SUPPORTED(hw)) {
		/* reset tkip mic keys */
		if (hw->shm_info.tkip_mic_key_base != 0) {
			wlc_set_shm(KM_HW_WLC(hw), hw->shm_info.tkip_mic_key_base, 0,
				hw->shm_info.max_tkip_mic_keys * (KM_HW_TKIP_MIC_KEY_SIZE << 1));
		}

	}

	/* clear the ucode default key flag */
	wlc_mhf(KM_HW_WLC(hw), MHF1, MHF1_DEFKEYVALID, 0, WLC_BAND_ALL);

	hw->flags &= ~KM_HW_DEFKEYS;
	hw->flags |= (KM_HW_RESET | KM_HW_INITED);

	KM_HW_LOG(("wl%d: %s: done\n", KM_HW_UNIT(hw), __FUNCTION__));
}

void
km_hw_key_create(km_hw_t *hw, const wlc_key_t *key,
	const wlc_key_info_t *key_info, hw_idx_t *hw_idx_out)
{
	hw_idx_t hw_idx;
	skl_idx_t skl_idx;
	size_t num_idx;
	km_alloc_key_info_t alloc_info;

	KM_DBG_ASSERT(KM_HW_VALID(hw));
	KM_DBG_ASSERT(hw_idx_out != NULL && key != NULL && key_info != NULL);
	KM_ASSERT(WLC_KEY_RX_ALLOWED(key_info) || WLC_KEY_TX_ALLOWED(key_info));

	num_idx = 1;
	hw_idx = WLC_KEY_INDEX_INVALID;
	skl_idx = KM_HW_SKL_IDX_INVALID;

	if (!KM_HW_PUB(hw)->hw_up)
		goto done;

	/* lazy h/w init */
	if (KM_HW_NEED_INIT(hw))
		km_hw_reset(hw);

	hw->flags &= ~KM_HW_RESET;

	/* check if key will fit into our slot */
	if (key_info->key_len > hw->max_key_size)
		goto done;

	km_get_alloc_key_info(KM_HW_KM(hw), key_info->key_idx, &alloc_info);
	KM_DBG_ASSERT(alloc_info.bss_info.bsscfg != NULL);

	if (WLC_KEY_IS_DEFAULT_BSS(key_info)) {
		KM_DBG_ASSERT(!WLC_KEY_IS_PAIRWISE(key_info));
		hw_idx = key_info->key_id;

		if (!KM_HW_KEY_IS_TXONLY(hw, key_info, alloc_info.bss_info.bsscfg))
			skl_idx = (skl_idx_t)hw_idx;
		else
			skl_idx = KM_HW_SKL_IDX_TXONLY;

		/* no amt */
		KM_HW_SKL_IDX(hw, hw_idx) = skl_idx;
		goto done;
	}

	/* no need to coordinate txonly keys with pairwise */
	if (KM_HW_KEY_IS_TXONLY(hw, key_info, alloc_info.bss_info.bsscfg)) {
		hw_idx = hw_idx_alloc(hw, 1, NULL);
		goto done;
	}

	KM_ASSERT(alloc_info.scb_info.scb != NULL);

	hw_get_hw_idx_from_alloc_info(hw, key_info, &alloc_info, &hw_idx);
	if (hw_idx != WLC_KEY_INDEX_INVALID) {
		KM_DBG_ASSERT(KM_HW_IDX_VALID(hw, hw_idx));
		skl_idx = KM_HW_SKL_IDX(hw, hw_idx);
		KM_DBG_ASSERT(KM_HW_SKL_IDX_VALID(hw, skl_idx));
		goto have_idx;
	}

	/* we need to allocate all of the keys, sta group keys first */
	if (KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(hw, alloc_info.bss_info.bsscfg))
		num_idx += WLC_KEYMGMT_NUM_STA_GROUP_KEYS;

	hw_idx = hw_idx_alloc(hw, num_idx, &alloc_info);
	if (hw_idx == WLC_KEY_INDEX_INVALID)
		goto done;

	if ((key_info->algo == CRYPTO_ALGO_TKIP) &&
		!KM_HW_TKIP_TSC_TTAK_SUPPORTED(hw, hw_idx + num_idx - 1)) {
		hw_idx_release(hw, hw_idx, num_idx, &alloc_info.scb_info.scb->ea);
		hw_idx = WLC_KEY_INDEX_INVALID;
		goto done;
	}

	skl_idx =  KM_HW_SKL_IDX(hw, hw_idx);
	KM_DBG_ASSERT(KM_HW_SKL_IDX_VALID(hw, skl_idx));

have_idx:
	/* hw_idx now has the pairwise idx, adjust it for caller */
	if (WLC_KEY_IS_GROUP(key_info) && !WLC_KEY_IS_AP(key_info)) {
		hw_idx += key_info->key_id;
	}

done:
	*hw_idx_out = hw_idx;
	KM_HW_LOG(("wl%d: %s: allocated %d indicies for key idx %d - [hw,skl]: %d, %d\n",
		KM_HW_UNIT(hw), __FUNCTION__, (int)num_idx, key_info->key_idx, hw_idx, skl_idx));
}

void km_hw_key_destroy(km_hw_t *hw, hw_idx_t *hw_idx_in,
	const wlc_key_info_t *key_info)
{
	hw_idx_t hw_idx;
	hw_idx_t base_hw_idx;
	km_alloc_key_info_t alloc_info;
	skl_idx_t 	skl_idx;
	const struct ether_addr *ea = NULL;
	size_t num_idx;

	KM_DBG_ASSERT(KM_HW_VALID(hw));
	if (!hw_idx_in) {
		KM_DBG_ASSERT(hw_idx_in != NULL);
		return;
	}

	/* note: the order of operations here is important */
	hw_idx = *hw_idx_in;
	if (hw_idx == WLC_KEY_INDEX_INVALID)
		goto done;

	skl_idx = KM_HW_SKL_IDX(hw, hw_idx);

	if (WLC_KEY_IS_DEFAULT_BSS(key_info)) {
		KM_ASSERT(hw_idx < WLC_KEYMGMT_NUM_GROUP_KEYS);
		hw_idx_release(hw, hw_idx, 1, NULL);
		goto skl_clean;
	}

	km_get_alloc_key_info(KM_HW_KM(hw), key_info->key_idx, &alloc_info);
	KM_DBG_ASSERT(alloc_info.bss_info.bsscfg != NULL);

	if (KM_HW_KEY_IS_TXONLY(hw, key_info, alloc_info.bss_info.bsscfg) ||
		WLC_KEY_IS_AP(key_info)) {
		if (!WLC_KEY_IS_GROUP(key_info))
			ea = &key_info->addr;
		hw_idx_release(hw, hw_idx, 1, ea);
		goto skl_clean;
	}

	/* we destroy our state for all related h/w keys together */
	if (!hw_idx_delete_ok(hw, hw_idx, &alloc_info))
		goto done;

	hw_get_hw_idx_from_alloc_info(hw, key_info, &alloc_info, &base_hw_idx);
	if (!KM_HW_IDX_VALID(hw, base_hw_idx)) {
		KM_HW_LOG(("wl%d: %s: base hw idx not found\n", KM_HW_UNIT(hw), __FUNCTION__));
		goto done;
	}

	ea = hw_amt_get_addr(hw, key_info, alloc_info.bss_info.bsscfg,
		alloc_info.scb_info.scb);

	num_idx = 1;
	if (KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(hw, alloc_info.bss_info.bsscfg))
		num_idx += WLC_KEYMGMT_NUM_STA_GROUP_KEYS;

	hw_idx_release(hw, base_hw_idx, num_idx, ea);

skl_clean:
	if (KM_HW_SKL_IDX_VALID(hw, skl_idx))
		wlc_write_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx), 0);

done:
	*hw_idx_in = WLC_KEY_INDEX_INVALID;
	KM_HW_LOG(("wl%d: %s: destroyed hw_idx %d for key_idx %d\n",
		KM_HW_UNIT(hw), __FUNCTION__, hw_idx, key_info->key_idx));
}

void
km_hw_key_update(km_hw_t *hw, wlc_key_hw_index_t hw_idx,
	wlc_key_t *key, const wlc_key_info_t *key_info)
{
	int		err = BCME_OK;
	amt_idx_t	amt_idx;
	amt_attr_t	amt_attr;
	skl_idx_t	skl_idx;
	uint16		skl_val;
	hw_idx_t	skl_hw_idx;
	km_alloc_key_info_t alloc_info;
	const km_hw_algo_entry_t* ae;
	bool mhf_defkeys;
	hw_algo_t skl_hw_algo;
	wlc_bsscfg_t *bsscfg;

	KM_DBG_ASSERT(KM_HW_VALID(hw));
	KM_DBG_ASSERT(key != NULL && key_info != NULL);

	KM_HW_LOG(("wl%d: %s: enter hw idx 0x%04x, key idx %d\n",
		KM_HW_UNIT(hw), __FUNCTION__, hw_idx, (int)key_info->key_idx));

	skl_idx = KM_HW_SKL_IDX_INVALID;
	amt_idx = KM_HW_AMT_IDX_INVALID;
	amt_attr = 0;
	bsscfg = NULL;

	if (!KM_HW_IDX_VALID(hw, hw_idx)) {
		KM_DBG_ASSERT(KM_HW_IDX_VALID(hw, hw_idx));
		err = BCME_BADKEYIDX;
		goto done;
	}

	/* skl idx must be valid except for tx only hw idx */
	skl_idx = KM_HW_SKL_IDX(hw, hw_idx);
	if (skl_idx == KM_HW_SKL_IDX_TXONLY)
		goto done_skl;

	if (!KM_HW_SKL_IDX_VALID(hw, skl_idx)) {
		err = BCME_BADKEYIDX;
		goto done;
	}

	/* clear addr match - to prevent ucode from getting a match during update */
	if (KM_HW_SKL_IDX_HAS_AMT(hw, skl_idx)) {
		amt_idx = skl_idx - WLC_KEYMGMT_NUM_GROUP_KEYS;
		amt_attr = wlc_clear_addrmatch(KM_HW_WLC(hw), amt_idx);
	}

	/* update skl; note that skl_val gets programmed with pairwise hw idx */
	km_get_alloc_key_info(KM_HW_KM(hw), key_info->key_idx, &alloc_info);
	bsscfg = alloc_info.bss_info.bsscfg;

	hw_get_hw_idx_from_alloc_info(hw, key_info, &alloc_info, &skl_hw_idx);
	skl_hw_algo = KM_HW_SKL_HW_ALGO(hw, key_info);
	skl_val = wlc_read_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx));

	{
		KM_DBG_ASSERT(!WLC_KEY_IS_IBSS(key_info) || (bsscfg->WPA_auth == WPA_AUTH_NONE) ||
			(bsscfg->WPA_auth == WPA_AUTH_DISABLED) ||
			BSS_TDLS_ENAB(KM_HW_WLC(hw), bsscfg));
		skl_val &= ~(SKL_ALGO_MASK | (KM_HW_SKL_INDEX_MASK(hw)) | SKL_WAPI_KEYID_MASK);
		skl_val |= (skl_hw_algo << SKL_ALGO_SHIFT) & SKL_ALGO_MASK;
		skl_val |= (KM_HW_IDX_TO_SLOT(hw, skl_hw_idx) << SKL_INDEX_SHIFT) &
			(KM_HW_SKL_INDEX_MASK(hw));

		if (WLC_KEY_IS_GROUP(key_info) &&
			KM_HW_BSSCFG_NEED_STA_GROUP_KEYS(hw, bsscfg)) {
			skl_val &= ~KM_HW_SKL_GRP_ALGO_MASK(hw);
			skl_val |= (skl_hw_algo << KM_HW_SKL_GRP_ALGO_SHIFT(hw)) &
				KM_HW_SKL_GRP_ALGO_MASK(hw);
		}

	}

	if (KM_HW_COREREV_GE80(hw) &&
		(key_info->algo == CRYPTO_ALGO_TKIP)) {
		/* Add TTAK P1 key index of shm M_TKIP_TSC_TTAK */
		skl_val |= ((skl_hw_idx - WLC_KEYMGMT_NUM_GROUP_KEYS) << SKL_TTAK_INDEX_SHIFT);
	}
	KM_HW_LOG(("wl%d: %s: updating skl idx %d with val %d - hw idx %d algo %d\n",
		KM_HW_UNIT(hw), __FUNCTION__, skl_idx, skl_val, skl_hw_idx, skl_hw_algo));

	wlc_write_shm(KM_HW_WLC(hw), KM_HW_SKL_IDX_ADDR(hw, skl_idx), skl_val);

done_skl:
	ae = km_hw_find_algo_entry(hw, key_info->algo);
	if (ae == NULL) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	err = km_hw_algo_set_hw_key(hw, hw_idx, ae, ae->impl.dt_mask, key, key_info);
	if (err != BCME_OK)
		goto done;

done:
	/* restore addr match adding A1/A2 on success */
	if (amt_idx != KM_HW_AMT_IDX_INVALID) {
		const struct ether_addr *ea;

		if (err == BCME_OK)
			amt_attr |= (AMT_ATTR_VALID | AMT_ATTR_A2);

		ea = hw_amt_get_addr(hw, key_info, bsscfg, alloc_info.scb_info.scb);
		wlc_set_addrmatch(KM_HW_WLC(hw), amt_idx, ea, amt_attr);
		if (RATELINKMEM_ENAB(hw->wlc->pub)) {
			wlc_ratelinkmem_update_link_entry(hw->wlc, alloc_info.scb_info.scb);
		}
	}

	/* update def key valid if it changes */
	if (err == BCME_OK) {
		bool upd_mhf = FALSE;

		mhf_defkeys = km_rxucdefkeys(KM_HW_KM(hw));
		if (mhf_defkeys && !(hw->flags & KM_HW_DEFKEYS)) {
			upd_mhf = TRUE;
			hw->flags |= KM_HW_DEFKEYS;
		} else if (!mhf_defkeys && (hw->flags & KM_HW_DEFKEYS)) {
			upd_mhf = TRUE;
			hw->flags &= ~KM_HW_DEFKEYS;
		}

		if (upd_mhf) {
			wlc_mhf(KM_HW_WLC(hw), MHF1, MHF1_DEFKEYVALID,
				(mhf_defkeys ? MHF1_DEFKEYVALID : 0), WLC_BAND_ALL);
			/* clear all amt entries if DEFKEYVALID is cleared, as no defkey is used */
			km_amt_defkey_clearall(KM_HW_KM(hw));

		}

		if (amt_idx != KM_HW_AMT_IDX_INVALID) {
			/* update defkey valid for an amt entry */
			km_amt_defkey_update(KM_HW_KM(hw), amt_idx);
		}
	}

	KM_HW_LOG(("wl%d: %s: updated hw_idx %d for key_idx %d amt_attr 0x%02x status %d\n",
		KM_HW_UNIT(hw), __FUNCTION__, hw_idx, key_info->key_idx, amt_attr, err));
}

bool
km_hw_key_hw_mic(km_hw_t *hw, hw_idx_t hw_idx, wlc_key_info_t *key_info)
{
	bool hw_mic = FALSE;

	KM_DBG_ASSERT(KM_HW_VALID(hw));

	if (hw_idx == WLC_KEY_INDEX_INVALID || key_info == NULL)
		goto done;

	switch (key_info->algo) {
	case CRYPTO_ALGO_TKIP:
		if (!KM_HW_KEYTAB_SUPPORTED(hw) && !hw->shm_info.tkip_mic_key_base)
				break;
		if (hw_idx >= hw->shm_info.max_tkip_mic_keys)
			break;
		hw_mic = TRUE;
		break;
	case CRYPTO_ALGO_SMS4:
		if (!KM_HW_KEYTAB_SUPPORTED(hw) && !hw->shm_info.wapi_mic_key_base)
			break;
		if (hw_idx >= hw->shm_info.max_wapi_mic_keys)
			break;
		hw_mic = TRUE;
		break;
	default:
		break;
	}

done:
	return hw_mic;
}

bool
km_hw_amt_idx_valid(km_hw_t *hw, amt_idx_t amt_idx)
{
	return KM_HW_AMT_IDX_VALID(hw, amt_idx);
}
