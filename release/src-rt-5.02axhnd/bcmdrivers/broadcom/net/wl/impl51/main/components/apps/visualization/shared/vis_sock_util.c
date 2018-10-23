/*
 * Linux Visualization System common socket utility implementation
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
 * $Id: vis_sock_util.c 657201 2016-08-31 09:12:55Z $
 */

#include "vis_sock_util.h"

static int send_data(int sockfd, char *data, unsigned int len);
static int recv_data(int sockfd, unsigned char *read_buf, uint32 size);

/* Initializes the socket */
int
init_socket()
{
#ifdef WIN32
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		VIS_SOCK(stderr, "WSAStartup failed.\n");
		return -1;
	}
#endif /* WIN32 */

	return 1;
}

/* Closes the socket */
void
close_socket(int *sockfd)
{
	if (*sockfd == INVALID_SOCKET)
		return;

#ifdef WIN32
	closesocket(*sockfd);
#else
	close(*sockfd);
#endif /* WIN32 */
	*sockfd = INVALID_SOCKET;
}

/* Get the address */
void*
get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/* Adds the length to the first 4 bytes and sends it to the server */
int
add_length_and_send_data(int sockfd, const char *data, unsigned int len)
{
	int	ret = 0;
	char	*sdata = NULL;
	int	totlen = 4 + len;

	sdata = (char*)malloc(sizeof(char) * (len + 4));
	if (sdata == NULL) {
		VIS_SOCK("Failed to allocate sdata buffer of size : %d\n", (len+4));
		return -1;
	}

	memcpy(sdata, &totlen, 4);

	memcpy(sdata+4, data, len);

	ret = send_data(sockfd, sdata, totlen);

	free(sdata);

	return ret;
}

/* Sends the data to socket */
static int
send_data(int sockfd, char *data, unsigned int len)
{
	int	nret = 0, ret = 0;
	int	totalsize = len, totalsent = 0;

	while (totalsent < totalsize) {
		fd_set WriteFDs, ExceptFDs;
		struct timeval tv;

		FD_ZERO(&WriteFDs);
		FD_ZERO(&ExceptFDs);

		if (sockfd == INVALID_SOCKET) {
			VIS_SOCK("sockfd[%d]. Invalid socket\n", sockfd);
			goto error;
		}

		FD_SET(sockfd, &WriteFDs);

		tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
		tv.tv_usec = 0;
		if ((ret = select(sockfd+1, NULL, &WriteFDs, &ExceptFDs, &tv)) > 0) {
			if (FD_ISSET(sockfd, &WriteFDs)) {
				;
			} else {
				VIS_SOCK("sockfd[%d]. Exception occured\n", sockfd);
				goto error;
			}
		} else {
			if (ret == 0) {
				VIS_SOCK("sockfd[%d]. Timeout occured\n", sockfd);
			} else {
				VIS_SOCK("sockfd[%d]. Error : %s\n", sockfd, strerror(errno));
			}
			goto error;
		}

		nret = send(sockfd, &(data[totalsent]), len, 0);
		if (nret < 0) {
			VIS_SOCK("send error is = %s\n", strerror(errno));
			goto error;
		}
		totalsent += nret;
		len -= nret;
		nret = 0;
	}

	return totalsent;

error:
	return INVALID_SOCKET;
}

/* return value contains the number of bytes read or -1 for error
 * free the buffer after reading the data
 */
int
on_receive(int sockfd, unsigned char **data)
{
	int		sz, size = 0;
	unsigned char	szbuff[VIS_LEN_FIELD_SZ + 1] = {0};
	unsigned char	*read_buf = NULL;

	sz = recv_data(sockfd, szbuff, VIS_LEN_FIELD_SZ);
	if (sz <= 0) {
		VIS_SOCK("Read Failed\n");
		return INVALID_SOCKET;
	}

	if (sz >= VIS_LEN_FIELD_SZ) {
		memcpy(&size, szbuff, VIS_LEN_FIELD_SZ);
		VIS_SOCK("Total size : %d Total Read : %d\n", size, sz);
	} else {
		VIS_SOCK("Doesn't contain any size\n");
		return -1;
	}
	if (size <= VIS_LEN_FIELD_SZ) {
		VIS_SOCK("Size of less than %d indicates there is no data\n", VIS_LEN_FIELD_SZ);
		return -1;
	}

	read_buf = (unsigned char *)malloc(sizeof(unsigned char) * ((size-VIS_LEN_FIELD_SZ) + 1));
	if (read_buf == NULL) {
		VIS_SOCK("Failed to allocate read_buf buffer of size : %d\n",
			(size-VIS_LEN_FIELD_SZ));
		return -1;
	}

	sz = recv_data(sockfd, read_buf, (size-VIS_LEN_FIELD_SZ));
	if (sz <= 0) {
		if (read_buf != NULL) {
			free(read_buf);
			read_buf = NULL;
		}
		return INVALID_SOCKET;
	}

	*data = read_buf;

	return sz;
}

/* to recieve the 'size' number of data */
static int
recv_data(int sockfd, unsigned char *read_buf, uint32 size)
{
	uint32		nbytes, totalread = 0, ret = 0;
	struct timeval	tv;
	fd_set		ReadFDs, ExceptFDs;

	while (totalread < size) {
		FD_ZERO(&ReadFDs);
		FD_ZERO(&ExceptFDs);
		FD_SET(sockfd, &ReadFDs);
		FD_SET(sockfd, &ExceptFDs);
		tv.tv_sec = VIS_SOCKET_WAIT_TIMEOUT;
		tv.tv_usec = 0;

		if ((ret = select(sockfd+1, &ReadFDs, NULL, &ExceptFDs, &tv)) > 0) {
			if (FD_ISSET(sockfd, &ReadFDs)) {
				/* fprintf(stdout, "SOCKET : Data is ready to read\n"); */;
			} else {
				VIS_SOCK("sockfd[%d]. Exception Occured\n", sockfd);
				goto error;
			}
		} else {
			if (ret == 0) {
				VIS_SOCK("sockfd[%d]. Timeout occured\n", sockfd);
			} else {
				VIS_SOCK("sockfd[%d]. Error : %s\n", sockfd, strerror(errno));
			}

			goto error;
		}

		nbytes = read(sockfd, &(read_buf[totalread]), size);
		VIS_SOCK("sockfd[%d]. Read bytes  = %d\n\n", sockfd, nbytes);

		if (nbytes <= 0) {
			VIS_SOCK("sockfd[%d]. Read Error : %s\n", sockfd, strerror(errno));
			goto error;
		}

		totalread += nbytes;
	}

	read_buf[totalread] = '\0';

	return totalread;

error:
	return INVALID_SOCKET;
}

/* Gets the socket error code */
int
get_socket_error_code()
{
#ifdef WIN32
	return WSAGetLastError();
#else
	return errno;
#endif /* WIN32 */
}

/* prints the error to the console */
void
print_error(char *str)
{
#ifdef WIN32
	VIS_SOCK("Error in %s is : %d\n", str, WSAGetLastError());
#else
	VIS_SOCK("Error in %s is : %s\n", str, strerror(errno));
#endif /* WIN32 */
}
