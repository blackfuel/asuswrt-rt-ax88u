#ifndef __PKT_RUNNER_PROTO_H_INCLUDED__
#define __PKT_RUNNER_PROTO_H_INCLUDED__

/*
<:copyright-BRCM:2013:proprietary:standard

   Copyright (c) 2013 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:> 
*/

#if defined(CONFIG_BLOG_IPV6)
#define CC_PKTRUNNER_IPV6
#endif


#if defined(CONFIG_BCM_RDPA_MCAST) || defined(RDP_SIM) || defined(CONFIG_BCM963158)
#define CC_PKTRUNNER_MCAST
#endif

#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158)
#define CC_PKTRUNNER_BRCM_TAG
#endif

/*******************************************************************************
 *
 * Debugging
 *
 *******************************************************************************/
#if defined(RDP_SIM)
#define bcmLog_setLogLevel(arg...)
#define __print(fmt, arg...) bdmf_trace(fmt, ##arg)
#define BCM_LOG_ERROR(x, fmt, arg...) __print(fmt, ##arg)
#define isLogDebug 1
#define __logDebug(fmt, arg...)     bdmf_trace(fmt, ##arg)
#define __logInfo(fmt, arg...)      bdmf_trace(fmt, ##arg)
#define __logNotice(fmt, arg...)    bdmf_trace(fmt, ##arg)
#define __logError(fmt, arg...)     bdmf_trace(fmt, ##arg)
#define __debug(fmt, arg...)        bdmf_trace(fmt, ##arg)
#define __dumpCmdList(_cmdList)     cmdlist_dump((_cmdList), RDPA_CMD_LIST_UCAST_LIST_SIZE_32)
#define __dumpPartialCmdList()      cmdlist_dump_partial(); __print("\n")
#define __dumpBlog(_blog_p)         
#define __dumpBlogRule(_blogRule_p) 
#else
#define __print(fmt, arg...) bcm_printk(fmt, ##arg)
#define isLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_PKTRUNNER, BCM_LOG_LEVEL_DEBUG)
#define __logDebug(fmt, arg...)   BCM_LOG_DEBUG(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logInfo(fmt, arg...)    BCM_LOG_INFO(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logNotice(fmt, arg...)  BCM_LOG_NOTICE(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)
#define __logError(fmt, arg...)   BCM_LOG_ERROR(BCM_LOG_ID_PKTRUNNER, fmt, ##arg)

#define __debug(fmt, arg...)                    \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
            __print(fmt, ##arg); )

#define __dumpCmdList(_cmdList)                                         \
    BCM_LOGCODE(                                                        \
        if(isLogDebug)                                                  \
            cmdlist_dump((_cmdList), RDPA_CMD_LIST_UCAST_LIST_SIZE_32); )

#define __dumpPartialCmdList()                  \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            cmdlist_dump_partial();                 \
            __print("\n");                      \
        } )

#define __dumpBlog(_blog_p)                     \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_dump((_blog_p));               \
            __print("\n");                      \
        } )

#define __dumpBlogRule(_blogRule_p)             \
    BCM_LOGCODE(                                \
        if(isLogDebug)                          \
        {                                       \
            blog_rule_dump((_blogRule_p));      \
            __print("\n");                      \
        } )
#endif // RDP_SIM

/*******************************************************************************
 *
 * Functions
 *
 *******************************************************************************/

#ifndef BRCM_TAG_TYPE2_LEN
#define BRCM_TAG_TYPE2_LEN  4
#endif

static inline int runner_get_hw_entix(uint32_t hwTuple)
{
    return (int)(hwTuple);
}

rdpa_mcast_flow_t *__mcastFlowMalloc(void);
void __mcastFlowFree(rdpa_mcast_flow_t *mcastFlow_p);
uint32_t __enetLogicalPortToPhysicalPort(uint32_t logicalPort);
uint32_t __skbMarkToQueuePriority(uint32_t skbMark);
uint32_t __skbMarkToTrafficClass(uint32_t skbMark);
int __isEnetWanPort(uint32_t logicalPort);
int __lagPortGet(Blog_t *blog_p);
int __isEnetBondedLanWanPort(uint32_t logicalPort);
int __isWlanPhy(Blog_t *blog_p);
int __isTxWlanPhy(Blog_t *blog_p);
int __ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_ip_flow_info_t *ip_flow_p);
int __l2ucastSetFwdAndFilters(Blog_t *blog_p, rdpa_l2_flow_info_t *l2_flow_p);

int __init runnerProto_construct(void);
void __exit runnerProto_destruct(void);

#endif  /* defined(__PKT_RUNNER_PROTO_H_INCLUDED__) */
