/*
 * NPHY RADIO specific portion of Broadcom BCM43XX 802.11abgn
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
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: wlc_phy_radio_n.c 676800 2016-12-24 19:51:32Z $
 */

#ifndef _wlc_phy_radio_n_
#define _wlc_phy_radio_n_

#endif // endif

#include <wlc_cfg.h>
#include <typedefs.h>
#include <qmath.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_phy_radio.h>
#include <bitfuncs.h>
#include <bcmdevs.h>
#include <bcmnvram.h>
#include <802.11.h>
#include <sbchipc.h>
#include <hndpmu.h>
#include <bcmsrom_fmt.h>
#include <sbsprom.h>
#include <bcmotp.h>

#include <wlc_phy_hal.h>
#include <wlc_phy_int.h>
#include <phy_utils_reg.h>

#include <phy_chanmgr_api.h>
#include <wlc_phyreg_n.h>
#include <wlc_phytbl_n.h>
#include <wlc_phy_n.h>
#include <phy_radio.h>

/* 2057 rev4 (43226A0) register initialization tables */
radio_20xx_regs_t regs_2057_rev4[] = {
	{ 0x00,          0x84,    0  },
	{ 0x01,             0,    0  },
	{ 0x02,          0x60,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    1  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0xf7,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x4,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    1  },
	{ 0x32,             0,    1  },
	{ 0x33,             0,    1  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x26,    1  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    1  },
	{ 0x3D,          0xff,    1  },
	{ 0x3E,          0xff,    1  },
	{ 0x3F,          0xff,    1  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x75,    0  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0xa8,    0  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x30,    0  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,          0x19,    0  },
	{ 0x64,          0x62,    0  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0xc8,    0  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x1e,    0  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x1e,    0  },
	{ 0x7C,          0x62,    0  },
	{ 0x7D,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,          0x9c,    0  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    1  },
	{ 0x8B,          0x10,    1  },
	{ 0x8C,          0xf0,    1  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0xe1,    0  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    1  },
	{ 0xA5,          0x6d,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    1  },
	{ 0xA9,           0xc,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    1  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,             0,    0  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x75,    0  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0xa8,    0  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x30,    0  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,          0x19,    0  },
	{ 0xE9,          0x62,    0  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0xc8,    0  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x1e,    0  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x1e,    0  },
	{ 0x101,          0x62,    0  },
	{ 0x102,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,          0x9c,    0  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    1  },
	{ 0x110,          0x10,    1  },
	{ 0x111,          0xf0,    1  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0xe1,    0  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    1  },
	{ 0x12A,          0x6d,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    1  },
	{ 0x12E,           0xc,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    1  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,             0,    0  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x152,             0,    0  },
	{ 0x153,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    1  },
	{ 0x15F,             0,    1  },
	{ 0x160,             0,    1  },
	{ 0x161,             0,    1  },
	{ 0x162,             0,    1  },
	{ 0x163,             0,    1  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,           0x2,    1  },
	{ 0x16A,             0,    1  },
	{ 0x16B,             0,    1  },
	{ 0x16C,             0,    1  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,          0x21,    0  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,          0x21,    0  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    1  },
	{ 0x1A5,             0,    1  },
	{ 0x1A6,             0,    1  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    1  },
	{ 0x1AB,             0,    1  },
	{ 0x1AC,             0,    1  },
	{ 0xFFFF,            0,    0  },
	};

/* 2057 rev5 (5357a0) register initialization tables */
radio_20xx_regs_t regs_2057_rev5[] = {
	{ 0x00,             0,    1  },
	{ 0x01,          0x57,    1  },
	{ 0x02,          0x20,    1  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    1  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    1  },
	{ 0x32,             0,    1  },
	{ 0x33,             0,    1  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    1  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    1  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,           0xf,    1  },
	{ 0x64,           0xf,    1  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    1  },
	{ 0x82,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,             0,    0  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    1  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    1  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    1  },
	{ 0xE9,           0xf,    1  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    1  },
	{ 0x107,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,             0,    0  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    1  },
	{ 0x15F,             0,    1  },
	{ 0x160,             0,    1  },
	{ 0x161,             0,    1  },
	{ 0x162,             0,    1  },
	{ 0x163,             0,    1  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    1  },
	{ 0x16B,             0,    1  },
	{ 0x16C,             0,    1  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17B,          0x21,    0  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19B,          0x21,    0  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    1  },
	{ 0x1A5,             0,    1  },
	{ 0x1A6,             0,    1  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    1  },
	{ 0x1AB,             0,    1  },
	{ 0x1AC,             0,    1  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0xc,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,           0x1,    1  },
	{ 0x1C2,          0x80,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,             0,    0  }
};

/* 2057 rev5v1 (5357b0) register initialization tables */
radio_20xx_regs_t regs_2057_rev5v1[] = {
	{ 0x00,          0x15,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x3,    1  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    0  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    0  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    0  },
	{ 0x63,           0xf,    0  },
	{ 0x64,           0xf,    0  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    0  },
	{ 0x82,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    0  },
	{ 0x92,          0x36,    0  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,          0x10,    1  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    0  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    0  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    0  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    0  },
	{ 0xE9,           0xf,    0  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    0  },
	{ 0x107,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    0  },
	{ 0x117,          0x36,    0  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,          0x10,    1  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    0  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17B,           0x1,    1  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19B,           0x1,    1  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0xc,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,           0x1,    1  },
	{ 0x1C2,          0x80,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* 2057 rev7 (43236a0) register initialization tables */
radio_20xx_regs_t regs_2057_rev7[] = {
	{ 0x00,             0,    1  },
	{ 0x01,          0x57,    1  },
	{ 0x02,          0x20,    1  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    1  },
	{ 0x32,             0,    1  },
	{ 0x33,             0,    1  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x29,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x88,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    1  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    1  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,           0xf,    1  },
	{ 0x64,          0x13,    1  },
	{ 0x65,             0,    0  },
	{ 0x66,          0xee,    1  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    1  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x13,    1  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x13,    1  },
	{ 0x7C,          0x14,    1  },
	{ 0x7D,          0xee,    1  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    1  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,             0,    0  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    1  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    1  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    1  },
	{ 0xE9,          0x13,    1  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0xee,    1  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    1  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x13,    1  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x13,    1  },
	{ 0x101,          0x14,    1  },
	{ 0x102,          0xee,    1  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    1  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,             0,    0  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    1  },
	{ 0x15F,             0,    1  },
	{ 0x160,             0,    1  },
	{ 0x161,             0,    1  },
	{ 0x162,             0,    1  },
	{ 0x163,             0,    1  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    1  },
	{ 0x16B,             0,    1  },
	{ 0x16C,             0,    1  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,           0x1,    1  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,           0x1,    1  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    1  },
	{ 0x1A5,             0,    1  },
	{ 0x1A6,             0,    1  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    1  },
	{ 0x1AB,             0,    1  },
	{ 0x1AC,             0,    1  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,          0xa0,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* 2057 rev7v1 (43236b0) register initialization tables */
radio_20xx_regs_t regs_2057_rev7v1[] = {
	{ 0x00,          0x17,    1  },
	{ 0x01,          0x57,    1  },
	{ 0x02,          0x20,    1  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    1  },
	{ 0x32,             0,    1  },
	{ 0x33,             0,    1  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x1,    1  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    1  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    1  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,           0xf,    1  },
	{ 0x64,          0x13,    1  },
	{ 0x65,             0,    0  },
	{ 0x66,          0xee,    1  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    1  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x13,    1  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x13,    1  },
	{ 0x7C,          0x14,    1  },
	{ 0x7D,          0xee,    1  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    1  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,          0x10,    1  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    1  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    1  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    1  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    1  },
	{ 0xE9,          0x13,    1  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0xee,    1  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    1  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x13,    1  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x13,    1  },
	{ 0x101,          0x14,    1  },
	{ 0x102,          0xee,    1  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    1  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,          0x10,    1  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    1  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    1  },
	{ 0x15F,             0,    1  },
	{ 0x160,             0,    1  },
	{ 0x161,             0,    1  },
	{ 0x162,             0,    1  },
	{ 0x163,             0,    1  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    1  },
	{ 0x16B,             0,    1  },
	{ 0x16C,             0,    1  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,           0x1,    1  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,           0x1,    1  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    1  },
	{ 0x1A5,             0,    1  },
	{ 0x1A6,             0,    1  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    1  },
	{ 0x1AB,             0,    1  },
	{ 0x1AC,             0,    1  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,          0xa0,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  },
	};

/* 2057 rev7v2 (43236b1) register initialization tables */

radio_20xx_regs_t regs_2057_rev7v2[] = {
	{ 0x45,           0x1,    1  },
	{ 0x51,          0x70,    1  },
	{ 0x59,          0x88,    1  },
	{ 0x5C,          0x20,    1  },
	{ 0x62,          0x33,    1  },
	{ 0x63,          0x12,    1  },
	{ 0x64,          0x12,    1  },
	{ 0x66,          0xff,    1  },
	{ 0x6E,          0x58,    1  },
	{ 0x75,          0x13,    1  },
	{ 0x7B,          0x13,    1  },
	{ 0x7C,          0x14,    1  },
	{ 0x7D,          0xee,    1  },
	{ 0x81,           0x1,    1  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0xA1,          0x20,    1  },
	{ 0xC4,          0x10,    1  },
	{ 0xC9,           0x1,    1  },
	{ 0xD6,          0x70,    1  },
	{ 0xDE,          0x88,    1  },
	{ 0xE1,          0x20,    1  },
	{ 0xE8,          0x12,    1  },
	{ 0xE9,          0x12,    1  },
	{ 0xEB,          0xff,    1  },
	{ 0xF3,          0x58,    1  },
	{ 0xFA,          0x13,    1  },
	{ 0x100,          0x13,    1  },
	{ 0x101,          0x14,    1  },
	{ 0x102,          0xee,    1  },
	{ 0x106,           0x1,    1  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x126,          0x20,    1  },
	{ 0x149,          0x10,    1  },
	{ 0x14E,           0x1,    1  },
	{ 0x17B,           0x2,    1  },
	{ 0x19B,           0x2,    1  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1C2,          0xa0,    1  },
	{ 0xFFFF,            0,    0  }
};

/* 2057 rev8 (6362b0) register initialization tables */
radio_20xx_regs_t regs_2057_rev8[] = {
	{ 0x00,           0x8,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x29,    1  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x88,    1  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    1  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    1  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,           0xf,    1  },
	{ 0x64,          0x13,    1  },
	{ 0x65,             0,    0  },
	{ 0x66,          0xee,    1  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    1  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x1a,    1  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x1a,    1  },
	{ 0x7C,          0x14,    1  },
	{ 0x7D,          0xee,    1  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    1  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    1  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    1  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    1  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    1  },
	{ 0xE9,          0x13,    1  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0xee,    1  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    1  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x1a,    1  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x1a,    1  },
	{ 0x101,          0x14,    1  },
	{ 0x102,          0xee,    1  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    1  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    1  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,           0x2,    1  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,           0x2,    1  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,          0xa0,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* 2057 rev12 (BCM63268B0) register initialization tables */
radio_20xx_regs_t regs_2057_rev12[] = {
	{ 0x00,           0xc,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    1  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    1  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    1  },
	{ 0x63,           0xf,    1  },
	{ 0x64,          0x13,    1  },
	{ 0x65,             0,    0  },
	{ 0x66,          0xee,    1  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    1  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x1a,    1  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x1a,    1  },
	{ 0x7C,          0x14,    1  },
	{ 0x7D,          0xee,    1  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    1  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    1  },
	{ 0x92,          0x36,    1  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    1  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    1  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    1  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    1  },
	{ 0xE9,          0x13,    1  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0xee,    1  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    1  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x1a,    1  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x1a,    1  },
	{ 0x101,          0x14,    1  },
	{ 0x102,          0xee,    1  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    1  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    1  },
	{ 0x117,          0x36,    1  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    1  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,           0x2,    1  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,           0x2,    1  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,          0xa0,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* LCNXN: 2057 rev13 (53572a0) register initialization tables */
radio_20xx_regs_t regs_2057_rev13[] = {
	{ 0x00,           0xd,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x3,    1  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    1  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    0  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    0  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    0  },
	{ 0x63,           0xf,    0  },
	{ 0x64,           0xf,    0  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    0  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x13,    0  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x13,    0  },
	{ 0x7C,           0xf,    0  },
	{ 0x7D,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    0  },
	{ 0x82,           0xa,    0  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    0  },
	{ 0x92,          0x36,    0  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    1  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,          0x10,    1  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    0  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    1  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    0  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    0  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    0  },
	{ 0xE9,           0xf,    0  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    0  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x13,    0  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x13,    0  },
	{ 0x101,           0xf,    0  },
	{ 0x102,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    0  },
	{ 0x107,           0xa,    0  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    0  },
	{ 0x117,          0x36,    0  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    1  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,          0x10,    1  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    0  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,          0x21,    0  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,          0x21,    0  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x47,    0  },
	{ 0x1B0,          0x47,    0  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0xc,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,           0x1,    1  },
	{ 0x1C2,          0x80,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* LCNXN: 2057 rev14 (43217 itr) register initialization tables */
radio_20xx_regs_t regs_2057_rev14[] = {
	{ 0x00,           0xe,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0xfc,    1  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x24,    1  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x1c,    1  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    0  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    0  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    0  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    0  },
	{ 0x63,           0xf,    0  },
	{ 0x64,           0xf,    0  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    0  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x13,    0  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x13,    0  },
	{ 0x7C,           0xf,    0  },
	{ 0x7D,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    0  },
	{ 0x82,           0x8,    1  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    0  },
	{ 0x92,          0x36,    0  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    0  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    1  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,             0,    0  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,           0x1,    1  },
	{ 0xC9,           0x1,    1  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    0  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    0  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    0  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    0  },
	{ 0xE9,           0xf,    0  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    0  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x13,    0  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x13,    0  },
	{ 0x101,           0xf,    0  },
	{ 0x102,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    0  },
	{ 0x107,           0x8,    1  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    0  },
	{ 0x117,          0x36,    0  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    0  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,             0,    0  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,           0x1,    1  },
	{ 0x14E,           0x1,    1  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,          0x21,    0  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,          0x21,    0  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x40,    1  },
	{ 0x1B0,          0x40,    1  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,             0,    0  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,             0,    0  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0x1CB,             0,    0  },
	{ 0x1CC,           0x1,    1  },
	{ 0x1CD,             0,    0  },
	{ 0x1CE,             0,    0  },
	{ 0x1CF,          0x10,    1  },
	{ 0x1D0,           0xf,    1  },
	{ 0x1D1,             0,    0  },
	{ 0x1D2,             0,    0  },
	{ 0x1D3,          0x10,    1  },
	{ 0x1D4,           0xf,    1  },
	{ 0xFFFF,            0,    0  }
};

/* LCNXN: 2057 rev14 (43217 etr) register initialization tables */
radio_20xx_regs_t regs_2057_rev14v1[] = {
	{ 0x00,          0x1e,    0  },
	{ 0x01,          0x57,    0  },
	{ 0x02,          0x20,    0  },
	{ 0x03,          0x1f,    0  },
	{ 0x04,           0x4,    0  },
	{ 0x05,           0x2,    0  },
	{ 0x06,           0x1,    0  },
	{ 0x07,           0x1,    0  },
	{ 0x08,           0x1,    0  },
	{ 0x09,          0x69,    0  },
	{ 0x0A,          0x66,    0  },
	{ 0x0B,           0x6,    0  },
	{ 0x0C,          0x18,    0  },
	{ 0x0D,           0x3,    0  },
	{ 0x0E,          0x20,    0  },
	{ 0x0F,          0x20,    0  },
	{ 0x10,             0,    0  },
	{ 0x11,          0x7c,    0  },
	{ 0x12,          0x42,    0  },
	{ 0x13,          0xbd,    0  },
	{ 0x14,           0x7,    0  },
	{ 0x15,          0x87,    0  },
	{ 0x16,           0x8,    0  },
	{ 0x17,          0x17,    0  },
	{ 0x18,           0x7,    0  },
	{ 0x19,             0,    0  },
	{ 0x1A,           0x2,    0  },
	{ 0x1B,          0x13,    0  },
	{ 0x1C,          0x3e,    0  },
	{ 0x1D,          0x3e,    0  },
	{ 0x1E,          0x96,    0  },
	{ 0x1F,           0x4,    0  },
	{ 0x20,             0,    0  },
	{ 0x21,             0,    0  },
	{ 0x22,          0x17,    0  },
	{ 0x23,           0x6,    0  },
	{ 0x24,           0x1,    0  },
	{ 0x25,           0x6,    0  },
	{ 0x26,           0x4,    0  },
	{ 0x27,           0xd,    0  },
	{ 0x28,           0xd,    0  },
	{ 0x29,          0x30,    0  },
	{ 0x2A,          0x32,    0  },
	{ 0x2B,           0x8,    0  },
	{ 0x2C,          0x1c,    0  },
	{ 0x2D,           0x2,    0  },
	{ 0x2E,           0x4,    0  },
	{ 0x2F,          0x7f,    0  },
	{ 0x30,          0x27,    0  },
	{ 0x31,             0,    0  },
	{ 0x32,             0,    0  },
	{ 0x33,             0,    0  },
	{ 0x34,             0,    0  },
	{ 0x35,          0x20,    0  },
	{ 0x36,          0x18,    0  },
	{ 0x37,           0x7,    0  },
	{ 0x38,          0x66,    0  },
	{ 0x39,          0x66,    0  },
	{ 0x3A,          0x66,    0  },
	{ 0x3B,          0x66,    0  },
	{ 0x3C,          0xff,    0  },
	{ 0x3D,          0xff,    0  },
	{ 0x3E,          0xff,    0  },
	{ 0x3F,          0xff,    0  },
	{ 0x40,          0x16,    0  },
	{ 0x41,           0x7,    0  },
	{ 0x42,          0x19,    0  },
	{ 0x43,           0x7,    0  },
	{ 0x44,           0x6,    0  },
	{ 0x45,           0x3,    0  },
	{ 0x46,           0x1,    0  },
	{ 0x47,           0x7,    0  },
	{ 0x48,          0x33,    0  },
	{ 0x49,           0x5,    0  },
	{ 0x4A,          0x77,    0  },
	{ 0x4B,          0x66,    0  },
	{ 0x4C,          0x66,    0  },
	{ 0x4D,             0,    0  },
	{ 0x4E,           0x4,    0  },
	{ 0x4F,           0xc,    0  },
	{ 0x50,             0,    0  },
	{ 0x51,          0x70,    0  },
	{ 0x56,           0x7,    0  },
	{ 0x57,             0,    0  },
	{ 0x58,             0,    0  },
	{ 0x59,          0x88,    0  },
	{ 0x5A,             0,    0  },
	{ 0x5B,          0x1f,    0  },
	{ 0x5C,          0x20,    0  },
	{ 0x5D,           0x1,    0  },
	{ 0x5E,          0x30,    0  },
	{ 0x5F,          0x70,    0  },
	{ 0x60,             0,    0  },
	{ 0x61,             0,    0  },
	{ 0x62,          0x33,    0  },
	{ 0x63,           0xf,    0  },
	{ 0x64,           0xf,    0  },
	{ 0x65,             0,    0  },
	{ 0x66,          0x11,    0  },
	{ 0x69,             0,    0  },
	{ 0x6A,          0x7e,    0  },
	{ 0x6B,          0x3f,    0  },
	{ 0x6C,          0x7f,    0  },
	{ 0x6D,          0x78,    0  },
	{ 0x6E,          0x58,    0  },
	{ 0x6F,          0x88,    0  },
	{ 0x70,           0x8,    0  },
	{ 0x71,           0xf,    0  },
	{ 0x72,          0xbc,    0  },
	{ 0x73,           0x8,    0  },
	{ 0x74,          0x60,    0  },
	{ 0x75,          0x13,    0  },
	{ 0x76,          0x70,    0  },
	{ 0x77,             0,    0  },
	{ 0x78,             0,    0  },
	{ 0x79,             0,    0  },
	{ 0x7A,          0x33,    0  },
	{ 0x7B,          0x13,    0  },
	{ 0x7C,           0xf,    0  },
	{ 0x7D,          0x11,    0  },
	{ 0x80,          0x3c,    0  },
	{ 0x81,           0x1,    0  },
	{ 0x82,           0x8,    1  },
	{ 0x83,          0x9d,    0  },
	{ 0x84,           0xa,    0  },
	{ 0x85,             0,    0  },
	{ 0x86,          0x40,    0  },
	{ 0x87,          0x40,    0  },
	{ 0x88,          0x88,    0  },
	{ 0x89,          0x10,    0  },
	{ 0x8A,          0xf0,    0  },
	{ 0x8B,          0x10,    0  },
	{ 0x8C,          0xf0,    0  },
	{ 0x8D,             0,    0  },
	{ 0x8E,             0,    0  },
	{ 0x8F,          0x10,    0  },
	{ 0x90,          0x55,    0  },
	{ 0x91,          0x3f,    0  },
	{ 0x92,          0x36,    0  },
	{ 0x93,             0,    0  },
	{ 0x94,             0,    0  },
	{ 0x95,             0,    0  },
	{ 0x96,          0x87,    0  },
	{ 0x97,          0x11,    0  },
	{ 0x98,             0,    0  },
	{ 0x99,          0x33,    0  },
	{ 0x9A,          0x88,    0  },
	{ 0x9B,             0,    0  },
	{ 0x9C,          0x87,    0  },
	{ 0x9D,          0x11,    0  },
	{ 0x9E,             0,    0  },
	{ 0x9F,          0x33,    0  },
	{ 0xA0,          0x88,    0  },
	{ 0xA1,          0x20,    0  },
	{ 0xA2,          0x3f,    0  },
	{ 0xA3,          0x44,    0  },
	{ 0xA4,          0x8c,    0  },
	{ 0xA5,          0x6c,    0  },
	{ 0xA6,          0x22,    0  },
	{ 0xA7,          0xbe,    0  },
	{ 0xA8,          0x55,    0  },
	{ 0xAA,           0xc,    0  },
	{ 0xAB,          0xaa,    0  },
	{ 0xAC,           0x2,    0  },
	{ 0xAD,             0,    0  },
	{ 0xAE,          0x10,    0  },
	{ 0xAF,           0x1,    0  },
	{ 0xB0,             0,    0  },
	{ 0xB1,             0,    0  },
	{ 0xB2,          0x80,    0  },
	{ 0xB3,          0x60,    0  },
	{ 0xB4,          0x44,    0  },
	{ 0xB5,          0x55,    0  },
	{ 0xB6,           0x1,    0  },
	{ 0xB7,          0x55,    0  },
	{ 0xB8,           0x1,    0  },
	{ 0xB9,           0x5,    0  },
	{ 0xBA,          0x55,    0  },
	{ 0xBB,          0x55,    0  },
	{ 0xC1,             0,    0  },
	{ 0xC2,             0,    0  },
	{ 0xC3,             0,    0  },
	{ 0xC4,          0x10,    1  },
	{ 0xC5,             0,    0  },
	{ 0xC6,             0,    0  },
	{ 0xC7,             0,    0  },
	{ 0xC8,             0,    0  },
	{ 0xC9,           0x1,    1  },
	{ 0xCA,             0,    0  },
	{ 0xCB,             0,    0  },
	{ 0xCC,             0,    0  },
	{ 0xCD,             0,    0  },
	{ 0xCE,          0x5e,    0  },
	{ 0xCF,           0xc,    0  },
	{ 0xD0,           0xc,    0  },
	{ 0xD1,           0xc,    0  },
	{ 0xD2,             0,    0  },
	{ 0xD3,          0x2b,    0  },
	{ 0xD4,           0xc,    0  },
	{ 0xD5,             0,    0  },
	{ 0xD6,          0x70,    0  },
	{ 0xDB,           0x7,    0  },
	{ 0xDC,             0,    0  },
	{ 0xDD,             0,    0  },
	{ 0xDE,          0x88,    0  },
	{ 0xDF,             0,    0  },
	{ 0xE0,          0x1f,    0  },
	{ 0xE1,          0x20,    0  },
	{ 0xE2,           0x1,    0  },
	{ 0xE3,          0x30,    0  },
	{ 0xE4,          0x70,    0  },
	{ 0xE5,             0,    0  },
	{ 0xE6,             0,    0  },
	{ 0xE7,          0x33,    0  },
	{ 0xE8,           0xf,    0  },
	{ 0xE9,           0xf,    0  },
	{ 0xEA,             0,    0  },
	{ 0xEB,          0x11,    0  },
	{ 0xEE,             0,    0  },
	{ 0xEF,          0x7e,    0  },
	{ 0xF0,          0x3f,    0  },
	{ 0xF1,          0x7f,    0  },
	{ 0xF2,          0x78,    0  },
	{ 0xF3,          0x58,    0  },
	{ 0xF4,          0x88,    0  },
	{ 0xF5,           0x8,    0  },
	{ 0xF6,           0xf,    0  },
	{ 0xF7,          0xbc,    0  },
	{ 0xF8,           0x8,    0  },
	{ 0xF9,          0x60,    0  },
	{ 0xFA,          0x13,    0  },
	{ 0xFB,          0x70,    0  },
	{ 0xFC,             0,    0  },
	{ 0xFD,             0,    0  },
	{ 0xFE,             0,    0  },
	{ 0xFF,          0x33,    0  },
	{ 0x100,          0x13,    0  },
	{ 0x101,           0xf,    0  },
	{ 0x102,          0x11,    0  },
	{ 0x105,          0x3c,    0  },
	{ 0x106,           0x1,    0  },
	{ 0x107,           0x8,    1  },
	{ 0x108,          0x9d,    0  },
	{ 0x109,           0xa,    0  },
	{ 0x10A,             0,    0  },
	{ 0x10B,          0x40,    0  },
	{ 0x10C,          0x40,    0  },
	{ 0x10D,          0x88,    0  },
	{ 0x10E,          0x10,    0  },
	{ 0x10F,          0xf0,    0  },
	{ 0x110,          0x10,    0  },
	{ 0x111,          0xf0,    0  },
	{ 0x112,             0,    0  },
	{ 0x113,             0,    0  },
	{ 0x114,          0x10,    0  },
	{ 0x115,          0x55,    0  },
	{ 0x116,          0x3f,    0  },
	{ 0x117,          0x36,    0  },
	{ 0x118,             0,    0  },
	{ 0x119,             0,    0  },
	{ 0x11A,             0,    0  },
	{ 0x11B,          0x87,    0  },
	{ 0x11C,          0x11,    0  },
	{ 0x11D,             0,    0  },
	{ 0x11E,          0x33,    0  },
	{ 0x11F,          0x88,    0  },
	{ 0x120,             0,    0  },
	{ 0x121,          0x87,    0  },
	{ 0x122,          0x11,    0  },
	{ 0x123,             0,    0  },
	{ 0x124,          0x33,    0  },
	{ 0x125,          0x88,    0  },
	{ 0x126,          0x20,    0  },
	{ 0x127,          0x3f,    0  },
	{ 0x128,          0x44,    0  },
	{ 0x129,          0x8c,    0  },
	{ 0x12A,          0x6c,    0  },
	{ 0x12B,          0x22,    0  },
	{ 0x12C,          0xbe,    0  },
	{ 0x12D,          0x55,    0  },
	{ 0x12F,           0xc,    0  },
	{ 0x130,          0xaa,    0  },
	{ 0x131,           0x2,    0  },
	{ 0x132,             0,    0  },
	{ 0x133,          0x10,    0  },
	{ 0x134,           0x1,    0  },
	{ 0x135,             0,    0  },
	{ 0x136,             0,    0  },
	{ 0x137,          0x80,    0  },
	{ 0x138,          0x60,    0  },
	{ 0x139,          0x44,    0  },
	{ 0x13A,          0x55,    0  },
	{ 0x13B,           0x1,    0  },
	{ 0x13C,          0x55,    0  },
	{ 0x13D,           0x1,    0  },
	{ 0x13E,           0x5,    0  },
	{ 0x13F,          0x55,    0  },
	{ 0x140,          0x55,    0  },
	{ 0x146,             0,    0  },
	{ 0x147,             0,    0  },
	{ 0x148,             0,    0  },
	{ 0x149,          0x10,    1  },
	{ 0x14A,             0,    0  },
	{ 0x14B,             0,    0  },
	{ 0x14C,             0,    0  },
	{ 0x14D,             0,    0  },
	{ 0x14E,           0x1,    1  },
	{ 0x14F,             0,    0  },
	{ 0x150,             0,    0  },
	{ 0x151,             0,    0  },
	{ 0x154,           0xc,    0  },
	{ 0x155,           0xc,    0  },
	{ 0x156,           0xc,    0  },
	{ 0x157,             0,    0  },
	{ 0x158,          0x2b,    0  },
	{ 0x159,          0x84,    0  },
	{ 0x15A,          0x15,    0  },
	{ 0x15B,           0xf,    0  },
	{ 0x15C,             0,    0  },
	{ 0x15D,             0,    0  },
	{ 0x15E,             0,    0  },
	{ 0x15F,             0,    0  },
	{ 0x160,             0,    0  },
	{ 0x161,             0,    0  },
	{ 0x162,             0,    0  },
	{ 0x163,             0,    0  },
	{ 0x164,             0,    0  },
	{ 0x165,             0,    0  },
	{ 0x166,             0,    0  },
	{ 0x167,             0,    0  },
	{ 0x168,             0,    0  },
	{ 0x169,             0,    0  },
	{ 0x16A,             0,    0  },
	{ 0x16B,             0,    0  },
	{ 0x16C,             0,    0  },
	{ 0x16D,             0,    0  },
	{ 0x170,             0,    0  },
	{ 0x171,          0x77,    0  },
	{ 0x172,          0x77,    0  },
	{ 0x173,          0x77,    0  },
	{ 0x174,          0x77,    0  },
	{ 0x175,             0,    0  },
	{ 0x176,           0x3,    0  },
	{ 0x177,          0x37,    0  },
	{ 0x178,           0x3,    0  },
	{ 0x179,             0,    0  },
	{ 0x17A,          0x21,    0  },
	{ 0x17B,          0x21,    0  },
	{ 0x17C,             0,    0  },
	{ 0x17D,          0xaa,    0  },
	{ 0x17E,             0,    0  },
	{ 0x17F,          0xaa,    0  },
	{ 0x180,             0,    0  },
	{ 0x190,             0,    0  },
	{ 0x191,          0x77,    0  },
	{ 0x192,          0x77,    0  },
	{ 0x193,          0x77,    0  },
	{ 0x194,          0x77,    0  },
	{ 0x195,             0,    0  },
	{ 0x196,           0x3,    0  },
	{ 0x197,          0x37,    0  },
	{ 0x198,           0x3,    0  },
	{ 0x199,             0,    0  },
	{ 0x19A,          0x21,    0  },
	{ 0x19B,          0x21,    0  },
	{ 0x19C,             0,    0  },
	{ 0x19D,          0xaa,    0  },
	{ 0x19E,             0,    0  },
	{ 0x19F,          0xaa,    0  },
	{ 0x1A0,             0,    0  },
	{ 0x1A1,           0x2,    0  },
	{ 0x1A2,           0xf,    0  },
	{ 0x1A3,           0xf,    0  },
	{ 0x1A4,             0,    0  },
	{ 0x1A5,             0,    0  },
	{ 0x1A6,             0,    0  },
	{ 0x1A7,           0x2,    0  },
	{ 0x1A8,           0xf,    0  },
	{ 0x1A9,           0xf,    0  },
	{ 0x1AA,             0,    0  },
	{ 0x1AB,             0,    0  },
	{ 0x1AC,             0,    0  },
	{ 0x1AD,          0x84,    0  },
	{ 0x1AE,          0x60,    0  },
	{ 0x1AF,          0x40,    1  },
	{ 0x1B0,          0x40,    1  },
	{ 0x1B1,             0,    0  },
	{ 0x1B2,             0,    0  },
	{ 0x1B3,             0,    0  },
	{ 0x1B4,             0,    0  },
	{ 0x1B5,             0,    0  },
	{ 0x1B6,             0,    0  },
	{ 0x1B7,           0x5,    1  },
	{ 0x1B8,             0,    0  },
	{ 0x1B9,             0,    0  },
	{ 0x1BA,             0,    0  },
	{ 0x1BB,             0,    0  },
	{ 0x1BC,             0,    0  },
	{ 0x1BD,             0,    0  },
	{ 0x1BE,             0,    0  },
	{ 0x1BF,             0,    0  },
	{ 0x1C0,             0,    0  },
	{ 0x1C1,             0,    0  },
	{ 0x1C2,          0xa0,    1  },
	{ 0x1C3,             0,    0  },
	{ 0x1C4,             0,    0  },
	{ 0x1C5,             0,    0  },
	{ 0x1C6,             0,    0  },
	{ 0x1C7,             0,    0  },
	{ 0x1C8,             0,    0  },
	{ 0x1C9,             0,    0  },
	{ 0x1CA,             0,    0  },
	{ 0x1CB,             0,    0  },
	{ 0x1CC,          0x6c,    1  },
	{ 0x1CD,             0,    0  },
	{ 0x1CE,             0,    0  },
	{ 0x1CF,             0,    0  },
	{ 0x1D0,             0,    0  },
	{ 0x1D1,             0,    0  },
	{ 0x1D2,             0,    0  },
	{ 0x1D3,             0,    0  },
	{ 0x1D4,             0,    0  },
	{ 0xFFFF,            0,    0  }
};

/* BEGINNING: these are dup from wlc_phy_n.c, need to move to some common place */
#define NPHY_IQLOCC_READ(val) ((uint8)(-(int8)(((val) & 0xf0) >> 4) + (int8)((val) & 0x0f)))

#define NPHY_INTF_RSSI_ENAB(pi)	(CHIPID_43236X_FAMILY(pi))

#define MOD_PHYREG3(pi, phy_type, reg, field, value)	\
	phy_utils_mod_phyreg(pi, phy_type##_##reg, \
	phy_type##_##reg##_##field##_##MASK, (value) << phy_type##_##reg##_##field##_##SHIFT);

#define READ_PHYREG3(pi, phy_type, reg, field)	\
	((phy_utils_read_phyreg(pi, phy_type##_##reg) \
	& phy_type##_##reg##_##field##_##MASK) >> phy_type##_##reg##_##field##_##SHIFT)

#define WRITE_PHY_REG3(pi, phy_type, reg, value)	\
	phy_utils_write_phyreg(pi, phy_type##_##reg, value);

#define	READ_RADIO_REG2(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1))
#define	WRITE_RADIO_REG2(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##jspace##_##reg_name | \
	((core == PHY_CORE_0) ? radio_type##_##jspace##0 : radio_type##_##jspace##1), value);
#define	WRITE_RADIO_SYN(pi, radio_type, reg_name, value) \
	phy_utils_write_radioreg(pi, radio_type##_##SYN##_##reg_name, value);

#define	READ_RADIO_REG3(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name));
#define	WRITE_RADIO_REG3(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##jspace##0##_##reg_name : \
	radio_type##_##jspace##1##_##reg_name), value);
#define	READ_RADIO_REG4(pi, radio_type, jspace, core, reg_name) \
	phy_utils_read_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1));
#define	WRITE_RADIO_REG4(pi, radio_type, jspace, core, reg_name, value) \
	phy_utils_write_radioreg(pi, ((core == PHY_CORE_0) ? \
	radio_type##_##reg_name##_##jspace##0 : \
	radio_type##_##reg_name##_##jspace##1), value);

#define NPHY_NUM_DIG_FILT_COEFFS 15

extern uint16 NPHY_IPA_REV4_txdigi_filtcoeffs[][NPHY_NUM_DIG_FILT_COEFFS];

/* END: these are dup from wlc_phy_n.c, need to move to some common place */

/* ### functions */
static uint16 wlc_phy_radio205x_rcal(phy_info_t *pi);
static uint16 wlc_phy_radio2057_rccal(phy_info_t *pi);

static void wlc_phy_radio_preinit_205x(phy_info_t *pi);
static void wlc_phy_radio_init_2057(phy_info_t *pi);
static void wlc_phy_radio_postinit_2057(phy_info_t *pi);

void
wlc_phy_switch_radio_nphy(phy_info_t *pi, bool on)
{
	if (on) {
		if (!pi->radio_is_on) {
			wlc_phy_radio_preinit_205x(pi);
			wlc_phy_radio_init_2057(pi);
			wlc_phy_radio_postinit_2057(pi);
		}
		/* !!! it could change bw inside */
		wlc_phy_chanspec_set((wlc_phy_t*)pi, pi->radio_chanspec);

		pi->radio_is_on = TRUE;

	} else {

		if (NREV_GE(pi->pubpi->phy_rev, 8)) {
			phy_utils_and_phyreg(pi, NPHY_RfctrlCmd, ~RFCC_CHIP0_PU);
			pi->radio_is_on = FALSE;
		}

	}
}

static void
wlc_phy_radio_preinit_205x(phy_info_t *pi)
{
	/* enable chip_pu and toggle 2056 and 2057 POR via mimophy RFCtrl: */
	ASSERT(NREV_GE(pi->pubpi->phy_rev, 3));

	phy_utils_and_phyreg(pi, NPHY_RfctrlCmd, ~RFCC_CHIP0_PU);

	PHY_REG_LIST_START
		PHY_REG_AND_ENTRY(NPHY, RfctrlCmd, RFCC_OE_POR_FORCE)
		PHY_REG_OR_ENTRY(NPHY, RfctrlCmd, ~RFCC_OE_POR_FORCE)
	PHY_REG_LIST_EXECUTE(pi);

	phy_utils_or_phyreg(pi, NPHY_RfctrlCmd, RFCC_CHIP0_PU);
}

static void
wlc_phy_radio_init_2057(phy_info_t *pi)
{
	radio_20xx_regs_t *regs_2057_ptr = NULL;

	if (NREV_IS(pi->pubpi->phy_rev, 7)) {
		/* Covers 43226a0/1 and 6362a0, until we need to separate them */
		regs_2057_ptr = regs_2057_rev4;

	} else if (NREV_IS(pi->pubpi->phy_rev, 8)) {
		/* 43236a0, 6362b0 */

		if (CHIPID_43236X_FAMILY(pi)) {
			/* 43236a0 */
			regs_2057_ptr = regs_2057_rev7;

		} else if (CHIPID(pi->sh->chip) == BCM6362_CHIP_ID) {
			if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268B0 */
				regs_2057_ptr = regs_2057_rev12;
			} else {
				/* 6362b0 */
				regs_2057_ptr = regs_2057_rev8;
			}
		} else {
			PHY_ERROR(("wlc_phy_radio_init_2057: NPHY 8 need radiorev 5, 7, 8: %x\n",
			           RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
		}

	} else if (NREV_IS(pi->pubpi->phy_rev, 9)) {
		/* 43236b0 */
		if (CHIPID_43236X_FAMILY(pi)) {
			if (CHIPREV(pi->sh->chiprev) == 2) {
				/* 43236b0 */
				regs_2057_ptr = regs_2057_rev7v1;
			} else if (CHIPREV(pi->sh->chiprev) == 3) {
				/* 43236b1 */
				regs_2057_ptr = regs_2057_rev7v2;
			}

		} else {
			PHY_ERROR(("wlc_phy_radio_init_2057: NPHY 9 need radiorev 5v1, 7v1 %x\n",
			           RADIOREV(pi->pubpi->radiorev)));
			ASSERT(0);
		}

	} else if (NREV_IS(pi->pubpi->phy_rev, LCNXN_BASEREV+1)) {
		if (RADIOREV(pi->pubpi->radiorev) == 14) {
			/* 43217 */
			if (RADIOVER(pi->pubpi->radiover) == 1) {
				/* etr sw */
				regs_2057_ptr = regs_2057_rev14v1;
			} else {
				/* itr sw and others */
				regs_2057_ptr = regs_2057_rev14;
			}
		} else {
			regs_2057_ptr = regs_2057_rev13;
		}
	} else {
		PHY_ERROR(("%s: only supports NPHY revs 7-10 and 16-18\n", __FUNCTION__));
		ASSERT(0);
	}

	/* Initialize the radio registers */
	if (phy_radio_init_radio_regs_allbands(pi, regs_2057_ptr) > 0x200)
		PHY_ERROR(("wlc_phy_radio_init_2057, wrong table regs_2057 !!\n"));
}

static void
wlc_phy_radio_postinit_2057(phy_info_t *pi)
{
	/* enable PHY pin-control */
	phy_utils_mod_radioreg(pi, RADIO_2057_XTALPUOVR_PINCTRL, 0x1, 0x1);

	if ((CHIPID(pi->sh->chip) != BCM6362_CHIP_ID) &&
	    ((BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) & BFL2_XTALBUFOUTEN) == 0)) {
		/* Enable xtal_pu_ovr to ensure BT xtal_buff out is off for 43226,
		 * but 6362 needs it on since it controls diff clock buff pu (not BT)
		 */
		phy_utils_mod_radioreg(pi, RADIO_2057_XTALPUOVR_PINCTRL, 0x2, 0x2);
	}

	/* reset synthesizer */
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_CAL_RESETN, 0x78,  0x78);
	phy_utils_mod_radioreg(pi, RADIO_2057_XTAL_CONFIG2,          0x80,  0x80);
	OSL_DELAY(2000);
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_CAL_RESETN, 0x78,  0x0);
	phy_utils_mod_radioreg(pi, RADIO_2057_XTAL_CONFIG2,          0x80,  0x0);

	/* R-cal and RC-cal are required after radio reset */
	if (pi->phy_init_por) {
		wlc_phy_radio205x_rcal(pi);
		wlc_phy_radio2057_rccal(pi);
	}

	/* Disable xo_jtag (6362A0 needs it ON at all times) and the spare_xtal_buffer */
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MASTER, 0x8, 0x0);
}

#if defined(BCMDBG) || defined(WLTEST)

static uint16
nphy_iqlocc_write(phy_info_t *pi, uint8 data)
{
	int32 data32 = (int8)data;
	int32 rf_data32;

	int32 ip, in;
	ip = 8 + (data32 >> 1);
	in = 8 - ((data32+1) >> 1);
	rf_data32 = (in << 4) | ip;

	return (uint16)(rf_data32);
}

void
wlc_nphy_get_radio_loft(phy_info_t *pi, uint8 *ei0, uint8 *eq0, uint8 *fi0,
	uint8 *fq0, uint8 *ei1, uint8 *eq1, uint8 *fi1, uint8 *fq1)
{
	*ei0 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX0_LOFT_FINE_I));
	*eq0 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX0_LOFT_FINE_Q));
	*fi0 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX0_LOFT_COARSE_I));
	*fq0 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX0_LOFT_COARSE_Q));
	*ei1 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX1_LOFT_FINE_I));
	*eq1 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX1_LOFT_FINE_Q));
	*fi1 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX1_LOFT_COARSE_I));
	*fq1 = NPHY_IQLOCC_READ(phy_utils_read_radioreg(pi, RADIO_2057_TX1_LOFT_COARSE_Q));
}

void
wlc_nphy_set_radio_loft(phy_info_t *pi, uint8 ei0, uint8 eq0, uint8 fi0,
	uint8 fq0, uint8 ei1, uint8 eq1, uint8 fi1, uint8 fq1)
{
	phy_utils_write_radioreg(pi, RADIO_2057_TX0_LOFT_FINE_I, nphy_iqlocc_write(pi, ei0));
	phy_utils_write_radioreg(pi, RADIO_2057_TX0_LOFT_FINE_Q, nphy_iqlocc_write(pi, eq0));
	phy_utils_write_radioreg(pi, RADIO_2057_TX0_LOFT_COARSE_I, nphy_iqlocc_write(pi, fi0));
	phy_utils_write_radioreg(pi, RADIO_2057_TX0_LOFT_COARSE_Q, nphy_iqlocc_write(pi, fq0));
	phy_utils_write_radioreg(pi, RADIO_2057_TX1_LOFT_FINE_I, nphy_iqlocc_write(pi, ei1));
	phy_utils_write_radioreg(pi, RADIO_2057_TX1_LOFT_FINE_Q, nphy_iqlocc_write(pi, eq1));
	phy_utils_write_radioreg(pi, RADIO_2057_TX1_LOFT_COARSE_I, nphy_iqlocc_write(pi, fi1));
	phy_utils_write_radioreg(pi, RADIO_2057_TX1_LOFT_COARSE_Q, nphy_iqlocc_write(pi, fq1));
}
#endif /* defined(BCMDBG) || defined(WLTEST) */

void
wlc_phy_radio205x_vcocal_nphy(phy_info_t *pi)
{
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_EN,         0x01, 0x0);
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_CAL_RESETN, 0x04, 0x0);
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_CAL_RESETN, 0x04, (1 << 2));
	phy_utils_mod_radioreg(pi, RADIO_2057_RFPLL_MISC_EN,         0x01, 0x01);

	/* Wait for open loop cal completion and settling */
	OSL_DELAY(300);
}

void
wlc_phy_radio205x_check_vco_cal_nphy(phy_info_t *pi)
{
	/* Monitor vcocal refresh bit and relock PLL if out of range */
	if (phy_utils_read_radioreg(pi, RADIO_2057_VCOCAL_STATUS) & 0x2) {

		bool suspend = !(R_REG(pi->sh->osh, D11_MACCONTROL(pi)) & MCTL_EN_MAC);

		PHY_INFORM(("wl%d: %s: vctrl out of range, trigger VCO cal\n",
		            pi->sh->unit, __FUNCTION__));

		if (!suspend)
			wlapi_suspend_mac_and_wait(pi->sh->physhim);

		wlc_phy_radio205x_vcocal_nphy(pi);

		if (!suspend)
			wlapi_enable_mac(pi->sh->physhim);
	}
}

#define MAX_205x_RCAL_WAITLOOPS 10000

static uint16
wlc_phy_radio205x_rcal(phy_info_t *pi)
{
	uint16 rcal_reg = 0;
	uint16 iqest_sel_pu_save = 0;
	uint16 iqest_sel_pu2_save = 0;
	int i;

	uint16 RfctrlRSSIOTHERS1_save, RfctrlRSSIOTHERS2_save;
	uint16 RfctrlAuxReg1_save, RfctrlAuxReg2_save;
	uint16 RfctrlRXGAIN1_save, RfctrlRXGAIN2_save;
	uint16 RfctrlTXGAIN1_save, RfctrlTXGAIN2_save;
	uint16 RfctrlMiscReg3_save, RfctrlMiscReg4_save;
	uint16 RfctrlMiscReg5_save, RfctrlMiscReg6_save;
	uint16 RfctrlOverride0_save, RfctrlOverride1_save;
	uint16 RfctrlOverride3_save, RfctrlOverride4_save;
	uint16 RfctrlOverride5_save, RfctrlOverride6_save;

	/* Save Register */
	RfctrlRSSIOTHERS1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS1);
	RfctrlRSSIOTHERS2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRSSIOTHERS2);
	RfctrlAuxReg1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlAuxReg1);
	RfctrlAuxReg2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlAuxReg2);
	RfctrlRXGAIN1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRXGAIN1);
	RfctrlRXGAIN2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlRXGAIN2);
	RfctrlTXGAIN1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlTXGAIN1);
	RfctrlTXGAIN2_save = phy_utils_read_phyreg(pi, NPHY_RfctrlTXGAIN2);
	RfctrlMiscReg3_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg3);
	RfctrlMiscReg4_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg4);
	RfctrlMiscReg5_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg5);
	RfctrlMiscReg6_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlMiscReg6);
	RfctrlOverride0_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride0);
	RfctrlOverride1_save = phy_utils_read_phyreg(pi, NPHY_RfctrlOverride1);
	RfctrlOverride3_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride3);
	RfctrlOverride4_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride4);
	RfctrlOverride5_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride5);
	RfctrlOverride6_save = phy_utils_read_phyreg(pi, NPHY_REV7_RfctrlOverride6);

	/* Power Down non-RCAL related Blocks to minimize IR drop on the RCAL supplies
	 *
	 * Technically - should shut down ONLY blocks on RCAL supplies.
	 * Since power-supply grouping differ chip to chip
	 * powering down non-RCAL related Blocks is a strong safeguard
	 */

	PHY_REG_LIST_START
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlRSSIOTHERS1, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlRSSIOTHERS2, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlAuxReg1, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlAuxReg2, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlRXGAIN1, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlRXGAIN2, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlTXGAIN1, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlTXGAIN2, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlMiscReg3, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlMiscReg4, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlMiscReg5, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlMiscReg6, 0x0)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverride0, 0x7ff)
		PHY_REG_WRITE_ENTRY(NPHY, RfctrlOverride1, 0x7ff)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride3, 0x7ff)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride4, 0x7ff)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride5, 0x7f)
		PHY_REG_WRITE_ENTRY(NPHY_REV7, RfctrlOverride6, 0x7f)
	PHY_REG_LIST_EXECUTE(pi);

	if ((RADIOREV(pi->pubpi->radiorev) == 5) || (RADIOREV(pi->pubpi->radiorev) == 13)) {

		/* only needed for 2057 rev5 */

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_REV7_RfctrlMiscReg3,
				NPHY_REV7_RfctrlMiscReg_lpf_pu_MASK,
				0x1 << NPHY_REV7_RfctrlMiscReg_lpf_pu_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_REV7_RfctrlOverride3,
				NPHY_REV7_RfctrlOverride_lpf_pu_MASK,
				0x1 << NPHY_REV7_RfctrlOverride_lpf_pu_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(10);

		/* enable extra switch for 5357 R-Cal */
		phy_utils_mod_radioreg(pi, RADIO_2057_IQTEST_SEL_PU,    0x1, 0x1);
		phy_utils_mod_radioreg(pi, RADIO_2057v7_IQTEST_SEL_PU2, 0x2, 0x2);
	}
	else if (RADIOREV(pi->pubpi->radiorev) == 14) {
		iqest_sel_pu_save = phy_utils_read_radioreg(pi, RADIO_2057_IQTEST_SEL_PU);
		iqest_sel_pu2_save =
		        phy_utils_read_radioreg(pi, RADIO_2057v7_IQTEST_SEL_PU2);

		PHY_REG_LIST_START
			PHY_REG_MOD_RAW_ENTRY(NPHY_REV7_RfctrlMiscReg3,
				NPHY_REV7_RfctrlMiscReg_lpf_pu_MASK,
				0x1 << NPHY_REV7_RfctrlMiscReg_lpf_pu_SHIFT)
			PHY_REG_MOD_RAW_ENTRY(NPHY_REV7_RfctrlOverride3,
				NPHY_REV7_RfctrlOverride_lpf_pu_MASK,
				0x1 << NPHY_REV7_RfctrlOverride_lpf_pu_SHIFT)
		PHY_REG_LIST_EXECUTE(pi);

		OSL_DELAY(10);

		phy_utils_write_radioreg(pi, RADIO_2057v7_IQTEST_SEL_PU2, 0x2);
		phy_utils_write_radioreg(pi, RADIO_2057_IQTEST_SEL_PU, 0x1);

	}

	/* Power up RCAL */
	phy_utils_mod_radioreg(pi, RADIO_2057_RCAL_CONFIG, 0x1, 0x1);
	OSL_DELAY(10);

	/* Trigger RCAL and wait */
	phy_utils_mod_radioreg(pi, RADIO_2057_RCAL_CONFIG, 0x2, 0x2);
	OSL_DELAY(100);

	/* Clear the trigger */
	phy_utils_mod_radioreg(pi, RADIO_2057_RCAL_CONFIG, 0x2, 0x0);

	for (i = 0; i < MAX_205x_RCAL_WAITLOOPS; i++) {
		rcal_reg = phy_utils_read_radioreg(pi, RADIO_2057_RCAL_STATUS);
		if (rcal_reg & 0x1) {
			break;
		}
		OSL_DELAY(100);
	}

	ASSERT(i < MAX_205x_RCAL_WAITLOOPS);

	/* Read the rcal result */
	rcal_reg = phy_utils_read_radioreg(pi, RADIO_2057_RCAL_STATUS) & 0x3e;

	/* Power down the RCAL */
	phy_utils_mod_radioreg(pi, RADIO_2057_RCAL_CONFIG, 0x1, 0x0);

	/* Restore Registers */
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride0, RfctrlOverride0_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlOverride1, RfctrlOverride1_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride3, RfctrlOverride3_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride4, RfctrlOverride4_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride5, RfctrlOverride5_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlOverride6, RfctrlOverride6_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS1, RfctrlRSSIOTHERS1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRSSIOTHERS2, RfctrlRSSIOTHERS2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlAuxReg1, RfctrlAuxReg1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlAuxReg2, RfctrlAuxReg2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRXGAIN1, RfctrlRXGAIN1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlRXGAIN2, RfctrlRXGAIN2_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlTXGAIN1, RfctrlTXGAIN1_save);
	phy_utils_write_phyreg(pi, NPHY_RfctrlTXGAIN2, RfctrlTXGAIN2_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg3, RfctrlMiscReg3_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg4, RfctrlMiscReg4_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg5, RfctrlMiscReg5_save);
	phy_utils_write_phyreg(pi, NPHY_REV7_RfctrlMiscReg6, RfctrlMiscReg6_save);

	if ((RADIOREV(pi->pubpi->radiorev) == 5) ||
	           (RADIOREV(pi->pubpi->radiorev) == 13)) {
		/* Disable extra switching needed for R-Cal */
		phy_utils_mod_radioreg(pi, RADIO_2057_IQTEST_SEL_PU,    0x1, 0x0);
		phy_utils_mod_radioreg(pi, RADIO_2057v7_IQTEST_SEL_PU2, 0x2, 0x0);

	} else if ((RADIOREV(pi->pubpi->radiorev) <= 4) ||
		(RADIOREV(pi->pubpi->radiorev) == 6)) {
		/* 2057 bandgap and tempsense writes; drop LSB and keep only top 4 bits */
		phy_utils_mod_radioreg(pi, RADIO_2057_TEMPSENSE_CONFIG,  0x3c, rcal_reg);
		phy_utils_mod_radioreg(pi, RADIO_2057_BANDGAP_RCAL_TRIM,
		                       0xf0, rcal_reg << 2);

	} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
		phy_utils_write_radioreg(pi, RADIO_2057_IQTEST_SEL_PU, iqest_sel_pu_save);
		phy_utils_write_radioreg(pi, RADIO_2057v7_IQTEST_SEL_PU2,
		                         iqest_sel_pu2_save);
	}

	return (rcal_reg & 0x3e);
}

void
wlc_phy_chanspec_radio2057_setup(phy_info_t *pi, const chan_info_nphy_radio2057_t *ci,
                                 const chan_info_nphy_radio2057_rev5_t *ci2)
{
	int coreNum;
	uint16 txmix2g_tune_boost_pu = 0;
	uint16 pad2g_tune_pus = 0;
	uint16 pad2g_tune_pus_core1 = 0;
	uint16 ipa5g_bf = 0;
	uint16 freq;
	uint16 rccal_bcap_val = 0;
	uint16 rccal_scap_val = 0;
	uint16 rccal_tx40_11n_bcap[] = {0, 0};
	uint16 rccal_tx40_11n_scap[] = {0, 0};
	uint16 tx_lpf_bw_ofdm_40mhz[] = {4, 4};
	uint16 rx2tx_lpf_rc_lut_tx40_11n = 0;
	int txdigi_filt_coeffs_type = 1;
	int j = 0;

	if ((RADIOREV(pi->pubpi->radiorev) == 5) ||
	    (RADIOREV(pi->pubpi->radiorev) == 13) ||
	    (RADIOREV(pi->pubpi->radiorev) == 14)) {
		/* 2057 rev5 is 2.4G only */
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOCAL_COUNTVAL0, ci2->RF_vcocal_countval0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOCAL_COUNTVAL1, ci2->RF_vcocal_countval1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_REFMASTER_SPAREXTALSIZE,
		                ci2->RF_rfpll_refmaster_sparextalsize);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_R1, ci2->RF_rfpll_loopfilter_r1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_C2, ci2->RF_rfpll_loopfilter_c2);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_C1, ci2->RF_rfpll_loopfilter_c1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_CP_KPD_IDAC, ci2->RF_cp_kpd_idac);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_MMD0, ci2->RF_rfpll_mmd0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_MMD1, ci2->RF_rfpll_mmd1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOBUF_TUNE, ci2->RF_vcobuf_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_MX2G_TUNE, ci2->RF_logen_mx2g_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_INDBUF2G_TUNE, ci2->RF_logen_indbuf2g_tune);

		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX2G_TUNE_BOOST_PU_CORE0,
		                ci2->RF_txmix2g_tune_boost_pu_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD2G_TUNE_PUS_CORE0, ci2->RF_pad2g_tune_pus_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA2G_TUNE_CORE0, ci2->RF_lna2g_tune_core0);

		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX2G_TUNE_BOOST_PU_CORE1,
		                ci2->RF_txmix2g_tune_boost_pu_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD2G_TUNE_PUS_CORE1, ci2->RF_pad2g_tune_pus_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA2G_TUNE_CORE1, ci2->RF_lna2g_tune_core1);

	} else {
		/* other 2057 revs are dual-band */
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOCAL_COUNTVAL0, ci->RF_vcocal_countval0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOCAL_COUNTVAL1, ci->RF_vcocal_countval1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_REFMASTER_SPAREXTALSIZE,
		                ci->RF_rfpll_refmaster_sparextalsize);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_R1, ci->RF_rfpll_loopfilter_r1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_C2, ci->RF_rfpll_loopfilter_c2);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_LOOPFILTER_C1, ci->RF_rfpll_loopfilter_c1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_CP_KPD_IDAC, ci->RF_cp_kpd_idac);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_MMD0, ci->RF_rfpll_mmd0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_RFPLL_MMD1, ci->RF_rfpll_mmd1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_VCOBUF_TUNE, ci->RF_vcobuf_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_MX2G_TUNE, ci->RF_logen_mx2g_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_MX5G_TUNE, ci->RF_logen_mx5g_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_INDBUF2G_TUNE, ci->RF_logen_indbuf2g_tune);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LOGEN_INDBUF5G_TUNE, ci->RF_logen_indbuf5g_tune);

		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX2G_TUNE_BOOST_PU_CORE0,
		                ci->RF_txmix2g_tune_boost_pu_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD2G_TUNE_PUS_CORE0, ci->RF_pad2g_tune_pus_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PGA_BOOST_TUNE_CORE0, ci->RF_pga_boost_tune_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX5G_BOOST_TUNE_CORE0,
		                ci->RF_txmix5g_boost_tune_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD5G_TUNE_MISC_PUS_CORE0,
		                ci->RF_pad5g_tune_misc_pus_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA2G_TUNE_CORE0, ci->RF_lna2g_tune_core0);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA5G_TUNE_CORE0, ci->RF_lna5g_tune_core0);

		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX2G_TUNE_BOOST_PU_CORE1,
		                ci->RF_txmix2g_tune_boost_pu_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD2G_TUNE_PUS_CORE1, ci->RF_pad2g_tune_pus_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PGA_BOOST_TUNE_CORE1, ci->RF_pga_boost_tune_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_TXMIX5G_BOOST_TUNE_CORE1,
		                ci->RF_txmix5g_boost_tune_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_PAD5G_TUNE_MISC_PUS_CORE1,
		                ci->RF_pad5g_tune_misc_pus_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA2G_TUNE_CORE1, ci->RF_lna2g_tune_core1);
		phy_utils_write_radioreg(pi,
		                RADIO_2057_LNA5G_TUNE_CORE1, ci->RF_lna5g_tune_core1);
	}

	if ((RADIOREV(pi->pubpi->radiorev) <= 4) || (RADIOREV(pi->pubpi->radiorev) == 6)) {
		/* for 2G band on 43226 boards, going to extreme PLL settings (190 KHz of PLL BW)
		 * helped improve sensitivity and lower PER floor at higher rx powers
		 * This implies VCO phase noise is the dominant source of noise
		 * Remains to be seen if the same settings work for 6362
		 * this will need to be enabled by board flags or chip ID if necessary
		 *
		 * Is this the right place for these band-specific settings?
		 * In TCL, they are in mimophy_workaround.
		 * In DRV, since tuning table gets reloaded when scanning, they need to be here.
		 */
		if (CHSPEC_IS2G(pi->radio_chanspec)) {
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_R1, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_CP_KPD_IDAC, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C1, 0x8);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C2, 0x8);
		} else {
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_R1, 0x1f);
			phy_utils_write_radioreg(pi, RADIO_2057_CP_KPD_IDAC, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C1, 0x8);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C2, 0x8);
		}
	} else if ((RADIOREV(pi->pubpi->radiorev) == 7) || (RADIOREV(pi->pubpi->radiorev) == 8) ||
	           (RADIOREV(pi->pubpi->radiorev) == 12)) {
	  if (CHSPEC_IS5G(pi->radio_chanspec)) {
		freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
		if (pi->sh->boardtype == BCM943236OLYMPICSULLEY_SSID) {
		  if ((freq >= 5240) && (freq <= 5825)) {
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_R1, 0x1f);
			phy_utils_write_radioreg(pi, RADIO_2057_CP_KPD_IDAC, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C1, 0x8);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C2, 0x8);
		  }
		} else {
		  if ((freq >= 5240) && (freq <= 5500)) {
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_R1, 0x1f);
			phy_utils_write_radioreg(pi, RADIO_2057_CP_KPD_IDAC, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C1, 0x8);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C2, 0x8);
		  }
		}
	  }
	} else if ((RADIOREV(pi->pubpi->radiorev) == 13) ||
		(RADIOREV(pi->pubpi->radiorev) == 14)) {
			/* narrow to 91KHz PLL BW */
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_R1, 0x1b);
			phy_utils_write_radioreg(pi, RADIO_2057_CP_KPD_IDAC, 0x3f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C1, 0x1f);
			phy_utils_write_radioreg(pi, RADIO_2057_RFPLL_LOOPFILTER_C2, 0x1f);
	} else {
		PHY_ERROR(("REV8 FIXME: need to find optimal PLL filter for 2057 radio rev %d\n",
		           RADIOREV(pi->pubpi->radiorev)));
	}

	freq = CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	if (CHSPEC_IS2G(pi->radio_chanspec)) {
		if (PHY_IPA(pi)) {
			/* Mixer Gain Boost */
			if (RADIOREV(pi->pubpi->radiorev) == 3) {
				txmix2g_tune_boost_pu = 0x6b;
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) {
				/* BCM63268 */
				if (!CHSPEC_IS40(pi->radio_chanspec)) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x43);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x23);
				} else {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x03);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x03);
				}
				for (coreNum = 0; coreNum <= 1; coreNum++) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE,
					                 coreNum, TXMIX2G_TUNE_BOOST_PU,
					                 0x41);
				}
			} else if (RADIOREV(pi->pubpi->radiorev) == 13) {
				pad2g_tune_pus = 0x53;
				txmix2g_tune_boost_pu = 0x61;
			} else if (RADIOREV(pi->pubpi->radiorev) == 14) {
				txmix2g_tune_boost_pu = 0x21;
				for (coreNum = 0; coreNum <= 1; coreNum++) {
					if (RADIOVER(pi->pubpi->radiover) == 0) {
	if (CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec)) <= 2452) {
		pad2g_tune_pus = 0x23;
	} else {
		pad2g_tune_pus = 0x13;
	}
					} else {
						if (CHSPEC_IS40(pi->radio_chanspec) == 1) {
							pad2g_tune_pus = 0x23;
						} else {
	if (CHAN2G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec)) <= 2452) {
		pad2g_tune_pus = 0x63;
	} else {
		pad2g_tune_pus = 0x53;
	}
						}
					}
				}
			}
			if (RADIOREV(pi->pubpi->radiorev) == 5) {
				if ((RADIOVER(pi->pubpi->radiover) == 0x0) ||
					(RADIOVER(pi->pubpi->radiover) == 0xe) ||
					(RADIOVER(pi->pubpi->radiover) == 0xf)) {
					/* 5357A0 and TC1, TC2 */
					pad2g_tune_pus = 0x73;
				} else {
					if ((RADIOVER(pi->pubpi->radiover) >= 0x2) ||
					    (RADIOVER(pi->pubpi->radiover) == 0xc)) {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
						                 PAD2G_TUNE_PUS, 0x53);
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
						                 PAD2G_TUNE_PUS, 0x53);
					} else {
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
						                 PAD2G_TUNE_PUS, 0x23);
						WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
						                 PAD2G_TUNE_PUS, 0x33);
					}

					txmix2g_tune_boost_pu = 0x61;

					if (CHSPEC_IS40(pi->radio_chanspec)) {
						if ((pi->sh->sromrev >= 8) &&
						  (BOARDFLAGS2(GENERIC_PHY_INFO(pi)->boardflags2) &
						  BFL2_FCC_BANDEDGE_WAR)) {
							if (freq == 2422) {
								txdigi_filt_coeffs_type = 7;
							} else {
								txdigi_filt_coeffs_type = 1;
							}
							for (j = 0; j < NPHY_NUM_DIG_FILT_COEFFS;
							     j++) {
								phy_utils_write_phyreg(pi,
								NPHY_txfilt40CoeffStg0A1+j,
								NPHY_IPA_REV4_txdigi_filtcoeffs
								[txdigi_filt_coeffs_type][j]);
							}

							rccal_bcap_val = phy_utils_read_radioreg(
								pi, RADIO_2057_RCCAL_BCAP_VAL);
							rccal_scap_val = phy_utils_read_radioreg(
								pi, RADIO_2057_RCCAL_SCAP_VAL);
							tx_lpf_bw_ofdm_40mhz[0] =
							        wlc_phy_read_lpf_bw_ctl_nphy(
									pi, 0x159);
							tx_lpf_bw_ofdm_40mhz[1] =
							        wlc_phy_read_lpf_bw_ctl_nphy(
									pi, 0x169);
							if (freq == 2422) {
								/* 40 MHz bandedge RCCAL tweak */
								rccal_tx40_11n_bcap[0] =
								        rccal_bcap_val;
								rccal_tx40_11n_scap[0] =
								        rccal_scap_val;
								rccal_tx40_11n_bcap[1] =
								        rccal_bcap_val;
								rccal_tx40_11n_scap[1] =
								        rccal_scap_val;
							} else {
								rccal_tx40_11n_bcap[0] =
								        (uint16)
								        MIN((int16)rccal_bcap_val +
								            20, 0x1f);
								rccal_tx40_11n_scap[0] =
								        (uint16)
								        MIN((int16)rccal_scap_val +
								            20, 0x1f);
								rccal_tx40_11n_bcap[1] =
								        (uint16)
								        MIN((int16)rccal_bcap_val +
								            10, 0x1f);
								rccal_tx40_11n_scap[1] =
								        (uint16)
								        MIN((int16)rccal_scap_val +
								            10, 0x1f);
							}
							for (coreNum = 0; coreNum <= 1; coreNum++) {
								rx2tx_lpf_rc_lut_tx40_11n =
								(1 << 13) |
								(rccal_tx40_11n_bcap[coreNum]<< 8) |
								(rccal_tx40_11n_scap[coreNum]<< 3) |
								tx_lpf_bw_ofdm_40mhz[coreNum];

								wlc_phy_table_write_nphy(
									pi, NPHY_TBL_ID_RFSEQ, 1,
									0x155 + coreNum * 0x10, 16,
									&rx2tx_lpf_rc_lut_tx40_11n);
								wlc_phy_table_write_nphy(
									pi, NPHY_TBL_ID_RFSEQ, 1,
									0x156 + coreNum * 0x10, 16,
									&rx2tx_lpf_rc_lut_tx40_11n);
								wlc_phy_table_write_nphy(
									pi, NPHY_TBL_ID_RFSEQ, 1,
									0x157 + coreNum * 0x10, 16,
									&rx2tx_lpf_rc_lut_tx40_11n);
								wlc_phy_table_write_nphy(
									pi, NPHY_TBL_ID_RFSEQ, 1,
									0x158 + coreNum * 0x10, 16,
									&rx2tx_lpf_rc_lut_tx40_11n);
								wlc_phy_table_write_nphy(
									pi, NPHY_TBL_ID_RFSEQ, 1,
									0x159 + coreNum * 0x10, 16,
									&rx2tx_lpf_rc_lut_tx40_11n);
							}
						}
					}
				}
			}
			if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
				(RADIOVER(pi->pubpi->radiover) > 0)) {
			  if (RADIOVER(pi->pubpi->radiover) == 1) {
				if (freq <= 2412)
				  pad2g_tune_pus = 0x63;
				else if (freq <= 2432)
				  pad2g_tune_pus = 0x53;
				else
				  pad2g_tune_pus = 0x43;
			  } else if (RADIOVER(pi->pubpi->radiover) == 2) {
				if (CHSPEC_IS40(pi->radio_chanspec)) {
				  if (freq <= 2422) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x23);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x33);
				  } else if (freq <= 2442) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x23);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x53);
				  } else if (freq <= 2447) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x53);
				  } else if (freq <= 2452) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x33);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x33);
				  } else {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x53);
				  }
				} else {
				  if (freq <= 2412) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x23);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x23);
				  } else if (freq <= 2422) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x23);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x43);
				  } else if (freq <= 2442) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x43);
				  } else if (freq <= 2447) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x53);
				  } else if (freq <= 2457) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x23);
				  } else if (freq <= 2462) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x33);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x43);
				  } else {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0,
					                 PAD2G_TUNE_PUS, 0x13);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
					                 PAD2G_TUNE_PUS, 0x23);
				  }
				}
			  } else {
				pad2g_tune_pus = 0x43;
			  }
			  txmix2g_tune_boost_pu = 0x01;

			  if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
			      (RADIOVER(pi->pubpi->radiover)  == 2) &&
			      (CHSPEC_IS40(pi->radio_chanspec))) {
			  /* 43236 b1 bandedge tweaking */
					rccal_bcap_val = phy_utils_read_radioreg(
						pi, RADIO_2057_RCCAL_BCAP_VAL);
					rccal_scap_val = phy_utils_read_radioreg(
						pi, RADIO_2057_RCCAL_SCAP_VAL);
					tx_lpf_bw_ofdm_40mhz[0] =
					  wlc_phy_read_lpf_bw_ctl_nphy(
					  pi, 0x159);
					tx_lpf_bw_ofdm_40mhz[1] =
					  wlc_phy_read_lpf_bw_ctl_nphy(
					  pi, 0x169);
					for (coreNum = 0; coreNum <= 1; coreNum++) {
					  /* overwrite rc-cal for bandedge channels only */
					  if ((freq == 2422) || (freq == 2452)) {
						rccal_tx40_11n_bcap[coreNum] = (uint16)
						  MAX((int16)rccal_bcap_val - 7, 0);
						rccal_tx40_11n_scap[coreNum] = (uint16)
						  MAX((int16)rccal_scap_val - 7, 0);
					  } else {
						rccal_tx40_11n_bcap[coreNum] =
						  rccal_bcap_val;
						rccal_tx40_11n_scap[coreNum] = (uint16)
						  MIN((int16)rccal_scap_val + 2,
						      0x1f);
					  }
					  rx2tx_lpf_rc_lut_tx40_11n = (1 << 13) |
					    (rccal_tx40_11n_bcap[coreNum] << 8) |
					    (rccal_tx40_11n_scap[coreNum] << 3) |
					    tx_lpf_bw_ofdm_40mhz[coreNum];
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x155 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x156 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x157 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x158 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x159 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					}
			  }
			}
		} else {
			if ((RADIOREV(pi->pubpi->radiorev) <= 4) ||
				(RADIOREV(pi->pubpi->radiorev) == 6)) {
				pad2g_tune_pus = 0x3;

				/* Reduce Tx mixer boost to reduce internal radio gain */
				txmix2g_tune_boost_pu = 0x61;
			} else if ((RADIOREV(pi->pubpi->radiorev) == 5) &&
				(RADIOVER(pi->pubpi->radiover) >= 2)) {
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 0, PAD2G_TUNE_PUS, 0x43);
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1, PAD2G_TUNE_PUS, 0x43);
			} else if (RADIOREV(pi->pubpi->radiorev) == 12) { /* BCM63268 */
				/* Reduce Tx mixer boost to reduce internal radio gain */
				txmix2g_tune_boost_pu = 0x61;
				for (coreNum = 0; coreNum <= 1; coreNum++) {
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 TXGM_IDAC_BLEED, 0x70);
					/* Reduce PAD tuning to reduce internal radio gain */
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 PAD2G_TUNE_PUS, 0x53);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_CASCONV, 0xf);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_IMAIN, 0xf);
					WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
					                 IPA2G_BIAS_FILTER, 0xee);
				}
			}
		}

		for (coreNum = 0; coreNum <= 1; coreNum++) {
			/* Mixer Gain Boost */
			if (txmix2g_tune_boost_pu != 0)
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
				                 TXMIX2G_TUNE_BOOST_PU, txmix2g_tune_boost_pu);

			if (pad2g_tune_pus != 0)
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum,
				                 PAD2G_TUNE_PUS, pad2g_tune_pus);

			if (pad2g_tune_pus_core1 != 0)
				WRITE_RADIO_REG4(pi, RADIO_2057, CORE, 1,
				                 PAD2G_TUNE_PUS, pad2g_tune_pus_core1);
		}
	} else {
	  freq = CHAN5G_FREQ(CHSPEC_CHANNEL(pi->radio_chanspec));
	if ((RADIOREV(pi->pubpi->radiorev) == 7) &&
		(RADIOVER(pi->pubpi->radiover)  == 2) &&
		(CHSPEC_IS40(pi->radio_chanspec))) {
			  /* 43236 b1 bandedge tweaking */
					rccal_bcap_val = phy_utils_read_radioreg(
						pi, RADIO_2057_RCCAL_BCAP_VAL);
					rccal_scap_val = phy_utils_read_radioreg(
						pi, RADIO_2057_RCCAL_SCAP_VAL);
					tx_lpf_bw_ofdm_40mhz[0] =
					  wlc_phy_read_lpf_bw_ctl_nphy(
					  pi, 0x159);
					tx_lpf_bw_ofdm_40mhz[1] =
					  wlc_phy_read_lpf_bw_ctl_nphy(
					  pi, 0x169);
					for (coreNum = 0; coreNum <= 1; coreNum++) {
					  /* overwrite rc-cal for freq = 5190 only for bandedge */
					  if (freq == 5190) {
						rccal_tx40_11n_bcap[coreNum] = (uint16)
						  MAX((int16)rccal_bcap_val - 11, 0);
						rccal_tx40_11n_scap[coreNum] = (uint16)
						  MAX((int16)rccal_scap_val - 11, 0);
					  } else {
						rccal_tx40_11n_bcap[coreNum] =
						  rccal_bcap_val;
						rccal_tx40_11n_scap[coreNum] = (uint16)
						  MIN((int16)rccal_scap_val + 2,
						      0x1f);
					  }
					  rx2tx_lpf_rc_lut_tx40_11n = (1 << 13) |
					    (rccal_tx40_11n_bcap[coreNum] << 8) |
					    (rccal_tx40_11n_scap[coreNum] << 3) |
					    tx_lpf_bw_ofdm_40mhz[coreNum];
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x155 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x156 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x157 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x158 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					wlc_phy_table_write_nphy(
						pi, NPHY_TBL_ID_RFSEQ, 1,
						0x159 + coreNum * 0x10, 16,
						&rx2tx_lpf_rc_lut_tx40_11n);
					}
	}
	if ((RADIOREV(pi->pubpi->radiorev) == 7) && (RADIOVER(pi->pubpi->radiover) == 2)) {
		if (freq >= 5805) {
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_IMAIN_CORE0, 0x16);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_IMAIN_CORE1, 0x13);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_CASCONV_CORE0, 0x12);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_CASCONV_CORE1, 0x14);
		} else {
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_IMAIN_CORE0, 0x13);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_IMAIN_CORE1, 0x13);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_CASCONV_CORE0, 0x14);
		  phy_utils_write_radioreg(pi, RADIO_2057_IPA5G_CASCONV_CORE1, 0x14);
		}
	  }
	  if ((RADIOREV(pi->pubpi->radiorev) == 7) && (RADIOVER(pi->pubpi->radiover) > 0)) {
		if (((freq >= 5180) && (freq <= 5300)) ||
			((freq >= 5745) && (freq <= 5805))) {
		  ipa5g_bf = 0xee;
		} else {
		  ipa5g_bf = 0xff;
		}
	  } else if (RADIOREV(pi->pubpi->radiorev) == 12) {
		  /* BCM63268 */
		  if ((freq >= 5180) && (freq <= 5510)) {
			phy_utils_write_radioreg(pi, RADIO_2057_TXMIX5G_BOOST_TUNE_CORE0,
			                         ((4 << 4)|0x8));
			phy_utils_write_radioreg(pi, RADIO_2057_PGA_BOOST_TUNE_CORE0,
			                         ((8 << 4)|0x8));
			phy_utils_write_radioreg(pi, RADIO_2057_PAD5G_TUNE_MISC_PUS_CORE0, 0x63);
		  }

		  if (!CHSPEC_IS40(pi->radio_chanspec)) {
			  phy_utils_write_radioreg(pi, RADIO_2057_LOGEN_INDBUF5G_IDAC, 0x44);
		  } else {
			  phy_utils_write_radioreg(pi, RADIO_2057_LOGEN_INDBUF5G_IDAC, 0x55);
		  }
		  phy_utils_write_radioreg(pi, RADIO_2057_LOGEN_MX5G_IDACS, 0x39);
		  for (coreNum = 0; coreNum <= 1; coreNum++) {
			  WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum, IPA5G_BIAS_FILTER, 0xee);
		  }
	  } else {
		if (((freq >= 5180) && (freq <= 5230)) ||
			((freq >= 5745) && (freq <= 5805))) {
		  ipa5g_bf = 0xff;
		}
	  }
	  if (ipa5g_bf != 0) {
		for (coreNum = 0; coreNum <= 1; coreNum++) {
		  WRITE_RADIO_REG4(pi, RADIO_2057, CORE, coreNum, IPA5G_BIAS_FILTER, ipa5g_bf);
		}
	  }
	}

	/* Guard time pre-vco-cal */
	OSL_DELAY(50);

	/* Do a VCO cal after writing the tuning table regs */
	wlc_phy_radio205x_vcocal_nphy(pi);
}

static uint16
wlc_phy_radio2057_rccal(phy_info_t *pi)
{
	uint16 rccal_valid;
	int i;
	bool chip43226_6362A0;

	/* Radio Rev 3,4,6 belongs to 43226/6362A0 */
	chip43226_6362A0 = ((RADIOREV(pi->pubpi->radiorev) == 3) ||
		(RADIOREV(pi->pubpi->radiorev) == 4) || (RADIOREV(pi->pubpi->radiorev) == 6));

	/* Cal bcap */
	rccal_valid = 0;
	if (chip43226_6362A0) {
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_MASTER,   0x61);
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0xc0);
	} else {
		phy_utils_write_radioreg(pi, RADIO_2057v7_RCCAL_MASTER, 0x61);
		/* All others revs except 43226/6362A0 has bigger Caps.
		 * Hence different settings
		 */
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0xe9);
	}
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_X1,               0x6e);
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1,   0x55);

	for (i = 0; i < MAX_205x_RCAL_WAITLOOPS; i++) {
		rccal_valid = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_DONE_OSCCAP);
		if (rccal_valid & 0x2) {
			break;
		}
		OSL_DELAY(25);
	}

	ASSERT(rccal_valid & 0x2);

	OSL_DELAY(35);

	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1, 0x15);

	OSL_DELAY(70);

	/* Cal scap */
	rccal_valid = 0;
	if (chip43226_6362A0) {
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_MASTER,   0x69);
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0xb0);
	} else {
		phy_utils_write_radioreg(pi, RADIO_2057v7_RCCAL_MASTER, 0x69);
		/* All others revs except 43226/6362A0 has bigger Caps.
		 * Hence different settings
		 */
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0xd5);
	}
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_X1,               0x6e);
	OSL_DELAY(35);
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1,   0x55);
	OSL_DELAY(70);
	for (i = 0; i < MAX_205x_RCAL_WAITLOOPS; i++) {
		rccal_valid = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_DONE_OSCCAP);
		if (rccal_valid & 0x2) {
			break;
		}
		OSL_DELAY(25);
	}

	ASSERT(rccal_valid & 0x2);

	OSL_DELAY(35);
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1, 0x15);
	OSL_DELAY(70);
	/* Cal hpc */
	rccal_valid = 0;
	if (chip43226_6362A0) {
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_MASTER,   0x73);
		/* Values of X1/TRC0 are not optimal
		 * However these chips are not going into production
		 */
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_X1,               0x28);
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0xb0);
	} else {
		phy_utils_write_radioreg(pi, RADIO_2057v7_RCCAL_MASTER, 0x73);
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_X1,               0x6e);
		phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_TRC0,             0x99);
	}
	OSL_DELAY(35);
	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1,   0x55);
	OSL_DELAY(70);
	for (i = 0; i < MAX_205x_RCAL_WAITLOOPS; i++) {
		rccal_valid = phy_utils_read_radioreg(pi, RADIO_2057_RCCAL_DONE_OSCCAP);
		if (rccal_valid & 0x2) {
			break;
		}
		OSL_DELAY(25);
	}

	ASSERT(rccal_valid & 0x2);
	OSL_DELAY(35);

	phy_utils_write_radioreg(pi, RADIO_2057_RCCAL_START_R1_Q1_P1, 0x15);
	OSL_DELAY(70);
	if (chip43226_6362A0) {
		phy_utils_mod_radioreg(pi, RADIO_2057_RCCAL_MASTER, 0x1, 0x0);
	} else {
		phy_utils_mod_radioreg(pi, RADIO_2057v7_RCCAL_MASTER, 0x1, 0x0);
	}

	return (rccal_valid);
}
