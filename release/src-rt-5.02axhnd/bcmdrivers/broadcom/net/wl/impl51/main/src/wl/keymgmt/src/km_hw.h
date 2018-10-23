/*
 * Key Management Module Implementation
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
 * $Id: km_hw.h 758912 2018-04-23 01:57:25Z $
 */

#ifndef _km_hw_h_
#define _km_hw_h_

#include "km.h"
#include "km_key.h"

struct km_hw;
typedef struct km_hw km_hw_t;

#define KM_HW_AMT_IDX_INVALID (-1)

typedef wlc_keymgmt_t keymgmt_t;

km_hw_t *km_hw_attach(wlc_info_t *wlc, wlc_keymgmt_t *km);
void km_hw_detach(km_hw_t **hw);
void km_hw_init(km_hw_t *hw);
void km_hw_reset(km_hw_t *hw);

void km_hw_key_create(km_hw_t *km_hw, const wlc_key_t *key,
	const wlc_key_info_t *key_info, wlc_key_hw_index_t *hw_idx);
void km_hw_key_destroy(km_hw_t *km_hw, wlc_key_hw_index_t *hw_idx,
	const wlc_key_info_t *key_info);

void km_hw_key_update(km_hw_t *km_hw, wlc_key_hw_index_t hw_idx,
	wlc_key_t *key, const wlc_key_info_t *key_info);

bool km_hw_key_hw_mic(km_hw_t *km_hw, wlc_key_hw_index_t hw_idx,
	wlc_key_info_t *key_info);

void km_hw_dump(km_hw_t *hm_hw, struct bcmstrbuf *b,
	km_key_dump_type_t dump_type);

wlc_key_hw_algo_t km_hw_algo_to_hw_algo(const km_hw_t *hw, wlc_key_algo_t algo);
wlc_key_algo_t km_hw_hw_algo_to_algo(const km_hw_t *hw, wlc_key_hw_algo_t algo);
bool km_hw_amt_idx_isset(km_hw_t *hw, int amt_idx);
km_amt_idx_t km_hw_amt_find_and_resrv(km_hw_t *hw);
#ifdef ACKSUPR_MAC_FILTER
km_amt_idx_t km_hw_amt_alloc_acksupr(km_hw_t *hw, scb_t *scb);
#endif /* ACKSUPR_MAC_FILTER */
km_amt_idx_t km_hw_amt_alloc(km_hw_t *hw, const struct ether_addr *ea);
void km_hw_amt_release(km_hw_t *hw, km_amt_idx_t *amt_idx);
void km_hw_amt_reserve(km_hw_t *hw, km_amt_idx_t amt_idx,
	size_t count, bool reserve);
bool km_hw_amt_idx_valid(km_hw_t *hw, km_amt_idx_t amt_idx);

#endif /* _km_hw_h_ */
