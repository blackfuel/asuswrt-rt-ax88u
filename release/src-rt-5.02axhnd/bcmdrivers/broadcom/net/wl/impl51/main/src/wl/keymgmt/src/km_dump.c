/*
 * Key Management Module Implementation - dump support
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
 * $Id: km_dump.c 714281 2017-08-03 22:00:04Z $
 */

#include "km_pvt.h"

#if defined(BCMDBG)

static int
km_dump_swkeys(keymgmt_t *km, struct bcmstrbuf *b)
{
	wlc_key_index_t key_idx;

	KM_DBG_ASSERT(KM_VALID(km));

	bcm_bprintf(b, "begin wl%d s/w key dump:\n", KM_UNIT(km));
	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key;
		wlc_key_info_t key_info;

		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;

		wlc_key_get_info(km_pvt_key->key, &key_info);
		if (key_info.algo == CRYPTO_ALGO_OFF)
			continue;

		bcm_bprintf(b, "begin wlc_key@%d\n", key_idx);
		km_key_dump(km_pvt_key->key, b);
		bcm_bprintf(b, "end wlc_key@%d\n", key_idx);
	}
	bcm_bprintf(b, "end s/w key dump\n");
	return BCME_OK;
}

static int
km_dump_hwkeys(keymgmt_t *km, struct bcmstrbuf *b)
{
	KM_DBG_ASSERT(KM_VALID(km));
	if (!km->wlc->clk)
		return BCME_NOCLK;

	km_hw_dump(km->hw, b, KM_KEY_DUMP_HW_KEYS);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_HW_KEYS);
#endif // endif
	return BCME_OK;
}

static int
km_dump_secalgo(keymgmt_t *km, struct bcmstrbuf *b)
{
	KM_DBG_ASSERT(KM_VALID(km));
	if (!km->wlc->clk)
		return BCME_NOCLK;

	km_hw_dump(km->hw, b, KM_KEY_DUMP_SECALGO);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_SECALGO);
#endif // endif
	return BCME_OK;
}

static int
km_dump(keymgmt_t *km, struct bcmstrbuf *b)
{
	wlc_key_index_t key_idx;
	char eabuf[ETHER_ADDR_STR_LEN];

	KM_DBG_ASSERT(KM_VALID(km));

	bcm_bprintf(b, "begin wl%d keymgmt dump:\n", KM_UNIT(km));

	bcm_bprintf(b, "\tmagic: %08x\n", km->magic);
	bcm_bprintf(b, "\twlc: %p\n", km->wlc);
	bcm_bprintf(b, "\thw: %p\n", km->hw);
	bcm_bprintf(b, "\tflags: 0x%04x\n", km->flags);
	bcm_bprintf(b, "\th_bsscfg: %d\n", km->h_bsscfg);
	bcm_bprintf(b, "\th_scb: %d\n", km->h_scb);
	bcm_bprintf(b, "\th_notif: %p\n", km->h_notif);

	for (key_idx = 0; key_idx < km->max_keys; ++key_idx) {
		km_pvt_key_t *km_pvt_key;
		km_pvt_key = &km->keys[key_idx];
		if (!KM_VALID_KEY(km_pvt_key))
			continue;
		bcm_bprintf(b, "begin km_pvt_key@%d\n", key_idx);

		bcm_bprintf(b, "\tkey: %p\n", km_pvt_key->key);
		bcm_bprintf(b, "\tflags: 0x%04x\n", km_pvt_key->flags);
		if (km_pvt_key->flags & KM_FLAG_SCB_KEY)
			bcm_bprintf(b, "\tscb@%p: %s in band %u\n", km_pvt_key->u.scb,
				bcm_ether_ntoa(&km_pvt_key->u.scb->ea, eabuf),
				km_pvt_key->u.scb->bandunit);
		if (km_pvt_key->flags & KM_FLAG_BSS_KEY)
			bcm_bprintf(b, "\tbss@%p bssidx %d\n",
				km_pvt_key->u.bsscfg, WLC_BSSCFG_IDX(km_pvt_key->u.bsscfg));
		bcm_bprintf(b, "\tkey algo: %d\n", km_pvt_key->key_algo);
		bcm_bprintf(b, "end km_pvt_key@%d\n", key_idx);
	}

	bcm_bprintf(b, "begin km stats\n");
	bcm_bprintf(b, "\tnum def bss wep: %d\n", km->stats.num_def_bss_wep);
	bcm_bprintf(b, "\tnum sw keys (algo != none): %d\n", km->stats.num_sw_keys);
	bcm_bprintf(b, "\tnum (km) hw keys: %d\n", km->stats.num_hw_keys);
	bcm_bprintf(b, "\tnum num bss up: %d\n", km->stats.num_bss_up);
	bcm_bprintf(b, "\tnum num pktfetch: %d\n", km->stats.num_pkt_fetch);
	bcm_bprintf(b, "end km stats\n");

	km_dump_swkeys(km, b);
	km_hw_dump(km->hw, b, KM_KEY_DUMP_ALL);
#ifdef WOWL
	km_hw_dump(km->wowl_hw, b, KM_KEY_DUMP_ALL);
#endif // endif
#ifdef BCMAPIVTW
	km_ivtw_dump(km->ivtw, b);
#endif // endif

	bcm_bprintf(b, "end keymgmt dump\n");
	return BCME_OK;
}

/* public interface */
void
km_bsscfg_dump(void *ctx, wlc_bsscfg_t *bsscfg, struct bcmstrbuf *b)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_bsscfg_t *bss_km;
	int i;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);

	bcm_bprintf(b, "\twsec: 0x%08x\n", bss_km->wsec);
	for (i = 0; i < WLC_KEYMGMT_NUM_GROUP_KEYS; ++i) {
		bcm_bprintf(b, "\tdef_key_idx[%d]: %d\n", i, bss_km->key_idx[i]);
	}
	bcm_bprintf(b, "\ttx_key_id: %d\n", bss_km->tx_key_id);
	bcm_bprintf(b, "\tflags: %02x\n", bss_km->flags);

#ifdef MFP
	bcm_bprintf(b, "\tigtk_tx_key_id: %d\n", bss_km->igtk_tx_key_id);
	for (i = 0; i < WLC_KEYMGMT_NUM_BSS_IGTK; ++i) {
		bcm_bprintf(b, "\tigtk_key_idx[%d]: %d\n", i, bss_km->igtk_key_idx[i]);
	}
#endif // endif

	/* Is this duplicate? */
	bcm_bprintf(b, "\ttkip_countermeasures: %d\n", wlc_keymgmt_tkip_cm_enabled(km, bsscfg));

	bcm_bprintf(b, "\ttkip_cm_detected: %d\n", bss_km->tkip_cm_detected);
	bcm_bprintf(b, "\ttkip_cm_blocked: %d\n", bss_km->tkip_cm_blocked);
	bcm_bprintf(b, "\talgo (cached): %d\n", bss_km->algo);
	bcm_bprintf(b, "\tscb key idx: %d\n", bss_km->scb_key_idx);
	bcm_bprintf(b, "\tamt idx: %d\n", bss_km->amt_idx);
}

void
km_scb_dump(void *ctx, scb_t *scb, struct bcmstrbuf *b)
{
	keymgmt_t *km = (keymgmt_t *)ctx;
	km_scb_t *scb_km;

	KM_DBG_ASSERT(KM_VALID(km) && scb != NULL);
	scb_km = KM_SCB(km, scb);

	bcm_bprintf(b, "\tflags 0x%04x\n", scb_km->flags);
	bcm_bprintf(b, "\tkey_idx %d\n", scb_km->key_idx);
	bcm_bprintf(b, "\tamt_idx %d\n", scb_km->amt_idx);
}

int
BCMATTACHFN(km_register_dump)(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));

#define REG(_km, _name, _func) (void)wlc_dump_register((_km)->wlc->pub, \
	(_name), (dump_fn_t)(_func), (void *)(_km))
	REG(km, KM_MODULE_NAME, km_dump);
	REG(km, "hwkeys", km_dump_hwkeys);
	REG(km, "swkeys", km_dump_swkeys);
	REG(km, "secalgo", km_dump_secalgo);
#undef REG
	return BCME_OK;
};

#endif // endif
