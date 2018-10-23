/**
 * @file
 * @brief
 *
 * Copyright 2017 Broadcom
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
 * $Id: $
 */
/* This file generates {core_name}revs.txt file while has the list of supported revs
 *	[similar to d11shm counterpart]
 */

#include <stdio.h>

#ifndef D11CONF
#define D11CONF 0
#endif // endif

#ifndef D11CONF2
#define D11CONF2 0
#endif // endif

#ifndef D11CONF3
#define D11CONF3 0
#endif // endif

#ifndef D11CONF4
#define D11CONF4 0
#endif // endif

#ifndef D11CONF5
#define D11CONF5 0
#endif // endif

#ifndef BCMPCIEREV
#define BCMPCIEREV 24
#endif // endif

static void
find_d11rev(FILE *fp, int *d11rev, int d11conf)
{
	int i;

	for (i = 0; i <= 31; i++, (*d11rev)++) {
		if (d11conf & (1 << i)) {
			fprintf(fp, "Rev ID: %d\n", (*d11rev));
			printf("%d ", (*d11rev));
		}
	}
}

int autoregs_strcmp(const char *s1, const char *s2) {
	const unsigned char *p1 = (const unsigned char *)s1;
	const unsigned char *p2 = (const unsigned char *)s2;

	while (*p1 != '\0') {
		if (*p2 == '\0') {
			return 1;
		}

		if (*p2 > *p1) {
			return -1;
		}

		if (*p1 > *p2) {
			return 1;
		}

		p1++;
		p2++;
	}

	if (*p2 != '\0') {
		return -1;
	}

	return 0;
}

int
main(int argc, char *argv[])
{
	int rev_id = 0;
	FILE *fp;

	if (!autoregs_strcmp(argv[1], "d11")) {
		fp = fopen("autoregs_d11revs_dbg.txt", "w");

		fprintf(fp, "D11CONF: 0x%08x\n", D11CONF);
		fprintf(fp, "D11CONF2: 0x%08x\n", D11CONF2);
		fprintf(fp, "D11CONF3: 0x%08x\n\n", D11CONF3);
		fprintf(fp, "D11CONF4: 0x%08x\n\n", D11CONF4);
		fprintf(fp, "D11CONF5: 0x%08x\n\n", D11CONF5);

		find_d11rev(fp, &rev_id, D11CONF);
		find_d11rev(fp, &rev_id, D11CONF2);
		find_d11rev(fp, &rev_id, D11CONF3);
		find_d11rev(fp, &rev_id, D11CONF4);
		find_d11rev(fp, &rev_id, D11CONF5);

		fclose(fp);
	} else if (!autoregs_strcmp(argv[1], "pcie")) {
		printf("%d ", BCMPCIEREV);
	}

	return 0;
}
