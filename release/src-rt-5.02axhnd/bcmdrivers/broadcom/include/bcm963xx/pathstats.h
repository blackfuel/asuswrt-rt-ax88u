#ifndef __PATHSTATS_H_INCLUDED__
#define __PATHSTATS_H_INCLUDED__
/*
*
*  Copyright 2017, Broadcom Corporation
*
* <:label-BRCM:2017:proprietary:standard
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#include <pktHdr.h>
#include <fcache.h>

#define CC_CONFIG_FCACHE_PATHSTAT_DBGLVL    0
#define CC_CONFIG_PATHSTAT_COLOR

#if defined(CC_CONFIG_FCACHE_DEBUG)    /* Runtime debug level setting */
int fcachePathstatDebug(int lvl);
#endif


/*
 *------------------------------------------------------------------------------
 * Implementation Constants 
 *------------------------------------------------------------------------------
 */

/*
 *------------------------------------------------------------------------------
 * PATHSTAT bind to Fcache call
 *------------------------------------------------------------------------------
 */
extern void path_bind_fc(int enable_flag);

/*
 *------------------------------------------------------------------------------
 * Function     : path_query_dev_stat
 * Description  : Query path stats of virtual device, invoked on fc_notify.
 *                fc_notify FETCH_NETIF_STATS query active bStat on virtual device.
 *------------------------------------------------------------------------------
 */
extern int path_query_dev_stat(void * net_p, BlogStats_t * sw_stats_p, BlogStats_t * hw_stats_p);    

/*
 *------------------------------------------------------------------------------
 * Function     : path_fhw_alloc
 * Description  : Allocate hw_table based on fhw resource
 * Design Note  : Invoked by fc_bind_fhw() in fcache.c
 *------------------------------------------------------------------------------
 */
extern int path_fhw_alloc(int hwacc_counter_num);

#endif  /* defined(__PATHSTATS_H_INCLUDED__) */
