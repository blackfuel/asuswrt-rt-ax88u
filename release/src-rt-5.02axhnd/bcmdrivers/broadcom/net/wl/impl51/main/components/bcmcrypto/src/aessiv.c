/*
 * aessiv.c
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
 * <<Broadcom-WL-IPTag/Proprietary:>>`
 *
 * $Id: aessiv.c 753561 2018-03-22 02:57:44Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>

#ifdef BCMDRIVER
#include <osl.h>
#else
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define ASSERT assert
#endif  /* BCMDRIVER */

#include <bcmendian.h>
#include <bcmutils.h>
#include <aessiv.h>

static int
cmac_cb(void *ctx_in, const uint8* data, size_t data_len, uint8* mic, const size_t mic_len)
{
	aessiv_ctx_t *ctx = (aessiv_ctx_t *)ctx_in;
	aes_cmac(ctx->nrounds, ctx->iv_rkey, ctx->iv_subk1, ctx->iv_subk2,
		NULL, 0, data_len, data, mic, mic_len);
	return BCME_OK;
}

static int ctr_cb(void *ctx_in, const uint8 *iv, uint8* data, size_t data_len)
{
	int err;

	aessiv_ctx_t *ctx = (aessiv_ctx_t *)ctx_in;
	err = aes_ctr_crypt(ctx->ctr_rkey, ctx->key_len, iv, data_len, data, data);
	return err ? BCME_DECERR : BCME_OK;
}

int
aessiv_init(aessiv_ctx_t *ctx, siv_op_type_t op_type,
	size_t key_len, const uint8 *iv_key, const uint8 *ctr_key)
{
	int err;

	memset(ctx, 0, sizeof(*ctx));

	if (!ctr_key && !iv_key) {
		err = BCME_BADARG;
		goto done;
	}
	else if (!ctr_key) {
		key_len >>= 1u;
		ctr_key = &iv_key[key_len];
	}

	ctx->key_len = key_len;
	ctx->nrounds = (int)AES_ROUNDS(key_len);
	rijndaelKeySetupEnc(ctx->iv_rkey, iv_key, (int)AES_KEY_BITLEN(key_len));
	aes_cmac_gen_subkeys(ctx->nrounds, ctx->iv_rkey, ctx->iv_subk1, ctx->iv_subk2);
	rijndaelKeySetupEnc(ctx->ctr_rkey, ctr_key, (int)AES_KEY_BITLEN(key_len));

	err = siv_init(&ctx->siv_ctx, op_type, cmac_cb, ctx, ctr_cb, ctx);
	if (err != BCME_OK)
		goto done;

done:
	return err;
}

int aessiv_update(aessiv_ctx_t *ctx, const uint8 *hdr, size_t hdr_len)
{
	ASSERT(ctx != NULL);
	return siv_update(&ctx->siv_ctx, hdr, hdr_len);
}

int
aessiv_update_with_prefixes(aessiv_ctx_t *ctx, const bcm_const_ulvp_t *prefixes, int num_prefixes,
	const uint8 *data, const size_t data_len)
{
	int j;
	int err = BCME_OK;
	ASSERT(ctx != NULL);

	/* handle prefixes, if any */
	for (j = 0; prefixes != NULL && j < num_prefixes; ++ j) {
		if (prefixes[j].len) {
			err = aessiv_update(ctx, prefixes[j].data, prefixes[j].len);
			if (err != BCME_OK) {
				goto done;
			}
		}
	}
	/* handle data */
	if (data && data_len) {
		err = aessiv_update(ctx, data, data_len);
	}
done:
	return err;

}

/* Finalize aessiv context. */
int
aessiv_final(aessiv_ctx_t *ctx, uint8 *iv, uint8 *data, size_t data_len)
{
	int err;

	ASSERT(ctx != NULL);
	err =  siv_final(&ctx->siv_ctx, iv, data, data_len);

	memset(ctx, 0, sizeof(*ctx));
	return err;
}
