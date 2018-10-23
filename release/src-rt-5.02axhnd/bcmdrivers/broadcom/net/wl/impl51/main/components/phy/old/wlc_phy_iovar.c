/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abg
 * PHY iovar processing of Broadcom BCM43XX 802.11abg
 * Networking Device Driver.
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
 * $Id: wlc_phy_iovar.c 764806 2018-06-04 23:04:30Z $
 */

/*
 * This file contains high portion PHY iovar processing and table.
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <wlc_iocv_types.h>
#include <wlc_iocv_reg.h>
#include <phy_calmgr_api.h>
#include <phy_stf.h>
#include <wlc_phy_iovar.h>
#include <phy_utils_pmu.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_n.h>
#include <phy_temp_api.h>

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(BCMDBG)
static void wlc_phy_iovar_set_papd_offset(phy_info_t *pi, int16 int_val);
static int wlc_phy_iovar_get_papd_offset(phy_info_t *pi);
#endif // endif

/* ************************************************************************************ */
/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
/* ************************************************************************************ */
/* OLD, PHYTYPE specific iovars, to phase out */
static const bcm_iovar_t phy_iovars[] = {
#if NCONF
#if defined(BCMDBG)
	{"nphy_initgain", IOV_NPHY_INITGAIN,
	IOVF_SET_UP, 0, IOVT_UINT16, 0
	},
	{"nphy_hpv1gain", IOV_NPHY_HPVGA1GAIN,
	IOVF_SET_UP, 0, IOVT_INT8, 0
	},
	{"nphy_tx_temp_tone", IOV_NPHY_TX_TEMP_TONE,
	IOVF_SET_UP, 0, IOVT_UINT32, 0
	},
	{"nphy_cal_reset", IOV_NPHY_CAL_RESET,
	IOVF_SET_UP, 0, IOVT_UINT32, 0
	},
	{"nphy_est_tonepwr", IOV_NPHY_EST_TONEPWR,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
	{"phy_est_tonepwr", IOV_PHY_EST_TONEPWR,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
	{"nphy_rfseq_txgain", IOV_NPHY_RFSEQ_TXGAIN,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
	{"phy_spuravoid", IOV_PHY_SPURAVOID,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_INT8, 0
	},
#endif /* defined(BCMDBG) */

#if defined(WLTEST)
	{"nphy_cal_sanity", IOV_NPHY_CAL_SANITY,
	IOVF_SET_UP, 0, IOVT_UINT32, 0
	},
	{"nphy_txiqlocal", IOV_NPHY_TXIQLOCAL,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_rxiqcal", IOV_NPHY_RXIQCAL,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_rxcalparams", IOV_NPHY_RXCALPARAMS,
	(0), 0, IOVT_UINT32, 0
	},
	{"nphy_rssisel", IOV_NPHY_RSSISEL,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_rssical", IOV_NPHY_RSSICAL,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_gpiosel", IOV_PHY_GPIOSEL,
	(IOVF_MFG), 0, IOVT_UINT16, 0
	},
	{"nphy_elna_gain_config", IOV_NPHY_ELNA_GAIN_CONFIG,
	(IOVF_SET_DOWN), 0, IOVT_UINT8, 0
	},
	{"nphy_aci_scan", IOV_NPHY_ACI_SCAN,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_pacaltype", IOV_NPHY_PAPDCALTYPE,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_papdcal", IOV_NPHY_PAPDCAL,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_skippapd", IOV_NPHY_SKIPPAPD,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"nphy_pacalindex", IOV_NPHY_PAPDCALINDEX,
	(IOVF_MFG), 0, IOVT_UINT16, 0
	},
	{"nphy_caltxgain", IOV_NPHY_CALTXGAIN,
	(IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"nphy_tbldump_minidx", IOV_NPHY_TBLDUMP_MINIDX,
	(IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"nphy_tbldump_maxidx", IOV_NPHY_TBLDUMP_MAXIDX,
	(IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"nphy_phyreg_skipdump", IOV_NPHY_PHYREG_SKIPDUMP,
	(IOVF_MFG), 0, IOVT_UINT16, 0
	},
	{"nphy_phyreg_skipcount", IOV_NPHY_PHYREG_SKIPCNT,
	(IOVF_MFG), 0, IOVT_INT8, 0
	},
#endif // endif
#endif /* NCONF */
#if defined(BCMDBG) || defined(WLTEST)
	{"fast_timer", IOV_FAST_TIMER,
	(IOVF_NTRL | IOVF_MFG), 0, IOVT_UINT32, 0
	},
	{"slow_timer", IOV_SLOW_TIMER,
	(IOVF_NTRL | IOVF_MFG), 0, IOVT_UINT32, 0
	},
#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG) || defined(WLTEST) || defined(PHYCAL_CHNG_CS)
	{"glacial_timer", IOV_GLACIAL_TIMER,
	IOVF_NTRL, 0, IOVT_UINT32, 0
	},
#endif // endif
#if defined(WLTEST) || defined(MACOSX) || defined(DBG_PHY_IOV) || defined(ATE_BUILD)
	{"phy_watchdog", IOV_PHY_WATCHDOG,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
#endif // endif
	{"cal_period", IOV_CAL_PERIOD,
	0, 0, IOVT_UINT32, 0
	},
#if defined(WLTEST)
	{"phymsglevel", IOV_PHYHAL_MSG,
	(0), 0, IOVT_UINT32, 0
	},
	{"phy_fixed_noise", IOV_PHY_FIXED_NOISE,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phynoise_polling", IOV_PHYNOISE_POLL,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"carrier_suppress", IOV_CARRIER_SUPPRESS,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
#ifdef BAND5G
	{"subband5gver", IOV_PHY_SUBBAND5GVER,
	0, 0, IOVT_INT8, 0
	},
#endif /* BAND5G */
	{"phy_txrx_chain", IOV_PHY_TXRX_CHAIN,
	(0), 0, IOVT_INT8, 0
	},
	{"phy_bphy_evm", IOV_PHY_BPHY_EVM,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_bphy_rfcs", IOV_PHY_BPHY_RFCS,
	(IOVF_SET_DOWN | IOVF_SET_BAND | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_scraminit", IOV_PHY_SCRAMINIT,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"phy_rfseq", IOV_PHY_RFSEQ,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_tx_tone_hz", IOV_PHY_TX_TONE_HZ,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT32, 0
	},
	{"phy_tx_tone_symm", IOV_PHY_TX_TONE_SYMM,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT32, 0
	},
	{"phy_test_tssi", IOV_PHY_TEST_TSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"phy_test_tssi_offs", IOV_PHY_TEST_TSSI_OFFS,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"phy_test_idletssi", IOV_PHY_TEST_IDLETSSI,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"phy_setrptbl", IOV_PHY_SETRPTBL,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_VOID, 0
	},
	{"phy_forceimpbf", IOV_PHY_FORCEIMPBF,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_VOID, 0
	},
	{"phy_forcesteer", IOV_PHY_FORCESTEER,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
#ifdef BAND5G
	{"phy_5g_pwrgain", IOV_PHY_5G_PWRGAIN,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_UINT8, 0
	},
#endif /* BAND5G */
	{"phy_enrxcore", IOV_PHY_ENABLERXCORE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_activecal", IOV_PHY_ACTIVECAL,
	IOVF_GET_UP, 0, IOVT_UINT8, 0
	},
	{"phy_bbmult", IOV_PHY_BBMULT,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_BUFFER, 0
	},
	{"phy_lowpower_beacon_mode", IOV_PHY_LOWPOWER_BEACON_MODE,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT32, 0
	},
#endif // endif
	{"phy_rx_gainindex", IOV_PHY_RXGAININDEX,
	IOVF_GET_UP | IOVF_SET_UP, 0, IOVT_UINT16, 0
	},
	{"phy_forcecal", IOV_PHY_FORCECAL,
	IOVF_SET_UP, 0, IOVT_UINT8, 0
	},
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
	{"phy_forcecal_obt", IOV_PHY_FORCECAL_OBT,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_UINT8, 0
	},
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WLTEST) || defined(MACOSX)
	{"phy_deaf", IOV_PHY_DEAF,
	(IOVF_GET_UP | IOVF_SET_UP), 0, IOVT_UINT8, 0
	},
#endif // endif
	{"num_stream", IOV_NUM_STREAM,
	(0), 0, IOVT_INT32, 0
	},
	{"min_txpower", IOV_MIN_TXPOWER,
	0, 0, IOVT_UINT32, 0
	},
#if defined(MACOSX)
	{"phywreg_limit", IOV_PHYWREG_LIMIT,
	0, 0, IOVT_UINT32, IOVT_UINT32
	},
#endif // endif
	{"phy_muted", IOV_PHY_MUTED,
	0, 0, IOVT_UINT8, 0
	},
#if defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG)
	{"ntd_gds_lowtxpwr", IOV_NTD_GDS_LOWTXPWR,
	IOVF_GET_UP, 0, IOVT_UINT8, 0
	},
#endif /* defined(WLMEDIA_N2DEV) || defined(WLMEDIA_N2DBG) */
	{"phy_rxantsel", IOV_PHY_RXANTSEL,
	(0), 0, IOVT_UINT8, 0
	},
	{"phy_dssf", IOV_PHY_DSSF,
	IOVF_SET_UP, 0, IOVT_UINT32, 0
	},
	{"phy_bphymrc", IOV_PHY_BPHYMRC,
	(IOVF_SET_DOWN | IOVF_SET_BAND), 0, IOVT_UINT8, 0
	},
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(BCMDBG)
	{"phy_papd_eps_offset", IOV_PAPD_EPS_OFFSET,
	(IOVF_SET_UP | IOVF_MFG), 0, IOVT_INT16, 0
	},
#endif // endif
	{"phydump_page", IOV_PHY_DUMP_PAGE,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
	{"phydump_entry", IOV_PHY_DUMP_ENTRY,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_INT16, 0
	},
	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */
#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
	{"phy_tempsense", IOV_PHY_TEMPSENSE,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
#endif /* BCMDBG || WLTEST || ATE_BUILD || BCMDBG_TEMPSENSE */
#if defined(WLTEST)
	{"phy_cal_disable", IOV_PHY_CAL_DISABLE,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phy_idletssi", IOV_PHY_IDLETSSI_REG,
	(IOVF_GET_UP | IOVF_SET_UP), 0, IOVT_BUFFER, 0
	},
	{"phy_tssi", IOV_PHY_AVGTSSI_REG,
	(IOVF_GET_UP), 0, IOVT_BUFFER, 0
	},
	{"phy_resetcca", IOV_PHY_RESETCCA,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_iqlocalidx", IOV_PHY_IQLOCALIDX,
	(IOVF_GET_UP | IOVF_MFG), 0, IOVT_UINT32, 0
	},
#endif // endif
	{"phy_sromtempsense", IOV_PHY_SROM_TEMPSENSE,
	(IOVF_SET_UP | IOVF_GET_UP | IOVF_MFG), 0, IOVT_INT16, 0
	},
	{"gain_cal_temp", IOV_PHY_GAIN_CAL_TEMP,
	(0), 0, IOVT_INT16, 0
	},
#ifdef PHYMON
	{"phycal_state", IOV_PHYCAL_STATE,
	IOVF_GET_UP, 0, IOVT_UINT32, 0,
	},
#endif /* PHYMON */
#if defined(WLTEST)
	{"aci_exit_check_period", IOV_ACI_EXIT_CHECK_PERIOD,
	(IOVF_MFG), 0, IOVT_UINT32, 0
	},
#endif // endif
#if defined(WLTEST)
	{"phy_glitchthrsh", IOV_PHY_GLITCHK,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_noise_up", IOV_PHY_NOISE_UP,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_noise_dwn", IOV_PHY_NOISE_DWN,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
#endif /* #if defined(WLTEST) */
#if defined(WLTEST)
	{"rpcalvars", IOV_RPCALVARS,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, 2*WL_NUM_RPCALVARS * sizeof(wl_rpcal_t)
	},
#endif // endif
#if defined(WLTEST)
	{"phy_tpc_av", IOV_TPC_AV,
	(IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_BUFFER, 0
	},
	{"phy_tpc_vmid", IOV_TPC_VMID,
	(IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_BUFFER, 0
	},
#endif // endif
#if defined(BCMDBG)
	{"phy_force_fdiqi", IOV_PHY_FORCE_FDIQI,
	IOVF_SET_UP, 0, IOVT_UINT32, 0
	},
#endif /* BCMDBG */
	{"edcrs", IOV_EDCRS,
	(IOVF_SET_UP|IOVF_GET_UP), 0, IOVT_UINT8, 0
	},
	{"phy_afeoverride", IOV_PHY_AFE_OVERRIDE,
	(0), 0, IOVT_UINT8, 0
	},
#if (NCONF || LCN20CONF || ACCONF || ACCONF2)
	{"phy_rssi_gain_cal_temp", IOV_PHY_RSSI_GAIN_CAL_TEMP,
	(0), 0, IOVT_INT32, 0
	},
#endif /* (NCONF || LCN20CONF || ACCONF || ACCONF2) && WLTEST */
#if defined(WLTEST)
	{"phy_auxpga", IOV_PHY_AUXPGA,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, 6*sizeof(uint8)
	},
#endif // endif
#if defined(WLTEST)
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	{"lcnphy_txclampdis", IOV_LCNPHY_TXPWRCLAMP_DIS,
	IOVF_GET_UP, 0, IOVT_UINT8, 0
	},
	{"lcnphy_txclampofdm", IOV_LCNPHY_TXPWRCLAMP_OFDM,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
#endif /* (defined(LCN20CONF) && (LCN20CONF != 0)) */
#endif /* #if defined(WLTEST) */
#if defined(BCMDBG)
	{"lcnphy_txclampcck", IOV_LCNPHY_TXPWRCLAMP_CCK,
	IOVF_GET_UP, 0, IOVT_INT32, 0
	},
#endif // endif

#if defined(WLTEST) || defined(ATE_BUILD)
	{"phy_txpwrctrl", IOV_PHY_TXPWRCTRL,
	(IOVF_MFG), 0, IOVT_UINT8, 0
	},
	{"phy_txpwrindex", IOV_PHY_TXPWRINDEX,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_BUFFER, 0
	},
#endif // endif
#if defined(WLTEST)
	{"patrim", IOV_PATRIM,
	(IOVF_MFG), 0, IOVT_INT32, 0
	},
	{"povars", IOV_POVARS,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, sizeof(wl_po_t)
	},
#endif // endif
	{"sromrev", IOV_SROM_REV,
	(IOVF_SET_DOWN), 0, IOVT_UINT8, 0
	},
#ifdef WLTEST
	{"maxpower", IOV_PHY_MAXP,
	(IOVF_SET_DOWN | IOVF_MFG), 0, IOVT_BUFFER, 0
	},
#endif /* WLTEST */
#ifdef SAMPLE_COLLECT
	{"sample_collect", IOV_PHY_SAMPLE_COLLECT,
	(IOVF_GET_CLK), 0, IOVT_BUFFER, WLC_IOCTL_SMLEN
	},
	{"sample_data", IOV_PHY_SAMPLE_DATA,
	(IOVF_GET_CLK), 0, IOVT_BUFFER, WLC_IOCTL_MEDLEN
	},
	{"sample_collect_gainadj", IOV_PHY_SAMPLE_COLLECT_GAIN_ADJUST,
	0, 0, IOVT_INT8, 0
	},
	{"mac_triggered_sample_collect", IOV_PHY_MAC_TRIGGERED_SAMPLE_COLLECT,
	0, 0, IOVT_BUFFER, WLC_IOCTL_SMLEN
	},
	{"mac_triggered_sample_data", IOV_PHY_MAC_TRIGGERED_SAMPLE_DATA,
	0, 0, IOVT_BUFFER, WLC_IOCTL_MEDLEN
	},
	{"sample_collect_gainidx", IOV_PHY_SAMPLE_COLLECT_GAIN_INDEX,
	0, 0, IOVT_UINT8, 0
	},
	{"iq_metric_data", IOV_IQ_IMBALANCE_METRIC_DATA,
	(IOVF_GET_DOWN | IOVF_GET_CLK), 0, IOVT_BUFFER, WLC_SAMPLECOLLECT_MAXLEN
	},
	{"iq_metric", IOV_IQ_IMBALANCE_METRIC,
	(IOVF_GET_DOWN | IOVF_GET_CLK), 0, IOVT_BUFFER, 0
	},
	{"iq_metric_pass", IOV_IQ_IMBALANCE_METRIC_PASS,
	(IOVF_GET_DOWN | IOVF_GET_CLK), 0, IOVT_BUFFER, 0
	},
#endif /* sample_collect */
#if defined(BCMDBG) || defined(WLTEST)
	{"phy_master_override", IOV_PHY_MASTER_OVERRIDE,
	(IOVF_GET_UP | IOVF_SET_UP | IOVF_MFG), 0, IOVT_INT8, 0
	},
#endif /* BCMDBG  || WLTEST */
	{"phy_bsscolor", IOV_PHY_BSSCOLOR,
	0, 0, IOVT_BUFFER, 0
	},
	{NULL, 0, 0, 0, 0, 0 }
	/* ************************************************************************************ */
	/* [PHY_REARCH] Do not add any variables here. Add them to the individual modules */
	/* ************************************************************************************ */
};

#include <typedefs.h>
/* *********************************************** */
#include <phy_dbg.h>
#include <phy_misc.h>
/* *********************************************** */
#include <bcmdefs.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phyreg_n.h>
#include <wlc_phyreg_ac.h>
#include <wlc_phytbl_n.h>
#include <wlc_phytbl_ac.h>
#include <wlc_phytbl_20691.h>
#include <wlc_phytbl_20693.h>
#include <wlc_phytbl_20694.h>
#include <wlc_phy_radio.h>
#include <wlc_phy_lcn20.h>
#include <wlc_phyreg_lcn20.h>
#include <bcmwifi_channels.h>
#include <bcmotp.h>
#ifdef WLSRVSDB
#include <saverestore.h>
#endif // endif
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_status.h>
#include <phy_utils_reg.h>
#include <phy_utils_channel.h>
#include <phy_utils_api.h>
#include <phy_cache_api.h>
#include <phy_misc_api.h>
#include <phy_btcx.h>
#include <phy_calmgr.h>
#include <phy_temp.h>
#include <phy_tpc.h>
#include <phy_ac_calmgr.h>
#include <phy_noise.h>

#if defined(WLTEST)
#define BFECONFIGREF_FORCEVAL    0x9
#define BFECONFIG0_IMPBF         0xa
#define BFMCON_FORCEVAL          0x8c03
#define BFMCON_RELEASEVAL        0x8c1d
#define REFRESH_THR_FORCEVAL     0xffff
#define REFRESH_THR_RELEASEVAL   0x186a
#define BFRIDX_POS_FORCEVAL      0x100
#define BFRIDX_POS_RELEASEVAL    0x0
#endif // endif

/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  macro, typedef, enum, structure, global variable		*/
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* Decode OFDM PLCP SIGNAL field RATE sub-field bits 0:2 (labeled R1-R3) into
 * 802.11 MAC rate in 500kbps units
 *
 * Table from 802.11-2012, sec 18.3.4.2.
 */
const uint8 ofdm_rate_lookup[] = {
	DOT11_RATE_48M, /* 8: 48Mbps */
	DOT11_RATE_24M, /* 9: 24Mbps */
	DOT11_RATE_12M, /* A: 12Mbps */
	DOT11_RATE_6M,  /* B:  6Mbps */
	DOT11_RATE_54M, /* C: 54Mbps */
	DOT11_RATE_36M, /* D: 36Mbps */
	DOT11_RATE_18M, /* E: 18Mbps */
	DOT11_RATE_9M   /* F:  9Mbps */
};

/* Modularise and clean up attach functions */
static void wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay);
static int wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val);
static int wlc_phy_iovar_set_dssf(phy_info_t *pi, int32 set_val);
static int wlc_phy_iovar_get_dssf(phy_info_t *pi, int32 *ret_val);
static int wlc_phy_iovar_set_bphymrc(phy_info_t *pi, int32 set_val);
static int wlc_phy_iovar_get_bphymrc(phy_info_t *pi, int32 *get_val);
#if defined(BCMDBG) || defined(WLTEST) || defined(MACOSX) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
static int
wlc_phy_iovar_tempsense_paldosense(phy_info_t *pi, int32 *ret_int_ptr, uint8 tempsense_paldosense);
#endif // endif
#if defined(WLTEST)
static int
wlc_phy_iovar_bbmult_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr);
static int wlc_phy_iovar_bbmult_set(phy_info_t *pi, void *p);
#endif // endif
#if defined(WLTEST)
static int
wlc_phy_iovar_avvmid_get(phy_info_t *pi, void *p,
	bool bool_val, int32 *ret_int_ptr, wlc_avvmid_t avvmid_type);
static int wlc_phy_iovar_avvmid_set(phy_info_t *pi, void *p,  wlc_avvmid_t avvmid_type);
#endif // endif
#if defined(WLTEST)
static int wlc_phy_iovar_idletssi_reg(phy_info_t *pi, int32 *ret_int_ptr, int32 int_val, bool set);
static int wlc_phy_iovar_avgtssi_reg(phy_info_t *pi, int32 *ret_int_ptr);
#endif // endif
#if defined(WLTEST)
static int wlc_phy_iovar_txrx_chain(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, bool set);
static void wlc_phy_iovar_bphy_testpattern(phy_info_t *pi, uint8 testpattern, bool enable);
static void wlc_phy_iovar_scraminit(phy_info_t *pi, int8 scraminit);
static int wlc_phy_iovar_force_rfseq(phy_info_t *pi, uint8 int_val);
static void wlc_phy_iovar_tx_tone_hz(phy_info_t *pi, int32 int_val);
static int16 wlc_phy_iovar_test_tssi(phy_info_t *pi, uint8 val, uint8 pwroffset);
static int16 wlc_phy_iovar_test_idletssi(phy_info_t *pi, uint8 val);
static int16 wlc_phy_iovar_setrptbl(phy_info_t *pi);
static int16 wlc_phy_iovar_forceimpbf(phy_info_t *pi);
static int16 wlc_phy_iovar_forcesteer(phy_info_t *pi, uint8 enable);
static void
wlc_phy_iovar_rxcore_enable(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set);
#endif // endif
#if defined(WLTEST) || defined(MACOSX)
static void wlc_phy_iovar_set_deaf(phy_info_t *pi, int32 int_val);
static int wlc_phy_iovar_get_deaf(phy_info_t *pi, int32 *ret_int_ptr);
#endif // endif
static int
wlc_phy_iovar_forcecal(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set);
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || \
	defined(ATE_BUILD)
#ifndef ATE_BUILD
static int
wlc_phy_iovar_forcecal_obt(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set);
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WLTEST) || defined(ATE_BUILD)
static int
wlc_phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set);
static int
wlc_phy_iovar_txpwrindex_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr);
#endif // endif

int phy_iovars_wrapper_sample_collect(phy_info_t *pi, uint32 actionid, uint16 type,
		void *p, uint plen, void *a, int alen, int vsize);

static void
wlc_phy_cal_perical_mphase_schedule(phy_info_t *pi, uint delay_val)
{
	/* for manual mode, let it run */
	if ((pi->phy_cal_mode != PHY_PERICAL_MPHASE) &&
	    (pi->phy_cal_mode != PHY_PERICAL_MANUAL))
		return;

	PHY_CAL(("wlc_phy_cal_perical_mphase_schedule\n"));

	/* use timer to wait for clean context since this
	 * may be called in the middle of nphy_init
	 */
	wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);

	pi->cal_info->cal_phase_id = PHY_CAL_PHASE_INIT;
	wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, delay_val, 0);
}

/* policy entry */
void
wlc_phy_cal_perical(wlc_phy_t *pih, uint8 reason)
{
	int16 current_temp = 0, delta_temp = 0;
	uint delta_time = 0;
	uint8 do_cals = 0;
	bool  suppress_cal = FALSE;

	phy_info_t *pi = (phy_info_t*)pih;

	phy_temp_info_t *ti = pi->tempi;
	phy_txcore_temp_t *temp = phy_temp_get_st(ti);

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = NULL;
#endif // endif

	if (!ISNPHY(pi) && !ISACPHY(pi))
		return;

	/* reset to default */
	pi->cal_info->cal_suppress_count = 0;

	/* do only init noisecal or trigger noisecal when STA
	 * joins to an AP (also trigger noisecal if AP roams)
	 */
	pi->trigger_noisecal = TRUE;

	if ((pi->phy_cal_mode == PHY_PERICAL_DISABLE) ||
	    (pi->phy_cal_mode == PHY_PERICAL_MANUAL))
		return;

	/* NPHY_IPA : disable PAPD cal for following calibration at least 4322A1? */

	PHY_CAL(("wlc_phy_cal_perical: reason %d chanspec 0x%x\n", reason,
	         pi->radio_chanspec));

	/* Update the Tx power per channel on ACPHY for 2GHz channels */
#ifdef POWPERCHANNL
	if (PWRPERCHAN_ENAB(pi)) {
		if (ISACPHY(pi) && !(ACMAJORREV_4(pi->pubpi->phy_rev)))
			wlc_phy_tx_target_pwr_per_channel_decide_run_acphy(pi);
	}
#endif /* POWPERCHANNL */

#if defined(PHYCAL_CACHING)
	ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
#endif // endif

	/* perical is enabled : Either single phase only, or mphase is allowed
	 * Dispatch to s-phase or m-phase based on reasons
	 */
	switch (reason) {

	case PHY_PERICAL_DRIVERUP:	/* always single phase ? */
		break;

	case PHY_PERICAL_PHYINIT:	/* always multi phase */
		if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
#if defined(PHYCAL_CACHING)
			if (ctx)
			{
				/* Switched context so restart a pending MPHASE cal or
				 * restore stored calibration
				 */

				/* If it was pending last time, just restart it */
				if (PHY_PERICAL_MPHASE_PENDING(pi)) {
					/* Delete any existing timer just in case */
					PHY_CAL(("%s: Restarting calibration for 0x%x phase %d\n",
						__FUNCTION__, pi->radio_chanspec,
						pi->cal_info->cal_phase_id));
					wlapi_del_timer(pi->sh->physhim, pi->phycal_timer);
					wlapi_add_timer(pi->sh->physhim, pi->phycal_timer, 0, 0);
				} else if (phy_cache_restore_cal(pi) != BCME_ERROR) {
					break;
				}
			}
			else
#endif /* PHYCAL_CACHING */
			{
				if (PHY_PERICAL_MPHASE_PENDING(pi)) {
					phy_calmgr_mphase_reset(pi->calmgri);
				}

				pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_RESTART;

				/* schedule mphase cal */
				wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_INIT_DELAY);
			}
		}
		break;

	case PHY_PERICAL_JOIN_BSS:
	case PHY_PERICAL_START_IBSS:
	case PHY_PERICAL_UP_BSS:
	case PHY_PERICAL_PHYMODE_SWITCH:

		/* These must run in single phase to ensure clean Tx/Rx
		 * performance so the auto-rate fast-start is promising
		 */

		if ((pi->phy_cal_mode == PHY_PERICAL_MPHASE) && PHY_PERICAL_MPHASE_PENDING(pi)) {
			phy_calmgr_mphase_reset(pi->calmgri);
		}

		/* Always do idle TSSI measurement at the end of NPHY cal
		 * while starting/joining a BSS/IBSS
		 */
		pi->first_cal_after_assoc = TRUE;

		if (ISNPHY(pi))
			pi->u.pi_nphy->cal_type_override = PHY_PERICAL_FULL;

		/* Update last cal temp to current tempsense reading */
		if (temp->phycal_tempdelta) {
			pi->cal_info->last_cal_temp = phy_temp_sense_read((wlc_phy_t *)pi);
		}

		/* Attempt cal cache restore if ctx is valid */
#if defined(PHYCAL_CACHING)
		if (ctx)
		{
			PHY_CAL(("wl%d: %s: Attempting to restore cals on JOIN...\n",
				pi->sh->unit, __FUNCTION__));

			if (phy_cache_restore_cal(pi) == BCME_ERROR) {
				phy_calmgr_cals(pi, PHY_PERICAL_FULL,
					PHY_CAL_SEARCHMODE_RESTART);
			}
		}
		else
#endif /* PHYCAL_CACHING */
		{
			phy_calmgr_cals(pi, PHY_PERICAL_FULL, PHY_CAL_SEARCHMODE_RESTART);
		}
		break;

	case PHY_PERICAL_WATCHDOG:

		if (PUB_NOT_ASSOC(pi) && ISACPHY(pi)) {
			/* Suppress calibration on channel until the next glacial timeout */
			pi->cal_info->cal_suppress_count = pi->sh->glacial_timer;
			return;
		}

		if ((ACMAJORREV_4(pi->pubpi->phy_rev)) && !(ROUTER_4349(pi))) {
			if ((pi->ldo3p3_voltage == -1) && (pi->paldo3p3_voltage == -1)) {
				if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
					do_cals = phy_ac_vbat_monitoring_algorithm_20694(
						(pi->u.pi_acphy)->tempi);
				} else {
					do_cals = wlc_phy_vbat_monitoring_algorithm_acphy(pi);
				}
			}
		}
		/* Disable periodic noisecal trigger */
		pi->trigger_noisecal = FALSE;

		PHY_CAL(("%s: %sPHY phycal_tempdelta=%d\n", __FUNCTION__,
			(ISNPHY(pi)) ? "N": (ISACPHY(pi) ? "AC" : "some"),
			temp->phycal_tempdelta));

		if (temp->phycal_tempdelta && (ISNPHY(pi) || ISACPHY(pi))) {

			int cal_chanspec = 0;
			current_temp = phy_temp_sense_read((wlc_phy_t *)pi);
			if (ISNPHY(pi)) {
				cal_chanspec = pi->cal_info->u.ncal.txiqlocal_chanspec;
			} else if (ISACPHY(pi)) {
				if (pi->sh->now - pi->cal_info->last_temp_cal_time >=
					pi->sh->glacial_timer) {
					pi->cal_info->last_temp_cal_time = pi->sh->now;
				} else {
					current_temp =  pi->cal_info->last_cal_temp;
				}
				cal_chanspec = pi->cal_info->u.accal.chanspec;
			}

			delta_temp = ((current_temp > pi->cal_info->last_cal_temp) ?
				(current_temp - pi->cal_info->last_cal_temp) :
				(pi->cal_info->last_cal_temp - current_temp));

			/* Only do WATCHDOG triggered (periodic) calibration if
			 * the channel hasn't changed and if the temperature delta
			 * is above the specified threshold
			 */
			PHY_CAL(("%sPHY temp is %d, delta %d, cal_delta %d, chanspec %04x/%04x\n",
				(ISNPHY(pi)) ? "N": (ISACPHY(pi) ? "AC" : "some"),
				current_temp, delta_temp, temp->phycal_tempdelta,
				cal_chanspec, pi->radio_chanspec));

			delta_time = pi->sh->now - pi->cal_info->last_cal_time;

			/* cal_period = 0 implies only temperature based cals */
			if (((delta_temp < temp->phycal_tempdelta) &&
				(((delta_time < pi->cal_period) &&
				(pi->cal_period > 0)) || (pi->cal_period == 0)) &&
				(cal_chanspec == pi->radio_chanspec)) && (!do_cals)) {

				suppress_cal = TRUE;
				pi->cal_info->cal_suppress_count = pi->sh->glacial_timer;
				PHY_CAL(("Suppressing calibration.\n"));

			} else {
				pi->cal_info->last_cal_temp = current_temp;
			}
		}

		if (!suppress_cal) {
			/* if mphase is allowed, do it, otherwise, fall back to single phase */
			if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
				/* only schedule if it's not in progress */
				if ((!PHY_PERICAL_MPHASE_PENDING(pi)) && (!do_cals)) {
					pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
					wlc_phy_cal_perical_mphase_schedule(pi,
						PHY_PERICAL_WDOG_DELAY);
				} else if (do_cals) {
					if (PHY_PERICAL_MPHASE_PENDING(pi)) {
						phy_calmgr_mphase_reset(pi->calmgri);
					}
					wlc_phy_cals_acphy(pi->u.pi_acphy->calmgri,
						PHY_PERICAL_UNDEF, PHY_CAL_SEARCHMODE_RESTART);
				}
			} else if (pi->phy_cal_mode == PHY_PERICAL_SPHASE) {
				phy_calmgr_cals(pi, PHY_PERICAL_AUTO, PHY_CAL_SEARCHMODE_RESTART);
			} else {
				ASSERT(0);
			}
		}
		break;

	case PHY_PERICAL_DCS:

		/* Only applicable for NPHYs */
		ASSERT(ISNPHY(pi));

		if (ISNPHY(pi)) {
			if (PHY_PERICAL_MPHASE_PENDING(pi)) {
				phy_calmgr_mphase_reset(pi->calmgri);

				if (temp->phycal_tempdelta) {
					current_temp = wlc_phy_tempsense_nphy(pi);
					pi->cal_info->last_cal_temp = current_temp;
				}
			} else if (temp->phycal_tempdelta) {

				current_temp = wlc_phy_tempsense_nphy(pi);

				delta_temp = ((current_temp > pi->cal_info->last_cal_temp) ?
					(current_temp - pi->cal_info->last_cal_temp) :
					(pi->cal_info->last_cal_temp - current_temp));

				if ((delta_temp < (int16)temp->phycal_tempdelta)) {
					suppress_cal = TRUE;
				} else {
					pi->cal_info->last_cal_temp = current_temp;
				}
			}

			if (suppress_cal) {
				wlc_phy_txpwr_papd_cal_nphy_dcs(pi);
			} else {
				/* only mphase is allowed */
				if (pi->phy_cal_mode == PHY_PERICAL_MPHASE) {
					pi->cal_info->cal_searchmode = PHY_CAL_SEARCHMODE_REFINE;
					wlc_phy_cal_perical_mphase_schedule(pi,
						PHY_PERICAL_WDOG_DELAY);
				} else {
					ASSERT(0);
				}
			}
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}

/* ************************************************************* */
/* ************************************************************* */
/*		Functions to be modularized			 */
/* ************************************************************* */
/* ************************************************************* */

/* wlc_awdl non-AC */
void
wlc_phy_cal_mode(wlc_phy_t *ppi, uint mode)
{
	phy_info_t *pi = (phy_info_t *)ppi;

	if (pi->pi_fptr->calibmodes)
		pi->pi_fptr->calibmodes(pi, mode);
}

#if defined(BCMTSTAMPEDLOGS)
void
phy_log(phy_info_t *pi, const char* str, uint32 p1, uint32 p2)
{
	/* Read a timestamp from the TSF timer register */
	uint32 tstamp = R_REG(pi->sh->osh, D11_TSFTimerLow(pi));

	/* Store the timestamp and the log message in the log buffer */
	bcmtslog(tstamp, str, p1, p2);
}
#endif // endif

uint32
wlc_phy_cap_get(wlc_phy_t *pih)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint32	cap = 0;

	switch (pi->pubpi->phy_type) {
	case PHY_TYPE_N:
		cap |= PHY_CAP_40MHZ;
		cap |= (PHY_CAP_SGI | PHY_CAP_STBC);
		break;

#if (ACCONF || ACCONF2)
	case PHY_TYPE_AC:
		cap |= wlc_phy_ac_caps(pi);
		break;
#endif /* ACCONF || ACCONF2 */

	case PHY_TYPE_LCN20:
	/*                 A0     non-A0
	 * FW default:     OFF    ON
	 * nvram: ldpc=0   OFF    OFF
	 * nvram: ldpc=1   ON     ON
	 * nvram: ldpc=2   ON     OFF
	 * nvram: ldpc=3   OFF    ON
	 */

#if (defined(LCN20CONF) && (LCN20CONF != 0))
#ifdef WL11ULB
		cap |= (wlc_phy_lcn20_ulb_10_capable(pi) ? PHY_CAP_10MHZ : 0);
		cap |= (wlc_phy_lcn20_ulb_5_capable(pi) ? PHY_CAP_5MHZ : 0);
#endif /* WL11ULB */
#endif /* (defined(LCN20CONF) && (LCN20CONF != 0)) */

		if  ((pi->ldpc_en == 1) ||
		     ((CHIPREV(pi->sh->chiprev) == 0) && (pi->ldpc_en == 2)) ||
		     (!(CHIPREV(pi->sh->chiprev) == 0) && (pi->ldpc_en == 3)))
			cap |= (PHY_CAP_HT_PROP_RATES | PHY_CAP_SGI | PHY_CAP_STBC | PHY_CAP_LDPC);
		else
			cap |= (PHY_CAP_HT_PROP_RATES | PHY_CAP_SGI | PHY_CAP_STBC);
		break;

	default:
		break;
	}
	return cap;
}

const uint8 *
BCMRAMFN(wlc_phy_get_ofdm_rate_lookup)(void)
{
	return ofdm_rate_lookup;
}

/* ************************************************************* */
/* ************************************************************* */
/*		Functions shared by HT and N PHY		 */
/* ************************************************************* */
/* ************************************************************* */
int
wlc_phy_acimode_noisemode_reset(wlc_phy_t *pih, chanspec_t chanspec,
	bool clear_aci_state, bool clear_noise_state, bool disassoc)
{
	phy_info_t *pi = (phy_info_t *)pih;
	uint channel = CHSPEC_CHANNEL(chanspec);

	pi->interf->cca_stats_func_called = FALSE;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;

	if (pi->sh->interference_mode_override == TRUE)
		return BCME_NOTREADY;

	PHY_TRACE(("%s: CurCh %d HomeCh %d disassoc %d\n",
		__FUNCTION__, channel,
		pi->interf->curr_home_channel, disassoc));

#ifndef WLC_DISABLE_ACI
	if ((disassoc) ||
		((channel != pi->interf->curr_home_channel) &&
		(disassoc == FALSE))) {
		/* not home channel... reset */
		if (ISNPHY(pi)) {
			wlc_phy_aci_noise_reset_nphy(pi, channel,
				clear_aci_state, clear_noise_state, disassoc);
		}
	}
#else
	UNUSED_PARAMETER(clear_aci_state);
	UNUSED_PARAMETER(clear_noise_state);
	UNUSED_PARAMETER(disassoc);
	BCM_REFERENCE(channel);
#endif /* Compiling out ACI code */

	return BCME_OK;
}

int
wlc_phy_interference_set(wlc_phy_t *pih, bool init)
{
	int wanted_mode;
	phy_info_t *pi = (phy_info_t *)pih;

	if (!ISNPHY(pi))
		return BCME_UNSUPPORTED;

	if (pi->sh->interference_mode_override == TRUE) {
		/* keep the same values */
#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			if (pi->sh->interference_mode_5G_override == 0 ||
				pi->sh->interference_mode_5G_override == 1) {
				wanted_mode = pi->sh->interference_mode_5G_override;
			} else {
				wanted_mode = 0;
			}
		} else
#endif /* BAND5G */
		{
			wanted_mode = pi->sh->interference_mode_2G_override;
		}
	} else {

#ifdef BAND5G
		if (CHSPEC_IS5G(pi->radio_chanspec)) {
			wanted_mode = pi->sh->interference_mode_5G;
		} else
#endif /* BAND5G */
		{
			wanted_mode = pi->sh->interference_mode_2G;
		}
	}

	if (CHSPEC_CHANNEL(pi->radio_chanspec) != pi->interf->curr_home_channel) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifndef WLC_DISABLE_ACI
		phy_noise_set_mode(pi->noisei, wanted_mode, init);
#endif // endif
		pi->sh->interference_mode = wanted_mode;

		wlapi_enable_mac(pi->sh->physhim);
	}

	return BCME_OK;
}

/* ************************************************************* */
/* ************************************************************* */

void
wlc_phy_trigger_cals_for_btc_adjust(phy_info_t *pi)
{
	phy_calmgr_mphase_reset(pi->calmgri);
	if (ISNPHY(pi)) {
		pi->u.pi_nphy->cal_type_override = PHY_PERICAL_FULL;
	}
	wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
}

#if defined(WLTEST)

static int
wlc_phy_iovar_bbmult_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr)
{
	int err = BCME_OK;

	if (ISNPHY(pi))
		wlc_phy_get_bbmult_nphy(pi, ret_int_ptr);
	else
		err = BCME_UNSUPPORTED;

	return err;
}

static int
wlc_phy_iovar_bbmult_set(phy_info_t *pi, void *p)
{
	int err = BCME_OK;
	uint16 bbmult[PHY_CORE_NUM_2] = { 0 };
	uint8 m0, m1;

	bcopy(p, bbmult, PHY_CORE_NUM_2 * sizeof(uint16));

	if (ISNPHY(pi)) {
		m0 = (uint8)(bbmult[0] & 0xff);
		m1 = (uint8)(bbmult[1] & 0xff);
		wlc_phy_set_bbmult_nphy(pi, m0, m1);
	} else
		err = BCME_UNSUPPORTED;

	return err;
}

static int
wlc_phy_iovar_avvmid_get(phy_info_t *pi, void *p, bool bool_val,
	int32 *ret_int_ptr, wlc_avvmid_t avvmid_type)
{
	int err = BCME_OK;
	uint8 core_sub_band[2];
	bcopy(p, core_sub_band, 2*sizeof(uint8));
	BCM_REFERENCE(bool_val);
	if (ISACPHY(pi))
		wlc_phy_get_avvmid_acphy(pi, ret_int_ptr, avvmid_type, core_sub_band);
	else
		err = BCME_UNSUPPORTED;
	return err;
}

static int
wlc_phy_iovar_avvmid_set(phy_info_t *pi, void *p, wlc_avvmid_t avvmid_type)
{
	int err = BCME_OK;
	uint8 avvmid[3];
	bcopy(p, avvmid, 3*sizeof(uint8));
	if (ISACPHY(pi)) {
		wlc_phy_set_avvmid_acphy(pi, avvmid, avvmid_type);
	} else
		err = BCME_UNSUPPORTED;
	return err;
}
#endif // endif

#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
static int
wlc_phy_iovar_tempsense_paldosense(phy_info_t *pi, int32 *ret_int_ptr, uint8 tempsense_paldosense)
{
	int err = BCME_OK;

	*ret_int_ptr = 0;
	if (ISNPHY(pi)) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		*ret_int_ptr = (int32)wlc_phy_tempsense_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
	} else if (ISACPHY(pi)) {
		/* No need to call suspend_mac and phyreg_enter since it
		* is done inside wlc_phy_tempsense_acphy
		*/
		if (pi->radio_is_on)
			*ret_int_ptr = (int32)wlc_phy_tempsense_acphy(pi);
		else
			err = BCME_RADIOOFF;
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	} else if (ISLCN20PHY(pi)) {
		int32 int_val = wlc_lcn20phy_tempsense(pi, TEMPER_VBAT_TRIGGER_NEW_MEAS);

		bcopy(&int_val, ret_int_ptr, sizeof(int_val));
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
	} else
		err = BCME_UNSUPPORTED;

	return err;
}
#endif	/* defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD) || defined(BCMDBG_TEMPSENSE) */

#if defined(WLTEST)
static int
wlc_phy_iovar_idletssi_reg(phy_info_t *pi, int32 *ret_int_ptr, int32 int_val, bool set)
{
	int err = BCME_OK;
	uint32 tmp;
	uint16 idle_tssi[NPHY_CORE_NUM];
	phy_info_nphy_t *pi_nphy = pi->u.pi_nphy;

	if (ISNPHY(pi)) {
		if (NREV_GE(pi->pubpi->phy_rev, LCNXN_BASEREV + 4)) {
			wlc_phy_lcnxn_rx2tx_stallwindow_nphy(pi, 1);
			wlc_phy_txpwrctrl_idle_tssi_nphy(pi);
			wlc_phy_lcnxn_rx2tx_stallwindow_nphy(pi, 0);

			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_2g;
				idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_2g;
			} else {
				idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_5g;
				idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_5g;
			}
			tmp = (idle_tssi[1] << 16) | idle_tssi[0];
			*ret_int_ptr = tmp;
		} else if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
			wlc_phy_txpwrctrl_idle_tssi_nphy(pi);
			if (CHSPEC_IS2G(pi->radio_chanspec)) {
				idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_2g;
				idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_2g;
			} else {
				idle_tssi[0] = (uint16)pi_nphy->nphy_pwrctrl_info[0].idle_tssi_5g;
				idle_tssi[1] = (uint16)pi_nphy->nphy_pwrctrl_info[1].idle_tssi_5g;
			}
			tmp = (idle_tssi[1] << 8) | idle_tssi[0];
			*ret_int_ptr = tmp;
		}
	}
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	else if (ISLCN20PHY(pi))
		*ret_int_ptr = wlc_lcn20phy_idle_tssi_reg_iovar(pi, int_val, set, &err);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */

	return err;
}

static int
wlc_phy_iovar_avgtssi_reg(phy_info_t *pi, int32 *ret_int_ptr)
{
	int err = BCME_OK;
	if (ISNPHY(pi)) {
		*ret_int_ptr = wlc_nphy_tssi_read_iovar(pi);
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	} else if (ISLCN20PHY(pi)) {
		*ret_int_ptr = wlc_lcn20phy_avg_tssi_reg_iovar(pi);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
	}
	return err;
}
#endif // endif

static int
wlc_phy_iovar_forcecal(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;

	if (!pi->sh->up)
		return BCME_NOTUP;

	if (ISACPHY(pi)) {
		uint8 mphase = FALSE;
		uint8 searchmode = PHY_CAL_SEARCHMODE_RESTART;
		/* Vbat monitoring done before triggering cals */
		if ((ACMAJORREV_4(pi->pubpi->phy_rev)) && !(ROUTER_4349(pi))) {
			if ((pi->ldo3p3_voltage == -1) && (pi->paldo3p3_voltage == -1)) {
				if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
					phy_ac_vbat_monitoring_algorithm_20694(
						(pi->u.pi_acphy)->tempi);
				} else {
					wlc_phy_vbat_monitoring_algorithm_acphy(pi);
				}
			}
		}
		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = 0x00;

		/* only values in range [0-3] are valids */
		if (int_val > 3)
			return BCME_BADARG;

		/* 3 is mphase, anything else is single phase */
		if (int_val == 3) {
			mphase = TRUE;
		}
		else {
			/* Single phase, using 2 means sphase partial */
			if (int_val == 2)
				searchmode = PHY_CAL_SEARCHMODE_REFINE;
		}

		PHY_CAL(("wlc_phy_iovar_forcecal (mphase = %d, refine = %d)\n",
			mphase, searchmode));

		/* reset the noise array to clean the cache */
		phy_ac_rxgcrs_clean_noise_array(pi->u.pi_acphy->rxgcrsi);

		/* call sphase or schedule mphase cal */
		phy_calmgr_mphase_reset(pi->calmgri);
		if (mphase) {
			pi->cal_info->cal_searchmode = searchmode;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
		} else {
			if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
				/* Indicate PHY forcecal requested */
				pi->cal_info->phy_forcecal_request = TRUE;
			}

			/* If phy_forcecal_request is set then noisecal is skipped in acphy_cals */
			wlc_phy_cals_acphy(pi->u.pi_acphy->calmgri, PHY_PERICAL_UNDEF,
					searchmode);

			if (ACMAJORREV_40(pi->pubpi->phy_rev)) {
				phy_ac_ovr_rxgain_noise_sample_request(
					pi->u.pi_acphy->rxgcrsi, TRUE);
			}
		}
	} else if (ISNPHY(pi)) {
		/* for get with no argument, assume 0x00 */
		if (!set)
			int_val = PHY_PERICAL_AUTO;

		if ((int_val == PHY_PERICAL_PARTIAL) ||
		    (int_val == PHY_PERICAL_AUTO) ||
		    (int_val == PHY_PERICAL_FULL)) {
			phy_calmgr_mphase_reset(pi->calmgri);
			pi->u.pi_nphy->cal_type_override = (uint8)int_val;
			wlc_phy_cal_perical_mphase_schedule(pi, PHY_PERICAL_NODELAY);
#ifdef WLOTA_EN
		} else if (int_val == PHY_FULLCAL_SPHASE) {
			wlc_phy_cal_perical((wlc_phy_t *)pi, PHY_FULLCAL_SPHASE);
#endif /* WLOTA_EN */
		} else
			err = BCME_RANGE;

		/* phy_forcecal will trigger noisecal */
		pi->trigger_noisecal = TRUE;

	} else if (ISLCN20PHY(pi)) {
		pi->pi_fptr->calibmodes(pi, PHY_FULLCAL);
	}

	return err;
}

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG) || \
	defined(ATE_BUILD)
#ifndef ATE_BUILD
static int
wlc_phy_iovar_forcecal_obt(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, int vsize, bool set)
{
	int err = BCME_OK;
	uint8 wait_ctr = 0;
	int val = 1;
	if (ISNPHY(pi)) {
			phy_calmgr_mphase_reset(pi->calmgri);

			pi->cal_info->cal_phase_id = PHY_CAL_PHASE_INIT;
			pi->trigger_noisecal = TRUE;

			while (wait_ctr < 50) {
				val = ((pi->cal_info->cal_phase_id !=
					PHY_CAL_PHASE_IDLE)? 1 : 0);
				if (val == 0) {
					err = BCME_OK;
					break;
				}
				else {
					wlc_phy_cal_perical_nphy_run(pi, PHY_PERICAL_FULL);
					wait_ctr++;
				}
			}
			if (wait_ctr >= 50) {
				return BCME_ERROR;
			}

		}

	return err;
}
#endif /* !ATE_BUILD */
#endif // endif

#if defined(WLTEST) || defined(MACOSX)
static void
wlc_phy_iovar_set_deaf(phy_info_t *pi, int32 int_val)
{
	if (int_val) {
		wlc_phy_set_deaf((wlc_phy_t *) pi, TRUE);
	} else {
		wlc_phy_clear_deaf((wlc_phy_t *) pi, TRUE);
	}
}

static int
wlc_phy_iovar_get_deaf(phy_info_t *pi, int32 *ret_int_ptr)
{
	if (ISNPHY(pi)) {
		*ret_int_ptr = (int32)wlc_phy_get_deaf_nphy(pi);
		return BCME_OK;
	} else if (ISACPHY(pi)) {
	        *ret_int_ptr = (int32)wlc_phy_get_deaf_acphy(pi);
		return BCME_OK;
	} else {
		return BCME_UNSUPPORTED;
	}
}
#endif // endif
#if defined(WLTEST) || defined(ATE_BUILD)
static int
wlc_phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	int err = BCME_OK;

	if (!set) {
		if (ISACPHY(pi)) {
			*ret_int_ptr = pi->txpwrctrl;
		} else if (ISNPHY(pi)) {
			*ret_int_ptr = pi->nphy_txpwrctrl;
#if (defined(LCN20CONF) && (LCN20CONF != 0))
		} else if (ISLCN20PHY(pi)) {
				err = wlc_lcn20phy_iovar_isenabled_tpc(pi, ret_int_ptr);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
		}

	} else {
		if (ISNPHY(pi) || ISACPHY(pi)) {
			if ((int_val != PHY_TPC_HW_OFF) && (int_val != PHY_TPC_HW_ON)) {
				err = BCME_RANGE;
				goto end;
			}

			pi->nphy_txpwrctrl = (uint8)int_val;
			pi->txpwrctrl = (uint8)int_val;

			/* if not up, we are done */
			if (!pi->sh->up)
				goto end;

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);
			if (ISNPHY(pi))
				wlc_phy_txpwrctrl_enable_nphy(pi, (uint8) int_val);
			else if (ISACPHY(pi))
				wlc_phy_txpwrctrl_enable_acphy(pi, (uint8) int_val);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);

#if (defined(LCN20CONF) && (LCN20CONF != 0))
		} else if (ISLCN20PHY(pi)) {
			err = wlc_lcn20phy_iovar_txpwrctrl(pi, int_val, ret_int_ptr);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
		}
	}

end:
	return err;
}

static int
wlc_phy_iovar_txpwrindex_get(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr)
{
	int err = BCME_OK;

	if (ISNPHY(pi)) {
		*ret_int_ptr = wlc_phy_txpwr_idx_get_nphy(pi);
	} else if (ISACPHY(pi)) {
		*ret_int_ptr = wlc_phy_txpwr_idx_get_acphy(pi);
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	} else if (ISLCN20PHY(pi)) {
		*ret_int_ptr = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
	}
	return err;
}
#endif // endif

int
wlc_phy_iovar_txpwrindex_set(phy_info_t *pi, void *p)
{
	int err = BCME_OK;
	uint32 *txpwridxp;
	int8 idx[PHY_CORE_MAX], core;
	phy_info_nphy_t *pi_nphy = NULL;

	if (ISNPHY(pi))
		pi_nphy = pi->u.pi_nphy;

	/* IOV has idx as uint32's, convert to int8 */
	txpwridxp = (uint32 *)p;
	FOREACH_CORE(pi, core) {
		idx[core] =  (int8)(txpwridxp[core] & 0xff);
		if (idx[core] < 0) {
			err = BCME_RANGE;
			goto end;
		}
	}

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (ISNPHY(pi)) {
		FOREACH_CORE(pi, core) {
			pi_nphy->nphy_txpwrindex[core].index_internal = idx[core];
			wlc_phy_store_txindex_nphy(pi);
			wlc_phy_txpwr_index_nphy(pi, (1 << core), idx[core], TRUE);
		}
	} else if (ISACPHY(pi)) {
		FOREACH_CORE(pi, core) {
			wlc_phy_txpwrctrl_enable_acphy(pi, PHY_TPC_HW_OFF);
			wlc_phy_txpwr_by_index_acphy(pi, (1 << core), idx[core]);
		}
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	} else if (ISLCN20PHY(pi)) {
		wlc_lcn20phy_set_tx_pwr_by_index(pi, idx[0]);
#endif /* #if (defined(LCN20CONF) && (LCN20CONF != 0)) */
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

end:
	return err;
}

#if defined(WLTEST)
static int
wlc_phy_iovar_txrx_chain(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr, bool set)
{
	int err = BCME_OK;

	if (!set) {
		if (ISNPHY(pi)) {
			*ret_int_ptr = (int)pi->nphy_txrx_chain;
		}
	} else {
		if (ISNPHY(pi)) {
			if ((int_val != AUTO) && (int_val != WLC_N_TXRX_CHAIN0) &&
				(int_val != WLC_N_TXRX_CHAIN1)) {
				err = BCME_RANGE;
				goto end;
			}

			if (pi->nphy_txrx_chain != (int8)int_val) {
				pi->nphy_txrx_chain = (int8)int_val;
				if (pi->sh->up) {
					wlapi_suspend_mac_and_wait(pi->sh->physhim);
					phy_utils_phyreg_enter(pi);
					wlc_phy_stf_chain_upd_nphy(pi);
					wlc_phy_force_rfseq_nphy(pi, NPHY_RFSEQ_RESET2RX);
					phy_utils_phyreg_exit(pi);
					wlapi_enable_mac(pi->sh->physhim);
				}
			}
		}
	}
end:
	return err;
}

static void
wlc_phy_iovar_bphy_testpattern(phy_info_t *pi, uint8 testpattern, bool enable)
{
	bool existing_enable = FALSE;

	/* WL out check */
	if (pi->sh->up) {
		PHY_ERROR(("wl%d: %s: This function needs to be called after 'wl out'\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	/* confirm band is locked to 2G */
	if (!CHSPEC_IS2G(pi->radio_chanspec)) {
		PHY_ERROR(("wl%d: %s: Band needs to be locked to 2G (b)\n",
		          pi->sh->unit, __FUNCTION__));
		return;
	}

	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		existing_enable = (bool) pi->phy_bphy_evm;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		existing_enable = (bool) pi->phy_bphy_rfcs;
	} else {
		PHY_ERROR(("Testpattern needs to be between [0 (BPHY_EVM), 1 (BPHY_RFCS)]\n"));
		ASSERT(0);
	}

	if (ISNPHY(pi)) {
		wlc_phy_bphy_testpattern_nphy(pi, testpattern, enable, existing_enable);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	/* Return state of testpattern enables */
	if (testpattern == NPHY_TESTPATTERN_BPHY_EVM) {    /* CW CCK for EVM testing */
		pi->phy_bphy_evm = enable;
	} else if (testpattern == NPHY_TESTPATTERN_BPHY_RFCS) { /* RFCS testpattern */
		pi->phy_bphy_rfcs = enable;
	}
}

static void
wlc_phy_iovar_scraminit(phy_info_t *pi, int8 scraminit)
{
	pi->phy_scraminit = (int8)scraminit;
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (ISNPHY(pi)) {
		wlc_phy_test_scraminit_nphy(pi, scraminit);
	} else if (ISACPHY(pi)) {
		wlc_phy_test_scraminit_acphy(pi, scraminit);
	} else {
		PHY_ERROR(("support yet to be added\n"));
		ASSERT(0);
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

static int
wlc_phy_iovar_force_rfseq(phy_info_t *pi, uint8 int_val)
{
	int err = BCME_OK;

	phy_utils_phyreg_enter(pi);
	if (ISNPHY(pi)) {
		wlc_phy_force_rfseq_nphy(pi, int_val);
	} else if (ISACPHY(pi)) {
		wlc_phy_force_rfseq_acphy(pi, int_val);
	} else {
		err = BCME_UNSUPPORTED;
	}
	phy_utils_phyreg_exit(pi);

	return err;
}

static void
wlc_phy_iovar_tx_tone_hz(phy_info_t *pi, int32 int_val)
{
	pi->phy_tx_tone_freq = (int32) int_val;
}

static int
wlc_phy_iovar_tx_tone_symm(phy_info_t *pi, int32 int_val)
{
	int err = BCME_OK;
	pi->phy_tx_tone_freq = (int32) int_val;

	pi->phytxtone_symm = TRUE;

	if (ISACPHY(pi)) {
	        if (pi->phy_tx_tone_freq == 0) {
	                wlc_phy_stopplayback_acphy(pi, STOPPLAYBACK_W_CCA_RESET);
	                phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
	                wlapi_enable_mac(pi->sh->physhim);
	        } else {
	                pi->phy_tx_tone_freq = pi->phy_tx_tone_freq * 1000; /* Covert to Hz */
	                wlapi_suspend_mac_and_wait(pi->sh->physhim);
	                phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);
	                wlc_phy_tx_tone_acphy(pi, (int32)int_val, 151, TX_TONE_IQCAL_MODE_OFF,
	                                      TRUE);
	        }
	} else {
	        err = BCME_UNSUPPORTED;
	}

	/* Disable symmetrical tone, falling back to default setting */
	pi->phytxtone_symm = FALSE;

	return err;
}

static int16
wlc_phy_iovar_test_tssi(phy_info_t *pi, uint8 val, uint8 pwroffset)
{
	int16 tssi = 0;
	if (ISNPHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_nphy(pi, val, pwroffset);
	} else if (ISACPHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_acphy(pi, val, pwroffset);
	} else if (ISLCN20PHY(pi)) {
		tssi = (int16) wlc_phy_test_tssi_lcn20phy(pi, val, pwroffset);
	}
	return tssi;
}

static int16
wlc_phy_iovar_test_idletssi(phy_info_t *pi, uint8 val)
{
	int16 idletssi = INVALID_IDLETSSI_VAL;
	if (ISACPHY(pi)) {
		idletssi = (int16) wlc_phy_test_idletssi_acphy(pi, val);
	} else if (ISLCN20PHY(pi)) {
		idletssi = (int16) wlc_phy_test_idletssi_lcn20phy(pi, val);
	}
	return idletssi;
}

static int16
wlc_phy_iovar_setrptbl(phy_info_t *pi)
{
	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi->phy_rev))) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_populate_recipcoeffs_acphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	}

	return BCME_UNSUPPORTED;
}

static int16
wlc_phy_iovar_forceimpbf(phy_info_t *pi)
{
	uint16 bfeConfigVal;

	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi->phy_rev))) {
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		/* reset bfe before using the engine */
		if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			bfeConfigVal = phy_utils_read_phyreg(pi,
				ACPHY_BfeConfigReg0(pi->pubpi->phy_rev));
			phy_utils_write_phyreg(pi,
				ACPHY_BfeConfigReg0(pi->pubpi->phy_rev), bfeConfigVal |
				ACPHY_BfeConfigReg0_bfe_reset_MASK(pi->pubpi->phy_rev));
			phy_utils_write_phyreg(pi, ACPHY_BfeConfigReg0(pi->pubpi->phy_rev),
				bfeConfigVal);
		}

		bfeConfigVal = ACMAJORREV_47_51(pi->pubpi->phy_rev)?
			BFECONFIG0_IMPBF : BFECONFIGREF_FORCEVAL;
		phy_utils_write_phyreg(pi, ACPHY_BfeConfigReg0(pi->pubpi->phy_rev),
			bfeConfigVal);

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev)) {
			WRITE_PHYREG(pi, BfeMuConfigReg1, 0x1000);
			WRITE_PHYREG(pi, BfeMuConfigReg2, 0x2000);
			WRITE_PHYREG(pi, BfeMuConfigReg3, 0x1000);
		} else if (ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
			/* put the steering report in index 72 */
			uint16 tmpdata16;
			uint32 tmpdata32 = 0x2a002800;

			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFDINDEXLUT,
				1, 132, 32, &tmpdata32);
			tmpdata16 = 0x2800;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_IBFERPTADDRLUT,
				1, 0, 16, &tmpdata16);
		}

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	}

	return BCME_UNSUPPORTED;
}

static int16
wlc_phy_iovar_forcesteer(phy_info_t *pi, uint8 enable)
{
#if (ACCONF || ACCONF2) && defined(WL_BEAMFORMING)
	uint16 bfmcon_val      = 0;
	uint16 bfridx_pos_val  = 0;
	uint16 refresh_thr_val = 0;
	uint16 shm_base, bfrctl = 0;
	uint8 nsts_shift;

	if (ISACPHY(pi) && (!ACMAJORREV_1(pi->pubpi->phy_rev)) &&
			!ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
		phy_stf_data_t *stf_shdata = phy_stf_get_data(pi->stfi);

		BCM_REFERENCE(stf_shdata);

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		bfmcon_val      = enable ? BFMCON_FORCEVAL      : BFMCON_RELEASEVAL;
		bfridx_pos_val  = enable ? BFRIDX_POS_FORCEVAL  : BFRIDX_POS_RELEASEVAL;
		refresh_thr_val = enable ? REFRESH_THR_FORCEVAL : REFRESH_THR_RELEASEVAL;

		shm_base = M_BFI_BLK(pi) >> 1;

		if (!ACMAJORREV_47(pi->pubpi->phy_rev)) {
			/* NDP streams */
			nsts_shift = C_BFI_BFRCTL_POS_NSTS_SHIFT;
			if (PHY_BITSCNT(stf_shdata->phytxchain) == 4) {
				/* 4 streams */
				bfrctl = (2 << nsts_shift);
			} else if (PHY_BITSCNT(stf_shdata->phytxchain) == 3) {
				/* 3 streams */
				bfrctl = (1 << nsts_shift);
			} else if (PHY_BITSCNT(stf_shdata->phytxchain) == 2) {
				/* 2 streams */
				bfrctl = 0;
			}
			bfrctl |= (stf_shdata->phytxchain << C_BFI_BFRCTL_POS_BFM_SHIFT);
			wlapi_bmac_write_shm(pi->sh->physhim,
				shm_addr(shm_base, C_BFI_BFRCTL_POS), bfrctl);
		}

		phy_utils_write_phyreg(pi, ACPHY_BfmCon(pi->pubpi->phy_rev), bfmcon_val);

		if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
		    ACMAJORREV_33(pi->pubpi->phy_rev) ||
		    ACMAJORREV_37(pi->pubpi->phy_rev)) {
			uint32 tmpaddr = 0x1000;
			wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_BFMUSERINDEX,
			1, 0x1000, 32, &tmpaddr);
			MOD_PHYREG(pi, BfeMuConfigReg0, useTxbfIndexAddr, 1);
		}

		wlapi_bmac_write_shm(pi->sh->physhim, M_BFI_REFRESH_THR(pi), refresh_thr_val);
		wlapi_bmac_write_shm(pi->sh->physhim, M_BFI_BLK(pi), bfridx_pos_val);

		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		return 0;
	} else if ((ACMAJORREV_47(pi->pubpi->phy_rev) && ACMINORREV_GE(pi, 1)) ||
		ACMAJORREV_51(pi->pubpi->phy_rev)) {

		wlapi_forcesteer_ge129(pi->sh->physhim);
		return 0;
	}
#endif /* (ACCONF || ACCONF2) && WL_BEAMFORMING */

	return BCME_UNSUPPORTED;
}

static void
wlc_phy_iovar_rxcore_enable(phy_info_t *pi, int32 int_val, bool bool_val, int32 *ret_int_ptr,
	bool set)
{
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);

	if (set) {
		if (ISNPHY(pi)) {
			wlc_phy_rxcore_setstate_nphy((wlc_phy_t *)pi, (uint8) int_val, 0);
		} else if (ISACPHY(pi)) {
			wlc_phy_rxcore_setstate_acphy((wlc_phy_t *)pi,
			    (uint8) int_val, phy_stf_get_data(pi->stfi)->phytxchain);
		}
	} else {
		if (ISNPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_nphy((wlc_phy_t *)pi);
		} else if (ISACPHY(pi)) {
			*ret_int_ptr =  (uint32)wlc_phy_rxcore_getstate_acphy((wlc_phy_t *)pi);
		}
	}

	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#endif // endif

static int
wlc_phy_iovar_set_dssf(phy_info_t *pi, int32 set_val)
{
	if (ISACPHY(pi) && PHY_ILNA(pi)) {
	  phy_utils_write_phyreg(pi, ACPHY_DSSF_C_CTRL(pi->pubpi->phy_rev), (uint16) set_val);

		return BCME_OK;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_get_dssf(phy_info_t *pi, int32 *ret_val)
{
	if (ISACPHY(pi) && PHY_ILNA(pi)) {
		*ret_val = (int32) phy_utils_read_phyreg(pi, ACPHY_DSSF_C_CTRL(pi->pubpi->phy_rev));

		return BCME_OK;
	}

	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_set_bphymrc(phy_info_t *pi, int32 set_val)
{
	if (ISACPHY(pi) && (ACPHY_bphymrcCtrl(pi->pubpi->phy_rev) != INVALID_ADDRESS)) {
		pi->sh->bphymrc_en = (uint8) set_val;

		return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}

static int
wlc_phy_iovar_get_bphymrc(phy_info_t *pi, int32 *ret_val)
{
	if (ISACPHY(pi) && (ACPHY_bphymrcCtrl(pi->pubpi->phy_rev) != INVALID_ADDRESS)) {
		*ret_val = pi->sh->bphymrc_en;
		return BCME_OK;
	}
	return BCME_UNSUPPORTED;
}

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(BCMDBG)
static void
wlc_phy_iovar_set_papd_offset(phy_info_t *pi, int16 int_val)
{
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	if (ISLCN20PHY(pi)) {
		wlc_phy_set_papd_offset_lcn20phy(pi, int_val);
	}
#endif /* #if ((defined(LCN20CONF) && (LCN20CONF != 0))) */
}

static int
wlc_phy_iovar_get_papd_offset(phy_info_t *pi)
{
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	if (ISLCN20PHY(pi)) {
		return wlc_phy_get_papd_offset_lcn20phy(pi);
	} else
#endif /* #if ((defined(LCN20CONF) && (LCN20CONF != 0))) */
		return BCME_UNSUPPORTED;
}
#endif // endif

static int
wlc_phy_iovar_dispatch_old(phy_info_t *pi, uint32 actionid, void *p, void *a, int vsize,
	int32 int_val, bool bool_val)
{
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	phy_info_nphy_t *pi_nphy;

	pi_nphy = pi->u.pi_nphy;
	BCM_REFERENCE(pi_nphy);
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
#if NCONF
#if defined(BCMDBG)
	case IOV_SVAL(IOV_NPHY_INITGAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_setinitgain_nphy(pi, (uint16) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_HPVGA1GAIN):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_sethpf1gaintbl_nphy(pi, (int8) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_SVAL(IOV_NPHY_TX_TEMP_TONE): {
		uint16 orig_BBConfig;
		uint16 m0m1;
		nphy_txgains_t target_gain;

		if ((uint32)int_val > 0) {
			pi->phy_tx_tone_freq = (uint32) int_val;
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);
			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, TRUE);

			/* Save the bbmult values,since it gets overwritten by mimophy_tx_tone() */
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
			phy_utils_mod_phyreg(pi, NPHY_BBConfig,
			                     NPHY_BBConfig_resample_clk160_MASK, 0);

			/* read current tx gain and use as target_gain */
			wlc_phy_get_tx_gain_nphy(pi, &target_gain);

			PHY_ERROR(("Tx gain core 0: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[0], target_gain.pga[0],
			         target_gain.txgm[0], target_gain.txlpf[0]));

			PHY_ERROR(("Tx gain core 1: target gain: ipa = %d,"
			         " pad = %d, pga = %d, txgm = %d, txlpf = %d\n",
			         target_gain.ipa[0], target_gain.pad[1], target_gain.pga[1],
			         target_gain.txgm[1], target_gain.txlpf[1]));

			/* play a tone for 10 secs and then stop it and return */
			wlc_phy_tx_tone_nphy(pi, (uint32)int_val, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

			phy_rxgcrs_stay_in_carriersearch(pi->rxgcrsi, FALSE);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;
	}
	case IOV_SVAL(IOV_NPHY_CAL_RESET):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		wlc_phy_cal_reset_nphy(pi, (uint32) int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_EST_TONEPWR):
	case IOV_GVAL(IOV_PHY_EST_TONEPWR): {
		int32 dBm_power[2];
		uint16 orig_BBConfig;
		uint16 m0m1;

		if (ISNPHY(pi)) {
			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);

			/* Save the bbmult values, since it gets overwritten
			   by mimophy_tx_tone()
			*/
			wlc_phy_table_read_nphy(pi, 15, 1, 87, 16, &m0m1);

			/* Disable the re-sampler (in case we are in spur avoidance mode) */
			orig_BBConfig = phy_utils_read_phyreg(pi, NPHY_BBConfig);
			phy_utils_mod_phyreg(pi, NPHY_BBConfig,
			                     NPHY_BBConfig_resample_clk160_MASK, 0);
			pi->phy_tx_tone_freq = (uint32) 4000;

			/* play a tone for 10 secs */
			wlc_phy_tx_tone_nphy(pi, (uint32)4000, 250, 0, 0, FALSE);

			/* Now restore the original bbmult values */
			wlc_phy_table_write_nphy(pi, 15, 1, 87, 16, &m0m1);
			wlc_phy_table_write_nphy(pi, 15, 1, 95, 16, &m0m1);

			OSL_DELAY(10000000);
			wlc_phy_est_tonepwr_nphy(pi, dBm_power, 128);
			wlc_phy_stopplayback_nphy(pi);

			/* Restore the state of the re-sampler
			   (in case we are in spur avoidance mode)
			*/
			phy_utils_write_phyreg(pi, NPHY_BBConfig, orig_BBConfig);

			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);

			int_val = dBm_power[0]/4;
			bcopy(&int_val, a, vsize);
			break;
		} else {
			err = BCME_UNSUPPORTED;
			break;
		}
	}

	case IOV_GVAL(IOV_NPHY_RFSEQ_TXGAIN): {
		uint16 rfseq_tx_gain[2];
		wlc_phy_table_read_nphy(pi, NPHY_TBL_ID_RFSEQ, 2, 0x110, 16, rfseq_tx_gain);
		int_val = (((uint32) rfseq_tx_gain[1] << 16) | ((uint32) rfseq_tx_gain[0]));
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_PHY_SPURAVOID):
		if ((int_val != SPURAVOID_DISABLE) && (int_val != SPURAVOID_AUTO) &&
		    (int_val != SPURAVOID_FORCEON) && (int_val != SPURAVOID_FORCEON2)) {
			err = BCME_RANGE;
			break;
		}

		pi->phy_spuravoid = (int8)int_val;
		break;

	case IOV_GVAL(IOV_PHY_SPURAVOID):
		int_val = pi->phy_spuravoid;
		bcopy(&int_val, a, vsize);
		break;
#endif /* defined(BCMDBG) */

#if defined(WLTEST)
	case IOV_GVAL(IOV_NPHY_CAL_SANITY):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		*ret_int_ptr = (uint32)wlc_phy_cal_sanity_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_EVM):
		*ret_int_ptr = pi->phy_bphy_evm;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_NPHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_NPHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_NPHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, pi->phy_scraminit);
		break;

	case IOV_SVAL(IOV_NPHY_RFSEQ):
		err = wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_NPHY_TXIQLOCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;
		if (ISNPHY(pi)) {

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			phy_utils_phyreg_enter(pi);

			/* read current tx gain and use as target_gain */
			wlc_phy_get_tx_gain_nphy(pi, &target_gain);
			tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
			wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

			err = wlc_phy_cal_txiqlo_nphy(pi, target_gain, TRUE, FALSE);
			if (err)
				break;
			wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
			phy_utils_phyreg_exit(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		*ret_int_ptr = 0;
		break;
	}
	case IOV_SVAL(IOV_NPHY_RXIQCAL): {
		nphy_txgains_t target_gain;
		uint8 tx_pwr_ctrl_state;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		/* read current tx gain and use as target_gain */
		wlc_phy_get_tx_gain_nphy(pi, &target_gain);
		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);
#ifdef RXIQCAL_FW_WAR
		if (wlc_phy_cal_rxiq_nphy_fw_war(pi, target_gain, 0, (bool)int_val, 0x3) != BCME_OK)
#else
		if (wlc_phy_cal_rxiq_nphy(pi, target_gain, 0, (bool)int_val, 0x3) != BCME_OK)
#endif // endif
		{
			break;
		}
		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;
	}
	case IOV_GVAL(IOV_NPHY_RXCALPARAMS):
		if (ISNPHY(pi)) {
			*ret_int_ptr = pi_nphy->nphy_rxcalparams;
		}
		break;

	case IOV_SVAL(IOV_NPHY_RXCALPARAMS):
		if (ISNPHY(pi)) {
			pi_nphy->nphy_rxcalparams = (uint32)int_val;
		}
		break;

	case IOV_GVAL(IOV_NPHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_RSSISEL):
		*ret_int_ptr = pi->nphy_rssisel;
		break;

	case IOV_SVAL(IOV_NPHY_RSSISEL):
		pi->nphy_rssisel = (uint8)int_val;

		if (!pi->sh->up)
			break;

		if (pi->nphy_rssisel < 0) {
			phy_utils_phyreg_enter(pi);
			wlc_phy_rssisel_nphy(pi, RADIO_MIMO_CORESEL_OFF, 0);
			phy_utils_phyreg_exit(pi);
		} else {
			int32 rssi_buf[4];
			phy_utils_phyreg_enter(pi);
			wlc_phy_poll_rssi_nphy(pi, (uint8)int_val, rssi_buf, 1);
			phy_utils_phyreg_exit(pi);
		}
		break;

	case IOV_GVAL(IOV_NPHY_RSSICAL): {
		/* if down, return the value, if up, run the cal */
		if (!pi->sh->up) {
			int_val = pi->nphy_rssical;
			bcopy(&int_val, a, vsize);
			break;
		}

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		/* run rssi cal */
		wlc_phy_rssi_cal_nphy(pi);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		int_val = pi->nphy_rssical;
		bcopy(&int_val, a, vsize);
		break;
	}

	case IOV_SVAL(IOV_NPHY_RSSICAL): {
		pi->nphy_rssical = bool_val;
		break;
	}

	case IOV_GVAL(IOV_NPHY_GPIOSEL):
	case IOV_GVAL(IOV_PHY_GPIOSEL):
		*ret_int_ptr = pi->phy_gpiosel;
		break;

	case IOV_SVAL(IOV_NPHY_GPIOSEL):
	case IOV_SVAL(IOV_PHY_GPIOSEL):
		pi->phy_gpiosel = (uint16) int_val;

		if (!pi->sh->up)
			break;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);
		if (ISNPHY(pi))
			wlc_phy_gpiosel_nphy(pi, (uint16)int_val);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);
		break;

	case IOV_GVAL(IOV_NPHY_TX_TONE):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_NPHY_TX_TONE):
		wlc_phy_iovar_tx_tone(pi, (uint32)int_val);
		break;

	case IOV_SVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		pi->nphy_elna_gain_config = (int_val != 0) ? TRUE : FALSE;
		break;

	case IOV_GVAL(IOV_NPHY_ELNA_GAIN_CONFIG):
		*ret_int_ptr = (int32)pi->nphy_elna_gain_config;
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_NPHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

#ifdef BAND5G
	case IOV_SVAL(IOV_NPHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_NPHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;
#endif /* BAND5G */

	case IOV_GVAL(IOV_NPHY_PERICAL):
		*ret_int_ptr = phy_calmgr_get_calmode(pi);
		break;

	case IOV_SVAL(IOV_NPHY_PERICAL):
		err = phy_calmgr_set_calmode(pi, int_val);
		break;

	case IOV_SVAL(IOV_NPHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;

#ifndef WLC_DISABLE_ACI
	case IOV_GVAL(IOV_NPHY_ACI_SCAN):
		if (SCAN_INPROG_PHY(pi)) {
			PHY_ERROR(("Scan in Progress, can execute %s\n", __FUNCTION__));
			*ret_int_ptr = -1;
		} else {
			if (pi->cur_interference_mode == INTERFERE_NONE) {
				PHY_ERROR(("interference mode is off\n"));
				*ret_int_ptr = -1;
				break;
			}

			wlapi_suspend_mac_and_wait(pi->sh->physhim);
			*ret_int_ptr = wlc_phy_aci_scan_nphy(pi);
			wlapi_enable_mac(pi->sh->physhim);
		}
		break;
#endif /* Compiling out ACI code */
	case IOV_SVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_NPHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_NPHY_PAPDCALTYPE):
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_cal_type = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCAL):
		if (ISNPHY(pi))
			pi_nphy->nphy_force_papd_cal = TRUE;
		int_val = 0;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_NPHY_SKIPPAPD):
		if ((int_val != 0) && (int_val != 1)) {
			err = BCME_RANGE;
			break;
		}
		if (ISNPHY(pi))
			pi_nphy->nphy_papd_skip = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PAPDCALINDEX):
		if (ISNPHY(pi)) {
			*ret_int_ptr = (pi_nphy->nphy_papd_cal_gain_index[0] << 8) |
				pi_nphy->nphy_papd_cal_gain_index[1];
		}
		break;

	case IOV_SVAL(IOV_NPHY_CALTXGAIN): {
		uint8 tx_pwr_ctrl_state;

		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		phy_utils_phyreg_enter(pi);

		if (ISNPHY(pi)) {
			pi_nphy->nphy_cal_orig_pwr_idx[0] =
			        (uint8) ((phy_utils_read_phyreg(pi,
			                  NPHY_Core0TxPwrCtrlStatus) >> 8) & 0x7f);
			pi_nphy->nphy_cal_orig_pwr_idx[1] =
				(uint8) ((phy_utils_read_phyreg(pi,
			                  NPHY_Core1TxPwrCtrlStatus) >> 8) & 0x7f);
		}

		tx_pwr_ctrl_state = pi->nphy_txpwrctrl;
		wlc_phy_txpwrctrl_enable_nphy(pi, PHY_TPC_HW_OFF);

		wlc_phy_cal_txgainctrl_nphy(pi, int_val, TRUE);

		wlc_phy_txpwrctrl_enable_nphy(pi, tx_pwr_ctrl_state);
		phy_utils_phyreg_exit(pi);
		wlapi_enable_mac(pi->sh->physhim);

		break;
	}

	case IOV_GVAL(IOV_NPHY_VCOCAL):
		wlapi_suspend_mac_and_wait(pi->sh->physhim);
		wlc_phy_radio205x_vcocal_nphy(pi);
		wlapi_enable_mac(pi->sh->physhim);
		*ret_int_ptr = 0;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MINIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_minidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MINIDX):
		pi->nphy_tbldump_minidx = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		*ret_int_ptr = (int32)pi->nphy_tbldump_maxidx;
		break;

	case IOV_SVAL(IOV_NPHY_TBLDUMP_MAXIDX):
		pi->nphy_tbldump_maxidx = (int8) int_val;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		if (pi->nphy_phyreg_skipcnt < 127) {
			pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt++] = (uint) int_val;
		}
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPDUMP):
		*ret_int_ptr = (pi->nphy_phyreg_skipcnt > 0) ?
			(int32) pi->nphy_phyreg_skipaddr[pi->nphy_phyreg_skipcnt-1] : 0;
		break;

	case IOV_SVAL(IOV_NPHY_PHYREG_SKIPCNT):
		pi->nphy_phyreg_skipcnt = (int8) int_val;
		break;

	case IOV_GVAL(IOV_NPHY_PHYREG_SKIPCNT):
		*ret_int_ptr = (int32)pi->nphy_phyreg_skipcnt;
		break;
#endif // endif
#endif /* NCONF */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

/* register iovar table to the system */
#include <phy_api.h>

int
phy_iovars_wrapper_sample_collect(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen,
	void *a, int alen, int vsize)
{
#ifdef SAMPLE_COLLECT
	return phy_iovars_sample_collect(pi, actionid, -1, p, plen, a, alen, vsize);
#endif /* SAMPLE_COLLECT */

	return BCME_UNSUPPORTED;
}
#include <wlc_patch.h>

static int
phy_doiovar(void *ctx, uint32 actionid, void *p, uint plen, void *a, uint alen, uint vsize,
	struct wlc_if *wlcif)
{
	phy_info_t *pi = (phy_info_t *)ctx;
	bool bool_val = FALSE;
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* bool conversion to avoid duplication below */
	bool_val = int_val != 0;

	BCM_REFERENCE(*ret_int_ptr);
	BCM_REFERENCE(bool_val);

	switch (actionid) {
#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_FAST_TIMER):
		*ret_int_ptr = (int32)pi->sh->fast_timer;
		break;

	case IOV_SVAL(IOV_FAST_TIMER):
		pi->sh->fast_timer = (uint32)int_val;
		break;

	case IOV_GVAL(IOV_SLOW_TIMER):
		*ret_int_ptr = (int32)pi->sh->slow_timer;
		break;

	case IOV_SVAL(IOV_SLOW_TIMER):
		pi->sh->slow_timer = (uint32)int_val;
		break;

#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG) || defined(WLTEST) || defined(PHYCAL_CHNG_CS)
	case IOV_GVAL(IOV_GLACIAL_TIMER):
		*ret_int_ptr = (int32)pi->sh->glacial_timer;
		break;

	case IOV_SVAL(IOV_GLACIAL_TIMER):
		pi->sh->glacial_timer = (uint32)int_val;
		break;
#endif /* BCMDBG || WLTEST || PHYCAL_CHNG_CS */
#if defined(WLTEST) || defined(MACOSX) || defined(DBG_PHY_IOV) || defined(ATE_BUILD)
	case IOV_GVAL(IOV_PHY_WATCHDOG):
		*ret_int_ptr = (int32)pi->phywatchdog_override;
		break;

	case IOV_SVAL(IOV_PHY_WATCHDOG):
		phy_wd_override(pi, bool_val);
		break;
#endif // endif
	case IOV_GVAL(IOV_CAL_PERIOD):
	        *ret_int_ptr = (int32)pi->cal_period;
	        break;

	case IOV_SVAL(IOV_CAL_PERIOD):
	        pi->cal_period = (uint32)int_val;
	        break;
#if defined(WLTEST)
	case IOV_GVAL(IOV_PHYHAL_MSG):
		*ret_int_ptr = (int32)phyhal_msg_level;
		break;

	case IOV_SVAL(IOV_PHYHAL_MSG):
		phyhal_msg_level = (uint32)int_val;
		break;

	case IOV_SVAL(IOV_PHY_FIXED_NOISE):
		pi->phy_fixed_noise = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_FIXED_NOISE):
		int_val = (int32)pi->phy_fixed_noise;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_GVAL(IOV_PHYNOISE_POLL):
		*ret_int_ptr = (int32)pi->phynoise_polling;
		break;

	case IOV_SVAL(IOV_PHYNOISE_POLL):
		pi->phynoise_polling = bool_val;
		break;

	case IOV_GVAL(IOV_CARRIER_SUPPRESS):
		err = BCME_UNSUPPORTED;
		*ret_int_ptr = (pi->carrier_suppr_disable == 0);
		break;

	case IOV_SVAL(IOV_CARRIER_SUPPRESS):
	{
		initfn_t carr_suppr_fn = pi->pi_fptr->carrsuppr;
		if (carr_suppr_fn) {
			pi->carrier_suppr_disable = bool_val;
			if (pi->carrier_suppr_disable) {
				(*carr_suppr_fn)(pi);
			}
			PHY_INFORM(("Carrier Suppress Called\n"));
		} else
			err = BCME_UNSUPPORTED;
		break;
	}

#ifdef BAND5G
	case IOV_GVAL(IOV_PHY_SUBBAND5GVER):
		/* Retrieve 5G subband version */
		int_val = (uint8)(pi->sromi->subband5Gver);
		bcopy(&int_val, a, vsize);
		break;
#endif /* BAND5G */
	case IOV_GVAL(IOV_PHY_TXRX_CHAIN):
		wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXRX_CHAIN):
		err = wlc_phy_iovar_txrx_chain(pi, int_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_BPHY_EVM):
		*ret_int_ptr = pi->phy_bphy_evm;
		break;

	case IOV_SVAL(IOV_PHY_BPHY_EVM):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_EVM, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_BPHY_RFCS):
		*ret_int_ptr = pi->phy_bphy_rfcs;
		break;

	case IOV_SVAL(IOV_PHY_BPHY_RFCS):
		wlc_phy_iovar_bphy_testpattern(pi, NPHY_TESTPATTERN_BPHY_RFCS, (bool) int_val);
		break;

	case IOV_GVAL(IOV_PHY_SCRAMINIT):
		*ret_int_ptr = pi->phy_scraminit;
		break;

	case IOV_SVAL(IOV_PHY_SCRAMINIT):
		wlc_phy_iovar_scraminit(pi, (uint8)int_val);
		break;

	case IOV_SVAL(IOV_PHY_RFSEQ):
		err = wlc_phy_iovar_force_rfseq(pi, (uint8)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TX_TONE_HZ):
	case IOV_GVAL(IOV_PHY_TX_TONE_SYMM):
		*ret_int_ptr = pi->phy_tx_tone_freq;
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE_HZ):
		wlc_phy_iovar_tx_tone_hz(pi, (int32)int_val);
		break;

	case IOV_SVAL(IOV_PHY_TX_TONE_SYMM):
		err = wlc_phy_iovar_tx_tone_symm(pi, (int32)int_val);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 0);
		break;

	case IOV_GVAL(IOV_PHY_TEST_TSSI_OFFS):
		*((uint*)a) = wlc_phy_iovar_test_tssi(pi, (uint8)int_val, 12);
		break;

	case IOV_GVAL(IOV_PHY_TEST_IDLETSSI):
		*((uint*)a) = wlc_phy_iovar_test_idletssi(pi, (uint8)int_val);
		break;

	case IOV_SVAL(IOV_PHY_SETRPTBL):
		wlc_phy_iovar_setrptbl(pi);
		break;

	case IOV_SVAL(IOV_PHY_FORCEIMPBF):
		wlc_phy_iovar_forceimpbf(pi);
		break;

	case IOV_SVAL(IOV_PHY_FORCESTEER):
		wlc_phy_iovar_forcesteer(pi, (uint8)int_val);
		break;
#ifdef BAND5G
	case IOV_SVAL(IOV_PHY_5G_PWRGAIN):
		pi->phy_5g_pwrgain = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_5G_PWRGAIN):
		*ret_int_ptr = (int32)pi->phy_5g_pwrgain;
		break;
#endif /* BAND5G */

	case IOV_SVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_ENABLERXCORE):
		wlc_phy_iovar_rxcore_enable(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_GVAL(IOV_PHY_ACTIVECAL):
		*ret_int_ptr = (int32)((pi->cal_info->cal_phase_id !=
			PHY_CAL_PHASE_IDLE)? 1 : 0);
		break;

	case IOV_SVAL(IOV_PHY_BBMULT):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_bbmult_set(pi, p);
		break;

	case IOV_GVAL(IOV_PHY_BBMULT):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_bbmult_get(pi, int_val, bool_val, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_TPC_AV):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_avvmid_set(pi, p, AV);
		break;

	case IOV_GVAL(IOV_TPC_AV):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_avvmid_get(pi, p, bool_val, ret_int_ptr, AV);
		break;

	case IOV_SVAL(IOV_TPC_VMID):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_avvmid_set(pi, p, VMID);
		break;

	case IOV_GVAL(IOV_TPC_VMID):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_avvmid_get(pi, p, bool_val, ret_int_ptr, VMID);
		break;

#if defined(WLC_LOWPOWER_BEACON_MODE)
	case IOV_GVAL(IOV_PHY_LOWPOWER_BEACON_MODE):
		break;

	case IOV_SVAL(IOV_PHY_LOWPOWER_BEACON_MODE):
		wlc_phy_lowpower_beacon_mode(pih, int_val);
		break;
#endif /* WLC_LOWPOWER_BEACON_MODE */
#endif // endif
	case IOV_GVAL(IOV_PHY_RXGAININDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_get_rx_gainindex(pi, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_PHY_RXGAININDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_set_rx_gainindex(pi, int_val);
		break;

	case IOV_GVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL):
		err = wlc_phy_iovar_forcecal(pi, int_val, ret_int_ptr, vsize, TRUE);
		break;
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(WFD_PHY_LL_DEBUG)
#ifndef ATE_BUILD
	case IOV_GVAL(IOV_PHY_FORCECAL_OBT):
		err = wlc_phy_iovar_forcecal_obt(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_FORCECAL_OBT):
		err = wlc_phy_iovar_forcecal_obt(pi, int_val, ret_int_ptr, vsize, FALSE);
		break;
#endif /* !ATE_BUILD */
#endif // endif
#if defined(WLTEST) || defined(MACOSX)
	case IOV_SVAL(IOV_PHY_DEAF):
		wlc_phy_iovar_set_deaf(pi, int_val);
		break;
	case IOV_GVAL(IOV_PHY_DEAF):
		err = wlc_phy_iovar_get_deaf(pi, ret_int_ptr);
		break;
#endif // endif
	case IOV_GVAL(IOV_NUM_STREAM):
		if (ISNPHY(pi)) {
			int_val = 2;
		} else {
			int_val = -1;
		}
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_MIN_TXPOWER):
		if (ACMAJORREV_4(pi->pubpi->phy_rev)) {
			pi->min_txpower_5g = (uint8)int_val;
		}
		pi->min_txpower = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_MIN_TXPOWER):
		if (ACMAJORREV_4(pi->pubpi->phy_rev) && CHSPEC_IS5G(pi->radio_chanspec)) {
			int_val = pi->min_txpower_5g;
		} else {
			int_val = pi->min_txpower;
		}
		bcopy(&int_val, a, sizeof(int_val));
		break;

#if defined(MACOSX)
	case IOV_GVAL(IOV_PHYWREG_LIMIT):
		int_val = pi->phy_wreg_limit;
		bcopy(&int_val, a, vsize);
		break;

	case IOV_SVAL(IOV_PHYWREG_LIMIT):
		pi->phy_wreg_limit = (uint8)int_val;
		break;
#endif // endif
	case IOV_GVAL(IOV_PHY_MUTED):
		*ret_int_ptr = PHY_MUTED(pi) ? 1 : 0;
		break;

	case IOV_GVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi))
			*ret_int_ptr = pi->nphy_enable_hw_antsel ? 1 : 0;
		break;

	case IOV_SVAL(IOV_PHY_RXANTSEL):
		if (ISNPHY(pi)) {
			pi->nphy_enable_hw_antsel = bool_val;
			/* make sure driver is up (so clks are on) before writing to PHY regs */
			if (pi->sh->up) {
				wlc_phy_init_hw_antsel(pi);
			}
		}
		break;
	case IOV_GVAL(IOV_PHY_DSSF):
	        err = wlc_phy_iovar_get_dssf(pi, ret_int_ptr);
		break;
	case IOV_SVAL(IOV_PHY_DSSF):
		err = wlc_phy_iovar_set_dssf(pi, int_val);
		break;

	case IOV_GVAL(IOV_PHY_BPHYMRC):
		err = wlc_phy_iovar_get_bphymrc(pi, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_PHY_BPHYMRC):
		err = wlc_phy_iovar_set_bphymrc(pi, int_val);
		break;
#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(BCMDBG)
		case IOV_SVAL(IOV_PAPD_EPS_OFFSET): {
			wlc_phy_iovar_set_papd_offset(pi, (int16)int_val);
			break;
		}

		case IOV_GVAL(IOV_PAPD_EPS_OFFSET): {
			*ret_int_ptr = wlc_phy_iovar_get_papd_offset(pi);
			break;
		}
#endif // endif

		case IOV_GVAL(IOV_PHY_DUMP_PAGE): {
			*ret_int_ptr = pi->pdpi->page;
			break;
		}
		case IOV_SVAL(IOV_PHY_DUMP_PAGE): {
			if ((int_val < 0) || (int_val > 0xFF)) {
			        err = BCME_RANGE;
			        PHY_ERROR(("Value out of range\n"));
			        break;
			}
			*ret_int_ptr = pi->pdpi->page = (uint8)int_val;
			break;
		}
		case IOV_GVAL(IOV_PHY_DUMP_ENTRY): {
			*ret_int_ptr = pi->pdpi->entry;
			break;
		}
		case IOV_SVAL(IOV_PHY_DUMP_ENTRY): {
			if ((int_val < 0) || (int_val > 0xFFFF)) {
			        err = BCME_RANGE;
			        PHY_ERROR(("Value out of range\n"));
			        break;
			}
			*ret_int_ptr = pi->pdpi->entry = (uint16)int_val;
			break;
		}
#if defined(WLTEST)
	case IOV_SVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		if (int_val == 0)
			err = BCME_RANGE;
		else
			pi->aci_exit_check_period = int_val;
		break;

	case IOV_GVAL(IOV_ACI_EXIT_CHECK_PERIOD):
		int_val = pi->aci_exit_check_period;
		bcopy(&int_val, a, vsize);
		break;

#endif // endif
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_GLITCHK):
		pi->tunings[0] = (uint16)int_val;
		break;

	case IOV_SVAL(IOV_PHY_NOISE_UP):
		pi->tunings[1] = (uint16)int_val;
		break;

	case IOV_SVAL(IOV_PHY_NOISE_DWN):
		pi->tunings[2] = (uint16)int_val;
		break;
#endif /* #if defined(WLTEST) */
#if (NCONF || LCN20CONF || ACCONF || ACCONF2)
	case IOV_GVAL(IOV_PHY_RSSI_GAIN_CAL_TEMP):
		*ret_int_ptr = pi->srom_gain_cal_temp;
		break;

	case IOV_SVAL(IOV_PHY_RSSI_GAIN_CAL_TEMP):
		pi->srom_gain_cal_temp  = (int16)int_val;
		break;
#endif /* (NCONF || LCN20CONF || ACCONF || ACCONF2) && WLTEST */

#if defined(BCMDBG) || defined(WLTEST) || defined(ATE_BUILD) || \
	defined(BCMDBG_TEMPSENSE)
	case IOV_GVAL(IOV_PHY_TEMPSENSE):
		err = wlc_phy_iovar_tempsense_paldosense(pi, ret_int_ptr, 0);
		break;
#endif /* BCMDBG || WLTEST || ATE_BUILD || BCMDBG_TEMPSENSE */
#if defined(WLTEST)
	case IOV_GVAL(IOV_PHY_CAL_DISABLE):
		*ret_int_ptr = (int32)pi->disable_percal;
		break;

	case IOV_SVAL(IOV_PHY_CAL_DISABLE):
		pi->disable_percal = bool_val;
		break;

	case IOV_GVAL(IOV_PHY_IDLETSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			err = wlc_phy_iovar_idletssi_reg(pi, ret_int_ptr, int_val, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_IDLETSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			err = wlc_phy_iovar_idletssi_reg(pi, ret_int_ptr, int_val, TRUE);
		break;

	case IOV_GVAL(IOV_PHY_AVGTSSI_REG):
		if (!pi->sh->clk)
			err = BCME_NOCLK;
		else
			wlc_phy_iovar_avgtssi_reg(pi, ret_int_ptr);
		break;

	case IOV_SVAL(IOV_PHY_RESETCCA):
		if (ISNPHY(pi)) {
			wlc_phy_resetcca_nphy(pi);
		}
		else if (ISACPHY(pi)) {
			bool macSuspended;
			/* check if MAC already suspended */
			macSuspended = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
			if (!macSuspended) {
				wlapi_suspend_mac_and_wait(pi->sh->physhim);
			}
			wlc_phy_resetcca_acphy(pi);
			if (!macSuspended)
				wlapi_enable_mac(pi->sh->physhim);
		}
		break;

	case IOV_SVAL(IOV_PHY_IQLOCALIDX):
		break;
#endif // endif
	case IOV_SVAL(IOV_PHY_SROM_TEMPSENSE):
	{
		pi->srom_rawtempsense = (int16)int_val;
		break;
	}

	case IOV_GVAL(IOV_PHY_SROM_TEMPSENSE):
	{
		*ret_int_ptr = pi->srom_rawtempsense;
		break;
	}
	case IOV_SVAL(IOV_PHY_GAIN_CAL_TEMP):
	{
		pi->srom_gain_cal_temp  = (int16)int_val;
		break;
	}
	case IOV_GVAL(IOV_PHY_GAIN_CAL_TEMP):
	{
		*ret_int_ptr = pi->srom_gain_cal_temp;
		break;
	}
#ifdef PHYMON
	case IOV_GVAL(IOV_PHYCAL_STATE): {
		if (alen < (int)sizeof(wl_phycal_state_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		if (ISNPHY(pi))
			err = wlc_phycal_state_nphy(pi, a, alen);
		else
			err = BCME_UNSUPPORTED;

		break;
	}
#endif /* PHYMON */
	case IOV_GVAL(IOV_PHY_PAPD_DEBUG):
		break;

	case IOV_GVAL(IOV_NOISE_MEASURE):
		int_val = 0;
		bcopy(&int_val, a, sizeof(int_val));
		break;

#if defined(WLTEST) || defined(ATE_BUILD)
	case IOV_GVAL(IOV_PHY_TXPWRCTRL):
		wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, FALSE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRCTRL):
		err = wlc_phy_iovar_txpwrctrl(pi, int_val, bool_val, ret_int_ptr, TRUE);
		break;

	case IOV_SVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		err = wlc_phy_iovar_txpwrindex_set(pi, p);
		break;

	case IOV_GVAL(IOV_PHY_TXPWRINDEX):
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}
		wlc_phy_iovar_txpwrindex_get(pi, int_val, bool_val, ret_int_ptr);
		break;
#endif // endif
#if defined(WLTEST)
	case IOV_GVAL(IOV_PATRIM):
		if (ISACPHY(pi))
			wlc_phy_iovar_patrim_acphy(pi, ret_int_ptr);
		else
			*ret_int_ptr = 0;
	break;

	case IOV_GVAL(IOV_POVARS): {
		wl_po_t tmppo;

		/* tmppo has the input phy_type and band */
		bcopy(p, &tmppo, sizeof(wl_po_t));
		if (ISNPHY(pi)) {
			if (tmppo.phy_type != PHY_TYPE_N)  {
				tmppo.phy_type = PHY_TYPE_NULL;
				break;
			}

			switch (tmppo.band) {
			case WL_CHAN_FREQ_RANGE_2G:
				tmppo.cckpo = pi->ppr->u.sr8.cck2gpo;
				tmppo.ofdmpo = pi->ppr->u.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr->u.sr8.mcs[tmppo.band][0], &tmppo.mcspo,
					8*sizeof(uint16));
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
				tmppo.ofdmpo = pi->ppr->u.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr->u.sr8.mcs[tmppo.band], &tmppo.mcspo,
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND1:
				tmppo.ofdmpo = pi->ppr->u.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr->u.sr8.mcs[tmppo.band], &tmppo.mcspo,
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND2:
				tmppo.ofdmpo = pi->ppr->u.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr->u.sr8.mcs[tmppo.band], &tmppo.mcspo,
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				tmppo.ofdmpo = pi->ppr->u.sr8.ofdm[tmppo.band];
				bcopy(&pi->ppr->u.sr8.mcs[tmppo.band], &tmppo.mcspo,
					8*sizeof(uint16));
				break;
#endif /* BAND5G */
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", tmppo.band));
				err = BCME_BADARG;
				break;
			}

			if (!err)
				bcopy(&tmppo, a, sizeof(wl_po_t));
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;

	case IOV_SVAL(IOV_POVARS): {
		wl_po_t inpo;

		bcopy(p, &inpo, sizeof(wl_po_t));

		if (ISNPHY(pi)) {
			if (inpo.phy_type != PHY_TYPE_N)
				break;

			switch (inpo.band) {
			case WL_CHAN_FREQ_RANGE_2G:
				pi->ppr->u.sr8.cck2gpo = inpo.cckpo;
				pi->ppr->u.sr8.ofdm[inpo.band]  = inpo.ofdmpo;
				bcopy(inpo.mcspo, &(pi->ppr->u.sr8.mcs[inpo.band][0]),
					8*sizeof(uint16));
				break;
#ifdef BAND5G
			case WL_CHAN_FREQ_RANGE_5G_BAND0:
				pi->ppr->u.sr8.ofdm[inpo.band] = inpo.ofdmpo;
				bcopy(inpo.mcspo, &(pi->ppr->u.sr8.mcs[inpo.band][0]),
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND1:
				pi->ppr->u.sr8.ofdm[inpo.band] = inpo.ofdmpo;
				bcopy(inpo.mcspo, &(pi->ppr->u.sr8.mcs[inpo.band][0]),
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND2:
				pi->ppr->u.sr8.ofdm[inpo.band] = inpo.ofdmpo;
				bcopy(inpo.mcspo, &(pi->ppr->u.sr8.mcs[inpo.band][0]),
					8*sizeof(uint16));
				break;

			case WL_CHAN_FREQ_RANGE_5G_BAND3:
				pi->ppr->u.sr8.ofdm[inpo.band] = inpo.ofdmpo;
				bcopy(inpo.mcspo, &(pi->ppr->u.sr8.mcs[inpo.band][0]),
					8*sizeof(uint16));
				break;
#endif /* BAND5G */
			default:
				PHY_ERROR(("bandrange %d is out of scope\n", inpo.band));
				err = BCME_BADARG;
				break;
			}
		} else {
			PHY_ERROR(("Unsupported PHY type!\n"));
			err = BCME_UNSUPPORTED;
		}
	}
	break;
#endif // endif

	case IOV_GVAL(IOV_SROM_REV): {
			*ret_int_ptr = pi->sh->sromrev;
	}
	break;

#ifdef WLTEST
	case IOV_GVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;
			uint8*	maxp = (uint8*)a;

			maxp[0] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G];
			maxp[1] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_2G];
			maxp[2] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND0];
			maxp[3] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND0];
			maxp[4] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND1];
			maxp[5] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND1];
			maxp[6] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND2];
			maxp[7] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND2];
			if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			{
				maxp[8] = pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND3];
				maxp[9] = pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND3];
			}
		}
		break;
	}
	case IOV_SVAL(IOV_PHY_MAXP): {
		if (ISNPHY(pi)) {
			uint8*	maxp = (uint8*)p;
			srom_pwrdet_t	*pwrdet  = pi->pwrdet;

			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G] = maxp[0];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_2G] = maxp[1];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND0] = maxp[2];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND0] = maxp[3];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND1] = maxp[4];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND1] = maxp[5];
			pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND2] = maxp[6];
			pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND2] = maxp[7];
			if (pi->sromi->subband5Gver == PHY_SUBBAND_4BAND)
			{
				pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_5G_BAND3] = maxp[8];
				pwrdet->max_pwr[PHY_CORE_1][WL_CHAN_FREQ_RANGE_5G_BAND3] = maxp[9];
			}
		}
		break;
	}
#endif /* WLTEST */
#if defined(WLTEST)
	case IOV_SVAL(IOV_RPCALVARS): {
		const wl_rpcal_t *rpcal = p;
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			PHY_ERROR(("Number of TX Chain has to be > 1!\n"));
			err = BCME_UNSUPPORTED;
		} else {
			pi->u.pi_acphy->sromi->rpcal2g = rpcal[WL_CHAN_FREQ_RANGE_2G].update ?
			rpcal[WL_CHAN_FREQ_RANGE_2G].value : pi->u.pi_acphy->sromi->rpcal2g;
			pi->u.pi_acphy->sromi->rpcal5gb0 =
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].value : pi->u.pi_acphy->sromi->rpcal5gb0;
			pi->u.pi_acphy->sromi->rpcal5gb1 =
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].value : pi->u.pi_acphy->sromi->rpcal5gb1;
			pi->u.pi_acphy->sromi->rpcal5gb2 =
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].value : pi->u.pi_acphy->sromi->rpcal5gb2;
			pi->u.pi_acphy->sromi->rpcal5gb3 =
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].update ?
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].value : pi->u.pi_acphy->sromi->rpcal5gb3;

			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				pi->sromi->rpcal2gcore3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].value :
				pi->sromi->rpcal2gcore3;
				pi->sromi->rpcal5gb0core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].value :
				pi->sromi->rpcal5gb0core3;
				pi->sromi->rpcal5gb1core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].value :
				pi->sromi->rpcal5gb1core3;
				pi->sromi->rpcal5gb2core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].value :
				pi->sromi->rpcal5gb2core3;
				pi->sromi->rpcal5gb3core3 =
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].update ?
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].value :
				pi->sromi->rpcal5gb3core3;
			}
		}
		break;
	}

	case IOV_GVAL(IOV_RPCALVARS): {
		wl_rpcal_t *rpcal = a;
		if (ACMAJORREV_1(pi->pubpi->phy_rev)) {
			PHY_ERROR(("Number of TX Chain has to be > 1!\n"));
			err = BCME_UNSUPPORTED;
		} else {
			rpcal[WL_CHAN_FREQ_RANGE_2G].value = pi->u.pi_acphy->sromi->rpcal2g;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND0].value = pi->u.pi_acphy->sromi->rpcal5gb0;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND1].value = pi->u.pi_acphy->sromi->rpcal5gb1;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND2].value = pi->u.pi_acphy->sromi->rpcal5gb2;
			rpcal[WL_CHAN_FREQ_RANGE_5G_BAND3].value = pi->u.pi_acphy->sromi->rpcal5gb3;
			if (ACMAJORREV_32(pi->pubpi->phy_rev) ||
				ACMAJORREV_33(pi->pubpi->phy_rev) ||
				ACMAJORREV_37(pi->pubpi->phy_rev) ||
				ACMAJORREV_47_51(pi->pubpi->phy_rev)) {
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_2G].value =
				pi->sromi->rpcal2gcore3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND0].value =
				pi->sromi->rpcal5gb0core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND1].value =
				pi->sromi->rpcal5gb1core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND2].value =
				pi->sromi->rpcal5gb2core3;
				rpcal[WL_NUM_RPCALVARS+WL_CHAN_FREQ_RANGE_5G_BAND3].value =
				pi->sromi->rpcal5gb3core3;
			}
		}
		break;
	}
#endif // endif
#if defined(BCMDBG)
	case IOV_SVAL(IOV_PHY_FORCE_FDIQI):
	{

		wlc_phy_force_fdiqi_acphy(pi, (uint16) int_val);

		break;
	}
#endif /* BCMDBG */

	case IOV_SVAL(IOV_EDCRS):
	{

		if (int_val == 0)
		{
			W_REG(pi->sh->osh, D11_IFS_CTL_SEL_PRICRS(pi), 0x000F);
		}
		else
		{
			W_REG(pi->sh->osh, D11_IFS_CTL_SEL_PRICRS(pi), 0x0F0F);
		}
		break;
	}
	case IOV_GVAL(IOV_EDCRS):
	{
		if (R_REG(pi->sh->osh, D11_IFS_CTL_SEL_PRICRS(pi)) == 0x000F)
		{
			*ret_int_ptr = 0;
		}
		else if (R_REG(pi->sh->osh, D11_IFS_CTL_SEL_PRICRS(pi)) == 0x0F0F)
		{
			*ret_int_ptr = 1;
		}
		break;
	}
	case IOV_GVAL(IOV_PHY_AFE_OVERRIDE):
		if (!ACMAJORREV_3(pi->pubpi->phy_rev)) {
			*ret_int_ptr = (int32)pi->afe_override;
		} else {
			PHY_ERROR(("PHY_AFE_OVERRIDE is not supported for this chip \n"));
		}
		break;
	case IOV_SVAL(IOV_PHY_AFE_OVERRIDE):
		if (!ACMAJORREV_3(pi->pubpi->phy_rev)) {
			pi->afe_override = (uint8)int_val;
		} else {
			PHY_ERROR(("PHY_AFE_OVERRIDE is not supported for this chip \n"));
		}
		break;
#if defined(WLTEST)
	case IOV_SVAL(IOV_PHY_AUXPGA):
		break;

	case IOV_GVAL(IOV_PHY_AUXPGA):
		break;
#endif // endif
#if defined(WLTEST)
#if (defined(LCN20CONF) && (LCN20CONF != 0))
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_DIS):
		if (ISLCN20PHY(pi)) {
			*ret_int_ptr = (int32)(int32)pi->u.pi_lcn20phy->txpwr_clamp_dis;
		}
		break;
	case IOV_SVAL(IOV_LCNPHY_TXPWRCLAMP_DIS):
		if (ISLCN20PHY(pi)) {
			pi->u.pi_lcn20phy->txpwr_clamp_dis = (uint8)int_val;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_OFDM):
		if (ISLCN20PHY(pi)) {
			*ret_int_ptr = (int32)pi->u.pi_lcn20phy->target_pwr_ofdm_max;
		}
		break;
	case IOV_GVAL(IOV_LCNPHY_TXPWRCLAMP_CCK):
		if (ISLCN20PHY(pi)) {
			*ret_int_ptr = (int32)pi->u.pi_lcn20phy->target_pwr_cck_max;
		}
		break;
#endif /* #if LCN20CONF */
#endif /* #if defined(WLTEST) */
#if defined(BCMDBG) || defined(WLTEST)
	case IOV_GVAL(IOV_PHY_MASTER_OVERRIDE): {
		*ret_int_ptr = phy_get_master(pi);
		break;
	}

	case IOV_SVAL(IOV_PHY_MASTER_OVERRIDE): {
		if ((int_val < -1) || (int_val > 1)) {
		  err = BCME_RANGE;
		  PHY_ERROR(("Value out of range\n"));
		  break;
		}
		*ret_int_ptr = phy_set_master(pi, (int8)int_val);
		break;
	}
#endif /* defined(BCMDBG) ||  defined(WLTEST) */

	default:
		err = BCME_UNSUPPORTED;
	}

	if (err == BCME_UNSUPPORTED)
		err = wlc_phy_iovar_dispatch_old(pi, actionid, p, a, vsize, int_val, bool_val);

	if (err == BCME_UNSUPPORTED)
		err = phy_iovars_wrapper_sample_collect(pi, actionid, -1, p, plen, a, alen, vsize);

	return err;
}

int phy_legacy_register_iovt(phy_info_t *pi, wlc_iocv_info_t *ii);

int
BCMATTACHFN(phy_legacy_register_iovt)(phy_info_t *pi, wlc_iocv_info_t *ii)
{
	wlc_iovt_desc_t iovd;
#if defined(WLC_PATCH_IOCTL)
	wlc_iov_disp_fn_t disp_fn = IOV_PATCH_FN;
	const bcm_iovar_t* patch_table = IOV_PATCH_TBL;
#else
	wlc_iov_disp_fn_t disp_fn = NULL;
	const bcm_iovar_t* patch_table = NULL;
#endif /* WLC_PATCH_IOCTL */

	ASSERT(ii != NULL);

	wlc_iocv_init_iovd(phy_iovars,
	                   phy_legacy_pack_iov, phy_legacy_unpack_iov,
	                   phy_doiovar, disp_fn, patch_table, pi,
	                   &iovd);
	return wlc_iocv_register_iovt(ii, &iovd);
}
