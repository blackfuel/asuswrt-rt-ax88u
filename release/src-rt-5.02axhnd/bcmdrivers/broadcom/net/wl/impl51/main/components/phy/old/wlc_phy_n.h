/*
 * ABGPHY module header file
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
 * $Id: wlc_phy_n.h 672101 2016-11-24 06:06:10Z $
 */

#ifndef _wlc_phy_n_h_
#define _wlc_phy_n_h_

#include <typedefs.h>
#include <phy_n_info.h>

/* iovar table */
enum {
	/* OLD PHYTYPE specific ones, to phase out, use unified ones at the end
	 * For each unified, mark the original one as "deprecated".
	 * User scripts should stop using them for new testcases
	 */
	IOV_PHY_OCLSCDENABLE = 100,	/* deprecated */
	IOV_PHY_RXANTSEL,
	IOV_NPHY_5G_PWRGAIN,
	IOV_NPHY_ACI_SCAN,
	IOV_NPHY_BPHY_EVM,
	IOV_NPHY_BPHY_RFCS,
	IOV_NPHY_CALTXGAIN,
	IOV_NPHY_CAL_RESET,
	IOV_NPHY_CAL_SANITY,
	IOV_NPHY_ELNA_GAIN_CONFIG,
	IOV_NPHY_ENABLERXCORE,
	IOV_NPHY_EST_TONEPWR,
	IOV_NPHY_FORCECAL,		/* deprecated */
	IOV_NPHY_GAIN_BOOST,
	IOV_NPHY_GPIOSEL,
	IOV_NPHY_HPVGA1GAIN,
	IOV_NPHY_INITGAIN,
	IOV_NPHY_PAPDCAL,
	IOV_NPHY_PAPDCALINDEX,
	IOV_NPHY_PAPDCALTYPE,
	IOV_NPHY_PERICAL,		/* deprecated */
	IOV_NPHY_PHYREG_SKIPCNT,
	IOV_NPHY_PHYREG_SKIPDUMP,
	IOV_NPHY_RFSEQ,
	IOV_NPHY_RFSEQ_TXGAIN,
	IOV_NPHY_RSSICAL,
	IOV_NPHY_RSSISEL,
	IOV_NPHY_RXCALPARAMS,
	IOV_NPHY_RXIQCAL,
	IOV_NPHY_SCRAMINIT,
	IOV_NPHY_SKIPPAPD,
	IOV_NPHY_TBLDUMP_MAXIDX,
	IOV_NPHY_TBLDUMP_MINIDX,
	IOV_NPHY_TEMPOFFSET,
	IOV_NPHY_TEMPSENSE,		/* deprecated */
	IOV_NPHY_TEST_TSSI,
	IOV_NPHY_TEST_TSSI_OFFS,
	IOV_NPHY_TXIQLOCAL,
	IOV_NPHY_TXPWRCTRL,		/* deprecated */
	IOV_NPHY_TXPWRINDEX,		/* deprecated */
	IOV_NPHY_TX_TEMP_TONE,
	IOV_NPHY_TX_TONE,
	IOV_NPHY_VCOCAL,
	IOV_NPHY_CCK_PWR_OFFSET		/* deprecated */
};

extern void
wlc_phy_rfctrl_override_nphy_rev7(phy_info_t *pi, uint16 field, uint16 value, uint8 core_mask,
                                  uint8 off, uint8 override_id);

extern void
wlc_phy_rfctrl_override_nphy(phy_info_t *pi, uint16 field, uint16 value, uint8 core_mask,
                             uint8 off);

/* ********************* REMOVE *********************** */
int16 wlc_phy_rxgaincode_to_dB_nphy(phy_info_t *pi, uint16 gain_code);
void wlc_phy_set_spurmode_nphy(phy_info_t *pi, chanspec_t chanspec);
void wlc_phy_nphy_tkip_rifs_war(phy_info_t *pi, uint8 rifs);
#endif /* _wlc_phy_n_h_ */
