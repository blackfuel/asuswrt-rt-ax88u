/*
 * siv.h
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
 * $Id: siv.h 679390 2017-01-14 00:13:47Z $
 */

#ifndef _SIV_H_
#define _SIV_H_

/* SIV is an authenticated encryption mechanism described in
 * http://www.cs.ucdavis.edu/~rogaway/papers/siv.pdf
 * also http://tools.ietf.org/search/rfc5297
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif // endif

/* SIV works with block ciphers (e.g. AES) with block length of 128 bits, but
 * is independent of the cipher and can support different key lengths.
 */

#define SIV_BLOCK_SZ 16
#define SIV_BLOCK_NBITS (SIV_BLOCK_SZ << 3)

/* SIV block - byte 0 is MSB byte, bit 0 is LSB bit */
typedef uint8 siv_block_t[SIV_BLOCK_SZ];

typedef int (*siv_cmac_fn_t)(void *cmac_ctx, const uint8* data, size_t data_len,
	uint8* mic, const size_t mic_len);
typedef int (*siv_ctr_fn_t)(void *ctr_ctx, const uint8 *iv, uint8* data, size_t data_len);

enum siv_op_type {
	SIV_ENCRYPT = 1,
	SIV_DECRYPT
};

typedef enum siv_op_type siv_op_type_t;

struct siv_ctx {
	siv_op_type_t op_type; 	/* selected operation */
	siv_cmac_fn_t cmac_cb;
	void *cmac_cb_ctx;
	siv_ctr_fn_t ctr_cb;
	void *ctr_cb_ctx;

	int num_hdr;
	siv_block_t iv;
};

typedef struct siv_ctx siv_ctx_t;

/* API returns bcm error status unless specified */

/* Initialize siv context */
int siv_init(siv_ctx_t *siv_ctx, siv_op_type_t op_type,
	siv_cmac_fn_t cmac_fn, void *cmac_ctx,
	siv_ctr_fn_t ctr_fn, void *ctr_ctx);

/* Update siv state with header data */
int siv_update(siv_ctx_t *ctx, const uint8 *hdr, const size_t hdr_len);

/* Finalize siv context. data length is for  plain/cipher text; for encryption, iv is
 * returned, for decryption iv is the iv received
 */
int siv_final(siv_ctx_t *ctx, uint8 *iv, uint8 *data, const size_t data_len);

#endif /* _SIV_H_ */
