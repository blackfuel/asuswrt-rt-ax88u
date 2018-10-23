/*
 * CHannel ConTeXt management module interface (shared by both MAC and PHY).
 *
 * Clients can register callbacks to work with the chctx module to save
 * and restore client data to and from the channel context. Callbacks are
 * invoked surrounding the channel switch operation with pointers
 * pointing to the proper channel context.
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
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_chctx_reg.h 688497 2017-03-06 20:18:52Z $
 */

#ifndef _wlc_chctx_reg_h_
#define _wlc_chctx_reg_h_

/* Terminologies and illustration of structures.
 *
 * Terminologies:
 *
 * - client: a feature providing a set of callback functions
 * - cubby: a space in the context occupied by a client instance
 * - context: all spaces occupied by all client instances
 *
 * There're two parts conprising each channel context registry object (wlc_chctx_reg_t):
 *
 * - Controls: clients, cubbies and contexts all have internal control blocks.
 * - Storages: contexts have externally supplied storage memories.
 *
 * Each channel establises a [channel] context at run time for clients to save
 * and restore channel specific data. Each context has a storage provided outside
 * this module at different times based on the registry types.
 *
 * Registry types:
 *
 * - H type: client provides storage at each cubby registration time, and the storage
 *   is divided by equal amount across all contexts.
 * - V type: client provides storage at each context registration time, and the storage
 *   is divided into cubbies
 *
 * The following diagrams show H type registry and V type registry respectively.
 * The tables illustrate the logical view of the externally supplied storage memories.
 *
 * Example 1: A channel context with 1 client, 2 cubbies, and 4 contexts:
 *
 *                                             4 contexts
 *                           /----------------------^---------------------\
 *                              chan 0      chan 1     chan 2     chan 3
 *                           +-----------+----------+----------+----------+
 *                   / scb 0 | X bytes   | X bytes  | X bytes  | X bytes  | \
 * 1 client <  NAN  <        +-----------+----------+----------+----------+  > 2 ccubbies
 *                   \ scb 1 | X bytes   | X bytes  | X bytes  | X bytes  | /
 *                           +-----------+----------+----------+----------+
 *
 * In this channel context NAN in each scb provides a storage memory of 4X bytes
 * when a cubby is reserved. It's a typical H type registry usage.
 *
 * Example 2: A channel context with 2 clients, 1 cubby each client, and 4 contexts:
 *
 *                                             4 contexts
 *                           /----------------------^---------------------\
 *                              chan 0      chan 1     chan 2     chan 3
 *                           +-----------+----------+----------+----------+
 *            /  TXIQCAL     | X bytes   | X bytes  | X bytes  | X bytes  | \
 * 2 clients <               +-----------+----------+----------+----------+  > 2 ccubbies
 *            \  RXIQCAL     | Y bytes   | Y bytes  | Y bytes  | Y bytes  | /
 *                           +-----------+----------+----------+----------+
 *
 * In this channel context PHY cache manager provides a storage memory of X+Y bytes
 * when a channel context is used. It's a typical V type registry usage.
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>

/* ================== interface for module attach/detach =================== */

/* module info forward declaration */
typedef struct wlc_chctx_reg wlc_chctx_reg_t;

/* attach/detach functions
 * - 'clients' specifies the maximum number of clients supported.
 * - 'cubbies' specifies the maximum number of cubbies supported.
 * - 'contexts' specifies the maximum number of contexts supported.
 */
wlc_chctx_reg_t *wlc_chctx_reg_attach(osl_t *osh, uint16 usersz, uint8 type,
	uint8 clients, uint16 cubbies, uint8 contexts);
void wlc_chctx_reg_detach(wlc_chctx_reg_t *ci);

/* 'type' parameter */
#define WLC_CHCTX_REG_H_TYPE	0
	/* storage is allocated for each cubby (accessed across all contexts) */
#define WLC_CHCTX_REG_V_TYPE	1
	/* storage is allocated for each context (accessed across all cubbies) */

/* query the 'user' portion of the wlc_chctx_reg_t */
void *wlc_chctx_reg_user_ctx(wlc_chctx_reg_t *ci);

/* ================== interface for registration(s) =================== */

/* client structure forward declaration */
typedef void wlc_chctx_client_ctx_t;
/* cubby structure forward declaration */
typedef void wlc_chctx_cubby_ctx_t;

/*
 * Register client callbacks and the callback context.
 * - 'init' callback is mandatory and is invoked whenever a cubby is added or
 *   when a channel is used the first time. It gives the client a chance to initialize
 *   its states in the context to known values before any save and restore operations
 *   take place.
 * - 'save' callback is optional and is invoked to save client states to the context.
 *   The invocation happens automatically when a channel is made non-current (i.e. when
 *   leaving a channel). This is particularly useful for clients with local data.
 * - 'restore' callback is mandatory and is invoked to restore client states from the
 *   context. It happens when a channel is made current (i.e. when entering a channel).
 *   Clients can save the pointer given to the callback and write their data into
 *   the context directly through the pointer; it's also used to notify the client of
 *   there will be no current channel (going to a scan channel e.g.) through a NULL 'buf'
 *   parameter.
 * - 'dump' callback is optional and is for debugging and dumpping the client data.
 * Return client ID if the return value is >= 0; error BCME_XXXX otherwise.
 * There's no duplicate check.
 */
typedef void (*wlc_chctx_init_fn_t)(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf);
typedef int (*wlc_chctx_save_fn_t)(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf);
typedef int (*wlc_chctx_restore_fn_t)(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf);
typedef int (*wlc_chctx_dump_fn_t)(wlc_chctx_client_ctx_t *client,
	wlc_chctx_cubby_ctx_t *cubby, uint8 *buf, struct bcmstrbuf *b);

typedef struct {
	wlc_chctx_init_fn_t init;
	wlc_chctx_save_fn_t save;
	wlc_chctx_restore_fn_t restore;
	wlc_chctx_dump_fn_t dump;
} wlc_chctx_client_fn_t;

int wlc_chctx_reg_add_client(wlc_chctx_reg_t *ci,
	wlc_chctx_client_fn_t *fn, wlc_chctx_client_ctx_t *client);

/*
 * Delete the client.
 * - 'clntid' is the client ID returned from wlc_chctx_reg_add_client().
 */
void wlc_chctx_reg_del_client(wlc_chctx_reg_t *ci, int clntid);

/*
 * Specify the cubby size and add the storage used by all contexts in H type
 * registry; reserve a cubby in V type registry.
 * - 'clntid' is the client ID returned from wlc_chctx_reg_add_client().
 * - 'storage' points to the memory used by all contexts in H type registry,
 *   the storage is divided into equal amount in each context; it is ununsed
 *   in V type registry.
 * - 'size' describes the memory size in bytes used by all contexts
 *   (i.e. size = <memory used by each cubby> x <maximum # contexts>)
 *   in H type registry; it is the cubby size in bytes in V type registry.
 * Return the cubby ID if the return value is >= 0; error BCME_XXXX otherwise.
 * There's no duplicate check.
 */
int wlc_chctx_reg_add_cubby(wlc_chctx_reg_t *ci, int clntid,
	uint8 *storage, uint16 size, wlc_chctx_cubby_ctx_t *cubby);

/*
 * Delete the cubby.
 * - 'cid' is the cubby ID returned from wlc_chctx_reg_add_cubby().
 */
void wlc_chctx_reg_del_cubby(wlc_chctx_reg_t *ci, int cid);

/*
 * Query the cubbie's context storage.
 * - 'cid' is the cubby ID returned from wlc_chctx_reg_add_cubby().
 */
uint8 *wlc_chctx_reg_query_cubby(wlc_chctx_reg_t *ci, int cid, chanspec_t chanspec);

/*
 * Use a context and add the storage used by all cubbies.
 * - 'chanspec' is used to look up an existing context entry or to allocate
 *   an unused context entry and assign the chanspec to it.
 * - 'storage' points to the memory used by all cubbies in the context,
 *   required by V type registry.
 * - 'size' describes the size of the memory in bytes used by all cubbies
 *   in the context.
 * Returns the context entry ID if the return value is >= 0; err BCME_XXXX otherwise.
 * There's duplicate check over chanspec.
 */
int wlc_chctx_reg_add_entry(wlc_chctx_reg_t *ci, chanspec_t chanspec,
	uint8 *storage, uint16 size);

/*
 * Release the context entry.
 * - 'ctl' is the context entry ID returned from wlc_chctx_reg_add_entry().
 */
void wlc_chctx_reg_del_entry(wlc_chctx_reg_t *ci, int ctl);

/*
 * Accessors
 */
uint8 wlc_chctx_reg_get_total_contexts(wlc_chctx_reg_t *chctx);

/* ================== interface for notification(s) =================== */

/*
 * notify clients of channel activity
 */
typedef struct {
	uint16 event;
	chanspec_t chanspec;
} wlc_chctx_notif_t;

/* event # */
#define WLC_CHCTX_OPEN_CHAN	1	/* open channel context */
#define WLC_CHCTX_CLOSE_CHAN	2	/* close channel context */
#define WLC_CHCTX_ENTER_CHAN	3	/* enter a new channel */
#define WLC_CHCTX_LEAVE_CHAN	4	/* leave the current channel */

int wlc_chctx_reg_notif(wlc_chctx_reg_t *ci, wlc_chctx_notif_t *data);

/* ================== interface for module debug =================== */

#if defined(BCMDBG)
/* Dump chctx module internals */
int wlc_chctx_reg_dump(wlc_chctx_reg_t *ci, struct bcmstrbuf *b);
#endif // endif

#endif /* _wlc_chctx_reg_h_ */
