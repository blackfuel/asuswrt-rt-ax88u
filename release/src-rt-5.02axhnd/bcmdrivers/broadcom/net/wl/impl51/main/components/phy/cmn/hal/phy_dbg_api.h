/*
 * Debug module public interface (to MAC driver).
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
 * $Id: phy_dbg_api.h 729400 2017-10-31 18:34:19Z $
 */

#ifndef _phy_dbg_api_h_
#define _phy_dbg_api_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <phy_api.h>

#define TAG_TYPE_PHY               0xf0
#define TAG_TYPE_RAD               0xf1

/* Error Types */
typedef enum {
	PHY_RC_NONE						= 0,
	PHY_RC_TXPOWER_LIMITS			= 1,
	PHY_RC_TEMPSENSE_LIMITS			= 2,
	PHY_RC_VCOCAL_FAILED			= 3,
	PHY_RC_PLL_NOTLOCKED			= 4,
	PHY_RC_DESENSE_LIMITS			= 5,
	PHY_RC_BASEINDEX_LIMITS			= 6,
	PHY_RC_TXCHAIN_INVALID			= 7,
	PHY_RC_PMUCAL_FAILED			= 8,
	PHY_RC_RCAL_INVALID				= 9,
	PHY_RC_FCBS_CHSW_FAILED			= 10,
	PHY_RC_IQEST_FAILED				= 11,
	PHY_RC_RESET2RX_FAILED			= 12,
	PHY_RC_PKTPROC_RESET_FAILED		= 13,
	PHY_RC_RFSEQ_STATUS_INVALID		= 14,
	PHY_RC_TXIQLO_CAL_FAILED		= 15,
	PHY_RC_PAPD_CAL_FAILED			= 16,
	PHY_RC_NOISE_CAL_FAILED			= 17,
	PHY_RC_RFSEQ_STATUS_OCL_INVALID	= 18,
	PHY_RC_RX2TX_FAILED				= 19,
	PHY_RC_AFE_CAL_FAILED			= 20,
	PHY_RC_NOMEM					= 21,
	PHY_RC_SAMPLEPLAY_LIMIT			= 22,
	PHY_RC_RCCAL_INVALID			= 23,
	PHY_RC_IDLETSSI_INVALID			= 24,
	PHY_RC_REUSE_BUFFER_SIZE_ERROR		= 25,
	PHY_RC_REUSE_BUFFER_LOCK_ERROR		= 26,
	PHY_RC_CALCACHEBUFACQ_FAILED		= 27,
	PHY_RC_CALCACHEBUFREL_FAILED		= 28,
	PHY_RC_LAST			/* This must be the last entry */
} phy_crash_reason_t;

typedef struct phy_dbg_ext_trap_err {
	uint16 err;
	uint16 RxFeStatus;
	uint16 TxFIFOStatus0;
	uint16 TxFIFOStatus1;
	uint16 RfseqMode;
	uint16 RfseqStatus0;
	uint16 RfseqStatus1;
	uint16 RfseqStatus_Ocl;
	uint16 RfseqStatus_Ocl1;
	uint16 OCLControl1;
	uint16 TxError;
	uint16 bphyTxError;
	uint16 TxCCKError;
	uint16 TxCtrlWrd0;
	uint16 TxCtrlWrd1;
	uint16 TxCtrlWrd2;
	uint16 TxLsig0;
	uint16 TxLsig1;
	uint16 TxVhtSigA10;
	uint16 TxVhtSigA11;
	uint16 TxVhtSigA20;
	uint16 TxVhtSigA21;
	uint16 txPktLength;
	uint16 txPsdulengthCtr;
	uint16 gpioClkControl;
	uint16 gpioSel;
	uint16 pktprocdebug;
	uint32 gpioOut[3];
} phy_dbg_ext_trap_err_t;

/*
 * Invoke dump function for module 'name'. Return BCME_XXXX.
 */
int phy_dbg_dump(phy_info_t *pi, const char *name, struct bcmstrbuf *b);

/*
 * Invoke dump clear function for module 'name'. Return BCME_XXXX.
 */
int phy_dbg_dump_clr(phy_info_t *pi, const char *name);

/*
 * List dump names
 */
int phy_dbg_dump_list(phy_info_t *pi, struct bcmstrbuf *b);

/*
 * Dump dump registry
 */
int phy_dbg_dump_dump(phy_info_t *pi, struct bcmstrbuf *b);

/*
 * Dump phy txerr
 */
void phy_dbg_txerr_dump(phy_info_t *pi, uint16 err);

#if defined(DNG_DBGDUMP)
/*
 * Print phy debug registers
 */
extern void wlc_phy_print_phydbg_regs(wlc_phy_t *ppi);
#endif /* DNG_DBGDUMP */

#if defined(BCMDBG) || defined(WL_MACDBG)
/*
 * Enable GPIO Out
 */
void phy_dbg_gpio_out_enab(phy_info_t *pi, bool enab);
#endif // endif

/*
 * phy and radio register binary dump size
 */
int phy_dbg_dump_getlistandsize(wlc_phy_t *ppi, uint8 type);

/*
* phy and radio register binary dump
*/
int phy_dbg_dump_tbls(wlc_phy_t *ppi, uint8 type, uchar *p, int * buf_len, bool fatal_error);

#if (defined(BCMDBG) && defined(DBG_PHY_IOV)) || defined(BCMDBG_PHYDUMP)
/*
* Create pages for dump.
*/
int phy_dbg_page_parser(phy_info_t *pi, uint8 type, int buf_len);
#endif // endif

#if defined(AWD_EXT_TRAP)
int phy_dbg_dump_ext_trap(phy_info_t *pi, phy_dbg_ext_trap_err_t *ext_trap);
#endif /* EXT_TAP */

#endif /* _phy_dbg_api_h_ */
