/*
 * Key Management Module Implementation - notify support
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
 * $Id: km_notify.c 756573 2018-04-09 21:44:16Z $
 */

#include "km_pvt.h"

#include <wlc_txc.h>
#include <wlc_btcx.h>
#include <wlc_bmac.h>
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#ifdef WLCFP
#include <wlc_cfp.h>
#endif // endif
#if defined(PKTC) || defined(PKTC_DONGLE)
/* TODO: remove this circular dependency */
#include <wlc_pktc.h>
#endif // endif
#ifdef WL_PWRSTATS
#include <wlc_pwrstats.h>
#endif /* WL_PWRSTATS */
#include <wlc_pm.h>
#include <wlc_ratelinkmem.h>

/* Security algos for which key exchange is required */
#define SEC_ALGO_KEY_EXCH (CRYPTO_ALGO_TKIP | CRYPTO_ALGO_AES_CCM)

/* internal interface */

/* ensure we have h/w key if necessary, and return TRUE if h/w idx was created */
static bool
km_ensure_hw_key(keymgmt_t *km, wlc_key_info_t *key_info)
{
	km_pvt_key_t *km_pvt_key;
	wlc_key_hw_index_t hw_idx;
	wlc_key_index_t key_idx;
	wlc_key_t *key;

	KM_DBG_ASSERT(KM_VALID(km) && key_info != NULL);

	hw_idx = WLC_KEY_INDEX_INVALID;
	key_idx = key_info->key_idx;
	if (key_idx == WLC_KEY_INDEX_INVALID)
		goto done;

	km_pvt_key = &km->keys[key_idx];
	if (!KM_VALID_KEY(km_pvt_key))
		goto done;

	if (!km_needs_hw_key(km, km_pvt_key, key_info))
		goto done;

	key = km_pvt_key->key;
	km_hw_key_create(km->hw, key, key_info, &hw_idx);
	if (hw_idx == WLC_KEY_INDEX_INVALID)
		goto done;
	km_key_set_hw_idx(key, hw_idx, km_hw_key_hw_mic(km->hw, hw_idx, key_info));
	++(km->stats.num_hw_keys);

	wlc_key_get_info(key, key_info);
	km_hw_key_update(km->hw, hw_idx, key, key_info);

done:
	KM_LOG(("wl%d: key idx %04x, hw idx %04x\n", KM_UNIT(km), key_info->key_idx, hw_idx));
	return (hw_idx != WLC_KEY_INDEX_INVALID);
}

static void
km_handle_key_delete(keymgmt_t *km, wlc_key_t *key, wlc_key_info_t *key_info)
{
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;
	wlc_key_hw_index_t hw_idx;

	/* note: km->null_key has invalid key index, destroyed upon km detach */
	key_idx = key_info->key_idx;
	KM_DBG_ASSERT((key_idx != WLC_KEY_INDEX_INVALID) &&
		KM_VALID_KEY_IDX(km, key_idx));

	km_pvt_key = &km->keys[key_idx];
	KM_DBG_ASSERT(KM_VALID_KEY(km_pvt_key));

	if (km_pvt_key->key_algo != CRYPTO_ALGO_NONE) {
		KM_ASSERT(km->stats.num_sw_keys > 0);
		--(km->stats.num_sw_keys);
	}

	/* release hw key, if any. if hw/idx is being release attempt
	 * to allocate it to another valid key
	 */
	if (!WLC_KEY_IN_HW(key_info))
		goto done;

	--(km->stats.num_hw_keys);
	hw_idx = wlc_key_get_hw_idx(km_pvt_key->key);
	km_hw_key_destroy(km->hw, &hw_idx, key_info);
	KM_DBG_ASSERT(hw_idx == WLC_KEY_INDEX_INVALID);
	km_key_set_hw_idx(km_pvt_key->key, WLC_KEY_INDEX_INVALID, FALSE);

	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key2;
		wlc_key_info_t key_info2;

		km_pvt_key2 = &km->keys[key_idx];
		if ((key_idx == key_info->key_idx) || !KM_VALID_KEY(km_pvt_key2))
			continue;
		wlc_key_get_info(km_pvt_key2->key, &key_info2);
		if (km_ensure_hw_key(km, &key_info2))
			break;
	}

done:
	km_event_signal(km, WLC_KEYMGMT_EVENT_KEY_DESTROY, NULL, key, NULL);
	memset(km_pvt_key, 0, sizeof(*km_pvt_key));
}

/* key update handler. note: it must not call anything that causes key update */
static void
km_handle_key_update(keymgmt_t *km, wlc_key_t *key, wlc_key_info_t *key_info)
{
	wlc_key_index_t key_idx;
	wlc_key_hw_index_t hw_idx;
	km_pvt_key_t *km_pvt_key;
	wlc_keymgmt_event_t event;
	wlc_bsscfg_t *bsscfg;

	KM_TRACE(("%s enter\n", __FUNCTION__));

	/* note: km->null_key has invalid key index, destroyed upon km detach, we also
	 * ignore notification of its update (creation)
	 */
	key_idx = key_info->key_idx;
	if (key_idx == WLC_KEY_INDEX_INVALID)
		return;

	KM_DBG_ASSERT(KM_VALID_KEY_IDX(km, key_idx));

	km_pvt_key = &km->keys[key_idx];
	KM_DBG_ASSERT(KM_VALID_KEY(km_pvt_key));

	/* algo none, is never in h/w, nothing to update. */
	if (km_pvt_key->key_algo == CRYPTO_ALGO_NONE && key_info->algo == CRYPTO_ALGO_NONE)
			return;

	/* algo change not-none to/from none */
	if (km_pvt_key->key_algo == CRYPTO_ALGO_NONE || key_info->algo == CRYPTO_ALGO_NONE) {
		if (km_pvt_key->key_algo == CRYPTO_ALGO_NONE) {
			++(km->stats.num_sw_keys);
		} else if (key_info->algo == CRYPTO_ALGO_NONE) {
			--(km->stats.num_sw_keys);
#ifdef BRCMAPIVTW
			km_ivtw_enable(km->ivtw, key_idx, FALSE);
#endif /* BRCMAPIVTW */
		}
	}

	/* if algorithm is changing from not-none, destory old h/w key */
	if (km_pvt_key->key_algo != CRYPTO_ALGO_NONE &&
		(km_pvt_key->key_algo != key_info->algo)) {
		if (WLC_KEY_IN_HW(key_info)) {
			--(km->stats.num_hw_keys);
			hw_idx = wlc_key_get_hw_idx(km_pvt_key->key);
			km_hw_key_destroy(km->hw, &hw_idx, key_info);
			KM_DBG_ASSERT(hw_idx == WLC_KEY_INDEX_INVALID);
			km_key_set_hw_idx(km_pvt_key->key, WLC_KEY_INDEX_INVALID, FALSE);
			wlc_key_get_info(km_pvt_key->key, key_info);
		}
	}

	/* note: none -> not none handled below by creation */
	if (km_needs_hw_key(km, km_pvt_key, key_info)) {
		km_hw_key_create(km->hw, key, key_info, &hw_idx);
		if (hw_idx != WLC_KEY_INDEX_INVALID) {
			km_key_set_hw_idx(key, hw_idx, km_hw_key_hw_mic(km->hw, hw_idx, key_info));
			wlc_key_get_info(key, key_info);
			++(km->stats.num_hw_keys);
		}
	} else {
		hw_idx = wlc_key_get_hw_idx(key);
	}

	if (hw_idx != WLC_KEY_INDEX_INVALID)
		km_hw_key_update(km->hw, hw_idx, key, key_info);

	/* update wep key count for default bss */
	if (WLC_KEY_IS_DEFAULT_BSS(key_info)) {
		if (KM_WEP_ALGO(key_info->algo) && !KM_WEP_ALGO(km_pvt_key->key_algo))
			++(km->stats.num_def_bss_wep);
		else if (!KM_WEP_ALGO(key_info->algo) && KM_WEP_ALGO(km_pvt_key->key_algo))
			--(km->stats.num_def_bss_wep);
	}

	/* compatibility: sync up scb wsec invalidate txc */
	if (km_pvt_key->flags & KM_FLAG_SCB_KEY) {
		scb_t *scb;
		uint bandunit;

		scb = km_pvt_key->u.scb;
		bsscfg = SCB_BSSCFG(scb);
		bandunit = CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec);

		/* update key scb if necessary */
		if (scb->bandunit != bandunit) {
			scb_t *other_scb;
			other_scb = wlc_scbfindband(km->wlc, bsscfg, &scb->ea, bandunit);
			if (other_scb != NULL) {
				km_pvt_key->u.scb = other_scb;
				scb = other_scb;
			}
		}

		if (km_pvt_key->key_algo != key_info->algo && !WLC_KEY_SMS4_PREV_KEY(key_info))
			km_sync_scb_wsec(km, scb, key_info->algo);
		wlc_txc_inv(km->wlc->txc, scb);

		if (RATELINKMEM_ENAB(km->wlc->pub)) {
			wlc_ratelinkmem_update_link_entry(km->wlc, scb);
		}

#if defined(PKTC) || defined(PKTC_DONGLE)
		/* for h/w keys, if keys are installed after the scb is authorized - e.g.
		 * m4 callback, enable pktc.
		 */
		if (SCB_AUTHORIZED(scb)) {
			wlc_scb_pktc_enable(scb, key_info);
		}
#endif /* PKTC || PKTC_DONGLE */

#ifdef WLCFP
		/* enable CFP for the scb now: */
		if (SCB_AUTHORIZED(scb)) {
			wlc_cfp_scb_state_upd(km->wlc->cfp, scb);
		}
#endif // endif

	} else {
		bsscfg = km_pvt_key->u.bsscfg;
		if (RATELINKMEM_ENAB(km->wlc->pub) && bsscfg) {
			/* update scb linkmem entry */
			scb_t *scb = wlc_scbfind(km->wlc, bsscfg, &bsscfg->BSSID);
			if (scb) {
				wlc_ratelinkmem_update_link_entry(km->wlc, scb);
			}
			/* update bsscfg scb linkmem entry */
			wlc_ratelinkmem_update_link_entry(km->wlc,
				WLC_RLM_BSSCFG_LINK_SCB(km->wlc, bsscfg));
		}
	}

	km_pvt_key->key_algo = key_info->algo;

#ifdef STA
	/* compatibility: for bss keys update ps state */
	if (BSSCFG_STA(bsscfg) &&
		((bsscfg->flags & WLC_BSSCFG_NOBCMC) || (km_pvt_key->flags & KM_FLAG_BSS_KEY)) &&
		(key_info->algo != CRYPTO_ALGO_OFF)) {
		bool ps_allowed;
		ps_allowed = PS_ALLOWED(bsscfg);
		bsscfg->wsec_portopen = TRUE;
		if ((key_info->algo & SEC_ALGO_KEY_EXCH) && (km->wlc->cfg == bsscfg)) {
#ifdef WL_PWRSTATS
			if (PWRSTATS_ENAB(km->wlc->pub)) {
				if (bsscfg->assoc->state == AS_IDLE)
					wlc_connect_time_upd(km->wlc);
			}
#endif /* WL_PWRSTATS */
#ifdef WLWNM
			/* Check DMS req for primary infra STA */
			if (WLWNM_ENAB(km->wlc->pub))
				wlc_wnm_check_dms_req(km->wlc, &bsscfg->current_bss->BSSID);
#endif /* WLWNM */
		}
		wlc_btc_set_ps_protection(km->wlc, bsscfg); /* enable */
		if (!ps_allowed && PS_ALLOWED(bsscfg))
			wlc_set_pmstate(bsscfg, bsscfg->pm->PMenabled);
	}
#endif /* STA */

#if defined(WLWNM_AP) && defined(MFP)
	if (WLWNM_ENAB(km->wlc->pub) && WLC_KEY_IS_AP(key_info) && !WLC_KEY_IS_PAIRWISE(key_info))
		wlc_wnm_sleep_key_update(km->wlc, bsscfg);
#endif /* WLWNM_AP && MFP */

	/* done creating/updating, signal the corresponding event */
	if (km_pvt_key->flags & KM_FLAG_CREATED_KEY) {
		event = WLC_KEYMGMT_EVENT_KEY_UPDATED;
	} else {
		event = WLC_KEYMGMT_EVENT_KEY_CREATED;
		km_pvt_key->flags |= KM_FLAG_CREATED_KEY;
	}

#ifdef BRCMAPIVTW
	/* key changed, reset any ivtw info */
	(void)km_ivtw_reset(km->ivtw, key_info->key_idx);
#endif /* BRCMAPIVTW */

	km_event_signal(km, event, NULL, key, NULL);
	KM_TRACE(("%s exit\n", __FUNCTION__));
}

static void
km_handle_wsec_change(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	wlc_key_index_t key_idx;
	uint32 prev_wsec;

	prev_wsec = km_bsscfg_get_wsec(km, bsscfg);
	if (prev_wsec == bsscfg->wsec)
			return;

	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key = &km->keys[key_idx];
		wlc_bsscfg_t *key_bsscfg;

		if (!KM_VALID_KEY(km_pvt_key))
			continue;

		key_bsscfg = (km_pvt_key->flags & KM_FLAG_BSS_KEY) ?
			km_pvt_key->u.bsscfg : SCB_BSSCFG(km_pvt_key->u.scb);

		if ((key_bsscfg != bsscfg) && !BSSCFG_PSTA(key_bsscfg))
			continue;

		/* check swkeys config */
		if ((prev_wsec & WSEC_SWFLAG) != (bsscfg->wsec & WSEC_SWFLAG)) {
			wlc_key_hw_index_t hw_idx;
			wlc_key_info_t key_info;

			wlc_key_get_info(km_pvt_key->key, &key_info);
			if (WLC_KEY_IN_HW(&key_info)) {
				--(km->stats.num_hw_keys);
				hw_idx = wlc_key_get_hw_idx(km_pvt_key->key);
				km_hw_key_destroy(km->hw, &hw_idx, &key_info);
				KM_DBG_ASSERT(hw_idx == WLC_KEY_INDEX_INVALID);
				km_key_set_hw_idx(km_pvt_key->key, WLC_KEY_INDEX_INVALID, FALSE);
			}

			km_pvt_key->flags &= ~KM_FLAG_SWONLY_KEY;
			km_pvt_key->flags |= (bsscfg->wsec & WSEC_SWFLAG) ? KM_FLAG_SWONLY_KEY : 0;

			wlc_key_get_info(km_pvt_key->key, &key_info);
			(void)km_ensure_hw_key(km, &key_info);
		}

		if (km_wsec_allows_algo(km, bsscfg->wsec, km_pvt_key->key_algo))
			continue;

		/* clear keys whose algo is disallowed by wsec */
		wlc_key_set_data(km_pvt_key->key, km_pvt_key->key_algo, NULL, 0);
	}

	km_bsscfg_update(km, bsscfg, KM_BSSCFG_WSEC_CHANGE);
}

static void
km_handle_wlc_up(keymgmt_t *km)
{
	wlc_key_info_t key_info;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;

	if (!(km->flags & KM_FLAG_WLC_DOWN))
		goto done;

	/* reset h/w, re-create h/w keys and update h/w with key info from s/w */
	km->flags &= ~(KM_FLAG_WLC_DOWN | KM_FLAG_WOWL_DOWN);
	km_hw_reset(km->hw);
	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;
		wlc_key_get_info(km_pvt_key->key, &key_info);
		(void)km_ensure_hw_key(km, &key_info);
	}
done:;
}

static void
km_handle_wlc_down(keymgmt_t *km)
{
	wlc_key_info_t key_info;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;
	wlc_key_hw_index_t hw_idx;

	if (km->flags & KM_FLAG_WLC_DOWN)
		goto done;

#ifdef WOWL
	if (wlc_bmac_get_noreset(KM_HW(km))) { /* wowl bypasss */
		km->flags |= (KM_FLAG_WLC_DOWN | KM_FLAG_WOWL_DOWN);
		goto done;
	}
#endif // endif

	/* remove all h/w keys and reset h/w */
	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;

		wlc_key_get_info(km_pvt_key->key, &key_info);
		if (WLC_KEY_IN_HW(&key_info)) {
			--(km->stats.num_hw_keys);
			hw_idx = wlc_key_get_hw_idx(km_pvt_key->key);
			km_hw_key_destroy(km->hw, &hw_idx, &key_info);
			KM_DBG_ASSERT(hw_idx == WLC_KEY_INDEX_INVALID);
			km_key_set_hw_idx(km_pvt_key->key, WLC_KEY_INDEX_INVALID, FALSE);
		}
	}

	km_hw_reset(km->hw);
	km->stats.num_bss_up = 0;
	km->flags |= KM_FLAG_WLC_DOWN;
done:;
}

static void
km_handle_scb_bsscfg_change(keymgmt_t *km, wlc_bsscfg_t *bsscfg, scb_t *scb)
{
	if (/* old */ bsscfg != NULL)
		km_bsscfg_reset(km, bsscfg, FALSE);

	if (scb != NULL) {
		if (!SCB_AUTHORIZED(scb))
			km_scb_reset(km, scb);
		bsscfg = SCB_BSSCFG(scb); /* new */
	} else {
		bsscfg = NULL;
	}

	/* for wpa none (ibss) use the key id 0 as key for scb */
	do {
		wlc_key_t *bss_key;
		wlc_key_info_t bss_key_info;
		wlc_key_t *scb_key;
		uint8 data[KM_KEY_MAX_DATA_LEN];
		size_t data_len;
		int err;

		if (bsscfg == NULL || bsscfg->BSS ||
			bsscfg->WPA_auth != WPA_AUTH_NONE ||
			BSS_TDLS_ENAB(km->wlc, bsscfg)) {
			break;
		}

		SCB_SET_IBSS_PEER(scb);
		bss_key = wlc_keymgmt_get_bss_key(km, bsscfg,
			WLC_KEY_ID_PAIRWISE, &bss_key_info);
		if (bss_key_info.algo  == CRYPTO_ALGO_NONE)
			break;
		err = wlc_key_get_data(bss_key, data, sizeof(data), &data_len);
		if (err != BCME_OK)
			break;

		scb_key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, NULL);
		err = wlc_key_set_data(scb_key, bss_key_info.algo, data, data_len);
		if (err != BCME_OK)
			KM_LOG(("wl%d.%d: %s: error %d updating key data for scb " MACF,
				KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
				err, ETHERP_TO_MACF(&scb->ea)));
	} while (0);
}

#ifdef WOWL
static void
km_handle_wowl_change(keymgmt_t *km, scb_t *scb)
{
	wlc_key_index_t key_idx;
	wlc_key_info_t key_info;
	km_pvt_key_t *km_pvt_key;

	/* handle down on wowl clear. h/w is already reset */
	if (!WOWL_ACTIVE(KM_PUB(km))) {
		if (!(km->flags & KM_FLAG_WOWL_DOWN))
			goto done;
		km_hw_reset(km->hw);
		for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
			km_pvt_key = &km->keys[key_idx];
			if (!KM_VALID_KEY(km_pvt_key))
				continue;
			wlc_key_get_info(km_pvt_key->key, &key_info);
			if (WLC_KEY_IN_HW(&key_info)) {
				--(km->stats.num_hw_keys);
				km_key_set_hw_idx(km_pvt_key->key,
					WLC_KEY_INDEX_INVALID, FALSE);
			}
		}
	}

	km_wowl_hw_set_mode(km->wowl_hw, scb, WOWL_ACTIVE(KM_PUB(km)));

done:;
}
#endif /* WOWL */

static void
km_handle_bss_up(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	wlc_key_id_t key_id;
	bool wowl_down;
	wlc_key_info_t key_info;
	wlc_key_t *key;

	KM_DBG_ASSERT(bsscfg != NULL);
	wowl_down = km_bsscfg_wowl_down(km, bsscfg);

	BCM_REFERENCE(key_id);
	BCM_REFERENCE(key);

	if (!(ASSOC_RECREATE_ENAB(KM_PUB(km)) &&
		((bsscfg->flags & WLC_BSSCFG_PRESERVE) ||
		bsscfg->flags & WLC_BSSCFG_RSDB_CLONE)) && !wowl_down) {
				km_bsscfg_reset(km, bsscfg, FALSE);
	}
#ifdef WOWL
	else {
		for (key_id = 0; key_id < WLC_KEYMGMT_NUM_GROUP_KEYS; ++key_id) {
			key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
			if (key_info.key_idx != WLC_KEY_INDEX_INVALID)
				(void)km_ensure_hw_key(km, &key_info);
		}
	}

	/* clear wowl state regarless of wowl down for better recovery */
	km_bsscfg_update(km, bsscfg, KM_BSSCFG_WOWL_UP);
#endif /* WOWL */

	BCM_REFERENCE(key_info);
	++(km->stats.num_bss_up);
}

static void
km_handle_bss_down(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	KM_DBG_ASSERT(bsscfg != NULL);
	if (!WOWL_ACTIVE(KM_PUB(km)) && !(km->flags & KM_FLAG_WOWL_DOWN) &&
		!(ASSOC_RECREATE_ENAB(KM_PUB(km)) &&
		(bsscfg->flags & WLC_BSSCFG_PRESERVE))) {
		km_bsscfg_reset(km, bsscfg, FALSE);
	} else if (WOWL_ACTIVE(KM_PUB(km))) {
		km_bsscfg_update(km, bsscfg, KM_BSSCFG_WOWL_DOWN);
	}
	--(km->stats.num_bss_up);
}

/* km internal interface */
void
km_notify(keymgmt_t *km, wlc_keymgmt_notif_t notif,
	wlc_bsscfg_t *bsscfg, scb_t *scb, wlc_key_t *key,
	void *pkt)
{
	wlc_key_info_t key_info;
	wlc_key_index_t key_idx;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	if (!KM_VALID(km))
		return;

	/* if keymgmt is shutting down, ignore notify requests */
	if (km->flags & KM_FLAG_DETACHING)
		return;

	wlc_key_get_info(key, &key_info);
	switch (notif) {
	case WLC_KEYMGMT_NOTIF_WLC_UP:
		km_handle_wlc_up(km);
		break;
	case WLC_KEYMGMT_NOTIF_WLC_DOWN:
		km_handle_wlc_down(km);
		break;
	case WLC_KEYMGMT_NOTIF_BSS_WSEC_CHANGED:
		KM_DBG_ASSERT(bsscfg != NULL);
		km_handle_wsec_change(km, bsscfg);
		break;
	case WLC_KEYMGMT_NOTIF_BSS_UP:
		km_handle_bss_up(km, bsscfg);
		break;
	case WLC_KEYMGMT_NOTIF_BSS_DOWN:
		km_handle_bss_down(km, bsscfg);
		break;
	case WLC_KEYMGMT_NOTIF_KEY_DELETE:
		km_handle_key_delete(km, key, &key_info);
		break;
	case WLC_KEYMGMT_NOTIF_KEY_UPDATE:
		km_handle_key_update(km, key, &key_info);
		break;
#ifdef STA
	case WLC_KEYMGMT_NOTIF_M1_RX: /* fall through */
	case WLC_KEYMGMT_NOTIF_M4_TX:
		km_b4m4_notify(km, notif, bsscfg, scb, key, pkt);
		break;
#endif /* STA */
	case WLC_KEYMGMT_NOTIF_DECODE_ERROR:
		if (key_info.algo == CRYPTO_ALGO_OFF) {
			KM_DBG_ASSERT(pkt != NULL);
			km_null_key_deauth(km, WLPKTTAGSCBGET(pkt), pkt);
		}
		km_event_signal(km, WLC_KEYMGMT_EVENT_DECODE_ERROR, NULL, key, pkt);
		break;
	case WLC_KEYMGMT_NOTIF_DECRYPT_ERROR:
		km_event_signal(km, WLC_KEYMGMT_EVENT_DECRYPT_ERROR, NULL, key, pkt);
		break;
	case WLC_KEYMGMT_NOTIF_MSDU_MIC_ERROR:
		key_idx = key_info.key_idx;
		KM_DBG_ASSERT((key_idx != WLC_KEY_INDEX_INVALID) &&
			KM_VALID_KEY_IDX(km, key_idx));

		if (key_info.algo == CRYPTO_ALGO_TKIP)
			km_tkip_mic_error(km, pkt, key, &key_info);

		km_event_signal(km, WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR, NULL, key, pkt);
		break;

	case WLC_KEYMGMT_NOTIF_TKIP_CM_REPORTED:
		KM_DBG_ASSERT(bsscfg != NULL && scb != NULL);
		km_tkip_cm_reported(km, bsscfg, scb);
		break;

#ifdef WOWL
	case WLC_KEYMGMT_NOTIF_WOWL_MICERR:
		key_idx = key_info.key_idx;
		KM_DBG_ASSERT((key_idx != WLC_KEY_INDEX_INVALID) &&
			KM_VALID_KEY_IDX(km, key_idx));

		if (key_info.algo != CRYPTO_ALGO_TKIP)
			break;

		km_tkip_mic_error(km, pkt, key, &key_info);
		km_event_signal(km, WLC_KEYMGMT_EVENT_MSDU_MIC_ERROR, NULL, key, pkt);
		break;

	case WLC_KEYMGMT_NOTIF_WOWL:
		km_handle_wowl_change(km, scb);
		break;
#endif /* WOWL */

	case WLC_KEYMGMT_NOTIF_SCB_BSSCFG_CHANGED:
		km_handle_scb_bsscfg_change(km, bsscfg, scb);
		break;

	case WLC_KEYMGMT_NOTIF_BSSID_UPDATE:
		km_bsscfg_update(km, bsscfg, KM_BSSCFG_BSSID_CHANGE);
		break;
	case WLC_KEYMGMT_NOTIF_NEED_PKTFETCH:
		++(km->stats.num_pkt_fetch);
		break;
	/* not handled here
	 * case WLC_KEYMGMT_NOTIF_SCB_CREATE:
	 * case WLC_KEYMGMT_NOTIF_SCB_DESTROY:
	 * case WLC_KEYMGMT_NOTIF_BSS_CREATE:
	 * case WLC_KEYMGMT_NOTIF_BSS_DESTROY:
	 * case WLC_KEYMGMT_NOTIF_NONE:
	 */
	default:
		break;
	}

	BCM_REFERENCE(key_idx);

	KM_LOG(("wl%d.%d: %s: notification %d - %s for key idx 0x%04x addr %s\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
		__FUNCTION__, notif, wlc_keymgmt_notif_name(notif), key_info.key_idx,
		bcm_ether_ntoa(&key_info.addr, eabuf)));
}

#ifdef BCMULP
int wlc_keymgmt_preattach()
{
	return km_ulp_preattach();
}
#endif /* BCMULP */

/* public interface */
void
wlc_keymgmt_notify(keymgmt_t *km, wlc_keymgmt_notif_t notif,
	wlc_bsscfg_t *bsscfg, scb_t *scb, wlc_key_t *key,
	void *pkt)
{
	STATIC_ASSERT(WLC_KEYMGMT_NOTIF_LAST <= (sizeof(km_notif_mask_t) * NBBY));
	KM_DBG_ASSERT(KM_VALID(km));

	if (KM_NOTIIF_IS_INTERNAL(notif)) {
		KM_LOG(("wl%d.%d: %s: external notification %d - %s not allowed.\n",
			KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg),
			__FUNCTION__, notif, wlc_keymgmt_notif_name(notif)));
	} else {
		km_notify(km, notif, bsscfg, scb, key, pkt);
	}
}
