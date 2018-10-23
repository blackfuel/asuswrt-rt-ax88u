/*
 * txflow_classifier.c
 *
 * Common TX/RX Flow Classification module
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
 * $Id$
 *
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <osl.h>
#include <ethernet.h>

#include "txflow_classifier.h"

#ifndef MAX_TXFLOWS
#define MAX_TXFLOWS	4
#endif	/* MAX_TXFLOWS */

#ifdef BCMDBG_ERR
#define FLOW_ERROR(args) printf args
#else
#define FLOW_ERROR(args)
#endif // endif

#ifdef BCMDBG
#define FLOW_INFORM(args) printf args
#else
#define FLOW_INFORM(args)
#endif // endif

#define MAX_IFIDX	15U

/* private module info */
typedef struct {
	uint16 flowID;
	struct ether_addr ether_addr;	/* SA or DA, depending on ifidx/if_type */
	uint8  ifidx;
	uint8  priority;
} txflow_classification_t;

struct txflow_class_info {
	osl_t *osh;			/* OSL handle */
	uint8 if_type[MAX_IFIDX + 1];	/* the interface type, indexed by interface index */
	uint8 num_entries;		/* the total number of entries in the Flow Table */
	txflow_classification_t *flow_tbl;	/* Pointer to Flow Classification Table */
};

#define TXFLOW_IFIDX_FLAG	(1 << 0U)
#define TXFLOW_PRIORITY_FLAG	(1 << 1U)
#define TXFLOW_SRC_ADDR_FLAG	(1 << 2U)
#define TXFLOW_DEST_ADDR_FLAG	(1 << 3U)

/*
 * matching_bitmask[] indexed by Interface Type, where
 * Interface Type is:
 *	0: DA - IBSS (AWDL, NAN, Ad-hoc, etc.)
 *	1: SA - STA (including GC & PSTA)
 *	2: DA - AP (including GO)
 *	3: NONE - WDS (including some of MESH nodes)
 */
static const uint8 matching_bitmask[IF_TYPE_WDS + 1] = {
	TXFLOW_IFIDX_FLAG | TXFLOW_PRIORITY_FLAG | TXFLOW_DEST_ADDR_FLAG,
	TXFLOW_IFIDX_FLAG | TXFLOW_PRIORITY_FLAG | TXFLOW_SRC_ADDR_FLAG,
	TXFLOW_IFIDX_FLAG | TXFLOW_PRIORITY_FLAG | TXFLOW_DEST_ADDR_FLAG,
	TXFLOW_IFIDX_FLAG | TXFLOW_PRIORITY_FLAG,
};

txflow_class_info_t *
BCMATTACHFN(txflow_classifier_attach)(osl_t *osh)
{
	txflow_class_info_t *flow_classifieri;
	uint8 ifidx;

	if ((flow_classifieri = MALLOCZ(osh, sizeof(*flow_classifieri))) == NULL) {
		FLOW_ERROR(("%s: MALLOC failed, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	flow_classifieri->osh = osh;
	flow_classifieri->num_entries = MAX_TXFLOWS;
	flow_classifieri->flow_tbl = MALLOCZ(osh,
	                                     sizeof(*(flow_classifieri->flow_tbl)) *
	                                     flow_classifieri->num_entries);
	if (flow_classifieri->flow_tbl == NULL) {
		FLOW_ERROR(("%s: MALLOC failed, malloced %d bytes\n", __FUNCTION__, MALLOCED(osh)));
		goto fail;
	}

	for (ifidx = 0; ifidx <= MAX_IFIDX; ++ifidx) {
		flow_classifieri->if_type[ifidx] = IF_TYPE_NONE;
	}

	return flow_classifieri;

fail:
	txflow_classifier_detach(flow_classifieri);

	return NULL;
}

void
BCMATTACHFN(txflow_classifier_detach)(txflow_class_info_t *flow_classifieri)
{
	if (flow_classifieri == NULL)
		return;

	if (flow_classifieri->flow_tbl != NULL) {
		MFREE(flow_classifieri->osh, flow_classifieri->flow_tbl,
		      sizeof(*(flow_classifieri->flow_tbl)) * flow_classifieri->num_entries);
	}

	MFREE(flow_classifieri->osh, flow_classifieri, sizeof(*flow_classifieri));
}

/*
 * txflow_classifier_set_if_type()
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	ifidx           : the interface index (0 - 15)
 *	if_type         : the interface type associated with the given index (0 - 3)
 */
void
txflow_classifier_set_if_type(txflow_class_info_t *flow_classifieri, uint8 ifidx, uint8 if_type)
{
	ASSERT(flow_classifieri != NULL);
	ASSERT(ifidx < sizeof(flow_classifieri->if_type));
	ASSERT(if_type < sizeof(matching_bitmask));

	flow_classifieri->if_type[ifidx] = if_type;
}

/*
 * txflow_classifier_add()
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	txhdr           : packet's tx ether header
 *	ifidx           : packet's interface index
 *	priority        : packet's priority
 *	flowID          : the ID of the flow to add to the classification table
 * Output:
 *	BCME_OK if success, or BCME_NORESOURCE if table is full
 */
int
txflow_classifier_add(txflow_class_info_t *flow_classifieri,
                      const uint8 txhdr[], uint8 ifidx, uint8 priority, uint16 flowID)
{
	txflow_classification_t *flow_tbl;
	uint8 i;
	uint8 if_type;

	ASSERT(flow_classifieri != NULL);
	ASSERT(flow_classifieri->num_entries != 0);
	ASSERT(ifidx < sizeof(flow_classifieri->if_type));
	ASSERT(FLOW_ID_VALID(flowID));

	flow_tbl = flow_classifieri->flow_tbl;

	/* scan table for first empty slot */
	for (i = 0; FLOW_ID_VALID(flow_tbl[i].flowID) && i < flow_classifieri->num_entries; ++i);

	/* already full table? */
	if (i == flow_classifieri->num_entries) {
		FLOW_INFORM(("%s: Flow Classification table full\n", __FUNCTION__));

		return BCME_NORESOURCE;
	}

	if_type = flow_classifieri->if_type[ifidx];

	/* assert trigger if ifidx => if_type is not yet configured */
	ASSERT(if_type < sizeof(matching_bitmask));

	/* add the entry */
	flow_tbl[i].flowID = flowID;
	flow_tbl[i].priority = priority;
	flow_tbl[i].ifidx = ifidx;

	if (matching_bitmask[if_type] & TXFLOW_SRC_ADDR_FLAG) {
		flow_tbl[i].ether_addr = *(const struct ether_addr *)&txhdr[ETHER_SRC_OFFSET];
	} else if (matching_bitmask[if_type] & TXFLOW_DEST_ADDR_FLAG) {
		flow_tbl[i].ether_addr = *(const struct ether_addr *)&txhdr[ETHER_DEST_OFFSET];
	} else {
		flow_tbl[i].ether_addr = ether_null;
	}

	FLOW_INFORM(("%s: Added FlowID %d to idx %d of classification table\n",
		__FUNCTION__, flowID, i));

	return BCME_OK;
}

/*
 * txflow_classifier_delete()
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	flowID          : the ID of the flow that we no longer use
 * Output:
 *	BCME_OK or flowID BCME_NOTFOUND
 */
int
txflow_classifier_delete(txflow_class_info_t *flow_classifieri, uint16 flowID)
{
	txflow_classification_t *flow_tbl;
	uint8 i;

	ASSERT(flow_classifieri != NULL);
	ASSERT(flow_classifieri->num_entries != 0);
	ASSERT(FLOW_ID_VALID(flowID));

	flow_tbl = flow_classifieri->flow_tbl;

	/* scan table for flowID match */
	for (i = 0; flow_tbl[i].flowID != flowID && i < flow_classifieri->num_entries; ++i);

	if (i == flow_classifieri->num_entries) {
		return BCME_NOTFOUND;
	}

	memset(&flow_tbl[i], 0, sizeof(*flow_tbl));

	return BCME_OK;
}

/*
 * txflow_classifier_get_flow()
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	txhdr           : packet's tx ether header
 *	ifidx           : packet's interface index
 *	priority        : packet's priority
 * Output:
 *	flowID          : if BCME_OK, the ID of the flow if found match in the classification table
 *	BCME_OK or BCME_NOTFOUND
 */
int
txflow_classifier_get_flow(txflow_class_info_t *flow_classifieri,
                           const uint8 txhdr[], uint8 ifidx, uint8 priority, uint16 *flowID)
{
	txflow_classification_t *flow_tbl;
	uint8 i;
	uint8 if_type;
	bool  match_found = FALSE;

	ASSERT(flow_classifieri != NULL);
	ASSERT(flow_classifieri->num_entries != 0);
	ASSERT(ifidx < sizeof(flow_classifieri->if_type));

	if_type = flow_classifieri->if_type[ifidx];

	/* assert trigger if ifidx => if_type is not yet configured */
	ASSERT(if_type < sizeof(matching_bitmask));

	flow_tbl = flow_classifieri->flow_tbl;

	/* scan table for match */
	i = 0;
	do {
		/* first find matching ifidx and priority */
		while ((!FLOW_ID_VALID(flow_tbl[i].flowID) ||
		       flow_tbl[i].priority != priority || flow_tbl[i].ifidx != ifidx) &&
		       i < flow_classifieri->num_entries) {
			++i;
		}

		if (i < flow_classifieri->num_entries) {
			/* found matching ifidx and priority */

			if ((matching_bitmask[if_type] &
			    (TXFLOW_SRC_ADDR_FLAG | TXFLOW_DEST_ADDR_FLAG)) == 0) {
				match_found = TRUE;	/* ifidx and priority match */
			} else {
				const struct ether_addr *ether_addr;

				if (matching_bitmask[if_type] & TXFLOW_SRC_ADDR_FLAG) {
					ether_addr = (const struct ether_addr *)
							&txhdr[ETHER_SRC_OFFSET];
				} else {
					ether_addr = (const struct ether_addr *)
							&txhdr[ETHER_DEST_OFFSET];
				}

				if (ether_cmp(&flow_tbl[i].ether_addr, ether_addr) == 0) {
					match_found = TRUE; /* the ether_addr (SA or DA) matches */
				} else {
					++i;	/* skip ... ether_addr doesn't match */
				}
			}
		}
	} while (i < flow_classifieri->num_entries && !match_found);

	if (!match_found) {
		return BCME_NOTFOUND;
	}

	*flowID = flow_tbl[i].flowID;

	return BCME_OK;
}

#if defined(BCMDBG)
/*
 * if_type_to_str() and ifidx_to_match_str()
 *
 * local functions for debugging to convert interface index/type to string.
 */
static const char *
if_type_to_str(uint8 if_type)
{
	static const char *if_type_str[] = { "IBSS", "STA", "AP", "WDS" };

	if (if_type < ARRAYSIZE(if_type_str)) {
		return if_type_str[if_type];
	}

	return (if_type == IF_TYPE_NONE) ? "NONE" : "UNDEFINED";
}

static char *
ifidx_to_match_str(txflow_class_info_t *flow_classifieri, uint8 ifidx)
{
	uint8 if_type = flow_classifieri->if_type[ifidx];

	if (matching_bitmask[if_type] & TXFLOW_SRC_ADDR_FLAG) {
		return "SA";
	}

	if (matching_bitmask[if_type] & TXFLOW_DEST_ADDR_FLAG) {
		return "DA";
	}

	return "";
}

/*
 * txflow_classifier_dump()
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	b               : string buffer to fill with info (need to use bcm_binit before use)
 */
void
txflow_classifier_dump(txflow_class_info_t *flow_classifieri, struct bcmstrbuf *b)
{
	txflow_classification_t *flow_tbl;
	uint8 i;

	ASSERT(flow_classifieri != NULL);
	ASSERT(b != NULL);

	bcm_bprintf(b, "ifidx\tif type\n");

	for (i = 0; i <= MAX_IFIDX; ++i) {
		bcm_bprintf(b, "%d\t%s\n", i, if_type_to_str(flow_classifieri->if_type[i]));
	}

	bcm_bprintf(b, "\nFlowID\tether_addr\t\tifidx\tpriority\n");
	flow_tbl = flow_classifieri->flow_tbl;

	for (i = 0; i < flow_classifieri->num_entries; ++i) {
		bcm_bprintf(b, "%d\t"MACF" (%s)\t%d\t%d\n",
		            flow_tbl[i].flowID,
		            ETHER_TO_MACF(flow_tbl[i].ether_addr),
		            ifidx_to_match_str(flow_classifieri, flow_tbl[i].ifidx),
		            flow_tbl[i].ifidx,
		            flow_tbl[i].priority);
	}
}
#endif // endif
