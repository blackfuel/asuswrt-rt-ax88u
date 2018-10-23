/*
 * ACPHY Prephy modules implementation
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
 * $Id: phy_ac_prephy.c 750488 2018-03-06 23:32:01Z $
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include <phy_prephy.h>
#include <phy_prephy_api.h>
#include "phy_type_prephy.h"
#include <phy_ac.h>
#include <phy_ac_info.h>
#include <phy_ac_prephy.h>
#include <wlc_phy_int.h>
/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_utils_var.h>
#include <phy_rstr.h>
#include <bcmdevs.h>

/* preattachphy with nopi phy reg accessor macros */
#define _PHY_REG_READ_PREPHY(pi, regs, reg)	phy_utils_read_phyreg_nopi(pi, regs, reg)

#define READ_PHYREG_PREPHY(pi, regs, reg) \
	_PHY_REG_READ_PREPHY(pi, regs, ACPHY_##reg(pi->pubpi->phy_rev))

#define READ_PHYREGFLD_PREPHY(pi, regs, reg, field)				\
	((READ_PHYREG_PREPHY(pi, regs, reg)					\
	 & ACPHY_##reg##_##field##_##MASK(pi->pubpi->phy_rev)) >>	\
	 ACPHY_##reg##_##field##_##SHIFT(pi->pubpi->phy_rev))

#define WRITE_PHYREG_PREPHY(pi, regs, addr, val) \
	phy_utils_write_phyreg_nopi(pi, regs, ACPHY_##addr(pi->pubpi->phy_rev), val)

#define PHYREGFLD_SHIFT_PREPHY(pi, reg, field) \
	ACPHY_##reg##_##field##_##SHIFT(pi->pubpi->phy_rev)

/* vasip version read without pi */
void
phy_ac_prephy_vasip_ver_get(prephy_info_t *pi, d11regs_t *regs, uint32 *vasipver)
{
	bool vasippresent;

	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		vasippresent = TRUE;
	} else {
		vasippresent = READ_PHYREGFLD_PREPHY(pi, regs, PhyCapability2, vasipPresent);
	}

	if (vasippresent) {
		*vasipver = READ_PHYREGFLD_PREPHY(pi, regs, MinorVersion, vasipversion);
	} else {
		*vasipver = VASIP_NOVERSION;
	}
}

/* vasip reset without pi */
void
phy_ac_prephy_vasip_proc_reset(prephy_info_t *pi, d11regs_t *regs, bool reset)
{
	uint16 reset_offset = reset ? VASIPREGISTERS_RESET : VASIPREGISTERS_SET;
	uint16 reset_val = 1;

	if (ACMAJORREV_47(pi->pubpi->phy_rev)) {
		reset_offset >>= 2;
	}

	WRITE_PHYREG_PREPHY(pi, regs, TableID, ACPHY_TBL_ID_VASIPREGISTERS);
	WRITE_PHYREG_PREPHY(pi, regs, TableOffset, reset_offset);
	WRITE_PHYREG_PREPHY(pi, regs, TableDataHi, 0);
	WRITE_PHYREG_PREPHY(pi, regs, TableDataLo, reset_val);
}

/* vasip reset without pi */
void
phy_ac_prephy_vasip_clk_set(prephy_info_t *pi, d11regs_t *regs, bool set)
{
	if (ACMAJORREV_GE40_NE47(pi->pubpi->phy_rev)) {
		phy_ac_vasip_clk_enable(pi->sh->sih, set);
	} else {
		uint16 regval;

		regval = READ_PHYREG_PREPHY(pi, regs, dacClkCtrl);
		if (set) {
			regval = regval |
				(1 << PHYREGFLD_SHIFT_PREPHY(pi, dacClkCtrl, vasipClkEn));
		} else {
			regval = regval &
				~(1 << PHYREGFLD_SHIFT_PREPHY(pi, dacClkCtrl, vasipClkEn));
		}
		WRITE_PHYREG_PREPHY(pi, regs, dacClkCtrl, regval);
	}
}

uint32
phy_ac_prephy_caps(prephy_info_t *pi, uint32 *pacaps)
{
	int ret = BCME_OK;

	ASSERT(pacaps);
	if (ACREV_LT(pi->pubpi->phy_rev, HECAP_FIRST_ACREV)) {
		ret = BCME_UNSUPPORTED;
		goto done;
	}

	*pacaps = READ_PHYREGFLD_PREPHY(pi, pi->regs, PhyCapability0, Support5GHz) ?
		PHY_PREATTACH_CAP_SUP_5G : 0;

	*pacaps |= READ_PHYREGFLD_PREPHY(pi, pi->regs, PhyInternalCapability1, Support2GHz) ?
		PHY_PREATTACH_CAP_SUP_2G : 0;

done:
	return ret;
}
