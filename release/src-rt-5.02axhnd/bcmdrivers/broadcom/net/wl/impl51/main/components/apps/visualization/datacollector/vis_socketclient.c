/*
 * Linux Visualization Data Collector client socket implementation
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
 * $Id: vis_socketclient.c 555336 2015-05-08 09:52:04Z $
 */
#include "vis_socketclient.h"

/* Given port and server address it connects to the server */
int
connect_to_server(uint32 nport, char *straddrs)
{
	struct sockaddr_in server_addr;
	int res, valopt;
	long arg;
	fd_set readfds;
	struct timeval tv;
	socklen_t lon;
	int sockfd;
	struct hostent *host;

	init_socket();

	sockfd = INVALID_SOCKET;
	memset(&server_addr, 0, sizeof(server_addr));

	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		print_error("socket");
		return INVALID_SOCKET;
	}

	VIS_SOCK("Server = %s\t port = %d\n", straddrs, nport);

	if ((host = gethostbyname(straddrs)) == NULL) {
		print_error("gethostbyname");
		return INVALID_SOCKET;
	}

	/* Set nonblock on the socket so we can timeout */
	if ((arg = fcntl(sockfd, F_GETFL, NULL)) < 0 ||
		fcntl(sockfd, F_SETFL, arg | O_NONBLOCK) < 0) {
			print_error("fcntl error");
			return INVALID_SOCKET;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(nport);
	server_addr.sin_addr = *((struct in_addr*)host -> h_addr);
	res = connect(sockfd, (struct sockaddr*)&server_addr,
		sizeof(struct sockaddr));

	if (res < 0) {
		if (errno == EINPROGRESS) {
			tv.tv_sec = 30;
			tv.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			if (select(sockfd+1, NULL, &readfds, NULL, &tv) > 0) {
				lon = sizeof(int);
				getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
					(void*)(&valopt), &lon);
				if (valopt) {
					VIS_SOCK("Error in connection() %d - %s\n",
						valopt, strerror(valopt));
					return INVALID_SOCKET;
				}
			} else {
				VIS_SOCK("Timeout or error() %d - %s\n",
					valopt, strerror(valopt));
				return INVALID_SOCKET;
			}
		} else {
			VIS_SOCK("Error connecting %d - %s\n",
				errno, strerror(errno));
			return INVALID_SOCKET;
		}
	}
	VIS_SOCK("Connection Successfull with server : %s\n", straddrs);

	return sockfd;
}
