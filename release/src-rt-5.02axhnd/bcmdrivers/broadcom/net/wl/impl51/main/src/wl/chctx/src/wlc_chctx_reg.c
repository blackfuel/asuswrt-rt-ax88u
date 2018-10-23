/*
 * CHannel ConTeXt management module implementation.
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
 * $Id: wlc_chctx_reg.c 691801 2017-03-24 01:23:19Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <wlc_chctx_reg.h>

/* client registry entry */
typedef struct {
	wlc_chctx_init_fn_t init;
	wlc_chctx_save_fn_t save;
	wlc_chctx_restore_fn_t restore;
	wlc_chctx_dump_fn_t dump;
	wlc_chctx_client_ctx_t *ctx;
	uint8 state;	/* see CLNT_ST_XXX */
} wlc_chctx_client_ent_t;

/* cubby registry entry */
typedef struct {
	wlc_chctx_cubby_ctx_t *ctx;
	uint8 *buf;	/* H type: address of the storage
			 * V type: NULL (unused)
			 */
	uint16 szoff;	/* H type: size of the cubby in each context
			 * V type: offset of the cubby in each context
			 */
	uint8 clntid;	/* client ID (link cubby to client) */
	uint8 state;	/* cubby state - see CUBBY_ST_XXX */
} wlc_chctx_cubby_ent_t;

/* context registry entry */
typedef struct {
	chanspec_t chanspec;
	uint8 state;	/* see CTL_ST_XXX */
	uint8 ref;	/* reference count */
	uint8 *buf;	/* H type: NULL (unused)
			 * V type: storage address
			 */
} wlc_chctx_ctl_ent_t;

/* module private info */
struct wlc_chctx_reg {
	osl_t *osh;

	uint16 usersz;	/* extra memory reserved for user */
	uint16 ctxsz;	/* context entry size (for V type) */
	uint8 type;	/* registry type (H or V) */

	/* client registry */
	uint8 client_sz;	/* registry size */
	uint8 client_cnt;
	wlc_chctx_client_ent_t *client;

	/* cubby registry */
	uint16 cubby_sz;	/* registry size */
	uint16 cubby_cnt;
	wlc_chctx_cubby_ent_t *cubby;

	/* context registry */
	uint8 ctl_sz;		/* registry size */
	uint8 ctl_cnt;
	int8 ctl_cur;		/* current context */
	wlc_chctx_ctl_ent_t *ctl;

	/* counters */
	uint clnt_save_err;
	uint clnt_restore_err;
	uint no_ctx_notif;
	uint open_fail;
	uint close_fail;
	uint enter_fail;
	uint leave_fail;
	uint other_fail;
};

/* client state */
#define CLNT_ST_USED	(1<<0)	/* entry is used */

/* cubby id */
#define CUBBY_ID_ALL	-1	/* all cubbies */

/* cubby state */
#define CUBBY_ST_VALID	(1<<0)	/* entry is valid */

/* context index */
#define CTL_IDX_INV	-1	/* invalid index */

/* context state */
#define CTL_ST_USED	(1<<0)	/* entry is used */
#define CTL_ST_SAVED	(1<<1)	/* entry is saved */

/* module private info memory layout */
/*
typedef struct {
	wlc_chctx_reg_t info;
	wlc_chctx_client_ent_t client[clients];
	wlc_chctx_cubby_ent_t cubby[cubbies];
	wlc_chctx_ctl_ent_t ctl[contexts];
} wlc_chctx_reg_mem_t;
*/

/* module private info allocation size */
#define WLC_CHCTX_INFO_SZ(clients, cubbies, contexts)	\
	(sizeof(wlc_chctx_reg_t) +			\
	 sizeof(wlc_chctx_client_ent_t) * (clients) +	\
	 sizeof(wlc_chctx_cubby_ent_t) * (cubbies) +	\
	 sizeof(wlc_chctx_ctl_ent_t) * (contexts) +	\
	 0)

/* context storage access */
#define WLC_CHCTX_CTX_ADDR(ci, ctl, cubby) \
	((ci)->type == WLC_CHCTX_REG_H_TYPE ? \
		(cubby)->buf + (cubby)->szoff * (ctl) :	\
	 (ci)->type == WLC_CHCTX_REG_V_TYPE ? \
		(ci)->ctl[ctl].buf + (cubby)->szoff : \
	 NULL)

/* debug */
#ifdef BCMDBG
#define CHCTX_TRACE(x)
#define CHCTX_ERROR(x)	printf x
#else
#define CHCTX_TRACE(x)
#define CHCTX_ERROR(x)
#endif // endif

/* small accessors */
uint8
wlc_chctx_reg_get_total_contexts(wlc_chctx_reg_t *chctx)
{
	return chctx->ctl_sz;
}

/* attach/detach */
wlc_chctx_reg_t *
BCMATTACHFN(wlc_chctx_reg_attach)(osl_t *osh, uint16 usersz, uint8 type,
	uint8 clients, uint16 cubbies, uint8 contexts)
{
	wlc_chctx_reg_t *info;

	CHCTX_TRACE(("%s: clients %u cubbies %u contexts %u\n",
	             __FUNCTION__, clients, cubbies, contexts));

	/* sanity check */
	ASSERT(type == WLC_CHCTX_REG_H_TYPE || type == WLC_CHCTX_REG_V_TYPE);

	/* allocate module private info */
	if ((info =
	     MALLOCZ(osh, WLC_CHCTX_INFO_SZ(clients, cubbies, contexts) + usersz)) == NULL) {
		CHCTX_ERROR(("%s: malloc failed. allocated %d bytes\n",
		             __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}
	info->osh = osh;
	info->usersz = usersz;
	info->type = type;

	/* sub-divide to different areas */
	info->client = (wlc_chctx_client_ent_t *)&info[1];
	info->client_sz = clients;

	info->cubby = (wlc_chctx_cubby_ent_t *)&info->client[clients];
	info->cubby_sz = cubbies;

	info->ctl = (wlc_chctx_ctl_ent_t *)&info->cubby[cubbies];
	info->ctl_sz = contexts;
	info->ctl_cur = CTL_IDX_INV;

	return info;

fail:
	/* error handling */
	MODULE_DETACH(info, wlc_chctx_reg_detach);
	return NULL;
}

void
BCMATTACHFN(wlc_chctx_reg_detach)(wlc_chctx_reg_t *info)
{
	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	if (info == NULL) {
		return;
	}

	MFREE(info->osh, info,
	      WLC_CHCTX_INFO_SZ(info->client_sz, info->cubby_sz, info->ctl_sz) +
	      info->usersz);
}

/* Return the pointer to the 'user' portion of the wlc_chctx_reg_t */
void *
wlc_chctx_reg_user_ctx(wlc_chctx_reg_t *ci)
{
	return (uint8 *)ci + WLC_CHCTX_INFO_SZ(ci->client_sz, ci->cubby_sz, ci->ctl_sz);
}

/*
 * register callbacks for a client, return the client ID if succeed, error otherwise.
 */
int
BCMATTACHFN(wlc_chctx_reg_add_client)(wlc_chctx_reg_t *ci,
	wlc_chctx_client_fn_t *fns, wlc_chctx_client_ctx_t *ctx)
{
	int clntid;

	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	/* find an empty entry */
	for (clntid = 0; clntid < (int)ci->client_cnt; clntid ++) {
		if (ci->client[clntid].state & CLNT_ST_USED) {
			continue;
		}
		goto init;
	}

	/* use the first avail entry, but check registry occupancy first */
	if (ci->client_cnt == ci->client_sz) {
		CHCTX_ERROR(("%s: too many clients (%u)\n", __FUNCTION__, ci->client_sz));
		return BCME_NORESOURCE;
	}
	ci->client_cnt ++;

init:
	/* sanity check */
	ASSERT(fns != NULL);
	ASSERT(fns->init != NULL);
	ASSERT(fns->restore != NULL);

	/* init the client */
	ci->client[clntid].init = fns->init;
	ci->client[clntid].save = fns->save;
	ci->client[clntid].restore = fns->restore;
	ci->client[clntid].dump = fns->dump;
	ci->client[clntid].ctx = ctx;
	ci->client[clntid].state |= CLNT_ST_USED;

	/* use the client index as the client ID */
	return clntid;
}

/* delete a client */
void
BCMATTACHFN(wlc_chctx_reg_del_client)(wlc_chctx_reg_t *ci, int clntid)
{
	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	ASSERT(clntid >= 0 && clntid < ci->client_cnt);

	bzero(&ci->client[clntid], sizeof(ci->client[clntid]));
}

/* Find the context entry index for 'chanspec' */
static int
wlc_chctx_reg_find_ctl(wlc_chctx_reg_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* TODO: change to faster search algorithm when necessary */
	for (ctl = 0; ctl < (int)ci->ctl_cnt; ctl ++) {
		/* Check if the chanspec already exists */
		if ((ci->ctl[ctl].state & CTL_ST_USED) &&
		    (ci->ctl[ctl].chanspec == chanspec)) {
			return ctl;
		}
	}

	return BCME_NOTFOUND;
}

/* Occupy an empty context entry for 'chanspec' */
static int
wlc_chctx_reg_alloc_ctl(wlc_chctx_reg_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* find an empty entry */
	for (ctl = 0; ctl < (int)ci->ctl_cnt; ctl ++) {
		if (ci->ctl[ctl].state & CTL_ST_USED) {
			continue;
		}
		goto init;
	}

	/* use the first avail entry, but check registry occupancy first */
	if (ci->ctl_cnt == ci->ctl_sz) {
		CHCTX_ERROR(("%s: too many channel contexts (%u)\n", __FUNCTION__, ci->ctl_sz));
		return BCME_NORESOURCE;
	}

	ci->ctl_cnt ++;

init:
	/* sanity check */
	ASSERT(!wf_chspec_malformed(chanspec));

	/* init the context entry */
	ci->ctl[ctl].chanspec = chanspec;
	ci->ctl[ctl].buf = NULL;
	ci->ctl[ctl].ref = 0;
	ci->ctl[ctl].state |= CTL_ST_USED;

	return ctl;
}

/* Free the context entry */
static void
wlc_chctx_reg_free_ctl(wlc_chctx_reg_t *ci, int ctl)
{
	ASSERT(ctl >= 0 && ctl < (int)ci->ctl_cnt);

	bzero(&ci->ctl[ctl], sizeof(ci->ctl[ctl]));
}

/* Initialize all context entries for a cubby.
 * 'cid' CUBBY_ID_ALL is a special id and it's for all cubbies of a particular context,
 * in which case 'chanspec' must be valid; otherwise it's for a particular cubby of all
 * valid context entries.
 */
static void
wlc_chctx_reg_init_context(wlc_chctx_reg_t *ci, int cid, int ctl)
{
	int first_ctl, last_ctl;
	int first_cubby, last_cubby;

	/* bail out if no cubby or no context */
	if (ci->cubby_cnt == 0 || ci->ctl_cnt == 0) {
		return;
	}

	/* walk through all cubbies of a particular context */
	if (cid == CUBBY_ID_ALL) {
		first_cubby = 0;
		last_cubby = ci->cubby_cnt - 1;
		ASSERT(ctl >= 0 && ctl < (int)ci->ctl_cnt);
		first_ctl = last_ctl = ctl;
	}
	/* work on the specified cubby of all context entries */
	else {
		ASSERT(cid >= 0 && cid < (int)ci->cubby_cnt);
		last_cubby = first_cubby = cid;
		first_ctl = 0;
		last_ctl = ci->ctl_cnt - 1;
	}

	/* perform init */
	for (cid = first_cubby; cid <= last_cubby; cid ++) {
		wlc_chctx_cubby_ent_t *cubby_ent = &ci->cubby[cid];

		if (cubby_ent->state & CUBBY_ST_VALID) {
			wlc_chctx_client_ent_t *client_ent = &ci->client[cubby_ent->clntid];

			for (ctl = first_ctl; ctl <= last_ctl; ctl ++) {

				if (!(ci->ctl[ctl].state & CTL_ST_USED)) {
					continue;
				}

				(client_ent->init)(client_ent->ctx, cubby_ent->ctx,
					WLC_CHCTX_CTX_ADDR(ci, ctl, cubby_ent));
			}
		}
	}
}

/*
 * Occupy an context entry for the 'chanspec'.
 */
static int
wlc_chctx_reg_add_context(wlc_chctx_reg_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* increment the ref count if it exists */
	if ((ctl = wlc_chctx_reg_find_ctl(ci, chanspec)) >= 0) {
		ci->ctl[ctl].ref ++;
		/* in case it's wrapped! */
		ASSERT(ci->ctl[ctl].ref != 0);
		return BCME_OK;
	}

	/* allocate an entry and init it */
	if ((ctl = wlc_chctx_reg_alloc_ctl(ci, chanspec)) < 0) {
		return ctl;
	}
	ASSERT(ci->ctl[ctl].ref == 0);
	ci->ctl[ctl].ref = 1;

	/* init all cubbies in this context */
	wlc_chctx_reg_init_context(ci, CUBBY_ID_ALL, ctl);

	return BCME_OK;
}

/*
 * Release the context entry for 'chanspec'.
 */
static int
wlc_chctx_reg_del_context(wlc_chctx_reg_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* bail out if it doesn't exist */
	if ((ctl = wlc_chctx_reg_find_ctl(ci, chanspec)) < 0) {
		return ctl;
	}

	ASSERT(ci->ctl[ctl].ref > 0);
	ci->ctl[ctl].ref --;

	if (ci->ctl[ctl].ref > 0) {
		return BCME_OK;
	}

	wlc_chctx_reg_free_ctl(ci, ctl);

	return BCME_OK;
}

/* perform save */
static void
_wlc_chctx_reg_save_context(wlc_chctx_reg_t *ci, int ctl)
{
	int cid;

	for (cid = 0; cid < (int)ci->cubby_cnt; cid ++) {
		wlc_chctx_cubby_ent_t *cubby_ent = &ci->cubby[cid];

		if (cubby_ent->state & CUBBY_ST_VALID) {
			wlc_chctx_client_ent_t *client_ent = &ci->client[cubby_ent->clntid];
			int err;

			/* save is optional */
			if (client_ent->save == NULL) {
				continue;
			}

			err = (client_ent->save)(client_ent->ctx, cubby_ent->ctx,
				WLC_CHCTX_CTX_ADDR(ci, ctl, cubby_ent));
			if (err != BCME_OK) {
				CHCTX_ERROR(("%s: save %p err %d\n",
				             __FUNCTION__, client_ent->save, err));
				ci->clnt_save_err ++;
			}
		}
	}
}

/*
 * Save client states through the registered 'save' callback.
 * This is optional.
 */
static int
wlc_chctx_reg_save_context(wlc_chctx_reg_t *ci)
{
	/* the current context is valid so invoke clients' save callbacks
	 * to save clients' data to the contexts (optional)
	 */
	if (ci->ctl_cur != CTL_IDX_INV) {
		if (!(ci->ctl[ci->ctl_cur].state & CTL_ST_SAVED)) {
			_wlc_chctx_reg_save_context(ci, ci->ctl_cur);
			ci->ctl[ci->ctl_cur].state |= CTL_ST_SAVED;
		}
	}

	return BCME_OK;
}

/* perform no-context notification through client's restore callback */
static void
wlc_chctx_reg_notif_no_ctx(wlc_chctx_reg_t *ci)
{
	int cid;

	for (cid = 0; cid < (int)ci->cubby_cnt; cid ++) {
		wlc_chctx_cubby_ent_t *cubby_ent = &ci->cubby[cid];

		if (cubby_ent->state & CUBBY_ST_VALID) {
			wlc_chctx_client_ent_t *client_ent = &ci->client[cubby_ent->clntid];

			(void)(client_ent->restore)(client_ent->ctx, cubby_ent->ctx, NULL);
		}
	}

	ci->no_ctx_notif ++;
}

/* perform restore */
static void
_wlc_chctx_reg_restore_context(wlc_chctx_reg_t *ci, int ctl)
{
	int cid;

	for (cid = 0; cid < (int)ci->cubby_cnt; cid ++) {
		wlc_chctx_cubby_ent_t *cubby_ent = &ci->cubby[cid];

		if (cubby_ent->state & CUBBY_ST_VALID) {
			wlc_chctx_client_ent_t *client_ent = &ci->client[cubby_ent->clntid];
			int err;

			err = (client_ent->restore)(client_ent->ctx, cubby_ent->ctx,
				WLC_CHCTX_CTX_ADDR(ci, ctl, cubby_ent));
			if (err != BCME_OK) {
				CHCTX_ERROR(("%s: restore %p err %d\n",
				             __FUNCTION__, client_ent->restore, err));
				ci->clnt_restore_err ++;
			}
		}
	}
}

/*
 * Set the context associated with 'chanspec' as the current and
 * restore client states through the registered 'restore' callback.
 * The client restore callback can save the buffer pointer and
 * save their data into the context directly.
 */
static int
wlc_chctx_reg_restore_context(wlc_chctx_reg_t *ci, chanspec_t chanspec)
{
	int ctl;

	/* unable to find the entry for the new chanspec so
	 * notify clients of no context and bail out...
	 */
	if ((ctl = wlc_chctx_reg_find_ctl(ci, chanspec)) < 0) {
		if (ci->ctl_cur != CTL_IDX_INV) {
			wlc_chctx_reg_notif_no_ctx(ci);
			ci->ctl_cur = CTL_IDX_INV;
		}
		return ctl;
	}

	/* found the entry so let's make it current and restore
	 * the context to clients...
	 */
	if (ci->ctl_cur != ctl) {
		_wlc_chctx_reg_restore_context(ci, ctl);
		ci->ctl[ctl].state &= ~CTL_ST_SAVED;
		ci->ctl_cur = (int8)ctl;
	}

	return BCME_OK;
}

/*
 * add a cubby and the storage.
 */

/* TODO Need to find a way in V type registry to tell users
 * "don't call this function after wlc_chctx_reg_add_entry() is called i.e.
 * when any of the context is used..."
 */

int
wlc_chctx_reg_add_cubby(wlc_chctx_reg_t *ci, int clntid,
	uint8 *storage, uint16 size, wlc_chctx_cubby_ctx_t *ctx)
{
	int cid;

	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	/* sanity check */
	ASSERT(clntid >= 0 && clntid < (int)ci->client_cnt);
	switch (ci->type) {
	case WLC_CHCTX_REG_H_TYPE:
		ASSERT(storage != NULL);
		ASSERT(size > 0 && size == size / ci->ctl_sz * ci->ctl_sz);
		break;
	case WLC_CHCTX_REG_V_TYPE:
		ASSERT(storage == NULL);
		break;
	}

	/* find an hole before adding to the first avail entry */
	for (cid = 0; cid < ci->cubby_cnt; cid ++) {
		if (!(ci->cubby[cid].state & CUBBY_ST_VALID)) {
			/* found one, go reuse it */
			goto init;
		}
	}

	/* use the first avail entry, but check registry occupancy first */
	if (ci->cubby_cnt == ci->cubby_sz) {
		CHCTX_ERROR(("%s: too many cubbies (%u)\n", __FUNCTION__, ci->cubby_sz));
		return BCME_NORESOURCE;
	}

	/* take the first avail entry */
	ci->cubby_cnt ++;

init:
	/* init the cubby */
	ci->cubby[cid].ctx = ctx;
	switch (ci->type) {
	case WLC_CHCTX_REG_H_TYPE:
		ci->cubby[cid].buf = storage;
		ci->cubby[cid].szoff = size / ci->ctl_sz;
		ASSERT(ci->cubby[cid].szoff * ci->ctl_sz == size);
		break;
	case WLC_CHCTX_REG_V_TYPE:
		ci->cubby[cid].szoff = ci->ctxsz;
		ci->ctxsz += ROUNDUP(size, sizeof(void *));
		break;
	default:
		ASSERT(0);
		break;
	}
	ci->cubby[cid].clntid = (uint8)clntid;
	ci->cubby[cid].state = CUBBY_ST_VALID;

	/* init all valid context entries for this cubby */
	wlc_chctx_reg_init_context(ci, cid, CTL_IDX_INV);

	/* return the cubby index as the cubby ID */
	return cid;
}

/*
 * unregister cubby.
 */
void
wlc_chctx_reg_del_cubby(wlc_chctx_reg_t *ci, int cid)
{
	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	ASSERT(cid >= 0 && cid < (int)ci->cubby_cnt);

	bzero(&ci->cubby[cid], sizeof(ci->cubby[cid]));
}

/* query the storage location */
uint8 *
wlc_chctx_reg_query_cubby(wlc_chctx_reg_t *ci, int cid, chanspec_t chanspec)
{
	int ctl;
	wlc_chctx_cubby_ent_t *cubby_ent;

	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	if ((ctl = wlc_chctx_reg_find_ctl(ci, chanspec)) < 0) {
		return NULL;
	}

	ASSERT(cid >= 0 && cid < (int)ci->cubby_cnt);
	cubby_ent = &ci->cubby[cid];

	return WLC_CHCTX_CTX_ADDR(ci, ctl, cubby_ent);
}

/*
 * Find a context entry and assign the storage to the entry.
 */
int
wlc_chctx_reg_add_entry(wlc_chctx_reg_t *ci, chanspec_t chanspec,
	uint8 *storage, uint16 size)
{
	int ctl;

	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	/* sanity check */
	ASSERT(ci->type == WLC_CHCTX_REG_V_TYPE);
	ASSERT(storage != NULL && size > 0);

	/* duplicate check:
	 * Return BCME_OK if chanspec is already present
	 */
	if ((ctl = wlc_chctx_reg_find_ctl(ci, chanspec)) >= 0) {
		return BCME_OK;
	}

	/* allocate an entry and init it */
	if ((ctl = wlc_chctx_reg_alloc_ctl(ci, chanspec)) < 0) {
		return ctl;
	}
	ci->ctl[ctl].buf = storage;
	ASSERT(size == ci->ctxsz);

	/* init all cubbies in this context */
	wlc_chctx_reg_init_context(ci, CUBBY_ID_ALL, ctl);

	/* return the entry index as the context entry ID */
	return ctl;
}

/*
 * Release the context.
 */
void
wlc_chctx_reg_del_entry(wlc_chctx_reg_t *ci, int ctl)
{
	CHCTX_TRACE(("%s:\n", __FUNCTION__));

	wlc_chctx_reg_free_ctl(ci, ctl);
}

/*
 * event notification entrance
 */
int
wlc_chctx_reg_notif(wlc_chctx_reg_t *ci, wlc_chctx_notif_t *data)
{
	int status;
#ifdef BCMDBG
	const char *evtname[] = {"unk", "open", "close", "enter", "leave"};
	char chanbuf[CHANSPEC_STR_LEN];

	BCM_REFERENCE(evtname);
	BCM_REFERENCE(chanbuf);
#endif // endif

	CHCTX_TRACE(("%s: event=%s(%d) chanspec=%s(0x%x)\n",
	             __FUNCTION__, data->event < ARRAYSIZE(evtname) ?
	             evtname[data->event] : evtname[0], data->event,
	             wf_chspec_ntoa(data->chanspec, chanbuf), data->chanspec));

	switch (data->event) {
	case WLC_CHCTX_OPEN_CHAN:
		status = wlc_chctx_reg_add_context(ci, data->chanspec);
		if (status != BCME_OK) {
			ci->open_fail ++;
		}
		break;
	case WLC_CHCTX_CLOSE_CHAN:
		status = wlc_chctx_reg_del_context(ci, data->chanspec);
		if (status != BCME_OK) {
			ci->close_fail ++;
		}
		break;
	case WLC_CHCTX_ENTER_CHAN:
		status = wlc_chctx_reg_save_context(ci);
		if (status != BCME_OK) {
			ci->enter_fail ++;
			break;
		}
		status = wlc_chctx_reg_restore_context(ci, data->chanspec);
		if (status != BCME_OK) {
			ci->enter_fail ++;
		}
		break;
	case WLC_CHCTX_LEAVE_CHAN:
		status = wlc_chctx_reg_save_context(ci);
		if (status != BCME_OK) {
			ci->leave_fail ++;
		}
		break;
	default:
		status = BCME_ERROR;
		ci->other_fail ++;
		break;
	}

	return status;
}

#if defined(BCMDBG)
int
wlc_chctx_reg_dump(wlc_chctx_reg_t *ci, struct bcmstrbuf *b)
{
	int ctl, clntid, cid;

	/* counters */
	bcm_bprintf(b, "clnt_save_err %u clnt_restore_err %u no_ctx_notif %u\n",
	            ci->clnt_save_err, ci->clnt_restore_err, ci->no_ctx_notif);
	bcm_bprintf(b, "open_fail %u close_fail %u enter_fail %u leave_fail %u other_fail %u\n",
	            ci->open_fail, ci->close_fail, ci->enter_fail, ci->leave_fail, ci->other_fail);

	/* control */
	bcm_bprintf(b, "clients: max %u cnt %u\n", ci->client_sz, ci->client_cnt);
	for (clntid = 0; clntid < (int)ci->client_sz; clntid ++) {
		bcm_bprintf(b, "  idx %d: init %p save %p restore %p dump %p ctx %p\n",
		            clntid, ci->client[clntid].init, ci->client[clntid].save,
		            ci->client[clntid].restore, ci->client[clntid].dump,
		            ci->client[clntid].ctx);
	}

	bcm_bprintf(b, "cubbies: max %u cnt %u\n", ci->cubby_sz, ci->cubby_cnt);
	for (cid = 0; cid < (int)ci->cubby_sz; cid ++) {
		bcm_bprintf(b, "  idx %d: ctx %p buf %p size/offset %u cid %u state 0x%x\n",
		            cid, ci->cubby[cid].ctx, ci->cubby[cid].buf,
		            ci->cubby[cid].szoff, ci->cubby[cid].clntid,
		            ci->cubby[cid].state);
	}

	/* context */
	bcm_bprintf(b, "contexts: max %u cur %d\n", ci->ctl_sz, ci->ctl_cur);
	for (ctl = 0; ctl < (int)ci->ctl_sz; ctl ++) {
		char chanbuf[CHANSPEC_STR_LEN];

		bcm_bprintf(b, "  idx %d: state 0x%x chanspec %s(0x%x) ref %u\n",
		            ctl, ci->ctl[ctl].state,
		            wf_chspec_ntoa(ci->ctl[ctl].chanspec, chanbuf),
		            ci->ctl[ctl].chanspec, ci->ctl[ctl].ref);

		if (!(ci->ctl[ctl].state & CTL_ST_USED)) {
			continue;
		}

		for (clntid = 0; clntid < (int)ci->client_cnt; clntid ++) {
			wlc_chctx_client_ent_t *client_ent = &ci->client[clntid];
			int idx = 0;

			bcm_bprintf(b, "    client %d:\n", clntid);

			if (client_ent->dump == NULL) {
				bcm_bprintf(b, "      [no dump fn]\n");
				continue;
			}

			for (cid = 0; cid < (int)ci->cubby_cnt; cid ++) {
				wlc_chctx_cubby_ent_t *cubby_ent = &ci->cubby[cid];

				if (!(cubby_ent->state & CUBBY_ST_VALID)) {
					continue;
				}
				if (cubby_ent->clntid != clntid) {
					continue;
				}

				bcm_bprintf(b, "      cubby %d(%d): ", idx, cid);

				(client_ent->dump)(client_ent->ctx, cubby_ent->ctx,
					WLC_CHCTX_CTX_ADDR(ci, ctl, cubby_ent), b);

				idx ++;
			}
		}
	}

	/* others */
	bcm_bprintf(b, "usersz %u ctxsz %u\n", ci->usersz, ci->ctxsz);

	return BCME_OK;
}
#endif // endif
