/*
 * Key Management Module Implementation - tkip support
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
 * $Id: km_tkip.c 716152 2017-08-16 20:49:30Z $
 */

#include "km_pvt.h"
#include <wlc_tx.h>

/* allow mic errors no more often than once in 60 sec */
static const km_time_t KM_TKIP_MIC_ERROR_MIN_INTVL_SEC = 60;
static const km_time_t KM_TKIP_CM_BLOCK_SEC = 60;

/* internal interface */
static void
km_tkip_deauth(keymgmt_t *km, wlc_bsscfg_t *bsscfg, scb_t *scb)
{
	wlc_info_t *wlc = km->wlc;

	KM_DBG_ASSERT(BSSCFG_STA(bsscfg));
	if (!BSSCFG_STA(bsscfg))
		return;

	wlc_senddeauth(wlc, bsscfg, scb, &bsscfg->BSSID, &bsscfg->BSSID,
		&bsscfg->cur_etheraddr, DOT11_RC_MIC_FAILURE);
	wlc_scb_disassoc_cleanup(wlc, scb);
	wlc_scb_clearstatebit(wlc, scb, AUTHENTICATED | ASSOCIATED | AUTHORIZED);
	wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &bsscfg->BSSID,
		DOT11_RC_MIC_FAILURE, DOT11_BSSTYPE_INFRASTRUCTURE);
#ifdef STA
	wlc_bss_clear_bssid(bsscfg); /* force STA roam */
#endif /* STA */
}

/* public interface */

void
km_tkip_mic_error(keymgmt_t *km, void *pkt, wlc_key_t *key,
	const wlc_key_info_t *key_info)
{
	km_time_t last_detect;
	wlc_bsscfg_t *bsscfg = NULL;
	km_pvt_key_t *km_pvt_key;
	km_bsscfg_t *bss_km;
	BCM_REFERENCE(pkt);

	KM_DBG_ASSERT(KM_VALID(km));

	KM_DBG_ASSERT(key_info->algo == CRYPTO_ALGO_TKIP &&
		key_info->key_idx != WLC_KEY_INDEX_INVALID &&
		KM_VALID_KEY_IDX(km, key_info->key_idx));

	/* no tracking mic errors for linux crypto */
	if (WLC_KEY_IS_LINUX_CRYPTO(key_info))
		return;

	km_pvt_key = &km->keys[key_info->key_idx];

	if (km_pvt_key->flags & KM_FLAG_SCB_KEY) {
		scb_t *scb = km_pvt_key->u.scb;
		KM_DBG_ASSERT(scb != NULL);
		bsscfg = SCB_BSSCFG(scb);
	} else if (km_pvt_key->flags & KM_FLAG_BSS_KEY) {
		bsscfg = km_pvt_key->u.bsscfg;
	}

	KM_DBG_ASSERT(bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);

	last_detect = bss_km->tkip_cm_detected;
	bss_km->tkip_cm_detected = (km_time_t)KM_PUB(km)->now;
	if ((bss_km->tkip_cm_detected - last_detect) <=
		KM_TKIP_MIC_ERROR_MIN_INTVL_SEC) {
		bss_km->tkip_cm_blocked = bss_km->tkip_cm_detected +
			KM_TKIP_CM_BLOCK_SEC;
		KM_ERR(("wl%d.%d: %s: TKIP countermeasures enabled from now for %d seconds\n",
			KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
			bss_km->tkip_cm_blocked - bss_km->tkip_cm_detected));

		/* STA: clear txq */
		if (BSSCFG_STA(bsscfg)) {
			if (KM_BSSCFG_WIN7PLUS(KM_PUB(km), bsscfg))
				wlc_txq_pktq_cfg_filter(km->wlc,
					WLC_GET_CQ(bsscfg->wlcif->qi), bsscfg);
			else
				cpktq_flush(km->wlc, &bsscfg->wlcif->qi->cpktq);
		}

		km_event_signal(km, WLC_KEYMGMT_EVENT_TKIP_CM_ACTIVE,
			bsscfg, key, NULL);
	}

	WLCNTINCR(KM_CNT(km)->tkipcntrmsr);
	if (key_info->flags & (WLC_KEY_FLAG_IBSS_PEER_GROUP|WLC_KEY_FLAG_GROUP))
		WLCNTINCR(KM_CNT(km)->tkipcntrmsr_mcst);
}

void km_tkip_cm_reported(keymgmt_t *km, wlc_bsscfg_t *bsscfg, scb_t *scb)
{
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km));

	bss_km = KM_BSSCFG(km, bsscfg);

	/* ignore if we are not CM blocked */
	if (!KM_BEFORE_NOW(km, bss_km->tkip_cm_blocked))
		return;

	/* On a STA, deauthenticate. On AP, the authenticator is
	 * responsible for deauthenticating STAs when countermeasures
	 * are in effect
	 */
	if (BSSCFG_STA(bsscfg))
		km_tkip_deauth(km, bsscfg, scb);
}

bool
wlc_keymgmt_tkip_cm_enabled(wlc_keymgmt_t *km,
    const struct wlc_bsscfg *bsscfg)
{
	const km_bsscfg_t *bss_km;
	bool cm = FALSE;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	if (!bsscfg->BSS) /* No countermeasures in IBSS */
		goto done;

	bss_km = KM_CONST_BSSCFG(km, bsscfg);
	if (KM_AFTER_NOW(km, bss_km->tkip_cm_blocked))
		cm = TRUE;

done:
	return cm;
}

void
wlc_keymgmt_tkip_set_cm(wlc_keymgmt_t *km,
    struct wlc_bsscfg *bsscfg, bool enable)
{
	km_bsscfg_t *bss_km;

	KM_DBG_ASSERT(KM_VALID(km) && bsscfg != NULL);
	bss_km = KM_BSSCFG(km, bsscfg);
	if (!bsscfg->BSS) /* No countermeasures in IBSS */
		goto done;

	if (enable) {
		bss_km->tkip_cm_detected = KM_PUB(km)->now;
		bss_km->tkip_cm_blocked = bss_km->tkip_cm_detected +
			KM_TKIP_CM_BLOCK_SEC;
		km_event_signal(km, WLC_KEYMGMT_EVENT_TKIP_CM_ACTIVE, bsscfg,
			NULL, NULL);
	} else {
		/* one should not have to disable, as after the blocking interval
		 * passes, they should be.
		 */
		if (KM_AFTER_NOW(km, bss_km->tkip_cm_blocked))
			bss_km->tkip_cm_blocked = KM_PUB(km)->now;
	}

done:
	KM_LOG(("wl%d.%d: %s: TKIP CM enabled until %d\n",
		KM_UNIT(km), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__, bss_km->tkip_cm_blocked));
}
