/*
 * Check decoding of SCSI ioctl commands.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SCSI_SG_H

# include <stdio.h>
# include <sys/ioctl.h>
# include <scsi/sg.h>
# define XLAT_MACROS_ONLY
#  include "xlat/scsi_sg_commands.h"
# undef XLAT_MACROS_ONLY

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
	printf("ioctl(-1, _IOC(%s, 0x22, 0xff, 0), 0) = -1 EBADF (%m)\n",
	       _IOC_NONE ? "0" : "_IOC_NONE");

	static const unsigned long magic =
		(unsigned long) 0xdeadbeeffacefeedULL;
	ioctl(-1, 0x22ff, magic);
	printf("ioctl(-1, _IOC(%s, 0x22, 0xff, 0), %#lx) = -1 EBADF (%m)\n",
	       _IOC_NONE ? "0" : "_IOC_NONE", magic);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SCSI_SG_H")

#endif
