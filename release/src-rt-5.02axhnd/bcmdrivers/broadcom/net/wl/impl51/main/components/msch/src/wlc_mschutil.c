/*
* Multiple channel scheduler
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
* $Id: wlc_mschutil.c 707078 2017-06-24 17:43:02Z $
*
*/

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_hrt.h>
#include <hndpmu.h>

#include <wlc_msch.h>
#include <wlc_mschutil.h>

void
msch_list_init(msch_list_elem_t *elem)
{
	ASSERT(elem);
	elem->prev = elem->next = NULL;
}

void
msch_list_add_at(msch_list_elem_t *at_elem, msch_list_elem_t *new_elem)
{
	ASSERT(at_elem && new_elem);

	new_elem->prev = at_elem;
	new_elem->next = at_elem->next;
	if (at_elem->next) {
		at_elem->next->prev = new_elem;
	}
	at_elem->next = new_elem;
}

void
msch_list_add_to_tail(msch_list_elem_t* list, msch_list_elem_t* new_elem)
{
	msch_list_elem_t *elem, *prev_elem;

	ASSERT(list && new_elem);

	prev_elem = list;

	/* find proper location */
	elem = prev_elem->next;
	while (elem) {
		prev_elem = elem;
		elem = elem->next;
	}

	msch_list_add_at(prev_elem, new_elem);
	return;
}

void
msch_list_sorted_add(msch_list_elem_t* list, msch_list_elem_t* new_elem,
	int comp_offset, int comp_size, msch_order_t order)
{
	msch_list_elem_t *elem, *prev_elem;

	ASSERT(list && new_elem && (order < MSCH_INVALID_ORDER));

	prev_elem = list;
	if (order == MSCH_NO_ORDER) {
		goto exit;
	}

	/* find proper location */
	elem = prev_elem->next;
	while (elem) {
		if ((bcm_cmp_bytes((const uchar *)new_elem + comp_offset,
			(const uchar *)elem + comp_offset, (uint8)comp_size) > 0) !=
			(int)(order == MSCH_ASCEND)) {
			break;
		}
		prev_elem = elem;
		elem = elem->next;
	}

exit:
	msch_list_add_at(prev_elem, new_elem);
	return;
}

msch_list_elem_t*
msch_list_remove(msch_list_elem_t *to_del_elem)
{
	ASSERT(to_del_elem);

	if (!to_del_elem->prev && to_del_elem->next) {
		WL_ERROR(("%s : %p => unhandled link prev %p next %p\n", __FUNCTION__,
			OSL_OBFUSCATE_BUF(to_del_elem), OSL_OBFUSCATE_BUF(to_del_elem->prev),
			OSL_OBFUSCATE_BUF(to_del_elem->next)));
	}

	if (to_del_elem->prev == NULL) {
		return NULL;
	}

	to_del_elem->prev->next = to_del_elem->next;
	if (to_del_elem->next) {
		to_del_elem->next->prev = to_del_elem->prev;
	}
	msch_list_init(to_del_elem);

	return to_del_elem;
}

msch_list_elem_t*
msch_list_remove_head(msch_list_elem_t *list)
{
	ASSERT(list);

	if (!list->next) {
		return NULL;
	}

	return msch_list_remove(list->next);
}

msch_list_elem_t*
msch_list_iterate(msch_list_elem_t *last_elem)
{
	if (!last_elem) {
		return NULL;
	}

	return last_elem->next;
}

bool
msch_list_empty(msch_list_elem_t* list)
{
	if (!list) {
		return (list == NULL);
	}

	return (list->next == NULL);
}

bool
msch_elem_inlist(msch_list_elem_t *elem)
{
	ASSERT(elem);

	return (elem->prev != NULL);
}

uint32
msch_list_length(msch_list_elem_t *elem)
{
	uint32 len = 0;

	while ((elem = msch_list_iterate(elem))) {
		len++;
	}

	return len;
}
