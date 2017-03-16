/*
 * Check decoding of SCSI ioctl commands.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_SCSI_SG_H

# include <stdio.h>
# include <sys/ioctl.h>
# include <scsi/sg.h>
# include "xlat/scsi_sg_commands.h"

# define TEST_NO_ARG(cmd)							\
	do {									\
		ioctl(-1, cmd, 0xdeadbeef);					\
		printf("ioctl(-1, %s) = -1 EBADF (%m)\n", #cmd);		\
	} while (0)

# define TEST_NULL_ARG(cmd)							\
	do {									\
		ioctl(-1, cmd, 0);						\
		printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", #cmd);		\
	} while (0)

# define TEST_TAKES_INT_BY_VAL(cmd, val)					\
	do {									\
		ioctl(-1, cmd, val);						\
		printf("ioctl(-1, %s, %#x) = -1 EBADF (%m)\n", #cmd, val);	\
	} while (0)

# define TEST_TAKES_INT_BY_PTR(cmd, pint)					\
	do {									\
		ioctl(-1, cmd, pint);						\
		printf("ioctl(-1, %s, [%d]) = -1 EBADF (%m)\n", #cmd, *(pint));	\
	} while (0)

# define TEST_RETURNS_INT_BY_PTR(cmd, pint)					\
	do {									\
		ioctl(-1, cmd, pint);						\
		printf("ioctl(-1, %s, %p) = -1 EBADF (%m)\n", #cmd, pint);	\
	} while (0)

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(int, pint);
	*pint = (int) 0xfacefeed;

	TEST_NO_ARG(SG_GET_TIMEOUT);

	TEST_NULL_ARG(SG_SET_TIMEOUT);
	TEST_NULL_ARG(SG_EMULATED_HOST);
	TEST_NULL_ARG(SG_GET_TRANSFORM);
	TEST_NULL_ARG(SG_GET_COMMAND_Q);
	TEST_NULL_ARG(SG_SET_COMMAND_Q);
	TEST_NULL_ARG(SG_GET_RESERVED_SIZE);
	TEST_NULL_ARG(SG_SET_RESERVED_SIZE);
	TEST_NULL_ARG(SG_GET_SCSI_ID);
	TEST_NULL_ARG(SG_SET_FORCE_LOW_DMA);
	TEST_NULL_ARG(SG_GET_LOW_DMA);
	TEST_NULL_ARG(SG_SET_FORCE_PACK_ID);
	TEST_NULL_ARG(SG_GET_PACK_ID);
	TEST_NULL_ARG(SG_GET_NUM_WAITING);
	TEST_NULL_ARG(SG_SET_DEBUG);
	TEST_NULL_ARG(SG_GET_SG_TABLESIZE);
	TEST_NULL_ARG(SG_GET_VERSION_NUM);
	TEST_NULL_ARG(SG_NEXT_CMD_LEN);
	TEST_NULL_ARG(SG_SCSI_RESET);
	TEST_NULL_ARG(SG_IO);
	TEST_NULL_ARG(SG_GET_REQUEST_TABLE);
	TEST_NULL_ARG(SG_SET_KEEP_ORPHAN);
	TEST_NULL_ARG(SG_GET_KEEP_ORPHAN);
	TEST_NULL_ARG(SG_GET_ACCESS_COUNT);

	TEST_TAKES_INT_BY_VAL(SG_SET_TRANSFORM, 0);
	TEST_TAKES_INT_BY_VAL(SG_SET_TRANSFORM, *pint);

	TEST_TAKES_INT_BY_PTR(SG_NEXT_CMD_LEN, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_COMMAND_Q, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_DEBUG, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_FORCE_LOW_DMA, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_FORCE_PACK_ID, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_KEEP_ORPHAN, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_RESERVED_SIZE, pint);
	TEST_TAKES_INT_BY_PTR(SG_SET_TIMEOUT, pint);

	TEST_RETURNS_INT_BY_PTR(SG_EMULATED_HOST, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_ACCESS_COUNT, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_COMMAND_Q, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_KEEP_ORPHAN, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_LOW_DMA, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_NUM_WAITING, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_PACK_ID, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_RESERVED_SIZE, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_SG_TABLESIZE, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_TRANSFORM, pint);
	TEST_RETURNS_INT_BY_PTR(SG_GET_VERSION_NUM, pint);

	ioctl(-1, SG_SCSI_RESET, pint);
	printf("ioctl(-1, %s, [%#x /* %s_??? */]) = -1 EBADF (%m)\n",
	       "SG_SCSI_RESET", *pint, "SG_SCSI_RESET");

	*pint = 0x100;
	ioctl(-1, SG_SCSI_RESET, pint);
	printf("ioctl(-1, %s, [SG_SCSI_RESET_NO_ESCALATE|SG_SCSI_RESET_NOTHING])"
	       " = -1 EBADF (%m)\n", "SG_SCSI_RESET");

	*pint = 1;
	ioctl(-1, SG_SCSI_RESET, pint);
	printf("ioctl(-1, %s, [SG_SCSI_RESET_DEVICE]) = -1 EBADF (%m)\n",
	       "SG_SCSI_RESET");

	ioctl(-1, 0x22ff, 0);
	printf("ioctl(-1, _IOC(0, 0x22, 0xff, 0), 0) = -1 EBADF (%m)\n");

	static const unsigned long magic =
		(unsigned long) 0xdeadbeeffacefeedULL;
	ioctl(-1, 0x22ff, magic);
	printf("ioctl(-1, _IOC(0, 0x22, 0xff, 0), %#lx) = -1 EBADF (%m)\n",
	       magic);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SCSI_SG_H")

#endif
