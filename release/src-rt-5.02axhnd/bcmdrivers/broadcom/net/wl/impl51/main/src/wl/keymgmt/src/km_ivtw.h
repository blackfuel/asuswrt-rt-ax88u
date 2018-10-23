/*
 * Internal interface to BRCMAPIVTW support
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
 * $Id: km_ivtw.h 617309 2016-02-04 23:50:36Z $
 */

#ifndef _km_ivtw_h_
#define _km_ivtw_h_

#include "km.h"

typedef struct km_ivtw km_ivtw_t;

km_ivtw_t* km_ivtw_attach(wlc_info_t *wlc, wlc_keymgmt_t *km);
void km_ivtw_detach(km_ivtw_t **ivtw);

int km_ivtw_get_mode(km_ivtw_t *ivtw);
int km_ivtw_set_mode(km_ivtw_t *ivtw, int val);

int km_ivtw_enable(km_ivtw_t *ivtw, wlc_key_index_t key_idx, bool enable);

bool km_ivtw_is_replay(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
	uint8 *key_seq, uint8 *rx_seq, size_t seq_len);

int km_ivtw_reset(km_ivtw_t *ivtw, wlc_key_index_t key_idx);

void km_ivtw_update(km_ivtw_t *ivtw, wlc_key_info_t *key_info, int ins,
    uint8 *rx_seq, size_t seq_len, bool chained);

#if defined(BCMDBG)
void km_ivtw_dump(km_ivtw_t *ivtw, struct bcmstrbuf *b);
#endif // endif

void km_ivtw_clone(km_ivtw_t *from_ivtw, km_ivtw_t *to_ivtw,
	wlc_key_index_t from_key_idx, wlc_key_index_t to_key_idx);

#endif /* _km_ivtw_h_ */
