/*
 *   bcmwpa.c - shared WPA-related functions
 *
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
 *
 * $Id: bcmwpa.c 722778 2017-09-21 09:58:38Z $
 */

#include <bcm_cfg.h>

/* include wl driver config file if this file is compiled for driver */
#ifdef BCMDRIVER
#include <osl.h>
#elif defined(BCMEXTSUP)
#include <string.h>
#include <bcm_osl.h>
#else
#include <string.h>
#endif /* BCMDRIVER */

#include <ethernet.h>
#include <eapol.h>
#include <802.11.h>
#include <wpa.h>
#include <802.11r.h>

#include <bcmutils.h>
#include <bcmendian.h>
#include <bcmwpa.h>
#include <sha2.h>
#include <aeskeywrap.h>

#include <wlioctl.h>

#include <bcmutils.h>
#include <bcmwpa.h>

#if defined(BCMSUP_PSK) || defined(WLFBT) || defined(BCMAUTH_PSK) || defined(WL_OKC) || \
	defined(WLTDLS) || defined(GTKOE) || defined(WLHOSTFBT)
#ifdef WLHOSTFBT
#include <string.h>
#endif // endif
#endif /* defined(BCMSUP_PSK) || defined(WLFBT) || defined(BCMAUTH_PSK) ||
	* defined(WL_OKC) || defined(WLTDLS) || defined(GTKOE) || defined(WLHOSTFBT)
	*/

/* prefix strings */
#define PMK_NAME_PFX "PMK Name"
#define FT_PTK_PFX "FT-PTK"
#define FT_R0_PFX "FT-R0"
#define FT_R0N_PFX "FT-R0N"
#define FT_R1_PFX "FT-R1"
#define FT_R1N_PFX "FT-R1N"
#define WPA_PTK_PFX "Pairwise key expansion"
#define TDLS_PMK_PFX "TDLS PMK"
/* end prefix strings */

#if defined(BCMSUP_PSK) || defined(WLFBT) || defined(WL_OKC) || defined(WLHOSTFBT)
#include <rc4.h>

/* calculate wpa PMKID: HMAC-SHA1-128(PMK, "PMK Name" | AA | SPA) */
static void
wpa_calc_pmkid_impl(sha2_hash_type_t hash_type,
	const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *pmk, int pmk_len, uint8 *pmkid)
{
	int err;
	hmac_sha2_ctx_t ctx;

	err = hmac_sha2_init(&ctx, hash_type, pmk, pmk_len);
	if (err != BCME_OK)
		goto done;
	hmac_sha2_update(&ctx, (const uint8 *)PMK_NAME_PFX, sizeof(PMK_NAME_PFX) - 1);
	hmac_sha2_update(&ctx, (const uint8 *)auth_ea, ETHER_ADDR_LEN);
	hmac_sha2_update(&ctx, (const uint8 *)sta_ea, ETHER_ADDR_LEN);
	hmac_sha2_final(&ctx, pmkid, WPA2_PMKID_LEN);
done:;
}

void
wpa_calc_pmkid(const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *pmk, int pmk_len, uint8 *pmkid)
{
	wpa_calc_pmkid_impl(HASH_SHA1, auth_ea, sta_ea, pmk, pmk_len, pmkid);
}

void
kdf_calc_pmkid(const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *pmk, int pmk_len, uint8 *pmkid)
{
	wpa_calc_pmkid_impl(HASH_SHA256, auth_ea, sta_ea, pmk, pmk_len, pmkid);
}

#if defined(WLFBT) || defined(WLHOSTFBT)
void
wpa_calc_pmkR0(const uint8 *ssid, int ssid_len, uint16 mdid,
	const uint8 *r0kh, uint r0kh_len, const struct ether_addr *sta_ea,
	const uint8 *pmk, uint pmk_len, uint8 *pmkr0, uint8 *pmkr0name)
{
	uint8 out[FBT_R0KH_ID_LEN - 1];
	int out_len = sizeof(out);
	bcm_const_xlvp_t pfx[7];
	bcm_const_xlvp_t pfx2[2];
	int npfx = 0;
	int npfx2 = 0;
	uint8 mdid_le[2];
	uint8 pfx_ssid_len;
	uint8 pfx_r0kh_len;

	/* create prefixes for pmkr0 */
	pfx[npfx].len = sizeof(FT_R0_PFX) - 1;
	pfx[npfx++].data = (uint8 *)FT_R0_PFX;

	/* ssid length and ssid */
	pfx_ssid_len = ssid_len & 0xff;
	pfx[npfx].len = (uint16)sizeof(pfx_ssid_len);
	pfx[npfx++].data = &pfx_ssid_len;

	pfx[npfx].len = (uint16)(ssid_len & 0xffff);
	pfx[npfx++].data = ssid;

	/* mdid */
	htol16_ua_store(mdid, mdid_le);
	pfx[npfx].len = sizeof(mdid_le);
	pfx[npfx++].data = mdid_le;

	/* r0kh len and r0kh */
	pfx_r0kh_len = r0kh_len & 0xff;
	pfx[npfx].len = sizeof(pfx_r0kh_len);
	pfx[npfx++].data = &pfx_r0kh_len;

	pfx[npfx].len = (uint16)(r0kh_len & 0xffff);
	pfx[npfx++].data = r0kh;

	/* sta addr */
	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *)sta_ea;

	hmac_sha2_n(HASH_SHA256, pmk, pmk_len, pfx, npfx, NULL, 0, out, out_len);
	memcpy(pmkr0, out, pmk_len);

	/* create prefixes for pmkr0 name */
	pfx2[npfx2].len = sizeof(FT_R0N_PFX) - 1;
	pfx2[npfx2++].data = (uint8 *)FT_R0N_PFX;
	pfx2[npfx2].len = WPA2_PMKID_LEN;
	pfx2[npfx2++].data = &out[pmk_len];

	(void)sha2(HASH_SHA256, pfx2, npfx2, NULL, 0, pmkr0name, WPA2_PMKID_LEN);
}

void
wpa_calc_pmkR1(const struct ether_addr *r1kh, const struct ether_addr *sta_ea,
	const uint8 *pmk, int pmk_len, const uint8 *pmkr0name, uint8 *pmkr1, uint8 *pmkr1name)
{
	bcm_const_xlvp_t pfx[3];
	bcm_const_xlvp_t pfx2[4];
	int npfx = 0;
	int npfx2 = 0;

	if (!pmkr1 && !pmkr1name)
		goto done;
	else if (!pmkr1)
		goto calc_r1name;

	/* create prefixes for pmkr1 */
	pfx[npfx].len = sizeof(FT_R1_PFX) - 1;
	pfx[npfx++].data = (uint8 *)FT_R1_PFX;

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *)r1kh;

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *)sta_ea;

	hmac_sha2_n(HASH_SHA256, pmk, pmk_len, pfx, npfx, NULL, 0,
		pmkr1, sha2_digest_len(HASH_SHA256));

calc_r1name:
	/* create prefixes for pmkr1 name */
	pfx2[npfx2].len = sizeof(FT_R1N_PFX) - 1;
	pfx2[npfx2++].data = (uint8 *)FT_R1N_PFX;

	pfx2[npfx2].len = WPA2_PMKID_LEN;
	pfx2[npfx2++].data = pmkr0name;

	pfx2[npfx2].len = ETHER_ADDR_LEN;
	pfx2[npfx2++].data = (const uint8 *)r1kh;

	pfx2[npfx2].len = ETHER_ADDR_LEN;
	pfx2[npfx2++].data = (const uint8 *)sta_ea;

	sha2(HASH_SHA256, pfx2, npfx2, NULL, 0, pmkr1name, WPA2_PMKID_LEN);
done:;
}

void
wpa_calc_ft_ptk(const struct ether_addr *bssid, const struct ether_addr *sta_ea,
	const uint8 *anonce, const uint8* snonce,
	const uint8 *pmk, int pmk_len, uint8 *ptk, int ptk_len)
{
	bcm_const_xlvp_t pfx[5];
	int npfx = 0;

	/* FT-PTK||SNONCE||ANONCE||BSSID||STA Addr */

	pfx[npfx].len = sizeof(FT_PTK_PFX) - 1;
	pfx[npfx++].data = (uint8 *)FT_PTK_PFX;

	pfx[npfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	pfx[npfx++].data = snonce;

	pfx[npfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	pfx[npfx++].data = anonce;

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *)bssid;

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *)sta_ea;

	hmac_sha2_n(HASH_SHA256, pmk, pmk_len, pfx, npfx, NULL, 0, ptk, ptk_len);
}

void
wpa_derive_pmkR1_name(struct ether_addr *r1kh, struct ether_addr *sta_ea,
	uint8 *pmkr0name, uint8 *pmkr1name)
{
	wpa_calc_pmkR1(r1kh, sta_ea, NULL /* pmk */, 0,
		pmkr0name, NULL /* pmkr1 */, pmkr1name);
}
#endif /* WLFBT || WLHOSTFBT */
#endif /* BCMSUP_PSK || WLFBT || WL_OKC */

#if defined(BCMSUP_PSK) || defined(GTKOE) || defined(BCMAUTH_PSK) || defined(WLFBT)
/* Decrypt a key data from a WPA key message */
bool
wpa_decr_key_data(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
                            uint8 *gtk, uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	uint16 len;

	switch (key_info & (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2)) {
	case WPA_KEY_DESC_V1:
		memcpy(encrkey, body->iv, WPA_MIC_KEY_LEN);
		memcpy(&encrkey[WPA_MIC_KEY_LEN], ekey, WPA_MIC_KEY_LEN);
		/* decrypt the key data */
		prepare_key(encrkey, WPA_MIC_KEY_LEN*2, rc4key);
		rc4(data, WPA_KEY_DATA_LEN_256, rc4key); /* dump 256 bytes */
		if (gtk)
			len = ntoh16_ua((uint8 *)&body->key_len);
		else
			len = ntoh16_ua((uint8 *)&body->data_len);
		rc4(body->data, len, rc4key);
		if (gtk)
			memcpy(gtk, body->data, len);
		break;

	case WPA_KEY_DESC_V2:
	case WPA_KEY_DESC_V3:
		len = ntoh16_ua((uint8 *)&body->data_len);
		if (!len || aes_unwrap(WPA_MIC_KEY_LEN, ekey, len, body->data,
		               gtk ? gtk : body->data)) {
			return FALSE;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/* internal function - assumes enouch space allocated, retuns written number */
static int
wpa_cacl_ptk_prefixes(const uint8 *prefix, int prefix_len,
	const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *anonce, const uint8 *snonce, bcm_const_xlvp_t *pfx)
{
	int npfx = 0;

	/* prefix || min ea || max ea || min nonce || max nonce */
	pfx[npfx].len = (uint16)(prefix_len & 0xffff);
	pfx[npfx++].data = prefix;

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *) wpa_array_cmp(MIN_ARRAY,
		(const uint8 *)auth_ea, (const uint8 *)sta_ea, ETHER_ADDR_LEN);

	pfx[npfx].len = ETHER_ADDR_LEN;
	pfx[npfx++].data = (const uint8 *) wpa_array_cmp(MAX_ARRAY,
		(const uint8 *)auth_ea, (const uint8 *)sta_ea, ETHER_ADDR_LEN);

	pfx[npfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	pfx[npfx++].data = (const uint8 *)wpa_array_cmp(MIN_ARRAY, snonce, anonce,
		EAPOL_WPA_KEY_NONCE_LEN);

	pfx[npfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	pfx[npfx++].data = (const uint8 *)wpa_array_cmp(MAX_ARRAY, snonce, anonce,
		EAPOL_WPA_KEY_NONCE_LEN);

	return npfx;
}

void
kdf_calc_ptk(const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *anonce, const uint8* snonce,
	const uint8 *pmk, uint pmk_len, uint8 *ptk, int ptk_len)
{
	bcm_const_xlvp_t pfx[5];
	int npfx;

	/* note: kdf omits trailing NULL in prefix */
	npfx = wpa_cacl_ptk_prefixes((uint8 *)WPA_PTK_PFX, sizeof(WPA_PTK_PFX) - 1,
		auth_ea, sta_ea, anonce, snonce, pfx);
	hmac_sha2_n(HASH_SHA256, pmk, pmk_len, pfx, npfx, NULL, 0, ptk, ptk_len);
}

/* Decrypt a group transient key from a WPA key message */
bool
wpa_decr_gtk(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
	uint8 *gtk, uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	return wpa_decr_key_data(body, key_info, ekey, gtk, data, encrkey, rc4key);
}
#endif	/* BCMSUP_PSK || GTKOE || BCMAUTH_PSK || WLFBT */

#if defined(BCMSUP_PSK) || defined(BCMAUTH_PSK) || defined(WLFBT) || defined(GTKOE)
/* Compute Message Integrity Code (MIC) over EAPOL message */
bool
wpa_make_mic(eapol_header_t *eapol, uint key_desc, uint8 *mic_key, uchar *mic)
{
	int data_len;
	int mic_len;
	int err = BCME_OK;

	/* length of eapol pkt from the version field on */
	data_len = 4 + ntoh16_ua((uint8 *)&eapol->length);
	mic_len = EAPOL_WPA_KEY_MIC_LEN;

	/* Create the MIC for the pkt */
	switch (key_desc) {
	case WPA_KEY_DESC_V1:
		err = hmac_sha2(HASH_MD5, mic_key, WPA_MIC_KEY_LEN, NULL, 0,
			(uint8 *)&eapol->version, data_len, mic, mic_len);
		break;
	case WPA_KEY_DESC_V2:
		/* note: transparent truncation to mic_len */
		err = hmac_sha2(HASH_SHA1, mic_key, WPA_MIC_KEY_LEN, NULL, 0,
			(uint8 *)&eapol->version, data_len, mic, mic_len);
		break;
	case WPA_KEY_DESC_V3:
		aes_cmac_calc(NULL, 0, &eapol->version, data_len, mic_key,
		          EAPOL_WPA_KEY_MIC_LEN, mic, AES_BLOCK_SZ);
		break;
	default:
		err = BCME_UNSUPPORTED;
	}

	return (err == BCME_OK);
}

void
wpa_calc_ptk(const struct ether_addr *auth_ea, const struct ether_addr *sta_ea,
	const uint8 *anonce, const uint8 *snonce, const uint8 *pmk, int pmk_len,
	uint8 *ptk, int ptk_len)
{
	bcm_const_xlvp_t pfx[5];
	int npfx;

	/* note: wpa needs trailing NULL in prefix */
	npfx = wpa_cacl_ptk_prefixes((uint8 *)WPA_PTK_PFX, sizeof(WPA_PTK_PFX),
		auth_ea, sta_ea, anonce, snonce, pfx);

	(void)hmac_sha2_n(HASH_SHA1, pmk, pmk_len, pfx, npfx, NULL, 0, ptk, ptk_len);
}

bool
wpa_encr_key_data(eapol_wpa_key_header_t *body, uint16 key_info, uint8 *ekey,
	uint8 *gtk,	uint8 *data, uint8 *encrkey, rc4_ks_t *rc4key)
{
	uint16 len;

	switch (key_info & (WPA_KEY_DESC_V1 | WPA_KEY_DESC_V2)) {
	case WPA_KEY_DESC_V1:
		if (gtk)
			len = ntoh16_ua((uint8 *)&body->key_len);
		else
			len = ntoh16_ua((uint8 *)&body->data_len);

		/* create the iv/ptk key */
		memcpy(encrkey, body->iv, 16);
		memcpy(&encrkey[16], ekey, 16);
		/* encrypt the key data */
		prepare_key(encrkey, 32, rc4key);
		rc4(data, WPA_KEY_DATA_LEN_256, rc4key); /* dump 256 bytes */
		rc4(body->data, len, rc4key);
		break;
	case WPA_KEY_DESC_V2: /* fall through */
	case WPA_KEY_DESC_V3:
		len = ntoh16_ua((uint8 *)&body->data_len);
		/* pad if needed - min. 16 bytes, 8 byte aligned */
		/* padding is 0xdd followed by 0's */
		if (len < 2*AKW_BLOCK_LEN) {
			body->data[len] = WPA2_KEY_DATA_PAD;
			memset(&body->data[len+1], 0, 2*AKW_BLOCK_LEN - (len+1));
			len = 2*AKW_BLOCK_LEN;
		} else if (len % AKW_BLOCK_LEN) {
			body->data[len] = WPA2_KEY_DATA_PAD;
			memset(&body->data[len+1], 0, AKW_BLOCK_LEN - ((len+1) % AKW_BLOCK_LEN));
			len += AKW_BLOCK_LEN - (len % AKW_BLOCK_LEN);
		}
		if (aes_wrap(WPA_MIC_KEY_LEN, ekey, len, body->data, body->data)) {
			return FALSE;
		}
		len += 8;
		hton16_ua_store(len, (uint8 *)&body->data_len);
		break;
	default:
		/* 11mc D8.0 key descriptor version 0 used */
		return FALSE;
	}

	return TRUE;
}

/* Check MIC of EAPOL message */
bool
wpa_check_mic(eapol_header_t *eapol, uint key_desc, uint8 *mic_key)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uchar digest[SHA2_MAX_DIGEST_LEN];
	uchar mic[EAPOL_WPA_KEY_MIC_LEN];

	/* save MIC and clear its space in message */
	memcpy(mic, &body->mic, EAPOL_WPA_KEY_MIC_LEN);
	memset(&body->mic, 0, EAPOL_WPA_KEY_MIC_LEN);

	if (!wpa_make_mic(eapol, key_desc, mic_key, digest)) {
		return FALSE;
	}
	return !memcmp(digest, mic, EAPOL_WPA_KEY_MIC_LEN);
}
#endif /* BCMSUP_PSK || BCMAUTH_PSK  || WLFBT || GTKOE */

#ifdef WLTDLS
void
wpa_calc_tpk(const struct ether_addr *init_ea, const struct ether_addr *resp_ea,
	const struct ether_addr *bssid, const uint8 *anonce, const uint8* snonce,
	uint8 *tpk, uint tpk_len)
{
	uint8 pmk[SHA2_MAX_DIGEST_LEN];
	int pmk_len;
	bcm_const_xlvp_t ikpfx[2];
	int nikpfx = 0;
	bcm_const_xlvp_t  tpkpfx[4];
	int ntpkpfx = 0;

	pmk_len = sha2_digest_len(HASH_SHA256);

	/* compute pmk to use - using anonce and snonce  - min and then max */
	ikpfx[nikpfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	ikpfx[nikpfx++].data = wpa_array_cmp(MIN_ARRAY, snonce, anonce,
	                    EAPOL_WPA_KEY_NONCE_LEN),

	ikpfx[nikpfx].len = EAPOL_WPA_KEY_NONCE_LEN;
	ikpfx[nikpfx++].data = wpa_array_cmp(MAX_ARRAY, snonce, anonce,
	                    EAPOL_WPA_KEY_NONCE_LEN),

	(void)sha2(HASH_SHA256, ikpfx, nikpfx, NULL, 0, pmk, SHA2_SHA256_DIGEST_LEN);

	/* compute the tpk - using prefix, min ea, max ea, bssid */
	tpkpfx[ntpkpfx].len = sizeof(TDLS_PMK_PFX) - 1;
	tpkpfx[ntpkpfx++].data = (const uint8 *)TDLS_PMK_PFX;

	tpkpfx[ntpkpfx].len = ETHER_ADDR_LEN;
	tpkpfx[ntpkpfx++].data = wpa_array_cmp(MIN_ARRAY, (const uint8 *)init_ea,
		(const uint8 *)resp_ea, ETHER_ADDR_LEN),

	tpkpfx[ntpkpfx].len = ETHER_ADDR_LEN;
	tpkpfx[ntpkpfx++].data = wpa_array_cmp(MAX_ARRAY, (const uint8 *)init_ea,
		(const uint8 *)resp_ea, ETHER_ADDR_LEN),

	tpkpfx[ntpkpfx].len = ETHER_ADDR_LEN;
	tpkpfx[ntpkpfx++].data = (const uint8 *)bssid;

	(void)hmac_sha2_n(HASH_SHA256, pmk, pmk_len, tpkpfx, ntpkpfx, NULL, 0, tpk, tpk_len);
}
#endif /* WLTDLS */

/* Convert WPA/WPA2 IE cipher suite to locally used value */
static bool
rsn_cipher(wpa_suite_t *suite, ushort *cipher, const uint8 *std_oui, bool wep_ok)
{
	bool ret = TRUE;

	if (!memcmp((const char *)suite->oui, std_oui, DOT11_OUI_LEN)) {
		switch (suite->type) {
		case WPA_CIPHER_TKIP:
			*cipher = CRYPTO_ALGO_TKIP;
			break;
		case WPA_CIPHER_AES_CCM:
			*cipher = CRYPTO_ALGO_AES_CCM;
			break;
		case WPA_CIPHER_AES_GCM:
			*cipher = CRYPTO_ALGO_AES_GCM;
			break;
		case WPA_CIPHER_AES_GCM256:
			*cipher = CRYPTO_ALGO_AES_GCM256;
			break;
		case WPA_CIPHER_WEP_40:
			if (wep_ok)
				*cipher = CRYPTO_ALGO_WEP1;
			else
				ret = FALSE;
			break;
		case WPA_CIPHER_WEP_104:
			if (wep_ok)
				*cipher = CRYPTO_ALGO_WEP128;
			else
				ret = FALSE;
			break;
		default:
			ret = FALSE;
			break;
		}
		return ret;
	}

	/* check for other vendor OUIs */
	return FALSE;
}

bool
wpa_cipher(wpa_suite_t *suite, ushort *cipher, bool wep_ok)
{
	return rsn_cipher(suite, cipher, (const uchar*)WPA_OUI, wep_ok);
}

bool
wpa2_cipher(wpa_suite_t *suite, ushort *cipher, bool wep_ok)
{
	return rsn_cipher(suite, cipher, (const uchar*)WPA2_OUI, wep_ok);
}

/* Is any of the tlvs the expected entry? If
 * not update the tlvs buffer pointer/length.
 */
bool
bcm_has_ie(uint8 *ie, uint8 **tlvs, uint *tlvs_len, const uint8 *oui, int oui_len, uint8 type)
{
	/* If the contents match the OUI and the type */
	if (ie[TLV_LEN_OFF] >= oui_len + 1 &&
	    !memcmp(&ie[TLV_BODY_OFF], oui, oui_len) &&
	    type == ie[TLV_BODY_OFF + oui_len]) {
		return TRUE;
	}

	/* point to the next ie */
	ie += ie[TLV_LEN_OFF] + TLV_HDR_LEN;
	/* calculate the length of the rest of the buffer */
	*tlvs_len -= (int)(ie - *tlvs);
	/* update the pointer to the start of the buffer */
	*tlvs = ie;

	return FALSE;
}

wpa_ie_fixed_t *
bcm_find_wpaie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_wpa_ie((uint8 *)ie, &parse, &len)) {
			return (wpa_ie_fixed_t *)ie;
		}
	}
	return NULL;
}

bcm_tlv_t *
bcm_find_wmeie(uint8 *parse, uint len, uint8 subtype, uint8 subtype_len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_wme_ie((uint8 *)ie, &parse, &len)) {
			uint ie_len = TLV_HDR_LEN + ie->len;
			wme_ie_t *ie_data = (wme_ie_t *)ie->data;
			/* the subtype_len must include OUI+type+subtype */
			if (subtype_len > WME_OUI_LEN + 1 &&
			    ie_len == (uint)TLV_HDR_LEN + subtype_len &&
			    ie_data->subtype == subtype) {
				return ie;
			}
			/* move to next IE */
			len -= (uint)((uint8 *)ie + ie_len - parse);
			parse = (uint8 *)ie + ie_len;
		}
	}
	return NULL;
}

wps_ie_fixed_t *
bcm_find_wpsie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_wps_ie((uint8 *)ie, &parse, &len)) {
			return (wps_ie_fixed_t *)ie;
		}
	}
	return NULL;
}

/* locate the Attribute in the WPS IE */
wps_at_fixed_t *
bcm_wps_find_at(wps_at_fixed_t *at, int len, uint16 id)
{
	while (len >= WPS_AT_FIXED_LEN) {
		int alen = WPS_AT_FIXED_LEN + ntoh16_ua(((wps_at_fixed_t *)at)->len);
		if (ntoh16_ua(((wps_at_fixed_t *)at)->at) == id && alen <= len)
			return at;
		at = (wps_at_fixed_t *)((uint8 *)at + alen);
		len -= alen;
	}
	return NULL;
}

#ifdef WLP2P
wifi_p2p_ie_t *
bcm_find_p2pie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_p2p_ie((uint8 *)ie, &parse, &len)) {
			return (wifi_p2p_ie_t *)ie;
		}
	}
	return NULL;
}
#endif // endif

bcm_tlv_t *
bcm_find_hs20ie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_hs20_ie((uint8 *)ie, &parse, &len)) {
			return ie;
		}
	}
	return NULL;
}

bcm_tlv_t *
bcm_find_osenie(uint8 *parse, uint len)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_VS_ID))) {
		if (bcm_is_osen_ie((uint8 *)ie, &parse, &len)) {
			return ie;
		}
	}
	return NULL;
}

#if defined(BCMSUP_PSK) || defined(BCMSUPPL) || defined(GTKOE)
#define wpa_is_kde(ie, tlvs, len, type)	bcm_has_ie(ie, tlvs, len, \
	(const uint8 *)WPA2_OUI, WPA2_OUI_LEN, type)

eapol_wpa2_encap_data_t *
wpa_find_kde(uint8 *parse, uint len, uint8 type)
{
	bcm_tlv_t *ie;

	while ((ie = bcm_parse_tlvs(parse, (int)len, DOT11_MNG_PROPR_ID))) {
		if (wpa_is_kde((uint8 *)ie, &parse, &len, type)) {
			return (eapol_wpa2_encap_data_t *)ie;
		}
	}
	return NULL;
}

bool
wpa_is_gtk_encap(uint8 *ie, uint8 **tlvs, uint *tlvs_len)
{
	return wpa_is_kde(ie, tlvs, tlvs_len, WPA2_KEY_DATA_SUBTYPE_GTK);
}

eapol_wpa2_encap_data_t *
wpa_find_gtk_encap(uint8 *parse, uint len)
{
	eapol_wpa2_encap_data_t *data;

	/* minimum length includes kde upto gtk field in eapol_wpa2_key_gtk_encap_t */
	data = wpa_find_kde(parse, len, WPA2_KEY_DATA_SUBTYPE_GTK);
	if (data && (data->length < EAPOL_WPA2_GTK_ENCAP_MIN_LEN)) {
		data = NULL;
	}

	return data;
}

eapol_wpa2_encap_data_t *
wpa_find_igtk_encap(uint8 *parse, uint len)
{
	return wpa_find_kde(parse, len, WPA2_KEY_DATA_SUBTYPE_IGTK);
}

#endif /* defined(BCMSUP_PSK) || defined(BCMSUPPL) || defined(GTKOE) */

const uint8 *
wpa_array_cmp(int max_array, const uint8 *x, const uint8 *y, uint len)
{
	uint i;
	const uint8 *ret = x;

	for (i = 0; i < len; i++)
		if (x[i] != y[i])
			break;

	if (i == len) {
		/* returning null will cause crash, return value used for copying */
		/* return first param in this case to close security loophole */
		return x;
	}
	if (max_array && (y[i] > x[i]))
		ret = y;
	if (!max_array && (y[i] < x[i]))
		ret = y;

	return (ret);
}

void
wpa_incr_array(uint8 *array, uint len)
{
	int i;

	for (i = (len-1); i >= 0; i--)
		if (array[i]++ != 0xff) {
			break;
		}
}

/* map akm suite to internal WPA_AUTH_XXXX */
/* akms points to 4 byte suite (oui + type) */
bool
bcmwpa_akm2WPAauth(uint8 *akm, uint32 *auth, bool sta_iswpa)
{
	BCM_REFERENCE(sta_iswpa);

	if (!memcmp(akm, WPA2_OUI, DOT11_OUI_LEN)) {
		switch (akm[DOT11_OUI_LEN]) {
		case RSN_AKM_NONE:
			*auth = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			*auth = WPA2_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			*auth = WPA2_AUTH_PSK;
			break;
		case RSN_AKM_FBT_1X:
			*auth = WPA2_AUTH_UNSPECIFIED | WPA2_AUTH_FT;
			break;
		case RSN_AKM_FBT_PSK:
			*auth = WPA2_AUTH_PSK | WPA2_AUTH_FT;
			break;
		case RSN_AKM_SHA256_1X:
			*auth = WPA2_AUTH_1X_SHA256;
			break;
		case RSN_AKM_SHA256_PSK:
			*auth = WPA2_AUTH_PSK_SHA256;
			break;
		case RSN_AKM_FILS_SHA256:
			*auth = WPA2_AUTH_FILS_SHA256;
			break;
		case RSN_AKM_FILS_SHA384:
			*auth = WPA2_AUTH_FILS_SHA384;
			break;

		default:
			return FALSE;
		}
		return TRUE;
	}
	else
	if (!memcmp(akm, WFA_OUI, WFA_OUI_LEN)) {
		switch (akm[WFA_OUI_LEN]) {
		case OSEN_AKM_UNSPECIFIED:
			*auth = WPA2_AUTH_UNSPECIFIED;
			break;

		default:
			return FALSE;
		}
		return TRUE;
	}
	else
	if (!memcmp(akm, WPA_OUI, DOT11_OUI_LEN)) {
		switch (akm[DOT11_OUI_LEN]) {
		case RSN_AKM_NONE:
			*auth = WPA_AUTH_NONE;
			break;
		case RSN_AKM_UNSPECIFIED:
			*auth = WPA_AUTH_UNSPECIFIED;
			break;
		case RSN_AKM_PSK:
			*auth = WPA_AUTH_PSK;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

/* map cipher suite to internal WSEC_XXXX */
/* cs points 4 byte cipher suite, and only the type is used for non CCX ciphers */
bool
bcmwpa_cipher2wsec(uint8 *cipher, uint32 *wsec)
{
	switch (cipher[DOT11_OUI_LEN]) {
	case WPA_CIPHER_NONE:
		*wsec = 0;
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		*wsec = WEP_ENABLED;
		break;
	case WPA_CIPHER_TKIP:
		*wsec = TKIP_ENABLED;
		break;
	case WPA_CIPHER_AES_CCM:
		/* fall through */
	case WPA_CIPHER_AES_GCM:
		/* fall through */
	case WPA_CIPHER_AES_GCM256:
		*wsec = AES_ENABLED;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

/* map WPA/RSN cipher to internal WSEC */
uint32
bcmwpa_wpaciphers2wsec(uint8 wpacipher)
{
	uint32 wsec = 0;

	switch (wpacipher) {
	case WPA_CIPHER_NONE:
		break;
	case WPA_CIPHER_WEP_40:
	case WPA_CIPHER_WEP_104:
		wsec = WEP_ENABLED;
		break;
	case WPA_CIPHER_TKIP:
		wsec = TKIP_ENABLED;
		break;
	case WPA_CIPHER_AES_OCB:
		/* fall through */
	case WPA_CIPHER_AES_CCM:
		wsec = AES_ENABLED;
		break;
	case WPA_CIPHER_AES_GCM:
	/* fall through */
	case WPA_CIPHER_AES_GCM256:
		wsec = AES_ENABLED;
		break;
	default:
		break;
	}

	return wsec;
}

bool
bcmwpa_is_wpa_auth(uint32 auth)
{
	if ((auth == WPA_AUTH_NONE) ||
	   (auth == WPA_AUTH_UNSPECIFIED) ||
	   (auth == WPA_AUTH_PSK))
		return TRUE;
	else
		return FALSE;
}

bool
bcmwpa_includes_wpa_auth(uint32 auth)
{
	if (auth & (WPA_AUTH_NONE |
		WPA_AUTH_UNSPECIFIED |
		WPA_AUTH_PSK))
		return TRUE;
	else
		return FALSE;
}

bool
bcmwpa_is_wpa2_auth(uint32 auth)
{
	auth = auth & ~WPA2_AUTH_FT;

	if ((auth == WPA2_AUTH_UNSPECIFIED) ||
	   (auth == WPA2_AUTH_PSK) ||
	   (auth == BRCM_AUTH_PSK) ||
	   (auth == WPA2_AUTH_1X_SHA256) ||
	   (auth == WPA2_AUTH_PSK_SHA256) ||
	   WPA2_AUTH_IS_FILS(auth))
		return TRUE;
	else
		return FALSE;
}

bool
bcmwpa_includes_wpa2_auth(uint32 auth)
{
	if (auth & (WPA2_AUTH_UNSPECIFIED |
		WPA2_AUTH_PSK |
		BRCM_AUTH_PSK | WPA2_AUTH_1X_SHA256| WPA2_AUTH_PSK_SHA256| WPA2_AUTH_IS_FILS(auth)))
		return TRUE;
	else
		return FALSE;
}
