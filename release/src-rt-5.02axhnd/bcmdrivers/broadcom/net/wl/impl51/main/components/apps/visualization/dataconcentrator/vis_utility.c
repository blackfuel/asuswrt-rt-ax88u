/*
 * Linux Visualization Data Concentrator some utility functions implementation
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
 * $Id: vis_utility.c 568904 2015-07-06 11:04:01Z $
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "vis_utility.h"
#include "vis_db_defines.h"
#include "vis_struct.h"

/* Utility function to remove white spaces from the string */
char*
remove_spaces(char *src, char *dst)
{
	int i = 0, j = 0;

	while (src[i] != '\0') {
		if (src[i] != ' ')
			dst[j++] = src[i];
		i++;
	}
	dst[j] = '\0';

	return dst;
}

/* to fill the graph details struct from the iovar struct */
void
fill_graph_details_struct(graphdetails_t *graphdet, iovar_handler_t *handle)
{
	char tablename[50];

	/* first find out the tab name */
	if (strcmp(handle->name, IOVAR_NAME_CHANNELTSTATS) == 0) {
		snprintf(graphdet->tab, sizeof(graphdet->tab), "%s", TAB_CHANNEL_STATISTICS);
	} else {
		strncpy(graphdet->tab, TAB_METRICS, sizeof(graphdet->tab)-1);
	}

	/* find the table name */
	/* If the plotwith is None then the table name is name in handle struct */
	/* else the table name is plotwith name */
	if (strcmp(handle->plotwith, "None") == 0) {
		snprintf(tablename, sizeof(tablename), "%s", handle->name);
	} else {
		snprintf(tablename, sizeof(tablename), "%s", handle->plotwith);
	}
	/* now remove the space */
	remove_spaces(tablename, graphdet->table);
}

/* Creates the directory */
void
create_directory(char *dir)
{
	mkdir(dir, 0700);
}

/* Replace plus sign with spaces */
void
replace_plus_with_space(char *str)
{
	int i;

	for (i = 0; i < strlen(str); i++) {
		if (str[i] == '+')
			str[i] = ' ';
	}
}

/* Replace special characters from string */
void
replace_special_characters(char *str, char replacechar)
{
	int i, j = 0;

	for (i = 0; i < strlen(str); i++, j++) {
		if (str[i] == '%') {
			i += 2;
			str[j] = replacechar;
		}
		else
			str[j] = str[i];
	}
	str[j] = '\0';
}

/* Get the channel string from the control and bandwidth */
int
get_channel_str(int centerch, int bw, int ctrlch, char *channel, size_t n)
{
	int i = 0;

	/* Default is ctrl for 20 MHZ */
	if (bw == 20) {
		snprintf(channel, n, "%d", ctrlch);
		return TRUE;
	} else if (bw == 40) {
		if (ctrlch <= 14) {
			if (centerch < ctrlch) {
				snprintf(channel, n, "%d%s", ctrlch, WF_40MHZ_U);
				return TRUE;
			} else {
				snprintf(channel, n, "%d%s", ctrlch, WF_40MHZ_L);
				return TRUE;
			}
		} else {
			for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
				if (wf_5g_40m_chans[i] == (ctrlch+2)) {
					snprintf(channel, n, "%d%s", ctrlch, WF_40MHZ_L);
					return TRUE;
				} else if (wf_5g_40m_chans[i] == (ctrlch-2)) {
					snprintf(channel, n, "%d%s", ctrlch, WF_40MHZ_U);
					return TRUE;
				}
			}
		}
	} else if (bw == 80) {
		/* As 80 MHz channels are represented as control channel/80 (ex : 36/80)
		 * Just append 80 string to control channel
		 */
		snprintf(channel, n, "%d%s", ctrlch, WF_80MHZ_L);
		return TRUE;
	}
	else if (bw == 160) {
		snprintf(channel, n, "%d%s", ctrlch, WF_160MHZ_L);
		return TRUE;
	}

	return FALSE;
}

/* To check whether the channel is valid 40 MHz channel in 5g or not */
int
is_valid_5g_40m_channel(int channel, int *upperch)
{
	int i;

	for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
		if (wf_5g_40m_chans[i] == (channel+2)) {
			*upperch = channel+4;
			return TRUE;
		}
	}
	return FALSE;
}

/* To check whether the channel is valid 80 MHz channel in 5g or not */
int
is_valid_5g_80m_channel(int channel, int *upperch)
{
	int i;

	for (i = 0; i < WF_NUM_5G_80M_CHANS; i++) {
		if (wf_5g_80m_chans[i] == (channel+6)) {
			*upperch = channel+12;
			return TRUE;
		}
	}
	return FALSE;
}

/* Get lower sideband channel given the control channel and bandwidth */
void
get_lower_sideband_channel(int ctrl, int *sidechnl, int bw)
{
	int i = 0;

	*sidechnl = 0;

	if (bw == 20) {
		*sidechnl = ctrl;
	} else if (bw == 40) {
		if (ctrl <= 14) {
			*sidechnl = ctrl;
		}
		else {
			for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
				if (wf_5g_40m_chans[i] == (ctrl+2)) {
					*sidechnl = ctrl;
					break;
				} else if (wf_5g_40m_chans[i] == (ctrl-2)) {
					*sidechnl = ctrl-4;
					break;
				}
			}
		}
	} else if (bw == 80) {
		for (i = 0; i < WF_NUM_5G_80M_CHANS; i++) {
			int k, sub = 6, found = 0;

			for (k = 0; k < 4; k++) {
				if (wf_5g_80m_chans[i] == (ctrl+sub)) {
					*sidechnl = ctrl+sub-6;
					found = 1;
					break;
				}
				sub -= 4;
			}
			if (found == 1)
				break;
		}
	} else if (bw == 160) {
		for (i = 0; i < WF_NUM_5G_160M_CHANS; i++) {
			int k, sub = 14, found = 0;

			for (k = 0; k < 8; k++) {
				if (wf_5g_160m_chans[i] == (ctrl+sub)) {
					*sidechnl = ctrl+sub-14;
					found = 1;
					break;
				}
				sub += 4;
			}
			if (found == 1)
				break;
		}
	}
}

/* Get the center channel given the control channel */
void
get_center_channel(int ctrl, int *centerchnl, int bw)
{
	int i = 0;

	*centerchnl = 0;

	if (bw == 20) {
		*centerchnl = ctrl;
	} else if (bw == 40) {
		if (ctrl <= 14) {
			*centerchnl = ctrl;
		}
		else {
			for (i = 0; i < WF_NUM_5G_40M_CHANS; i++) {
				if (wf_5g_40m_chans[i] == (ctrl+2)) {
					*centerchnl = ctrl+2;
					break;
				} else if (wf_5g_40m_chans[i] == (ctrl-2)) {
					*centerchnl = ctrl-2;
					break;
				}
			}
		}
	} else if (bw == 80) {
		for (i = 0; i < WF_NUM_5G_80M_CHANS; i++) {
			int k, sub = 6, found = 0;

			for (k = 0; k < 4; k++) {
				if (wf_5g_80m_chans[i] == (ctrl+sub)) {
					*centerchnl = ctrl+sub;
					found = 1;
					break;
				}
				sub -= 4;
			}
			if (found == 1)
				break;
		}
	} else if (bw == 160) {
		for (i = 0; i < WF_NUM_5G_160M_CHANS; i++) {
			int k, sub = 14, found = 0;

			for (k = 0; k < 8; k++) {
				if (wf_5g_160m_chans[i] == (ctrl+sub)) {
					*centerchnl = ctrl+sub;
					found = 1;
					break;
				}
				sub += 4;
			}
			if (found == 1)
				break;
		}
	}
}

/* Read the filecontent and store it in data variable. Return the size of the file */
unsigned long
read_file_content(char *filename, char **data)
{
	FILE *file;
	char *buffer;
	unsigned long nfilelen = 0;

	/* Open the file */
	file = fopen(filename, "rb");
	if (!file) {
		return nfilelen; /* Unable to open the file so return zero as length */
	}

	/* Get file length */
	fseek(file, 0, SEEK_END);
	nfilelen = ftell(file);
	fseek(file, 0, SEEK_SET);

	/* Allocate memory */
	buffer = (char *)malloc(nfilelen);
	if (!buffer) {
		fclose(file);
		return 0;
	}

	/* Read file contents into buffer */
	fread(buffer, nfilelen, 1, file);
	fclose(file);

	*data = buffer;

	return nfilelen;
}

/* Remove the trailing 'chartoremove' character from the string */
char*
remove_trailing_character(char *str, char chartoremove)
{
	char *end;

	if (!str)
		return str;
	end = str + strlen(str) - 1;
	while ((end > str) && (*end == chartoremove))
		end--;

	/* Write new null terminator */
	*(end + 1) = '\0';

	return str;
}
