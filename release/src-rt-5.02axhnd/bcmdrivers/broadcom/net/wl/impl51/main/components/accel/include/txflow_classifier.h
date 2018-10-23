/*
 * txflow_classifier.h
 *
 * Common interface for TX Flow Classification module
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
#ifndef _txflow_classifier_h
#define _txflow_classifier_h

#include <typedefs.h>
#include <osl.h>

struct txflow_class_info;	/* opaque type */
typedef struct txflow_class_info txflow_class_info_t;

txflow_class_info_t *txflow_classifier_attach(osl_t *osh);
void txflow_classifier_detach(txflow_class_info_t *flow_classifieri);

#define FLOW_ID_VALID(flowID)	((flowID) > 0 && (flowID) < 0xFFF)

/* The interface type corresponding to dot11 BSS type */
#define IF_TYPE_IBSS	0U
#define IF_TYPE_STA	1U
#define IF_TYPE_AP	2U
#define IF_TYPE_WDS	3U
#define IF_TYPE_NONE	255U

/*
 * Associate an interface type with an interface index.
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	ifidx           : the interface index (0 - 15)
 *	if_type         : the interface type associated with the given index (0 - 3)
 */
void txflow_classifier_set_if_type(txflow_class_info_t *flow_classifieri,
                                   uint8 ifidx, uint8 if_type);

/*
 * Add a TX classification (given flowID and packet header)
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
int txflow_classifier_add(txflow_class_info_t *flow_classifieri,
                          const uint8 txhdr[], uint8 ifidx, uint8 priority, uint16 flowID);

/*
 * Delete a TX classifiction (given flowID).
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	flowID          : the ID of the flow that we no longer use
 * Output:
 *	BCME_OK or flowID BCME_NOTFOUND
 */
int txflow_classifier_delete(txflow_class_info_t *flow_classifieri, uint16 flowID);

/*
 * Find a matching Flow from the given packet (based on match bitmask)
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
int txflow_classifier_get_flow(txflow_class_info_t *flow_classifieri,
                               const uint8 txhdr[], uint8 ifidx, uint8 priority, uint16 *flowID);

#if defined(BCMDBG)
/*
 * Dump the internals (in human readable form) to the given buffer
 *
 * Inputs:
 *	flow_classifieri: flow classifier module handle (returned from txflow_classifier_attach)
 *	b               : string buffer to fill with info (need to use bcm_binit before use)
 */
void txflow_classifier_dump(txflow_class_info_t *flow_classifieri, struct bcmstrbuf *b);
#endif // endif

#endif /* _txflow_classifier_h */
