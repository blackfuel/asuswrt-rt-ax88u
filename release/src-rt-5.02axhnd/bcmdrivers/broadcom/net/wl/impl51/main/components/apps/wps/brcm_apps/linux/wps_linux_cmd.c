/*
 * WPS main (platform dependent portion)
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
 * $Id: wps_linux_cmd.c 525052 2015-01-08 20:18:35Z $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include <typedefs.h>
#include <security_ipc.h>
#include <shutils.h>
#include <tutrace.h>
#include <wps_ui.h>
#include <net/if.h>

typedef struct {
	int wps_config_command;
	int wps_action;
	int wps_method;
	char wps_autho_sta_mac[sizeof("00:00:00:00:00:00")];
	char wps_ifname[16];
#ifdef WPS_UPNP_DEVICE
	char wps_uuid[36];
#endif // endif
	int wps_aplockdown;
	int wps_proc_status;
	int wps_pinfail;
	char wps_sta_mac[sizeof("00:00:00:00:00:00")];
	char wps_sta_devname[32];
#ifdef WPS_NFC_DEVICE
	int wps_nfc_dm_status;
	int wps_nfc_err_code;
#endif // endif
	unsigned int wps_msglevel;
} wps_env_t;

typedef struct {
	uint32 value;
	const char *string;
} wps_str_t;

/* wps_config_command */
static wps_str_t _cmd_str[] = {
	{WPS_UI_CMD_NONE,	"None"},
	{WPS_UI_CMD_START,	"Start"},
	{WPS_UI_CMD_STOP,	"Stop"},
	{WPS_UI_CMD_NFC_WR_CFG,	"NFC Write Configuration"},
	{WPS_UI_CMD_NFC_RD_CFG,	"NFC Read Configuration"},
	{WPS_UI_CMD_NFC_WR_PW,	"NFC Write Password"},
	{WPS_UI_CMD_NFC_RD_PW,	"NFC Read Password"},
	{WPS_UI_CMD_NFC_HO_S,	"NFC Hand Over Selector"},
	{WPS_UI_CMD_NFC_HO_R,	"NFC Hand Over Rqquester"},
	{WPS_UI_CMD_NFC_FM,	"NFC Format"},
	{WPS_UI_CMD_MSGLEVEL,	"WPS msglevel"},
	{0,		NULL}
};

/* wps_action */
static wps_str_t _action_str[] = {
	{WPS_UI_ACT_NONE,		"None"},
	{WPS_UI_ACT_ENROLL,		"Enroll"},
	{WPS_UI_ACT_CONFIGAP,		"Config AP"},
	{WPS_UI_ACT_ADDENROLLEE,	"Add Enrollee"},
	{WPS_UI_ACT_STA_CONFIGAP,	"STA Config AP"},
	{WPS_UI_ACT_STA_GETAPCONFIG,	"STA Get AP Config"},
	{0,				NULL}
};

/* wps_method */
static wps_str_t _method_str[] = {
	{WPS_UI_METHOD_NONE,	"NONE"},
	{WPS_UI_METHOD_PIN,	"PIN"},
	{WPS_UI_METHOD_PBC,	"PBC"},
	{WPS_UI_METHOD_NFC_PW,	"NFC_PW"},
	{WPS_UI_METHOD_NFC_CHO,	"NFC_CHO"},
	{0,			NULL}
};

/* wps_proc_status */
static wps_str_t _status_str[] = {
	{WPS_UI_INIT,			"Init"},
	{WPS_UI_ASSOCIATED,		"Processing WPS start..."},
	{WPS_UI_OK,			"Success"},
	{WPS_UI_MSG_ERR,		"Fail due to WPS message exchange error!"},
	{WPS_UI_TIMEOUT,		"Fail due to WPS time out!"},
	{WPS_UI_SENDM2,			"Sent M2"},
	{WPS_UI_SENDM7,			"Sent M7"},
	{WPS_UI_MSGDONE,		"Success"},
	{WPS_UI_PBCOVERLAP,		"Fail due to WPS session overlap!"},
	{WPS_UI_FIND_PBC_AP,		"Search PBC AP"},
	{WPS_UI_FIND_SEL_AP,		"Search a specific selected registrar AP"},
	{WPS_UI_ASSOCIATING,		"Associating"},
	{WPS_UI_NFC_WR_CFG,		"Please place your NFC token now. [WR CFG]"},
	{WPS_UI_NFC_WR_PW,		"Please place your NFC token now. [WR PW]"},
	{WPS_UI_NFC_WR_CPLT,		"NFC write token successful, please remove the tag."},
	{WPS_UI_NFC_RD_CFG,		"Please place your NFC token now. [RD CFG]"},
	{WPS_UI_NFC_RD_PW,		"Please place your NFC token now. [RD PW]"},
	{WPS_UI_NFC_RD_CPLT,		"NFC read token successful, please remove the tag."},
	{WPS_UI_NFC_HO_S,		"Handover as selector."},
	{WPS_UI_NFC_HO_R,		"Handover as requester."},
	{WPS_UI_NFC_HO_NDEF,		"Handover done, please remove the peer."},
	{WPS_UI_NFC_HO_CPLT,		"Handover successful."},
	{WPS_UI_NFC_OP_ERROR,		"NFC operation fail."},
	{WPS_UI_NFC_OP_STOP,		"NFC operation stop."},
	{WPS_UI_NFC_OP_TO,		"NFC operation timeout."},
	{WPS_UI_NFC_FM,			"Formating NFC, please place your NFC token now!."},
	{WPS_UI_NFC_FM_CPLT,		"Format NFC successful, please remove the tag."},
	{WPS_UI_NFC_HO_DPI_MISMATCH,	"Handover failed, device password ID mismatch."},
	{WPS_UI_NFC_HO_PKH_MISMATCH,	"Handover failed, public key hash mismatch."},
	{0,				NULL}
};
/* wps msglevel */
static wps_str_t _msglevel_str[] = {
	{TUERR, 	"error"},
	{TUERR, 	"err"},
	{TUINFO,	"info"},
	{TUINFO,	"inf"},
	{TUNFC,		"nfc"},
	{TUDUMP_MSG,	"msg_dump"},
	{TUDUMP_IE,	"ie_dump"},
	{TUDUMP_PROBE,	"probe_dump"},
	{TUDUMP_KEY,	"key_dump"},
	{TUDUMP_NFC,	"nfc_dump"},
	{TUTIME,	"time"},
	{0,		NULL}
};

#define NFC_STATUS_STR(status) \
	((status == WPS_UI_NFC_STATUS_INITING) ? "Initiating" : \
	 (status == WPS_UI_NFC_STATUS_INITED) ? "Successful Initiated" : "Error")

static wps_env_t s_wps_env;
static wps_env_t *wps_env = &s_wps_env;

#define MAX_WPS_ENV_ARGS 32

static int wps_hwaddr_check(char *value);

static const char *
_get_wps_str(wps_str_t *wps_str, uint32 value)
{
	int i;

	if (wps_str == NULL)
		return "Null Argument";

	for (i = 0; wps_str[i].string != NULL; i++) {
		if (wps_str[i].value == value)
			return wps_str[i].string;
	}

	return "Uknown";
}

static int
parse_wps_env(char *buf)
{
	char *argv[MAX_WPS_ENV_ARGS] = {0};
	char *value, *p, *name;
	int i;

	/* Seperate buf into argv[], we have to make sure at least one is empty */
	for (i = 0, p = buf; i < MAX_WPS_ENV_ARGS-1; i++) {
		/* Eat white space */
		while (*p == ' ')
			p++;
		if (*p == 0)
			goto all_found;

		/* Save this item */
		argv[i] = p;

		 /* Search until space */
		while (*p != ' ' && *p) {
			/* Take care of doube quot */
			if (*p == '\"') {
				char *qs, *qe;

				qs = p;
				qe = strchr(p+1, '\"');
				if (qe == NULL) {
					fprintf(stderr, "%s:%d, unbalanced quote string!",
						__func__, __LINE__);
					argv[i] = 0;
					goto all_found;
				}

				/* Null eneded quot string and do shift */
				*qe = '\0';
				memmove(qs, qs+1, (int)(qe-qs));

				p = qe+1;
				break;
			}

			p++;
		}

		if (*p)
			*p++ = '\0';
	}

all_found:
	/* Parse message */
	wps_env->wps_config_command = WPS_UI_CMD_NONE;
	wps_env->wps_method = 0;
	memset(wps_env->wps_autho_sta_mac, 0, sizeof(wps_env->wps_autho_sta_mac));

	for (i = 0; argv[i]; i++) {
		value = argv[i];
		name = strsep(&value, "=");
		if (name && value) {
			if (!strcmp(name, "wps_config_command"))
				wps_env->wps_config_command = atoi(value);
			else if (!strcmp(name, "wps_action"))
				wps_env->wps_action = atoi(value);
			else if (!strcmp(name, "wps_method"))
				wps_env->wps_method = atoi(value);
			else if (!strcmp(name, "wps_autho_sta_mac"))
				memcpy(wps_env->wps_autho_sta_mac, value,
					sizeof(wps_env->wps_autho_sta_mac));
			else if (!strcmp(name, "wps_ifname")) {
				strncpy(wps_env->wps_ifname, value, sizeof(wps_env->wps_ifname));
				wps_env->wps_ifname[sizeof(wps_env->wps_ifname) -1 ] = 0;
			}
#ifdef WPS_UPNP_DEVICE
			else if (!strcmp(name, "wps_uuid")) {
				strncpy(wps_env->wps_uuid, value, sizeof(wps_env->wps_uuid));
				wps_env->wps_uuid[sizeof(wps_env->wps_uuid) - 1] = 0;
			}
#endif // endif
			else if (!strcmp(name, "wps_aplockdown"))
				wps_env->wps_aplockdown = atoi(value);
			else if (!strcmp(name, "wps_proc_status"))
				wps_env->wps_proc_status = atoi(value);
			else if (!strcmp(name, "wps_pinfail"))
				wps_env->wps_pinfail = atoi(value);
			else if (!strcmp(name, "wps_sta_mac"))
				memcpy(wps_env->wps_sta_mac, value, sizeof(wps_env->wps_sta_mac));
			else if (!strcmp(name, "wps_sta_devname")) {
				strncpy(wps_env->wps_sta_devname, value,
					sizeof(wps_env->wps_sta_devname));
				wps_env->wps_sta_devname[sizeof(wps_env->wps_sta_devname) - 1] = 0;
			}
#ifdef WPS_NFC_DEVICE
			else if (!strcmp(name, "wps_nfc_dm_status"))
				wps_env->wps_nfc_dm_status = atoi(value);
			else if (!strcmp(name, "wps_nfc_err_code"))
				wps_env->wps_nfc_err_code = atoi(value);
#endif // endif
			else if (!strcmp(name, "wps_msglevel"))
				wps_env->wps_msglevel = strtoul(value, NULL, 16);
		}
	}

	return 0;
}

static void
print_wps_env()
{
	/* Should not have WPS_UI_CMD_MSGLEVEL case */
	fprintf(stderr, "Command: %d (%s)\n", wps_env->wps_config_command,
		_get_wps_str(_cmd_str, wps_env->wps_config_command));
	fprintf(stderr, "      Action: %d (%s)\n", wps_env->wps_action,
		_get_wps_str(_action_str, wps_env->wps_action));
	fprintf(stderr, "      Method: %d (%s)\n", wps_env->wps_method,
		_get_wps_str(_method_str, wps_env->wps_method));
	fprintf(stderr, "      Ifname: %s\n", wps_env->wps_ifname);
#ifdef WPS_UPNP_DEVICE
	fprintf(stderr, "        UUID: %s\n", wps_env->wps_uuid);
#endif // endif
	fprintf(stderr, "  WPS status: %d (%s)\n", wps_env->wps_proc_status,
		_get_wps_str(_status_str, wps_env->wps_proc_status));
#ifdef WPS_NFC_DEVICE
	fprintf(stderr, "  NFC status: %d (%s)\n", wps_env->wps_nfc_dm_status,
		NFC_STATUS_STR(wps_env->wps_nfc_dm_status));
	fprintf(stderr, "  NFC ercode: %d\n", wps_env->wps_nfc_err_code);
#endif // endif
}

static int
read_from_wps(int fd, char *databuf, int datalen)
{
	int n, max_fd = -1;
	fd_set fdvar;
	struct timeval timeout;
	int recvBytes;
	struct sockaddr_in addr;
	socklen_t size = sizeof(struct sockaddr);

	/* for safety, set databuf null terminate */
	if (databuf == NULL || datalen == 0)
		return -1;
	databuf[0] = 0;

	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	FD_ZERO(&fdvar);

	/* get ui fd */
	if (fd >= 0) {
		FD_SET(fd, &fdvar);
		max_fd = fd;
	}

	if (max_fd == -1) {
		fprintf(stderr, "wps ui utility: no fd set!\n");
		return -1;
	}

	n = select(max_fd + 1, &fdvar, NULL, NULL, &timeout);

	if (n < 0) {
		return -1;
	}

	if (n > 0) {
		if (fd >= 0) {
			if (FD_ISSET(fd, &fdvar)) {
				recvBytes = recvfrom(fd, databuf, datalen,
					0, (struct sockaddr *)&addr, &size);

				if (recvBytes == -1) {
					fprintf(stderr,
					"wps ui utility:recv failed, recvBytes = %d\n", recvBytes);
					return -1;
				}

				return recvBytes;
			}

			return 0;
		}
	}

	return -1;
}

static int
write_to_wps(int fd, char *cmd)
{
	int n;
	int len;
	struct sockaddr_in to;

	len = strlen(cmd)+1;

	/* open loopback socket to communicate with wps */
	memset(&to, 0, sizeof(to));
	to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	to.sin_family = AF_INET;
	to.sin_port = htons(WPS_UI_PORT);

	if ((n = sendto(fd, cmd, len, 0, (struct sockaddr *)&to,
		sizeof(struct sockaddr_in))) < 0) {
		perror("write_to_wps: sendto failed");
	}
	else {
		/* Sleep 100 ms to make sure WPS have received socket */
		usleep(100*1000);
	}

	return n;
}

static int
get_wps_env(char *uibuf, uint32 uilen)
{
	int wps_fd;
	int readBytes = 0;

	if ((wps_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	if (write_to_wps(wps_fd, "GET") >= 0)
		readBytes = read_from_wps(wps_fd, uibuf, uilen);

	close(wps_fd);

	return readBytes;
}

static void
set_wps_env(char *uibuf)
{
	int wps_fd = -1;

	if ((wps_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		fprintf(stderr, "Cannot open socket!\n");
		return;
	}

	if (write_to_wps(wps_fd, uibuf) < 0)
		fprintf(stderr, "Cannot set wps env!\n");

	close(wps_fd);
}

static void
usage(char *prog)
{
	fprintf(stderr,
		"usage:\n"
		"    %s\n"
		"        stop\n"
		"        status\n"
		"        msglevel [level]\n"
		"        configap [ifname]\n"
		"        addenrollee [ifname] pbc\n"
		"        addenrollee [ifname] sta_pin=[pin] [mac]\n"
		"        enroll [ifname] [pbc|pin] [ssid] [bssid] [wsec]\n"
		"        enroll [ifname] pbc\n"
		"        enroll [ifname] scan\n"
#ifdef WPS_NFC_DEVICE
		"        nfc [ifname] enroll [ssid] [bssid] [wsec]\n"
		"           STA do enrollment by writing STA's password token to tag\n"
		"        nfc [ifname] configap [ssid] [bssid] [wsec]\n"
		"           STA do registration by reading AP's password token from tag\n"
		"        nfc [ifname] wrpasswd\n"
		"           AP do enrollment by writing AP's password token to tag\n"
		"        nfc [ifname] rdpasswd\n"
		"           AP do enrollment by writing AP's password token to tag\n"
		"        nfc [ifname] wrconfig\n"
		"           AP/STA write configuration token to tag\n"
		"        nfc [ifname] rdconfig\n"
		"           AP/STA read configuration token from tag\n"
		"        nfc [ifname] chos\n"
		"           AP do connection handover as selector\n"
		"        nfc [ifname] chor [ssid] [bssid] [wsec]\n"
		"           STA do connection handover as requester\n"
		"        nfc [ifname] fmt\n"
		"           Format the tag\n"
#endif /* WPS_NFC_DEVICE */
		"",
		prog);
}

/*
 * Do wps commands
 */
static void
wps_stop()
{
	char uibuf[512];
	struct strbuf b;

	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "SET ");
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_STOP);
	str_bprintf(&b, "wps_action=%d ", WPS_UI_ACT_NONE);

	set_wps_env(uibuf);
}

static void
wps_msglevel(char **argv)
{
	int i;
	char uibuf[512];
	char *endptr = NULL;
	int bytes;
	unsigned int val = 0, last_val = 0;
	unsigned int msglevel = 0, msglevel_add = 0, msglevel_del = 0;
	struct strbuf b;

	bytes = get_wps_env(uibuf, sizeof(uibuf)-1);
	if (bytes <= 0)
		return;
	uibuf[bytes] = 0;

	memset(wps_env, 0, sizeof(*wps_env));
	parse_wps_env(uibuf);

	msglevel = wps_env->wps_msglevel;
	if (!*argv) {
		/* Show */
		fprintf(stderr, "0x%x ", msglevel);
		for (i = 0; (val = _msglevel_str[i].value); i++) {
			if ((msglevel & val) && (val != last_val))
				fprintf(stderr, " %s", _msglevel_str[i].string);
			last_val = val;
		}
		fprintf(stderr, "\n");
		return;
	}
	else {
		/* Set */
		while (*argv) {
			char *s = *argv;
			if (*s == '+' || *s == '-')
				s++;
			else
				msglevel_del = ~0; /* make the whole list absolute */
			val = strtoul(s, &endptr, 0);
			if (val == 0xFFFFFFFF) {
				fprintf(stderr, "Bits >32 are not supported\n");
				val = 1;
			}
			/* not an integer if not all the string was parsed by strtoul */
			if (*endptr != '\0') {
				for (i = 0; (val = _msglevel_str[i].value); i++)
					if (strcmp(_msglevel_str[i].string, s) == 0)
						break;
					if (!val)
						goto usage;
			}
			if (**argv == '-')
				msglevel_del |= val;
			else
				msglevel_add |= val;
			++argv;
		}
		msglevel &= ~msglevel_del;
		msglevel |= msglevel_add;

		str_binit(&b, uibuf, sizeof(uibuf));

		str_bprintf(&b, "SET ");
		str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_MSGLEVEL);
		str_bprintf(&b, "wps_msglevel=0x%x ", msglevel);
		set_wps_env(uibuf);
		return;
	}

usage:
	fprintf(stderr, "msg values may be a list of numbers or names from the following set.\n");
	fprintf(stderr, "Use a + or - prefix to make an incremental change.");

	for (i = 0; (val = _msglevel_str[i].value); i++) {
		if (val != last_val)
			fprintf(stderr, "\n0x%04x %s", val, _msglevel_str[i].string);
		else
			fprintf(stderr, ", %s", _msglevel_str[i].string);
		last_val = val;
	}
	fprintf(stderr, "\n");
	return;

}

static void
wps_status()
{
	char uibuf[512];
	int bytes;

	bytes = get_wps_env(uibuf, sizeof(uibuf)-1);
	if (bytes > 0) {
		uibuf[bytes] = 0;
		memset(wps_env, 0, sizeof(*wps_env));
		parse_wps_env(uibuf);
		print_wps_env();
	}

	return;
}

static void
wps_configap(char *ifname)
{
	char uibuf[512];
	struct strbuf b;

	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "SET ");
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_START);
	str_bprintf(&b, "wps_action=%d ", WPS_UI_ACT_CONFIGAP);
	str_bprintf(&b, "wps_ifname=%s ", ifname);

	set_wps_env(uibuf);
}

static int
wps_pin_check(char *pin_string)
{
	unsigned long PIN = strtoul(pin_string, NULL, 10);
	unsigned long int accum = 0;
	unsigned int len = strlen(pin_string);

	if (len != 4 && len != 8)
		return 	-1;

	if (len == 8) {
		accum += 3 * ((PIN / 10000000) % 10);
		accum += 1 * ((PIN / 1000000) % 10);
		accum += 3 * ((PIN / 100000) % 10);
		accum += 1 * ((PIN / 10000) % 10);
		accum += 3 * ((PIN / 1000) % 10);
		accum += 1 * ((PIN / 100) % 10);
		accum += 3 * ((PIN / 10) % 10);
		accum += 1 * ((PIN / 1) % 10);

		if ((accum % 10) == 0)
			return 0;
	}
	else if (len == 4)
		return 0;

	return -1;
}

static int
wps_addenrollee(char *ifname, char *method, char *autho_sta_mac)
{
	char uibuf[512];
	char pin[9] = {0};
	char *value;
	struct strbuf b;

	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "SET ");
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_START);
	str_bprintf(&b, "wps_action=%d ", WPS_UI_ACT_ADDENROLLEE);
	str_bprintf(&b, "wps_ifname=%s ", ifname);

	if (!strcmp(method, "pbc")) {
		strcpy(pin, "00000000");
		str_bprintf(&b, "wps_method=%d ", WPS_UI_METHOD_PBC);
	}
	else if (memcmp(method, "sta_pin=", 8) == 0) {
		value = method+8;
		if (wps_pin_check(value) != 0)
			return -1;

		strncpy(pin, value, 8);
		str_bprintf(&b, "wps_method=%d ", WPS_UI_METHOD_PIN);

		/* WSC 2.0, authorized mac */
		if (autho_sta_mac) {
			if (wps_hwaddr_check(autho_sta_mac) != 0)
				return -1;
			str_bprintf(&b, "wps_autho_sta_mac=%s ", autho_sta_mac);
		}
	}
	else {
		return -1;
	}

	str_bprintf(&b, "wps_sta_pin=%s ", pin);
	str_bprintf(&b, "wps_pbc_method=%d ", WPS_UI_PBC_SW);

	set_wps_env(uibuf);
	return 0;
}

static int
wps_ether_atoe(const char *a, unsigned char *e)
{
#define ETHER_ADDR_LEN	6

	char *c = (char *) a;
	int i = 0;

	memset(e, 0, ETHER_ADDR_LEN);
	for (;;) {
		e[i++] = (unsigned char) strtoul(c, &c, 16);
		if (!*c++ || i == ETHER_ADDR_LEN)
			break;
	}
	return (i == ETHER_ADDR_LEN);
}

static int
wps_hwaddr_check(char *value)
{
	unsigned char hwaddr[6];

	/* Check for bad, multicast, broadcast, or null address */
	if (!wps_ether_atoe(value, hwaddr) ||
	    (hwaddr[0] & 1) ||
	    (hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] & hwaddr[5]) == 0xff ||
	    (hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] | hwaddr[5]) == 0x00) {
		return -1;
	}

	return 0;
}

static int
wps_enrollee(char *ifname, char *method, char *ssid, char *bssid, char *wsc)
{
	char uibuf[512];
	int privacy;
	struct strbuf b;

	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "SET ");
	str_bprintf(&b, "wps_config_command=%d ", WPS_UI_CMD_START);
	str_bprintf(&b, "wps_action=%d ", WPS_UI_ACT_ENROLL);
	str_bprintf(&b, "wps_ifname=%s ", ifname);

	/* Scan/Method */
	if (!strcmp(method, "scan")) {
		str_bprintf(&b, "wps_enr_scan=%d ", 1);
		set_wps_env(uibuf);
		return 0;
	}

	else if (!strcmp(method, "pin"))
		str_bprintf(&b, "wps_method=%d ", WPS_UI_METHOD_PIN);
	else if (!strcmp(method, "pbc"))
		str_bprintf(&b, "wps_method=%d ", WPS_UI_METHOD_PBC);
	else
		return -1;

	str_bprintf(&b, "wps_pbc_method=%d ", ssid ? WPS_UI_PBC_SW : WPS_UI_PBC_HW);

	if (ssid) {
		/* ssid */
		if (strlen(ssid) == 0 || strlen(ssid) > 32)
			return -1;

		if (wps_hwaddr_check(bssid) != 0)
			return -1;

		privacy = atoi(wsc);
		if (privacy != 0 && privacy != 1)
			return -1;

		str_bprintf(&b, "wps_enr_ssid=%s ", ssid);
		str_bprintf(&b, "wps_enr_bssid=%s ", bssid);
		str_bprintf(&b, "wps_enr_wsec=%d ", privacy);
	}
	set_wps_env(uibuf);
	return 0;
}

#ifdef WPS_NFC_DEVICE
static int
wps_nfc_cmd(char *ifname, char *method, char **argv)
{
	char uibuf[512];
	struct strbuf b;

	str_binit(&b, uibuf, sizeof(uibuf));

	str_bprintf(&b, "SET ");

	if (!strcmp(method, "configap") || !strcmp(method, "enroll") || !strcmp(method, "chor")) {
		char *ssid = NULL, *bssid = NULL, *wsec = NULL;
		int not_chor = strcmp(method, "chor");

		ssid = *argv;
		++argv;
		if (!ssid) {
			if (not_chor)
				return -1;
			goto done;
		}

		bssid = *argv;
		++argv;
		if (!bssid) {
			if (not_chor)
				return -1;
			goto done;
		}

		wsec = *argv;
		++argv;
		if (!wsec) {
			if (not_chor)
				return -1;
			goto done;
		}

done:
		if (!strcmp(method, "configap")) {
			/* STA do registration by reading AP's password token from tag */
			str_bprintf(&b, "wps_stareg_ap_pin=\"NFC_PW\" ");
			str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_STA_CONFIGAP);
			str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_PW);
			str_bprintf(&b, "wps_enr_ssid=\"%s\" ", ssid);
			str_bprintf(&b, "wps_enr_bssid=\"%s\" ", bssid);
			str_bprintf(&b, "wps_enr_wsec=\"%s\" ", wsec);
			/* No specific credential here, use wps_ui internal */
			str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_RD_PW);
		}
		else if (!strcmp(method, "enroll")) {
			/* STA do enrollment by writing STA's password token to tag */
			str_bprintf(&b, "wps_sta_pin=\"NFC_PW\" ");
			str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_ENROLL);
			str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_PW);
			str_bprintf(&b, "wps_enr_ssid=\"%s\" ", ssid);
			str_bprintf(&b, "wps_enr_bssid=\"%s\" ", bssid);
			str_bprintf(&b, "wps_enr_wsec=\"%s\" ", wsec);
			/* No specific credential here, use wps_ui internal */
			str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_WR_PW);
		}
		else {
			/* STA do connection handover as requester */
			str_bprintf(&b, "wps_sta_pin=\"NFC_CHO\" ");
			str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_ENROLL);
			str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_CHO);
			str_bprintf(&b, "wps_enr_ssid=\"%s\" ", ssid ? ssid : "");
			str_bprintf(&b, "wps_enr_bssid=\"%s\" ", bssid ? bssid : "");
			str_bprintf(&b, "wps_enr_wsec=\"%s\" ", wsec ? wsec : "");
			/* Use wps_sta_pin=NFC_CHO to indicate STA as CHO-R */
			str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_HO_R);
		}
	}
	else if (!strcmp(method, "wrpasswd")) {
		/* AP do enrollment by writing AP's password token to tag */
		/* API PIN comes from NFC Password, leverage wps_stareg_ap_pin */
		str_bprintf(&b, "wps_stareg_ap_pin=\"NFC_PW\" ");
		str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_CONFIGAP);
		str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_PW);
		/* No specific credential here, use wps_ui internal */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_WR_PW);
	}
	else if (!strcmp(method, "rdpasswd")) {
		/* AP do registration by reading STA's password token from tag */
		/* Station PIN comes from NFC Password */
		str_bprintf(&b, "wps_sta_pin=\"NFC_PW\" ");
		str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_ADDENROLLEE);
		str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_PW);
		/* No specific credential here, use wps_ui internal */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_RD_PW);
	}
	else if (!strcmp(method, "wrconfig")) {
		/* AP/STA write configuration token to tag */
		/* No specific credential here, use wps_ui internal */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_WR_CFG);
	}
	else if (!strcmp(method, "rdconfig")) {
		/* AP/STA read configuration token from tag */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_RD_CFG);
	}
	else if (!strcmp(method, "chos")) {
		/* AP do connection handover as selector */
		str_bprintf(&b, "wps_sta_pin=\"NFC_CHO\" ");
		str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_ADDENROLLEE);
		str_bprintf(&b, "wps_method=\"%d\" ", WPS_UI_METHOD_NFC_CHO);
		/* No specific credential here, use wps_ui internal */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_HO_S);
	}
	else if (!strcmp(method, "fmt")) {
		/* Format tag */
		str_bprintf(&b, "wps_config_command=\"%d\" ", WPS_UI_CMD_NFC_FM);
		str_bprintf(&b, "wps_action=\"%d\" ", WPS_UI_ACT_NONE);
	}
	else
		return -1;

	str_bprintf(&b, "wps_ifname=\"%s\" ", ifname);
	set_wps_env(uibuf);

	return 0;
}
#endif /* WPS_NFC_DEVICE */

/*
 * Name        : main
 * Description : Main entry point for the WPS monitor
 * Arguments   : int argc, char *argv[] - command line parameters
 * Return type : int
 */
int
main(int argc, char *argv[])
{
	char *prog;
	char *command;
	char *ifname;
	char *method;
	char *ssid = 0;
	char *bssid = 0;
	char *wsec = 0;
	char *autho_sta_mac = 0; /* WSC 2.0 */

	/* Skip program name */
	--argc;
	prog = *argv;
	++argv;

	if (!*argv)
		goto out;

	/*
	  * Process wps command
	  */
	command = *argv;
	++argv;
	if (!command ||
		(strcmp(command, "stop") != 0 &&
		strcmp(command, "configap") != 0 &&
		strcmp(command, "addenrollee") != 0 &&
		strcmp(command, "enroll") != 0 &&
#ifdef WPS_NFC_DEVICE
		strcmp(command, "nfc") != 0 &&
#endif // endif
#ifdef _TUDEBUGTRACE
		strcmp(command, "msglevel") != 0 &&
#endif // endif
		strcmp(command, "status") != 0)) {
		/* Print usage and exit */
		goto out;
	}

	/* Do "wps stop" */
	if (!strcmp(command, "stop")) {
		wps_stop();
		return 0;
	}

	/* Do "wps msglevel" */
	if (!strcmp(command, "msglevel")) {
		wps_msglevel(argv);
		return 0;
	}

	/* Do "wps status" */
	if (!strcmp(command, "status")) {
		wps_status();
		return 0;
	}

	/*
	  * Process ifname
	  */
	ifname = *argv;
	++argv;
	if (!ifname)
		goto out;

	/* Do "wps configap ifname" */
	if (!strcmp(command, "configap")) {
		wps_configap(ifname);
		return 0;
	}

	/*
	  * Process method
	  */
	method = *argv;
	++argv;
	if (!method)
		goto out;

	/* Do "wps addenrollee [pbc|sta_pin=[pin]]" */
	if (!strcmp(command, "addenrollee")) {
		autho_sta_mac = *argv;
		++argv;
		if (wps_addenrollee(ifname, method, autho_sta_mac) != 0)
			goto out;

		return 0;
	}

#ifdef WPS_NFC_DEVICE
	/* Do "wps nfc [configap|enroll|wrpasswd|rdpasswd|wrconfig|rdconfig|chos|chor|fmt]" */
	if (!strcmp(command, "nfc")) {
		if (wps_nfc_cmd(ifname, method, argv) != 0)
			goto out;

		return 0;
	}
#endif /* WPS_NFC_DEVICE */

	/*
	  * Process enroll mode
	  */
	if (strcmp(method, "scan") != 0) {
		ssid = *argv;
		++argv;
		if (!ssid) {
			if (strcmp(method, "pbc") == 0)
				goto done; /* Hardware push button */
			else
				goto out;
		}

		bssid = *argv;
		++argv;
		if (!bssid)
			goto out;

		wsec = *argv;
		++argv;
		if (!wsec)
			goto out;
	}

done:
	if (!strcmp(command, "enroll")) {
		if (wps_enrollee(ifname, method, ssid, bssid, wsec) != 0)
			goto out;

		return 0;
	}

out:
	usage(prog);
	return -1;
}
