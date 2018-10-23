/*
 * Key Management Module Implementation - ioctl support
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
 * $Id: km_ioctl.c 660347 2016-09-20 07:21:58Z $
 */

#include "km_pvt.h"
#include <wlc_iocv.h>

/* internal interface */

#define KM_IOCTL_IOCF_FLAGS (WLC_IOCF_OPEN_ALLOW)

static const wlc_ioctl_cmd_t km_ioctls[] = {
	{WLC_GET_KEY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_KEY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_GET_KEY_SEQ, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_GET_KEY_PRIMARY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_KEY_PRIMARY, KM_IOCTL_IOCF_FLAGS, sizeof(int)},
	{WLC_SET_WSEC_TEST, KM_IOCTL_IOCF_FLAGS, sizeof(int)}
};

static int km_ioctl(void *ctx, uint32 cmd, void *arg, uint len, struct wlc_if *wlcif);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static int
km_ioctl(void *ctx, uint32 cmd, void *arg, uint len, struct wlc_if *wlcif)
{
	keymgmt_t *km = (wlc_keymgmt_t*)ctx;
	wlc_info_t *wlc;
	int err = BCME_OK;
	int val;
	int *pval;
	wlc_bsscfg_t *bsscfg;
	wlc_key_id_t key_id;

	KM_DBG_ASSERT(KM_VALID(km)); /* wlcif may be NULL (primary) */
	wlc = km->wlc;
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	KM_DBG_ASSERT(bsscfg != NULL);

	pval = (int *)arg;
	if (pval != NULL && (uint32)len >= sizeof(val))
		memcpy(&val, pval, sizeof(val));
	else
		val = 0;

	KM_TRACE(("wl%d.%d: %s: cmd %d, val 0x%x (%d) len %d\n",
		WLCWLUNIT(wlc), WLC_BSSCFG_IDX(bsscfg), __FUNCTION__,
		cmd, val, val, len));

	switch (cmd) {
	case WLC_SET_KEY:
		err = km_doiovar_wrapper(km, IOV_SVAL(IOV_WSEC_KEY),
			NULL, 0, arg, len, 0, wlcif);
		break;

#if defined(BCMDBG)
	case WLC_GET_KEY: {
		wl_wsec_key_t wl_key;

		if ((size_t)len < sizeof(wl_key)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		memset(&wl_key, 0, sizeof(wl_key));
		wl_key.index = val;
		err = km_doiovar_wrapper(km, IOV_GVAL(IOV_WSEC_KEY), NULL, 0,
			&wl_key, sizeof(wl_key), 0, wlcif);
		if (err != BCME_OK)
			break;

		memcpy(arg, &wl_key, sizeof(wl_key));
		break;
	}
#endif // endif

#ifdef BCMDBG
	case WLC_SET_WSEC_TEST: {
		wl_wsec_key_t wl_key;
		wlc_key_t *key;
		wlc_key_info_t key_info;
		wlc_key_flags_t key_flags;

		if (!wlc->pub->up)  {
			err = BCME_NOTUP;
			break;
		}

		memcpy(&wl_key, ((uchar*)arg)+sizeof(val), sizeof(wl_key));

		key = wlc_keymgmt_get_key_by_addr(wlc->keymgmt, bsscfg, &wl_key.ea,
			wl_key.flags & WL_IBSS_PEER_GROUP_KEY, &key_info);

		key_flags = key_info.flags;
		switch (val) {
		case WSEC_GEN_MIC_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MIC_ERR;
			break;
		case WSEC_GEN_REPLAY:
			key_flags |= WLC_KEY_FLAG_GEN_REPLAY;
			break;
		case WSEC_GEN_ICV_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_ICV_ERR;
			break;
		case WSEC_GEN_MFP_ACT_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_ACT_ERR;
			break;
		case WSEC_GEN_MFP_DISASSOC_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_DISASSOC_ERR;
			break;
		case WSEC_GEN_MFP_DEAUTH_ERROR:
			key_flags |= WLC_KEY_FLAG_GEN_MFP_DEAUTH_ERR;
			break;
		default:
			err = BCME_RANGE;
			break;
		}

		if (err != BCME_OK)
			break;

		err = wlc_key_set_flags(key, key_flags);
		break;
	}
#endif /* BCMDBG */
	case WLC_GET_KEY_SEQ:
		err = km_doiovar_wrapper(km, IOV_GVAL(IOV_WSEC_KEY_SEQ), &val, sizeof(val),
			arg, len, 0, wlcif);
		break;
	case WLC_GET_KEY_PRIMARY:
		if ((uint)len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		key_id = wlc_keymgmt_get_bss_tx_key_id(wlc->keymgmt, bsscfg, FALSE);
		if (pval != NULL)
			*pval = key_id == val ? TRUE : FALSE;
		else
			err = BCME_BADARG;

		break;
	case WLC_SET_KEY_PRIMARY:
		if ((uint)len < sizeof(val)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		err = wlc_keymgmt_set_bss_tx_key_id(wlc->keymgmt, bsscfg, (wlc_key_id_t)val, FALSE);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	if (err != BCME_OK) {
		if (VALID_BCMERROR(err))
			wlc->pub->bcmerror = err;
	}
	return err;
}

/* public interface */
int
BCMATTACHFN(km_register_ioctl)(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return wlc_module_add_ioctl_fn(KM_PUB(km), km, km_ioctl,
		ARRAYSIZE(km_ioctls), km_ioctls);
}

int
BCMATTACHFN(km_unregister_ioctl)(keymgmt_t *km)
{
	KM_DBG_ASSERT(KM_VALID(km));
	return wlc_module_remove_ioctl_fn(KM_PUB(km), km);
}
