/*
 * Key Management Module Implementation - b4m4 support
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
 * $Id: km_b4m4.c 654771 2016-08-16 13:04:43Z $
 */

#include "km_pvt.h"

#ifdef STA

static void
km_b4m4_install_keys(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;
	int i;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);
	if (!KM_BSSCFG_B4M4_ENABLED(bss_km))
		return;

	for (i = 0; i < KM_B4M4_NUM_KEYS; ++i) {
		wl_wsec_key_t *wl_key;
		int err;

		if (bss_km->b4m4_keys[i] == NULL)
			continue;

		wl_key = bss_km->b4m4_keys[i];
		err = km_doiovar_wrapper(km, IOV_SVAL(IOV_WSEC_KEY),
			NULL, 0, wl_key, sizeof(*wl_key), 0, bsscfg->wlcif);

		if (err != BCME_OK)
			KM_ERR(("wl%d: %s: error %d installing %s key\n",
				KM_UNIT(km), __FUNCTION__, err,
				((i == KM_B4M4_PAIRWISE_KEY_ID) ? "pairwise" : "group")));
		else
			KM_LOG(("wl%d: %s: installed %s key\n",
				KM_UNIT(km), __FUNCTION__,
				((i == KM_B4M4_PAIRWISE_KEY_ID) ? "pairwise" : "group")));
	}

	/* done with the keys */
	km_b4m4_reset_keys(km, bsscfg);
}

#endif /* STA */

/* public interface */

bool
wlc_keymgmt_b4m4_enabled(keymgmt_t *km, struct wlc_bsscfg *bsscfg)
{
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	return ((bss_km->flags & KM_BSSCFG_FLAG_B4M4) != 0);
}

#ifdef STA
void
km_b4m4_set(keymgmt_t *km, wlc_bsscfg_t *bsscfg, bool enable)
{
	km_bsscfg_t *bss_km;
	bool prev_enable;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	prev_enable = ((bss_km->flags & KM_BSSCFG_FLAG_B4M4) != 0);
	if ((enable && prev_enable) || (!enable && !prev_enable))
		goto done;

	if (enable) {
		bss_km->flags |= KM_BSSCFG_FLAG_B4M4;
		KM_ASSERT(bss_km->b4m4_keys[0] == NULL); /* partial sanity check */
	} else {
		km_b4m4_reset_keys(km, bsscfg); /* start fresh */
		bss_km->flags &= ~KM_BSSCFG_FLAG_B4M4;
	}

done:
	KM_LOG(("wl%d.%d: %s: enable %d previous %d\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, enable,
		prev_enable));
}

void
km_b4m4_reset_keys(keymgmt_t *km, wlc_bsscfg_t *bsscfg)
{
	km_bsscfg_t *bss_km;
	int i;
	KM_LOG_DECL(char eabuf[ETHER_ADDR_STR_LEN]);

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	if (!KM_BSSCFG_B4M4_ENABLED(bss_km))
		return;

	for (i = 0; i < KM_B4M4_NUM_KEYS; ++i) {
		if (bss_km->b4m4_keys[i] != NULL) {
			wl_wsec_key_t *wl_key = bss_km->b4m4_keys[i];
			KM_LOG(("wl%d.%d: %s: resetting key id %02d addr %s\n",
				KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
				wl_key->index, bcm_ether_ntoa(&wl_key->ea, eabuf)));

			MFREE(KM_OSH(km), wl_key, sizeof(*wl_key));
			bss_km->b4m4_keys[i] = NULL;
		}
	}

	KM_LOG(("wl%d.%d: %s: reset keys\n", KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
}

int
km_b4m4_buffer_key(keymgmt_t *km, wlc_bsscfg_t *bsscfg, const wl_wsec_key_t *wl_key)
{
	km_bsscfg_t *bss_km;
	int err = BCME_NOTENABLED;
	size_t pos;
	wl_wsec_key_t *b4m4_key;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL && wl_key != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	do {
		if (!KM_BSSCFG_B4M4_ENABLED(bss_km))
			break;

		/* only buffer between M1 and M4 */
		if (!KM_BSSCFG_M1RX_DONE(bss_km) || KM_BSSCFG_M4TX_DONE(bss_km))
			break;

		pos =  ETHER_ISNULLADDR(&wl_key->ea) ? KM_B4M4_GROUP_KEY_ID :
			KM_B4M4_PAIRWISE_KEY_ID;

		b4m4_key = bss_km->b4m4_keys[pos];
		if (b4m4_key == NULL) {
			b4m4_key = (wl_wsec_key_t *) MALLOCZ(KM_OSH(km), sizeof(wl_wsec_key_t));
			if (b4m4_key == NULL) {
				err = BCME_NOMEM;
				break;
			}
			bss_km->b4m4_keys[pos] = b4m4_key;
		}

		*b4m4_key = *wl_key;
		err = BCME_OK;
	} while (0);

	KM_LOG(("wl%d: %s: b4m4 status %d, %sbuffering key, algo %d len %d\n",
		KM_UNIT(km), __FUNCTION__, err, ((err != BCME_OK) ? "not " : ""),
		wl_key->algo, wl_key->len));
	return err;
}

static void
km_b4m4_m4cb(wlc_info_t *wlc, uint tx_status, void *arg)
{
	km_bsscfg_t *bss_km;
	wlc_bsscfg_t *bsscfg;
	wlc_keymgmt_t *km;

	/* if no ack is received, skip installing keys */
	if (!(tx_status  & TX_STATUS_ACK_RCV))
		goto done;

	km = wlc->keymgmt;
	bsscfg = (wlc_bsscfg_t *)arg;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);

	bss_km = KM_BSSCFG(km, bsscfg);
	bss_km->flags |= KM_BSSCFG_FLAG_M4TX;
	bss_km->flags &= ~KM_BSSCFG_FLAG_M1RX;

	km_b4m4_install_keys(wlc->keymgmt, bsscfg);
done:;
}

void
km_b4m4_notify(keymgmt_t *km, wlc_keymgmt_notif_t notif,
	wlc_bsscfg_t *bsscfg, scb_t *scb, wlc_key_t *key, void *pkt)
{
	km_bsscfg_t *bss_km;
	int err;
	BCM_REFERENCE(scb);
	BCM_REFERENCE(key);

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);

	switch (notif) {
	case WLC_KEYMGMT_NOTIF_M1_RX:
		bss_km->flags |= KM_BSSCFG_FLAG_M1RX;
		bss_km->flags &= ~KM_BSSCFG_FLAG_M4TX;
		km_b4m4_reset_keys(km, bsscfg);
		break;
	case WLC_KEYMGMT_NOTIF_M4_TX:
		err = wlc_pcb_fn_register(KM_PCB(km), km_b4m4_m4cb, bsscfg, pkt);
		if (err != BCME_OK) {
			KM_REGST_ERR(("wl%d: %s: error %d registering packet callback\n",
				KM_UNIT(km), __FUNCTION__, err));
			km_b4m4_m4cb(km->wlc, TX_STATUS_ACK_RCV, bsscfg);
		}
		break;
	default:
		KM_DBG_ASSERT(!"unsupported b4m4 notification!");
		break;
	}
}
#endif /* STA */
