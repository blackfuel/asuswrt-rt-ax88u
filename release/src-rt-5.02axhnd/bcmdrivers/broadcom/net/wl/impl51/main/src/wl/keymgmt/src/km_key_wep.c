/*
 * Implementation of wlc_key algo 'wep'
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
 * $Id: km_key_wep.c 685197 2017-02-15 20:37:25Z $
 */

#include "km_key_wep_pvt.h"

/* internal interface */

static int
wep_destroy(wlc_key_t *key)
{
	KM_DBG_ASSERT(WEP_KEY_VALID(key));

	if (key->algo_impl.ctx != NULL) {
		MFREE(KEY_OSH(key), key->algo_impl.ctx, sizeof(wep_key_t));
	}

	key->algo_impl.cb = NULL;
	key->algo_impl.ctx = NULL;
	key->info.key_len = 0;
	key->info.iv_len = 0;
	key->info.icv_len = 0;
	return BCME_OK;
}

static int
wep_get_data(wlc_key_t *key, uint8 *data, size_t data_size,
	size_t *data_len, key_data_type_t data_type, int ins, bool tx)
{
	wep_key_t *wep_key;
	int err = BCME_OK;

	KM_DBG_ASSERT(WEP_KEY_VALID(key));

	wep_key = (wep_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		if (data_len != NULL) {
			*data_len = key->info.key_len;
		}

		if (data_size < key->info.key_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(data, wep_key->key, key->info.key_len);
		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (!tx || (ins != 0)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (data_len != NULL) {
			*data_len = key->info.iv_len;
		}

		if (data_size < key->info.iv_len) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		memcpy(data, wep_key->tx_seq, key->info.iv_len);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wep_set_data(wlc_key_t *key, const uint8 *data,
    size_t data_len, key_data_type_t data_type, int ins, bool tx)
{
	wep_key_t *wep_key;
	int err = BCME_OK;
	int i;
	wlc_bsscfg_t *bsscfg;
	scb_t *scb;

	KM_DBG_ASSERT(WEP_KEY_VALID(key));

	wep_key = (wep_key_t *)key->algo_impl.ctx;
	switch (data_type) {
	case WLC_KEY_DATA_TYPE_KEY:
		/* note: we don't anticipate hex keys here */
		if (data_len != key->info.key_len) {
			if (!data_len) {
				memset(wep_key->key, 0, key->info.key_len);
				break;
			}
			if (data_len < key->info.key_len) {
				err = BCME_BADLEN;
				break;
			}
			data_len = key->info.key_len;
		}

		if (!data) {
			err = BCME_BADARG;
			break;
		}

		/* when key changes, initialize tx_seq also */
		memset(wep_key, 0, sizeof(wep_key_t));
		memcpy(wep_key->key, data, key->info.key_len);
		for (i = 0; i < (key->info.iv_len - 1); i += 2) {
			uint16 r;
			if (KEY_PUB(key)->up)
				r = R_REG(KEY_OSH(key), D11_TSF_RANDOM(KEY_WLC(key)));
			else
				r = (uint16)(KEY_PUB(key)->now + (i << 8) + i);
			wep_key->tx_seq[i] = r & 0xff;
			wep_key->tx_seq[i+1] = r >> 8 & 0xff;
		}
		wep_key->tx_seq[key->info.iv_len-1] = 0;

		/* support for multiple wep keys (no wpa) where scb key is configured. key id of
		* the key needs fixup - non standard
		*/
		scb = wlc_keymgmt_get_scb(KEY_KM(key), key->info.key_idx);
		if (scb)
			bsscfg = SCB_BSSCFG(scb);
		else
			bsscfg = wlc_keymgmt_get_bsscfg(KEY_KM(key), key->info.key_idx);

		KM_DBG_ASSERT(bsscfg != NULL);

		if (scb != NULL && bsscfg->WPA_auth == WPA_AUTH_DISABLED) {
			wlc_key_t *bss_key;
			bss_key = wlc_keymgmt_get_bss_tx_key(KEY_KM(key), bsscfg, FALSE, NULL);
			if (key->info.algo == bss_key->info.algo &&
				bss_key->info.key_id != key->info.key_id) {
				const wep_key_t *bss_wep_key =
					(const wep_key_t *)key->algo_impl.ctx;
				KM_DBG_ASSERT(bss_key->info.key_len == key->info.key_len);
				if (!memcmp(bss_wep_key->key, wep_key->key, key->info.key_len))
					key->info.key_id = bss_key->info.key_id;
			}
		}

		break;
	case WLC_KEY_DATA_TYPE_SEQ:
		if (!tx || (ins != 0)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (data_len > key->info.iv_len) {
			err = BCME_BUFTOOLONG;
			break;
		}
		memcpy(wep_key->tx_seq, data, key->info.iv_len);
		wep_key->tx_seq[key->info.iv_len-1] = 0;
		/* note: weak iv checks are done during tx prep */
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wep_rx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, const key_pkt_info_t *pkt_info)
{
	wep_key_t *wep_key;
	int err = BCME_OK;
	uint8 rc4_data[WEP_RC4_ALLOC_SIZE];
	rc4_ks_t rc4_ks;
	BCM_REFERENCE(pkt);

	KM_ASSERT(WEP_KEY_VALID(key));
	KM_DBG_ASSERT(pkt_info != NULL);

#ifdef BCMDBG
	if (body[KEY_ID_BODY_OFFSET] & DOT11_EXT_IV_FLAG) {
		KEY_LOG(("wl%d: %s: EXT IV is set for pkt, key idx %d\n",
			KEY_WLUNIT(key), __FUNCTION__, key->info.key_idx));
	}
#endif /* BCMDBG */

	if (pkt_info->flags & KEY_PKT_HWDEC) {
		err = pkt_info->status;
		goto done;
	}

	wep_key = (wep_key_t *)key->algo_impl.ctx;

	{
		memcpy(&rc4_data[0], body, WEP_RC4_IV_SIZE);
		memcpy(&rc4_data[WEP_RC4_IV_SIZE], wep_key->key, key->info.key_len);
		prepare_key(&rc4_data[0], WEP_RC4_IV_SIZE + key->info.key_len, &rc4_ks);
	}

	rc4(body + key->info.iv_len, body_len - key->info.iv_len, &rc4_ks);
	if (hndcrc32(body + key->info.iv_len, body_len - key->info.iv_len,
		CRC32_INIT_VALUE) != CRC32_GOOD_VALUE) {
		err = BCME_DECERR;
	}

	/* although the packet is decrypted successfully, do not
	 * clear the FC_WEP bit. Writing to packet data sometimes causes
	 * cache coherence issues (e.g. TKIP MIC)
	 */
done:
	if (err != BCME_OK) {
		WLCNTINCR(KEY_CNT(key)->wepicverr);
		WLCNTINCR(KEY_CNT(key)->wepundec);
		if (ETHER_ISMULTI(&hdr->a1)) {
			WLCNTINCR(KEY_CNT(key)->wepicverr_mcst);
			WLCNTINCR(KEY_CNT(key)->wepundec_mcst);
		}
	}

	return err;
}

/* check for weak ivs  used in several published WEP cracking tools */
static bool
wep_is_weak_iv(uint32 iv, uint8 key_len)
{
	bool weak = TRUE;
	uchar x, y, a, B;

	x = iv & 0xff;
	y = (iv >> 8) & 0xff;
	a = x + y;

	if (a <= key_len)
		goto done;

	/* iv[1] == 0xff and iv[0] is N+3 for some N 0..key_len -1 */
	if ((y == RC4_STATE_NBYTES - 1) &&
		(x > 2 && x <= key_len + 2))
		goto done;

	if (x == 1 && (y >= 2 && y <= ((key_len-1)/2 + 1)))
		goto done;

	weak = FALSE;

	B = 3;
	while (B < key_len/2 + 3) {
		if ((x == B) && (y == (RC4_STATE_NBYTES-1)-x)) {
			weak = TRUE;
			break;
		}
		B++;
	}

done:
	return weak;
}

static int
wep_tx_mpdu(wlc_key_t *key, void *pkt, struct dot11_header *hdr,
	uint8 *body, int body_len, d11txhdr_t *txd)
{
	int err = BCME_OK;
	uint8 rc4_data[WEP_RC4_ALLOC_SIZE];
	rc4_ks_t rc4_ks;
	uint32 icv;
	wep_key_t *wep_key;
	BCM_REFERENCE(pkt);
	BCM_REFERENCE(hdr);

	KM_ASSERT(WEP_KEY_VALID(key));
	KM_DBG_ASSERT(key->info.icv_len == sizeof(icv));

	wep_key = (wep_key_t *)key->algo_impl.ctx;

#ifdef BCMDBG
	/* wep does not have replay checks, but ... */
	if (key->info.flags & WLC_KEY_FLAG_GEN_REPLAY) {
		key->info.flags &= ~WLC_KEY_FLAG_GEN_REPLAY;
	} else
#endif /* BCMDBG */
	/* check weak wep seq/ivs and update tx seq */
	{
		uint32 tx_seq;
		tx_seq = (ltoh32_ua(wep_key->tx_seq) + 1) & 0x00ffffff;
		while (wep_is_weak_iv(tx_seq, key->info.key_len))
			tx_seq = (tx_seq + 1) & 0x00ffffff;
		htol32_ua_store(tx_seq, wep_key->tx_seq);
	}

	/* update iv in pkt - seq and key id  */
	memcpy(body, wep_key->tx_seq, WEP_RC4_IV_SIZE);
	body[KEY_ID_BODY_OFFSET] = (key->info.key_id) << DOT11_KEY_INDEX_SHIFT;

	if (WLC_KEY_IN_HW(&key->info) && txd != NULL) {
		if (KEY_COREREV_LT40(key)) {
			d11txh_pre40_t *txh = &txd->pre40;
			memcpy(txh->IV, body, key->info.iv_len);
		}
		goto done;
	}

	/* add icv to body */
	icv =  htol32(~hndcrc32(body + key->info.iv_len,
		body_len - key->info.iv_len, CRC32_INIT_VALUE));

	memcpy(body + body_len, &icv, sizeof(icv));
	body_len += sizeof(icv);
	/* now body starts at iv spans to end-of icv. */

	{
		/* encrypt from end-of-iv to end-of-icv */
		memcpy(&rc4_data[0], body, WEP_RC4_IV_SIZE);
		memcpy(&rc4_data[WEP_RC4_IV_SIZE], wep_key->key, key->info.key_len);
		prepare_key(&rc4_data[0], WEP_RC4_IV_SIZE + key->info.key_len, &rc4_ks);
	}

	rc4(body + key->info.iv_len, body_len - key->info.iv_len, &rc4_ks);

done:
	return err;
}

#if defined(BCMDBG)
static int
wep_dump(const wlc_key_t *key, struct bcmstrbuf *b)
{
	wep_key_t *wep_key;
	size_t i;

	KM_DBG_ASSERT(WEP_KEY_VALID(key));

	wep_key = (wep_key_t *)key->algo_impl.ctx;

	bcm_bprintf(b, "\twep key: ");
	for (i = 0; i < key->info.key_len; ++i)
		bcm_bprintf(b, "%02x", wep_key->key[i]);
	bcm_bprintf(b, "\n");

	bcm_bprintf(b, "\twep iv: ");
	for (i = 0; i < key->info.iv_len; ++i)
		bcm_bprintf(b, "%02x", wep_key->tx_seq[i]);
	bcm_bprintf(b, "\n");

	return BCME_OK;
}
#define WEP_DUMP wep_dump
#else
#define WEP_DUMP NULL
#endif // endif

/* ccx supports mic checks for wep */
#define WEP_RX_MSDU NULL
#define WEP_TX_MSDU NULL

static const key_algo_callbacks_t key_wep_callbacks = {
    wep_destroy,	/* destroy */
    wep_get_data,	/* get data */
    wep_set_data,	/* set data */
    wep_rx_mpdu,	/* rx mpdu */
    WEP_RX_MSDU,	/* rx msdu */
    wep_tx_mpdu,	/* tx mpdu */
    WEP_TX_MSDU,	/* tx msdu */
    WEP_DUMP		/* dump */
};

/* public interface */
int
km_key_wep_init(wlc_key_t *key)
{
	switch (key->info.algo) {
	case CRYPTO_ALGO_WEP1:
		key->info.key_len = WEP1_KEY_SIZE;
		break;
	case CRYPTO_ALGO_WEP128:
		key->info.key_len = WEP128_KEY_SIZE;
		break;
	default:
		KM_DBG_ASSERT(FALSE);
		return BCME_BADARG;
	}

	key->info.iv_len = DOT11_IV_LEN;
	key->info.icv_len = DOT11_ICV_LEN;

	key->algo_impl.cb = &key_wep_callbacks;
	key->algo_impl.ctx = MALLOCZ(KEY_OSH(key), sizeof(wep_key_t));
	return (key->algo_impl.ctx != NULL) ? BCME_OK : BCME_NOMEM;
}

int
km_key_wep_rx_defkey_fixup(wlc_key_t *rx_key, uint8 *body, int body_len)
{
	uint8 rc4_data[WEP_RC4_ALLOC_SIZE];
	rc4_ks_t rc4_ks;
	wlc_key_t *key;
	wep_key_t *wep_key;
	wlc_key_info_t key_info;
	int err = BCME_OK;

	KM_DBG_ASSERT(WEP_KEY_VALID(rx_key));

	key = wlc_keymgmt_get_bss_key(KEY_KM(rx_key), KEY_DEFAULT_BSSCFG(rx_key),
		KM_PKT_KEY_ID(body), &key_info);

	if (!KM_WEP_ALGO(key_info.algo)) {
		err = BCME_DECERR;
		goto done;
	}

	wep_key = (wep_key_t *)key->algo_impl.ctx;

	memcpy(&rc4_data[0], body, WEP_RC4_IV_SIZE);
	memcpy(&rc4_data[WEP_RC4_IV_SIZE], wep_key->key, key->info.key_len);
	prepare_key(&rc4_data[0], WEP_RC4_IV_SIZE + key->info.key_len, &rc4_ks);
	rc4(body + key->info.iv_len, body_len - key->info.iv_len, &rc4_ks);

done:
	return err;
}
