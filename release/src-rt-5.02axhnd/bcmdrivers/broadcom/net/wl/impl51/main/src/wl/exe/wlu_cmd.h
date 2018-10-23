/*
 * Command structure for wl command line utility
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
 * $Id: wlu_cmd.h 674525 2016-12-09 04:17:19Z $
 */

#ifndef _wlu_cmd_h_
#define _wlu_cmd_h_

/*   wl_nan_sub_cmd may also be used in dhd  */
typedef struct wl_nan_sub_cmd wl_nan_sub_cmd_t;
typedef int (cmd_handler_t)(void *wl, const wl_nan_sub_cmd_t *cmd, int argc, char **argv,
	bool *is_set, uint8 *iov_data, uint16 *avail_len);
/*
 * nan sub-commands list entry
 */
struct wl_nan_sub_cmd {
	char *name;
	uint8 version;
	uint16 id;
	uint16 type;
	cmd_handler_t *handler;
};

/*
 * NAN sub-command help list entry
 */
typedef struct wl_nan_cmd_help wl_nan_cmd_help_t;
typedef void (cmd_help_handler_t)(void);
struct wl_nan_cmd_help {
	uint16 type;
	cmd_help_handler_t *help_handler;
};

typedef struct cmd cmd_t;
typedef int (cmd_func_t)(void *wl, cmd_t *cmd, char **argv);

/* generic command line argument handler */
struct cmd {
	const char *name;
	cmd_func_t *func;
	int get;
	int set;
	const char *help;
};

/* list of command line arguments */
extern cmd_t wl_cmds[];
extern cmd_t wl_varcmd;

/* per-port ioctl handlers */
extern int wl_get(void *wl, int cmd, void *buf, int len);
extern int wl_set(void *wl, int cmd, void *buf, int len);

/*
 * Flags for command categories.  A command may belong to
 * multiple categories.  These are used in bitmaps, so be careful
 * to keep the macro value (2 ^ n) and the array indexes (n)
 * consistent.
 */

#define CMD_PHY               0x1
#define CMD_CHAN              0x2
#define CMD_RATE              0x4
#define CMD_POWER             0x8
#define CMD_MAC               0x10
#define CMD_MGMT              0x20
#define CMD_SEC               0x40
#define CMD_WME               0x80
#define CMD_MON               0x100
#define CMD_AP                0x200
#define CMD_STA               0x400
#define CMD_BOARD             0x800
#define CMD_ADMIN             0x1000
#define CMD_DEV               0x2000
#define CMD_DEP               0x4000
#define CMD_UNCAT             0x8000

#define CMD_ALL               0xffff

/* Initializer for category string array */

#define CMD_CATEGORY_STRINGS_INIT { \
	"phy",   \
	"chan",  \
	"rate",  \
	"power", \
	"mac",   \
	"mgmt",  \
	"sec",   \
	"wme",   \
	"mon",   \
	"ap",    \
	"sta",   \
	"board", \
	"admin", \
	"dev",   \
	"dep",   \
	"uncat", \
	"" }

extern const char *wl_cmd_category_strings[];
extern const int wl_cmd_category_count;

/* Initializer for category description strings array */
#define CMD_CATEGORY_DESC_INIT { \
	"PHY and radio; speed, band, etc", \
	"Channel; subclass of phy",  \
	"Rate; subclass of phy, a/b/g",  \
	"Power; subclass of phy", \
	"MAC; Media access",   \
	"Management, association, IE, etc",  \
	"Security; subclass of mgmt",   \
	"WME; media extensions",   \
	"Monitoring device (counters, etc)",   \
	"AP subclass of mgmt", \
	"STA subclass of mgmt", \
	"Board, hardware", \
	"Administration; software, UI, diags", \
	"Device; low level control", \
	"Deprecated", \
	"Uncategorized so far", \
	"" }

extern const char *wl_cmd_category_desc[];

/*
 *
 * IO variable information
 *
 */

/* Supplemental IO variable info structure */
typedef const struct wlu_iov_info_s {
	const char *name;
	uint32 cat;  /* Category flags; same as command categories */
	uint32 flags; /* See below */
	int dflt;  /* Only for integers; see flags */
	const char *desc;  /* Description */
} wlu_iov_info_t;

/* Flags for wlu_iov_info_t */
#define WLU_IOVI_READ_ONLY         0x1   /* Known to be read only */
#define WLU_IOVI_WRITE_ONLY        0x2   /* Known to be write only */
#define WLU_IOVI_BCM_INTERNAL      0x4   /* Known to be BCM internal */
#define WLU_IOVI_DEFAULT_VALID     0x8   /* Default value in structure is valid */

extern wlu_iov_info_t wlu_iov_info[];
extern int wlu_iov_info_count;

#define WLU_IOV_BLOCK_LEN	10

#define WLU_MOD_NAME_MAX	64 /* Max num modules */
#define WLU_MOD_NAME_BYTES	16 /* Size of modules name */

extern int wl_get_scan(void *wl, int opc, char *scan_buf, uint buf_len);

#endif /* _wlu_cmd_h_ */
