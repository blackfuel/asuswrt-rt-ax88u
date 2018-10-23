#
# Assorted cryptographic algorithms
#
# Copyright 2018 Broadcom
#
# This program is the proprietary software of Broadcom and/or
# its licensors, and may only be used, duplicated, modified or distributed
# pursuant to the terms and conditions of a separate, written license
# agreement executed between you and Broadcom (an "Authorized License").
# Except as set forth in an Authorized License, Broadcom grants no license
# (express or implied), right to use, or waiver of any kind with respect to
# the Software, and Broadcom expressly reserves all rights in and to the
# Software and all intellectual property rights therein.  IF YOU HAVE NO
# AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
# WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
# THE SOFTWARE.
#
# Except as expressly set forth in the Authorized License,
#
# 1. This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof, and to
# use this information only in connection with your use of Broadcom
# integrated circuit products.
#
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
# "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
# OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
#
# 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
# SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
# IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
# IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
# ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
# OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
# NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
#
#
# <<Broadcom-WL-IPTag/Proprietary:>>
#
# $Id: Makefile 679390 2017-01-14 00:13:47Z $
#

SRCBASE	:= ..

CC	:= gcc
CFLAGS	+= -I. -I$(SRCBASE)/include -Wall
CFLAGS	+= -g -DDEBUG -ffunction-sections
#CFLAGS	+= -O2
CFLAGS	+= -Wall -Werror

#CC	:= cl
#CFLAGS	+= -I. -I../include -Ic:/tools/msdev/2600ddk/inc/crt
#LDFLAGS	+= -link -LIBPATH:c:/tools/msdev/2600ddk/lib/wxp/i386

PROGS	:= aes aeskeywrap des dh passhash prf rc4 sha1 tkhash tkhash_shared_sbox sha256

all: $(PROGS)

clean:
	rm -f *.o *.obj $(PROGS) *.exe

AESFLAGS := -DBCMAES_TEST -DBCMAES_TEST_STANDALONE -DBCMAES_GENTABLE
aes:	aes.c rijndael-alg-fst.c
	$(CC) $(CFLAGS) $(AESFLAGS) -o $@ $^ $(LDFLAGS)

aeskeywrap:	aeskeywrap.c aes.c rijndael-alg-fst.c
	$(CC) $(CFLAGS) -DBCMAESKEYWRAP_TEST -o $@ $^ $(LDFLAGS)

des:	des.c
	$(CC) $(CFLAGS) -DDES_TEST_STANDALONE -o $@ $^ $(LDFLAGS)

passhash:	passhash.c sha1.c prf.c
	$(CC) $(CFLAGS) -DBCMPASSHASH_TEST -o $@ $^ $(LDFLAGS)

prf:	prf.c hmac.c sha1.c md5.c
	$(CC) $(CFLAGS) -DBCMPRF_TEST -DBCMCCX -o $@ $^ $(LDFLAGS)

rc4:	rc4.c
	$(CC) $(CFLAGS) -DBCMRC4_TEST -o $@ $^ $(LDFLAGS)

sha1:	sha1.c
	$(CC) $(CFLAGS) -DBCMSHA1_TEST -o $@ $^ $(LDFLAGS)

sha256:	sha256.c
	$(CC) $(CFLAGS) -DBCMSHA256_TEST -o $@ $^ $(LDFLAGS)

tkhash:	tkhash.c
	$(CC) $(CFLAGS) -DBCMTKHASH_TEST -o $@ $^ $(LDFLAGS)

tkhash_shared_sbox:	tkhash.c rijndael-alg-fst.c
	$(CC) $(CFLAGS) -DBCMTKHASH_TEST -DSHARE_RIJNDAEL_SBOX -o $@ $^ $(LDFLAGS)

dh:	dh.c bn.c random.c
	$(CC) $(CFLAGS) -m32 -DBCMDH_TEST -o $@ $^ $(LDFLAGS)

install:
	install -d $(INSTALLDIR)/usr/sbin
	install $(PROGS) $(INSTALLDIR)/usr/sbin
	$(STRIP) $(foreach file,$(PROGS),$(INSTALLDIR)/usr/sbin/$(file))

test_all: $(PROGS)
	@for i in $(PROGS); do \
		echo ""; \
		echo "==> Testing $$i"; \
		$$i || exit 127; \
	done
