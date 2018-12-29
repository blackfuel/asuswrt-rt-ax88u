/*
 * Chip-specific hardware definitions for
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: d11.h 767725 2018-09-25 01:10:34Z $
 */

#ifndef	_D11_H
#define	_D11_H

/*
 * Notes:
 * 1. pre40/pre rev40: corerev < 40
 * 2. pre80/pre rev80: 40 <= corerev < 80
 * 3. rev40/D11AC: 80 > corerev >= 40
 * 4. rev80: 128 > corerev >= 80
 * 5. rev128: corerev >= 128
 */

#include <typedefs.h>
#include <hndsoc.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <802.11.h>

#if defined(BCMDONGLEHOST)|| defined(WL_UNITTEST)
typedef struct {
	uint32 pad;
} shmdefs_t;

#else	/* defined(BCMDONGLEHOST)|| defined(WL_UNITTEST) */
#include <d11shm.h>

#endif /* !defined(BCMDONGLEHOST)|| !defined(WL_UNITTEST) */

#include <d11regs.h>

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/* cpp contortions to concatenate w/arg prescan */
#ifndef	PAD
#define	_PADLINE(line)	pad ## line
#define	_XSTR(line)	_PADLINE(line)
#define	PAD		_XSTR(__LINE__)
#endif // endif

#define NBITSZ(name)	((name##_LB) - (name##_NBIT) + 1)

#define	D11AC_BCN_TMPL_LEN	640	/**< length of the BCN template area for 11AC */
#define	BCN_TMPL_LEN		512	/**< length of the BCN template area */
#define LPRS_TMPL_LEN		512	/**< length of the legacy PRS template area */

#ifndef NFIFO_EXT_MAX
#define NFIFO_EXT_MAX		NFIFO_EXT_REV128		/* Largest FIFO count */
#endif // endif
#define TX_FIFO_BITMAP_SIZE_MAX	((NFIFO_EXT_MAX+NBBY-1)/NBBY)	/* Largest FIFO bitmap size */

/* Offset of SU (legacy) TX FIFOs */
#define TX_FIFO_SU_OFFSET_REV128	64
#define TX_FIFO_SU_OFFSET_REV130	32
#define TX_FIFO_SU_OFFSET(corerev) \
	(D11REV_GE((corerev), 130) ? TX_FIFO_SU_OFFSET_REV130 : \
	(D11REV_GE((corerev), 128) ? TX_FIFO_SU_OFFSET_REV128 : \
				     0))

#define NFIFO_EXT_REV130	38	/* embedded 2x2 AX core */
#define NFIFO_EXT_REV128	70

#define NFIFO_EXT_REV64		32
#define	NFIFO_LEGACY		6	/* Legacy FIFO count */
#define NFIFO_LEGACY_MASK	0x3f

#define NFIFO_EXT_TRIGGERQ	10	/* FIFOs used with 11ax triggerQ DMA */

/* RX FIFO numbers */
#define	RX_FIFO			0	/**< data and ctl frames */
#define	RX_FIFO1		1	/**< ctl frames */
#define RX_FIFO2		2	/**< ctl frames */
#define STS_FIFO		3	/**< phyrxstatus frames */
#define RX_FIFO_NUMBER		4
#define	RX_TXSTATUS_FIFO	3	/**< RX fifo for tx status packages */

/* TX FIFO numbers using WME Access Classes */
#define	TX_AC_BK_FIFO		0	/**< Access Category Background TX FIFO */
#define	TX_AC_BE_FIFO		1	/**< Access Category Best-Effort TX FIFO */
#define	TX_AC_VI_FIFO		2	/**< Access Class Video TX FIFO */
#define	TX_AC_VO_FIFO		3	/**< Access Class Voice TX FIFO */
#define	TX_BCMC_FIFO		4	/**< Broadcast/Multicast TX FIFO */
#define	TX_ATIM_FIFO		5	/**< TX fifo for ATIM window info */

/* TX FIFO numbers for trigger queues for HE STA only chips (i.e
 * This is valid only for 4369 or similar STA chips that supports
 * a single HE STA connection.
 */
#define	TX_TRIG_BK_FIFO		6	/**< Access Category Background TX FIFO */
#define	TX_TRIG_BE_FIFO		7	/**< Access Category Best-Effort TX FIFO */
#define	TX_TRIG_VI_FIFO		8	/**< Access Class Video TX FIFO */
#define	TX_TRIG_VO_FIFO		9	/**< Access Class Voice TX FIFO */

/** Legacy TX FIFO numbers */
#define	TX_DATA_FIFO		TX_AC_BE_FIFO
#define	TX_CTL_FIFO		TX_AC_VO_FIFO

/* Extended FIFOs for corerev >= 64 */
#define TX_FIFO_6		6
#define TX_FIFO_7		7
#define TX_FIFO_16		16
#define TX_FIFO_23		23
#define TX_FIFO_25		25

#define TX_FIFO_EXT_START	6	/* Index at which extended FIFOs start */
#define TX_FIFO_MU_START	8	/* Starting index of 11ac MU FIFOs */
#define TX_FIFO_HE_MU_START	6	/* Starting index of 11ax MU FIFOs */

#define D11REG_IHR_WBASE	0x200
#define D11REG_IHR_BASE		(D11REG_IHR_WBASE << 1)

#define	PIHR_BASE	0x0400		/**< byte address of packed IHR region */

/* biststatus */
#define	BT_DONE		(1U << 31)	/**< bist done */
#define	BT_B2S		(1 << 30)	/**< bist2 ram summary bit */

/* DMA intstatus and intmask */
#define	I_PC		(1 << 10)	/**< pci descriptor error */
#define	I_PD		(1 << 11)	/**< pci data error */
#define	I_DE		(1 << 12)	/**< descriptor protocol error */
#define	I_RU		(1 << 13)	/**< receive descriptor underflow */
#define	I_RO		(1 << 14)	/**< receive fifo overflow */
#define	I_XU		(1 << 15)	/**< transmit fifo underflow */
#define	I_RI		(1 << 16)	/**< receive interrupt */
#define	I_XI		(1 << 24)	/**< transmit interrupt */

/* interrupt receive lazy */
#define	IRL_TO_MASK		0x00ffffff	/**< timeout */
#define	IRL_FC_MASK		0xff000000	/**< frame count */
#define	IRL_FC_SHIFT		24		/**< frame count */
#define	IRL_DISABLE		0x01000000	/**< Disabled value: int on 1 frame, zero time */

/* maccontrol register */
#define	MCTL_GMODE		(1U << 31)
#define	MCTL_DISCARD_PMQ	(1 << 30)
#define	MCTL_DISCARD_TXSTATUS	(1 << 29)
#define	MCTL_TBTT_HOLD		(1 << 28)
#define	MCTL_CLOSED_NETWORK	(1 << 27)
#define	MCTL_WAKE		(1 << 26)
#define	MCTL_HPS		(1 << 25)
#define	MCTL_PROMISC		(1 << 24)
#define	MCTL_KEEPBADFCS		(1 << 23)
#define	MCTL_KEEPCONTROL	(1 << 22)
#define	MCTL_PHYLOCK		(1 << 21)
#define	MCTL_BCNS_PROMISC	(1 << 20)
#define	MCTL_LOCK_RADIO		(1 << 19)
#define	MCTL_AP			(1 << 18)
#define	MCTL_INFRA		(1 << 17)
#define	MCTL_BIGEND		(1 << 16)
#define	MCTL_GPOUT_SEL_MASK	(3 << 14)
#define	MCTL_GPOUT_SEL_SHIFT	14
#define	MCTL_EN_PSMDBG		(1 << 13)
#define	MCTL_IHR_EN		(1 << 10)
#define	MCTL_SHM_UPPER		(1 <<  9)
#define	MCTL_SHM_EN		(1 <<  8)
#define	MCTL_PSM_JMP_0		(1 <<  2)
#define	MCTL_PSM_RUN		(1 <<  1)
#define	MCTL_EN_MAC		(1 <<  0)

#if defined(WL_PSMR1)
/* Mask of valid MCTL bits */
#define MCTL_PSMR1_MASK (\
		MCTL_GMODE | MCTL_DISCARD_PMQ | \
		MCTL_CLOSED_NETWORK | MCTL_WAKE | MCTL_HPS | \
		MCTL_PROMISC | MCTL_KEEPBADFCS | MCTL_KEEPCONTROL | \
		MCTL_AP | MCTL_INFRA | MCTL_BIGEND | \
		MCTL_EN_PSMDBG | MCTL_IHR_EN | MCTL_SHM_UPPER | \
		MCTL_SHM_EN | MCTL_PSM_JMP_0 | MCTL_PSM_RUN)
#endif /* defined(WL_PSMR1) */

/* maccontrol1 register */
#define	MCTL1_GCPS		0x00000001
#define MCTL1_EGS_MASK		0x0000c000
#define MCTL1_EGS_SHIFT		14
#define MCTL1_AVB_ENABLE	2
#define MCTL1_GPIOSEL_SHIFT	8
#define MCTL1_GPIOSEL_MASK	(0x3F << MCTL1_GPIOSEL_SHIFT)

/* maccontrol2 */
#define MCTL2_INVALID_SCHIDX	-1
#define MCTL2_FIXED_SCHIDX	0
#define MCTL2_TRIVIAL_SCHIDX	1

#define MCTL2_INVALID_SCHPOS	-1
#define MCTL2_SCHPOS_MASK	0x00F0
#define MCTL2_SCHPOS_SHIFT	4
#define MCTL2_SCHIDX_MASK	0x000F
#define MCTL2_SCHIDX_SHIFT	0

/* ucode_features register */
#define UCFEAT_SUBANK_MASK	0xE000
#define UCFEAT_SUBANK_SHIFT	13

/* maccommand register */
#define	MCMD_BCN0VLD		(1 <<  0)
#define	MCMD_BCN1VLD		(1 <<  1)
#define	MCMD_DIRFRMQVAL		(1 <<  2)
#define	MCMD_CCA		(1 <<  3)
#define	MCMD_BG_NOISE		(1 <<  4)
#define	MCMD_SKIP_SHMINIT	(1 <<  5)	/**< only used for simulation */
#define MCMD_SLOWCAL		(1 <<  6)
#define MCMD_SAMPLECOLL		MCMD_SKIP_SHMINIT	/**< reuse for sample collect */
#define MCMD_BCNREL		(1 << 8 )	/**< release anybuffered bcns from ucode  */
#define MCMD_TOF		(1 << 9) /**< wifi ranging processing in ucode for rxd frames */
#define MCMD_TSYNC		(1 << 10) /**< start timestamp sync process in ucode */

/* maccommand_x register */
#define	MCMDX_SND		(1 <<  0)
#define	MCMDX_CLR_MUBF		(1 <<  1)

/* macintstatus/macintmask */
#define	MI_MACSSPNDD     (1 <<  0)	/**< MAC has gracefully suspended */
#define	MI_BCNTPL        (1 <<  1)	/**< beacon template available */
#define	MI_TBTT          (1 <<  2)	/**< TBTT indication */
#define	MI_BCNSUCCESS    (1 <<  3)	/**< beacon successfully tx'd */
#define	MI_BCNCANCLD     (1 <<  4)	/**< beacon canceled (IBSS) */
#define	MI_ATIMWINEND    (1 <<  5)	/**< end of ATIM-window (IBSS) */
#define	MI_PMQ           (1 <<  6)	/**< PMQ entries available */
#define	MI_ALTTFS        (1 <<  7)	/**< TX status interrupt for ARM offloads */
#define	MI_NSPECGEN_1    (1 <<  8)	/**< non-specific gen-stat bits that are set by PSM */
#define	MI_MACTXERR      (1 <<  9)	/**< MAC level Tx error */
#define MI_PMQERR        (1 << 10)
#define	MI_PHYTXERR      (1 << 11)	/**< PHY Tx error */
#define	MI_PME           (1 << 12)	/**< Power Management Event */
#define	MI_GP0           (1 << 13)	/**< General-purpose timer0 */
#define	MI_GP1           (1 << 14)	/**< General-purpose timer1 */
#define	MI_DMAINT        (1 << 15)	/**< (ORed) DMA-interrupts */
#define	MI_TXSTOP        (1 << 16)	/**< MAC has completed a TX FIFO Suspend/Flush */
#define	MI_CCA           (1 << 17)	/**< MAC has completed a CCA measurement */
#define	MI_BG_NOISE      (1 << 18)	/**< MAC has collected background noise samples */
#define	MI_DTIM_TBTT     (1 << 19)	/**< MBSS DTIM TBTT indication */
#define MI_PRQ           (1 << 20)	/**< Probe response queue needs attention */
#define	MI_HEB           (1 << 21)	/**< HEB (Hardware Event Block) interrupt - 11ax cores */
#define	MI_BT_RFACT_STUCK	(1 << 22)	/**< MAC has detected invalid BT_RFACT pin,
						 * valid when rev < 15
						 */
#define MI_TTTT          (1 << 22)	/**< Target TIM Transmission Time,
						 * valid in rev = 26/29, or rev >= 42
						 */
#define	MI_BT_PRED_REQ   (1 << 23)	/**< MAC requested driver BTCX predictor calc */
#define	MI_BCNTRIM_RX	 (1 << 24)	/**< PSM received a partial beacon */
#define MI_P2P           (1 << 25)	/**< WiFi P2P interrupt */
#define MI_DMATX         (1 << 26)	/**< MAC new frame ready */
#define MI_TSSI_LIMIT    (1 << 27)	/**< Tssi Limit Reach, TxIdx=0/127 Interrupt */
#define MI_HWACI_NOTIFY  (1 << 27)	/**< HWACI detects ACI, Apply Mitigation settings */
#define MI_RFDISABLE     (1 << 28)	/**< MAC detected a change on RF Disable input
						 * (corerev >= 10)
						 */
#define	MI_TFS           (1 << 29)	/**< MAC has completed a TX (corerev >= 5) */
#define	MI_BUS_ERROR     (1 << 30)	/**< uCode indicated bus error */
#define MI_PSMX          (1 << 30)	/**< PSMx interrupt (corerev >= 65) */
#define	MI_TO            (1U << 31)	/**< general purpose timeout (corerev >= 3) */

#define MI_RXOV                 MI_NSPECGEN_1   /**< rxfifo overflow interrupt */
#define MIX_AIRIQ        (1 << 16)	/* Air-IQ interrupt */

/* Mac capabilities registers */
/* machwcap for corerev < 128 */
#define	MCAP_TKIPMIC		0x80000000	/**< TKIP MIC hardware present */
#define	MCAP_TKIPPH2KEY		0x40000000	/**< TKIP phase 2 key hardware present */
#define	MCAP_BTCX		0x20000000	/**< BT coexistence hardware and pins present */
#define	MCAP_MBSS		0x10000000	/**< Multi-BSS hardware present */
#define	MCAP_RXFSZ_MASK		0x0ff80000	/**< Rx fifo size in blocks (revid >= 16) */
#define	MCAP_RXFSZ_SHIFT	19
#define	MCAP_NRXQ_MASK		0x00070000	/**< Max Rx queues supported - 1 */
#define	MCAP_NRXQ_SHIFT		16
#define	MCAP_UCMSZ_MASK		0x0000e000	/**< Ucode memory size */
#define	MCAP_UCMSZ_3K3		0		/**< 3328 Words Ucode memory, in unit of 50-bit */
#define	MCAP_UCMSZ_4K		1		/**< 4096 Words Ucode memory */
#define	MCAP_UCMSZ_5K		2		/**< 5120 Words Ucode memory */
#define	MCAP_UCMSZ_6K		3		/**< 6144 Words Ucode memory */
#define	MCAP_UCMSZ_8K		4		/**< 8192 Words Ucode memory */
#define	MCAP_UCMSZ_SHIFT	13
#define	MCAP_TXFSZ_MASK		0x00000ff8	/**< Tx fifo size (* 512 bytes) */
#define	MCAP_TXFSZ_SHIFT	3
#define	MCAP_NTXQ_MASK		0x00000007	/**< Max Tx queues supported - 1 */
#define	MCAP_NTXQ_SHIFT		0

/* machwcap for corerev >= 128 */
#define	MCAP_BTCX_GE128		0x80000000	/**< BT coexistence hardware and pins present */
#define	MCAP_UCMSZ_MASK_GE128	0x001e0000	/**< Ucode memory size */
#define	MCAP_UCMSZ_SHIFT_GE128	17
#define	MCAP_TXFSZ_MASK_GE128	0x0001ff80	/**< Tx fifo size (* 512 bytes) */
#define	MCAP_TXFSZ_SHIFT_GE128	7
#define	MCAP_NTXQ_MASK_GE128	0x0000007f	/**< Max Tx queues supported - 1 */

#define	MCAP_BTCX_SUP(corerev)	((D11REV_GE((corerev), 128)) ? MCAP_BTCX_GE128 : MCAP_BTCX)

/* machwcap for corerev >= 130 */
#define	MCAP_UCMSZ_MASK_GE130	0x003c0000	/**< Ucode memory size */
#define	MCAP_UCMSZ_SHIFT_GE130	18
#define	MCAP_TXFSZ_MASK_GE130	0x0003ff80	/**< Tx fifo size (* 512 bytes) */
#define	MCAP_TXFSZ_SHIFT_GE130	7

#define	MCAP_UCMSZ_TYPES_GE128	9		/**< different Ucode memory size types */

#define MSWCAP_DEAGG_OFF_NBIT	14		/**< turn off rx-deagg, override TKIPPH2 cap */

/* machwcap1 */
#define MCAP1_BTCX_GE130	0x80000000 /**< this field moved from maccap to hwcap1 register */
#define	MCAP1_ERC_MASK		0x00000001	/**< external radio coexistence */
#define	MCAP1_ERC_SHIFT		0
#define	MCAP1_SHMSZ_MASK	0x0000000e	/**< shm size (corerev >= 16) */
#define	MCAP1_SHMSZ_SHIFT	1
#define MCAP1_SHMSZ_1K		0		/**< 1024 words in unit of 32-bit */
#define MCAP1_SHMSZ_2K		1		/**< 1536 words in unit of 32-bit */
#define MCAP1_NUMMACCHAINS	0x00001000 /**< Indicates one less than the number of MAC chains */
#define MCAP1_NUMMACCHAINS_SHIFT	12

/* BTCX control */
#define BTCX_CTRL_EN		0x0001	/**< Enable BTCX module */
#define BTCX_CTRL_SW		0x0002	/**< Enable software override */

#define BTCX_CTRL_PRI_POL	0x0080  /* Invert prisel polarity */

/* BTCX status */
#define BTCX_STAT_RA		0x0001	/**< RF_ACTIVE state */

/* BTCX transaction control */
#define BTCX_TRANS_ANTSEL	0x0040	/**< ANTSEL output */
#define BTCX_TRANS_TXCONF	0x0080	/**< TX_CONF output */

/* pmqhost data */
#define	PMQH_DATA_MASK		0xffff0000	/**< data entry of head pmq entry */
#define	PMQH_BSSCFG		0x00100000	/**< PM entry for BSS config */
#define	PMQH_PMOFF		0x00010000	/**< PM Mode OFF: power save off */
#define	PMQH_PMON		0x00020000	/**< PM Mode ON: power save on */
#define	PMQH_PMPS		0x00200000	/**< PM Mode PRETEND */
#define	PMQH_DASAT		0x00040000	/**< Dis-associated or De-authenticated */
#define	PMQH_ATIMFAIL		0x00080000	/**< ATIM not acknowledged */
#define	PMQH_DEL_ENTRY		0x00000001	/**< delete head entry */
#define	PMQH_DEL_MULT		0x00000002	/**< delete head entry to cur read pointer -1 */
#define	PMQH_OFLO		0x00000004	/**< pmq overflow indication */
#define	PMQH_NOT_EMPTY		0x00000008	/**< entries are present in pmq */

/* phydebug (corerev >= 3) */
#define	PDBG_CRS		(1 << 0)  /**< phy is asserting carrier sense */
#define	PDBG_TXA		(1 << 1)  /**< phy is taking xmit byte from mac this cycle */
#define	PDBG_TXF		(1 << 2)  /**< mac is instructing the phy to transmit a frame */
#define	PDBG_TXE		(1 << 3)  /**< phy is signaling a transmit Error to the mac */
#define	PDBG_RXF		(1 << 4)  /**< phy detected the end of a valid frame preamble */
#define	PDBG_RXS		(1 << 5)  /**< phy detected the end of a valid PLCP header */
#define	PDBG_RXFRG		(1 << 6)  /**< rx start not asserted */
#define	PDBG_RXV		(1 << 7)  /**< mac is taking receive byte from phy this cycle */
#define	PDBG_RFD		(1 << 16) /**< RF portion of the radio is disabled */

/* objaddr register */
#define	OBJADDR_SEL_MASK	0x000F0000
#define	OBJADDR_UCM_SEL		0x00000000
#define	OBJADDR_SHM_SEL		0x00010000
#define	OBJADDR_SCR_SEL		0x00020000
#define	OBJADDR_IHR_SEL		0x00030000
#define	OBJADDR_RCMTA_SEL	0x00040000
#define	OBJADDR_AMT_SEL		0x00040000
#define	OBJADDR_SRCHM_SEL	0x00060000
#define	OBJADDR_AUXPMQ_SEL	0x00070000
#define	OBJADDR_UCMX_SEL	0x00080000
#define	OBJADDR_SHMX_SEL	0x00090000
#define	OBJADDR_SCRX_SEL	0x000a0000
#define	OBJADDR_IHRX_SEL	0x000b0000
#define	OBJADDR_KEYTBL_SEL	0x000c0000
#define OBJADDR_UCM1_SEL	0x00100000
#define OBJADDR_SHM1_SEL	0x00110000
#define OBJADDR_SCR1_SEL	0x00120000
#define OBJADDR_IHR1_SEL	0x00130000
#define	OBJADDR_HEB_SEL		0x00120000
#define	OBJADDR_WINC		0x01000000
#define	OBJADDR_RINC		0x02000000
#define	OBJADDR_AUTO_INC	0x03000000
/* SHM/SCR/IHR/SHMX/SCRX/IHRX allow 2 bytes read/write, else only 4 bytes */
#define	OBJADDR_2BYTES_ACCESS(sel)	\
	(((sel & 0x70000) == OBJADDR_SHM_SEL) || \
	((sel & 0x70000) == OBJADDR_SCR_SEL) || \
	((sel & 0x70000) == OBJADDR_IHR_SEL))

/* pcmaddr bits */
#define	PCMADDR_INC		0x4000
#define	PCMADDR_UCM_SEL		0x0000

#define	WEP_PCMADDR		0x07d4
#define	WEP_PCMDATA		0x07d6

/* frmtxstatus */
#define	TXS_V			(1 << 0)	/**< valid bit */

#define	TXS_STATUS_MASK		0xffff
/* sw mask to map txstatus for corerevs <= 4 to be the same as for corerev > 4 */
#define	TXS_COMPAT_MASK		0x3
#define	TXS_COMPAT_SHIFT	1
#define	TXS_FID_MASK		0xffff0000
#define	TXS_FID_SHIFT		16

/* frmtxstatus2 */
#define	TXS_SEQ_MASK		0xffff
#define	TXS_PTX_MASK		0xffff0000
#define	TXS_PTX_SHIFT		16
#define	TXS_MU_MASK		0x01000000
#define	TXS_MU_SHIFT		24

/* clk_ctl_st, corerev >= 17 */
#define CCS_ERSRC_REQ_D11PLL	0x00000100	/**< d11 core pll request */
#define CCS_ERSRC_REQ_PHYPLL	0x00000200	/**< PHY pll request */
#define CCS_ERSRC_AVAIL_D11PLL	0x01000000	/**< d11 core pll available */
#define CCS_ERSRC_AVAIL_PHYPLL	0x02000000	/**< PHY pll available */

/* d11_pwrctl, corerev16 only */
#define D11_PHYPLL_AVAIL_REQ	0x000010000	/**< request PHY PLL resource */
#define D11_PHYPLL_AVAIL_STS	0x001000000	/**< PHY PLL is available */

/* tsf_cfprep register */
#define	CFPREP_CBI_MASK		0xffffffc0
#define	CFPREP_CBI_SHIFT	6
#define	CFPREP_CFPP		0x00000001

/* transmit fifo control for 2-byte pio */
#define	XFC_BV_MASK		0x3		/**< bytes valid */
#define	XFC_LO			(1 << 0)	/**< low byte valid */
#define	XFC_HI			(1 << 1)	/**< high byte valid */
#define	XFC_BOTH		(XFC_HI | XFC_LO) /**< both bytes valid */
#define	XFC_EF			(1 << 2)	/**< end of frame */
#define	XFC_FR			(1 << 3)	/**< frame ready */
#define	XFC_FL			(1 << 5)	/**< flush request */
#define	XFC_FP			(1 << 6)	/**< flush pending */
#define	XFC_SE			(1 << 7)	/**< suspend request */
#define	XFC_SP			(1 << 8)
#define	XFC_CC_MASK		0xfc00		/**< committed count */
#define	XFC_CC_SHIFT		10

/* transmit fifo control for 4-byte pio */
#define	XFC4_BV_MASK		0xf		/**< bytes valid */
#define	XFC4_EF			(1 << 4)	/**< end of frame */
#define	XFC4_FR			(1 << 7)	/**< frame ready */
#define	XFC4_SE			(1 << 8)	/**< suspend request */
#define	XFC4_SP			(1 << 9)
#define	XFC4_FL			(1 << 10)	/**< flush request */
#define	XFC4_FP			(1 << 11)	/**< flush pending */

/* receive fifo control */
#define	RFC_FR			(1 << 0)	/**< frame ready */
#define	RFC_DR			(1 << 1)	/**< data ready */

/* tx fifo sizes for corerev >= 9 */
/* tx fifo sizes values are in terms of 256 byte blocks */
#define TXFIFOCMD_RESET_MASK	(1 << 15)	/**< reset */
#define TXFIFOCMD_FIFOSEL_SHIFT	8		/**< fifo */
#define TXFIFOCMD_FIFOSEL_SET(val)	((val & 0x7) << TXFIFOCMD_FIFOSEL_SHIFT)	/* fifo */
#define TXFIFOCMD_FIFOSEL_GET(val)	((val >> TXFIFOCMD_FIFOSEL_SHIFT) & 0x7)	/* fifo */
#define TXFIFO_FIFOTOP_SHIFT	8		/**< fifo start */

#define TXFIFO_FIFO_START(def, def1)	((def & 0xFF) | ((def1 & 0xFF) << 8))
#define TXFIFO_FIFO_END(def, def1)	(((def & 0xFF00) >> 8) | (def1 & 0xFF00))

/* Must redefine to 65 for 16 MBSS */
#ifdef WLLPRS
#define TXFIFO_START_BLK16	(65+16)	/**< Base address + 32 * 512 B/P + 8 * 512 11g P */
#else /* WLLPRS */
#define TXFIFO_START_BLK16	65	/**< Base address + 32 * 512 B/P */
#endif /* WLLPRS */
#define TXFIFO_START_BLK	 6	/**< Base address + 6 * 256 B */
#define TXFIFO_START_BLK_NIN 7		/**< Base address + 6 * 256 B */
#define TXFIFO_SIZE_PER_UNIT	256	/**< one unit corresponds to 256 bytes */
#define TXFIFO_AC_SIZE_PER_UNIT	512	/**< one unit corresponds to 512 bytes */

#define MBSS16_TEMPLMEM_MINBLKS	65	/**< one unit corresponds to 256 bytes */

/* phy versions, PhyVersion:Revision field */
#define	PV_AV_MASK		0xf000		/**< analog block version */
#define	PV_AV_SHIFT		12		/**< analog block version bitfield offset */
#define	PV_PT_MASK		0x0f00		/**< phy type */
#define	PV_PT_SHIFT		8		/**< phy type bitfield offset */
#define	PV_PV_MASK		0x00ff		/**< phy version */
#define	PHY_TYPE(v)		((v & PV_PT_MASK) >> PV_PT_SHIFT)

/* phy types, PhyVersion:PhyType field */
#define	PHY_TYPE_A		0	/**< A-Phy value */
#define	PHY_TYPE_B		1	/**< B-Phy value */
#define	PHY_TYPE_G		2	/**< G-Phy value */
#define	PHY_TYPE_N		4	/**< N-Phy value */
/* #define	PHY_TYPE_LP		5 */	/**< LP-Phy value */
/* #define	PHY_TYPE_SSN		6 */	/**< SSLPN-Phy value */
#define	PHY_TYPE_HT		7	/**< 3x3 HTPhy value */
#define	PHY_TYPE_LCN		8	/**< LCN-Phy value */
#define	PHY_TYPE_LCNXN		9	/**< LCNXN-Phy value */
#define	PHY_TYPE_LCN40		10	/**< LCN40-Phy value */
#define	PHY_TYPE_AC		11	/**< AC-Phy value */
#define	PHY_TYPE_LCN20		12	/**< LCN20-Phy value */
#define	PHY_TYPE_HE		13	/**< HE-Phy value */
#define	PHY_TYPE_NULL		0xf	/**< Invalid Phy value */

/* analog types, PhyVersion:AnalogType field */
#define	ANA_11G_018		1
#define	ANA_11G_018_ALL		2
#define	ANA_11G_018_ALLI	3
#define	ANA_11G_013		4
#define	ANA_11N_013		5
#define	ANA_11LP_013		6

/** 802.11a PLCP header def */
typedef struct ofdm_phy_hdr ofdm_phy_hdr_t;
BWL_PRE_PACKED_STRUCT struct ofdm_phy_hdr {
	uint8	rlpt[3];	/**< rate, length, parity, tail */
	uint16	service;
	uint8	pad;
} BWL_POST_PACKED_STRUCT;

#define	D11A_PHY_HDR_GRATE(phdr)	((phdr)->rlpt[0] & 0x0f)
#define	D11A_PHY_HDR_GRES(phdr)		(((phdr)->rlpt[0] >> 4) & 0x01)
#define	D11A_PHY_HDR_GLENGTH(phdr)	(((*((uint32 *)((phdr)->rlpt))) >> 5) & 0x0fff)
#define	D11A_PHY_HDR_GPARITY(phdr)	(((phdr)->rlpt[3] >> 1) & 0x01)
#define	D11A_PHY_HDR_GTAIL(phdr)	(((phdr)->rlpt[3] >> 2) & 0x3f)

/** rate encoded per 802.11a-1999 sec 17.3.4.1 */
#define	D11A_PHY_HDR_SRATE(phdr, rate)		\
	((phdr)->rlpt[0] = ((phdr)->rlpt[0] & 0xf0) | ((rate) & 0xf))
/** set reserved field to zero */
#define	D11A_PHY_HDR_SRES(phdr)		((phdr)->rlpt[0] &= 0xef)
/** length is number of octets in PSDU */
#define	D11A_PHY_HDR_SLENGTH(phdr, length)	\
	(*(uint32 *)((phdr)->rlpt) = *(uint32 *)((phdr)->rlpt) | \
	(((length) & 0x0fff) << 5))
/** set the tail to all zeros */
#define	D11A_PHY_HDR_STAIL(phdr)	((phdr)->rlpt[3] &= 0x03)

#define	D11A_PHY_HDR_LEN_L	3	/**< low-rate part of PLCP header */
#define	D11A_PHY_HDR_LEN_R	2	/**< high-rate part of PLCP header */

#define	D11A_PHY_TX_DELAY	(2)	/**< 2.1 usec */

#define	D11A_PHY_HDR_TIME	(4)	/**< low-rate part of PLCP header */
#define	D11A_PHY_PRE_TIME	(16)
#define	D11A_PHY_PREHDR_TIME	(D11A_PHY_PRE_TIME + D11A_PHY_HDR_TIME)

/** 802.11b PLCP header def */
typedef struct cck_phy_hdr cck_phy_hdr_t;
BWL_PRE_PACKED_STRUCT struct cck_phy_hdr {
	uint8	signal;
	uint8	service;
	uint16	length;
	uint16	crc;
} BWL_POST_PACKED_STRUCT;

#define	D11B_PHY_HDR_LEN	6

#define	D11B_PHY_TX_DELAY	(3)	/**< 3.4 usec */

#define	D11B_PHY_LHDR_TIME	(D11B_PHY_HDR_LEN << 3)
#define	D11B_PHY_LPRE_TIME	(144)
#define	D11B_PHY_LPREHDR_TIME	(D11B_PHY_LPRE_TIME + D11B_PHY_LHDR_TIME)

#define	D11B_PHY_SHDR_TIME	(D11B_PHY_LHDR_TIME >> 1)
#define	D11B_PHY_SPRE_TIME	(D11B_PHY_LPRE_TIME >> 1)
#define	D11B_PHY_SPREHDR_TIME	(D11B_PHY_SPRE_TIME + D11B_PHY_SHDR_TIME)

#define	D11B_PLCP_SIGNAL_LOCKED	(1 << 2)
#define	D11B_PLCP_SIGNAL_LE	(1 << 7)

/* AMPDUXXX: move to ht header file once it is ready: Mimo PLCP */
#define MIMO_PLCP_MCS_MASK	0x7f	/**< mcs index */
#define MIMO_PLCP_40MHZ		0x80	/**< 40 Hz frame */
#define MIMO_PLCP_AMPDU		0x08	/**< ampdu */

#define WLC_GET_CCK_PLCP_LEN(plcp) (plcp[4] + (plcp[5] << 8))
#define WLC_GET_MIMO_PLCP_LEN(plcp) (plcp[1] + (plcp[2] << 8))
#define WLC_SET_MIMO_PLCP_LEN(plcp, len) \
	plcp[1] = len & 0xff; plcp[2] = ((len >> 8) & 0xff);

#define WLC_SET_MIMO_PLCP_AMPDU(plcp) (plcp[3] |= MIMO_PLCP_AMPDU)
#define WLC_CLR_MIMO_PLCP_AMPDU(plcp) (plcp[3] &= ~MIMO_PLCP_AMPDU)
#define WLC_IS_MIMO_PLCP_AMPDU(plcp) (plcp[3] & MIMO_PLCP_AMPDU)

/**
 * The dot11a PLCP header is 5 bytes.  To simplify the software (so that we don't need eg different
 * tx DMA headers for 11a and 11b), the PLCP header has padding added in the ucode.
 */
#define	D11_PHY_HDR_LEN	6

/** For the AC phy PLCP is 12 bytes and not all bytes are used for all the modulations */
#define D11AC_PHY_HDR_LEN	12
#define D11AC_PHY_VHT_PLCP_OFFSET	0
#define D11AC_PHY_HTMM_PLCP_OFFSET	0
#define D11AC_PHY_HTGF_PLCP_OFFSET	3
#define D11AC_PHY_OFDM_PLCP_OFFSET	3
#define D11AC_PHY_CCK_PLCP_OFFSET	6
#define D11AC_PHY_BEACON_PLCP_OFFSET	0

/** For 43684 PHY, RX PLCP is 14 bytes and not all bytes are used for all the modulations */
#define	D11_RXPLCP_LEN_GE128		14
#define D11_PHY_RXPLCP_LEN(rev)		(D11REV_GE(rev, 128) ? D11_RXPLCP_LEN_GE128 : \
					(D11_PHY_HDR_LEN))

/* HT/VHT/HE-SIG-A start from plcp[4] in rev128 */
#define D11_RXPLCP_OFF_GE128		4
#define D11_PHY_RXPLCP_OFF(rev)		(D11REV_GE(rev, 128) ? D11_RXPLCP_OFF_GE128 : 0)

/** TX descriptor - pre40 */
typedef struct d11txh_pre40 d11txh_pre40_t;
BWL_PRE_PACKED_STRUCT struct d11txh_pre40 {
	uint16	MacTxControlLow;		/* 0x0 */
	uint16	MacTxControlHigh;		/* 0x1 */
	uint16	MacFrameControl;		/* 0x2 */
	uint16	TxFesTimeNormal;		/* 0x3 */
	uint16	PhyTxControlWord;		/* 0x4 */
	uint16	PhyTxControlWord_1;		/* 0x5 */
	uint16	PhyTxControlWord_1_Fbr;		/* 0x6 */
	uint16	PhyTxControlWord_1_Rts;		/* 0x7 */
	uint16	PhyTxControlWord_1_FbrRts;	/* 0x8 */
	uint16	MainRates;			/* 0x9 */
	uint16	XtraFrameTypes;			/* 0xa */
	uint8	IV[16];				/* 0x0b - 0x12 */
	uint8	TxFrameRA[6];			/* 0x13 - 0x15 */
	uint16	TxFesTimeFallback;		/* 0x16 */
	uint8	RTSPLCPFallback[6];		/* 0x17 - 0x19 */
	uint16	RTSDurFallback;			/* 0x1a */
	uint8	FragPLCPFallback[6];		/* 0x1b - 1d */
	uint16	FragDurFallback;		/* 0x1e */
	uint16	MModeLen;			/* 0x1f */
	uint16	MModeFbrLen;			/* 0x20 */
	uint16	TstampLow;			/* 0x21 */
	uint16	TstampHigh;			/* 0x22 */
	uint16	ABI_MimoAntSel;			/* 0x23 */
	uint16	PreloadSize;			/* 0x24 */
	uint16	AmpduSeqCtl;			/* 0x25 */
	uint16	TxFrameID;			/* 0x26 */
	uint16	TxStatus;			/* 0x27 */
	uint16	MaxNMpdus;			/* 0x28 corerev >=16 */
	BWL_PRE_PACKED_STRUCT union {
		uint16 MaxAggDur;		/* 0x29 corerev >=16 */
		uint16 MaxAggLen;
	} BWL_POST_PACKED_STRUCT u1;
	BWL_PRE_PACKED_STRUCT union {
		BWL_PRE_PACKED_STRUCT struct {	/* 0x29 corerev >=16 */
			uint8 MaxRNum;
			uint8 MaxAggBytes;	/* Max Agg Bytes in power of 2 */
		} BWL_POST_PACKED_STRUCT s1;
		uint16	MaxAggLen_FBR;
	} BWL_POST_PACKED_STRUCT u2;
	uint16	MinMBytes;			/* 0x2b corerev >=16 */
	uint8	RTSPhyHeader[D11_PHY_HDR_LEN];	/* 0x2c - 0x2e */
	struct	dot11_rts_frame rts_frame;	/* 0x2f - 0x36 */
	uint16	pad;				/* 0x37 */
} BWL_POST_PACKED_STRUCT;

#define	D11_TXH_LEN		112	/**< bytes */

/* Frame Types */
#define FT_LEGACY	(-1)
#define FT_CCK		0
#define FT_OFDM		1
#define FT_HT		2
#define FT_VHT		3
#define FT_HE		4
#define FT_AH		5

/* HE Format */
#define D11_FT_MASK		0x07
#define D11_FT_SHIFT		0
#define	D11_FT(FTFMT)		(FTFMT & D11_FT_MASK)

#define D11_HEFMT_MASK		0x18
#define D11_HEFMT_SHIFT		3
#define	D11_HEFMT(FTFMT)	((FTFMT & D11_HEFMT_MASK) >> D11_HEFMT_SHIFT)

#define HE_FMT_HESU	0	/* HE SU */
#define HE_FMT_HESURT	1	/* HE SU range ext */
#define HE_FMT_HEMU	2	/* HE MU */
#define HE_FMT_HETB	3	/* HE TB */

#define HE_FTFMT_HESU	(FT_HE | (HE_FMT_HESU << D11_HEFMT_SHIFT))	/* HE SU (0x4) */
#define HE_FTFMT_HESURT	(FT_HE | (HE_FMT_HESURT << D11_HEFMT_SHIFT))	/* HE SU range ext (0x6) */
#define HE_FTFMT_HEMU	(FT_HE | (HE_FMT_HEMU << D11_HEFMT_SHIFT))	/* HE MU (0x14) */
#define HE_FTFMT_HETB	(FT_HE | (HE_FMT_HETB << D11_HEFMT_SHIFT))	/* HE TB (0x16) */

/* Position of MPDU inside A-MPDU; indicated with bits 10:9 of MacTxControlLow */
#define TXC_AMPDU_SHIFT		9	/**< shift for ampdu settings */
#define TXC_AMPDU_NONE		0	/**< Regular MPDU, not an A-MPDU */
#define TXC_AMPDU_FIRST		1	/**< first MPDU of an A-MPDU */
#define TXC_AMPDU_MIDDLE	2	/**< intermediate MPDU of an A-MPDU */
#define TXC_AMPDU_LAST		3	/**< last (or single) MPDU of an A-MPDU */

/* MacTxControlLow */
#define TXC_AMIC		0x8000
#define TXC_USERIFS		0x4000
#define TXC_LIFETIME		0x2000
#define	TXC_FRAMEBURST		0x1000
#define	TXC_SENDCTS		0x0800
#define TXC_AMPDU_MASK		0x0600
#define TXC_BW_40		0x0100
#define TXC_FREQBAND_5G		0x0080
#define	TXC_DFCS		0x0040
#define	TXC_IGNOREPMQ		0x0020
#define	TXC_HWSEQ		0x0010
#define	TXC_STARTMSDU		0x0008
#define	TXC_SENDRTS		0x0004
#define	TXC_LONGFRAME		0x0002
#define	TXC_IMMEDACK		0x0001

/* MacTxControlHigh */
#define TXC_PREAMBLE_RTS_FB_SHORT	0x8000	/* RTS fallback preamble type 1 = SHORT 0 = LONG */
#define TXC_PREAMBLE_RTS_MAIN_SHORT	0x4000	/* RTS main rate preamble type 1 = SHORT 0 = LONG */
#define TXC_PREAMBLE_DATA_FB_SHORT	0x2000	/**< Main fallback rate preamble type
					 * 1 = SHORT for OFDM/GF for MIMO
					 * 0 = LONG for CCK/MM for MIMO
					 */
/* TXC_PREAMBLE_DATA_MAIN is in PhyTxControl bit 5 */
#define	TXC_AMPDU_FBR		0x1000	/**< use fallback rate for this AMPDU */
#define	TXC_SECKEY_MASK		0x0FF0
#define	TXC_SECKEY_SHIFT	4
#define	TXC_ALT_TXPWR		0x0008	/**< Use alternate txpwr defined at loc. M_ALT_TXPWR_IDX */
#define	TXC_SECTYPE_MASK	0x0007
#define	TXC_SECTYPE_SHIFT	0

/* Null delimiter for Fallback rate */
#define AMPDU_FBR_NULL_DELIM  5		/**< Location of Null delimiter count for AMPDU */

/* PhyTxControl for Mimophy */
#define	PHY_TXC_PWR_MASK	0xFC00
#define	PHY_TXC_PWR_SHIFT	10
#define	PHY_TXC_ANT_MASK	0x03C0	/**< bit 6, 7, 8, 9 */
#define	PHY_TXC_ANT_SHIFT	6
#define	PHY_TXC_ANT_0_1		0x00C0	/**< auto, last rx */
#define	PHY_TXC_LPPHY_ANT_LAST	0x0000
#define	PHY_TXC_ANT_3		0x0200	/**< virtual antenna 3 */
#define	PHY_TXC_ANT_2		0x0100	/**< virtual antenna 2 */
#define	PHY_TXC_ANT_1		0x0080	/**< virtual antenna 1 */
#define	PHY_TXC_ANT_0		0x0040	/**< virtual antenna 0 */

#define	PHY_TXC_SHORT_HDR	0x0010
#define PHY_TXC_FT_MASK		0x0003
#define	PHY_TXC_FT_CCK		0x0000
#define	PHY_TXC_FT_OFDM		0x0001
#define	PHY_TXC_FT_HT		0x0002
#define	PHY_TXC_FT_VHT		0x0003
#define PHY_TXC_FT_HE		0x0004

#define	PHY_TXC_OLD_ANT_0	0x0000
#define	PHY_TXC_OLD_ANT_1	0x0100
#define	PHY_TXC_OLD_ANT_LAST	0x0300

/** PhyTxControl_1 for Mimophy */
#define PHY_TXC1_BW_MASK		0x0007
#define PHY_TXC1_BW_10MHZ		0
#define PHY_TXC1_BW_10MHZ_UP		1
#define PHY_TXC1_BW_20MHZ		2
#define PHY_TXC1_BW_20MHZ_UP		3
#define PHY_TXC1_BW_40MHZ		4
#define PHY_TXC1_BW_40MHZ_DUP		5
#define PHY_TXC1_MODE_SHIFT		3
#define PHY_TXC1_MODE_MASK		0x0038
#define PHY_TXC1_MODE_SISO		0
#define PHY_TXC1_MODE_CDD		1
#define PHY_TXC1_MODE_STBC		2
#define PHY_TXC1_MODE_SDM		3
#define PHY_TXC1_CODE_RATE_SHIFT	8
#define PHY_TXC1_CODE_RATE_MASK		0x0700
#define PHY_TXC1_CODE_RATE_1_2		0
#define PHY_TXC1_CODE_RATE_2_3		1
#define PHY_TXC1_CODE_RATE_3_4		2
#define PHY_TXC1_CODE_RATE_4_5		3
#define PHY_TXC1_CODE_RATE_5_6		4
#define PHY_TXC1_CODE_RATE_7_8		6
#define PHY_TXC1_MOD_SCHEME_SHIFT	11
#define PHY_TXC1_MOD_SCHEME_MASK	0x3800
#define PHY_TXC1_MOD_SCHEME_BPSK	0
#define PHY_TXC1_MOD_SCHEME_QPSK	1
#define PHY_TXC1_MOD_SCHEME_QAM16	2
#define PHY_TXC1_MOD_SCHEME_QAM64	3
#define PHY_TXC1_MOD_SCHEME_QAM256	4

/* PhyTxControl for HTphy that are different from Mimophy */
#define	PHY_TXC_HTANT_MASK		0x3fC0	/**< bit 6, 7, 8, 9, 10, 11, 12, 13 */
#define	PHY_TXC_HTCORE_MASK		0x03C0	/**< core enable core3:core0, 1=enable, 0=disable */
#define	PHY_TXC_HTCORE_SHIFT		6	/**< bit 6, 7, 8, 9 */
#define	PHY_TXC_HTANT_IDX_MASK		0x3C00	/**< 4-bit, 16 possible antenna configuration */
#define	PHY_TXC_HTANT_IDX_SHIFT		10
#define	PHY_TXC_HTANT_IDX0		0
#define	PHY_TXC_HTANT_IDX1		1
#define	PHY_TXC_HTANT_IDX2		2
#define	PHY_TXC_HTANT_IDX3		3

/* PhyTxControl_1 for HTphy that are different from Mimophy */
#define PHY_TXC1_HTSPARTIAL_MAP_MASK	0x7C00	/**< bit 14:10 */
#define PHY_TXC1_HTSPARTIAL_MAP_SHIFT	10
#define PHY_TXC1_HTTXPWR_OFFSET_MASK	0x01f8	/**< bit 8:3 */
#define PHY_TXC1_HTTXPWR_OFFSET_SHIFT	3

/* XtraFrameTypes */
#define XFTS_RTS_FT_SHIFT	2
#define XFTS_FBRRTS_FT_SHIFT	4
#define XFTS_CHANNEL_SHIFT	8

/** Antenna diversity bit in ant_wr_settle */
#define	PHY_AWS_ANTDIV		0x2000

/* PHY CRS states */
#define	APHY_CRS_RESET			0
#define	APHY_CRS_SEARCH			1
#define	APHY_CRS_CLIP			3
#define	APHY_CRS_G_CLIP_POW1		4
#define	APHY_CRS_G_CLIP_POW2		5
#define	APHY_CRS_G_CLIP_NRSSI1		6
#define	APHY_CRS_G_CLIP_NRSSI1_POW1	7
#define	APHY_CRS_G_CLIP_NRSSI2		8

/* IFS ctl */
#define IFS_USEEDCF	(1 << 2)

/* IFS ctl1 */
#define IFS_CTL1_EDCRS	(1 << 3)
#define IFS_CTL1_EDCRS_20L (1 << 4)
#define IFS_CTL1_EDCRS_40 (1 << 5)
#define IFS_EDCRS_MASK	(IFS_CTL1_EDCRS | IFS_CTL1_EDCRS_20L | IFS_CTL1_EDCRS_40)
#define IFS_EDCRS_SHIFT	3

/* IFS ctl sel pricrs  */
#define IFS_CTL_CRS_SEL_20LL    1
#define IFS_CTL_CRS_SEL_20LU    2
#define IFS_CTL_CRS_SEL_20UL    4
#define IFS_CTL_CRS_SEL_20UU    8
#define IFS_CTL_CRS_SEL_MASK    (IFS_CTL_CRS_SEL_20LL | IFS_CTL_CRS_SEL_20LU | \
				IFS_CTL_CRS_SEL_20UL | IFS_CTL_CRS_SEL_20UU)
#define IFS_CTL_ED_SEL_20LL     (1 << 8)
#define IFS_CTL_ED_SEL_20LU     (1 << 9)
#define IFS_CTL_ED_SEL_20UL     (1 << 10)
#define IFS_CTL_ED_SEL_20UU     (1 << 11)
#define IFS_CTL_ED_SEL_MASK     (IFS_CTL_ED_SEL_20LL | IFS_CTL_ED_SEL_20LU | \
				IFS_CTL_ED_SEL_20UL | IFS_CTL_ED_SEL_20UU)

/* ABI_MimoAntSel */
#define ABI_MAS_ADDR_BMP_IDX_MASK	0x0f00
#define ABI_MAS_ADDR_BMP_IDX_SHIFT	8
#define ABI_MAS_FBR_ANT_PTN_MASK	0x00f0
#define ABI_MAS_FBR_ANT_PTN_SHIFT	4
#define ABI_MAS_MRT_ANT_PTN_MASK	0x000f
#define ABI_MAS_TIMBC_TSF		0x2000	/**< Enable TIMBC tsf field present */

/* MinMBytes */
#define MINMBYTES_PKT_LEN_MASK                  0x0300
#define MINMBYTES_FBRATE_PWROFFSET_MASK         0xFC00
#define MINMBYTES_FBRATE_PWROFFSET_SHIFT        10

/* FrameId */
#define TXFID_FIFO_MASK		0x001F		/* TX FIFO index */
#define TXFID_SEQ_MASK		0x7F00		/* Frame sequence number */
#define TXFID_SEQ_SHIFT		8		/* Frame sequence number shifts */
#define TXFID_MAX_BCMC_FID	((TXFID_SEQ_MASK >> TXFID_SEQ_SHIFT)+1) /* 128 */
#define	TXFID_RATE_PROBE_MASK	0x8000		/* Rate probe */
#define TXFID_RATE_MASK		0x00E0		/* Rate mask */
#define TXFID_RATE_SHIFT	5		/* Rate shifts */

#define WLC_TXFID_SET_QUEUE(fifo)	((fifo) & TXFID_FIFO_MASK)

/* Rev40 template constants */

/** templates include a longer PLCP header that matches the MAC / PHY interface */
#define	D11_VHT_PLCP_LEN	12

/* 11AC TX DMA buffer header */

#define D11AC_TXH_NUM_RATES			4

/** per rate info - rev40 */
typedef struct d11actxh_rate d11actxh_rate_t;
BWL_PRE_PACKED_STRUCT struct d11actxh_rate {
	uint16  PhyTxControlWord_0;             /* 0 - 1 */
	uint16  PhyTxControlWord_1;             /* 2 - 3 */
	uint16  PhyTxControlWord_2;             /* 4 - 5 */
	uint8   plcp[D11_PHY_HDR_LEN];          /* 6 - 11 */
	uint16  FbwInfo;                        /* 12 -13, fall back bandwidth info */
	uint16  TxRate;                         /* 14 */
	uint16  RtsCtsControl;                  /* 16 */
	uint16  Bfm0;                           /* 18 */
} BWL_POST_PACKED_STRUCT;

/* Bit definition for FbwInfo field */
#define FBW_BW_MASK             3
#define FBW_BW_SHIFT            0
#define FBW_TXBF                4
#define FBW_TXBF_SHIFT          2
#define FBW_BFM0_TXPWR_MASK     0x1F8
#define FBW_BFM0_TXPWR_SHIFT    3
#define FBW_BFM_TXPWR_MASK      0x7E00
#define FBW_BFM_TXPWR_SHIFT     9

/* Bit definition for Bfm0 field */
#define BFM0_TXPWR_MASK         0x3f
#define BFM0_STBC_SHIFT         6
#define BFM0_STBC               (1 << BFM0_STBC_SHIFT)
#define D11AC2_BFM0_TXPWR_MASK  0x7f
#define D11AC2_BFM0_STBC_SHIFT  7
#define D11AC2_BFM0_STBC        (1 << D11AC2_BFM0_STBC_SHIFT)

/* per packet info */
typedef struct d11pktinfo_common d11pktinfo_common_t;
typedef struct d11pktinfo_common d11actxh_pkt_t;
BWL_PRE_PACKED_STRUCT struct d11pktinfo_common {
	/* Per pkt info */
	uint16  TSOInfo;                        /* 0 */
	uint16  MacTxControlLow;                /* 2 */
	uint16  MacTxControlHigh;               /* 4 */
	uint16  Chanspec;                       /* 6 */
	uint8   IVOffset;                       /* 8 */
	uint8   PktCacheLen;                    /* 9 */
	uint16  FrameLen;                       /* 10. In [bytes] units. */
	uint16  TxFrameID;                      /* 12 */
	uint16  Seq;                            /* 14 */
	uint16  Tstamp;                         /* 16 */
	uint16  TxStatus;                       /* 18 */
} BWL_POST_PACKED_STRUCT;

/* common cache info between rev40 and rev80 formats */
typedef struct d11txh_cache_common d11txh_cache_common_t;
BWL_PRE_PACKED_STRUCT struct d11txh_cache_common {
	uint8   BssIdEncAlg;                    /* 0 */
	uint8   KeyIdx;                         /* 1 */
	uint8   PrimeMpduMax;                   /* 2 */
	uint8   FallbackMpduMax;                /* 3 */
	uint16  AmpduDur;                       /* 4 - 5 */
	uint8   BAWin;                          /* 6 */
	uint8   MaxAggLen;                      /* 7 */
} BWL_POST_PACKED_STRUCT;

/** Per cache info - rev40 */
typedef struct d11actxh_cache d11actxh_cache_t;
BWL_PRE_PACKED_STRUCT struct d11actxh_cache {
	d11txh_cache_common_t common;		/*  0 -  7 */
	uint8   TkipPH1Key[10];                 /*  8 - 17 */
	uint8   TSCPN[6];                       /* 18 - 23 */
} BWL_POST_PACKED_STRUCT;

/** Long format tx descriptor - rev40 */
typedef struct d11actxh d11actxh_t;
BWL_PRE_PACKED_STRUCT struct d11actxh {
	/* Per pkt info */
	d11actxh_pkt_t	PktInfo;			/* 0 - 19 */

	union {

		/** Rev 40 to rev 63 layout */
		struct {
			/** Per rate info */
			d11actxh_rate_t RateInfo[D11AC_TXH_NUM_RATES];  /* 20 - 99 */

			/** Per cache info */
			d11actxh_cache_t	CacheInfo;                    /* 100 - 123 */
		} rev40;

		/** Rev >= 64 layout */
		struct {
			/** Per cache info */
			d11actxh_cache_t	CacheInfo;                    /* 20 - 43 */

			/** Per rate info */
			d11actxh_rate_t RateInfo[D11AC_TXH_NUM_RATES];  /* 44 - 123 */
		} rev64;

	};

#ifdef WL_EAP_UCODE_TX_DESC
	/* Extend the Tx Descriptor by 48 bytes for EAP */
#ifdef WL_EAP_RTS_OVERRIDE
	d11actxh_rts_override_t rts;        /* 124 - 131 */
#else
	uint8   Pad0[8];
#endif // endif

#ifdef WL_EAP_SAS
	d11actxh_sas_t sas;                 /* 132 - 139 */
#else
	uint8   Pad1[8];
#endif // endif

	/* Bitfield used by EAP features */
	uint16  wlent_flags;                /* 140 - 141 */

#ifdef WL_EAP_PER_PKT_SOUND_PERIOD
	uint16  sounding_period;            /* 142 - 143 */
#else
	uint16  Pad2;
#endif // endif

#ifdef WL_EAP_DESC_KEY
	/* Encryption key for hw to use to encrypt outgoing frame */
	/* Unfortunately, DOT11_MAX_KEY_SIZE is 32 */
	uint8 key[16];                      /* 144 - 159 */
#else
	uint8 Pad3[16];
#endif // endif

#ifdef WL_EAP_PER_PKT_RTX
	/* Retry and fallback limits */
	d11actxh_per_pkt_rtx_t rtx;         /* 160 - 167 */
#else
	uint8 Pad4[8];
#endif // endif

#ifdef WL_EAP_TXQ_PRUNING
	/* TXQ flow ID to be used as bit position in ucode bitmap */
	uint16 txq_flowid;                  /* 168 - 169 */
	uint16 Reserved2;                   /* 170 - 171 */
#else
	uint32 Pad5;
#endif // endif
#endif /* WL_EAP_UCODE_TX_DESC */
} BWL_POST_PACKED_STRUCT;

#define D11AC_TXH_LEN		sizeof(d11actxh_t)	/* 124 bytes */

/* Short format tx descriptor only has per packet info */
#define D11AC_TXH_SHORT_LEN	sizeof(d11actxh_pkt_t)	/* 20 bytes */

/* MacTxControlLow */
#define D11AC_TXC_HDR_FMT_SHORT		0x0001	/**< 0: long format, 1: short format */
#define D11AC_TXC_UPD_CACHE		0x0002
#define D11AC_TXC_CACHE_IDX_MASK	0x003C	/**< Cache index 0 .. 15 */
#define D11AC_TXC_CACHE_IDX_SHIFT	2
/* cache index is not used from rev ge128 */
#define D11REV128_TXC_ENC		0x0020 /**< HW encryption */
#define D11AC_TXC_AMPDU			0x0040	/**< Is aggregate-able */
#define D11AC_TXC_IACK			0x0080	/**< Expect immediate ACK */
#define D11AC_TXC_LFRM			0x0100	/**< Use long/short retry frame count/limit */
#define D11AC_TXC_IPMQ			0x0200	/**< Ignore PMQ */
#define D11AC_TXC_MBURST		0x0400	/**< Burst mode */
#define D11AC_TXC_ASEQ			0x0800	/**< Add ucode generated seq num */
#define D11AC_TXC_AGING			0x1000	/**< Use lifetime */
#define D11AC_TXC_AMIC			0x2000	/**< Compute and add TKIP MIC */
#define D11AC_TXC_STMSDU		0x4000	/**< First MSDU */
#define D11AC_TXC_URIFS			0x8000	/**< Use RIFS */

/* MacTxControlHigh */
#define D11AC_TXC_DISFCS		0x0001	/**< Discard FCS */
#define D11AC_TXC_FIX_RATE		0x0002	/**< Use primary rate only */
#define D11AC_TXC_SVHT			0x0004	/**< Single VHT mpdu ampdu */
#define D11AC_TXC_PPS			0x0008	/**< Enable PS Pretend feature */
#define D11AC_TXC_UCODE_SEQ		0x0010	/* Sequence counter for BK traffic, for offloads */
#define D11AC_TXC_TIMBC_TSF		0x0020	/**< Enable TIMBC tsf field present */
#define D11AC_TXC_TCPACK		0x0040
#define D11AC_TXC_TOF			0x0100 /**< Enable wifi ranging processing for rxd frames */
#define D11AC_TXC_MU			0x0200 /**< MU Tx data, any MU (HE, VHT) */
#define D11REV128_TXC_HEMUMIMO		0x0400 /**< MU Tx data, HE MU-MIMO */
#define D11REV128_TXC_HEOFDMA		0x0800 /**< MU Tx data, HE OFDMA */
#define D11REV128_TXC_INSAVB		0x2000 /**< Insert 10-byte AVB Tx timestamp */
#define D11AC_TXC_BFIX			0x0800 /**< BFI from SHMx */

#define D11AC_TSTAMP_SHIFT		8	/**< Tstamp in 256us units */

/* PhyTxControlWord_0 */
#define D11AC_PHY_TXC_FT_MASK		0x0003

/* vht txctl0 */
#define D11AC_PHY_TXC_NON_SOUNDING	0x0004
#define D11AC_PHY_TXC_BFM			0x0008
#define D11AC_PHY_TXC_SHORT_PREAMBLE	0x0010
#define D11AC2_PHY_TXC_STBC		0x0020
#define D11AC_PHY_TXC_ANT_MASK		0x3FC0
#define D11AC_PHY_TXC_CORE_MASK		0x03C0
#define D11AC_PHY_TXC_CORE_SHIFT	6
#define D11AC_PHY_TXC_ANT_IDX_MASK	0x3C00
#define D11AC_PHY_TXC_ANT_IDX_SHIFT	10
#define D11AC_PHY_TXC_BW_MASK		0xC000
#define D11AC_PHY_TXC_BW_SHIFT		14
#define D11AC_PHY_TXC_BW_20MHZ		0x0000
#define D11AC_PHY_TXC_BW_40MHZ		0x4000
#define D11AC_PHY_TXC_BW_80MHZ		0x8000
#define D11AC_PHY_TXC_BW_160MHZ		0xC000

/* PhyTxControlWord_1 */
#define D11AC_PHY_TXC_PRIM_SUBBAND_MASK		0x0007
#define D11AC_PHY_TXC_PRIM_SUBBAND_LLL		0x0000
#define D11AC_PHY_TXC_PRIM_SUBBAND_LLU		0x0001
#define D11AC_PHY_TXC_PRIM_SUBBAND_LUL		0x0002
#define D11AC_PHY_TXC_PRIM_SUBBAND_LUU		0x0003
#define D11AC_PHY_TXC_PRIM_SUBBAND_ULL		0x0004
#define D11AC_PHY_TXC_PRIM_SUBBAND_ULU		0x0005
#define D11AC_PHY_TXC_PRIM_SUBBAND_UUL		0x0006
#define D11AC_PHY_TXC_PRIM_SUBBAND_UUU		0x0007
#define D11AC_PHY_TXC_TXPWR_OFFSET_MASK 	0x01F8
#define D11AC_PHY_TXC_TXPWR_OFFSET_SHIFT	3
#define D11AC2_PHY_TXC_TXPWR_OFFSET_MASK 	0x03F8
#define D11AC2_PHY_TXC_TXPWR_OFFSET_SHIFT	3
#define D11AC_PHY_TXC_TXBF_USER_IDX_MASK	0x7C00
#define D11AC_PHY_TXC_TXBF_USER_IDX_SHIFT	10
#define D11AC2_PHY_TXC_DELTA_TXPWR_OFFSET_MASK 	0x7C00
#define D11AC2_PHY_TXC_DELTA_TXPWR_OFFSET_SHIFT	10
/* Rather awkward bit mapping to keep pctl1 word same as legacy, for proprietary 11n rate support */
#define D11AC_PHY_TXC_11N_PROP_MCS		0x8000 /* this represents bit mcs[6] */
#define D11AC2_PHY_TXC_MU			0x8000

/* PhyTxControlWord_2 phy rate */
#define D11AC_PHY_TXC_PHY_RATE_MASK		0x003F
#define D11AC2_PHY_TXC_PHY_RATE_MASK		0x007F

/* 11b phy rate */
#define D11AC_PHY_TXC_11B_PHY_RATE_MASK		0x0003
#define D11AC_PHY_TXC_11B_PHY_RATE_1		0x0000
#define D11AC_PHY_TXC_11B_PHY_RATE_2		0x0001
#define D11AC_PHY_TXC_11B_PHY_RATE_5_5		0x0002
#define D11AC_PHY_TXC_11B_PHY_RATE_11		0x0003

/* 11a/g phy rate */
#define D11AC_PHY_TXC_11AG_PHY_RATE_MASK	0x0007
#define D11AC_PHY_TXC_11AG_PHY_RATE_6		0x0000
#define D11AC_PHY_TXC_11AG_PHY_RATE_9		0x0001
#define D11AC_PHY_TXC_11AG_PHY_RATE_12		0x0002
#define D11AC_PHY_TXC_11AG_PHY_RATE_18		0x0003
#define D11AC_PHY_TXC_11AG_PHY_RATE_24		0x0004
#define D11AC_PHY_TXC_11AG_PHY_RATE_36		0x0005
#define D11AC_PHY_TXC_11AG_PHY_RATE_48		0x0006
#define D11AC_PHY_TXC_11AG_PHY_RATE_54		0x0007

/* 11ac phy rate */
#define D11AC_PHY_TXC_11AC_MCS_MASK		0x000F
#define D11AC_PHY_TXC_11AC_NSS_MASK		0x0030
#define D11AC_PHY_TXC_11AC_NSS_SHIFT		4

/* 11n phy rate */
#define D11AC_PHY_TXC_11N_MCS_MASK		0x003F
#define D11AC2_PHY_TXC_11N_MCS_MASK		0x007F
#define D11AC2_PHY_TXC_11N_PROP_MCS		0x0040 /* this represents bit mcs[6] */

/* PhyTxControlWord_2 rest */
#define D11AC_PHY_TXC_STBC			0x0040
#define D11AC_PHY_TXC_DYN_BW_IN_NON_HT_PRESENT	0x0080
#define D11AC_PHY_TXC_DYN_BW_IN_NON_HT_DYNAMIC	0x0100
#define D11AC2_PHY_TXC_TXBF_USER_IDX_MASK	0xFE00
#define D11AC2_PHY_TXC_TXBF_USER_IDX_SHIFT	9

/* RtsCtsControl */
#define D11AC_RTSCTS_FRM_TYPE_MASK	0x0001	/**< frame type */
#define D11AC_RTSCTS_FRM_TYPE_11B	0x0000	/**< 11b */
#define D11AC_RTSCTS_FRM_TYPE_11AG	0x0001	/**< 11a/g */
#define D11AC_RTSCTS_USE_RTS		0x0004	/**< Use RTS */
#define D11AC_RTSCTS_USE_CTS		0x0008	/**< Use CTS */
#define D11AC_RTSCTS_SHORT_PREAMBLE	0x0010	/**< Long/short preamble: 0 - long, 1 - short? */
#define D11AC_RTSCTS_LAST_RATE		0x0020	/**< this is last rate */
#define D11AC_RTSCTS_IMBF               0x0040	/**< Implicit TxBF */
#define D11AX_RTSCTS_BFM		0x0080	/**< beamforming */
#define D11AC_RTSCTS_BF_IDX_MASK	0xF000	/**< 4-bit index to the beamforming block */
#define D11AC_RTSCTS_BF_IDX_SHIFT	12
#define D11AC_RTSCTS_RATE_MASK		0x0F00	/**< Rate table offset: bit 3-0 of PLCP byte 0 */
#define D11AC_RTSCTS_USE_RATE_SHIFT	8

/* BssIdEncAlg */
#define D11AC_BSSID_MASK		0x000F	/**< BSS index */
#define D11AC_BSSID_SHIFT		0
#define D11AC_ENCRYPT_ALG_MASK		0x00F0	/**< Encryption algoritm */
#define D11AC_ENCRYPT_ALG_SHIFT		4
#define D11AC_ENCRYPT_ALG_NOSEC		0x0000	/**< No security */
#define D11AC_ENCRYPT_ALG_WEP		0x0010	/**< WEP */
#define D11AC_ENCRYPT_ALG_TKIP		0x0020	/**< TKIP */
#define D11AC_ENCRYPT_ALG_AES		0x0030	/**< AES */
#define D11AC_ENCRYPT_ALG_WEP128	0x0040	/**< WEP128 */
#define D11AC_ENCRYPT_ALG_NA		0x0050	/**< N/A */
#define D11AC_ENCRYPT_ALG_WAPI		0x0060	/**< WAPI */

/* AmpduDur */
#define D11AC_AMPDU_MIN_DUR_IDX_MASK	0x000F	/**< AMPDU minimum duration index */
#define D11AC_AMPDU_MIN_DUR_IDX_SHIFT	0
#define D11AC_AMPDU_MAX_DUR_MASK	0xFFF0	/**< AMPDU maximum duration in unit 16 usec */
#define D11AC_AMPDU_MAX_DUR_SHIFT	4

/**
 * TX Descriptor definitions for supporting rev80 (HE)
 */
/* Maximum number of TX fallback rates per packet */
#define D11_REV80_TXH_NUM_RATES			4
#define D11_REV80_TXH_PHYTXCTL_MIN_LENGTH	1

/** per rate info - fixed portion - rev80 */
typedef struct d11txh_rev80_rate_fixed d11txh_rev80_rate_fixed_t;
BWL_PRE_PACKED_STRUCT struct d11txh_rev80_rate_fixed {
	uint16	TxRate;			/* rate in 500Kbps */
	uint16	RtsCtsControl;		/* RTS - CTS control */
	uint8	plcp[D11_PHY_HDR_LEN];	/* 6 bytes */
} BWL_POST_PACKED_STRUCT;

/* rev80 specific per packet info fields */
typedef struct d11pktinfo_rev80 d11pktinfo_rev80_t;
BWL_PRE_PACKED_STRUCT struct d11pktinfo_rev80 {
	uint16	HEModeControl;			/* 20 */
	uint16  length;				/* 22 - length of txd in bytes */
} BWL_POST_PACKED_STRUCT;

#define D11_REV80_TXH_TX_MODE_SHIFT		0	/* Bits 2:0 of HeModeControl */
#define D11_REV80_TXH_TX_MODE_MASK		0x3
#define D11_REV80_TXH_HTC_OFFSET_SHIFT		4	/* Bits 8:4 of HeModeControl */
#define D11_REV80_TXH_HTC_OFFSET_MASK  0x01F0

#define D11_REV80_PHY_TXC_EDCA			0x00
#define D11_REV80_PHY_TXC_OFDMA_RA		0x01	/* Use Random Access Trigger for Tx */
#define D11_REV80_PHY_TXC_OFDMA_DT		0x02	/* Use Directed Trigger for Tx */
#define D11_REV80_PHY_TXC_OFDMA_ET		0x03	/* Use earliest Trigger Opportunity */

#define IS_TRIGGERQ_TRAFFIC(a)		((a)->PktInfoExt.HEModeControl & \
						D11_REV80_TXH_TX_MODE_MASK)

/** Per cache info - rev80 */
typedef struct d11txh_rev80_cache d11txh_rev80_cache_t;
BWL_PRE_PACKED_STRUCT struct d11txh_rev80_cache {
	d11txh_cache_common_t common;		/* 0 - 7 */
	uint16	ampdu_mpdu_all;			/* 8 - 9 */
	uint16	aggid;				/* 10 - 11 */
	uint8	tkipph1_index;			/* 12 */
	uint8	pktext;				/* 13 */
	uint16	reserved;			/* 14 - 15 (for 4 byte alignement) */
} BWL_POST_PACKED_STRUCT;

/** Fixed size portion of TX descriptor - rev80 */
typedef struct d11txh_rev80 d11txh_rev80_t;
BWL_PRE_PACKED_STRUCT struct d11txh_rev80 {
	/**
	 * Per pkt info fields (common + rev80 specific)
	 *
	 * Note : Ensure that PktInfo field is always the first member
	 * of the d11txh_rev80 struct (that is at OFFSET - 0)
	 */
	d11pktinfo_common_t PktInfo;	/* 0 - 20 */
	d11pktinfo_rev80_t PktInfoExt;	/* 21 - 23 */

	/** Per cache info */
	d11txh_rev80_cache_t CacheInfo;	/* 24 - 39 */

	/**
	 * D11_REV80_TXH_NUM_RATES number of Rate Info blocks
	 * contribute to the variable size portion of the TXD.
	 * Each Rate Info element (block) is a funtion of
	 * (N_PwrOffset, N_RU, N_User).
	 */
	uint8 RateInfoBlock[1];
} BWL_POST_PACKED_STRUCT;

/**
 * Size of fixed size portion of TX descriptor : size of
 * d11txh_rev80_t minus size of RateInfoBlock[1]
 */
#define D11_REV80_TXH_FIXED_LEN	((uint)(uintptr)&((d11txh_rev80_t *)0)->RateInfoBlock)

/* Short format tx descriptor only has per packet info (24 bytes) */
#define D11_REV80_TXH_SHORT_LEN	(sizeof(d11pktinfo_common_t) + \
				sizeof(d11pktinfo_rev80_t))

/* Length of BFM0 field in RateInfo Blk */
#define	D11_REV80_TXH_BFM0_FIXED_LEN(pwr_offs)		(pwr_offs)

/**
 * Length of FBWInfo field in RateInfo Blk
 *
 * Note : for now return fixed length of 1 word
 */
#define	D11_REV80_TXH_FBWINFO_FIXED_LEN(pwr_offs)	2

#define D11_REV80_TXH_FIXED_RATEINFO_LEN	sizeof(d11txh_rev80_rate_fixed_t)

/**
 * Macros to find size of N-RUs field in the PhyTxCtlWord.
 */
#define D11_REV80_TXH_TXC_N_RUs_FIELD_SIZE		1
#define D11_REV80_TXH_TXC_PER_RU_INFO_SIZE		4
#define D11_REV80_TXH_TXC_PER_RU_MIN_SIZE		2

#define D11_REV80_TXH_TXC_RU_FIELD_SIZE(n_rus)	((n_rus == 1) ? \
						(D11_REV80_TXH_TXC_PER_RU_MIN_SIZE) : \
						((D11_REV80_TXH_TXC_N_RUs_FIELD_SIZE) + \
						((n_rus) * D11_REV80_TXH_TXC_PER_RU_INFO_SIZE)))

/**
 * Macros to find size of N-Users field in the TXCTL_EXT
 */
#define D11_REV80_TXH_TXC_EXT_N_USERs_FIELD_SIZE	1
#define D11_REV80_TXH_TXC_EXT_PER_USER_INFO_SIZE	4

#define D11_REV80_TXH_TXC_N_USERs_FIELD_SIZE(n_users) \
	((n_users) ? \
	 (((n_users) * \
	   (D11_REV80_TXH_TXC_EXT_PER_USER_INFO_SIZE)) + \
	  (D11_REV80_TXH_TXC_EXT_N_USERs_FIELD_SIZE)) :	\
	 (n_users))

/**
 * Size of each Tx Power Offset field in PhyTxCtlWord.
 */
#define D11_REV80_TXH_TXC_PWR_OFFSET_SIZE		1

/**
 * Size of fixed / static fields in PhyTxCtlWord (all fields except N-RUs, N-Users and Pwr offsets)
 */
#define D11_REV80_TXH_TXC_CONST_FIELDS_SIZE		6

/**
 * Macros used for filling PhyTxCtlWord
 */

/* PhyTxCtl Word 0 Byte 0 */
#define D11_REV80_PHY_TXC_FT_MASK		0x0007
#define D11_REV80_PHY_TXC_FTFMT_MASK		0x001F
#define D11_REV80_PHY_TXC_NON_SOUNDING		0x0040
#define D11_REV80_PHY_TXC_SHORT_PREAMBLE	0x0080
/* PhyTxCtl Word 0 Byte 1 */
#define D11_REV80_PHY_RATE_MASK			0x007F
#define D11_REV80_PHY_RATE_SHIFT		8
#define D11_REV80_PHY_TXC_STBC			0x0080

/* PhyTxCtl Word 1 (Bytes 2 - 3) */
#define D11_REV80_PHY_TXC_MU			0x8000
#define D11_REV80_PHY_TXC_BW_20MHZ		0x0000
#define D11_REV80_PHY_TXC_BW_40MHZ		0x0001
#define D11_REV80_PHY_TXC_BW_80MHZ		0x0002
#define D11_REV80_PHY_TXC_BW_160MHZ		0x0003
/* PhyTxCtl Word 2 (Bytes 4 -5) */
/* Though the width antennacfg, coremask fields are 8-bits,
 * only 4 bits is valid for 4369a0, hence masking only 4 bits
 */
#define D11_REV80_PHY_TXC_ANT_CONFIG_MASK		0x00F0
#define D11_REV80_PHY_TXC_CORE_MASK			0x000F
#define D11_REV80_PHY_TXC_ANT_CONFIG_SHIFT		4
/* upper byte- Ant. cfg, lower byte - Core  */
#define D11_REV80_PHY_TXC_ANT_CORE_MASK		0x0F0F

/* PhyTxCtl BFM field */
#define D11_REV80_PHY_TXC_BFM			0x80

/* PhyTxCtl power offsets */
#define D11_REV80_PHY_TXC_PWROFS0_BYTE_POS	6

/* Phytx Ctl Sub band location */
#define D11_REV80_PHY_TXC_SB_SHIFT		2
#define D11_REV80_PHY_TXC_SB_MASK		0x001C

/* 11n phy rate */
#define D11_REV80_PHY_TXC_11N_MCS_MASK		0x003F
#define D11_REV80_PHY_TXC_11N_PROP_MCS		0x0040 /* this represents bit mcs[6] */

/* 11ac phy rate */
#define D11_REV80_PHY_TXC_11AC_NSS_SHIFT	4

/* PhyTxCtl Word0  */
#define D11_REV80_PHY_TXC_MCS_NSS_SHIFT		8

/* 11ax phy rate */
#define D11_REV80_PHY_TXC_11AX_NSS_SHIFT	4

#define D11_PHY_TXC_FT_MASK(corerev)	((D11REV_GE(corerev, 80)) ? D11_REV80_PHY_TXC_FT_MASK : \
					(D11REV_GE(corerev, 40)) ? D11AC_PHY_TXC_FT_MASK : \
					PHY_TXC_FT_MASK)

#define D11_PHY_TXC_FTFMT_MASK(corerev)	((D11REV_GE(corerev, 80)) ?\
					D11_REV80_PHY_TXC_FTFMT_MASK : \
					(D11REV_GE(corerev, 40)) ? D11AC_PHY_TXC_FT_MASK : \
					PHY_TXC_FT_MASK)

/**
 * TX Descriptor definitions for supporting rev128 (HE AP)
 */
/** TXDescriptor for Corerev 128 */
typedef struct d11pktinfo_rev128 d11txh_rev128_t;
BWL_PRE_PACKED_STRUCT struct d11pktinfo_rev128 {
	uint16  TSOInfo;                        /* 0. Not used */
	uint16  MacControl_0;                   /* 2. MacTxControlLow */
	uint16  MacControl_1;                   /* 4. MacTxControlHigh */
	uint16  MacControl_2;                   /* 6. HE control */
	uint16  Chanspec;                       /* 8 */
	uint16  FrameID;                        /* 10. Ucode expects queue ID in lower 7 bits */
	uint16  SeqCtl;                         /* 12 */
	uint16  FrmLen;                         /* 14. From FC to FCS. In [bytes] units. */
	uint8   IVOffset;                       /* 16 */
	uint8   Lifetime;                       /* 17. Expiration time. In [512 usec] units */
	uint16  EnqueueTimestamp_L;             /* 18. 32-bit timestamp (TSF) when posted */
	uint16  EnqueueTimestamp_ML;            /* 20 */
	uint16  LinkMemIdxTID;                  /* 22. Idx link table AMTIdx b0-11. TID b12-15 */
	uint16  RateMemIdxRateIdx;              /* 24. Idx rate table AMTIdx b0-8. b11-13 blk idx */
	uint16  BcnTargetTxTime;		/* 26. NAN: beacon target tx time. */
	uint16	reserved[2];			/* 28-31. Not used */

#ifdef WL_EAP_UCODE_TX_DESC
	/* Extend the Tx Descriptor by 48 bytes for EAP */
#ifdef WL_EAP_RTS_OVERRIDE
	d11actxh_rts_override_t rts;        /* 124 - 131 */
#else
	uint8   Pad0[8];
#endif // endif

#ifdef WL_EAP_SAS
	d11actxh_sas_t sas;                 /* 132 - 139 */
#else
	uint8   Pad1[8];
#endif // endif

	/* Bitfield used by EAP features */
	uint16  wlent_flags;                /* 140 - 141 */

#ifdef WL_EAP_PER_PKT_SOUND_PERIOD
	uint16  sounding_period;            /* 142 - 143 */
#else
	uint16  Pad2;
#endif // endif

#ifdef WL_EAP_DESC_KEY
	/* Encryption key for hw to use to encrypt outgoing frame */
	/* Unfortunately, DOT11_MAX_KEY_SIZE is 32 */
	uint8 key[16];                      /* 144 - 159 */
#else
	uint8 Pad3[16];
#endif // endif

#ifdef WL_EAP_PER_PKT_RTX
	/* Retry and fallback limits */
	d11actxh_per_pkt_rtx_t rtx;         /* 160 - 167 */
#else
	uint8 Pad4[8];
#endif // endif

#ifdef WL_EAP_TXQ_PRUNING
	/* TXQ flow ID to be used as bit position in ucode bitmap */
	uint16 txq_flowid;                  /* 168 - 169 */
	uint16 Reserved2;                   /* 170 - 171 */
#else
	uint32 Pad5;
#endif // endif
#endif /* WL_EAP_UCODE_TX_DESC */
} BWL_POST_PACKED_STRUCT;

#define D11_REV128_TXH_LEN			sizeof(d11txh_rev128_t)	/* 32 bytes */
#define D11_REV128_TXFID_FIFO_MASK		0x007F		/* TX FIFO index */
#define D11_REV128_TXFID_SEQ_MASK		0xFF80		/* Frame sequence number */
#define D11_REV128_TXFID_SEQ_SHIFT		7		/* Frame sequence number shifts */
#define D11_REV128_TXFID_MAX_BCMC_FID \
		((D11_REV128_TXFID_SEQ_MASK >> D11_REV128_TXFID_SEQ_SHIFT)+1) /* 512 */
#define D11_REV128_LIFETIME_SHIFT		9		/* Lifetime in 512us units */
#define D11_REV128_LINKIDX_MASK			0xFFF
#define D11_REV128_LINKTID_SHIFT		12
#define D11_REV128_RATEIDX_MASK			0x1FF
#define D11_REV128_RATE_SPECRATEIDX_SHIFT	12
#define D11_REV128_RATE_SPECRATEIDX_MASK	0x3000
#define D11_REV128_TXH_SHORT_LEN		D11_REV128_TXH_LEN
#define D11_REV128_TXH_LEN_EX			D11_REV128_TXH_LEN
#define D11_LINKENTRY_NUM_TIDS			8
#define D11_RATE_LINK_MEM_IDX_INVALID		0xFFFF

/** Per-TID info in LinkMem entry for LinkMem table (new in corerev 128) */
typedef struct d11linkmem_ptid d11linkmem_ptid_t;
BWL_PRE_PACKED_STRUCT struct d11linkmem_ptid {
	uint8  BAWin;         /* BA window -1. 0 means 1, 255 means 256 */
	uint8  ampdu_mpdu;    /* Max ampdu mpdu num - 1. 0 means 1, 255 means 256 */
} BWL_POST_PACKED_STRUCT;

/** LinkMem entry for LinkMem table (new in corerev 128) */
typedef struct d11linkmem_entry d11linkmem_entry_t;
BWL_PRE_PACKED_STRUCT struct d11linkmem_entry {
	/* first part is shared with/written by all of SW and consumed by ucode */
	uint8               BssIdx;            /* 5 bits P2P/MBSS BssIdx */
	uint8               BssColor;          /* 6 bits */
	uint16              StaID_IsAP;        /* 12 bits AID, bit15=IsAP */
	uint16              EncryptInfo;       /* 4 bits of EncryptAlgo, 9 of KeyIdx, 3 of TKIP */
	uint16              OMI;               /* OMI HE-control field */
	uint16              RtsDurThresh;      /* unit: usec */
	uint16              ampdu_info;        /* 6 bits MaxRxFactor, 10 bits ampdu_mpdu_all */
	uint8               MultiTIDAggBitmap; /* TID bitmap that can be aggregated; 0 = disabled */
	uint8               MultiTIDAggNum;    /* Max nr of TIDs that can be aggregated */
	uint16              AmpMaxDurIdx;        /* AMPDU max dur in usec. Mixed-mode << 5 msec */
	uint32              PPET0_AmpMinDur;     /* 24 bits of PPET for NSS1, 8 bits AmpMinDur */
	uint32              PPET1_PEdef;         /* 24 bits of PPET for NSS2, 5 bits Default PE */
	uint32              PPET2_TrigMACPadDur; /* 24 bits of PPET for NSS3, 5 bits Min trig pad */
	uint32              PPET3;               /* 24 bits of PPET for NSS4 */
	d11linkmem_ptid_t   ampdu_ptid[D11_LINKENTRY_NUM_TIDS];
	uint16              baLen;		/* BA len for each tid */
	uint16              fragTx;		/* frag tx params */
	uint16              BFIConfig0;		/* 3 bits bfenss, 3 bits bfrnss, 8 bits BFIIdx */
	uint16              BFIConfig1;		/* 8 bits bfecap, 8 bits bfrcap */
	/* next part is ucode internal data initted by SW, maintained by ucode */
	uint16              BFRStat0;		/* Ucode internal */
	uint16              BFRStat1;		/* Ucode internal */
	uint16              BFRStat2;		/* Ucode internal */
	uint16              BFRStat3;		/* Ucode internal */
	/* another 32 bytes reserved exclusively for ucode usage */
	// uint8 RESERVED[32]; /* not defined to reserve SW memory */
} BWL_POST_PACKED_STRUCT;

#define D11_REV128_RATEENTRY_NUM_RATES			4 /* primary + 3 fallback rates */
#define D11_REV128_RATEENTRY_TXPHYCTL_WORDS		5
#define D11_REV128_RATEENTRY_NUM_BWS			4 /* 20, 40, 80, 160MHz */
#define D11_REV128_RATEENTRY_FBWPWR_WORDS		3

enum d11_rev128_bw_enum {
	D11_REV128_BW_20MHZ = 0,
	D11_REV128_BW_40MHZ = 1,
	D11_REV128_BW_80MHZ = 2,
	D11_REV128_BW_160MHZ = 3,
	D11_REV128_BW_SZ = 4
};

/* C_LTX_OMI_POS */
typedef enum
{
	C_LNK_OMI_BW_NBIT       = 3,	// BW
	C_LNK_OMI_BW_LB         = 4
} eLnkOmiBitsDefinitions;

#define D11_REV128_PHYCTL1_BW_SHIFT		0	/* bw */
#define D11_REV128_PHYCTL1_BW_MASK		0x0003
#define D11_REV128_PHYCTL1_POSB_SHIFT		5	/* partial ofdma subband */
#define D11_REV128_PHYCTL1_POSB_MASK		0x1FE0

/* Per rate info in RateMem entry for RateMem table (new in corerev 128) */
typedef struct d11ratemem_rev128_rate d11ratemem_rev128_rate_t;
BWL_PRE_PACKED_STRUCT struct d11ratemem_rev128_rate {
	uint16      RateCtl;
	uint8       plcp[D11_PHY_HDR_LEN];              /* 6 bytes */
	 /* txphyctl word offsets {0, 1, 2, 6, 7} */
	uint16      TxPhyCtl[D11_REV128_RATEENTRY_TXPHYCTL_WORDS];
	uint16      BFM0;                               /* 2 bits {valid, stbc} per BW */
	uint8       txpwr_bw[D11_REV128_RATEENTRY_NUM_BWS]; /* indexed by d11_rev128_bw_enum */
	uint16      FbwCtl;      /* txmode (bfm, stbc, valid) for each of three fallback bws */
	/* 1 word for txmode for each of 3 bw, 2x3 sets of txpwr for each fallback bw. */
	uint16      FbwPwr[D11_REV128_RATEENTRY_FBWPWR_WORDS];
} BWL_POST_PACKED_STRUCT;

/** RateMem entry for RateMem table (new in corerev 128) */
typedef struct d11ratemem_rev128_entry d11ratemem_rev128_entry_t;
BWL_PRE_PACKED_STRUCT struct d11ratemem_rev128_entry {
	uint16                     flags;   /* flags: (ldpc | probe | epoch | cnt) */
	uint8                      reserved[2];
	/* Rate info for primary and 3 fallback rates */
	d11ratemem_rev128_rate_t   rate_info_block[D11_REV128_RATEENTRY_NUM_RATES];
	d11ratemem_rev128_rate_t   rate_info_block_tpl; /* Rsrvd for ucode. E.g. MU-mimo/OFDMA. */
	uint32                     pad[1];              /* Ensure 8-byte aligned */
} BWL_POST_PACKED_STRUCT;

/* rtflags (rate_flags) format:
 * bit  8  : LDPC cap flag
 * bit  7  : RATE PROBE flag
 * bits 6:4: RATE_EPOCH maintained by ratesel module
 * bits 3:0: rmem_nupd
 */
#define D11_REV128_RATE_LDPC_FLAG		0x100
#define D11_REV128_RATE_PROBE_FLAG		0x80
#define D11_REV128_RATE_EPOCH_MASK		0x70
#define D11_REV128_RATE_EPOCH_SHIFT		4
#define D11_REV128_RATE_NUPD_MASK		0x0F
#define D11_REV128_RATE_NUPD_SHIFT		0
#define D11_REV128_RATE_TXS_MASK		\
	(D11_REV128_RATE_NUPD_MASK | D11_REV128_RATE_EPOCH_MASK | D11_REV128_RATE_PROBE_FLAG)
#define D11_RATEENTRY_IS_PROBE(rate_entry)	(((rate_entry) != NULL) && \
	(((rate_entry)->epoch & D11_REV128_RATE_PROBE_FLAG) != 0))
#define D11_REV128_PPETX_MASK			0x00FFFFFF
#define D11_REV128_AMPMINDUR_MASK		0xFF000000
#define D11_REV128_AMPMINDUR_SHIFT		24
#define D11_REV128_PEDEF_MASK			0x1F000000
#define D11_REV128_PEDEF_SHIFT			24
#define D11_REV128_TRIGMACPADDUR_MASK		0x1F000000
#define D11_REV128_TRIGMACPADDUR_SHIFT		24
#define D11_REV128_BSSIDX_MASK			0x1F
#define D11_REV128_SECBSS_NBIT			7
#define D11_REV128_COLOR_MASK			0x3F
#define D11_REV128_STAID_MASK			0x0FFF
#define D11_REV128_STAID_ISAP			0x8000
#define D11_REV128_ENCRYPTALGO_MASK		0xF
#define D11_REV128_ENCRYPTALGO_SHIFT		0
#define D11_REV128_KEYIDX_MASK			0x1FF0
#define D11_REV128_KEYIDX_SHIFT			4
#define D11_REV128_TKIPPHASE1KEYIDX_MASK	0xE000
#define D11_REV128_TKIPPHASE1KEYIDX_SHIFT	13
#define D11_REV128_MAXRXFACTOR_MASK		0x3F
#define D11_REV128_MAXRXFACTOR_SHIFT		0
#define D11_REV128_AMPDUMPDUALL_MASK		0xFFC0
#define D11_REV128_AMPDUMPDUALL_SHIFT		6

/* A wrapper structure for all versions of TxD/d11txh structures */
typedef union d11txhdr {
	d11txh_pre40_t pre40;
	d11actxh_t rev40;
	d11txh_rev80_t rev80;
	d11txh_rev128_t rev128;
} d11txhdr_t;

/**
 * Generic tx status packet for software use. This is independent of hardware
 * structure for a particular core. Hardware structure should be read and converted
 * to this structure before being sent for the sofware consumption.
 */
typedef struct tx_status tx_status_t;
typedef struct tx_status_macinfo tx_status_macinfo_t;

BWL_PRE_PACKED_STRUCT struct tx_status_macinfo {
	int8 pad0;
	int8 is_intermediate;
	int8 pm_indicated;
	int8 pad1;
	uint8 suppr_ind;
	int8 was_acked;

	uint16 rts_tx_cnt;
	uint16 frag_tx_cnt;
	uint16 cts_rx_cnt;

	uint16 raw_bits;

	uint32 s3;
	uint32 s4;
	uint32 s5;
	uint32 ack_map1;
	uint32 ack_map2;
	uint32 s8;
	uint32 s9;
	uint32 s10;
	uint32 s11;
} BWL_POST_PACKED_STRUCT;

BWL_PRE_PACKED_STRUCT struct tx_status {
	uint16 framelen;
	uint16 frameid;
	uint16 sequence;
	uint16 phyerr;
	uint32 lasttxtime;
	uint16 ackphyrxsh;
	uint16 procflags;	/* tx status processing flags */
	uint32 dequeuetime;
	tx_status_macinfo_t status;
} BWL_POST_PACKED_STRUCT;

/* Bits in struct tx_status procflags */
#define TXS_PROCFLAG_AMPDU_BA_PKG2_READ_REQD	0x1	/* AMPDU BA txs pkg2 read required */

/* status field bit definitions */
#define	TX_STATUS_FRM_RTX_MASK	0xF000
#define	TX_STATUS_FRM_RTX_SHIFT	12
#define	TX_STATUS_RTS_RTX_MASK	0x0F00
#define	TX_STATUS_RTS_RTX_SHIFT	8
#define TX_STATUS_MASK		0x00FE
#define	TX_STATUS_PMINDCTD	(1 << 7)	/**< PM mode indicated to AP */
#define	TX_STATUS_INTERMEDIATE	(1 << 6)	/**< intermediate or 1st ampdu pkg */
#define	TX_STATUS_AMPDU		(1 << 5)	/**< AMPDU status */
#define TX_STATUS_SUPR_MASK	0x1C		/**< suppress status bits (4:2) */
#define TX_STATUS_SUPR_SHIFT	2
#define	TX_STATUS_ACK_RCV	(1 << 1)	/**< ACK received */
#define	TX_STATUS_VALID		(1 << 0)	/**< Tx status valid (corerev >= 5) */
#define	TX_STATUS_NO_ACK	0
#define TX_STATUS_BE		(TX_STATUS_ACK_RCV | TX_STATUS_PMINDCTD)

/* status field bit definitions phy rev > 40 */
#define TX_STATUS40_FIRST		0x0002
#define TX_STATUS40_INTERMEDIATE	0x0004
#define TX_STATUS40_PMINDCTD		0x0008

#define TX_STATUS40_SUPR		0x00f0
#define TX_STATUS40_SUPR_SHIFT		4

#define TX_STATUS40_NCONS		0x7f00

#define TX_STATUS40_NCONS_SHIFT		8

#define TX_STATUS40_ACK_RCV		0x8000

/* tx status bytes 8-16 */
#define TX_STATUS40_TXCNT_RATE0_MASK	0x000000ff
#define TX_STATUS40_TXCNT_RATE0_SHIFT	0

#define TX_STATUS40_ACKCNT_RATE0_MASK	0x0000ff00
#define TX_STATUS40_ACKCNT_RATE0_SHIFT	8

#define TX_STATUS40_TXCNT_RATE1_MASK	0x00ff0000
#define TX_STATUS40_TXCNT_RATE1_SHIFT	16

#define TX_STATUS40_MEDIUM_DELAY_MASK   0xFFFF

#define TX_STATUS40_TXCNT(s3, s4) \
	(((s3 & TX_STATUS40_TXCNT_RATE0_MASK) >> TX_STATUS40_TXCNT_RATE0_SHIFT) + \
	((s3 & TX_STATUS40_TXCNT_RATE1_MASK) >> TX_STATUS40_TXCNT_RATE1_SHIFT) + \
	((s4 & TX_STATUS40_TXCNT_RATE0_MASK) >> TX_STATUS40_TXCNT_RATE0_SHIFT) + \
	((s4 & TX_STATUS40_TXCNT_RATE1_MASK) >> TX_STATUS40_TXCNT_RATE1_SHIFT))

#define TX_STATUS40_TXCNT_RT0(s3) \
	((s3 & TX_STATUS40_TXCNT_RATE0_MASK) >> TX_STATUS40_TXCNT_RATE0_SHIFT)

#define TX_STATUS40_ACKCNT_RT0(s3) \
	((s3 & TX_STATUS40_ACKCNT_RATE0_MASK) >> TX_STATUS40_ACKCNT_RATE0_SHIFT)

#define TX_STATUS40_TX_MEDIUM_DELAY(txs)    ((txs)->status.s8 & TX_STATUS40_MEDIUM_DELAY_MASK)

/* chip rev 40 pkg 2 fields */
#define TX_STATUS40_IMPBF_MASK		0x0000000C	/* implicit bf applied */
#define TX_STATUS40_IMPBF_BAD_MASK	0x00000010	/* impl bf applied but ack frm has no bfm */
#define TX_STATUS40_IMPBF_LOW_MASK	0x00000020	/* ack received with low rssi */
#define TX_STATUS40_BFTX		0x0040		/* Beamformed pkt TXed */
/* pkt two status field bit definitions mac rev > 64 */
#define TX_STATUS64_MUTX		0x0080

#define TX_STATUS40_RTS_RTX_MASK	0x00ff0000
#define TX_STATUS40_RTS_RTX_SHIFT	16
#define TX_STATUS40_CTS_RRX_MASK	0xff000000
#define TX_STATUS40_CTS_RRX_SHIFT	24

/* MU group info txstatus field (s3 b[31:16]) */
#define TX_STATUS64_MU_GID_MASK		0x003f0000
#define TX_STATUS64_MU_GID_SHIFT	16
#define TX_STATUS64_MU_BW_MASK		0x00c00000
#define TX_STATUS64_MU_BW_SHIFT		22
#define TX_STATUS64_MU_TXPWR_MASK	0x7f000000
#define TX_STATUS64_MU_TXPWR_SHIFT	24
#define TX_STATUS64_MU_SGI_MASK		0x80000080
#define TX_STATUS64_MU_SGI_SHIFT	31

#define TX_STATUS64_MU_GID(s3) ((s3 & TX_STATUS64_MU_GID_MASK) >> TX_STATUS64_MU_GID_SHIFT)
#define TX_STATUS64_MU_BW(s3) ((s3 & TX_STATUS64_MU_BW_MASK) >> TX_STATUS64_MU_BW_SHIFT)
#define TX_STATUS64_MU_TXPWR(s3) ((s3 & TX_STATUS64_MU_TXPWR_MASK) >> TX_STATUS64_MU_TXPWR_SHIFT)
#define TX_STATUS64_MU_SGI(s3) ((s3 & TX_STATUS64_MU_SGI_MASK) >> TX_STATUS64_MU_SGI_SHIFT)

/* MU user info0 txstatus field (s4 b[15:0]) */
#define TX_STATUS64_MU_MCS_MASK		0x0000000f
#define TX_STATUS64_MU_MCS_SHIFT	0
#define TX_STATUS64_MU_NSS_MASK		0x00000070
#define TX_STATUS64_MU_NSS_SHIFT	4
#define TX_STATUS64_MU_SNR_MASK		0x0000ff00
#define TX_STATUS64_MU_SNR_SHIFT	8

#define TX_STATUS64_MU_MCS(s4) ((s4 & TX_STATUS64_MU_MCS_MASK) >> TX_STATUS64_MU_MCS_SHIFT)
#define TX_STATUS64_MU_NSS(s4) ((s4 & TX_STATUS64_MU_NSS_MASK) >> TX_STATUS64_MU_NSS_SHIFT)
#define TX_STATUS64_MU_SNR(s4) ((s4 & TX_STATUS64_MU_SNR_MASK) >> TX_STATUS64_MU_SNR_SHIFT)

/* MU txstatus rspec field (NSS | MCS) */
#define TX_STATUS64_MU_RSPEC_MASK	(TX_STATUS64_MU_NSS_MASK | TX_STATUS64_MU_MCS_MASK)
#define TX_STATUS64_MU_RSPEC_SHIFT	0

#define TX_STATUS64_MU_RSPEC(s4) ((s4 & TX_STATUS64_MU_RSPEC_MASK) >> TX_STATUS64_MU_RSPEC_SHIFT)

/* MU user info0 txstatus field (s4 b[31:16]) */
#define TX_STATUS64_MU_GBMP_MASK	0x000f0000
#define TX_STATUS64_MU_GBMP_SHIFT	16
#define TX_STATUS64_MU_GPOS_MASK	0x00300000
#define TX_STATUS64_MU_GPOS_SHIFT	20
#define TX_STATUS64_MU_TXCNT_MASK	0x0fc00000
#define TX_STATUS64_MU_TXCNT_SHIFT	22

#define TX_STATUS64_MU_GBMP(s4) ((s4 & TX_STATUS64_MU_GBMP_MASK) >> TX_STATUS64_MU_GBMP_SHIFT)
#define TX_STATUS64_MU_GPOS(s4) ((s4 & TX_STATUS64_MU_GPOS_MASK) >> TX_STATUS64_MU_GPOS_SHIFT)
#define TX_STATUS64_MU_TXCNT(s4) ((s4 & TX_STATUS64_MU_TXCNT_MASK) >> TX_STATUS64_MU_TXCNT_SHIFT)

/* pkt two status field bit definitions mac rev >= 128 */
/* MU type bitmask and shift */
#define TX_STATUS128_MUTP_MASK		0x0300
#define TX_STATUS128_MUTP_SHIFT		8

#define TX_STATUS128_MUTYP(s5)	(((s5) & TX_STATUS128_MUTP_MASK) >> TX_STATUS128_MUTP_SHIFT)

#define TX_STATUS_MUTYP(rev, s5)	(D11REV_GE((rev), 128)) ? \
			(((s5) & TX_STATUS128_MUTP_MASK) >> TX_STATUS128_MUTP_SHIFT) : \
			(TX_STATUS_MUTP_VHTMU)

/* HEOM tx status rate info */
#define TX_STATUS128_HEOM_RUIDX_MASK		0x0000ff00
#define TX_STATUS128_HEOM_RUIDX_SHIFT		8
#define TX_STATUS128_HEOM_RUIDX(s4) (((s4) & TX_STATUS128_HEOM_RUIDX_MASK) >> \
		TX_STATUS128_HEOM_RUIDX_SHIFT)

#define TX_STATUS128_HEOM_RTIDX_MASK		0x30000000
#define TX_STATUS128_HEOM_RTIDX_SHIFT		28
#define TX_STATUS128_HEOM_RTIDX(s4) (((s4) & TX_STATUS128_HEOM_RTIDX_MASK) >> \
		TX_STATUS128_HEOM_RTIDX_SHIFT)

#define TX_STATUS128_HEOM_CPLTF_MASK		0xc0000000
#define TX_STATUS128_HEOM_CPLTF_SHIFT		30
#define TX_STATUS128_HEOM_CPLTF(s4) (((s4) & TX_STATUS128_HEOM_CPLTF_MASK) >> \
		TX_STATUS128_HEOM_CPLTF_SHIFT)

/* MU type enum */
enum d11_rev128_txs_mutype_enum {
	TX_STATUS_MUTP_VHTMU		= 0, /* VHT MUMIMO */
	TX_STATUS_MUTP_HEMM		= 1, /* HE MUMIMO */
	TX_STATUS_MUTP_HEOM		= 2, /* HE MU OFDMA */
	TX_STATUS_MUTP_HEMOM		= 3, /* HE MUMIMO + OFDMA */
};

#define TX_STATUS128_RTFLAGS_MASK	0xFF000000
#define TX_STATUS128_RTFLAGS_SHIFT	24
#define TX_STATUS128_RATEIDX_MASK	0x00FF0000
#define TX_STATUS128_RATEIDX_SHIFT	16
#define TX_STATUS128_RATEEPOCH_MASK	0xFF000000
#define TX_STATUS128_RATEEPOCH_SHIFT	24

#define TX_STATUS128_RTFLAGS(s8)	(((s8) & TX_STATUS128_RTFLAGS_MASK) >> \
					TX_STATUS128_RTFLAGS_SHIFT)
#define TX_STATUS128_RATEIDX(s8)	(((s8) & TX_STATUS128_RATEIDX_MASK) >> \
					TX_STATUS128_RATEIDX_SHIFT)
#define TX_STATUS128_RATEEPOCH(s8)	(((s8) & TX_STATUS128_RATEEPOCH_MASK) >> \
					TX_STATUS128_RATEEPOCH_SHIFT)

/* WARNING: Modifying suppress reason codes?
 * Update wlc_tx_status_t and TX_STS_REASON_STRINGS and
 * wlc_tx_status_map_hw_to_sw_supr_code() also
 */
/* status field bit definitions */
/** suppress status reason codes */
enum  {
	TX_STATUS_SUPR_NONE		= 0,
	TX_STATUS_SUPR_PMQ		= 1,	/**< PMQ entry */
	TX_STATUS_SUPR_FLUSH		= 2,	/**< flush request */
	TX_STATUS_SUPR_FRAG		= 3,	/**< previous frag failure */
	TX_STATUS_SUPR_TBTT		= 3,	/**< SHARED: Probe response supr for TBTT */
	TX_STATUS_SUPR_BADCH		= 4,	/**< channel mismatch */
	TX_STATUS_SUPR_EXPTIME		= 5,	/**< lifetime expiry */
	TX_STATUS_SUPR_UF		= 6,	/**< underflow */
#ifdef WLP2P_UCODE
	TX_STATUS_SUPR_NACK_ABS		= 7,	/**< BSS entered ABSENCE period */
#endif // endif
	TX_STATUS_SUPR_PPS		= 8,	/**< Pretend PS */
	TX_STATUS_SUPR_PHASE1_KEY	= 9,	/**< Request new TKIP phase-1 key */
	TX_STATUS_UNUSED		= 10,	/**< Unused in trunk */
	TX_STATUS_SUPR_AGG0		= 11,	/**< rev >= 128: continuous AGG0 */
	NUM_TX_STATUS_SUPR
};

/** Unexpected tx status for rate update */
#define TX_STATUS_UNEXP(status) \
	((((status.is_intermediate))) && \
	 TX_STATUS_UNEXP_AMPDU(status))

/** Unexpected tx status for A-MPDU rate update */
#ifdef WLP2P_UCODE
#define TX_STATUS_UNEXP_AMPDU(status) \
	((((status.suppr_ind)) != TX_STATUS_SUPR_NONE) && \
	 (((status.suppr_ind)) != TX_STATUS_SUPR_EXPTIME) && \
	 (((status.suppr_ind)) != TX_STATUS_SUPR_NACK_ABS))
#else
#define TX_STATUS_UNEXP_AMPDU(status) \
	((((status.suppr_ind)) != TX_STATUS_SUPR_NONE) && \
	 (((status.suppr_ind)) != TX_STATUS_SUPR_EXPTIME))
#endif // endif

/**
 * This defines the collection of supp reasons (including none)
 * for which mac has done its (re-)transmission in any of ucode retx schemes
 * which include ucode/hw/aqm agg
 */
#define TXS_SUPR_MAGG_DONE_MASK ((1 << TX_STATUS_SUPR_NONE) | \
		(1 << TX_STATUS_SUPR_UF) |   \
		(1 << TX_STATUS_SUPR_FRAG) | \
		(1 << TX_STATUS_SUPR_EXPTIME))
#define TXS_SUPR_MAGG_DONE(suppr_ind) \
		((1 << (suppr_ind)) & TXS_SUPR_MAGG_DONE_MASK)

#define TX_STATUS_BA_BMAP03_MASK	0xF000	/**< ba bitmap 0:3 in 1st pkg */
#define TX_STATUS_BA_BMAP03_SHIFT	12	/**< ba bitmap 0:3 in 1st pkg */
#define TX_STATUS_BA_BMAP47_MASK	0x001E	/**< ba bitmap 4:7 in 2nd pkg */
#define TX_STATUS_BA_BMAP47_SHIFT	3	/**< ba bitmap 4:7 in 2nd pkg */

/* RXE (Receive Engine) */

/* RCM_CTL */
#define	RCM_INC_MASK_H		0x0080
#define	RCM_INC_MASK_L		0x0040
#define	RCM_INC_DATA		0x0020
#define	RCM_INDEX_MASK		0x001F
#define	RCM_SIZE		15

#define	RCM_MAC_OFFSET		0	/**< current MAC address */
#define	RCM_BSSID_OFFSET	3	/**< current BSSID address */
#define	RCM_F_BSSID_0_OFFSET	6	/**< foreign BSS CFP tracking */
#define	RCM_F_BSSID_1_OFFSET	9	/**< foreign BSS CFP tracking */
#define	RCM_F_BSSID_2_OFFSET	12	/**< foreign BSS CFP tracking */

#define RCM_WEP_TA0_OFFSET	16
#define RCM_WEP_TA1_OFFSET	19
#define RCM_WEP_TA2_OFFSET	22
#define RCM_WEP_TA3_OFFSET	25

/* AMT - Address Match Table */

/* AMT Attribute bits */
#define AMT_ATTR_VALID          0x8000	/**< Mark the table entry valid */
#define AMT_ATTR_A1             0x0008	/**< Match for A1 */
#define AMT_ATTR_A2             0x0004	/**< Match for A2 */
#define AMT_ATTR_A3             0x0002	/**< Match for A3 */
#define AMT_ATTR_ADDR_MASK	0xF	/**< Address Mask */

/* AMT Index defines */
#define AMT_SIZE_64		64  /* number of AMT entries */
#define AMT_SIZE_128		128 /* number of AMT entries for corerev >= 64 */
#define AMT_SIZE_256		256 /* number of AMT entries for corerev >= 128 */
#define AMT_IDX_MAC		63	/**< device MAC */
#define AMT_IDX_BSSID		62	/**< BSSID match */

/* In case of Ratelinkmem */
#define AMT_IDX_RSVD_SIZE_MIN  8       /**< reserve 248-255 rate entries for ucode and */
/**< these indexes are also used for Virtual BSS linkmem */
#define AMT_IDX_RSVD_SIZE      (MAX(AMT_IDX_RSVD_SIZE_MIN, WLC_MAXBSSCFG))
#define AMT_IDX_RSVD_START     (AMT_SIZE_256 - AMT_IDX_RSVD_SIZE)

#define AMT_SIZE(_corerev)	(D11REV_GT((_corerev), 128) ? AMT_SIZE_256 : \
	(D11REV_IS((_corerev), 128) ? AMT_SIZE_128 : \
	(D11REV_GE((_corerev), 80) ? AMT_SIZE_64 : \
	(D11REV_GE((_corerev), 64) ? AMT_SIZE_128 : AMT_SIZE_64))))

#define AMT_FLOWID_INVALID(_corerev)	(AMT_SIZE((_corerev)))
#define CCA_MESH_MAX		 8
#ifdef WL_CCA_STATS_MESH
#if defined(WL_RELMCAST) && !defined(WL_RELMCAST_DISABLED)
#error "CCA_STATS_MESH and Reliable Mcast have over lapping amt indices"
#else
#define AMT_IDX_CCA_MESH_START	51
#define AMT_IDX_CCA_MESH_END	(AMT_IDX_CCA_MESH_START + CCA_MESH_MAX -2)
#endif /* WL_RELMCAST */
#endif /* WL_CCA_STATS_MESH */
/* RMC entries */
#define AMT_IDX_MCAST_ADDR	61	/**< MCAST address for Reliable Mcast feature */
#define AMT_IDX_MCAST_ADDR1	59	/**< MCAST address for Reliable Mcast feature */
#define AMT_IDX_MCAST_ADDR2	58	/**< MCAST address for Reliable Mcast feature */
#define AMT_IDX_MCAST_ADDR3	57	/**< MCAST address for Reliable Mcast feature */

#ifdef WLMESH
/* note: this is max supported by ucode. But ARM-driver can
 * only mesh_info->mesh_max_peers which should be <= this value.
 */

#define AMT_MAX_MESH_PEER		10
#define AMT_MAXIDX_MESH_PEER            60
#define AMT_MAXIDX_P2P_USE	\
	(AMT_MAXIDX_MESH_PEER - AMT_MAX_MESH_PEER)
#else
#define AMT_MAXIDX_P2P_USE	60	/**< Max P2P entry to use */
#endif /* WL_STA_MONITOR */

#define AMT_MAX_TXBF_ENTRIES	7	/**< Max tx beamforming entry */
/* PSTA AWARE AP: Max PSTA Tx beamforming entry */
#define AMT_MAX_TXBF_PSTA_ENTRIES	20

#define AUXPMQ_ENTRIES			64  /* number of AUX PMQ entries */
#define AUXPMQ_ENTRY_SIZE       8

/* PSM Block */

/* psm_phy_hdr_param bits */
#define MAC_PHY_RESET		1
#define MAC_PHY_CLOCK_EN	2
#define MAC_PHY_FORCE_CLK	4
#define MAC_IHRP_CLOCK_EN	15

/* PSMCoreControlStatus (IHR Address 0x078) bit definitions */
#define PSM_CORE_CTL_AR		(1 << 0)
#define PSM_CORE_CTL_HR		(1 << 1)
#define PSM_CORE_CTL_IR		(1 << 2)
#define PSM_CORE_CTL_AAR	(1 << 3)
#define PSM_CORE_CTL_HAR	(1 << 4)
#define PSM_CORE_CTL_PPAR	(1 << 5)
#define PSM_CORE_CTL_SS		(1 << 6)
#define PSM_CORE_CTL_REHE	(1 << 7)
#define PSM_CORE_CTL_PPAS	(1 << 13)
#define PSM_CORE_CTL_AAS	(1 << 14)
#define PSM_CORE_CTL_HAS	(1 << 15)

#define PSM_CORE_CTL_LTR_BIT	9
#define PSM_CORE_CTL_LTR_MASK	0x3

#define M_PSM_SOFT_REGS_EXT  (0xc0*2) /* corerev >= 40 only */

/* WEP Block */

/* WEP_WKEY */
#define	WKEY_START		(1 << 8)
#define	WKEY_SEL_MASK		0x1F

/* WEP data formats */

/* the number of RCMTA entries */
#define RCMTA_SIZE 50

/* max keys in M_TKMICKEYS_BLK - 96 * sizeof(uint16) */
#define	WSEC_MAX_TKMIC_ENGINE_KEYS(_corerev) ((D11REV_GE(_corerev, 64)) ? \
	AMT_SIZE(_corerev) : 12) /* 8 + 4 default - 2 mic keys 8 bytes each */

/* max keys in M_WAPIMICKEYS_BLK - 64 * sizeof(uint16) */
#define	WSEC_MAX_SMS4MIC_ENGINE_KEYS(_corerev) ((D11REV_GE(_corerev, 64)) ? \
	AMT_SIZE(_corerev) : 8)  /* 4 + 4 default  - 16 bytes each */

/* max RXE match registers */
#define WSEC_MAX_RXE_KEYS	4

/* SECKINDXALGO (Security Key Index & Algorithm Block) word format */
/* SKL (Security Key Lookup) */
#define	SKL_ALGO_MASK		0x0007
#define	SKL_ALGO_SHIFT		0
#define	SKL_WAPI_KEYID_MASK	0x0008
#define	SKL_WAPI_KEYID_SHIFT	3
#define	SKL_INDEX_SHIFT		4

#define SKL_INDEX_MASK(_corerev)   ((D11REV_GE(_corerev, 64)) ? \
	(0x0FF0) : (0x03F0))
#define SKL_GRP_ALGO_MASK(_corerev)   ((D11REV_GE(_corerev, 64)) ? \
	(0x7000) : (0x1c00))
#define SKL_GRP_ALGO_SHIFT(_corerev)   ((D11REV_GE(_corerev, 64)) ? \
	(12) : (10))

#define	SKL_STAMON_NBIT		0x8000 /* STA monitor bit */

/* additional bits defined for IBSS group key support */
#define	SKL_IBSS_INDEX_MASK	0x01F0
#define	SKL_IBSS_INDEX_SHIFT	4
#define	SKL_IBSS_KEYID1_MASK	0x0600
#define	SKL_IBSS_KEYID1_SHIFT	9
#define	SKL_IBSS_KEYID2_MASK	0x1800
#define	SKL_IBSS_KEYID2_SHIFT	11
#define	SKL_IBSS_KEYALGO_MASK	0xE000
#define	SKL_IBSS_KEYALGO_SHIFT	13

#define	WSEC_MODE_OFF		0
#define	WSEC_MODE_HW		1
#define	WSEC_MODE_SW		2

#define	WSEC_ALGO_OFF			0
#define	WSEC_ALGO_WEP1			1
#define	WSEC_ALGO_TKIP			2
#define	WSEC_ALGO_WEP128		3
#define	WSEC_ALGO_AES_LEGACY		4
#define	WSEC_ALGO_AES			5
#define	WSEC_ALGO_SMS4			6
#define	WSEC_ALGO_SMS4_DFT_2005_09_07	7
#define	WSEC_ALGO_NALG			8
#define	WSEC_ALGO_GCM			6
#define	WSEC_ALGO_GCM256		8

/* D11 COREREV 80 TTAK KEY INDEX SHIFT */
#define	SKL_TTAK_INDEX_SHIFT	13

#define	D11_PRE40_WSEC_ALGO_AES		3
#define	D11_PRE40_WSEC_ALGO_WEP128	4
#define	D11_PRE40_WSEC_ALGO_AES_LEGACY	5
#define	D11_PRE40_WSEC_ALGO_SMS4	6
#define	D11_PRE40_WSEC_ALGO_NALG	7

#define	AES_MODE_NONE		0
#define	AES_MODE_CCM		1
#define	AES_MODE_OCB_MSDU	2
#define	AES_MODE_OCB_MPDU	3
#define	AES_MODE_CMAC		4
#define	AES_MODE_GCM		5
#define	AES_MODE_GMAC		6

/* WEP_CTL (Rev 0) */
#define	WECR0_KEYREG_SHIFT	0
#define	WECR0_KEYREG_MASK	0x7
#define	WECR0_DECRYPT		(1 << 3)
#define	WECR0_IVINLINE		(1 << 4)
#define	WECR0_WEPALG_SHIFT	5
#define	WECR0_WEPALG_MASK	(0x7 << 5)
#define	WECR0_WKEYSEL_SHIFT	8
#define	WECR0_WKEYSEL_MASK	(0x7 << 8)
#define	WECR0_WKEYSTART		(1 << 11)
#define	WECR0_WEPINIT		(1 << 14)
#define	WECR0_ICVERR		(1 << 15)

/* Frame template map byte offsets */
#define	T_ACTS_TPL_BASE		(0)
#define	T_NULL_TPL_BASE		(0xc * 2)
#define	T_QNULL_TPL_BASE	(0x1c * 2)
#define	T_RR_TPL_BASE		(0x2c * 2)
#define	T_BCN0_TPL_BASE		(0x34 * 2)
#define	T_PRS_TPL_BASE		(0x134 * 2)
#define	T_BCN1_TPL_BASE		(0x234 * 2)
#define	T_P2P_NULL_TPL_BASE	(0x340 * 2)
#define	T_P2P_NULL_TPL_SIZE	(32)

/* FCBS base addresses and sizes in BM */

#define FCBS_DS0_BM_CMD_SZ_CORE0	0x0200	/* 512 bytes */
#define FCBS_DS0_BM_DAT_SZ_CORE0	0x0200	/* 512 bytes */

#define FCBS_DS0_BM_CMDPTR_BASE_CORE0	0x3000
#define FCBS_DS0_BM_DATPTR_BASE_CORE0	(FCBS_DS0_BM_CMDPTR_BASE_CORE0 + FCBS_DS0_BM_CMD_SZ_CORE0)

#define FCBS_DS0_BM_CMD_SZ_CORE1	0x0200	/* 512 bytes */
#define FCBS_DS0_BM_DAT_SZ_CORE1	0x0200	/* 512 bytes */

#define FCBS_DS0_BM_CMDPTR_BASE_CORE1	0x2400
#define FCBS_DS0_BM_DATPTR_BASE_CORE1	(FCBS_DS0_BM_CMDPTR_BASE_CORE1 + FCBS_DS0_BM_CMD_SZ_CORE1)

#define FCBS_DS1_BM_CMD_SZ_CORE0	0x2000	/* Not used */
#define FCBS_DS1_BM_DAT_SZ_CORE0	0x2000	/* Not used */

#define FCBS_DS1_BM_CMDPTR_BASE_CORE0	0x17B4
#define FCBS_DS1_BM_DATPTR_BASE_CORE0	(FCBS_DS1_BM_CMDPTR_BASE_CORE0 + FCBS_DS1_BM_CMD_SZ_CORE0)

#define FCBS_DS1_BM_CMD_SZ_CORE1	0x2000	/* Not used */
#define FCBS_DS1_BM_DAT_SZ_CORE1	0x2000	/* Not used */

#define FCBS_DS1_BM_CMDPTR_BASE_CORE1	0x17B4
#define FCBS_DS1_BM_DATPTR_BASE_CORE1	(FCBS_DS1_BM_CMDPTR_BASE_CORE1 + FCBS_DS1_BM_CMD_SZ_CORE1)

#define T_BA_TPL_BASE		T_QNULL_TPL_BASE	/**< template area for BA */

#define T_RAM_ACCESS_SZ		4	/**< template ram is 4 byte access only */

#define TPLBLKS_PER_BCN_NUM	2
#define TPLBLKS_AC_PER_BCN_NUM	1

#if defined(WLLPRS) && defined(MBSS)
#define TPLBLKS_PER_PRS_NUM	4
#define TPLBLKS_AC_PER_PRS_NUM	2
#else
#define TPLBLKS_PER_PRS_NUM	2
#define TPLBLKS_AC_PER_PRS_NUM	1
#endif /* WLLPRS && MBSS */

/* SHM_reg = 2*(wlc_read_shm(M_SSLPNPHYREGS_PTR) + offset) */
#define M_SSLPNPHYREGS_PTR(x)	(71 * 2)
/* MAC Sample Collect Params */

/* SampleCapture set-up options in
 * different registers based on CoreRev
 */
/* CoreRev >= 50, use SMP_CTRL in TXE_IHR */
#define SC_SRC_MAC		2 /* MAC as Sample Collect Src */
#define SC_SRC_SHIFT		3 /* SC_SRC bits [3:4] */
#define SC_STOP_SHIFT		2 /* stop mode bit 2 */
#define SC_TRIG_SHIFT		5
#define SC_TRANS_SHIFT		6
#define SC_MATCH_SHIFT		7
#define SC_STORE_SHIFT		8
#define SC_STRIG_SHIFT		14

#define SC_STRT		1
#define SC_STOP_EN	(1 << SC_STOP_SHIFT)  // smp_ctrl stop mode
#define SC_TRIG_EN	(1 << SC_TRIG_SHIFT)
#define SC_TRANS_EN	(1 << SC_TRANS_SHIFT)
#define SC_MATCH_EN	(1 << SC_MATCH_SHIFT)
#define SC_STORE_EN	(1 << SC_STORE_SHIFT)
#define SC_STRIG_EN	(1 << SC_STRIG_SHIFT)

/* CoreRev < 50, use PHY_CTL in PSM_IHR */
#define PHYCTL_PHYCLKEN		(1 << 1)
#define PHYCTL_SC_STRT		(1 << 4)
#define PHYCTL_SC_SRC_LB	(1 << 7)
#define PHYCTL_SC_TRIG_EN	(1 << 8)
#define PHYCTL_SC_TRANS_EN	(1 << 9)
#define PHYCTL_SC_STR_EN	(1 << 10)
/* End MAC Sample Collect Params */

/* REVID >= 128 */
#define PHYCTL_PHYRST_SHIFT      0
#define PHYCTL_PHYFGC_SHIFT      2

#define PHYCTL_PHYRST            (1 << PHYCTL_PHYRST_SHIFT)
#define PHYCTL_PHYCLKEN_GE128    (1 << 1)
#define PHYCTL_PHYFGC            (1 << PHYCTL_PHYFGC_SHIFT)
#define PHYCTL_PHYRSTVAISP_GE128 (1 << 5)
#define PHYCTL_PHYRSTSMCSW_GE128 (1 << 6)
#define PHYCTL_PHYRSTSMC_GE128   (1 << 7)

#define ANTSEL_CLKDIV_4MHZ	6
#define MIMO_ANTSEL_BUSY	0x4000		/**< bit 14 (busy) */
#define MIMO_ANTSEL_SEL		0x8000		/**< bit 15 write the value */
#define MIMO_ANTSEL_WAIT	50		/**< 50us wait */
#define MIMO_ANTSEL_OVERRIDE	0x8000		/**< flag */

typedef struct shm_acparams shm_acparams_t;
BWL_PRE_PACKED_STRUCT struct shm_acparams {
	uint16	txop;
	uint16	cwmin;
	uint16	cwmax;
	uint16	cwcur;
	uint16	aifs;
	uint16	bslots;
	uint16	reggap;
	uint16	status;
	uint16  txcnt;
	uint16	rsvd[7];
} BWL_POST_PACKED_STRUCT;

#define WME_STATUS_NEWAC	(1 << 8)

/* M_HOST_FLAGS */
#define MHFMAX		5 /* Number of valid hostflag half-word (uint16) */
#define MHF1		0 /* Hostflag 1 index */
#define MHF2		1 /* Hostflag 2 index */
#define MHF3		2 /* Hostflag 3 index */
#define MHF4		3 /* Hostflag 4 index */
#define MHF5		4 /* Hostflag 5 index */

#define MXHFMAX		1 /* Number of valid PSMx hostflag half-word (uint16) */
#define MXHF0		64 /* PSMx Hostflag 0 index */

/* Flags in M_HOST_FLAGS */
#define	MHF1_ANTDIV		0x0001	/**< Enable ucode antenna diversity help */
#define MHF1_WLAN_CRITICAL	0x0002	/**< WLAN is in critical state */
#define	MHF1_MBSS_EN		0x0004	/**< Enable MBSS: RXPUWAR deprecated for rev >= 9 */
#define	MHF1_CCKPWR		0x0008	/**< Enable 4 Db CCK power boost */
#define	MHF1_BTCOEXIST		0x0010	/**< Enable Bluetooth / WLAN coexistence */
#define	MHF1_DCFILTWAR		0x0020	/**< Enable g-mode DC canceler filter bw WAR */
#define	MHF1_P2P_SKIP_TIME_UPD	0x0020	/**< Skip P2P SHM updates and P2P event generations */
#define	MHF1_PACTL		0x0040	/**< Enable PA gain boost for OFDM frames */
#define	MHF1_HDRCONVENABLE	0x0040	/**< Enable header conversion (mode 4) */
#define	MHF1_ACPRWAR		0x0080	/**< Enable ACPR.  Disable for Japan, channel 14 */
#define	MHF1_RXFIFO1		0x0080	/**< Switch data reception from RX fifo 0 to fifo 1 */
#define	MHF1_EDCF		0x0100	/**< Enable EDCF access control */
#define MHF1_IQSWAP_WAR		0x0200
#define MHF1_ULP		0x0200	/**< Force Ucode to put chip in low power state */
#define	MHF1_FORCEFASTCLK	0x0400	/**< Disable Slow clock request, for corerev < 11 */
#define	MHF1_ACIWAR		0x0800	/**< Enable ACI war: shiftbits by 2 on PHY_CRS */
#define	MHF1_FORCE_SEND_BCN	0x0800	/**< Force send bcn, even if rcvd from peer STA (IBSS) */
#define	MHF1_A2060WAR		0x1000	/**< PR15874WAR */
#define	MHF1_TIMBC_EN		0x1000	/**< Enable Target TIM Transmission Time function */
#define MHF1_RADARWAR		0x2000
#define MHF1_DEFKEYVALID	0x4000	/**< Enable use of the default keys */
#define	MHF1_CTS2SELF		0x8000	/**< Enable CTS to self full phy bw protection */

/* Definition changed in corerev >= 40 */
#define	MHF1_D11AC_DYNBW		0x0001	/**< dynamic bw */
#define MHF2_RSPBW20		0x0020		/**< Uses bw20 for response frames ack/ba/cts */

/* Flags in M_HOST_FLAGS2 */
#define MHF2_DISABLE_PRB_RESP	0x0001		/**< disable Probe Response in ucode */
#define MHF2_4317FWAKEWAR	0x0002		/**< PR19311WAR: 4317PCMCIA, fast wakeup ucode */
#define MHF2_SYNTHPUWAR		0x0004
#define MHF2_HIB_FEATURE_ENABLE	0x0008		/* Enable HIB feature in ucode */
#define MHF2_SKIP_ADJTSF	0x0010		/**< skip TSF update when receiving bcn/probeRsp */
#define MHF2_4317PIORXWAR	0x0020		/**< PR38778WAR : PIO receiving */
#define MHF2_TXBCMC_NOW		0x0040		/**< Flush BCMC FIFO immediately */
#define MHF2_PPR_HWPWRCTL	0x0080		/**< For corerev24+, ppr; for GPHY, Hwpwrctrl */
#define MHF2_BTC2WIRE_ALTGPIO	0x0100		/**< BTC 2wire in alternate pins */
#define MHF2_BTCPREMPT		0x0200		/**< BTC enable bluetooth check during tx */
#define MHF2_SKIP_CFP_UPDATE	0x0400		/**< Skip CFP update */
#define MHF2_NPHY40MHZ_WAR	0x0800
#define MHF2_TX_TMSTMP		0x0800		/**< Enable passing tx-timestamps in tx-status */
#define MHF2_TMP_HTRSP		0x1000		/**< Temp hack to use HT response frames in ucode */
#define MHF2_PRELD_GE64		0x2000		/**< preloading, used in ctdma path */
#define MHF2_BTCANTMODE		0x4000		/**< BTC ant mode ?? */
#define MHF2_NITRO_MODE		0x8000		/**< Enable Nitro mode */

/* GE128 setting */
#define MHF2_RXWDT		0x0080		/**< RXWDT, 128/129 */

/* Flags in M_HOST_FLAGS3 */
#define MHF3_ANTSEL_EN		0x0001	/**< enabled mimo antenna selection */
#define MHF3_ANTSEL_MODE	0x0002	/**< antenna selection mode: 0: 2x3, 1: 2x4 */
#define MHF3_BTCX_DEF_BT	0x0004	/**< corerev >= 13 BT Coex. */
#define MHF3_BTCX_ACTIVE_PROT	0x0008	/**< corerev >= 13 BT Coex. */
#define MHF3_USB_OLD_NPHYMLADVWAR 0x0010
#define MHF3_PKTENG_PROMISC	0x0010	/**< pass frames to driver in packet engine Rx mode */
#define MHF3_KNOISE		0x0020	/**< Use this to enable/disable knoise. */
#define MHF3_UCAMPDU_RETX	0x0040	/**< ucode handles AMPDU retransmission */
#define MHF3_BTCX_DELL_WAR	0x0080
#define MHF3_PM_BCNRX		0x0080	/**< PM single core beacon RX for power reduction */
#define MHF3_BTCX_SIM_RSP	0x0100	/**< allow limited lwo power tx when BT is active */
#define MHF3_BTCX_PS_PROTECT	0x0200	/**< use PS mode to protect BT activity */
#define MHF3_BTCX_SIM_TX_LP	0x0400	/**< use low power for simultaneous tx responses */
#define MHF3_PR45960_WAR	0x0800
#define MHF3_SELECT_RXF1	0x0800	/**< enable frame classification in pcie FD */
#define MHF3_BTCX_ECI		0x1000	/**< Enable BTCX ECI interface */
/* BTCX_EXTRA_PRI Used in DCF only */
#define MHF3_BTCX_EXTRA_PRI	0x2000	/**< Extra priority for 4th wire */
#define MHF3_PAPD_OFF_CCK	0x4000	/**< Disable PAPD comp for CCK frames */
#define MHF3_PAPD_OFF_OFDM	0x8000	/**< Disable PAPD comp for OFDM frames */

/* Flags in M_HOST_FLAGS4 */
#define MHF4_RTS_INFB		0x0001	/**< Change WME timings under certain conditions */
#define MHF4_RCMTA_BSSID_EN	0x0002  /**< BTAMP: multiSta BSSIDs matching in RCMTA area */
#define	MHF4_BCN_ROT_RR		0x0004	/**< MBSSID: beacon rotate in round-robin fashion */
#define	MHF4_OPT_SLEEP		0x0008	/**< enable opportunistic sleep */
#define	MHF4_PROXY_STA		0x0010	/**< enable proxy-STA feature */
#define MHF4_AGING		0x0020	/**< Enable aging threshold for RF awareness */
#define MHF4_BPHY_2TXCORES	0x0040	/**< bphy Tx on both cores (negative logic) */
#define MHF4_BPHY_TXCORE0	0x0080	/**< force bphy Tx on core 0 (board level WAR) */
#define MHF4_NOPHYHANGWAR	0x0100  /**< disable ucode WAR for idletssi cal */
#define MHF4_WMAC_ACKTMOUT	0x0200	/**< reserved for WMAC testing */
#define MHF4_NAPPING_ENABLE	0x0400	/**< Napping enable */
#define MHF4_IBSS_SEC		0x0800	/**< IBSS WPA2-PSK operating mode */
#define MHF4_SISO_BCMC_RX	0x1000	/* Disable switch to MIMO on recving multicast TIM */
#define MHF4_EXTPA_ENABLE	0x4000	/**< for 4313A0 FEM boards */
#define MHF4_RSDB_CR1_MINIPMU_CAL_EN	0x8000
#ifdef ACKSUPR_MAC_FILTER
#define MHF4_EN_ACKSUPR_BITMAP 0x4000 /* for pre-11ac acksupr enable */
#endif /* ACKSUPR_MAC_FILTER */

/* Flags in M_HOST_FLAGS5 */
#define MHF5_4331_BTCX_LOWISOLATION     0x0001  /**< Turn off txpu due to low antenna isolation */
#define MHF5_BTCX_LIGHT         0x0002	/**< light coex mode, off txpu only for critical BT */
#define MHF5_BTCX_PARALLEL      0x0004	/**< BT and WLAN run in parallel. */
#define MHF5_BTCX_DEFANT        0x0008	/**< default position for shared antenna */
#define MHF5_P2P_MODE		0x0010	/**< Enable P2P mode */
#define MHF5_LEGACY_PRS		0x0020	/**< Enable legacy probe resp support */
#define MHF5_LCN40PHY_ANTDIV_WAR	0x0040	/**< Enable LCN40PHY antidv WAR in ucode */
#define MHF5_HTPHY_RSSI_PWRDN	0x0080	/**< Disable RSSI_PWRDN feature */
#define MHF5_TONEJAMMER_WAR	0x0100	/**< Enable Tone Jammer war */
#define MHF5_UC_PRELOAD		0x0200		/* Enable pre-loading frames into the tx-fifo */
#define MHF5_SPIN_AT_SLEEP	0x0800	/**< Let ucode spin instead of setting SLOWCTL_PDE (dcf) */
#define MHF5_HWRSSI_EN		0x0800	/**< Enable HW RSSI (ac) */
#define MHF5_TXLOFT_WAR		0x1000	/**< Enable TX LOFT supression war (dcf) */
#define MHF5_NAVUPD_DIS		0x2000	/**< Disable NAV update */
#define MHF5_BTCX_GPIO_DEBUG	0x4000	/**< Enable gpio pins for btcoex ECI signals */
#define MHF5_SUPPRESS_PRB_REQ	0x8000	/**< Suppress probe requests at ucode level */

/* Flags in M_HOST_FLAGS6 */
#define MHF6_DIS_PRE_WDS        0x0010 /** < Disable Pre-Watchsdog in ucode */
#define MHF6_TXPWRCAP_EN        0x0002 /** < Enable TX power capping in ucode */
#define MHF6_TSYNC_EN		0x0020 /** < Enable 3rd txstatus package */
#define MHF6_TDMTX		0x0040 /** < Enable SDB TDM in ucode */

/* MX_HOST_FLAGS */
/* Flags for MX_HOST_FLAGS0 */
#define MXHF0_RSV0		0x0001		/* ucode internal, not exposed yet */
#define MXHF0_TXDRATE		0x0002		/* mu txrate to use rate from txd */
#define MXHF0_CHKFID		0x0004		/* check if frameid->fifo matches hw txfifo idx */
#define MXHF0_DISWAR		0x0008		/* disable some WAR. */
#define MXHF0_MUFBRDIS		0x0010		/* disable MUTX rate fallback */
#define MXHF0_MUBCC		0x0020		/* clear LDPC in plcp for MUTX. CRWLDOT11M-1774 */
#define MXHF0_DYNSND		0x0040		/* traffic based sounding interval management */
#define MXHF0_AGFSND		0x0080		/* AGF based sounding interval management */
#define MXHF0_MUAGGOPT		0x0100		/* MU aggregation optimization */
#define MXHF0_MUTXUFWAR		0x0200		/* vasip wds handling */
/* M_HOST_FLAGS6 */
#define MHF6_RESERVED		0x0001
#define MHF6_HTC_SUPPORT_SHIFT	1		/* 11AX HTC field support */
#define MHF6_HTC_SUPPORT	(1 << MHF6_HTC_SUPPORT_SHIFT)
/* Flags in M_HOST_FLAGS6 */
#define MHF6_HEB_CONFIG_SHIFT	2u		/* HEB Configured or not */
#define MHF6_HEB_CONFIG		(1u << MHF6_HEB_CONFIG_SHIFT)	/* HEB Configured or not */

/** Short version of receive frame status. Only used for non-last MSDU of AMSDU - rev61.1 */
typedef struct d11rxhdrshort_rev61_1 d11rxhdrshort_rev61_1_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdrshort_rev61_1 {
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */

	/* These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;    /**< bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;         /**< rx fifo number */
	uint16 mrxs;        /**< MAC Rx Status */
	uint16 RxFrameSize0;	/**< rxframesize for fifo-0 (in bytes). */
	uint16 HdrConvSt;   /**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint16 RxTSFTimeL;  /**< RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY */
	uint16 RxTSFTimeH;  /**< RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY */
	uint16 aux_status;  /**< DMA writes into this field. ucode treats as reserved. */
} BWL_POST_PACKED_STRUCT;

/** Short version of receive frame status. Only used for non-last MSDU of AMSDU - pre80 */
typedef struct d11rxhdrshort_lt80 d11rxhdrshort_lt80_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdrshort_lt80 {
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */

	/* These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;    /**< bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;         /**< rx fifo number */
	uint16 mrxs;        /**< MAC Rx Status */
	uint16 RxTSFTime;   /**< RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY */
	uint16 HdrConvSt;   /**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint16 aux_status;  /**< DMA writes into this field. ucode treats as reserved. */
} BWL_POST_PACKED_STRUCT;

/** Short version of receive frame status. Only used for non-last MSDU of AMSDU - rev80 */
typedef struct d11rxhdrshort_ge80 d11rxhdrshort_ge80_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdrshort_ge80 {
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */

	/**
	 * These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;	/**< bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;		/**< rx fifo number */

	uint16 mrxs;		/**< MAC Rx Status */
	uint16 RxFrameSize0;	/**< rxframesize for fifo-0 (in bytes). */
	uint16 HdrConvSt;	/**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint32 filtermap;	/**< 32 bit bitmap indicates which "Filters" have matched. */
	/**
	 * 16 bit bitmap is a result of Packet (or Flow ) Classification.
	 *
	 *	0	:	Flow ID Different
	 *	1,2,3	:	A1, A2, A3 Different
	 *	4	:	TID Different
	 *	5, 6	:	DA, SA from AMSDU SubFrame Different
	 *	7	:	FC Different
	 *	8	:	AMPDU boundary
	 *	9 - 15	:	Reserved
	 */
	uint16 pktclass;
	uint16 flowid;		/**< result of Flow ID Look Up performed by the HW. */
	/**
	 * These bits indicate specific errors detected by the HW on the Rx Path.
	 * However, these will be relevant for Last MSDU Status only.
	 *
	 * Whenever there is an error at any MSDU, HW treats it as last
	 * MSDU and send out last MSDU status.
	 */
	uint16 errflags;
} BWL_POST_PACKED_STRUCT;

/** Receive Frame Data Header - pre80 */
typedef struct d11rxhdr_lt80 d11rxhdr_lt80_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdr_lt80 {
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */

	/**
	 * These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;    /* bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;         /* rx fifo number */

	uint16 PhyRxStatus_0;	/**< PhyRxStatus 15:0 */
	uint16 PhyRxStatus_1;	/**< PhyRxStatus 31:16 */
	uint16 PhyRxStatus_2;	/**< PhyRxStatus 47:32 */
	uint16 PhyRxStatus_3;	/**< PhyRxStatus 63:48 */
	uint16 PhyRxStatus_4;	/**< PhyRxStatus 79:64 */
	uint16 PhyRxStatus_5;	/**< PhyRxStatus 95:80 */
	uint16 RxStatus1;	/**< MAC Rx Status */
	uint16 RxStatus2;	/**< extended MAC Rx status */

	/**
	 * - RxTSFTime time of first MAC symbol + M_PHY_PLCPRX_DLY
	 */
	uint16 RxTSFTime;

	uint16 RxChan;		/**< Rx channel info or chanspec */
	uint16 RxFrameSize_0;	/**< size of rx-frame in fifo-0 in case frame is copied to fifo-1 */
	uint16 HdrConvSt;	/**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint16 AvbRxTimeL;	/**< AVB RX timestamp low16 */
	uint16 AvbRxTimeH;	/**< AVB RX timestamp high16 */
	uint16 MuRate;		/**< MU rate info (bit3:0 MCS, bit6:4 NSTS) */
	/**
	 * These bits indicate specific errors detected by the HW on the Rx Path.
	 * However, these will be relevant for Last MSDU Status only.
	 *
	 * Whenever there is an error at any MSDU, HW treats it as last
	 * MSDU and send out last MSDU status.
	 */
	uint16 errflags;
} BWL_POST_PACKED_STRUCT;

#define N_PRXS_GE128	76		/* Total number of PhyRx status words for corerev >= 128 */
#define N_PRXS_GE80	16		/* Total number of PhyRx status words for corerev >= 80 */
#define N_PRXS_LT80	6		/* Total number of PhyRx status words for corerev < 80 */

/* number of PhyRx status words newly added for (corerev >= 128) */
#define N_PRXS_REM_GE128	(N_PRXS_GE128 - N_PRXS_LT80)
/* number of PhyRx status words newly added for (corerev >= 80) */
#define N_PRXS_REM_GE80	(N_PRXS_GE80 - N_PRXS_LT80)

#define PHYRXSTS_GE129_SZ	152  /**< number of PhyRx status bytes for corerev >= 129 */

/** RX Hdr definition - rev80 */
typedef struct d11rxhdr_ge80 d11rxhdr_ge80_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdr_ge80 {
	/**
	 * Even though rxhdr can be in short or long format, always declare it here
	 * to be in long format. So the offsets for the other fields are always the same.
	 */

	/**< HW Generated Status (20 Bytes) */
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */
	/**
	 * These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;	/* bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;		/* rx fifo number */
	uint16 mrxs;		/**< MAC Rx Status */
	uint16 RxFrameSize_0;	/**< size of rx-frame in fifo-0 in case frame is copied to fifo-1 */
	uint16 HdrConvSt;	/**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint32 filtermap;	/**< 32 bit bitmap indicates which "Filters" have matched. */
	/**
	 * 16 bit bitmap is a result of Packet (or Flow ) Classification.
	 *
	 *	0	:	Flow ID Different
	 *	1,2,3	:	A1, A2, A3 Different
	 *	4	:	TID Different
	 *	5	:	AMPDU boundary
	 *	6, 7	:	DA, SA from AMSDU SubFrame Different
	 *	8	:	FC Different
	 *	9 - 15	:	Reserved
	 */
	uint16 pktclass;
	uint16 flowid;		/**< result of Flow ID Look Up performed by the HW. */
	/**
	 * These bits indicate specific errors detected by the HW on the Rx Path.
	 * However, these will be relevant for Last MSDU Status only.
	 *
	 * Whenever there is an error at any MSDU, HW treats it as last
	 * MSDU and send out last MSDU status.
	 */
	uint16 errflags;

	/**< Ucode Generated Status (16 Bytes) */
	uint16 RxStatus1;		/**< MAC Rx Status */
	uint16 RxStatus2;		/**< extended MAC Rx status */
	uint16 RxChan;			/**< Rx channel info or chanspec */
	uint16 AvbRxTimeL;		/**< AVB RX timestamp low16 */
	uint16 AvbRxTimeH;		/**< AVB RX timestamp high16 */
	uint16 RxTSFTime;		/**< Lower 16 bits of Rx timestamp */
	uint16 RxTsfTimeH;		/**< Higher 16 bits of Rx timestamp */
	uint16 MuRate;			/**< MU rate info (bit3:0 MCS, bit6:4 NSTS) */

	/**< PHY Generated Status (32 Bytes) */
	uint16 PhyRxStatus_0;		/**< PhyRxStatus 15:0 */
	uint16 PhyRxStatus_1;		/**< PhyRxStatus 31:16 */
	uint16 PhyRxStatus_2;		/**< PhyRxStatus 47:32 */
	uint16 PhyRxStatus_3;		/**< PhyRxStatus 63:48 */
	uint16 PhyRxStatus_4;		/**< PhyRxStatus 79:64 */
	uint16 PhyRxStatus_5;		/**< PhyRxStatus 95:80 */
	uint16 phyrxs_rem[N_PRXS_REM_GE80];	/**< 20 bytes of remaining prxs (corerev >= 80) */
} BWL_POST_PACKED_STRUCT;

/** RX Status definition - rev128 */
typedef struct d11rxhdr_ge128 d11rxhdr_ge128_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdr_ge128 {
	/**
	 * only one format in rev128
	 */

	/**< HW Generated Status (20 Bytes) */
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */
	/**
	 * These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;	/* bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;		/* rx fifo number */
	uint16 mrxs;		/**< MAC Rx Status */
	uint16 RxFrameSize_0;	/**< size of rx-frame in fifo-0 in case frame is copied to fifo-1 */
	uint16 HdrConvSt;	/**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	uint32 filtermap;	/**< 32 bit bitmap indicates which "Filters" have matched. */
	/**
	 * 16 bit bitmap is a result of Packet (or Flow ) Classification.
	 *
	 *	0	:	Flow ID Different
	 *	1,2,3	:	A1, A2, A3 Different
	 *	4	:	TID Different
	 *	5	:	AMPDU boundary
	 *	6, 7	:	DA, SA from AMSDU SubFrame Different
	 *	8	:	FC Different
	 *	9 - 15	:	Reserved
	 */
	uint16 pktclass;
	uint16 flowid;		/**< result of Flow ID Look Up performed by the HW. */
	/**
	 * These bits indicate specific errors detected by the HW on the Rx Path.
	 * However, these will be relevant for Last MSDU Status only.
	 *
	 * Whenever there is an error at any MSDU, HW treats it as last
	 * MSDU and send out last MSDU status.
	 */
	uint16 errflags;

	/**< Ucode Generated Status (16 Bytes) */
	uint16 RxStatus1;		/**< MAC Rx Status */
	uint16 RxStatus2;		/**< extended MAC Rx status */
	uint16 RxChan;			/**< Rx channel info or chanspec */
	uint16 AvbRxTimeL;		/**< AVB RX timestamp low16 */
	uint16 AvbRxTimeH;		/**< AVB RX timestamp high16 */
	uint16 RxTSFTime;		/**< Lower 16 bits of Rx timestamp */
	uint16 RxTsfTimeH;		/**< Higher 16 bits of Rx timestamp */
	uint16 MuRate;			/**< MU rate info (bit3:0 MCS, bit6:4 NSTS) */

	uint16 PhyRxStatusLen;		/**< PhyRxStatus length */
	/**< PHY Generated Status (152 Bytes) */
	uint16 PhyRxStatus_0;		/**< PhyRxStatus 15:0 */
	uint16 PhyRxStatus_1;		/**< PhyRxStatus 31:16 */
	uint16 PhyRxStatus_2;		/**< PhyRxStatus 47:32 */
	uint16 PhyRxStatus_3;		/**< PhyRxStatus 63:48 */
	uint16 PhyRxStatus_4;		/**< PhyRxStatus 79:64 */
	uint16 PhyRxStatus_5;		/**< PhyRxStatus 95:80 */
	uint16 phyrxs_rem[N_PRXS_REM_GE128];	/**< bytes of remaining prxs (corerev >= 128) */
	uint16 pad[1];			/** pad to 8-Byte alignment for >=128 */
} BWL_POST_PACKED_STRUCT;

/** RX Status definition - rev129 */
typedef struct d11rxhdr_ge129 d11rxhdr_ge129_t;
BWL_PRE_PACKED_STRUCT struct d11rxhdr_ge129 {
	/**
	 * only one format in rev129
	 */

	/**< HW Generated Status (20 Bytes) */
	uint16 RxFrameSize;	/**< Actual byte length of the frame data received */
	/**
	 * These two 8-bit fields remain in the same order regardless of
	 * processor byte order.
	 */
	uint8 dma_flags;	/* bit 0 indicates short or long rx status. 1 == short. */
	uint8 fifo;		/* rx fifo number */
	uint16 mrxs;		/**< MAC Rx Status */
	uint16 RxFrameSize_0;	/**< size of rx-frame in fifo-0 in case frame is copied to fifo-1 */
	uint16 HdrConvSt;	/**< hdr conversion status. Copy of ihr(RCV_HDR_CTLSTS). */
	union {
		uint32 filtermap; /**< 32 bit bitmap indicates which "Filters" have matched. */
		struct {
			uint32 filtermap16: 16;
			uint32 sts_buf_idx: 13;
			uint32 hw_if: 3;
		};
	};
	/**
	 * 16 bit bitmap is a result of Packet (or Flow ) Classification.
	 *
	 *	0	:	Flow ID Different
	 *	1,2,3	:	A1, A2, A3 Different
	 *	4	:	TID Different
	 *	5	:	AMPDU boundary
	 *	6, 7	:	DA, SA from AMSDU SubFrame Different
	 *	8	:	FC Different
	 *	9 - 15	:	Reserved
	 */
	uint16 pktclass;
	uint16 flowid;		/**< result of Flow ID Look Up performed by the HW. */
	/**
	 * These bits indicate specific errors detected by the HW on the Rx Path.
	 * However, these will be relevant for Last MSDU Status only.
	 *
	 * Whenever there is an error at any MSDU, HW treats it as last
	 * MSDU and send out last MSDU status.
	 */
	uint16 errflags;

	/**< Ucode Generated Status (16 Bytes) */
	uint16 RxStatus1;		/**< MAC Rx Status */
	uint16 RxStatus2;		/**< extended MAC Rx status */
	uint16 RxChan;			/**< Rx channel info or chanspec */
	uint16 AvbRxTimeL;		/**< AVB RX timestamp low16 */
	uint16 AvbRxTimeH;		/**< AVB RX timestamp high16 */
	uint16 RxTSFTime;		/**< Lower 16 bits of Rx timestamp */
	uint16 RxTsfTimeH;		/**< Higher 16 bits of Rx timestamp */
	uint16 MuRate;			/**< MU rate info (bit3:0 MCS, bit6:4 NSTS) */
	uint32 pad;
} BWL_POST_PACKED_STRUCT;

typedef struct d11phystshdr d11phystshdr_t;
BWL_PRE_PACKED_STRUCT struct d11phystshdr {
	uint16 len;
	uint16 seq;
	uint16 pad0;
	uint16 pad1;
	/**< PHY Generated Status (152 Bytes) */
	uint16 PhyRxStatusLen;
	uint16 PhyRxStatus_0;		/**< PhyRxStatus 15:0 */
	uint16 PhyRxStatus_1;		/**< PhyRxStatus 31:16 */
	uint16 PhyRxStatus_2;		/**< PhyRxStatus 47:32 */
	uint16 PhyRxStatus_3;		/**< PhyRxStatus 63:48 */
	uint16 PhyRxStatus_4;		/**< PhyRxStatus 79:64 */
	uint16 PhyRxStatus_5;		/**< PhyRxStatus 95:80 */
	/**< bytes of remaining prxs (corerev >= 129) */
	uint16 phyrxs_rem[N_PRXS_REM_GE128 - 1];
} BWL_POST_PACKED_STRUCT;

/* A wrapper structure for all versions of d11rxh short structures */
typedef union d11rxhdrshort {
	d11rxhdrshort_rev61_1_t rev61_1;
	d11rxhdrshort_lt80_t lt80;
	d11rxhdrshort_ge80_t ge80;
} d11rxhdrshort_t;

/* A wrapper structure for all versions of d11rxh structures */
typedef union d11rxhdr {
	d11rxhdr_lt80_t lt80;
	d11rxhdr_ge80_t ge80;
	d11rxhdr_ge128_t ge128;
	d11rxhdr_ge129_t ge129;
} d11rxhdr_t;

#define D11RXHDRSHORT_GE80_ACCESS_REF(srxh, member) \
	(&((((d11rxhdrshort_t *)(srxh))->ge80).member))

#define D11RXHDRSHORT_LT80_ACCESS_REF(srxh, member) \
	(&((((d11rxhdrshort_t *)(srxh))->lt80).member))

#define D11RXHDRSHORT_REV61_1_ACCESS_REF(srxh, member) \
	(&((((d11rxhdrshort_t *)(srxh))->rev61_1).member))

#define D11RXHDRSHORT_GE80_ACCESS_VAL(srxh, member) \
	((((d11rxhdrshort_t *)(srxh))->ge80).member)

#define D11RXHDRSHORT_LT80_ACCESS_VAL(srxh, member) \
	((((d11rxhdrshort_t *)(srxh))->lt80).member)

#define D11RXHDRSHORT_REV61_1_ACCESS_VAL(srxh, member) \
	((((d11rxhdrshort_t *)(srxh))->rev61_1).member)

#define D11RXHDR_GE129_ACCESS_REF(rxh, member) \
	(&((rxh)->ge129).member)

#define D11RXHDR_GE128_ACCESS_REF(rxh, member) \
	(&((rxh)->ge128).member)

#define D11RXHDR_GE80_ACCESS_REF(rxh, member) \
	(&((rxh)->ge80).member)

#define D11RXHDR_LT80_ACCESS_REF(rxh, member) \
	(&((rxh)->lt80).member)

#define D11RXHDR_GE129_ACCESS_VAL(rxh, member) \
	(((rxh)->ge129).member)

#define D11RXHDR_GE128_ACCESS_VAL(rxh, member) \
	(((rxh)->ge128).member)

#define D11RXHDR_GE80_ACCESS_VAL(rxh, member) \
	(((rxh)->ge80).member)

#define D11RXHDR_LT80_ACCESS_VAL(rxh, member) \
	(((rxh)->lt80).member)

/** Computes array idx of a sts element, given the sts array base and elements address */
#define STS_MP_TABLE_INDX_GE129(type, base, elem) \
	((type*)(elem) - (type*)(base))

/** Computes a ptr to a sts element, given the sts array base and the array index */
#define STS_MP_TABLE_ELEM_GE129(type, base, indx) \
	(((type*)(base)) + (indx))

#define D11PHYSTSBUF_GE129_ACCESS_REF(rxh, member) \
	(&(((sts_buff_t *)(STS_MP_TABLE_ELEM_GE129(sts_buff_t, \
	                                        sts_mp_base_addr[((rxh)->ge129).hw_if], \
	                                        ((rxh)->ge129).sts_buf_idx)))->phystshdr->member))

#define D11PHYSTSBUF_GE128_ACCESS_REF(rxh, member) \
	(&((rxh)->ge128).member)

#define D11PHYSTSBUF_GE80_ACCESS_REF(rxh, member) \
	(&((rxh)->ge80).member)

#define D11PHYSTSBUF_LT80_ACCESS_REF(rxh, member) \
	(&((rxh)->lt80).member)

#define D11PHYSTSBUF_GE129_ACCESS_VAL(rxh, member) \
	(*D11PHYSTSBUF_GE129_ACCESS_REF((rxh), member))

#define D11PHYSTSBUF_GE128_ACCESS_VAL(rxh, member) \
	(((rxh)->ge128).member)

#define D11PHYSTSBUF_GE80_ACCESS_VAL(rxh, member) \
	(((rxh)->ge80).member)

#define D11PHYSTSBUF_LT80_ACCESS_VAL(rxh, member) \
	(((rxh)->lt80).member)

/** For accessing members of d11rxhdrshort_t by reference (address of members) */
#define D11RXHDRSHORT_ACCESS_REF(srxh, corerev, corerev_minor, member) \
	(D11REV_GE(corerev, 80) ? D11RXHDRSHORT_GE80_ACCESS_REF(srxh, member) : \
	(D11REV_IS(corerev, 61) && D11MINORREV_IS(corerev_minor, 1)) ? \
	D11RXHDRSHORT_REV61_1_ACCESS_REF(srxh, member) : \
	D11RXHDRSHORT_LT80_ACCESS_REF(srxh, member))

/** For accessing members of d11rxhdrshort_t by value (only value stored inside members accessed) */
#define D11RXHDRSHORT_ACCESS_VAL(srxh, corerev, corerev_minor, member) \
	(D11REV_GE(corerev, 80) ? D11RXHDRSHORT_GE80_ACCESS_VAL(srxh, member) : \
	(D11REV_IS(corerev, 61) && D11MINORREV_IS(corerev_minor, 1)) ? \
	D11RXHDRSHORT_REV61_1_ACCESS_VAL(srxh, member) : \
	D11RXHDRSHORT_LT80_ACCESS_VAL(srxh, member))

/** For accessing members of d11rxhdr_t by reference (address of members) */
#define D11RXHDR_ACCESS_REF(rxh, corerev, member) \
	(D11REV_GE(corerev, 129) ? D11RXHDR_GE129_ACCESS_REF(rxh, member) : \
	D11REV_IS(corerev, 128) ? D11RXHDR_GE128_ACCESS_REF(rxh, member) : \
	D11REV_GE(corerev, 80) ? D11RXHDR_GE80_ACCESS_REF(rxh, member) : \
	D11RXHDR_LT80_ACCESS_REF(rxh, member))

/** For accessing members of d11rxhdr_t by value (only value stored inside members accessed) */
#define D11RXHDR_ACCESS_VAL(rxh, corerev, member) \
	(D11REV_GE(corerev, 129) ? D11RXHDR_GE129_ACCESS_VAL(rxh, member) : \
	D11REV_IS(corerev, 128) ? D11RXHDR_GE128_ACCESS_VAL(rxh, member) : \
	D11REV_GE(corerev, 80) ? D11RXHDR_GE80_ACCESS_VAL(rxh, member) : \
	D11RXHDR_LT80_ACCESS_VAL(rxh, member))

/** For accessing members of d11rxhdr_t by reference (address of members) */
#define D11PHYSTSBUF_ACCESS_REF(rxh, corerev, member) \
	(D11REV_GE(corerev, 129) ? D11PHYSTSBUF_GE129_ACCESS_REF(rxh, member) : \
	D11REV_IS(corerev, 128) ? D11PHYSTSBUF_GE128_ACCESS_REF(rxh, member) : \
	D11REV_GE(corerev, 80) ? D11PHYSTSBUF_GE80_ACCESS_REF(rxh, member) : \
	D11PHYSTSBUF_LT80_ACCESS_REF(rxh, member))

/** For accessing members of d11rxhdr_t by value (only value stored inside members accessed) */
#define D11PHYSTSBUF_ACCESS_VAL(rxh, corerev, member) \
	(D11REV_GE(corerev, 129) ? D11PHYSTSBUF_GE129_ACCESS_VAL(rxh, member) : \
	D11REV_IS(corerev, 128) ? D11PHYSTSBUF_GE128_ACCESS_VAL(rxh, member) : \
	D11REV_GE(corerev, 80) ? D11PHYSTSBUF_GE80_ACCESS_VAL(rxh, member) : \
	D11PHYSTSBUF_LT80_ACCESS_VAL(rxh, member))

/** Length of HW RX status in RxStatus */
#define HW_RXHDR_LEN_REV_GE128	(sizeof(d11rxhdrshort_ge80_t))		/* 20 bytes */
#define HW_RXHDR_LEN_REV_GE80	(sizeof(d11rxhdrshort_ge80_t))		/* 20 bytes */
#define HW_RXHDR_LEN_REV_LT80	(sizeof(d11rxhdrshort_lt80_t))		/* 12 bytes */
#define HW_RXHDR_LEN_REV_61_1	(sizeof(d11rxhdrshort_rev61_1_t))	/* 16 bytes */

/** Length of HW RX status + ucode RX status + PHY RX status + padding(if need align) */
#define D11_RXHDR_LEN_REV_GE129	(sizeof(d11rxhdr_ge129_t))		/* 40 bytes */
#define D11_RXHDR_LEN_REV_GE128	(sizeof(d11rxhdr_ge128_t))		/* 192 bytes */
#define D11_RXHDR_LEN_REV_GE80	(sizeof(d11rxhdr_ge80_t))		/*  68 bytes */
#define D11_RXHDR_LEN_REV_LT80	(sizeof(d11rxhdr_lt80_t))		/*  36 bytes */

#define HW_RXHDR_LEN(corerev, corerev_minor) \
	(D11REV_GE(corerev, 128) ? HW_RXHDR_LEN_REV_GE128 : \
	D11REV_GE(corerev, 80) ? HW_RXHDR_LEN_REV_GE80 : \
	(D11REV_IS(corerev, 61) && \
	D11MINORREV_IS(corerev_minor, 1)) ? HW_RXHDR_LEN_REV_61_1 : \
	HW_RXHDR_LEN_REV_LT80)

#define D11_RXHDR_LEN(corerev) \
	(D11REV_GE(corerev, 129) ? D11_RXHDR_LEN_REV_GE129 : \
	(D11REV_IS(corerev, 128) ? D11_RXHDR_LEN_REV_GE128 : \
	D11REV_GE(corerev, 80) ? D11_RXHDR_LEN_REV_GE80 : \
	D11_RXHDR_LEN_REV_LT80))

#define	D11RXHDR_FRAMELEN(corerev, rxh)	D11RXHDR_ACCESS_VAL(rxh, corerev, RxFrameSize)

/* The following are for corerev >= 128 */
#define RXS_DMAFLAGS_MASK	0x03	/**< dma_flags field of d11rxhdr */
enum rxs_dmaflags {
	RXS_MAC_UCODE = 0x0,		/**< MAC+uCode RX status in dma_flags of d11rxhdr */
	RXS_MAC = 0x1,			/**< MAC RX status in dma_flags of d11rxhdr */
	RXS_MAC_UCODE_PHY = 0x2,	/**< MAC+uCode+PHY RX status in dma_flags of d11rxhdr */
	RXS_INVALID = 0x3		/**< Invalid value in dma_flags of d11rxhdr */
};

#define RXS_PHYSTS_VALID_REV128(corerev, rxh) \
	((D11RXHDR_ACCESS_VAL(rxh, corerev, dma_flags) & RXS_DMAFLAGS_MASK) == \
	RXS_MAC_UCODE_PHY)

#define RXS_UCODESTS_VALID_REV128(wlc, rxh) \
	({\
		uint8 dma_flags = D11RXHDR_ACCESS_VAL((rxh), (wlc)->pub->corerev, dma_flags); \
		(((dma_flags & RXS_DMAFLAGS_MASK) == RXS_MAC_UCODE) || \
			((dma_flags & RXS_DMAFLAGS_MASK) == RXS_MAC_UCODE_PHY)); \
	})

/** Frame ID */
#define D11_TXFID_GET_FIFO_REV_GE128(wlc, fid) \
	WLC_HW_UNMAP_TXFIFO((wlc), (fid) & D11_REV128_TXFID_FIFO_MASK)
#define D11_TXFID_GET_FIFO_REV_LT128(fid)	((fid) & TXFID_FIFO_MASK)
#define D11_TXFID_GET_FIFO(wlc, fid) \
	(D11REV_GE((wlc)->pub->corerev, 128) ? D11_TXFID_GET_FIFO_REV_GE128((wlc), (fid)) : \
					       D11_TXFID_GET_FIFO_REV_LT128(fid))
#define D11_TXFID_GET_MAX(wlc) \
	(D11REV_GE((wlc)->pub->corerev, 128) ? D11_REV128_TXFID_MAX_BCMC_FID : \
					       TXFID_MAX_BCMC_FID)

#define D11_TXFID_GET_SEQ_REV_GE128(fid) \
	(((fid) & D11_REV128_TXFID_SEQ_MASK) >> D11_REV128_TXFID_SEQ_SHIFT)
#define D11_TXFID_GET_SEQ_REV_LT128(fid)	(((fid) & TXFID_SEQ_MASK) >> TXFID_SEQ_SHIFT)
#define D11_TXFID_GET_SEQ(wlc, fid) \
	(D11REV_GE((wlc)->pub->corerev, 128) ? D11_TXFID_GET_SEQ_REV_GE128(fid) : \
		D11_TXFID_GET_SEQ_REV_LT128(fid))

#define D11_TXFID_IS_RATE_PROBE(_rev, fid) ((D11REV_GE(_rev, 128)) ? (0) : \
	(((fid) & TXFID_RATE_PROBE_MASK) != 0))

/* For corerev >= 65 */
#define RXS_SHORT_MASK		RXS_MAC	/**< Short RX status in dma_flags of d11rxhdr */

/* Header conversion status register bit fields */
#define HDRCONV_USR_ENAB	0x0001
#define HDRCONV_ENAB		0x0100
#define HDRCONV_ETH_FRAME	0x0200
#define HDRCONV_STATUS_VALID	0x8000

/* PhyRxStatus_0: */
#define	PRXS0_FT_MASK		0x0003	/**< [PRE-HE] NPHY only: CCK, OFDM, HT, VHT */
#define	PRXS0_CLIP_MASK		0x000C	/**< NPHY only: clip count adjustment steps by AGC */
#define	PRXS0_CLIP_SHIFT	2	/**< SHIFT bits for clip count adjustment */
#define	PRXS0_UNSRATE		0x0010	/**< PHY received a frame with unsupported rate */
#define	PRXS0_RXANT_UPSUBBAND	0x0020	/**< GPHY: rx ant, NPHY: upper sideband */
#define	PRXS0_LCRS		0x0040	/**< CCK frame only: lost crs during cck frame reception */
#define	PRXS0_SHORTH		0x0080	/**< Short Preamble */
#define	PRXS0_PLCPFV		0x0100	/**< PLCP violation */
#define	PRXS0_PLCPHCF		0x0200	/**< PLCP header integrity check failed */
#define	PRXS0_GAIN_CTL		0x4000	/**< legacy PHY gain control */
#define PRXS0_ANTSEL_MASK	0xF000	/**< NPHY: Antennas used for received frame, bitmask */
#define PRXS0_ANTSEL_SHIFT	12	/**< SHIFT bits for Antennas used for received frame */

/* subfield PRXS0_FT_MASK [PRXS0_PRE_HE_FT_MASK] */
#define	PRXS0_CCK		0x0000
#define	PRXS0_OFDM		0x0001	/**< valid only for G phy, use rxh->RxChan for A phy */
#define	PRXS0_PREN		0x0002
#define	PRXS0_STDN		0x0003

/* subfield PRXS0_ANTSEL_MASK */
#define PRXS0_ANTSEL_0		0x0	/**< antenna 0 is used */
#define PRXS0_ANTSEL_1		0x2	/**< antenna 1 is used */
#define PRXS0_ANTSEL_2		0x4	/**< antenna 2 is used */
#define PRXS0_ANTSEL_3		0x8	/**< antenna 3 is used */

/* PhyRxStatus_1: */
#define	PRXS1_JSSI_MASK		0x00FF
#define	PRXS1_JSSI_SHIFT	0
#define	PRXS1_SQ_MASK		0xFF00
#define	PRXS1_SQ_SHIFT		8

/* nphy PhyRxStatus_1: */
#define PRXS1_nphy_PWR0_MASK	0x00FF
#define PRXS1_nphy_PWR1_MASK	0xFF00

/* PhyRxStatus_2: */
#define	PRXS2_LNAGN_MASK	0xC000
#define	PRXS2_LNAGN_SHIFT	14
#define	PRXS2_PGAGN_MASK	0x3C00
#define	PRXS2_PGAGN_SHIFT	10
#define	PRXS2_FOFF_MASK		0x03FF

/* nphy PhyRxStatus_2: */
#define PRXS2_nphy_SQ_ANT0	0x000F	/**< nphy overall signal quality for antenna 0 */
#define PRXS2_nphy_SQ_ANT1	0x00F0	/**< nphy overall signal quality for antenna 0 */
#define PRXS2_nphy_cck_SQ	0x00FF	/**< bphy signal quality(when FT field is 0) */
#define PRXS3_nphy_SSQ_MASK	0xFF00	/**< spatial conditioning of the two receive channels */
#define PRXS3_nphy_SSQ_SHIFT	8

/* PhyRxStatus_3: */
#define	PRXS3_DIGGN_MASK	0x1800
#define	PRXS3_DIGGN_SHIFT	11
#define	PRXS3_TRSTATE		0x0400

/* nphy PhyRxStatus_3: */
#define PRXS3_nphy_MMPLCPLen_MASK	0x0FFF	/**< Mixed-mode preamble PLCP length */
#define PRXS3_nphy_MMPLCP_RATE_MASK	0xF000	/**< Mixed-mode preamble rate field */
#define PRXS3_nphy_MMPLCP_RATE_SHIFT	12

/* HTPHY Rx Status defines */
/* htphy PhyRxStatus_1: */
#define PRXS1_HTPHY_CORE_MASK	0x000F	/**< core enables for {3..0}, 0=disabled, 1=enabled */
#define PRXS1_HTPHY_ANTCFG_MASK	0x00F0	/**< antenna configuration */
#define PRXS1_HTPHY_MMPLCPLenL_MASK	0xFF00	/**< Mixmode PLCP Length low byte mask */

/* htphy PhyRxStatus_2: */
#define PRXS2_HTPHY_MMPLCPLenH_MASK	0x000F	/**< Mixmode PLCP Length high byte maskw */
#define PRXS2_HTPHY_MMPLCH_RATE_MASK	0x00F0	/**< Mixmode PLCP rate mask */
#define PRXS2_HTPHY_RXPWR_ANT0	0xFF00	/**< Rx power on core 0 */

/* htphy PhyRxStatus_3: */
#define PRXS3_HTPHY_RXPWR_ANT1	0x00FF	/**< Rx power on core 1 */
#define PRXS3_HTPHY_RXPWR_ANT2	0xFF00	/**< Rx power on core 2 */

/* htphy PhyRxStatus_4: */
#define PRXS4_HTPHY_RXPWR_ANT3	0x00FF	/**< Rx power on core 3 */
#define PRXS4_HTPHY_CFO		0xFF00	/**< Coarse frequency offset */

/* htphy PhyRxStatus_5: */
#define PRXS5_HTPHY_FFO	        0x00FF	/**< Fine frequency offset */
#define PRXS5_HTPHY_AR	        0xFF00	/**< Advance Retard */

/* ACPHY RxStatus defs */

/* ACPHY PhyRxStatus_0: */
#define PRXS0_ACPHY_FT_MASK      0x0003  /**< CCK, OFDM, HT, VHT */
#define PRXS0_ACPHY_CLIP_MASK    0x000C  /**< clip count adjustment steps by AGC */
#define PRXS0_ACPHY_CLIP_SHIFT        2
#define PRXS0_ACPHY_UNSRATE      0x0010  /**< PHY received a frame with unsupported rate */
#define PRXS0_ACPHY_BAND5G       0x0020  /**< Rx Band indication: 0 -> 2G, 1 -> 5G */
#define PRXS0_ACPHY_LCRS         0x0040  /**< CCK frame only: lost crs during cck frame reception */
#define PRXS0_ACPHY_SHORTH       0x0080  /**< Short Preamble (CCK), GF preamble (HT) */
#define PRXS0_ACPHY_PLCPFV       0x0100  /**< PLCP violation */
#define PRXS0_ACPHY_PLCPHCF      0x0200  /**< PLCP header integrity check failed */
#define PRXS0_ACPHY_MFCRS        0x0400  /**< Matched Filter CRS fired */
#define PRXS0_ACPHY_ACCRS        0x0800  /**< Autocorrelation CRS fired */
#define PRXS0_ACPHY_SUBBAND_MASK 0xF000  /**< FinalBWClassification:
	                                  * lower nibble Bitfield of sub-bands occupied by Rx frame
	                                  */
/* ACPHY PhyRxStatus_1: */
#define PRXS1_ACPHY_ANT_CORE0	0x0001	/* Antenna Config for core 0 */
#define PRXS1_ACPHY_BIT_HACK	0x0008
#define PRXS1_ACPHY_ANTCFG	0x00F0	/* Antenna Config */
#define PRXS1_ACPHY_COREMAP	0x000F	/**< Core enable bits for core0/1/2/3 */
#define PRXS1_ACPHY_SUBBAND_MASK_GEN2 0xFF00  /**< FinalBWClassification:
					 * lower byte Bitfield of sub-bands occupied by Rx frame
					 */
#define PRXS0_ACPHY_SUBBAND_SHIFT    12
#define PRXS1_ACPHY_SUBBAND_SHIFT_GEN2    8

/* acphy PhyRxStatus_3: */
#define PRXS2_ACPHY_RXPWR_ANT0	0xFF00	/**< Rx power on core 1 */
#define PRXS3_ACPHY_RXPWR_ANT1	0x00FF	/**< Rx power on core 1 */
#define PRXS3_ACPHY_RXPWR_ANT2	0xFF00	/**< Rx power on core 2 */
#define PRXS3_ACPHY_SNR_ANT0 0xFF00     /* SNR on core 0 */

/* acphy PhyRxStatus_4: */
/** FinalBWClassification:upper nibble of sub-bands occupied by Rx frame */
#define PRXS4_ACPHY_SUBBAND_MASK 0x000F
#define PRXS4_ACPHY_RXPWR_ANT3	0x00FF	/**< Rx power on core 3 */
#define PRXS4_ACPHY_SNR_ANT1 0xFF00     /* SNR on core 1 */

#define PRXS5_ACPHY_CHBWINNONHT_MASK 0x0003
#define PRXS5_ACPHY_CHBWINNONHT_20MHZ	0
#define PRXS5_ACPHY_CHBWINNONHT_40MHZ	1
#define PRXS5_ACPHY_CHBWINNONHT_80MHZ	2
#define PRXS5_ACPHY_CHBWINNONHT_160MHZ	3 /* includes 80+80 */
#define PRXS5_ACPHY_DYNBWINNONHT_MASK 0x0004

#define ACPHY_COREMAP(rxs)	((rxs)->lt80.PhyRxStatus_1 & PRXS1_ACPHY_COREMAP)
#define ACPHY_ANTMAP(rxs)	(((rxs)->lt80.PhyRxStatus_1 & PRXS1_ACPHY_ANTCFG) >> 4)
/** Get Rx power on core 0 */
#define ACPHY_RXPWR_ANT0(rxs)	(((rxs)->lt80.PhyRxStatus_2 & PRXS2_ACPHY_RXPWR_ANT0) >> 8)
/** Get Rx power on core 1 */
#define ACPHY_RXPWR_ANT1(rxs)	((rxs)->lt80.PhyRxStatus_3 & PRXS3_ACPHY_RXPWR_ANT1)
/** Get Rx power on core 2 */
#define ACPHY_RXPWR_ANT2(rxs)	(((rxs)->lt80.PhyRxStatus_3 & PRXS3_ACPHY_RXPWR_ANT2) >> 8)
/** Get Rx power on core 3 */
#define ACPHY_RXPWR_ANT3(rxs)	((rxs)->lt80.PhyRxStatus_4 & PRXS4_ACPHY_RXPWR_ANT3)
/* Get whether the rxpwr is hacked for 11b rssi WAR */
/* Tells whether the PhyRxStatus_2 & 0xFF00, PhyRxStatus_3 & 0xFFFF,
 *  PhyRxStatus_4 & 0xFF are hacked or not,0=disabled, 1=enabled
 */
#define ACPHY_HACK_PWR_STATUS(rxs)	(((rxs)->lt80.PhyRxStatus_1 & PRXS1_ACPHY_BIT_HACK) >> 3)

#define PRXS5_ACPHY_DYNBWINNONHT(rxs) ((rxs)->lt80.PhyRxStatus_5 & PRXS5_ACPHY_DYNBWINNONHT_MASK)
#define PRXS5_ACPHY_CHBWINNONHT(rxs) ((rxs)->lt80.PhyRxStatus_5 & PRXS5_ACPHY_CHBWINNONHT_MASK)

#define NPHY_MMPLCPLen(rxs)	((rxs)->lt80.PhyRxStatus_3 & PRXS3_nphy_MMPLCPLen_MASK)
#define D11N_MMPLCPLen(rxs)	((rxs)->lt80.PhyRxStatus_3 & PRXS3_nphy_MMPLCPLen_MASK)
#define D11HT_MMPLCPLen(rxs) ((((rxs)->lt80.PhyRxStatus_1 & PRXS1_HTPHY_MMPLCPLenL_MASK) >> 8) | \
			      (((rxs)->lt80.PhyRxStatus_2 & PRXS2_HTPHY_MMPLCPLenH_MASK) << 8))

/* ACPHY Gen2 RxStatus defs */

/* ACPHY Gen2 PhyRxStatus_0: */
#define PRXS0_ACPHY2_MUPPDU     0x1000	/**< 0: SU PPDU; 1: MU PPDU */
#define PRXS0_ACPHY2_OBSS       0xE000	/**< OBSS mitigation state */

/* ACPHY Gen2 PhyRxStatus_1: */
#define PRXS1_ACPHY2_SUBBAND_MASK 0xFF00  /**< FinalBWClassification:
	                                   * 8-bit bitfield of sub-bands occupied by Rx frame
	                                   */
#define PRXS1_ACPHY2_SUBBAND_SHIFT     8

/* ACPHY Gen2 PhyRxStatus_2: */
#define PRXS2_ACPHY2_MU_INT     0x003F	/**< MU interference processing type */

/* ACPHY Gen2 PhyRxStatus_5: */
#define PRXS5_ACPHY2_RSSI_FRAC  0xFF00	/**< RSSI fractional bits */

/* REV80 Defintions (corerev >= 80) */

/** Get RxStatus1 */
#define RXSTATUS1_REV_GE80(rxs)		((rxs)->ge80.RxStatus1)
#define RXSTATUS1_REV_LT80(rxs)		((rxs)->lt80.RxStatus1)

#define PHY_RXSTATUS1(corerev, rxs)	(D11REV_GE(corerev, 80) ? \
					(RXSTATUS1_REV_GE80(rxs)) : \
					(RXSTATUS1_REV_LT80(rxs)))

/* (Corerev >= 80) PhyRxStatus_2: */
#define PRXS2_RXPWR_ANT0_REV_GE80	0x00FF	/**< (corerev >= 80) Rx power on first antenna */
#define PRXS2_RXPWR_ANT1_REV_GE80	0xFF00	/**< (corerev >= 80) Rx power on second antenna */

/* (Corerev >= 80) PhyRxStatus_3: */
#define PRXS3_RXPWR_ANT2_REV_GE80	0x00FF	/**< (corerev >= 80) Rx power on third antenna */
#define PRXS3_RXPWR_ANT3_REV_GE80	0xFF00	/**
						 * (corerev >= 80) Rx power on fourth antenna.
						 *
						 * Note: For PHY revs 3 and > 4, OCL Status
						 * byte 0 will be reported if PHY register
						 * OCL_RxStatus_Ctrl is set to 0x2 or 0x6.
						 */

/** Get Rx power on ANT 0 */
#define RXPWR_ANT0_REV_GE129(rxs)		(D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						PhyRxStatus_2) & (PRXS2_RXPWR_ANT0_REV_GE80))

#define RXPWR_ANT0_REV_GE128(rxs)		((rxs)->ge128.PhyRxStatus_2 & \
						(PRXS2_RXPWR_ANT0_REV_GE80))
#define RXPWR_ANT0_REV_GE80(rxs)		((rxs)->ge80.PhyRxStatus_2 & \
						(PRXS2_RXPWR_ANT0_REV_GE80))

#define PHY_RXPWR_ANT0(corerev, rxs)		(D11REV_GE(corerev, 129) ? \
						(RXPWR_ANT0_REV_GE129(rxs)) : \
						(D11REV_GE(corerev, 128) ? \
						(RXPWR_ANT0_REV_GE128(rxs)) : \
						(D11REV_GE(corerev, 80) ? \
						(RXPWR_ANT0_REV_GE80(rxs)) : \
						(ACPHY_RXPWR_ANT0(rxs)))))

/** Get Rx power on ANT 1 */
#define RXPWR_ANT1_REV_GE129(rxs)		(((D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						PhyRxStatus_2) & \
						(PRXS2_RXPWR_ANT1_REV_GE80)) >> 8))

#define RXPWR_ANT1_REV_GE128(rxs)		(((rxs)->ge128.PhyRxStatus_2 & \
						(PRXS2_RXPWR_ANT1_REV_GE80)) >> 8)
#define RXPWR_ANT1_REV_GE80(rxs)		(((rxs)->ge80.PhyRxStatus_2 & \
						(PRXS2_RXPWR_ANT1_REV_GE80)) >> 8)

#define PHY_RXPWR_ANT1(corerev, rxs)		(D11REV_GE(corerev, 129) ? \
						(RXPWR_ANT1_REV_GE129(rxs)) : \
						(D11REV_GE(corerev, 128) ? \
						(RXPWR_ANT1_REV_GE128(rxs)) : \
						(D11REV_GE(corerev, 80) ? \
						(RXPWR_ANT1_REV_GE80(rxs)) : \
						(ACPHY_RXPWR_ANT1(rxs)))))

/** Get Rx power on ANT 2 */
#define RXPWR_ANT2_REV_GE129(rxs)		(D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						PhyRxStatus_3) & (PRXS3_RXPWR_ANT2_REV_GE80))

#define RXPWR_ANT2_REV_GE128(rxs)		((rxs)->ge128.PhyRxStatus_3 & \
						(PRXS3_RXPWR_ANT2_REV_GE80))
#define RXPWR_ANT2_REV_GE80(rxs)		((rxs)->ge80.PhyRxStatus_3 & \
						(PRXS3_RXPWR_ANT2_REV_GE80))

#define PHY_RXPWR_ANT2(corerev, rxs)		(D11REV_GE(corerev, 129) ? \
						(RXPWR_ANT2_REV_GE129(rxs)) : \
						(D11REV_GE(corerev, 128) ? \
						(RXPWR_ANT2_REV_GE128(rxs)) : \
						(D11REV_GE(corerev, 80) ? \
						(RXPWR_ANT2_REV_GE80(rxs)) : \
						(ACPHY_RXPWR_ANT2(rxs)))))

/** Get Rx power on ANT 3 */
#define RXPWR_ANT3_REV_GE129(rxs)		(((D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						PhyRxStatus_3) & \
						(PRXS3_RXPWR_ANT3_REV_GE80)) >> 8))

#define RXPWR_ANT3_REV_GE128(rxs)		(((rxs)->ge128.PhyRxStatus_3 & \
						(PRXS3_RXPWR_ANT3_REV_GE80)) >> 8)
#define RXPWR_ANT3_REV_GE80(rxs)		(((rxs)->ge80.PhyRxStatus_3 & \
						(PRXS3_RXPWR_ANT3_REV_GE80)) >> 8)

#define PHY_RXPWR_ANT3(corerev, rxs)		(D11REV_GE(corerev, 129) ? \
						(RXPWR_ANT3_REV_GE129(rxs)) : \
						(D11REV_GE(corerev, 128) ? \
						(RXPWR_ANT3_REV_GE128(rxs)) : \
						(D11REV_GE(corerev, 80) ? \
						(RXPWR_ANT3_REV_GE80(rxs)) : \
						(ACPHY_RXPWR_ANT3(rxs)))))

/* HECAPPHY PhyRxStatus_4: */
#define PRXS4_DYNBWINNONHT_MASK_REV_GE80	0x1000
#define PRXS4_DYNBWINNONHT_REV_GE129(rxs)	(D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						PhyRxStatus_4) & \
						PRXS4_DYNBWINNONHT_MASK_REV_GE80)

#define PRXS4_DYNBWINNONHT_REV_GE80(rxs)	((rxs)->ge80.PhyRxStatus_4 & \
						PRXS4_DYNBWINNONHT_MASK_REV_GE80)

#define PRXS_PHY_DYNBWINNONHT(corerev, rxs)     (D11REV_GE(corerev, 129) ? \
						PRXS4_DYNBWINNONHT_REV_GE129(rxs) : \
						(D11REV_GE(corerev, 80) ? \
						PRXS4_DYNBWINNONHT_REV_GE80(rxs) : \
						PRXS5_ACPHY_DYNBWINNONHT(rxs)))

/* HECAPPHY PhyRxStatus_8 (part of phyrxs_rem[2]) : */
#define PRXS8_CHBWINNONHT_MASK_REV_GE80		0x0100
#define PRXS8_CHBWINNONHT_REV_GE129(rxs)	(D11PHYSTSBUF_GE129_ACCESS_VAL(rxs, \
						phyrxs_rem[2]) & \
						PRXS8_CHBWINNONHT_MASK_REV_GE80)

#define PRXS8_CHBWINNONHT_REV_GE80(rxs)		((rxs)->ge80.phyrxs_rem[2] & \
						PRXS8_CHBWINNONHT_MASK_REV_GE80)

#define PRXS_PHY_CHBWINNONHT(corerev, rxs)	(D11REV_GE(corerev, 129) ? \
						PRXS8_CHBWINNONHT_REV_GE129(rxs) : \
						(D11REV_GE(corerev, 80) ? \
						PRXS8_CHBWINNONHT_REV_GE80(rxs) : \
						PRXS5_ACPHY_CHBWINNONHT(rxs)))

/* REV128 Defintions (corerev >= 128) */
/* PhyRxStatus_0: for rev128 */
#define	PRXS0_FT_MASK_GE128		0x0007	/**< 0-5: CCK, OFDM, HT, VHT, HE, AH. 6-15 Rsvd */
#define	PRXS0_HEF_MASK_GE128		0x0018	/**< HE format. 0-3: HE_SU, HE_SU_Range Ext,
						 * HE_MU, HE_TRIG
						 */
#define	PRXS0_HEF_SHIFT_GE128		3	/**< SHIFT bits for HE Format */
#define	PRXS0_FTFMT_MASK_GE128		0x1F	/**< FT and HE format */
#define	PRXS0_UNSRATE_GE128		0x0010	/**< PHY received a frame with unsupported rate */
#define PRXS0_BAND_GE128		0x0020	/**< 0 = 2.4G, 1 = 5G */
#define	PRXS0_LCRS_GE128		0x0040	/**< lost CRS during cck frame reception */
#define	PRXS0_SHORTH_GE128		0x0080	/**< Short Preamble */

#define	PRXS0_PLCPFV_GE128		0x0100	/**< PLCP violation */
#define	PRXS0_PLCPHCF_GE128		0x0200	/**< PLCP header integrity check failed */
#define PRXS0_ACPHY_MFCRS_GE128		0x0400	/**< Matched Filter CRS fired */
#define PRXS0_ACPHY_ACCRS_GE128		0x0800	/**< Autocorrelation CRS fired */
#define PRXS0_MUPPDU_GE128		0x1000	/**< 0 = SU PPDU, 1 = MU PPDU */
#define PRXS0_OBSS_GE128		0xE000	/**< OBSS mitigation state */

#define	PRXS0_HEF_MASK			0	/**< LT128 does not have this field */
#define	PRXS0_HEF_SHIFT			0	/**< LT128 does not have this field */

/* PhyRxStatus_0: for rev129 */
#define	PRXS0_SHORTH_GE129		0x0020	/**< Short Preamble */
#define PRXS0_BAND_GE129		0x0040	/**< 0 = 2.4G, 1 = 5G */
#define	PRXS0_UNSRATE_GE129		0x0080	/**< PHY received a frame with unsupported rate */

#define	PRXS0_PLCPFV_GE129		PRXS0_PLCPFV_GE128
#define	PRXS0_PLCPHCF_GE129		PRXS0_PLCPHCF_GE128
#define PRXS0_ACPHY_MFCRS_GE129		PRXS0_ACPHY_MFCRS_GE128
#define PRXS0_ACPHY_ACCRS_GE129		PRXS0_ACPHY_ACCRS_GE128
#define	PRXS0_LCRS_GE129		0x1000	/**< lost CRS during cck frame reception */
#define PRXS0_OBSS_GE129		PRXS0_OBSS_GE128

/* PHY RX status "Frame Type" field mask. */
#define PRXS_FT_MASK(corerev)		(D11REV_GE(corerev, 128) ? (PRXS0_FT_MASK_GE128) : \
					(PRXS0_FT_MASK))

#define D11PPDU_FT(rxh, rev)		((D11REV_GE(rev, 128) ? \
					(D11RXHDR_ACCESS_VAL(rxh, rev, RxStatus2) >> 8) : \
					D11PHYSTSBUF_ACCESS_VAL(rxh, rev, PhyRxStatus_0)) & \
						PRXS_FT_MASK(rev))

#define PRXS_HEF_MASK(corerev)		(D11REV_GE(corerev, 128) ? (PRXS0_HEF_MASK_GE128) : \
					(PRXS0_HEF_MASK))

#define PRXS_HEF_SHIFT(corerev)		(D11REV_GE(corerev, 128) ? (PRXS0_HEF_SHIFT_GE128) : \
					(PRXS0_HEF_SHIFT))

#define D11PPDU_HEF(rxh, rev)		(((D11REV_GE(rev, 128) ? \
					(D11RXHDR_ACCESS_VAL(rxh, rev, RxStatus2) >> 8) : \
					0) & PRXS_HEF_MASK(rev)) >> PRXS_HEF_SHIFT(rev))

#define PRXS_FTFMT_MASK(corerev)	(D11REV_GE(corerev, 128) ? (PRXS0_FTFMT_MASK_GE128) : \
					(PRXS0_FT_MASK))

#define D11PPDU_FTFMT(rxh, rev)		((D11REV_GE(rev, 128) ? \
					(D11RXHDR_ACCESS_VAL(rxh, rev, RxStatus2) >> 8) : \
					D11PHYSTSBUF_ACCESS_VAL(rxh, rev, PhyRxStatus_0)) & \
						PRXS_FTFMT_MASK(rev))

#define PRXS_SHORTH(corerev)		(D11REV_GE(corerev, 129) ? (PRXS0_SHORTH_GE129) : \
					(D11REV_GE(corerev, 128) ? (PRXS0_SHORTH_GE128) : \
					(PRXS0_SHORTH)))

/**
 * ACPHY PhyRxStatus0 SubBand (FinalBWClassification) bit defs
 * FinalBWClassification is a 4 bit field, each bit representing one 20MHz sub-band
 * of a channel.
 */
enum prxs_subband {
	PRXS_SUBBAND_20LL = 0x0001,
	PRXS_SUBBAND_20LU = 0x0002,
	PRXS_SUBBAND_20UL = 0x0004,
	PRXS_SUBBAND_20UU = 0x0008,
	PRXS_SUBBAND_40L  = 0x0003,
	PRXS_SUBBAND_40U  = 0x000C,
	PRXS_SUBBAND_80   = 0x000F,
	PRXS_SUBBAND_20LLL = 0x0001,
	PRXS_SUBBAND_20LLU = 0x0002,
	PRXS_SUBBAND_20LUL = 0x0004,
	PRXS_SUBBAND_20LUU = 0x0008,
	PRXS_SUBBAND_20ULL = 0x0010,
	PRXS_SUBBAND_20ULU = 0x0020,
	PRXS_SUBBAND_20UUL = 0x0040,
	PRXS_SUBBAND_20UUU = 0x0080,
	PRXS_SUBBAND_40LL = 0x0003,
	PRXS_SUBBAND_40LU = 0x000c,
	PRXS_SUBBAND_40UL = 0x0030,
	PRXS_SUBBAND_40UU = 0x00c0,
	PRXS_SUBBAND_80L = 0x000f,
	PRXS_SUBBAND_80U = 0x00f0,
	PRXS_SUBBAND_160 = 0x00ff
};

enum prxs_subband_bphy {
	PRXS_SUBBAND_BPHY_20L = 0x0000,
	PRXS_SUBBAND_BPHY_20U = 0x0001
};

/* ucode RxStatus1: */
#define	RXS_BCNSENT		0x8000
#define	RXS_TOFINFO		0x4000		/**< Rxed measurement frame processed by ucode */
#define	RXS_GRANTBT		0x2000		/* Indicate medium given to BT */
#define	RXS_SECKINDX_MASK(rev)	(D11REV_GE((rev), 64) ? 0x1FE0 : 0x07E0)
#define	RXS_SECKINDX_SHIFT	5
#define	RXS_DECERR		(1 << 4)
#define	RXS_DECATMPT		(1 << 3)
#define	RXS_PBPRES		(1 << 2)	/**< PAD bytes to make IP data 4 bytes aligned */
#define	RXS_RESPFRAMETX		(1 << 1)
#define	RXS_FCSERR		(1 << 0)

/* ucode RxStatus2: */
#define RXS_AMSDU_MASK		1
#define RXS_AGGTYPE_MASK	0x6
#define RXS_AGGTYPE_SHIFT	1
#define RXS_AMSDU_FIRST		1
#define RXS_AMSDU_INTERMEDIATE	0
#define RXS_AMSDU_LAST		2
#define RXS_AMSDU_N_ONE		3
#define RXS_TKMICATMPT		(1 << 3)
#define RXS_TKMICERR		(1 << 4)
#define RXS_PHYRXST_PRISEL_CLR	(1 << 5)
						/* packet was received while the antenna	*/
						/* (prisel) had been granted to BT.		*/
#define	RXS_S_MPDU		(1 << 7)	/**< Identify S MPDU frame */
#define RXS_PHYRXST_VALID	(1 << 8)
#define RXS_BCNCLSG		(1 << 9)	/**< Coleasced beacon packet */
#define RXS_RXANT_MASK		0x3
#define RXS_RXANT_SHIFT_LT80	12
#define RXS_RXANT_SHIFT_GE80	5

/* Bit definitions for MRXS word for short rx status. */
/* RXSS = RX Status Short */
#define RXSS_AMSDU_MASK         1	/**< 1: AMSDU */
#define RXSS_AGGTYPE_MASK     0x6	/**< 0 intermed, 1 first, 2 last, 3 single/non-AMSDU */
#define	RXSS_AGGTYPE_SHIFT      1
#define RXSS_PBPRES       (1 << 3)	/**< two-byte PAD prior to plcp */
#define RXSS_HDRSTS       (1 << 4)	/**< header conversion status. 1 enabled, 0 disabled */
#define RXSS_RES_MASK        0xE0	/**< reserved */
#define RXSS_MSDU_CNT_MASK 0xFF00	/**< index of this AMSDU sub-frame in the AMSDU */
#define RXSS_MSDU_CNT_SHIFT     8

/* RxChan */
#define RXS_CHAN_40		0x1000
#define RXS_CHAN_5G		0x0800
#define	RXS_CHAN_ID_MASK	0x07f8
#define	RXS_CHAN_ID_SHIFT	3

#define C_BTCX_AGGOFF_BLE		(1 << 0)
#define C_BTCX_AGGOFF_A2DP		(1 << 1)
#define C_BTCX_AGGOFF_PER		(1 << 2)

#define BTCX_HFLG_NO_A2DP_BFR		(1 << 0) /**< no check a2dp buffer */
#define BTCX_HFLG_NO_CCK		(1 << 1) /**< no cck rate for null or cts2self */
#define BTCX_HFLG_NO_OFDM_FBR		(1 << 2) /**< no ofdm fbr for null or cts2self */
#define	BTCX_HFLG_NO_INQ_DEF		(1 << 3) /**< no defer inquery */
#define	BTCX_HFLG_GRANT_BT		(1 << 4) /**< always grant bt */
#define BTCX_HFLG_ANT2WL		(1 << 5) /**< force prisel to wl */
#define BTCX_HFLG_PS4ACL		(1 << 7) /**< use ps null for unsniff acl */
#define BTCX_HFLG_DYAGG			(1 << 8) /**< dynamic tx aggregation */
#define BTCX_HFLG_SKIPLMP		(1 << 10) /**< no LMP check for 4331 (w 20702 A1/A3) */

#define BTCX_HFLG2_TRAP_RFACTIVE		(1 << 0) /* trap when RfActive too long */
#define BTCX_HFLG2_TRAP_TXCONF		(1 << 1) /* trap when coex grants txconf late */
#define BTCX_HFLG2_TRAP_ANTDLY		(1 << 2) /* trap when coex grants antdly late */
#define BTCX_HFLG2_TRAP_BTTYPE		(1 << 3) /* trap when illegal BT tasktype receive */
/* Bit definitions for M_BTCX_CONFIG */
#define BTCX_CONFIG_FORCE_TRAP		(1 << 13) /* Force a specific BTCoex TRAP when set */

/* BTCX_CONFIG bits */
#define	C_BTCX_CONFIG_LOW_RSSI		(1 << 7)
#define C_BTCX_CONFIG_BT_STROBE		(1 << 9)
#define C_BTCX_CONFIG_SCO_PROT		(1 << 10)
#define C_BTCX_CFG_CMN_CTS2SELF		(1 << 11)

#define BTC_PARAMS_FW_START_IDX		1000	/**< starting index of FW only btc params */
/** BTC_PARAMS_FW definitions */
typedef enum
{
	// allow rx-agg to be re-enabled after SCO session completes
	BTC_FW_RX_REAGG_AFTER_SCO	= BTC_PARAMS_FW_START_IDX,
	// RSSI threshold at which SCO grant/deny limits are changed dynamically
	BTC_FW_RSSI_THRESH_SCO		= BTC_PARAMS_FW_START_IDX + 1,
	// Enable the dynamic LE scan priority
	BTC_FW_ENABLE_DYN_LESCAN_PRI	= BTC_PARAMS_FW_START_IDX + 2,
	// If Tput(mbps) is above this, then share antenna with BT's LE_SCAN packet type.
	BTC_FW_LESCAN_LO_TPUT_THRESH	= BTC_PARAMS_FW_START_IDX + 3,
	// If Tput(mbps) is below this, then share antenna with BT's LE_SCAN packet type.
	// sampled once a second.
	BTC_FW_LESCAN_HI_TPUT_THRESH	= BTC_PARAMS_FW_START_IDX + 4,
	// Numbers of denials before granting LS scans
	BTC_FW_LESCAN_GRANT_INT		= BTC_PARAMS_FW_START_IDX + 5,
	// number of times algorighm changes lescn pri
	BTC_FW_LESCAN_ALG_CNT		= BTC_PARAMS_FW_START_IDX + 6,
	// RSSI threshold at which aggregation will be disabled during frequent BLE activity
	BTC_FW_RSSI_THRESH_BLE		= BTC_PARAMS_FW_START_IDX + 7,
	// AMPDU Aggregation state requested by BTC
	BTC_FW_AGG_STATE_REQ		= BTC_PARAMS_FW_START_IDX + 8,
	// Reserving space for parameters used in other projects
	BTC_FW_RSVD_1			= BTC_PARAMS_FW_START_IDX + 9,
	BTC_FW_HOLDSCO_LIMIT		= BTC_PARAMS_FW_START_IDX + 10,	// Lower Limit
	BTC_FW_HOLDSCO_LIMIT_HI		= BTC_PARAMS_FW_START_IDX + 11,	// Higher Limit
	BTC_FW_SCO_GRANT_HOLD_RATIO	= BTC_PARAMS_FW_START_IDX + 12,	// Low Ratio
	BTC_FW_SCO_GRANT_HOLD_RATIO_HI	= BTC_PARAMS_FW_START_IDX + 13,	// High Ratio
	BTC_FW_HOLDSCO_HI_THRESH	= BTC_PARAMS_FW_START_IDX + 14,	// BT Period Threshold
	BTC_FW_MOD_RXAGG_PKT_SZ_FOR_SCO	= BTC_PARAMS_FW_START_IDX + 15,
	/* Modify Rx Aggregation size when SCO/eSCO detected */
	BTC_FW_AGG_SIZE_LOW	= BTC_PARAMS_FW_START_IDX + 16,
	/* Agg size when BT period < 7500 ms */
	BTC_FW_AGG_SIZE_HIGH	= BTC_PARAMS_FW_START_IDX + 17,
	/* Agg size when BT period >= 7500 ms */
	BTC_FW_MOD_RXAGG_PKT_SZ_FOR_A2DP = BTC_PARAMS_FW_START_IDX + 18,
	BTC_FW_MAX_INDICES		// Maximum number of btc_fw sw registers
} btcParamsFirmwareDefinitions;

#define BTC_FW_NUM_INDICES		(BTC_FW_MAX_INDICES - BTC_PARAMS_FW_START_IDX)

// 1: Re-enable aggregation after SCO
#define BTC_FW_RX_REAGG_AFTER_SCO_INIT_VAL	1
#define BTC_FW_MOD_RXAGG_PKT_SZ_FOR_SCO_INIT_VAL	1
#define BTC_FW_MOD_RXAGG_PKT_SZ_FOR_A2DP_INIT_VAL	1
/* RX aggregation packet size when SCO */
#define BTC_FW_AGG_SIZE_LOW_INIT_VAL			1
/* aggregation size when BT period < BT_AMPDU_RESIZE_THRESH */
#define BTC_FW_AGG_SIZE_HIGH_INIT_VAL			2
/* aggregation size when BT period > BT_AMPDU_RESIZE_THRESH */
// 0: disable weak-rssi SCO coex feature. If > 0, adjust SCO COEX algorithm for weak RSSI scenario.
#define BTC_FW_RSSI_THRESH_SCO_INIT_VAL		0
// Enable LE Scan Priority Algorithm  0: Disable, 1: Enable
#define BTC_FW_ENABLE_DYN_LESCAN_PRI_INIT_VAL	0
// If WL Tput below 7 mbps, don't grant background LE Scans
#define BTC_FW_LESCAN_LO_TPUT_THRESH_INIT_VAL	7
// If WL Tput above 30 mbps, don't grant background LE Scans
#define BTC_FW_LESCAN_HI_TPUT_THRESH_INIT_VAL	30
// If LE Priority algorithm is triggered, grant one out of 2 LE_SCAN requests
#define BTC_FW_LESCAN_GRANT_INT_INIT_VAL	2
// If RSSI is weaker than -70 dBm and BLE activity is frequent, then disable
// RX aggregation, and clamp TX aggregation.
#define	BTC_FW_RSSI_THRESH_BLE_INIT_VAL		70
#define	BTC_FW_HOLDSCO_LIMIT_INIT_VAL		100
#define	BTC_FW_HOLDSCO_LIMIT_HI_INIT_VAL	10
#define	BTC_FW_SCO_GRANT_HOLD_RATIO_INIT_VAL	1500
#define	BTC_FW_SCO_GRANT_HOLD_RATIO_HI_INIT_VAL	1000
#define	BTC_FW_HOLDSCO_HI_THRESH_INIT_VAL	7400

#ifdef GPIO_TXINHIBIT
/* GPIO based TX_INHIBIT:SWWLAN-109270 */
typedef enum shm_macintstatus_ext_e {
	C_MISE_GPIO_TXINHIBIT_VAL_NBIT	= 0,
	C_MISE_GPIO_TXINHIBIT_INT_NBIT	= 1
} shm_macintstatus_ext_t;
#define C_MISE_GPIO_TXINHIBIT_VAL_MASK (1 << C_MISE_GPIO_TXINHIBIT_VAL_NBIT)
#define C_MISE_GPIO_TXINHIBIT_INT_MASK (1 << C_MISE_GPIO_TXINHIBIT_INT_NBIT)
#define M_MACINTSTATUS_EXT (0x3b3*2)
#define	M_PSM_SOFT_REGS	0x0
#define M_GPIO_TX_INHIBIT_TOUT (M_PSM_SOFT_REGS + (0x3be * 2))
#endif // endif

/* Pktclass bit definitions from rxstatus */
#define PKTCLASS_FLOWID_MASK	0x0001
#define PKTCLASS_A1_MASK	0x0002
#define PKTCLASS_A2_MASK	0x0004
#define PKTCLASS_A3_MASK	0x0008
#define PKTCLASS_TID_MASK	0x0010
#define PKTCLASS_AMPDU_MASK	0x0020
#define PKTCLASS_AMSDU_DA_MASK	0x0040
#define PKTCLASS_AMSDU_SA_MASK	0x0080
#define PKTCLASS_FC_MASK	0x0100

/* HWA2.a block in 43684 behaves slightly different than expected.
 * Instead of marking AMPDU boundary on bit-5, its marking bit-5
 * as an ampdu epoch which just flips on every AMPDU.
 * So to check for a fast path, exclude AMPDU bit.
 * Below bits are also skipped.
 *
 * 1. FLOWID is not checked by HW. So skip that.
 * 2. A3 address would change when AMSDU/non-AMSDU frames come in.
 * 3. SA/DA address would change if there are multiple end points behind router.
 * With HW header conversion enabled, no need to tag same DA/SA frames.
 * Mixed frames could be sent up.
 *
 */
#define PKTCLASS_FAST_PATH_MASK		(PKTCLASS_A1_MASK | PKTCLASS_A2_MASK | PKTCLASS_FC_MASK)

/** Scratch Reg defs */
typedef enum
{
	S_RSV0 = 0,
	S_RSV1,
	S_RSV2,

	/* scratch registers for Dot11-constants */
	S_DOT11_CWMIN,		/**< CW-minimum					0x03 */
	S_DOT11_CWMAX,		/**< CW-maximum					0x04 */
	S_DOT11_CWCUR,		/**< CW-current					0x05 */
	S_DOT11_SRC_LMT,	/**< short retry count limit			0x06 */
	S_DOT11_LRC_LMT,	/**< long retry count limit			0x07 */
	S_DOT11_DTIMCOUNT,	/**< DTIM-count					0x08 */

	/* Tx-side scratch registers */
	S_SEQ_NUM,		/**< hardware sequence number reg			0x09 */
	S_SEQ_NUM_FRAG,		/**< seq-num for frags (Set at the start os MSDU	0x0A */
	S_FRMRETX_CNT,		/**< frame retx count				0x0B */
	S_SSRC,			/**< Station short retry count			0x0C */
	S_SLRC,			/**< Station long retry count			0x0D */
	S_EXP_RSP,		/**< Expected response frame			0x0E */
	S_OLD_BREM,		/**< Remaining backoff ctr			0x0F */
	S_OLD_CWWIN,		/**< saved-off CW-cur				0x10 */
	S_TXECTL,		/**< TXE-Ctl word constructed in scr-pad		0x11 */
	S_CTXTST,		/**< frm type-subtype as read from Tx-descr	0x12 */

	/* Rx-side scratch registers */
	S_RXTST,		/**< Type and subtype in Rxframe			0x13 */

	/* Global state register */
	S_STREG,		/**< state storage actual bit maps below		0x14 */

	S_TXPWR_SUM,		/**< Tx power control: accumulator		0x15 */
	S_TXPWR_ITER,		/**< Tx power control: iteration			0x16 */
	S_RX_FRMTYPE,		/**< Rate and PHY type for frames			0x17 */
	S_THIS_AGG,		/**< Size of this AGG (A-MSDU)			0x18 */

	S_KEYINDX,		/*						0x19 */
	S_RXFRMLEN,		/**< Receive MPDU length in bytes			0x1A */

	/* Receive TSF time stored in SCR */
	S_RXTSFTMRVAL_WD3,	/**< TSF value at the start of rx			0x1B */
	S_RXTSFTMRVAL_WD2,	/**< TSF value at the start of rx			0x1C */
	S_RXTSFTMRVAL_WD1,	/**< TSF value at the start of rx			0x1D */
	S_RXTSFTMRVAL_WD0,	/**< TSF value at the start of rx			0x1E */
	S_RXSSN,		/**< Received start seq number for A-MPDU BA	0x1F */
	S_RXQOSFLD,		/**< Rx-QoS field (if present)			0x20 */

	/* Scratch pad regs used in microcode as temp storage */
	S_TMP0,			/**< stmp0					0x21 */
	S_TMP1,			/**< stmp1					0x22 */
	S_TMP2,			/**< stmp2					0x23 */
	S_TMP3,			/**< stmp3					0x24 */
	S_TMP4,			/**< stmp4					0x25 */
	S_TMP5,			/**< stmp5					0x26 */
	S_PRQPENALTY_CTR,	/**< Probe response queue penalty counter		0x27 */
	S_ANTCNT,		/**< unsuccessful attempts on current ant.	0x28 */
	S_SYMBOL,		/**< flag for possible symbol ctl frames		0x29 */
	S_RXTP,			/**< rx frame type				0x2A */
	S_STREG2,		/**< extra state storage				0x2B */
	S_STREG3,		/**< even more extra state storage		0x2C */
	S_STREG4,		/**< ...						0x2D */
	S_STREG5,		/**< remember to initialize it to zero		0x2E */

	S_RXBW,			/**< rx bandwidth, from phyreg 0x143		0x2F */
	S_UPTR,			/* Use this to initialize utrace */
	S_ADJPWR_IDX,
	S_CUR_PTR,		/**< Temp pointer for A-MPDU re-Tx SHM table	0x32 */
	S_REVID4,		/**< 0x33 */
	S_INDX,			/**< 0x34 */
	S_MBS_NBCN,		/**< 0x35 */
	S_ADDR1,		/**< 0x36 */
	S_ADDR2,		/**< 0x37 */
	S_ADDR3,		/**< 0x38 */
	S_ADDR4,		/**< 0x39 */
	S_ADDR5,		/**< 0x3A */
	S_TMP6,			/**< 0x3B */
	S_KEYINDX_BU,		/**< Backup for Key index 			0x3C */
	S_MFGTEST_TMP0,		/**< Temp register used for RX test calculations	0x3D */
	S_RXESN,		/**< Received end sequence number for A-MPDU BA	0x3E */
	S_STREG6,		/**< 0x3F */
} ePsmScratchPadRegDefinitions;

#define C_STREG_SLOWCAL_PD_NBIT 0x00000004        /* BIT 2 slow clock cal is pending */
#define C_STREG_SLOWCAL_DN_NBIT 0x00000008        /* BIT 3 slow clock cal is done */

#define S_BEACON_INDX	S_OLD_BREM
#define S_PRS_INDX	S_OLD_CWWIN
#define S_BTCX_BT_DUR	S_REVID4
#define S_PHYTYPE	S_SSRC
#define S_PHYVER	S_SLRC

/* IHR offsets */
#define PHY_CTRL		0x49

#define TSF_TMR_TSF_L		0x119
#define TSF_TMR_TSF_ML		0x11A
#define TSF_TMR_TSF_MU		0x11B
#define TSF_TMR_TSF_H		0x11C

#define TSF_GPT_0_STAT		0x123
#define TSF_GPT_1_STAT		0x124
#define TSF_GPT_0_CTR_L		0x125
#define TSF_GPT_1_CTR_L		0x126
#define TSF_GPT_0_CTR_H		0x127
#define TSF_GPT_1_CTR_H		0x128
#define TSF_GPT_0_VAL_L		0x129
#define TSF_GPT_1_VAL_L		0x12A
#define TSF_GPT_0_VAL_H		0x12B
#define TSF_GPT_1_VAL_H		0x12C

/* GPT_2 is corerev >= 3 */
#define TSF_GPT_2_STAT		0x133
#define TSF_GPT_2_CTR_L		0x134
#define TSF_GPT_2_CTR_H		0x135
#define TSF_GPT_2_VAL_L		0x136
#define TSF_GPT_2_VAL_H		0x137

/* Slow timer registers */
#define SLOW_CTRL		0x150
#define SLOW_TIMER_L		0x151
#define SLOW_TIMER_H		0x152
#define SLOW_FRAC		0x153
#define FAST_PWRUP_DLY		0x154

/* IHR PHY_CTRL STAT values */
#define PHY_CTRL_MC		(1 << 1)
#define PHY_CTRL_RESTORESTART	(1 << 14)

/* PSO mode */
#define PSO_CTRL		0x290
#define PSO_RXWORD_WMK		0x291
#define PSO_RXCNT_WMK		0x292

/* IHR TSF_GPT STAT values */
#define TSF_GPT_PERIODIC	(1 << 12)
#define TSF_GPT_ADJTSF		(1 << 13)
#define TSF_GPT_USETSF		(1 << 14)
#define TSF_GPT_ENABLE		(1 << 15)

/* IHR SLOW_CTRL values */
#define SLOW_CTRL_PDE		(1 << 0)
#define SLOW_CTRL_FD		(1 << 8)

/* PSO CTRL values */
#define PSO_FRM_SUPPRESS	(1 << 8)
#define PSO_MODE			(1 << 0)

/** ucode mac statistic counters in shared memory */
#define MACSTAT_OFFSET_SZ 64

/* MACSTAT offset to SHM address */
#define MACSTAT_ADDR(x, offset) (M_PSM2HOST_STATS(x) + 2 * (offset))

/** ucode mac statistic counters offset in shared memory */
typedef enum {
	MCSTOFF_TXFRAME = 0,
	MCSTOFF_TXRTSFRM = 1,
	MCSTOFF_TXCTSFRM = 2,
	MCSTOFF_TXACKFRM = 3,
	MCSTOFF_TXDNLFRM = 4,
	MCSTOFF_TXBCNFRM = 5,		/**< 5 */
	MCSTOFF_TXFUNFL = 6,		/**< 6ea (number of tx/rx fifo) */
	MCSTOFF_TXAMPDU = 12,
	MCSTOFF_TXMPDU = 13,
	MCSTOFF_TXTPLUNFL = 14,
	MCSTOFF_TXPHYERR = 15,
	MCSTOFF_RXGOODUCAST = 16,
	MCSTOFF_RXGOODOCAST = 17,
	MCSTOFF_RXFRMTOOLONG = 18,
	MCSTOFF_RXFRMTOOSHRT = 19,
	MCSTOFF_RXANYERR = 20,		/**< 20 */
	MCSTOFF_RXBADFCS = 21,
	MCSTOFF_RXBADPLCP = 22,
	MCSTOFF_RXCRSGLITCH = 23,
	MCSTOFF_RXSTRT = 24,
	MCSTOFF_RXDFRMUCASTMBSS = 25,	/**< 25 */
	MCSTOFF_RXMFRMUCASTMBSS = 26,
	MCSTOFF_RXCFRMUCAST = 27,
	MCSTOFF_RXRTSUCAST = 28,
	MCSTOFF_RXCTSUCAST = 29,
	MCSTOFF_RXACKUCAST = 30,		/**< 30 */
	MCSTOFF_RXDFRMOCAST = 31,
	MCSTOFF_RXMFRMOCAST = 32,
	MCSTOFF_RXCFRMOCAST = 33,
	MCSTOFF_RXRTSOCAST = 34,
	MCSTOFF_RXCTSOCAST = 35,		/**< 35 */
	MCSTOFF_RXDFRMMCAST = 36,
	MCSTOFF_RXMFRMMCAST = 37,
	MCSTOFF_RXCFRMMCAST = 38,
	MCSTOFF_RXBEACONMBSS = 39,
	MCSTOFF_RXDFRMUCASTOBSS = 40,	/**< 40 */
	MCSTOFF_RXBEACONOBSS = 41,
	MCSTOFF_RXRSPTMOUT = 42,
	MCSTOFF_BCNTXCANCL = 43,
	MCSTOFF_RXNODELIM = 44,
	MCSTOFF_RXF0OVFL = 45,		/**< 45 */
	MCSTOFF_RXF1OVFL = 46,		/**< correv >= 40 */
	MCSTOFF_DBGOFF46_CNT = 46,		/**< correv < 40 */
	MCSTOFF_RXHLOVFL = 47,		/**< correv >= 40 */
	MCSTOFF_DBGOFF47_CNT = 47,		/**< correv < 40 */
	MCSTOFF_MISSBCNDBG = 48,		/**< correv >= 40 */
	MCSTOFF_DBGOFF48_CNT = 48,		/**< correv < 40 */
	MCSTOFF_PMQOVFL = 49,
	MCSTOFF_RXCGPRQFRM = 50,		/**< 50 */
	MCSTOFF_RXCGPRSQOVFL = 51,
	MCSTOFF_TXCGPRSFAIL = 52,
	MCSTOFF_TXCGPRSSUC = 53,
	MCSTOFF_PRS_TIMEOUT = 54,
	MCSTOFF_TXRTSFAIL = 55,		/**< 55 */
	MCSTOFF_TXUCAST = 56,
	MCSTOFF_TXINRTSTXOP = 57,
	MCSTOFF_RXBACK = 58,
	MCSTOFF_TXBACK = 59,
	MCSTOFF_BPHYGLITCH = 60,		/**< 60 */
	MCSTOFF_RXDROP20S = 61,		/**< correv >= 40 */
	MCSTOFF_PHYWATCH = 61,		/**< correv < 40 */
	MCSTOFF_RXTOOLATE = 62,
	MCSTOFF_BPHY_BADPLCP = 63
} macstat_offset_t;

/** ucode mac statistic counters in shared memory, base addr defined in M_UCODE_MACSTAT1 */
typedef struct macstat1 {
	uint16 txndpa;                  /* + 0 (0x0) */
	uint16 txndp;                   /* + 1*2 (0x2) */
	uint16 txsf;                    /* + 2*2 (0x4) */
	uint16 txcwrts;                 /* + 3*2 (0x6) */
	uint16 txcwcts;                 /* + 4*2 (0x8) */
	uint16 txbfm;                   /* + 5*2 (0xa) */
	uint16 rxndpaucast;             /* + 6*2 (0xc) */
	uint16 bferptrdy;               /* + 7*2 (0xe) */
	uint16 rxsfucast;               /* + 8*2 (0x10) */
	uint16 rxcwrtsucast;            /* + 9*2 (0x12) */
	uint16 rxcwctsucast;            /* +10*2 (0x14) */
	uint16 rx20s;                  /* +11*2 (0x16) */
	uint16 bcntrim;                  /* +12*2 (0x18) */
	uint16 btc_rfact_l;             /* +13*2 (0x1a) */
	uint16 btc_rfact_h;             /* +14*2 (0x1c) */
	uint16 btc_txconf_l;            /* +15*2 (0x1e) : cnt */
	uint16 btc_txconf_h;            /* +16*2 (0x20) : cnt */
	uint16 btc_txconf_durl;         /* +17*2 (0x22) : dur */
	uint16 btc_txconf_durh;         /* +18*2 (0x24) : dur */
	uint16 rxsecrssi0;              /* +19*2 (0x26) : high bin */
	uint16 rxsecrssi1;              /* +20*2 (0x28) : med bin */
	uint16 rxsecrssi2;              /* +21*2 (0x2a) : low bin */
	uint16 rxpri_durl;              /* +22*2 (0x2c) : dur */
	uint16 rxpri_durh;              /* +23*2 (0x2e) : dur */
	uint16 rxsec20_durl;            /* +24*2 (0x30) : dur */
	uint16 rxsec20_durh;            /* +25*2 (0x32) : dur */
	uint16 rxsec40_durl;            /* +26*2 (0x34) : dur */
	uint16 rxsec40_durh;            /* +27*2 (0x36) : dur */
} macstat1_t;

#define MX_UCODEX_MACSTAT (0x40 * 2)
/* ucodex mac statistic counters in shared memory */
#define MACXSTAT_OFFSET_SZ 6

/* psm2 statistic counters in shared memory, base addr defined in MX_PSM2HOST_STATS */
typedef enum {
	MCXSTOFF_MACXSUSP = 0,
	MCXSTOFF_M2VMSG = 1,
	MCXSTOFF_V2MMSG = 2,
	MCXSTOFF_MBOXOUT = 3,
	MCXSTOFF_MUSND = 4,
	MCXSTOFF_SFB2V = 5
} macxstat_offset_t;

/* dot11 core-specific control flags */
#define SICF_MCLKE		0x0001          /* Mac core clock Enable */
#define SICF_FCLKON		0x0002          /* Force clocks On */
#define	SICF_PCLKE		0x0004		/**< PHY clock enable */
#define	SICF_PRST		0x0008		/**< PHY reset */
#define	SICF_MPCLKE_SHIFT	4
#define	SICF_MPCLKE		0x0010		/**< MAC PHY clockcontrol enable */
#define	SICF_FASTCLKRQ		0x0020		/**< bit5, introduced in rev 130 */

/* NOTE: the following bw bits only apply when the core is attached
 * to a NPHY (and corerev >= 11 which it will always be for NPHYs).
 */
#define	SICF_BWMASK		0x08c0		/**< phy clock mask (b6 & b7 & b11) */
#define	SICF_BW160		0x0800		/**< 160MHz BW */
#define	SICF_BW80		0x00c0		/**< 80MHz BW */
#define	SICF_BW40		0x0080		/**< 40MHz BW (160MHz phyclk) */
#define	SICF_BW20		0x0040		/**< 20MHz BW (80MHz phyclk) */
#define	SICF_BW10		0x0000		/**< 10MHz BW (40MHz phyclk) */
#define	SICF_DAC		0x0300		/**< Highspeed DAC mode control field */
#define	SICF_GMODE		0x2000		/**< gmode enable */

/* Macmode / Phymode / Opmode are used interchangebly sometimes
 * even though they all mean the same. Going ahead with the HW
 * signal name - using phymode here on (even though we know its
 * a misnomer). Applicable to d11 corerev >= 50 ---- ACPHY only
 */
#define SICF_PHYMODE_SHIFT	16
#define	SICF_PHYMODE		0xf0000		/**< mask */

/* dot11 core-specific status flags */
#define	SISF_2G_PHY		0x0001		/**< 2.4G capable phy (corerev >= 5) */
#define	SISF_5G_PHY		0x0002		/**< 5G capable phy (corerev >= 5) */
#define	SISF_FCLKA		0x0004		/**< FastClkAvailable (corerev >= 5) */
#define	SISF_DB_PHY		0x0008		/**< Dualband phy (corerev >= 11) */

/* === End of MAC reg, Beginning of PHY(b/a/g/n) reg, radio and LPPHY regs are separated === */

#define	BPHY_REG_OFT_BASE	0x0
/* offsets for indirect access to bphy registers */
#define	BPHY_BB_CONFIG		0x01
#define	BPHY_ADCBIAS		0x02
#define	BPHY_ANACORE		0x03
#define	BPHY_PHYCRSTH		0x06
#define	BPHY_TEST		0x0a
#define	BPHY_PA_TX_TO		0x10
#define	BPHY_SYNTH_DC_TO	0x11
#define	BPHY_PA_TX_TIME_UP	0x12
#define	BPHY_RX_FLTR_TIME_UP	0x13
#define	BPHY_TX_POWER_OVERRIDE	0x14
#define	BPHY_RF_OVERRIDE	0x15
#define	BPHY_RF_TR_LOOKUP1	0x16
#define	BPHY_RF_TR_LOOKUP2	0x17
#define	BPHY_COEFFS		0x18
#define	BPHY_PLL_OUT		0x19
#define	BPHY_REFRESH_MAIN	0x1a
#define	BPHY_REFRESH_TO0	0x1b
#define	BPHY_REFRESH_TO1	0x1c
#define	BPHY_RSSI_TRESH		0x20
#define	BPHY_IQ_TRESH_HH	0x21
#define	BPHY_IQ_TRESH_H		0x22
#define	BPHY_IQ_TRESH_L		0x23
#define	BPHY_IQ_TRESH_LL	0x24
#define	BPHY_GAIN		0x25
#define	BPHY_LNA_GAIN_RANGE	0x26
#define	BPHY_JSSI		0x27
#define	BPHY_TSSI_CTL		0x28
#define	BPHY_TSSI		0x29
#define	BPHY_TR_LOSS_CTL	0x2a
#define	BPHY_LO_LEAKAGE		0x2b
#define	BPHY_LO_RSSI_ACC	0x2c
#define	BPHY_LO_IQMAG_ACC	0x2d
#define	BPHY_TX_DC_OFF1		0x2e
#define	BPHY_TX_DC_OFF2		0x2f
#define	BPHY_PEAK_CNT_THRESH	0x30
#define	BPHY_FREQ_OFFSET	0x31
#define	BPHY_DIVERSITY_CTL	0x32
#define	BPHY_PEAK_ENERGY_LO	0x33
#define	BPHY_PEAK_ENERGY_HI	0x34
#define	BPHY_SYNC_CTL		0x35
#define	BPHY_TX_PWR_CTRL	0x36
#define BPHY_TX_EST_PWR 	0x37
#define	BPHY_STEP		0x38
#define	BPHY_WARMUP		0x39
#define	BPHY_LMS_CFF_READ	0x3a
#define	BPHY_LMS_COEFF_I	0x3b
#define	BPHY_LMS_COEFF_Q	0x3c
#define	BPHY_SIG_POW		0x3d
#define	BPHY_RFDC_CANCEL_CTL	0x3e
#define	BPHY_HDR_TYPE		0x40
#define	BPHY_SFD_TO		0x41
#define	BPHY_SFD_CTL		0x42
#define	BPHY_DEBUG		0x43
#define	BPHY_RX_DELAY_COMP	0x44
#define	BPHY_CRS_DROP_TO	0x45
#define	BPHY_SHORT_SFD_NZEROS	0x46
#define	BPHY_DSSS_COEFF1	0x48
#define	BPHY_DSSS_COEFF2	0x49
#define	BPHY_CCK_COEFF1		0x4a
#define	BPHY_CCK_COEFF2		0x4b
#define	BPHY_TR_CORR		0x4c
#define	BPHY_ANGLE_SCALE	0x4d
#define	BPHY_TX_PWR_BASE_IDX	0x4e
#define	BPHY_OPTIONAL_MODES2	0x4f
#define	BPHY_CCK_LMS_STEP	0x50
#define	BPHY_BYPASS		0x51
#define	BPHY_CCK_DELAY_LONG	0x52
#define	BPHY_CCK_DELAY_SHORT	0x53
#define	BPHY_PPROC_CHAN_DELAY	0x54
#define	BPHY_DDFS_ENABLE	0x58
#define	BPHY_PHASE_SCALE	0x59
#define	BPHY_FREQ_CONTROL	0x5a
#define	BPHY_LNA_GAIN_RANGE_10	0x5b
#define	BPHY_LNA_GAIN_RANGE_32	0x5c
#define	BPHY_OPTIONAL_MODES	0x5d
#define	BPHY_RX_STATUS2		0x5e
#define	BPHY_RX_STATUS3		0x5f
#define	BPHY_DAC_CONTROL	0x60
#define	BPHY_ANA11G_FILT_CTRL	0x62
#define	BPHY_REFRESH_CTRL	0x64
#define	BPHY_RF_OVERRIDE2	0x65
#define	BPHY_SPUR_CANCEL_CTRL	0x66
#define	BPHY_FINE_DIGIGAIN_CTRL	0x67
#define	BPHY_RSSI_LUT		0x88
#define	BPHY_RSSI_LUT_END	0xa7
#define	BPHY_TSSI_LUT		0xa8
#define	BPHY_TSSI_LUT_END	0xc7
#define	BPHY_TSSI2PWR_LUT	0x380
#define	BPHY_TSSI2PWR_LUT_END	0x39f
#define	BPHY_LOCOMP_LUT		0x3a0
#define	BPHY_LOCOMP_LUT_END	0x3bf
#define	BPHY_TXGAIN_LUT		0x3c0
#define	BPHY_TXGAIN_LUT_END	0x3ff

/* Bits in BB_CONFIG: */
#define	PHY_BBC_ANT_MASK	0x0180
#define	PHY_BBC_ANT_SHIFT	7
#define	BB_DARWIN		0x1000
#define BBCFG_RESETCCA		0x4000
#define BBCFG_RESETRX		0x8000

/* Bits in phytest(0x0a): */
#define	TST_DDFS		0x2000
#define	TST_TXFILT1		0x0800
#define	TST_UNSCRAM		0x0400
#define	TST_CARR_SUPP		0x0200
#define	TST_DC_COMP_LOOP	0x0100
#define	TST_LOOPBACK		0x0080
#define	TST_TXFILT0		0x0040
#define	TST_TXTEST_ENABLE	0x0020
#define	TST_TXTEST_RATE		0x0018
#define	TST_TXTEST_PHASE	0x0007

/* phytest txTestRate values */
#define	TST_TXTEST_RATE_1MBPS	0
#define	TST_TXTEST_RATE_2MBPS	1
#define	TST_TXTEST_RATE_5_5MBPS	2
#define	TST_TXTEST_RATE_11MBPS	3
#define	TST_TXTEST_RATE_SHIFT	3

typedef struct shm_mbss_prq_entry_s shm_mbss_prq_entry_t;
BWL_PRE_PACKED_STRUCT struct shm_mbss_prq_entry_s {
	struct ether_addr ta;
	uint8 prq_info[2];
	uint8 time_stamp;
	uint8 flags;	/**< bit 0 HT STA Indication, bit 7:1 Reserved */
} BWL_POST_PACKED_STRUCT;

typedef enum shm_mbss_prq_ft_e {
	SHM_MBSS_PRQ_FT_CCK,
	SHM_MBSS_PRQ_FT_OFDM,
	SHM_MBSS_PRQ_FT_MIMO,
	SHM_MBSS_PRQ_FT_RESERVED
} shm_mbss_prq_ft_t;

#define SHM_MBSS_PRQ_FT_COUNT SHM_MBSS_PRQ_FT_RESERVED

#define SHM_MBSS_PRQ_ENT_FRAMETYPE(entry)      ((entry)->prq_info[0] & 0x3)
#define SHM_MBSS_PRQ_ENT_UPBAND(entry)         ((((entry)->prq_info[0] >> 2) & 0x1) != 0)

/** What was the index matched? */
#define SHM_MBSS_PRQ_ENT_UC_BSS_IDX(entry)     (((entry)->prq_info[0] >> 2) & 0x3)
#define SHM_MBSS_PRQ_ENT_PLCP0(entry)          ((entry)->prq_info[1])

/** Was this directed to a specific SSID or BSSID? If bit clear, quantity known */
#define SHM_MBSS_PRQ_ENT_DIR_SSID(entry) \
	((((entry)->prq_info[0] >> 6) == 0) || ((entry)->prq_info[0] >> 6) == 1)
#define SHM_MBSS_PRQ_ENT_DIR_BSSID(entry) \
	((((entry)->prq_info[0] >> 6) == 0) || ((entry)->prq_info[0] >> 6) == 2)

#define SHM_MBSS_PRQ_ENT_TIMESTAMP(entry)	((entry)->time_stamp)
/** Was the probe request from a ht STA or a legacy STA */
#define SHM_MBSS_PRQ_ENT_HTSTA(entry)		((entry)->flags & 0x1)

typedef struct d11ac_tso_s d11ac_tso_t;

BWL_PRE_PACKED_STRUCT struct d11ac_tso_s {
	uint8 flag[3];
	uint8 sfh_hdr_offset;
	uint16 tso_mss;		/**< tso segment size */
	uint16 msdu_siz;	/**< msdu size */
	uint32 tso_payload_siz;	/**< total byte cnt in tcp payload */
	uint16 ip_hdr_offset;	/**< relative to the start of txd header */
	uint16 tcp_hdr_offset;	/**< relative to start of txd header */
} BWL_POST_PACKED_STRUCT;

#define TSO_HDR_TOE_FLAG_OFFSET			0

#define TOE_F0_HDRSIZ_NORMAL   (1 << 0)
#define TOE_F0_PASSTHROUGH     (1 << 1)
#define TOE_F0_TCPSEG_EN       (1 << 3)
#define TOE_F0_IPV4            (1 << 4)
#define TOE_F0_IPV6            (1 << 5)
#define TOE_F0_TCP             (1 << 6)
#define TOE_F0_UDP             (1 << 7)

#define TOE_F1_IPV4_CSUM_EN    (1 << 0)
#define TOE_F1_TCPUDP_CSUM_EN  (1 << 1)
#define TOE_F1_PSEUDO_CSUM_EN  (1 << 2)
#define TOE_F1_FRAG_ALLOW      (1 << 5)
#define TOE_F1_FRAMETYPE_1     (1 << 6)
#define TOE_F1_FRAMETYPE_2     (1 << 7)
#define TOE_F1_FT_MASK         (TOE_F1_FRAMETYPE_1 | TOE_F1_FRAMETYPE_2)
#define TOE_F1_FT_SHIFT        6

#define TOE_F2_TXD_HEAD_SHORT  (1 << 0)
#define TOE_F2_EPOCH           (1 << 1)
#define TOE_F2_EPOCH_EXT       (1 << 2)
#define TOE_F2_EPOCH_MASK      (TOE_F2_EPOCH | TOE_F2_EPOCH_EXT)
#define TOE_F2_EPOCH_SHIFT     1
#define TOE_F2_AMSDU_AGGR_EN   (1 << 4)
#define TOE_F2_AMSDU_CSUM_EN   (1 << 5)
#define TOE_F2_AMSDU_FS_MID    (1 << 6)
#define TOE_F2_AMSDU_FS_LAST   (1 << 7)

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#define SHM_BYT_CNT	0x2			/**< IHR location */
#define MAX_BYT_CNT	0x600			/**< Maximum frame len */

/* WOWL Template Regions */
#define WOWL_NS_CHKSUM		 (0x57 * 2)
#define WOWL_PSP_TPL_BASE   (0x334 * 2)
#define WOWL_GTK_MSG2             (0x434 * 2)
#define WOWL_NS_OFFLOAD     (0x634 * 2)
#define T_KEEPALIVE_0       (0x6b4 * 2)
#define T_KEEPALIVE_1       ((0x6b4 + 0x40) * 2)
#define WOWL_ARP_OFFLOAD    (0x734 * 2)
#define WOWL_TX_FIFO_TXRAM_BASE (0x774 * 2)

/* template regions for 11ac */
#define D11AC_WOWL_PSP_TPL_BASE   (0x4c0 * 2)
#define D11AC_WOWL_GTK_MSG2       (0x5c0 * 2)	/**< for core rev >= 42 */
#define WOWL_NS_OFFLOAD_GE42	 (0x7c0 * 2)
#define T_KEEPALIVE_0_GE42       (0x840 * 2)
#define T_KEEPALIVE_1_GE42       ((0x840 + 0x40) * 2)
#define WOWL_ARP_OFFLOAD_GE42    (0x8c0 * 2)
#define D11AC_WOWL_TX_FIFO_TXRAM_BASE   (0x900 * 2)	/**< GTKM2 for core rev >= 42 */

/* Event definitions */
#define WOWL_MAGIC       (1 << 0)	/**< Wakeup on Magic packet */
#define WOWL_NET         (1 << 1)	/**< Wakeup on Netpattern */
#define WOWL_DIS         (1 << 2)	/**< Wakeup on loss-of-link due to Disassoc/Deauth */
#define WOWL_RETR        (1 << 3)	/**< Wakeup on retrograde TSF */
#define WOWL_BCN         (1 << 4)	/**< Wakeup on loss of beacon */
#define WOWL_TST         (1 << 5)	/**< Wakeup after test */
#define WOWL_M1          (1 << 6)	/**< Wakeup after PTK refresh */
#define WOWL_EAPID       (1 << 7)	/**< Wakeup after receipt of EAP-Identity Req */
#define WOWL_PME_GPIO    (1 << 8)	/**< Wakeind via PME(0) or GPIO(1) */
#define WOWL_NEEDTKIP1   (1 << 9)	/**< need tkip phase 1 key to be updated by the driver */
#define WOWL_GTK_FAILURE (1 << 10)	/**< enable wakeup if GTK fails */
#define WOWL_EXTMAGPAT   (1 << 11)	/**< support extended magic packets */
#define WOWL_ARPOFFLOAD  (1 << 12)	/**< support ARP/NS offloading */
#define WOWL_WPA2        (1 << 13)	/**< read protocol version for EAPOL frames */
#define WOWL_KEYROT      (1 << 14)	/**< If the bit is set, use key rotaton */
#define WOWL_BCAST       (1 << 15)	/**< If the bit is set, frm received was bcast frame */

#define MAXBCNLOSS (1 << 13) - 1	/**< max 12-bit value for bcn loss */

/* UCODE shm view:
 * typedef struct {
 *         uint16 offset; // byte offset
 *         uint16 patternsize; // the length of value[.] in bytes
 *         uchar bitmask[MAXPATTERNSIZE/8]; // 16 bytes, the effect length is (patternsize+7)/8
 *         uchar value[MAXPATTERNSIZE]; // 128 bytes, the effect length is patternsize.
 *   } netpattern_t;
 */
#define NETPATTERNSIZE	(148) /* 128 value + 16 mask + 4 offset + 4 patternsize */
#define MAXPATTERNSIZE 128
#define MAXMASKSIZE	MAXPATTERNSIZE/8

/** Security Algorithm defines */
#define WOWL_TSCPN_SIZE 6
#define WOWL_TSCPN_COUNT  4			/**< 4 ACs */
#define WOWL_TSCPN_BLK_SIZE	(WOWL_TSCPN_SIZE * WOWL_TSCPN_COUNT)

#define	WOWL_SECSUITE_GRP_ALGO_MASK		0x0007
#define	WOWL_SECSUITE_GRP_ALGO_SHIFT	0
#define	WOWL_SECSUITE_ALGO_MASK			0x0700
#define	WOWL_SECSUITE_ALGO_SHIFT		8

#define EXPANDED_KEY_RNDS 10
#define EXPANDED_KEY_LEN  176 /* the expanded key from KEK (4*11*4, 16-byte state, 11 rounds) */

/* Organization of Template RAM is as follows
 *   typedef struct {
 *      uint8 AES_XTIME9DBE[1024];
 *	uint8 AES_INVSBOX[256];
 *	uint8 AES_KEYW[176];
 * } AES_TABLES_t;
 */
/* See dot11_firmware/diag/wmac_tcl/wmac_762_wowl_gtk_aes: proc write_aes_tables,
 *  for an example of writing those tables into the tx fifo buffer.
 */

typedef struct {
	uint16 MacTxControlLow;		/**< mac-tx-ctl-low word */
	uint16 MacTxControlHigh;	/**< mac-tx-ctl-high word */
	uint16 PhyTxControlWord;	/**< phy control word */
	uint16 PhyTxControlWord_1;	/**< extra phy control word for mimophy */
	union {
		uint16 XtraFrameTypes;	/**< frame type for RTS/FRAG fallback (used only for AES) */
		uint16 bssenc_pos;	/**< BssEnc includes key ID , for corerev >= 42 */
	} u1;
	uint8 plcp[6];			/**< plcp of template */

	/* For detailed definition of the above field,
	 * please see the general description of the tx descriptor
	 * at http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/TxDescriptor.
	 */

	uint16 mac_frmtype; /**< MAC frame type for GTK MSG2, can be
			     * dot11_data frame (0x20) or dot11_QoS_Data frame (0x22).
			     */
	uint16 frm_bytesize; /**< number of bytes in the template, it includes:
			      * PLCP, MAC header, IV/EIV, the data payload
			      * (eth-hdr and EAPOL-Key), TKIP MIC
			      */
	uint16 payload_wordoffset;	/**< the word offset of the data payload */

	uint16 seqnum;		/**< Sequence number for this frame */
	uint8  seciv[18]; /**< 10-byte TTAK used for TKIP, 8-byte IV/EIV.
			   * See <SecurityInitVector> in the general tx descriptor.
			   */
} wowl_templ_ctxt_t;

#define WOWL_TEMPL_CTXT_LEN 42	/**< For making sure that no PADs are needed */
#define WOWL_TEMPL_CTXT_FRMTYPE_DATA    0x2
#define WOWL_TEMPL_CTXT_FRMTYPE_QOS     0x22

/** constant tables required for AES key unwrapping for key rotation */
extern uint16 aes_invsbox[128];
extern uint16 aes_xtime9dbe[512];

#define MAX_MPDU_SPACE           (D11_TXH_LEN + 1538)

/* Bits in TXE_BMCCTL */
#define BMCCTL_INITREQ_SHIFT	0
#define BMC_CTL_DONE		(1 << BMCCTL_INITREQ_SHIFT)
#define BMCCTL_RESETSTATS_SHIFT	1
#define BMCCTL_TXBUFSIZE_SHIFT	2
#define BMCCTL_LOOPBACK_SHIFT	5
#define BMCCTL_TXBUFSZ_MASK	((1 << BMCCTL_LOOPBACK_SHIFT) - (1 << BMCCTL_TXBUFSIZE_SHIFT))
#define BMCCTL_CLKGATEEN_SHIFT  8

/* Bits in TXE_BMCConfig */
#define BMCCONFIG_BUFCNT_SHIFT		0
#define BMCCONFIG_DISCLKGATE_SHIFT	13
#define BMCCONFIG_BUFCNT_MASK	((1 << BMCCONFIG_DISCLKGATE_SHIFT) - (1 << BMCCONFIG_BUFCNT_SHIFT))
#define BMCCONFIG_BUFCNT_MASK_GE128	0x7fff

/* Bits in TXE_BMCStartAddr */
#define BMCSTARTADDR_STRTADDR_MASK	0x3ff

/* Bits in TXE_BMCDescrLen */
#define BMCDescrLen_ShortLen_SHIFT	0
#define BMCDescrLen_LongLen_SHIFT	8

/* Bits in TXE_BMCAllocCtl */
#define BMCAllocCtl_AllocCount_SHIFT		0
/* Rev==50 || Rev>52
*	BMCAllocCtl.AllocCount [0:10]
*	BMCAllocCtl.AllocThreshold [11:14]
* !Rev50
*	BMCAllocCtl.AllocCount [0:7]
*	BMCAllocCtl.AllocThreshold [8:15]
*/
#define BMCAllocCtl_AllocThreshold_SHIFT_Rev50	11
#define BMCAllocCtl_AllocThreshold_SHIFT	8

/* Bits in TXE_BMCCmd1 */
#define BMCCMD1_TIDSEL_SHIFT		1
#define BMCCMD1_RDSRC_SHIFT_LT128	6
#define BMCCmd1_RXMapPassThru_SHIFT	12

#define BMCCMD1_SELTYPE_NBIT		9
#define BMCCMD1_SELNUM_NBIT		1
#define BMCCMD1_RDSRC_SHIFT_GE128	12

/* Bits in TXE_BMCCmd */
#define BMCCmd_TIDSel_SHIFT		0
#define BMCCmd_Enable_SHIFT		4
#define BMCCmd_ReleasePreAlloc_SHIFT	5
#define BMCCmd_ReleasePreAllocAll_SHIFT	6
#define BMCCmd_UpdateBA_SHIFT		7
#define BMCCmd_Consume_SHIFT		8
#define BMCCmd_Aggregate_SHIFT		9
#define BMCCmd_UpdateRetryCount_SHIFT	10
#define BMCCmd_DisableTID_SHIFT		11

#define BMCCmd_BQSelType_TX	0
#define BMCCmd_BQSelType_RX	1
#define BMCCmd_BQSelType_Templ	2

/* Bits in TXE_BMCCMD for rev >= 128 */
#define BMCCmd_BQSelType_MASK_Rev128	0x0300
#define BMCCmd_BQSelType_SHIFT_Rev128	8
#define BMCCmd_Enable_SHIFT_rev128	10
#define BMCCmd_ReleasePreAllocAll_SHIFT_rev128	12

/* Bits in TXE_BMCCMD for rev >= 80 */
#define BMCCmd_BQSelType_MASK_Rev80	0x00c0
#define BMCCmd_BQSelType_SHIFT_Rev80	6
#define BMCCmd_Enable_SHIFT_rev80	8
#define BMCCmd_ReleasePreAllocAll_SHIFT_rev80	10

/* Bits in TXE_BMCCmd1 */
#define BMCCmd1_Minmaxappall_SHIFT	0
#define BMCCmd1_Minmaxlden_SHIFT	5
#define BMCCmd1_Minmaxffszlden_SHIFT	8
#define BMCCmd_Core1_Sel_MASK		0x2000

/* Bits in BMVpConfig */
#define BMCVPConfig_SingleVpModePortA_SHIFT	4

/* Bits in TXE_PsmMSDUAccess */
#define PsmMSDUAccess_TIDSel_SHIFT	0
#define PsmMSDUAccess_MSDUIdx_SHIFT	4
#define PsmMSDUAccess_ReadBusy_SHIFT	14
#define PsmMSDUAccess_WriteBusy_SHIFT	15

/* Bits in TXE_PsmMSDUAccess for rev >= 80 */
#define PsmMSDUAccess_BQSelType_SHIFT	5
#define PsmMSDUAccess_MSDUIdx_SHIFT_rev80	7
#define PsmMSDUAccess_BQSelType_Templ	2
#define PsmMSDUAccess_BQSelType_TX	0
#define PsmMSDUAccess_BQSel_SHIFT_rev128	0
#define PsmMSDUAccess_MSDUIdx_SHIFT_rev128	4

#ifdef WLRSDB
#define MAX_RSDB_MAC_NUM 2
#else
#define MAX_RSDB_MAC_NUM 1
#endif // endif
#define MAX_MIMO_MAC_NUM 1

#define MAC_CORE_UNIT_0				0x0 /**< First mac core unit */
#define MAC_CORE_UNIT_1				0x1 /**< Second mac core unit */

/* Supported phymodes / macmodes / opmodes */
#define SINGLE_MAC_MODE				0x0 /**< only single mac is enabled */
#define DUAL_MAC_MODE				0x1 /**< enables dual mac */
#define SUPPORT_EXCLUSIVE_REG_ACCESS_CORE0	0x2
#define SUPPORT_EXCLUSIVE_REG_ACCESS_CORE1	0x4 /**< not functional in 4349A0 */
#define SUPPORT_CHANNEL_BONDING			0x8 /**< enables channel bonding,
						     * supported in single mac mode only
						     */
#define SCAN_CORE_ACTIVE			0x10 /* scan core enabled for background DFS */

#define PHYMODE_MIMO		(SINGLE_MAC_MODE)
#define PHYMODE_80P80		(SINGLE_MAC_MODE | SUPPORT_CHANNEL_BONDING)
#define PHYMODE_RSDB_SISO_0	(DUAL_MAC_MODE | SUPPORT_EXCLUSIVE_REG_ACCESS_CORE0)
#define PHYMODE_RSDB_SISO_1	(DUAL_MAC_MODE | SUPPORT_EXCLUSIVE_REG_ACCESS_CORE1)
#define PHYMODE_RSDB		(PHYMODE_RSDB_SISO_0 | PHYMODE_RSDB_SISO_1)
#define PHYMODE_BGDFS		31
#define PHYMODE_3x3_1x1		31

#define RX_INTR_FIFO_0		0x1		/**< FIFO-0 interrupt */
#define RX_INTR_FIFO_1		0x2		/**< FIFO-1 interrupt */
#define RX_INTR_FIFO_2		0x4		/**< FIFO-2 interrupt */

#define MAX_RX_FIFO		4
#define RXFIFO_CTL_FIFOSEL_MASK		(0x3 << 8)

#define RX_CTL_FIFOSEL_SHIFT	8
#define RX_CTL_FIFOSEL_MASK	(0x3 << RX_CTL_FIFOSEL_SHIFT)

/* For corerev >= 64
 * Additional DMA descriptor flags for AQM Descriptor. These are used in
 * conjunction with the descriptor control flags defined in sbhnddma.h
 */
/* AQM DMA Descriptor control flags 1 */
#define D64_AQM_CTRL1_SOFPTR		0x0000FFFF	/* index of the descr which
							 * is SOF decriptor in DMA table
							 */
#define D64_AQM_CTRL1_EPOCH		0x00010000	/* Epoch bit for the frame */
#define D64_AQM_CTRL1_NUMD_MASK		0x00F00000	/* NumberofDescriptors(NUMD) */
#define D64_AQM_CTRL1_NUMD_SHIFT	20
#define D64_AQM_CTRL1_AC_MASK		0x0F000000	/* AC of the current frame */
#define D64_AQM_CTRL1_AC_SHIFT		24

/* AQM DMA Descriptor control flags 2 */
#define D64_AQM_CTRL2_MPDULEN_MASK	0x00003FFF	/* Length of the entire MPDU */
#define D64_AQM_CTRL2_TXDTYPE		0x00080000	/* When set to 1 the long form of the
							 * TXD is used for the frame.
							 */
/* For corerev >= 128
 * There are some additional flags.
 */
#define D64_AQM_CTRL2_EPOCH		0x00100000	/* Epoch bit for the frame */
#define D64_AQM_CTRL2_EPOCH_EXT		0x00200000	/* Epoch ext bit for the frame */
#define D64_AQM_CTRL2_EPOCH_MASK	0x00300000
#define D64_AQM_CTRL2_FRAMETYPE_1	0x00400000	/* Frame Type, Indicate whether frame is
							 * Data, Management or Control Frame. 2bits
							 * 2'b00=Data, 2'b01=Management,
							 * 2'b10=Control, 2'b11=Invalid value
							 */
#define D64_AQM_CTRL2_FRAMETYPE_2	0x00800000
#define D64_AQM_CTRL2_FRAG_ALLOW	0x01000000	/* Indicate whether the frame allow
							 * to fragment
							 */

/* values for psm_patchcopy_ctrl (0x1AC) post corerev 60 */
#define PSM_PATCHCC_PMODE_MASK		(0x3)
#define PSM_PATCHCC_PMODE_RAM		(0)	/* default */
#define PSM_PATCHCC_PMODE_ROM_RO	(1)
#define PSM_PATCHCC_PMODE_ROM_PATCH	(2)

#define PSM_PATCHCC_PENG_TRIGGER_SHIFT	(2)
#define PSM_PATCHCC_PENG_TRIGGER_MASK	(1 << PSM_PATCHCC_PENG_TRIGGER_SHIFT)
#define PSM_PATCHCC_PENG_TRIGGER	(1 << PSM_PATCHCC_PENG_TRIGGER_SHIFT)

#define PSM_PATCHCC_PCTRL_RST_SHIFT	(3)
#define PSM_PATCHCC_PCTRL_RST_MASK	(0x3 << PSM_PATCHCC_PCTRL_RST_SHIFT)
#define PSM_PATCHCC_PCTRL_RST_RESET	(0x0 << PSM_PATCHCC_PCTRL_RST_SHIFT)
#define PSM_PATCHCC_PCTRL_RST_HW	(0x1 << PSM_PATCHCC_PCTRL_RST_SHIFT)

#define PSM_PATCHCC_COPYEN_SHIFT	(5)
#define PSM_PATCHCC_COPYEN_MASK		(1 << PSM_PATCHCC_COPYEN_SHIFT)
#define PSM_PATCHCC_COPYEN		(1 << PSM_PATCHCC_COPYEN_SHIFT)

#define PSM_PATCHCC_UCIMGSEL_SHIFT	(16)
#define PSM_PATCHCC_UCIMGSEL_MASK	(0x30000)
#define PSM_PATCHCC_UCIMGSEL_DS0	(0x00000)	/* default image */
#define PSM_PATCHCC_UCIMGSEL_DS1	(0x10000)	/* image 1 */

/* patch copy delay for psm: 2millisec */
#define PSM_PATCHCOPY_DELAY		(2000)

/* Addr is byte address used by SW; offset is word offset used by uCode */

/** Per AC TX limit settings */
#define M_AC_TXLMT_ADDR(x, _ac)         (M_AC_TXLMT_BLK(x) + (2 * (_ac)))

/** delay from end of PLCP reception to RxTSFTime */
#define	M_APHY_PLCPRX_DLY	3
#define	M_BPHY_PLCPRX_DLY	4
#define C_BTCX_DBGBLK_SZ        6	/**< Number of 16bit words */
#define C_BTCX_DBGBLK2_SZ	11 /* size of statistics at 2nd SHM segment */

#define D11_DMA_CHANNELS	6

/* WME shared memory */
#define M_EDCF_STATUS_OFF(x)	(0x007 * 2)

/* Beacon-related parameters */
#define M_BCN_LI(x)		M_PS_MORE_DTIM_TBTT(x)	/**< beacon listen interval */

#define	D11_PRE40_M_SECKINDXALGO_BLK(x)	(0x2ea * 2)

#define	D11_MAX_KEY_SIZE	16

#define M_SECKINDXALGO_BLK_SZ(_corerev)   (((D11REV_GE(_corerev, 40)) ? \
	(AMT_SIZE(_corerev)) : (RCMTA_SIZE)) + 4 /* default keys */)

#define	C_CTX_PCTLWD_POS	(0x4 * 2)

#define D11_MAX_TX_FRMS		32		/**< max frames allowed in tx fifo */

/* Current channel number plus upper bits */
#define D11_CURCHANNEL_5G	0x0100;
#define D11_CURCHANNEL_40	0x0200;
#define D11_CURCHANNEL_MAX	0x00FF;

#define INVALIDFID		0xffff

#define M_LCNPHYREGS_PTR(x)	M_SSLPNPHYREGS_PTR(x)
#define M_LCN40PHYREGS_PTR(x)	M_SSLPNPHYREGS_PTR(x)
#define M_SWDIV_BLK_PTR(x)	M_LCN40PHYREGS_PTR(x)

#define	D11_RT_DIRMAP_SIZE	16

/** Rate table entry offsets */
#define	M_RT_PRS_PLCP_POS(x)	10
#define	M_RT_PRS_DUR_POS(x)	16
#define	M_RT_OFDM_PCTL1_POS(x)	18
#define	M_RT_TXPWROFF_POS(x)	20
#define	M_REV40_RT_TXPWROFF_POS(x)	14

#define M_20IN40_IQ(x)			(0x380 * 2)

/** SHM locations where ucode stores the current power index */
#define M_CURR_IDX1(x)		(0x384 *2)
#define M_CURR_IDX2(x)		(0x387 *2)

#define MIMO_MAXSYM_DEF		0x8000 /* 32k */
#define MIMO_MAXSYM_MAX		0xffff /* 64k */

#define WATCHDOG_8TU_DEF_LT42	5
#define WATCHDOG_8TU_MAX_LT42	10
#define WATCHDOG_8TU_DEF	3
#define WATCHDOG_8TU_MAX	4

#define M_PKTENG_RXAVGPWR_ANT(x, w)            (M_MFGTEST_RXAVGPWR_ANT0(x) + (w) * 2)

/* M_MFGTEST_NUM (pkt eng) bit definitions */
#define MFGTEST_TXMODE			0x0001 /* TX frames indefinitely */
#define MFGTEST_RXMODE			0x0002 /* RX frames */
#define MFGTEST_RXMODE_ACK		0x0402 /* RX frames with sending ACKs back */
#define MFGTEST_TXMODE_FRMCNT		0x0101 /* TX frames by frmcnt */
#define MFGTEST_RU_TXMODE		0x0011	/* RU frames TX indefinetly */
#define MFGTEST_RU_TXMODE_FRMCNT	0x0111 /* RU TX frames by frmcnt */

/* UOTA interface bit definitions */
enum {
	C_UOTA_CNTSRT_NBIT = 0,	 /* 0 OTA rx frame count start bit (14 LSB's) */
	C_UOTA_RXFST_NBIT = 14,	 /* 14 indicating first frame */
	C_UOTA_RSSION_NBIT = 15, /* 15 OTA rx ON bit position */
};

#define M_EDCF_QLEN(x)	(M_EDCF_QINFO1_OFFSET(x))
#define M_PWRIND_MAP(x, core)		(M_PWRIND_BLKS(x) + ((core)<<1))

#define M_BTCX_MAX_INDEX		240
#define M_BTCX_BACKUP_SIZE		130
#define BTCX_AMPDU_MAX_DUR		2500

#ifdef WLP2P_UCODE

/** The number of scheduling blocks */
#define M_P2P_BSS_MAX		4

/** WiFi P2P interrupt block positions */
#define M_P2P_I_BLK_SZ		4
#define M_P2P_I_BLK_OFFSET(x)	(M_P2P_INTR_BLK(x) - M_P2P_INTF_BLK(x))
#define M_P2P_I_BLK(x, b)		(M_P2P_I_BLK_OFFSET(x) + (M_P2P_I_BLK_SZ * (b) * 2))
#define M_P2P_I(x, b, i)		(M_P2P_I_BLK(x, b) + ((i) * 2))

#define M_P2P_I_PRE_TBTT	0	/**< pretbtt, wake up just before beacon reception */
#define M_P2P_I_CTW_END		1	/**< CTWindow ends */
#define M_P2P_I_ABS		2	/**< absence period start, trigger for switching channels */
#define M_P2P_I_PRS		3	/**< presence period starts */

/** P2P hps flags */
#define M_P2P_HPS_CTW(b)	(1 << (b))
#define M_P2P_HPS_NOA(b)	(1 << ((b) + M_P2P_BSS_MAX))

/** WiFi P2P address attribute block */
#define M_ADDR_BMP_BLK_SZ	12
#define M_ADDR_BMP_BLK(x, b)	(M_ADDR_BMP_BLK_OFFSET(x) + ((b) * 2))

#define ADDR_BMP_RA		(1 << 0)	/**< Receiver Address (RA) */
#define ADDR_BMP_TA		(1 << 1)	/**< Transmitter Address (TA) */
#define ADDR_BMP_BSSID		(1 << 2)	/**< BSSID */
#define ADDR_BMP_AP		(1 << 3)	/**< Infra-BSS Access Point (AP) */
#define ADDR_BMP_STA		(1 << 4)	/**< Infra-BSS Station (STA) */
#define ADDR_BMP_P2P_DISC	(1 << 5)	/**< P2P Device */
#define ADDR_BMP_P2P_GO		(1 << 6)	/**< P2P Group Owner */
#define ADDR_BMP_P2P_GC		(1 << 7)	/**< P2P Client */
#define ADDR_BMP_BSS_IDX_MASK	(3 << 8)	/**< BSS control block index */
#define ADDR_BMP_BSS_IDX_SHIFT	8

/** WiFi P2P address starts from this entry in RCMTA */
#define P2P_ADDR_STRT_INDX	(RCMTA_SIZE - M_ADDR_BMP_BLK_SZ)

/* WiFi P2P per BSS control block positions.
 * all time related fields are in units of (1<<P2P_UCODE_TIME_SHIFT)us unless noted otherwise.
 */

#define P2P_UCODE_TIME_SHIFT	7
#define M_P2P_BSS_BLK_SZ	12
#define M_P2P_BSS_BLK_OFFSET(x)		(M_P2P_PERBSS_BLK(x) - M_P2P_INTF_BLK(x))
#define M_P2P_BSS_BLK(x, b)	(M_P2P_BSS_BLK_OFFSET(x) + (M_P2P_BSS_BLK_SZ * (b) * 2))
#define M_P2P_BSS(x, b, p)		(M_P2P_BSS_BLK(x, b) + (p) * 2)
#define M_P2P_BSS_BCN_INT(x, b)	(M_P2P_BSS_BLK(x, b) + (0 * 2))	/**< beacon interval */
#define M_P2P_BSS_DTIM_PRD(x, b)	(M_P2P_BSS_BLK(x, b) + (1 * 2))	/**< DTIM period */
#define M_P2P_BSS_ST(x, b)		(M_P2P_BSS_BLK(x, b) + (2 * 2))	/**< current state */
#define M_P2P_BSS_N_PRE_TBTT(x, b)	(M_P2P_BSS_BLK(x, b) + (3 * 2))	/**< next pretbtt time */
#define M_P2P_BSS_CTW(x, b)	(M_P2P_BSS_BLK(x, b) + (4 * 2))	/**< CTWindow duration */
#define M_P2P_BSS_N_CTW_END(x, b)	(M_P2P_BSS_BLK(x, b) + (5 * 2))	/**< next CTWindow end */
#define M_P2P_BSS_NOA_CNT(x, b)	(M_P2P_BSS_BLK(x, b) + (6 * 2))	/**< NoA count */
#define M_P2P_BSS_N_NOA(x, b)	(M_P2P_BSS_BLK(x, b) + (7 * 2))	/**< next absence time */
#define M_P2P_BSS_NOA_DUR(x, b)	(M_P2P_BSS_BLK(x, b) + (8 * 2))	/**< absence period */
#define M_P2P_BSS_NOA_TD(x, b)	(M_P2P_BSS_BLK(x, b) + (9 * 2))	/**< presence period (int - dur) */
#define M_P2P_BSS_NOA_OFS(x, b)	(M_P2P_BSS_BLK(x, b) + (10 * 2)) /* last 7 bits of interval in us */
#define M_P2P_BSS_DTIM_CNT(x, b)	(M_P2P_BSS_BLK(x, b) + (11 * 2))	/**< DTIM count */

/* M_P2P_BSS_ST word positions. */
#define M_P2P_BSS_ST_CTW	(1 << 0)	/**< BSS is in CTWindow */
#define M_P2P_BSS_ST_SUPR	(1 << 1)	/**< BSS is suppressing frames */
#define M_P2P_BSS_ST_ABS	(1 << 2)	/**< BSS is in absence period */
#define M_P2P_BSS_ST_WAKE	(1 << 3)
#define M_P2P_BSS_ST_AP		(1 << 4)	/**< BSS is Infra-BSS AP */
#define M_P2P_BSS_ST_STA	(1 << 5)	/**< BSS is Infra-BSS STA */
#define M_P2P_BSS_ST_GO		(1 << 6)	/**< BSS is P2P Group Owner */
#define M_P2P_BSS_ST_GC		(1 << 7)	/**< BSS is P2P Client */
#define M_P2P_BSS_ST_IBSS	(1 << 8)	/**< BSS is an IBSS */
#define M_P2P_BSS_ST_AWDL	(1 << 9)	/* BSS is AWDL */
#define M_P2P_BSS_ST_NAN	(1 << 10)	/**< BSS is NAN */
#define M_P2P_BSS_ST_MULTIDTIM	(1 << 11)	/* BSS is Muti-DTIM enabled */

/** WiFi P2P TSF block positions */
#define M_P2P_TSF_BLK_SZ	4
#define M_P2P_TSF_BLK_OFFSET(x) (M_P2P_TSF_OFFSET_BLK(x) - M_P2P_INTF_BLK(x))
#define M_P2P_TSF_BLK(x, b)	(M_P2P_TSF_BLK_OFFSET(x) + (M_P2P_TSF_BLK_SZ * (b) * 2))
#define M_P2P_TSF(x, b, w)		(M_P2P_TSF_BLK(x, b) + (w) * 2)

#define M_P2P_TSF_DRIFT_OFFSET(x) (M_P2P_TSF_DRIFT_WD0(x) - M_P2P_INTF_BLK(x))
#define M_P2P_TSF_DRIFT(x, w)	(M_P2P_TSF_DRIFT_OFFSET(x) + (w) * 2)

#define M_P2P_GO_CHANNEL_OFFSET(x)	(M_P2P_GO_CHANNEL(x) - M_P2P_INTF_BLK(x))
#define M_P2P_GO_IND_BMP_OFFSET(x)	(M_P2P_GO_IND_BMP(x) - M_P2P_INTF_BLK(x))

/**
 * M_P2P_GO_IND_BMP now has multiple fields:
 *	7:0	- GO_IND_BMP
 *	10:8	- BSS Index
 *	15:11	- Reserved
*/
#define M_P2P_GO_IND_BMP_MASK		(0xFF)
#define M_P2P_BSS_INDEX_MASK		(0x700)
#define M_P2P_BSS_INDEX_SHIFT_BITS	(8)

/* per BSS PreTBTT */
#define M_P2P_PRE_TBTT_OFFSET(x) (M_P2P_PRETBTT_BLK(x) - M_P2P_INTF_BLK(x))
#define M_P2P_PRE_TBTT(x, b)	(M_P2P_PRE_TBTT_OFFSET(x) + ((b) * 2))	/**< in us */

/* Reserve bottom of RCMTA for P2P Addresses */
#define	WSEC_MAX_RCMTA_KEYS	(54 - M_ADDR_BMP_BLK_SZ)
#else
#define	WSEC_MAX_RCMTA_KEYS	54
#endif	/* WLP2P_UCODE */

#define TXCOREMASK		0x0F
#define SPATIAL_SHIFT		8
#define MAX_COREMASK_BLK	5

#define BPHY_ONE_CORE_TX	(1 << 15)	/**< enable TX ant diversity for 11b frames */

#define M_WLCX_CONFIG_EN(x)	0x1				/**< 1: enable wifi coex */
#define M_WLCX_CONFIG_MASTER(x)	0x2				/**< 1: Coex Master(5357) */

/* ucode debug status codes */
#define	DBGST_INACTIVE		0		/**< not valid really */
#define	DBGST_INIT		1		/**< after zeroing SHM, before suspending at init */
#define	DBGST_ACTIVE		2		/**< "normal" state */
#define	DBGST_SUSPENDED		3		/**< suspended */
#define	DBGST_ASLEEP		4		/**< asleep (PS mode) */

/* Radio ID */
#define M_RADIOID_L_OFFSET(x)     0x43

/**
 * Defines for Self Mac address (used currently for CTS2SELF frames
 * generated by BTCX ucode for protection purposes) in SHM. GE40 only.
 */
#define M_MYMAC_ADDR_L(x)                (M_MYMAC_ADDR(x))
#define M_MYMAC_ADDR_M(x)                (M_MYMAC_ADDR(x) + (1*2))
#define M_MYMAC_ADDR_H(x)                (M_MYMAC_ADDR(x) + (2*2))

/** ucode mac statistic counters in shared memory */
#define MACSTAT_OFFSET_SZ 64

/* MACSTAT offset to SHM address */
#define MACSTAT_ADDR(x, offset) (M_PSM2HOST_STATS(x) + 2 * (offset))

/* Re-uses M_SSID */
#define SHM_MBSS_BCNLEN0(x)		M_SSID(x)

#define SHM_MBSS_CLOSED_NET(x)		(0x80)	/**< indicates closed network */

/** SSID Search Engine entries */
#define SHM_MBSS_SSIDSE_BASE_ADDR(x)	(0)
#define SHM_MBSS_SSIDSE_BLKSZ(x)		(36)
#define SHM_MBSS_SSIDLEN_BLKSZ		(4)
#define SHM_MBSS_SSID_BLKSZ			(32)

/* END New for ucode template based mbss */

/** Definitions for PRQ fifo data as per MultiBSSUcode Twiki page */

#define SHM_MBSS_PRQ_ENTRY_BYTES 10	/**< Size of each PRQ entry */
#define SHM_MBSS_PRQ_ENTRY_COUNT 12	/**< Number of PRQ entries */
#define SHM_MBSS_PRQ_TOT_BYTES   (SHM_MBSS_PRQ_ENTRY_BYTES * SHM_MBSS_PRQ_ENTRY_COUNT)

#define M_WOWL_NOBCN	(0x06c * 2)		/**< loss of bcn value */

#define M_RXCNT(x)	(0xa86 * 2)	/* length of Rxpkt received which caused ULP bailout */

/* for sync up b/w ARM and host */
#define M_REPLCNT_BLK(x)		(0x1e2 * 2)
#define M_SEQNUM_TID(x)		(M_REPLCNT_BLK(x) + 0)
#define M_REPCNT_TID(x)		(M_REPLCNT_BLK(x) + 0x1*2)

#define M_KEK(x)		M_EAPOLMICKEY_BLK(x) + (0x10 * 2) /* < KEK for WEP/TKIP */

#define M_ARPRESP_BYTESZ_OFFSET		0	/**< 2 bytes; ARP resp pkt size */
#define M_NA_BYTESZ_0_OFFSET		2	/**< 2 bytes ; NA pkt size */
#define M_NA_BYTESZ_1_OFFSET		4	/**< 2 bytes ; NA pkt size */
#define M_KEEPALIVE_BYTESZ_0_OFFSET	6	/**< 2 bytes; size of first keepalive */
#define M_KEEPALIVE_BYTESZ_1_OFFSET	8	/**< 2 bytes; size of second keepalive */
#define M_NPAT_ARPIDX_OFFSET		10	/**< 2 bytes; net pattern index of ARP */
#define M_NPAT_NS0IDX_OFFSET		12	/**< 2 bytes; net pattern index of NS 0 */
#define M_NPAT_NS1IDX_OFFSET		14	/**< 2 bytes; net pattern index of NS 1 */
#define M_EXTWAKEPATTERN_0_OFFSET	16	/**< 6 bytes; ext magic pattern */
#define M_EXTWAKEPATTERN_U0_OFFSET	22	/**< 8 bytes; unaligned ext magic pattern */
#define M_KEEPALIVE_INTVL_0_OFFSET	30	/**< 2 bytes; in no of beacon intervals */
#define M_KEEPALIVE_INTVL_1_OFFSET	32	/**< 2 bytes; in no of beacon intervals */

#define M_COREMASK_BLK_WOWL_L30     (0x298 * 2)

/* corerev > 29 && corerev < 40 */
#define M_COREMASK_BLK_WOWL         (0x7e8 *2)

/* corerev >= 42 */
#define D11AC_M_COREMASK_BLK_WOWL       (0x1b0*2)

#ifdef WLAMPDU_UCODE
/* ucode assisted AMPDU aggregation */
/* ucode allocates a big block starting with 4 side channels, followed by 4 descriptor blocks */
#define TOT_TXFS_WSIZE          50	/**< totally 50 entries */
#define C_TXFSD_WOFFSET         TOT_TXFS_WSIZE	/**< offset of M_TXFS_INTF_BLK in M_TXFS_BLK */

#define C_TXFSD_SIZE	          10		/**< Each descriptor is 10 bytes */
#define C_TXFSD_STRT_POS(base, q)  (base + (q * C_TXFSD_SIZE) + 0) /**< start */
#define C_TXFSD_END_POS(base, q)   (base + (q * C_TXFSD_SIZE) + 2) /**< end */
#define C_TXFSD_WPTR_POS(base, q)  (base + (q * C_TXFSD_SIZE) + 4) /**< driver updates */
#define C_TXFSD_RPTR_POS(base, q)  (base + (q * C_TXFSD_SIZE) + 6) /**< ucode updates */
#define C_TXFSD_RNUM_POS(base, q)  (base + (q * C_TXFSD_SIZE) + 8) /**< For ucode debugging */

#define MPDU_LEN_SHIFT		0
#define MPDU_LEN_MASK		(0xfff << MPDU_LEN_SHIFT)	/**< Bits 0 - 11 */
#define MPDU_EPOCH_SHIFT	14
#define MPDU_EPOCH_MASK		(0x1 << MPDU_EPOCH_SHIFT)	/**< Bit 14 */
#define MPDU_DEBUG_SHIFT	15
#define MPDU_DEBUG_MASK		(0x1 << MPDU_DEBUG_SHIFT)	/**< Bit 15 */
#endif /* WLAMPDU_UCODE */

#if defined(WLAMPDU_MAC)
/* WLAMPDU_AQM defines, but WLAMPDU_AQM is only defined in dongle builds,
 * so put under WLAMPDU_MAC
 */

/** # of bins for mpdu density histogram : this ony exists for aqm agg */
#define C_MPDUDEN_NBINS		64
/** # of bins for frameburst length histogram : this ony exists for aqm agg */
#define C_MBURST_NBINS          8

#define C_AGGSTOP_NBINS         8
#define C_AMP_STATS_SIZE        (C_MPDUDEN_NBINS + C_AGGSTOP_NBINS + C_MBURST_NBINS)
#endif /* WLAMPDU_MAC */

#define	M_EXTLNA_PWRSAVE(x)	M_RADIO_PWR(x)	/**< External LNA power control support */
#define M_PHY_ANTDIV_REG_4314(x) (0xa94 * 2)
#define M_PHY_ANTDIV_MASK_4314(x) (0xa95 * 2)

/* D11AC shm location changes */
#define	D11AC_T_NULL_TPL_BASE		(0x16 * 2)
#define D11AC_T_NULL_TPL_SIZE_BYTES	(24)
#define D11_T_BCN0_TPL_BASE	T_BCN0_TPL_BASE
#define D11AC_T_BCN0_TPL_BASE	(0x100 * 2)
#define D11_T_BCN1_TPL_BASE	T_BCN1_TPL_BASE
#define D11AC_T_BCN1_TPL_BASE	(0x240 * 2)

/* The response (ACK/BA) phyctrl words */
#define D11AC_RSP_TXPCTL0      (0x4c * 2)
#define D11AC_RSP_TXPCTL1      (0x4d * 2)

#define D11_T_PRS_TPL_BASE T_PRS_TPL_BASE
#define D11AC_T_PRS_TPL_BASE    (0x380 * 2)

#define	D11_M_RT_PRS_PLCP_POS(x) M_RT_PRS_PLCP_POS(x)
#define	D11_M_RT_PRS_DUR_POS(x) M_RT_PRS_DUR_POS(x)
#define D11AC_M_RT_PRS_PLCP_POS 8
#define D11AC_M_RT_PRS_DUR_POS 12

/* Field definitions for M_REV40_RT_TXPWROFF_POS */
#define M_REV40_RT_HTTXPWR_OFFSET_MASK	0x01f8	/**< bit 8:3 */
#define M_REV40_RT_HTTXPWR_OFFSET_SHIFT	3

#define PHYCTL_TXPWR_CORE1_SHIFT	8
#define M_REV128_RT_HTTXPWR_OFFSET_MASK	0xffff

/* shmem locations for Beamforming */
/* shmem defined with prefix M_ are in shmem */
#define shm_addr(base, offset)  (((base)+(offset))*2)

/* BFI block definitions (Beamforming) */
#define C_BFI_BFRIDX_POS		0
#define	C_BFI_NDPA_TST_POS		1
#define	C_BFI_NDPA_TXCNT_POS		2
#define C_BFI_NDPA_SEQ_POS		3
#define C_BFI_NDPA_FCTST_POS		4
#define C_BFI_BFRCTL_POS		5
#define C_BFI_BFR_CONFIG0_POS		6
#define C_BFI_AVAIL_POS			7
#define C_BFI_BFE_MIMOCTL_POS		8
#define C_BFI_BSSID0_POS		9
#define C_BFI_BSSID1_POS		10
#define C_BFI_BSSID2_POS		11
#define C_BFI_STAINFO_POS		12
#define C_BFI_BFE_MYAID_POS		13
#define C_BFI_BFMSTAT_POS		14
/* used by BFR */
#define C_BFI_STA_ADDR_POS C_BFI_BSSID0_POS

/* Phy cache index Bit<8> indicates the validity. Cleared during TxBf link Init
 * to trigger a new sounding sequence.
 */
#define	C_BFRIDX_MASK         0x001F
#define C_BFRIDX_VLD_NBIT     8   /* valid */
#define C_BFRIDX_EN_NBIT      7   /* BFI block is enabled (has valid info),
				   * applicable only for MU BFI block in shmemx
				   */
#define C_STAINFO_FBT_NBIT   12   /* 0: SU; 1: MU */
#define C_STAINFO_NCIDX_NBIT 13 /* Bits13-15: NC IDX; Reserved if Feedback Type is SU */

#define C_BFI_BFRCTL_POS_NDP_TYPE_SHIFT	0	/* 0 HT NDP; 1 VHT NDP */
#define C_BFI_BFRCTL_POS_NSTS_SHIFT	1
/* 0: 2 stream; 1: 3 streams 2: 4 streams */
#define C_BFI_BFRCTL_NSTS_SHFT(_rev)	((D11REV_GE(_rev, 128)) ? (2) : (1))
#define C_BFI_BFRCTL_POS_MLBF_SHIFT	4	/* 1  enable MLBF(used for corerev < 64) */
#define C_BFI_BFRCTL_POS_BFM_SHIFT	8	/* Bits15-8: BFM mask for BFM frame tx */
#define C_BFI_MUMBSS_NBIT		9	/* MU MBSS feature 0=disabled, 1=enabled */
#define C_BFI_BSSID_NBIT		10	/* bssid for C_BFI_NDPA_FCTST_POS */

/** dynamic rflo ucode WAR defines */
#define UCODE_WAR_EN		1
#define UCODE_WAR_DIS		0

/** LTE coex definitions */
#define LTECX_FLAGS_LPBK_OFF 0

/** LTECX shares BTCX shmem block */
#define M_LTECX_BLK_PTR(x)				M_BTCX_BLK_PTR(x)

/* CORE0 MODE */
#define CORE0_MODE_RSDB		0x0
#define CORE0_MODE_MIMO		0x1
#define CORE0_MODE_80P80	0x2

#define CORE1_MODE_RSDB		0x100

/* TOF Support */
#define	M_TOF_PTR		(69*2)		/**< TOF block pointer */

#define M_FCBS_DS1_MAC_INIT_BLOCK	(0x5 * 2)
#define M_FCBS_DS1_PHY_RADIO_BLOCK	(0x36 * 2)
#define M_FCBS_DS1_RADIO_PD_BLOCK	(0x97 * 2)
#define M_FCBS_DS1_EXIT_BLOCK		(0xb0 * 2)

/* Ucode Crash debug block pointer rev42/47/48/49 */
#define M_UDBG_CRASH_BLK_PTR_AC         (M_PSM_SOFT_REGS_EXT + (0x16 * 2))
/* Ucode Crash debug block pointer rev23/29 */
#define M_UDBG_CRASH_BLK_PTR_LE30       (M_PSM_SOFT_REGS + (0x35 * 2))

#define M_HWACI_ST	(M_PSM_SOFT_REGS_EXT + 0x20) /* HWACI ucode sts */
#define HWACI_HOST_FLAG_ADDR		(0x186)
#define HWACI_SET_SW_MITIGATION_MODE	(0x0008)

/* split RX war shm locations  */
#define RXFIFO_0_OFFSET 0x1A0
#define RXFIFO_1_OFFSET 0x19E
#define HDRCONV_FIFO0_STSLEN    0x4	/* status length in header conversion mode */
#define DEFAULT_FIFO0_STSLEN    0x24	/* status length in default mode */

/* Following are the offsets in M_DRVR_UCODE_IF_PTR block for P2P ucode.
 * Start address of M_DRVR_UCODE_IF_PTR block is present in M_DRVR_UCODE_IF_PTR.
 */
#define M_FCBS_DEBUG_P2P					(0x1b * 2)

/* M_ULP_WAKEIND bits */
#define	C_WATCHDOG_EXPIRY	(1 << 0)
#define	C_FCBS_ERROR		(1 << 1)
#define	C_RETX_FAILURE		(1 << 2)
#define	C_HOST_WAKEUP		(1 << 3)
#define	C_INVALID_FCBS_BLOCK	(1 << 4)
#define	C_HUDI_DS1_EXIT		(1 << 5)
#define	C_LOB_SLEEP		(1 << 6)

/* values for M_ULP_FEATURES */
#define C_P2P_NOA			(0x0001)
#define C_INFINITE_NOA			(0x0002)
#define C_P2P_CTWIN			(0x0004)
#define C_P2P_GC			(0x0008)
#define C_BCN_TRIM			(0x0010)
#define C_BT_COEX			(0x0020)
#define C_LTE_COEX			(0x0040)
#define C_ADS1				(0x0080)
#define C_LTECX_PSPOLL_PRIO_EN		(0x0100)
#define C_ULP_SLOWCAL_SKIP		(0x0200)
#define C_HUDI_ENABLE			(0x0400)

#define M_WOWL_ULP_SW_DAT_BLK	(0xBFF * 2)	/* (0xFFF * 2) - 1024 */
#define M_WOWL_ULP_SW_DAT_BLK_MAX_SZ	(0x400)	/* 1024 bytes */

/* TOF Support */
#define	M_TOF_PTR		(69*2)		/* TOF block pointer */

#define	M_TOF_CMD		(0x0*2)		/* command txed from fw to ucode */
#define	M_TOF_RSP		(0x1*2)		/* response from ucode to fw */
#define	M_TOF_CHNSM_0		(0x2*2)		/* Channel smoothing 0 */
#define	M_TOF_DOT11DUR		(0x3*2)		/* 802.11 reseved dur value */
#define	M_TOF_PHYCTL0		(0x4*2)		/* PHYCTL0 value */
#define	M_TOF_PHYCTL1		(0x5*2)		/* PHYCTL1 value */
#define	M_TOF_PHYCTL2		(0x6*2)		/* PHYCTL2 value */
#define	M_TOF_LSIG		(0x7*2)		/* LSIG value */
#define	M_TOF_VHTA0		(0x8*2)		/* VHTA0 value */
#define	M_TOF_VHTA1		(0x9*2)		/* VHTA1 value */
#define	M_TOF_VHTA2		(0xa*2)		/* VHTA2 value */
#define	M_TOF_VHTB0		(0xb*2)		/* VHTB0 value */
#define	M_TOF_VHTB1		(0xc*2)		/* VHTB1 value */
#define	M_TOF_AMPDU_CTL		(0xd*2)		/* AMPDU_CTL value */
#define	M_TOF_AMPDU_DLIM	(0xe*2)		/* AMPDU_DLIM value */
#define	M_TOF_AMPDU_LEN		(0xf*2)		/* AMPDU length */
#define M_TOF_UCODE_SET         (0x35*2)        /* FLAG to set ucode */
#define M_TOF_TRIG_DLY		(0x33*2)	/* Trigger Delay from ACK for ranging */
#define RX_INTR_FIFO_0		0x1		/* FIFO-0 interrupt */
#define RX_INTR_FIFO_1		0x2		/* FIFO-1 interrupt */
#define RX_INTR_FIFO_2		0x4		/* FIFO-2 interrupt */

/* M_TOF_UCODE_SET bits */
typedef enum {
	TOF_RX_FTM_NBIT = 0,
	TOF_SHARED_ANT	= 1
} eTOFFlags;

/* Following are the offsets in M_DRVR_UCODE_IF_PTR block. Start address of
 * M_DRVR_UCODE_IF_PTR block is present in M_DRVR_UCODE_IF_PTR.
 */
#define M_ULP_FEATURES			(0x0 * 2)

#define M_ACPHY_SWDIV_BLK_PTR		(99 * 2)
#define M_ACPHY_SWDIV_EN			(0 * 2)
#define M_ACPHY_SWDIV_PREF_ANT		(1 * 2)
#define M_ACPHY_SWDIV_TX_PREF_ANT	(2 * 2)
#define M_ACPHY_SWDIV_GPIO_MASK		(3 * 2)
#define M_ACPHY_SWDIV_GPIO_NUM		(4 * 2)

/* M_HOST_FLAGS5 offset changed in ULP ucode */
#define M_ULP_HOST_FLAGS5   (0x3d * 2)

#define M_PMU_FCBS_BLK				(0xdae * 2)
#define M_PMU_FCBS_CMD_PTR			((M_PMU_FCBS_BLK)+(0x0))
#define M_PMU_FCBS_DATA_PTR			((M_PMU_FCBS_BLK)+(0x2))
#define M_PMU_FCBS_CTRL_STATUS		((M_PMU_FCBS_BLK)+(0x4))
#define M_PMU_FCBS_RUN_TIME_CNT		((M_PMU_FCBS_BLK)+(0x6))

#define M_PHY_ANTDIV_REG_4314(x)	(0xa94 * 2)
#define M_PHY_ANTDIV_MASK_4314(x)	(0xa95 * 2)

#define M_RADAR_REG			(0x033 * 2)

#define M_NOISE_CAL_TIMEOUT_OFFSET(x)		19
#define M_NOISE_CAL_CMD_OFFSET(x)			20
#define M_NOISE_CAL_RSP_OFFSET(x)			21
#define M_NOISE_CAL_DATA_OFFSET(x)			23

/* Bit masks for ClkGateUcodeReq2: Ucode MAC Clock Request2 (IHR Address 0x375)  register */
#define D11_FUNC16_MAC_CLOCKREQ_MASK (0x3)

/* Clock gating registers
 * http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/Dot11macRev61
 */
#define CLKREQ_BLOCK	0
#define CLKREQ_MAC_ILP	1
#define CLKREQ_MAC_ALP	2
#define CLKREQ_MAC_HT	3

/* ClkGateSts */
#define CLKGTE_FORCE_MAC_CLK_REQ_SHIFT			0
#define CLKGTE_MAC_PHY_CLK_REQ_SHIFT			4

/* ClkGateReqCtrl0 */
#define CLKGTE_PSM_PATCHCOPY_CLK_REQ_SHIFT		0
#define CLKGTE_RXKEEP_OCP_CLK_REQ_SHIFT			2
#define CLKGTE_PSM_MAC_CLK_REQ_SHIFT			4
#define CLKGTE_TSF_CLK_REQ_SHIFT			6
#define CLKGTE_AQM_CLK_REQ_SHIFT			8
#define CLKGTE_SERIAL_CLK_REQ_SHIFT			10
#define CLKGTE_TX_CLK_REQ_SHIFT				12
#define CLKGTE_POSTTX_CLK_REQ_SHIFT			14

/* ClkGateReqCtrl1 */
#define CLKGTE_RX_CLK_REQ_SHIFT				0
#define CLKGTE_TXKEEP_OCP_CLK_REQ_SHIFT			2
#define CLKGTE_HOST_RW_CLK_REQ_SHIFT			4
#define CLKGTE_IHR_WR_CLK_REQ_SHIFT			6
#define CLKGTE_TKIP_KEY_CLK_REQ_SHIFT			8
#define CLKGTE_TKIP_MISC_CLK_REQ_SHIFT			10
#define CLKGTE_AES_CLK_REQ_SHIFT			12
#define CLKGTE_WAPI_CLK_REQ_SHIFT			14

/* ClkGateReqCtrl2 */
#define CLKGTE_WEP_CLK_REQ_SHIFT			0
#define CLKGTE_PSM_CLK_REQ_SHIFT			2
#define CLKGTE_MACPHY_CLK_REQ_BY_PHY_SHIFT		4
#define CLKGTE_FCBS_CLK_REQ_SHIFT			6
#define CLKGTE_HIN_AXI_MAC_CLK_REQ_SHIFT		8

/* ClkGateStretch0 */
#define CLKGTE_MAC_HT_CLOCK_STRETCH_SHIFT		0
#define CLKGTE_MAC_ALP_CLOCK_STRETCH_SHIFT		8
#define CLKGTE_MAC_HT_CLOCK_STRETCH_VAL			0x4

/* ClkGateStretch1 */
#define CLKGTE_MAC_PHY_CLOCK_STRETCH_SHIFT		13

/* ClkGateMisc */
#define CLKGTE_TPF_CLK_REQTHRESH			0xF
#define CLKGTE_AQM_CLK_REQEXT				0x70

/* ClkGateDivCtrl */
#define CLKGTE_MAC_ILP_OFF_COUNT_MASK			0x0007
#define CLKGTE_MAC_ILP_OFF_COUNT_SHIFT			0
#define CLKGTE_MAC_ILP_ON_COUNT_MASK			0x0020
#define CLKGTE_MAC_ILP_ON_COUNT_MASK_GE_REV80		0x0030
#define CLKGTE_MAC_ALP_OFF_COUNT_MASK			0x03C0
#define CLKGTE_MAC_ALP_OFF_COUNT_SHIFT			6

/* ClkGatePhyClkCtrl */
#define CLKGTE_PHY_MAC_PHY_CLK_REQ_EN_SHIFT		0
#define CLKGTE_O2C_HIN_PHY_CLK_EN_SHIFT			1
#define CLKGTE_HIN_PHY_CLK_EN_SHIFT			2
#define CLKGTE_IHRP_PHY_CLK_EN_SHIFT			3
#define CLKGTE_CCA_MAC_PHY_CLK_REQ_EN_SHIFT		4
#define CLKGTE_TX_MAC_PHY_CLK_REQ_EN_SHIFT		5
#define CLKGTE_HRP_MAC_PHY_CLK_REQ_EN_SHIFT		6
#define CLKGTE_SYNC_MAC_PHY_CLK_REQ_EN_SHIFT		7
#define CLKGTE_RX_FRAME_MAC_PHY_CLK_REQ_EN_SHIFT	8
#define CLKGTE_RX_START_MAC_PHY_CLK_REQ_EN_SHIFT	9
#define CLKGTE_FCBS_MAC_PHY_CLK_REQ_SHIFT		10
#define CLKGTE_POSTRX_MAC_PHY_CLK_REQ_EN_SHIFT		11
#define CLKGTE_DOT11_MAC_PHY_RXVALID_SHIFT		12
#define CLKGTE_NOT_PHY_FIFO_EMPTY_SHIFT			13
#define CLKGTE_DOT11_MAC_PHY_BFE_REPORT_DATA_READY	14
#define CLKGTE_DOT11_MAC_PHY_CLK_BIT15			15

/* ClkGateExtReq0 */
#define CLKGTE_TOE_SYNC_MAC_CLK_REQ_SHIFT		0
#define CLKGTE_TXBF_SYNC_MAC_CLK_REQ_SHIFT		2
#define CLKGTE_HIN_SYNC_MAC_CLK_REQ_SHIFT		4
#define CLKGTE_SLOW_SYNC_CLK_REQ_SHIFT			6
#define CLKGTE_ERCX_SYNC_CLK_REQ_SHIFT			8
#define CLKGTE_BTCX_SYNC_CLK_REQ_SHIFT			10
#define CLKGTE_IFS_CRS_SYNC_CLK_REQ_SHIFT		12
#define CLKGTE_IFS_GCI_SYNC_CLK_REQ_SHIFT		14

/* ClkGateExtReq1 */
#define CLKGTE_PHY_FIFO_SYNC_CLK_REQ_SHIFT		0
#define CLKGTE_RXE_CHAN_SYNC_CLK_REQ_SHIFT		2
#define CLKGTE_PMU_MDIS_SYNC_MAC_CLK_REQ_SHIFT		4
#define CLKGTE_PSM_IPC_SYNC_CLK_REQ_SHIFT		6

/* =========== LHL regs =========== */
/* WL ARM Timer0 Interrupt Status (lhl_wl_armtim0_st_adr) */
#define LHL_WL_ARMTIM0_ST_WL_ARMTIM_INT_ST	0x00000001

#define AUTO_MEM_STBY_RET_SHIFT	4

/* WiFi P2P TX stop timestamp block SHM's */
#define M_P2P_TX_STOP_BLK_SZ	2	/* 2 SHM words */
#define M_P2P_TX_STOP_BLK(b)	((0x67 * 2) + (M_P2P_TX_STOP_BLK_SZ * (b) * 2))
#define M_P2P_TX_STOP_TS(b, w)	(M_P2P_TX_STOP_BLK(b) + (w) * 2)

#define D11TXHDR_RATEINFO_ACCESS_VAL(txh, corerev, member) \
	((((txh)->corerev).RateInfo[3]).member)

#define D11_BSR_SF_2048_SHIFT		11
#define D11_BSR_SF_2048			(1 << D11_BSR_SF_2048_SHIFT)

/* 2 bits for HE signature and 4 bits for control ID */
#define D11_BSR_HE_SIG_SHIFT		6
/* HE Variant with BSR control ID */
#define D11_BSR_HE_SIG			(0xf)
#define D11_BSR_ACI_BMAP_SHIFT		(0 + D11_BSR_HE_SIG_SHIFT)
#define D11_BSR_DELTA_TID_SHIFT		(4 + D11_BSR_HE_SIG_SHIFT)
#define D11_BSR_SF_SHIFT			(8 + D11_BSR_HE_SIG_SHIFT)
#define D11_BSR_QUEUE_SIZE_ALL_SHIFT	(18 + D11_BSR_HE_SIG_SHIFT)

#define D11_BSR_DELTA_TID_ALLTID_SIGNATURE	3
#define D11_BSR_QUEUE_SIZE_ALL_WIDTH	8
#define D11_BSR_QUEUE_SIZE_ALL_MAX		((1 << D11_BSR_QUEUE_SIZE_ALL_WIDTH) - 1)
#define D11_BSR_QUEUE_SIZE_ALL_MASK		(D11_BSR_QUEUE_SIZE_ALL_MAX <<\
		D11_BSR_QUEUE_SIZE_ALL_SHIFT)

#define D11_BSR_WD1_SHIFT			16

enum {
	D11_BSR_SF_ID_16 = 0,	/* 0 */
	D11_BSR_SF_ID_128 = 1,	/* 1 */
	D11_BSR_SF_ID_2048 = 2,	/* 2 */
	D11_BSR_SF_ID_16384 = 3	/* 3 */
};

enum {
	D11_PING_BLOCK_VALID = 0,		/* 0 */
	D11_PONG_BLOCK_VALID = 1,		/* 1 */
	D11_UC_READING_PING_BLOCK = 2,	/* 2 */
	D11_UC_READING_PONG_BLOCK = 3	/* 3 */
};

enum {
	D11_BSR_TID0_POS = 0,	/* 0  */
	D11_BSR_TID1_POS = 1,	/* 1 */
	D11_BSR_TID2_POS = 2,	/* 2 */
	D11_BSR_TID3_POS = 3,	/* 3 */
	D11_BSR_TID4_POS = 4,	/* 4 */
	D11_BSR_TID5_POS = 5,	/* 5 */
	D11_BSR_TID6_POS = 6,	/* 6 */
	D11_BSR_TID7_POS = 7,	/* 7 */
	D11_BSR_WD0_POS = 8,	/* 8 */
	D11_BSR_WD1_POS = 9,	/* 9 */
};

#define D11_IS_PING_PONG_IN_RESET(i)	(((i) & ((1 << D11_PING_BLOCK_VALID) |\
	(1 << D11_UC_READING_PING_BLOCK) | (1 << D11_PONG_BLOCK_VALID) |\
	(1 << D11_UC_READING_PONG_BLOCK))) == 0)
#define D11_PING_BLOCK_VALID_MASK		((1 << D11_PONG_BLOCK_VALID) |\
		(1 << D11_UC_READING_PING_BLOCK))
#define D11_PONG_BLOCK_VALID_MASK		((1 << D11_PING_BLOCK_VALID) |\
		(1 << D11_UC_READING_PONG_BLOCK))
#define D11_IS_PING_BLOCK_WRITABLE(i)	(((i) & D11_PING_BLOCK_VALID_MASK) == \
		(1 << D11_PONG_BLOCK_VALID))
#define D11_IS_PONG_BLOCK_WRITABLE(i)	(((i) & D11_PONG_BLOCK_VALID_MASK) == \
		(1 << D11_PING_BLOCK_VALID))
#define D11_SET_PING_BLOCK_VALID(i)		(((i) & ~(1 << D11_PONG_BLOCK_VALID)) |\
		(1 << D11_PING_BLOCK_VALID))
#define D11_SET_PONG_BLOCK_VALID(i)		(((i) & ~(1 << D11_PING_BLOCK_VALID)) |\
		(1 << D11_PONG_BLOCK_VALID))

#define PLCP_VALID(wlc, rxh, plcp) \
	(D11REV_GE((wlc)->pub->corerev, 128) ? \
		((D11RXHDR_ACCESS_VAL((rxh), (wlc)->pub->corerev, dma_flags) & RXS_DMAFLAGS_MASK) \
		== RXS_MAC_UCODE_PHY) : (((plcp)[0] | (plcp)[1] | (plcp)[2]) != 0))

#define T_WOWL_BASE		(0x4c0 * 2) /* (256+320+320+320) = 0x4c0 */

#ifdef WL_EAP_NOISE_MEASUREMENTS
/* ucode  knoise gain override shmem */
#define RxGAIN_HI_5G 0x5d7
#define RxGAIN_LO_5G 0x457
#define RxGAIN_HI_2G 0x1d5
#define RxGAIN_LO_2G 0x55

/* Noise Cal Functional Bitmap  - from ucode PsmSoftRegDefinitions.h */
#define C_NCALCFG_FGAIN_NBIT	0x1 // Force Gain
#define C_NCALCFG_KNOISE_NBIT	0x2 // Use Knoise as opposed to "regular" noise measurement
#define C_NCALCFG_IMM_MEAS	0x4 // Used by LTE-Coex; Immediate measurement
#define C_NCALCFG_IMM_MEAS2	0x8 // Used by LTE-Coex sub case; Immediate measurement
#endif /* WL_EAP_NOISE_MEASUREMENTS */

/* Define D11 WAR FLAGS HERE, 32bits */
#define D11_WAR_BFD_TXVMEM_RESET		0x00000001
#define D11_CHKWAR_BFD_TXVMEM_RESET(wlc_hw)	\
	(wlc_bmac_check_d11war((wlc_hw), D11_WAR_BFD_TXVMEM_RESET))

#define D11_COREVMAJOR_MASK	0x0fff
#define D11_COREVMINOR_MASK	0xf000
#define D11_COREVMINOR_SHIFT	12
#define D11_MACHW_VER(major, minor)	\
	((major & D11_COREVMAJOR_MASK) | ((minor << D11_COREVMINOR_SHIFT) & D11_COREVMINOR_MASK))

#endif	/* _D11_H */
