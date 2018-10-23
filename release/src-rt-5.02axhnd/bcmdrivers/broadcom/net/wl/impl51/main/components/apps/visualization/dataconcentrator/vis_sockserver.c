/*
 * Linux Visualization Data Concentrator socket server implementation
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
 * $Id: vis_sockserver.c 544877 2015-03-30 08:47:06Z $
 */

#include "vis_sockserver.h"
#include "vis_struct.h"
#include "vis_xmlutility.h"

static int servsock_fd = INVALID_SOCKET;

#ifdef WIN32
static const char*
inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
	struct sockaddr_in in;

	memset(&in, 0, sizeof(in));
	in.sin_family = AF_INET;
	memcpy(&in.sin_addr, src, sizeof(struct in_addr));

	getnameinfo((struct sockaddr *)&in, sizeof(struct sockaddr_in),
		dst, cnt, NULL, 0, NI_NUMERICHOST);

	return dst;
}
#endif // endif

/* Client handler for thread */
void*
client_handler(void *pnewsock)
{
	/* Get the socket descriptor */
	int		sockfd = *(int*)pnewsock;
	int		sz = 0;
	unsigned char	*data = NULL;

	sz = on_receive(sockfd, &data);
	if (sz < 0 || data == NULL) {
		close_socket(&sockfd);
		free(pnewsock);
		return NULL;
	} else if (sz == 0) {
		close_socket(&sockfd);
		free(pnewsock);
		free(data);
		return NULL;
	}
	VIS_SOCK("\n\nSize of Request : %d Data Recieved : %s\n\n", sz, data);
	parse_response(sockfd, &data, sz);
	if (data != NULL) {
		free(data);
		data = NULL;
	}

	close_socket(&sockfd);
	free(pnewsock);
	pthread_exit(0);

	return NULL;
}

/* creates the server */
int
create_server(uint32 portno, int twocpuenabled)
{
	struct sockaddr_in serv_addr;
	struct sockaddr_storage		incoming_addr;
	char				portStr[10];
	int				ret = 0, new_fd, optval = 1;
	char				hostChar[INET6_ADDRSTRLEN];
	socklen_t			addrSize;

	servsock_fd = INVALID_SOCKET;

	init_socket();

	snprintf(portStr, 10, "%u", portno);

	if ((servsock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		print_error("socket");
		return ret;
	}

	setsockopt(servsock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	/* Initialize socket structure */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	/* If two CPU architecture is enabled in NVRAM config, then use INADDR_ANY
	 * else use the LOOPBACK IP address
	 */
	if (twocpuenabled) {
		serv_addr.sin_addr.s_addr = INADDR_ANY;
	} else {
		serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	}
	/* Assign port number */
	serv_addr.sin_port = htons(portno);

	if (bind(servsock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
		print_error("bind");
		close_socket(&servsock_fd);
		return ret;
	}

	if (listen(servsock_fd, 10) == -1) {
		print_error("bind");
		close_socket(&servsock_fd);
		return ret;
	}

	/* Main loop */
	while (1) {
		int *new_sock = NULL;

		addrSize	= sizeof(incoming_addr);
		new_fd		= INVALID_SOCKET;

		new_fd = accept(servsock_fd, (struct sockaddr *)&incoming_addr, &addrSize);
		if (new_fd == -1) {
			print_error("accept");
			close_server();
			return 0;
		}

		new_sock = (int*)malloc(sizeof(int) * 1);
		if (new_sock == NULL) {
			VIS_SOCK("Failed to allocate memory for new_sock : %d\n", 1);
			close_socket(&new_fd);
			break;
		}
		*new_sock = new_fd;

		if (inet_ntop(incoming_addr.ss_family,
			get_in_addr((struct sockaddr *)&incoming_addr),
			hostChar, sizeof(hostChar)) != NULL) {
			VIS_SOCK("Client : %s connected. SOCKFD : %d\n", hostChar, new_fd);
		} else {
			VIS_SOCK("Failed to get host address for : %d\n",
				new_fd);
			print_error("inet_ntop");
		}

#ifdef WIN32
		uintptr_t thread;

		thread = _beginthread(client_handler, 0, (void*)new_fd);
#else
		pthread_t thread;
		pthread_attr_t attr;
		int rc = 0;

		rc = pthread_attr_init(&attr);
		if (rc != 0) {
			print_error("pthread_attr_init");
			close_socket(&new_fd);
			free(new_sock);
			continue;
		}
		rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		if (rc != 0) {
			print_error("pthread_attr_setdetachstate");
			close_socket(&new_fd);
			free(new_sock);
			pthread_attr_destroy(&attr);
			continue;
		}

		if (pthread_create(&thread, &attr, client_handler, (void*)new_sock) != 0) {
			VIS_SOCK("Failed to create thread : %d\n", 1);
			print_error("pthread_create");
			close_socket(&new_fd);
			free(new_sock);
			pthread_attr_destroy(&attr);
			continue;
		}
		pthread_attr_destroy(&attr);
#endif /* WIN32 */
	}

	close_server();

	return 1;
}

/* Closes the server socket */
void
close_server()
{
	close_socket(&servsock_fd);
}
