/*
 * private interface for wlc_key algo 'wep'
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
 * $Id: km_key_wep_pvt.h 684564 2017-02-13 21:59:26Z $
 */

#ifndef km_key_wep_pvt_h_
#define km_key_wep_pvt_h_

#include "km_key_pvt.h"

#include <rc4.h>
#include <wep.h>

#define WEP_KEY_ALLOC_SIZE ROUNDUP(WEP128_KEY_SIZE, 16)
#define WEP_RC4_IV_SIZE (DOT11_IV_LEN - 1)
#define WEP_RC4_ALLOC_SIZE (WEP_RC4_IV_SIZE + WEP_KEY_ALLOC_SIZE)

#define WEP_KEY_VALID(_key) ((((_key)->info.algo == CRYPTO_ALGO_WEP1) ||\
		((_key)->info.algo == CRYPTO_ALGO_WEP128)) &&\
		((_key)->info.key_len <= WEP_KEY_ALLOC_SIZE) &&\
		((_key)->info.iv_len <= DOT11_IV_LEN))

/* context data type for wep. note that wep has two key sizes
 * as selected by key algo, and no replay protection. hence there
 * is no need to allocate rx seq (iv, replay counter).
 */
struct wep_key {
	uint8 key[WEP_KEY_ALLOC_SIZE];		/* key data */
	uint8 tx_seq[DOT11_IV_LEN];			/* LE order - need only 24 bits */

};

typedef struct wep_key wep_key_t;

#endif /* km_key_wep_pvt_h_ */
