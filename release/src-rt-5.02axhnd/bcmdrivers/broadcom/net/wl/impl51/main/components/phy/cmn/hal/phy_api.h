/*
 * PHY Core module public interface (to MAC driver).
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
 * $Id: phy_api.h 691048 2017-03-20 16:47:17Z $
 */

#ifndef _phy_api_h_
#define _phy_api_h_

#include <typedefs.h>
#include <bcmwifi_channels.h>
#include <wlc_iocv_types.h>
#include <wlc_phy_hal.h>

#define	NORADIO_ID		0xe4f5
#define	NORADIO_IDCODE		0x4e4f5246

#define IDCODE_ACPHY_ID_MASK   0xffff
#define IDCODE_ACPHY_ID_SHIFT  0
#define IDCODE_ACPHY_REV_MASK  0xffff0000
#define IDCODE_ACPHY_REV_SHIFT 16
#define IDCODE_ACPHY_MAJORREV_MASK  0xfff00000
#define IDCODE_ACPHY_MAJORREV_SHIFT 20
#define IDCODE_ACPHY_MINORREV_MASK  0x000f0000
#define IDCODE_ACPHY_MINORREV_SHIFT 16

/* Phy capabilities. Not a PHYHW/SW interface definition but an arbitrary software definition. */
#define PHY_CAP_40MHZ			0x00000001	/* 40MHz channels supported */
#define PHY_CAP_STBC			0x00000002
#define PHY_CAP_SGI			0x00000004
#define PHY_CAP_80MHZ			0x00000008	/* 80MHz channels supported */
#define PHY_CAP_LDPC			0x00000010
#define PHY_CAP_VHT_PROP_RATES		0x00000020	/* Supports Proprietary VHT rates */
#define PHY_CAP_8080MHZ			0x00000040	/* 80+80 MHz channels supported */
#define PHY_CAP_160MHZ			0x00000080	/* 160 MHz channels supported */
#define PHY_CAP_HT_PROP_RATES		0x00000100	/* Supports Proprietary 11n rates */
#define PHY_CAP_SU_BFR			0x00000200	/* Su Beamformer cap */
#define PHY_CAP_SU_BFE			0x00000400	/* Su Beamformee cap */
#define PHY_CAP_MU_BFR			0x00000800	/* Mu Beamformer cap */
#define PHY_CAP_MU_BFE			0x00001000	/* Mu Beamformee cap */
#define PHY_CAP_1024QAM			0x00002000	/* 1024QAM (c10, c11) rates supported */
#define PHY_CAP_2P5MHZ			0x00004000	/* 2.5 MHz channels supported */
#define PHY_CAP_5MHZ			0x00008000	/* 5   MHz channels supported */
#define PHY_CAP_10MHZ			0x00010000	/* 10  MHz channels supported */
#define PHY_CAP_SUP_5G			0x00020000	/* 5G supported */

/* PHY Capabilities added for ACREV_GE 44 */
#define PHY_CAP_FDCS			0x00020000 /* Supports FDCS */
#define PHY_CAP_11N_TURBO_QAM		0x00040000 /* Supports 11n Turbo QAM */
#define PHY_CAP_HE			0x00080000 /* Supports High Efficieny (802.11ax) */
#define PHY_CAP_11AX_DCM		0x00100000 /* Supports HE (11ax) Dual carrier mode */

/* PhyInternalCapability1 */

#define PHY_CAP_256QAM_BCC_RX_NSS1_NGI	0x00000001
#define PHY_CAP_256QAM_BCC_RX_NSS2_NGI	0x00000002
#define PHY_CAP_256QAM_BCC_RX_NSS1_SGI	0x00000004
#define PHY_CAP_256QAM_BCC_RX_NSS2_SGI	0x00000008
#define PHY_CAP_256QAM_LDPC_RX_NSS1_NGI	0x00000010
#define PHY_CAP_256QAM_LDPC_RX_NSS2_NGI	0x00000020
#define PHY_CAP_256QAM_LDPC_RX_NSS1_SGI	0x00000040
#define PHY_CAP_256QAM_LDPC_RX_NSS2_SGI	0x00000080
#define PHY_CAP_1024QAM_LDPC_RX		0x00000100
#define PHY_CAP_STBC_RX			0x00000200
#define PHY_CAP_SUP_2G			0x00000400

/* PhyInternalCapability2 */
#define PHY_CAP_256QAM_BCC_TX_NSS1_NGI	0x00010000
#define PHY_CAP_256QAM_BCC_TX_NSS2_NGI	0x00020000
#define PHY_CAP_256QAM_BCC_TX_NSS1_SGI	0x00040000
#define PHY_CAP_256QAM_BCC_TX_NSS2_SGI	0x00080000
#define PHY_CAP_256QAM_LDPC_TX_NSS1_NGI	0x00100000
#define PHY_CAP_256QAM_LDPC_TX_NSS2_NGI	0x00200000
#define PHY_CAP_256QAM_LDPC_TX_NSS1_SGI	0x00400000
#define PHY_CAP_256QAM_LDPC_TX_NSS2_SGI	0x00800000
#define PHY_CAP_1024QAM_LDPC_TX		0x01000000
#define PHY_CAP_STBC_TX			0x02000000

/* Phy pre attach caps used by wlc */
#define PHY_PREATTACH_CAP_SUP_5G	0x00000001
#define PHY_PREATTACH_CAP_SUP_2G	0x00000002

/* AFE Override */
#define PHY_AFE_OVERRIDE_USR	1
#define PHY_AFE_OVERRIDE_DRV	2

#ifdef BCMPHYCORENUM
#define PHY_CORE_MAX	(BCMPHYCORENUM)

#ifdef WLRSDB
#if PHY_CORE_MAX > 1
#define PHYCORENUM(cn)	(((cn) > PHY_CORE_MAX) ? (PHY_CORE_MAX) : (cn))
#else
#define PHYCORENUM(cn) PHY_CORE_MAX
#endif /* PHY_CORE_MAX */
#else
#define PHYCORENUM(cn) PHY_CORE_MAX
#endif /* WLRSDB */

#else
#define PHY_CORE_MAX	4
#define PHYCORENUM(cn)	(cn)
#endif /* BCMPHYCORENUM */

#ifdef BCMPHYCOREMASK
#define PHYCOREMASK(cm)	(BCMPHYCOREMASK)
#else
#define PHYCOREMASK(cm)	(cm)
#endif // endif

#define PHY_INVALID_RSSI (-127)
#define DUAL_MAC_SLICES 2

#define PHY_COREMASK_SISO(cm) ((cm == 1 || cm == 2 || cm == 4 || cm == 8) ? 1 : 0)

#define DUAL_PHY_NUM_CORE_MAX 4

/* add the feature to take out the BW RESET duriny the BW change  0: disable 1: enable */
#define BW_RESET 1
/* add the feature to disable DSSF  0: disable 1: enable */
#define DSSF_ENABLE 1
#define DSSFB_ENABLE 1

/*
 * Attach/detach all PHY modules to/from the system.
 */
phy_info_t *phy_module_attach(shared_phy_t *sh, void *regs, int bandtype, char *vars);
void phy_module_detach(phy_info_t *pi);

prephy_info_t *prephy_module_attach(shared_phy_t *sh, void *regs);
void prephy_module_detach(prephy_info_t *pi);

extern shared_phy_t *wlc_prephy_shared_attach(shared_phy_params_t *shp);
extern void  wlc_prephy_shared_detach(shared_phy_t *phy_sh);

void wlc_phy_set_shmdefs(wlc_phy_t *ppi, const shmdefs_t *shmdefs);

/*
 * Register all iovar/ioctl tables/handlers to/from the system.
 */
int phy_register_iovt_all(phy_info_t *pi, wlc_iocv_info_t *ii);
int phy_register_ioct_all(phy_info_t *pi, wlc_iocv_info_t *ii);

/*
 * TODO: These functions should be registered to bmac in phy_module_attach(),
 * which requires bmac to have some registration infrastructure...
 */

/*
 * Init/deinit the PHY h/w.
 */
/* generic init */
int phy_init(phy_info_t *pi, chanspec_t chanspec);
/* generic deinit */
int phy_down(phy_info_t *pi);

int phy_bss_init(wlc_phy_t *ppi, bool bonlyap, int noise);

/* Publish phyAPI's here.. */
#define PHY_RSBD_PI_IDX_CORE0 0
#define PHY_RSBD_PI_IDX_CORE1 1

void phy_set_phymode(phy_info_t *pi, uint16 new_phymode);
uint16 phy_get_phymode(const phy_info_t *pi);
phy_info_t *phy_get_pi(const phy_info_t *pi, int idx);
bool phy_init_pending(phy_info_t *pi);
mbool phy_get_measure_hold_status(phy_info_t *pi);
void phy_set_measure_hold_status(phy_info_t *pi, mbool set);
void phy_machwcap_set(wlc_phy_t *ppi, uint32 machwcap);
int8 phy_preamble_override_get(wlc_phy_t *ppi);
int  phy_hw_state_upd(wlc_phy_t *ppi, bool newstate);
int  phy_hw_clk_state_upd(wlc_phy_t *ppi, bool newstate);
int phy_numofcores(wlc_phy_t *pih);

/* Feature bitmap index. */
enum {
	PHY_FEATURE_TXPWRCTRL_IDX = 0,
	PHY_FEATURE_OCL_IDX = 1,
	PHY_FEATURE_NAP_IDX = 2,
	PHY_FEATURE_UCM_IDX = 3,
	PHY_FEATURE_MAX_IDX
};

bool phy_feature_enabled(phy_info_t *pi, uint idx);
void phy_set_feature_flag(phy_info_t *pi, uint idx, bool enable);

#endif /* _phy_api_h_ */
