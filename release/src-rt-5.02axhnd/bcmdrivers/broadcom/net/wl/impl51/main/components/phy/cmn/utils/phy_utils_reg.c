/*
 * PHY utils - register access functions.
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
 * $Id: phy_utils_reg.c 752567 2018-03-16 13:25:20Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <osl.h>
#include <bcmutils.h>
#include <d11.h>

#include <wlc_phy_radio.h>

#include <wlc_phyreg_n.h>
#include <wlc_phyreg_ac.h>
#include <wlc_radioreg_20693.h>

#include <phy_utils_reg.h>

#include <pci_core.h>
#include <hndpci.h>
#include <pcicfg.h>

#include <sbchipc.h>
#include <hndchipc.h>

#define P1C_PHYREG_VIA_TABLE

static void wlc_phy_conditional_assert(phy_info_t *pi);
/* coordinate with MAC before access PHY register */
void
phy_utils_phyreg_enter(phy_info_t *pi)
{
#ifdef STA
	/* track the nested enter calls */
	pi->phyreg_enter_depth++;
	if (pi->phyreg_enter_depth > 1)
		return;

	wlapi_bmac_ucode_wake_override_phyreg_set(pi->sh->physhim);
#endif // endif
}

void
phy_utils_phyreg_exit(phy_info_t *pi)
{
#ifdef STA
	ASSERT(pi->phyreg_enter_depth > 0);
	pi->phyreg_enter_depth--;
	if (pi->phyreg_enter_depth > 0)
		return;

	wlapi_bmac_ucode_wake_override_phyreg_clear(pi->sh->physhim);
#endif // endif
}

/* coordinate with MAC before access RADIO register */
void
phy_utils_radioreg_enter(phy_info_t *pi)
{
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_LOCK_RADIO, MCTL_LOCK_RADIO);

	/* allow any ucode radio reg access to complete */
	OSL_DELAY(10);
}

void
phy_utils_radioreg_exit(phy_info_t *pi)
{
	volatile uint16 dummy;

	/* allow our radio reg access to complete */
	dummy = R_REG(pi->sh->osh, D11_PHY_REG_0(pi));
	BCM_REFERENCE(dummy);

	pi->phy_wreg = 0;
	wlapi_bmac_mctrl(pi->sh->physhim, MCTL_LOCK_RADIO, 0);
}

/* All radio regs other than idcode are less than 16bits, so
 * {read, write}_radio_reg access the low 16bits only.
 */
uint16
phy_utils_read_radioreg(phy_info_t *pi, uint16 addr)
{
	uint16 data;

	wlc_phy_conditional_assert(pi);

	if ((addr == RADIO_IDCODE) && (!ISACPHY(pi))) {
		return RADIO_REG_READ_FAIL;
	}

	/* Check for valid radio address access */
	if (addr == INVALID_ADDRESS) {
		ASSERT(addr != INVALID_ADDRESS);
		return RADIO_REG_READ_FAIL;
	}

	addr |= pi->pubpi->radiooffset;

	/* check for accessing other radio in dual mac mode  */
	if ((D11REV_IS(pi->sh->corerev, 50) || (D11REV_IS(pi->sh->corerev, 55)) ||
		(D11REV_IS(pi->sh->corerev, 56)) || (D11REV_IS(pi->sh->corerev, 59))) &&
		(addr & JTAG_20693_MASK)) {

		ASSERT(pi == phy_get_pi(pi, PHY_RSBD_PI_IDX_CORE0));	/* must be pi for core 0 */
		ASSERT((phy_get_phymode(pi) & DUAL_MAC_MODE) == 0); /* must be single mac mode */
		ASSERT((addr & JTAG_20693_CR1) != 0);   /* altradioreg only for core 1 */
		addr &= ~JTAG_20693_MASK;       /* clear the core bit */
		W_REG(pi->sh->osh, D11_radioregaddr_cross(pi), addr);   /* use 2nd core */
#ifdef __mips__
		(void)R_REG(pi->sh->osh, D11_radioregaddr_cross(pi));
#endif /* __mips__ */
		data = R_REG(pi->sh->osh, D11_radioregdata_cross(pi));
	} else if ((D11REV_GE(pi->sh->corerev, 24) && !D11REV_IS(pi->sh->corerev, 27)) ||
	    (D11REV_IS(pi->sh->corerev, 22))) {

		W_REG(pi->sh->osh, D11_radioregaddr(pi), addr);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, D11_radioregaddr(pi));
#endif // endif
		data = R_REG(pi->sh->osh, D11_radioregdata(pi));
	} else {
		W_REG(pi->sh->osh, D11_PHY_REG_B(pi), addr);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, D11_PHY_REG_B(pi));
#endif // endif
		data = R_REG(pi->sh->osh, D11_PHY_REG_D(pi));
	}
	pi->phy_wreg = 0;	/* clear reg write metering */

	return data;
}

#if defined(BCMDBG_PHYREGS_TRACE)
uint16
phy_utils_read_radioreg_debug(phy_info_t *pi, uint16 addr, const char *reg_name)
{
	/* Check for valid radio address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for radio rev %d\n", pi->sh->unit,
		          reg_name, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(addr != INVALID_ADDRESS);
		return 0xffffU;
	}

	return phy_utils_read_radioreg(pi, addr);
}
#endif /* BCMDBG_PHYREGS_TRACE */

void
phy_utils_write_radioreg(phy_info_t *pi, uint16 addr, uint16 val)
{
	osl_t *osh;

	osh = pi->sh->osh;

	wlc_phy_conditional_assert(pi);

	/* The nphy chips with with corerev 22 have the new i/f, the one with
	 * ssnphy (4319b0) does not.
	 */
	if (BUSTYPE(pi->sh->bustype) == PCI_BUS) {
		if (++pi->phy_wreg >= pi->phy_wreg_limit) {
			(void)R_REG(osh, D11_MACCONTROL(pi));
			pi->phy_wreg = 0;
		}
	}

	if (addr == INVALID_ADDRESS) {
		ASSERT(addr != INVALID_ADDRESS);
		return;
	}

	/* check for accessing other radio in dual mac mode  */
	if ((D11REV_IS(pi->sh->corerev, 50) || (D11REV_IS(pi->sh->corerev, 55)) ||
		(D11REV_IS(pi->sh->corerev, 56)) || (D11REV_IS(pi->sh->corerev, 59))) &&
		(addr & JTAG_20693_MASK)) {

		ASSERT(pi == phy_get_pi(pi, PHY_RSBD_PI_IDX_CORE0));	/* must be pi for core 0 */
		ASSERT((phy_get_phymode(pi) & DUAL_MAC_MODE) == 0); /* must be single mac mode */
		ASSERT((addr & JTAG_20693_CR1) != 0);   /* altradioreg only for core 1 */
		addr &= ~JTAG_20693_MASK;       /* clear the core bit */
		W_REG(osh, D11_radioregaddr_cross(pi), addr);   /* use 2nd core */
#ifdef __mips__
		(void)R_REG(osh, D11_radioregaddr_cross(pi));
#endif /* __mips__ */
		W_REG(osh, D11_radioregdata_cross(pi), val);
	} else if ((D11REV_GE(pi->sh->corerev, 24) && !D11REV_IS(pi->sh->corerev, 27)) ||
	    (D11REV_IS(pi->sh->corerev, 22))) {

		W_REG(osh, D11_radioregaddr(pi), addr);
#ifdef __mips__
		(void)R_REG(osh, D11_radioregaddr(pi));
#endif // endif
		W_REG(osh, D11_radioregdata(pi), val);
	} else {
		W_REG(osh, D11_PHY_REG_B(pi), addr);
#ifdef __mips__
		(void)R_REG(osh, D11_PHY_REG_B(pi));
#endif // endif
		W_REG(osh, D11_PHY_REG_D(pi), val);
	}

	if ((BUSTYPE(pi->sh->bustype) == PCMCIA_BUS) &&
	    (pi->sh->buscorerev <= 3)) {
		volatile uint16 dummy = R_REG(osh, D11_PHY_REG_0(pi));

		BCM_REFERENCE(dummy);
	}
}

#if defined(BCMDBG_PHYREGS_TRACE)
void
phy_utils_write_radioreg_debug(phy_info_t *pi, uint16 addr, uint16 val, const char *reg_name)
{
	/* Check for valid radio address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for radio rev %d\n", pi->sh->unit,
		          reg_name, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(addr != INVALID_ADDRESS);
	} else {
		phy_utils_write_radioreg(pi, addr, val);
	}
}
#endif /* BCMDBG_PHYREGS_TRACE */

void
phy_utils_and_radioreg(phy_info_t *pi, uint16 addr, uint16 val)
{
	uint16 rval = phy_utils_read_radioreg(pi, addr);

	phy_utils_write_radioreg(pi, addr, (rval & val));
}

void
phy_utils_or_radioreg(phy_info_t *pi, uint16 addr, uint16 val)
{
	uint16 rval = phy_utils_read_radioreg(pi, addr);

	phy_utils_write_radioreg(pi, addr, (rval | val));
}

void
phy_utils_xor_radioreg(phy_info_t *pi, uint16 addr, uint16 mask)
{
	uint16 rval = phy_utils_read_radioreg(pi, addr);

	phy_utils_write_radioreg(pi, addr, (rval ^ mask));
}

void
phy_utils_mod_radioreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val)
{
	uint16 rval = phy_utils_read_radioreg(pi, addr);

	phy_utils_write_radioreg(pi, addr, (rval & ~mask) | (val & mask));
}

#if defined(BCMDBG_PHYREGS_TRACE)
void
phy_utils_mod_radioreg_debug(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	const char *reg_name)
{
	/* Check for valid radio address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for radio rev %d\n", pi->sh->unit,
		          reg_name, RADIOREV(pi->pubpi->radiorev)));
		ASSERT(addr != INVALID_ADDRESS);
	} else {
		phy_utils_mod_radioreg(pi, addr, mask, val);
	}
}
#endif /* BCMDBG_PHYREGS_TRACE */

void
phy_utils_gen_radioreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data)
{
	*orig_reg_addr = addr;
	*orig_reg_data = phy_utils_read_radioreg(pi, addr);

	*updated_reg_addr = addr;
	*updated_reg_data = ((*orig_reg_data) & ~mask) | (val & mask);
}

void
phy_utils_write_phychannelreg(phy_info_t *pi, uint val)
{
	volatile uint16 dummy;

	wlc_phy_conditional_assert(pi);

	W_REG(pi->sh->osh, D11_PHY_REG_8(pi), val);

	if ((BUSTYPE(pi->sh->bustype) == PCMCIA_BUS) &&
	    (pi->sh->buscorerev <= 3)) {
		dummy = R_REG(pi->sh->osh, D11_PHY_REG_0(pi));
		BCM_REFERENCE(dummy);
	}
}

uint16
phy_utils_read_phyreg(phy_info_t *pi, uint16 addr)
{
#ifdef P1C_PHYREG_VIA_TABLE
	uint32 tbl_val;
#else
	uint rval;
#if defined(DONGLEBUILD)
	volatile uint32 *r;
#else
	si_t *sih = (si_t*)pi->sh->sih;
	uint bar0win;
	uint org_bar0win;
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	wlc_phy_conditional_assert(pi);

	ASSERT(addr != INVALID_ADDRESS);	/* Check for valid PHY address access */

	if (addr < 0xc000) {
		W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), addr);
#ifdef __mips__
		(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi));
#endif // endif

		pi->phy_wreg = 0;	/* clear reg write metering */
		return (R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi)));
	} else { /* p1c phyreg read */
#ifdef P1C_PHYREG_VIA_TABLE
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &tbl_val);
		return (uint16) tbl_val;
#else
#if defined(DONGLEBUILD)
		addr = addr & 0x3fff;
		r = (volatile uint32 *)((volatile char *)0x284f0000 + (addr<<1));
		rval = R_REG(pi->sh->osh, (volatile uint16*)r);
#else
		org_bar0win = OSL_PCI_READ_CONFIG(pi->sh->osh, 0x80, 4);
		bar0win = 0x284f0000 + ((addr & 0x3fff) << 1);
		si_backplane_access(sih, bar0win, sizeof(uint16), &rval, TRUE);
		OSL_PCI_WRITE_CONFIG(pi->sh->osh, 0x80, sizeof(uint32), org_bar0win);
#endif /* DONGLEBUILD */
		return (uint16) rval;
#endif /* P1C_PHYREG_VIA_TABLE */
	}
}

uint16
phy_utils_read_phyreg_p1c(phy_info_t *pi, uint16 addr)
{
#ifdef P1C_PHYREG_VIA_TABLE
	uint32 tbl_val;
#else
	uint rval;
#if defined(DONGLEBUILD)
	volatile uint32 *r;
#else
	si_t *sih = (si_t*)pi->sh->sih;
	uint bar0win;
	uint org_bar0win;
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	wlc_phy_conditional_assert(pi);

	ASSERT(addr != INVALID_ADDRESS);	/* Check for valid PHY address access */

	addr = addr | 0xc000;

	/* p1c phyreg read */
#ifdef P1C_PHYREG_VIA_TABLE
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &tbl_val);
	return (uint16) tbl_val;
#else
#if defined(DONGLEBUILD)
	addr = addr & 0x3fff;
	r = (volatile uint32 *)((volatile char *)0x284f0000 + (addr<<1));
	rval = R_REG(pi->sh->osh, (volatile uint16*)r);
#else
	org_bar0win = OSL_PCI_READ_CONFIG(pi->sh->osh, 0x80, 4);
	bar0win = 0x284f0000 + ((addr & 0x3fff) << 1);
	si_backplane_access(sih, bar0win, sizeof(uint16), &rval, TRUE);
	OSL_PCI_WRITE_CONFIG(pi->sh->osh, 0x80, sizeof(uint32), org_bar0win);
#endif // endif
	return (uint16) rval;
#endif /* P1C_PHYREG_VIA_TABLE */
}

#if defined(BCMDBG_PHYREGS_TRACE)
uint16
phy_utils_read_phyreg_debug(phy_info_t *pi, uint16 addr, const char *reg_name)
{
	/* Check for valid phy address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for phy rev %d\n", pi->sh->unit,
		          reg_name, pi->pubpi->phy_rev));
		ASSERT(addr != INVALID_ADDRESS);
		return 0xffffU;
	}

	return phy_utils_read_phyreg(pi, addr);
}
#endif /* BCMDBG_PHYREGS_TRACE */

uint16
phy_utils_read_phyreg_wide(phy_info_t *pi)
{
	pi->phy_wreg = 0;	/* clear reg write metering */

	wlc_phy_conditional_assert(pi);

	return (R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi)));
}

void phy_utils_write_phyreg_array(phy_info_t *pi, const uint16* regp, int length)
{
	ASSERT(regp != NULL && length > 0);
	while (length-- > 0)
	{
		uint16 addr;
		uint16 access_type;
#if defined(DONGLEBUILD)
		access_type = *regp & PHY_RADIO_REG_MASK_TYPE;
		if ((ACREV_GE(pi->pubpi->phy_rev, 40) || ACREV_IS(pi->pubpi->phy_rev, 32) ||
		     ACREV_IS(pi->pubpi->phy_rev, 33)) && (access_type != RADIO_REG_TYPE)) {
			/* Broadcast bit is 14 for rev40 */
			addr = *regp++ & ~ 0x8000;
		} else {
			addr = *regp++ & ~ PHY_RADIO_REG_MASK_TYPE;
		}

#else
		access_type = *regp++ & PHY_RADIO_REG_MASK_TYPE;
		--length;
		if ((ACREV_GE(pi->pubpi->phy_rev, 40) || ACREV_IS(pi->pubpi->phy_rev, 32) ||
		     ACREV_IS(pi->pubpi->phy_rev, 33)) && (access_type != RADIO_REG_TYPE)) {
			/* Broadcast bit is 14 for rev40 */
			addr = *regp++ & ~ 0x8000;
		} else {
			addr = *regp++ & ~ PHY_RADIO_REG_MASK_TYPE;
		}
#endif /* DONGLEBUILD */
		switch (access_type)
		{
			case PHY_REG_MOD_TYPE:
				phy_utils_mod_phyreg(pi, addr, *regp, *(regp+1));
				++regp;
				--length;
				break;
			case PHY_REG_MOD_TYPE | RADIO_REG_TYPE:
				phy_utils_mod_radioreg(pi, addr, *regp, *(regp+1));
				++regp;
				--length;
				break;

			case PHY_REG_WRITE_TYPE:
				phy_utils_write_phyreg(pi, addr, *regp);
				break;

			case PHY_REG_WRITE_TYPE | RADIO_REG_TYPE:
				phy_utils_write_radioreg(pi, addr, *regp);
				break;

			case PHY_REG_AND_TYPE:
				phy_utils_and_phyreg(pi, addr, *regp);
				break;

			case PHY_REG_AND_TYPE | RADIO_REG_TYPE:
				phy_utils_and_radioreg(pi, addr, *regp);
				break;

			case PHY_REG_OR_TYPE:
				phy_utils_or_phyreg(pi, addr, *regp);
				break;

			case PHY_REG_OR_TYPE | RADIO_REG_TYPE:
				phy_utils_or_radioreg(pi, addr, *regp);
				break;

			default: ASSERT(0);
		}
		++regp;
		--length;
	}
}

void phy_utils_write_radioreg_array(phy_info_t *pi, uint16* regp, int length, uint8 core)
{
#define JTAG_SHIFT	9	/* should be the same as all the JTAG_2069*_SHIFT */
/* 43684BU There is a problem with this construction. Normally the core is (JTAG) is filtered away
 * and replaced with coreidx. But that doesnt work for PLL cores. It is not an option to use core
 * PLL since that as core for the callee (wlc_phy_radio20698_xtal_pwrup) will result in ASSERT.
 * Considering the assert (see below) it is likely that this function was not supposed to be used
 * pll cores. However to get it working for now the JTAG_MASK is limited to 0x3 and the callee is
 * kept as is using core 0 for the table.
 */
#define JTAG_MASK	(0x3 << JTAG_SHIFT) /* should be the same as all the JTAG_2069*_MASK */
	uint16 *table = regp;
	int len = length;

	ASSERT(regp != NULL && length > 0 && core < PHY_CORE_MAX);

	/* modify table by changing all radio addresses based on core */
	while (length-- > 0)
	{
		uint16 access_type;

#if defined(DONGLEBUILD)
		/* *regp = addr | type */
		access_type = *regp & PHY_RADIO_REG_MASK_TYPE;
#else
		/* *regp = type */
		access_type = *regp++ & PHY_RADIO_REG_MASK_TYPE;
		--length;
		/* *regp = addr */
#endif /* DONGLEBUILD */
		if (access_type & RADIO_REG_TYPE) {
			/* change register address based on core */
			*regp = (*regp & ~JTAG_MASK) | (core << JTAG_SHIFT);
		}
		++regp;

		switch (access_type)
		{
			/* an extra field is used for these types */
			case PHY_REG_MOD_TYPE:
			case PHY_REG_MOD_TYPE | RADIO_REG_TYPE:
				/* *regp = mask */
				++regp;
				--length;
				break;

			default:
				;
		}
		/* *regp = value */
		++regp;
		--length;
	}
	phy_utils_write_phyreg_array(pi, table, len);
}

void
phy_utils_write_phyreg(phy_info_t *pi, uint16 addr, uint16 val)
{
#ifndef P1C_PHYREG_VIA_TABLE
#if defined(DONGLEBUILD)
	volatile uint32 *r;
#else
	uint rval;
	si_t *sih = (si_t*)pi->sh->sih;
	uint bar0win;
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	wlc_phy_conditional_assert(pi);

	ASSERT(addr != INVALID_ADDRESS);	/* Check for valid PHY address access */

	if (addr < 0xc000) {
#ifdef __mips__
		W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), addr);
		(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi));
		W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), val);
		if (addr == NPHY_TableAddress)
			(void)R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi));
#else
		if (BUSTYPE(pi->sh->bustype) == PCI_BUS) {
			if (++pi->phy_wreg >= pi->phy_wreg_limit) {
				pi->phy_wreg = 0;
				(void)R_REG(pi->sh->osh, D11_PHY_REG_0(pi));
			}
		}
		W_REG(pi->sh->osh, (volatile uint32 *)(uintptr)(D11_PHY_REG_ADDR(pi)),
			addr | (val << 16));
#endif /* __mips__ */
	} else { /* p1c phyreg access */
#ifdef P1C_PHYREG_VIA_TABLE
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &val);
#else
#if defined(DONGLEBUILD)
		addr = addr & 0x3fff;
		r = (volatile uint32 *)((volatile char *)0x284f0000 + (addr<<1));
		W_REG(pi->sh->osh, (volatile uint16*)r, val);
#else
		bar0win = 0x284f0000 + ((addr & 0x3fff) << 1);
		rval = (uint) val;
		si_backplane_access(sih, bar0win, sizeof(uint16), &rval, FALSE);
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	}
}

void
phy_utils_write_phyreg_p1c(phy_info_t *pi, uint16 addr, uint16 val)
{
#ifndef P1C_PHYREG_VIA_TABLE
#if defined(DONGLEBUILD)
	volatile uint32 *r;
#else
	uint rval;
	si_t *sih = (si_t*)pi->sh->sih;
	uint bar0win;
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	wlc_phy_conditional_assert(pi);

	ASSERT(addr != INVALID_ADDRESS);        /* Check for valid PHY address access */

	addr = addr | 0xc000;

#ifdef P1C_PHYREG_VIA_TABLE
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &val);
#else
#if defined(DONGLEBUILD)
	addr = addr & 0x3fff;
	r = (volatile uint32 *)((volatile char *)0x284f0000 + (addr<<1));
	W_REG(pi->sh->osh, (volatile uint16*)r, val);
#else
	bar0win = 0x284f0000 + ((addr & 0x3fff) << 1);
	rval = (uint) val;
	si_backplane_access(sih, bar0win, sizeof(uint16), &rval, FALSE);
#endif // endif
#endif /* P1C_PHYREG_VIA_TABLE */
}

#if defined(BCMDBG_PHYREGS_TRACE)
void
phy_utils_write_phyreg_debug(phy_info_t *pi, uint16 addr, uint16 val, const char *reg_name)
{
	/* Check for valid phy address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for phy rev %d\n", pi->sh->unit,
		          reg_name, pi->pubpi->phy_rev));
		ASSERT(addr != INVALID_ADDRESS);
	} else {
		phy_utils_write_phyreg(pi, addr, val);
	}
}
#endif /* BCMDBG_PHYREGS_TRACE */

void
phy_utils_write_phyreg_wide(phy_info_t *pi, uint16 val)
{
	wlc_phy_conditional_assert(pi);

	W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), val);
}

void
phy_utils_and_phyreg(phy_info_t *pi, uint16 addr, uint16 val)
{
	/* Check for valid PHY address access */
	if (addr == INVALID_ADDRESS) {
		ASSERT(0);
		return;
	}
	wlc_phy_conditional_assert(pi);

	W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), addr);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi));
#endif // endif

	W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), (R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi)) & val));
	pi->phy_wreg = 0;	/* clear reg write metering */
}

void
phy_utils_or_phyreg(phy_info_t *pi, uint16 addr, uint16 val)
{
	wlc_phy_conditional_assert(pi);

	/* Check for valid PHY address access */
	if (addr == INVALID_ADDRESS) {
		ASSERT(addr != INVALID_ADDRESS);
		return;
	}

	W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), addr);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi));
#endif // endif

	W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi), (R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi)) | val));
	pi->phy_wreg = 0;	/* clear reg write metering */
}

void
phy_utils_mod_phyreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val)
{
	wlc_phy_conditional_assert(pi);

	/* Check for valid PHY address access */
	if (addr == INVALID_ADDRESS) {
		ASSERT(addr != INVALID_ADDRESS);
		return;
	}

	W_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi), addr);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR(pi));
#endif // endif

	W_REG(pi->sh->osh, D11_PHY_REG_DATA(pi),
	      ((R_REG(pi->sh->osh, D11_PHY_REG_DATA(pi)) & ~mask) | (val & mask)));
	pi->phy_wreg = 0;	/* clear reg write metering */
}

void
phy_utils_mod_phyreg_p1c(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val)
{
#ifdef P1C_PHYREG_VIA_TABLE
	uint32 tbl_val;
#else
	uint rval;
#if defined(DONGLEBUILD)
	volatile uint32 *r;
#else
	si_t *sih = (si_t*)pi->sh->sih;
	uint bar0win;
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLE */
	wlc_phy_conditional_assert(pi);

	/* Check for valid PHY address access */
	if (addr == INVALID_ADDRESS) {
		ASSERT(addr != INVALID_ADDRESS);
		return;
	}

	addr = addr | 0xc000;

#ifdef P1C_PHYREG_VIA_TABLE
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &tbl_val);
	tbl_val = ((tbl_val & ~mask) | (val & mask));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, addr, 32, &tbl_val);
#else
#if defined(DONGLEBUILD)
	addr = addr & 0x3fff;
	r = (volatile uint32 *)((volatile char *)0x284f0000 + (addr<<1));
	rval = R_REG(pi->sh->osh, (volatile uint16*)r);
	rval = ((rval & ~mask) | (val & mask));
	W_REG(pi->sh->osh, (volatile uint16*)r, rval);
#else
	/* access the p1c phyregs with regbase */
	bar0win = 0x284f0000 + ((addr & 0x3fff) << 1);
	si_backplane_access(sih, bar0win, sizeof(uint16), &rval, TRUE);
	rval = ((rval & ~mask) | (val & mask));
	si_backplane_access(sih, bar0win, sizeof(uint16), &rval, FALSE);
#endif /* DONGLEBUILD */
#endif /* P1C_PHYREG_VIA_TABLEP1C_PHYREG_VIA_TABLE */
}

#if defined(BCMDBG_PHYREGS_TRACE)
void
phy_utils_mod_phyreg_p1c_debug(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	const char *reg_name)
{
	/* Check for valid phy address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for phy rev %d\n", pi->sh->unit,
		          reg_name, pi->pubpi->phy_rev));
		ASSERT(addr != INVALID_ADDRESS);
	} else {
		phy_utils_mod_phyreg_p1c(pi, addr, mask, val);
	}
}
#endif /* BCMDBG_PHYREGS_TRACE */

#if defined(BCMDBG_PHYREGS_TRACE)
void
phy_utils_mod_phyreg_debug(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	const char *reg_name)
{
	/* Check for valid phy address access */
	if (addr == INVALID_ADDRESS) {
		PHY_ERROR(("wl%d: Reg \"%s\" invalid for phy rev %d\n", pi->sh->unit,
		          reg_name, pi->pubpi->phy_rev));
		ASSERT(addr != INVALID_ADDRESS);
	} else {
		phy_utils_mod_phyreg(pi, addr, mask, val);
	}
}
#endif /* BCMDBG_PHYREGS_TRACE */

void
phy_utils_gen_phyreg(phy_info_t *pi, uint16 addr, uint16 mask, uint16 val,
	uint16* orig_reg_addr, uint16* orig_reg_data,
	uint16* updated_reg_addr, uint16* updated_reg_data)
{
	*orig_reg_addr = addr;
	*orig_reg_data = phy_utils_read_phyreg(pi, addr);

	*updated_reg_addr = addr;
	*updated_reg_data = (((*orig_reg_data) & ~mask) | (val & mask));
}

/* Do the initial table address write given the phy specific table access register
 * locations, the table ID and offset to start read/write operations
 */
void
phy_utils_write_phytable_addr(phy_info_t *pi, uint tbl_id, uint tbl_offset,
                   uint16 tblAddr, uint16 tblDataHi, uint16 tblDataLo)
{
	PHY_TRACE(("wl%d: %s ID %u offset %u\n", pi->sh->unit, __FUNCTION__, tbl_id, tbl_offset));

	phy_utils_write_phyreg(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	pi->tbl_data_hi = tblDataHi;
	pi->tbl_data_lo = tblDataLo;
}

/* Write the given value to the phy table which has been set up with an earlier call
 * to phy_utils_write_phytable_addr()
 */
void
phy_utils_write_phytable_data(phy_info_t *pi, uint width, uint32 val)
{
	ASSERT((width == 8) || (width == 16) || (width == 32));

	PHY_TRACE(("wl%d: %s width %u val %u\n", pi->sh->unit, __FUNCTION__, width, val));

	if (width == 32) {
		/* width is 32-bit */
		phy_utils_write_phyreg(pi, pi->tbl_data_hi, (uint16)(val >> 16));
		phy_utils_write_phyreg(pi, pi->tbl_data_lo, (uint16)val);
	} else {
		/* width is 16-bit or 8-bit */
		phy_utils_write_phyreg(pi, pi->tbl_data_lo, (uint16)val);
	}
}

/* Takes the table name, list of entries, offset to load the table,
 * see xxxphyprocs.tcl, proc xxxphy_write_table
 */
void
phy_utils_write_phytable(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	const uint8  *ptbl_8b    = (const uint8  *)ptbl_info->tbl_ptr;
	const uint16 *ptbl_16b   = (const uint16 *)ptbl_info->tbl_ptr;
	const uint32 *ptbl_32b   = (const uint32 *)ptbl_info->tbl_ptr;

	ASSERT((tbl_width == 8) || (tbl_width == 16) ||
		(tbl_width == 32));

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	phy_utils_write_phyreg(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {
		if (tbl_width == 32) {
			/* width is 32-bit */
			phy_utils_write_phyreg(pi, tblDataHi, (uint16)(ptbl_32b[idx] >> 16));
			phy_utils_write_phyreg(pi, tblDataLo, (uint16)ptbl_32b[idx]);
		} else if (tbl_width == 16) {
			/* width is 16-bit */
			phy_utils_write_phyreg(pi, tblDataLo, ptbl_16b[idx]);
		} else {
			/* width is 8-bit */
			phy_utils_write_phyreg(pi, tblDataLo, ptbl_8b[idx]);
		}
	}
}

void
phy_utils_read_phytable(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblAddr,
	uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	uint8  *ptbl_8b    = (uint8  *)(uintptr)ptbl_info->tbl_ptr;
	uint16 *ptbl_16b   = (uint16 *)(uintptr)ptbl_info->tbl_ptr;
	uint32 *ptbl_32b   = (uint32 *)(uintptr)ptbl_info->tbl_ptr;

	ASSERT((tbl_width == 8) || (tbl_width == 16) ||
		(tbl_width == 32) || (tbl_width == 64));

	phy_utils_write_phyreg(pi, tblAddr, (tbl_id << 10) | tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {
		if (tbl_width == 32) {
			/* width is 32-bit */
			ptbl_32b[idx]  =  phy_utils_read_phyreg(pi, tblDataLo);
			ptbl_32b[idx] |= (phy_utils_read_phyreg(pi, tblDataHi) << 16);
		} else if (tbl_width == 16) {
			/* width is 16-bit */
			ptbl_16b[idx]  =  phy_utils_read_phyreg(pi, tblDataLo);
		} else {
			/* width is 8-bit */
			ptbl_8b[idx]   =  (uint8)phy_utils_read_phyreg(pi, tblDataLo);
		}
	}
}

/* Extended table write feature introduced with ACPHY */
/* Takes the table name, list of entries, offset to load the table,
 * see xxxphyprocs.tcl, proc xxxphy_write_table
 */
void
phy_utils_write_phytable_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint 	idx48, idx60, word_idx;
	uint32  data32;
	uint16  data16;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	const uint8  *ptbl_8b    = (const uint8  *)ptbl_info->tbl_ptr;
	const uint16 *ptbl_16b   = (const uint16 *)ptbl_info->tbl_ptr;
	const uint32 *ptbl_32b   = (const uint32 *)ptbl_info->tbl_ptr;
	ASSERT((tbl_width == 8) || (tbl_width == 16) || (tbl_width == 32) ||
		(tbl_width == 42) || (tbl_width == 48) || (tbl_width == 60) || (tbl_width == 64));

	PHY_TRACE(("wl%d: %s ID %u offset %u\n", pi->sh->unit, __FUNCTION__, tbl_id, tbl_offset));

	phy_utils_write_phyreg(pi, tblId, (uint16) tbl_id);
	phy_utils_write_phyreg(pi, tblOffset, (uint16) tbl_offset);

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {
		switch (tbl_width) {
		case 60:
		case 64:
			/* width is 60/64 bits */

			/* Here the data is accessed with a 32-bit pointer. The lowest 32-bit word
			   is at the lowest memory address.

			   The assumption is that the data buffer passed in is (2 * tbl_len)
			   where tbl_len is the number of table entries to write
			*/
			idx60 = 2 * idx;
			for (word_idx = 0; word_idx < 2; word_idx++) {
				data32 = ptbl_32b[idx60+word_idx];

				data16 = (uint16) (data32 & 0xFFFF);
				if (word_idx == 0) {
					phy_utils_write_phyreg(pi, tblDataWide, data16);
				} else {
					phy_utils_write_phyreg_wide(pi, data16);
				}

				data16 = (uint16) ((data32 >> 16) & 0xFFFF);
				phy_utils_write_phyreg_wide(pi, data16);
			}
			break;

		case 42:
		case 48:
			/* width is 48-bit */

			/* Here the data is accessed with a 16-bit pointer. The lowest 16-bit word
			   is at the lowest memory address.

			   The assumption is that the data buffer passed in is (3 * tbl_len)
			   where tbl_len is the number of table entries to write
			*/
			idx48 = 3 * idx;
			for (word_idx = 0; word_idx < 3; word_idx++) {
				if (word_idx == 0) {
					phy_utils_write_phyreg(pi, tblDataWide,
					    (uint16)(ptbl_16b[idx48+word_idx]));
				} else {
					phy_utils_write_phyreg_wide(pi,
					    (uint16)(ptbl_16b[idx48+word_idx]));
				}
			}
			break;

		case 32:
			/* width is 32-bit */
			phy_utils_write_phyreg(pi, tblDataHi, (uint16)(ptbl_32b[idx] >> 16));
			phy_utils_write_phyreg(pi, tblDataLo, (uint16)ptbl_32b[idx]);
			break;

		case 16:
			/* width is 16-bit */
			phy_utils_write_phyreg(pi, tblDataLo, ptbl_16b[idx]);
			break;

		case 8:
			/* width is 8-bit */
			phy_utils_write_phyreg(pi, tblDataLo, ptbl_8b[idx]);
		}
	}
}

/* Extended table read feature introduced with ACPHY */
void
phy_utils_read_phytable_ext(phy_info_t *pi, const phytbl_info_t *ptbl_info, uint16 tblId,
    uint16 tblOffset, uint16 tblDataWide, uint16 tblDataHi, uint16 tblDataLo)
{
	uint    idx;
	uint 	idx48, idx60, word_idx;
	uint32  data32;
	uint16  data16, u16temp = 0;
	uint    tbl_id     = ptbl_info->tbl_id;
	uint    tbl_offset = ptbl_info->tbl_offset;
	uint	tbl_width = ptbl_info->tbl_width;
	uint8  *ptbl_8b    = (uint8  *)(uintptr)ptbl_info->tbl_ptr;
	uint16 *ptbl_16b   = (uint16 *)(uintptr)ptbl_info->tbl_ptr;
	uint32 *ptbl_32b   = (uint32 *)(uintptr)ptbl_info->tbl_ptr;

	ASSERT((tbl_width == 8) || (tbl_width == 16) ||
		(tbl_width == 32) || (tbl_width == 48) || (tbl_width == 60) || (tbl_width == 42));

	phy_utils_write_phyreg(pi, tblId, (uint16) tbl_id);
	phy_utils_write_phyreg(pi, tblOffset, (uint16) tbl_offset);

	if (tbl_id == 20) {
		if (ISACPHY(pi) && (ACREV_IS(pi->pubpi->phy_rev, 3) ||
		     (ACREV_GE(pi->pubpi->phy_rev, 6) && ACREV_LE(pi->pubpi->phy_rev, 11)) ||
		     (ACREV_GE(pi->pubpi->phy_rev, 13) && ACREV_LE(pi->pubpi->phy_rev, 15)) ||
		     ACREV_IS(pi->pubpi->phy_rev, 18) || ACREV_IS(pi->pubpi->phy_rev, 36))) {
			u16temp = phy_utils_read_phyreg(pi,
				ACPHY_fineclockgatecontrol(pi->pubpi->phy_rev));
			phy_utils_mod_phyreg(pi, ACPHY_fineclockgatecontrol(pi->pubpi->phy_rev),
				ACPHY_fineclockgatecontrol_forceRfSeqgatedClksOn_MASK
				(pi->pubpi->phy_rev),
				0x01 << ACPHY_fineclockgatecontrol_forceRfSeqgatedClksOn_SHIFT
				(pi->pubpi->phy_rev));
		}
	}

	if (tbl_id == 71) {
		/* In case of epsilon table for 43012 the DAC clock needs to be enabled */
		if (ACREV_IS(pi->pubpi->phy_rev, 36)) {
			/* The sequence below has to be followed for dac_clk reg write */
			//wlc_phy_force_dac_clk_from_bbpll(pi, 1, &orig_dac_clk_pu);
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       1);
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       3);
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       7);
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       5);
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       4);
		} else if (ACREV_IS(pi->pubpi->phy_rev, 44)) {
			phy_utils_write_phyreg(pi, ACPHY_papdEpsilonTable0(pi->pubpi->phy_rev),
			                       0);
		}
	}

	if (tbl_id == 103 && ACREV_IS(pi->pubpi->phy_rev, 44)) {
		phy_utils_write_phyreg(pi, ACPHY_papdEpsilonTable1(pi->pubpi->phy_rev),
		                       0);
	}

	for (idx = 0; idx < ptbl_info->tbl_len; idx++) {
		switch (tbl_width) {
		case 60:
		case 64:
			/* width is 60/64 bits */

			/* Here the data is accessed with a 32-bit pointer. The lowest 32-bit word
			   is at the lowest memory address.

			   The assumption is that the data buffer passed in is (2 * tbl_len)
			   where tbl_len is the number of table entries to write
			*/
			idx60 = 2 * idx;
			for (word_idx = 0; word_idx < 2; word_idx++) {
				if (word_idx == 0) {
					data16 = phy_utils_read_phyreg(pi, tblDataWide);
				} else {
					data16 = phy_utils_read_phyreg_wide(pi);
				}
				data32 = (uint32) phy_utils_read_phyreg_wide(pi);

				data32 = (data32 << 16) | (uint32)data16;

				ptbl_32b[idx60+word_idx] = data32;
			}

			break;
		case 42:
		case 48:
			/* width is 48-bit */

			/* Here the data is accessed with a 16-bit pointer. The lowest 16-bit word
			   is at the lowest memory address.

			   The assumption is that the data buffer passed in is (3 * tbl_len)
			   where tbl_len is the number of table entries to read
			*/
			idx48 = 3 * idx;
			for (word_idx = 0; word_idx < 3; word_idx++) {
				if (word_idx == 0) {
					ptbl_16b[idx48+word_idx] = phy_utils_read_phyreg(pi,
						tblDataWide);
				} else {
					ptbl_16b[idx48+word_idx] = phy_utils_read_phyreg_wide(pi);
				}
			}
			break;

		case 32:
			/* width is 32-bit */
			ptbl_32b[idx]  =  phy_utils_read_phyreg(pi, tblDataLo);
			ptbl_32b[idx] |= (phy_utils_read_phyreg(pi, tblDataHi) << 16);
			break;

		case 16:
			/* width is 16-bit */
			ptbl_16b[idx]  =  phy_utils_read_phyreg(pi, tblDataLo);
			break;

		case 8:
			/* width is 8-bit */
			ptbl_8b[idx]   =  (uint8)phy_utils_read_phyreg(pi, tblDataLo);
		}
	}
	if (tbl_id == 20) {
		if (ISACPHY(pi) && (ACREV_IS(pi->pubpi->phy_rev, 3) ||
		     (ACREV_GE(pi->pubpi->phy_rev, 6) && ACREV_LE(pi->pubpi->phy_rev, 11)) ||
		     (ACREV_GE(pi->pubpi->phy_rev, 13) && ACREV_LE(pi->pubpi->phy_rev, 15)) ||
		     ACREV_IS(pi->pubpi->phy_rev, 18) || ACREV_IS(pi->pubpi->phy_rev, 36))) {
			phy_utils_write_phyreg(pi, ACPHY_fineclockgatecontrol(pi->pubpi->phy_rev),
			                       u16temp);
		}
	}
	if (tbl_id == 71) {
		if (ACREV_IS(pi->pubpi->phy_rev, 36)) {
			phy_utils_write_phyreg(pi, ACPHY_dac_clock_from_bbpll(pi->pubpi->phy_rev),
			                       0);
		} else if (ACREV_IS(pi->pubpi->phy_rev, 44)) {
			phy_utils_write_phyreg(pi, ACPHY_papdEpsilonTable0(pi->pubpi->phy_rev),
			                       1);
		}
	}
	if (tbl_id == 103 && ACREV_IS(pi->pubpi->phy_rev, 44)) {
		phy_utils_write_phyreg(pi, ACPHY_papdEpsilonTable1(pi->pubpi->phy_rev),
		                       1);
	}
}

void
phy_utils_read_common_phytable(phy_info_t *pi, uint32 tbl_id,
	const void *tbl_ptr, uint32 tbl_len, uint32 tbl_width,
	uint32 tbl_offset,
	void (*tbl_rfunc)(phy_info_t *, phytbl_info_t *))
{
	phytbl_info_t tab;
	tab.tbl_id = tbl_id;
	tab.tbl_ptr = tbl_ptr;	/* ptr to buf */
	tab.tbl_len = tbl_len;			/* # values   */
	tab.tbl_width = tbl_width;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = tbl_offset;		/* tbl offset */
	tbl_rfunc(pi, &tab);
}

void
phy_utils_write_common_phytable(phy_info_t *pi, uint32 tbl_id, const void *tbl_ptr,
	uint32 tbl_len, uint32 tbl_width, uint32 tbl_offset,
	void (*tbl_wfunc)(phy_info_t *, const phytbl_info_t *))
{

	phytbl_info_t tab;
	tab.tbl_id = tbl_id;
	tab.tbl_ptr = tbl_ptr;	/* ptr to buf */
	tab.tbl_len = tbl_len;			/* # values   */
	tab.tbl_width = tbl_width;			/* gain_val_tbl_rev3 */
	tab.tbl_offset = tbl_offset;		/* tbl offset */
	tbl_wfunc(pi, &tab);
}

/* Generic routines to read and write sets of phy regs */
/* Callers to make sure the clocks are on, memory for the save/restore space is valid */

typedef enum access_phy_or_radio {
	ACCESS_PHY = 0,
	ACCESS_RADIO = 1
} access_phy_or_radio;

/*
 * regs_bulkread - Table driven read of multiple PHY or RADIO registers
 *
 * Parameters:
 *   pi           - Phy Info for the call to read_XXX_reg()
 *   addrvals     - A series of uint16: addr1, value1, addr2, value2, ..., addrN, valueN
 *                  The register contents are written into the "valueX" elements.
 *   nregs        - Number of registers to read.
 *                  There are this many addresses *plus* this many values in the array parameter.
 *   which        - Whether reading PHY or RADIO registers
 */
static void
regs_bulkread(phy_info_t *pi, uint16 *addrvals, uint32 nregs, access_phy_or_radio which)
{
	/* Pointer to step through array of address/value pairs. */
	uint16 * reginfop = addrvals;

	/*
	 * Use a function pointer to choose between
	 * access PHY or RADIO registers.
	 */
	uint16 (*readfp)(phy_info_t*, uint16) = 0;

	if (which == ACCESS_PHY)
		readfp = phy_utils_read_phyreg;
	else
		readfp = phy_utils_read_radioreg;

	/*
	 * Read the registers using address from the array parameters,
	 * and store the results back into the array.
	 */
	while (reginfop < addrvals + (2 * nregs)) {
		reginfop[1] = readfp(pi, reginfop[0]);
		reginfop += 2;
	}
}

/*
 * regs_bulkwrite - Table driven write of multiple PHY registers
 *
 * Parameters:
 *   pi           - Phy Info for the call to phy_utils_read_phyreg()
 *   addrvals     - A series of uint16: addr1, value1, addr2, value2, ..., addrN, valueN
 *                  The register contents are written into the "valueX" elements.
 *   nregs        - Number of registers to read.
 *                  There are this many addresses *plus* this many values in the array parameter.
 *   which        - Whether reading phy or radio registers (0=phy, 1=radio)
 */
static void
regs_bulkwrite(phy_info_t *pi, const uint16 *addrvals, uint32 nregs, access_phy_or_radio which)
{
	/* Pointer to step through array of address/value pairs. */
	const uint16 *reginfop = addrvals;

	/*
	 * Use a function pointer to choose between
	 * accessing PHY or RADIO registers.
	 */
	void (*writefp)(phy_info_t*, uint16 addr, uint16 value) = 0;

	if (which == ACCESS_PHY)
		writefp = phy_utils_write_phyreg;
	else
		writefp = phy_utils_write_radioreg;

	while (reginfop < addrvals + (2 * nregs)) {
		writefp(pi, reginfop[0], reginfop[1]);
		reginfop += 2;
	}
}

void
phy_utils_bulkread_phyregs(phy_info_t *pi, uint16 *addrvals, uint32 nregs)
{
	regs_bulkread(pi, addrvals, nregs, ACCESS_PHY);
}

void
phy_utils_bulkwrite_phyregs(phy_info_t *pi, const uint16 *addrvals, uint32 nregs)
{
	regs_bulkwrite(pi, addrvals, nregs, ACCESS_PHY);
}

void
phy_utils_bulkread_radioregs(phy_info_t *pi, uint16 *addrvals, uint32 nregs)
{
	regs_bulkread(pi, addrvals, nregs, ACCESS_RADIO);
}

void
phy_utils_bulkwrite_radioregs(phy_info_t *pi, const uint16 *addrvals, uint32 nregs)
{
	regs_bulkwrite(pi, addrvals, nregs, ACCESS_RADIO);
}

void
phy_utils_bulkmod_phyregs(phy_info_t *pi, uint16 *regs, uint16 *mask, uint16 *val, uint32 nregs)
{
	uint32 i;

	for (i = 0; i < nregs; i++)
		phy_utils_mod_phyreg(pi, regs[i], mask[i], val[i]);
}

static void
wlc_phy_conditional_assert(phy_info_t *pi)
{
	BCM_REFERENCE(pi);

#ifdef CHECK_MACSUSPEND_REGACCESS
	ASSERT(!(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC));
	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
#endif // endif

#ifdef PHYWAR_43012_CRWLDOT11M_2177
	if ((R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC)) {
		ROMMABLE_ASSERT(0);
		PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));
	}
#endif // endif
}

/* direct phy register read without pi structure
 * this can be used at preattach or so
 */
uint16
phy_utils_read_phyreg_nopi(prephy_info_t *pi, d11regs_t *regs, uint16 addr)
{
	W_REG(pi->sh->osh, D11_PHY_REG_ADDR_ALTBASE(regs, pi->regoffsets), addr);
#ifdef __mips__
	(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR_ALTBASE(regs, pi->regoffsets));
#endif // endif
	return (R_REG(pi->sh->osh, D11_PHY_REG_DATA_ALTBASE(regs, pi->regoffsets)));
}

/* direct phy register write without pi structure
 * this can be used at preattach or so
 */
void
phy_utils_write_phyreg_nopi(prephy_info_t *pi, d11regs_t *regs, uint16 addr, uint16 val)
{
#ifdef __mips__
	W_REG(pi->sh->osh, D11_PHY_REG_ADDR_ALTBASE(regs, pi->regoffsets), addr);
	(void)R_REG(pi->sh->osh, D11_PHY_REG_ADDR_ALTBASE(regs, pi->regoffsets));
	W_REG(OSH_NULL, D11_PHY_REG_DATA_ALTBASE(regs, pi->regoffsets), val);
	if (addr == NPHY_TableAddress)
		(void)R_REG(pi->sh->osh, D11_PHY_REG_DATA_ALTBASE(regs, pi->regoffsets));
#else
	W_REG(pi->sh->osh,
		(volatile uint32 *)(uintptr)(D11_PHY_REG_ADDR_ALTBASE(regs, pi->regoffsets)),
		addr | (val << 16));
#endif /* __mips__ */
}
