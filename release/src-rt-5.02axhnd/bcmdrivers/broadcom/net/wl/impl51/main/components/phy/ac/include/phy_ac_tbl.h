/*
 * ACPHY PHYTableInit module interface
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
 * $Id: phy_ac_tbl.h 720902 2017-09-12 17:20:17Z $
 */

#ifndef _phy_ac_tbl_h_
#define _phy_ac_tbl_h_

#include <phy_api.h>
#include <phy_ac.h>
#include <phy_tbl.h>

/* forward declaration */
typedef struct phy_ac_tbl_info phy_ac_tbl_info_t;
typedef struct phytbl_info acphytbl_info_t;

typedef struct phy_ac_tbl_data {
	/* this data is shared between tbl and radio */
	void	*chan_tuning;
	uint32	chan_tuning_tbl_len;
	/* this data is shared between tbl and tpc */
	bool	is_p25TxGainTbl;
} phy_ac_tbl_data_t;

/* register/unregister ACPHY specific implementations to/from common */
phy_ac_tbl_info_t *phy_ac_tbl_register_impl(phy_info_t *pi,
	phy_ac_info_t *aci, phy_tbl_info_t *ti);
void phy_ac_tbl_unregister_impl(phy_ac_tbl_info_t *info);

/* inter-module data API */
phy_ac_tbl_data_t *phy_ac_tbl_get_data(phy_ac_tbl_info_t *tbli);

uint8 wlc_phy_get_tbl_id_gainctrlbbmultluts(phy_info_t *pi, uint8 core);
uint8 wlc_phy_get_tbl_id_estpwrshftluts(phy_info_t *pi, uint8 core);
extern uint32 wlc_phy_ac_caps(phy_info_t *pi);
extern uint32 wlc_phy_ac_caps1(phy_info_t *pi);
extern uint8 wlc_phy_ac_phycap_maxbw(phy_info_t *pi);
extern void wlc_phy_table_write_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info);
extern void wlc_phy_table_write_acphy(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, const void *data);
extern void wlc_phy_table_read_ext_acphy(phy_info_t *pi, const acphytbl_info_t *ptbl_info);
extern void wlc_phy_table_read_acphy(phy_info_t *pi, uint32 i, uint32 l, uint32 o, uint32 w,
	void *d);
extern void wlc_phy_table_setbit_acphy(phy_info_t *pi, uint32 id, uint32 offset,
	uint32 width, uint32 mask);
extern void wlc_phy_table_clrbit_acphy(phy_info_t *pi, uint32 id, uint32 offset,
	uint32 width, uint32 mask);
extern void wlc_phy_table_write_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len,
	uint32 offset, uint32 width, void *data, uint8 core);
extern void wlc_phy_table_read_acphy_dac_war(phy_info_t *pi, uint32 id, uint32 len,
	uint32 offset, uint32 width, void *data, uint8 core);
extern void wlc_phy_table_write_tiny_chnsmth(phy_info_t *pi, uint32 id, uint32 len, uint32 offset,
	uint32 width, const void *data);
extern void wlc_phy_force_mac_clk(phy_info_t *pi, uint16 *orig_phy_ctl);
extern void wlc_phy_clear_static_table_acphy(phy_info_t *pi, const phytbl_info_t *ptbl_info,
	const uint32 tbl_info_cnt);
const uint16 *wlc_phy_get_txgain_tbl_20695(phy_info_t *pi);
void wlc_phy_ac_gains_load(phy_ac_tbl_info_t *tbli);
void wlc_phy_tx_gain_table_write_acphy(phy_ac_tbl_info_t *tbli, uint32 l,
	uint32 o, uint32 w, const void *d);
#if defined(BCMDBG)
int phy_ac_dump_axmacphyif(phy_info_t *pi, struct bcmstrbuf *b, bool suspend);
#endif // endif
#endif /* _phy_ac_tbl_h_ */
