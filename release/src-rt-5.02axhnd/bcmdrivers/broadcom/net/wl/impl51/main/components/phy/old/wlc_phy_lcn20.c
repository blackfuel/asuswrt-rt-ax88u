/*
 * PHY and RADIO specific portion of Broadcom BCM43XX 802.11abgn
 * Networking Device Driver.
 *
 * -----------------------------------------------------------------------------
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
 * -----------------------------------------------------------------------------
 *
 * $Id: wlc_phy_lcn20.c 679815 2017-01-17 13:01:39Z $
 */

#include <wlc_cfg.h>

#if (defined(LCN20CONF) && (LCN20CONF != 0))
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcm_math.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <802.11.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <wlc_phy_lcn20.h>
#include <wlc_phyreg_lcn20.h>
#include <sbchipc.h>
#include <wlc_phyreg_lcn20.h>
#include <wlc_phytbl_lcn20.h>
#include <wlc_radioreg_20692.h>
#include <bcmotp.h>
#include <wlc_phy_shim.h>
#include <phy_rxgcrs_api.h>
#include <phy_tpc_api.h>
#include <phy_chanmgr.h>
#include <phy_chanmgr_api.h>
#include <phy_utils_channel.h>
#include <phy_utils_math.h>
#include <phy_utils_var.h>
#include <phy_utils_reg.h>
#include <phy_btcx.h>
#include <phy_lcn20_tpc.h>
#include <phy_calmgr_api.h>

#ifdef ATE_BUILD
#include <wl_ate.h>
#endif // endif

void wlc_phy_detach_lcn20phy(phy_info_t *pi);
void wlc_phy_txpower_recalc_target_lcn20phy(phy_info_t *pi);
static void wlc_lcn20phy_txpower_recalc_target(phy_info_t *pi);
static void wlc_lcn20phy_txpower_recalc_target_2pwr(phy_info_t *pi);
static void wlc_phy_txpwr_sromlcn20_read_ppr_parameters(phy_info_t *pi);
void wlc_lcn20phy_set_tx_pwr_by_index(phy_info_t *pi, int indx);

#if defined(WLTEST)
static void wlc_phy_carrier_suppress_lcn20phy(phy_info_t *pi);
#endif // endif

#if defined(BCMDBG) || defined(WLTEST)
static int wlc_phy_long_train_lcn20phy(phy_info_t *pi, int channel);
#endif /* defined(BCMDBG) || defined(WLTEST) */

void wlc_lcn20phy_anacore(phy_info_t *pi, bool on);
void wlc_lcn20phy_switch_radio(phy_info_t *pi, bool on);
static bool wlc_phy_watchdog_lcn20phy(phy_wd_ctx_t *ctx);
void wlc_lcn20phy_write_table(phy_info_t *pi, const phytbl_info_t *pti);
void wlc_lcn20phy_read_table(phy_info_t *pi, phytbl_info_t *pti);
bool wlc_lcn20phy_txpwr_srom_read(phy_info_t *pi);
static void wlc_lcn20phy_sw_ctrl_tbl_init(phy_info_t *pi);
static void wlc_lcn20phy_rev0_reg_init(phy_info_t *pi);
static void wlc_lcn20phy_agc_setup(phy_info_t *pi);
static void wlc_lcn20phy_rev1_agc_setup(phy_info_t *pi, uint8 channel);
static void wlc_lcn20phy_rx_pu(phy_info_t *pi, bool bEnable);
static void wlc_lcn20phy_rx_pwrup(phy_info_t *pi, bool bEnable);
static void wlc_lcn20phy_tx_pu(phy_info_t *pi, bool bEnable, bool rxrfmode);
static void wlc_lcn20phy_dccal_force_cal(phy_info_t *pi, uint8 dccal_mode, int8 num_retry);
static void wlc_lcn20phy_dccal_init(phy_info_t *pi, uint8 dccal_mode, bool reset_tbl,
	uint8 bias_adj);
static void
wlc_lcn20phy_dccal_set_mode(phy_info_t *pi, uint8 dccal_mode, uint8 idacc_reinit_mode);

static void wlc_lcn20phy_rx_iq_cal(phy_info_t *pi);
static void wlc_lcn20phy_tx_iqlo_cal_txpwr(phy_info_t *pi);
static void wlc_lcn20phy_apply_gainidx_settings(phy_info_t *pi, uint8 gain_idx);
static int16 wlc_lcn20phy_noise_est(phy_info_t *pi, uint8 dvga1, int16 rfgain,
	int16 *noise_pwr_dB);
static void wlc_lcn20phy_dssf_cal(phy_info_t *pi);
static void wlc_lcn20phy_xtalpn_cal(phy_info_t *pi);
static uint16 wlc_lcn20phy_measure_rxspur(phy_info_t *pi, int16 freqKHz, int16 *spur_pwr_dB,
	uint16 ampl, uint8 count_log2, uint8 dvga1_gain);
static void wlc_lcn20phy_set_dssf_mode(phy_info_t *pi, uint8 mode, bool enable1,
	bool enable2, uint8 depth1, uint8 depth2, bool hgforced);
static void wlc_lcn20phy_set_dssf_gainth(phy_info_t *pi, uint8 notchIdx, const uint8 *gainth);
static void wlc_lcn20phy_set_dssf_freq(phy_info_t *pi, uint8 notchIdx, int32 f_Hz);

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
static void wlc_lcn20phy_lpc_write_maclut(phy_info_t *pi);
#endif /* LP_P2P_SOFTAP || WL_LPC */

static void wlc_lcn20phy_radio20692_init(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_rc_cal(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_tia_config(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_channel_tune(phy_info_t *pi, uint8 channel);
static void wlc_lcn20phy_radio20692_vcocal_isdone(phy_info_t *pi, bool set_delay);
static void wlc_lcn20phy_radio20692_vcocal(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_tssisetup(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_rx_iq_cal_setup(phy_info_t *pi);
static void wlc_lcn20phy_radio20692_rx_iq_cal_cleanup(phy_info_t *pi);
#ifdef LCN20_PAPD_ENABLE
/* functions for PAPD cal */
void
wlc_phy_papd_comp_rates_lcn20(phy_info_t *pi);
static void
wlc_phy_papd_cal_run_lcn20(phy_info_t *pi);
static void
wlc_phy_papd_phy_setup_lcn20(phy_info_t *pi);
static void
wlc_phy_papd_loopback_radio20692_setup_lcn20(phy_info_t *pi);
static void
wlc_phy_papd_gain_radio20692_setup_lcn20(phy_info_t *pi);
void
wlc_phy_papd_cal_lcn20(phy_info_t *pi, uint16 num_iter, uint16 startindex,
	uint16 yrefindex, uint16 stopindex);
static void
wlc_phy_papd_loopback_radio20692_cleanup_lcn20(phy_info_t *pi);
static void
wlc_phy_papd_phy_cleanup_lcn20(phy_info_t *pi);
bool
wlc_phy_papd_eps_valid_lcn20(phy_info_t *pi);

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
void
wlc_phy_papd_dump_eps_trace_lcn20(phy_info_t *pi, struct bcmstrbuf *b);
#endif // endif
void
wlc_phy_set_papd_offset_lcn20phy(phy_info_t *pi, int16 int_val);
int
wlc_phy_get_papd_offset_lcn20phy(phy_info_t *pi);

static void
wlc_lcn20phy_papd_init(phy_info_t *pi);
int32
wlc_lcn20phy_get_signal_power(phy_info_t *pi);

int
wlc_phy_papd_calidx_estimate_lcn20(phy_info_t *pi);

#define LIMIT(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#endif /* LCN20_PAPD_ENABLE */

static bool
BCMATTACHFN(wlc_lcn20phy_tuningtbl_select)(phy_info_t *pi);

static bool
BCMATTACHFN(wlc_lcn20phy_rx_gain_tbl_select)(phy_info_t *pi);

static bool
BCMATTACHFN(wlc_lcn20phy_tx_gain_tbl_select)(phy_info_t *pi);

static bool
BCMATTACHFN(wlc_lcn20phy_lna_params_select)(phy_info_t *pi);

static void
BCMATTACHFN(wlc_lcn20phy_tx_gain_tbl_copy)(lcn20phy_tx_gain_tbl_entry *gain_table_dst,
	const lcn20phy_tx_gain_tbl_entry * gain_table, uint32 num_elem);

static void
wlc_lcn20phy_tx_gain_tbl_free(phy_info_t *pi);

static bool wlc_lcn20phy_eu_edcrs_detect(phy_info_t *pi);

#ifdef WL11ULB
static void
wlc_phy_ulb_mode_lcn20phy(phy_info_t *pi, uint8 ulb_mode_set);
#endif /* WL11ULB */

#define wlc_radio20692_rc_cal_done(pi)  \
	(0 != (READ_RADIO_REGFLD_20692(pi, WL_RCCAL_CFG3, 0, wl_rccal_DONE)))

#define XTAL_FREQ_37P4MHZ			37400000
#define XTAL_FREQ_26P0MHZ			26000000
#define XTAL_FREQ_19P2MHZ			19200000

#define LCN20PHY_MASK_BT_RX	     0xff
#define LCN20PHY_SHIFT_BT_RX	 0
#define LCN20PHY_MASK_BT_ELNARX	 0xff00
#define LCN20PHY_SHIFT_BT_ELNARX 8
#define LCN20PHY_MASK_BT_TX	     0xff0000
#define LCN20PHY_SHIFT_BT_TX	 16

#define LCN20PHY_MASK_WL_MASK 0xff
#define LCN20PHY_MASK_TDM	  0x100
#define LCN20PHY_MASK_OVR_EN  0x200
#define LCN20PHY_MASK_ANT	  0x400

#define LCN20PHY_SW_CTRL_MAP_ANT     0x1
#define LCN20PHY_SW_CTRL_MAP_WL_RX   0x2
#define LCN20PHY_SW_CTRL_MAP_WL_TX   0x4
#define LCN20PHY_SW_CTRL_MAP_BT_TX   0x8
#define LCN20PHY_SW_CTRL_MAP_BT_PRIO 0x10
#define LCN20PHY_SW_CTRL_MAP_ELNA    0x20

#define LCN20PHY_SW_CTRL_NVRAM_PARAMS 5
#define LCN20PHY_SW_CTRL_TBL_LENGTH	  64
#define LCN20PHY_SW_CTRL_TBL_WIDTH	  16

/* swctrl GPIOs */
#define LCN20PHY_SWCTRL_GPIO_EN       0xff

/* Table ID's and offsets */
#define LCN20PHY_TBL_ID_IQLOCAL			0x00
#define LCN20PHY_TBL_ID_MINSIGSQR		0x02
#define LCN20PHY_TBL_ID_TXPWRCTL		0x07
#define LCN20PHY_TBL_ID_RFSEQ			0x08
#define LCN20PHY_TBL_ID_GAINIDX			0x0d
#define LCN20PHY_TBL_ID_SW_CTRL			0x0f
#define LCN20PHY_TBL_ID_GAINVAL			0x11
#define LCN20PHY_TBL_ID_GAIN			0x12
#define LCN20PHY_TBL_ID_DCOE			0x22
#define LCN20PHY_TBL_ID_IDAC			0x23
#define LCN20PHY_TBL_ID_IDACGMAP		0x24
#define LCN20PHY_TBL_ID_DCOE			0x22
#define LCN20PHY_TBL_ID_IDAC			0x23
#define LCN20PHY_TBL_ID_IDACGMAP		0x24
#define LCN20PHY_TBL_ID_SAMPLEPLAY		0x15
#ifdef LCN20_PAPD_ENABLE
#define  LCN20PHY_TBL_ID_SCALAR			0x1c
#define LCN20PHY_TBL_ID_EPSILON			0x21
#endif /* LCN20_PAPD_ENABLE */

/* CAL MODES */
#define LCN20PHY_CALMODE_DC		0x10
#define LCN20PHY_CALMODE_TXIQLO	0x20
#define LCN20PHY_CALMODE_RXIQ	0x40
#ifdef LCN20_PAPD_ENABLE
#define LCN20PHY_CALMODE_PAPD	0x80
#endif /* LCN20_PAPD_ENABLE */
#define LCN20PHY_CALMODE_DSSF	0x100
#define LCN20PHY_CALMODE_XTALPN	0x200

#define TEMPSENSE 			1
#define VBATSENSE       2
/* Vmid/Gain settings */
#define AUXPGA_VBAT_VMID_VAL 0xa4
#define AUXPGA_VBAT_GAIN_VAL 0x3
#define AUXPGA_TEMPER_VMID_VAL 0xa4
#define AUXPGA_TEMPER_GAIN_VAL 0x3

/* DCCAL modes 0-7: 3bits {idact,idacc,dcoe} */
#define LCN20PHY_DCCALMODE_BYPASS	0
#define LCN20PHY_DCCALMODE_DCOE		1
#define LCN20PHY_DCCALMODE_IDACC	2
#define LCN20PHY_DCCALMODE_IDACT	4
#define LCN20PHY_DCCALMODE_ALL	\
	(LCN20PHY_DCCALMODE_DCOE |\
	LCN20PHY_DCCALMODE_IDACC |\
	LCN20PHY_DCCALMODE_IDACT)

#define LCN20PHY_BYPASS_MASK	LCN20PHY_DCCALMODE_ALL

/* idacc_reinit_mode:
* disable reinits			0
* per-packet reinit			1
* tx2rx_only				4
* tx2rx_reinit+tx2rx_only	6
*/
#define LCN20PHY_IDACCMODE_DISREINIT	0
#define LCN20PHY_IDACCMODE_PPREINIT		1
#define LCN20PHY_IDACCMODE_TX2RX		4
#define LCN20PHY_IDACCMODE_REINITTX2RX	6

/* delay in usec */
#define LCN20PHY_SPINWAIT_DCCAL	100
#define LCN20PHY_DCCAL_DELAY 50

#define LCN20PHY_IDACCMODE_MASK	\
	(LCN20PHY_phyreg2dccal_config_3_idacc_ppkt_reinit_MASK |\
	LCN20PHY_phyreg2dccal_config_3_idacc_tx2rx_reinit_MASK |\
	LCN20PHY_phyreg2dccal_config_3_idacc_tx2rx_only_MASK)

#define LCN20PHY_INITDCOE_MASK	\
	(LCN20PHY_phyreg2dccal_config_0_dcoe_acc_cexp_MASK |\
	LCN20PHY_phyreg2dccal_config_0_dcoe_wait_cinit_MASK |\
	LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_inpshort_MASK |\
	LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_outshort_MASK |\
	LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_init_MASK |\
	LCN20PHY_phyreg2dccal_config_0_dcoe_zero_idac_MASK)

#define LCN20PHY_INITDCOE_VAL	\
	((6 << LCN20PHY_phyreg2dccal_config_0_dcoe_acc_cexp_SHIFT) |\
	(40 << LCN20PHY_phyreg2dccal_config_0_dcoe_wait_cinit_SHIFT) |\
	(0 << LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_inpshort_SHIFT) |\
	(1 << LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_outshort_SHIFT) |\
	(0 << LCN20PHY_phyreg2dccal_config_0_dcoe_lna1_init_SHIFT) |\
	(1 << LCN20PHY_phyreg2dccal_config_0_dcoe_zero_idac_SHIFT))

#define LCN20PHY_INITIDACC_MASK	\
	(LCN20PHY_phyreg2dccal_config_2_idacc_acc_cexp_MASK |\
	LCN20PHY_phyreg2dccal_config_2_idacc_wait_cinit_MASK |\
	LCN20PHY_phyreg2dccal_config_2_idacc_mag_select_MASK |\
	LCN20PHY_phyreg2dccal_config_2_idac_lna1_refidx_MASK |\
	LCN20PHY_phyreg2dccal_config_2_idac_lna1_scaling_MASK)

#define LCN20PHY_INITIDACC_VAL	\
	((6 << LCN20PHY_phyreg2dccal_config_2_idacc_acc_cexp_SHIFT) |\
	(40 << LCN20PHY_phyreg2dccal_config_2_idacc_wait_cinit_SHIFT) |\
	(3 << LCN20PHY_phyreg2dccal_config_2_idacc_mag_select_SHIFT) |\
	(5 << LCN20PHY_phyreg2dccal_config_2_idac_lna1_refidx_SHIFT) |\
	(0 << LCN20PHY_phyreg2dccal_config_2_idac_lna1_scaling_SHIFT))

#define LCN20PHY_NUM_DIG_FILT_COEFFS 17
#define LCN20PHY_NUM_TX_DIG_FILTERS_CCK 17
/* filter id, followed by coefficients */
static const uint16 LCN20PHY_txdigfiltcoeffs_cck[LCN20PHY_NUM_TX_DIG_FILTERS_CCK]
	[1+LCN20PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 0, 20, -92, 301, 301, 0, 378, -184, 64, 128, 64, 504,
	-128, 64, 128, 64, 8},
	{ 1, 0, 165, -92, 150, 150, 0, 505, -184, 64, 128, 64, 508, -128, 64, 128, 64, 8},
	{ 2, 1, 415, 1874, 64, 128, 64, 792, 1656, 192, 384, 192, 778, 1582, 64, 128, 64, 8},
	{ 3, 1, 302, 1841, 129, 258, 129, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 20, 1, 360, -164, 242, -314, 242, 752, -328, 205, -203, 205, 767, -288, 253, 183, 253, 8},
	{ 21, 1, 360, 1884, 149, 1874, 149, 752, 1720, 205, 1884, 205, 767, 1760, 256, 273, 256, 8},
	{ 22, 1, 360, 1884, 98, 1948, 98, 752, 1720, 205, 1924, 205, 767, 1760, 256, 352, 256, 8},
	{ 23, 1, 350, 1884, 116, 1966, 116, 752, 1720, 205, 2008, 205, 767, 1760, 129, 235, 129, 8},
	{ 24, 1, 325, 1884, 32, 40, 32, 756, 1720, 256, 471, 256, 766, 1760, 262, 1878, 262, 8},
	{ 25, 1, 299, 1884, 51, 64, 51, 736, 1720, 256, 471, 256, 765, 1760, 262, 1878, 262, 8},
	{ 26, 1, 277, 1943, 39, 117, 88, 637, 1838, 64, 192, 144, 614, 1864, 128, 384, 288, 8},
	{ 27, 1, 245, 1943, 49, 147, 110, 626, 1838, 162, 485, 363, 613, 1864, 62, 186, 139, 8},
	{ 28, 1, 360, 1884, 149, 1874, 149, 752, 1720, 205, 1884, 205, 767, 1760, 114, 121, 114, 8},
	{ 30, 1, 302, 1841, 61, 122, 61, 658, 1720, 205, 410, 205, 754, 1760, 170, 340, 170, 8},
	{ 31, 1, 319, 1817, 490, 0, 490, 699, 1678, 324, 0, 324, 754, 1760, 114, 0, 114, 8},
	{ 40, 1, 360, 1884, 242, 1734, 242, 752, 1720, 205, 1845, 205, 767, 1760, 511, 370, 511, 8},
	{ 50, 1, 0x1d9, 0xff0c, 0x20, 0x40, 0x20, 0x3a2, 0xfe41, 0x10, 0x20, 0x10, 0x3a1,
	0xfe58, 0x10, 0x20, 0x10, 8}
	};

#define LCN20PHY_NUM_TX_DIG_FILTERS_OFDM 19
static const uint16 LCN20PHY_txdigfiltcoeffs_ofdm[LCN20PHY_NUM_TX_DIG_FILTERS_OFDM]
	[1+LCN20PHY_NUM_DIG_FILT_COEFFS] = {
	{ 0, 0, 0, 0, 511, 0, 0x0, 0x0, 0x0, 511, 0x0, 0x0,
	0, 0, 511, 0, 0, 9},
	{ 1, 0, 164, 0, 301, 301, 0, 482, -285, 128, 256, 128,
	261, -431, 64, 128, 64, 8},
	{ 2, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32,
	748, -270, 128, -30, 128, 8},
	{3, 0, 375, 0xFF16, 37, 76, 37, 799, 0xFE74, 32, 20, 32, 748,
	0xFEF2, 148, 0xFFDD, 148, 8},
	{4, 0, 307, 1966, 53, 106, 53, 779, 1669, 53, 2038, 53, 765,
	1579, 212, 1846, 212, 8},
	{5, 0, 0x1c5, 0xff1d, 0x20, 0x40, 0x20, 0, 0, 0x100, 0, 0, 0x36b,
	0xfe82, 0x14, 0x29, 0x14, 8},
	{ 6, 0, 375, -234, 37, 76, 37, 799, -396, 32, 20, 32,
	748, -270, 114, -27, 114, 8},
	{ 7, 0, 0xaa, 0, 0x1d3, 0x1d2, 0, 0x199, 0x6c9, 0x80, 0x100, 0x80,
	0x62, 0x642, 0x24, 0x48, 0x24, 9}, /* BB LPF Filter cutoff = 9.5MHz */
	{ 8, 0, 0xae, 0, 0x179, 0x178, 0, 0x1c2, 0x6c4, 0x80, 0x100, 0x80,
	0x9e, 0x641, 0x24, 0x48, 0x24, 0x0009}, /* BB LPF Filter cutoff = 9.1MHz */
	{ 9, 0, 0xb2, 0, 0x13f, 0x13f, 0, 0x1df, 0x6c0, 0x80, 0x100, 0x80,
	0xca, 0x641, 0x24, 0x48, 0x24, 0x0009}, /* BB LPF Filter cutoff = 8.8MHz */
	{10, 0, 0xa2, 0, 0x100, 0x100, 0x0, 0, 0, 511, 0, 0, 0x278,
	0xfea0, 256, 511, 256, 8},
	{12, 0, 394, -234, 29, 58, 29, 800, -394, 24, 48, 24, 836,
	-352, 38, 76, 38, 8},
	{ 13, 0, 174, 0, 467, 466, 0, 439, -320, 120, 240, 120,
	85, -423, 36, 72, 36, 9},
	{ 14, 0, 177, 0, 438, 437, 0, 457, -324, 120, 240, 120,
	77, -411, 35, 70, 35, 9},
	{ 15, 0, 186, 0, 319, 319, 0, 523, -327, 120, 240, 120,
	174, -417, 35, 70, 35, 9},
	{ 16, 0, 0, 0, 256, 256, 0, 0, 0, 511, 0, 0,
	0, 0, 511, 0, 0, 9},
	{ 17, 0, 0, 0, 256, 256, 0, 0, 0, 511, 0, 0,
	0, 0, 492, 20, 0, 9},
	{ 18, 0, 0, 0, 256, 256, 0, 0, 0, 511, 0, 0,
	0, 0, 292, 292, 0, 9},
	{ 19, 0, 0, 0, 256, 256, 0, 0, 0, 511, 0, 0,
	0, 0, 501, 63, 0, 9}
	/* Filter 19 added to increase spectral flatness margin by ~1.2 dB on Maccabee module */
	};

#define  LCN20PHY_MAX_CCK_DB_SCALING 7
#define  LCN20PHY_DB2SCALEFCTR_SHIFT 6
/* The scale factors are based on the below formula
	round ((pow(10, (-db)/20.0)) << DB2SCALEFCTR_SHIFT)
*/
static const uint8 LCN20PHY_db2scalefctr_cck[LCN20PHY_MAX_CCK_DB_SCALING] =
	{57, 51, 45, 40, 36, 32, 29};

/* RX farrow defines and macros */
#define LCN20PHY_RXFAR_BITSINMU	24
#define LCN20PHY_RXFAR_NUM		3
#define LCN20PHY_RXFAR_DEN		2
#define LCN20PHY_RXFAR_BW		20
#define LCN20PHY_RXFAR_M		8
#define LCN20PHY_RXFAR_VCODIV	6

/* den * vco_div * M * 2 * bw / num */
#define LCN20PHY_RXFAR_FACTOR	\
	((LCN20PHY_RXFAR_DEN) *\
	(LCN20PHY_RXFAR_VCODIV) *\
	(LCN20PHY_RXFAR_M) *\
	(2 * LCN20PHY_RXFAR_BW))/LCN20PHY_RXFAR_NUM

#define LCN20PHY_TX_FARROW_RATIO	1920
static const
uint32 mu_deltaLUT[14] = {
	6677499,
	6663685,
	6649929,
	6636229,
	6622585,
	6608998,
	6595466,
	6581989,
	6568567,
	6555200,
	6541888,
	6528629,
	6515424,
	6483948
};

#define LCN20PHY_ACITBL_OFFSET 6

#define LCN20PHY_NORTBL_ACITBL_RSSIOFFSET_REPORT 10
#define LCN20PHY_NORTBL_ACITBL_OFFSET_NEW 6
#define LCN20PHY_NORTBL_OFFSET_NEW 18
#define LCN20PHY_ACITBL_OFFSET_NEW (LCN20PHY_NORTBL_ACITBL_OFFSET_NEW+LCN20PHY_NORTBL_OFFSET_NEW)

/* TR switch modes */
#define LCN20PHY_TRS_TXMODE	0
#define LCN20PHY_TRS_RXMODE	1
#define LCN20PHY_TRS_MUTE	2
#define LCN20PHY_TRS_TXRXMODE	3

/* ------ Gain Table Config ------ */
#define LCN20PHY_GAIN_TBL_OFFSET         41
#define LCN20PHY_INIT_GAIN_DB            63
#define LCN20PHY_MAX_GAIN_DB			 72

/* ------ rx power measurement ----- */
#define MAX_NOMAL_GAINTBL_INDEX				37
#define MAX_WHOLE_GAINTBL_INDEX				75
#define NOISE_MEAS_MIN_GAIN_DB_NEW		(-16+LCN20PHY_NORTBL_OFFSET_NEW)
#define NOISE_MEAS_MAX_DVGA_GAIN		15
/* the max positive number of int16 is defined as INVALID */
#define NOISE_MEAS_INVALID_TOT_GAIN_DB	32767

/* ------ Spur-Config ------ */
#define	LCN20PHY_SPURCONFIG_DEFAULT  0
#define	LCN20PHY_SPURCONFIG_AUTO     1

/* ------ definitions for noise estimation ----- */
#define LCN20PHY_NOISEDB_SCALE           2
#define LCN20PHY_NOISEEST_MAXTRY         10
#define LCN20PHY_NOISEEST_SAMPSLOG2      9
#define LCN20PHY_NOISEEST_NUM_SAMPS      (1 << LCN20PHY_NOISEEST_SAMPSLOG2)
#define LCN20PHY_NOISEEST_COLLECTLOG2    3
#define LCN20PHY_NOISEEST_NUM_COLLECT    ((1 << LCN20PHY_NOISEEST_COLLECTLOG2) + 2)
#define LCN20PHY_NOISEEST_REFDBM         -98
#define LCN20PHY_NOISEEST_REFNOISEPWRDB  36

/* ------ definitions for RSSI cal ------ */
#define LCN20PHY_NORTBL_GAINIDX_BOUNDRY1  31
#define LCN20PHY_ACITBL_GAINIDX_BOUNDRY1  61
#define LCN20PHY_ACITBL_GAIN_OFFSET1      4
#define LCN20PHY_ACITBL_GAINIDX_BOUNDRY2  58
#define LCN20PHY_ACITBL_GAIN_OFFSET2      30
#define LCN20PHY_RSSI_DELTA_ROUT0_IDX     0
#define LCN20PHY_RSSI_DELTA_ROUT8_IDX     1
#define LCN20PHY_RSSI_DELTA_ELNABYP_IDX   2
#define LCN20PHY_GAIN_STEP				  3
#define LCN20PHY_ACITBL_TIA_OFFSET        20
#define LCN20PHY_CCK                      0
#define LCN20PHY_OFDM                     1
#define LCN20PHY_HT                       2
#define LCN20PHY_INVALID_FRAMETYPE        255
#define LCN20PHY_RATE_1M                  10
#define LCN20PHY_RATE_2M                  20
#define LCN20PHY_RATE_5M5                 55
#define LCN20PHY_RATE_11M                 46
#define LCN20PHY_INVALID_RATE             255
#define LCN20PHY_OFDM_PLCP_OFFSET         8
#define LCN20PHY_OFDM_IDX                 4
#define LCN20PHY_HT_IDX                   12

/* ------ definitions and tables for DSSF calibrations ----- */
#define LCN20PHY_DSSF_MAX_NOTCHES        2  /* must be 2 */
#define LCN20PHY_DSSFSPUR_LOW_BOUND      38 /* (spur - noise) lower than this level is ignored */
#define LCN20PHY_DSSFSPUR_THRESH_1       66
#define LCN20PHY_DSSFSPUR_THRESH_2       80
#define LCN20PHY_DSSF_SC_LUTSIZE         6
#define LCN20PHY_SPUR_NOISE_DELTA_ADJ    28 /* Q2 format */

#define LCN20PHY_MEAS_RXSPUR_AMPL        500
#define LCN20PHY_MEAS_RXSPUR_CNTLOG2     4
#define LCN20PHY_MEAS_RXSPUR_DVGA1       11
#define LCN20PHY_MEAS_RXSPUR_IDACIDX     7    /* must correspond to init gain */
#define LCN20PHY_MEAS_RXSPUR_GAINTBLIDX  (((LCN20PHY_GAIN_TBL_OFFSET+LCN20PHY_INIT_GAIN_DB)*85)>>8)

/* ------ definitions and tables for XTALPN calibration ----- */
#define LCN20PHY_XTALPN_CAL_THRESH       500
#define LCN20PHY_XTALPN_CAL_LOG2NUMDFT   2

/* Bits in GAINIDXTBL */
#define LCN20PHY_GainIdxTbl_tr_attn_SHIFT       23
#define LCN20PHY_GainIdxTbl_tr_attn_MASK       (0x1 << LCN20PHY_GainIdxTbl_tr_attn_SHIFT)
#define LCN20PHY_GainIdxTbl_elna_SHIFT          22
#define LCN20PHY_GainIdxTbl_elna_MASK          (0x1 << LCN20PHY_GainIdxTbl_elna_SHIFT)
#define LCN20PHY_GainIdxTbl_slna_byp_SHIFT      21
#define LCN20PHY_GainIdxTbl_slna_byp_MASK      (0x1 << LCN20PHY_GainIdxTbl_slna_byp_SHIFT)
#define LCN20PHY_GainIdxTbl_slna_rout_SHIFT     17
#define LCN20PHY_GainIdxTbl_slna_rout_MASK     (0xf << LCN20PHY_GainIdxTbl_slna_rout_SHIFT)
#define LCN20PHY_GainIdxTbl_slna_gain_SHIFT     14
#define LCN20PHY_GainIdxTbl_slna_gain_MASK     (0x7 << LCN20PHY_GainIdxTbl_slna_gain_SHIFT)
#define LCN20PHY_GainIdxTbl_gaintbl_idx_SHIFT   7
#define LCN20PHY_GainIdxTbl_gaintbl_idx_MASK   (0x7f << LCN20PHY_GainIdxTbl_gaintbl_idx_SHIFT)

/* Bits in GAINTBL */
#define LCN20PHY_GainTbl_lna2_gain_SHIFT      0
#define LCN20PHY_GainTbl_lna2_gain_MASK      (0xf << LCN20PHY_GainTbl_lna2_gain_SHIFT)
#define LCN20PHY_GainTbl_tia_gain_SHIFT       7
#define LCN20PHY_GainTbl_tia_gain_MASK       (0xf << LCN20PHY_GainTbl_tia_gain_SHIFT)
#define LCN20PHY_GainTbl_dvga1_gain_SHIFT     19
#define LCN20PHY_GainTbl_dvga1_gain_MASK     (0xf << LCN20PHY_GainTbl_dvga1_gain_SHIFT)

#define LCN20PHY_DSSF_GAIN_TH111  (41 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH112  (99 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH113  (99 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH121  (41 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH122  (99 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH123  (99 + LCN20PHY_NORTBL_OFFSET_NEW)

#define LCN20PHY_DSSF_GAIN_TH211  (25 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH212  (25 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH213  (99 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH221  (41 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH222  (73 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH223  (99 + LCN20PHY_NORTBL_OFFSET_NEW)

#define LCN20PHY_DSSF_GAIN_TH311  (25 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH312  (25 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH313  (79 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH321  (41 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH322  (73 + LCN20PHY_NORTBL_OFFSET_NEW)
#define LCN20PHY_DSSF_GAIN_TH323  (79 + LCN20PHY_NORTBL_OFFSET_NEW)

#define LCN20PHY_DSSS_DETECTION_THRESHOLD 97
#define LCN20PHY_OFDM_DETECTION_THRESHOLD 97

static const uint8 dssf_gainth_set[3][6] = {
	{
	LCN20PHY_DSSF_GAIN_TH111, LCN20PHY_DSSF_GAIN_TH112, LCN20PHY_DSSF_GAIN_TH113,
	LCN20PHY_DSSF_GAIN_TH121, LCN20PHY_DSSF_GAIN_TH122, LCN20PHY_DSSF_GAIN_TH123
	},
	{
	LCN20PHY_DSSF_GAIN_TH211, LCN20PHY_DSSF_GAIN_TH212, LCN20PHY_DSSF_GAIN_TH213,
	LCN20PHY_DSSF_GAIN_TH221, LCN20PHY_DSSF_GAIN_TH222, LCN20PHY_DSSF_GAIN_TH223
	},
	{
	LCN20PHY_DSSF_GAIN_TH311, LCN20PHY_DSSF_GAIN_TH312, LCN20PHY_DSSF_GAIN_TH313,
	LCN20PHY_DSSF_GAIN_TH321, LCN20PHY_DSSF_GAIN_TH322, LCN20PHY_DSSF_GAIN_TH323
	}
};

static const int16 spur_offset_37p4MHz[14][2] = { /* offset from center in KHz */
	{0, 0}, {0, 0}, {9000, 8000}, {4000, 3000}, {-1000, -2000},
	{-6000, -7000}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
	{6400, 0}, {1400, 0}, {-3600, 0}, {0, 0}
};

static const int16 spur_offset_37p4MHz_vco980[14][2] = { /* offset from center in KHz */
	{0, -6500}, {0, 0}, {9000, 0}, {4000, 0}, {-1000, 0},
	{-6000, 0}, {0, 8000}, {0, 3000}, {0, -2000}, {0, -7000},
	{6400, 0}, {1400, 0}, {-3600, 0}, {0, 0}
};

static const int16 spur_offset_37p4MHz_vco326p4[14][2] = { /* offset from center in KHz */
	{0, 0}, {0, 0}, {9000, 0}, {4000, 0}, {-1000, 0},
	{-6000, 0}, {0, 6000}, {0, 1000}, {0, -4000}, {0, -9000},
	{6400, 0}, {1400, 0}, {-3600, 0}, {0, 0}
};

static const int16 spur_offset_26MHz[14][2] = { /* offset from center in KHz */
	{6000, 0}, {1000, 0}, {-4000, 8000}, {-9000, 3000}, {0, -2000},
	{7000, -7000}, {2000, 0}, {-3000, 0}, {-8000, 0}, {0, 0},
	{8000, 0}, {3000, 0}, {-2000, 0}, {0, 0}
};

static const int16 spur_offset_26MHz_vco980[14][2] = { /* offset from center in KHz */
	{6000, -6500}, {1000, 0}, {-4000, 0}, {-9000, 0}, {0, 0},
	{7000, 0}, {2000, 8000}, {-3000, 3000}, {-8000, -2000}, {0, -7000},
	{8000, 0}, {3000, 0}, {-2000, 0}, {0, 0}
};

static const int16 spur_offset_26MHz_vco326p4[14][2] = { /* offset from center in KHz */
	{6000, 0}, {1000, 0}, {-4000, 0}, {-9000, 0}, {0, 0},
	{7000, 0}, {2000, 6000}, {-3000, 1000}, {-8000, -4000}, {0, -9000},
	{8000, 0}, {3000, 0}, {-2000, 0}, {0, 0}
};

static const int16 spur_offset_19p2MHz[14][2] = { /* offset from center in KHz */
	{7200, 0}, {2200, 0}, {-2800, 8000}, {-7800, 3000}, {6400, -2000},
	{1400, -7000}, {-3600, 0}, {-8600, 0}, {5600, 0}, {600, 0},
	{-4400, 0}, {-9400, 9800}, {4800, 0}, {-7200, 0}
};

static const int16 spur_offset_19p2MHz_vco980[14][2] = { /* offset from center in KHz */
	{7200, -6500}, {2200, 0}, {-2800, 0}, {-7800, 0}, {6400, 0},
	{1400, 0}, {-3600, 8000}, {-8600, 3000}, {5600, -2000}, {600, -7000},
	{-4400, 0}, {-9400, 9800}, {4800, 0}, {-7200, 0}
};

static const int16 spur_offset_19p2MHz_vco326p4[14][2] = { /* offset from center in KHz */
	{7200, 0}, {2200, 0}, {-2800, 0}, {-7800, 0}, {6400, 0},
	{1400, 0}, {-3600, 6000}, {-8600, 1000}, {5600, -4000}, {600, -9000},
	{-4400, 0}, {-9400, 9800}, {4800, 0}, {-7200, 0}
};

static const uint16 sc_deweight[LCN20PHY_DSSF_SC_LUTSIZE][3] = {
	{125, 61, 38}, {250, 54, 46}, {300, 51, 49},
	{25, 64, 32}, {50, 64, 34}, {225, 56, 44}
};

/* ------ definitions abd tables for RXIQ calibrations ----- */

/* setmode: 0 - fetch values from phyregs into *pcomp
 *		  1 - deposit values from *pcomp into phyregs
 *		  2 - set all rxiq coeffs to 0
 *
 * pcomp: input/output comp buffer
 */
#define LCN20PHY_RXIQCOMP_GET	0
#define LCN20PHY_RXIQCOMP_SET	1
#define LCN20PHY_RXIQCOMP_RESET	2

#define LCN20PHY_RXIQCAL_TONEFREQKHZ_0		2000
#define LCN20PHY_RXIQCAL_TONEFREQKHZ_1		6000
#define LCN20PHY_RXIQCAL_MAXNUM_FREQS	4
#define LCN20PHY_RXIQCAL_TONEAMP		181
#define LCN20PHY_RXIQCAL_NUMSAMPS		8000

#define LCN20PHY_RXCAL_NUMRXGAINS		16

#define LCN20PHY_RXIQCAL_LEAKAGEPATH	1
#define LCN20PHY_RXIQCAL_PAPDPATH		2

#define LCN20PHY_SPINWAIT_IQEST_QT_USEC		1000*1000
#define LCN20PHY_SPINWAIT_IQEST_USEC		100*1000

/* Gain Candidates For leakage path :  LNA, TIA, Farrow
* LNA, TIA, Farrow
* Leakage path works only for 5g
*/
static const lcn20phy_rxcal_rxgain_t
gaintbl_lkpath[LCN20PHY_RXCAL_NUMRXGAINS] = {
	{ -4, 0, 2, 0 },
	{ -4, 0, 1, 0 },
	{ -4, 0, 0, 0 },
	{ -3, 0, 0, 0 },
	{ -2, 0, 0, 0 },
	{ -1, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 2, 0, 0 },
	{ 0, 3, 0, 0 },
	{ 0, 4, 0, 0 },
	{ 0, 5, 0, 0 },
	{ 0, 6, 0, 0 },
	{ 0, 7, 0, 0 },
	{ 0, 8, 0, 0 },
	{ 0, 9, 0, 0 }
};
/* Gain Candidates For papd loopback  : Tia ,Farrow & dvga2 */
static const lcn20phy_rxcal_rxgain_t
gaintbl_papdpath[LCN20PHY_RXCAL_NUMRXGAINS] = {
	{ 0, 1, 4, 0 },
	{ 0, 0, 3, 0 },
	{ 0, 1, 3, 0 },
	{ 0, 2, 3, 0 },
	{ 0, 3, 3, 0 },
	{ 0, 4, 3, 0 },
	{ 0, 5, 3, 0 },
	{ 0, 6, 3, 0 },
	{ 0, 7, 3, 0 },
	{ 0, 8, 3, 0 },
	{ 0, 9, 3, 0 },
	{ 0, 10, 3, 0 },
	{ 0, 10, 3, 1 },
	{ 0, 10, 3, 2 },
	{ 0, 10, 3, 3 },
	{ 0, 10, 3, 4 }
};

/* ------ definitions and tables for TXIQLO calibrations ----- */

/* restart : start from zero coeffs
* refine : from previous results
* dlo_recal : do only digital LO cal
*/
typedef enum {
	TXIQLOCAL_RESTART = 0,
	TXIQLOCAL_REFINE = 1,
	TXIQLOCAL_DLORECAL = 2
} phy_txiqlocal_mode_t;

/* Reference Gain "Ladders" (bbmult is normalized to 1.0 here for later scaling) */
static const
lcn20phy_txiqcal_ladder_t lcn20phy_txiqlocal_ladder_lo[] = {
{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
{25, 0}, {25, 1}, {25, 2}, {25, 3}, {25, 4}, {25, 5},
{25, 6}, {25, 7}, {35, 7}, {50, 7}, {71, 7}, {100, 7}};
static const
lcn20phy_txiqcal_ladder_t lcn20phy_txiqlocal_ladder_iq[] = {
{3, 0}, {4, 0}, {6, 0}, {9, 0}, {13, 0}, {18, 0},
{25, 0}, {35, 0}, {50, 0}, {71, 0}, {100, 0}, {100, 1},
{100, 2}, {100, 3}, {100, 4}, {100, 5}, {100, 6}, {100, 7}};

#define LCN20PHY_TXIQLOCAL_THRSLAD	0x3d
/* gain control segments */
#define LCN20PHY_TXIQLOCAL_NSGCTRL	0x76
/* correlation segments */
#define LCN20PHY_TXIQLOCAL_NSCORRS	0x79
#define LCN20PHY_TXIQLOCAL_CMDNUM	\
	(LCN20PHY_TXIQLOCAL_NSCORRS << 8) |\
	(LCN20PHY_TXIQLOCAL_NSGCTRL)

/* 1 = requesting iqcal mode of sample play buffer */
#define LCN20PHY_TXIQLOCAL_SPBMODE	1

#define LCN20PHY_TXIQLOCAL_TONEFHZ	(2 * 1000 * 1000)
#define LCN20PHY_TXIQLOCAL_TONEAMP	250

#define LCN20PHY_SPINWAIT_TXIQLOCAL_USEC	100*1000

#define LCN20PHY_CAL_TYPE_IQ                 0
#define LCN20PHY_CAL_TYPE_LOFT_DIG           2

/* ------ definitions and tables for RCCAL ----- */

/* Coefficients generated by 47xxtcl/rgphy/20691/ */
/* lpf_tx_coefficient_generator/filter_tx_tiny_generate_python_and_tcl.py */
static const uint16 lpf_g10[6][15] = {
	{1188, 1527, 1866, 2206, 2545, 2545, 1188, 1188,
	1188, 1188, 1188, 1188, 1188, 1188, 1188},
	{3300, 4242, 5185, 6128, 7071, 7071, 3300, 3300,
	3300, 3300, 3300, 3300, 3300, 3300, 3300},
	{16059, 16059, 16059, 17294, 18529, 18529, 9882,
	1976, 2470, 3088, 3953, 4941, 6176, 7906, 12353},
	{24088, 24088, 25941, 31500, 37059, 37059, 14823,
	2964, 3705, 4632, 5929, 7411, 9264, 11859, 18529},
	{29647, 32118, 34589, 42001, 49412, 49412, 19765,
	3705, 4941, 6176, 7411, 9882, 12353, 14823, 24706},
	{32941, 36236, 39530, 42824, 46118, 49412, 19765,
	4117, 4941, 6588, 8235, 9882, 13176, 16470, 26353}
};
static const uint16 lpf_g12[6][15] = {
	{1882, 1922, 1866, 1752, 1606, 1275, 2984, 14956,
	11880, 9436, 7495, 5954, 4729, 3756, 2370},
	{5230, 5341, 5185, 4868, 4461, 3544, 8289, 41544,
	33000, 26212, 20821, 16539, 13137, 10435, 6584},
	{24872, 19757, 15693, 13424, 11425, 9075, 24258, 24316,
	24144, 23972, 24374, 24201, 24029, 24432, 24086},
	{37309, 29635, 25351, 24452, 22850, 18151, 36388, 36474,
	36216, 35959, 36561, 36302, 36044, 36648, 36130},
	{44360, 38172, 32654, 31496, 29433, 23379, 46870, 44045,
	46648, 46318, 44150, 46759, 46428, 44254, 46538},
	{49288, 43066, 37319, 32113, 27471, 23379, 46870, 48939,
	46648, 49406, 49055, 46759, 49523, 49172, 49640}
};
static const uint16 lpf_g21[6][15] = {
	{1529, 1497, 1542, 1643, 1793, 2257, 965, 192, 242,
	305, 384, 483, 609, 766, 1215},
	{4249, 4160, 4285, 4565, 4981, 6270, 2681, 534, 673,
	847, 1067, 1343, 1691, 2129, 3375},
	{6135, 7723, 9723, 11367, 13356, 16814, 6290, 6275,
	6320, 6365, 6260, 6305, 6350, 6245, 6335},
	{9202, 11585, 13543, 14041, 15025, 18916, 9435, 9413,
	9480, 9548, 9391, 9458, 9525, 9368, 9503},
	{13760, 15990, 18693, 19380, 20738, 26108, 13023, 13858,
	13085, 13178, 13825, 13054, 13147, 13793, 13116},
	{22016, 25197, 29078, 33791, 39502, 46415, 23152, 22173,
	23262, 21964, 22121, 23207, 21912, 22068, 21860}
};
static const uint16 lpf_g11[6] = {994, 2763, 12353, 18529, 17470, 23293};
static const uint16 g_passive_rc_tx[6] = {62, 172, 772, 1158, 1544, 2058};
static const uint16 biases[6] = {24, 48, 96, 96, 128, 128};
static const int8 g_index1[15] = {0, 1, 2, 3, 4, 5, -2, -9, -8, -7, -6, -5, -4, -3, -1};

#define LCN20PHY_MAX_2069_RCCAL_WAITLOOPS 100
/* LCN20PHY_NUM_2069_RCCAL_CAPS: 3->2 to skip DACbuf RC cal */
#define LCN20PHY_NUM_2069_RCCAL_CAPS 2

/* ------ definitions for PAPD cal ----- */
#ifdef LCN20_PAPD_ENABLE
#define PAPD_TX_IIR_FILTER_TYPE 18
#define PAPD_EPSILON_OFFSET_DEFAULT -38
#define PAPD_LUT_OFFSET 7
#define PAPD_GAIN_CTRL_INIT_IDX 63
#define PAPD_GAIN_CTRL_PCKTS_PER_ITER_NPT 1
#define PAPD_GAIN_CTRL_MAX_NUM_STEP 5
#define LCN20PHY_SPINWAIT_PAPDCAL_USEC	200*1000
#define LCN20_PAPD_EPS_TBL_SIZE 64
#define LCN20_PAPD_TBL_WRITE_STEP_SIZE 8
#define LCN20_PAPD_LUT_SELECT_TBL_SIZE 128
/* The following parameter is used to update the RF offset table for powers less
 * than (LCN20_PAPD_RF_OFFSET_EN_POW_Q2 / 4) dBm
 */
#define LCN20_PAPD_RF_OFFSET_EN_POW_Q2 56

static const uint32 lcn20_papd_scaltbl[] = {
	0xB460066, 0xAA5006C, 0xA0C0073, 0x97C007A, 0x8F40081, 0x8740089,
	0x7FB0091, 0x7890099, 0x71D00A2, 0x6B700AC, 0x65700B6, 0x5FC00C1,
	0x5A600CC, 0x55600D8, 0x50900E5, 0x4C100F3, 0x47D0101, 0x43D0110,
	0x4000121, 0x3C70132, 0x3910144, 0x35E0157, 0x32D016B, 0x3000181,
	0x2D50198, 0x2AC01B0, 0x28601C9, 0x26201E5, 0x2400201, 0x2200220,
	0x2010240, 0x1E50262, 0x1C90286, 0x1B002AC, 0x19802D5, 0x1810300,
	0x16B032D, 0x157035E, 0x1440391, 0x13203C7, 0x1210400, 0x110043D,
	0x101047D, 0x0F304C1, 0x0E50509, 0x0D80556, 0x0CC05A6, 0x0C105FC,
	0x0B60657, 0x0AC06B7, 0x0A2071D, 0x0990789, 0x09107FB, 0x0890874,
	0x08108F4, 0x07A097C, 0x0730A0C, 0x06C0AA5, 0x0660B46, 0x0610BF1,
	0x05B0CA6, 0x0560D66, 0x0510E31, 0x04D0F09};

/* PAPD MODE CONFIGURATION */
#define PAPD_LMS 0
#define PAPD_ANALYTIC 1
#define PAPD_ANALYTIC_WO_YREF 2
/* PHY_PACALSTATUS */
#define PHY_PACALSTATUS_CAL_TIMEOUT 0x1
#define PHY_PACALSTATUS_POWER_CONTROL_DIVERGED 0x2
#define PHY_PACALSTATUS_AMPM_OVERFLOW 0x4
/* PAPD EVM VALID TEST */
#define LCN20_PAPD_EVAL_IDX_LOW       3
#define LCN20_PAPD_EVAL_MIN_EPS_V     35
#define LCN20_PAPD_EVAL_MIN_EPS_DIFF  -5
#define LCN20_PAPD_EVAL_MAX_EPS_V	  1950
#define LCN20_PAPD_SHIFT_D_PHASE	  12
#define LCN20_PAPD_SHIFT_D2_PHASE	  7

#endif /* LCN20_PAPD_ENABLE */

/* ------ definitions and tables for Radio tuning ----- */

/* AUTO-GENERATED (by gen_tune_20692, called from tunedb2tcl_20692.sh) */
static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev2_lcn20phy_19p2MHz)[] = {
	{
	1,    2412,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x096c,  0x0000,  0x0000,  0x0bc7,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5150,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	2,    2417,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0971,  0x0000,  0x0000,  0x0bcd,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	3,    2422,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0976,  0x0000,  0x0000,  0x0bd3,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	4,    2427,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x097b,  0x0000,  0x0000,  0x0bd9,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	5,    2432,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0980,  0x0000,  0x0000,  0x0be0,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	6,    2437,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0985,  0x0000,  0x0000,  0x0be6,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x098a,  0x0000,  0x0000,  0x0bec,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	8,    2447,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0bf2,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	9,    2452,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0bf9,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6009,
	0x603f
	},
	{
	10,    2457,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0bff,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6009,
	0x603f
	},
	{
	11,    2462,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0c05,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	12,    2467,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0c0b,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	13,    2472,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0c12,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	14,    2484,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x0c21,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5153,  0x6119,  0x0000,  0x6008,
	0x603f
	}
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev2_lcn20phy_26MHz)[] = {
	{
	1,    2412,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x096c,  0x0000,  0x0000,  0x08b2,  0x7627,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	2,    2417,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0971,  0x0000,  0x0000,  0x08b7,  0x13b1,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	3,    2422,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0976,  0x0000,  0x0000,  0x08bb,  0xb13b,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	4,    2427,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x097b,  0x0000,  0x0000,  0x08c0,  0x4ec4,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	5,    2432,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0980,  0x0000,  0x0000,  0x08c4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x600a,
	0x703f
	},
	{
	6,    2437,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0985,  0x0000,  0x0000,  0x08c9,  0x89d8,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098a,  0x0000,  0x0000,  0x08ce,  0x2762,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	8,    2447,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098f,  0x0000,  0x0000,  0x08d2,  0xc4ec,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	9,    2452,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0994,  0x0000,  0x0000,  0x08d7,  0x6276,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6009,
	0x603f
	},
	{
	10,    2457,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0999,  0x0000,  0x0000,  0x08dc,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6009,
	0x603f
	},
	{
	11,    2462,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x099e,  0x0000,  0x0000,  0x08e0,  0x9d89,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	12,    2467,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x08e5,  0x3b13,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	13,    2472,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x08e9,  0xd89d,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	14,    2484,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x08f4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0000,  0x6008,
	0x603f
	}
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev2_lcn20phy_37p4MHz)[] = {
	{
	1,    2412,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x096c,  0x0000,  0x0000,  0x0305,  0xe75b,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0000,  0x600a,
	0x703f
	},
	{
	2,    2417,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0971,  0x0000,  0x0000,  0x0307,  0x820d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0000,  0x600a,
	0x703f
	},
	{
	3,    2422,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0976,  0x0000,  0x0000,  0x0309,  0x1cbf,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0000,  0x600a,
	0x703f
	},
	{
	4,    2427,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x097b,  0x0000,  0x0000,  0x030a,  0xb771,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0000,  0x600a,
	0x703f
	},
	{
	5,    2432,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0980,  0x0000,  0x0000,  0x030c,  0x5223,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x600a,
	0x703f
	},
	{
	6,    2437,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0985,  0x0000,  0x0000,  0x030d,  0xecd5,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098a,  0x0000,  0x0000,  0x030f,  0x8787,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	8,    2447,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0311,  0x2239,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	9,    2452,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0312,  0xbceb,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6009,
	0x603f
	},
	{
	10,    2457,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0314,  0x579d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6009,
	0x603f
	},
	{
	11,    2462,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0315,  0xf24f,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6008,
	0x603f
	},
	{
	12,    2467,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0317,  0x8d01,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6008,
	0x603f
	},
	{
	13,    2472,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0319,  0x27b3,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0000,  0x6008,
	0x603f
	},
	{
	14,    2484,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x031d,  0x015e,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0000,  0x6008,
	0x603f
	}
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev3_lcn20phy_19p2MHz)[] = {
	{
	1,	  2412,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x096c,  0x0000,  0x0000,	0x0bc7,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5150,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	2,	  2417,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0971,  0x0000,  0x0000,	0x0bcd,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	3,	  2422,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0976,  0x0000,  0x0000,	0x0bd3,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	4,	  2427,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x097b,  0x0000,  0x0000,	0x0bd9,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	5,	  2432,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0980,  0x0000,  0x0000,	0x0be0,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	6,	  2437,  0x0026,  0x00c0,  0x000a,	0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,	0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0985,  0x0000,  0x0000,	0x0be6,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5151,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x098a,  0x0000,  0x0000,  0x0bec,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5151,  0x6119,  0x0000,  0x6008,
	0x703f
	},
	{
	8,    2447,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0bf2,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6008,
	0x703f
	},
	{
	9,    2452,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0bf9,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x5152,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	10,    2457,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0bff,  0x4000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5152,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	11,    2462,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0c05,  0x8000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5152,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	12,    2467,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0c0b,  0xc000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5152,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	13,    2472,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0c12,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5152,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	14,    2484,  0x0026,  0x00c0,  0x000a,  0x301a,  0x0080,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0013,  0x0000,  0x09b0,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x0c21,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x5153,  0x6119,  0x0000,  0x6007,
	0x603f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev3_lcn20phy_26MHz)[] = {
	{
	1,    2412,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x096c,  0x0000,  0x0000,  0x08b2,  0x7627,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513b,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	2,    2417,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0971,  0x0000,  0x0000,  0x08b7,  0x13b1,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513b,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	3,    2422,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0976,  0x0000,  0x0000,  0x08bb,  0xb13b,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	4,    2427,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x097b,  0x0000,  0x0000,  0x08c0,  0x4ec4,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	5,    2432,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0980,  0x0000,  0x0000,  0x08c4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	6,    2437,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0985,  0x0000,  0x0000,  0x08c9,  0x89d8,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513c,  0x6119,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098a,  0x0000,  0x0000,  0x08ce,  0x2762,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6008,
	0x703f
	},
	{
	8,    2447,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098f,  0x0000,  0x0000,  0x08d2,  0xc4ec,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6008,
	0x703f
	},
	{
	9,    2452,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0994,  0x0000,  0x0000,  0x08d7,  0x6276,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	10,    2457,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0999,  0x0000,  0x0000,  0x08dc,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513c,  0x6119,  0x0000,  0x6008,
	0x603f
	},
	{
	11,    2462,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x099e,  0x0000,  0x0000,  0x08e0,  0x9d89,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513d,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	12,    2467,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x08e5,  0x3b13,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513d,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	13,    2472,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x08e9,  0xd89d,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513d,  0x6119,  0x0000,  0x6007,
	0x603f
	},
	{
	14,    2484,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x08f4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,	0x513d,  0x6119,  0x0000,  0x6007,
	0x603f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev3_lcn20phy_37p4MHz)[] = {
	{
	1,    2412,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x096c,  0x0000,  0x0000,  0x0305,  0xe75b,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5120,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	2,    2417,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0971,  0x0000,  0x0000,  0x0307,  0x820d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5120,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	3,    2422,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0976,  0x0000,  0x0000,  0x0309,  0x1cbf,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5120,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	4,    2427,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x097b,  0x0000,  0x0000,  0x030a,  0xb771,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5120,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	5,    2432,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0980,  0x0000,  0x0000,  0x030c,  0x5223,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	6,    2437,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0985,  0x0000,  0x0000,  0x030d,  0xecd5,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6127,  0x0000,  0x6009,
	0x703f
	},
	{
	7,    2442,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098a,  0x0000,  0x0000,  0x030f,  0x8787,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6008,
	0x703f
	},
	{
	8,    2447,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0311,  0x2239,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6008,
	0x703f
	},
	{
	9,    2452,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0312,  0xbceb,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0000,  0x6008,
	0x603f
	},
	{
	10,    2457,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0314,  0x579d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6127,  0x0000,  0x6008,
	0x603f
	},
	{
	11,    2462,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0315,  0xf24f,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6127,  0x0000,  0x6007,
	0x603f
	},
	{
	12,    2467,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0317,  0x8d01,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6127,  0x0000,  0x6007,
	0x603f
	},
	{
	13,    2472,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0319,  0x27b3,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6128,  0x0000,  0x6007,
	0x603f
	},
	{
	14,    2484,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x031d,  0x015e,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,	0x5121,  0x6128,  0x0000,  0x6007,
	0x603f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev4_lcn20phy_37p4MHz)[] = {
	{
	1,    2412,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x096c,  0x0000,  0x0000,  0x0305,  0xe75b,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600e,
	0x403f
	},
	{
	2,    2417,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0971,  0x0000,  0x0000,  0x0307,  0x820d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x403f
	},
	{
	3,    2422,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0976,  0x0000,  0x0000,  0x0309,  0x1cbf,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	4,    2427,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x097b,  0x0000,  0x0000,  0x030a,  0xb771,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	5,    2432,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0980,  0x0000,  0x0000,  0x030c,  0x5223,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	6,    2437,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0985,  0x0000,  0x0000,  0x030d,  0xecd5,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	7,    2442,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098a,  0x0000,  0x0000,  0x030f,  0x8787,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	8,    2447,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0311,  0x2239,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	9,    2452,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0312,  0xbceb,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	10,    2457,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0314,  0x579d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x203f
	},
	{
	11,    2462,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0315,  0xf24f,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0001,  0x600c,
	0x203f
	},
	{
	12,    2467,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0317,  0x8d01,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0001,  0x600c,
	0x103f
	},
	{
	13,    2472,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0319,  0x27b3,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0001,  0x600a,
	0x103f
	},
	{
	14,    2484,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x031d,  0x015e,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0001,  0x600a,
	0x103f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev4_lcn20phy_26MHz)[] = {
	{
	1,    2412,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x096c,  0x0000,  0x0000,  0x08b2,  0x7627,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0003,  0x600e,
	0x403f
	},
	{
	2,    2417,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0971,  0x0000,  0x0000,  0x08b7,  0x13b1,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0003,  0x600c,
	0x403f
	},
	{
	3,    2422,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0976,  0x0000,  0x0000,  0x08bb,  0xb13b,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	4,    2427,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x097b,  0x0000,  0x0000,  0x08c0,  0x4ec4,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	5,    2432,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0980,  0x0000,  0x0000,  0x08c4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	6,    2437,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0985,  0x0000,  0x0000,  0x08c9,  0x89d8,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	7,    2442,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098a,  0x0000,  0x0000,  0x08ce,  0x2762,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	8,    2447,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098f,  0x0000,  0x0000,  0x08d2,  0xc4ec,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	9,    2452,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0994,  0x0000,  0x0000,  0x08d7,  0x6276,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	10,    2457,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0999,  0x0000,  0x0000,  0x08dc,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x203f
	},
	{
	11,    2462,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x099e,  0x0000,  0x0000,  0x08e0,  0x9d89,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600c,
	0x203f
	},
	{
	12,    2467,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x08e5,  0x3b13,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600c,
	0x103f
	},
	{
	13,    2472,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x08e9,  0xd89d,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600a,
	0x103f
	},
	{
	14,    2484,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x08f4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600a,
	0x103f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev5_lcn20phy_37p4MHz)[] = {
	{
	1,    2412,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x096c,  0x0000,  0x0000,  0x0305,  0xe75b,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600e,
	0x403f
	},
	{
	2,    2417,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0971,  0x0000,  0x0000,  0x0307,  0x820d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x403f
	},
	{
	3,    2422,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0976,  0x0000,  0x0000,  0x0309,  0x1cbf,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	4,    2427,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x097b,  0x0000,  0x0000,  0x030a,  0xb771,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5120,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	5,    2432,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0980,  0x0000,  0x0000,  0x030c,  0x5223,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0003,  0x600c,
	0x303f
	},
	{
	6,    2437,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0985,  0x0000,  0x0000,  0x030d,  0xecd5,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	7,    2442,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098a,  0x0000,  0x0000,  0x030f,  0x8787,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	8,    2447,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x098f,  0x0000,  0x0000,  0x0311,  0x2239,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	9,    2452,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0994,  0x0000,  0x0000,  0x0312,  0xbceb,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x303f
	},
	{
	10,    2457,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x0999,  0x0000,  0x0000,  0x0314,  0x579d,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0002,  0x600c,
	0x203f
	},
	{
	11,    2462,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x099e,  0x0000,  0x0000,  0x0315,  0xf24f,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0001,  0x600c,
	0x203f
	},
	{
	12,    2467,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x0317,  0x8d01,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6127,  0x0001,  0x600c,
	0x103f
	},
	{
	13,    2472,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x0319,  0x27b3,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0001,  0x600a,
	0x103f
	},
	{
	14,    2484,  0x004b,  0x0176,  0x0013,  0x3032,  0x0016,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x0025,  0x0000,  0x0991,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x031d,  0x015e,  0x0008,  0x583f,
	0x1212,  0x142f,  0x1f1f,  0x1f1f,  0x5121,  0x6128,  0x0001,  0x600a,
	0x103f
	},
};

static const chan_info_20692_lcn20phy_t BCMATTACHDATA(chan_info_20692_rev5_lcn20phy_26MHz)[] = {
	{
	1,    2412,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x096c,  0x0000,  0x0000,  0x08b2,  0x7627,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0003,  0x600e,
	0x403f
	},
	{
	2,    2417,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0971,  0x0000,  0x0000,  0x08b7,  0x13b1,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513b,  0x6119,  0x0003,  0x600c,
	0x403f
	},
	{
	3,    2422,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0976,  0x0000,  0x0000,  0x08bb,  0xb13b,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	4,    2427,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x097b,  0x0000,  0x0000,  0x08c0,  0x4ec4,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	5,    2432,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0980,  0x0000,  0x0000,  0x08c4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0003,  0x600c,
	0x303f
	},
	{
	6,    2437,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0985,  0x0000,  0x0000,  0x08c9,  0x89d8,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	7,    2442,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098a,  0x0000,  0x0000,  0x08ce,  0x2762,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	8,    2447,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x098f,  0x0000,  0x0000,  0x08d2,  0xc4ec,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	9,    2452,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0994,  0x0000,  0x0000,  0x08d7,  0x6276,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x303f
	},
	{
	10,    2457,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x0999,  0x0000,  0x0000,  0x08dc,  0x0000,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513c,  0x6119,  0x0002,  0x600c,
	0x203f
	},
	{
	11,    2462,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x099e,  0x0000,  0x0000,  0x08e0,  0x9d89,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600c,
	0x203f
	},
	{
	12,    2467,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a3,  0x0000,  0x0000,  0x08e5,  0x3b13,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600c,
	0x103f
	},
	{
	13,    2472,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09a8,  0x0000,  0x0000,  0x08e9,  0xd89d,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600a,
	0x103f
	},
	{
	14,    2484,  0x0034,  0x0104,  0x000d,  0x3023,  0x004f,  0x0003,
	0x7e41,  0x0600,  0x0a00,  0x03d9,  0x0027,  0x001a,  0x0000,  0x09a2,
	0x0000,  0x09b4,  0x0000,  0x0000,  0x08f4,  0xec4e,  0x0008,  0x583f,
	0x1717,  0x193c,  0x1f1f,  0x1f1f,  0x513d,  0x6119,  0x0001,  0x600a,
	0x103f
	},
};

/* TIA LUT tables */
#define TIA_LUT_20692_LEN 108 /* 0-108, Note 67 doesn't exist */

static const uint8 tiaRC_20692_8b_20[]= { /* LUT 0--51 (20 MHz) */
	0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff,
	0xb7, 0xb5, 0x97, 0x60, 0xe5, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x14, 0x1d, 0x28, 0x34, 0x34, 0x34,
	0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40,
	0x5b, 0x6c, 0x00, 0x80
};

static const uint16 tiaRC_20692_16b_20[]= { /* LUT 52--108 (20 MHz) */
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x00b5, 0x0080, 0x005a,
	0x0040, 0x002d, 0x0020, 0x0000, 0x0000, 0x0100, 0x00b5, 0x0080,
	0x005b, 0x0040, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0006, 0x0080, 0x081f, 0x17e0, 0x7fff, 0x0001, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

#define LCN20_RSSI_INVALID_TEMP	255
#define LCN20_QDB_SHIFT	2
/* Vbat/Tempsense Radio REgister addresses for save/restore */
static const uint16 temp_vbat_radioreg_savrest[] = {
		RF0_20692_WL_TEMP_SENS_OVR1(rev),
		RF0_20692_WL_TEMPSENSE_CFG(rev),
		RF0_20692_WL_GPABUF_OVR1(rev),
		RF0_20692_WL_TESTBUF_CFG1(rev),
		RF0_20692_WL_AUX_RXPGA_OVR1(rev),
		RF0_20692_WL_AUXPGA_CFG1(rev),
		RF0_20692_WL_AUXPGA_VMID(rev),
		RF0_20692_WL_RX_ADC_OVR1(rev),
		RF0_20692_WL_ADC_CFG10(rev),
		RF0_20692_WL_TX_DAC_CFG5(rev),
		RF0_20692_WL_RX_BB_OVR1(rev),
		RF0_20692_WL_TIA_CFG7(rev),
		RF0_20692_WL_TIA_CFG8(rev),
		RF0_20692_WL_MINIPMU_OVR1(rev),
		RF0_20692_WL_VBAT_MONITOR_OVR1(rev),
		RF0_20692_WL_TIA_CFG5(rev),
		RF0_20692_WL_TIA_CFG6(rev),
		RF0_20692_WL_TIA_CFG1(rev),
		RF0_20692_WL_PMU_OP(rev),
		RF0_20692_WL_VBAT_CFG(rev)
		};
#define NUM_RADIOREG_SAVREST ARRAYSIZE(temp_vbat_radioreg_savrest)

/* ------- PHY register read/write/modfiy macros ----- */
/* radio-specific macros */
#define RADIO_REG_2069X(pi, id, regnm, core)	RF##core##_##id##_##regnm(pi->pubpi->radiorev)
#define RADIO_REG_20691(pi, regnm, core)	RADIO_REG_2069X(pi, 20691, regnm, 0)
#define RADIO_REG_20692(pi, regnm, core)	RADIO_REG_2069X(pi, 20692, regnm, 0)
#define RADIO_REG_20693(pi, regnm, core)	RADIO_REG_2069X(pi, 20693, regnm, 0)

#define _READ_RADIO_REG(pi, reg)		phy_utils_read_radioreg(pi, reg)

#define READ_RADIO_REG(pi, regpfx, regnm) \
	_READ_RADIO_REG(pi, regpfx##_2069_##regnm)

#define READ_RADIO_REG_20692(pi, regnm, core) \
	_READ_RADIO_REG(pi, RADIO_REG_20692(pi, regnm, core))

#define READ_RADIO_REGFLD(pi, regpfx, regnm, fldname) \
	((_READ_RADIO_REG(pi, regpfx##_2069_##regnm) & \
		RF_2069_##regnm##_##fldname##_MASK) \
		>> RF_2069_##regnm##_##fldname##_SHIFT)

#define READ_RADIO_REGFLD_20692(pi, regnm, core, fldname) \
	((_READ_RADIO_REG(pi, RADIO_REG_20692(pi, regnm, core)) & \
		RF_20692_##regnm##_##fldname##_MASK(pi->pubpi->radiorev)) \
		>> RF_20692_##regnm##_##fldname##_SHIFT(pi->pubpi->radiorev))

#define RADIO_REG(pi, regnm, core)	\
	((RADIOID((pi)->pubpi->radioid) == BCM20691_ID) \
		? RADIO_REG_20691(pi, regnm, core) : \
	 (RADIOID((pi)->pubpi->radioid) == BCM20692_ID) \
		? RADIO_REG_20692(pi, regnm, core) : \
	 (RADIOID((pi)->pubpi->radioid) == BCM20693_ID) \
		? RADIO_REG_20693(pi, regnm, core) : INVALID_ADDRESS)

#define _WRITE_RADIO_REG(pi, reg, val)		phy_utils_write_radioreg(pi, reg, val)

#define WRITE_RADIO_REG_20692(pi, regnm, core, val) \
	_WRITE_RADIO_REG(pi, RADIO_REG_20692(pi, regnm, core), val)

#define _MOD_RADIO_REG(pi, reg, mask, val)	phy_utils_mod_radioreg(pi, reg, mask, val)

#define MOD_RADIO_REG(pi, regpfx, regnm, fldname, value) \
	_MOD_RADIO_REG(pi, \
		regpfx##_2069_##regnm, \
		RF_2069_##regnm##_##fldname##_MASK, \
		((value) << RF_2069_##regnm##_##fldname##_SHIFT))

#define MOD_RADIO_REG_2069X(pi, id, regnm, core, fldname, value) \
		_MOD_RADIO_REG(pi, \
			RADIO_REG_##id(pi, regnm, core), \
			RF_##id##_##regnm##_##fldname##_MASK(pi->pubpi->radiorev), \
			((value) << RF_##id##_##regnm##_##fldname##_SHIFT(pi->pubpi->radiorev)))

#define MOD_RADIO_REG_20692(pi, regnm, core, fldname, value) \
		MOD_RADIO_REG_2069X(pi, 20692, regnm, core, fldname, value)

/* phyreg-specific macros */
#define _PHY_REG_READ(pi, reg)			phy_utils_read_phyreg(pi, reg)

#define READ_LCN20PHYREG(pi, reg) \
	_PHY_REG_READ(pi, LCN20PHY_##reg)

#define READ_LCN20PHYREGFLD(pi, reg, field)				\
	((READ_LCN20PHYREG(pi, reg)					\
	 & LCN20PHY_##reg##_##field##_##MASK) >>	\
	 LCN20PHY_##reg##_##field##_##SHIFT)

#define _PHY_REG_MOD(pi, reg, mask, val)	phy_utils_mod_phyreg(pi, reg, mask, val)

#define MOD_LCN20PHYREG(pi, reg, field, value)				\
	_PHY_REG_MOD(pi, LCN20PHY_##reg,		\
		LCN20PHY_##reg##_##field##_MASK,	\
		((value) << LCN20PHY_##reg##_##field##_##SHIFT))

#define _PHY_REG_WRITE(pi, reg, val)		phy_utils_write_phyreg(pi, reg, val)

#define WRITE_LCN20PHYREG(pi, reg, val) \
	_PHY_REG_WRITE(pi, LCN20PHY_##reg, val)

/* Table read/write macros */
#define wlc_lcn20phy_common_read_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_read_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn20phy_read_table)

#define wlc_lcn20phy_common_write_table(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset) \
	phy_utils_write_common_phytable(pi, tbl_id, tbl_ptr, tbl_len, tbl_width, tbl_offset, \
	wlc_lcn20phy_write_table)

/* Power control macros */
#define LCN20PHY_DACGAIN_MASK	\
	(0xf << 7)
#define LCN20PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT \
	(LCN20PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT + 8)
#define LCN20PHY_txgainctrlovrval1_pagain_ovr_val1_MASK \
	(0xff << LCN20PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT)

#define wlc_lcn20phy_set_start_tx_pwr_idx(pi, idx) \
	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlCmd, \
		LCN20PHY_TxPwrCtrlCmd_pwrIndex_init_MASK, \
		(uint16)(idx*2) << LCN20PHY_TxPwrCtrlCmd_pwrIndex_init_SHIFT)

#define wlc_lcn20phy_set_start_CCK_tx_pwr_idx(pi, idx) \
		phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlCmdCCK, \
			LCN20PHY_TxPwrCtrlCmdCCK_pwrIndex_init_cck_MASK, \
			(uint16)(idx*2) << LCN20PHY_TxPwrCtrlCmdCCK_pwrIndex_init_cck_SHIFT)

#define wlc_lcn20phy_set_tx_pwr_npt(pi, npt) \
	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlNnum, \
		LCN20PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK, \
		(uint16)(npt) << LCN20PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

#define wlc_lcn20phy_get_tx_pwr_npt(pi) \
	((phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlNnum) & \
		LCN20PHY_TxPwrCtrlNnum_Npt_intg_log2_MASK) >> \
		LCN20PHY_TxPwrCtrlNnum_Npt_intg_log2_SHIFT)

/* the bitsize of the register is 9 bits for lcn20phy */

#define wlc_lcn20phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi) \
	(phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusExt) & 0x1ff)

#define LCN20PHY_DISABLE_STALL(pi)	\
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 1)

#define LCN20PHY_ENABLE_STALL(pi, stall_val) \
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, stall_val)

void wlc_lcn20phy_radio20692_rc_cal_lpf(phy_info_t *pi, uint8 cal, uint32 dn);
void wlc_lcn20phy_radio20692_rc_cal_adc(phy_info_t *pi, uint32 dn);
void wlc_lcn20phy_radio20692_rc_cal_dacbuf(phy_info_t *pi);
void wlc_lcn20phy_tssi_setup_estPwrLuts(phy_info_t *pi, phytbl_info_t *tab);
void wlc_lcn20phy_tssi_setup_cfg_phy(phy_info_t *pi);
void wlc_lcn20phy_tssi_setup_cfg_radio(phy_info_t *pi);
void wlc_phy_radio20692_tx_iqlo_cal_radio_setup_save_state(phy_info_t *pi);
void wlc_lcn20phy_tx_iqlo_cal_setup(phy_info_t *pi,
	phy_txiqlocal_mode_t cal_mode, uint16 *bbmult_for_lad);
#ifdef PAPD_DEBUG
void wlc_phy_dump_epsilon_lcn20(phy_info_t *pi, uint32	*eps_table);
#endif // endif
void wlc_phy_papd_loopback_radio20692_setup_save_lcn20(phy_info_t *pi);

uint16 wlc_lcn20phy_get_rxiqcal_tonefreqkhz_0(void);
uint16 wlc_lcn20phy_get_rxiqcal_tonefreqkhz_1(void);
uint16 wlc_lcn20phy_get_rxiqcal_toneamp(void);
uint16 wlc_lcn20phy_get_rxiqcal_numsamps(void);

uint32 wlc_lcn20phy_get_spinwait_iqest_qt_usec(void);
uint32 wlc_lcn20phy_get_spinwait_iqest_usec(void);
uint32 wlc_lcn20phy_get_txiqlocal_tonefhz(void);
uint16 wlc_lcn20phy_get_txiqlocal_toneamp(void);
uint32 wlc_lcn20phy_get_spinwait_txiqlocal_usec(void);

typedef struct measure_rxspur_savestate {
	uint16 SAVE_agc_fsm_en;
	uint16 SAVE_txpwrctrl;
	uint16 SAVE_iqloCalCmd;
	uint16 SAVE_iqloCalCmdNnum;
	uint16 SAVE_txpwrctrlcmd;
	uint16 SAVE_wl_tx2g_cfg1;
	uint16 SAVE_wl_iqcal_cfg1;
	uint16 SAVE_wl_testbuf_cfg1;
	uint16 SAVE_wl_auxpga_cfg1;
	uint16 SAVE_wl_tx_top_2g_ovr2;
	uint16 SAVE_wl_tssi_iqcal_ovr1;
	uint16 SAVE_wl_GPABuf_ovr1;
	uint16 SAVE_wl_AUX_RXPGA_ovr1;
} measure_rxspur_savestate_params_t;

void wlc_lcn20phy_measure_rxspur_save_state(phy_info_t *pi,
	measure_rxspur_savestate_params_t *save_state);
void wlc_lcn20phy_measure_rxspur_restore_state(phy_info_t *pi,
	measure_rxspur_savestate_params_t *save_state);

typedef struct rxspur_params {
	int16 freqKHz;
	int16 delta_dB;
	int16 spurlvl;
} rxspur_params_t;

void wlc_lcn20phy_dssf_cal_est_noise_chk_spur(phy_info_t *pi, const int16 *spur_offset,
	int16 *delta_dB, bool *enableDSSF, rxspur_params_t *spur_list, uint16 *spurListSize);

void wlc_lcn20phy_tx_iqlo_cal_execute(phy_info_t *pi, uint16 bbmult_for_lad,
	uint8 n_cal_cmds, uint16 *cal_cmds, uint16 *coeffs);
void wlc_lcn20phy_tx_iqlo_cal_cleanup(phy_info_t *pi);
void wlc_lcn20phy_tx_iqlo_cal_process_results(phy_info_t *pi,
	uint16 *coeffs, phy_txiqlocal_mode_t cal_mode);

#ifdef WL_PROXDETECT
static void wlc_phy_nvram_proxd_read(phy_info_t *pi);
#endif /* WL_PROXDETECT */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */
/*  Function Implementation */
/* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% */

/* ATTACH */
bool
BCMATTACHFN(wlc_phy_attach_lcn20phy)(phy_info_t *pi)
{
	uint16 otpdate = 0;
	uint16 rcal = 0;

	phy_info_lcn20phy_t *pi_lcn20;

	pi->u.pi_lcn20phy = (phy_info_lcn20phy_t*)MALLOCZ(pi->sh->osh, sizeof(phy_info_lcn20phy_t));
	if (pi->u.pi_lcn20phy == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20 = pi->u.pi_lcn20phy;
#if defined(WL_PROXDETECT)
	wlc_phy_nvram_proxd_read(pi);
#endif /* WL_PROXDETECT */
	pi_lcn20->calbuffer = (uint8*)MALLOCZ(pi->sh->osh, LCN20PHY_CALBUFFER_MAX_SZ);
	if (pi_lcn20->calbuffer == NULL) {
		PHY_ERROR(("wl%d: %s: cal buffer MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->rate_table = (uint32*)MALLOCZ(pi->sh->osh, sizeof(uint32) * WL_RATESET_SZ);
	if (pi_lcn20->rate_table == NULL) {
		PHY_ERROR(("wl%d: %s: rate table MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->rxiqcal_phyregs = (lcn20phy_rxiqcal_phyregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_rxiqcal_phyregs_t));
	if (pi_lcn20->rxiqcal_phyregs == NULL) {
		PHY_ERROR(("wl%d: %s: rxiq phyregs MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->rxiqcal_radioregs = (lcn20phy_rxiqcal_radioregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_rxiqcal_radioregs_t));
	if (pi_lcn20->rxiqcal_radioregs == NULL) {
		PHY_ERROR(("wl%d: %s: rxiq radioreg MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->papd_radioregs = (lcn20phy_papd_radioregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_papd_radioregs_t));
	if (pi_lcn20->papd_radioregs == NULL) {
		PHY_ERROR(("wl%d: %s: papd radioreg MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->papd_phyregs = (lcn20phy_papd_phyregs_struct_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_papd_phyregs_struct_t));
	if (pi_lcn20->papd_phyregs == NULL) {
		PHY_ERROR(("wl%d: %s: papd phyregs MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->txiqlocal_phyregs = (lcn20phy_txiqlocal_phyregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_txiqlocal_phyregs_t));
	if (pi_lcn20->txiqlocal_phyregs == NULL) {
		PHY_ERROR(("wl%d: %s: txiq phyregs MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->txiqcal_radioregs = (lcn20phy_txiqcal_radioregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_txiqcal_radioregs_t));
	if (pi_lcn20->txiqcal_radioregs == NULL) {
		PHY_ERROR(("wl%d: %s: txiq radregs MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->tempsense_vbat_radioregs =
			(lcn20phy_tempsense_vbat_radioregs_t*)MALLOCZ(pi->sh->osh,
			sizeof(lcn20phy_tempsense_vbat_radioregs_t));
	if (pi_lcn20->tempsense_vbat_radioregs == NULL) {
		PHY_ERROR(("wl%d: %s: temp rad regs MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi_lcn20->rssi_corr_gain_delta_2g_sub =
			(int8*)MALLOCZ(pi->sh->osh,
			sizeof(pi_lcn20->rssi_corr_gain_delta_2g_sub[0])*
			LCN20PHY_GROUPS * LCN20PHY_GAIN_DELTA_2G_PARAMS);
	if (pi_lcn20->rssi_corr_gain_delta_2g_sub == NULL) {
		PHY_ERROR(("wl%d: %s: rssi corr tbl MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

#if defined(PHYCAL_CACHING)
	/* Reset the var as no cal cache context should exist yet */
	pi->phy_calcache_num = 0;
#endif // endif

	if (!NORADIO_ENAB(pi->pubpi)) {
		pi->hwpwrctrl = TRUE;
		pi->hwpwrctrl_capable = TRUE;
	}

	if (!NORADIO_ENAB(pi->pubpi)) {
		/* Get xtal frequency from PMU */
		pi->xtalfreq = si_alp_clock(pi->sh->sih);
		ASSERT((pi->xtalfreq % 1000) == 0);
	} else {
		pi->xtalfreq = XTAL_FREQ_37P4MHZ;
	}

	PHY_INFORM(("wl%d: %s: using %d.%d MHz xtalfreq for RF PLL\n",
		pi->sh->unit, __FUNCTION__,
		pi->xtalfreq / 1000000, pi->xtalfreq % 1000000));

#if defined(BCMDBG) || defined(WLTEST)
	pi->pi_fptr->longtrn = wlc_phy_long_train_lcn20phy;
#endif // endif

	/* register watchdog fn */
	if (phy_wd_add_fn(pi->wdi, wlc_phy_watchdog_lcn20phy, pi,
		PHY_WD_PRD_1TICK, PHY_WD_GLACIAL_CAL, PHY_WD_FLAG_DEF_DEFER) != BCME_OK) {
		PHY_ERROR(("%s: phy_wd_add_fn failed\n", __FUNCTION__));
		return FALSE;
	}

#if defined(WLTEST)
	pi->pi_fptr->carrsuppr = wlc_phy_carrier_suppress_lcn20phy;
#endif // endif
	pi->pi_fptr->calibmodes = wlc_lcn20phy_calib_modes;

	if (LCN20REV_IS(pi->pubpi->phy_rev, 1) && (RADIOREV(pi->pubpi->radiorev) == 5))
		pi->u.pi_lcn20phy->trsw_ctrl_etr = TRUE;	/* eTR */
	else
		pi->u.pi_lcn20phy->trsw_ctrl_etr = FALSE;	/* iTR */

	/* Use OTP RCAL value, if the chip has OTP date programed, otherwise use default values */
	/* Read OTP wafer Y number, OTP month and date at offset LCN20PHY_OTP_DATE_OFFSET */
	otp_read_word(pi->sh->sih, LCN20PHY_OTP_DATE_OFFSET, &otpdate);
	if (otpdate) {
		/* Read OTP RCAL value (bits 0:3) at LCN20PHY_OTP_RCAL_OFFSET */
		otp_read_word(pi->sh->sih, LCN20PHY_OTP_RCAL_OFFSET, &rcal);
		rcal &= 0xf;
	} else {
		if (RADIOREV(pi->pubpi->radiorev) == 4 || RADIOREV(pi->pubpi->radiorev) == 5)
			rcal = 0x9;
		else
			rcal = 0x7;
	}
	pi->u.pi_lcn20phy->rcal = (uint8)rcal;

	/* be aware that wlc_lcn20phy_txpwr_srom_read reads nvram parameters that
	 * subsequent functions will use. Please keep this near top.
	 */
	if (!wlc_lcn20phy_txpwr_srom_read(pi))
		return FALSE;

	if (!wlc_lcn20phy_tuningtbl_select(pi))
		return FALSE;

	if (!wlc_lcn20phy_rx_gain_tbl_select(pi))
		return FALSE;

	if (!wlc_lcn20phy_tx_gain_tbl_select(pi))
		return FALSE;

	if (!wlc_lcn20phy_lna_params_select(pi))
		return FALSE;

#ifdef LCN20_PAPD_ENABLE
	wlc_lcn20phy_papd_init(pi);
#endif /* LCN20_PAPD_ENABLE */

	pi->u.pi_lcn20phy->lcn20_rxldpc_override = ON; /* default ldpc value for lcn20 */
	return TRUE;
}
void
BCMATTACHFN(wlc_phy_detach_lcn20phy)(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	wlc_lcn20phy_tx_gain_tbl_free(pi);
	MFREE(pi->sh->osh, pi_lcn20->lna_params, sizeof(*pi_lcn20->lna_params));
	MFREE(pi->sh->osh, pi_lcn20->gainvaltbl, sizeof(*pi_lcn20->gainvaltbl) *
		LCN20PHY_RXGAINVAL_TBL_SZ);
	MFREE(pi->sh->osh, pi_lcn20->gaintbl, sizeof(*pi_lcn20->gaintbl) * LCN20PHY_RXGAIN_TBL_SZ);
	MFREE(pi->sh->osh, pi_lcn20->gainidxtbl, sizeof(*pi_lcn20->gainidxtbl) *
		LCN20PHY_RXGAINIDX_TBL_SZ);
	MFREE(pi->sh->osh, pi_lcn20->tuningtbl, (sizeof(chan_info_20692_lcn20phy_t) * 14));
	MFREE(pi->sh->osh, pi_lcn20->rssi_corr_gain_delta_2g_sub,
		(sizeof(pi_lcn20->rssi_corr_gain_delta_2g_sub[0])*
		LCN20PHY_GROUPS * LCN20PHY_GAIN_DELTA_2G_PARAMS));
	MFREE(pi->sh->osh, pi_lcn20->tempsense_vbat_radioregs,
			sizeof(lcn20phy_tempsense_vbat_radioregs_t));
	MFREE(pi->sh->osh, pi_lcn20->txiqcal_radioregs,	sizeof(lcn20phy_txiqcal_radioregs_t));
	MFREE(pi->sh->osh, pi_lcn20->txiqlocal_phyregs,	sizeof(lcn20phy_txiqlocal_phyregs_t));
	MFREE(pi->sh->osh, pi_lcn20->papd_phyregs, sizeof(lcn20phy_papd_phyregs_struct_t));
	MFREE(pi->sh->osh, pi_lcn20->papd_radioregs, sizeof(lcn20phy_papd_radioregs_t));
	MFREE(pi->sh->osh, pi_lcn20->rxiqcal_radioregs, sizeof(lcn20phy_rxiqcal_radioregs_t));
	MFREE(pi->sh->osh, pi_lcn20->rxiqcal_phyregs, sizeof(lcn20phy_rxiqcal_phyregs_t));
	MFREE(pi->sh->osh, pi_lcn20->rate_table, sizeof(uint32) * WL_RATESET_SZ);
	MFREE(pi->sh->osh, pi_lcn20->calbuffer, LCN20PHY_CALBUFFER_MAX_SZ);
	MFREE(pi->sh->osh, pi_lcn20, sizeof(phy_info_lcn20phy_t));
}

static bool
BCMATTACHFN(wlc_lcn20phy_tuningtbl_select)(phy_info_t *pi)
{
	const chan_info_20692_lcn20phy_t *chi = NULL;

	pi->u.pi_lcn20phy->tuningtbl =
		(chan_info_20692_lcn20phy_t*)MALLOC(pi->sh->osh,
		sizeof(chan_info_20692_lcn20phy_t) * 14);

	if (pi->u.pi_lcn20phy->tuningtbl == NULL)
		return FALSE;

	if (RADIOREV(pi->pubpi->radiorev) < 3) {
		if (pi->xtalfreq == XTAL_FREQ_37P4MHZ)
			chi = chan_info_20692_rev2_lcn20phy_37p4MHz;
		else if (pi->xtalfreq == XTAL_FREQ_26P0MHZ)
			chi = chan_info_20692_rev2_lcn20phy_26MHz;
		else if (pi->xtalfreq == XTAL_FREQ_19P2MHZ)
			chi = chan_info_20692_rev2_lcn20phy_19p2MHz;
	} else if (RADIOREV(pi->pubpi->radiorev) == 3) {
		if (pi->xtalfreq == XTAL_FREQ_37P4MHZ)
			chi = chan_info_20692_rev3_lcn20phy_37p4MHz;
		else if (pi->xtalfreq == XTAL_FREQ_26P0MHZ)
			chi = chan_info_20692_rev3_lcn20phy_26MHz;
		else if (pi->xtalfreq == XTAL_FREQ_19P2MHZ)
			chi = chan_info_20692_rev3_lcn20phy_19p2MHz;
	} else if (RADIOREV(pi->pubpi->radiorev) == 4) {
		if (pi->xtalfreq == XTAL_FREQ_37P4MHZ)
			chi = chan_info_20692_rev4_lcn20phy_37p4MHz;
		else if (pi->xtalfreq == XTAL_FREQ_26P0MHZ)
			chi = chan_info_20692_rev4_lcn20phy_26MHz;
	} else if (RADIOREV(pi->pubpi->radiorev) == 5) {
		if (pi->xtalfreq == XTAL_FREQ_37P4MHZ)
			chi = chan_info_20692_rev5_lcn20phy_37p4MHz;
		else if (pi->xtalfreq == XTAL_FREQ_26P0MHZ)
			chi = chan_info_20692_rev5_lcn20phy_26MHz;
	}

	if (chi) {
		memcpy(pi->u.pi_lcn20phy->tuningtbl, chi,
			(sizeof(chan_info_20692_lcn20phy_t) * 14));
	} else {
		PHY_ERROR(("%s: Unsupported xtal freq %d!\n", __FUNCTION__, pi->xtalfreq));
		return FALSE;
	}

	return TRUE;
}

void *
wlc_lcn20_malloc(phy_info_lcn20phy_t *pi_lcn20phy, uint16 size, uint32 line)
{
	uint8 *ret_ptr = NULL;
	if (pi_lcn20phy->calbuffer_inuse) {
		PHY_ERROR(("FATAL Error: Concurrent LCN20PHY memory allocation @ line %d\n", line));
		ASSERT(FALSE);
	} else if (size > LCN20PHY_CALBUFFER_MAX_SZ) {
		PHY_ERROR(("FATAL Error: Buffer size (%d) required > MAX @ line %d\n", size, line));
		ASSERT(FALSE);
	} else {
		/* printf("Allocation @ line %d ...", line); */
		pi_lcn20phy->calbuffer_inuse = TRUE;
		ret_ptr = pi_lcn20phy->calbuffer;
	}
	return ret_ptr;
}

#ifdef LCN20_PAPD_ENABLE
static const char BCMATTACHDATA(rstr_papdepsoffset)[] = "papdepsoffset";
static const char BCMATTACHDATA(rstr_papd_valid_retest_num)[] = "papdvalidtest";
static const char BCMATTACHDATA(rstr_papd_cal_idx)[] = "pacalidx2g";
static void
BCMATTACHFN(wlc_lcn20phy_papd_init)(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	pi_lcn20->papd_valid_retest_num = PHY_GETINTVAR_DEFAULT(pi,
		rstr_papd_valid_retest_num, -1);
	pi_lcn20->papd_cal_idx = PHY_GETINTVAR_DEFAULT(pi,
		rstr_papd_cal_idx, 23);
	pi_lcn20->papd_cal_res_valid = FALSE;

#if defined(WLTEST) || defined(DBG_PHY_IOV) || defined(BCMDBG)
			/* read PAPD parameters from NVRAM */
	if (pi_lcn20->epsdelta2g_flag != LCN20PHY_PAPD_OFFSET_SRC_IOVAR) {
		pi_lcn20->epsdelta2g = (int16) PHY_GETINTVAR_DEFAULT(pi,
			rstr_papdepsoffset, PAPD_EPSILON_OFFSET_DEFAULT);
		pi_lcn20->epsdelta2g_flag =
			(pi_lcn20->epsdelta2g == PAPD_EPSILON_OFFSET_DEFAULT) ?
			LCN20PHY_PAPD_OFFSET_SRC_DEFAULT : LCN20PHY_PAPD_OFFSET_SRC_NVRAM;
	}
	PHY_PAPD(("Defined epsilon offset = %d from source ID %d \n\n",
		pi_lcn20->epsdelta2g, pi_lcn20->epsdelta2g_flag));
#else /* defined(BCMINTERNAL) || defined(WLTEST) || defined(DBG_PHY_IOV) ||  defined(BCMDBG) */
		/* Default assignments for PAPD parameters */
	pi_lcn20->epsdelta2g = (int16) PHY_GETINTVAR_DEFAULT(pi,
		rstr_papdepsoffset, PAPD_EPSILON_OFFSET_DEFAULT);
#endif // endif
}
#endif /* LCN20_PAPD_ENABLE */

void
wlc_lcn20_mfree(phy_info_lcn20phy_t *pi_lcn20phy, uint32 line)
{
	if (!pi_lcn20phy->calbuffer_inuse) {
		PHY_ERROR(("FATAL Error: MFree called but no prev alloc @ line %d\n", line));
		ASSERT(FALSE);
	} else {
		/* printf("Deallocation @ line %d\n", line); */
		pi_lcn20phy->calbuffer_inuse = FALSE;
	}
}

/* Below functions are used for RF PU and PD */
#ifdef LCN20PHY_DEBUG
void int
wlc_lcn20_rfpu_war(phy_info_t *pi, int32 int_val)
{
	uint32 blk = int_val >> 4;
	uint8 en = int_val & 1;

	PHY_INFORM(("\n value passed for pu is : 0x%x", int_val));
	if (blk == 5) {
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG1, 0, wl_lpf_pu, en);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_pu_bq_i, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG1, 0, wl_lpf_pu_bq_i, en);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_pu_bq_q, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG1, 0, wl_lpf_pu_bq_q, en);
	} else if (blk == 0xf) { /* full pwr dound tx */
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG2, 0, wl_lpf_sel_5g_out_gm, 0);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG3, 0, wl_lpf_sel_2g_5g_cmref_gm, 0);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_tx2g_bias_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_tx2g_bias_pu, en);
	} else if (blk == 4) {
	/* Gm/Mixer/LOgen */
		MOD_RADIO_REG_20692(pi, WL_LOGEN_OVR1, 0, wl_ovr_logencore_en_2g, 1);
		MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG1, 0, wl_logencore_en_2g, en);
	/* LOGEN LDO needs to be in high-power mode when LOGEN on */
		MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_VCOldo_hpm, en);
	} else if (blk == 2) {
		MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG4, 0, wl_mx2g_bbpdI_en, 0);
		MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG4, 0, wl_mx2g_bbpdQ_en, 0);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_mx2g_bias_en, 1);
		MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG2, 0, wl_mx2g_bias_en, en);
	} else if (blk == 3) {
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu, en);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu_bias, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu_bias, en);
	} else if (blk == 1) {
	/* PA */
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_pa2g_2gtx_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_2gtx_pu, en);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_pa2g_bias_cas_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_bias_cas_pu, en);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_pa2g_bias_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_bias_pu, en);
	} else if (blk == 0xe) { /* tr sw */
	/* MISC */
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_pu, en);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_bias_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_bias_pu, en);
	} else if (blk == 8) {
		MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_pu, 0);
		MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_pa2g_tssi_ctrl_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_pa2g_tssi_ctrl_pu, en);
		MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_tr_rx_en, 1);
		MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_tr_rx_en, 0);
	} else if (blk == 6) {
	/* DAC */
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_DAC_pwrup, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG1, 0, wl_DAC_pwrup, en);
	} else if (blk == 7) {
		MOD_RADIO_REG_20692(pi, WL_CLK_DIV_OVR1, 0, wl_ovr_dac_clk_pu, 1);
		MOD_RADIO_REG_20692(pi, WL_CLK_DIV_CFG1, 0, wl_afeclkdiv_dac_clk_pu, en);
	}

	return BCME_OK;
}
#endif /* LCN20PHY_DEBUG */

/* BEGIN: SET CHANNEL Below functions are part of set channel funtionality */

static void
wlc_lcn20phy_bandset(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, lpphyCtrl, muxGmode, 1);
	PHY_REG_MOD(pi, LCN20PHY, ChannelControl, currentBand, 0);
}

static void
wlc_lcn20phy_tx_farrow_init(phy_info_t *pi, uint8 channel)
{
	chan_info_20692_lcn20phy_t *chi;
	int fi = LCN20PHY_TX_FARROW_RATIO;
	uint32 mu_delta, freq = 0;

	/* Write the tuning tables */
	chi = &(pi->u.pi_lcn20phy->tuningtbl[channel -1]);

	freq = chi->freq;

	mu_delta = mu_deltaLUT[channel - 1];

	phy_utils_write_phyreg(pi, LCN20PHY_MuInitialLOAD, 0x2);
	phy_utils_write_phyreg(pi, LCN20PHY_TxResamplerSampSyncVal,
		freq / math_gcd_32(freq, fi));
	phy_utils_write_phyreg(pi, LCN20PHY_TxResamplerMuDelta_u, ((mu_delta >> 16) & 0xff));
	phy_utils_write_phyreg(pi, LCN20PHY_TxResamplerMuDelta_l, (mu_delta & 0xffff));
	phy_utils_write_phyreg(pi, LCN20PHY_TxResamplerMuDeltaInit_u, ((mu_delta >> 16) & 0xff));
	phy_utils_write_phyreg(pi, LCN20PHY_TxResamplerMuDeltaInit_l, (mu_delta & 0xffff));
}

static void wlc_lcn20phy_reset_iir_filter(phy_info_t *pi)
{
	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 1)

		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCtrl0, txSoftReset, 1)

		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCtrl0, txSoftReset, 0)

		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcn20phy_cckscale_tx_iir_filter(phy_info_t *pi)
{
	int8 scale_fctr_db = pi->u.pi_lcn20phy->cckscale_fctr_db;

	/* scale CCK tx-iir coefficients based on nvram
	* parameter 'cckscale' (default set to -1)
	*/
	if (scale_fctr_db > 0) {
		uint8 scale_fctr;
		uint16 fltcoeffB1, fltcoeffB2, fltcoeffB3;

		if (scale_fctr_db > LCN20PHY_MAX_CCK_DB_SCALING)
			scale_fctr_db =  LCN20PHY_MAX_CCK_DB_SCALING;
		scale_fctr = LCN20PHY_db2scalefctr_cck[scale_fctr_db-1];

		fltcoeffB1 = phy_utils_read_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B1);
		fltcoeffB2 = phy_utils_read_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B2);
		fltcoeffB3 = phy_utils_read_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B3);

		phy_utils_write_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B1,
		((fltcoeffB1 * scale_fctr) >> LCN20PHY_DB2SCALEFCTR_SHIFT));
		phy_utils_write_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B2,
		((fltcoeffB2 * scale_fctr) >> LCN20PHY_DB2SCALEFCTR_SHIFT));
		phy_utils_write_phyreg(pi, LCN20PHY_ccktxfilt20CoeffStg2B3,
		((fltcoeffB3 * scale_fctr) >> LCN20PHY_DB2SCALEFCTR_SHIFT));
	}
}

static int
wlc_lcn20phy_load_tx_iir_filter(phy_info_t *pi, phy_tx_iir_filter_mode_t mode, int16 filt_type)
{
	int16 filt_index = -1, j;
	const uint16 (*dac_coeffs_table)[LCN20PHY_NUM_DIG_FILT_COEFFS+1];
	uint8 max_filter_type, max_filter_coeffs;
	uint16 *addr_coeff;

	uint16 addr_cck[] = {
		LCN20PHY_ccktxfilt20Stg1Shft,
		LCN20PHY_ccktxfilt20CoeffStg0A1,
		LCN20PHY_ccktxfilt20CoeffStg0A2,
		LCN20PHY_ccktxfilt20CoeffStg0B1,
		LCN20PHY_ccktxfilt20CoeffStg0B2,
		LCN20PHY_ccktxfilt20CoeffStg0B3,
		LCN20PHY_ccktxfilt20CoeffStg1A1,
		LCN20PHY_ccktxfilt20CoeffStg1A2,
		LCN20PHY_ccktxfilt20CoeffStg1B1,
		LCN20PHY_ccktxfilt20CoeffStg1B2,
		LCN20PHY_ccktxfilt20CoeffStg1B3,
		LCN20PHY_ccktxfilt20CoeffStg2A1,
		LCN20PHY_ccktxfilt20CoeffStg2A2,
		LCN20PHY_ccktxfilt20CoeffStg2B1,
		LCN20PHY_ccktxfilt20CoeffStg2B2,
		LCN20PHY_ccktxfilt20CoeffStg2B3,
		LCN20PHY_ccktxfilt20CoeffStg0_leftshift
		};

	uint16 addr_ofdm[] = {
		LCN20PHY_txfilt20Stg1Shft,
		LCN20PHY_txfilt20CoeffStg0A1,
		LCN20PHY_txfilt20CoeffStg0A2,
		LCN20PHY_txfilt20CoeffStg0B1,
		LCN20PHY_txfilt20CoeffStg0B2,
		LCN20PHY_txfilt20CoeffStg0B3,
		LCN20PHY_txfilt20CoeffStg1A1,
		LCN20PHY_txfilt20CoeffStg1A2,
		LCN20PHY_txfilt20CoeffStg1B1,
		LCN20PHY_txfilt20CoeffStg1B2,
		LCN20PHY_txfilt20CoeffStg1B3,
		LCN20PHY_txfilt20CoeffStg2A1,
		LCN20PHY_txfilt20CoeffStg2A2,
		LCN20PHY_txfilt20CoeffStg2B1,
		LCN20PHY_txfilt20CoeffStg2B2,
		LCN20PHY_txfilt20CoeffStg2B3,
		LCN20PHY_txfilt20CoeffStg0_leftshift
		};
	switch (mode) {
	case (TX_IIR_FILTER_OFDM):
		addr_coeff = (uint16 *)addr_ofdm;
		dac_coeffs_table = LCN20PHY_txdigfiltcoeffs_ofdm;
		max_filter_type = LCN20PHY_NUM_TX_DIG_FILTERS_OFDM;
		pi->u.pi_lcn20phy->ofdm_filt_type = filt_type;
		break;
	case (TX_IIR_FILTER_CCK):
		addr_coeff = (uint16 *)addr_cck;
		dac_coeffs_table = LCN20PHY_txdigfiltcoeffs_cck;
		max_filter_type = LCN20PHY_NUM_TX_DIG_FILTERS_CCK;
		break;
	default:
		/* something wierd happened if coming here */
		addr_coeff = NULL;
		dac_coeffs_table = NULL;
		max_filter_type = 0;
		ASSERT(FALSE);
	}

	max_filter_coeffs = LCN20PHY_NUM_DIG_FILT_COEFFS;

	/* Search for the right entry in the table */
	for (j = 0; j < max_filter_type; j++) {
		if (filt_type == dac_coeffs_table[j][0]) {
			filt_index = (int16)j;
			break;
		}
	}

	/* Grave problem if entry not found */
	if (filt_index == -1) {
		ASSERT(FALSE);
	} else {
		/* Apply the coefficients to the filter type */
		for (j = 0; j < max_filter_coeffs; j++)
			phy_utils_write_phyreg(pi, addr_coeff[j],
				dac_coeffs_table[filt_index][j+1]);

		/* Scale cck coeffs */
		if (mode == TX_IIR_FILTER_CCK)
			wlc_lcn20phy_cckscale_tx_iir_filter(pi);
	}

	/* Reset the iir filter after setting the coefficients */
	wlc_lcn20phy_reset_iir_filter(pi);

	return (filt_index != -1) ? 0 : -1;
}

static void
wlc_lcn20phy_tx_init(phy_info_t *pi, uint8 channel)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int16 cck_ftype, ofdm_ftype;

	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, loft_comp_shift, 0);
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, copyinsert_ofdm_en, 0);
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, copyinsert_cck_en, 1);

	wlc_lcn20phy_tx_farrow_init(pi, channel);

	/* Verified this tailcount value for 11g and 11n.  (default/reset value is 0x29).
	* (lower tailcount values will result in degraded Tx-PSD, seen in max-hold mode)
	* Verfied that the default tailcountvalue for 11b works.
	* (default value for 11b phyreg "bphyFrmTailCntValue" =128).
	*/
	PHY_REG_MOD(pi, LCN20PHY, txTailCountValue, TailCountValue, 90);

	/* Set DAC mode to use 12 MSBs when converting from 13-bit TX samples
	* to 12-bit DAC samples
	*/
	phy_utils_write_phyreg(pi, LCN20PHY_BypassPredacRound, 0x0);

	/* Init/load the real iir filter coefficients */
	if (pi_lcn20->cckdigiftype >= 0)
		cck_ftype = pi_lcn20->cckdigiftype;
	else
		cck_ftype = 0;

	if (((channel == 1) || (channel == 11)) && (pi_lcn20->ofdmdigiftypebe >= 0))
		ofdm_ftype = pi_lcn20->ofdmdigiftypebe;
	else if (pi_lcn20->ofdmdigiftype >= 0)
		ofdm_ftype = pi_lcn20->ofdmdigiftype;
	else
		ofdm_ftype = 0;

	wlc_lcn20phy_load_tx_iir_filter(pi, TX_IIR_FILTER_CCK, cck_ftype);
	wlc_lcn20phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, ofdm_ftype);

	PHY_REG_MOD(pi, LCN20PHY, lpphyCtrl, txfiltSelect, 2);
}

static void
wlc_lcn20phy_agc_reset(phy_info_t *pi)
{
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN20PHY, crsgainCtrl_new, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_OR_ENTRY(LCN20PHY, resetCtrl, 0x44)
		PHY_REG_WRITE_ENTRY(LCN20PHY, resetCtrl, 0x80)
		PHY_REG_WRITE_ENTRY(LCN20PHY, crsgainCtrl_new, 0xff)
		PHY_REG_MOD_ENTRY(LCN20PHY, agcControl4, c_agc_fsm_en, 1)
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_lcn20phy_agc_tweaks(phy_info_t *pi, uint8 channel)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi))
		wlc_lcn20phy_agc_reset(pi);
	else if (LCN20REV_IS(pi->pubpi->phy_rev, 0))
		wlc_lcn20phy_agc_setup(pi);
	else if (LCN20REV_IS(pi->pubpi->phy_rev, 1))
		wlc_lcn20phy_rev1_agc_setup(pi, channel);
	else {
		PHY_ERROR(("wl%d: %s: Unsupported phy rev!\n",
			pi->sh->unit, __FUNCTION__));
		ASSERT(FALSE);
	}
}

static void
wlc_lcn20phy_rx_farrow_init(phy_info_t *pi, uint8 channel)
{
	uint32 freq = 0, farrow_mu, highest_period, GCD, rxperiod;
	chan_info_20692_lcn20phy_t *chi;
	uint32 far_muLUT[14] = {
	31614566, 31680102, 31745638, 31811174, 31876710, 31942246, 32007782,
	32073318, 32138854,	32204390, 32269926,	32335462, 32400998,	32558285
	};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Write the tuning tables */
	chi = &(pi->u.pi_lcn20phy->tuningtbl[channel -1]);

	freq = chi->freq;

	farrow_mu = far_muLUT[channel - 1];

	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig2, fcw_value_lo, (farrow_mu & 0xffff));
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig3, fcw_value_hi, ((farrow_mu >> 16) & 0xffff));
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig3, fast_ADC_en, 0);

	highest_period = 1280;
	GCD = math_gcd_32(highest_period, freq);
	rxperiod = highest_period / GCD;

	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig6, rx_farrow_drift_period, rxperiod);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_drift_en, 1);
}

static void
wlc_lcn20phy_set_sfo_chan_centers(phy_info_t *pi, uint8 channel)
{
	uint16 freq = 0, tmp;
	chan_info_20692_lcn20phy_t *chi = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Write the tuning tables */
	chi = &(pi->u.pi_lcn20phy->tuningtbl[channel -1]);

	freq = chi->freq;

	/* sfo_chan_center_Ts20 = round(fc / 20e6*(ten_mhz+1) * 8), fc in Hz
	*                      = round($channel * 0.4 *($ten_mhz+1)), $channel in MHz
	*/
	tmp = (freq * 4 / 5 + 1) >> 1;
	PHY_REG_MOD(pi, LCN20PHY, ptcentreTs20, centreTs20, tmp);
	/* sfo_chan_center_factor = round(2^17 ./ (fc/20e6)/(ten_mhz+1)), fc in Hz
	*                        = round(2621440 ./ $channel/($ten_mhz+1)), $channel in MHz
	*/
	tmp = (2621440 * 2 / freq + 1) >> 1;
	PHY_REG_MOD(pi, LCN20PHY, ptcentreFactor, centreFactor, tmp);
}

static void
wlc_lcn20phy_rx_init(phy_info_t *pi, uint8 channel)
{

	wlc_lcn20phy_rx_farrow_init(pi, channel);
	wlc_lcn20phy_set_sfo_chan_centers(pi, channel);

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_MOD(pi, LCN20PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 0x3);
	}

	{
		phytbl_info_t tab;
		uint16 rfseq[19] =
		{8, 5, 6, 1600, 8, 5, 8, 6, 119, 8, 138, 8, 3, 5, 3, 10, 3, 5, 1600};

		tab.tbl_id = LCN20PHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 13 bit wide	*/
		tab.tbl_ptr = &rfseq;

		tab.tbl_offset = 3;
		tab.tbl_len = 19;
		wlc_lcn20phy_write_table(pi,  &tab);

		rfseq[0] = 12; rfseq[1] = 1; rfseq[2] = 8;
		tab.tbl_offset = 29;
		tab.tbl_len = 3;
		wlc_lcn20phy_write_table(pi,  &tab);
	}

	/* power down lna1 in bypass mode; [LONG] */
	PHY_REG_MOD(pi, LCN20PHY, rfseq_cfg, lna1_pd_on_bypass, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfseq_cfg, lna1_pulse_on_pu_bypass, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfseq_cfg, lna1_pulse_on_pd_bypass, 1);

	/* hiip3 mode */
	PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, lna1_byp_hiip3_ovr_val, 1);

	/* To avoid DC offset change after TX2RX turnaround */
	/* set RF override mix and div2 pu values appropriately */
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_ADCldo_pu, 1);
	/* LOGEN LDO needs to be in high-power mode when LOGEN on */
	MOD_RADIO_REG_20692(pi, WL_PMU_CFG3, 0, wl_VCOldo_hpm, 1);
	MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG1, 0, wl_logencore_en_2g, 1);
	MOD_RADIO_REG_20692(pi, WL_LOGEN_OVR1, 0, wl_ovr_logencore_en_2g, 1);

	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxmix2g_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxmix2g_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rxdiv2g_rs, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxdiv2g_rs, 0x1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rxdiv2g_pu_bias, 0x1);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxdiv2g_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_clk_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_clk_slow_pu, 0x1);

	PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, lna1_byp_hiip3_ovr_val, 1);

	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, swap_iq, 1);

	/* Adjust ofdm sifs timing */
	PHY_REG_MOD(pi, LCN20PHY, readsym2resetCtrl, readsym2resetwaitlen, 0x72);

	/* initialize DCC registers and tables */
	wlc_lcn20phy_dccal_init(pi, (LCN20PHY_DCCALMODE_DCOE |
		LCN20PHY_DCCALMODE_IDACC | LCN20PHY_DCCALMODE_IDACT), 1, 2);

	/* Radio power save settings for RX from Long */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG6, 0, wl_tia_opamp1_biasadj, 0x8);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG7, 0, wl_tia_opamp2_biasadj, 0x8);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_IDAC1, 0, wl_lna2g_lna1_bias_idac, 0x0);

	/* Tweak AGC parameters and reset AGC */
	wlc_lcn20phy_agc_tweaks(pi, channel);
}

void
wlc_lcn20phy_radio20692_rc_cal_lpf(phy_info_t *pi, uint8 cal, uint32 dn)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	BCM_REFERENCE(cal);

	/* lpf */
	/* set k [expr {$is_adc ? 75000 : 101541}] */
	/* set gmult_p12 [expr {$prod1 / $fxtal_pm12}] */
	pi_lcn20->rccal_gmult = (101541 * dn) / (pi->xtalfreq >> 12);
	pi_lcn20->rccal_gmult_rc = pi_lcn20->rccal_gmult;
	PHY_INFORM(("wl%d: %s rccal_lpf_gmult = %d\n", pi->sh->unit,
		__FUNCTION__, pi_lcn20->rccal_gmult));

#ifdef ATE_BUILD
	ate_buffer_regval.gmult_lpf = pi_lcn20->rccal_gmult;
#endif // endif
}

void
wlc_lcn20phy_radio20692_rc_cal_adc(phy_info_t *pi, uint32 dn)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	/* adc */
	/* set k [expr {$is_adc ? 75000 : 101541}] */
	/* set gmult_p12 [expr {$prod1 / $fxtal_pm12}] */
	pi_lcn20->rccal_adc_gmult = (75000 * dn) / (pi->xtalfreq >> 12);
	PHY_INFORM(("wl%d: %s rccal_adc = %d\n", pi->sh->unit,
		__FUNCTION__, pi_lcn20->rccal_adc_gmult));
#ifdef ATE_BUILD
	ate_buffer_regval.gmult_adc = pi_lcn20->rccal_adc_gmult;
#endif // endif
}

void
wlc_lcn20phy_radio20692_rc_cal_dacbuf(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	/* dacbuf */
	pi_lcn20->rccal_dacbuf =
	READ_RADIO_REGFLD_20692(pi, WL_RCCAL_CFG6, 0, wl_rccal_raw_dacbuf);
	MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_DACbuf_fixed_cap, 0);
	PHY_INFORM(("wl%d: %s rccal_dacbuf = %d\n", pi->sh->unit,
		__FUNCTION__, pi_lcn20->rccal_dacbuf));
#ifdef ATE_BUILD
	ate_buffer_regval.rccal_dacbuf = pi_lcn20->rccal_dacbuf;
#endif // endif
}

static void
wlc_lcn20phy_radio20692_rc_cal(phy_info_t *pi)
{
	uint8 cal, done;
	uint16 rccal_itr, n0, n1;

	/* lpf, adc, dacbuf */
	uint8 sr[] = {0x1, 0x1, 0x0};
	uint8 sc[] = {0x0, 0x2, 0x1};
	uint8 x1[] = {0x1c, 0x70, 0x40};
	uint16 trc[] = {0x22d, 0xf0, 0x10a};
	uint32 dn;

	if (NORADIO_ENAB(pi->pubpi))
		return;

	/* ASSERT(RADIOID(pi->pubpi->radioid) == BCM20692_ID); */

	/* Powerup rccal clock & set divider radio (rccal needs to run at 20mhz) */
	MOD_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0, wl_xtal_out_pu,
		(READ_RADIO_REGFLD_20692(pi, WL_XTAL_CFG1, 0, wl_xtal_out_pu) | 0x440));

	/* Calibrate lpf, adc, dacbuf */
	for (cal = 0; cal < LCN20PHY_NUM_2069_RCCAL_CAPS; cal++) {
		/* Setup */
		MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_rccal_sr, sr[cal]);
		MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_rccal_sc, sc[cal]);
		MOD_RADIO_REG_20692(pi, WL_RCCAL_CFG1, 0, wl_rccal_X1, x1[cal]);
		MOD_RADIO_REG_20692(pi, WL_RCCAL_CFG2, 0, wl_rccal_Trc, trc[cal]);

		/* Toggle RCCAL power */
		MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_rccal_pu, 0);
		OSL_DELAY(1);
		MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_rccal_pu, 1);

		OSL_DELAY(35);

		/* Start RCCAL */
		MOD_RADIO_REG_20692(pi, WL_RCCAL_CFG1, 0, wl_rccal_START, 1);

		/* Wait for rcal to be done, max = 100us * 100 = 10ms  */
		done = 0;
		for (rccal_itr = 0;
			(rccal_itr < LCN20PHY_MAX_2069_RCCAL_WAITLOOPS) && (done == 0);
			rccal_itr++) {
			OSL_DELAY(100);
			done = READ_RADIO_REGFLD_20692(pi, WL_RCCAL_CFG3, 0, wl_rccal_DONE);
		}

		/* Stop RCCAL */
		MOD_RADIO_REG_20692(pi, WL_RCCAL_CFG1, 0, wl_rccal_START, 0);

		/* Make sure that RC Cal ran to completion */
		ASSERT(done);

		n0 = READ_RADIO_REGFLD_20692(pi, WL_RCCAL_CFG4, 0, wl_rccal_N0);
		n1 = READ_RADIO_REGFLD_20692(pi, WL_RCCAL_CFG5, 0, wl_rccal_N1);
		dn = n1 - n0; /* set dn [expr {$N1 - $N0}] */

		if (cal == 0) {
			wlc_lcn20phy_radio20692_rc_cal_lpf(pi, cal, dn);
		} else if (cal == 1) {
			wlc_lcn20phy_radio20692_rc_cal_adc(pi, dn);
		}
		/* Turn off rccal */
		MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_rccal_pu, 0);
	}

	/* DACbuf RC cal is skipped but this function is retained
	 * to update ate_buffer_regval.rccal_dacbuf
	 */
	wlc_lcn20phy_radio20692_rc_cal_dacbuf(pi);

	/* disable rccal clock */
	WRITE_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0,
		(READ_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0) & (~0x400)));
}

/* 20692_lpf_tx_set is the top Tx LPF function
*/
static void
wlc_lcn20phy_radio20692_lpf_tx_set(phy_info_t *pi, int8 bq_bw, int8 bq_gain,
	int8 rc_bw_ofdm, int8 rc_bw_cck)
{
	uint8 i;
	uint16 gmult;
	uint16 gmult_rc;
	uint16 g10_tuned, g11_tuned, g12_tuned, g21_tuned, bias;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	BCM_REFERENCE(rc_bw_cck);

	gmult = pi_lcn20->rccal_gmult;
	gmult_rc = pi_lcn20->rccal_gmult_rc;

	/* search for given bq_gain */
	for (i = 0; i < ARRAYSIZE(g_index1); i++) {
		if (bq_gain == g_index1[i])
			break;
	}

	if (i < ARRAYSIZE(g_index1)) {
		uint16 g_passive_rc_tx_tuned_ofdm;
		g10_tuned = (lpf_g10[bq_bw][i] * gmult) >> 15;
		g11_tuned = (lpf_g11[bq_bw] * gmult) >> 15;
		g12_tuned = (lpf_g12[bq_bw][i] * gmult) >> 15;
		g21_tuned = (lpf_g21[bq_bw][i] * gmult) >> 15;
		g_passive_rc_tx_tuned_ofdm = (g_passive_rc_tx[rc_bw_ofdm] * gmult_rc) >> 15;
		bias = biases[bq_bw];

		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_g10, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_g12, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_g21, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_g11, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_g_passive_rc_tx, 1);
		MOD_RADIO_REG_20692(pi, WL_TX_BB_OVR1, 0, wl_ovr_lpf_bias_bq, 1);

		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG3, 0, wl_lpf_g10, g10_tuned);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG7, 0, wl_lpf_g11, g11_tuned);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG4, 0, wl_lpf_g12, g12_tuned);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG5, 0, wl_lpf_g21, g21_tuned);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG6, 0, wl_lpf_g_passive_rc_tx,
			g_passive_rc_tx_tuned_ofdm);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG8, 0, wl_lpf_bias_bq, bias);

		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG2, 0, wl_lpf_sel_5g_out_gm, 0);
		MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG3, 0, wl_lpf_sel_2g_5g_cmref_gm, 0);
	} else {
		PHY_ERROR(("wl%d: %s: Invalid bq_gain %d\n", pi->sh->unit, __FUNCTION__, bq_gain));
	}
}

static void
wlc_lcn20phy_detection_level(phy_info_t *pi, int dsss_thres, int ofdm_thres)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, SignalBlock_edet1, signalblk_det_thresh_dsss, dsss_thres);
	PHY_REG_MOD(pi, LCN20PHY, SignalBlock_edet1, signalblk_det_thresh_ofdm, ofdm_thres);
}

static void
wlc_lcn20phy_set_ed_thres(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_REG_MOD(pi, LCN20PHY, eddisable20ul, crs_ed_asserts_crs, 1);
	PHY_REG_MOD(pi, LCN20PHY, eddisable20ul, crseddisable_20L, 0);

	PHY_REG_MOD(pi, LCN20PHY, edthresh20ul, edonthreshold20L,
		pi_lcn20->edonthreshold20L);
	PHY_REG_MOD(pi, LCN20PHY, crsedthresh, edoffthreshold,
		pi_lcn20->edoffthreshold);
}

void
BCMATTACHFN(wlc_phy_interference_mode_attach_lcn20phy)(phy_info_t *pi)
{
	pi->sh->interference_mode = pi->sh->interference_mode_2G = WLAN_AUTO;
}

bool
wlc_lcn20phy_acimode_valid(phy_info_t *pi, int wanted_mode)
{
	BCM_REFERENCE(pi);
	if ((wanted_mode == WLAN_AUTO) ||
		(wanted_mode == WLAN_MANUAL) ||
		(wanted_mode == INTERFERE_NONE))
		return TRUE;
	else
		return FALSE;
}

int
wlc_lcn20phy_aci_modes(phy_info_t *pi, int wanted_mode)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	switch (wanted_mode) {
		case WLAN_AUTO:
			/* Automatic ACI detection */
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_sel, 0);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2,
				aci_mitigation_sw_enable, 0);
			break;

		case WLAN_MANUAL:
			/* Force ACI gain table */
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_sel, 1);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2,
				aci_mitigation_sw_enable, 1);
			break;

		case INTERFERE_NONE:
			/* Force normal gain table */
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_sel, 0);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2,
				aci_mitigation_sw_enable, 1);
			break;

		default:
			return BCME_BADOPTION;
	}

	return BCME_OK;
}

void
wlc_lcn20phy_aci_init(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* init aci detector registers */

	/* the aci detector enable - disabling */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detect_enable, 0);
	/* if under sw control (aci_mitigation_sw_enable) then this will be */
	/* the value of the aci_mitigation_state */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_sel, 0);
	/* if this field is '1', the sw_aci_report_ctr will be set to 0 */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_report_ctr_clren, 0);
	/* if this field is '1', the sw_aci_detected_ctr will be set to 0 */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detected_ctr_clren, 0);
	/* block values are summed over window size to produce a sliding window */
	/* value (e.g. w1_window_acc) */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detect_window_size_1, 15);
	/* result in aci_mitigation_state going positve as long as */
	/* aci_detect_direct_output is not set */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detect_window_size_2, 15);
	/*  if it is '1', reset all aci detector state */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detector_soft_reset, 0);
	/* control is handed to driver */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2, aci_mitigation_sw_enable, 0);
	/* has values 0,1,2. */
	/* 0 means if not direct output controlled move mitigation state to 0 i.e. disabled */
	/* 1 means aci_mitigation state returns to zero after a timeout aci_mitigation_timeout */
	/* 2 means we bypass the timeout and go back to 0 if aci_present is 0 */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2, aci_mitigation_hw_enable, 2);

	/* threshold e.g. (1/2, 1/4 etc of all window blocks counted i.e. */
	/* aci_report_ctr) for non-ACI mode if  ACI mitigation is enabled */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2, aci_present_th_mit_off, 1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2, aci_present_th_mit_on, 0);
	/* A is selected through T_RSSI_select, 0: w3, 1:nb, 2:w1 */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL3, aci_T_RSSI_select, 1);
	/* if this is enabled then the aci mitigation block is bypassed i.e. do */
	/* not need k out of m window values positive. Just require output.aci_sel */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL3, aci_detect_direct_output, 1);
	/* the input scaling applied to samples entering the block sum */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL3, aci_pwr_input_shift, 8);
	/* the pwr block scaling applied to blocks entering the sliding window */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL3, aci_pwr_block_shift, 4);
	/* each scaled A must exceed scaled B by this threshold for output.aci_sel to be true */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config1, aci_det_threshold_nor_0, 0x1000);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config2, aci_det_threshold_nor_1, -0x3000);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config3, aci_det_threshold_nor_2, -0x800);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config4, aci_det_threshold_aci_0, 0x10);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config5, aci_det_threshold_aci_1, -0x800);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config6, aci_det_threshold_aci_2, -0x300);
	/* A is selected through T_RSSI_select. Apply A_shift before comparsion with B. */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config7, aci_A_shift_nor_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config7, aci_A_shift_nor_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config7, aci_A_shift_nor_2, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config8, aci_A_shift_aci_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config8, aci_A_shift_aci_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config8, aci_A_shift_aci_2, 0);
	/* B is always farrow output accumulated power i.e pwr_window_acc. */
	/* Apply B_shift before comparsion with A. */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config9, aci_B_shift_nor_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config9, aci_B_shift_nor_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config9, aci_B_shift_nor_2, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config10, aci_B_shift_aci_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config10, aci_B_shift_aci_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config10, aci_B_shift_aci_2, 0);
	/* A is selected through T_RSSI_select. Apply A_sign change before comparsion with B. */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_nor_0, 1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_nor_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_nor_2, 1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_aci_0, 1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_aci_1, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config11, aci_A_sign_aci_2, 1);
	/* B is always farrow output accumulated power i.e pwr_window_acc. */
	/* Apply B_sign change before comparsion with A. */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_nor_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_nor_1, -1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_nor_2, -1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_aci_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_aci_1, -1);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config12, aci_B_sign_aci_2, -1);
	/* freeze all settings and states */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_freeze, aci_detect_freeze, 0);
	/* only after max_counter (i.e. the m in the k out m of above) decisions */
	/* have been made when the aci mitigation block engage */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_max_count_hi, aci_detect_max_count_hi, 0x1f);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_max_count_lo, aci_detect_max_count_lo, 0xffff);
	/* mitigation timeout for (aci_mitigation_hw_enable == 1) */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Miti_timeout_lo, aci_mitigation_timeout_lo, 0xffff);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detdct_Miti_timeout_hi, aci_mitigation_timeout_hi, 0x7fff);
	/* not only do k out of m decision have to be made but k must be greater */
	/* than report_ctr_th to change aci_mitigation_state i.e. aci_present */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_report_ctr_th, aci_report_ctr_th, 8);
	/* wait period: maximum 2^16 for non-ACI mode */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_wait_period1, aci_detect_wait_period_1, 0x3f);
	/* wait period: maximum 2^16 for ACI mode */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_wait_period2, aci_detect_wait_period_2, 0xff);
	/* collect interval where this many rssi samples (e.g. input.w1_value, 0.4us */
	/* for lcn20) are summed to produce a block value (e.g. w1_block_acc) */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_collect_interval, aci_detect_collect_interval_1, 15);
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_collect_interval, aci_detect_collect_interval_2, 15);
	/* the aci detector enable */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1, aci_detect_enable, 1);
}

static void
wlc_lcn20phy_agc_setup(phy_info_t *pi)
{
#if defined(WLPROPRIETARY_11N_RATES)
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
#endif // endif
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* selection of low/mid/high levels for nbrssi/wrssi3 detectors
	* Nbrssi
	*/
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG12, 0, wl_tia_nbrssi_ref_low_sel, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG12, 0, wl_tia_nbrssi_ref_mid_sel, 4);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG12, 0, wl_tia_nbrssi_ref_high_sel, 4);
	/* Wrssi3 */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG12, 0, wl_tia_wrssi3_ref_low_sel, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG11, 0, wl_tia_wrssi3_ref_mid_sel, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG11, 0, wl_tia_wrssi3_ref_high_sel, 7);

	/* Nbrssi/Wrssi3 */
	/* output drive strength adjust for wrrsi3 and nbrssi detectors */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG11, 0, wl_tia_rssi_drive_strength, 0);
	/* binary bias code for NBRSSI (and WRSSI3, detectors */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG11, 0, wl_tia_rssi_biasadj, 0x10);
	/* Wrssi1 */
	/* lna1 power detector threshold */
	MOD_RADIO_REG_20692(pi, WL_LNA2G_RSSI1, 0, wl_lna2g_dig_wrssi1_threshold, 8);
	/* enables x8 drive */
	MOD_RADIO_REG_20692(pi, WL_LNA2G_RSSI1, 0, wl_lna2g_dig_wrssi1_drive_strength, 0);
	/* end of RSSI SETTINGS from LCN20 radio sheet */

	/* TR Attenuation Switch */
	PHY_REG_MOD(pi, LCN20PHY, radioTRCtrl, gainrequestTRAttnOnEn, 0);
	/* gain settle delay(2, 5MHz clk, after the radio gain is changed */
	PHY_REG_MOD(pi, LCN20PHY, agcControl11, gain_settle_dly_cnt, 4);
	PHY_REG_MOD(pi, LCN20PHY, agcControl11, rssi_delay, 0);

	/* clip detector (farrow o/p, */
	/* threshold to determine clip count of Farrow o/p signal (8bit, */
	PHY_REG_MOD(pi, LCN20PHY, ClipThresh, ClipThresh, 65);
	/* clip counter threshold in HIGH GAIN state);
	* after clip count(number of I/Q clips in last 16 samples)
	* exceeds this threshold AGC goes to WAIT RSSI
	*/
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrThresh, ClipCtrThreshHiGain, 12);

	/* RSSI CLIP GAINS/THREHSOLDS in WAIT RSSI STATE */
	/*  Clip gains */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl14, rssi_clip_gain_norm_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl14, rssi_clip_gain_norm_1, 30);
	PHY_REG_MOD(pi, LCN20PHY, agcControl15, rssi_clip_gain_norm_2, 21);
	/* Wrssi3 */
	/* Clip gain of 11 used to be 15 but PER humps were seen on UMC */
	/* boards with this setting */
	PHY_REG_MOD(pi, LCN20PHY, agcControl15, rssi_clip_gain_norm_3, 21);
	PHY_REG_MOD(pi, LCN20PHY, agcControl16, rssi_clip_gain_norm_4, 21);
	PHY_REG_MOD(pi, LCN20PHY, agcControl16, rssi_clip_gain_norm_5, 12);
	/* Wrssi2 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl17, rssi_clip_gain_norm_6, 12);
	PHY_REG_MOD(pi, LCN20PHY, agcControl17, rssi_clip_gain_norm_7, 12);
	PHY_REG_MOD(pi, LCN20PHY, agcControl18, rssi_clip_gain_norm_8, 12);
	/* Wrssi1 */
	PHY_REG_MOD(pi, LCN20PHY, agcContro245, rssi_clip_gain_norm_9, 3);
	PHY_REG_MOD(pi, LCN20PHY, agcContro245, rssi_clip_gain_norm_10, -6);
	PHY_REG_MOD(pi, LCN20PHY, agcContro246, rssi_clip_gain_norm_11, -18);

	/*  Clip counter thresholds */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl19, rssi_clip_thresh_norm_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl19, rssi_clip_thresh_norm_1, 15);
	PHY_REG_MOD(pi, LCN20PHY, agcControl20, rssi_clip_thresh_norm_2, 21);
	/* W3 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl20, rssi_clip_thresh_norm_3, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl21, rssi_clip_thresh_norm_4, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl21, rssi_clip_thresh_norm_5, 19);
	/* w2 rssi (not used, */
	PHY_REG_MOD(pi, LCN20PHY, agcControl22, rssi_clip_thres_norm_6, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl22, rssi_clip_thresh_norm_7, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl23, rssi_clip_thresh_norm_8, 0);
	/* W1 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl23, rssi_clip_thresh_norm_9,  18);
	PHY_REG_MOD(pi, LCN20PHY, agcContro240, rssi_clip_thresh_norm_10, 22);
	PHY_REG_MOD(pi, LCN20PHY, agcContro240, rssi_clip_thresh_norm_11, 25);

	/* rssi no clip gain */
	PHY_REG_MOD(pi, LCN20PHY, agcControl38, rssi_no_clip_gain_normal, 36);

	/* Clip gains */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl33, rssi_clip_gain_aci_0, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl33, rssi_clip_gain_aci_1, 21+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl34, rssi_clip_gain_aci_2, 12+LCN20PHY_ACITBL_OFFSET);
	/* Wrssi3 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl34, rssi_clip_gain_aci_3, 12+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl35, rssi_clip_gain_aci_4, 12+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl35, rssi_clip_gain_aci_5, 12+LCN20PHY_ACITBL_OFFSET);
	/* Wrssi2 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl36, rssi_clip_gain_aci_6, 12+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl36, rssi_clip_gain_aci_7, 12+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl37, rssi_clip_gain_aci_8, 12+LCN20PHY_ACITBL_OFFSET);
	/* Wrssi1 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl238, rssi_clip_gain_aci_9, 3+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl238, rssi_clip_gain_aci_10, -6+LCN20PHY_ACITBL_OFFSET);
	PHY_REG_MOD(pi, LCN20PHY, agcControl239, rssi_clip_gain_aci_11, -18+LCN20PHY_ACITBL_OFFSET);

	/* Clip counter thresholds */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl43, rssi_clip_thresh_aci_0,  0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl44, rssi_clip_thresh_aci_1,  40);
	PHY_REG_MOD(pi, LCN20PHY, agcControl44, rssi_clip_thresh_aci_2,  34);
	/* W3 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl45, rssi_clip_thresh_aci_3,  0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl45, rssi_clip_thresh_aci_4,  0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl46, rssi_clip_thresh_aci_5,  0);
	/* W2 rssi (not used) */
	PHY_REG_MOD(pi, LCN20PHY, agcControl46, rssi_clip_thresh_aci_6,  0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl47, rssi_clip_thresh_aci_7,  0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl47, rssi_clip_thresh_aci_8,  0);
	/* W1 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcContro243, rssi_clip_thresh_aci_9,  5);
	PHY_REG_MOD(pi, LCN20PHY, agcContro243, rssi_clip_thresh_aci_10, 10);
	PHY_REG_MOD(pi, LCN20PHY, agcContro244, rssi_clip_thresh_aci_11, 18);

	/* rssi no clip gain */
	PHY_REG_MOD(pi, LCN20PHY, agcControl37, rssi_no_clip_gain_aci, 24+LCN20PHY_ACITBL_OFFSET);
	/* changed from -4 as was causing clipping. See ~0.5dB sensitivity */
	/* improvement for rate 11 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl27, data_mode_gain_db_adj_dsss, -6);

	/* few more tweaks from Mark */
	PHY_REG_MOD(pi, LCN20PHY, agcControl18, rssi_clip_start, 1);
	/* 0: farrow put, 2: DC free farrow out, 1: DC filt out */
	PHY_REG_MOD(pi, LCN20PHY, dcBlockInit, clipMuxSel, 2);
	PHY_REG_MOD(pi, LCN20PHY, SyncPeakCnt, MaxPeakCntM1, 7);

	PHY_REG_MOD(pi, LCN20PHY, agcControl11, rssi_gain, 57);
	PHY_REG_MOD(pi, LCN20PHY, agcControl24, rssi_no_clip_gain_mismatch, -6);
	PHY_REG_MOD(pi, LCN20PHY, agcControl24, rssi_no_clip_gain_adj, -9);
	PHY_REG_MOD(pi, LCN20PHY, agcControl2, hg_clip_count, 0);
	PHY_REG_MOD(pi, LCN20PHY, PwrThresh1, PktRxSignalDropThresh, 18);
	PHY_REG_MOD(pi, LCN20PHY, agcControl12, crs_gain_high_gain_db_40mhz, LCN20PHY_INIT_GAIN_DB);
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrDefThresh, hi_gain_ofdm_mismatch, LCN20PHY_INIT_GAIN_DB+3);
	PHY_REG_MOD(pi, LCN20PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, LCN20PHY_GAIN_TBL_OFFSET);

	PHY_REG_MOD(pi, LCN20PHY, agcControl48, repeat_clip_backoff, 0);
	PHY_REG_MOD(pi, LCN20PHY, nfSubtractVal, nfSubVal, 170);

	/* end of RSSI CLIP GAINS/THREHSOLDS in WAIT RSSI STATE */

	PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz, -16);
	PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable6_new,
		crssignalblk_input_pwr_offset_db, -16);

	PHY_REG_MOD(pi, LCN20PHY, ClipDetector, clip_delay_count, 1);
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrDefThresh, clipCtrThresh, 46);

#if defined(WLPROPRIETARY_11N_RATES)
	if (pi_lcn20->qam256en) {
		PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable2_new, crssignalblk_noi_pwr, 97);
		PHY_REG_MOD(pi, LCN20PHY, DcFiltAddress_0, dcBypass, 1);
		PHY_REG_MOD(pi, LCN20PHY, channelUpdateReg, HardDeciBasedalpha, 5);
		PHY_REG_MOD(pi, LCN20PHY, channelUpdateReg, vitbBasedalpha, 0);
	} else
		PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable2_new, crssignalblk_noi_pwr, 91);
#else
	PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable2_new, crssignalblk_noi_pwr, 91);
#endif // endif

	/* lower DSSS threshold */
	wlc_lcn20phy_detection_level(pi, 97,  97);

	/* set ED thresholds */
	wlc_lcn20phy_set_ed_thres(pi);

	wlc_lcn20phy_aci_init(pi);

	wlc_lcn20phy_agc_reset(pi);
}

static void
wlc_lcn20phy_rev1_agc_setup(phy_info_t *pi, uint8 channel)
{
	lcn20phy_lna_params_t *lna_params = pi->u.pi_lcn20phy->lna_params;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_lcn20phy_agc_setup(pi);

	/* Changing the threshold value to fix the adaptivity timing
	     failures on Maccabee/Maccabee-Lite (Radar 24604299)
	  */
	PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_Config3, aci_det_threshold_nor_2, 0x1500);

	MOD_RADIO_REG_20692(pi, WL_LNA2G_RSSI1, 0, wl_lna2g_dig_wrssi1_threshold,
		lna_params->wl_lna2g_dig_wrssi1_threshold);

	/* eLNA bypass control */
	PHY_REG_MOD(pi, LCN20PHY, radioTRCtrl, gainrequestTRAttnOnEn,
		lna_params->gainrequestTRAttnOnEn);
	PHY_REG_MOD(pi, LCN20PHY, radioTRCtrlCrs1,
		gainReqTrAttOnEnByCrs, lna_params->gainReqTrAttOnEnByCrs);
	PHY_REG_MOD(pi, LCN20PHY, radioTRCtrlCrs1, trGainThresh, lna_params->trGainThresh);
	PHY_REG_MOD(pi, LCN20PHY, radioTRCtrl, gainrequestTRAttnOnOffset,
		lna_params->gainrequestTRAttnOnOffset);
	/* RSSI CLIP GAINS/THREHSOLDS in WAIT RSSI STATE */
	/*  Clip gains */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl14, rssi_clip_gain_norm_1,
		30+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl15, rssi_clip_gain_norm_2,
		21+LCN20PHY_NORTBL_OFFSET_NEW);
	/* Wrssi3 */
	/* Clip gain of 11 used to be 15 but PER humps were seen on UMC */
	/* boards with this setting */
	PHY_REG_MOD(pi, LCN20PHY, agcControl15, rssi_clip_gain_norm_3,
		21+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl16, rssi_clip_gain_norm_4,
		21+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl16, rssi_clip_gain_norm_5,
		12+LCN20PHY_NORTBL_OFFSET_NEW);
	/* Wrssi2 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl17, rssi_clip_gain_norm_6,
		12+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl17, rssi_clip_gain_norm_7,
		12+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl18, rssi_clip_gain_norm_8,
		12+LCN20PHY_NORTBL_OFFSET_NEW);
	/* Wrssi1 */
	PHY_REG_MOD(pi, LCN20PHY, agcContro245, rssi_clip_gain_norm_9,
		3+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcContro245, rssi_clip_gain_norm_10,
		-6+LCN20PHY_NORTBL_OFFSET_NEW);
	/* W1 High Clip Gain increased from -18dB to -12dB to overcome PER humps at ~-27 dBm RSSI */
	PHY_REG_MOD(pi, LCN20PHY, agcContro246, rssi_clip_gain_norm_11,
		-12+LCN20PHY_NORTBL_OFFSET_NEW);

	/* W1 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl23, rssi_clip_thresh_norm_9,
		lna_params->rssi_clip_thresh_norm_9);
	PHY_REG_MOD(pi, LCN20PHY, agcContro240, rssi_clip_thresh_norm_10,
		lna_params->rssi_clip_thresh_norm_10);
	PHY_REG_MOD(pi, LCN20PHY, agcContro240, rssi_clip_thresh_norm_11,
		lna_params->rssi_clip_thresh_norm_11);

	/* rssi no clip gain */
	PHY_REG_MOD(pi, LCN20PHY, agcControl38, rssi_no_clip_gain_normal,
		36+LCN20PHY_NORTBL_OFFSET_NEW);

	/* Clip gains */
	/* Nbrssi */
	PHY_REG_MOD(pi, LCN20PHY, agcControl33, rssi_clip_gain_aci_1,
		21+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl34, rssi_clip_gain_aci_2,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	/* Wrssi3 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl34, rssi_clip_gain_aci_3,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl35, rssi_clip_gain_aci_4,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl35, rssi_clip_gain_aci_5,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	/* Wrssi2 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl36, rssi_clip_gain_aci_6,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl36, rssi_clip_gain_aci_7,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl37, rssi_clip_gain_aci_8,
		12+LCN20PHY_ACITBL_OFFSET_NEW);
	/* Wrssi1 */
	PHY_REG_MOD(pi, LCN20PHY, agcControl238, rssi_clip_gain_aci_9,
		3+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl238, rssi_clip_gain_aci_10,
		-6+LCN20PHY_ACITBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl239, rssi_clip_gain_aci_11,
		-18+LCN20PHY_ACITBL_OFFSET_NEW);

	/* W1 rssi */
	PHY_REG_MOD(pi, LCN20PHY, agcContro243, rssi_clip_thresh_aci_9,
		lna_params->rssi_clip_thresh_aci_9);
	PHY_REG_MOD(pi, LCN20PHY, agcContro243, rssi_clip_thresh_aci_10,
		lna_params->rssi_clip_thresh_aci_10);
	PHY_REG_MOD(pi, LCN20PHY, agcContro244, rssi_clip_thresh_aci_11,
		lna_params->rssi_clip_thresh_aci_11);

	/* rssi no clip gain */
	PHY_REG_MOD(pi, LCN20PHY, agcControl37, rssi_no_clip_gain_aci,
		24+LCN20PHY_ACITBL_OFFSET_NEW);

	PHY_REG_MOD(pi, LCN20PHY, agcControl11, rssi_gain, lna_params->rssi_gain);
	PHY_REG_MOD(pi, LCN20PHY, agcControl12, crs_gain_high_gain_db_40mhz,
		lna_params->crs_gain_high_gain_db_40mhz);
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrDefThresh, hi_gain_ofdm_mismatch,
		lna_params->crs_gain_high_gain_db_40mhz+3);
	PHY_REG_MOD(pi, LCN20PHY, wl_gain_tbl_offset, wl_gain_tbl_offset,
		lna_params->wl_gain_tbl_offset);
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrDefThresh, clipCtrThresh, lna_params->clipCtrThresh);

	PHY_REG_MOD(pi, LCN20PHY, nfSubtractVal, nfSubVal,
		170+((LCN20PHY_NORTBL_OFFSET_NEW*4*85)>>5));

	PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable5_new,
		crssignalblk_input_pwr_offset_db_40mhz, -16+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, SignalBlockConfigTable6_new,
		crssignalblk_input_pwr_offset_db, -16+LCN20PHY_NORTBL_OFFSET_NEW);

	PHY_REG_MOD(pi, LCN20PHY, agcControl5, ss_min_gain, -18+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl6, c_ng_ofdm_detect_gain,
		80+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, agcControl48, fstr_digi_gain_mask_thresh,
		77+LCN20PHY_NORTBL_OFFSET_NEW);

	PHY_REG_MOD(pi, LCN20PHY, crsTimingCtrl20U_new, c_ofdm_thresh_gain_20U,
		77+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, crsTimingCtrl20U_new, gainThrsh4Timing,
		77+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, crsTimingCtrl20L_new, c_ofdm_thresh_gain_20L,
		77+LCN20PHY_NORTBL_OFFSET_NEW);
	PHY_REG_MOD(pi, LCN20PHY, crsTimingCtrl20L_new, gainThrsh4Timing,
		77+LCN20PHY_NORTBL_OFFSET_NEW);

	PHY_REG_MOD(pi, LCN20PHY, ClassfyblkConfigTable8_new, crsclassfyblk_classfy_thresh_gain,
		76+LCN20PHY_NORTBL_OFFSET_NEW);

	/* Adjust the DSSS detect thresholds from nvram offset parameters */
	wlc_lcn20phy_detection_level(pi,
	LCN20PHY_DSSS_DETECTION_THRESHOLD-lna_params->dsss_threshold_offset[channel-1],
	LCN20PHY_OFDM_DETECTION_THRESHOLD);

	PHY_REG_MOD(pi, LCN20PHY, detcondbypass20_new, ofdmDetCond1Byp, 0);
	wlc_lcn20phy_agc_reset(pi);
}

#ifdef WL11ULB
/* Function to set/reset 5/10MHz mode (cor. TCL proc is ulb_mode in chipc.tcl) */
/* ulb_mode: 0 - reset to normal mode
 * ulb_mode: 1 - 10MHz mode
 * ulb_mode: 2 - 5MHz mode
*/

void
wlc_phy_ulb_mode_lcn20phy(phy_info_t *pi, uint8 ulb_mode_set)
{
	bool suspend;
	uint8 dsss_detection_level, ofdm_detection_level, ofdmDetCond1Byp_val,
		crs_gain_high_gain_db_val, hi_gain_ofdm_mismatch_val,
		modebphyULB_val, disableIQswapULB_val, rssi_gain_val;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(wlc)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	lcn20phy_lna_params_t *lna_params = pi->u.pi_lcn20phy->lna_params;

	dsss_detection_level = 97;
	ofdm_detection_level = 97;
	ofdmDetCond1Byp_val = 0;
	crs_gain_high_gain_db_val = lna_params->crs_gain_high_gain_db_40mhz;
	hi_gain_ofdm_mismatch_val = lna_params->crs_gain_high_gain_db_40mhz+3;
	modebphyULB_val = 0;
	disableIQswapULB_val = 1;
	rssi_gain_val = lna_params->rssi_gain;

	switch (ulb_mode_set) {
	case PMU_ULB_BW_NONE :
		/* use defalut value defined before switch */
		break;

	case PMU_ULB_BW_10MHZ :
		dsss_detection_level = 100;
		ofdm_detection_level = 98;
		ofdmDetCond1Byp_val = 1;
		crs_gain_high_gain_db_val = lna_params->crs_gain_high_gain_db_40mhz+3;
		hi_gain_ofdm_mismatch_val = lna_params->crs_gain_high_gain_db_40mhz+6;
		modebphyULB_val = 1;
		rssi_gain_val = lna_params->rssi_gain + 3;
		break;

	case PMU_ULB_BW_5MHZ :
		dsss_detection_level = 103;
		ofdm_detection_level = 101;
		ofdmDetCond1Byp_val = 1;
		crs_gain_high_gain_db_val = lna_params->crs_gain_high_gain_db_40mhz+6;
		hi_gain_ofdm_mismatch_val = lna_params->crs_gain_high_gain_db_40mhz+9;
		modebphyULB_val = 1;
		rssi_gain_val = lna_params->rssi_gain + 6;
		/* In large frequency offset case, disableIQswapULB may need to set to 0 */
		/* disableIQswapULB_val = 0; */
		break;

	default:
		ASSERT(FALSE);
		ulb_mode_set = PMU_ULB_BW_NONE;
		break;
	}

	wlc_lcn20phy_detection_level(pi, dsss_detection_level, ofdm_detection_level);
	PHY_REG_MOD(pi, LCN20PHY, detcondbypass20_new, ofdmDetCond1Byp, ofdmDetCond1Byp_val);
	PHY_REG_MOD(pi, LCN20PHY, agcControl12, crs_gain_high_gain_db_40mhz,
		crs_gain_high_gain_db_val);
	PHY_REG_MOD(pi, LCN20PHY, agcControl11, rssi_gain, rssi_gain_val);
	PHY_REG_MOD(pi, LCN20PHY, ClipCtrDefThresh, hi_gain_ofdm_mismatch,
		hi_gain_ofdm_mismatch_val);
	PHY_REG_MOD(pi, LCN20PHY, bphyBBConfig, modebphyULB, modebphyULB_val);
	PHY_REG_MOD(pi, LCN20PHY, bphyBBConfig, disableIQswapULB, disableIQswapULB_val);
	PHY_REG_MOD(pi, LCN20PHY, UlbCtrl, ulb_mode, ulb_mode_set);
	si_pmu_set_ulbmode(pi->sh->sih, pi->sh->osh, ulb_mode_set);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);
}
#endif /* WL11ULB */

void
wlc_phy_chanspec_set_lcn20phy(phy_info_t *pi, chanspec_t chanspec)
{
	uint8 channel = CHSPEC_CHANNEL(chanspec); /* see wlioctl.h */
	bool suspend;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_lcn20phy_bandset(pi);

	wlc_lcn20phy_deaf_mode(pi, TRUE);

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

#ifdef WL11ULB
	pi->u.pi_lcn20phy->ulb_mode = PMU_ULB_BW_NONE;
#endif /* WL11ULB */

	/* Tune radio for the channel */
	if (!NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn20phy_radio20692_channel_tune(pi, channel);
	}

	wlc_phy_chanspec_radio_set((wlc_phy_t *)pi, chanspec);

	wlc_lcn20phy_tx_init(pi, channel);

	wlc_lcn20phy_rx_init(pi, channel);

	/* tune alpf settings for LCN20 radio */
	if (!NORADIO_ENAB(pi->pubpi)) {
		if (pi->low_power_mode) { /* low power mode */
			wlc_lcn20phy_radio20692_lpf_tx_set(pi, 0, 2, 0, 0);
		} else { /* regular mode */
			wlc_lcn20phy_radio20692_lpf_tx_set(pi, 2, 2, 2, 1);
		}
	}

	/* Perform all PHY cals only if no scan in progress */
	if (!SCAN_RM_IN_PROGRESS(pi) && !NORADIO_ENAB(pi->pubpi) &&
		!VSDB_CHANCHG_IN_PROGRESS(pi)) {

		wlc_lcn20phy_calib_modes(pi,
			LCN20PHY_CALMODE_DC |
			LCN20PHY_CALMODE_XTALPN |
			LCN20PHY_CALMODE_TXIQLO |
			LCN20PHY_CALMODE_RXIQ |
			LCN20PHY_CALMODE_DSSF |
			LCN20PHY_CALMODE_PAPD);
	} else
		wlc_lcn20phy_calib_modes(pi, LCN20PHY_CALMODE_DC);

#ifdef WL11ULB
	if (PHY_ULB_ENAB(pi->sh->physhim)) {
		if (CHSPEC_IS10(chanspec))
			pi->u.pi_lcn20phy->ulb_mode = PMU_ULB_BW_10MHZ;
		else if (CHSPEC_IS5(chanspec))
			pi->u.pi_lcn20phy->ulb_mode = PMU_ULB_BW_5MHZ;
	}

	wlc_phy_ulb_mode_lcn20phy(pi, pi->u.pi_lcn20phy->ulb_mode);
#endif /* WL11ULB */

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	wlc_lcn20phy_deaf_mode(pi, FALSE);

	/* Initialize power control */
	wlc_lcn20phy_txpwrctrl_init(pi);

	/* Additions to address fsm hangs */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, hg_gate_ofdm_filt_en, 1);
	PHY_REG_MOD(pi, LCN20PHY, pktfsmctrl, dmdStExitEn_alt, 1);
	PHY_REG_MOD(pi, LCN20PHY, watchdog_en, global_en, 1);
	PHY_REG_MOD(pi, LCN20PHY, watchdog_pktfsm_timeout_CRS_STR1, thr, 10);
	PHY_REG_MOD(pi, LCN20PHY, watchdog_agcfsm_timeout_SS_PACKET_RX_STATE, thr, 4095);
}

/* END: SET CHANNEL above functions are part of set channel funtionality */

void
wlc_phy_txpower_recalc_target_lcn20phy(phy_info_t *pi)
{
	uint16 pwr_ctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	int8 target_pwr_reg;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);
	wlc_lcn20phy_txpower_recalc_target_2pwr(pi);

	target_pwr_reg = wlc_lcn20phy_get_target_tx_pwr(pi);
	if ((target_pwr_reg > pi->tx_power_min_per_core[0]) &&
		(!pi_lcn20->offset_targetpwr)) {
			PHY_FATAL_ERROR_MESG(("target_pwr_reg = %d, ppr_min = %d\n",
				target_pwr_reg, pi->tx_power_min_per_core[0]));
			PHY_FATAL_ERROR(pi, PHY_RC_TXPOWER_LIMITS);
			wlc_lcn20phy_set_target_tx_pwr(pi, pi->tx_power_min_per_core[0]);
	}

	/* Restore power control */
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, pwr_ctrl);
}

#if defined(BCMDBG) || defined(WLTEST)
static int
wlc_phy_long_train_lcn20phy(phy_info_t *pi, int channel)
{
	BCM_REFERENCE(channel);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	return 0;
}
#endif /* defined(BCMDBG) || defined(WLTEST) */

static bool
wlc_lcn20phy_cal_reqd(phy_info_t *pi)
{
	uint time_since_last_cal = (pi->sh->now >= pi->phy_lastcal)?
		(pi->sh->now - pi->phy_lastcal):
		(((uint)~0) - pi->phy_lastcal + pi->sh->now);

	return (time_since_last_cal >= pi->sh->glacial_timer);
}

static bool
wlc_phy_watchdog_lcn20phy(phy_wd_ctx_t *ctx)
{
	phy_info_t *pi = (phy_info_t *) ctx;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (wlc_lcn20phy_cal_reqd(pi)) {
		if (!(SCAN_RM_IN_PROGRESS(pi) || PLT_INPROG_PHY(pi) ||
			ASSOC_INPROG_PHY(pi) || pi->carrier_suppr_disable ||
			(pi->measure_hold & PHY_HOLD_FOR_PKT_ENG) || pi->disable_percal ||
			(wlc_lcn20phy_eu_edcrs_detect(pi)))) {
			wlc_lcn20phy_calib_modes(pi,
				LCN20PHY_CALMODE_DC |
				LCN20PHY_CALMODE_XTALPN |
				LCN20PHY_CALMODE_TXIQLO |
				LCN20PHY_CALMODE_RXIQ |
				LCN20PHY_CALMODE_DSSF |
				LCN20PHY_CALMODE_PAPD);
		}
	}

	return TRUE;
}

#if defined(WLTEST)
static void
wlc_phy_carrier_suppress_lcn20phy(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
}
#endif // endif

void
wlc_lcn20phy_anacore(phy_info_t *pi, bool on)
{
	BCM_REFERENCE(on);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
}

void
wlc_lcn20phy_switch_radio(phy_info_t *pi, bool on)
{
	BCM_REFERENCE(on);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
}

void
wlc_lcn20phy_write_table(phy_info_t *pi, const phytbl_info_t *pti)
{
	uint16 saved_reg = 0;
	uint16 save_AfeCtrlOvr1Val = 0;
	uint16 save_AfeCtrlOvr1 = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT((pti->tbl_width == 8) || (pti->tbl_width == 16) ||
		(pti->tbl_width == 32));

	saved_reg = phy_utils_read_phyreg(pi, LCN20PHY_RxFeCtrl1);
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 1);

	if (pti->tbl_id == LCN20PHY_TBL_ID_EPSILON) {
		save_AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
		save_AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);

		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
	}

	phy_utils_write_phytable_ext(pi, pti, LCN20PHY_TableID,
		LCN20PHY_TableOffset, LCN20PHY_TableOffset,
		LCN20PHY_TableDataHi, LCN20PHY_TableDataLo);

	if (pti->tbl_id == LCN20PHY_TBL_ID_EPSILON) {
		WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
		WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, save_AfeCtrlOvr1);
	}

	phy_utils_write_phyreg(pi, LCN20PHY_RxFeCtrl1, saved_reg);
}

void
wlc_lcn20phy_read_table(phy_info_t *pi, phytbl_info_t *pti)
{
	uint16 saved_reg = 0;
	uint16 save_AfeCtrlOvr1Val = 0;
	uint16 save_AfeCtrlOvr1 = 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	ASSERT((pti->tbl_width == 8) || (pti->tbl_width == 16) ||
		(pti->tbl_width == 32));

	saved_reg = phy_utils_read_phyreg(pi, LCN20PHY_RxFeCtrl1);
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 1);

	if (pti->tbl_id == LCN20PHY_TBL_ID_EPSILON) {
		save_AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
		save_AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);

		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
	}

	phy_utils_read_phytable_ext(pi, pti, LCN20PHY_TableID, LCN20PHY_TableOffset,
		LCN20PHY_TableOffset, LCN20PHY_TableDataHi, LCN20PHY_TableDataLo);

	if (pti->tbl_id == LCN20PHY_TBL_ID_EPSILON) {
		WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
		WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, save_AfeCtrlOvr1);
	}

	phy_utils_write_phyreg(pi, LCN20PHY_RxFeCtrl1, saved_reg);
}

/* return the status of energy-detect(ED)/carrier-sense(CRS) */
static bool
wlc_lcn20phy_eu_edcrs_detect(phy_info_t *pi)
{
	bool suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	bool edcrs = FALSE;
	/* Empirically it is determined that in the present of energy,
	* the debug register showed loop count of 50% consistently.
	* Hence the percentage is set to 50
	*/
	uint loop_count = 50, percentage = 50, edcrs_high_count = 0;
	uint16 reg_val;
	uint8 j = 0;
	uint8 region_group = wlc_phy_get_locale(pi->rxgcrsi);

	/* ETSI 50mS backoff requirement is for EU region only */
	if (region_group != REGION_EU)
		return edcrs;

	/* suspend mac if haven't done so */
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	phy_utils_phyreg_enter(pi);

	/* Forcing WLAN antenna and priority is required to get a
	* valid status in stMcDebugReg2 register
	*/
	wlc_btcx_override_enable(pi);

	/* Check EDCRS a few times to decide if the medium is busy */
	/* If medium is busy, skip phy cal this time around */
	for (j = 0; j < loop_count; j++) {
		reg_val =
			(PHY_REG_READ(pi, LCN20PHY, stMcDebugReg2, reg2)
			& 0x10);

		if ((reg_val) > 0) {
			edcrs_high_count++;
		}
	}

	if (100*edcrs_high_count > (loop_count*percentage))
		edcrs = TRUE;

	wlc_phy_btcx_override_disable(pi);
	phy_utils_phyreg_exit(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return edcrs;
}

void
wlc_lcn20phy_calib_modes(phy_info_t *pi, uint mode)
{
	bool suspend;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint16 cts_time_us;
	uint16 swval_tx, swval_rx, swval_mask;
	uint32 *swctrlmap;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	swctrlmap = pi_lcn20->swctrlmap_2g;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	suspend =
		!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (suspend) {
		wlapi_enable_mac(pi->sh->physhim);
	}

#ifdef WL11ULB
		/* always switch to 20MHz BW mode for calibration */
		wlc_phy_ulb_mode_lcn20phy(pi, PMU_ULB_BW_NONE);
#endif /* WL11ULB */

	/* Save the converged base index to restore during power control ON phase */
	pi->u.pi_lcn20phy->tssi_idx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	pi->u.pi_lcn20phy->cck_tssi_idx =
		PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusExtCCK, baseIndexCCK)>>1;

	/* Total budget of PHY_FULLCAL = (dccal+iqlo+rxiq+idletssi+papd+xtalpn+margin) */
	/* The split = (0.15+4.4+3.8+0.5+4.8+5+1) ~ 20ms. There is scope to reduce this */
	/* to 15 ms by removing the xtal pn cal duration of 5 ms when it is disabled */

	if (!(SCAN_RM_IN_PROGRESS(pi) || VSDB_CHANCHG_IN_PROGRESS(pi)) &&
		(mode != LCN20PHY_CALMODE_DC))
		cts_time_us = 20000;
	else
		cts_time_us = 250;

	/* Set non-zero duration for CTS-to-self */
	wlapi_bmac_write_shm(pi->sh->physhim, M_CTS_DURATION(pi), cts_time_us);
	wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_btcx_override_enable(pi);

	if (!(SCAN_RM_IN_PROGRESS(pi) || VSDB_CHANCHG_IN_PROGRESS(pi)))
		pi->phy_lastcal = pi->sh->now;

	/* All cals */
	if ((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_DC))
		wlc_lcn20phy_dccal_force_cal(pi,
		(LCN20PHY_DCCALMODE_DCOE | LCN20PHY_DCCALMODE_IDACC), 10);

	if (((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_XTALPN)) &&
		(pi->u.pi_lcn20phy->xtalpn_nv > 0))
		wlc_lcn20phy_xtalpn_cal(pi);

	if ((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_TXIQLO))
		wlc_lcn20phy_tx_iqlo_cal_txpwr(pi);

	if ((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_RXIQ))
		wlc_lcn20phy_rx_iq_cal(pi);

	if (((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_DSSF)) &&
		pi->u.pi_lcn20phy->dssfmode)
		wlc_lcn20phy_dssf_cal(pi);

	wlc_lcn20phy_idle_tssi_est(pi);
	wlc_btcx_override_enable(pi);

#ifdef LCN20_PAPD_ENABLE
	if ((mode == PHY_FULLCAL) || (mode & LCN20PHY_CALMODE_PAPD)) {
		if (pi->u.pi_lcn20phy->do_papd_calidx_est) {
			PHY_PAPD(("--- Idle tssi : %d  \n",
				PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0)));

			PHY_PAPD(("--- papd idx estimate params:(base:%d, target:%d)\n",
				pi->u.pi_lcn20phy->base_pwr_dbm,
				pi->u.pi_lcn20phy->target_pwr_dbm));

			pi->u.pi_lcn20phy->papd_cal_idx =
				wlc_phy_papd_calidx_estimate_lcn20(pi);

			if (pi->u.pi_lcn20phy->logain_NBcal) {
				pi->u.pi_lcn20phy->papd_cal_idx +=
					pi->u.pi_lcn20phy->papdcalidxoffset;
			} else {
			/* Applying offset to PAPD cal index from the
		         * nvram parameters for channel 12/13  when logain_NBcal is not set.
			*/
				if (channel == 12) {
					pi->u.pi_lcn20phy->papd_cal_idx  +=
						pi->u.pi_lcn20phy->papdidx_offset_chan12;
				} else if (channel == 13) {
					pi->u.pi_lcn20phy->papd_cal_idx  +=
						pi->u.pi_lcn20phy->papdidx_offset_chan13;
				}
			}

			PHY_PAPD(("--- papd index estimate result: %d\n",
				pi->u.pi_lcn20phy->papd_cal_idx));
		}
		wlc_phy_papd_cal_run_lcn20(pi);
	}
#endif /* LCN20_PAPD_ENABLE */

#ifdef WL11ULB
		/* because calibration were performed in 20MHz BW mode,
		*   need switch back to ULB mode here if applicable
		*/
		wlc_phy_ulb_mode_lcn20phy(pi, pi->u.pi_lcn20phy->ulb_mode);
#endif /* WL11ULB */

	wlc_phy_btcx_override_disable(pi);
	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi->u.pi_lcn20phy->lcn20_twopwr_txpwrctrl_en) {
		/* Restoring scheme value to 2, it was set to 0 by idle_tssi_est() above */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 2);
	}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */
	swval_tx = (uint16) (swctrlmap[LCN20PHY_I_WL_TX] & 0xff);
	swval_rx = (uint16) (swctrlmap[LCN20PHY_I_WL_RX] & 0xff);
	swval_mask = swval_tx | swval_rx;
	/* Toggle the switch ctrl lines to TX and RX states to reset the eLNA if stuck
	 * in bypass mode to normal high gain mode.
	 */
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr, swCtrl_ovr, swval_mask);
	/* Toggle to Tx state using swctrl value read from nvram entry */
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr_val, swCtrl_ovr_val, swval_tx);
	OSL_DELAY(5);
	/* Toggle to Rx state using swctrl value read from nvram entry */
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr_val, swCtrl_ovr_val, swval_rx);
	OSL_DELAY(5);
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr_val, swCtrl_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr, swCtrl_ovr, 0);
}

/* ----START------- LPC feature macros and functions ------------ */
#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
#define LPC_MIN_IDX 19

#define LPC_TOT_IDX (LPC_MIN_IDX + 1)
#define LCN20PHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES	64
#define LCN20PHY_TX_PWR_CTRL_MACLUT_WIDTH	8

#ifdef WL_LPC
static uint8 lpc_pwr_level[LPC_TOT_IDX] =
	{0, 2, 4, 6, 8, 10,
	12, 14, 16, 18, 20,
	22, 24, 26, 28, 30,
	32, 34, 36, 38};
#endif /* WL_LPC */

static void
wlc_lcn20phy_lpc_write_maclut(phy_info_t *pi)
{
	phytbl_info_t tab;

#if defined(LP_P2P_SOFTAP)
	uint8 i;
	uint8 pwr_lvl_qdB[LCN20PHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES];

	/* Assign values from 0 to 63 qdB for now */
	for (i = 0; i < LCN20PHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES; i++)
		pwr_lvl_qdB[i] = i;
	tab.tbl_ptr = pwr_lvl_qdB;
	tab.tbl_len = LCN20PHY_TX_PWR_CTRL_MACLUT_MAX_ENTRIES;

#elif defined(WL_LPC)

	/* If not enabled, no need to clear out the table, just quit */
	if (!pi->lpc_algo)
		return;
	tab.tbl_ptr = lpc_pwr_level;
	tab.tbl_len = LPC_TOT_IDX;
#endif /* WL_LPC */

	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = LCN20PHY_TX_PWR_CTRL_MACLUT_WIDTH;
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_MAC_OFFSET;

	/* Write to it */
	wlc_lcn20phy_write_table(pi, &tab);
}

#endif /* WL_LPC || LP_P2P_SOFTAP */

/* ----END------- LPC feature macros and functions ------------ */

/* BEGIN : TxPwrCtrl Below functions are part of Tx Power Control funtionality */

static void
wlc_lcn20phy_set_pa_gain(phy_info_t *pi, uint16 gain)
{
	PHY_REG_MOD(pi, LCN20PHY, txgainctrlovrval1, pagain_ovr_val1, gain);
}

static void
wlc_lcn20phy_set_dac_gain(phy_info_t *pi, uint16 dac_gain)
{
	uint16 dac_ctrl;

	dac_ctrl = (phy_utils_read_phyreg(pi, LCN20PHY_AfeDACCtrl)
		>> LCN20PHY_AfeDACCtrl_dac_ctrl_SHIFT);
	dac_ctrl = dac_ctrl & ~(LCN20PHY_DACGAIN_MASK);
	dac_ctrl = dac_ctrl | (dac_gain << 7);
	PHY_REG_MOD(pi, LCN20PHY, AfeDACCtrl, dac_ctrl, dac_ctrl);
}

static uint16
wlc_lcn20phy_get_pa_gain(phy_info_t *pi)
{
	uint16 pa_gain;

	pa_gain = (phy_utils_read_phyreg(pi, LCN20PHY_txgainctrlovrval1) &
		LCN20PHY_txgainctrlovrval1_pagain_ovr_val1_MASK) >>
		LCN20PHY_txgainctrlovrval1_pagain_ovr_val1_SHIFT;

	return pa_gain;
}

void
wlc_lcn20phy_set_tx_gain(phy_info_t *pi,  phy_txgains_t *target_gains)
{
	uint16 pa_gain = wlc_lcn20phy_get_pa_gain(pi);

	PHY_REG_MOD(pi, LCN20PHY, txgainctrlovrval0, txgainctrl_ovr_val0,
		(target_gains->gm_gain) | (target_gains->pga_gain << 8));

	PHY_REG_MOD(pi, LCN20PHY, txgainctrlovrval1, txgainctrl_ovr_val1,
		(target_gains->pad_gain) | (pa_gain << 8));

	wlc_lcn20phy_set_dac_gain(pi, target_gains->dac_gain);
	/* Enable gain overrides */
	wlc_lcn20phy_enable_tx_gain_override(pi);
}

static void
wlc_lcn20phy_force_pwr_index(phy_info_t *pi, int indx)
{
	phytbl_info_t tab;
	uint16 a, b;
	uint8 bb_mult;
	uint32 bbmult_dac, txgain, locoeffs, iqcomp;
	phy_txgains_t gains;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

#if defined(PHYCAL_CACHING)
	ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	lcnphy_calcache_t *cache = NULL;

	if (ctx)
		cache = &ctx->u.lcnphy_cache;
#endif // endif

	ASSERT(indx <= LCN20PHY_MAX_TX_POWER_INDEX);

	/* Save forced index */
	pi_lcn20->tx_power_idx_override = (int8)indx;
	pi_lcn20->current_index = (uint8)indx;

	/* Preset txPwrCtrltbl */
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */

	/* Read index based bb_mult, a, b from the table */
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_PWR_OFFSET + indx; /* iqCoefLuts */
	tab.tbl_ptr = &bbmult_dac; /* ptr to buf */
	wlc_lcn20phy_read_table(pi,  &tab);

	/* Read index based tx gain from the table */
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_GAIN_OFFSET + indx; /* gainCtrlLuts */
	tab.tbl_width = 32;
	tab.tbl_ptr = &txgain; /* ptr to buf */
	wlc_lcn20phy_read_table(pi,  &tab);
	/* Apply tx gain */
	gains.gm_gain = (uint16)(txgain & 0x1f);
	gains.pga_gain = (uint16)(txgain >> 5) & 0xff;
	gains.pad_gain = (uint16)(txgain >> 13) & 0xff;
	gains.dac_gain = (uint16)(bbmult_dac >> 17) & 0xf;
	wlc_lcn20phy_set_tx_gain(pi, &gains);
	wlc_lcn20phy_set_pa_gain(pi,  (uint16)(txgain >> 21) & 0xff);
	/* Apply bb_mult */
	bb_mult = (uint8)((bbmult_dac >> 9) & 0xff);
	wlc_lcn20phy_set_bbmult(pi, bb_mult);
	/* Apply PAPD rf power correction */
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0,
		gain_dac_rf_override, 1);
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0,
		gain_dac_rf_reg, (bbmult_dac & 0x1ff));
	/* Enable gain overrides */
	wlc_lcn20phy_enable_tx_gain_override(pi);
	/* the reading and applying lo, iqcc coefficients is not getting done for 4313A0 */
	/* to be fixed */

	/* Apply iqcc */
	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;	/* 32 bit wide	*/
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_IQ_OFFSET; /* iqCoefLuts */
	tab.tbl_ptr = &iqcomp; /* ptr to buf */
	wlc_lcn20phy_read_table(pi,  &tab);
	a = (uint16)((iqcomp >> 10) & 0x3ff);
	b = (uint16)(iqcomp & 0x3ff);

	PHY_INFORM(("\n iqcomp : 0x%x a: 0x%x b: 0x%x\n", iqcomp, a, b));

#if defined(PHYCAL_CACHING)
	if (ctx && cache->txiqlocal_a[0]) {
#else
	/* if (pi_lcn20->lcnphy_cal_results.txiqlocal_a[0]) { */
	{
#endif /* defined(PHYCAL_CACHING) */
		wlc_phy_set_tx_iqcc_lcn20phy(pi, a, b);
	/* Read index based di & dq from the table */
	}
#if defined(PHYCAL_CACHING)
	if (ctx && cache->txiqlocal_didq[0]) {
#else
	/* if (pi_lcn->lcnphy_cal_results.txiqlocal_didq[0]) { */
	{
#endif /* defined(PHYCAL_CACHING) */
		tab.tbl_width = 16; /* 16 bit wide	*/
		tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_LO_OFFSET + indx; /* loftCoefLuts */
		tab.tbl_ptr = &locoeffs; /* ptr to buf */
		wlc_lcn20phy_read_table(pi,  &tab);
		/* Apply locc */
		wlc_phy_set_tx_locc_lcn20phy(pi, (uint16)locoeffs);
	}

#ifdef LCN20_PAPD_ENABLE
	/* Force PAPD to work with Fixed Epsilon offset TODO:
	 * this is a WAR until offset per Tx idx is supported
	 */
	PHY_REG_MOD(pi, LCN20PHY, papd_analog_gain_ovr_val,
		papd_analog_gain_ovr_val, 0);
#endif /* LCN20_PAPD_ENABLE */
}

void
wlc_lcn20phy_set_tx_pwr_by_index(phy_info_t *pi, int indx)
{
	/* Turn off automatic power control */
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* Force tx power from the index */
	wlc_lcn20phy_force_pwr_index(pi, indx);
}

void
wlc_lcn20phy_set_tx_gain_override(phy_info_t *pi, bool bEnable)
{
	uint16 bit = bEnable ? 1 : 0;

	PHY_REG_MOD(pi, LCN20PHY, rfoverride2, txgainctrl_ovr, bit);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr, dacattctrl_ovr, bit);
}

int32
wlc_lcn20phy_tssi2dbm(int32 tssi, int32 a1, int32 b0, int32 b1)
{
	int32 a, b, p;
	/* On lcnphy, estPwrLuts0/1 table entries are in S6.3 format */
	a = 32768 + (a1 * tssi);
	b = (1024 * b0) + (64 * b1 * tssi);
	p = ((2 * b) + a) / (2 * a);

	return p;
}

static void
wlc_lcn20phy_txpower_reset_npt(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	pi_lcn20->tssi_npt = LCN20PHY_TX_PWR_CTRL_START_NPT;
}

/* Wrapper func for txpower_recalc_target() is required to prevent ROM invalidation */
/* from target power */
static void
wlc_lcn20phy_txpower_recalc_target_2pwr(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	wlc_lcn20phy_txpower_recalc_target(pi);

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (pi_lcn20->lcn20_twopwr_txpwrctrl_en) {
		int8 target_pwr = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlTargetPwr, targetPwr0);
		/* RTL calculates intended TXPwr as targetPwr + RateOffset */
		/* While actual intended power is targetPower - RateOffset */
		int8 pmin_new = 2*target_pwr - pi_lcn20->pmax;
		int8 pmax_new = 2*target_pwr - pi_lcn20->pmin;
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlPwrRange2, pwrMax_range2, pmax_new);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlPwrRange2, pwrMin_range2, pmin_new);
	}
#endif /* #if TWO_POWER_RANGE_TXPWR_CTRL */
}

static void
wlc_lcn20phy_txpower_recalc_target(phy_info_t *pi)
{
	phytbl_info_t tab;
	ppr_dsss_rateset_t dsss_limits;
	ppr_ofdm_rateset_t ofdm_limits;
	ppr_ht_mcs_rateset_t mcs_limits;
	uint32 rate_table[WL_RATESET_SZ];
	wl_tx_bw_t bw_mcs = WL_TX_BW_20;
	uint i, j;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	if (pi_lcn20->offset_targetpwr) {
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlTargetPwr,
			targetPwr0, (phy_tpc_get_target_min((wlc_phy_t*)pi) -
			(pi_lcn20->offset_targetpwr * 4)));
		return;
	}

	if (pi->tx_power_offset == NULL)
		return;

	/* Adjust rate based power offset */
	ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss_limits);
	ppr_get_ofdm(pi->tx_power_offset, WL_TX_BW_20, WL_TX_MODE_NONE, WL_TX_CHAINS_1,
		&ofdm_limits);
	ppr_get_ht_mcs(pi->tx_power_offset, bw_mcs, WL_TX_NSS_1,
		WL_TX_MODE_NONE, WL_TX_CHAINS_1, &mcs_limits);

	j = 0;
	for (i = 0; i < WL_RATESET_SZ_DSSS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-dsss_limits.pwr[i]));
		pi_lcn20->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_OFDM; i++, j++) {
		rate_table[j] = (uint32)((int32)(-ofdm_limits.pwr[i]));
		pi_lcn20->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	for (i = 0; i < WL_RATESET_SZ_HT_MCS; i++, j++) {
		rate_table[j] = (uint32)((int32)(-mcs_limits.pwr[i]));
		pi_lcn20->rate_table[j] = rate_table[j];
		PHY_TMP((" Rate %d, offset %d\n", j, rate_table[j]));
	}

	if (!pi_lcn20->uses_rate_offset_table) {
		/* Preset txPwrCtrltbl */
		tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
		tab.tbl_width = 32;	/* 32 bit wide	*/
		tab.tbl_len = ARRAYSIZE(rate_table); /* # values   */
		tab.tbl_ptr = rate_table; /* ptr to buf */
		tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_RATE_OFFSET;
		wlc_lcn20phy_write_table(pi, &tab);
	}

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
	/* Update the MACAddr LUT which is cleared when doing recal */
#ifdef LP_P2P_SOFTAP
	if (pi_lcn20->pwr_offset_val)
#endif // endif
#ifdef WL_LPC
			if (pi->lpc_algo)
#endif /* WL_LPC */
			wlc_lcn20phy_lpc_write_maclut(pi);
#endif /* LP_P2P_SOFTAP || WL_LPC */

	/* Set new target power */
	wlc_lcn20phy_set_target_tx_pwr(pi, phy_tpc_get_target_min((wlc_phy_t*)pi));

	/* Should reset power index cache */
	wlc_lcn20phy_txpower_reset_npt(pi);
}

void
wlc_lcn20phy_set_tx_pwr_ctrl(phy_info_t *pi, uint16 mode)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16 old_mode = wlc_lcn20phy_get_tx_pwr_ctrl(pi);

	ASSERT(
		(LCN20PHY_TX_PWR_CTRL_OFF == mode) ||
		(LCN20PHY_TX_PWR_CTRL_SW == mode) ||
		(LCN20PHY_TX_PWR_CTRL_HW == mode));

	if (old_mode != mode) {
	        /* As pwrctrl is not dependant on txfront end clk, disabling it to save pwr */
	        PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn, 0);
		/* Feed back RF power level to PAPD block */
		PHY_REG_MOD(pi, LCN20PHY, papd_control2, papd_analog_gain_ovr,
			(LCN20PHY_TX_PWR_CTRL_HW == mode) ? 0 : 1);
		/* Setting to use IQ and LO coeffs from HW pwrctrl table accordingly */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, use_txPwrCtrlCoefsIQ,
			(LCN20PHY_TX_PWR_CTRL_HW == mode) ? 1 : 0);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, use_txPwrCtrlCoefsLO,
			(LCN20PHY_TX_PWR_CTRL_HW == mode) ? 1 : 0);

		if (LCN20PHY_TX_PWR_CTRL_HW == mode) {
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, interpol_en, 1);
			/* interpolate using bbshift */
			PHY_REG_MOD(pi, LCN20PHY, bbShiftCtrl, bbshift_mode, 0);
		} else {
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, interpol_en, 0);
			/* interpolate using bbshift */
			PHY_REG_MOD(pi, LCN20PHY, bbShiftCtrl, bbshift_mode, 0);
		}

		if (LCN20PHY_TX_PWR_CTRL_HW == old_mode) {
			/* Clear out all power offsets */
			wlc_lcn20phy_clear_tx_power_offsets(pi);
			phy_utils_write_phyreg(pi, LCN20PHY_BBmultCoeffSel, 0);
		}
		if (LCN20PHY_TX_PWR_CTRL_HW == mode) {
			/* Recalculate target power to restore power offsets */
			wlc_lcn20phy_txpower_recalc_target_2pwr(pi);
			/* Set starting index & NPT to best known values for that target */
			wlc_lcn20phy_set_start_tx_pwr_idx(pi, pi_lcn20->tssi_idx);
			wlc_lcn20phy_set_start_CCK_tx_pwr_idx(pi, pi_lcn20->cck_tssi_idx);
			wlc_lcn20phy_set_tx_pwr_npt(pi, pi_lcn20->tssi_npt);
			phy_utils_write_phyreg(pi, LCN20PHY_BBmultCoeffSel, 1);
			/* Reset frame counter for NPT calculations */
			pi_lcn20->tssi_tx_cnt = PHY_TOTAL_TX_FRAMES(pi);
			/* Disable any gain overrides */
			wlc_lcn20phy_disable_tx_gain_override(pi);
			pi_lcn20->tx_power_idx_override = -1;
			/* Disable PAPD RF gain override */
			PHY_REG_MOD(pi, LCN20PHY, PapdEnable0,
				gain_dac_rf_override, 0);
		}
		else
			wlc_lcn20phy_enable_tx_gain_override(pi);

		/* Set requested tx power control mode */
		phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlCmd,
			(LCN20PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
			LCN20PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
			LCN20PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK),
			mode);

		PHY_INFORM(("wl%d: %s: %s \n", pi->sh->unit, __FUNCTION__,
			mode ? ((LCN20PHY_TX_PWR_CTRL_HW == mode) ? "Auto" : "Manual") : "Off"));
	}
}

int8
wlc_lcn20phy_get_current_tx_pwr_idx(phy_info_t *pi)
{
	int8 indx;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	/* for txpwrctrl_off, return current_index */
	if (txpwrctrl_off(pi))
		indx = pi_lcn20->current_index;
	else
		indx = (int8)(wlc_lcn20phy_get_current_tx_pwr_idx_if_pwrctrl_on(pi)/2);

	return indx;
}

static void
wlc_lcn20phy_set_tssi_mux(phy_info_t *pi, lcn20phy_tssi_mode_t pos)
{
	/* Set TSSI/RSSI mux */
	if (LCN20PHY_TSSI_POST_PA == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 1)
		PHY_REG_LIST_EXECUTE(pi);
	} else if (LCN20PHY_TSSI_EXT_POST_PAD == pos) {
		PHY_REG_LIST_START
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 1)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrl0, tssiRangeVal1, 1)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverride, 1)
			PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride0, tssiRangeOverrideVal,
				0)
		PHY_REG_LIST_EXECUTE(pi);
	} else if (LCN20PHY_TSSI_PRE_PA == pos) {
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal0, 0x1);
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrl0, tssiSelVal1, 0);
	}

	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmdNew, txPwrCtrlScheme, 0);
}

static uint16
wlc_lcn20phy_rfseq_tbl_adc_pwrup(phy_info_t *pi)
{
	uint16 N1, N2, N3, N4, N5, N6, N;

	N1 = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNnum, Ntssi_delay);
	N2 = 1 << PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNnum, Ntssi_intg_log2);
	N3 = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_delay);
	N4 = 1 << PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2);
	N5 = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNum_temp, Ntemp_delay);
	N6 = 1 << PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2);
	N = 2 * (N1 + N2 + N3 + N4 + 2 *(N5 + N6)) + 80;
	if (N < 1600)
		N = 1600; /* min 20 us to avoid tx evm degradation */
	return N;
}

static void
wlc_lcn20phy_pwrctrl_rssiparams(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16 auxpga_gain = pi_lcn20->auxpga_gain;
	uint16 auxpga_vmid = pi_lcn20->auxpga_vmid;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelVmidOverride, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride1, afeAuxpgaSelGainOverride, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 0)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlRfCtrl2,
		LCN20PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_MASK |
		LCN20PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_MASK,
		(auxpga_vmid << LCN20PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelVmidVal0_SHIFT) |
		(auxpga_gain << LCN20PHY_TxPwrCtrlRfCtrl2_afeAuxpgaSelGainVal0_SHIFT));

	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlRfCtrl3,
		LCN20PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_MASK |
		LCN20PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_MASK,
		(auxpga_vmid << LCN20PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelVmidVal1_SHIFT) |
		(auxpga_gain << LCN20PHY_TxPwrCtrlRfCtrl3_afeAuxpgaSelGainVal1_SHIFT));

	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlRfCtrl4,
		LCN20PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_MASK |
		LCN20PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_MASK,
		(auxpga_vmid << LCN20PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelVmidVal2_SHIFT) |
		(auxpga_gain << LCN20PHY_TxPwrCtrlRfCtrl4_afeAuxpgaSelGainVal2_SHIFT));

	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlRfCtrl5,
		LCN20PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelVmidVal3_MASK |
		LCN20PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelGainVal3_MASK,
		(AUXPGA_VBAT_VMID_VAL << LCN20PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelVmidVal3_SHIFT) |
		(AUXPGA_VBAT_GAIN_VAL << LCN20PHY_TxPwrCtrlRfCtrl5_afeAuxpgaSelGainVal3_SHIFT));

	phy_utils_mod_phyreg(pi, LCN20PHY_TxPwrCtrlRfCtrl6,
		LCN20PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_MASK |
		LCN20PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_MASK,
		(AUXPGA_TEMPER_VMID_VAL << LCN20PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelVmidVal4_SHIFT) |
		(AUXPGA_TEMPER_GAIN_VAL << LCN20PHY_TxPwrCtrlRfCtrl6_afeAuxpgaSelGainVal4_SHIFT));
}

void
wlc_lcn20phy_tssi_setup_estPwrLuts(phy_info_t *pi, phytbl_info_t *tab)
{
	uint32 *indxTbl;
	uint8 i = 0;
	if ((indxTbl = (uint32*) LCN20PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* Setup estPwrLuts for measuring idle TSSI */
	tab->tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab->tbl_width = 32; /* 32 bit wide	*/
	tab->tbl_ptr = indxTbl; /* ptr to buf */
	tab->tbl_len = 128;		  /* # values	*/
	tab->tbl_offset = 0;
	for (i = 0; i < 128; i++) {
		*(indxTbl + i) = i;
	}
	wlc_lcn20phy_write_table(pi, tab);
	tab->tbl_offset = LCN20PHY_TX_PWR_CTRL_EST_PWR_OFFSET;
	wlc_lcn20phy_write_table(pi, tab);

	if (indxTbl)
		LCN20PHY_MFREE(pi, indxTbl, 128 * sizeof(uint32));

}

void
wlc_lcn20phy_tssi_setup_cfg_phy(phy_info_t *pi)
{
	int16 power_correction;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlCmd, hwtxPwrCtrl_en, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_en, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, force_vbatTemp, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlCmd, pwrIndex_init, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlCmd, invertTssiSamples, 1)

		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNnum, Ntssi_delay, 300)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNnum, Ntssi_intg_log2, 5)

		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNnum, Npt_intg_log2, 0)

		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4)

		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlDeltaPwrLimit, DeltaPwrLimit, 0x1)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRangeCmd, cckPwrOffset, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, TempSenseCorrection, tempsenseCorr, 0)

		/*  Set idleTssi to (2^9-1) in OB format = (2^9-1-2^8) = 0xff in 2C format */
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlIdleTssi, rawTssiOffsetBinFormat, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0, 0xff)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlIdleTssi1, idleTssi1, 0xff)
	PHY_REG_LIST_EXECUTE(pi);

	if (pi_lcn20->tssical_time) {
		PHY_REG_MOD(pi, LCN20PHY, perPktIdleTssiCtrl, perPktIdleTssiUpdate_en, 1);
		power_correction = pi_lcn20->tempsenseCorr + pi_lcn20->idletssi_corr;
		PHY_REG_MOD(pi, LCN20PHY, TempSenseCorrection,
		tempsenseCorr, power_correction);
	} else
		PHY_REG_MOD(pi, LCN20PHY, perPktIdleTssiCtrl, perPktIdleTssiUpdate_en, 0);

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN20PHY, perPktIdleTssiCtrl, perPktIdleTssi_en, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, perPktIdleTssiCtrl, Nidletssi_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN20PHY, perPktIdleTssiCtrl, Nidletssi_delay, 55)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlIdleTssi2, Nidletssi_delay_cck, 120)

		/*  for CCK average over 40<<0 samples */
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNnumCCK, Ntssi_intg_log2_cck, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNnumCCK, Ntssi_delay_cck, 400)

		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverride, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlRfCtrlOverride0, amuxSelPortOverrideVal, 2)
		/* set DVGA1 gain code (farrow gain during tx) */
		PHY_REG_MOD_ENTRY(LCN20PHY, RxSdFeConfig1, farrow_rshift_tx, 5)
		/* Swap iq:
		* txPwrCtrl_swap_iq is used by RTL in PAPD/IQLO CAL path and
		* tssiADCSel is used for Pwrctrl block
		* Change to 1 as RX farrow i/q is swapped
		*/
		PHY_REG_MOD_ENTRY(LCN20PHY, TSSIMode, tssiADCSel, 1)
	PHY_REG_LIST_EXECUTE(pi);

}

void
wlc_lcn20phy_tssi_setup_cfg_radio(phy_info_t *pi)
{
	wlc_lcn20phy_radio20692_tssisetup(pi);

	/* Disable all overrides for tssi feedback path as they are in direct control
	* JIRA-468: Do not enable overrides for tssi feedback path,
	* direct control takes care of pu/pd
	*/

	/* Disable PU override for AMUX (a.k.a. testbuf) */
	MOD_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, wl_ovr_testbuf_PU, 0x0);

	/* Disable PU override for AUX PGA */
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_pu, 0x0);

	/* Disable pu for post envelope detector tssi blocks */
	MOD_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, wl_ovr_iqcal_PU_tssi, 0x0);

	/* Disable PU override for TSSI envelope detector */
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_pa2g_tssi_ctrl_pu, 0x0);

	/* Disable PU for vbat monitor and tempsense */
	MOD_RADIO_REG_20692(pi, WL_VBAT_MONITOR_OVR1, 0, wl_ovr_vbat_monitor_pu, 0x0);
	MOD_RADIO_REG_20692(pi, WL_TEMP_SENS_OVR1, 0, wl_ovr_tempsense_pu, 0x0);

	/* set auxpga vcm ctrl to default value */
	MOD_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, wl_auxpga_i_vcm_ctrl, 0x0);

	/* ensure that the dac mux is OFF because it
	* shares a line with the auxpga output
	*/
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_txbb_dac2adc, 0x0);
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_txbb_daciso_sw, 0x1);
}

void
wlc_lcn20phy_tssi_setup(phy_info_t *pi)
{
	phytbl_info_t tab;
	uint16 rfseq;

	/* if (!PHY_EPA_SUPPORT(pi_lcn20->ePA)) */
		wlc_lcn20phy_set_tssi_mux(pi, LCN20PHY_TSSI_POST_PA);
	/* else
	* 	wlc_lcn20phy_set_tssi_mux(pi, LCN20PHY_TSSI_EXT);
	*/

	wlc_lcn20phy_tssi_setup_cfg_phy(pi);
	wlc_lcn20phy_clear_tx_power_offsets(pi);

	if (0) {
		rfseq = wlc_lcn20phy_rfseq_tbl_adc_pwrup(pi);

		tab.tbl_id = LCN20PHY_TBL_ID_RFSEQ;
		tab.tbl_width = 16;	/* 13 bit wide	*/
		tab.tbl_ptr = &rfseq;
		tab.tbl_len = 1;
		tab.tbl_offset = 6;
		wlc_lcn20phy_write_table(pi,  &tab);
	}

	if (!NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn20phy_tssi_setup_cfg_radio(pi);
	}

	/* set envelope detector gain */
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverride, 1);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRfCtrlOverride0, paCtrlTssiOverrideVal, 1);

	wlc_lcn20phy_pwrctrl_rssiparams(pi);

	/* disable override  */
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 0);
}

uint8
wlc_lcn20phy_get_bbmult(phy_info_t *pi)
{
	uint16 m0m1;
	phytbl_info_t tab;

	tab.tbl_ptr = &m0m1;
	tab.tbl_len = 1;
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL;
	tab.tbl_offset = 99;
	tab.tbl_width = 8;
	wlc_lcn20phy_read_table(pi, &tab);

	return (uint8)(m0m1);
}

void
wlc_lcn20phy_get_tx_gain(phy_info_t *pi, phy_txgains_t *gains)
{
	uint16 dac_gain;

	dac_gain = phy_utils_read_phyreg(pi, LCN20PHY_AfeDACCtrl) >>
		LCN20PHY_AfeDACCtrl_dac_ctrl_SHIFT;
	gains->dac_gain = (dac_gain & LCN20PHY_DACGAIN_MASK) >> 7;

	{
		uint16 rfgain0, rfgain1;

		rfgain0 = (phy_utils_read_phyreg(pi, LCN20PHY_txgainctrlovrval0) &
			LCN20PHY_txgainctrlovrval0_txgainctrl_ovr_val0_MASK) >>
			LCN20PHY_txgainctrlovrval0_txgainctrl_ovr_val0_SHIFT;
		rfgain1 = (phy_utils_read_phyreg(pi, LCN20PHY_txgainctrlovrval1) &
			LCN20PHY_txgainctrlovrval1_txgainctrl_ovr_val1_MASK) >>
			LCN20PHY_txgainctrlovrval1_txgainctrl_ovr_val1_SHIFT;

		gains->gm_gain = rfgain0 & 0x1f;
		gains->pga_gain = (rfgain0 >> 8) & 0xff;
		gains->pad_gain = rfgain1 & 0xff;
	}
}

/* Convert tssi to power LUT */
void
wlc_lcn20phy_set_estPwrLUT(phy_info_t *pi, int32 lut_num)
{
	phytbl_info_t tab;
	int32 tssi;
	uint32 *pwr_table = NULL;
	int32 a1 = 0, b0 = 0, b1 = 0;
	int16 m = 0, i = 0;
	int32 pwr0 = 0; //init for 1st estPwr anchor point
	int32 pwr1 = 0; //2nd estPwr anchor point 0dBm
	uint32 tssi0 = LCN20PHY_INIT_ANCRPOINT; //default for 1st tssi anchor point
	int8 found_tssi = 0;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	if ((pwr_table = (uint32*) LCN20PHY_MALLOC(pi, LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ
		* sizeof(*pwr_table))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}
	if (lut_num == 0) {
		/* Get the PA params for the particular channel we are in */
		phy_tpc_get_paparams_for_band(pi, &a1, &b0, &b1);
		 /* estPwrLuts */
		tab.tbl_offset = 0;
#if TWO_POWER_RANGE_TXPWR_CTRL
	} else if (pi_lcn20->lcn20_twopwr_txpwrctrl_en) {
		b0 = pi->txpa_2g_2pwr[0];
		b1 = pi->txpa_2g_2pwr[1];
		a1 = pi->txpa_2g_2pwr[2];
		/* estPwrLuts1 */
		tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_EST_PWR_OFFSET;
#endif // endif
	}

	/* Fix for power runaway to idx=127 is only applied for Normal power range */
	/* lut_num=0 is for normal pwr range and pi->low_power_mode=1 only in Low power mode */
	if ((lut_num == 0) && (!pi->low_power_mode)) {
		/* Normal pwr range scheme and the Normal(high) pwr range of 2pwr range scheme */
		for (tssi = LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ-1; tssi >= 0; tssi--) {
			*(pwr_table + tssi) = wlc_lcn20phy_tssi2dbm(tssi, a1, b0, b1);
			/* To prevent power runaway to idx=127 at low target powers, */
			/* interpolate lower power values till 0dBm */
			/* find 1st tssi anchor point */
			if ((tssi <= LCN20PHY_INIT_ANCRPOINT) && (*(pwr_table + tssi) >=
				LCN20PHY_PWR_THRESHOLD) && (!found_tssi)) {
					tssi0 = tssi;
					found_tssi = 1;
			}
		}
		/* Interpolate lower power values between (tssi0,pwr0) to (127,pwr1) in estPwrLUT */
		pwr0 = *(pwr_table + tssi0);  //1st estPwr anchor point
		/* slope of the line between (tssi0,pwr0) to (127,pwr1) */
		//left shift done to keep fraction values
		m = ((pwr1-pwr0) << LCN20PHY_FRAC_SHIFT)/(127-tssi0);
		/* overwrite low power estPwr entries with interpolated values */
		for (i = tssi0+1; i < LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ; i++) {
			//Right shift done to undo left shift
			*(pwr_table + i) = pwr0+((m*(i-tssi0))>> LCN20PHY_FRAC_SHIFT);
		}
	} else {
		/* Low power range of 2 pwr range scheme and Low power mode */
		for (tssi = 0; tssi < 128; tssi++) {
			*(pwr_table + tssi) = wlc_lcn20phy_tssi2dbm(tssi, a1, b0, b1);
		}
	}

	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;
	tab.tbl_ptr = pwr_table;
	tab.tbl_len = LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ;
	wlc_lcn20phy_write_table(pi,  &tab);

	if (pwr_table) {
		LCN20PHY_MFREE(pi, pwr_table, LCN20PHY_TX_PWR_CTRL_ESTPWRLUT_SZ
		* sizeof(*pwr_table));
	}
}

/* END: TxPwrCtrl Below functions are part of Tx Power Control funtionality */

/* BEGIN: INIT Below functions are part of PHY/RADIO initialization functionality  */

static void
wlc_lcn20phy_phy_and_radio_reset(phy_info_t *pi)
{
	if (NORADIO_ENAB(pi->pubpi))
		return;

	phy_utils_or_phyreg(pi, LCN20PHY_resetCtrl, 0x100);
	phy_utils_and_phyreg(pi, LCN20PHY_resetCtrl, ~0x100);

}

static void wlc_lcn20phy_load_rx_gain_tables(phy_info_t *pi)
{
	phytbl_info_t tab;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	tab.tbl_id = LCN20PHY_TBL_ID_GAINVAL;
	tab.tbl_width = 8;
	tab.tbl_ptr = pi_lcn20->gainvaltbl;
	tab.tbl_len = LCN20PHY_RXGAINVAL_TBL_SZ;
	tab.tbl_offset = 0;
	wlc_lcn20phy_write_table(pi, &tab);

	tab.tbl_id = LCN20PHY_TBL_ID_GAIN;
	tab.tbl_width = 32;
	tab.tbl_ptr = pi_lcn20->gaintbl;
	tab.tbl_len = LCN20PHY_RXGAIN_TBL_SZ;
	wlc_lcn20phy_write_table(pi, &tab);

	tab.tbl_id = LCN20PHY_TBL_ID_GAINIDX;
	tab.tbl_width = 32;
	tab.tbl_ptr = pi_lcn20->gainidxtbl;
	tab.tbl_len = LCN20PHY_RXGAINIDX_TBL_SZ;
	wlc_lcn20phy_write_table(pi, &tab);
}

static void wlc_lcn20phy_load_tx_gain_table(phy_info_t *pi,
	const lcn20phy_tx_gain_tbl_entry * gain_table)
{
	uint32 j, k, min_txpwrindex;
	phytbl_info_t tab;
	uint32 val;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	uint32* val_array = NULL;

	tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
	tab.tbl_width = 32;     /* 32 bit wide  */
	tab.tbl_len = 1;        /* # values   */
	tab.tbl_ptr = &val; /* ptr to buf */

	if ((val_array = (uint32 *)LCN20PHY_MALLOC(pi, 128 * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	min_txpwrindex = pi_lcn20->min_txpwrindex_2g;

	for (j = 0; j < 128; j++) {

		if (j <= min_txpwrindex)
			k = min_txpwrindex;
		else
			k = j;

		val = (((uint32)gain_table[k].ipa << 21) |
			(gain_table[k].casc << 13) |
			(gain_table[k].mix << 5) | gain_table[k].gm);

		*(val_array + j) = val;
	}
	tab.tbl_ptr = val_array;
	tab.tbl_len = 128;
	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_GAIN_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);

	for (j = 0; j < 128; j++) {

		if (j <= min_txpwrindex)
			k = min_txpwrindex;
		else
			k = j;

		val = (gain_table[k].dac_attn << 17) |
			(gain_table[k].bb_mult << 9) |
			(gain_table[k].rf_power);

		*(val_array + j) = val;
	}

	tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_PWR_OFFSET;
	wlc_lcn20phy_write_table(pi, &tab);
	pi->u.pi_lcn20phy->txgaintable = (CONST lcn20phy_tx_gain_tbl_entry *)gain_table;

	if (val_array)
		LCN20PHY_MFREE(pi, val_array, 128 * sizeof(uint32));

}

#ifdef LCN20_PAPD_ENABLE
static void
WLBANDINITFN(lcn20phy_papd_tbl_init)(phy_info_t *pi)
{
	uint8 j;
	uint8 initvalue[LCN20_PAPD_TBL_WRITE_STEP_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	PHY_PAPD(("\n PAPD Tables init \n"));

	/* lcn20_clear_eps_tbl */
	for (j = 0; j < LCN20_PAPD_EPS_TBL_SIZE; j++)
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_EPSILON, initvalue, 1, 32, j);

	/* fill the scalar table */
	wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_SCALAR, lcn20_papd_scaltbl,  64, 32, 0);
	/* Write papd lut select table used in txpwrctrl */
	/* use lut0 for all tx gain indices and rates */
	for (j = 0; j < LCN20_PAPD_LUT_SELECT_TBL_SIZE; j += LCN20_PAPD_TBL_WRITE_STEP_SIZE) {
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_TXPWRCTL,
			initvalue, LCN20_PAPD_TBL_WRITE_STEP_SIZE, 8,
			LCN20PHY_TX_PAPD_LUT_SELECT_OFFSET+j);
	}
}
#endif /* LCN20_PAPD_ENABLE */

static bool
BCMATTACHFN(wlc_lcn20phy_rx_gain_tbl_select)(phy_info_t *pi)
{
	CONST uint8 *gainval_tbl_ptr = NULL;
	CONST uint32 *gain_tbl_ptr = NULL, *gainidx_tbl_ptr = NULL;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	gainval_tbl_ptr = (CONST uint8 *) dot11lcn20_gain_val_tbl_2G_rev0;
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		gain_tbl_ptr = (CONST uint32 *) dot11lcn20_gain_tbl_2G_ext_lna_rev0;
		gainidx_tbl_ptr = (CONST uint32 *) dot11lcn20_gain_idx_tbl_2G_ext_lna_rev0;
	} else {
		gain_tbl_ptr = (CONST uint32 *) dot11lcn20_gain_tbl_2G_rev0;
		gainidx_tbl_ptr = (CONST uint32 *) dot11lcn20_gain_idx_tbl_2G_rev0;
	}

	/* gainval table */
	pi_lcn20->gainvaltbl = MALLOC(pi->sh->osh, sizeof(*pi_lcn20->gainvaltbl) *
		LCN20PHY_RXGAINVAL_TBL_SZ);

	if (!pi_lcn20->gainvaltbl) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	memcpy(pi_lcn20->gainvaltbl, gainval_tbl_ptr,
		(sizeof(*pi_lcn20->gainvaltbl) * LCN20PHY_RXGAINVAL_TBL_SZ));

	/* gain table */
	pi_lcn20->gaintbl = MALLOC(pi->sh->osh, sizeof(*pi_lcn20->gaintbl) *
		LCN20PHY_RXGAIN_TBL_SZ);

	if (!pi_lcn20->gaintbl) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	memcpy(pi_lcn20->gaintbl, gain_tbl_ptr,
		(sizeof(*pi_lcn20->gaintbl) * LCN20PHY_RXGAIN_TBL_SZ));

	/* gainidx table */
	pi_lcn20->gainidxtbl = MALLOC(pi->sh->osh, sizeof(*pi_lcn20->gainidxtbl) *
		LCN20PHY_RXGAINIDX_TBL_SZ);

	if (!pi_lcn20->gainidxtbl) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}

	memcpy(pi_lcn20->gainidxtbl, gainidx_tbl_ptr,
		(sizeof(*pi_lcn20->gainidxtbl) * LCN20PHY_RXGAINIDX_TBL_SZ));

	return TRUE;
}

static bool
BCMATTACHFN(wlc_lcn20phy_tx_gain_tbl_select)(phy_info_t *pi)
{
	lcn20phy_tx_gain_tbl_entry *gain_table_ptr = NULL;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	pi_lcn20->gain_table_tx_2g = NULL;

	if (pi->low_power_mode) {
		gain_table_ptr = (lcn20phy_tx_gain_tbl_entry *)
			&dot11lcn20phy_2GHz_lowpow_gaintable_rev0;
	} else {
		gain_table_ptr = (lcn20phy_tx_gain_tbl_entry *)
			&dot11lcn20phy_2GHz_gaintable_rev0;
	}

	/* preserve the 2g gain table for later use */
	pi_lcn20->gain_table_tx_2g = MALLOC(pi->sh->osh, sizeof(lcn20phy_tx_gain_tbl_entry) *
		LCN20PHY_TX_GAIN_TABLE_SZ);

	if (!pi_lcn20->gain_table_tx_2g) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	wlc_lcn20phy_tx_gain_tbl_copy((lcn20phy_tx_gain_tbl_entry *) pi_lcn20->gain_table_tx_2g,
		gain_table_ptr, LCN20PHY_TX_GAIN_TABLE_SZ);

	return TRUE;
}

static const char BCMATTACHDATA(rstr_gainadj)[] = "gainadj";
static const char BCMATTACHDATA(rstr_dsssthresholdoffset)[] = "dsssthreshoff";
static bool
BCMATTACHFN(wlc_lcn20phy_lna_params_select)(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_lna_params_t *lna_params;
	int i;
	lna_params = (lcn20phy_lna_params_t *)MALLOC(pi->sh->osh, sizeof(*lna_params));
	if (lna_params == NULL) {
		PHY_ERROR(("wl%d: %s: lna params MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return FALSE;
	}
	pi->u.pi_lcn20phy->lna_params = lna_params;

	/* boardflags are read in wlc_lcn20phy_txpwr_srom_read so this function
	 * needs to precede this one
	 */
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		lna_params->gainrequestTRAttnOnEn = 1;
		lna_params->gainReqTrAttOnEnByCrs = 1;

		if (pi_lcn20->elna_off_gain_idx_2g != 0xFF)
			lna_params->trGainThresh = pi_lcn20->elna_off_gain_idx_2g +
				LCN20PHY_NORTBL_OFFSET_NEW;
		else
			lna_params->trGainThresh = 30+LCN20PHY_NORTBL_OFFSET_NEW;

		if (pi_lcn20->tr_isolation != 0xFF)
			lna_params->gainrequestTRAttnOnOffset = pi_lcn20->tr_isolation;
		else
			lna_params->gainrequestTRAttnOnOffset = 7;

		pi_lcn20->gainadj = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_gainadj, 0);
		lna_params->wl_gain_tbl_offset = 38-LCN20PHY_NORTBL_OFFSET_NEW
			+ pi_lcn20->gainadj;

		lna_params->rssi_gain = 54+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->crs_gain_high_gain_db_40mhz = 63+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->max_gain_of_tbl = 69+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->wl_lna2g_dig_wrssi1_threshold = 5;
		lna_params->rssi_clip_thresh_norm_9 = 18;
		lna_params->rssi_clip_thresh_norm_10 = 21;
		lna_params->rssi_clip_thresh_norm_11 = 21;
		lna_params->rssi_clip_thresh_aci_9 = 8;
		lna_params->rssi_clip_thresh_aci_10 = 10;
		lna_params->rssi_clip_thresh_aci_11 = 12;
		lna_params->clipCtrThresh = 52;
		for (i = 0; i < 14; i++) {
			lna_params->dsss_threshold_offset[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
				rstr_dsssthresholdoffset, i, 0);
		}
	} else {
		lna_params->gainrequestTRAttnOnEn = 0;
		lna_params->gainReqTrAttOnEnByCrs = 0;
		lna_params->trGainThresh = 30;
		lna_params->gainrequestTRAttnOnOffset = 8;
		/* iLNA gain table has extended 6 dB comparing with elna gain talbe */
		lna_params->wl_gain_tbl_offset = 35-LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->rssi_gain = 57+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->crs_gain_high_gain_db_40mhz = 63+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->max_gain_of_tbl = 78+LCN20PHY_NORTBL_OFFSET_NEW;
		lna_params->wl_lna2g_dig_wrssi1_threshold = 8;
		lna_params->rssi_clip_thresh_norm_9 = 18;
		lna_params->rssi_clip_thresh_norm_10 = 22;
		lna_params->rssi_clip_thresh_norm_11 = 25;
		lna_params->rssi_clip_thresh_aci_9 = 5;
		lna_params->rssi_clip_thresh_aci_10 = 10;
		lna_params->rssi_clip_thresh_aci_11 = 18;
		lna_params->clipCtrThresh = 46;
	}

	lna_params->meas_rxspur_gaintblidx = (uint32)((lna_params->wl_gain_tbl_offset +
		lna_params->crs_gain_high_gain_db_40mhz)*85)>>8;

	return TRUE;
}

static void
BCMATTACHFN(wlc_lcn20phy_tx_gain_tbl_copy)(lcn20phy_tx_gain_tbl_entry *gain_table_dst,
	const lcn20phy_tx_gain_tbl_entry * gain_table, uint32 num_elem)
{
	uint j;
	for (j = 0; j < num_elem; j++) {
		gain_table_dst[j] = gain_table[j];
	}
}

static void
wlc_lcn20phy_tx_gain_tbl_free(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	if (pi_lcn20->gain_table_tx_2g)
		MFREE(pi->sh->osh, pi_lcn20->gain_table_tx_2g, sizeof(lcn20phy_tx_gain_tbl_entry) *
			LCN20PHY_TX_GAIN_TABLE_SZ);
}

static uint16
lcn20phy_div_based_init_h(uint16 ant, uint16 swidx, uint32 *swmap)
{
	uint16 ret;

	if (ant)
		ret = (uint16) ((swmap[swidx] & 0xff000000) >> 24);
	else
		ret = (uint16) ((swmap[swidx] & 0xff0000) >> 16);

	return ret;
}

static uint16
lcn20phy_div_based_init_l(uint16 ant, uint16 swidx, uint32 *swmap)
{
	uint16 ret;

	if (ant)
		ret = (uint16) ((swmap[swidx] & 0xff00) >> 8);
	else
		ret = (uint16) (swmap[swidx] & 0xff);

	return ret;
}

static void
wlc_lcn20phy_sw_ctrl_tbl_init(phy_info_t *pi)
{
	uint16 tblval;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint8 idx;
	uint32 *swctrlmap;
	uint16 tdm, ovr_en;
	uint16 mask;

	swctrlmap = pi_lcn20->swctrlmap_2g;
	tdm = (swctrlmap[LCN20PHY_I_WL_MASK] & LCN20PHY_MASK_TDM);
	ovr_en = (swctrlmap[LCN20PHY_I_WL_MASK] & LCN20PHY_MASK_OVR_EN);

	for (idx = 0; idx < LCN20PHY_SW_CTRL_TBL_LENGTH; idx++) {
		uint8	bt_pri = idx & LCN20PHY_SW_CTRL_MAP_BT_PRIO;
		uint16	ant = idx & LCN20PHY_SW_CTRL_MAP_ANT;

		tblval = 0;
		/* BT Prio */
		if (bt_pri) {
			/* Diasble diversity in WL to ensure
			both BT and WL recieve from the same ant
			*/
			if (ovr_en)
				ant = (swctrlmap[LCN20PHY_I_WL_MASK] & LCN20PHY_MASK_ANT);

			if (idx & LCN20PHY_SW_CTRL_MAP_BT_TX) {
				/* BT Tx */

				tblval |=
				(swctrlmap[LCN20PHY_I_BT] & LCN20PHY_MASK_BT_TX)
					>> LCN20PHY_SHIFT_BT_TX;
			} else {
				/* BT Rx */
				if (idx & LCN20PHY_SW_CTRL_MAP_ELNA) {
					tblval |=
					(swctrlmap[LCN20PHY_I_BT] & LCN20PHY_MASK_BT_ELNARX)
					>> LCN20PHY_SHIFT_BT_ELNARX;
				} else {
					tblval |=
					(swctrlmap[LCN20PHY_I_BT] & LCN20PHY_MASK_BT_RX)
					>> LCN20PHY_SHIFT_BT_RX;
				}
			}
		}
		/* WL Tx/Rx */
		if (!tdm || !bt_pri) {
			if (idx & LCN20PHY_SW_CTRL_MAP_WL_TX) {
				/* PA on */
				if (idx & LCN20PHY_SW_CTRL_MAP_WL_RX) {
					/* Rx with PA on */
					tblval |=
					lcn20phy_div_based_init_h(ant, LCN20PHY_I_WL_TX, swctrlmap);
				} else {
					/* WL Tx with PA on */
					tblval |=
					lcn20phy_div_based_init_l(ant, LCN20PHY_I_WL_TX, swctrlmap);
				}
			} else {
				if (idx & LCN20PHY_SW_CTRL_MAP_ELNA) {
					if (idx & LCN20PHY_SW_CTRL_MAP_WL_RX) {
						/* WL Rx eLNA */
						tblval |=
						lcn20phy_div_based_init_h(ant, LCN20PHY_I_WL_RX,
						swctrlmap);
					} else {
						/* WL Rx Attn eLNA */
						tblval |=
						lcn20phy_div_based_init_h
						(ant, LCN20PHY_I_WL_RX_ATTN, swctrlmap);
					}
				} else { /* Without eLNA */
					if (idx & LCN20PHY_SW_CTRL_MAP_WL_RX) {
						/* WL Rx */
						tblval |=
						lcn20phy_div_based_init_l
						(ant, LCN20PHY_I_WL_RX, swctrlmap);
					} else {
						/* WL Rx Attn */
						tblval |=
						lcn20phy_div_based_init_l
						(ant, LCN20PHY_I_WL_RX_ATTN, swctrlmap);
					}
				}
			}
		}

		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_SW_CTRL,
			&tblval,  1, LCN20PHY_SW_CTRL_TBL_WIDTH, idx);
	}

	pi_lcn20->elna_bypsss_val |=
		lcn20phy_div_based_init_l
		(0, LCN20PHY_I_WL_RX_ATTN, swctrlmap);

	/* Writing the fields into the LCN20PHY_swctrlconfig register */
	mask = (uint16)(swctrlmap[LCN20PHY_I_WL_MASK] & LCN20PHY_MASK_WL_MASK)
			<< LCN20PHY_sw_ctrl_config_sw_ctrl_mask_SHIFT;
	phy_utils_mod_phyreg(pi, LCN20PHY_sw_ctrl_config,
		LCN20PHY_sw_ctrl_config_sw_ctrl_mask_MASK, mask);

	/* BT sends clb2lcn20_bt_ext_lna_gain inverted */
	PHY_REG_MOD(pi, LCN20PHY, swctrlOvr_val, bt_extlna_lut, 1);
}

static void
WLBANDINITFN(wlc_lcn20phy_tbl_init)(phy_info_t *pi)
{
	uint idx, tbl_info_sz;
	CONST phytbl_info_t *tbl_info = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tbl_info_sz = dot11lcn20phytbl_info_sz_rev0;
	tbl_info = (CONST phytbl_info_t *)dot11lcn20phytbl_info_rev0;

	for (idx = 0; idx < tbl_info_sz; idx++)
		wlc_lcn20phy_write_table(pi, &tbl_info[idx]);

	wlc_lcn20phy_load_rx_gain_tables(pi);

#ifdef LCN20_PAPD_ENABLE
	lcn20phy_papd_tbl_init(pi);
#endif /* LCN20_PAPD_ENABLE */
	wlc_lcn20phy_load_tx_gain_table(pi, pi->u.pi_lcn20phy->gain_table_tx_2g);
}

static void
WLBANDINITFN(wlc_lcn20phy_rev0_reg_init)(phy_info_t *pi)
{

	PHY_REG_LIST_START
		PHY_REG_MOD_ENTRY(LCN20PHY, agcControl4, c_agc_fsm_en, 0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, resetCtrl, 0x004f)
		PHY_REG_WRITE_ENTRY(LCN20PHY, AfeCtrlOvr, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, AfeCtrlOvr1, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, RFOverride0, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride2, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride3, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride4, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride5, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride7, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, rfoverride8, 0x0000)
		PHY_REG_WRITE_ENTRY(LCN20PHY, swctrlOvr, 0x0000)

		PHY_REG_MOD_ENTRY(LCN20PHY, wl_gain_tbl_offset, wl_gain_tbl_offset, 18)
		PHY_REG_MOD_ENTRY(LCN20PHY, nftrAdj, bt_gain_tbl_offset, 6)

		PHY_REG_MOD_ENTRY(LCN20PHY, RFOverrideVal0, rfpll_pu_ovr_val, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, RFOverride0, rfpll_pu_ovr, 1)

		/* RFPLL is not controlled by RFSEQ so set rfpll_pu_byp to 1 */
		PHY_REG_MOD_ENTRY(LCN20PHY, radio_pu_cfg, rfpll_pu_byp, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_LIST_START
		/* In a 2-chip database although the connection is directly from
			* txbbmux o/p to rxfilt(ACI) i/p, rx decimation chain is somehow still
			* running which causes periodic clk-stalls if not disabled.
			* This will create glitches in the rx signal so we need to disable stalls
			* at all time in QT.
			*/
		PHY_REG_MOD_ENTRY(LCN20PHY, RxFeCtrl1, disable_stalls, 1)

		/* These two may be redundant. */
		PHY_REG_MOD_ENTRY(LCN20PHY, RFOverride0, rfpll_pu_ovr, 0)
		PHY_REG_MOD_ENTRY(LCN20PHY, radio_pu_cfg, rfpll_pu_byp, 1)

		/* Need to set ldo. */
		PHY_REG_MOD_ENTRY(LCN20PHY, RFOverrideVal0, ldos_on_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, RFOverride0, ldos_on_ovr, 1)

		/* On QT ACI i/p is the txbbmux o/p right-shifted by 2.
		* For 11b frames we need to further scale down the txbbmux o/p to have
		* proper rx magnitude.
		*/
		PHY_REG_MOD_ENTRY(LCN20PHY, BphyControl3, bphyScale, 5)
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START
		/* Have finished radio, RFPLL stable, so reenable PHY */
		PHY_REG_MOD_ENTRY(LCN20PHY, agcControl4, c_agc_fsm_en, 1)
		PHY_REG_WRITE_ENTRY(LCN20PHY, resetCtrl, 0x0000)
	PHY_REG_LIST_EXECUTE(pi);

	PHY_REG_LIST_START
		/* Update the LMS based alpha value to 8 instead of 5(default)
		* in order to mask the slicer EVM degradation over time
		*/
		PHY_REG_MOD_ENTRY(LCN20PHY, channelUpdateReg, HardDeciBasedalpha, 8)

		/* Making ovren 1 gives control to ldpc_support bit
		* which is used to control LDPC ON/OFF
		*/
		PHY_REG_MOD_ENTRY(LCN20PHY, LDPCControl, ldpc_support_ovren, 1)
	PHY_REG_LIST_EXECUTE(pi);

	if (NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn20phy_set_bbmult(pi, 128);
		return;
	}

	wlc_lcn20phy_set_tx_pwr_by_index(pi, 40);
}

static void
WLBANDINITFN(wlc_lcn20phy_reg_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	wlc_lcn20phy_rev0_reg_init(pi);
}

static void
WLBANDINITFN(wlc_lcn20phy_baseband_init)(phy_info_t *pi)
{
	PHY_TRACE(("%s:\n", __FUNCTION__));
	/* Initialize LCN20PHY tables */
	wlc_lcn20phy_tbl_init(pi);

	/* Initialize swctrl-LUT and enable swctrl-GPIOs if needed */
	if (pi->u.pi_lcn20phy->trsw_ctrl_etr == TRUE) {
		wlc_lcn20phy_sw_ctrl_tbl_init(pi);
		si_gci_chipcontrol(pi->sh->sih, CC_GCI_CHIPCTRL_05,
			CC43430_RFSWCTRL_EN_MASK,
			(pi->u.pi_lcn20phy->swctrl_gpios << CC43430_RFSWCTRL_EN_SHIFT));
	} else if (LCN20REV_GE(pi->pubpi->phy_rev, 1)) {
		uint8  j;
		uint16 initval = 0;

		for (j = 0; j < LCN20PHY_SW_CTRL_TBL_LENGTH; j++)
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_SW_CTRL,
				&initval,  1, LCN20PHY_SW_CTRL_TBL_WIDTH, j);
	}

	wlc_lcn20phy_reg_init(pi);
	phy_utils_write_phyreg(pi, LCN20PHY_DSSF_C_CTRL, 0x0);
}

static void
WLBANDINITFN(wlc_lcn20phy_radio_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (NORADIO_ENAB(pi->pubpi))
		return;
}

void
WLBANDINITFN(wlc_phy_init_lcn20phy)(phy_info_t *pi)
{
	/* phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy; */

	PHY_TRACE(("%s:\n", __FUNCTION__));

	/* reset the PHY and the radio
	* calls the phy_reset and resetCtrl.radioReset
	* that resets all LDOs
	*/
	wlc_lcn20phy_phy_and_radio_reset(pi);

	/*  set band to g band */
	wlc_lcn20phy_bandset(pi);

	/* Initialize baseband */
	wlc_lcn20phy_baseband_init(pi);

#if defined(LP_P2P_SOFTAP) || defined(WL_LPC)
#ifdef WL_LPC
	if (pi->lpc_algo)
#endif /* WL_LPC */
		wlc_lcn20phy_lpc_write_maclut(pi);
#endif /* LP_P2P_SOFTAP || WL_LPC */

	if (!NORADIO_ENAB(pi->pubpi)) {
		/* initialize the radio registers from 20692_rev0_regs.tcl to chipdefaults */
		wlc_lcn20phy_radio_init(pi);
		wlc_lcn20phy_radio20692_init(pi);
	}

	/* Tune to the current channel */
	wlc_phy_chanspec_set_lcn20phy(pi, pi->radio_chanspec);

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 1);

		/* Settings ported from DV */
		PHY_REG_WRITE(pi, LCN20PHY, DSSF_C_CTRL, 0);
		PHY_REG_MOD(pi, LCN20PHY, DSSSConfirmCnt, DSSSConfirmCntLoGain, 0x3);
	}

}
/* END: INIT above functions are part of PHY/RADIO initialization functionality  */

void
wlc_lcn20phy_set_bbmult(phy_info_t *pi, uint8 m0)
{
	uint16 m0m1 = (uint16)m0;
	phytbl_info_t tab;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	tab.tbl_ptr = &m0m1; /* ptr to buf */
	tab.tbl_len = 1; /* values   */
	tab.tbl_id = LCN20PHY_TBL_ID_IQLOCAL; /* iqloCaltbl      */
	tab.tbl_offset = 99; /* tbl offset */
	tab.tbl_width = 16; /* 16 bit wide */
	wlc_lcn20phy_write_table(pi, &tab);
	tab.tbl_offset = 115; /* cck tbl offset */
	wlc_lcn20phy_write_table(pi, &tab);
}

void
wlc_lcn20phy_deaf_mode(phy_info_t *pi, bool mode)
{
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, !mode);
}

/* BEGIN: PLAYTONE Below functions are part of play tone feature */
static void
wlc_lcn20phy_tx_tone_samples(phy_info_t *pi, int32 f_Hz, uint16 max_val, uint32 *data_buf,
	uint32 phy_bw, uint16 num_samps)
{
	fixed theta = 0, rot = 0;
	uint16 i_samp, q_samp, t;
	math_cint32 tone_samp;
	BCM_REFERENCE(pi);
	/* set up params to generate tone */
	rot = FIXED((ABS(f_Hz) * 36)/(phy_bw * 1000)) / 100; /* 2*pi*f/bw/1000  Note: f in KHz */
	theta = 0;			/* start angle 0 */

	/* tone freq = f_c MHz ; phy_bw = phy_bw MHz ; # samples = phy_bw (1us) ; max_val = 151 */
	/* TCL: set tone_buff [mimophy_gen_tone $f_c $phy_bw $phy_bw $max_val] */
	for (t = 0; t < num_samps; t++) {
		/* compute phasor */
		math_cmplx_cordic(theta, &tone_samp);
		/* update rotation angle */
		if (f_Hz > 0)
			theta += rot;
		else
			theta -= rot;
		/* produce sample values for play buffer */
		i_samp = (uint16)(FLOAT(tone_samp.i * max_val) & 0x3ff);
		q_samp = (uint16)(FLOAT(tone_samp.q * max_val) & 0x3ff);
		data_buf[t] = (i_samp << 10) | q_samp;
	}
}

void
wlc_lcn20phy_stop_tx_tone(phy_info_t *pi)
{
	int16 playback_status, mask;
	int cnt = 0;
	pi->phy_tx_tone_freq = 0;

	/* Stop sample buffer playback */
	playback_status = READ_LCN20PHYREG(pi, sampleStatus);
	mask = LCN20PHY_sampleStatus_NormalPlay_MASK | LCN20PHY_sampleStatus_iqlocalPlay_MASK;
	do {
		playback_status = READ_LCN20PHYREG(pi, sampleStatus);
		if (playback_status & LCN20PHY_sampleStatus_NormalPlay_MASK) {
			wlc_lcn20phy_tx_pu(pi, 0, 0);
			PHY_REG_MOD(pi, LCN20PHY, sampleCmd, stop, 1);
		} else if (playback_status & LCN20PHY_sampleStatus_iqlocalPlay_MASK)
			PHY_REG_MOD(pi, LCN20PHY, iqloCalCmdGctl, iqlo_cal_en, 0);
		OSL_DELAY(1);
		playback_status = READ_LCN20PHYREG(pi, sampleStatus);
		cnt++;
	} while ((cnt < 10) && (playback_status & mask));

	ASSERT(!(playback_status & mask));

	PHY_REG_LIST_START
		/* put back SPB into standby */
		PHY_REG_WRITE_ENTRY(LCN20PHY, sslpnCtrl3, 1)
		/* disable clokc to spb */
		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 0)
		/* disable clock to txFrontEnd */
		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, forceaphytxFeclkOn, 0)
		/* disable clock to tx iir */
		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, forceTxfiltClkOn, 0)
	PHY_REG_LIST_EXECUTE(pi);

	/* Restore all the crs signals to the MAC */
	wlc_lcn20phy_deaf_mode(pi, FALSE);

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 0);
	}

	PHY_REG_MOD(pi, LCN20PHY, AphyControlAddr, phyloopbackEn, 0);
}

static uint16
wlc_lcn20phy_num_samples(phy_info_t *pi, int32 f_Hz, uint32 phy_bw)
{
	uint16 num_samps, k;
	uint32 bw;
	BCM_REFERENCE(pi);

	/* allocate buffer */
	if (f_Hz) {
		k = 1;
		do {
			bw = phy_bw * 1000 * k * 1000;
			num_samps = bw / ABS(f_Hz);
			ASSERT(num_samps <= 256);
			k++;
		} while ((num_samps * (uint32)(ABS(f_Hz))) !=  bw);
	} else
		num_samps = 2;

	return num_samps;
}

/*
 * Play samples from sample play buffer
 */
static void
wlc_lcn20phy_run_samples(phy_info_t *pi,
	uint16 num_samps,
	uint16 num_loops,
	uint16 wait,
	bool iqcalmode,
	bool rxrfmode)
{
	uint16 playback_status, mask;
	int cnt = 0;

	/* enable clk to txFrontEnd */
	phy_utils_or_phyreg(pi, LCN20PHY_sslpnCalibClkEnCtrl, 0x8080);

	phy_utils_mod_phyreg(pi, LCN20PHY_sampleDepthCount,
		LCN20PHY_sampleDepthCount_DepthCount_MASK,
		(num_samps - 1) << LCN20PHY_sampleDepthCount_DepthCount_SHIFT);

	if (num_loops != 0xffff)
		num_loops--;
	phy_utils_mod_phyreg(pi, LCN20PHY_sampleLoopCount,
		LCN20PHY_sampleLoopCount_LoopCount_MASK,
		num_loops << LCN20PHY_sampleLoopCount_LoopCount_SHIFT);

	phy_utils_mod_phyreg(pi, LCN20PHY_sampleInitWaitCount,
		LCN20PHY_sampleInitWaitCount_InitWaitCount_MASK,
		wait << LCN20PHY_sampleInitWaitCount_InitWaitCount_SHIFT);

	mask = iqcalmode ? LCN20PHY_sampleStatus_iqlocalPlay_MASK :
		LCN20PHY_sampleStatus_NormalPlay_MASK;
	do {
		if (iqcalmode)
			/* Enable calibration */
			PHY_REG_MOD(pi, LCN20PHY, iqloCalCmdGctl, iqlo_cal_en, 1);
		else
			PHY_REG_MOD(pi, LCN20PHY, sampleCmd, start, 1);
		OSL_DELAY(1);
		cnt++;
		playback_status = READ_LCN20PHYREG(pi, sampleStatus);
	} while ((cnt < 10) && !(playback_status & mask));

	ASSERT((playback_status & mask));

	if (!iqcalmode)
		wlc_lcn20phy_tx_pu(pi, 1, rxrfmode);
}

static void
wlc_lcn20phy_set_trsw_override(phy_info_t *pi, uint8 tr_mode, bool agc_reset)
{
	bool rx_switch = 0, tx_switch = 0;

	switch (tr_mode) {
		case LCN20PHY_TRS_TXMODE:
			rx_switch = 0;
			tx_switch = 1;
			break;
		case LCN20PHY_TRS_RXMODE:
			rx_switch = 1;
			tx_switch = 0;
			break;
		case LCN20PHY_TRS_MUTE:
			rx_switch = 0;
			tx_switch = 0;
			break;
		case LCN20PHY_TRS_TXRXMODE:
			rx_switch = 1;
			tx_switch = 1;
			break;
		default:
			PHY_ERROR(("wl%d: %s: Bad tr switch mode\n",
				pi->sh->unit, __FUNCTION__));
			break;
	}

	/* Apply TR switch override settings: */
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_rx_pwrup_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, trsw_rx_pwrup_ovr_val, rx_switch);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, trsw_tx_pwrup_ovr_val, tx_switch);

	/* set eTR for radio rev = 5 and */
	/* set to a known state if it is in iTR mode(radio rev <= 4) */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, rx_switch);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_tx_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_tx_pu_ovr_val, tx_switch);

	if (agc_reset)
		wlc_lcn20phy_agc_reset(pi);
}

void
wlc_lcn20phy_rx_pu(phy_info_t *pi, bool bEnable)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, adc_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_rx_pwrup_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, papu_ovr, 0);
	} else {
		/* Power down iPA; make sure this happens BEFORE tr-sw is changed to RX */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, papu_ovr_val, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, papu_ovr, 1);

		/* Power up ADC */
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, adc_pu_ovr_val, 31);
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, adc_pu_ovr, 1);

		/* Power up GM, Mixer, TIA, Logen */
		PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 1);

		/* Power up LNA1 */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, slna_pu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_pu_ovr, 1);

		/* Force the TR switch to RX */
		wlc_lcn20phy_set_trsw_override(pi, LCN20PHY_TRS_RXMODE, FALSE);
	}
}

void
wlc_lcn20phy_rx_pwrup(phy_info_t *pi, bool bEnable)
{
	wlc_lcn20phy_rx_pu(pi, bEnable);
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		/* Disable eTR switch overrides for B0, does not have effect for A0/A1  */
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_tx_pu_ovr, 0);
	}

}

void
wlc_lcn20phy_tx_pu(phy_info_t *pi, bool bEnable, bool rxrfmode)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (!bEnable) {
		/* Disable TX overrides: */
		/* iPA: Make sure iPA is powered-down first */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, papu_ovr, 0);
		/* Mixer, cascode, , .. */
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 0);
		/* DAC and LPF: */
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 0);

		/* Disable RX overrides: */
		/* gm/LNA2, mixer: */
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 0);
		/* LNA1: */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_pu_ovr, 0);

		/* Disable TR switch overrides: */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_rx_pwrup_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 0);

		/* Disable eTR switch overrides for B0, does not have effect for A0/A1  */
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, 0);
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_tx_pu_ovr, 0);
	} else {
		/* Power down receiver: */
		PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 1);
		if (!rxrfmode)
			/* Power down gm/LNA2, mixer: */
			PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 0);
		else
			/* Power up gm/LNA2, mixer for Loopback mode */
			PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);

		/* Power down LNA1: */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, slna_pu_ovr_val, 0);

		/* Power up transmitter */
		/* Force the TR switch to transmit */
		wlc_lcn20phy_set_trsw_override(pi, LCN20PHY_TRS_TXMODE, FALSE);

		/* Power up mixer, cascode, , .. */
		phy_utils_mod_phyreg(pi, LCN20PHY_RFOverrideVal0,
			LCN20PHY_RFOverrideVal0_internalrftxpu_ovr_val_MASK,
			(1 << LCN20PHY_RFOverrideVal0_internalrftxpu_ovr_val_SHIFT));
		phy_utils_mod_phyreg(pi, LCN20PHY_RFOverride0,
			LCN20PHY_RFOverride0_internalrftxpu_ovr_MASK,
			(1 << LCN20PHY_RFOverride0_internalrftxpu_ovr_SHIFT));

		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 1);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, trsw_tx_pwrup_ovr_val, 1);

		/* Power up DAC and LPF: */
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 0);
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
		PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);

		/* Power up iPA: Make sure iPA is powered-up only
		* AFTER TR-switch is configured for TX
		*/
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, papu_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride4, papu_ovr, 1);
	}
}

/*
* Given a test tone frequency, continuously play the samples. Ensure that num_periods
* specifies the number of periods of the underlying analog signal over which the
* digital samples are periodic
*/
/* equivalent to lcn20phy_play_tone */
void
wlc_lcn20phy_start_tx_tone(phy_info_t *pi,
	int32 f_Hz,
	uint16 max_val,
	bool iqcalmode,
	bool deafmode,
	bool rxrfmode)
{
	uint8 phy_bw;
	uint16 num_samps;
	uint32 *data_buf;

	phytbl_info_t tab;

	if ((data_buf = LCN20PHY_MALLOC(pi, sizeof(uint32) * 256)) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	if (NORADIO_ENAB(pi->pubpi)) {
		PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, disable_stalls, 1);
	}

	/* Save active tone frequency */
	pi->phy_tx_tone_freq = f_Hz;

	PHY_REG_MOD(pi, LCN20PHY, AphyControlAddr, phyloopbackEn, 1);

	/* Turn off all the crs signals to the MAC */
	wlc_lcn20phy_deaf_mode(pi, deafmode);

	phy_bw = 40;

	num_samps = wlc_lcn20phy_num_samples(pi, f_Hz, phy_bw);

	PHY_INFORM(("wl%d: %s: %d Hz, %d samples\n",
		pi->sh->unit, __FUNCTION__,
		f_Hz, num_samps));

	if (num_samps > 256) {
		PHY_ERROR(("wl%d: %s: Too many samples to fit in SPB\n",
			pi->sh->unit, __FUNCTION__));
		LCN20PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
		return;
	}

	/* we need to bring SPB out of standby before using it */
	PHY_REG_MOD(pi, LCN20PHY, sslpnCtrl3, sram_stby, 0);

	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 1);

	wlc_lcn20phy_tx_tone_samples(pi, f_Hz, max_val, data_buf, phy_bw, num_samps);

	/* lcn20phy_load_sample_table */
	tab.tbl_ptr = data_buf;
	tab.tbl_len = num_samps;
	tab.tbl_id = LCN20PHY_TBL_ID_SAMPLEPLAY;
	tab.tbl_offset = 0;
	tab.tbl_width = 32;
	wlc_lcn20phy_write_table(pi, &tab);
	/* play samples from the sample play buffer */
	wlc_lcn20phy_run_samples(pi, num_samps, 0xffff, 0, iqcalmode, rxrfmode);
	LCN20PHY_MFREE(pi, data_buf, 256 * sizeof(uint32));
}

void
wlc_lcn20phy_set_tx_tone_and_gain_idx(phy_info_t *pi)
{
	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	if (LCN20PHY_TX_PWR_CTRL_OFF != wlc_lcn20phy_get_tx_pwr_ctrl(pi)) {
		int8 curr_pwr_idx_val;
		curr_pwr_idx_val = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
		wlc_lcn20phy_set_tx_pwr_by_index(pi, (int)curr_pwr_idx_val);
	}

	phy_utils_write_phyreg(pi, LCN20PHY_sslpnCalibClkEnCtrl, 0xffff);
	wlc_lcn20phy_start_tx_tone(pi, pi->phy_tx_tone_freq, 120, 0, TRUE, 0); /* play tone */
}
/* END: PLAYTONE Above functions are part of play tone feature */

/* BEGIN: DCCAL Below functions are part of DC calibration feature */

/* initialize dcoe, idacc registers and tables
* arguments
* init_dcoe: 1 to initialize dcoe related parameters
* init_idacc: 1 to initialize idacc related parameters
* reset_table: 1 to clear tables
*/
static void
wlc_lcn20phy_dccal_init(phy_info_t *pi, uint8 dccal_mode, bool reset_tbl, uint8 bias_adj)
{

	/* Below is a place holder to assit table write in one shot */
	uint32 zero_tbl[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	phytbl_info_t tab;

	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_pktproc_ctrl, dccal_pktproc_initwait, 32);

	if (dccal_mode & LCN20PHY_DCCALMODE_DCOE) {
		phy_utils_mod_phyreg(pi, LCN20PHY_phyreg2dccal_config_0,
			LCN20PHY_INITDCOE_MASK, LCN20PHY_INITDCOE_VAL);

		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_1,
			dcoe_abort_threshold, 25);

		/* clear dcoe table */
		if (reset_tbl) {
			tab.tbl_ptr = zero_tbl;
			tab.tbl_len = 12;
			tab.tbl_id = LCN20PHY_TBL_ID_DCOE;
			tab.tbl_width = 32;
			tab.tbl_offset = 0;
			wlc_lcn20phy_write_table(pi, &tab);
		}

		/* clear dcoe_done */
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_8, ld_dcoe_done, 0);
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_dcoe_done, 1);
	}

	if (dccal_mode & LCN20PHY_DCCALMODE_IDACC) {
		/* idac_gmap: entry format
		 * MSB->LSB {LNA1 bypass, LNA2 gm[3:0], LNA1 low-ct, gm[2:0], Rout[3:0]}
		 * populate idac_gmap table entries 1:7 with gain combinations
		 * to be calibrated in order of increasing gain, highest at index 7.
		 * index 0 is used for DCOE and also when a given gain combination
		 * does not match any of the combinations in idac_gmap[1:7].
		 */
		uint16 idac_gmap_elna[8] = {
			((0 << 12) | (0 << 8) | (0 << 7) | (2 << 4) | 8),
			((0 << 12) | (0 << 8) | (0 << 7) | (2 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (2 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (3 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (4 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (5 << 4) | 8),
			((0 << 12) | (3 << 8) | (0 << 7) | (5 << 4) | 8),
			((0 << 12) | (3 << 8) | (0 << 7) | (5 << 4) | 0)
			};
		uint16 idac_gmap[8] = {
			((0 << 12) | (0 << 8) | (0 << 7) | (2 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (2 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (3 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (4 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (5 << 4) | 8),
			((0 << 12) | (1 << 8) | (0 << 7) | (5 << 4) | 5),
			((0 << 12) | (1 << 8) | (0 << 7) | (5 << 4) | 0),
			((0 << 12) | (3 << 8) | (0 << 7) | (5 << 4) | 0)
			};
		uint16 mask;

		phy_utils_mod_phyreg(pi, LCN20PHY_phyreg2dccal_config_2,
			LCN20PHY_INITIDACC_MASK, LCN20PHY_INITIDACC_VAL);

		/* increase abort threshold for idac cal */
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_1, idacc_abort_threshold, 40);

		/* re-init value for idac_cal_done */
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_3, idacc_done_init, 1);

		/* loading IDAC GMAP table */
		if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)
			tab.tbl_ptr = idac_gmap_elna;
		else
			tab.tbl_ptr = idac_gmap;
		tab.tbl_len = 8;
		tab.tbl_id = LCN20PHY_TBL_ID_IDACGMAP;
		tab.tbl_width = 16;
		tab.tbl_offset = 0;
		wlc_lcn20phy_write_table(pi, &tab);

		/* clear idac table */
		if (reset_tbl) {
			tab.tbl_ptr = zero_tbl;
			tab.tbl_len = 8;
			tab.tbl_id = LCN20PHY_TBL_ID_IDAC;
			tab.tbl_width = 32;
			tab.tbl_offset = 0;
			wlc_lcn20phy_write_table(pi, &tab);
		}

		/* set idac_cal_done to 0x01 */
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_11, ld_idac_cal_done, 1);
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_idac_cal_done, 1);

		/* init override values */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, tia_offset_dac_pu_ovr_val, 0);
		mask = (LCN20PHY_rfoverride2_tia_offset_dac_pu_ovr_MASK |
			LCN20PHY_rfoverride2_tia_offset_dac_ovr_MASK |
			LCN20PHY_rfoverride2_tia_offset_dac_sign_ovr_MASK);
		_PHY_REG_MOD(pi, LCN20PHY_rfoverride2, mask, 0);

		/* init biasadj */
		MOD_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, wl_tia_offset_dac_biasadj, bias_adj);
	}

	if (dccal_mode & LCN20PHY_DCCALMODE_IDACT) {
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_5, idact_abort_threshold, 40);
		PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_7, idact_wait_cinit, 24);
	}
}

/* enable dccal functions by setting bypass bits and idacc_*_reinit bits
* arguments
* mode[2:0] = {idact, idacc, dcoe}, set bit to 1 to enable particular function
* idacc_reinit_mode: disable reinits 0, per-packet reinit 1
*					tx2rx_only 4, tx2rx_reinit+tx2rx_only 6
*/
static void
wlc_lcn20phy_dccal_set_mode(phy_info_t *pi, uint8 dccal_mode, uint8 idacc_reinit_mode)
{
	uint16 dccalctrl0_val, dccalctrl0_mask;

	/* Setting the dccal ctrl0
	 * dccal_en is used in RTL as an extra gating signal for
	 * dccal_reqd/rdy signals in dcctrigger
	 * set dccal_en to 1 and use *_bypass to switch on/off dcoe/idacc/idact
	 * dccal_en default reset value is 0, so it needs to be set to 1
	 * bt2wl_dccal_mode is used to delay enabling dccal after bt2wl transition;
	 * 0-less delay, 1-more delay
	 */
	dccalctrl0_val = (1 << LCN20PHY_phyreg2dccal_control_0_dccal_en_SHIFT |
		(((dccal_mode & LCN20PHY_BYPASS_MASK) ^ LCN20PHY_BYPASS_MASK)
		<< LCN20PHY_phyreg2dccal_control_0_dcoe_bypass_SHIFT) |
		(0 << LCN20PHY_phyreg2dccal_control_0_bt2wl_dccal_mode_SHIFT));
	dccalctrl0_mask = (LCN20PHY_phyreg2dccal_control_0_dccal_en_MASK |
		LCN20PHY_phyreg2dccal_control_0_dcoe_bypass_MASK |
		LCN20PHY_phyreg2dccal_control_0_idacc_bypass_MASK |
		LCN20PHY_phyreg2dccal_control_0_idact_bypass_MASK |
		LCN20PHY_phyreg2dccal_control_0_bt2wl_dccal_mode_MASK);
	PHY_INFORM(("dccal_ctrl0: %d\n", dccalctrl0_val));
	_PHY_REG_MOD(pi, LCN20PHY_phyreg2dccal_control_0, dccalctrl0_mask, dccalctrl0_val);

	_PHY_REG_MOD(pi, LCN20PHY_phyreg2dccal_config_3, LCN20PHY_IDACCMODE_MASK,
		(idacc_reinit_mode << LCN20PHY_phyreg2dccal_config_3_idacc_ppkt_reinit_SHIFT));
}

/* mode 0-7: 3bits {idact,idacc,dcoe} set bit to 1 to run particular cal */
static void
wlc_lcn20phy_dccal_force_cal(phy_info_t *pi, uint8 dccal_mode, int8 num_retry)
{
	/* variables to support TR, elna overrides */
	uint16 save_RFOvr0, save_RFOvr0val, save_rfovr4, save_rfovr2, save_rfovr2val;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	/* save TR switch and elna override registers to be modified */
	save_rfovr4 = READ_LCN20PHYREG(pi, rfoverride4);
	save_RFOvr0 = READ_LCN20PHYREG(pi, RFOverride0);
	save_RFOvr0val = READ_LCN20PHYREG(pi, RFOverrideVal0);
	save_rfovr2 = READ_LCN20PHYREG(pi, rfoverride2);
	save_rfovr2val = READ_LCN20PHYREG(pi, rfoverride2val);

	/* make sure iTR overrides are cleared so that DCC can control iTR switch during cal */
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 0);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_rx_pwrup_ovr, 0);

	/* set eTR switches to RX mode */
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_tx_pu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_tx_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, 1);

	/* set elna on if present */
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA) {
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, ext_lna_gain_ovr_val, 1);
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, ext_lna_gain_ovr, 1);
	}

	/* reinit dcoe_done and idac_cal_done */
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_dcoe_done, 1);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_idac_cal_done, 1);

	wlc_lcn20phy_dccal_set_mode(pi, dccal_mode, LCN20PHY_IDACCMODE_DISREINIT);

	/* put AGC fsm in HG lock mode, so that there is no RX activity between cals */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);

	do {
			/* reset pkt_proc to perform dcoe, idacc; assumes PHY is in RX */
		PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 1);
		PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 0);

		/* wait for DCOE+IDACC to finish, max time depends on settling
		* and accumulation times
		* This delay is important for idac_cal_done to assert.
		*/
		OSL_DELAY(LCN20PHY_DCCAL_DELAY);
		if ((num_retry == 0) ||
			((READ_LCN20PHYREGFLD(pi, phyreg2dccal_config_11a, idac_cal_done)
			== 0xff) &&
			(READ_LCN20PHYREGFLD(pi, phyreg2dccal_config_9, dcoe_done) == 0xfff)))
			num_retry = 0;
		else
			num_retry--;
	} while (num_retry);

	wlc_lcn20phy_dccal_set_mode(pi, LCN20PHY_DCCALMODE_BYPASS, LCN20PHY_IDACCMODE_DISREINIT);
	/* remove HG lock on AGC fsm */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);

	/* restore eTR/iTR and elna overrides */
	WRITE_LCN20PHYREG(pi, rfoverride4, save_rfovr4);
	WRITE_LCN20PHYREG(pi, RFOverrideVal0, save_RFOvr0val);
	WRITE_LCN20PHYREG(pi, RFOverride0, save_RFOvr0);
	WRITE_LCN20PHYREG(pi, rfoverride2val, save_rfovr2val);
	WRITE_LCN20PHYREG(pi, rfoverride2, save_rfovr2);
}
/* Run idac cal for auxlna path by applying a softreset to pkt_proc fsm
* assumes RF is configured to use aux LNA and that the input to aux LNA is 0
* idac cal is run only for index 0; result from idac_table[0] is written to the
* override registers tiaOffsetDacQVal and tiaOffsetDacIVal and idac_table[0] is cleared
*/
static void
wlc_lcn20phy_dccal_auxlna(phy_info_t *pi)
{
	uint16 control_0, config_3, idac_cal_done_save;
	uint32 idac_aux_lna;
	phytbl_info_t tab;
	int8 num_retry;

	/* Save registers which will be modified below */
	control_0 = READ_LCN20PHYREG(pi, phyreg2dccal_control_0);
	config_3 = READ_LCN20PHYREG(pi, phyreg2dccal_config_3);
	idac_cal_done_save = READ_LCN20PHYREGFLD(pi, phyreg2dccal_config_11a, idac_cal_done);

	/* Disable dccal */
	wlc_lcn20phy_dccal_set_mode(pi, LCN20PHY_DCCALMODE_BYPASS,
		LCN20PHY_IDACCMODE_DISREINIT);

	/* Load 0xfe to idac_cal_done to run cal for index 0 only */
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_11, ld_idac_cal_done, 0xfe);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_idac_cal_done, 1);

	/* To run idac cal only */
	wlc_lcn20phy_dccal_set_mode(pi, LCN20PHY_DCCALMODE_IDACC, LCN20PHY_IDACCMODE_DISREINIT);

	/* put AGC fsm in HG lock mode, so that there is no RX activity between cals */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);

	/* reset pkt_proc to perform dcoe, idacc; assumes PHY is in RX,
	 * adc clock should be on
	 */
	PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 1);
	PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 0);

	/* wait for IDACC to finish, max time depends on settling and accumulation times */
	num_retry = 5;
	do {
			/* reset pkt_proc to perform dcoe, idacc; assumes PHY is in RX */
		PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 1);
		PHY_REG_MOD(pi, LCN20PHY, resetCtrl, pktfsmSoftReset, 0);

		/* wait for DCOE+IDACC to finish,
		* max time depends on settling and accumulation times
		*/
			SPINWAIT((READ_LCN20PHYREGFLD(pi, phyreg2dccal_status, idacc_on)),
			LCN20PHY_SPINWAIT_DCCAL);
		if ((num_retry == 0) ||
			((READ_LCN20PHYREGFLD(pi, phyreg2dccal_config_11a, idac_cal_done) == 0xff)))
			num_retry = 0;
		else
			num_retry--;
	} while (num_retry);

	/* remove HG lock on AGC fsm */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);

	/* read calibrated idac value */
	tab.tbl_ptr = &idac_aux_lna;
	tab.tbl_len = 1;
	tab.tbl_id = LCN20PHY_TBL_ID_IDAC;
	tab.tbl_width = 32;
	tab.tbl_offset = 0;
	wlc_lcn20phy_read_table(pi,  &tab);
	/* write idac values to the override offset dac registers */
	PHY_REG_WRITE(pi, LCN20PHY, tiaOffsetDacQVal, (idac_aux_lna & 0x1ff));
	PHY_REG_WRITE(pi, LCN20PHY, tiaOffsetDacIVal, ((idac_aux_lna >> 9) & 0x1ff));
	PHY_INFORM(("override tiaOffsetDacVal, 0x%3x\n", idac_aux_lna));
	/* clear entry 0 in idac_table */
	idac_aux_lna = 0;
	wlc_lcn20phy_write_table(pi,  &tab);

	/* write back saved parameters */
	PHY_REG_WRITE(pi, LCN20PHY, phyreg2dccal_control_0, control_0);
	PHY_REG_WRITE(pi, LCN20PHY, phyreg2dccal_config_3, config_3);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_config_11, ld_idac_cal_done, idac_cal_done_save);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dccal_control_1, override_idac_cal_done, 1);
}

/* set/clear the offset_dac override bits
* arguments
* ovr_mode: 0 - clear override bits, 1 - set override bits and set pu_ovr_val to 1
* ovr_idac: 1 - write ovr_idac_value into tiaOffsetDac override registers
* ovr_idac_value: 18bits input to be used to set tiaOffsetDacQVal,
*					tiaOffsetDacIVal when ovr_mode=2
*/
static void
wlc_lcn20phy_dccal_ovr_idac(phy_info_t *pi, bool ovr_mode, bool ovr_idac, uint32 ovr_idac_value)
{
	uint16 mask = (LCN20PHY_rfoverride2_tia_offset_dac_pu_ovr_MASK |
		LCN20PHY_rfoverride2_tia_offset_dac_ovr_MASK |
		LCN20PHY_rfoverride2_tia_offset_dac_sign_ovr_MASK);

	if (!ovr_mode) {
		/* clear ovr bits */
		_PHY_REG_MOD(pi, LCN20PHY_rfoverride2, mask, 0);
	} else {
		/* set pu_ovr_val to 1 */
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, tia_offset_dac_pu_ovr_val, 1);
		/* set ovr bits */
		_PHY_REG_MOD(pi, LCN20PHY_rfoverride2, mask, 7);
	}

	if (ovr_idac) {
		/* write idac values to the override offset dac registers */
		PHY_REG_WRITE(pi, LCN20PHY, tiaOffsetDacQVal, (ovr_idac_value & 0x1ff));
		PHY_REG_WRITE(pi, LCN20PHY, tiaOffsetDacIVal, ((ovr_idac_value >> 9) & 0x1ff));
	}
}
/* END:    DCCAL Above functions are part of play tone feature */

/* START: RXIQCAL Below functions are part of Rx IQ Calibrations  feature */

static void
wlc_lcn20phy_rx_iq_cal_phy_setup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_phyregs_t *psave = pi_lcn20->rxiqcal_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	/* Save RFCTRL state, save all RFCTRL override register */
	psave->RFOverride0 = READ_LCN20PHYREG(pi, RFOverride0);
	psave->RFOverrideVal0 = READ_LCN20PHYREG(pi, RFOverrideVal0);
	psave->rfoverride2 = READ_LCN20PHYREG(pi, rfoverride2);
	psave->rfoverride2val = READ_LCN20PHYREG(pi, rfoverride2val);
	psave->rfoverride3 = READ_LCN20PHYREG(pi, rfoverride3);
	psave->rfoverride3_val = READ_LCN20PHYREG(pi, rfoverride3_val);
	psave->rfoverride4 = READ_LCN20PHYREG(pi, rfoverride4);
	psave->rfoverride4val = READ_LCN20PHYREG(pi, rfoverride4val);
	psave->rfoverride5 = READ_LCN20PHYREG(pi, rfoverride5);
	psave->rfoverride5val = READ_LCN20PHYREG(pi, rfoverride5val);
	psave->rfoverride7 = READ_LCN20PHYREG(pi, rfoverride7);
	psave->rfoverride7val = READ_LCN20PHYREG(pi, rfoverride7val);
	psave->rfoverride8 = READ_LCN20PHYREG(pi, rfoverride8);
	psave->rfoverride8val = READ_LCN20PHYREG(pi, rfoverride8val);
	psave->sslpnCalibClkEnCtrl = READ_LCN20PHYREG(pi, sslpnCalibClkEnCtrl);

	/* Save tx gain state
	* save bb_mult, txgain, papr/IIR related parameter
	*/
	psave->bbmult = wlc_lcn20phy_get_bbmult(pi);
	/* Turn off tx pwr ctrl */
	psave->txidx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	psave->SAVE_txpwrctrl_on = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* temporarily turn off PAPD in case it is enabled, disable PAPR */
	psave->PapdEnable0 = READ_LCN20PHYREG(pi, PapdEnable0);
	psave->papr_ctrl = READ_LCN20PHYREG(pi, papr_ctrl);
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, 0);
	PHY_REG_MOD(pi, LCN20PHY, papr_ctrl, papr_blk_en, 0);

	/* enable TIA and Mixer */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);

	/* save Rx/Tx farrow related register setting and DVGA2 related register setting */
	psave->RxSdFeConfig1 = READ_LCN20PHYREG(pi, RxSdFeConfig1);
	psave->RxSdFeConfig6 = READ_LCN20PHYREG(pi, RxSdFeConfig6);
	psave->phyreg2dvga2 = READ_LCN20PHYREG(pi, phyreg2dvga2);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dvga2, dvga2_gain_ovr, 1);

	/* enable LOFT comp, enable TX IQ comp */
	psave->SAVE_Core1TxControl = READ_LCN20PHYREG(pi, Core1TxControl);
	/* enable LOFT comp and TX IQ Ccmp */
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, iqImbCompEnable, 1);
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, loft_comp_en, 1);

	/* ensure the cal clock has been enabled */
	PHY_REG_WRITE(pi, LCN20PHY, sslpnCalibClkEnCtrl, 0x0);
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, samplePlayClkEn, 1);
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1);
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, forceiqEstclkOn, 1);

	/* save DSSF control register */
	psave->DSSF_control_0 = READ_LCN20PHYREG(pi, DSSF_control_0);

	/* Disable DSSF as it can suppress reference loopback tone or its image,  restore it */
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s1, 0);
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s2, 0);

	psave->RxFeCtrl1 = READ_LCN20PHYREG(pi, RxFeCtrl1);
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, forceSdFeClkEn, 1);

	/* because DC cal is not applied for loopback path, so we want to */
	/* force idac = 0 explicitlyto avoid a unwant big idac number applied; */
	/* the DC of auxlna and DCOE is filtered out by DC notch filter */
	wlc_lcn20phy_dccal_ovr_idac(pi, 1, 1, 0);
}

static void
wlc_lcn20phy_rx_gain_override(phy_info_t *pi, uint16 slna_byp, uint16 slna_rout,
	uint16 slna_gain, uint16 lna2_gain,
	uint16 tia, uint16 dvga1_gain, uint16 dvga2_gain)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, slna_byp_ovr_val, slna_byp);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride3_val, slna_rout_ctrl_ovr_val, slna_rout);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, slna_gain_ctrl_ovr_val, slna_gain);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride5val, rxrf_lna2_gain_ovr_val, lna2_gain);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride5val, rxrf_tia_gain_ovr_val, tia);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, dvga1_gain);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dvga2, dvga2_gain_ovr_val, dvga2_gain);
}

static void
wlc_lcn20phy_rx_gain_override_wrapper(phy_info_t *pi, uint16 tr_attn, uint16 elna,
	uint16 slna_byp, uint16 slna_rout,
	uint16 slna_gain, uint16 lna2_gain,
	uint16 tia, uint16 dvga1_gain, uint16 dvga2_gain)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, (!tr_attn));
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, trsw_rx_pwrup_ovr_val, (!tr_attn));
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2val, ext_lna_gain_ovr_val, elna);
	wlc_lcn20phy_rx_gain_override(pi, slna_byp, slna_rout, slna_gain, lna2_gain,
		tia, dvga1_gain, dvga2_gain);
}

static void
wlc_lcn20phy_rx_gain_override_enable(phy_info_t *pi, bool enable)
{
	uint16 ebit = enable ? 1 : 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_byp_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride3, slna_rout_ctrl_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride2, slna_gain_ctrl_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride5, rxrf_lna2_gain_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride5, rxrf_tia_gain_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, ebit);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dvga2, dvga2_gain_ovr, ebit);
}

static void
wlc_lcn20phy_rx_gain_override_enable_wrapper(phy_info_t *pi, bool enable)
{
	uint16 ebit = enable ? 1 : 0;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, ebit);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_rx_pwrup_ovr, ebit);
	if (BOARDFLAGS(GENERIC_PHY_INFO(pi)->boardflags) & BFL_EXTLNA)
		PHY_REG_MOD(pi, LCN20PHY, rfoverride2, ext_lna_gain_ovr, ebit);
	wlc_lcn20phy_rx_gain_override_enable(pi, enable);
}

static void
wlc_lcn20phy_rx_iq_est(phy_info_t *pi, phy_iq_est_t *est, uint16 num_samps,
	uint8 wait_time, uint8 wait_for_crs, bool noise_cal)
{
	uint32 timeout_us;
	BCM_REFERENCE(noise_cal);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	timeout_us = wlc_lcn20phy_get_spinwait_iqest_usec();
	if (NORADIO_ENAB(pi->pubpi)) {
		timeout_us = wlc_lcn20phy_get_spinwait_iqest_qt_usec();
	}

	/* Get Rx IQ Imbalance Estimate from modem */
	PHY_REG_MOD(pi, LCN20PHY, IqestSampleCount, NumSampToCol, num_samps);
	PHY_REG_MOD(pi, LCN20PHY, IqestWaitTime, waitTime, wait_time);
	PHY_REG_MOD(pi, LCN20PHY, IqestCmd, iqMode, wait_for_crs);
	PHY_REG_MOD(pi, LCN20PHY, IqestCmd, iqstart, 1);

	/* wait for estimate */
	SPINWAIT((READ_LCN20PHYREGFLD(pi, IqestCmd, iqstart) != 0), timeout_us);

	if (READ_LCN20PHYREGFLD(pi, IqestCmd, iqstart) == 0) {
		est->i_pwr = (READ_LCN20PHYREGFLD(pi, IqestipwrAccHi, ipwrAccHi) << 16) |
			READ_LCN20PHYREGFLD(pi, IqestipwrAccLo, ipwrAccLo);
		est->q_pwr = (READ_LCN20PHYREGFLD(pi, IqestqpwrAccHi, qpwrAccHi) << 16) |
			READ_LCN20PHYREGFLD(pi, IqestqpwrAccLo, qpwrAccLo);
		est->iq_prod = (READ_LCN20PHYREGFLD(pi, IqestIqAccHi, iqAccHi) << 16) |
			READ_LCN20PHYREGFLD(pi, IqestIqAccLo, iqAccLo);
		PHY_INFORM(("wlc_lcn20phy_rx_iq_est: "
					"i_pwr = %u, q_pwr = %u, iq_prod = %d\n",
					est->i_pwr, est->q_pwr,
					est->iq_prod));
	} else {
		PHY_ERROR(("wl%d: %s: IQ measurement timed out\n", pi->sh->unit, __FUNCTION__));
	}
}

static void
wlc_lcn20phy_rx_iq_cal_txrxgain_control(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	const lcn20phy_rxcal_rxgain_t *gaintbl;
	uint8 g_index, done, found_ideal, wn, txindex;
	bool txdone;
	uint8 do_max = 8;
	uint8 tia, far;
	uint16 num_samps_log2 = 10;
	uint16 num_samps;
	uint32 meansq_max = 7000;
	uint32 meansq_min = 1111;
	uint32 i_meansq, q_meansq;
	uint8 dvga2 = 0;
	/* the gain range is from +8dBm to 20dBm, each step is 3 dB */
	const uint8 txindx_start = 56;
	const uint8 txindx_stop  = 56;
	const uint8 txindx_step  = 12;
	int p;
	phy_iq_est_t est = {0, 0, 0};

	ASSERT(pi_lcn20->rxiqcal_lpbkpath <= LCN20PHY_RXIQCAL_PAPDPATH);

	if (pi_lcn20->rxiqcal_lpbkpath == LCN20PHY_RXIQCAL_PAPDPATH)
		gaintbl = gaintbl_papdpath;
	else
		gaintbl = gaintbl_lkpath;

	num_samps = 1 << num_samps_log2;
	g_index = 8;
	done = 0;
	found_ideal = 0;
	txdone = 0;
	wn = 0;
	txindex = txindx_start;

	wlc_lcn20phy_start_tx_tone(pi, (2*1000*1000),
		wlc_lcn20phy_get_rxiqcal_toneamp(), 0, TRUE, 1);

	while ((done != do_max) && (g_index != 0) && (g_index != LCN20PHY_RXCAL_NUMRXGAINS)) {
		if (pi_lcn20->rxiqcal_lpbkpath == LCN20PHY_RXIQCAL_PAPDPATH) {
			/* papd loopback path */
			tia = gaintbl[g_index].tia;
			far = gaintbl[g_index].far;
			dvga2 = gaintbl[g_index].dvga;
		} else {
			/* leakage path for 5G */
			/* lna = gaintbl[g_index].lna; */
			tia = gaintbl[g_index].tia;
			far = gaintbl[g_index].far;
		}
		wlc_lcn20phy_rx_gain_override_wrapper(pi, 1, 0, 0, 0, 0, 0, tia, far, dvga2);
		wlc_lcn20phy_rx_gain_override_enable_wrapper(pi, TRUE);

		if (!txdone)
			wlc_lcn20phy_set_tx_pwr_by_index(pi, txindex);

		/* RSSI reading */
		for (p = 0; p < 8; p++) {
			wn +=  READ_RADIO_REGFLD_20692(pi, WL_TIA_CFG13, 0, nbrssi_Ich_low);
			wn +=  READ_RADIO_REGFLD_20692(pi, WL_TIA_CFG13, 0, nbrssi_Qch_low);
		}

		/* estimate digital power using rx_iq_est */
		wlc_lcn20phy_rx_iq_est(pi, &est, num_samps, 32, 0, FALSE);

		i_meansq = (est.i_pwr + (num_samps >> 1)) >> num_samps_log2;
		q_meansq = (est.q_pwr + (num_samps >> 1)) >> num_samps_log2;

		PHY_INFORM(("Rx IQCAL: txindx=%d g_index=%d tia=%d far=%d dvga=%d\n",
			txindex, g_index, tia, far, dvga2));
		PHY_INFORM(("Rx IQCAL: i_meansq=%d q_meansq=%d meansq_max=%d meansq_min=%d\n",
			i_meansq, q_meansq, meansq_max, meansq_min));

		txdone = txdone ||
			(txindex < txindx_stop) || (wn > 0) || (i_meansq > meansq_min) ||
			(q_meansq > meansq_min);

		if (!txdone) {
			txindex -= txindx_step;
			continue;
		}

		if ((i_meansq > meansq_max) || (q_meansq > meansq_max)) {
			g_index--;
			done++;
		} else if ((i_meansq < meansq_min) && (q_meansq < meansq_min)) {
			g_index++;
			done++;
		} else {
			done = do_max;
			found_ideal = 1;
		}
	}
	if (!found_ideal) {
		PHY_ERROR(("%s: Too much or too little power? (gain_index=%d)\n",
			__FUNCTION__, g_index));
	}
	wlc_lcn20phy_stop_tx_tone(pi);
}

static void
wlc_lcn20phy_rx_iq_cal_loopback_setup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_lcn20phy_deaf_mode(pi, TRUE);
	wlc_lcn20phy_rx_iq_cal_phy_setup(pi);

	if (!NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn20phy_radio20692_rx_iq_cal_setup(pi);
	}

	if (pi_lcn20->dccalen_lpbkpath) {
		wlc_lcn20phy_start_tx_tone(pi, (2*1000*1000),
			wlc_lcn20phy_get_rxiqcal_toneamp(), 0, TRUE, 1);
		wlc_lcn20phy_set_bbmult(pi, 0);
		wlc_lcn20phy_dccal_auxlna(pi);
		wlc_lcn20phy_dccal_ovr_idac(pi, 1, 0, 0);
		wlc_lcn20phy_stop_tx_tone(pi);
	}
	wlc_lcn20phy_rx_iq_cal_txrxgain_control(pi);
}

static void
wlc_lcn20phy_rx_iq_comp(phy_info_t *pi, uint8 setmode, phy_iq_comp_t *pcomp)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* write values */
	switch (setmode) {
		case LCN20PHY_RXIQCOMP_GET:
			pcomp->a = READ_LCN20PHYREG(pi, Core1RxIQCompA0);
			pcomp->b = READ_LCN20PHYREG(pi, Core1RxIQCompB0);
			break;
		case LCN20PHY_RXIQCOMP_SET:
			WRITE_LCN20PHYREG(pi, Core1RxIQCompA0, pcomp->a);
			WRITE_LCN20PHYREG(pi, Core1RxIQCompB0, pcomp->b);
			break;
		case LCN20PHY_RXIQCOMP_RESET:
			WRITE_LCN20PHYREG(pi, Core1RxIQCompA0, 0);
			WRITE_LCN20PHYREG(pi, Core1RxIQCompB0, 0);
			break;
		default:
			ASSERT(setmode <= 2);
			break;
	}
}

static void
wlc_lcn20phy_calc_iq_mismatch(phy_info_t *pi, phy_iq_est_t *est, lcn20phy_iq_mismatch_t *mismatch)
{
	int32  iq = est->iq_prod;
	uint32 ii = est->i_pwr;
	uint32 qq = est->q_pwr;

	int16  iq_nbits, qq_nbits, ii_nbits;
	int32  tmp;
	int32  den, num;
	int32  angle;
	math_cint32 val;
	BCM_REFERENCE(pi);

	/* angle = asin (-iq / sqrt( ii*qq ))
	* mag   = sqrt ( qq/ii )
	*/
	iq_nbits = math_nbits_32(iq);
	qq_nbits = math_nbits_32(qq);
	ii_nbits = math_nbits_32(ii);
	if (ii_nbits > qq_nbits)
		qq_nbits = ii_nbits;

	if (30 >=  qq_nbits) {
		tmp = ii;
		tmp = tmp << (30 - qq_nbits);
		den = (int32) math_sqrt_int_32((uint32) tmp);
		tmp = qq;
		tmp = tmp << (30 - qq_nbits);
		den *= (int32) math_sqrt_int_32((uint32) tmp);
	} else {
		tmp = ii;
		tmp = tmp >> (qq_nbits - 30);
		den = (int32) math_sqrt_int_32((uint32) tmp);
		tmp = qq;
		tmp = tmp >> (qq_nbits - 30);
		den *= (int32) math_sqrt_int_32((uint32) tmp);
	}
	if (qq_nbits <= iq_nbits + 16) {
		den = den >> (16 + iq_nbits - qq_nbits);
	} else {
		den = den << (qq_nbits - (16 + iq_nbits));
	}

	tmp = -iq;
	num = (tmp << (30 - iq_nbits));
	if (num > 0)
		num += (den >> 1);
	else
		num -= (den >> 1);

	if (den == 0) {
		tmp = 0;
	} else {
		tmp = num / den; /* in X,16 */
	}

	mismatch->sin_angle = tmp;

	tmp = (tmp >> 1);
	tmp *= tmp;
	tmp = (1 << 30) - tmp;
	val.i = (int32) math_sqrt_int_32((uint32) tmp);
	val.i = ( val.i << 1) ;

	val.q = mismatch->sin_angle;
	math_cmplx_invcordic(val, &angle);
	mismatch->angle = angle; /* in X,16 */

	iq_nbits = math_nbits_32(qq - ii);
	if (iq_nbits % 2 == 1)
		iq_nbits++;

	den = ii;

	num = qq - ii;
	num = num << (30 - iq_nbits);
	if (iq_nbits > 10)
		den = den >> (iq_nbits - 10);
	else
		den = den << (10 - iq_nbits);
	if (num > 0)
		num += (den >> 1);
	else
		num -= (den >> 1);

	if (den == 0) {
		mismatch->mag = (1 << 10); /* in X,10 */
	} else {
		tmp = num / den + (1 << 20);
		mismatch->mag = (int32) math_sqrt_int_32((uint32) tmp); /* in X,10 */
	}
	PHY_INFORM(("      Mag=%d, Angle=%d, cos(angle)=%d, sin(angle)=%d\n",
	(int)mismatch->mag, (int)mismatch->angle, (int)val.i, (int)val.q));
}

static void
wlc_lcn20phy_rxiqcal_pnp(phy_info_t *pi, phy_iq_est_t *iqest, int32 *angle, int32 *mag)
{
	lcn20phy_iq_mismatch_t mismatch;

	wlc_lcn20phy_calc_iq_mismatch(pi, iqest, &mismatch);
	*angle = mismatch.angle;
	*mag = mismatch.mag;
}

static void
wlc_lcn20phy_lin_reg(phy_info_t *pi, lcn20phy_rx_fam_t *freq_ang_mag, uint16 num_data)
{
	int32 Sf2 = 0;
	int32 Sfa, Sa, Sm;
	int32 intcp, mag;
	int8 idx;
	phy_iq_comp_t coeffs;
	int32 sin_angle, cos_angle;
	math_cint32 cordic_out;
	int32  a, b, sign_sa;

	Sfa = 0; Sa = 0; Sm = 0;

	for (idx = 0; idx < num_data; idx++) {
		Sf2 += freq_ang_mag[idx].freq * freq_ang_mag[idx].freq;

		Sfa += freq_ang_mag[idx].freq * freq_ang_mag[idx].angle;
		Sa += freq_ang_mag[idx].angle;
		Sm += freq_ang_mag[idx].mag;
	}

	sign_sa = Sa >= 0 ? 1 : -1;
	intcp = (Sa + sign_sa * (num_data >> 1)) / num_data;
	mag   = (Sm + (num_data >> 1)) / num_data;

	math_cmplx_cordic(intcp, &cordic_out);
	sin_angle = cordic_out.q;
	cos_angle = cordic_out.i;

	b = mag * cos_angle;
	a = mag * sin_angle;

	b = ((b >> 15) + 1) >> 1;
	b -= (1 << 10);  /* 10 bit */
	a = ((a >> 15) + 1) >> 1;

	a = (a < -512) ? -512 : ((a > 511) ? 511 : a);
	b = (b < -512) ? -512 : ((b > 511) ? 511 : b);

	coeffs.a = a & 0x3ff;
	coeffs.b = b & 0x3ff;

	PHY_INFORM(("   a=%d b=%d :: ", a, b));

	wlc_lcn20phy_rx_iq_comp(pi, LCN20PHY_RXIQCOMP_SET, &coeffs);
}

static void
wlc_lcn20phy_rx_iq_cal_phy_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_phyregs_t *psave = pi_lcn20->rxiqcal_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	/* PAPD, PAPR, TX gain, TX power control restore */
	wlc_lcn20phy_set_tx_pwr_by_index(pi, psave->txidx);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, psave->SAVE_txpwrctrl_on);
	WRITE_LCN20PHYREG(pi, papr_ctrl, psave->papr_ctrl);
	WRITE_LCN20PHYREG(pi, PapdEnable0, psave->PapdEnable0);
	WRITE_LCN20PHYREG(pi, Core1TxControl, psave->SAVE_Core1TxControl);
	wlc_lcn20phy_set_bbmult(pi, psave->bbmult);

	/* Save RFCTRL state, save all RFCTRL override register */
	WRITE_LCN20PHYREG(pi, RFOverride0, psave->RFOverride0);
	WRITE_LCN20PHYREG(pi, RFOverrideVal0, psave->RFOverrideVal0);
	WRITE_LCN20PHYREG(pi, rfoverride2, psave->rfoverride2);
	WRITE_LCN20PHYREG(pi, rfoverride2val, psave->rfoverride2val);
	WRITE_LCN20PHYREG(pi, rfoverride3, psave->rfoverride3);
	WRITE_LCN20PHYREG(pi, rfoverride3_val, psave->rfoverride3_val);
	WRITE_LCN20PHYREG(pi, rfoverride4, psave->rfoverride4);
	WRITE_LCN20PHYREG(pi, rfoverride4val, psave->rfoverride4val);
	WRITE_LCN20PHYREG(pi, rfoverride5, psave->rfoverride5);
	WRITE_LCN20PHYREG(pi, rfoverride5val, psave->rfoverride5val);
	WRITE_LCN20PHYREG(pi, rfoverride7, psave->rfoverride7);
	WRITE_LCN20PHYREG(pi, rfoverride7val, psave->rfoverride7val);
	WRITE_LCN20PHYREG(pi, rfoverride8, psave->rfoverride8);
	WRITE_LCN20PHYREG(pi, rfoverride8val, psave->rfoverride8val);

	/* save DSSF control register */
	WRITE_LCN20PHYREG(pi, DSSF_control_0, psave->DSSF_control_0);

	/* ensure the cal clock has been enabled */
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, psave->sslpnCalibClkEnCtrl);

	/* save Rx/Tx farrow related register setting and DVGA2 related register setting */
	WRITE_LCN20PHYREG(pi, RxSdFeConfig1, psave->RxSdFeConfig1);
	WRITE_LCN20PHYREG(pi, RxSdFeConfig6, psave->RxSdFeConfig6);
	WRITE_LCN20PHYREG(pi, phyreg2dvga2, psave->phyreg2dvga2);
	WRITE_LCN20PHYREG(pi, RxFeCtrl1, psave->RxFeCtrl1);

	wlc_lcn20phy_dccal_ovr_idac(pi, 0, 0, 0);
}

static void
wlc_lcn20phy_rx_iq_cal_loopback_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (!NORADIO_ENAB(pi->pubpi)) {
		wlc_lcn20phy_radio20692_rx_iq_cal_cleanup(pi);
	}

	wlc_lcn20phy_rx_iq_cal_phy_cleanup(pi);

	wlc_lcn20phy_deaf_mode(pi, FALSE);

	if (pi_lcn20->dccalen_lpbkpath) {
		wlc_lcn20phy_dccal_ovr_idac(pi, 0, 0, 0);
	}
}

static void
wlc_lcn20phy_rx_iq_cal(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	phy_iq_comp_t pcomp;
	int8 rxiqcal_freqs[LCN20PHY_RXIQCAL_MAXNUM_FREQS];
	uint8 fidx;
	lcn20phy_rx_fam_t freq_ang_mag[LCN20PHY_RXIQCAL_MAXNUM_FREQS];
	phy_iq_est_t loopback_rx_iq;
	uint16 num_samps;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	pi_lcn20->dccalen_lpbkpath = 0;
	pi_lcn20->rxiqcal_lpbkpath = LCN20PHY_RXIQCAL_PAPDPATH;

	wlc_lcn20phy_rx_iq_cal_loopback_setup(pi);

	wlc_lcn20phy_rx_iq_comp(pi, LCN20PHY_RXIQCOMP_RESET, &pcomp);

	rxiqcal_freqs[0] = (wlc_lcn20phy_get_rxiqcal_tonefreqkhz_0()/1000);
	rxiqcal_freqs[1] = - rxiqcal_freqs[0];
	rxiqcal_freqs[2] = (wlc_lcn20phy_get_rxiqcal_tonefreqkhz_1()/1000);
	rxiqcal_freqs[3] = - rxiqcal_freqs[2];

	num_samps = wlc_lcn20phy_get_rxiqcal_numsamps();

	for (fidx = 0; fidx < LCN20PHY_RXIQCAL_MAXNUM_FREQS; fidx++) {
		int32 tone_freqhz;

		tone_freqhz = (int32)(rxiqcal_freqs[fidx] * 1000);
		tone_freqhz = tone_freqhz * 1000;

		/* Freq in MHz */
		freq_ang_mag[fidx].freq = rxiqcal_freqs[fidx];
		wlc_lcn20phy_start_tx_tone(pi, tone_freqhz,
			wlc_lcn20phy_get_rxiqcal_toneamp(), 0, TRUE, 1);

		wlc_lcn20phy_rx_iq_est(pi, &loopback_rx_iq, num_samps, 32, 0, FALSE);

		wlc_lcn20phy_stop_tx_tone(pi);

		wlc_lcn20phy_rxiqcal_pnp(pi, &loopback_rx_iq,
			&(freq_ang_mag[fidx].angle), &(freq_ang_mag[fidx].mag));
	}

	/* convert [mag phase] to [a b] coeff for IQ compensation block */
	wlc_lcn20phy_lin_reg(pi, freq_ang_mag, LCN20PHY_RXIQCAL_MAXNUM_FREQS);

	wlc_lcn20phy_rx_iq_cal_loopback_cleanup(pi);
}

/* END: RXIQCAL Above functions are part of Rx IQ Calibrations  feature */

/* START: TXIQLOCAL Below functions are part of Tx IQ LO calibration feature */

static void
wlc_lcn20phy_tx_iqlo_cal_phy_save_state(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_txiqlocal_phyregs_t *psave = pi_lcn20->txiqlocal_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	/* Save tx gain state
	* save bb_mult, txgain, papr/IIR related parameter
	*/
	psave->bbmult = wlc_lcn20phy_get_bbmult(pi);

	/* Save and Adjust State of various PhyRegs
	* Internal RFCtrl: save and adjust state of internal PA override
	*/
	psave->RxFeCtrl1 = READ_LCN20PHYREG(pi, RxFeCtrl1);
	psave->TxPwrCtrlCmd = READ_LCN20PHYREG(pi, TxPwrCtrlCmd);
	psave->RxSdFeConfig1 = READ_LCN20PHYREG(pi, RxSdFeConfig1);
	psave->sslpnCalibClkEnCtrl = READ_LCN20PHYREG(pi, sslpnCalibClkEnCtrl);
	psave->AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
	psave->AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);
	psave->ClkEnCtrl = READ_LCN20PHYREG(pi, ClkEnCtrl);
	psave->RFOverride0 = READ_LCN20PHYREG(pi, RFOverride0);
	psave->RFOverrideVal0 = READ_LCN20PHYREG(pi, RFOverrideVal0);
	psave->rfoverride4 = READ_LCN20PHYREG(pi, rfoverride4);
	psave->rfoverride4val = READ_LCN20PHYREG(pi, rfoverride4val);
	psave->PapdEnable = READ_LCN20PHYREG(pi, PapdEnable0);
}

void
wlc_phy_radio20692_tx_iqlo_cal_radio_setup_save_state(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_txiqcal_radioregs_t *psave = pi_lcn20->txiqcal_radioregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	psave->iqcal_cfg1 = READ_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0);
	psave->tssi_iqcal_ovr1 = READ_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0);
	psave->auxpga_cfg1 = READ_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0);
	psave->iqcal_cfg3 = READ_RADIO_REG_20692(pi, WL_IQCAL_CFG3, 0);
	psave->adc_cfg10 = READ_RADIO_REG_20692(pi, WL_ADC_CFG10, 0);
	psave->AUX_RXPGA_ovr1 = READ_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0);
	psave->GPABuf_ovr1 = READ_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0);
	psave->pa2g_cfg1 = READ_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0);
	psave->rx_adc_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0);
	psave->rx_top_2g_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0);
	psave->tx_top_2g_ovr2 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0);
	psave->rx_bb_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0);
	psave->rx_bb_ovr2 = READ_RADIO_REG_20692(pi, WL_RX_BB_OVR2, 0);
	psave->minipmu_ovr1 = READ_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0);
}

static void
wlc_phy_radio20692_tx_iqlo_cal_radio_setup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_radio20692_tx_iqlo_cal_radio_setup_save_state(pi);

	/* Enabling and Muxing */
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_sel_sw, 0x8);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_tssi_ctrl_sel, 0);
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_tssi_GPIO_ctrl, 0x0);

	/* power up iqlocal / powerdown tssi */
	MOD_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, wl_ovr_iqcal_PU_iqcal, 1);
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_PU_iqcal, 0x1);
	MOD_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, wl_ovr_iqcal_PU_tssi, 1);
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_PU_tssi, 0);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_pa2g_tssi_ctrl_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_pa2g_tssi_ctrl_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_pa2g_tssi_ctrl_range, 1);
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_pa2g_tssi_ctrl_range, 0);
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_pa2g_tssi_ctrl, 3);

	/* powerup testbuf and configure for txiqcal */
	MOD_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, wl_ovr_testbuf_PU, 1);
	MOD_RADIO_REG_20692(pi, WL_TESTBUF_CFG1, 0, wl_testbuf_PU, 1);
	MOD_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, wl_ovr_testbuf_sel_test_port, 1);
	MOD_RADIO_REG_20692(pi, WL_TESTBUF_CFG1, 0, wl_testbuf_sel_test_port, 0);

	/* powerup auxpga */
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, wl_auxpga_i_pu, 1);

	/* Setup auxpga for txiqcal */
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_sel_input, 1);
	MOD_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, wl_auxpga_i_sel_input, 0x0);
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_sel_gain, 1);
	MOD_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0,
		wl_auxpga_i_sel_gain, pi_lcn20->iqcal_auxpga_gain);
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_sel_vmid, 1);
	MOD_RADIO_REG_20692(pi, WL_AUXPGA_VMID, 0,
		wl_auxpga_i_sel_vmid, pi_lcn20->iqcal_auxpga_vmid);

	/* configure the testmux to connect auxpga to adc */
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_in_test, 1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG10, 0, wl_adc_in_test, 0xF);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG5, 0, wl_tia_out_test, 0x0);
	/* ensure that the dac mux is OFF because it shares a line with the auxpga output */
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_txbb_dac2adc, 0x0);

	/* pwr up adc */
	/*
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_clk_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_clk_slow_pu, 1);
	*/
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_slow_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_sipo_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_sipo_pu, 1);

	/* powerdown lna1 */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_lna1_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_lna1_pu, 0);

	/* powerdown rx main gm2g */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_gm2g_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_pwrup, 0x0);

	/* powerdown auxgm2g */
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_auxgm_pwrup, 0);

	/* powerdown rx mixer */
	/*
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxmix2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxmix2g_pu, 0);
	*/

	/* powerdown rxdiv2g */
	/*
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxdiv2g_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rxdiv2g_pu_bias, 0);
	*/

	/* powerdown tia */
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_resstring, 1);

	/* Power up common mode reference  */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_resstring, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_amp1_pwrup, 1);

	/* PU 1st amp */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_amp1_pwrup, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_amp2, 1);

	/* PU 2nd amp   */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_amp2, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_amp2_bypass, 1);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG1, 0, wl_tia_amp2_bypass, 0);

	/* powerdown rssi */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_dig_wrssi1_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_RSSI1, 0, wl_lna2g_dig_wrssi1_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR2, 0, wl_ovr_tia_offset_rssi_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG11, 0, wl_tia_rssi_pwrup, 0);
}

static void
wlc_phy_radio20692_tx_iqlo_cal_radio_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_txiqcal_radioregs_t *psave = pi_lcn20->txiqcal_radioregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	WRITE_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, psave->iqcal_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, psave->tssi_iqcal_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, psave->auxpga_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_IQCAL_CFG3, 0, psave->iqcal_cfg3);
	WRITE_RADIO_REG_20692(pi, WL_ADC_CFG10, 0, psave->adc_cfg10);
	WRITE_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, psave->AUX_RXPGA_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, psave->GPABuf_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, psave->pa2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, psave->rx_adc_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, psave->rx_top_2g_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, psave->tx_top_2g_ovr2);
	WRITE_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, psave->rx_bb_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_RX_BB_OVR2, 0, psave->rx_bb_ovr2);
	WRITE_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, psave->minipmu_ovr1);
}

static void
wlc_lcn20phy_tx_iqlo_cal_phy_setup(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/*
	* Note that this Routine also takes care of Phyregs that control the
	* Radio through RFCtrl, while the corresponding radio_setup Routine
	* only takes care of Radio Settings that are controlled via direct
	* Radio-reg Access
	*/

	/* set sd adc full scale */
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, 3);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 0);

	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1);

	/* disable papd comp */
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, 0);

	/*
	* The phy reg overrides here are from lcn20_play_tone routine
	* as they are not set there during iqlocal
	* The sequence of power up is important
	*/
	/* Power up mixer, cascode, ... */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrftxpu_ovr_val, 1);

	/* Force TR switch to Tx for iTR and eTR configurations */
	wlc_lcn20phy_set_trsw_override(pi, LCN20PHY_TRS_TXMODE, FALSE);

	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, trsw_tx_pwrup_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, trsw_tx_pwrup_ovr_val, 1);
	/* Power up DAC and LPF: */
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	/* Power up iPA: Make sure iPA is powered-up only AFTER TR-switch is configured for TX */
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4, papu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride4val, papu_ovr_val, 1);
}

static void
wlc_lcn20phy_tx_iqlo_cal_phy_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_txiqlocal_phyregs_t *psave = pi_lcn20->txiqlocal_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	/* Save tx gain state
	* save bb_mult, txgain, papr/IIR related parameter
	*/
	wlc_lcn20phy_set_bbmult(pi, psave->bbmult);

	/* Save and Adjust State of various PhyRegs
	* Internal RFCtrl: save and adjust state of internal PA override
	*/
	WRITE_LCN20PHYREG(pi, RxFeCtrl1, psave->RxFeCtrl1);
	WRITE_LCN20PHYREG(pi, TxPwrCtrlCmd, psave->TxPwrCtrlCmd);
	WRITE_LCN20PHYREG(pi, RxSdFeConfig1, psave->RxSdFeConfig1);
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, psave->sslpnCalibClkEnCtrl);
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, psave->AfeCtrlOvr1Val);
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, psave->AfeCtrlOvr1);
	WRITE_LCN20PHYREG(pi, ClkEnCtrl, psave->ClkEnCtrl);
	WRITE_LCN20PHYREG(pi, RFOverride0, psave->RFOverride0);
	WRITE_LCN20PHYREG(pi, RFOverrideVal0, psave->RFOverrideVal0);
	WRITE_LCN20PHYREG(pi, rfoverride4, psave->rfoverride4);
	WRITE_LCN20PHYREG(pi, rfoverride4val, psave->rfoverride4val);
	WRITE_LCN20PHYREG(pi, PapdEnable0, psave->PapdEnable);
}

void
wlc_lcn20phy_tx_iqlo_cal_setup(phy_info_t *pi, phy_txiqlocal_mode_t cal_mode,
	uint16 *bbmult_for_lad)
{
	uint8 lad_len;
	BCM_REFERENCE(cal_mode);

	/* first save the original values of all the registers we are going to touch */
	wlc_lcn20phy_tx_iqlo_cal_phy_save_state(pi);

	/* Radio and Phy Setup */
	wlc_phy_radio20692_tx_iqlo_cal_radio_setup(pi);
	wlc_lcn20phy_tx_iqlo_cal_phy_setup(pi);
	wlc_lcn20phy_deaf_mode(pi, TRUE);

	/* Enable all clk */
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, 0x0808);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);

	/* no radio LOFT or programmable radio gain */
	WRITE_LCN20PHYREG(pi, TX_iqcal_gain_bwAddress, 0);
	WRITE_LCN20PHYREG(pi, TX_loft_fine_iAddress, 0);
	WRITE_LCN20PHYREG(pi, TX_loft_fine_qAddress, 0);
	WRITE_LCN20PHYREG(pi, TX_loft_coarse_iAddress, 0);
	WRITE_LCN20PHYREG(pi, TX_loft_coarse_qAddress, 0);

	/* save for gain ladder setup */
	*bbmult_for_lad = wlc_lcn20phy_get_bbmult(pi);

	lad_len = sizeof(lcn20phy_txiqlocal_ladder_lo)/
		sizeof(lcn20phy_txiqcal_ladder_t);
	/* ladders assumed to be of equal and even lengths */
	ASSERT(sizeof(lcn20phy_txiqlocal_ladder_lo) ==
		sizeof(lcn20phy_txiqlocal_ladder_lo));
	ASSERT((sizeof(lcn20phy_txiqlocal_ladder_lo) & 0x1) == 0);
	/* Turn on Testtone in iqcal mode */
	/* if tone is already played out with iq cal mode zero then
	* stop the tone and re-play with iq cal mode 1.
	*/
	wlc_lcn20phy_stop_tx_tone(pi);
	OSL_DELAY(5);
	wlc_lcn20phy_start_tx_tone(pi, wlc_lcn20phy_get_txiqlocal_tonefhz(),
		wlc_lcn20phy_get_txiqlocal_toneamp(), LCN20PHY_TXIQLOCAL_SPBMODE, TRUE, 0);
	/* Set Gain Control Parameters including IQ/LO cal enable bit
	*  iqlocal_en<15> = 1
	* gain start_index = 0xa
	* ladder_length
	*/
	WRITE_LCN20PHYREG(pi, iqloCalCmdGctl,
		(1 << LCN20PHY_iqloCalCmdGctl_iqlo_cal_en_SHIFT) |
		(0xa << LCN20PHY_iqloCalCmdGctl_index_gctl_start_SHIFT) |
		((lad_len/2) << LCN20PHY_iqloCalCmdGctl_gctl_LADlen_d2_SHIFT));

	/* Interval Lengths (number of samples) */
	WRITE_LCN20PHYREG(pi, iqloCalCmdNnum, LCN20PHY_TXIQLOCAL_CMDNUM);

}

static void
wlc_lcn20phy_tx_iqlo_cal(phy_info_t *pi, phy_txiqlocal_mode_t cal_mode)
{
	/* Cal Commands
	*
	* This uses the following format (three hex nibbles left to right)
	* 1. cal_type: 0 = IQ (a/b),   1 = deprecated 2 = LOFT digital (di/dq)
	* 2. initial stepsize (in log2)
	* 3. number of cal precision "levels"
	*/
	uint16 commands_restart[] =  {0x265, 0x234, 0x084, 0x074, 0x056};
	uint16 commands_refine[] =  {0x265, 0x234, 0x084, 0x074, 0x056};
	uint16 commands_dlo_recal[] = {0x265, 0x234};

	uint16 syst_coeffs[] =
		{0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
	uint16 coeffs[2], bbmult_for_lad;
	uint8 n_cal_cmds;
	uint16 *cal_cmds = NULL;

#if 0 //defined(PHYCAL_CACHING) //for testing to complie for now
	//ch_calcache_t *ctx = wlc_phy_get_chanctx(pi, pi->radio_chanspec);
	//lcnphy_calcache_t *cache = NULL;

	//if (ctx)
	//	cache = &ctx->u.lcnphy_cache;
#endif // endif

	if (NORADIO_ENAB(pi->pubpi))
		return;

	wlc_lcn20phy_tx_iqlo_cal_setup(pi, cal_mode, &bbmult_for_lad);

	/* Retrieve Start Coefficients */
	switch (cal_mode) {
		case TXIQLOCAL_RESTART:
			cal_cmds = commands_restart;
			n_cal_cmds = ARRAYSIZE(commands_restart);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL, syst_coeffs,
				ARRAYSIZE(syst_coeffs), 16, LCN20PHY_TXIQLOCAL_IQCOEFF_OFFSET);
			break;

		case TXIQLOCAL_REFINE:
			cal_cmds = commands_refine;
			n_cal_cmds = ARRAYSIZE(commands_refine);
			/* for refine, get from data path regs
			* assumes dig BPHY = dig OFDM coeffs)
			*/
			/* iq comp */
			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQCOEFF_OFFSET);
			/* dig loft comp */
			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOCOEFF_OFFSET);
			break;

		case TXIQLOCAL_DLORECAL:
			cal_cmds = commands_dlo_recal;
			n_cal_cmds = ARRAYSIZE(commands_dlo_recal);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL, syst_coeffs,
				ARRAYSIZE(syst_coeffs), 16, LCN20PHY_TXIQLOCAL_DLOCOEFF_OFFSET);
			break;

		default:
			ASSERT(FALSE);
			goto cleanup;
	}

	wlc_lcn20phy_tx_iqlo_cal_execute(pi, bbmult_for_lad, n_cal_cmds, cal_cmds, coeffs);

	wlc_lcn20phy_tx_iqlo_cal_process_results(pi, coeffs, cal_mode);

cleanup:
	wlc_lcn20phy_tx_iqlo_cal_cleanup(pi);
}

static void
wlc_lcn20phy_populate_tx_iqlo_comp_tables(phy_info_t *pi, uint8 startidx, uint8 stopidx)
{
	uint16 iqcomp[2], didq, idx;
	uint32 *val32_array = NULL;
	uint16 *val16_array = NULL;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if ((val32_array = (uint32*)
		LCN20PHY_MALLOC(pi, (stopidx - startidx + 1) * sizeof(uint32))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}

	/* load IQ comp tables (19:10 -- "a" coeff   9:0 -- "b" coeff)
	*   load LO comp tables (15:8 -- I offset   7:0 -- Q offset)
	*   get from IQ cal result (iqloCaltbl) and use same IQ/LO comp for all
	*/
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
		iqcomp, 2, 16, LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET);

	for (idx = 0; idx <= (stopidx - startidx); idx++) {
		*(val32_array + idx) = (*(val32_array + idx) & 0x0ff00000) |
			((uint32)(iqcomp[0] & 0x3FF) << 10) | (iqcomp[1] & 0x3ff);
	}
	wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_TXPWRCTL,
		val32_array, (stopidx - startidx + 1), 32, LCN20PHY_TX_PWR_CTRL_IQ_OFFSET);

	if (val32_array)
			LCN20PHY_MFREE(pi, val_array, (stopidx - startidx + 1) * sizeof(uint32));

	if ((val16_array = (uint16*)
		LCN20PHY_MALLOC(pi, (stopidx - startidx + 1) * sizeof(uint16))) == NULL) {
		PHY_ERROR(("wl%d: %s: MALLOC failure\n", pi->sh->unit, __FUNCTION__));
		return;
	}
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
		&didq, 1, 16, LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET);

	for (idx = 0; idx <= (stopidx - startidx); idx++)
			*(val16_array + idx) = didq;
	wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_TXPWRCTL,
		val16_array, (stopidx - startidx + 1), 16, LCN20PHY_TX_PWR_CTRL_LO_OFFSET);

	if (val16_array)
		LCN20PHY_MFREE(pi, val16_array, (stopidx - startidx + 1) * sizeof(uint16));
}

/* Top level chip dependent iqlo cal routine
* Pick gain or gains at which to run cal and populate per gain index cal coeff tables
*/
static void
wlc_lcn20phy_tx_iqlo_cal_txpwr(phy_info_t *pi)
{
	uint8 SAVE_indx; /* SAVE_lpf_ofdm_tx_bw, SAVE_lpf_cck_tx_bw; */
	uint16 SAVE_txpwrctrl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	SAVE_txpwrctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	SAVE_indx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	/* ofdm and cck setting */

	/* set LPF bw for ofdm and cck */
	PHY_REG_MOD(pi, LCN20PHY, lpfbwlutreg3, lpf_cck_tx_bw_ctl, 0);

	wlc_lcn20phy_set_tx_pwr_by_index(pi, 70);

	wlc_lcn20phy_tx_iqlo_cal(pi, TXIQLOCAL_RESTART);

	wlc_lcn20phy_populate_tx_iqlo_comp_tables(pi, 0, 127);

	wlc_lcn20phy_set_tx_pwr_by_index(pi, SAVE_indx);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, SAVE_txpwrctrl);
}

/* END: TXIQLOCAL Above functions are part of Tx IQ LO calibration feature */

/* BEGIN : SROM Below functions are part of srom read feature */
static const char BCMATTACHDATA(rstr_AvVmid_c0)[] = "AvVmid_c0";
static const char BCMATTACHDATA(rstr_AvVmidIQcal)[] = "AvVmidIQcal";

static const char BCMATTACHDATA(rstr_pa2ga0)[] = "pa2ga0";
static const char BCMATTACHDATA(rstr_pa2ga0_lo)[] = "pa2ga0_lo";
static const char BCMATTACHDATA(rstr_pa2ga0_2pwr)[] = "pa2ga0_2pwr";
static const char BCMATTACHDATA(rstr_pmax)[] = "pmax";
static const char BCMATTACHDATA(rstr_pmin)[] = "pmin";

static const char BCMATTACHDATA(rstr_tssitime)[] = "tssitime";
static const char BCMATTACHDATA(rstr_itssicorr)[] = "itssicorr";
static const char BCMATTACHDATA(rstr_initxidx)[] = "initxidx";
static const char BCMATTACHDATA(rstr_tssifloor2g)[] = "tssifloor2g";

static const char BCMATTACHDATA(rstr_cckdigfilttype)[] = "cckdigfilttype";
static const char BCMATTACHDATA(rstr_ofdmdigfilttype)[] = "ofdmdigfilttype";
static const char BCMATTACHDATA(rstr_ofdmdigfilttypebe)[] = "ofdmdigfilttypebe";

static const char BCMATTACHDATA(rstr_cckscale)[] = "cckscale";

static const char BCMATTACHDATA(rstr_maxp2ga0)[] = "maxp2ga0";
static const char BCMATTACHDATA(rstr_sromrev)[] = "sromrev";
static const char BCMATTACHDATA(rstr_cckbw202gpo)[] = "cckbw202gpo";
static const char BCMATTACHDATA(rstr_cck2gpo)[] = "cck2gpo";
static const char BCMATTACHDATA(rstr_legofdmbw202gpo)[] = "legofdmbw202gpo";
static const char BCMATTACHDATA(rstr_ofdm2gpo)[] = "ofdm2gpo";
static const char BCMATTACHDATA(rstr_mcsbw202gpo)[] = "mcsbw202gpo";
static const char BCMATTACHDATA(rstr_propbw202gpo)[] = "propbw202gpo";
static const char BCMATTACHDATA(rstr_mcs2gpo1)[] = "mcs2gpo1";
static const char BCMATTACHDATA(rstr_mcs2gpo0)[] = "mcs2gpo0";
static const char BCMATTACHDATA(rstr_txpwroffset2g)[] = "txpwroffset2g";

static const char BCMATTACHDATA(rstr_temp_mult)[] = "temp_mult";
static const char BCMATTACHDATA(rstr_temp_add)[] = "temp_add";
static const char BCMATTACHDATA(rstr_temp_q)[] = "temp_q";
static const char BCMATTACHDATA(rstr_vbat_mult)[] = "vbat_mult";
static const char BCMATTACHDATA(rstr_vbat_add)[] = "vbat_add";
static const char BCMATTACHDATA(rstr_vbat_q)[] = "vbat_q";

static const char BCMATTACHDATA(rstr_edonthd20l)[] = "edonthd20l";
static const char BCMATTACHDATA(rstr_edoffthd20ul)[] = "edoffthd20ul";
#ifdef LCN20_PAPD_ENABLE
static const char BCMATTACHDATA(rstr_papdmode)[] = "papdmode";
static const char BCMATTACHDATA(rstr_papdendidx)[] = "papdendidx";
static const char BCMATTACHDATA(rstr_papdbasepwr)[] = "calidxestbase2g";
static const char BCMATTACHDATA(rstr_papdtrgtpwr)[] = "calidxesttarget2g";
static const char BCMATTACHDATA(rstr_papdcalidxoffset)[] = "papdcalidxoffset";
static const char BCMATTACHDATA(rstr_papdidxoffsetchan12)[] = "papdidxoffsetchan12";
static const char BCMATTACHDATA(rstr_papdidxoffsetchan13)[] = "papdidxoffsetchan13";
#endif /* LCN20_PAPD_ENABLE */
static const char BCMATTACHDATA(rstr_spurconfig)[] = "spurconfig";
static const char BCMATTACHDATA(rstr_dssfth)[] = "dssfth";
static const char BCMATTACHDATA(rstr_xtalpn)[] = "xtalpn";
static const char BCMATTACHDATA(rstr_rcor_aci)[] = "rcor_aci";
#if defined(WLPROPRIETARY_11N_RATES)
static const char BCMATTACHDATA(rstr_qam256en)[] = "qam256en";
#endif // endif
static const char BCMATTACHDATA(rstr_swctrlmap_2g)[] = "swctrlmap_2g";
static const char BCMATTACHDATA(rstr_swctrl_gpios)[] = "swctrl_gpios";
static const char BCMATTACHDATA(rstr_triso2g)[] = "triso2g";
static const char BCMATTACHDATA(rstr_elna_off_gain_idx_2g)[] = "elna_off_gain_idx_2g";
static const char BCMATTACHDATA(rstr_rssi_delta_2gb0)[] = "rssi_delta_2gb0";
static const char BCMATTACHDATA(rstr_rssi_delta_2gb1)[] = "rssi_delta_2gb1";
static const char BCMATTACHDATA(rstr_rssi_delta_2gb2)[] = "rssi_delta_2gb2";
static const char BCMATTACHDATA(rstr_rssi_delta_2gb3)[] = "rssi_delta_2gb3";
static const char BCMATTACHDATA(rstr_rssi_delta_2gb4)[] = "rssi_delta_2gb4";
static const char BCMATTACHDATA(rstr_rxgaintempcoeff2g)[] = "rxgaintempcoeff2g";
static const char BCMATTACHDATA(rstr_gain_cal_temp)[] = "gain_cal_temp";
static const char BCMATTACHDATA(rstr_rssi_cal_freq_grp_2g)[] = "rssi_cal_freq_grp_2g";
static const char BCMATTACHDATA(rstr_rssi_channel_offset_rout0)[] = "rssi_channel_offset_rout0";
static const char BCMATTACHDATA(rstr_rssi_channel_offset_rout8)[] = "rssi_channel_offset_rout8";
static const char BCMATTACHDATA(rstr_rssi_channel_offset_elnabyp)[] = "rssi_channel_offset_elnabyp";
static const char BCMATTACHDATA(rstr_noise_log_nsamps)[] = "noise_log_nsamps";
static const char BCMATTACHDATA(rstr_noise_iqest_en)[] = "noise_iqest_en";
static const char BCMATTACHDATA(rstr_noiseiqgainadj2g)[] = "noiseiqgainadj2g";
static const char BCMATTACHDATA(rstr_rssi_rate_offset)[] = "rssi_rate_offset";

bool
BCMATTACHFN(wlc_lcn20phy_txpwr_srom_read)(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16	papd_mode;
	uint8 i;
	uint8 spurconfig;
	int8 *p_rssi_delta_2gb0 = pi_lcn20->rssi_corr_gain_delta_2g_sub;
	int8 *p_rssi_delta_2gb1 = p_rssi_delta_2gb0 + LCN20PHY_GAIN_DELTA_2G_PARAMS;
	int8 *p_rssi_delta_2gb2 = p_rssi_delta_2gb1 + LCN20PHY_GAIN_DELTA_2G_PARAMS;
	int8 *p_rssi_delta_2gb3 = p_rssi_delta_2gb2 + LCN20PHY_GAIN_DELTA_2G_PARAMS;
	int8 *p_rssi_delta_2gb4 = p_rssi_delta_2gb3 + LCN20PHY_GAIN_DELTA_2G_PARAMS;
	uint8 cal_grp_no;

	pi_lcn20->auxpga_gain = (uint8)PHY_GETINTVAR_ARRAY(pi, rstr_AvVmid_c0, 0);
	pi_lcn20->auxpga_vmid = (uint8)PHY_GETINTVAR_ARRAY(pi, rstr_AvVmid_c0, 1);

	pi_lcn20->iqcal_auxpga_gain =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_AvVmidIQcal, 0, 0x7);
	pi_lcn20->iqcal_auxpga_vmid =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_AvVmidIQcal, 1, 0x92);

	pi_lcn20->tssical_time = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_tssitime, 0);
	pi_lcn20->idletssi_corr = (uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_itssicorr, 0);
	pi_lcn20->init_ccktxpwrindex = pi_lcn20->init_txpwrindex =
		(uint32)PHY_GETINTVAR_DEFAULT(pi, rstr_initxidx,
		LCN20PHY_TX_PWR_CTRL_START_INDEX_2G);
	pi_lcn20->tssi_floor =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_tssifloor2g, 0);

	pi_lcn20->cckdigiftype =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_cckdigfilttype, -1);
	pi_lcn20->ofdmdigiftype =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_ofdmdigfilttype, -1);
	pi_lcn20->ofdmdigiftypebe =
		(int16)PHY_GETINTVAR_DEFAULT(pi, rstr_ofdmdigfilttypebe, -1);

	pi_lcn20->cckscale_fctr_db =
		(int8) PHY_GETINTVAR_DEFAULT(pi, rstr_cckscale, -1);

	for (i = 0; i < 14; i++)
		pi_lcn20->txpwroffset2g[i] = (int8)PHY_GETINTVAR_ARRAY(pi, rstr_txpwroffset2g, i);

	/* Coefficients for Temperature Conversion to Centigrade */
	pi_lcn20->temp_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_mult, 161); /* 0.6282<<8 */
	pi_lcn20->temp_add = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_add, 26127); /* 102.06<<8 */
	pi_lcn20->temp_q = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_temp_q, 8);

	/* Coefficients for vbat conversion to Volts */
	pi_lcn20->vbat_mult = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_mult, -9); /* -.009<<10 */
	pi_lcn20->vbat_add = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_add, 4027); /* 3.932<<10 */
	pi_lcn20->vbat_q = (int32)PHY_GETINTVAR_DEFAULT(pi, rstr_vbat_q, 10);

	pi_lcn20->edonthreshold20L = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edonthd20l, -66);
	pi_lcn20->edoffthreshold = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_edoffthd20ul, -72);

#ifdef LCN20_PAPD_ENABLE
	papd_mode = (uint16)PHY_GETINTVAR_DEFAULT(pi, rstr_papdmode, 0x1);
	pi_lcn20->papd_enable = papd_mode > 0;
	pi_lcn20->papd_mcs_comp = papd_mode & 0x2;
	pi_lcn20->papd_end_idx = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_papdendidx, 63);
	pi_lcn20->base_pwr_dbm = PHY_GETINTVAR_DEFAULT(pi, rstr_papdbasepwr, 100);
	pi_lcn20->target_pwr_dbm = PHY_GETINTVAR_DEFAULT(pi, rstr_papdtrgtpwr, -1);
	if (pi_lcn20->target_pwr_dbm != -1) {
		pi_lcn20->do_papd_calidx_est = TRUE;
	} else {
		pi_lcn20->do_papd_calidx_est = FALSE;
	}
	pi_lcn20->papdidx_offset_chan12 =
	  (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_papdidxoffsetchan12, 0);
	pi_lcn20->papdidx_offset_chan13 =
	  (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_papdidxoffsetchan13, 0);
	pi_lcn20->papdcalidxoffset = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_papdcalidxoffset, 40);

#endif /* LCN20_PAPD_ENABLE */

	spurconfig = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_spurconfig,
		((0 << 4) | LCN20PHY_SPURCONFIG_DEFAULT));
	pi_lcn20->spurmode = spurconfig & 0xf;
	pi_lcn20->dssfmode = (spurconfig >> 4) & 0x1;

	pi_lcn20->dssf_thresh[0] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_dssfth,
		0, LCN20PHY_DSSFSPUR_LOW_BOUND);
	pi_lcn20->dssf_thresh[1] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_dssfth,
		1, LCN20PHY_DSSFSPUR_THRESH_1);
	pi_lcn20->dssf_thresh[2] = (int16)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_dssfth,
		2, LCN20PHY_DSSFSPUR_THRESH_2);

	pi_lcn20->xtalpn_nv = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_xtalpn, 0);
	if (pi_lcn20->xtalpn_nv > 16)
		pi_lcn20->xtalpn_nv = 16;
	for (i = 0; i < 14; i++)
		pi_lcn20->xtal_pn[i] = 0;

	pi_lcn20->rssicorr_aci = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_rcor_aci,
		LCN20PHY_NORTBL_ACITBL_RSSIOFFSET_REPORT);
#if defined(WLPROPRIETARY_11N_RATES)
	pi_lcn20->qam256en = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_qam256en, 0);
#endif // endif
	wlc_phy_txpwr_sromlcn20_read_ppr_parameters(pi);

	if (!pi->low_power_mode) {
		pi->txpa_2g[2] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 0);
		pi->txpa_2g[0] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 1);
		pi->txpa_2g[1] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0, 2);
	} else {
		pi->txpa_2g[2] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_lo, 0);
		pi->txpa_2g[0] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_lo, 1);
		pi->txpa_2g[1] =
			(int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_lo, 2);
	}

	/* By default Two power range feature is not enabled */
	pi_lcn20->lcn20_twopwr_txpwrctrl_en = 0;

#if TWO_POWER_RANGE_TXPWR_CTRL
	if (PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_pa2ga0_2pwr, 2, 0))
		pi_lcn20->lcn20_twopwr_txpwrctrl_en = 1;

	if (pi_lcn20->lcn20_twopwr_txpwrctrl_en) {
		pi->txpa_2g_2pwr[2] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_2pwr, 0);
		pi->txpa_2g_2pwr[0] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_2pwr, 1);
		pi->txpa_2g_2pwr[1] = (int16)PHY_GETINTVAR_ARRAY(pi, rstr_pa2ga0_2pwr, 2);
		/* Low power range limits by default are 0 to 10dBm */
		pi_lcn20->pmin = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pmin, 0);
		pi_lcn20->pmax = (int8)PHY_GETINTVAR_DEFAULT(pi, rstr_pmax, 40);
		pi->min_txpower = 0;
	}
#endif /* TWO_POWER_RANGE_TXPWR_CTRL */

	for (i = 0; i < LCN20PHY_SW_CTRL_NVRAM_PARAMS; i++) {
		pi_lcn20->swctrlmap_2g[i] =
			(uint32) PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_swctrlmap_2g, i, 0);
	}

	pi_lcn20->swctrl_gpios = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_swctrl_gpios, 0x0);

#ifndef ATE_BUILD
	if ((pi_lcn20->trsw_ctrl_etr == TRUE) &&
		((PHY_GETVAR(pi, rstr_swctrlmap_2g) == NULL) ||
		(pi_lcn20->swctrl_gpios == 0x0)) &&
		!NORADIO_ENAB(pi->pubpi)) {
		PHY_ERROR(("Missing swctrl parameters in NVRAM file %s \n", __FUNCTION__));
		return FALSE;
	}
#endif /* ATE_BUILD */

	/* eLNA-bypass */
	pi_lcn20->tr_isolation = (uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_triso2g, 0xff);
	pi_lcn20->elna_off_gain_idx_2g =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_elna_off_gain_idx_2g, 0xff);

	for (i = 0; i < LCN20PHY_GAIN_DELTA_2G_PARAMS; i++) {
		p_rssi_delta_2gb0[i] = (int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
				rstr_rssi_delta_2gb0, i, 0);

		if (PHY_GETVAR(pi, rstr_rssi_delta_2gb1))
			p_rssi_delta_2gb1[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_delta_2gb1, i);
		else
			p_rssi_delta_2gb1[i] = p_rssi_delta_2gb0[i];

		if (PHY_GETVAR(pi, rstr_rssi_delta_2gb2))
			p_rssi_delta_2gb2[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_delta_2gb2, i);
		else
			p_rssi_delta_2gb2[i] = p_rssi_delta_2gb0[i];

		if (PHY_GETVAR(pi, rstr_rssi_delta_2gb3))
			p_rssi_delta_2gb3[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_delta_2gb3, i);
		else
			p_rssi_delta_2gb3[i] = p_rssi_delta_2gb0[i];

		if (PHY_GETVAR(pi, rstr_rssi_delta_2gb4))
			p_rssi_delta_2gb4[i] =
				(int8) PHY_GETINTVAR_ARRAY(pi, rstr_rssi_delta_2gb4, i);
		else
			p_rssi_delta_2gb4[i] = p_rssi_delta_2gb0[i];
	}

	pi_lcn20->rxgain_tempadj_2g = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_rxgaintempcoeff2g, 72);
	pi->srom_gain_cal_temp = (int16)PHY_GETINTVAR_DEFAULT(pi, rstr_gain_cal_temp,
			LCN20_RSSI_INVALID_TEMP);

	for (i = 0; i < 7; i++) {
		uint8 grp = (uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_cal_freq_grp_2g, i, 0);

		pi_lcn20->rssi_cal_freq_grp[2*i] = (grp >> 4) & 0xF;
		pi_lcn20->rssi_cal_freq_grp[(2*i) + 1] = (grp & 0xF);
	}

	for (i = 0; i < 14; i++) {
		if ((pi_lcn20->rssi_cal_freq_grp[i] & 0x8) != 0) {
			cal_grp_no = pi_lcn20->rssi_cal_freq_grp[i] & 0x7;
			if (cal_grp_no < LCN20PHY_GROUPS)
				pi_lcn20->rssi_cal_channel[cal_grp_no] = i+1;
		}
	}

	for (i = 0; i < 14; i++) {
		pi_lcn20->rssi_channel_offset_rout0[i] =
			(int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_channel_offset_rout0, i, 0);
	}

	for (i = 0; i < 14; i++) {
		pi_lcn20->rssi_channel_offset_rout8[i] =
			(int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_channel_offset_rout8, i, 0);
	}

	for (i = 0; i < 14; i++) {
		pi_lcn20->rssi_channel_offset_elnabyp[i] =
			(int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi,
				rstr_rssi_channel_offset_elnabyp, i, 0);
	}

	pi_lcn20->noise_log_nsamps =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_log_nsamps, 6);
	pi_lcn20->noise_iqest_en =
		(uint8)PHY_GETINTVAR_DEFAULT(pi, rstr_noise_iqest_en, 1);
	pi_lcn20->noise_iqest_gain_adj_2g =
		(int8)PHY_GETINTVAR_DEFAULT(pi, rstr_noiseiqgainadj2g, -42);

	for (i = 0; i < LCN20PHY_NUM_RATE_OFFSETS; i++) {
		pi_lcn20->rssi_rate_offset[i] =
			(int8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_rssi_rate_offset, i, 0);
	}
	return TRUE;
}
/* END: SROM Above functions are part of srom read feature */

#define WLC_20692_GI_MULT_P12		4096U
#define WLC_20692_GI_MULT_TWEAK_P12	4096U
#define WLC_20692_GI_MULT		WLC_20692_GI_MULT_P12

static int
wlc_20692_sigdel_wrap(int prod, int max_val)
{
	/* to make sure you hit the maximum number of bits in word allocated  */
	return (prod > max_val) ? max_val : prod;
}

static void
wlc_20692_sigdel_slow0g6_tune(phy_info_t *pi, int g_mult_raw_p12, adc_tuning_array_20962_t *gvalues)
{
	int g_mult_p12;
	int ri;
	int r21;
	int r32;
	int r43;
	int rff1;
	int rff2;
	int rff3;
	int rff4;
	int r12v;
	int r34v;
	int r11v;
	int g21;
	int g32;
	int g43;
	int r12;
	int r34;
	int g11;
	int temp;
	BCM_REFERENCE(pi);

	/* RC cals the slow ADC IN 20MHz channels or 10MHz bandwidth, based on g_mult */
	/* input signal scaling, changes ADC gain, 4096 <=> 1.0 for g_mult and gi_mult */
	/* Function is 32 bit (signed) integer arithmetic and a/b division rounding  */
	/* is performed in integers from: (a-1)/b+1 */

	/* ERR! 20691_rc_cal "adc" returns the RC value, so correction is 1/rccal! */
	/* so invert it */
	/* This is a nice way of inverting the number... jnh */
	/* inverse of gmult precomputed to minimise division operations for speed */
	/* 4.12 fixed point so scale reciprocal by 2^24 */
	g_mult_p12 = g_mult_raw_p12 > 0 ? g_mult_raw_p12 : 1;
	g_mult_p12 = 16777216 / g_mult_p12;
	g_mult_p12 = (WLC_20692_GI_MULT_TWEAK_P12 * g_mult_p12) >> 12;

	/* to avoid divide by zeros and negative values */
	if (g_mult_p12 <= 0)
		g_mult_p12 = 1;

	/* RC cal in slow ADC is mostly of the form Runit/(Rval/(g_mult/2**12)-Roff). */
	/* For integer manipulation do Runit/({Rval*2**12}/gmult-Roff).  */
	/* where Rval*2**12 are res design values in matlab script {x kint234 , kr12, kr34} */
	/* but right shifted a number of times */
	/* All but r11 and rff4 resistances are x2 for 20MHz. */

	/* Rvals from matlab already scaled by kint234, kr12 */
	/* or kr34 (due to amplifier finite GBW) */
	/* x2 for half the BW and half the sampling frequency. */
	ri = 10176 << 1;
	r21 = 8323 << 1;
	r32 = 6390 << 1;
	r43 = 6827 << 1;
	rff1 = 19768 << 1;
	rff2 = 16916 << 1;
	rff3 = 29113  << 1;
	/* rff4 does not double with 20MHz channels, 10MHz BW. */
	rff4 = 100000;
	r12v = 83205 << 1;
	r34v = 243530 << 1;
	/* rff4 does not double with 20MHz channels, 10MHz BW. */
	r11v = 8000;

	/* saturate correctly when you get negative numbers and round divisions */
	/* subject to gmult twice so scale gmult back to 12b so it only divides with 12b+ r21 */
	g21 = (g_mult_p12 * g_mult_p12) >> 12;
	if (g21 <= 0)
		g21 = 1;
	g21 = ((r21 << 12) - 1) / g21 + 1;
	g21 = (512000 - 1) / g21 + 1;
	g21 = wlc_20692_sigdel_wrap(g21, 127);
	g32 = (256000 - 1) / (((r32 << 12) - 1) / g_mult_p12 + 1) + 1;
	g32 = wlc_20692_sigdel_wrap(g32, 127);
	g43 = (256000 - 1) / (((r43 << 12) - 1) / g_mult_p12 + 1) + 1;
	g43  = wlc_20692_sigdel_wrap(g43, 127);

	/* gff1234 subject to gmult and gimult; step operations so range */
	/* is not exceeded and scale is correct */
	/* gff1234 overflow if $g_mult_p12*$gi_mult < 1023*2, eq. to 0.25, */
	/* assuming rff1234 {<<2} < 131072 */

	/* gi */
	temp = (((ri << 12) - 1) / WLC_20692_GI_MULT) - 4000 + 1;
	ASSERT(temp > 0);	/* should be a positive value */
	temp = (256000 - 1) / temp + 1;
	temp = wlc_20692_sigdel_wrap(temp, 127);
	gvalues->gi = (uint16) temp;

	/* gff1 */
	temp = ((((((rff1 << 12) - 1) / g_mult_p12 + 1) << 12) - 1) / WLC_20692_GI_MULT) - 8000 + 1;
	if (temp <= 0)
		temp = 1;
	temp = (256000 - 1) / temp + 1;
	temp = wlc_20692_sigdel_wrap(temp, 127);
	gvalues->gff1 = (uint16) temp;

	/* gff2 */
	temp = ((((((rff2 << 12) - 1) / g_mult_p12 + 1) << 12) - 1) / WLC_20692_GI_MULT) - 4000 + 1;
	if (temp <= 0)
		temp = 1;
	temp = (256000 - 1) / temp + 1;
	temp = wlc_20692_sigdel_wrap(temp, 127);
	gvalues->gff2 = (uint16) temp;

	/* gff3 */
	temp = ((((((rff3 << 12) - 1) / g_mult_p12 + 1) << 12) - 1) / WLC_20692_GI_MULT) - 32000
		+ 1;
	if (temp <= 0)
		temp = 1;
	temp = (256000 - 1) / temp + 1;
	temp = wlc_20692_sigdel_wrap(temp, 127);
	gvalues->gff3 = (uint16) temp;

	/* gff4 */
	temp = (((rff4 << 12) - 1) / WLC_20692_GI_MULT) - 72000 + 1;	/* subject to gimult only */
	ASSERT(temp > 0);	/* should be a positive value */
	temp = (256000 - 1) / temp + 1;
	temp  = wlc_20692_sigdel_wrap(temp, 255);
	gvalues->gff4 = (uint16) temp;

	/* stays constant to RC shifts, g21 shifts twice for it. */
	r12 = (r12v - 1) / 4000 + 1;
	r12 = wlc_20692_sigdel_wrap(r12, 127);
	r34 = ((((r34v << 12) - 1) / g_mult_p12 +1) - 128000 - 1) / 4000 + 1;
	if (r34 <= 0)
		r34 = 1;
	r34 = wlc_20692_sigdel_wrap(r34, 127);
	g11 = (((r11v << 12) - 1) / g_mult_p12 +1) - 2000;
	if (g11 <= 0)
		g11 = 1;
	g11 = (128000 - 1) / g11 + 1;
	g11 = wlc_20692_sigdel_wrap(g11, 127);

	gvalues->g21 = (uint16) g21;
	gvalues->g32 = (uint16) g32;
	gvalues->g43 = (uint16) g43;
	gvalues->r12 = (uint16) r12;
	gvalues->r34 = (uint16) r34;
	gvalues->g11 = (uint16) g11;
	PHY_TRACE(("gi   = %i\n", gvalues->gi));
	PHY_TRACE(("g21  = %i\n", gvalues->g21));
	PHY_TRACE(("g32  = %i\n", gvalues->g32));
	PHY_TRACE(("g43  = %i\n", gvalues->g43));
	PHY_TRACE(("r12  = %i\n", gvalues->r12));
	PHY_TRACE(("r34  = %i\n", gvalues->r34));
	PHY_TRACE(("gff1 = %i\n", gvalues->gff1));
	PHY_TRACE(("gff2 = %i\n", gvalues->gff2));
	PHY_TRACE(("gff3 = %i\n", gvalues->gff3));
	PHY_TRACE(("gff4 = %i\n", gvalues->gff4));
	PHY_TRACE(("g11  = %i\n", gvalues->g11));
}

static void
wlc_20692_adc_setup_slow0g6(phy_info_t *pi, adc_tuning_array_20962_t *gvalues)
{
	/* SETS UP THE slow ADC IN 20MHz channels or 10MHz bandwidth */
	/* Function should be 32 bit (signed) arithmetic */

	/* EXPLICITELY ENABLE/DISABLE ADCs and INTERNAL CLKs? */
	/* Only changes between fast/slow ADC, not 20/40MHz */
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_slow_pu, 0x0);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_clk_slow_pu, 0x0);

	/* Setup internal dividers and sipo for 1G2Hz mode */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_sipo_drive_strength, 0x4);
	/* set adc_clk_slow_div3 to 0x0 in 20MHz mode, 0x1 in 40MHz mode */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_clk_slow_div3, 0x0);

	/* Setup biases */
	/* Slow adc halves opamp current from 20MHz to 40MHz channels */
	/* Opamp1 is 26u/0, 4u/2 other 3 are 26u/0, 4u/4,    */
	/* so opamp1 (40M, 20M, = (0x20, 0x10,, opamp234 = (0x10, 0x8, */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG6, 0, wl_adc_biasadj_opamp1, 0x10);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG6, 0, wl_adc_biasadj_opamp2, 0x8);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG7, 0, wl_adc_biasadj_opamp3, 0x8);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG7, 0, wl_adc_biasadj_opamp4, 0x8);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_ff_mult_opamp, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_cmref_control, 0x40);

	/* Setup transconductances, These are tuned with gmult(RC, and/or gimult(input gain, */
	/* rnm */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG2, 0, wl_adc_gi, gvalues->gi);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG3, 0, wl_adc_g21, gvalues->g21);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG3, 0, wl_adc_g32, gvalues->g32);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG4, 0, wl_adc_g43, gvalues->g43);
	/* rff */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG11, 0, wl_adc_gff1, gvalues->gff1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG11, 0, wl_adc_gff2, gvalues->gff2);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG12, 0, wl_adc_gff3, gvalues->gff3);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG12, 0, wl_adc_gff4, gvalues->gff4);
	/* resonator and r11 */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG5, 0, wl_adc_r12, gvalues->r12);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG5, 0, wl_adc_r34, gvalues->r34);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG4, 0, wl_adc_g54, gvalues->g11);

	/* Setup feedback DAC and tweak delay compensation */
	/* In slow 40MHz ADC rt is 0x0, in 20MHz ADC rt is 0x2 */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG13, 0, wl_adc_rt, 0x2);
	/* In slow 40MHz ADC slow_dacs is 0x2 in 20MHz ADC rt is 0x1 */
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG13, 0, wl_adc_slow_dacs, 0x1);

	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_reset_adc, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_adcs_reset, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_adcs_reset, 0x0);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_reset_adc, 0x0);
}

static void
wlc_lcn20phy_radio20692_afe_cal(phy_info_t *pi)
{
	adc_tuning_array_20962_t gvalues;
	/* NOTE: rc cal and tia config are not implemented as they are done
	* outside in wlc_lcn20phy_radio20692_init
	*/
	/* 20MHz channel */
	wlc_20692_sigdel_slow0g6_tune(pi, pi->u.pi_lcn20phy->rccal_adc_gmult, &gvalues);
	wlc_20692_adc_setup_slow0g6(pi, &gvalues);
}

static void
WLBANDINITFN(wlc_lcn20phy_radio20692_init)(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* powerup doubler */
	if ((pi->xtalfreq == XTAL_FREQ_37P4MHZ) && (pi->u.pi_lcn20phy->xtalpn_nv == 0))
		MOD_RADIO_REG_20692(pi, WL_XTAL_CFG3, 0, wl_xtal_doubler_pu, 1);
	else
		MOD_RADIO_REG_20692(pi, WL_XTAL_CFG3, 0, wl_xtal_doubler_pu, 0);

	/* PU the TX/RX/VCO/..LDO's	*/

	/* mini PMU init */
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_wlpmu_en, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_wlpmu_en, 1);
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_TXldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_TXldo_pu, 1);
	OSL_DELAY(100);
	MOD_RADIO_REG_20692(pi, WL_VREG_HIGHV_VBAT_TOP_OVR1, 0, wl_ovr_vreg25_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_VREG_CFG, 0, wl_vreg_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_BG_TOP_V3_OVR1, 0, wl_ovr_bg_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_BG_CFG1, 0, wl_bg_pu, 1);
	OSL_DELAY(20);
	/* Turn on all wlpmu outputs */
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_VCOldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_AFEldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_RXldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_VCOldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_AFEldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_RXldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_ADCldo_pu, 1);
	OSL_DELAY(200);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG2, 0, wl_synth_pwrsw_en, 1);

	/* disable vco cal */
	MOD_RADIO_REG_20692(pi, WL_PLL_VCOCAL1, 0, wl_rfpll_vcocal_enableCal, 1);

	/* powerup pll */
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_vco_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vco_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_vco_buf_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vco_buf_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_synth_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_synth_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_monitor_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_monitor_pu, 1);

	/* Rcal */
	MOD_RADIO_REG_20692(pi, WL_BG_TOP_V3_OVR1, 0, wl_ovr_bg_rcal_trim, 1);

	MOD_RADIO_REG_20692(pi, WL_BG_CFG1, 0, wl_bg_rcal_trim, pi->u.pi_lcn20phy->rcal);
	MOD_RADIO_REG_20692(pi, WL_BG_TOP_V3_OVR1, 0, wl_ovr_otp_rcal_sel, 0);
	/* Disable Rcal Clock */
	WRITE_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0,
		(READ_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0) & (~0x200)));

	/* RC-CAL */
	wlc_lcn20phy_radio20692_rc_cal(pi);

	/* AFE calibration */
	/* Need to run 20692_rc_calbeforehand to set gmult */
	wlc_lcn20phy_radio20692_afe_cal(pi);

	/* Dac settings */
	MOD_RADIO_REG_20692(pi, WL_CLK_DIV_CFG1, 0, wl_afeclkdiv_dac_driver_size, 4);
	MOD_RADIO_REG_20692(pi, WL_CLK_DIV_CFG1, 0, wl_afeclkdiv_sel_dac_div, 4);
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_DAC_invclk, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_DAC_pd_mode, 0);
	MOD_RADIO_REG_20692(pi, WL_BG_CFG1, 0, wl_bg_pulse, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG1, 0, wl_DAC_scram_off, 1);

	/* populate TIA LUT */
	wlc_lcn20phy_radio20692_tia_config(pi);

	/* User Preferred Values */
	/* For vreg logic issue */
	MOD_RADIO_REG_20692(pi, WL_VREG_CFG, 0, wl_vreg_vout_sel, 3);
	/* logen tuning */
	MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG2, 0, wl_logencore_x2_ctune, 0);
	MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG2, 0, wl_logencore_lobuf2g_ctune, 0);
	/* logen bias */
	MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG1, 0, wl_logencore_bias_x2, 2);
	MOD_RADIO_REG_20692(pi, WL_LOGEN_CFG1, 0, wl_logencore_bias_lobuf2g, 7);
	MOD_RADIO_REG_20692(pi, WL_XTAL_CFG4, 0, wl_xtal_wlanbb_ctrl, 1);

	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_tssi_ctrl_sel, 0);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_gpio_sw_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_bias_reset, 0);

	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG3, 0, wl_pa2g_sel_bias_type, 0x30);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG3, 0, wl_pa2g_ptat_slope_cas, 0xf);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG3, 0, wl_pa2g_ptat_slope_main, 0);
	/* EVM optimization w PAPD off */

	MOD_RADIO_REG_20692(pi, WL_PA2G_IDAC1, 0, wl_pa2g_idac_main, 0x19);
	MOD_RADIO_REG_20692(pi, WL_PA2G_IDAC1, 0, wl_pa2g_idac_cas, 0x19);

	MOD_RADIO_REG_20692(pi, WL_PA2G_INCAP, 0, wl_pa2g_ptat_slope_incap_compen_main, 0);
	MOD_RADIO_REG_20692(pi, WL_PA2G_INCAP, 0, wl_pa2g_idac_incap_compen_main, 0x21);

	MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG2, 0, wl_mx2g_idac_cascode, 0x15);
	MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG6, 0, wl_mx2g_ptat_slope_bbdc, 0x0);
	MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG6, 0, wl_mx2g_idac_lodc, 0x28);
	MOD_RADIO_REG_20692(pi, WL_TXMIX2G_CFG6, 0, wl_mx2g_idac_bbdc, 0x16);

	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG4, 0, wl_pa2g_bias_bw_cas, 0x4);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG4, 0, wl_pa2g_bias_bw_main, 0x4);
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG4, 0, wl_pa2g_bias_bw_pmos, 0x1);

	MOD_RADIO_REG_20692(pi, WL_RCCAL_LPO_CFG1, 0, wl_lpo_pu, 0);
}

static void
wlc_lcn20phy_radio20692_tia_config(phy_info_t *pi)
{
	const uint8  *p8;
	const uint16 *p16;
	uint16 lut;

	STATIC_ASSERT(ARRAYSIZE(tiaRC_20692_8b_20) +
		ARRAYSIZE(tiaRC_20692_16b_20) == TIA_LUT_20692_LEN);

	p8 = tiaRC_20692_8b_20;
	p16 = tiaRC_20692_16b_20;

	lut = RADIO_REG_20692(pi, WL_TIA_LUT_0, 0);

	/* the assumption is that all the TIA LUT registers are in sequence */
	ASSERT(RADIO_REG_20692(pi, WL_TIA_LUT_108, core) - lut == (TIA_LUT_20692_LEN - 1));

	do {
		phy_utils_write_radioreg(pi, lut++, *p8++);
	} while (lut <= RADIO_REG_20692(pi, WL_TIA_LUT_51, 0));

	do {
		phy_utils_write_radioreg(pi, lut++, *p16++);
	} while (lut <= RADIO_REG_20692(pi, WL_TIA_LUT_108, 0));

}

static void
wlc_lcn20phy_radio20692_channel_tune(phy_info_t *pi, uint8 channel)
{
	chan_info_20692_lcn20phy_t *chi;
	uint32 pll_mmd;
	const uint8 mmd_offset[14] = {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
	const uint8 cp_kpd_scale_doubler0[14] = {0x40, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x41, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42};

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Write the tuning tables */
	chi = &(pi->u.pi_lcn20phy->tuningtbl[channel -1]);

	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL18, 0, chi->pll_vcocal18);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL3, 0, chi->pll_vcocal3);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL4, 0, chi->pll_vcocal4);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL7, 0, chi->pll_vcocal7);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL8, 0, chi->pll_vcocal8);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL20, 0, chi->pll_vcocal20);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL1, 0, chi->pll_vcocal1);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL12, 0, chi->pll_vcocal12);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL13, 0, chi->pll_vcocal13);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL10, 0, chi->pll_vcocal10);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL11, 0, chi->pll_vcocal11);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL19, 0, chi->pll_vcocal19);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL6, 0, chi->pll_vcocal6);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL9, 0, chi->pll_vcocal9);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL17, 0, chi->pll_vcocal17);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL5, 0, chi->pll_vcocal5);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL15, 0, chi->pll_vcocal15);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCOCAL2, 0, chi->pll_vcocal2);

	/* The pll_mmd values in tuning tables chan_info_20692_rev*_lcn20phy_37p4MHz
	 * are generated assuming the xtal doubler is enabled. If xtalpn_nv>0,
	 * the xtal doubler is disabled, and pll_mmd, cp_kpd_scale are doubled.
	 */
	if ((pi->xtalfreq == XTAL_FREQ_37P4MHZ) && (pi->u.pi_lcn20phy->xtalpn_nv > 0)) {
		pll_mmd = (chi->pll_mmd1 << 17) | (chi->pll_mmd2 << 1) | mmd_offset[channel-1];
		WRITE_RADIO_REG_20692(pi, WL_PLL_MMD1, 0, ((pll_mmd >> 16) & 0xffff));
		WRITE_RADIO_REG_20692(pi, WL_PLL_MMD2, 0, (pll_mmd & 0xffff));
		WRITE_RADIO_REG_20692(pi, WL_PLL_CP1, 0,
			(chi->pll_cp1 & 0xff80) | cp_kpd_scale_doubler0[channel-1]);
	} else {
		WRITE_RADIO_REG_20692(pi, WL_PLL_MMD1, 0, chi->pll_mmd1);
		WRITE_RADIO_REG_20692(pi, WL_PLL_MMD2, 0, chi->pll_mmd2);
		WRITE_RADIO_REG_20692(pi, WL_PLL_CP1, 0, chi->pll_cp1);
	}

	WRITE_RADIO_REG_20692(pi, WL_PLL_VCO2, 0, chi->pll_vco2);
	WRITE_RADIO_REG_20692(pi, WL_PLL_VCO1, 0, chi->pll_vco1);
	WRITE_RADIO_REG_20692(pi, WL_PLL_LF4, 0, chi->pll_lf4);
	WRITE_RADIO_REG_20692(pi, WL_PLL_LF5, 0, chi->pll_lf5);
	WRITE_RADIO_REG_20692(pi, WL_PLL_LF2, 0, chi->pll_lf2);
	WRITE_RADIO_REG_20692(pi, WL_PLL_LF3, 0, chi->pll_lf3);
	WRITE_RADIO_REG_20692(pi, WL_PLL_CP2, 0, chi->pll_cp2);
	WRITE_RADIO_REG_20692(pi, WL_LOGEN_CFG2, 0, chi->logen_cfg2);
	WRITE_RADIO_REG_20692(pi, WL_LNA2G_TUNE, 0, chi->lna2g_tune);
	WRITE_RADIO_REG_20692(pi, WL_TXMIX2G_CFG5, 0, chi->txmix2g_cfg5);

	/* apply xtal duty cycle tweak */
	MOD_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0, wl_xtal_dty_adj,
		(pi->u.pi_lcn20phy->xtal_pn[channel-1] & 0xf));

	/* Band Set */
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_auxgm_pwrup, 0);
	MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG2, 0, wl_lpf_sel_5g_out_gm, 0);
	MOD_RADIO_REG_20692(pi, WL_TX_LPF_CFG3, 0, wl_lpf_sel_2g_5g_cmref_gm, 0);

	/* VCO Cal */
	wlc_lcn20phy_radio20692_vcocal(pi);
	wlc_lcn20phy_radio20692_vcocal_isdone(pi, TRUE);
}

static void
wlc_lcn20phy_radio20692_vcocal(phy_info_t *pi)
{
	/* enable vco cal clock */
	WRITE_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0,
		(READ_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0) | 0x100));

	MOD_RADIO_REG_20692(pi, WL_PMU_CFG2, 0, wl_VCOldo_adj, 0);
	MOD_RADIO_REG_20692(pi, WL_PLL_VCOCAL1, 0, wl_rfpll_vcocal_enableCal, 0);
	OSL_DELAY(1);
	MOD_RADIO_REG_20692(pi, WL_PLL_VCOCAL1, 0, wl_rfpll_vcocal_enableCal, 1);
	/* VCO-Cal startup seq */
	/* TODO: The below registers have direct PHY control in 20692
		 so this reset should ideally be done by writing phy registers
	*/
	/* Disable PHY direct control for delta-sigma modulator reset signal */
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_rst_n, 1);
	/* Reset delta-sigma modulator */
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_rst_n, 0);
	/* Disable PHY direct control for vcocal reset */
	MOD_RADIO_REG_20692(pi, WL_SYNTH_OVR1, 0, wl_ovr_rfpll_vcocal_rst_n, 1);
	/* Reset VCO cal */
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vcocal_rst_n, 0);
	OSL_DELAY(1);
	/* Release Reset */
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_rst_n, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vcocal_rst_n, 1);
	OSL_DELAY(1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_cp_bias_reset, 1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vco_bias_reset, 1);
	OSL_DELAY(1);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_cp_bias_reset, 0);
	MOD_RADIO_REG_20692(pi, WL_PLL_CFG1, 0, wl_rfpll_vco_bias_reset, 0);
	OSL_DELAY(50);
}

#define MAX_2069x_VCOCAL_WAITLOOPS 100
/* vcocal should take < 120 us */
static void
wlc_lcn20phy_radio20692_vcocal_isdone(phy_info_t *pi, bool set_delay)
{
	uint8 done, itr;

	/* Wait for vco_cal to be done, max = 100us * 10 = 1ms  */
	done = 0;
	for (itr = 0; itr < MAX_2069x_VCOCAL_WAITLOOPS; itr++) {
		OSL_DELAY(10);
		done = READ_RADIO_REGFLD_20692(pi, WL_PLL_STATUS1, 0, rfpll_vcocal_done_cal);
		if (done == 1)
			break;
	}

	/* Need to wait extra time after vcocal done bit is high for it to settle */
	if (set_delay == TRUE)
		OSL_DELAY(100);

	ASSERT(done & 0x1);

	/* disable vco cal clock */
	WRITE_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0,
		(READ_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0) & (~0x100)));

	PHY_INFORM(("wl%d: %s vcocal done\n", pi->sh->unit, __FUNCTION__));
}

static void
wlc_lcn20phy_radio20692_tssisetup(phy_info_t *pi)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Powerup gpaio block, powerdown rcal, clear all test point selection */
	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL2, 0, wl_cgpaio_pu, 1);

	/* Powerdown rcal otherwise it wont let any other test point go through */
	MOD_RADIO_REG_20692(pi, WL_RCAL_CFG1, 0, wl_rcal_pu, 0);

	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x0);
	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL1, 0, wl_cgpaio_sel_16to31_port, 0x0);

	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL0, 0, wl_cgpaio_sel_0to15_port, 0x0);
	MOD_RADIO_REG_20692(pi, WL_GPAIO_SEL1, 0, wl_cgpaio_sel_16to31_port, 0x0);

	/* INT TSSI setup */
	/* # B3: (1 = iqcal, 0 = tssi);
	 * # B2: (1 = ext-tssi, 0 = int-tssi)
	 * # B1: (1 = 5g, 0 = 2g)
	 * # B0: (1 = wo filter, 0 = w filter for ext-tssi)
	 */
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_sel_sw, 0x0);

	/* Select PA output (and not PA input) */
	MOD_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, wl_pa2g_tssi_ctrl_sel, 0);

	/* int-tssi select */
	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_sel_ext_tssi, 0);

}

static void
wlc_lcn20phy_tia_gain(phy_info_t *pi, uint16 gain)
{
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R1, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R2, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R3, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R4, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R5, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_R6, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_C1, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_C2, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_amp2, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_amp2_bypass, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_enable_st1, 0);

	PHY_REG_MOD(pi, LCN20PHY, rfoverride5, rxrf_tia_gain_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, rfoverride5val, rxrf_tia_gain_ovr_val, gain);
}

static void
wlc_lcn20phy_radio20692_rx_iq_cal_setup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_radioregs_t *psave = pi_lcn20->rxiqcal_radioregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	psave->wl_rx_adc_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0);
	psave->wl_tx_top_2g_ovr1 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0);
	psave->wl_tx_top_2g_ovr2 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0);
	psave->wl_rx_top_2g_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0);
	psave->wl_rx_bb_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0);
	psave->wl_minipmu_ovr1 = READ_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0);

	/* force off the dac 2 adc switches */
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_in_test, 1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG10, 0, wl_adc_in_test, 0);

	/* Don't enable bbpd input to TIA */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, wl_tia_tx_lpbck_i, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, wl_tia_tx_lpbck_q, 0);

	/* PAPD LOOPBACK PATH */

	/* TR sw in TX mode */
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_bias_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_bias_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_tr_rx_en, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_tr_rx_en, 0);

	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu_bias, 1);

	/* Enable ipapd */
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_loopback2g_papdcal_pu, 1);

	/* Set txattn and rxattn */
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_atten, 3);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rf2g_papdcal_rx_attn, 3);

	/* Powerdown lna1 */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_lna1_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_lna1_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_lna1_out_short_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_lna1_out_short_pu, 1);

	/* powerdown rx main gm2g  */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_gm2g_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_pwrup, 0x0);

	/* powerup auxgm2g */
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_auxgm_pwrup, 0x1);

	/* powerup rx mixer */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxmix2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxmix2g_pu, 1);

	/* powerup rxdiv2g */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxdiv2g_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rxdiv2g_pu_bias, 1);

	/* powerup tia */
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_resstring, 1);
	/* Power up common mode reference */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_resstring, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_amp1_pwrup, 1);
	/* PU 1st amp */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_amp1_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_amp2, 1);
	/* PU 2nd amp */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_amp2, 1);

	wlc_lcn20phy_tia_gain(pi, 3);

	/* powerup adc */
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_clk_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_clk_slow_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_slow_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_sipo_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_sipo_pu, 1);
}

static void
wlc_lcn20phy_radio20692_rx_iq_cal_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_radioregs_t *psave = pi_lcn20->rxiqcal_radioregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	WRITE_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, psave->wl_rx_adc_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, psave->wl_tx_top_2g_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, psave->wl_tx_top_2g_ovr2);
	WRITE_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, psave->wl_rx_top_2g_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, psave->wl_rx_bb_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, psave->wl_minipmu_ovr1);

	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_loopback2g_papdcal_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_auxgm_pwrup, 0);
}

int wlc_lcn20phy_idle_tssi_reg_iovar(phy_info_t *pi, int32 int_val, bool set, int *err)
{
	uint16 perPktIdleTssi;
	uint16 idleTssi_2C;

	perPktIdleTssi = PHY_REG_READ(pi, LCN20PHY, perPktIdleTssiCtrl, perPktIdleTssiUpdate_en);
	if (set) {
		if (perPktIdleTssi)
			*err = BCME_UNSUPPORTED; /* avgidletssi is not writeable */
		else
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0, (uint16)int_val);
	}

	if (perPktIdleTssi)
		idleTssi_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew6, avgidletssi);
	else
		idleTssi_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0);
	return idleTssi_2C;
}

int wlc_lcn20phy_avg_tssi_reg_iovar(phy_info_t *pi)
{
	int16 avgTssi_2C;
	avgTssi_2C = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi);
	return avgTssi_2C;
}

int32
wlc_lcn20phy_iovar_txpwrctrl(phy_info_t *pi, int32 int_val, int32 *ret_int_ptr)
{
	uint16 pwrctrl;
	BCM_REFERENCE(ret_int_ptr);

	if (!pi->sh->clk)
		return BCME_NOCLK;

	pwrctrl = (int_val > 1) ? LCN20PHY_TX_PWR_CTRL_SW :
		(int_val ? LCN20PHY_TX_PWR_CTRL_HW : LCN20PHY_TX_PWR_CTRL_OFF);

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	if (!int_val)
		wlc_lcn20phy_set_tx_pwr_by_index(pi, LCN20PHY_MAX_TX_POWER_INDEX);

	wlc_lcn20phy_set_tx_pwr_ctrl(pi, pwrctrl);

	pi->phy_forcecal = TRUE;
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}

int32
wlc_lcn20phy_iovar_isenabled_tpc(phy_info_t *pi, int32 *is_enabled)
{
	if (!pi->sh->clk)
		return BCME_NOCLK;

	*is_enabled = ((phy_utils_read_phyreg((pi), LCN20PHY_TxPwrCtrlCmd) &
		(LCN20PHY_TxPwrCtrlCmd_txPwrCtrl_en_MASK |
		LCN20PHY_TxPwrCtrlCmd_hwtxPwrCtrl_en_MASK |
		LCN20PHY_TxPwrCtrlCmd_use_txPwrCtrlCoefs_MASK))
		== LCN20PHY_TX_PWR_CTRL_HW);

	return BCME_OK;
}

void
wlc_lcn20phy_get_tssi(phy_info_t *pi, int8 *ofdm_pwr, int8 *cck_pwr)
{
	int8 cck_offset;
	ppr_dsss_rateset_t dsss;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	*cck_pwr = 0;
	*ofdm_pwr = 0;

	if (pi->hwpwrctrl_capable)	{
		if (phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatus)
			& LCN20PHY_TxPwrCtrlStatus_estPwrValid_MASK)
			*ofdm_pwr =
				(int8)(PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatus, estPwr) >> 1);
		else if (phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusNew2)
			& LCN20PHY_TxPwrCtrlStatusNew2_estPwrValid1_MASK)
			*ofdm_pwr = (int8)(PHY_REG_READ(pi,
				LCN20PHY, TxPwrCtrlStatusNew2, estPwr1) >> 1);
		ppr_get_dsss(pi->tx_power_offset, WL_TX_BW_20, WL_TX_CHAINS_1, &dsss);
		cck_offset = dsss.pwr[0];
		/* change to 6.2 */
		*cck_pwr = *ofdm_pwr + cck_offset;
	}
}

static void
wlc_phy_txpwr_sromlcn20_read_ppr_parameters(phy_info_t *pi)
{
	srom_pwrdet_t *pwrdet  = pi->pwrdet;
	uint sromrev;

	/* Max tx power */
	pi->tx_srom_max_2g = (int8)PHY_GETINTVAR(pi, rstr_maxp2ga0);

	/* When the max Tx power is less than or euqal to 10dBm, the board is in the
	 * low-power mode, otherwise the board is in the regular mode.
	 */
	if (pi->tx_srom_max_2g > 40) {
		pi->low_power_mode = FALSE;
	} else {
		pi->low_power_mode = TRUE;
	}

	pwrdet->max_pwr[PHY_CORE_0][WL_CHAN_FREQ_RANGE_2G] = pi->tx_srom_max_2g;

	sromrev = (uint)PHY_GETINTVAR(pi, rstr_sromrev);
	if (sromrev >= 9)
		pi->ppr->u.sr_lcn20.cck202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cckbw202gpo);
	else
		pi->ppr->u.sr_lcn20.cck202gpo = (uint16)PHY_GETINTVAR(pi, rstr_cck2gpo);

	/* Extract offsets for 8 OFDM rates */
	if (sromrev >= 9)
		pi->ppr->u.sr_lcn20.ofdmbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_legofdmbw202gpo);
	else
		pi->ppr->u.sr_lcn20.ofdmbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_ofdm2gpo);

	/* Extract offsets for 8 MCS rates */
	/* mcs2gpo(x) are 16 bit numbers */
	if (sromrev >= 9)
		pi->ppr->u.sr_lcn20.mcsbw202gpo = (uint32)PHY_GETINTVAR(pi, rstr_mcsbw202gpo);
	else
		pi->ppr->u.sr_lcn20.mcsbw202gpo =
			((uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo1) << 16) |
			(uint16)PHY_GETINTVAR(pi, rstr_mcs2gpo0);

	pi->ppr->u.sr_lcn20.propbw202gpo = (uint8)PHY_GETINTVAR(pi, rstr_propbw202gpo);
}

static void
wlc_lcn20phy_sample_collect_init(phy_info_t *pi)
{
	/* initialize phy */
	/* Reset sample capture related registers */
	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(LCN20PHY, Debug_Ctrl, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, DebugMux_PingPong, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, DebugMux_TestReg_31_16, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, DebugMux_TestReg_15_0, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerConfiguration, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioTriggerConfig, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig1Low, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig1High, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig2Low, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig2High, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig3Low, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerDlyConfig3High, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerTypeConfig, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, triggerStateConfig, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioTriggerStateConfig, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpiomuxConfig, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioBitEn, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioValidBit, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioTriggerBit, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpioClockBit, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpiomuxConfigExtra1, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, gpiomuxConfigExtra2, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, Intf2txfifo_Ctrl, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, dbg_samp_coll_ctrl, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, dbg_samp_coll_start_mac_xfer_trig_timer_15_0, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, dbg_samp_coll_start_mac_xfer_trig_timer_31_16, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, dbg_samp_coll_end_mac_xfer_trig_timer_15_0, 0x0)
		PHY_REG_WRITE_ENTRY(LCN20PHY, dbg_samp_coll_end_mac_xfer_trig_timer_31_16, 0x0)
	PHY_REG_LIST_EXECUTE(pi);
}

int
wlc_phy_sample_collect_lcn20phy(phy_info_t *pi, wl_samplecollect_args_t *collect, uint32 *buf)
{
	bool suspend;
	uint32 samp_coll_ctl;
	uint32 wait_count = 0;
	uint32 start_ptr, nsamps_out, data, cnt = 0;
	uint16 old_sslpnCalibClkEnCtrl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_lcn20phy_sample_collect_init(pi);

	/* start and stop pointer address */
	W_REG(pi->sh->osh, D11_SampleCollectStartPtr(pi), 0);
	W_REG(pi->sh->osh, D11_SampleCollectStopPtr(pi), collect->nsamps - 1);

	old_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN20PHY_sslpnCalibClkEnCtrl);

	/* Force WLAN antenna */
	wlc_btcx_override_enable(pi);

	/* enabling debug clock */
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, debugClkEn, 1);

	/* 0: from mux output; (See RTL)
	* 1: force to 1; (Valid is always high)
	* 2: force to clken40;
	* 3: force to clken20_OneCycle
	*/
	PHY_REG_MOD(pi, LCN20PHY, DebugMux_PingPong, validSel, collect->module_sel2);

	/* Choose where to tap-off for sample capture */
	PHY_REG_MOD(pi, LCN20PHY, DebugMux_PingPong,
		debugmux_ping_pong_1, collect->module_sel1);

	/* ================================
	* Start Capturing
	* =================================
	* Clear stop state
	*/
	/* set start_sample_collect (bit 4) and stop_sample_collect (bit 5) bits */
	/* setting bit 5 makes it a NON-circular buffer */
	samp_coll_ctl =
		(R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi)) & 0xffcd) |
		(1 << 1) |
		(1 << 4) |
		(1 << 5);
	/* Single shot sample capture PHY_CTL = 0x832 */
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), samp_coll_ctl);

	bzero((uint8 *)buf, collect->nsamps * 4);

	/* Programming the triggers */
	phy_utils_write_phyreg(pi, LCN20PHY_triggerConfiguration, 0x12);
	PHY_REG_MOD(pi, LCN20PHY, triggerTypeConfig,
		resetCounter, 1);
	PHY_REG_MOD(pi, LCN20PHY, Debug_Ctrl,
		debugmux_enable, 1);
	phy_utils_write_phyreg(pi, LCN20PHY_dbg_samp_coll_ctrl, 0x161);
	PHY_REG_MOD(pi, LCN20PHY, gpioTriggerConfig,
		swTrig, 1);

	/* Wait for Sample Collect to complete */
	while (R_REG(pi->sh->osh, D11_SampleCollectCurPtr(pi)) !=
		R_REG(pi->sh->osh, D11_SampleCollectStopPtr(pi))) {
		/* Check for timeout */
		if (wait_count > collect->timeout) {
			PHY_ERROR(("wl%d: %s: Sample Capture failed to complete\n",
				pi->sh->unit, __FUNCTION__));
			break;
		}
		OSL_DELAY(10);
		wait_count++;
	}

	/* Reset things for next iteration */
	PHY_REG_MOD(pi, LCN20PHY, Debug_Ctrl,
		debugmux_enable, 0);
	phy_utils_write_phyreg(pi, LCN20PHY_dbg_samp_coll_ctrl, 0);
	PHY_REG_MOD(pi, LCN20PHY, gpioTriggerConfig,
		swTrig, 0);
	samp_coll_ctl = (R_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi)) & 0xffcd);
	W_REG(pi->sh->osh, D11_PSM_PHY_CTL(pi), samp_coll_ctl);

	/* Copy data into a buffer */
	start_ptr = R_REG(pi->sh->osh, D11_SampleCollectStartPtr(pi));
	nsamps_out = collect->nsamps;

	while (nsamps_out > 0) {
		wlapi_bmac_templateptr_wreg(pi->sh->physhim, start_ptr << 2);
		data = wlapi_bmac_templatedata_rreg(pi->sh->physhim);
		buf[cnt++] = data;

		start_ptr += 1;
		nsamps_out -= 1;
	}

	PHY_REG_WRITE(pi, LCN20PHY, sslpnCalibClkEnCtrl, old_sslpnCalibClkEnCtrl);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return BCME_OK;
}

void
wlc_phy_set_papd_offset_lcn20phy(phy_info_t *pi, int16 int_val)
{
#ifdef LCN20_PAPD_ENABLE
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16 NewRegVal;

	PHY_PAPD(("PAPD Cal Change epsilon offset, new = %d, OLD state:\n", int_val));
	wlc_phy_get_papd_offset_lcn20phy(pi);
	pi_lcn20->epsdelta2g_flag = LCN20PHY_PAPD_OFFSET_SRC_IOVAR;

	NewRegVal = (int_val > 0) ? int_val : (512+int_val);
	PHY_PAPD(("Hex write val: %x\n", NewRegVal));

	PHY_REG_MOD(pi, LCN20PHY, EpsilonTableAdjust, epsilonOffset, NewRegVal);
	PHY_PAPD(("PAPD Cal Change epsilon offset, NEW state:\n"));
	wlc_phy_get_papd_offset_lcn20phy(pi);

#endif /* LCN20_PAPD_ENABLE */
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
}
int
wlc_phy_get_papd_offset_lcn20phy(phy_info_t *pi)
{
#ifdef LCN20_PAPD_ENABLE
#ifdef PAPD_DEBUG
	uint16 regVal;
	int16 regPAPDoffset;

	regVal = (uint16)READ_LCN20PHYREG(pi, EpsilonTableAdjust);
	regPAPDoffset = (regVal>>7) & 0x1ff;
	regPAPDoffset = (regPAPDoffset & 0x100) ? -(512-regPAPDoffset) : regPAPDoffset;
	PHY_PAPD(("Epsilon offset: DRV = %d, REG = %d epsilonScalar=%d regVal=%d\n",
		(int16) pi->u.pi_lcn20phy->epsdelta2g, regPAPDoffset,
		(int16) (regVal & 0x7f), regVal));
#endif /* PAPD_DEBUG */
	return (int)regPAPDoffset;
#else
	return 0;
#endif /* LCN20_PAPD_ENABLE */
}

#ifdef LCN20_PAPD_ENABLE
/* Set PAPD compensation per rate */
void
wlc_phy_papd_comp_rates_lcn20(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint16 PAPDreg;
	PAPDreg = READ_LCN20PHYREG(pi, PapdEnable0);

	if (pi_lcn20->papd_enable) {
		PAPDreg = PAPDreg | 0x1;
	} else {
		PAPDreg = PAPDreg & 0xfffe;
	}
	if (pi_lcn20->papd_mcs_comp) {
		PAPDreg = PAPDreg | 0x2;
	} else {
		PAPDreg = PAPDreg & 0xfffd;
	}
	PHY_REG_WRITE(pi, LCN20PHY, PapdEnable0, PAPDreg);
}

int32
wlc_lcn20phy_get_signal_power(phy_info_t *pi)
{
	uint16 signal_tssi, idle_tssi, adjusted_tssi, tssi_idx;
	int32 estpwr;
	int32 a1 = 0, b0 = 0, b1 = 0;

	signal_tssi = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi);
	signal_tssi ^= 0x100; /* OB format */
	idle_tssi = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0);
	idle_tssi ^= 0x100; /* OB format */

	phy_tpc_get_paparams_for_band(pi, &a1, &b0, &b1);

	adjusted_tssi = signal_tssi - idle_tssi + 511;
	tssi_idx = adjusted_tssi >> 2;
	estpwr = wlc_lcn20phy_tssi2dbm(tssi_idx, a1, b0, b1);

	PHY_PAPD(("     adjTssi:%d, estPw:%d, sigTssi:%d, idTssi:%d \n",
		tssi_idx, estpwr, signal_tssi, idle_tssi));
	return estpwr;
}

int
wlc_phy_papd_calidx_estimate_lcn20(phy_info_t *pi)
{
	bool tx_gain_override_old;
	uint8 save_indx, num_pckts_per_iter;
	uint16 save_txpwrctrl, save_papd;
	phy_txgains_t old_gains;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int step_size, count, count_pkt, tmp_idx, final_idx;
	int sig_pwr_dbm, delta_idx, est_pwr;

	/* save Tx power control, Tx power index, PAPD, and idle TSSI */
	save_txpwrctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	save_indx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	save_papd = PHY_REG_READ(pi, LCN20PHY, PapdEnable0, papd_compEnb);
	tx_gain_override_old = wlc_lcn20phy_tx_gain_override_enabled(pi);
	wlc_lcn20phy_get_tx_gain(pi, &old_gains);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* disable PAPD */
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, 0);

	/* set Tx gains in override mode */
	tmp_idx = PAPD_GAIN_CTRL_INIT_IDX;
	wlc_lcn20phy_enable_tx_gain_override(pi);
	wlc_lcn20phy_set_tx_pwr_by_index(pi, tmp_idx);

	PHY_PAPD(("      tmp cal idx = %d\n", tmp_idx));

	/* number of packets considered at each step of the gain control
	 * algorithm is given by 2^PAPD_GAIN_CTRL_PCKTS_PER_ITER_NPT
	 */
	num_pckts_per_iter = 1 << PAPD_GAIN_CTRL_PCKTS_PER_ITER_NPT;

	for (count = 0; count < PAPD_GAIN_CTRL_MAX_NUM_STEP; count++)
	{
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_en, 1);

		/* estimate signal power */
		est_pwr = 0;
		for (count_pkt = 0; count_pkt < num_pckts_per_iter; count_pkt++) {
			phy_tssical_do_dummy_tx(pi, TRUE, OFF);
			OSL_DELAY(100);
			est_pwr += wlc_lcn20phy_get_signal_power(pi);
		}
		sig_pwr_dbm = est_pwr >> PAPD_GAIN_CTRL_PCKTS_PER_ITER_NPT;

		/* compute the next power index */
		delta_idx = sig_pwr_dbm - pi_lcn20->base_pwr_dbm;
		if (delta_idx > 0) {
			step_size = delta_idx >> 1;
		} else {
			step_size = (delta_idx + 1) >> 1;
		}

		/* verify whether the algorithm has converged */
		if (ABS(delta_idx) <= 2) {
			PHY_PAPD((" *** Break: tmp cal idx = %d, sig pwr = %d, base pwr = %d, ",
				tmp_idx, sig_pwr_dbm, pi_lcn20->base_pwr_dbm));
			PHY_PAPD(("step size = %d, diff = %d \n\n",
				step_size, sig_pwr_dbm - pi_lcn20->base_pwr_dbm));
			break;
		}

		/* set the next power index */
		tmp_idx = LIMIT((tmp_idx + step_size), 0, 127);
		PHY_PAPD(("     (%d) tmpIdx:%d, sigPw:%d, basePw:%d, step:%d(%d)\n",
			count, tmp_idx, sig_pwr_dbm, pi_lcn20->base_pwr_dbm,
			step_size, sig_pwr_dbm - pi_lcn20->base_pwr_dbm));
		wlc_lcn20phy_enable_tx_gain_override(pi);
		wlc_lcn20phy_set_tx_pwr_by_index(pi, tmp_idx);
	}

	/* compute the target power index based on the base power index */
	step_size = (sig_pwr_dbm - pi_lcn20->target_pwr_dbm + 1) >> 1;
	final_idx = LIMIT((tmp_idx + step_size), 0, 127);
	PHY_PAPD(("\n\n *** Closest idx for %d dBm = %d, count = %d\n",
		pi_lcn20->base_pwr_dbm, tmp_idx, count));
	PHY_PAPD((" *** Estimated idx for %d dBm = %d\n",
		pi_lcn20->target_pwr_dbm, final_idx));

	/* restore Tx power control and PAPD state */
	wlc_lcn20phy_set_tx_gain_override(pi, tx_gain_override_old);
	wlc_lcn20phy_set_tx_gain(pi, &old_gains);
	wlc_lcn20phy_set_tx_pwr_by_index(pi, save_indx);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, save_txpwrctrl);
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, save_papd);

	return final_idx;
}

/* Run PAPD calibration
 * TCL: proc lcn20_papd_cal {{mode ""}}
 */
void
wlc_phy_papd_cal_run_lcn20(phy_info_t *pi)
{
	uint8	j;
	uint32 initvalue[LCN20_PAPD_TBL_WRITE_STEP_SIZE] =
		{0, 0, 0, 0, 0, 0, 0, 0};
	uint16	numiter = 16;
	uint32	numParam;
	uint32  eps_table[LCN20_PAPD_EPS_TBL_SIZE];
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint8	yref = 5, start = 5, end = pi_lcn20->papd_end_idx;
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	if (pi_lcn20->papd_enable == 0) {
		/* Disable PAPD compensation */
		PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, 0);
		return;
	}
	/* save and set up phy & Radio regs, then change the gain */
	wlc_phy_papd_phy_setup_lcn20(pi);
	wlc_phy_papd_loopback_radio20692_setup_lcn20(pi);
	wlc_phy_papd_gain_radio20692_setup_lcn20(pi);
	/* Update TIA gain here due to the fact that wlc_phy_papd_gain_radio20692_setup_lcn20
	 * is located in ROM.
	 */
	wlc_lcn20phy_rx_gain_override_wrapper(pi, 1, 0, 0, 0, 0, 0, 4, 3, 0);
	wlc_lcn20phy_rx_gain_override_enable_wrapper(pi, TRUE);
	/* lcn20_clear_eps_tbl */
	for (j = 0; j < LCN20_PAPD_EPS_TBL_SIZE; j += LCN20_PAPD_TBL_WRITE_STEP_SIZE) {
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_EPSILON,
			initvalue, LCN20_PAPD_TBL_WRITE_STEP_SIZE, 32, j);
	}
	wlc_phy_papd_cal_lcn20(pi, numiter, start, yref, end);

	/* fill PAPD Epsilon table fully */
	/*   Write the Epsilon table with 0's for index idx=0 upto idx=start */
	for (j = 0; j < start; j++) {
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_EPSILON,
			initvalue, 1, 32, j);
	}
	/* Write the Epsilon table with last entry value from $end to 63 */
	if (end < LCN20_PAPD_EPS_TBL_SIZE - 1) {
		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_EPSILON,
			&numParam, 1, 32, end);
		for (j = end+1; j < LCN20_PAPD_EPS_TBL_SIZE; j++)
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_EPSILON,
				&numParam, 1, 32, j);
	}

	/* m scale step is epsilonScalar/2^shift=8/2^4=0.5 dB lut step */
	PHY_REG_MOD(pi, LCN20PHY, EpsilonTableAdjust, epsilonScalar, 8);

	/* Cleanup Radio, Gain and PHY */
	wlc_phy_papd_loopback_radio20692_cleanup_lcn20(pi);
	wlc_phy_papd_phy_cleanup_lcn20(pi);

	/* Enable PAPD */
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compCckEnb, 1);
	wlc_phy_papd_comp_rates_lcn20(pi);
	PHY_PAPD(("\n PAPD Cal ENDED, status = %d\n", pi->phy_pacalstatus));

/* Below is epsilon dump debug since regular epsilon dump refuses to work */
#ifdef PAPD_DEBUG
	wlc_phy_dump_epsilon_lcn20(pi, eps_table);
#endif /* PAPD_DEBUG */
}

#ifdef PAPD_DEBUG
void
wlc_phy_dump_epsilon_lcn20(phy_info_t *pi, uint32	*eps_table)
{
	int32 eps_re, eps_im;
	uint16 save_AfeCtrlOvr1Val, save_AfeCtrlOvr1;
	uint8 j = 0;
	/* for eps table */
	save_AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
	save_AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);

	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_EPSILON,
		eps_table, LCN20_PAPD_EPS_TBL_SIZE, 32, 0);

	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, save_AfeCtrlOvr1);

	PHY_PAPD((" PAPD Epsilon Table  Real Image CORE 0 \n"));
	for (j = 0; j < LCN20_PAPD_EPS_TBL_SIZE; j++) {
		phy_papdcal_decode_epsilon(eps_table[j], &eps_re, &eps_im);
		PHY_PAPD(("{%d %d}\n ", eps_re, eps_im));
	}
	PHY_PAPD(("\n\n"));
}
#endif /* PAPD_DEBUG */

bool
wlc_phy_papd_eps_valid_lcn20(phy_info_t *pi)
{
	bool is_cal_valid = TRUE;
	int idx = LCN20_PAPD_EVAL_IDX_LOW;
	uint32 eps_table[LCN20_PAPD_EPS_TBL_SIZE];
	int32 eps_re, eps_im, eps_re_next, eps_im_next;
	int32 angle_num = 0, angle_den = 4096;
	int32 angle_prev_num = 0, angle_prev_den = 4096;
	int32 angle_2nd_prev_num = 0, angle_2nd_prev_den = 4096;
	int32 eps_re_eps_im_next, eps_re_next_eps_im;
	int32 eps_re_eps_re_next;
	int32 tmp_num, tmp_den;

	/* read the eps table */
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_EPSILON,
		eps_table, LCN20_PAPD_EPS_TBL_SIZE, 32, 0);

	phy_papdcal_decode_epsilon(eps_table[LCN20_PAPD_EVAL_IDX_LOW],
		&eps_re, &eps_im);
	eps_re += 4096;

	/* Compare the difference in angles of (1 + epsilon) corresponding
	 * to entries idx and idx-2, from the epsilon table.
	 * If the difference in angles is bigger than 5 degrees, calibration
	 * would be classified as a bad calibration. If the difference in
	 * angles for the entire table is less than 5 degrees, the calibration
	 * would be classified as a valid calibration.
	 */
	for (idx = LCN20_PAPD_EVAL_IDX_LOW + 1;
		(idx <= pi->u.pi_lcn20phy->papd_end_idx) && is_cal_valid;
		idx ++) {

		phy_papdcal_decode_epsilon(eps_table[idx],
			&eps_re_next, &eps_im_next);
		eps_re_next += 4096;

		angle_2nd_prev_num = angle_prev_num;
		angle_2nd_prev_den = angle_prev_den;
		angle_prev_num = angle_num;
		angle_prev_den = angle_den;

		/* To compute the difference in the angle of (1 + epsilon) values
		 * corresponding to indices idx and idx-1, from the epsilon table,
		 * the following approximation is used:
		 *
		 * D_phase(idx) = atan(y_idx/x_idx) - atan(y_{idx-1}/x_{idx-1})
		 *              ~= (y_idx.x_{idx-1} - y_{idx-1}.x_idx)/(x_{idx-1}.x_idx)
		 *
		 * where ~= represents "approximately equal to" sign. Moreover, here
		 * it is assumed that the difference between atan(y_idx/x_idx) and
		 * atan(y_{idx-1}/x_{idx-1}) is sufficiently small.
		 */
		eps_re_eps_im_next = eps_re * eps_im_next;
		eps_re_next_eps_im = eps_re_next * eps_im;
		eps_re_eps_re_next = eps_re * eps_re_next;
		angle_num = (eps_re_eps_im_next - eps_re_next_eps_im)
			>> LCN20_PAPD_SHIFT_D_PHASE;
		angle_den = (eps_re_eps_re_next)
			>> LCN20_PAPD_SHIFT_D_PHASE;

		/* A calibration is declares as a bad calibration if there exists an index idx,
		 * for which
		 *
		 *              |D_phase(idx+2) - D_phse(idx)]| > 0.7 degree/index
		 *
		 * It worth noting that 0.7 degree/index ~= (1/82) radian/index
		 */
		tmp_num = ABS((angle_num * angle_2nd_prev_den) -
			(angle_den * angle_2nd_prev_num)) >> LCN20_PAPD_SHIFT_D2_PHASE;
		tmp_den = ABS(angle_2nd_prev_den * angle_den) >> LCN20_PAPD_SHIFT_D2_PHASE;
		if ((tmp_num * 82) > tmp_den) {
			is_cal_valid = FALSE;
		}

		eps_re = eps_re_next;
		eps_im = eps_im_next;
	}
	return is_cal_valid;
}

void
wlc_phy_papd_cal_lcn20(phy_info_t *pi, uint16 num_iter, uint16 startindex,
	uint16 yrefindex, uint16 stopindex)
{
	int calmode = 0;
	int16 cal_tone_mag = 100;
	int16 cal_tone_freq_kHz = 2500;
	int rf_offset_en_idx;
	int indx, cal_idx;
	int16 rf_offset = 0;
	uint32 gains = 0;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	phytbl_info_t tab;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compCckEnb, 1);
	PHY_REG_MOD(pi, LCN20PHY, PapdEnable0, papd_compEnb, 1);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papd_calEnb, 1);
	PHY_REG_WRITE(pi, LCN20PHY, PapdEpsilonUpdateIterations, num_iter);
	PHY_REG_WRITE(pi, LCN20PHY, PapdCalSettle, 0x20);
	PHY_REG_WRITE(pi, LCN20PHY, PapdIpaOffCorr, 0x0);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalYrefEpsilon, papdYrefAddr, yrefindex);

	/* Epsilon fixed point representation and range. 1 = s.12 range +/-1 */
	PHY_REG_MOD(pi, LCN20PHY, EpsilonOverrideI, epsilonFixedPoint, 0x1);
	PHY_REG_WRITE(pi, LCN20PHY, PapdCalCorrelate, 0x80);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papdCorrShift, 0x7);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papdLambda_I, 0x0);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papdLambda_Q, 0x0);

	/* Support only mode 0 at this point */
#ifdef LCN20_PAPD_WARMUP
	wlc_lcn20phy_start_tx_tone(pi, (cal_tone_freq_kHz*1000),
			cal_tone_mag, TRUE, 0);
	OSL_DELAY(5000);
	wlc_lcn20phy_stop_tx_tone(pi);
#endif /* LCN20_PAPD_WARMUP	*/

	PHY_REG_MOD(pi, LCN20PHY, PapdCalAddress, papdStartAddr, startindex);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalAddress, papdEndAddr, stopindex);

	wlc_lcn20phy_start_tx_tone(pi, (cal_tone_freq_kHz*1000), cal_tone_mag, 0, TRUE, 1);
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, forceSdFeClkEn, 0x1);

	/* resetting Tx FIFO */
	PHY_REG_MOD(pi, LCN20PHY, TxFIFORST, txfifo_rst, 1);
	OSL_DELAY(300);
	if (calmode == 0) {
		PHY_REG_WRITE(pi, LCN20PHY, PapdCalStart, 0x1);  /* Run the PAPD */

		/* Wait for cal to end and verify it ended */
		SPINWAIT(READ_LCN20PHYREG(pi, PapdCalStart), LCN20PHY_SPINWAIT_PAPDCAL_USEC);
		ASSERT(!(READ_LCN20PHYREG(pi, PapdCalStart) & 1));
		SPINWAIT(READ_LCN20PHYREG(pi, PapdCalStart), LCN20PHY_SPINWAIT_PAPDCAL_USEC);
		if (READ_LCN20PHYREG(pi, PapdCalStart) & 1)
			pi->phy_pacalstatus |= PHY_PACALSTATUS_CAL_TIMEOUT;
		PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, forceSdFeClkEn, 0x0);
		wlc_lcn20phy_stop_tx_tone(pi);
	}
	/* short wideband burst with 4 tones to meet FCC 6dB OBW compliance.
	  * tone freqs at cal tone freq -/+ 1500 kHz and -/+ 500 kHz
	  */
		wlc_lcn20phy_start_tx_tone(pi,
			((cal_tone_freq_kHz-1500)*1000), (2*cal_tone_mag), 0, TRUE, 1);
		OSL_DELAY(50);
		wlc_lcn20phy_stop_tx_tone(pi);
		wlc_lcn20phy_start_tx_tone(pi,
			((cal_tone_freq_kHz+1500)*1000), (2*cal_tone_mag), 0, TRUE, 1);
		OSL_DELAY(50);
		wlc_lcn20phy_stop_tx_tone(pi);
		wlc_lcn20phy_start_tx_tone(pi,
			((cal_tone_freq_kHz-500)*1000), (2*cal_tone_mag), 0, TRUE, 1);
		OSL_DELAY(50);
		wlc_lcn20phy_stop_tx_tone(pi);
		wlc_lcn20phy_start_tx_tone(pi,
			((cal_tone_freq_kHz+500)*1000), (2*cal_tone_mag), 0, TRUE, 1);
		OSL_DELAY(50);
		wlc_lcn20phy_stop_tx_tone(pi);

	if (pi->u.pi_lcn20phy->do_papd_calidx_est) {
		cal_idx = pi_lcn20->papd_cal_idx;
		rf_offset = lcn20phy_2GHz_rf_offset_tbl_rev0[cal_idx];
		/* determine Tx gain idx "rf_offset_en_idx" corresponding to Q2-format power,
		 * LCN20_PAPD_RF_OFFSET_EN_POW_Q2 based on the assumption that cal_idx correponds
		 * to pi_lcn20->target_pwr_dbm in Q3 format
		 */
		rf_offset_en_idx = cal_idx +
			(pi_lcn20->target_pwr_dbm >> 1) - LCN20_PAPD_RF_OFFSET_EN_POW_Q2;
		tab.tbl_width = 32;
		tab.tbl_len = 1;
		tab.tbl_id = LCN20PHY_TBL_ID_TXPWRCTL;
		for (indx = 0; indx < 128; indx++) {
			tab.tbl_offset = LCN20PHY_TX_PWR_CTRL_PWR_OFFSET + indx;
			tab.tbl_ptr = &gains;
			wlc_lcn20phy_read_table(pi,  &tab);
			if (indx < rf_offset_en_idx) {
				gains = (gains & 0xfffffe00) | rf_offset;
			} else {
				gains = (gains & 0xfffffe00) |
					lcn20phy_2GHz_rf_offset_tbl_rev0[indx];
			}
			wlc_lcn20phy_write_table(pi, &tab);
		}
	}
	/* update epsilon offset value such that the combination of the epsilon offset
	 * and rf offset settings at papd cal index correspond to the epsilon offset
	 * value suggested by nvram file.
	 * rf_offset and epsdelta2g are, respectively, in Q3 and Q2 formats.
	 */
	rf_offset = pi_lcn20->epsdelta2g - (rf_offset >> 1);
	wlc_phy_set_papd_offset_lcn20phy(pi, rf_offset);
	PHY_REG_MOD(pi, LCN20PHY, papd_analog_gain_ovr_val, papd_analog_gain_ovr_val, 0);
}

/* Set the Radio gains for PAPD
 * note - the gain register values are saved in function
 * wlc_phy_papd_loopback_radio20692_setup_lcn20
 */
/* TO DO : function currently uses fixed gain value */
static void
wlc_phy_papd_gain_radio20692_setup_lcn20(phy_info_t *pi)
{
	uint8 tx_atten = 3, rx_atten = 3;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Set txattn and rxattn */
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_atten, tx_atten);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rf2g_papdcal_rx_attn, rx_atten);

	/* Set PAPD calibration Tx power index */
	wlc_lcn20phy_set_tx_pwr_by_index(pi, pi_lcn20->papd_cal_idx);

	/* Set the loopback gains */
	wlc_lcn20phy_rx_gain_override(pi, 0, 0, 0, 0, 3, 3, 0);
	wlc_lcn20phy_rx_gain_override_enable(pi, TRUE);
}

void
wlc_phy_papd_loopback_radio20692_setup_save_lcn20(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_radioregs_t *psave = pi_lcn20->rxiqcal_radioregs;
	lcn20phy_papd_radioregs_t	 *psave_add = pi_lcn20->papd_radioregs;
	phy_txgains_t *old_gains = &(psave_add->gains);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;
	/* Make sure we didn't call the setup twice, in add struct too */
	ASSERT(!psave_add->is_orig);
	psave_add->is_orig = TRUE;

	/* Registers not in IQ cal
	 * In TCL these regs are set in the main PAPD cal proc - lcn20_papd_cal
	 */

	psave_add->wl_pa2g_cfg1 = READ_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0);
	psave_add->wl_pa2g_cfg3 = READ_RADIO_REG_20692(pi, WL_PA2G_CFG3, 0);
	psave_add->wl_rxmix2g_cfg1 = READ_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0);
	psave_add->wl_tx_top_2g_ovr2 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0);
	psave_add->wl_tx_dac_cfg5 = READ_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0);
	psave_add->wl_tia_cfg5 = READ_RADIO_REG_20692(pi, WL_TIA_CFG5, 0);

	psave_add->wl_adc_cfg10   = READ_RADIO_REG_20692(pi, WL_ADC_CFG10, 0);
	psave_add->wl_tia_cfg8    = READ_RADIO_REG_20692(pi, WL_TIA_CFG8, 0);
	psave_add->wl_trsw2g_cfg1 = READ_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0);
	psave_add->wl_lna2g_cfg1 = READ_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0);
	psave_add->wl_tx_logen2g_cfg1 = READ_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0);
	psave_add->wl_tx2g_cfg1 = READ_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0);

	psave_add->wl_rxrf2g_cfg1 = READ_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0);
	psave_add->wl_tia_cfg3 = READ_RADIO_REG_20692(pi, WL_TIA_CFG3, 0);
	psave_add->wl_pmu_op = READ_RADIO_REG_20692(pi, WL_PMU_OP, 0);
	psave_add->wl_adc_cfg8 = READ_RADIO_REG_20692(pi, WL_ADC_CFG8, 0);
	psave_add->wl_adc_cfg1 = READ_RADIO_REG_20692(pi, WL_ADC_CFG1, 0);

	/* Registers saved and restored even though PAPD does NOT change them currently,
	 * keeping save for later debugs
	 */
	psave_add->wl_pa2g_idac1 = READ_RADIO_REG_20692(pi, WL_PA2G_IDAC1, 0);
	psave_add->wl_pa2g_incap = READ_RADIO_REG_20692(pi, WL_PA2G_INCAP, 0);

	wlc_lcn20phy_get_tx_gain(pi, old_gains);
	psave_add->bbmult = wlc_lcn20phy_get_bbmult(pi);
	psave_add->pa_gain = wlc_lcn20phy_get_pa_gain(pi);

	MOD_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, wl_txbb_dac2adc, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG5, 0, wl_tia_out_test, 0x0);

	/* Registers also in IQ cal */
	psave->wl_rx_adc_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0);
	psave->wl_tx_top_2g_ovr1 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0);
	psave->wl_rx_top_2g_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0);
	psave->wl_rx_bb_ovr1 = READ_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0);
	psave->wl_minipmu_ovr1 = READ_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0);
}

static void
wlc_phy_papd_loopback_radio20692_setup_lcn20(phy_info_t *pi)
{

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	wlc_phy_papd_loopback_radio20692_setup_save_lcn20(pi);

	/* force off the dac 2 adc switches */
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_in_test, 1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG10, 0, wl_adc_in_test, 0);

	/* TODO:
	 * check if WL_TIA_CFG8: wl_tia_tx_lpbck_i and wl_tia_tx_lpbck_q
	 * should be set to 0 or left unchanged
	 */
	/* Don't enable bbpd input to TIA */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, wl_tia_tx_lpbck_i, 0);
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, wl_tia_tx_lpbck_q, 0);

	/* PAPD LOOPBACK PATH */

	/* TR sw in TX mode */
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, wl_ovr_trsw2g_bias_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, wl_trsw2g_bias_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_tr_rx_en, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_tr_rx_en, 0);

	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_logen2g_tx_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, wl_logen2g_tx_pu_bias, 1);

	/* Enable ipapd */
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_cal2g_pa_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_loopback2g_papdcal_pu, 1);

	/* Powerdown lna1 */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_lna1_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_lna1_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_lna2g_lna1_out_short_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, wl_lna2g_lna1_out_short_pu, 1);

	/* powerdown rx main gm2g  */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_gm2g_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_pwrup, 0x0);

	/* powerup auxgm2g */
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxgm2g_auxgm_pwrup, 0x1);

	/* powerup rx mixer */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxmix2g_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, wl_rxmix2g_pu, 1);

	/* powerup rxdiv2g */
	MOD_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, wl_ovr_rxdiv2g_pu_bias, 1);
	MOD_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, wl_rxdiv2g_pu_bias, 1);

	/* powerup tia */
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_resstring, 1);
	/* Power up common mode reference */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_resstring, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_amp1_pwrup, 1);
	/* PU 1st amp */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_amp1_pwrup, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, wl_ovr_tia_pwrup_amp2, 1);
	/* PU 2nd amp */
	MOD_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, wl_tia_pwrup_amp2, 1);

	wlc_lcn20phy_tia_gain(pi, 3);

	/* powerup adc */
	MOD_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, wl_ovr_pmu_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_PMU_OP, 0, wl_ADCldo_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_clk_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, wl_adc_clk_slow_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_slow_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_slow_pu, 1);
	MOD_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, wl_ovr_adc_sipo_pu, 0x1);
	MOD_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, wl_adc_sipo_pu, 1);

}

static void
wlc_phy_papd_loopback_radio20692_cleanup_lcn20(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_radioregs_t *psave = pi_lcn20->rxiqcal_radioregs;
	lcn20phy_papd_radioregs_t	 *psave_add = pi_lcn20->papd_radioregs;
	phy_txgains_t *old_gains = &(psave_add->gains);

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the cleanup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;
	/* Make sure we didn't call the setup twice, in add struct too */
	ASSERT(psave_add->is_orig);
	psave_add->is_orig = FALSE;

	/* Registers also in IQ cal */

	WRITE_RADIO_REG_20692(pi, WL_RX_ADC_OVR1, 0, psave->wl_rx_adc_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR1, 0, psave->wl_tx_top_2g_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_RX_TOP_2G_OVR1, 0, psave->wl_rx_top_2g_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_RX_BB_OVR1, 0, psave->wl_rx_bb_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_MINIPMU_OVR1, 0, psave->wl_minipmu_ovr1);

	/* Registers not in IQ cal */

	WRITE_RADIO_REG_20692(pi, WL_PA2G_CFG1, 0, psave_add->wl_pa2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_PA2G_CFG3, 0, psave_add->wl_pa2g_cfg3);
	WRITE_RADIO_REG_20692(pi, WL_RXMIX2G_CFG1, 0, psave_add->wl_rxmix2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, psave_add->wl_tx_top_2g_ovr2);
	WRITE_RADIO_REG_20692(pi, WL_TX_DAC_CFG5, 0, psave_add->wl_tx_dac_cfg5);
	WRITE_RADIO_REG_20692(pi, WL_TIA_CFG5, 0, psave_add->wl_tia_cfg5);

	WRITE_RADIO_REG_20692(pi, WL_ADC_CFG10, 0, psave_add->wl_adc_cfg10);
	WRITE_RADIO_REG_20692(pi, WL_TIA_CFG8, 0, psave_add->wl_tia_cfg8);
	WRITE_RADIO_REG_20692(pi, WL_TRSW2G_CFG1, 0, psave_add->wl_trsw2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_PA2G_INCAP, 0, psave_add->wl_pa2g_incap);
	WRITE_RADIO_REG_20692(pi, WL_LNA2G_CFG1, 0, psave_add->wl_lna2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TX_LOGEN2G_CFG1, 0, psave_add->wl_tx_logen2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, psave_add->wl_tx2g_cfg1);

	WRITE_RADIO_REG_20692(pi, WL_RXRF2G_CFG1, 0, psave_add->wl_rxrf2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TIA_CFG3, 0, psave_add->wl_tia_cfg3);
	WRITE_RADIO_REG_20692(pi, WL_PMU_OP, 0, psave_add->wl_pmu_op);
	WRITE_RADIO_REG_20692(pi, WL_ADC_CFG8, 0, psave_add->wl_adc_cfg8);
	WRITE_RADIO_REG_20692(pi, WL_ADC_CFG1, 0, psave_add->wl_adc_cfg1);

	/* Registers saved and restored even though PAPD does NOT change them currently,
	 * keeping save for later debugs
	 */
	WRITE_RADIO_REG_20692(pi, WL_PA2G_IDAC1, 0, psave_add->wl_pa2g_idac1);
	WRITE_RADIO_REG_20692(pi, WL_PA2G_INCAP, 0, psave_add->wl_pa2g_incap);

	/* restore the gain settings */
	wlc_lcn20phy_set_tx_gain(pi, old_gains);
	wlc_lcn20phy_set_bbmult(pi, (uint8) psave_add->bbmult);
	wlc_lcn20phy_set_pa_gain(pi, psave_add->pa_gain);

}

static void
wlc_phy_papd_phy_setup_lcn20(phy_info_t *pi)
{

	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	lcn20phy_rxiqcal_phyregs_t *psave = pi_lcn20->rxiqcal_phyregs;
	lcn20phy_papd_phyregs_struct_t *psave_add = pi_lcn20->papd_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	psave->ofdm_filt_type = pi_lcn20->ofdm_filt_type;
	wlc_lcn20phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM,
			PAPD_TX_IIR_FILTER_TYPE);

	/* Registers not in Rx IQ cal */
	psave_add->AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
	psave_add->AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);

	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1); /* for eps table */
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1); /* for eps table */

	/* Registers in Rx IQ cal */
	/* Save RFCTRL state, save all RFCTRL override register */
	psave->RFOverride0 = READ_LCN20PHYREG(pi, RFOverride0);
	psave->RFOverrideVal0 = READ_LCN20PHYREG(pi, RFOverrideVal0);
	psave->rfoverride2 = READ_LCN20PHYREG(pi, rfoverride2);
	psave->rfoverride2val = READ_LCN20PHYREG(pi, rfoverride2val);
	psave->rfoverride3 = READ_LCN20PHYREG(pi, rfoverride3);
	psave->rfoverride3_val = READ_LCN20PHYREG(pi, rfoverride3_val);
	psave->rfoverride4 = READ_LCN20PHYREG(pi, rfoverride4);
	psave->rfoverride4val = READ_LCN20PHYREG(pi, rfoverride4val);
	psave->rfoverride5 = READ_LCN20PHYREG(pi, rfoverride5);
	psave->rfoverride5val = READ_LCN20PHYREG(pi, rfoverride5val);
	psave->rfoverride7 = READ_LCN20PHYREG(pi, rfoverride7);
	psave->rfoverride7val = READ_LCN20PHYREG(pi, rfoverride7val);
	psave->rfoverride8 = READ_LCN20PHYREG(pi, rfoverride8);
	psave->rfoverride8val = READ_LCN20PHYREG(pi, rfoverride8val);
	psave->sslpnCalibClkEnCtrl = READ_LCN20PHYREG(pi, sslpnCalibClkEnCtrl);

	/* Save tx gain state
	* save bb_mult, txgain, papr/IIR related parameter
	*/
	psave->bbmult = wlc_lcn20phy_get_bbmult(pi);
	/* Turn off tx pwr ctrl */
	psave->txidx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	psave->SAVE_txpwrctrl_on = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* temporarily turn off PAPD in case it is enabled, disable PAPR */
	psave->PapdEnable0 = READ_LCN20PHYREG(pi, PapdEnable0);
	psave->papr_ctrl = READ_LCN20PHYREG(pi, papr_ctrl);
	PHY_REG_MOD(pi, LCN20PHY, papr_ctrl, papr_blk_en, 0);

	/* save Rx/Tx farrow related register setting and DVGA2 related register setting */
	psave_add->RxFeCtrl1 = READ_LCN20PHYREG(pi, RxFeCtrl1);
	psave->RxSdFeConfig1 = READ_LCN20PHYREG(pi, RxSdFeConfig1);
	psave->RxSdFeConfig6 = READ_LCN20PHYREG(pi, RxSdFeConfig6);
	psave->phyreg2dvga2 = READ_LCN20PHYREG(pi, phyreg2dvga2);
	PHY_REG_MOD(pi, LCN20PHY, RxFeCtrl1, forceSdFeClkEn, 1);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, 0x1);

	/* enable LOFT comp, enable TX IQ comp */
	psave->SAVE_Core1TxControl = READ_LCN20PHYREG(pi, Core1TxControl);
	/* enable LOFT comp and TX IQ Ccmp */
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, iqImbCompEnable, 1);
	PHY_REG_MOD(pi, LCN20PHY, Core1TxControl, loft_comp_en, 1);

	/* ensure the cal clock has been enabled */
	PHY_REG_WRITE(pi, LCN20PHY, sslpnCalibClkEnCtrl, 0x780f);

	/* save DSSF control register */
	psave->DSSF_control_0 = READ_LCN20PHYREG(pi, DSSF_control_0);

	/* Disable DSSF as it can suppress reference loopback tone or its image,  restore it */
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s1, 0);
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s2, 0);
	/* Enable TIA */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrfrxpu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrfrxpu_ovr_val, 1);
}

static void
wlc_phy_papd_phy_cleanup_lcn20(phy_info_t *pi)
{

	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_rxiqcal_phyregs_t *psave = pi_lcn20->rxiqcal_phyregs;
	lcn20phy_papd_phyregs_struct_t *psave_add = pi_lcn20->papd_phyregs;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the cleanup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	wlc_lcn20phy_load_tx_iir_filter(pi, TX_IIR_FILTER_OFDM, psave->ofdm_filt_type);

	/* Registers not in Rx IQ cal */
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, psave_add->AfeCtrlOvr1Val);
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, psave_add->AfeCtrlOvr1);
	WRITE_LCN20PHYREG(pi, RxFeCtrl1, psave_add->RxFeCtrl1);

	/* Registers in Rx IQ cal */

	WRITE_LCN20PHYREG(pi, RFOverride0, psave->RFOverride0);
	WRITE_LCN20PHYREG(pi, RFOverrideVal0, psave->RFOverrideVal0);
	WRITE_LCN20PHYREG(pi, rfoverride2, psave->rfoverride2);
	WRITE_LCN20PHYREG(pi, rfoverride2val, psave->rfoverride2val);
	WRITE_LCN20PHYREG(pi, rfoverride3, psave->rfoverride3);
	WRITE_LCN20PHYREG(pi, rfoverride3_val, psave->rfoverride3_val);
	WRITE_LCN20PHYREG(pi, rfoverride4, psave->rfoverride4);
	WRITE_LCN20PHYREG(pi, rfoverride4val, psave->rfoverride4val);
	WRITE_LCN20PHYREG(pi, rfoverride5, psave->rfoverride5);
	WRITE_LCN20PHYREG(pi, rfoverride5val, psave->rfoverride5val);
	WRITE_LCN20PHYREG(pi, rfoverride7, psave->rfoverride7);
	WRITE_LCN20PHYREG(pi, rfoverride7val, psave->rfoverride7val);
	WRITE_LCN20PHYREG(pi, rfoverride8, psave->rfoverride8);
	WRITE_LCN20PHYREG(pi, rfoverride8val, psave->rfoverride8val);

	/* restore tx gain state
	* save bb_mult, txgain, papr/IIR related parameter
	*/
	wlc_lcn20phy_set_bbmult(pi, psave->bbmult);
	wlc_lcn20phy_set_tx_pwr_by_index(pi, psave->txidx);
	/* restore tx pwr ctrl */

	wlc_lcn20phy_set_tx_pwr_ctrl(pi, psave->SAVE_txpwrctrl_on);
	/* restore PAPR & PAPD */
	WRITE_LCN20PHYREG(pi, papr_ctrl, psave->papr_ctrl);
	WRITE_LCN20PHYREG(pi, PapdEnable0, psave->PapdEnable0);

	/* restore Rx/Tx farrow related register setting and DVGA2 related register setting */

	WRITE_LCN20PHYREG(pi, RxSdFeConfig1, psave->RxSdFeConfig1);
	WRITE_LCN20PHYREG(pi, RxSdFeConfig6, psave->RxSdFeConfig6);
	WRITE_LCN20PHYREG(pi, phyreg2dvga2, psave->phyreg2dvga2);
	WRITE_LCN20PHYREG(pi, Core1TxControl, psave->SAVE_Core1TxControl);
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, psave->sslpnCalibClkEnCtrl);
	WRITE_LCN20PHYREG(pi, DSSF_control_0, psave->DSSF_control_0);
}

#if defined(BCMDBG) || defined(BCMDBG_PHYDUMP) || defined(WLTEST)
void
wlc_phy_papd_dump_eps_trace_lcn20(phy_info_t *pi, struct bcmstrbuf *b)
{
	uint8 j;
	uint32 eps_table[LCN20_PAPD_EPS_TBL_SIZE];
	int32 eps_re, eps_im;
	uint16 save_AfeCtrlOvr1Val, save_AfeCtrlOvr1;
	BCM_REFERENCE(b);
	/* for eps table */
	save_AfeCtrlOvr1Val = READ_LCN20PHYREG(pi, AfeCtrlOvr1Val);
	save_AfeCtrlOvr1 = READ_LCN20PHYREG(pi, AfeCtrlOvr1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, dac_pu_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, dac_pu_ovr, 1);

	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_EPSILON,
		eps_table, LCN20_PAPD_EPS_TBL_SIZE, 32, 0);

	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
	WRITE_LCN20PHYREG(pi, AfeCtrlOvr1, save_AfeCtrlOvr1);

	PHY_PAPD((" PAPD Epsilon Table  Real Image CORE 0 \n"));
	for (j = 0; j < LCN20_PAPD_EPS_TBL_SIZE; j++) {
		phy_papdcal_decode_epsilon(eps_table[j], &eps_re, &eps_im);
		PHY_PAPD(("{%d %d}\n ", eps_re, eps_im));
	}
	PHY_PAPD(("\n\n"));
/*
	bcm_bprintf(b, "  PAPD Epsilon Table  Real Image CORE 0 \n");
	for (j = 0; j < LCN20_PAPD_EPS_TBL_SIZE; j++) {
		phy_papdcal_decode_epsilon(eps_table[j], &eps_re, &eps_im);
		PHY_CAL(("{%d %d} ", eps_re, eps_im));
		bcm_bprintf(b, "{%d %d}\n ", eps_re, eps_im);
	}
	PHY_CAL(("\n\n"));
*/
}
#endif // endif
#endif /* LCN20_PAPD_ENABLE */

static void wlc_lcn20phy_apply_gainidx_settings(phy_info_t *pi, uint8 gain_idx)
{
	uint32 gains;
	uint16 rxgains[8];

	/* Set to init gains through overrides */
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAINIDX,
		&gains, 1, 32, gain_idx);
	rxgains[0] = (gains & LCN20PHY_GainIdxTbl_tr_attn_MASK) >>
		LCN20PHY_GainIdxTbl_tr_attn_SHIFT;
	rxgains[1] = (gains & LCN20PHY_GainIdxTbl_elna_MASK) >>
		LCN20PHY_GainIdxTbl_elna_SHIFT;
	rxgains[2] = (gains & LCN20PHY_GainIdxTbl_slna_byp_MASK) >>
		LCN20PHY_GainIdxTbl_slna_byp_SHIFT;
	rxgains[3] = (gains & LCN20PHY_GainIdxTbl_slna_rout_MASK) >>
		LCN20PHY_GainIdxTbl_slna_rout_SHIFT;
	rxgains[4] = (gains & LCN20PHY_GainIdxTbl_slna_gain_MASK) >>
		LCN20PHY_GainIdxTbl_slna_gain_SHIFT;
	rxgains[5] = (gains & LCN20PHY_GainIdxTbl_gaintbl_idx_MASK) >>
		LCN20PHY_GainIdxTbl_gaintbl_idx_SHIFT;

	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAIN, &gains, 1, 32, rxgains[5]);
	rxgains[5] = (gains & LCN20PHY_GainTbl_lna2_gain_MASK) >>
		LCN20PHY_GainTbl_lna2_gain_SHIFT;
	rxgains[6] = (gains & LCN20PHY_GainTbl_tia_gain_MASK) >>
		LCN20PHY_GainTbl_tia_gain_SHIFT;
	rxgains[7] = (gains & LCN20PHY_GainTbl_dvga1_gain_MASK) >>
		LCN20PHY_GainTbl_dvga1_gain_SHIFT;

#ifdef DSSF_DEBUG
	PHY_DSSF(("IDX %d: %d %d %d %d %d %d %d %d\n", gain_idx, rxgains[0], rxgains[1], rxgains[2],
		rxgains[3], rxgains[4], rxgains[5], rxgains[6], rxgains[7]));
#endif // endif

	wlc_lcn20phy_rx_gain_override_wrapper(pi, rxgains[0], rxgains[1], rxgains[2],
		rxgains[3], rxgains[4], rxgains[5], rxgains[6], rxgains[7], 0);
	wlc_lcn20phy_rx_gain_override_enable_wrapper(pi, TRUE);
}

static int16 wlc_lcn20phy_noise_est(phy_info_t *pi, uint8 dvga1, int16 rfgain,
	int16 *noise_pwr_dB)
{
	int16 n_dbm_total[PHY_CORE_MAX], ref_dbm_total, total_gain[PHY_CORE_MAX];
	int16 i, j;
	phy_iq_est_t est = {0, 0, 0};
	uint32 noise_powers[LCN20PHY_NOISEEST_NUM_COLLECT], cmplx_pwr[PHY_CORE_MAX];
	uint32 temp, tj, ti;
	uint16 idx = 0, crsminpwr[PHY_CORE_MAX];
	uint16 SAVE_sslpnCalibClkEnCtrl, SAVE_RxSdFeConfig1, SAVE_DVGA2, SAVE_agc_fsm_en;
	phy_iq_comp_t pcomp;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	*noise_pwr_dB = LCN20PHY_NOISEEST_REFNOISEPWRDB << LCN20PHY_NOISEDB_SCALE;
	ref_dbm_total = LCN20PHY_NOISEEST_REFDBM << LCN20PHY_NOISEDB_SCALE;

	/* Put AGC fsm in HG lock mode */
	SAVE_agc_fsm_en = PHY_REG_READ(pi, LCN20PHY, agcControl4, c_agc_fsm_en);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
	OSL_DELAY(1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
	OSL_DELAY(1);

	SAVE_sslpnCalibClkEnCtrl = READ_LCN20PHYREG(pi, sslpnCalibClkEnCtrl);
	SAVE_RxSdFeConfig1 = READ_LCN20PHYREG(pi, RxSdFeConfig1);
	SAVE_DVGA2 = READ_LCN20PHYREG(pi, phyreg2dvga2);
	wlc_lcn20phy_rx_iq_comp(pi, LCN20PHY_RXIQCOMP_GET, &pcomp);

	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1);
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, forceiqEstclkOn, 1);

	PHY_REG_MOD(pi, LCN20PHY, phyreg2dvga2, dvga2_gain_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, phyreg2dvga2, dvga2_gain_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, dvga1);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);
	wlc_lcn20phy_rx_iq_comp(pi, LCN20PHY_RXIQCOMP_RESET, NULL);

	for (j = 0; j < LCN20PHY_NOISEEST_MAXTRY; j++) {
		wlc_lcn20phy_rx_iq_est(pi, &est, LCN20PHY_NOISEEST_NUM_SAMPS, 32, 0, FALSE);
		temp = est.i_pwr + est.q_pwr;
		if (temp > 0) {
			noise_powers[idx++] = temp;
			if (idx == LCN20PHY_NOISEEST_NUM_COLLECT)
				break;
		}
	}

	wlc_lcn20phy_rx_iq_comp(pi, LCN20PHY_RXIQCOMP_SET, &pcomp);
	WRITE_LCN20PHYREG(pi, phyreg2dvga2, SAVE_DVGA2);
	WRITE_LCN20PHYREG(pi, RxSdFeConfig1, SAVE_RxSdFeConfig1);
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, SAVE_sslpnCalibClkEnCtrl);

	/* Remove HG-lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, SAVE_agc_fsm_en);

	if (idx < LCN20PHY_NOISEEST_NUM_COLLECT) {
		PHY_TRACE(("Error during noise-est, return default value; %s \n",
		__FUNCTION__));
		return ref_dbm_total;
	}

	for (j = 1; j < LCN20PHY_NOISEEST_NUM_COLLECT; j++) {
		tj = noise_powers[j];
		for (i = 0; i < j; i++) {
			ti = noise_powers[i];
			if (tj <= ti) {
				noise_powers[i] = tj;
				tj = ti;
			}
		}
		noise_powers[j] = tj;
	}

#ifdef DSSF_DEBUG
	PHY_DSSF(("post-sorted: "));
	for (j = 0; j < LCN20PHY_NOISEEST_NUM_COLLECT; j++) {
		PHY_DSSF(("%d ", noise_powers[j]));
	}
	PHY_DSSF(("\n"));
#endif // endif
	temp = 0;
	for (j = 1; j < (LCN20PHY_NOISEEST_NUM_COLLECT-1); j++)
		temp += noise_powers[j];

	temp += 1 << (LCN20PHY_NOISEEST_SAMPSLOG2+LCN20PHY_NOISEEST_COLLECTLOG2-1);
	temp >>= LCN20PHY_NOISEEST_SAMPSLOG2 + LCN20PHY_NOISEEST_COLLECTLOG2;
	temp *= 256;
	cmplx_pwr[0] = temp;
	total_gain[0] = rfgain + ((3*(dvga1-5)) << LCN20PHY_NOISEDB_SCALE);
	wlc_phy_noise_calc_fine_resln(pi, cmplx_pwr, crsminpwr, n_dbm_total, 0, total_gain);

	temp = ABS(ref_dbm_total - n_dbm_total[0]);
	if (temp > (6 << LCN20PHY_NOISEDB_SCALE)) {
		PHY_TRACE(("Delta bigger than threshold, return default value; %s \n",
		__FUNCTION__));
		return ref_dbm_total;
	} else {
		*noise_pwr_dB = n_dbm_total[0] + total_gain[0] - LCN20PHY_NOISE_PWR_TO_DBM;

#ifdef DSSF_DEBUG
		PHY_DSSF(("rfgain:%d, noise dBm:%d, dB:%d\n", rfgain,
			n_dbm_total[0], *noise_pwr_dB));
#endif // endif
		return n_dbm_total[0];
	}
}

#if defined(WLTEST)
static void
wlc_lcn20phy_get_gain_settings(phy_info_t *pi, int8 gainidx, uint8 *pslna_byp, uint8 *pslna_rout,
	uint8 *pslna_gain, uint8 *plna2_gain, uint8 *ptia_gain, uint8 *pdvga1_gain)
{
	uint32 gaintbl_val, gaintbl_idx;
	uint16 gainTblOffset, initgain_dB;

	if (gainidx == -1) {
		gainTblOffset = PHY_REG_READ(pi, LCN20PHY, wl_gain_tbl_offset, wl_gain_tbl_offset);
		initgain_dB = PHY_REG_READ(pi, LCN20PHY, agcControl12, crs_gain_high_gain_db_40mhz);
		gainidx = ((initgain_dB + gainTblOffset) * 85) >> 8;
	} else if (gainidx > MAX_WHOLE_GAINTBL_INDEX) {
		PHY_INFORM(("wl%d: %s: gainidx is higher than 75, cap it to 75\n",
		pi->sh->unit, __FUNCTION__));
		gainidx = MAX_WHOLE_GAINTBL_INDEX;
	}

	/* access gainidxtbl/gaintbl */
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAINIDX,
		&gaintbl_val, 1, 32, gainidx);
	*pslna_byp = (gaintbl_val & LCN20PHY_GainIdxTbl_slna_byp_MASK) >>
		LCN20PHY_GainIdxTbl_slna_byp_SHIFT;
	*pslna_rout = (gaintbl_val & LCN20PHY_GainIdxTbl_slna_rout_MASK) >>
		LCN20PHY_GainIdxTbl_slna_rout_SHIFT;
	*pslna_gain = (gaintbl_val & LCN20PHY_GainIdxTbl_slna_gain_MASK) >>
		LCN20PHY_GainIdxTbl_slna_gain_SHIFT;
	gaintbl_idx = (gaintbl_val & LCN20PHY_GainIdxTbl_gaintbl_idx_MASK) >>
		LCN20PHY_GainIdxTbl_gaintbl_idx_SHIFT;

	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAIN, &gaintbl_val, 1, 32, gaintbl_idx);
	*plna2_gain = (gaintbl_val & LCN20PHY_GainTbl_lna2_gain_MASK) >>
		LCN20PHY_GainTbl_lna2_gain_SHIFT;
	*ptia_gain = (gaintbl_val & LCN20PHY_GainTbl_tia_gain_MASK) >>
		LCN20PHY_GainTbl_tia_gain_SHIFT;
	*pdvga1_gain = (gaintbl_val & LCN20PHY_GainTbl_dvga1_gain_MASK) >>
		LCN20PHY_GainTbl_dvga1_gain_SHIFT;

}

void
wlc_lcn20phy_rx_power(phy_info_t *pi, uint16 num_samps, uint8 wait_time,
	uint8 wait_for_crs, phy_iq_est_t* est, int16 *tot_gain)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_lna_params_t *lna_params = pi->u.pi_lcn20phy->lna_params;
	uint16 SAVE_agc_fsm_en, savesslpnCalibClkEnCtrl;
	uint16 SAVE_DVGA2, SAVE_RxSdFeConfig1, SAVE_agcControl12;
	uint16 SAVE_ACI_Detect_CTRL1, SAVE_ACI_Detect_CTRL2;
	uint16 SAVE_swctrlOvr_val, SAVE_swctrlOvr;

	phy_iq_est_t est1;
	uint16 reduced_num_samps_log2 = 10;
	uint16 reduced_num_samps;
	uint32 meansq_max = 600;
	uint32 meansq_min = 150;
	uint32 i_meansq, q_meansq;
	int16 rxpath_gain;
	uint8 found_ideal;
	uint8 slna_byp, slna_rout, slna_gain, lna2_gain, tia_gain, dvga1_gain;
	uint8 do_max = MAX_NOMAL_GAINTBL_INDEX;
	uint8 do_counter;
	bool suspend;
	int16 init_gain_index, delta_index;
	uint8 acitbl_flg;
	int elna_bypass_delta;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	int16 rssi_corr_gain_delta;
	int8 rssi_delta_bgn;
	int8 rssi_delta_end;
	uint8 channel_seg;
	int8 delta_slope[LCN20PHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_delta_2g[LCN20PHY_GAIN_DELTA_2G_PARAMS];
	int8 delta_idx;

	suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	wlc_btcx_override_enable(pi);

	bzero(delta_slope, sizeof(delta_slope));
	channel_seg = 0;
	while ((channel > pi_lcn20->rssi_cal_channel[channel_seg]) &&
		(channel_seg < LCN20PHY_GROUPS-1))
		channel_seg++;

	if (channel <= pi_lcn20->rssi_cal_channel[channel_seg]) {
		if (channel == pi_lcn20->rssi_cal_channel[channel_seg]) {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++)
				rssi_delta_2g[delta_idx] = *(pi_lcn20->rssi_corr_gain_delta_2g_sub
					+ (channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS)+delta_idx);
		} else if (channel < pi_lcn20->rssi_cal_channel[channel_seg]) {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++) {
				rssi_delta_bgn = *(pi_lcn20->rssi_corr_gain_delta_2g_sub +
					((channel_seg-1)*LCN20PHY_GAIN_DELTA_2G_PARAMS) +
					delta_idx);
				rssi_delta_end = *(pi_lcn20->rssi_corr_gain_delta_2g_sub +
					channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS + delta_idx);
				delta_slope[delta_idx] =  (int16)(rssi_delta_end -
					rssi_delta_bgn) *
					(channel-pi_lcn20->rssi_cal_channel[channel_seg-1]) /
					(pi_lcn20->rssi_cal_channel[channel_seg]
					- pi_lcn20->rssi_cal_channel[channel_seg-1]);
				rssi_delta_2g[delta_idx] = rssi_delta_bgn +
					delta_slope[delta_idx];
			}
		}
	} else {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++)
				rssi_delta_2g[delta_idx] = *(pi_lcn20->rssi_corr_gain_delta_2g_sub
					+ (channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS)+delta_idx);
	}

	PHY_INFORM(("channel slope comp p_rssi_delta_2g:%d %d %d, channel_seg:%d,"
		"delta_slope:%d, %d, %d\n", rssi_delta_2g[0], rssi_delta_2g[1],
		rssi_delta_2g[2], channel_seg, delta_slope[0],
		delta_slope[1], delta_slope[2]));

	/* save register */
	SAVE_agc_fsm_en = PHY_REG_READ(pi, LCN20PHY, agcControl4, c_agc_fsm_en);
	savesslpnCalibClkEnCtrl = READ_LCN20PHYREG(pi, sslpnCalibClkEnCtrl);
	SAVE_RxSdFeConfig1 = READ_LCN20PHYREG(pi, RxSdFeConfig1);
	SAVE_DVGA2 = READ_LCN20PHYREG(pi, phyreg2dvga2);
	SAVE_agcControl12 = READ_LCN20PHYREG(pi, agcControl12);
	SAVE_ACI_Detect_CTRL1 = READ_LCN20PHYREG(pi, ACI_Detect_CTRL1);
	SAVE_ACI_Detect_CTRL2 = READ_LCN20PHYREG(pi, ACI_Detect_CTRL2);
	SAVE_swctrlOvr_val = READ_LCN20PHYREG(pi, swctrlOvr_val);
	SAVE_swctrlOvr = READ_LCN20PHYREG(pi, swctrlOvr);

	/* enable clock for RX IQ Cal Block */
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, iqEstClkEn, 1);
	PHY_REG_MOD(pi, LCN20PHY, sslpnCalibClkEnCtrl, forceiqEstclkOn, 1);

	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);

	reduced_num_samps = 1 << reduced_num_samps_log2;
	i_meansq = 0;
	q_meansq = 0;
	do_counter = 0;
	found_ideal = 0;
	rssi_corr_gain_delta = 0;
	acitbl_flg = 0;
	dvga1_gain = 0;

	/* check if gain index is fix in command line
	 * if it is fixed, use this gain and skip rx gain control
	 * if it is not fixed, do rx gain control
	 */
	if (pi_lcn20->rxpath_index <= MAX_WHOLE_GAINTBL_INDEX) {
		if (pi_lcn20->rxpath_elna == 1) {
			PHY_REG_MOD(pi, LCN20PHY, swctrlOvr_val,
				swCtrl_ovr_val, (uint8)pi_lcn20->elna_bypsss_val);
			PHY_REG_MOD(pi, LCN20PHY, swctrlOvr,
				swCtrl_ovr, 0xff);
		}

		init_gain_index = ((lna_params->wl_gain_tbl_offset+
			lna_params->crs_gain_high_gain_db_40mhz)*85)>>8;

		if (pi_lcn20->rxpath_index <= MAX_NOMAL_GAINTBL_INDEX) {
			/* normal gain index */
			delta_index = init_gain_index - (int16)(pi_lcn20->rxpath_index);
			acitbl_flg = 0;

			/* force non-ACI mode */
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_sel, 0);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2,
				aci_mitigation_sw_enable, 1);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_detect_enable, 0);
		} else {
			acitbl_flg = 1;
			/* ACI gain index */
			delta_index = init_gain_index - (int16)(pi_lcn20->rxpath_index)
				+ MAX_NOMAL_GAINTBL_INDEX + 1;

			/* force ACI mode */
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_sel, 1);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL2,
				aci_mitigation_sw_enable, 1);
			PHY_REG_MOD(pi, LCN20PHY, ACI_Detect_CTRL1,
				aci_detect_enable, 0);
		}
		rxpath_gain = lna_params->crs_gain_high_gain_db_40mhz - delta_index*3;
		PHY_REG_MOD(pi, LCN20PHY, agcControl12,
			crs_gain_high_gain_db_40mhz, rxpath_gain);
		PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
		OSL_DELAY(1);
		PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
		OSL_DELAY(1);
		found_ideal = 1;

		if (acitbl_flg) {
			if (pi_lcn20->rxpath_index <= LCN20PHY_ACITBL_GAINIDX_BOUNDRY2)
				rssi_corr_gain_delta =
					rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX] -
					LCN20PHY_ACITBL_GAIN_OFFSET2 +
					(pi->u.pi_lcn20phy->rssicorr_aci << LCN20_QDB_SHIFT);
			else if (pi_lcn20->rxpath_index <= LCN20PHY_ACITBL_GAINIDX_BOUNDRY1)
				rssi_corr_gain_delta =
					rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX] -
					LCN20PHY_ACITBL_GAIN_OFFSET1 +
					(pi->u.pi_lcn20phy->rssicorr_aci << LCN20_QDB_SHIFT);
			else
				rssi_corr_gain_delta =
					rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX] +
					(pi->u.pi_lcn20phy->rssicorr_aci << LCN20_QDB_SHIFT);
		} else {
			if (pi_lcn20->rxpath_index >= LCN20PHY_NORTBL_GAINIDX_BOUNDRY1)
				rssi_corr_gain_delta =
					rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT0_IDX];
			else
				rssi_corr_gain_delta =
					rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX];
		}

		if (pi_lcn20->rxpath_elna == 1)
		{
			elna_bypass_delta = rssi_delta_2g[LCN20PHY_RSSI_DELTA_ELNABYP_IDX] -
				rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX];
			elna_bypass_delta += ((pi_lcn20->tr_isolation * LCN20PHY_GAIN_STEP)
				<< LCN20_QDB_SHIFT);
		}
		else
			elna_bypass_delta = 0;

		rssi_corr_gain_delta = rssi_corr_gain_delta + elna_bypass_delta;
		PHY_INFORM(("%s: rxpath_index:%d, elna:%d, rxpath_gain:%d, "
			"rssi_corr_gain_delta:%d, acitbl_flg:%d\n",
			__FUNCTION__, pi_lcn20->rxpath_index, pi_lcn20->rxpath_elna,
			rxpath_gain, rssi_corr_gain_delta, acitbl_flg));
	} else {
		rxpath_gain = PHY_REG_READ(pi, LCN20PHY, agcControl12,
			crs_gain_high_gain_db_40mhz);
		wlc_lcn20phy_get_gain_settings(pi, MAX_NOMAL_GAINTBL_INDEX, &slna_byp,
			&slna_rout, &slna_gain, &lna2_gain, &tia_gain, &dvga1_gain);

		PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
		OSL_DELAY(1);
		PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
		OSL_DELAY(1);

		/* rx gain control */
		while ((do_counter < do_max) && (rxpath_gain > NOISE_MEAS_MIN_GAIN_DB_NEW) &&
			(found_ideal == 0)) {

			/* estimate digital power using rx_iq_est */
			wlc_lcn20phy_rx_iq_est(pi, &est1, reduced_num_samps, wait_time, 0, FALSE);
			OSL_DELAY(1);

			i_meansq = (est1.i_pwr + (reduced_num_samps >> 1)) >>
				reduced_num_samps_log2;
			q_meansq = (est1.q_pwr + (reduced_num_samps >> 1)) >>
				reduced_num_samps_log2;

			PHY_INFORM(("rx gain control est->i_pwr:%d, est->q_pwr:%d, "
				"i_meansq:%d, q_meansq:%d, rxpath_gain:%d, "
				"dvga1_gain:%d, found_ideal:%d\n",
				est1.i_pwr, est1.q_pwr, i_meansq, q_meansq, rxpath_gain,
				dvga1_gain, found_ideal));

			if ((i_meansq > meansq_max) || (q_meansq > meansq_max)) {
				if ((rxpath_gain > NOISE_MEAS_MIN_GAIN_DB_NEW) &&
						(rxpath_gain <= lna_params->max_gain_of_tbl)) {
					rxpath_gain = rxpath_gain - LCN20PHY_GAIN_STEP;
					PHY_REG_MOD(pi, LCN20PHY, agcControl12,
						crs_gain_high_gain_db_40mhz, rxpath_gain);
					PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1,
						farrow_rshift_force, 0);
					PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
					OSL_DELAY(1);
					PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
					OSL_DELAY(1);
				} else if (rxpath_gain > lna_params->max_gain_of_tbl) {
					dvga1_gain = dvga1_gain - 1;
					rxpath_gain = rxpath_gain - LCN20PHY_GAIN_STEP;
					PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1,
						rx_farrow_rshift_0, dvga1_gain);
					PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1,
						farrow_rshift_force, 1);
				}
			} else if ((i_meansq < meansq_min) && (q_meansq < meansq_min)) {
				if (rxpath_gain < lna_params->max_gain_of_tbl) {
					rxpath_gain = rxpath_gain + LCN20PHY_GAIN_STEP;
					PHY_REG_MOD(pi, LCN20PHY, agcControl12,
						crs_gain_high_gain_db_40mhz, rxpath_gain);
					PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
					OSL_DELAY(1);
					PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
					OSL_DELAY(1);
				} else {
					if (dvga1_gain >= NOISE_MEAS_MAX_DVGA_GAIN)
						break;
					dvga1_gain = dvga1_gain + 1;
					rxpath_gain = rxpath_gain + LCN20PHY_GAIN_STEP;
					PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1,
						rx_farrow_rshift_0, dvga1_gain);
					PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1,
						farrow_rshift_force, 1);
				}
			} else {
				found_ideal = 1;
			}

			do_counter ++;
		}
	}

	if (found_ideal) {
		/* estimate digital power using rx_iq_est */
		wlc_lcn20phy_rx_iq_est(pi, est, num_samps, wait_time, 0, FALSE);
		OSL_DELAY(1);

		PHY_INFORM(("after rx gain control, est->i_pwr:%d, est->q_pwr:%d, "
			"rxpath_gain:%d, dvga1_gain:%d, found_ideal:%d\n",
			est->i_pwr, est->q_pwr, rxpath_gain, dvga1_gain, found_ideal));

		tot_gain[0] = (rxpath_gain - LCN20PHY_NORTBL_OFFSET_NEW +
			pi_lcn20->gainadj) << LCN20_QDB_SHIFT;

		/* Temp sense based correction */
		rssi_corr_gain_delta += wlc_lcn20phy_rssi_tempcorr(pi, 0);

		tot_gain[0] = tot_gain[0] - rssi_corr_gain_delta;
		PHY_INFORM(("result est->i_pwr:%d, est->q_pwr:%d, rxpath_gain:%d, tot_gain:%d\n",
			est->i_pwr, est->q_pwr, rxpath_gain, tot_gain[0]));
	}
	else {
		PHY_ERROR(("wl%d: %s: Final IQ Estimation Failed\n",
		pi->sh->unit, __FUNCTION__));
		est->i_pwr = 0;
		est->q_pwr = 0;
		tot_gain[0] = NOISE_MEAS_INVALID_TOT_GAIN_DB;
	}

	WRITE_LCN20PHYREG(pi, swctrlOvr, SAVE_swctrlOvr);
	WRITE_LCN20PHYREG(pi, swctrlOvr_val, SAVE_swctrlOvr_val);
	WRITE_LCN20PHYREG(pi, ACI_Detect_CTRL2, SAVE_ACI_Detect_CTRL2);
	WRITE_LCN20PHYREG(pi, ACI_Detect_CTRL1, SAVE_ACI_Detect_CTRL1);
	WRITE_LCN20PHYREG(pi, agcControl12, SAVE_agcControl12);
	WRITE_LCN20PHYREG(pi, phyreg2dvga2, SAVE_DVGA2);
	WRITE_LCN20PHYREG(pi, RxSdFeConfig1, SAVE_RxSdFeConfig1);
	WRITE_LCN20PHYREG(pi, sslpnCalibClkEnCtrl, savesslpnCalibClkEnCtrl);

	/* Remove HG-lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, SAVE_agc_fsm_en);

	wlc_phy_btcx_override_disable(pi);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

}
#endif /* defined(WLTEST) */

static const int16 *
wlc_lcn20phy_get_spur_offset(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	const int16 *spur_offset = NULL;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	uint8 spurmode;

	if (pi_lcn20->spurmode_override)
		spurmode = pi_lcn20->forced_spurmode;
	else
		spurmode = pi_lcn20->spurmode;

#ifdef DSSF_DEBUG
	PHY_DSSF(("\nch %d spurmode %d\n", channel, spurmode));
#endif // endif

	if (pi->xtalfreq == XTAL_FREQ_37P4MHZ) {
		if (spurmode == LCN20PHY_SPURMODE_FVCO_980)
			spur_offset = spur_offset_37p4MHz_vco980[channel - 1];
		else if (spurmode == LCN20PHY_SPURMODE_FVCO_326P4)
			spur_offset = spur_offset_37p4MHz_vco326p4[channel - 1];
		else
			spur_offset = spur_offset_37p4MHz[channel - 1];
	} else if (pi->xtalfreq == XTAL_FREQ_26P0MHZ) {
		if (spurmode == LCN20PHY_SPURMODE_FVCO_980)
			spur_offset = spur_offset_26MHz_vco980[channel - 1];
		else if (spurmode == LCN20PHY_SPURMODE_FVCO_326P4)
			spur_offset = spur_offset_26MHz_vco326p4[channel - 1];
		else
			spur_offset = spur_offset_26MHz[channel - 1];
	} else if (pi->xtalfreq == XTAL_FREQ_19P2MHZ) {
		if (spurmode == LCN20PHY_SPURMODE_FVCO_980)
			spur_offset = spur_offset_19p2MHz_vco980[channel - 1];
		else if (spurmode == LCN20PHY_SPURMODE_FVCO_326P4)
			spur_offset = spur_offset_19p2MHz_vco326p4[channel - 1];
		else
			spur_offset = spur_offset_19p2MHz[channel - 1];
	} else {
		PHY_ERROR(("FATAL Error: Incorrect xtalfreq \n"));
		ASSERT(FALSE);
	}

	return spur_offset;

}

static void
wlc_lcn20phy_dssf_cal(phy_info_t *pi)
{
	bool enableDSSF = FALSE;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint8 j, k = 0;
	uint16 spurListSize = 0;
	const int16 *spur_offset = NULL;
	int16 delta_dB;
	rxspur_params_t spur_list[LCN20PHY_DSSF_MAX_NOTCHES] = {{0, 0, 0}, {0, 0, 0}};
	phytbl_info_t tab;
	uint16 minsigsqr_base = 0;

	PHY_TRACE(("%s:\n", __FUNCTION__));

	/* get spur offset wrt DC from table */
	spur_offset = wlc_lcn20phy_get_spur_offset(pi);

	wlc_lcn20phy_deaf_mode(pi, TRUE);

	wlc_lcn20phy_dssf_cal_est_noise_chk_spur(pi, spur_offset,
		&delta_dB, &enableDSSF, spur_list, &spurListSize);

#ifdef DSSF_DEBUG
	PHY_DSSF(("DSSF decision boundaries : %d %d %d\n", pi_lcn20->dssf_thresh[0],
		pi_lcn20->dssf_thresh[1], pi_lcn20->dssf_thresh[2]));
#endif // endif

	/*
	 * Always clear any existing settings.
	 */

	/* Populate minsigsqr table with the same default value. */
	tab.tbl_id = LCN20PHY_TBL_ID_MINSIGSQR;
	tab.tbl_width = 16;	/* 10 bit wide	*/
	tab.tbl_ptr = &minsigsqr_base;
	tab.tbl_len = 1;
	tab.tbl_offset = 0; /* DC */
	wlc_lcn20phy_read_table(pi, &tab);

	for (j = 0; j < 64; j++)
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_MINSIGSQR,
			&minsigsqr_base,  1, 16, j);

	/* Clear DSSF control settings. */
	wlc_lcn20phy_set_dssf_mode(pi, 0, 0, 0, 0, 0, 0);

	/*
	 * Spur suppression configurations.
	 */
	if (enableDSSF) {
		/* Configure notch freq. and gain thresholds for each 'significant' spur. */
		for (j = 0; j < spurListSize; j++) {
			int16 freqKHz = spur_list[j].freqKHz;
			bool deweight = TRUE;

			wlc_lcn20phy_set_dssf_freq(pi, (j+1), (freqKHz*1000));

			delta_dB = spur_list[j].delta_dB;
			if (delta_dB < pi_lcn20->dssf_thresh[1])
				wlc_lcn20phy_set_dssf_gainth(pi, (j+1), &dssf_gainth_set[0][0]);
			else if (delta_dB < pi_lcn20->dssf_thresh[2])
				wlc_lcn20phy_set_dssf_gainth(pi, (j+1), &dssf_gainth_set[1][0]);
			else
				wlc_lcn20phy_set_dssf_gainth(pi, (j+1), &dssf_gainth_set[2][0]);

			if (deweight) {
				int8 sc;
				int16 dist;
				int32 abs_freq = ABS(freqKHz)*4;

				for (k = 0; k < 32; k++) {
					if ((abs_freq - 1250*k) <= 625)
						break;
				}
				ASSERT(k != 32); /* shouldn't reach the end of loop */

				if (freqKHz > 0)
					sc = k;
				else
					sc = -k;

				dist = freqKHz*2 - sc*625;

				if (ABS(sc) >= 3) {
					for (k = 0; k < LCN20PHY_DSSF_SC_LUTSIZE; k++) {
						if (sc_deweight[k][0] == ABS(dist)) {
							int16 w_1 = sc_deweight[k][1];
							int16 w_2 = sc_deweight[k][2];
							int8 sc_1, sc_2;

							if (dist < 0) {
								/* nearest sc is on the right */
								/* of spur */
								if (sc > 0) {
									sc_1 = sc;
									sc_2 = sc-1;
								} else {
									sc_1 = 64+sc;
									sc_2 = 63+sc;
								}
							} else {
								/* nearest sc is on the left */
								/* of spur */
								if (sc > 0) {
									sc_1 = sc;
									sc_2 = sc+1;
								} else {
									sc_1 = 64+sc;
									sc_2 = 65+sc;
								}
							}

							tab.tbl_offset = sc_1;
							wlc_lcn20phy_read_table(pi, &tab);
							minsigsqr_base += w_1;
							wlc_lcn20phy_write_table(pi, &tab);

							tab.tbl_offset = sc_2;
							wlc_lcn20phy_read_table(pi, &tab);
							minsigsqr_base += w_2;
							wlc_lcn20phy_write_table(pi, &tab);
#ifdef DSSF_DEBUG
							PHY_DSSF(("De-weight sc %d:+%d, %d:+%d\n",
								sc_1, w_1, sc_2, w_2));
#endif // endif
							break;
						}
					}
				}
			}
		}

		/* Enable DSSF */
		if (spurListSize == 1)
			wlc_lcn20phy_set_dssf_mode(pi, 5, 1, 0, 0, 0, 0);
		else
			wlc_lcn20phy_set_dssf_mode(pi, 5, 1, 1, 0, 0, 0);
	}

	wlc_lcn20phy_deaf_mode(pi, FALSE);
}

static uint16 wlc_lcn20phy_measure_rxspur(phy_info_t *pi, int16 freqKHz, int16 *spur_pwr_dB,
	uint16 ampl, uint8 count_log2, uint8 dvga1_gain)
{
	lcn20phy_lna_params_t *lna_params = pi->u.pi_lcn20phy->lna_params;
	uint16 ripple_bin = 0, temp, crsminpwr[PHY_CORE_MAX];
	uint8 num_avg = (1 << (count_log2-1));
	uint32 bin_sqr[PHY_CORE_MAX];
	int16 j, tempI, tempQ, pwrant[PHY_CORE_MAX], total_gain[PHY_CORE_MAX];

	measure_rxspur_savestate_params_t save_state;

	wlc_lcn20phy_measure_rxspur_save_state(pi, &save_state);

	/*
	*  Power-down Tx and power-up Rx, call wrapper for wlc_lcn20phy_rx_pu
	*/
	wlc_lcn20phy_rx_pwrup(pi, 1);

	/* Power down Tx mixer, cascode etc. */
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrftxpu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 1);

	/* Power down loopback paths */
	MOD_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, wl_pa2g_tssi_ctrl_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, wl_ovr_pa2g_tssi_ctrl_pu, 1);

	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_PU_iqcal, 0);
	MOD_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, wl_ovr_iqcal_PU_iqcal, 1);

	MOD_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, wl_iqcal_PU_tssi, 0);
	MOD_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, wl_ovr_iqcal_PU_tssi, 1);

	MOD_RADIO_REG_20692(pi, WL_TESTBUF_CFG1, 0, wl_testbuf_PU, 0);
	MOD_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, wl_ovr_testbuf_PU, 1);

	MOD_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, wl_auxpga_i_pu, 0);
	MOD_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, wl_ovr_auxpga_i_pu, 1);

	/* Set to init gains through overrides */
	wlc_lcn20phy_apply_gainidx_settings(pi, (uint8) lna_params->meas_rxspur_gaintblidx);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, dvga1_gain);

	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1);

	/* play tone in iqmode and skip bedeaf */
	wlc_lcn20phy_start_tx_tone(pi, (freqKHz*1000), ampl, 1, FALSE, 0);

	PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, iqloCalCmd, 0);
	PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, iqloCalDFTCmd, 0);
	PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, cal_type, 2);
	PHY_REG_MOD(pi, LCN20PHY, iqloCalCmdGctl, iqlo_cal_en, 1);
	WRITE_LCN20PHYREG(pi, iqloCalCmdNnum, 0x2922);

	bin_sqr[0] = 0;

	for (j = 0; j < num_avg; j++) {
		/* Conduct measurements on I-ch */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 1);
		PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, iqloCalDFTCmd, 1);

		/* wait for 100msec */
		SPINWAIT(((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) != 0),
			wlc_lcn20phy_get_spinwait_txiqlocal_usec());
		ASSERT((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) == 0);

		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&temp, 1, 16, LCN20PHY_TXIQLOCAL_RIPPLE_BIN_OFFSET);

		ripple_bin += temp;

		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tempI, 1, 16, LCN20PHY_TXIQLOCAL_CORR_I_OFFSET);
		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tempQ, 1, 16, LCN20PHY_TXIQLOCAL_CORR_Q_OFFSET);

		bin_sqr[0] += tempI*tempI + tempQ*tempQ;

		/* Conduct measurements on Q-ch */
		PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, 0);
		PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, iqloCalDFTCmd, 1);

		/* wait for 100msec */
		SPINWAIT(((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) != 0),
			wlc_lcn20phy_get_spinwait_txiqlocal_usec());
		ASSERT((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) == 0);

		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&temp, 1, 16, LCN20PHY_TXIQLOCAL_RIPPLE_BIN_OFFSET);

		ripple_bin += temp;

		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tempI, 1, 16, LCN20PHY_TXIQLOCAL_CORR_I_OFFSET);
		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tempQ, 1, 16, LCN20PHY_TXIQLOCAL_CORR_Q_OFFSET);

		bin_sqr[0] += tempI*tempI + tempQ*tempQ;
	}

	/* use the following call to provide the dB conversion needed */
	bin_sqr[0] = (bin_sqr[0] + (1<<(count_log2-1))) >> count_log2;
	total_gain[0] = 0;
	wlc_phy_noise_calc_fine_resln(pi, bin_sqr, crsminpwr, pwrant, 0, total_gain);
	*spur_pwr_dB = pwrant[0] + total_gain[0] - LCN20PHY_NOISE_PWR_TO_DBM;
	ripple_bin = ripple_bin >> count_log2;
#ifdef DSSF_DEBUG
	PHY_DSSF(("=> ripple_bin:%d, spur_pwr_dB:%d\n", ripple_bin, *spur_pwr_dB));
#endif // endif
	/*
	*  Cleanups
	*/
	wlc_lcn20phy_stop_tx_tone(pi);

	/* Clear iDAC override */
	wlc_lcn20phy_dccal_ovr_idac(pi, 0, 0, 0);

	wlc_lcn20phy_rx_gain_override_enable_wrapper(pi, FALSE);

	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 0);

	/* Call the wrapper of wlc_lcn20phy_rx_pu(pi, 0) to enable eTR switch overrides */
	wlc_lcn20phy_rx_pwrup(pi, 0);

	wlc_lcn20phy_measure_rxspur_restore_state(pi, &save_state);

	return ripple_bin;
}

static void
wlc_lcn20phy_set_dssf_mode(phy_info_t *pi, uint8 mode, bool enable1, bool enable2,
	uint8 depth1, uint8 depth2, bool hgforced)
{
	PHY_REG_MOD(pi, LCN20PHY, DSSF_C_CTRL, mode, mode);
	PHY_REG_MOD(pi, LCN20PHY, DSSF_C_CTRL, dssf_highGainDepthForced, hgforced);

	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s1, enable1);
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, enabled_s2, enable2);

	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, idepth_s1, depth1);
	PHY_REG_MOD(pi, LCN20PHY, DSSF_control_0, idepth_s2, depth2);
#ifdef DSSF_DEBUG
	PHY_DSSF(("DSSF mode=%d hgforced=%d en1=%d en2=%d depth1=%d depth2=%d\n", mode,
		hgforced, enable1, enable2, depth1, depth2));
#endif // endif
}

static void
wlc_lcn20phy_set_dssf_gainth(phy_info_t *pi, uint8 notchIdx, const uint8 *gainth)
{
	if (notchIdx == 1) {
		/* Gain Thresholds Set 1 (used by non-PACKET_RX states) */
		/* 1st notch */
		WRITE_LCN20PHYREG(pi, DSSF_gain_th0_s10, gainth[0]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th1_s10, gainth[1]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th2_s10, gainth[2]);

		/* Gain Thresholds Set 2 (used in PACKET_RX state) */
		/* 1st notch */
		WRITE_LCN20PHYREG(pi, DSSF_gain_th20_s10, gainth[3]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th21_s10, gainth[4]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th22_s10, gainth[5]);
	} else if (notchIdx == 2) {
		/* 2nd notch (Set 1) */
		WRITE_LCN20PHYREG(pi, DSSF_gain_th0_s20, gainth[0]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th1_s20, gainth[1]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th2_s20, gainth[2]);

		/* 2nd notch (Set 2) */
		WRITE_LCN20PHYREG(pi, DSSF_gain_th20_s20, gainth[3]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th21_s20, gainth[4]);
		WRITE_LCN20PHYREG(pi, DSSF_gain_th22_s20, gainth[5]);
	}
#ifdef DSSF_DEBUG
	PHY_DSSF(("notch%d : %d %d %d %d %d %d\n", notchIdx, gainth[0], gainth[1], gainth[2],
		gainth[3], gainth[4], gainth[5]));
#endif // endif
}

static void
wlc_lcn20phy_set_dssf_freq(phy_info_t *pi, uint8 notchIdx, int32 f_Hz)
{
	fixed theta = 0, rot = 0;
	uint16 i_samp, q_samp;
	math_cint32 tone_samp;

	rot = FIXED((ABS(f_Hz) * 36)/(20 * 1000)) / 100; /* 2*pi*f/20/1000  Note: f in Hz */
	theta = 0;

	if (f_Hz > 0)
		theta += rot;
	else
		theta -= rot;

	math_cmplx_cordic(theta, &tone_samp);

	i_samp = (uint16)(FLOAT(tone_samp.i * 4095) & 0x1fff);
	q_samp = (uint16)(FLOAT(tone_samp.q * 4095) & 0x1fff);

	if (notchIdx == 1) {
		/* 1st notch */
		WRITE_LCN20PHYREG(pi, DSSF_exp_j_theta_i_s10, i_samp);
		WRITE_LCN20PHYREG(pi, DSSF_exp_i_theta_q_s10, q_samp);
	} else if (notchIdx == 2) {
		/* 2nd notch */
		WRITE_LCN20PHYREG(pi, DSSF_exp_j_theta_i_s20, i_samp);
		WRITE_LCN20PHYREG(pi, DSSF_exp_i_theta_q_s20, q_samp);
	}

#ifdef DSSF_DEBUG
	PHY_DSSF(("notch%d : %d %d\n", notchIdx, i_samp, q_samp));
#endif // endif
}

static void
wlc_lcn20phy_xtalpn_cal(phy_info_t *pi)
{
	const int16 *spur_offset = NULL;
	uint16 tmp_TxPwrCtrlCmd;
	uint8 tmp_agc_fsm_en, tmp_use_farrow_msb;
	uint32 bin_sqr, bin_min, bin_max;
	int16 tempI, tempQ;
	/* pn_list: ordered list of (n*4+p) values; first xtalpn_nv values are used in search
	 * (n,p) -> {0 0} {1 0} {0 1} {2 1} {1 2} {1 1} {0 2} {2 0}
	 *          {3 1} {1 3} {2 2} {3 0} {0 3} {3 3} {3 2} {2 3}
	 */
	const uint8 pn_list[16] = {0, 4, 1, 9, 6, 5, 2, 8, 13, 7, 10, 12, 3, 15, 14, 11};
	uint8 idx, idx_min, n_dft;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);

	/* get spur offset wrt DC from table */
	spur_offset = wlc_lcn20phy_get_spur_offset(pi);
	if (spur_offset[0] == 0)
		return;

	tmp_agc_fsm_en = PHY_REG_READ(pi, LCN20PHY, agcControl4, c_agc_fsm_en);
	tmp_use_farrow_msb = PHY_REG_READ(pi, LCN20PHY, PapdCalShifts, papd_cal_use_farrow_msb);

	/* discard 2lsbs of Farrow output and force DVGA1 to 5 */
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papd_cal_use_farrow_msb, 0);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, 5);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);

	/* Put AGC fsm in HG lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
	OSL_DELAY(1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
	OSL_DELAY(1);

	/* save power control command */
	tmp_TxPwrCtrlCmd = READ_LCN20PHYREG(pi, TxPwrCtrlCmd);
	WRITE_LCN20PHYREG(pi, TxPwrCtrlCmd, 0);

	/* Power-down Tx and power-up Rx, call wrapper for wlc_lcn20phy_rx_pu */
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, internalrftxpu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 1);
	wlc_lcn20phy_rx_pwrup(pi, 1);

	/* set eTR to TX mode */
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_rx_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_rx_pu_ovr_val, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, trsw_tx_pu_ovr, 1);
	PHY_REG_MOD(pi, LCN20PHY, RFOverrideVal0, trsw_tx_pu_ovr_val, 1);

	/* Enable iqadc_aux; required to enable correlation engine */
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1);
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1);

	/* play tone in iqmode and skip bedeaf */
	wlc_lcn20phy_start_tx_tone(pi, (spur_offset[0]*1000),
		LCN20PHY_MEAS_RXSPUR_AMPL, 1, FALSE, 0);

	/* DFT estimation parameters */
	WRITE_LCN20PHYREG(pi, iqloCalCmd, 0x0200);
	WRITE_LCN20PHYREG(pi, iqloCalCmdNnum, 0x2c22);
	WRITE_LCN20PHYREG(pi, iqloCalCmdGctl, 0x8000);

	/* initialize some loop parameters */
	bin_min = 0xffffffff;
	bin_max = 0;
	idx_min = 0;

	/* cal loop */
	for (idx = 0; idx < pi->u.pi_lcn20phy->xtalpn_nv; idx++) {
		/* set x_n, x_p */
		MOD_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0, wl_xtal_dty_adj, pn_list[idx]);
		OSL_DELAY(40);

		bin_sqr = 0;
		for (n_dft = 0; n_dft < (1 << LCN20PHY_XTALPN_CAL_LOG2NUMDFT); n_dft++) {

			/* swap i/q data input to the correlator, only i channel is used */
			PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlCmd, txPwrCtrl_swap_iq, (n_dft & 0x1));

			/* Trigger 1 DFT measurement */
			PHY_REG_MOD(pi, LCN20PHY, iqloCalCmd, iqloCalDFTCmd, 1);

			/* wait until correlation is completed */
			OSL_DELAY(110);
			ASSERT((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) == 0);

			/* read correlator I,Q values */
			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				&tempI, 1, 16, LCN20PHY_TXIQLOCAL_CORR_I_OFFSET);

			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				&tempQ, 1, 16, LCN20PHY_TXIQLOCAL_CORR_Q_OFFSET);

			/* spur power estimate accumulation with averaging */
			bin_sqr += ((int32)tempI * tempI + (int32)tempQ * tempQ)
				>> LCN20PHY_XTALPN_CAL_LOG2NUMDFT;
		}

		/* update bin_min */
		if (bin_min > bin_sqr) {
			bin_min = bin_sqr;
			idx_min = idx;
		}

		/* update bin_max */
		if (bin_sqr > bin_max)
			bin_max = bin_sqr;
	}

	/* save cal result if good */
	if ((bin_max-bin_min) > LCN20PHY_XTALPN_CAL_THRESH)
		pi->u.pi_lcn20phy->xtal_pn[channel-1] = pn_list[idx_min];
	MOD_RADIO_REG_20692(pi, WL_XTAL_CFG1, 0, wl_xtal_dty_adj,
		pi->u.pi_lcn20phy->xtal_pn[channel-1]);

	/* stop play-tone */
	wlc_lcn20phy_stop_tx_tone(pi);

	/* clear iqadc_aux_en_ovr */
	PHY_REG_MOD(pi, LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 0);

	/* remove overrides */
	wlc_lcn20phy_rx_pwrup(pi, 0);
	PHY_REG_MOD(pi, LCN20PHY, RFOverride0, internalrftxpu_ovr, 0);
	PHY_REG_MOD(pi, LCN20PHY, PapdCalShifts, papd_cal_use_farrow_msb, tmp_use_farrow_msb);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 0);

	/* restore power control command */
	WRITE_LCN20PHYREG(pi, TxPwrCtrlCmd, tmp_TxPwrCtrlCmd);

	/* Remove HG-lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, tmp_agc_fsm_en);
}

static void
wlc_phy_radio20692_tempsense_vbat_radio_setup(phy_info_t *pi, int mode)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_tempsense_vbat_radioregs_t *psave = pi_lcn20->tempsense_vbat_radioregs;
	uint16 *regdata;
	uint8 i;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(!psave->is_orig);
	psave->is_orig = TRUE;

	/* Get address of first radio register in the radioregs structure */
	regdata = &(psave->wl_temp_sens_ovr1);

	/* Save the current values of radio regs to restore later */
	for (i = 0; i < NUM_RADIOREG_SAVREST; i++, regdata++) {
		*regdata = phy_utils_read_radioreg(pi, temp_vbat_radioreg_savrest[i]);
	}

	PHY_REG_LIST_START
		/* Setup testbuf */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_GPABUF_OVR1(rev),
			RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_PU_MASK(rev),
			1 << RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_PU_SHIFT(rev))
		/* powerup testbuf */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TESTBUF_CFG1(rev),
			RF_20692_WL_TESTBUF_CFG1_wl_testbuf_PU_MASK(rev),
			1 << RF_20692_WL_TESTBUF_CFG1_wl_testbuf_PU_SHIFT(rev))
		/* Disable the output to gpaio */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TESTBUF_CFG1(rev),
			RF_20692_WL_TESTBUF_CFG1_wl_testbuf_GPIO_EN_MASK(rev),
			0 << RF_20692_WL_TESTBUF_CFG1_wl_testbuf_GPIO_EN_SHIFT(rev))
	PHY_REG_LIST_EXECUTE(pi);

	if (mode == TEMPSENSE) {
		/* PU tempsense */
		/* Override for tempsense pu */
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TEMP_SENS_OVR1(rev),
				RF_20692_WL_TEMP_SENS_OVR1_wl_ovr_tempsense_pu_MASK(rev),
				1 << RF_20692_WL_TEMP_SENS_OVR1_wl_ovr_tempsense_pu_SHIFT(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TEMPSENSE_CFG(rev),
				RF_20692_WL_TEMPSENSE_CFG_wl_tempsense_pu_MASK(rev),
				1 << RF_20692_WL_TEMPSENSE_CFG_wl_tempsense_pu_SHIFT(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_VBAT_MONITOR_OVR1(rev),
				RF_20692_WL_VBAT_MONITOR_OVR1_wl_ovr_vbat_monitor_pu_MASK(rev),
				1 << RF_20692_WL_VBAT_MONITOR_OVR1_wl_ovr_vbat_monitor_pu_SHIFT
				(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_VBAT_CFG(rev),
				RF_20692_WL_VBAT_CFG_wl_vbat_monitor_pu_MASK(rev),
				0 << RF_20692_WL_VBAT_CFG_wl_vbat_monitor_pu_SHIFT(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_GPABUF_OVR1(rev),
				RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_sel_test_port_MASK(rev),
				1 << RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_sel_test_port_SHIFT
				(rev))
			/* Select input 2 */
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TESTBUF_CFG1(rev),
				RF_20692_WL_TESTBUF_CFG1_wl_testbuf_sel_test_port_MASK(rev),
				1 << RF_20692_WL_TESTBUF_CFG1_wl_testbuf_sel_test_port_SHIFT(rev))
		PHY_REG_LIST_EXECUTE(pi);
	} else if (mode == VBATSENSE) {
		PHY_REG_LIST_START
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_VBAT_MONITOR_OVR1(rev),
				RF_20692_WL_VBAT_MONITOR_OVR1_wl_ovr_vbat_monitor_pu_MASK(rev),
				1 << RF_20692_WL_VBAT_MONITOR_OVR1_wl_ovr_vbat_monitor_pu_SHIFT
				(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_VBAT_CFG(rev),
				RF_20692_WL_VBAT_CFG_wl_vbat_monitor_pu_MASK(rev),
				1 << RF_20692_WL_VBAT_CFG_wl_vbat_monitor_pu_SHIFT(rev))
			/* O	verride for tempsense pu */
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TEMP_SENS_OVR1(rev),
				RF_20692_WL_TEMP_SENS_OVR1_wl_ovr_tempsense_pu_MASK(rev),
				1 << RF_20692_WL_TEMP_SENS_OVR1_wl_ovr_tempsense_pu_SHIFT(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TEMPSENSE_CFG(rev),
				RF_20692_WL_TEMPSENSE_CFG_wl_tempsense_pu_MASK(rev),
				0 << RF_20692_WL_TEMPSENSE_CFG_wl_tempsense_pu_SHIFT(rev))
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_GPABUF_OVR1(rev),
				RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_sel_test_port_MASK(rev),
				1 << RF_20692_WL_GPABUF_OVR1_wl_ovr_testbuf_sel_test_port_SHIFT
				(rev))
			/* Select input 4 */
			RADIO_REG_MOD_ENTRY(RF0_20692_WL_TESTBUF_CFG1(rev),
				RF_20692_WL_TESTBUF_CFG1_wl_testbuf_sel_test_port_MASK(rev),
				3 << RF_20692_WL_TESTBUF_CFG1_wl_testbuf_sel_test_port_SHIFT(rev))
		PHY_REG_LIST_EXECUTE(pi);
	}

	PHY_REG_LIST_START
		/* Setup AuxPGA */
		/* powerup auxpga */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUX_RXPGA_OVR1(rev),
			RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_pu_MASK(rev),
			1 << RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUXPGA_CFG1(rev),
			RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_pu_MASK(rev),
			1 << RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUX_RXPGA_OVR1(rev),
			RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_gain_MASK(rev),
			1 << RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_gain_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUXPGA_CFG1(rev),
			RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_sel_gain_MASK(rev),
			3 << RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_sel_gain_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUX_RXPGA_OVR1(rev),
			RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_vmid_MASK(rev),
			1 << RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_vmid_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUXPGA_VMID(rev),
			RF_20692_WL_AUXPGA_VMID_wl_auxpga_i_sel_vmid_MASK(rev),
			0xa4 << RF_20692_WL_AUXPGA_VMID_wl_auxpga_i_sel_vmid_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUX_RXPGA_OVR1(rev),
			RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_input_MASK(rev),
			1 << RF_20692_WL_AUX_RXPGA_OVR1_wl_ovr_auxpga_i_sel_input_SHIFT(rev))
		/* Setup auxpga for tempsense */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUXPGA_CFG1(rev),
			RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_sel_input_MASK(rev),
			1 << RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_sel_input_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_AUXPGA_CFG1(rev),
			RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_vcm_ctrl_MASK(rev),
			0 << RF_20692_WL_AUXPGA_CFG1_wl_auxpga_i_vcm_ctrl_SHIFT(rev))

		/* Setup Aux Path */
		/* Make sure DAC-to-ADC loop back is not active */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TX_DAC_CFG5(rev),
			RF_20692_WL_TX_DAC_CFG5_wl_txbb_dac2adc_MASK(rev),
			0 << RF_20692_WL_TX_DAC_CFG5_wl_txbb_dac2adc_SHIFT(rev))
		/* Disconnect TIA testmux output */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TIA_CFG5(rev),
			RF_20692_WL_TIA_CFG5_wl_tia_out_test_MASK(rev),
			0 << RF_20692_WL_TIA_CFG5_wl_tia_out_test_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_ADC_OVR1(rev),
			RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_in_test_MASK(rev),
			1 << RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_in_test_SHIFT(rev))
		/* Connect the AuxPGA output to ADC input */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_ADC_CFG10(rev),
			RF_20692_WL_ADC_CFG10_wl_adc_in_test_MASK(rev),
			0xf << RF_20692_WL_ADC_CFG10_wl_adc_in_test_SHIFT(rev))

		/* Turn off TIA otherwise it dominates the ADC input */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_BB_OVR1(rev),
			RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_amp1_pwrup_MASK(rev),
			1 << RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_amp1_pwrup_SHIFT(rev))
		/* PD 1st amp */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TIA_CFG3(rev),
			RF_20692_WL_TIA_CFG3_wl_tia_amp1_pwrup_MASK(rev),
			0 << RF_20692_WL_TIA_CFG3_wl_tia_amp1_pwrup_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_BB_OVR1(rev),
			RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_pwrup_amp2_MASK(rev),
			1 << RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_pwrup_amp2_SHIFT(rev))
		/* PD 2nd amp */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TIA_CFG3(rev),
			RF_20692_WL_TIA_CFG3_wl_tia_pwrup_amp2_MASK(rev),
			0 << RF_20692_WL_TIA_CFG3_wl_tia_pwrup_amp2_SHIFT(rev))
		/* dont bypass tia amp2 */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_BB_OVR1(rev),
			RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_amp2_bypass_MASK(rev),
			1 << RF_20692_WL_RX_BB_OVR1_wl_ovr_tia_amp2_bypass_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_TIA_CFG1(rev),
			RF_20692_WL_TIA_CFG1_wl_tia_amp2_bypass_MASK(rev),
			0 << RF_20692_WL_TIA_CFG1_wl_tia_amp2_bypass_SHIFT(rev))

		/* powerup adc */
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_MINIPMU_OVR1(rev),
			RF_20692_WL_MINIPMU_OVR1_wl_ovr_pmu_ADCldo_pu_MASK(rev),
			1 << RF_20692_WL_MINIPMU_OVR1_wl_ovr_pmu_ADCldo_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_PMU_OP(rev),
			RF_20692_WL_PMU_OP_wl_ADCldo_pu_MASK(rev),
			1 << RF_20692_WL_PMU_OP_wl_ADCldo_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_ADC_OVR1(rev),
			RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_clk_slow_pu_MASK(rev),
			1 << RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_clk_slow_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_ADC_CFG8(rev),
			RF_20692_WL_ADC_CFG8_wl_adc_clk_slow_pu_MASK(rev),
			1 << RF_20692_WL_ADC_CFG8_wl_adc_clk_slow_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_ADC_OVR1(rev),
			RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_slow_pu_MASK(rev),
			1 << RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_slow_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_ADC_CFG1(rev),
			RF_20692_WL_ADC_CFG1_wl_adc_slow_pu_MASK(rev),
			1 << RF_20692_WL_ADC_CFG1_wl_adc_slow_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_RX_ADC_OVR1(rev),
			RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_sipo_pu_MASK(rev),
			1 << RF_20692_WL_RX_ADC_OVR1_wl_ovr_adc_sipo_pu_SHIFT(rev))
		RADIO_REG_MOD_ENTRY(RF0_20692_WL_ADC_CFG1(rev),
			RF_20692_WL_ADC_CFG1_wl_adc_sipo_pu_MASK(rev),
			1 << RF_20692_WL_ADC_CFG1_wl_adc_sipo_pu_SHIFT(rev))
	PHY_REG_LIST_EXECUTE(pi);
}

static void
wlc_phy_radio20692_tempsense_vbat_radio_cleanup(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_tempsense_vbat_radioregs_t *psave = pi_lcn20->tempsense_vbat_radioregs;
	uint8 i;
	uint16 *ptr;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Make sure we didn't call the setup twice */
	ASSERT(psave->is_orig);
	psave->is_orig = FALSE;

	ptr = &psave->wl_temp_sens_ovr1;

	/* Restore saved original radio reg values */
	for (i = 0; i < NUM_RADIOREG_SAVREST; i++, ptr++) {
		phy_utils_write_radioreg(pi, temp_vbat_radioreg_savrest[i], *ptr);
	}
}

static int16
wlc_lcn20phy_read_tempsense_regs(phy_info_t *pi)
{
	uint16 tempsenseval1, tempsenseval2;
	uint16 tempsenseval3, tempsenseval4;
	int16 temp2, temp1, temp3, temp4;
	int16 avg;

	tempsenseval1 = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusTemp) & 0x1FF;
	tempsenseval2 = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusTemp1) & 0x1FF;
	tempsenseval3 = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusTemp2) & 0x1FF;
	tempsenseval4 = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusTemp3) & 0x1FF;

	if (tempsenseval1 > 255)
		temp1 = (int16)(tempsenseval1 - 512);
	else
		temp1 = (int16)tempsenseval1;

	if (tempsenseval2 > 255)
		temp2 = (int16)(tempsenseval2 - 512);
	else
		temp2 = (int16)tempsenseval2;

	if (tempsenseval3 > 255)
		temp3 = (int16)(tempsenseval3 - 512);
	else
		temp3 = (int16)tempsenseval3;

	if (tempsenseval4 > 255)
		temp4 = (int16)(tempsenseval4 - 512);
	else
		temp4 = (int16)tempsenseval4;

	avg = ((temp2 - temp1) + (temp4 - temp3)) >> 1;

	return avg;
}

static int16
wlc_lcn20phy_temp_sense_vbatTemp_on(phy_info_t *pi, int mode)
{
	uint16 vbat;
	int16 avg = 0;
	uint16 save_AfeCtrlOvr1, save_AfeCtrlOvr1Val;
	uint16 save_sslpnCalibClkEnCtrl, save_TSSIMode;
	uint16 save_TempSenseCorrection, save_agcControl4;
	uint8 SAVE_baseIdx, SAVE_baseIdx_cck;
	uint16 save_RxSdFeConfig1;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	bool suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);
	if (!suspend)
		wlapi_suspend_mac_and_wait(pi->sh->physhim);

	SAVE_baseIdx = wlc_lcn20phy_get_current_tx_pwr_idx(pi);
	SAVE_baseIdx_cck = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusExtCCK, baseIndexCCK)>>1;

	save_AfeCtrlOvr1 = phy_utils_read_phyreg(pi, LCN20PHY_AfeCtrlOvr1);
	save_AfeCtrlOvr1Val = phy_utils_read_phyreg(pi, LCN20PHY_AfeCtrlOvr1Val);
	save_sslpnCalibClkEnCtrl = phy_utils_read_phyreg(pi, LCN20PHY_sslpnCalibClkEnCtrl);
	save_TSSIMode = phy_utils_read_phyreg(pi, LCN20PHY_TSSIMode);
	save_TempSenseCorrection = PHY_REG_READ(pi, LCN20PHY, TempSenseCorrection, tempsenseCorr);
	save_RxSdFeConfig1 = phy_utils_read_phyreg(pi, LCN20PHY_RxSdFeConfig1);
	save_agcControl4 = phy_utils_read_phyreg(pi, LCN20PHY_agcControl4);

	/* Ovr DVGA1 */
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, rx_farrow_rshift_0, 4);
	PHY_REG_MOD(pi, LCN20PHY, RxSdFeConfig1, farrow_rshift_force, 1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);

	PHY_REG_LIST_START
		/* settings for txpwrctrl block */
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_delay, 64)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_Vbat, Nvbat_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_temp, Ntemp_delay, 64)
		PHY_REG_MOD_ENTRY(LCN20PHY, TxPwrCtrlNum_temp, Ntemp_intg_log2, 4)
		PHY_REG_MOD_ENTRY(LCN20PHY, TempSenseCorrection, tempsenseCorr, 0)
		/* Swap iq */
		PHY_REG_MOD_ENTRY(LCN20PHY, TSSIMode, tssiADCSel, 1)

		/* clock for pwrctrl block */
		PHY_REG_MOD_ENTRY(LCN20PHY, sslpnCalibClkEnCtrl, txFrontEndCalibClkEn, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, AfeCtrlOvr1Val, afe_iqadc_aux_en_ovr_val, 1)
		PHY_REG_MOD_ENTRY(LCN20PHY, AfeCtrlOvr1, afe_iqadc_aux_en_ovr, 1)
	PHY_REG_LIST_EXECUTE(pi);

	wlc_lcn20phy_set_start_tx_pwr_idx(pi, SAVE_baseIdx);
	wlc_lcn20phy_set_start_CCK_tx_pwr_idx(pi, SAVE_baseIdx_cck);

	wlc_phy_radio20692_tempsense_vbat_radio_setup(pi, mode);

	OSL_DELAY(10);
	PHY_REG_MOD(pi, LCN20PHY, TxPwrCtrlRangeCmd, force_vbatTemp, 1);
	OSL_DELAY(10);

	if (mode == TEMPSENSE) {
		avg = wlc_lcn20phy_read_tempsense_regs(pi);
	}
	else {
		vbat = phy_utils_read_phyreg(pi, LCN20PHY_TxPwrCtrlStatusVbat) & 0x1FF;
		if (vbat > 255)
			avg = (int16)(vbat - 512);
		else
			avg = (int16)vbat;
	}

	wlc_phy_radio20692_tempsense_vbat_radio_cleanup(pi);

	PHY_REG_WRITE(pi, LCN20PHY, RxSdFeConfig1, save_RxSdFeConfig1);
	PHY_REG_WRITE(pi, LCN20PHY, agcControl4, save_agcControl4);
	PHY_REG_WRITE(pi, LCN20PHY, AfeCtrlOvr1, save_AfeCtrlOvr1);
	PHY_REG_WRITE(pi, LCN20PHY, AfeCtrlOvr1Val, save_AfeCtrlOvr1Val);
	PHY_REG_WRITE(pi, LCN20PHY, sslpnCalibClkEnCtrl, save_sslpnCalibClkEnCtrl);
	PHY_REG_WRITE(pi, LCN20PHY, TSSIMode, save_TSSIMode);
	PHY_REG_MOD(pi, LCN20PHY, TempSenseCorrection, tempsenseCorr, save_TempSenseCorrection);

	wlc_lcn20phy_set_start_tx_pwr_idx(pi, pi_lcn20->tssi_idx);
	wlc_lcn20phy_set_start_CCK_tx_pwr_idx(pi, pi_lcn20->cck_tssi_idx);

	if (!suspend)
		wlapi_enable_mac(pi->sh->physhim);

	return avg;
}

int16
wlc_lcn20phy_tempsense(phy_info_t *pi, bool mode)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;

	int32 b = pi_lcn20->temp_add;
	int32 a = pi_lcn20->temp_mult;
	int32 q = pi_lcn20->temp_q;
	int16 avg, degree;

	/* mode 1 will use force_vbat and do the measurement */
	if (mode == TEMPER_VBAT_TRIGGER_NEW_MEAS) {
		avg = wlc_lcn20phy_temp_sense_vbatTemp_on(pi, TEMPSENSE);
	} else {
			avg = wlc_lcn20phy_read_tempsense_regs(pi);
	}
	/* Temp in deg = (temp_add + (avg)*temp_mult)>>temp_q; */
	degree = ((b + (avg * a)) + (1 << (q-1))) >> q;

#ifdef ATE_BUILD
	ate_buffer_regval.curr_radio_temp = degree;
#endif // endif

	return degree;
}

uint16
wlc_lcn20phy_get_rxiqcal_tonefreqkhz_0(void)
{
	return LCN20PHY_RXIQCAL_TONEFREQKHZ_0;
}

uint16
wlc_lcn20phy_get_rxiqcal_tonefreqkhz_1(void)
{
	return LCN20PHY_RXIQCAL_TONEFREQKHZ_1;
}

uint16
wlc_lcn20phy_get_rxiqcal_toneamp(void)
{
	return LCN20PHY_RXIQCAL_TONEAMP;
}

uint16
wlc_lcn20phy_get_rxiqcal_numsamps(void)
{
	return LCN20PHY_RXIQCAL_NUMSAMPS;
}

uint32
wlc_lcn20phy_get_spinwait_iqest_qt_usec(void)
{
	return LCN20PHY_SPINWAIT_IQEST_QT_USEC;
}

uint32
wlc_lcn20phy_get_spinwait_iqest_usec(void)
{
	return LCN20PHY_SPINWAIT_IQEST_USEC;
}

uint32
wlc_lcn20phy_get_txiqlocal_tonefhz(void)
{
	return LCN20PHY_TXIQLOCAL_TONEFHZ;
}

uint16
wlc_lcn20phy_get_txiqlocal_toneamp(void)
{
	return LCN20PHY_TXIQLOCAL_TONEAMP;
}

uint32
wlc_lcn20phy_get_spinwait_txiqlocal_usec(void)
{
	return LCN20PHY_SPINWAIT_TXIQLOCAL_USEC;
}

void
wlc_lcn20phy_measure_rxspur_save_state(phy_info_t *pi,
	measure_rxspur_savestate_params_t *save_state)
{
	uint32 idac_val;

	save_state->SAVE_agc_fsm_en = PHY_REG_READ(pi, LCN20PHY, agcControl4, c_agc_fsm_en);

	/* Put AGC fsm in HG lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 0);
	OSL_DELAY(1);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, 1);
	OSL_DELAY(1);

	/* Apply iDAC ovr; this MUST be done when c_agc_fsm_en = 1 */
	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IDAC,
		&idac_val, 1, 32, LCN20PHY_MEAS_RXSPUR_IDACIDX);

	wlc_lcn20phy_dccal_ovr_idac(pi, 1, 1, idac_val);

	/* Save pwr-ctrl state and turn off */
	save_state->SAVE_txpwrctrl = wlc_lcn20phy_get_tx_pwr_ctrl(pi);
	wlc_lcn20phy_set_tx_pwr_ctrl(pi, LCN20PHY_TX_PWR_CTRL_OFF);

	/* Save phy/radio regs */
	save_state->SAVE_iqloCalCmd = READ_LCN20PHYREG(pi, iqloCalCmd);
	save_state->SAVE_iqloCalCmdNnum = READ_LCN20PHYREG(pi, iqloCalCmdNnum);
	save_state->SAVE_txpwrctrlcmd = READ_LCN20PHYREG(pi, TxPwrCtrlCmd);

	save_state->SAVE_wl_tx2g_cfg1 = READ_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0);
	save_state->SAVE_wl_iqcal_cfg1 = READ_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0);
	save_state->SAVE_wl_testbuf_cfg1 = READ_RADIO_REG_20692(pi, WL_TESTBUF_CFG1, 0);
	save_state->SAVE_wl_auxpga_cfg1 = READ_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0);
	save_state->SAVE_wl_tx_top_2g_ovr2 = READ_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0);
	save_state->SAVE_wl_tssi_iqcal_ovr1 = READ_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0);
	save_state->SAVE_wl_GPABuf_ovr1 = READ_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0);
	save_state->SAVE_wl_AUX_RXPGA_ovr1 = READ_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0);

}

void
wlc_lcn20phy_measure_rxspur_restore_state(phy_info_t *pi,
	measure_rxspur_savestate_params_t *save_state)
{
	/* Restore phy/radio regs */
	WRITE_RADIO_REG_20692(pi, WL_TX2G_CFG1, 0, save_state->SAVE_wl_tx2g_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_IQCAL_CFG1, 0, save_state->SAVE_wl_iqcal_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TESTBUF_CFG1, 0, save_state->SAVE_wl_testbuf_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_AUXPGA_CFG1, 0, save_state->SAVE_wl_auxpga_cfg1);
	WRITE_RADIO_REG_20692(pi, WL_TX_TOP_2G_OVR2, 0, save_state->SAVE_wl_tx_top_2g_ovr2);
	WRITE_RADIO_REG_20692(pi, WL_TSSI_IQCAL_OVR1, 0, save_state->SAVE_wl_tssi_iqcal_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_GPABUF_OVR1, 0, save_state->SAVE_wl_GPABuf_ovr1);
	WRITE_RADIO_REG_20692(pi, WL_AUX_RXPGA_OVR1, 0, save_state->SAVE_wl_AUX_RXPGA_ovr1);

	WRITE_LCN20PHYREG(pi, TxPwrCtrlCmd, save_state->SAVE_txpwrctrlcmd);
	WRITE_LCN20PHYREG(pi, iqloCalCmd, save_state->SAVE_iqloCalCmd);
	WRITE_LCN20PHYREG(pi, iqloCalCmdNnum, save_state->SAVE_iqloCalCmdNnum);

	wlc_lcn20phy_set_tx_pwr_ctrl(pi, save_state->SAVE_txpwrctrl);

	/* Remove HG-lock mode */
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_hg_lock, 0);
	PHY_REG_MOD(pi, LCN20PHY, agcControl4, c_agc_fsm_en, save_state->SAVE_agc_fsm_en);
}

void
wlc_lcn20phy_dssf_cal_est_noise_chk_spur(phy_info_t *pi, const int16 *spur_offset,
	int16 *delta_dB, bool *enableDSSF, rxspur_params_t *spur_list, uint16 *spurListSize)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	lcn20phy_lna_params_t *lna_params = pi_lcn20->lna_params;
	int16 noise_pwr_dB = 0;
	uint8 j, count = 0;
	int16 spurLvl, temp;
	int16 spur_pwr_dB = 0;
	uint32 initgains;
	uint16 dvga1;

	/*
	 * Estimate noise floor.
	 */
	for (j = 0; j < LCN20PHY_DSSF_MAX_NOTCHES; j++) {
		if (spur_offset[j] != 0) {
			count++;
			wlc_lcn20phy_set_dssf_freq(pi, count, (spur_offset[j]*1000));
		}
	}

	if (count != 0) {
		if (count == 1)
			wlc_lcn20phy_set_dssf_mode(pi, 1, 1, 0, 2, 0, 0);
		else if (count == 2)
			wlc_lcn20phy_set_dssf_mode(pi, 1, 1, 1, 2, 2, 0);

		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAINIDX,
			&initgains, 1, 32, lna_params->meas_rxspur_gaintblidx);
		dvga1 = (initgains & LCN20PHY_GainIdxTbl_gaintbl_idx_MASK) >>
			LCN20PHY_GainIdxTbl_gaintbl_idx_SHIFT;
		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_GAIN, &initgains, 1, 32, dvga1);
		dvga1 = (initgains & LCN20PHY_GainTbl_dvga1_gain_MASK) >>
			LCN20PHY_GainTbl_dvga1_gain_SHIFT;
		temp = 3 * (dvga1 - 5);
		temp = (LCN20PHY_INIT_GAIN_DB - temp) << LCN20PHY_NOISEDB_SCALE;

		wlc_lcn20phy_apply_gainidx_settings(pi, (uint8) lna_params->meas_rxspur_gaintblidx);
		wlc_lcn20phy_noise_est(pi, 5, temp, &noise_pwr_dB);
		wlc_lcn20phy_rx_gain_override_enable_wrapper(pi, FALSE);
	}

	/* Check for spurs */
	*spurListSize = 0;
	for (j = 0; j < LCN20PHY_DSSF_MAX_NOTCHES; j++) {
		if (spur_offset[j] != 0) {
			/* Measure strength of spur and record the 'significant' one */
			spurLvl = wlc_lcn20phy_measure_rxspur(pi, spur_offset[j],
				&spur_pwr_dB,
				LCN20PHY_MEAS_RXSPUR_AMPL,
				LCN20PHY_MEAS_RXSPUR_CNTLOG2,
				LCN20PHY_MEAS_RXSPUR_DVGA1);

			*delta_dB = spur_pwr_dB - noise_pwr_dB - LCN20PHY_SPUR_NOISE_DELTA_ADJ;
#ifdef DSSF_DEBUG
			PHY_DSSF(("spur=%dKHz delta_dB=%d\n", spur_offset[j], *delta_dB));
#endif // endif
			if (*delta_dB >= pi_lcn20->dssf_thresh[0]) {
				*enableDSSF = TRUE;
				spur_list[*spurListSize].freqKHz = spur_offset[j];
				spur_list[*spurListSize].delta_dB = *delta_dB;
				spur_list[*spurListSize].spurlvl = spurLvl;
				*spurListSize = *spurListSize + 1;
			}
		}
	}
}

void
wlc_lcn20phy_tx_iqlo_cal_process_results(phy_info_t *pi,
	uint16 *coeffs, phy_txiqlocal_mode_t cal_mode)
{
	/* ----------------
	* Process Results
	*  ----------------
	*/
	/* copy "best" to "normal" */
	if (cal_mode != TXIQLOCAL_DLORECAL) {
		wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQBESTCOEFF_OFFSET);
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQCOMP_OFFSET);
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			coeffs, 2, 16, LCN20PHY_TXIQLOCAL_BPHY_IQCOMP_OFFSET);
	}

	wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
		coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOBESTCOEFF_OFFSET);
	wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
		coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOCOMP_OFFSET);
	wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
		coeffs, 1, 16, LCN20PHY_TXIQLOCAL_BPHY_DLOCOMP_OFFSET);
}

void
wlc_lcn20phy_tx_iqlo_cal_cleanup(phy_info_t *pi)
{
	/* Switch off test tone */
	wlc_lcn20phy_stop_tx_tone(pi);
	wlc_lcn20phy_tx_iqlo_cal_phy_cleanup(pi);
	wlc_phy_radio20692_tx_iqlo_cal_radio_cleanup(pi);
	wlc_lcn20phy_deaf_mode(pi, FALSE);
}

void
wlc_lcn20phy_tx_iqlo_cal_execute(phy_info_t *pi, uint16 bbmult_for_lad,
	uint8 n_cal_cmds, uint16 *cal_cmds, uint16 *coeffs)
{
	uint8 indx = 0;
	uint32 bbmult_scaled = 0;
	uint16 tblentry = 0;

	/* -----------
	* Main Loop
	*-----------
	* dynamically set up gain ladder based on this core's bbmult
	* (the bbmult chosen above will become max entry in the ladder)
	*/
	for (indx = 0; indx < 18; indx++) {
		/* calculate and write LO cal gain ladder */
		bbmult_scaled = lcn20phy_txiqlocal_ladder_lo[indx].percent * bbmult_for_lad;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | lcn20phy_txiqlocal_ladder_lo[indx].g_env;
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tblentry, 1, 16, indx);

		/* calculate and write IQ cal gain ladder */
		bbmult_scaled = lcn20phy_txiqlocal_ladder_iq[indx].percent * bbmult_for_lad;
		bbmult_scaled /= 100;
		tblentry = ((bbmult_scaled & 0xff) << 8) | lcn20phy_txiqlocal_ladder_iq[indx].g_env;
		wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
			&tblentry, 1, 16, indx+32);
	}

	for (indx = 0; indx < n_cal_cmds; indx++) {
		uint8 cal_type;

		WRITE_LCN20PHYREG(pi, iqloCalgtlthres, LCN20PHY_TXIQLOCAL_THRSLAD);

		/* trigger calibration step */
		WRITE_LCN20PHYREG(pi, iqloCalCmd, (cal_cmds[indx] | 0x8000));

		/* wait for 100msec */
		SPINWAIT(((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) != 0),
			wlc_lcn20phy_get_spinwait_txiqlocal_usec());
		ASSERT((READ_LCN20PHYREG(pi, iqloCalCmd) & 0xc000) == 0);

		cal_type = READ_LCN20PHYREGFLD(pi, iqloCalCmd, cal_type);
		if (cal_type == LCN20PHY_CAL_TYPE_IQ) {
			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQBESTCOEFF_OFFSET);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 2, 16, LCN20PHY_TXIQLOCAL_IQCOEFF_OFFSET);
		} else {
			wlc_lcn20phy_common_read_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOBESTCOEFF_OFFSET);
			wlc_lcn20phy_common_write_table(pi, LCN20PHY_TBL_ID_IQLOCAL,
				coeffs, 1, 16, LCN20PHY_TXIQLOCAL_DLOCOEFF_OFFSET);
		}
	} /* For loop for cals */
}

int16
wlc_lcn20phy_rxpath_rssicorr(phy_info_t *pi, int16 rssi,
	lcn20phy_rssi_gain_params_t *rssi_gain_param, int frm_type, int rate)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	uint8 channel = CHSPEC_CHANNEL(pi->radio_chanspec);
	int8 rssi_delta_2g[LCN20PHY_GAIN_DELTA_2G_PARAMS];
	int8 rssi_delta_bgn;
	int8 rssi_delta_end;
	int16 rssi_corr_gain_delta;
	int8 elna_bypass_delta;
	uint8 channel_seg;
	int8 delta_slope[LCN20PHY_GAIN_DELTA_2G_PARAMS];
	int8 delta_idx;
	uint8 elna_bypass;
	uint8 lna1_rout;
	uint8 lna2_gain;
	uint8 tia_amp2_bypass;
	uint8 aci_tbl_ind;
	uint16 tia_R1_val, tia_R2_val, tia_R3_val;
	uint8 dvga1_val;
	uint8 lna1_gain;
	int16 real_lna1_gain;
	uint8 tia_index;
	int16 real_tia_gain;
	int16 real_dvga1_gain;
	int16 real_total_gain;
	int16 nominal_power;
	int16 new_rssi_value;

	bzero(delta_slope, sizeof(delta_slope));
	bzero(rssi_delta_2g, sizeof(rssi_delta_2g));
	elna_bypass = rssi_gain_param->elna_bypass;
	lna1_rout = rssi_gain_param->lna1_rout;
	lna1_gain = rssi_gain_param->lna1_gain;
	lna2_gain = rssi_gain_param->lna2_gain;
	tia_amp2_bypass = rssi_gain_param->tia_amp2_bypass;
	aci_tbl_ind = rssi_gain_param->aci_tbl_ind;
	tia_R1_val = rssi_gain_param->tia_R1_val;
	tia_R2_val = rssi_gain_param->tia_R2_val;
	tia_R3_val = rssi_gain_param->tia_R3_val;
	dvga1_val = rssi_gain_param->dvga1_val;

	/* translate radio setting back into radio gain index
	 * these parameters are from TIA config table
	 */
	if ((tia_R1_val == 32) && (tia_R2_val == 64) && (tia_R3_val == 181))
		tia_index = 5;
	else if ((tia_R1_val == 32) && (tia_R2_val == 64) && (tia_R3_val == 128))
		tia_index = 6;
	else if ((tia_R1_val == 32) && (tia_R2_val == 64) && (tia_R3_val == 90))
		tia_index = 7;
	else if ((tia_R1_val == 32) && (tia_R2_val == 64) && (tia_R3_val == 64))
		tia_index = 8;
	else if ((tia_R1_val == 32) && (tia_R2_val == 91) && (tia_R3_val == 45))
		tia_index = 9;
	else if ((tia_R1_val == 32) && (tia_R2_val == 108) && (tia_R3_val == 32))
		tia_index = 10;
	else if ((tia_R1_val == 0) && (tia_R2_val == 0) &&
		(tia_R3_val == 0) && (tia_amp2_bypass == 1))
		tia_index = 11;
	else
		tia_index = 0;

	/* TIA gain */
	if (tia_index == 5)
		real_tia_gain = 26;    /* 26 dB */
	else if (tia_index == 6)
		real_tia_gain = 29;    /* 29 dB */
	else if (tia_index == 7)
		real_tia_gain = 32;    /* 32 dB */
	else if (tia_index == 8)
		real_tia_gain = 35;    /* 35 dB */
	else if (tia_index == 9)
		real_tia_gain = 38;    /* 38 dB */
	else if (tia_index == 10)
		real_tia_gain = 41;    /* 41 dB */
	else if (tia_index == 11)
		real_tia_gain = 44;	   /* 44 dB */
	else
		real_tia_gain = 0;

	/* TIA gain is different with different LNA2 gain setting */
	if (lna2_gain == 1)
		real_tia_gain = real_tia_gain - 6;
	else if (lna2_gain == 0)
		real_tia_gain = real_tia_gain - 12;

	/* LNA1 gain in RX ELG mode */
	if ((lna1_rout == 8) && (lna1_gain == 2))
		real_lna1_gain = -4;   /* -4 dB */
	else if ((lna1_rout == 8) && (lna1_gain == 3))
		real_lna1_gain = 2;   /* 2 dB */
	else if ((lna1_rout == 8) && (lna1_gain == 4))
		real_lna1_gain = 8;   /* 8 dB */
	else if ((lna1_rout == 8) && (lna1_gain == 5))
		real_lna1_gain = 14;   /* 14 dB */
	else if ((lna1_rout == 5) && (lna1_gain == 5))
		real_lna1_gain = 20;   /* 20 dB */
	else if ((lna1_rout == 0) && (lna1_gain == 5))
		real_lna1_gain = 26;   /* 26 dB */
	else
		real_lna1_gain = 0;

	real_dvga1_gain = (dvga1_val - 3)*3;

	real_total_gain = real_lna1_gain + real_tia_gain + real_dvga1_gain + 2;

	nominal_power = rssi + (real_total_gain<<LCN20_QDB_SHIFT);
	if (elna_bypass) {
		nominal_power = nominal_power - ((pi_lcn20->tr_isolation * LCN20PHY_GAIN_STEP)
				<< LCN20_QDB_SHIFT);
	}

	/* enable post-compensation only for 11b rates */
	if (frm_type == LCN20PHY_CCK) {
		if (!(aci_tbl_ind && tia_index != 11)) {

				if ((nominal_power >= -18*4) && (nominal_power <= -10*4)) {
					new_rssi_value = rssi - 8;
				}
				else if ((nominal_power >= -20*4) && (nominal_power < -18*4)) {
					new_rssi_value = rssi - 4;
				}
				else if ((nominal_power > -23*4) && (nominal_power < -20*4)) {
					new_rssi_value = rssi;
				}
				else if ((nominal_power > -26*4) && (nominal_power <= -23*4)) {
					new_rssi_value = rssi + 4;
				}
				else if ((nominal_power >= -30*4) && (nominal_power <= -26*4)) {
					new_rssi_value = rssi + 8;
				}
				else if ((nominal_power >= -35*4) && (nominal_power < -30*4)) {
					new_rssi_value = rssi + 12;
				}
				else {
					new_rssi_value = rssi;
				}
		} else {
				/* Offset for aci cases with tia_index != 11 */
				new_rssi_value = rssi + LCN20PHY_ACITBL_TIA_OFFSET;
		}
	} else {
			new_rssi_value = rssi;
	}

	rssi = new_rssi_value;

	channel_seg = 0;
	while ((channel > pi_lcn20->rssi_cal_channel[channel_seg]) &&
		(channel_seg < LCN20PHY_GROUPS-1))
		channel_seg++;

	if (channel <= pi_lcn20->rssi_cal_channel[channel_seg]) {
		if (channel == pi_lcn20->rssi_cal_channel[channel_seg]) {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++)
				rssi_delta_2g[delta_idx] = *(pi_lcn20->rssi_corr_gain_delta_2g_sub
					+ (channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS)+delta_idx);
		} else if (channel < pi_lcn20->rssi_cal_channel[channel_seg]) {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++) {
				rssi_delta_bgn = *(pi_lcn20->rssi_corr_gain_delta_2g_sub +
					((channel_seg-1)*LCN20PHY_GAIN_DELTA_2G_PARAMS) +
					delta_idx);
				rssi_delta_end = *(pi_lcn20->rssi_corr_gain_delta_2g_sub +
					channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS + delta_idx);
				delta_slope[delta_idx] =  (int16)(rssi_delta_end -
					rssi_delta_bgn) *
					(channel-pi_lcn20->rssi_cal_channel[channel_seg-1]) /
					(pi_lcn20->rssi_cal_channel[channel_seg]
					- pi_lcn20->rssi_cal_channel[channel_seg-1]);
				rssi_delta_2g[delta_idx] = rssi_delta_bgn +
					delta_slope[delta_idx];
			}
		}
	} else {
			for (delta_idx = 0; delta_idx < LCN20PHY_GAIN_DELTA_2G_PARAMS;
				delta_idx++)
				rssi_delta_2g[delta_idx] = *(pi_lcn20->rssi_corr_gain_delta_2g_sub
					+ (channel_seg*LCN20PHY_GAIN_DELTA_2G_PARAMS)+delta_idx);
	}

	PHY_INFORM(("channel slope comp p_rssi_delta_2g:%d %d %d, channel_seg:%d,"
		"delta_slope:%d, %d, %d\n", rssi_delta_2g[0], rssi_delta_2g[1],
		rssi_delta_2g[2], channel_seg, delta_slope[0],
		delta_slope[1], delta_slope[2]));

	/* adjust the delta for different channel offset */
	rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT0_IDX]
		+= pi_lcn20->rssi_channel_offset_rout0[channel-1];
	rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX]
		+= pi_lcn20->rssi_channel_offset_rout8[channel-1];
	rssi_delta_2g[LCN20PHY_RSSI_DELTA_ELNABYP_IDX]
		+= pi_lcn20->rssi_channel_offset_elnabyp[channel-1];

	PHY_INFORM(("channel offset p_rssi_delta_2g:%d %d %d, channel_seg:%d\n",
		rssi_delta_2g[0], rssi_delta_2g[1], rssi_delta_2g[2], channel_seg));

	PHY_INFORM(("rssiQDB:%d, elna_bypass:%d, lna1_rout%d, "
		"lna2_gain:%d, tia_amp2_bypass:%d, aci_tbl_ind:%d\n",
		rssi, elna_bypass, lna1_rout,
		lna2_gain, tia_amp2_bypass, aci_tbl_ind));

	rssi += (pi->rssi_corr_normal << LCN20_QDB_SHIFT);

	/* Temp sense based correction */
	rssi += wlc_lcn20phy_rssi_tempcorr(pi, 0);

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {

		case WL_CHAN_FREQ_RANGE_2G:
			if (elna_bypass) {
				/* get the extra delta for board atten by substracting
				 * last index from 2nd index
				*/
				elna_bypass_delta = rssi_delta_2g[LCN20PHY_RSSI_DELTA_ELNABYP_IDX]
					- rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX];
				rssi += elna_bypass_delta;
				PHY_INFORM((" elna_bypass branch rssiQdB=%d, elna_bypass=%d, "
					"elna_bypass_delta =%d\n", rssi,
					elna_bypass, elna_bypass_delta));
			}

			if (aci_tbl_ind) {
		/* if(pi_lcn20->rxpath_index <=  LCN20PHY_ACITBL_GAINIDX_BOUNDRY2) */
				if (tia_amp2_bypass == 0)
					rssi_corr_gain_delta =
						rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX]
						- LCN20PHY_ACITBL_GAIN_OFFSET2 +
						(pi->u.pi_lcn20phy->rssicorr_aci <<
						LCN20_QDB_SHIFT);
		/* else if(pi_lcn20->rxpath_index <=  LCN20PHY_ACITBL_GAINIDX_BOUNDRY1) */
				else if ((tia_amp2_bypass == 1) && (lna2_gain == 0))
					rssi_corr_gain_delta =
						rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX] -
						LCN20PHY_ACITBL_GAIN_OFFSET1 +
						(pi->u.pi_lcn20phy->rssicorr_aci <<
						LCN20_QDB_SHIFT);
				else
					rssi_corr_gain_delta =
						rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX] +
						(pi->u.pi_lcn20phy->rssicorr_aci <<
						LCN20_QDB_SHIFT);
			} else {
		/* if(pi_lcn20->rxpath_index >= LCN20PHY_NORTBL_GAINIDX_BOUNDRY1) */
				if (lna1_rout == 0)
					rssi_corr_gain_delta =
						rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT0_IDX];
				else
					rssi_corr_gain_delta =
						rssi_delta_2g[LCN20PHY_RSSI_DELTA_ROUT8_IDX];
			}

			rssi += rssi_corr_gain_delta;

			PHY_INFORM(("%s: gain delta corrected : rssiQdB= %d, "
				"rssi_gain_delta= %d\n",
				__FUNCTION__, rssi, rssi_corr_gain_delta));

			break;
		default:
			break;
	}

	if (frm_type == LCN20PHY_CCK) {
		if (rate == LCN20PHY_RATE_1M) {
			rssi += pi_lcn20->rssi_rate_offset[0];
		}
		else if (rate == LCN20PHY_RATE_2M) {
			rssi += pi_lcn20->rssi_rate_offset[1];
		}
		else if (rate == LCN20PHY_RATE_5M5) {
			rssi += pi_lcn20->rssi_rate_offset[2];
		}
		else if (rate == LCN20PHY_RATE_11M) {
			rssi += pi_lcn20->rssi_rate_offset[3];
		}
		else {
			PHY_INFORM(("%s: Incorrect Rate\n", __FUNCTION__));
		}
	}
	else if (frm_type == LCN20PHY_OFDM) {

		rssi += pi_lcn20->rssi_rate_offset[LCN20PHY_OFDM_IDX + rate
			- LCN20PHY_OFDM_PLCP_OFFSET];
	}
	else if (frm_type == LCN20PHY_HT) {
		rssi += pi_lcn20->rssi_rate_offset[LCN20PHY_HT_IDX +rate];
	}
	else {
		PHY_INFORM(("%s: Incorrect Frame Type \n", __FUNCTION__));
	}
	return (rssi);
}

/* Returns correction factor in qdb */
int16
wlc_lcn20phy_rssi_tempcorr(phy_info_t *pi, bool mode)
{
	int16 curr_temp, ret;
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	int16 temp_coeff;
	int16 gain_err_temp_adj;

	PHY_INFORM(("%s: mode= %d\n", __FUNCTION__, mode));

	curr_temp = wlc_lcn20phy_tempsense(pi, mode);

	/* temp_coeff in nvram is multiplied by 2^10 */

	switch (wlc_phy_chanspec_bandrange_get(pi, pi->radio_chanspec)) {
		case WL_CHAN_FREQ_RANGE_2G:
			temp_coeff = pi_lcn20->rxgain_tempadj_2g;
			break;
		default:
			temp_coeff = 0;
			break;
	}

	/* gain_err_temp_adj is on 0.25dB steps */
	if (pi->srom_gain_cal_temp != LCN20_RSSI_INVALID_TEMP)
		gain_err_temp_adj = ((curr_temp - pi->srom_gain_cal_temp) * temp_coeff);
	else
		gain_err_temp_adj = 0;

	ret = ABS(gain_err_temp_adj) >> 8;
	if (gain_err_temp_adj < 0)
		ret = -ret;

	PHY_INFORM(("%s: curr_temp= %d, gain_cal_temp= %d, temp_coeff= %d, "
			"gain_err_temp_adj= %d, ret= %d\n",
			__FUNCTION__, curr_temp, pi->srom_gain_cal_temp,
			temp_coeff, gain_err_temp_adj, ret));
	return ret;
}

#if defined(WLTEST)
int16
wlc_phy_test_tssi_lcn20phy(phy_info_t *pi, int8 ctrl_type, int8 pwr_offs)
{
	int16 tssi_OB = 0;
	int16 tssi_reg;
	BCM_REFERENCE(pwr_offs);

	tssi_reg = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlStatusNew4, avgTssi) & 0x1ff;
	/* Convert avgtssi value from 2's complement 9bits to OB 7bits */
	if (tssi_reg >= 256)
		tssi_OB = (tssi_reg - 256)/4;
	else
		tssi_OB = (tssi_reg + 256)/4;

	return tssi_OB;
}

int16
wlc_phy_test_idletssi_lcn20phy(phy_info_t *pi, int8 ctrl_type)
{
	int16 idletssi_OB = INVALID_IDLETSSI_VAL;
	int16 idletssi_reg;

	idletssi_reg = PHY_REG_READ(pi, LCN20PHY, TxPwrCtrlIdleTssi, idleTssi0) & 0x1ff;
	/* Convert idletssi value from 2's complement 9bits to OB 7bits */
	if (idletssi_reg >= 256)
		idletssi_OB = (idletssi_reg - 256)/4;
	else
		idletssi_OB = (idletssi_reg + 256)/4;

	return idletssi_OB;
}
#endif // endif
#ifdef WL11ULB
bool
wlc_phy_lcn20_ulb_10_capable(phy_info_t *pi)
{
	return TRUE;
}

bool
wlc_phy_lcn20_ulb_5_capable(phy_info_t *pi)
{
	return TRUE;
}
#endif /* WL11ULB */

#if defined(WL_PROXDETECT)
static const char BCMATTACHDATA(rstr_proxd_loopback_gain_2G)[] = "proxd_loopback_gain_2G";
static void
BCMATTACHFN(wlc_phy_nvram_proxd_read)(phy_info_t *pi)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	/* Read proxd rx gain overrides from nvram */
	pi_lcn20->proxd_rx_gain_override.slna_byp =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 0, 0);
	pi_lcn20->proxd_rx_gain_override.slna_rout =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 1, 0);
	pi_lcn20->proxd_rx_gain_override.slna_gain =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 2, 4);
	pi_lcn20->proxd_rx_gain_override.lna2_gain =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 3, 2);
	pi_lcn20->proxd_rx_gain_override.tia =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 4, 3);
	pi_lcn20->proxd_rx_gain_override.dvga1_gain =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 5, 3);
	pi_lcn20->proxd_rx_gain_override.dvga2_gain =
		(uint8)PHY_GETINTVAR_ARRAY_DEFAULT(pi, rstr_proxd_loopback_gain_2G, 6, 0);
}

static void wlc_phy_tof_apply_loopback_gain(phy_info_t *pi, bool enable)
{
	phy_info_lcn20phy_t *pi_lcn20 = pi->u.pi_lcn20phy;
	wlc_lcn20phy_rx_gain_override(pi,
		pi_lcn20->proxd_rx_gain_override.slna_byp,
		pi_lcn20->proxd_rx_gain_override.slna_rout,
		pi_lcn20->proxd_rx_gain_override.slna_gain,
		pi_lcn20->proxd_rx_gain_override.lna2_gain,
		pi_lcn20->proxd_rx_gain_override.tia,
		pi_lcn20->proxd_rx_gain_override.dvga1_gain,
		pi_lcn20->proxd_rx_gain_override.dvga2_gain);
	wlc_lcn20phy_rx_gain_override_enable(pi, enable);

}
#endif /* WL_PROXDETECT */
#endif /* #if ((defined(LCN20CONF) && (LCN20CONF != 0))) */
