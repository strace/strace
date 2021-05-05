/*
 * Check decoding of close_range syscall.
 *
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/close_range.h>

static const char *errstr;

static long
k_close_range(const unsigned int fd1, const unsigned int fd2, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd1;
	const kernel_ulong_t arg2 = fill | fd2;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_close_range, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	k_close_range(-2U, -1U, 1);
	printf("close_range(%u, %u, 0x1 /* CLOSE_RANGE_??? */) = %s\n",
	       -2U, -1U, errstr);

	k_close_range(-1U, -2U, 2);
	printf("close_range(%u, %u, CLOSE_RANGE_UNSHARE) = %s\n",
	       -1U, -2U, errstr);

	k_close_range(-3U, 0, 4);
	printf("close_range(%u, 0, CLOSE_RANGE_CLOEXEC) = %s\n", -3U, errstr);

	k_close_range(0, -4U, -1);
	printf("close_range(0, %u"
	       ", CLOSE_RANGE_UNSHARE|CLOSE_RANGE_CLOEXEC|%#x) = %s\n",
	       -4U, (-1U & ~(CLOSE_RANGE_UNSHARE | CLOSE_RANGE_CLOEXEC)),
	       errstr);

	k_close_range(-5U, 7, 0);
	printf("close_range(%u, 7, 0) = %s\n", -5U, errstr);

	k_close_range(7, -6U, 1);
	printf("close_range(7, %u, 0x1 /* CLOSE_RANGE_??? */) = %s\n",
	       -6U, errstr);

	k_close_range(7, 7, 8);
	printf("close_range(7, 7, 0x8 /* CLOSE_RANGE_??? */) = %s\n", errstr);

	k_close_range(-7U, -7U, 7);
	printf("close_range(%u, %u"
	       ", CLOSE_RANGE_UNSHARE|CLOSE_RANGE_CLOEXEC|0x1) = %s\n",
	       -7U, -7U, errstr);

	k_close_range(7, 0, 0);
	printf("close_range(7, 0, 0) = %s\n", errstr);

	k_close_range(0, 0, 0);
	printf("close_range(0, 0, 0) = %s\n", errstr);

	k_close_range(7, -1U, 0);
	printf("close_range(7, %u, 0) = %s\n", -1U, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
