/*
 * Interface definitions for multi-channel scheduler
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
 * $Id: wlc_mschutil.h 707078 2017-06-24 17:43:02Z $
 */

#ifndef _wlc_mschutil_h_
#define _wlc_mschutil_h_

typedef enum {
	MSCH_NO_ORDER = 0,
	MSCH_ASCEND = 1,
	MSCH_DESCEND = 2,
	MSCH_INVALID_ORDER
} msch_order_t;

typedef struct msch_list_elem {
	struct msch_list_elem *next;
	struct msch_list_elem *prev;
} msch_list_elem_t;

extern void msch_list_init(msch_list_elem_t *elem);
extern void msch_list_add_at(msch_list_elem_t *at_elem, msch_list_elem_t *new_elem);
extern void msch_list_add_to_tail(msch_list_elem_t* list, msch_list_elem_t* new_elem);
extern void msch_list_sorted_add(msch_list_elem_t* list, msch_list_elem_t* new_elem,
	int comp_offset, int comp_size, msch_order_t order);
extern msch_list_elem_t * msch_list_remove(msch_list_elem_t *to_del_elem);
extern msch_list_elem_t * msch_list_remove_head(msch_list_elem_t *list);
extern msch_list_elem_t * msch_list_iterate(msch_list_elem_t *last_elem);
extern bool msch_list_empty(msch_list_elem_t* list);
extern bool msch_elem_inlist(msch_list_elem_t *elem);
extern uint32 msch_list_length(msch_list_elem_t *elem);

#endif /* _wlc_mschutil _h_ */
