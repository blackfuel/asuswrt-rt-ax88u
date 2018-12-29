/*
 * Key Management Module Implementation - utilities
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
 * $Id: km_util.c 767108 2018-08-28 12:23:52Z $
 */

#include "km_pvt.h"

#include <nan.h>

/* 802.1X LLC header, * DSAP/SSAP/CTL = AA:AA:03
 * OUI = 00:00:00
 * Ethertype = 0x888e (802.1X Port Access Entity)
 */
#define KM_DOT1X_SNAP(pbody) \
	(bcmp(wlc_802_1x_hdr, (pbody), DOT11_LLC_SNAP_HDR_LEN) == 0)

static bool km_nan_allow_unencrypted(keymgmt_t *km, const wlc_key_info_t *key_info,
	scb_t *scb, const struct dot11_header *hdr, uint8 *body, int body_len);

scb_t*
km_find_scb(keymgmt_t *km, wlc_bsscfg_t *bsscfg, const struct ether_addr *addr,
	bool create)
{
	scb_t *scb = NULL;
	uint bandunit;
	uint other_bandunit;
	wlc_info_t *wlc;

	wlc = km->wlc;

	if (addr == NULL || KM_ADDR_IS_BCMC(addr))
		goto done;

	bandunit = (bsscfg->associated) ?
		CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec) :
		wlc->band->bandunit;

	scb = wlc_scbfindband(wlc, bsscfg, addr, bandunit);
	if ((scb != NULL) || (KM_NBANDS(km) < 2))
		goto done;

	/* look in other band */
	other_bandunit = (bandunit == BAND_2G_INDEX) ? BAND_5G_INDEX : BAND_2G_INDEX;
	scb = wlc_scbfindband(wlc, bsscfg, addr, other_bandunit);
	if ((scb != NULL) || !create)
		goto done;

	/* need to create it */
	scb = wlc_scblookupband(wlc, bsscfg, addr, bandunit);

done:
	if (create && (scb == NULL))
		KM_ERR(("wl%d.%d: %s: out of scbs\n", KM_UNIT(km),
			WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
	return scb;
}

wlc_key_t*
km_find_key(keymgmt_t *km, wlc_bsscfg_t *bsscfg, const struct ether_addr *addr,
	wlc_key_id_t key_id, wlc_key_flags_t key_flags, wlc_key_info_t *key_info)
{
	wlc_key_t *key;
	scb_t *scb;
	wlc_key_info_t tmp_ki;

	KM_DBG_ASSERT(KM_VALID(km));

	if (!key_info)
		key_info = &tmp_ki;

	if (KM_ADDR_IS_BCMC(addr)) {
		key = wlc_keymgmt_get_bss_key(km, (BSSCFG_PSTA(bsscfg) ?
			KM_DEFAULT_BSSCFG(km) : bsscfg), key_id, key_info);
		goto done;
	}

	scb = km_find_scb(km, bsscfg, addr, TRUE);
	if (scb == NULL) {
		key = km->null_key;
		wlc_key_get_info(key, key_info);
		goto done;
	}

	key = wlc_keymgmt_get_scb_key(km, scb, key_id, key_flags, key_info);

	/* if scb key index has not been allocated, allocate it now */
	if (!KM_VALID_KEY_IDX(km, key_info->key_idx)) {
		km_scb_reset(km, scb);
		key = wlc_keymgmt_get_scb_key(km, scb, key_id, key_flags, key_info);
	}

done:
	KM_DBG_ASSERT(key != NULL);
	return key;
}

void
km_sync_scb_wsec(keymgmt_t *km, scb_t *scb, wlc_key_algo_t key_algo)
{
	KM_DBG_ASSERT(KM_VALID(km));
	switch (key_algo) {
		case CRYPTO_ALGO_WEP1:
		case CRYPTO_ALGO_WEP128:
			scb->wsec = WEP_ENABLED;
			break;

		case CRYPTO_ALGO_TKIP:
			scb->wsec = TKIP_ENABLED;
			break;

		/* Enable AES for both CCM and GCM modes w/ 128 or 256 bit keys */
		case CRYPTO_ALGO_AES_CCM:
		case CRYPTO_ALGO_AES_OCB_MSDU:
		case CRYPTO_ALGO_AES_OCB_MPDU:
		case CRYPTO_ALGO_AES_CCM256:
		case CRYPTO_ALGO_AES_GCM:
		case CRYPTO_ALGO_AES_GCM256:
			scb->wsec = AES_ENABLED;
			break;

		/* not valid, ignore
		 * case CRYPTO_ALGO_BIP:
		 * case CRYPTO_ALGO_BIP_CMAC256:
		 * case CRYPTO_ALGO_BIP_GMAC:
		 * case CRYPTO_ALGO_BIP_GMAC256:
		 * case CRYPTO_ALGO_PMK:
		 * case CRYPTO_ALGO_NALG:
		 */
		default:
			scb->wsec = 0;
	}
}

wlc_info_t*
wlc_keymgmt_get_wlc(keymgmt_t * km)
{
	return KM_VALID(km) ? km->wlc : NULL;
}

wlc_bsscfg_t*
wlc_keymgmt_get_bsscfg(keymgmt_t *km, wlc_key_index_t key_idx)
{
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(KM_VALID(km));

	if (!KM_VALID_KEY_IDX(km, key_idx))
		return NULL;

	km_pvt_key = &km->keys[key_idx];
	if (!KM_VALID_KEY(km_pvt_key))
		return NULL;

	return (km_pvt_key->flags & KM_FLAG_SCB_KEY) ?
		SCB_BSSCFG(km_pvt_key->u.scb) : km_pvt_key->u.bsscfg;
}

scb_t*
wlc_keymgmt_get_scb(keymgmt_t *km, wlc_key_index_t key_idx)
{
	km_pvt_key_t *km_pvt_key;

	KM_DBG_ASSERT(KM_VALID(km));

	if (!KM_VALID_KEY_IDX(km, key_idx))
		return NULL;

	km_pvt_key = &km->keys[key_idx];
	if (!KM_VALID_KEY(km_pvt_key))
		return NULL;

	return (km_pvt_key->flags & KM_FLAG_SCB_KEY) ?
		km_pvt_key->u.scb : NULL;
}

wlc_key_t*
wlc_keymgmt_get_key_by_addr(keymgmt_t *km,
	wlc_bsscfg_t *bsscfg, const struct ether_addr *addr,
	wlc_key_flags_t key_flags, wlc_key_info_t *key_info)
{
	wlc_key_id_t key_id;
	wlc_key_t *key;

	KM_DBG_ASSERT(KM_VALID(km));

	key = km->null_key;
	if (bsscfg == NULL || addr == NULL)
		goto done;

	if (KM_ADDR_IS_BCMC(addr)) {
		key_id = wlc_keymgmt_get_bss_tx_key_id(km, bsscfg,
			(key_flags & WLC_KEY_FLAG_MGMT_GROUP));
	} else {
		key_id = WLC_KEY_ID_PAIRWISE;
	}

	if (key_id != WLC_KEY_ID_INVALID)
		key = km_find_key(km, bsscfg, addr, key_id, key_flags, NULL);

done:
	KM_DBG_ASSERT(key != NULL);
	wlc_key_get_info(key, key_info);
	return key;
}

wlc_key_index_t
wlc_keymgmt_get_key_index(keymgmt_t *km, wlc_key_t *key)
{
	wlc_key_info_t key_info;

	KM_DBG_ASSERT(KM_VALID(km));

	wlc_key_get_info(key, &key_info);
	KM_DBG_ASSERT(key_info.key_idx == WLC_KEY_INDEX_INVALID ||
		KM_VALID_KEY_IDX(km, key_info.key_idx));
	return key_info.key_idx;
}

wlc_key_t*
wlc_keymgmt_get_key(keymgmt_t *km, wlc_key_index_t key_idx,
	wlc_key_info_t *key_info)
{
	km_pvt_key_t *km_pvt_key;
	wlc_key_t *key;

	KM_DBG_ASSERT(KM_VALID(km));

	key = km->null_key;
	if (KM_VALID_KEY_IDX(km, key_idx)) {
		km_pvt_key = &km->keys[key_idx];
		if (KM_VALID_KEY(km_pvt_key)) {
			key = km_pvt_key->key;
		}
	}

	wlc_key_get_info(key, key_info);
	return key;
}

bool
km_allow_unencrypted(keymgmt_t *km, const wlc_key_info_t *key_info,
	scb_t *scb, const struct dot11_header *hdr, uint16 qc,
	uint8 *body, int body_len, void *pkt)
{
	bool allow = FALSE;
	wlc_bsscfg_t *bsscfg;
	uint16 seq;
	bool frag0;
	uint16 fc;
	uint16 fk;
	uint32 wpa_auth;
	BCM_REFERENCE(km);

	KM_DBG_ASSERT(key_info != NULL && scb != NULL &&
		hdr != NULL && body != NULL);

	bsscfg = SCB_BSSCFG(scb);

	do {
		if (!bsscfg->wsec || !bsscfg->wsec_restrict) {
			allow = TRUE;
			break;
		}

		if (BSSCFG_NAN_DATA(bsscfg)) {
			allow = km_nan_allow_unencrypted(km, key_info, scb, hdr, body, body_len);
			break;
		}

		seq = ltoh16(hdr->seq);
		frag0 = ((seq & FRAGNUM_MASK) == 0);
		seq &= ~FRAGNUM_MASK;

		fc = ltoh16(hdr->fc);
		fk = (fc & FC_KIND_MASK);
		BCM_REFERENCE(fk);
		if ((fc & (FC_TODS|FC_FROMDS)) == (FC_TODS|FC_FROMDS)) /* wds ? */
			wpa_auth = bsscfg->WPA_auth;
		else
			wpa_auth = scb->WPA_auth;

		if (FC_TYPE(fc) == FC_TYPE_MNG) {
#ifdef MFP
			/* deauth and disassoc need protection when MFP is enabled.
			 * action frames are allowed, but may be filtered out  by mfp
			 * processing for robust category checking
			 */
			if (KM_SCB_MFP(scb) && KM_BSSCFG_ASSOCIATED(bsscfg)) {
				if (fk == FC_DEAUTH || fk == FC_DISASSOC)
					break;
			}
#endif // endif
			allow = TRUE;
			break;
		}

		if (FC_TYPE(fc) != FC_TYPE_DATA) {
			KM_DBG_ASSERT(FC_TYPE(fc) == FC_TYPE_DATA);
			break;
		}

		/* accept 802.1x, but only if WPA is disabled, or if WPA is enabled
		 * and we don't have a key (with a valid algo)
		 */
		if (frag0) {
			uint8 *pbody = (uint8 *)body;
			if (body_len < DOT11_LLC_SNAP_HDR_LEN)
				break;

			/* if AMSDU frame, skip ETHER_HDR_LEN to get body */
			if (qc & QOS_AMSDU_MASK)
				pbody += ETHER_HDR_LEN;

			if (!KM_DOT1X_SNAP(pbody) &&
				TRUE) {
					break;
			}
		}
		else {
			uint16 scb_seq;
			scb_seq = scb->seqctl[QOS_PRIO(qc)] & ~FRAGNUM_MASK;
			if ((seq == scb_seq) &&
				(!(scb->flags & SCB_8021XHDR)) &&
				TRUE) {
				break;
			}
		}

		if ((wpa_auth != WPA_AUTH_DISABLED) &&
			(key_info->algo != CRYPTO_ALGO_OFF)) {

			if (KM_BSSCFG_IS_BSS(bsscfg))
				break;
		}

		allow = TRUE;
	} while (0);

	return allow;
}

void km_null_key_deauth(keymgmt_t *km, scb_t *scb, void *pkt)
{
#if defined(AP)
	wlc_bsscfg_t *bsscfg;
	struct dot11_header *hdr;
	wlc_info_t *wlc;

	/* scb can be null for unexpected encrypted frames */
	if (scb == NULL)
		return;

	wlc = km->wlc;
	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	hdr = (struct dot11_header *)PKTDATA(KM_OSH(km), pkt);

	/* 802.11i D5.0 8.4.10.1 Illegal data transfer */
	if (!ETHER_ISMULTI(&hdr->a1) && BSSCFG_AP(bsscfg) && SCB_WDS(scb) &&
		SCB_AUTHORIZED(scb) && (bsscfg->WPA_auth != WPA_AUTH_DISABLED)) {
		/* pairwise key is out of sync with peer, send deauth */
		if (!(scb->flags & SCB_DEAUTH)) {
			/* Use the cur_etheraddr of the BSSCFG that this WDS
			 * interface is tied to as our BSSID.  We can't use the
			 * BSSCFG's BSSID because the BSSCFG may not be "up" (yet).
			 */
			wlc_senddeauth(wlc, bsscfg, scb, &scb->ea,
				&bsscfg->cur_etheraddr, &bsscfg->cur_etheraddr,
				DOT11_RC_AUTH_INVAL);
			wlc_scb_clearstatebit(wlc, scb, AUTHORIZED);
			wlc_scb_disassoc_cleanup(wlc, scb);
			wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &scb->ea,
				DOT11_RC_AUTH_INVAL, 0);
			scb->flags |= SCB_DEAUTH;
		}
	}
#endif /* AP */
}

wlc_key_hw_index_t
km_get_hw_idx(keymgmt_t *km, wlc_key_index_t key_idx)
{
	wlc_key_hw_index_t hw_idx = WLC_KEY_INDEX_INVALID;

	KM_DBG_ASSERT(KM_VALID(km));
	if (KM_VALID_KEY_IDX(km, key_idx)) {
		km_pvt_key_t *km_pvt_key;
		km_pvt_key = &km->keys[key_idx];
		if (KM_VALID_KEY(km_pvt_key))
			hw_idx = wlc_key_get_hw_idx(km_pvt_key->key);
	}

	return hw_idx;
}

/* public interface */
int BCMFASTPATH
wlc_keymgmt_recvdata(keymgmt_t *km, wlc_frminfo_t *f)
{
	int err;
	wlc_bsscfg_t *bsscfg = NULL;
	scb_t *scb = NULL;
	wlc_key_info_t *key_info;
	wlc_key_t *key;
	wlc_key_id_t key_id;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_ASSERT(f != NULL && f->p != NULL);

	key_info = &f->key_info;

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* fast path lookup */
	{
		wlc_key_hw_index_t hw_idx;
		uint16 RxStatus1 = D11RXHDR_ACCESS_VAL(f->rxh,
			KM_COREREV(km), RxStatus1);

		hw_idx = KM_RXS_SECKINDX(km, (RxStatus1 & KM_RXS_SECKINDX_MASK(km))
						>> RXS_SECKINDX_SHIFT);
		if ((RxStatus1 & RXS_DECATMPT) &&
			(hw_idx == km->key_cache->hw_idx)) {
			key = km->key_cache->key;
			*key_info = *km->key_cache->key_info;
			goto have_key;
		}
	}
#endif /* PKTC || PKTC_DONGLE */

	scb = WLPKTTAGSCBGET(f->p);
	KM_ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	if (!f->rx_wep) {
		key_id = WLC_KEY_ID_PAIRWISE;
	} else {
		key_id =  KM_PKT_KEY_ID(f->pbody);
	}

	if (!f->rx_wep) {
		key = wlc_keymgmt_get_scb_key(km, scb, key_id, WLC_KEY_FLAG_NONE, key_info);
	} else if (!f->ismulti) { /* pairwise key if available bss key */
		key = wlc_keymgmt_get_scb_key(km, scb, key_id, WLC_KEY_FLAG_NONE, key_info);
		if (key_info->algo == CRYPTO_ALGO_OFF)
			key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, key_info);
	} else {
		{
			key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, key_info);
		}
	}

#if defined(PKTC) || defined(PKTC_DONGLE)
have_key:
#endif /* PKTC || PKTC_DONGLE */

	f->key = key;

	KM_LOG(("wl%d.%d: %s: key_lookup: "
		"key for %scast frame key id %d, key idx %d, algo %d\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
		(f->ismulti ? "multi" : "uni"), key_info->key_id,
		key_info->key_idx, key_info->algo));

	/* receive mpdu; if s/w decryption is required it will handled
	 * by wlc_key_rx_mpdu. Any required mic checks are deferred to sdu processing.
	 */
	err = wlc_key_rx_mpdu(key, f->p, f->rxh);
	if (err != BCME_OK) {
		KM_LOG(("wl%d.%d: %s: error %d on receive\n",
			KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, err));
		goto done;
	}

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* update key cache */
	if (!f->ismulti && (km->key_cache->key != key) &&
			!WLC_KEY_SMS4_PREV_KEY(key_info)) {
		km_key_update_key_cache(key, km->key_cache);
	}
#endif /* PKTC || PKTC_DONGLE */

	/* adjust frame body and length unless not wep or algo off */
	if (!f->rx_wep || (key_info->algo == CRYPTO_ALGO_OFF))
		goto done;

	/* rx stripped icv, update frame info */
	f->pbody += key_info->iv_len;
	f->body_len -= (key_info->iv_len);
	f->totlen -= (key_info->iv_len);
	if ((PKTISHDRCONVTD(KM_OSH(km), f->p)) && (f->isamsdu)) {
	} else {
		f->body_len -= key_info->icv_len;
		f->totlen -= key_info->icv_len;
	}

	f->len = PKTLEN(KM_OSH(km), f->p);

done:
	return err;
}

bool
km_needs_hw_key(keymgmt_t *km, km_pvt_key_t *km_pvt_key, wlc_key_info_t *key_info)
{
	bool needs = FALSE;

	KM_DBG_ASSERT(KM_VALID(km));

	do {
		if (km->flags & KM_FLAG_WLC_DOWN)
			break;

		if (!KM_VALID_KEY(km_pvt_key) || !key_info) /* invalid keys */
			break;

		if (WLC_KEY_IN_HW(key_info)) /* already has it */
			break;

		if (key_info->algo == CRYPTO_ALGO_NONE)	/* no real crypto */
			break;

		if (km_algo_is_swonly(km, key_info->algo)) /* h/w support disabled */
			break;

		if (key_info->hw_algo == WSEC_ALGO_OFF)	/* no h/w support */
			break;

		if (km_pvt_key->flags & KM_FLAG_SWONLY_KEY) /* s/w only key */
			break;

		if (WLC_KEY_IS_LINUX_CRYPTO(key_info)) /* external crypto */
			break;

		/* note: group keys on AP used only for tx, no rx is expected. */
		if (!WLC_KEY_RX_ALLOWED(key_info) && !WLC_KEY_TX_ALLOWED(key_info))
			break;

#ifdef MFP
		if (WLC_KEY_IS_MGMT_GROUP(key_info))
			break;
#endif // endif

		if (km_pvt_key->flags & KM_FLAG_BSS_KEY) {
			wlc_bsscfg_t *bsscfg = km_pvt_key->u.bsscfg;

			if (!KM_BSSCFG_UP(bsscfg))
				break;

			if (BSSCFG_PSTA(bsscfg)) /* non-primary psta group key */
				break;

			if (BSSCFG_STA(bsscfg)) {
				/* default bss tkip group keys - no space for phase1.
				 * non-default bss tkip group keys need ucode support
				 */
				if (key_info->algo == CRYPTO_ALGO_TKIP)
					break;
				if (!WLC_KEY_IS_DEFAULT_BSS(key_info)) {
					/* non sta group keys in non-default bss */
					if (!WLC_KEY_ID_IS_STA_GROUP(key_info->key_id))
						break;
				}
			}
			/* AP group keys need hw keys, for tx */
		}

		/* IBSS group keys are tx only - h/w support is needed */

		needs = TRUE;
	} while (0);

	KM_LOG(("wl%d: %s: key with key idx %d - needs hw key: %d\n",
		KM_UNIT(km), __FUNCTION__, (key_info != NULL ? key_info->key_idx :
		WLC_KEY_INDEX_INVALID), needs));

	return needs;
}

void
km_init_pvt_key(keymgmt_t *km, km_pvt_key_t *km_pvt_key, wlc_key_algo_t algo,
	wlc_key_t *key, km_flags_t flags, wlc_bsscfg_t *bsscfg, scb_t *scb)
{
	KM_DBG_ASSERT(key != NULL);

	/* mark the key as valid and allocated */
	flags |= KM_FLAG_VALID_KEY | KM_FLAG_IDX_ALLOC;

	km_pvt_key->key =  key;
	km_pvt_key->flags =  flags;
	km_pvt_key->key_algo = algo;

	if (flags & KM_FLAG_BSS_KEY) {
		KM_DBG_ASSERT(bsscfg != NULL);
		km_pvt_key->u.bsscfg = bsscfg;
	} else {
		KM_DBG_ASSERT(scb != NULL);
		km_pvt_key->u.scb = scb;
	}

	km_notify(km, WLC_KEYMGMT_NOTIF_KEY_UPDATE,
        NULL /* bsscfg */, NULL /* scb */, key, NULL /* pkt */);
}

#if defined(BCMDBG)
void
km_get_hw_idx_key_info(keymgmt_t *km, wlc_key_hw_index_t hw_idx,
    wlc_key_info_t *key_info)
{
	wlc_key_t *key;
	int i;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(key_info != NULL);

	key = km->null_key;
	for (i = 0; i < km->max_keys; ++i) {
		if (hw_idx == km_get_hw_idx(km, (wlc_key_index_t)i)) {
			key = km->keys[i].key;
			break;
		}
	}

	wlc_key_get_info(key, key_info);
}
#endif // endif

/* determine if default keys are valid for rx unicast */
bool
km_rxucdefkeys(keymgmt_t *km)
{
	bool defkeys = FALSE;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;

	KM_DBG_ASSERT(KM_VALID(km));

	wlc = km->wlc;

	BCM_REFERENCE(wlc);

	bsscfg = KM_DEFAULT_BSSCFG(km);
	KM_DBG_ASSERT(bsscfg != NULL);

	/* WPA must have uc keys except ibss/wpa none - def keys not valid for rx */
	if (bsscfg->WPA_auth != WPA_AUTH_DISABLED) {
		if (!bsscfg->BSS && (bsscfg->WPA_auth == WPA_AUTH_NONE))
			defkeys = TRUE;
		goto done;
	}

	if (AP_ENAB(wlc->pub) && bsscfg->eap_restrict)
		goto done;

	/* sw keys not in hw are present */
	if (km->stats.num_sw_keys != km->stats.num_hw_keys)
		goto done;

	/* multi SSID */
	if ((APSTA_ENAB(wlc->pub) || BSSCFG_AP(bsscfg)) && (km->stats.num_bss_up > 1))
		goto done;

	/* one or more default bss keys are wep */
	if (km->stats.num_def_bss_wep != 0)
		defkeys = TRUE;
done:
	KM_LOG(("wl%d: %s: defkeys %d\n", KM_UNIT(km), __FUNCTION__, defkeys));
	return defkeys;
}

/* Update AMT info DEFKEY to all the amt entries using default key
 */
void
km_amt_defkey_clearall(keymgmt_t *km)
{
	km_amt_idx_t amt_idx;
	uint16 val;

	/* update with clearing amt entries with default keys with attribute AMT_ATTR_DEFKEY */
	for (amt_idx = 0; km_hw_amt_idx_valid(km->hw, amt_idx); ++amt_idx) {
		val = wlc_read_amtinfo_by_idx(km->wlc, amt_idx);

		val &= ~NBITVAL(C_ADDR_DEFKEY_NBIT);
		wlc_write_amtinfo_by_idx(km->wlc, amt_idx, val);
	}
}

/* Update AMT info DEFKEY to all the amt entries using default key
 */
void
km_amt_defkey_update(keymgmt_t *km, km_amt_idx_t amt_idx)
{
	uint16 val = 0;
	bool mhf_defkeys = km_rxucdefkeys(km);

	/* update amt entry on whether default key is valid */
	if (km_hw_amt_idx_valid(km->hw, amt_idx)) {
		val = wlc_read_amtinfo_by_idx(km->wlc, amt_idx);
		if (mhf_defkeys) {
			val |= NBITVAL(C_ADDR_DEFKEY_NBIT);
		} else {
			val &= ~NBITVAL(C_ADDR_DEFKEY_NBIT);
		}
		wlc_write_amtinfo_by_idx(km->wlc, amt_idx, val);
	}
	KM_LOG(("wl%d: %s: amt_idx %d amt_info 0x%02x\n", KM_UNIT(km), __FUNCTION__,
		amt_idx, val));
}

bool
km_is_replay(keymgmt_t *km, wlc_key_info_t *key_info, int ins,
	uint8 *key_seq, uint8 *rx_seq, size_t seq_len)
{
	bool replay = FALSE;
	BCM_REFERENCE(ins);

	KM_DBG_ASSERT(KM_VALID(km) && key_info != NULL);

	if (WLC_KEY_CHECK_REPLAY(key_info)) {
#ifdef BRCMAPIVTW
		KM_DBG_ASSERT(km->ivtw != NULL);
		if (WLC_KEY_USE_IVTW(key_info))
			replay = km_ivtw_is_replay(km->ivtw, key_info,
				ins, key_seq, rx_seq, seq_len);
		else
#endif /* BRCMAPIVTW */
			replay = !km_key_seq_less(key_seq, rx_seq, seq_len);
	}

#ifdef GTK_RESET
	/* one shot deal to ignore replay error after gtk is reset */
	if (WLC_KEY_IS_GROUP(key_info) && (key_info->flags & WLC_KEY_FLAG_GTK_RESET)) {
		replay = FALSE;
		key_info->flags &= ~WLC_KEY_FLAG_GTK_RESET;
	}
#endif /* GTK_RESET */

	return replay;
}

#ifdef BRCMAPIVTW
void
km_update_ivtw(keymgmt_t *km, wlc_key_info_t *key_info, int ins,
	uint8 *rx_seq, size_t seq_len, bool chained)
{
	KM_DBG_ASSERT(KM_VALID(km) && key_info != NULL);
	KM_DBG_ASSERT(WLC_KEY_USE_IVTW(key_info));
	KM_DBG_ASSERT(km->ivtw != NULL);
	km_ivtw_update(km->ivtw, key_info, ins, rx_seq, seq_len, chained);
}
#endif /* BRCMAPIVTW */

size_t
km_get_max_keys(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return km->max_keys;
}

wlc_key_algo_t
wlc_keymgmt_hw_algo_to_algo(keymgmt_t *km, wlc_key_hw_algo_t algo)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return km_hw_hw_algo_to_algo(km->hw, algo);
}

wlc_key_hw_algo_t
wlc_keymgmt_algo_to_hw_algo(keymgmt_t *km, wlc_key_algo_t algo)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return km_hw_algo_to_hw_algo(km->hw, algo);
}

bool
km_algo_is_supported(wlc_keymgmt_t *km, wlc_key_algo_t algo)
{
	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(algo < KM_SIZE_BITS(km_algo_mask_t));
	return !(km->algo_unsup & (1 << algo));
}

bool
km_algo_is_swonly(wlc_keymgmt_t *km, wlc_key_algo_t algo)
{
	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(algo < KM_SIZE_BITS(km_algo_mask_t));
	return (km->algo_swonly & (1 << algo));
}

bool
km_wsec_allows_algo(keymgmt_t *km, uint32 wsec, wlc_key_algo_t algo)
{
	bool allows = FALSE;

	(void)km;
	switch (algo) {
		case CRYPTO_ALGO_OFF:
			allows = TRUE;
			break;

		case CRYPTO_ALGO_WEP1:
		case CRYPTO_ALGO_WEP128:
			if (wsec & WEP_ENABLED)
				allows = TRUE;
			break;

		case CRYPTO_ALGO_TKIP:
			if (wsec & TKIP_ENABLED)
				allows = TRUE;
			break;

		case CRYPTO_ALGO_AES_CCM:
		case CRYPTO_ALGO_AES_CCM256:
		case CRYPTO_ALGO_AES_OCB_MSDU:
		case CRYPTO_ALGO_AES_OCB_MPDU:
		case CRYPTO_ALGO_AES_GCM:
		case CRYPTO_ALGO_AES_GCM256:
#ifdef MFP
		case CRYPTO_ALGO_BIP:
		case CRYPTO_ALGO_BIP_CMAC256:
		case CRYPTO_ALGO_BIP_GMAC:
		case CRYPTO_ALGO_BIP_GMAC256:
#endif /* MFP */
			if (wsec & AES_ENABLED)
				allows = TRUE;
			break;

		/* not handled by wsec setting
		 * case CRYPTO_ALGO_PMK:
		 * case CRYPTO_ALGO_NALG:
		*/
		default:
			break;
	}

	return allows;
}

#if defined(BCMDBG) || defined(WLMSG_WSEC)

#define CASE(x) case WLC_KEYMGMT_NOTIF_##x: return #x
const char*
wlc_keymgmt_notif_name(wlc_keymgmt_notif_t notif)
{
	switch (notif) {
	CASE(NONE);
	CASE(WLC_UP);
	CASE(WLC_DOWN);
	CASE(BSS_UP);
	CASE(BSS_DOWN);
	CASE(BSS_CREATE);
	CASE(BSS_DESTROY);
	CASE(BSS_WSEC_CHANGED);
	CASE(SCB_CREATE);
	CASE(SCB_DESTROY);
	CASE(KEY_UPDATE);
	CASE(KEY_DELETE);
	CASE(M1_RX);
	CASE(M4_TX);
	CASE(WOWL);
	CASE(SCB_BSSCFG_CHANGED);
	CASE(DECODE_ERROR);
	CASE(DECRYPT_ERROR);
	CASE(MSDU_MIC_ERROR);
	CASE(TKIP_CM_REPORTED);
	CASE(OFFLOAD);
	CASE(BSSID_UPDATE);
	CASE(WOWL_MICERR);
	CASE(NEED_PKTFETCH);
	default:
		break;
	}
	return "unknown";
}
#undef CASE

const char*
wlc_keymgmt_get_algo_name(keymgmt_t *km, wlc_key_algo_t algo)
{
	const char *name = "unknown";

	(void)km;
	switch (algo) {
		case CRYPTO_ALGO_OFF: name = "off"; break;
		case CRYPTO_ALGO_WEP1: name = "wep1"; break;
		case CRYPTO_ALGO_WEP128: name = "wep128"; break;
		case CRYPTO_ALGO_TKIP: name = "tkip"; break;
		case CRYPTO_ALGO_AES_CCM: name = "aes-ccm"; break;
		case CRYPTO_ALGO_AES_CCM256: name = "aes-ccm-256"; break;
		case CRYPTO_ALGO_AES_GCM: name = "aes-gcm"; break;
		case CRYPTO_ALGO_AES_GCM256: name = "aes-gcm-256"; break;
		case CRYPTO_ALGO_AES_OCB_MSDU: name = "aes-ocb-msdu"; break;
		case CRYPTO_ALGO_AES_OCB_MPDU: name = "aes-ocb-mpdu"; break;
		case CRYPTO_ALGO_BIP: name = "bip"; break;
		case CRYPTO_ALGO_BIP_CMAC256: name = "bip-cmac-256"; break;
		case CRYPTO_ALGO_BIP_GMAC: name = "bip-gmac"; break;
		case CRYPTO_ALGO_BIP_GMAC256: name = "bip-gmac-256"; break;
		case CRYPTO_ALGO_PMK: name = "pmk"; break;
		case CRYPTO_ALGO_NALG: name = "nalg"; break;
		default:
			break;
	}
	return name;
}

const char*
wlc_keymgmt_get_hw_algo_name(keymgmt_t *km, wlc_key_hw_algo_t hw_algo, int mode)
{
	const char *name = "unknown";
	bool rev40plus;

	KM_DBG_ASSERT(KM_VALID(km));
	rev40plus = KM_COREREV_GE40(km);

	if (rev40plus) {
		switch (hw_algo) {
		case WSEC_ALGO_OFF: name = "off"; break;
		case WSEC_ALGO_WEP1: name = "wep1"; break;
		case WSEC_ALGO_TKIP: name = "tkip"; break;

		case WSEC_ALGO_WEP128: name = "wep128"; break;
		case WSEC_ALGO_AES_LEGACY: name = "aes-legacy"; break;
		case WSEC_ALGO_AES:
			switch (mode) {
			case AES_MODE_NONE: name = "aes-*"; break;
			case AES_MODE_CCM: name = "aes-ccm"; break;
			case AES_MODE_OCB_MSDU: name = "aes-ocb-msdu"; break;
			case AES_MODE_OCB_MPDU: name = "aes-ocb-mpdu"; break;
			case AES_MODE_CMAC: name = "aes-cmac"; break;
			case AES_MODE_GCM: name = "aes-gcm"; break;
			case AES_MODE_GMAC: name = "aes-gmac"; break;
			default: break;
			}
			break;

		case WSEC_ALGO_SMS4_DFT_2005_09_07: /* fall through */
		case WSEC_ALGO_SMS4: name = "sms4"; break;
		case WSEC_ALGO_NALG: name = "nalg"; break;
		default: break;
		}
	} else {
		switch (hw_algo) {
		case WSEC_ALGO_OFF: name = "off"; break;
		case WSEC_ALGO_WEP1: name = "wep1"; break;
		case WSEC_ALGO_TKIP: name = "tkip"; break;
		case D11_PRE40_WSEC_ALGO_AES:
			switch (mode) {
			case AES_MODE_NONE: name = "aes-*"; break;
			case AES_MODE_CCM: name = "aes-ccm"; break;
			case AES_MODE_OCB_MSDU: name = "aes-ocb-msdu"; break;
			case AES_MODE_OCB_MPDU: name = "aes-ocb-mpdu"; break;
			case AES_MODE_CMAC: name = "aes-cmac"; break;
			case AES_MODE_GCM: name = "aes-gcm"; break;
			case AES_MODE_GMAC: name = "aes-gmac"; break;
			default: break;
			}
			break;
		case D11_PRE40_WSEC_ALGO_WEP128: name = "wep128"; break;
		case D11_PRE40_WSEC_ALGO_AES_LEGACY: name = "aes-legacy"; break;
		case D11_PRE40_WSEC_ALGO_SMS4: name = "sms4"; break;
		case D11_PRE40_WSEC_ALGO_NALG: name = "nalg"; break;
		default: break;
		}
	}
	return name;
}
#endif // endif

#ifdef BCMDBG
static unsigned
__h2i(int c)
{
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= '0' && c <= '9')
		return c - '0';
	else
		return 0;
}

uint8
km_hex2int(uchar lo, uchar hi)
{
	return (uint8)(__h2i(hi) << 4 | __h2i(lo));
}
#endif /* BCMDBG */

#ifdef KM_SERIAL_SUPPORTED
int
km_serial_parse_hdr(keymgmt_t *km, int obj_type, int obj_ver,
	const uint8 *buf, size_t buf_len, int *num_tlvs)
{
	int err  = BCME_OK;
	const km_serial_t *ser = NULL;

	KM_DBG_ASSERT(KM_VALID(km));

	if (buf_len < OFFSETOF(km_serial_t, tlvs)) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	ser = (const km_serial_t *)buf;
	if (KM_SERIAL_VERSION(KM_SERIAL_HDR_VERSION, obj_ver) != ser->version) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	if (ser->obj_type != obj_type) {
		err = BCME_BADARG;
		goto done;
	}

	if (num_tlvs) {
		*num_tlvs = ltoh16_ua((const uint8 *)&ser->num_tlvs);
		KM_DBG_ASSERT(buf_len >= KM_SERIAL_MIN_SIZE(ser, *num_tlvs));
	}

done:
	return err;
}
#endif /* KM_SERIAL_SUPPORTED */

void
wlc_keymgmt_amt_reserve(wlc_keymgmt_t *km, uint8  amt_idx, size_t count, bool reserve)
{
	km_hw_amt_reserve(km->hw, amt_idx, count, reserve);
}

int
wlc_keymgmt_clone_bsscfg(wlc_keymgmt_t *from_km, wlc_keymgmt_t *to_km,
wlc_bsscfg_t *from_cfg, wlc_bsscfg_t *to_cfg)
{
	struct scb *from_scb = NULL, *to_scb = NULL;
	wlc_key_t *to_key, *from_key;
	wlc_key_info_t key_info, to_key_info;
	int err = BCME_OK;
	uint8 data[KM_KEY_MAX_DATA_LEN], i, id;
	size_t data_len;
	wlc_key_id_t key;
	if (!BSSCFG_AP(from_cfg)) {
		from_scb = wlc_scblookupband(from_cfg->wlc, from_cfg,
			&to_cfg->current_bss->BSSID,
			CHSPEC_WLCBANDUNIT(to_cfg->current_bss->chanspec));

		if (!from_scb) {
			KM_ERR(("wlc%d: from_scb is not found \n", WLCWLUNIT(from_cfg->wlc)));
			return BCME_NOTFOUND;
		}

		to_scb = wlc_scblookupband(to_cfg->wlc, to_cfg,
		&from_cfg->current_bss->BSSID, CHSPEC_WLCBANDUNIT(to_cfg->current_bss->chanspec));

		if (!to_scb) {
			KM_ERR(("wlc%d: to_scb is not found \n", WLCWLUNIT(to_cfg->wlc)));
			return BCME_NOTFOUND;
		}
	}
	/* Copy all bss keys */
	for (id = 0; id < WLC_KEYMGMT_NUM_GROUP_KEYS; id++) {
		from_key = wlc_keymgmt_get_bss_key(from_km, from_cfg, id,
			&key_info);

		err = wlc_key_get_data(from_key, data, sizeof(data), &data_len);
		if (err != BCME_OK)
			continue;

		to_key = wlc_keymgmt_get_bss_key(to_km, to_cfg, id, &to_key_info);
		/* Clone key data */
		wlc_key_set_data(to_key, key_info.algo, data, data_len);

		if (to_key_info.flags != key_info.flags) {
			bool do_notify = KM_KEY_FLAGS_NOTIFY_MASK(to_key_info.flags) !=
				KM_KEY_FLAGS_NOTIFY_MASK(key_info.flags);
			to_key_info.flags = key_info.flags;
			if (do_notify)
				km_notify(to_km, WLC_KEYMGMT_NOTIF_KEY_UPDATE,
					NULL /* bsscfg */, NULL /* scb */, to_key, NULL /* pkt */);
		}

		/* Clone tx seq number */
		data_len = sizeof(data);
		if (wlc_key_get_seq(from_key, data, data_len, 0, 1) >= 0)
			(void)wlc_key_set_seq(to_key, data, data_len, 0, 1);

		/* Clone rx seq number */
		for (i = 0; i < (size_t)WLC_KEY_NUM_RX_SEQ; i++) {
			data_len = sizeof(data);
			if (wlc_key_get_seq(from_key, data, data_len, i, 0) >= 0)
				(void)wlc_key_set_seq(to_key, data, data_len, i, 0);
		}

#if defined(BRCMAPIVTW)
		km_ivtw_clone(from_km->ivtw, to_km->ivtw,
		key_info.key_idx, to_key_info.key_idx);
#endif // endif
	}
	/* For AP, the SCB cloning is handled in scb update function.
	* STA Cfg does not enter the update functionality and therefore
	* still needs the below from_scb -> to_scb data copy.
	*/
	if (!BSSCFG_AP(from_cfg)) {
		/* Copy scb keys */
		for (id = 0; id < WLC_KEYMGMT_NUM_GROUP_KEYS; id++) {
			wlc_key_flags_t key_flags = WLC_KEY_FLAG_NONE;

			if (WLC_KEY_ID_IS_STA_GROUP(id)) {
				if (!from_cfg->BSS)
					key_flags = WLC_KEY_FLAG_IBSS_PEER_GROUP;
				else
					key_flags = WLC_KEY_FLAG_GROUP;
			}

			from_key = wlc_keymgmt_get_scb_key(from_km, from_scb, id,
				key_flags, &key_info);
			if (!from_key)
				continue;

			err = wlc_key_get_data(from_key, data, sizeof(data), &data_len);

			if (err != BCME_OK)
				continue;

			to_key = wlc_keymgmt_get_scb_key(to_km, to_scb, id,
				key_flags, &to_key_info);
			if (!to_key)
				continue;

			/* Clone key data */
			wlc_key_set_data(to_key, key_info.algo, data, data_len);
			/* Clone key flags */
			if (to_key_info.flags != key_info.flags) {
				bool do_notify = KM_KEY_FLAGS_NOTIFY_MASK(to_key_info.flags) !=
					KM_KEY_FLAGS_NOTIFY_MASK(key_info.flags);
				to_key_info.flags = key_info.flags;
				if (do_notify)
					km_notify(to_km, WLC_KEYMGMT_NOTIF_KEY_UPDATE,
						NULL /* bsscfg */, NULL /* scb */,
						to_key, NULL /* pkt */);
			}

			/* Clone tx seq number */
			data_len = sizeof(data);
			if (wlc_key_get_seq(from_key, data, data_len, 0, 1) >= 0)
				(void)wlc_key_set_seq(to_key, data, data_len, 0, 1);

			/* Clone rx seq number */
			for (i = 0; i < (size_t)WLC_KEY_NUM_RX_SEQ; i++) {
				data_len = sizeof(data);
				if (wlc_key_get_seq(from_key, data, data_len, i, 0) >= 0)
					(void)wlc_key_set_seq(to_key, data, data_len, i, 0);
			}

#if defined(BRCMAPIVTW)
			km_ivtw_clone(from_km->ivtw, to_km->ivtw,
			key_info.key_idx, to_key_info.key_idx);
#endif // endif
		}
	}
	key = wlc_keymgmt_get_bss_tx_key_id(from_km,
		from_cfg, FALSE);
	wlc_keymgmt_set_bss_tx_key_id(to_km, to_cfg,
		key, FALSE);

	return BCME_OK;
}

int
wlc_keymgmt_alloc_amt(wlc_keymgmt_t *km)
{
	km_amt_idx_t  amt_idx;

	if ((amt_idx = km_hw_amt_find_and_resrv(km->hw)) != KM_HW_AMT_IDX_INVALID)
		return amt_idx;

	return BCME_NORESOURCE;
}

void
wlc_keymgmt_free_amt(wlc_keymgmt_t *km, int *amt_idx)
{
	KM_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(amt_idx != NULL);

	km_hw_amt_release(km->hw, amt_idx);
}

static bool
km_nan_allow_unencrypted(keymgmt_t *km, const wlc_key_info_t *key_info,
	scb_t *scb, const struct dot11_header *hdr, uint8 *body, int body_len)
{
	bool allow = FALSE;
	uint16 fc = ltoh16(hdr->fc);
	uint16 fk = (fc & FC_KIND_MASK);

	if (ETHER_ISMULTI(&hdr->a1.octet[0]) ||
			(key_info->algo == CRYPTO_ALGO_OFF)) {
		allow = TRUE;
	}
	/* Unicast SDF frames should be allowed even if they are unecrypted */
	else if (fk == FC_ACTION) {
		nan_pub_act_frame_t *act_frame = (nan_pub_act_frame_t *)body;

		if ((body_len >= NAN_PUB_ACT_FRAME_FIXED_LEN) &&
				(act_frame->action_field == NAN_PUB_AF_ACTION) &&
				!memcmp(act_frame->oui, WFA_OUI, DOT11_OUI_LEN) &&
				(act_frame->oui_type == NAN_OUI_TYPE) &&
				(act_frame->category_id == NAN_PUB_AF_CATEGORY)) {
			/* the frame is NAN SDF frame.
			 * unicast NAN SDF frames will come unencrypted.
			 */
			allow = TRUE;
		}
	}

	return allow;
}

static int
km_nan_is_mc_key_valid(wlc_keymgmt_t *km, struct scb *scb, void *pkt)
{
	/* check if mc key existing for the NMSG group */
	return FALSE;
}

int
wlc_keymgmt_nan_pkt_need_enc(wlc_keymgmt_t *km, struct scb *scb, void *pkt)
{
	struct ether_header *eh;
	int skip_enc = FALSE;
	wlc_key_info_t key_info;
	wlc_info_t *wlc;

	wlc = SCB_WLC(scb);
	km = wlc->keymgmt;

	eh = (struct ether_header*) PKTDATA(wlc->osh, pkt);
	if (!ETHER_ISMULTI(&eh->ether_dhost[0])) {
		wlc_keymgmt_get_tx_key(km, scb, scb->bsscfg, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF) {
			skip_enc = TRUE;
		}
	} else if (ETHER_ISBCAST(&eh->ether_dhost[0]) ||
		(ETHER_ISMULTI(&eh->ether_dhost[0]) && !km_nan_is_mc_key_valid(km, scb, pkt))) {
		skip_enc = TRUE;
	}
	return skip_enc;
}
