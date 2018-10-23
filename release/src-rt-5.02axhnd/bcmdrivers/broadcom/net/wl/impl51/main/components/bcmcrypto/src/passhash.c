/*
 * passhash.c
 * Perform password to key hash algorithm as defined in WPA and 802.11i
 * specifications
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
 * $Id: passhash.c 759009 2018-04-23 15:14:10Z $
 */

#include <passhash.h>

#define passhash_hmac_sha1(_data, _data_len, _key, _key_len, _digest) \
	hmac_sha2(HASH_SHA1, (_key), (_key_len), NULL, 0, (_data), (_data_len), \
		(_digest), SHA2_SHA1_DIGEST_LEN)

#ifdef BCMDRIVER
#include <osl.h>
#else
#include <string.h>
#endif	/* BCMDRIVER */

#include <bcmutils.h>
#include <bcmendian.h>

#define PBKDF2_NXVLPS 2

/* PBKDF2i(P, S, c, i) = U1 xor U2 xor ... Uc
 * U1 = PRF(P, S || Int(i) - 4 octet MSB first
 * U2 = PRF(P, U1)...Uc = PRF(P, Uc-1)
 */
void
PBKDF2i(sha2_hash_type_t hash_type,  const char *pass, int pass_len,
	const uint8 *salt, int salt_len, int niter, uint32 i,
	uint8 *out, int out_len)
{
	uint8 digest[SHA2_MAX_DIGEST_LEN];
	int digest_len;
	uint8 ibuf[sizeof(uint32)];
	bcm_const_xlvp_t pfxs[PBKDF2_NXVLPS];
	int npfxs = 0;
	int iter;
	int j;

	digest_len = sha2_digest_len(hash_type);
	if (!digest_len || !out_len)
		goto done;

	pfxs[npfxs].len = (uint16)salt_len;
	pfxs[npfxs++].data = salt;

	hton32_ua_store(i, ibuf);
	pfxs[npfxs].len = (uint16)sizeof(ibuf);
	pfxs[npfxs++].data = (const uint8 *)ibuf;

	/* U1 = PRF(P, S || int(i)) */
	(void)hmac_sha2(hash_type, (const uint8 *)pass, pass_len, pfxs, npfxs, NULL, 0,
		digest, digest_len);
	memcpy(out, digest, MIN(out_len, digest_len));

	for (iter = 2; iter <= niter; iter++) { /* Un = PRF(P, Un-1) */
		(void)hmac_sha2(hash_type, (const uint8 *)pass, pass_len, NULL, 0,
			digest, digest_len, digest, digest_len);

		/* output = output xor Un */
		for (j = 0; j < MIN(out_len, digest_len); j++) {
			out[j] ^= digest[j];
		}
	}

done:;
}

/* passhash: perform passwork->key hash algorithm as defined in WPA and 802.11i
 * specifications.
 *	password is an ascii string of 8 to 63 characters in length
 *	ssid is up to 32 bytes
 *	ssidlen is the length of ssid in bytes
 *	output must be at lest 40 bytes long, and returns a 256 bit key
 *	returns 0 on success, non-zero on failure
 */
int
passhash(char *password, int passlen, unsigned char *ssid, int ssidlen,
                   unsigned char *output)
{
	if ((strlen(password) < 8) || (strlen(password) > 63) || (ssidlen > 32)) return 1;

	PBKDF2i(HASH_SHA1, password, passlen, ssid, ssidlen, 4096, 1,
		output, SHA2_SHA1_DIGEST_LEN);
	PBKDF2i(HASH_SHA1, password, passlen, ssid, ssidlen, 4096, 2,
		&output[SHA2_SHA1_DIGEST_LEN], SHA2_SHA1_DIGEST_LEN);
	return 0;
}

static void
init_F(char *password, int passlen, unsigned char *ssid, int ssidlength,
       int count, unsigned char *lastdigest, unsigned char *output)
{
	unsigned char digest[36];

	/* U0 = PRF(P, S || int(i)) */
	/* output = U0 */
	if (ssidlength > 32)
		ssidlength = 32;
	memcpy(digest, ssid, ssidlength);
	digest[ssidlength]   = (unsigned char)((count>>24) & 0xff);
	digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff);
	digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff);
	digest[ssidlength+3] = (unsigned char)(count & 0xff);
	passhash_hmac_sha1(digest, ssidlength+4, (unsigned char *)password, passlen, output);

	/* Save U0 for next PRF() */
	memcpy(lastdigest, output, SHA2_SHA1_DIGEST_LEN);
}

static void
do_F(char *password, int passlen, int iterations, unsigned char *lastdigest, unsigned char *output)
{
	unsigned char digest[SHA2_SHA1_DIGEST_LEN];
	int i, j;

	for (i = 0; i < iterations; i++) {
		/* Un = PRF(P, Un-1) */
		passhash_hmac_sha1(lastdigest, SHA2_SHA1_DIGEST_LEN,
			(unsigned char *)password, passlen, digest);
		/* output = output xor Un */
		for (j = 0; j < SHA2_SHA1_DIGEST_LEN; j++)
			output[j] ^= digest[j];

		/* Save Un for next PRF() */
		memcpy(lastdigest, digest, SHA2_SHA1_DIGEST_LEN);
	}
}

/* passhash: Perform passwork to key hash algorithm as defined in WPA and 802.11i
 * specifications. We are breaking this lengthy process into smaller pieces. Users
 * are responsible for making sure password length is between 8 and 63 inclusive.
 *
 * init_passhash: initialize passhash_t structure.
 * do_passhash: advance states in passhash_t structure and return 0 to indicate
 *              it is done and 1 to indicate more to be done.
 * get_passhash: copy passhash result to output buffer.
 */
int
init_passhash(passhash_t *ph,
              char *password, int passlen, unsigned char *ssid, int ssidlen)
{
	if (strlen(password) < 8 || strlen(password) > 63)
		return -1;

	memset(ph, 0, sizeof(*ph));
	ph->count = 1;
	ph->password = password;
	ph->passlen = passlen;
	ph->ssid = ssid;
	ph->ssidlen = ssidlen;

	return 0;
}

int
do_passhash(passhash_t *ph, int iterations)
{
	unsigned char *output;

	if (ph->count > 2)
		return -1;
	output = ph->output + SHA2_SHA1_DIGEST_LEN * (ph->count - 1);
	if (ph->iters == 0) {
		init_F(ph->password, ph->passlen, ph->ssid, ph->ssidlen,
		       ph->count, ph->digest, output);
		ph->iters = 1;
		iterations --;
	}
	if (ph->iters + iterations > 4096)
		iterations = 4096 - ph->iters;
	do_F(ph->password, ph->passlen, iterations, ph->digest, output);
	ph->iters += iterations;
	if (ph->iters == 4096) {
		ph->count ++;
		ph->iters = 0;
		if (ph->count > 2)
			return 0;
	}
	return 1;
}

int
get_passhash(passhash_t *ph, unsigned char *output, int outlen)
{
	if (ph->count > 2 && outlen <= (int)sizeof(ph->output)) {
		memcpy(output, ph->output, outlen);
		return 0;
	}
	return -1;
}

#ifdef BCMPASSHASH_TEST

#include <stdio.h>

#define	dbg(args)	printf args

void
prhash(char *password, int passlen, unsigned char *ssid, int ssidlen, unsigned char *output)
{
	int k;
	printf("pass\n\t%s\nssid(hex)\n\t", password);
	for (k = 0; k < ssidlen; k++) {
		printf("%02x ", ssid[k]);
		if (!((k+1)%16)) printf("\n\t");
	}
	printf("\nhash\n\t");
	for (k = 0; k < 2 * SHA2_SHA1_DIGEST_LEN; k++) {
		printf("%02x ", output[k]);
		if (!((k + 1) % SHA2_SHA1_DIGEST_LEN)) printf("\n\t");
	}
	printf("\n");
}

#include "passhash_vectors.h"

int
main(int argc, char **argv)
{
	unsigned char output[2*SHA2_SHA1_DIGEST_LEN];
	int retv, k, fail = 0, fail1 = 0;
	passhash_t passhash_states;

	dbg(("%s: testing passhash()\n", *argv));

	for (k = 0; k < NUM_PASSHASH_VECTORS; k++) {
		printf("Passhash test %d:\n", k);
		memset(output, 0, sizeof(output));
		retv = passhash(passhash_vec[k].pass, passhash_vec[k].pl,
			passhash_vec[k].salt, passhash_vec[k].sl, output);
		prhash(passhash_vec[k].pass, passhash_vec[k].pl,
			passhash_vec[k].salt, passhash_vec[k].sl, output);

		if (retv) {
			dbg(("%s: passhash() test %d returned error\n", *argv, k));
			fail++;
		}
		if (memcmp(output, passhash_vec[k].ref, 2*SHA2_SHA1_DIGEST_LEN) != 0) {
			dbg(("%s: passhash test %d reference mismatch\n", *argv, k));
			fail++;
		}
	}

	dbg(("%s: %s\n", *argv, fail?"FAILED":"PASSED"));

	dbg(("%s: testing init_passhash()/do_passhash()/get_passhash()\n", *argv));

	for (k = 0; k < NUM_PASSHASH_VECTORS; k++) {
		printf("Passhash test %d:\n", k);
		init_passhash(&passhash_states,
		              passhash_vec[k].pass, passhash_vec[k].pl,
		              passhash_vec[k].salt, passhash_vec[k].sl);
		while ((retv = do_passhash(&passhash_states, 100)) > 0)
			;
		get_passhash(&passhash_states, output, sizeof(output));
		prhash(passhash_vec[k].pass, passhash_vec[k].pl,
		       passhash_vec[k].salt, passhash_vec[k].sl, output);

		if (retv < 0) {
			dbg(("%s: passhash() test %d returned error\n", *argv, k));
			fail1++;
		}
		if (memcmp(output, passhash_vec[k].ref, 2*SHA2_SHA1_DIGEST_LEN) != 0) {
			dbg(("%s: passhash test %d reference mismatch\n", *argv, k));
			fail1++;
		}
	}

	dbg(("%s: %s\n", *argv, fail1?"FAILED":"PASSED"));
	return (fail+fail1);
}

#endif	/* BCMPASSHASH_TEST */
