/*
 * AWD component interface file.
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
 * $Id:$
 */

#ifndef _AWD_H_
#define _AWD_H_

#ifdef AWD_EXT_TRAP
#include <hnd_trap.h>
#endif // endif

/* Tags for structures being used by awd_data info iovar.
 * Related structures are defined in wlioctl.h.
 */
#define AWD_DATA_TAG_JOIN_CLASSIFICATION_INFO 10 /* general information about join request */
#define AWD_DATA_TAG_JOIN_TARGET_CLASSIFICATION_INFO 11	/* per target (AP) join information */
#define AWD_DATA_TAG_ASSOC_STATE 12 /* current state of the Device association state machine */
#define AWD_DATA_TAG_CHANNEL 13	/* current channel on which the association was performed */
#define AWD_DATA_TAG_TOTAL_NUM_OF_JOIN_ATTEMPTS 14 /* number of join attempts (bss_retries) */

#ifndef _LANGUAGE_ASSEMBLY

#define HND_EXTENDED_TRAP_VERSION  1
#define HND_EXTENDED_TRAP_BUFLEN   256

typedef struct hnd_ext_trap_hdr {
	uint8 version;    /* Extended trap version info */
	uint8 reserved;   /* currently unused */
	uint16 len;       /* Length of data excluding this header */
	uint8 data[];     /* TLV data */
} hnd_ext_trap_hdr_t;

#define TAG_TRAP_SIGNATURE       1  /* Processor register dumps */
#define TAG_TRAP_STACK           2  /* Processor stack dump (possible code locations) */
#define TAG_TRAP_MEMORY          3  /* Memory subsystem dump */
#define TAG_TRAP_DEEPSLEEP       4  /* Deep sleep health check failures */
#define TAG_TRAP_PSM_WD          5  /* PSM watchdog information */
#define TAG_TRAP_PHY             6  /* Phy related issues */
#define TAG_TRAP_BUS             7  /* Bus level issues */
#define TAG_TRAP_MAC_SUSP        8  /* Mac level suspend issues */
#define TAG_TRAP_BACKPLANE       9  /* Backplane related errors */
/*
 * Values 10 through 14 are in use by awd_data info iovar.
 */
#define TAG_TRAP_PCIE_Q         15  /* PCIE Queue state during memory trap */
#define TAG_TRAP_WLC_STATE      16  /* WLAN state during memory trap */
#define TAG_TRAP_MAC_WAKE       17  /* Mac level wake issues */

typedef struct hnd_ext_trap_bp_err
{
	uint32 error;
	uint32 coreid;
	uint32 baseaddr;
	uint32 ioctrl;
	uint32 iostatus;
	uint32 resetctrl;
	uint32 resetstatus;
	uint32 errlogctrl;
	uint32 errlogdone;
	uint32 errlogstatus;
	uint32 errlogaddrlo;
	uint32 errlogaddrhi;
	uint32 errlogid;
	uint32 errloguser;
	uint32 errlogflags;
} hnd_ext_trap_bp_err_t;

#define HND_EXT_TRAP_PSMWD_INFO_VER	1
typedef struct hnd_ext_trap_psmwd {
	uint16 xtag;
	uint16 version; /* version of the information following this */
	uint32 i32_maccontrol;
	uint32 i32_maccommand;
	uint32 i32_macintstatus;
	uint32 i32_phydebug;
	uint32 i32_clk_ctl_st;
	uint32 i32_psmdebug[3];
	uint16 i16_0x1a8; /* gated clock en */
	uint16 i16_0x406; /* Rcv Fifo Ctrl */
	uint16 i16_0x408; /* Rx ctrl 1 */
	uint16 i16_0x41a; /* Rxe Status 1 */
	uint16 i16_0x41c; /* Rxe Status 2 */
	uint16 i16_0x424; /* rcv wrd count 0 */
	uint16 i16_0x426; /* rcv wrd count 1 */
	uint16 i16_0x456; /* RCV_LFIFO_STS */
	uint16 i16_0x480; /* PSM_SLP_TMR */
	uint16 i16_0x490; /* PSM BRC */
	uint16 i16_0x500; /* TXE CTRL */
	uint16 i16_0x50e; /* TXE Status */
	uint16 i16_0x55e; /* TXE_xmtdmabusy */
	uint16 i16_0x566; /* TXE_XMTfifosuspflush */
	uint16 i16_0x690; /* IFS Stat */
	uint16 i16_0x692; /* IFS_MEDBUSY_CTR */
	uint16 i16_0x694; /* IFS_TX_DUR */
	uint16 i16_0x6a0; /* SLow_CTL */
	uint16 i16_0x838; /* TXE_AQM fifo Ready */
	uint16 i16_0x8c0; /* Dagg ctrl */
	uint16 shm_prewds_cnt;
	uint16 shm_txtplufl_cnt;
	uint16 shm_txphyerr_cnt;
} hnd_ext_trap_psmwd_t;

#define HEAP_HISTOGRAM_DUMP_LEN	6
#define HEAP_MAX_SZ_BLKS_LEN	2

typedef struct hnd_ext_trap_heap_err {
	uint32 arena_total;
	uint32 heap_free;
	uint32 heap_inuse;
	uint32 heap_inuse_plus_ohd;
	uint32 mf_count;
	uint32 stack_lwm;
	uint16 heap_histogm[HEAP_HISTOGRAM_DUMP_LEN * 2]; /* size/number */
	uint16 max_sz_free_blk[HEAP_MAX_SZ_BLKS_LEN];
} hnd_ext_trap_heap_err_t;

#define MEM_TRAP_NUM_WLC_TX_QUEUES	6
typedef struct hnd_ext_trap_wlc_mem_err {
	uint8 instance;
	uint8 associated;
	uint8 ap_count;
	uint8 soft_ap_client_cnt;
	uint16 txqueue_len[MEM_TRAP_NUM_WLC_TX_QUEUES];
	uint8 awdl_peer_cnt;
	uint8 PAD[3];
} hnd_ext_trap_wlc_mem_err_t;

typedef struct hnd_ext_trap_pcie_mem_err {
	uint16 d2h_queue_len;
	uint16 d2h_req_queue_len;
} hnd_ext_trap_pcie_mem_err_t;

#define HND_EXT_TRAP_MACSUSP_INFO_VER	1
typedef struct hnd_ext_trap_macsusp {
	uint16 xtag;
	uint8 version; /* version of the information following this */
	uint8 trap_reason;
	uint32 i32_maccontrol;
	uint32 i32_maccommand;
	uint32 i32_macintstatus;
	uint32 i32_phydebug[4];
	uint32 i32_psmdebug[8];
	uint16 i16_0x41a; /* Rxe Status 1 */
	uint16 i16_0x41c; /* Rxe Status 2 */
	uint16 i16_0x490; /* PSM BRC */
	uint16 i16_0x50e; /* TXE Status */
	uint16 i16_0x55e; /* TXE_xmtdmabusy */
	uint16 i16_0x566; /* TXE_XMTfifosuspflush */
	uint16 i16_0x690; /* IFS Stat */
	uint16 i16_0x692; /* IFS_MEDBUSY_CTR */
	uint16 i16_0x694; /* IFS_TX_DUR */
	uint16 i16_0x7c0; /* WEP CTL */
	uint16 i16_0x838; /* TXE_AQM fifo Ready */
	uint16 i16_0x880; /* MHP_status */
	uint16 shm_prewds_cnt;
	uint16 shm_ucode_dbgst;
} hnd_ext_trap_macsusp_t;

#define HND_EXT_TRAP_MACENAB_INFO_VER	1
typedef struct hnd_ext_trap_macenab {
	uint16 xtag;
	uint8 version; /* version of the information following this */
	uint8 trap_reason;
	uint32 i32_maccontrol;
	uint32 i32_maccommand;
	uint32 i32_macintstatus;
	uint32 i32_psmdebug[8];
	uint32 i32_clk_ctl_st;
	uint32 i32_powerctl;
	uint16 i16_0x1a8; /* gated clock en */
	uint16 i16_0x480; /* PSM_SLP_TMR */
	uint16 i16_0x490; /* PSM BRC */
	uint16 i16_0x600; /* TSF CTL */
	uint16 i16_0x690; /* IFS Stat */
	uint16 i16_0x692; /* IFS_MEDBUSY_CTR */
	uint16 i16_0x6a0; /* SLow_CTL */
	uint16 i16_0x6a6; /* SLow_FRAC */
	uint16 i16_0x6a8; /* fast power up delay */
	uint16 i16_0x6aa; /* SLow_PER */
	uint16 shm_ucode_dbgst;
	uint16 PAD;
} hnd_ext_trap_macenab_t;

#ifdef AWD_EXT_TRAP

#define AWD_EXT_TRAP_SW_FLAG_MEM		0x00000001

void awd_ext_trap_init(osl_t *osh);
int awd_register_trap_ext_callback(void *cb, void *arg);
int (awd_register_trap_ext_callback_late)(void *cb, void *arg);
uint32 *awd_get_trap_ext_data(void);
uint32 awd_get_trap_ext_swflags(void);
void awd_set_trap_ext_swflag(uint32 flag);
void awd_notify_trap_ext_callback(trap_t *tr);
#endif /* AWD_EXT_TRAP */

#endif /* !LANGUAGE_ASSEMBLY */

#endif /* _AWD_H_ */
