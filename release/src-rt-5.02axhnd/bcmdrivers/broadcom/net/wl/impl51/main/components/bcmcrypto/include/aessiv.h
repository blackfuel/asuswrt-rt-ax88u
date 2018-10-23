/*
 * aessiv.h
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
 * $Id: aessiv.h 753561 2018-03-22 02:57:44Z $
 */

#ifndef _AESSIV_H_
#define _AESSIV_H_

/* aessiv is an authenticated encryption mechanism described in
 * also http://tools.ietf.org/search/rfc5297
 */

#include <typedefs.h>
#ifdef BCMDRIVER
#include <osl.h>
#else
#include <stddef.h>  /* For size_t */
#endif // endif

#include <siv.h>
#include <aes.h>

struct aessiv_ctx {
	int	key_len;
	int	nrounds;
	siv_block_t iv_subk1;
	siv_block_t iv_subk2;
	uint32    iv_rkey[(AES_MAXROUNDS + 1) << 2];
	uint32    ctr_rkey[(AES_MAXROUNDS + 1) << 2];
	siv_ctx_t siv_ctx;
};

typedef struct aessiv_ctx aessiv_ctx_t;

/* API returns bcm error status unless specified */

/* Initialize aessiv context */
int aessiv_init(aessiv_ctx_t *aessiv_ctx, siv_op_type_t op_type,
	const size_t key_len, const uint8 *iv_key, const uint8 *ctr_key);

/* Update aessiv state with header data */
int aessiv_update(aessiv_ctx_t *ctx, const uint8 *hdr, const size_t hdr_len);

/* Update aessiv state with array of prefixes and data */
int aessiv_update_with_prefixes(aessiv_ctx_t *ctx, const bcm_const_ulvp_t *prefixes,
	int num_prefixes, const uint8 *data, const size_t data_len);

/* Finalize aessiv context. */
int aessiv_final(aessiv_ctx_t *ctx, uint8 *iv, uint8 *data, const size_t data_len);

#endif /* _AESSIV_H_ */
