/*
 * This file is part of ioctl_mtd strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
# include "mtd-abi.h"
#else
# include <mtd/mtd-abi.h>
#endif

static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

#define TEST_NULL_ARG(cmd) \
	do { \
		ioctl(-1, cmd, 0); \
		if (_IOC_DIR(cmd) == _IOC_WRITE) \
			printf("ioctl(-1, MIXER_WRITE(%u) or %s, NULL)" \
			       " = -1 EBADF (%m)\n", \
			       (unsigned int) _IOC_NR(cmd), #cmd); \
		else if (_IOC_DIR(cmd) == _IOC_READ) \
			printf("ioctl(-1, MIXER_READ(%u) or %s, NULL)" \
			       " = -1 EBADF (%m)\n", \
			       (unsigned int) _IOC_NR(cmd), #cmd); \
		else \
			printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", #cmd); \
		} while (0)

#define TEST_erase_info_user(cmd, eiu) \
	ioctl(-1, cmd, eiu); \
	printf("ioctl(-1, MIXER_%s(%u) or %s, {start=%#x, length=%#x})" \
	       " = -1 EBADF (%m)\n", \
	       (_IOC_DIR(cmd) == _IOC_READ) ? "READ" : "WRITE", \
	       (unsigned int) _IOC_NR(cmd), #cmd, \
	       eiu->start, eiu->length)

int
main(void)
{
	TEST_NULL_ARG(ECCGETLAYOUT);
	TEST_NULL_ARG(ECCGETSTATS);
	TEST_NULL_ARG(MEMERASE);
	TEST_NULL_ARG(MEMERASE64);
	TEST_NULL_ARG(MEMGETBADBLOCK);
	TEST_NULL_ARG(MEMGETINFO);
	TEST_NULL_ARG(MEMGETOOBSEL);
	TEST_NULL_ARG(MEMGETREGIONCOUNT);
	TEST_NULL_ARG(MEMISLOCKED);
	TEST_NULL_ARG(MEMLOCK);
	TEST_NULL_ARG(MEMREADOOB);
	TEST_NULL_ARG(MEMREADOOB64);
	TEST_NULL_ARG(MEMSETBADBLOCK);
	TEST_NULL_ARG(MEMUNLOCK);
	TEST_NULL_ARG(MEMWRITE);
	TEST_NULL_ARG(MEMWRITEOOB);
	TEST_NULL_ARG(MEMWRITEOOB64);
	TEST_NULL_ARG(OTPGETREGIONCOUNT);
	TEST_NULL_ARG(OTPGETREGIONINFO);
	TEST_NULL_ARG(OTPLOCK);
	TEST_NULL_ARG(OTPSELECT);

	ioctl(-1, MTDFILEMODE, MTD_FILE_MODE_NORMAL);
	printf("ioctl(-1, MTDFILEMODE, MTD_FILE_MODE_NORMAL) = -1 EBADF (%m)\n");

	TAIL_ALLOC_OBJECT_CONST_PTR(int, opt);
	*opt = MTD_OTP_OFF;
	ioctl(-1, OTPSELECT, opt);
	printf("ioctl(-1, MIXER_READ(%u) or OTPSELECT, [MTD_OTP_OFF])"
	       " = -1 EBADF (%m)\n", (unsigned int) _IOC_NR(OTPSELECT));

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, v64);
	fill_memory(v64, sizeof(*v64));

	ioctl(-1, MEMGETBADBLOCK, v64);
	printf("ioctl(-1, MIXER_WRITE(%u) or MEMGETBADBLOCK, [%" PRIu64 "])"
	       " = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(MEMGETBADBLOCK), *v64);

	ioctl(-1, MEMSETBADBLOCK, v64);
	printf("ioctl(-1, MIXER_WRITE(%u) or MEMSETBADBLOCK, [%" PRIu64 "])"
	       " = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(MEMSETBADBLOCK), *v64);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct region_info_user, riu);
	fill_memory(riu, sizeof(*riu));
	ioctl(-1, MEMGETREGIONINFO, riu);
	printf("ioctl(-1, %s, {regionindex=%#x}) = -1 EBADF (%m)\n",
	       "MEMGETREGIONINFO"
#ifdef __i386__
	       " or MTRRIOC_GET_PAGE_ENTRY"
#endif
	       , riu->regionindex);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct erase_info_user, eiu);
	fill_memory(eiu, sizeof(*eiu));

	TEST_erase_info_user(MEMERASE, eiu);
	TEST_erase_info_user(MEMLOCK, eiu);
	TEST_erase_info_user(MEMUNLOCK, eiu);
	TEST_erase_info_user(MEMISLOCKED, eiu);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct erase_info_user64, eiu64);
	fill_memory(eiu64, sizeof(*eiu64));
	ioctl(-1, MEMERASE64, eiu64);
	printf("ioctl(-1, MIXER_WRITE(%u) or %s, {start=%#llx, length=%#llx})"
	       " = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(MEMERASE64), "MEMERASE64",
	       (unsigned long long) eiu64->start,
	       (unsigned long long) eiu64->length);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_oob_buf, oob);
	fill_memory(oob, sizeof(*oob));

	ioctl(-1, MEMWRITEOOB, oob);
	printf("ioctl(-1, MEMWRITEOOB, {start=%#x, length=%#x, ptr=%p})"
	       " = -1 EBADF (%m)\n", oob->start, oob->length, oob->ptr);

	ioctl(-1, MEMREADOOB, oob);
	printf("ioctl(-1, MEMREADOOB, {start=%#x, length=%#x, ptr=%p})"
	       " = -1 EBADF (%m)\n", oob->start, oob->length, oob->ptr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_oob_buf64, oob64);
	fill_memory(oob64, sizeof(*oob64));

	ioctl(-1, MEMWRITEOOB64, oob64);
	printf("ioctl(-1, MEMWRITEOOB64"
	       ", {start=%#llx, length=%#x, usr_ptr=%#llx}) = -1 EBADF (%m)\n",
	       (unsigned long long) oob64->start, oob64->length,
	       (unsigned long long) oob64->usr_ptr);

	ioctl(-1, MEMREADOOB64, oob64);
	printf("ioctl(-1, MEMREADOOB64"
	       ", {start=%#llx, length=%#x, usr_ptr=%#llx}) = -1 EBADF (%m)\n",
	       (unsigned long long) oob64->start, oob64->length,
	       (unsigned long long) oob64->usr_ptr);


	TAIL_ALLOC_OBJECT_CONST_PTR(struct otp_info, oi);
	fill_memory(oi, sizeof(*oi));
	ioctl(-1, OTPLOCK, oi);
	printf("ioctl(-1, MIXER_READ(%u) or OTPLOCK"
	       ", {start=%#x, length=%#x, locked=%u}) = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(OTPLOCK),  oi->start, oi->length, oi->locked);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct mtd_write_req, wr);
	fill_memory(wr, sizeof(*wr));
	wr->mode = MTD_OPS_PLACE_OOB;
	ioctl(-1, MEMWRITE, wr);
	printf("ioctl(-1, MEMWRITE, {start=%#llx, len=%#llx, ooblen=%#llx"
	       ", usr_data=%#llx, usr_oob=%#llx, mode=MTD_OPS_PLACE_OOB})"
	       " = -1 EBADF (%m)\n",
	       (unsigned long long) wr->start,
	       (unsigned long long) wr->len,
	       (unsigned long long) wr->ooblen,
	       (unsigned long long) wr->usr_data,
	       (unsigned long long) wr->usr_oob);

	ioctl(-1, _IOC(_IOC_READ|_IOC_WRITE, 0x4d, 0xfe, 0xff), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n",
	       "_IOC(_IOC_READ|_IOC_WRITE, 0x4d, 0xfe, 0xff)", lmagic);

	puts("+++ exited with 0 +++");
	return 0;
}
