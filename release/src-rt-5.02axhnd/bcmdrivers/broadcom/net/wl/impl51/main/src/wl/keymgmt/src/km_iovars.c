/*
 * Key Management Module Implementation - iovar support
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
 * $Id: km_iovars.c 748967 2018-02-27 04:57:32Z $
 */

#include "km_pvt.h"
#include <wlc_pm.h>

/* wsec info tlv support */
typedef int (*km_iov_wsec_tlv_get_cb_t)(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const uint8 *data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max);
typedef int (*km_iov_wsec_tlv_set_cb_t)(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const wl_wsec_info_tlv_t *req);

#define DECL_KM_IOV_WSEC_TLV_GET_CB(_name) \
static int km_iov_get_wsec_info_ ## _name(keymgmt_t *km, wlc_bsscfg_t *bsscfg, \
    const uint8 *data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_end)

#define DECL_KM_IOV_WSEC_TLV_SET_CB(_name) \
static int km_iov_set_wsec_info_ ## _name(keymgmt_t *km, wlc_bsscfg_t *bsscfg, \
    const wl_wsec_info_tlv_t *req)

DECL_KM_IOV_WSEC_TLV_GET_CB(max_keys);
DECL_KM_IOV_WSEC_TLV_GET_CB(bss_key_len);
DECL_KM_IOV_WSEC_TLV_GET_CB(bss_algo);
DECL_KM_IOV_WSEC_TLV_GET_CB(tx_key_id);
DECL_KM_IOV_WSEC_TLV_SET_CB(algos);

struct km_iov_wsec_tlv_ent {
	wl_wsec_info_type_t type;
	uint16 min_req_len;
	uint16 max_req_len;
	km_iov_wsec_tlv_get_cb_t get_cb;
	km_iov_wsec_tlv_set_cb_t set_cb;
};
typedef struct km_iov_wsec_tlv_ent km_iov_wsec_tlv_ent_t;

typedef struct {
	keymgmt_t *km;
	wlc_bsscfg_t *bsscfg;
	wl_wsec_info_tlv_t *tlvs;
	uint8 *buf_end;
} km_iov_wsec_info_ctx_t;

static int
km_iov_get_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint p_len, uint8 *outbuf, size_t outbuf_len);
static int
km_iov_set_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint8 *outbuf, size_t outbuf_len);
static const km_iov_wsec_tlv_ent_t*
km_iov_get_tlv_ent(keymgmt_t *km, wl_wsec_info_type_t type);
#ifdef PSTA
static int
km_iov_check_psta_replay(keymgmt_t *km, wlc_bsscfg_t *bsscfg);
#endif /* PSTA */

/* wsec info tlv entries */
static const km_iov_wsec_tlv_ent_t km_iov_wsec_tlv_entries[] = {
	{WL_WSEC_INFO_MAX_KEYS, 0, sizeof(uint32), km_iov_get_wsec_info_max_keys, NULL},
	{WL_WSEC_INFO_BSS_KEY_LEN, 0, sizeof(uint32), km_iov_get_wsec_info_bss_key_len, NULL},
	{WL_WSEC_INFO_BSS_ALGO, 0, sizeof(uint32), km_iov_get_wsec_info_bss_algo, NULL},
	{WL_WSEC_INFO_BSS_TX_KEY_ID, 0, sizeof(uint32), km_iov_get_wsec_info_tx_key_id, NULL},
	{WL_WSEC_INFO_BSS_ALGOS, 0, sizeof(wl_wsec_info_algos_t), NULL, km_iov_set_wsec_info_algos}
};

static const size_t km_iov_wsec_tlv_num_entries =
    sizeof(km_iov_wsec_tlv_entries)/sizeof(km_iov_wsec_tlv_entries[0]);

/* iovar table - includes vars that may be unsupported to minimize rom inval */
static const bcm_iovar_t km_iovars[] = {
	{"wsec_key", IOV_WSEC_KEY, (IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, sizeof(wl_wsec_key_t)},
	{"wsec_key_seq", IOV_WSEC_KEY_SEQ, (IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, KEY_SEQ_SIZE},
	{"buf_key_b4_m4", IOV_BUF_KEY_B4_M4, (0), 0, IOVT_BOOL, 0},
	{"wapi_hw_enabled", IOV_WAPI_HW_ENABLED, (0), 0, IOVT_BOOL, 0},
	{"brcmapivtwo", IOV_BRCMAPIVTW_OVERRIDE, IOVF_RSDB_SET, 0, IOVT_INT8, 0},
	{"wsec_info", IOV_WSEC_INFO, (IOVF_OPEN_ALLOW), 0, IOVT_BUFFER, sizeof(wl_wsec_info_t)},
	{"wsec", IOV_WSEC, (IOVF_OPEN_ALLOW), IOVF2_RSDB_LINKED_CFG, IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
km_doiovar(void *ctx, uint32 actionid,
	void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	wlc_bsscfg_t *bsscfg;
	int err = BCME_OK;
	keymgmt_t *km = (keymgmt_t *)ctx;
	int val;
	int32 *ret_int_ptr;
	km_bsscfg_t *bss_km;
	wlc_info_t *wlc;
	wlc_key_t *key = NULL;
	wlc_key_info_t key_info;
	wlc_key_flags_t key_flags;
	scb_t *scb;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	BCM_REFERENCE(val_size);

	KM_DBG_ASSERT(KM_VALID(km));

	KM_TRACE(("wl%d: %s: enter\n",  KM_UNIT(km), __FUNCTION__));

	wlc = km->wlc;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	KM_DBG_ASSERT(bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);

	ret_int_ptr = (int32*)arg;
	if (p_len >= sizeof(val))
		memcpy(&val, params, sizeof(val));
	else
		val = 0;

	BCM_REFERENCE(bss_km);
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
#ifdef STA
	case IOV_GVAL(IOV_BUF_KEY_B4_M4):
	{
		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if ((uint)len < sizeof(int32)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		*ret_int_ptr = KM_BSSCFG_B4M4_ENABLED(bss_km);
		break;
	}
	case IOV_SVAL(IOV_BUF_KEY_B4_M4):
	{
		if (!BSSCFG_STA(bsscfg)) {
			err = BCME_UNSUPPORTED;
			break;
		}

		if (p_len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		km_b4m4_set(km, bsscfg, val != 0);
		break;
	}
#endif /* STA */
	case IOV_SVAL(IOV_WSEC_KEY):
	{
		wl_wsec_key_t wl_key, *out_wl_key = (wl_wsec_key_t *)arg;
		wlc_key_id_t key_id;
		struct ether_addr *ea;

		/* check arg */
		if ((uint)len < sizeof(wl_key)) {
			err = BCME_BADARG;
			break;
		}

		memcpy(&wl_key, arg, sizeof(wl_key));
		key_id = (wlc_key_id_t)wl_key.index;
		ea = &wl_key.ea;

		KM_LOG(("wl%d: %s: key id %02x addr %s\n",  KM_UNIT(km), __FUNCTION__,
			key_id,  bcm_ether_ntoa(ea, eabuf)));

		/* disallow multicast addresses - unicast and broadcast okay */
		if (ETHER_ISMULTI(ea) && !ETHER_ISBCAST(ea)) {
			err = BCME_BADADDR;
			break;
		}

		/* group keys have null addr, lookup key */
		memset(&key_info, 0, sizeof(key_info));
		key_info.key_idx = WLC_KEY_INDEX_INVALID;
		if (ETHER_ISNULLADDR(ea) ||
			(BSSCFG_AP(bsscfg) && !eacmp(ea->octet, bsscfg->BSSID.octet))) {
			key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID)
				key =  wlc_keymgmt_get_key(km, key_id, &key_info);
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
				err = BCME_BADKEYIDX;
				break;
			}
			key_id = key_info.key_id;
		}

#ifdef  MFP
		if (WLC_MFP_ENAB(wlc->pub) && KM_VALID_MGMT_KEY_ID(key_id)) {
			uint16 lo;
			uint32 hi;
			size_t seq_len;
			uint8 seq[DOT11_WPA_KEY_RSC_LEN];

			/* must have  valid key info */
			if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
				err = BCME_BADKEYIDX;
				break;
			}

			/* handle update and removal */
			if (wl_key.len)
				err = wlc_key_set_data(key, CRYPTO_ALGO_BIP,
					wl_key.data, wl_key.len);
			else
				err = wlc_key_set_data(key, key_info.algo, NULL, 0);

			if (err != BCME_OK)
				break;

			if (!wl_key.len) /* nothing more for removal */
				break;

			/* Note: lo and hi are interchanged between wsec_key and igtk.
			 * this is is due to layout of KDE where lo is 32 bits
			 */
			lo = wl_key.rxiv.hi & 0xffff;
			hi = (wl_key.rxiv.lo << 16) | ((wl_key.rxiv.hi & 0xffff0000) >> 16);
			seq_len = wlc_key_pn_to_seq(seq, sizeof(seq), lo, hi);
			err = wlc_key_set_seq(key, seq, seq_len, WLC_KEY_SEQ_ID_ALL /* seq_id */,
				/* tx */ BSSCFG_AP(bsscfg));
			if (err != BCME_OK) {
				break;
			}
#ifdef PSTA
			if (BSSCFG_PSTA(bsscfg) && (err == BCME_REPLAY)) {
				/* Ignore replay error in psta cases, if primary is
				 * already authorized
				 */
				err = km_iov_check_psta_replay(km, bsscfg);
			}
#endif /* PSTA */
			if ((wl_key.flags & WL_PRIMARY_KEY) || BSSCFG_AP(bsscfg)) {
				err = wlc_keymgmt_set_bss_tx_key_id(km,
					bsscfg, key_info.key_id, TRUE);
				if (err != BCME_OK)
					break;
			}
			break;
		}
#endif /* MFP */

		/* if removal, lookup based on key and remove it */
		if (!wl_key.len) {
			if (WLC_KEY_IS_GROUP(&key_info)) {
				if (!KM_VALID_DATA_KEY_ID(key_id)) {
					err = BCME_BADKEYIDX;
					break;
				}
				err = wlc_key_reset(key);
			} else if (ETHER_ISBCAST(ea)) { /* reset all */
				struct scb_iter scbiter;
				FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
					wlc_keymgmt_reset(km, NULL, scb);
				}
				wlc_keymgmt_reset(km, bsscfg, NULL);
			} else {
				scb = km_find_scb(km, bsscfg, ea, FALSE /* !create */);
				if (!scb) {
					err = BCME_NOTFOUND;
					break;
				}

				key_flags = WLC_KEY_FLAG_NONE;
				key = wlc_keymgmt_get_scb_key(km, scb, key_id, key_flags, NULL);
				err = wlc_key_reset(key);
				if (err != BCME_OK)
					break;
			}
			break;
		}

		/* handle key addition/update */
		key = km_find_key(km, bsscfg, ea, key_id,
			((wl_key.flags & WL_IBSS_PEER_GROUP_KEY) ?
			WLC_KEY_FLAG_IBSS_PEER_GROUP : 0), &key_info);

		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			KM_ERR(("wl%d: %s: status %d, key lookup failed.\n",
				KM_UNIT(km), __FUNCTION__, err));
			break;
		}

		if  (wl_key.algo == CRYPTO_ALGO_OFF) {
			switch (wl_key.len) {
			case WEP1_KEY_SIZE:  wl_key.algo = CRYPTO_ALGO_WEP1; break;
			case WEP128_KEY_SIZE:  wl_key.algo = CRYPTO_ALGO_WEP128; break;
			case TKIP_KEY_SIZE:
				/* fall through */
			case AES_KEY_SIZE:
			{
				err = wlc_keymgmt_get_cur_wsec_algo(km, bsscfg, wl_key.len,
					(wlc_key_algo_t *)&(wl_key.algo));
				if (err == BCME_OK) {
					break;
				}
			}
			default:
				err = BCME_BADLEN;
				KM_ERR(("wl%d: %s: status %d, "
					"can not deduce algo from key len %d.\n",
					KM_UNIT(km), __FUNCTION__, err, wl_key.len));
				break;
			}
		}

		if (err != BCME_OK)
			break;

#ifdef STA
		/* b4m4: keys set before M4 is sent may be buffered */
		if (BSSCFG_STA(bsscfg) && (bsscfg->WPA_auth != WPA_AUTH_DISABLED)) {
			err = km_b4m4_buffer_key(km, bsscfg, &wl_key);
			if (err == BCME_OK)
				break;
			/* not enabled or failed, install the key */
		}
#endif /* STA */

		/* update key algorithm and data */
		err = wlc_key_set_data(key, (wlc_key_algo_t)wl_key.algo, wl_key.data, wl_key.len);
		if (err != BCME_OK)
			break;

		key_info.algo = (wlc_key_algo_t)wl_key.algo;

		/* update key id in key if necessary */
		if (key_id != key_info.key_id) {
			err = km_key_set_key_id(key, key_id);
			if (err != BCME_OK)
				break;
			key_info.key_id = key_id;
		}

		/* update rxiv */
		if (wl_key.iv_initialized) {
			uint8 seq[DOT11_IV_MAX_LEN];
			size_t seq_len;

			seq_len = wlc_key_pn_to_seq(seq, sizeof(seq),
				wl_key.rxiv.lo, wl_key.rxiv.hi);
			err = wlc_key_set_seq(key, seq, seq_len,
				WLC_KEY_SEQ_ID_ALL, FALSE /* !tx */);
#ifdef PSTA
			if (BSSCFG_PSTA(bsscfg) && (err == BCME_REPLAY)) {
				/* Ignore replay error in psta cases, if primary is
				 * already authorized
				 */
				err = km_iov_check_psta_replay(km, bsscfg);
			}
#endif /* PSTA */
			if (err != BCME_OK)
				break;
		}

		if (WLC_KEY_IS_GROUP(&key_info) &&
			((wl_key.flags & WL_PRIMARY_KEY) || BSSCFG_AP(bsscfg) || !bsscfg->BSS ||
			(WLC_KEY_IS_DEFAULT_BSS(&key_info) && KM_WEP_ALGO(key_info.algo)))) {
			bool tx_key_upd;
			wlc_key_info_t bss_ki;

			(void)wlc_keymgmt_get_bss_tx_key(km, bsscfg, FALSE, &bss_ki);
			tx_key_upd = ((wl_key.flags & WL_PRIMARY_KEY) ||
				bss_ki.algo == CRYPTO_ALGO_NONE);
			if (tx_key_upd) {
				err = wlc_keymgmt_set_bss_tx_key_id(km, bsscfg, key_info.key_id,
					FALSE);
				if (err != BCME_OK)
					break;
			}
		}

		/* update flags if needed, after sync'ing since they may have changed above  */
		wlc_key_get_info(key, &key_info);
		key_flags = key_info.flags;

		if (bsscfg->BSS)
			key_flags &= ~WLC_KEY_FLAG_IBSS;
		else
			key_flags |= WLC_KEY_FLAG_IBSS;

		if ((key_info.algo != CRYPTO_ALGO_OFF) && WLC_KEY_IS_GROUP(&key_info)) {
			/* update rx flag. tx is updated by WL_PRIMARY_KEY handling above */
			if ((bsscfg->WPA_auth == WPA_AUTH_DISABLED) || BSSCFG_STA(bsscfg))
				key_flags |= WLC_KEY_FLAG_RX;
		}

		if (key_flags != key_info.flags)
			km_key_set_flags(key, key_flags);

		if (wl_key.index != key_info.key_idx) {
			KM_ASSERT(sizeof(out_wl_key->index) >= sizeof(wlc_key_index_t));
			memcpy(&out_wl_key->index, &key_info.key_idx,
				sizeof(wlc_key_index_t));
		}

		if (!err && (BSSCFG_SLOTTED_BSS(bsscfg))) {
			if ((scb = wlc_scbfind_dualband(wlc, bsscfg, &wl_key.ea)) != NULL) {
				if (wl_key.len > 0)
					wlc_scb_setstatebit(wlc, scb,
						AUTHENTICATED | AUTHORIZED);
				else
					wlc_scb_clearstatebit(wlc, scb,
						AUTHENTICATED | AUTHORIZED);
			}
		}
		break;
	}
	case IOV_GVAL(IOV_WSEC_KEY_SEQ):
	{
		uint8 *seq = arg;
		wlc_key_id_t key_id;
		int seq_len;

		if (p_len < sizeof(val))
			return BCME_BADARG;

		key_id = (wlc_key_id_t)val;

		/* try bss first - including MFP/igtk; if invalid attempt to use
		 * arg as key index
		 */
		key = wlc_keymgmt_get_bss_key(km, bsscfg, key_id, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID)
			key = wlc_keymgmt_get_key(km, (wlc_key_index_t)(val), &key_info);

		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		memset(seq, 0, len);
		seq_len = wlc_key_get_seq(key, seq, len, 0, TRUE /* tx */);
		if (seq_len < 0)
			err = BCME_BUFTOOSHORT;
		/* else okay */

		break;
	}
#if defined(BCMDBG)
	case IOV_SVAL(IOV_WSEC_KEY_SEQ): /* params <key idx:4x><tx:1x><seq id:2x>|<seq(LE):12> */
	{
		wlc_key_index_t key_idx;
		uint8 seq[KEY_SEQ_SIZE];
		wlc_key_seq_id_t seq_id;
		int i;
		bool tx;
		uint8 *p;

		if (p_len < (2*KEY_SEQ_SIZE + 7))  {
			err = BCME_BUFTOOSHORT;
			break;
		}

		p = (uint8 *)params;
		key_idx = km_hex2int(p[3], p[2]);
		key_idx |= km_hex2int(p[1], p[0]) << 8;

		tx = km_hex2int(p[4], '0');
		seq_id = km_hex2int(p[6], p[5]);

		for (i = 0; i < KEY_SEQ_SIZE; ++i) {
			int j;
			j =  7 + (i << 1);
			seq[i] = km_hex2int(p[j+1], p[j]);
		}

		key = wlc_keymgmt_get_key(km, key_idx, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		err = wlc_key_set_seq(key, seq, KEY_SEQ_SIZE, seq_id, tx);
		break;
	}
#endif /* BCMDBG */
#if defined(BCMDBG)
	case IOV_GVAL(IOV_WSEC_KEY):
	{
		int wl_idx;
		wl_wsec_key_t wl_key;
		size_t wl_key_len;

		if ((uint)len < sizeof(wl_key)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memcpy(&wl_key, arg, sizeof(wl_key));
		wl_idx = wl_key.index;

		key = wlc_keymgmt_get_key(km, (wlc_key_index_t)wl_idx, &key_info);
		if (key_info.key_idx == WLC_KEY_INDEX_INVALID) {
			err = BCME_BADKEYIDX;
			break;
		}

		memset(&wl_key, 0, sizeof(wl_key));
		wl_key.index = key_info.key_id;
		wl_key.len = sizeof(wl_key.data);
		wl_key.algo = key_info.algo;
		wl_key.ea = key_info.addr;

		err = wlc_key_get_data(key, wl_key.data, wl_key.len, &wl_key_len);
		if (err == BCME_UNSUPPORTED) {
			wl_key.len = 0;
			err = BCME_OK;
		} else if (err == BCME_OK) {
			wl_key.len = (uint32)wl_key_len;
		} else {
			break;
		}

		if (!WLC_KEY_IN_HW(&key_info))
			wl_key.flags |= WL_SOFT_KEY;
		if (WLC_KEY_IS_PRIMARY(&key_info))
			wl_key.flags |= WL_PRIMARY_KEY;

		/* no iv returned */

		memcpy(arg, &wl_key, sizeof(wl_key));
		break;
	}
#endif // endif

#ifdef BRCMAPIVTW
	case IOV_GVAL(IOV_BRCMAPIVTW_OVERRIDE):
		*ret_int_ptr = km_ivtw_get_mode(km->ivtw);
		break;
	case IOV_SVAL(IOV_BRCMAPIVTW_OVERRIDE):
		err = km_ivtw_set_mode(km->ivtw, val);
		break;
#endif /* BRCMAPIVTW */

	case IOV_GVAL(IOV_WSEC_INFO):
		err = km_iov_get_wsec_info(km, bsscfg, params, p_len, arg, (size_t)len);
		break;
	case IOV_SVAL(IOV_WSEC_INFO):
		err = km_iov_set_wsec_info(km, bsscfg, params, arg, (size_t)len);
		break;
	case IOV_GVAL(IOV_WSEC):
		*((uint*)arg) = km_bsscfg_get_wsec(km, bsscfg);
		break;

	case IOV_SVAL(IOV_WSEC):
		err = wlc_keymgmt_wsec(wlc, bsscfg, (uint32)val);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	KM_TRACE(("wl%d: %s: exit status %d\n",  KM_UNIT(km), __FUNCTION__, err));
	return err;
}

int
km_doiovar_wrapper(void *ctx, uint32 actionid,
	void *params, uint p_len,
	void *arg, uint len, uint val_size, struct wlc_if *wlcif)
{
	return  km_doiovar(ctx, actionid, params, p_len, arg, len, val_size, wlcif);
}

static int
km_iov_get_wsec_info_tlv_cb(void *ctx, const uint8 *data, uint16 type, uint16 tlv_len)
{
	km_iov_wsec_info_ctx_t *iov_ctx = ctx;
	keymgmt_t *km = iov_ctx->km;
	wlc_bsscfg_t *bsscfg = iov_ctx->bsscfg;
	wl_wsec_info_type_t tlv_type;
	const km_iov_wsec_tlv_ent_t *tlv_ent;
	wl_wsec_info_tlv_t *rsp_tlv;
	int err;

	tlv_type = type;
	tlv_ent = km_iov_get_tlv_ent(km, tlv_type);

	if  (!tlv_ent || !tlv_ent->get_cb) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	err = (*tlv_ent->get_cb)(km, bsscfg, data, tlv_len, iov_ctx->tlvs, iov_ctx->buf_end);
	rsp_tlv = iov_ctx->tlvs;

	/* Add length for type and len fields */
	iov_ctx->tlvs = (iov_ctx->tlvs + ltoh16(rsp_tlv->len));
done:
	return err;
}

static int
km_iov_get_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint p_len, uint8 *outbuf, size_t outbuf_len)
{
	km_iov_wsec_info_ctx_t ctx;
	wl_wsec_info_t *req;
	wl_wsec_info_t *rsp;
	int err = BCME_OK;
	wl_wsec_info_tlv_t *rsp_tlv;
	uint8 *rsp_max;
	size_t rsp_len;
	uint8 *ptlv;
	uint32 size = 0;
	int i = 0;

	/* make a copy of request, and return response in the buffer */
	req = (wl_wsec_info_t *)MALLOCZ(KM_OSH(km), (uint)outbuf_len);
	if (!req) {
		err = BCME_NOMEM;
		goto done;
	}

	memcpy(req, params, outbuf_len);

	if (req->version != WL_WSEC_INFO_VERSION) {
		err = BCME_VERSION;
		goto done;
	}

	rsp = (wl_wsec_info_t *)outbuf;
	memset(rsp, 0, outbuf_len);
	rsp->version = WL_WSEC_INFO_VERSION;
	rsp_len = OFFSETOF(wl_wsec_info_t, tlvs);
	rsp_tlv = (wl_wsec_info_tlv_t *)((uint8 *)rsp + rsp_len);
	rsp_max = (uint8 *)rsp + outbuf_len;

	ctx.km = km;
	ctx.bsscfg = bsscfg;
	ctx.tlvs = rsp_tlv;
	ctx.buf_end = rsp_max;

	ptlv = (uint8 *)req->tlvs;

	for (i = 0; i < req->num_tlvs; i++) {
		uint16 type, len;
		const uint8 *data;
		bcm_xtlv_opts_t opts = BCM_XTLV_OPTION_ALIGN32;

		bcm_xtlv_unpack_xtlv((const bcm_xtlv_t *)ptlv, &type, &len, &data, opts);
		size = bcm_xtlv_size_for_data(len, opts);

		/* size = tlv_hdr + datalen */
		ptlv += size;

		if ((err = km_iov_get_wsec_info_tlv_cb(&ctx, data, type, len)) != BCME_OK) {
			break;
		}
	}

done:
	if (req) {
		MFREE(KM_OSH(km), req, (uint)outbuf_len);
	}

	KM_LOG(("wl%d: %s: exit status %d\n",  KM_UNIT(km), __FUNCTION__, err));
	return err;
}

static int
BCMRAMFN(km_iov_set_wsec_info_tlv_cb)(void *ctx, const uint8 *data, uint16 type, uint16 len)
{
	km_iov_wsec_info_ctx_t *iov_ctx = ctx;
	keymgmt_t *km = iov_ctx->km;
	wlc_bsscfg_t *bsscfg = iov_ctx->bsscfg;
	wl_wsec_info_tlv_t *req_tlv;
	const km_iov_wsec_tlv_ent_t *tlv_ent;
	int err;

	tlv_ent = km_iov_get_tlv_ent(km, type);
	if  (!tlv_ent || !tlv_ent->set_cb) {
		err = BCME_UNSUPPORTED;
		goto done;
	}

	if (len < tlv_ent->min_req_len || len > tlv_ent->max_req_len) {
		err = BCME_BADLEN;
		goto done;
	}

	req_tlv = (wl_wsec_info_tlv_t *)MALLOCZ(KM_OSH(km),
		sizeof(wl_wsec_info_tlv_t) + len);
	if (!req_tlv) {
		err = BCME_NOMEM;
		goto done;
	}

	req_tlv->type = type;
	req_tlv->len = len;
	memcpy(req_tlv->data, data, len);
	err = (tlv_ent->set_cb)(km, bsscfg, req_tlv);
	MFREE(KM_OSH(km), req_tlv, sizeof(wl_wsec_info_tlv_t) + len);
done:
	return err;
}

int
wlc_keymgmt_wsec(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint32 val)
{
	KM_LOG(("wl%d: wlc_keymgmt_wsec: setting wsec 0x%x\n", wlc->pub->unit, val));

	bsscfg->wsec = val & (WEP_ENABLED|TKIP_ENABLED|AES_ENABLED|WSEC_SWFLAG|
	                      SES_OW_ENABLED);

#ifdef STA
	/* change of wsec may modify the PS_ALLOWED state */
	if (BSSCFG_STA(bsscfg) &&
		!BSSCFG_IS_RSDB_CLONE(bsscfg))
		wlc_set_pmstate(bsscfg, bsscfg->pm->PMenabled);
#endif /* STA */
	km_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_BSS_WSEC_CHANGED,
		bsscfg, NULL /* scb */, NULL /* key */, NULL /* pkt */);

	if (!wlc->pub->up)
		return (0);

	if (AIBSS_ENAB(wlc->pub)) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
	return (0);
} /* wlc_keymgmt_wsec */

static int
km_iov_set_wsec_info_algos(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const wl_wsec_info_tlv_t *req_tlv)
{
	km_bsscfg_t *bss_km;
	km_algo_mask_t algos;
	km_algo_mask_t mask;
	km_algo_mask_t cur_algos;
	wl_wsec_info_algos_t algos_t;
	uint32 wsec = 0;

	if (req_tlv->len < sizeof(algos)) {
		return BCME_ERROR;
	}

	bss_km = KM_BSSCFG(km, bsscfg);
	cur_algos = bss_km->wsec_config_algos;

	memcpy(&algos_t, req_tlv->data, sizeof(wl_wsec_info_algos_t));
	algos = (km_algo_mask_t)(algos_t.algos);
	mask = (km_algo_mask_t)(algos_t.mask);

	algos = ((cur_algos & ~(mask)) | (algos & mask));

	if (bss_km->wsec_config_algos == algos) {
		/* no change; return */
		return BCME_OK;
	}

	bss_km->wsec_config_algos = algos;

	/* algo changed */
	if (algos & KEY_ALGO_MASK_WEP) {
		wsec = WEP_ENABLED;
	}

	if (algos & KEY_ALGO_MASK_TKIP) {
		wsec |= TKIP_ENABLED;
	}

	if (algos & KEY_ALGO_MASK_AES) {
		wsec |= AES_ENABLED;
	}

	if (algos & KEY_ALGO_MASK_WAPI) {
		wsec |= WSEC_SWFLAG;
	}

	return BCME_OK;
}

static int
km_iov_set_wsec_info(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	wl_wsec_info_t *params, uint8 *outbuf, size_t outbuf_len)
{
	wl_wsec_info_t *req;
	km_iov_wsec_info_ctx_t ctx;
	uint16 total_tlv_len;
	int err;

	req = params;
	if (req->version != WL_WSEC_INFO_VERSION) {
		err = BCME_VERSION;
		goto done;
	}

	ctx.km = km;
	ctx.bsscfg = bsscfg;

	total_tlv_len = (uint16)(outbuf_len - OFFSETOF(wl_wsec_info_t, tlvs));
	err = bcm_unpack_xtlv_buf((void*)&ctx, (uint8*)req->tlvs, total_tlv_len,
		BCM_XTLV_OPTION_ALIGN32, km_iov_set_wsec_info_tlv_cb);
done:
	KM_LOG(("wl%d: %s: exit status %d\n",  KM_UNIT(km), __FUNCTION__, err));
	return err;
}

static const km_iov_wsec_tlv_ent_t *
km_iov_get_tlv_ent(keymgmt_t *km, wl_wsec_info_type_t type)
{
	size_t i;
	const km_iov_wsec_tlv_ent_t *ent = NULL;
	BCM_REFERENCE(km);
	for (i = 0; i < km_iov_wsec_tlv_num_entries; ++i) {
		if (km_iov_wsec_tlv_entries[i].type == type) {
			ent = &km_iov_wsec_tlv_entries[i];
			break;
		}
	}
	return ent;
}

static int
km_iov_get_wsec_info_max_keys(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const uint8 *data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 max_keys;
	bcm_xtlv_t *rsp_xtlv = (bcm_xtlv_t *)rsp;
	BCM_REFERENCE(bsscfg);

	if (rsp_max < (rsp->data + sizeof(max_keys))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	max_keys = (uint32)km_get_max_keys(km);
	bcm_xtlv_pack_xtlv(rsp_xtlv, WL_WSEC_INFO_MAX_KEYS, (uint16)sizeof(max_keys),
		(uint8 *)&max_keys, 0);

done:
	return err;
}

static int
km_iov_get_wsec_info_bss_key_len(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const uint8 *req_data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 key_len;
	wlc_key_info_t key_info;
	bcm_xtlv_t *rsp_xtlv = (bcm_xtlv_t *)rsp;

	if (rsp_max < (rsp->data + sizeof(key_len))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	wlc_keymgmt_get_bss_tx_key(km, bsscfg, FALSE, &key_info);
	key_len = (uint32)key_info.key_len;
	bcm_xtlv_pack_xtlv(rsp_xtlv, WL_WSEC_INFO_BSS_KEY_LEN, (uint16)sizeof(key_len),
		(uint8 *)&key_len, 0);

done:
	return err;
}

static int
km_iov_get_wsec_info_bss_algo(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
	const uint8 *data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	bcm_xtlv_t *rsp_xtlv = (bcm_xtlv_t *)rsp;
	uint32 key_algo;
	if (rsp_max < (rsp->data + sizeof(key_algo))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	key_algo = (uint32)wlc_keymgmt_get_bss_key_algo(km, bsscfg, FALSE);
	bcm_xtlv_pack_xtlv(rsp_xtlv, WL_WSEC_INFO_BSS_ALGO, (uint16)sizeof(key_algo),
		(uint8 *)&key_algo, 0);

done:
	return err;
}

static int
km_iov_get_wsec_info_tx_key_id(keymgmt_t *km, wlc_bsscfg_t *bsscfg,
    const uint8* data, uint16 req_len, wl_wsec_info_tlv_t *rsp, uint8 *rsp_max)
{
	int err = BCME_OK;
	uint32 key_id;
	bcm_xtlv_t *rsp_xtlv = (bcm_xtlv_t *)rsp;

	if (rsp_max < (rsp->data + sizeof(key_id))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	key_id = (uint32)wlc_keymgmt_get_bss_tx_key_id(km, bsscfg, FALSE);
	bcm_xtlv_pack_xtlv(rsp_xtlv, WL_WSEC_INFO_BSS_ALGO, (uint16)sizeof(key_id),
		(uint8 *)&key_id, 0);
done:
	return err;
}

int
BCMATTACHFN(km_module_register)(keymgmt_t *km)
{
	return wlc_module_register(km->wlc->pub, km_iovars, KM_MODULE_NAME, km,
		km_doiovar, NULL /* wdog */, km_wlc_up, km_wlc_down);
}

#ifdef PSTA
static int
km_iov_check_psta_replay(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	int err = BCME_REPLAY;
	wlc_bsscfg_t *pri_bsscfg = NULL;
	scb_t *pri_scb = NULL;
	scb_t *scb = NULL;
	struct scb_iter scbiter;
	pri_bsscfg = bsscfg->wlc->cfg;
	if (pri_bsscfg == NULL) {
		KM_ERR(("wl%d: %s:%d Primary bsscfg is NULL\n",
			KM_UNIT(km), __FUNCTION__, __LINE__));
		return err;
	}
	FOREACH_BSS_SCB(km->wlc->scbstate, &scbiter, pri_bsscfg, scb) {
		if (scb->bsscfg == pri_bsscfg) {
			pri_scb = scb;
			break;
		}
	}
	if (pri_scb == NULL) {
		KM_ERR(("wl%d: %s:%d Primary bsscfg scb not found\n",
			KM_UNIT(km), __FUNCTION__, __LINE__));
		return err;
	}
	if ((bsscfg != pri_bsscfg) /* !primary */ &&
			SCB_AUTHORIZED(pri_scb)) {
		KM_LOG(("wl%d: %s: Ignore replay error in psta cases, "
				"if primary is already authorized\n",
				KM_UNIT(km), __FUNCTION__));
		err = BCME_OK;
	}
	return err;
}
#endif /* PSTA */
