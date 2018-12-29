/*
 * Key Management Module Implementation - scb support
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
 * $Id: km_scb.c 767601 2018-09-18 14:29:52Z $
 */

#include "km_pvt.h"
/* internal interface */
#include <wlc_ratelinkmem.h>

static void
km_scb_update_key(keymgmt_t *km, wlc_key_index_t key_idx,
	scb_t *scb, bool multi_band, wlc_key_info_t *key_info)
{
	wlc_key_t *key;
	wlc_key_info_t tmp_ki;

	KM_DBG_ASSERT(scb != NULL);

	KM_TRACE(("wl%d: %s: scb %p enter\n",  KM_UNIT(km), __FUNCTION__, scb));

	if (key_info == NULL)
		key_info = &tmp_ki;

	key = km->keys[key_idx].key;
	wlc_key_get_info(key, key_info);
	if (multi_band)
		key_info->flags |= WLC_KEY_FLAG_MULTI_BAND;
	else
		key_info->flags &= ~WLC_KEY_FLAG_MULTI_BAND;

	km_key_set_flags(key, key_info->flags);
	km->keys[key_idx].u.scb = scb;
}

static void
km_scb_cleanup(keymgmt_t *km, scb_t *scb)
{
	km_scb_t *scb_km;
	scb_t *other_scb;
	wlc_key_id_t key_id;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_LOG(("wl%d: %s: scb@%p %s enter\n",  KM_UNIT(km), __FUNCTION__,
		scb, bcm_ether_ntoa(&scb->ea, eabuf)));

	/* ignore internal scb */
	if (SCB_INTERNAL(scb))
		goto done;

	(void)key_id;

	/* check for scb in other band with the same address. If it is present,
	 * do not destroy keys, as they are shared. Also update the scb
	 * reference in km_pvt_key to other scb to keep it valid.
	 */
	other_scb =  (KM_ADDR_IS_BCMC(&scb->ea) || KM_NBANDS(km) < 2)  ?
		NULL : wlc_scbfindband(km->wlc, SCB_BSSCFG(scb),
		&scb->ea, KM_OTHERBANDUNIT(scb->bandunit));

	scb_km = KM_SCB(km, scb);

	if (!other_scb)
		km_bsscfg_reset_sta_info(km, SCB_BSSCFG(scb), NULL);

	if (scb_km->key_idx != WLC_KEY_INDEX_INVALID) {
		if (!other_scb) {
			km_free_key_block(km, WLC_KEY_FLAG_NONE, &scb_km->key_idx, 1);
			KM_DBG_ASSERT(scb_km->key_idx == WLC_KEY_INDEX_INVALID);
		} else {
			km_scb_update_key(km, scb_km->key_idx, other_scb, FALSE, NULL);
			scb_km->key_idx = WLC_KEY_INDEX_INVALID;
		}
	}

	km_scb_amt_release(km, scb);

done:
	KM_LOG(("wl%d: %s: exit\n",  KM_UNIT(km), __FUNCTION__));
}

static int
km_scb_init_internal(keymgmt_t *km, scb_t *scb)
{
	int err = BCME_OK;
	km_scb_t *scb_km;
	wlc_key_flags_t alloc_flags;
	size_t num_keys;
	wlc_key_index_t key_idx_arr[2*WLC_KEYMGMT_NUM_STA_GROUP_KEYS + 1];
	size_t key_idx_arr_pos = 0;
	wlc_key_index_t key_idx;
	wlc_key_t *key;
	wlc_key_info_t key_info;
	km_pvt_key_t *km_pvt_key;
	bool swkeys = FALSE;
	scb_t *other_scb;
	km_scb_t *other_scb_km = NULL;
	bool multi_band = FALSE;
	km_flags_t km_flags;
	wlc_key_id_t key_id;
	wlc_bsscfg_t *bsscfg;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	STATIC_ASSERT(WLC_KEYMGMT_NUM_STA_GROUP_KEYS == 2);

	KM_LOG(("wl%d: %s: scb@%p %s enter\n",  KM_UNIT(km), __FUNCTION__,
		scb, bcm_ether_ntoa(&scb->ea, eabuf)));

	/* ignore internal scb */
	if (SCB_INTERNAL(scb)) {
		goto done;
	}

	(void)key_id;

	scb_km = KM_SCB(km, scb);
	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	/* if scb exists in another band for the same bsscfg and address, use the key
	 * indicies allocated for it
	 */
	other_scb = (KM_ADDR_IS_BCMC(&scb->ea) || KM_NBANDS(km) < 2)  ?  NULL :
		wlc_scbfindband(km->wlc, bsscfg, &scb->ea,
		KM_OTHERBANDUNIT(scb->bandunit));

	if (other_scb) {
		other_scb_km = KM_SCB(km, other_scb);
		multi_band = TRUE;
		*scb_km = *other_scb_km;

		/* If keys have not been allocated, allocate them now. If keys are present
		 * mark them as multi-band
		 */
		if (scb_km->key_idx != WLC_KEY_INDEX_INVALID) {
			key_idx = scb_km->key_idx;

			km_scb_update_key(km, key_idx, scb, TRUE, &key_info);
			km_bsscfg_reset_sta_info(km, bsscfg, &key_info);

			goto done;
		}
	}

	swkeys = km_bsscfg_swkeys(km, bsscfg);

	alloc_flags = WLC_KEY_FLAG_NONE;
	num_keys = 1;	/* only pairwise key, by default */

	scb_km->key_idx = WLC_KEY_INDEX_INVALID;
	scb_km->amt_idx = KM_HW_AMT_IDX_INVALID;

	/* if BSS has no security, we can defer the rest since the key idx(s) in
	 * scb are now invalid. Nothing more for bcmc scb's, but the above init
	 * will allow them to be reused.
	 */
	if (KM_ADDR_IS_BCMC(&scb->ea) || !bsscfg->wsec)
		goto done;

	if (BSSCFG_AP(bsscfg))
		alloc_flags |= WLC_KEY_FLAG_AP;

	err = km_alloc_key_block(km, alloc_flags, key_idx_arr, num_keys);
	if (err != BCME_OK)
		goto done;

	/* create the pairwise key */
	key_idx = key_idx_arr[key_idx_arr_pos++];

	memset(&key_info, 0, sizeof(key_info));
	key_info.key_idx = key_idx;
	key_info.addr = scb->ea;
	key_info.flags = alloc_flags | WLC_KEY_FLAG_RX | WLC_KEY_FLAG_TX;
	if (multi_band)
		key_info.flags |= WLC_KEY_FLAG_MULTI_BAND;
	key_info.key_id = WLC_KEY_ID_PAIRWISE;
	err = km_key_create(km, &key_info, &key);
	if (err != BCME_OK) {
		km_free_key_block(km, alloc_flags, key_idx_arr, num_keys);
		goto done;
	}

	scb_km->key_idx = key_idx;

	km_pvt_key = &km->keys[key_idx];
	km_flags = KM_FLAG_TX_KEY | KM_FLAG_SCB_KEY;
	if (swkeys)
		km_flags |= KM_FLAG_SWONLY_KEY;

	km_init_pvt_key(km, km_pvt_key, CRYPTO_ALGO_NONE, key, km_flags, NULL, scb);
	km_bsscfg_reset_sta_info(km, bsscfg, &key_info);

	/* assign created keys to other scb if applicable */
	if (multi_band) {
		KM_DBG_ASSERT(other_scb_km != NULL);
		*other_scb_km = *scb_km;
	}

done:
	KM_LOG(("wl%d: %s: scb %s exit status %d\n",  KM_UNIT(km), __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, eabuf), err));

	return BCME_OK;
}

#ifdef WLRSDB
int
km_scb_serialize(wlc_keymgmt_t *km, struct scb *scb, void *buf_ptr, size_t buf_len)
{
	wlc_key_t *key;
	size_t tot_len = 0, data_len = 0;
	wlc_key_id_t id = WLC_KEY_ID_PAIRWISE;
	uint8 data[KM_KEY_MAX_DATA_LEN];
	int seq_id = 0, err = BCME_OK;
	wlc_key_info_t key_info_f;
	bool tx = TRUE;
	memset(buf_ptr, 0, buf_len);
	{
		key = wlc_keymgmt_get_scb_key(km, scb, id, WLC_KEY_FLAG_NONE,
			&key_info_f);

		/* Copy key algo to buffer */
		buf_ptr = bcm_write_tlv(KM_SERIAL_TLV_KEY_ALGO, &key_info_f.algo,
			sizeof(key_info_f.algo), buf_ptr);
		tot_len += sizeof(key_info_f.algo);

		/* Back the key data */
		err = wlc_key_get_data(key, data, sizeof(data), &data_len);
		if (err) {
			return BCME_ERROR;
		}
		buf_ptr = bcm_write_tlv(KM_SERIAL_TLV_KEY_DATA, data, data_len, buf_ptr);
		tot_len += data_len;

		/* Back-up tx seq number */
		data_len = wlc_key_get_seq(key, data, sizeof(data), seq_id, tx);
		buf_ptr = bcm_write_tlv(KM_SERIAL_TLV_TX_SEQ, data, data_len, buf_ptr);
		tot_len += data_len;

		/* Back-up rx seq number */
		for (seq_id = 0; seq_id < (size_t)WLC_KEY_NUM_RX_SEQ; seq_id++) {
			data_len = wlc_key_get_seq(key, data, sizeof(data), seq_id, !tx);
			buf_ptr = bcm_write_tlv(KM_SERIAL_TLV_RX_SEQ, data, data_len, buf_ptr);
			tot_len += data_len;
		}
	}
	KM_DBG_ASSERT(tot_len <= buf_len);
	return BCME_OK;
}

int
km_scb_deserialize(wlc_keymgmt_t *km, struct scb *scb, void* buf_ptr, size_t buf_len)
{
	wlc_key_info_t key_info_t, key_info_f;
	bcm_tlv_t *key_tlv;
	wlc_key_t *to_key;
	bool tx = TRUE;
	wlc_key_id_t id = WLC_KEY_ID_PAIRWISE;
	int seq_id = 0, ret = BCME_OK;
	{
		/* Get the SCB key on the new wlc */
		to_key = wlc_keymgmt_get_scb_key(km, scb, id, WLC_KEY_FLAG_NONE, &key_info_t);

		/* Get the key algo from backup */
		key_tlv = bcm_parse_tlvs(buf_ptr, buf_len, KM_SERIAL_TLV_KEY_ALGO);
		if (key_tlv && key_tlv->len) {
			memcpy(&key_info_f.algo, key_tlv->data, key_tlv->len);
			buf_ptr = (uint8 *)buf_ptr + (key_tlv->len + TLV_HDR_LEN);
			buf_len -= (key_tlv->len + TLV_HDR_LEN);
		}

		/* Set the data taken from old wlc to new wlc */
		key_tlv = bcm_parse_tlvs(buf_ptr, buf_len, KM_SERIAL_TLV_KEY_DATA);
		if (key_tlv && key_tlv->len) {
			ret = wlc_key_set_data(to_key, key_info_f.algo, key_tlv->data,
				key_tlv->len);
			buf_ptr = (uint8 *)buf_ptr + (key_tlv->len + TLV_HDR_LEN);
			buf_len -= (key_tlv->len + TLV_HDR_LEN);
		}

		/* Clone tx seq number */
		key_tlv = bcm_parse_tlvs(buf_ptr, buf_len, KM_SERIAL_TLV_TX_SEQ);
		if (key_tlv && key_tlv->len) {
			ret = wlc_key_set_seq(to_key, key_tlv->data, key_tlv->len,
				seq_id, tx);
			buf_ptr = (uint8 *)buf_ptr + (key_tlv->len + TLV_HDR_LEN);
			buf_len -= (key_tlv->len + TLV_HDR_LEN);
		}

		/* Clone rx seq number */
		for (seq_id = 0; seq_id < (size_t)WLC_KEY_NUM_RX_SEQ; seq_id++) {
			key_tlv = bcm_parse_tlvs(buf_ptr, buf_len, KM_SERIAL_TLV_RX_SEQ);
			if (key_tlv && key_tlv->len) {
				ret = wlc_key_set_seq(to_key, key_tlv->data, key_tlv->len,
					seq_id, !tx);
				buf_ptr = (uint8 *)buf_ptr + (key_tlv->len + TLV_HDR_LEN);
				buf_len -= (key_tlv->len + TLV_HDR_LEN);
			}
		}
	}
	return ret;
}

int
km_scb_update(void *context, struct scb *scb, wlc_bsscfg_t* new_cfg)
{
	keymgmt_t *km = (keymgmt_t *)context;
	wlc_info_t *new_wlc = new_cfg->wlc;
	wlc_bsscfg_t *from_bsscfg = scb->bsscfg;

	KM_TRACE(("wl%d: %s: scb %p enter\n",  KM_UNIT(km), __FUNCTION__, scb));

	/* Back up the SCB clone data to buffer */
	km_scb_serialize(km, scb, km->tlv_buffer, km->tlv_buf_size);
	/* Cleanup the keys on from_wlc */
	km_scb_cleanup(km, scb);
	/* Update the cfg to new cfg pointer */
	scb->bsscfg = new_cfg;
	/* Initialize the keys on to_wlc with default values */
	km_scb_init_internal(new_wlc->keymgmt, scb);
	/* Restore the SCB clone data */
	km_scb_deserialize(new_wlc->keymgmt, scb, km->tlv_buffer, km->tlv_buf_size);
	/* Restore back the old cfg value
	* so that remaining update functions
	* can complete their operation.
	*/
	scb->bsscfg = from_bsscfg;
	return BCME_OK;
}
#endif /* WLRSDB */

/* public interface */
int
km_scb_init(void *ctx, scb_t *scb)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_scb_t *scb_km;
	int err;

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);

	KM_TRACE(("wl%d: %s: scb %p enter\n",  KM_UNIT(km), __FUNCTION__, scb));

	scb_km = KM_SCB(km, scb);
	memset(scb_km, 0, sizeof(*scb_km));
	/* When alloc failure for scb, driver may use un-initialize scb cubby in cubby fn_deinit.
	 * This will cause some unexpected handling and trap.
	 * Set flags to indicate that this scb_cubby fn_init is executed.
	 */
	scb_km->flags |= KM_SCB_FLAG_INIT;
	err = km_scb_init_internal(km, scb);
	if (err == BCME_OK) {
		wlc_keymgmt_notify(km, WLC_KEYMGMT_NOTIF_SCB_BSSCFG_CHANGED,
			NULL, scb, NULL, NULL);
	}
	return err;
}

void
km_scb_deinit(void *ctx,  scb_t *scb)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_scb_t *scb_km;

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);
	/* If the KM_SCB_FLAG_INIT is not set. It means that cubby fn_init is not executed.
	 * Driver should not do km_scb_cleanup().
	 */
	KM_TRACE(("wl%d: %s: scb %p enter\n",  KM_UNIT(km), __FUNCTION__, scb));

	scb_km = KM_SCB(km, scb);
	if (scb_km->flags & KM_SCB_FLAG_INIT) {
		scb_km->flags &= ~KM_SCB_FLAG_INIT;
		km_scb_cleanup(km, scb);
	}
}

void
km_scb_reset(keymgmt_t *km, scb_t *scb)
{
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_DBG_ASSERT(KM_VALID(km));
	KM_DBG_ASSERT(scb != NULL);
	KM_LOG(("wl%d: %s: scb %s enter\n",  KM_UNIT(km), __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, eabuf)));

	km_scb_cleanup(km, scb);
	km_scb_init_internal(km, scb);

	KM_LOG(("wl%d: %s: scb %s exit\n",  KM_UNIT(km), __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, eabuf)));
}

wlc_key_t*
wlc_keymgmt_get_scb_key(wlc_keymgmt_t *km, scb_t *scb,
	wlc_key_id_t key_id, wlc_key_flags_t key_flags, wlc_key_info_t *key_info)
{
	km_scb_t *scb_km;
	wlc_key_index_t key_idx;
	km_pvt_key_t *km_pvt_key;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info_s;

	KM_DBG_ASSERT(KM_VALID(km));
	KM_ASSERT(scb != NULL);

	if (!key_info)
		key_info = &key_info_s;

	if (KM_IGNORED_SCB(scb))
		goto done;

	scb_km = KM_SCB(km, scb);
	if (!(key_flags & WLC_KEY_FLAG_GROUP) &&
		!(key_flags & WLC_KEY_FLAG_IBSS_PEER_GROUP)) { /* pairwise key */
		key_idx = scb_km->key_idx;
		if (!KM_VALID_KEY_IDX(km, key_idx))
			goto done;

		km_pvt_key = &km->keys[key_idx];
		KM_DBG_ASSERT(KM_VALID_KEY(km_pvt_key));

		wlc_key_get_info(km_pvt_key->key, key_info);

		/* scb key will have pairwise key id (0) except for wapi */
		if (key_id == key_info->key_id) {
			key = km_pvt_key->key;
			goto done;
		}

		/* allow lookup in dynamic 802.1x/wep case irrespective of key id */
		{
			wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
			if (WSEC_WEP_ENABLED(bsscfg->wsec) && (key_id == WLC_KEY_ID_8021X_WEP)) {
				key = km_pvt_key->key;
				goto done;
			}
		}
	}
	else if (KM_BSSCFG_NEED_STA_GROUP_KEYS(km, SCB_BSSCFG(scb)) &&
		(key_flags & WLC_KEY_FLAG_GROUP) && WLC_KEY_ID_IS_DEFAULT(key_id)) {
			key = wlc_keymgmt_get_bss_key(km, SCB_BSSCFG(scb), key_id, key_info);
			goto done;
	}

done:
	/* check for NULL */
	if (!key) {
		key = km->null_key;
		wlc_key_get_info(key, key_info);
	}
	return key;
}

wlc_key_t*
wlc_keymgmt_get_tx_key(wlc_keymgmt_t *km, scb_t *scb,
	wlc_bsscfg_t *bsscfg, wlc_key_info_t *key_info)
{
	wlc_key_t *key = NULL;
	km_scb_t *scb_km;

	if (scb) {
		const km_key_cache_t *kc = km->key_cache;
		scb_km = KM_SCB(km, scb);
		if (kc && key_info && !BSSCFG_PSTA(scb->bsscfg) &&
				!ether_cmp(&scb->ea, &kc->key_info->addr)) {
			if (key_info->key_idx == scb_km->key_idx) {
				key = kc->key;
				*key_info = *kc->key_info;
				goto done;
			}
		}

		key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, key_info);
	} else {
		key = wlc_keymgmt_get_bss_tx_key(km, bsscfg, FALSE, key_info);
	}

done:
	return key;
}

#ifdef BRCMAPIVTW
int
wlc_keymgmt_ivtw_enable(wlc_keymgmt_t *km, scb_t *scb, bool enable)
{
	int err = BCME_OK;
	km_scb_t *scb_km;
	wlc_bsscfg_t *bsscfg;

	KM_DBG_ASSERT(KM_VALID(km));
	if (scb == NULL)
		goto done;

	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	scb_km = KM_SCB(km, scb);
	if (KM_VALID_KEY_IDX(km, scb_km->key_idx))
		err = km_ivtw_enable(km->ivtw, scb_km->key_idx, enable);
	err =  km_bsscfg_ivtw_enable(km, bsscfg, enable);
done:
	return err;
}
#endif /* BRCMAPIVTW */

void
km_scb_state_upd(keymgmt_t *km, scb_state_upd_data_t *data)
{
	scb_t *scb;
	wlc_bsscfg_t *bsscfg;
	wlc_key_t *key;
	wlc_key_info_t key_info;

	KM_DBG_ASSERT(data != NULL && data->scb != NULL);

	scb = data->scb;
	if (KM_IGNORED_SCB(scb))
		return;

	bsscfg = SCB_BSSCFG(scb);

	/* clear scb key on state update when wpa scb is unauthorized */
	key = wlc_keymgmt_get_scb_key(km, scb, WLC_KEY_ID_PAIRWISE, WLC_KEY_FLAG_NONE, &key_info);
	if ((key_info.algo != CRYPTO_ALGO_OFF) &&
			((!bsscfg->BSS && !SCB_AUTHORIZED(scb)) ||
			(bsscfg->BSS && !SCB_WDS(scb) && !SCB_ASSOCIATED(scb)) ||
			(bsscfg->BSS && SCB_WDS(scb) && !SCB_AUTHORIZED(scb))) &&
			(bsscfg->WPA_auth != WPA_AUTH_DISABLED) &&
			(bsscfg->WPA_auth != WPA_AUTH_NONE)) {
		wlc_key_set_data(key, CRYPTO_ALGO_OFF, NULL, 0);
		if (BSSCFG_STA(bsscfg))
			km_bsscfg_reset(km, bsscfg, FALSE);
	}

	/* note: scb amt is released when the scb is reset - even though
	 * it is not needed once the sta is disassociated because the process
	 * of destroying the key clears key data and thus may use the key/amt idx
	 */

#ifdef BRCMAPIVTW
	/* update ivtw for key as necessary */
	{
		int ivtw_mode;
		bool enable = FALSE;

		if (key_info.key_idx == WLC_KEY_INDEX_INVALID ||
			bsscfg->WPA_auth == WPA_AUTH_DISABLED ||
			(bsscfg->BSS && !SCB_ASSOCIATED(scb))) {
			goto upd_ivtw;
		}

		ivtw_mode = km_ivtw_get_mode(km->ivtw);
		if (ivtw_mode == ON) {
			enable = TRUE;
		}
#ifdef WLAMPDU_HOSTREORDER
		else if (AMPDU_HOST_REORDER_ENAB(KM_PUB(km))) {
			if (ivtw_mode == OFF)
				(void)km_ivtw_set_mode(km->ivtw, AUTO);
			enable = TRUE;
		}
#endif /* WLAMPDU_HOSTREORDER */

upd_ivtw:
		(void)wlc_keymgmt_ivtw_enable(km, scb, enable);
	}
#endif /* BRCMAPIVTW */
}

km_amt_idx_t
km_scb_amt_alloc(keymgmt_t *km, scb_t *scb)
{
	km_amt_idx_t amt_idx;
	km_scb_t *scb_km;
	wlc_bsscfg_t *bsscfg;
	const struct ether_addr *ea;
	km_amt_attr_t amt_attr;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	KM_DBG_ASSERT(bsscfg != NULL);

	ea = &scb->ea;
#ifdef PSTA
	if (BSSCFG_STA(bsscfg) && PSTA_ENAB(KM_PUB(km)))
		ea = &bsscfg->cur_etheraddr; /* PSTA uses A1 match */
#endif // endif
#if defined(WET) || defined(WET_DONGLE)
	if (BSSCFG_STA(bsscfg) && (WET_ENAB(km->wlc) || WET_DONGLE_ENAB(km->wlc))) {
		ea = &bsscfg->cur_etheraddr; /* WET uses A1 match */
	}
#endif // endif

	scb_km = KM_SCB(km, scb);
	amt_idx = scb_km->amt_idx;
	if (amt_idx != KM_HW_AMT_IDX_INVALID) {
		/* if wlc is reinitialized - i.e. s/w state is preserved, but km_hw
		 * is reset, any reservation for this amt index is lost; so restore it
		 */
		km_hw_amt_reserve(km->hw, amt_idx, 1, TRUE);
		goto done;
	}

	/* use bss amt allocation for a sta, if available */
	if (bsscfg->BSS && BSSCFG_STA(bsscfg)) {
		amt_idx = km_bsscfg_get_amt_idx(km, bsscfg);
		if (amt_idx != KM_HW_AMT_IDX_INVALID)
			goto upd_amt;
	}

	/* there is no sta amt entry for default bss scb without wpa unless wep is not
	 * enabled
	 */
	if (KM_IS_DEFAULT_BSSCFG(km, bsscfg) && !BSSCFG_AP(bsscfg) &&
#ifdef DWDS
		!DWDS_ENAB(bsscfg) &&
#endif /* DWDS */
		!RATELINKMEM_ENAB(KM_PUB(km)) && /* always need an AMT entry for RateLinkMem */
		(bsscfg->WPA_auth == WPA_AUTH_DISABLED) &&
		(WSEC_WEP_ENABLED(bsscfg->wsec))) {
		goto done;
	}

#ifdef ACKSUPR_MAC_FILTER
	/* find key idx from amt table confiured by white list in macfilter module */
	if ((BSSCFG_AP(bsscfg) && WLC_ACKSUPR(km->wlc))) {
		amt_idx = km_hw_amt_alloc_acksupr(km->hw, scb);
		if (amt_idx != KM_HW_AMT_IDX_INVALID) {
			scb_km = KM_SCB(km, scb);
			scb_km->flags |= KM_SCB_FLAG_OWN_AMT;
			scb_km->amt_idx = amt_idx;
			KM_LOG(("wl%d.%d: %s: allocate amt_idx %d for %s\n",
				km->wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
				__FUNCTION__, amt_idx, bcm_ether_ntoa(ea, eabuf)));
			goto done;
		}
	}
#endif /* ACKSUPR_MAC_FILTER */

	/* reserve and own an amt entry */
	amt_idx = km_hw_amt_alloc(km->hw, ea);
	if (amt_idx == KM_HW_AMT_IDX_INVALID)
		goto done;

	scb_km->flags |= KM_SCB_FLAG_OWN_AMT;

upd_amt:
	/* preserve any valid amt attributes */
	amt_attr = wlc_clear_addrmatch(km->wlc, amt_idx);
	if (!(amt_attr & AMT_ATTR_VALID))
		amt_attr = 0;
	amt_attr |= (AMT_ATTR_VALID | AMT_ATTR_A2);
	wlc_set_addrmatch(km->wlc, amt_idx, ea, amt_attr);

	km_amt_defkey_update(km, amt_idx);

	scb_km->amt_idx = amt_idx;

done:
	KM_LOG(("wl%d.%d: %s: reserved amt index %d for addr %s\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
		amt_idx,  bcm_ether_ntoa(ea, eabuf)));
	return amt_idx;
}

void
km_scb_amt_release(keymgmt_t *km, scb_t *scb)
{
	km_scb_t *scb_km;
#ifdef ACKSUPR_MAC_FILTER
	wlc_bsscfg_t *tmp_cfg;
	uint16 idx;
#endif /* ACKSUPR_MAC_FILTER */
	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);

	scb_km = KM_SCB(km, scb);
	KM_LOG(("wl%d.%d: %s: releasing amt index %d\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__, scb_km->amt_idx));

	if (scb_km->amt_idx == KM_HW_AMT_IDX_INVALID)
		goto done;
#ifdef ACKSUPR_MAC_FILTER
	if (WLC_ACKSUPR(km->wlc)) {
		FOREACH_AP(km->wlc, idx, tmp_cfg) {
			if (BSSCFG_ACKSUPR(tmp_cfg) && (wlc_macfltr_addr_match(km->wlc->macfltr,
				tmp_cfg, &scb->ea) == WLC_MACFLTR_ADDR_ALLOW)) {
				km_hw_amt_reserve(km->hw, scb_km->amt_idx, 1, FALSE);
				scb_km->amt_idx = KM_HW_AMT_IDX_INVALID;
				scb_km->flags &= ~KM_SCB_FLAG_OWN_AMT;
				goto done;
			}
		}
	}
#endif /* ACKSUPR_MAC_FILTER */

#ifdef PSTA
	if (BSSCFG_PSTA(SCB_BSSCFG(scb)))
		km_hw_amt_reserve(km->hw, scb_km->amt_idx, 1, FALSE);
#endif // endif

	if (RATELINKMEM_ENAB(KM_PUB(km))) {
		/* Notify RateLinkMem of the pending release */
		wlc_ratelinkmem_scb_amt_release(km->wlc, scb, scb_km->amt_idx);
	}

	if (!KM_SCB_OWN_AMT(scb_km)) {
		scb_km->amt_idx = KM_HW_AMT_IDX_INVALID;
		goto done;
	}

	km_hw_amt_release(km->hw, &scb_km->amt_idx);
	scb_km->flags &= ~KM_SCB_FLAG_OWN_AMT;

done:
	KM_DBG_ASSERT(scb_km->amt_idx == KM_HW_AMT_IDX_INVALID);
}

int wlc_keymgmt_get_scb_amt_idx(wlc_keymgmt_t *km, scb_t *scb)
{
	km_amt_idx_t amt_idx;
	int err;

	KM_ASSERT(KM_VALID(km));

	err = BCME_NOTFOUND;
	/* OLPC SCB is not ignored */
	if (!scb || (KM_IGNORED_SCB(scb) && !SCB_OLPC(scb)))
		goto done;

	ASSERT((KM_SCB(km, scb)->flags & KM_SCB_FLAG_INIT) != 0);

	amt_idx = km_scb_amt_alloc(km, scb);
	if (amt_idx != KM_HW_AMT_IDX_INVALID)
		err = amt_idx;

done:
	return err;
}

void wlc_keymgmt_restore_scb_amt_entry(wlc_keymgmt_t *km, scb_t *scb)
{
	km_amt_idx_t amt_idx;
	km_scb_t *scb_km;
	const struct ether_addr *ea;
	km_amt_attr_t amt_attr;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);

	KM_DBG_ASSERT(SCB_BSSCFG(scb) != NULL);

	KM_DBG_ASSERT(RATELINKMEM_ENAB(KM_PUB(km)));

	ea = &scb->ea;
	if (KM_IGNORED_SCB(scb)) {
		return;
	}

	scb_km = KM_SCB(km, scb);
	amt_idx = scb_km->amt_idx;

	if (amt_idx == KM_HW_AMT_IDX_INVALID) {
		return;
	}

	/* if wlc is reinitialized - i.e. s/w state is preserved, but km_hw
	 * is reset, any reservation for this amt index is lost; so restore it
	 */
	km_hw_amt_reserve(km->hw, amt_idx, 1, TRUE);

	/* preserve any valid amt attributes */
	amt_attr = wlc_clear_addrmatch(km->wlc, amt_idx);
	if (!(amt_attr & AMT_ATTR_VALID)) {
		amt_attr = 0;
	}
	amt_attr |= (AMT_ATTR_VALID | AMT_ATTR_A2);
	wlc_set_addrmatch(km->wlc, amt_idx, ea, amt_attr);

	KM_LOG(("wl%d.%d: %s: reserved amt index %d for addr %s flags 0x%04x\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__,
		amt_idx,  bcm_ether_ntoa(ea, eabuf), scb_km->flags));
}

#if defined(ACKSUPR_MAC_FILTER) || defined(PSTA) || defined(WL_CCA_STATS_MESH)
bool wlc_keymgmt_amt_idx_isset(wlc_keymgmt_t *km, int amt_idx)
{
	return km_hw_amt_idx_isset(km->hw, amt_idx);
}
#endif /* ACKSUPR_MAC_FILTER */
