/*
 * Check decoding of close_range syscall.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_close_range

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# ifdef HAVE_LINUX_CLOSE_RANGE_H
#  include <linux/close_range.h>
# else
#  define CLOSE_RANGE_UNSHARE	(1U << 1)
# endif

# ifndef FD0_PATH
#  define FD0_PATH ""
# endif
# ifndef FD7_PATH
#  define FD7_PATH ""
# endif
# ifndef SKIP_IF_PROC_IS_UNAVAILABLE
#  define SKIP_IF_PROC_IS_UNAVAILABLE
# endif

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

static void
xdup2(int fd1, int fd2)
{
	if (dup2(fd1, fd2) != fd2)
		perror_msg_and_fail("dup2(%d, %d)", fd1, fd2);
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	const int fd0 = dup(0);
	const int fd7 = dup(7);
	const int fd7_min = MIN(7, fd7);
	const int fd7_max = MAX(7, fd7);

	k_close_range(-2, -1, 1);
# ifndef PATH_TRACING
	printf("close_range(-2, -1, 0x1 /* CLOSE_RANGE_??? */) = %s\n", errstr);
# endif

	k_close_range(-1, -2, 2);
# ifndef PATH_TRACING
	printf("close_range(-1, -2, CLOSE_RANGE_UNSHARE) = %s\n", errstr);
# endif

	k_close_range(-3, 0, 4);
# ifndef PATH_TRACING
	printf("close_range(-3, 0" FD0_PATH ", 0x4 /* CLOSE_RANGE_??? */)"
	       " = %s\n", errstr);
# endif

	k_close_range(0, -4, -1);
# ifndef PATH_TRACING
	printf("close_range(0" FD0_PATH ", -4, CLOSE_RANGE_UNSHARE|%#x) = %s\n",
	       (-1U & ~CLOSE_RANGE_UNSHARE), errstr);
# endif

	k_close_range(-5, 7, 0);
	printf("close_range(-5, 7" FD7_PATH ", 0) = %s\n", errstr);

	k_close_range(7, -6, 1);
	printf("close_range(7" FD7_PATH ", -6, 0x1 /* CLOSE_RANGE_??? */)"
	       " = %s\n", errstr);

	k_close_range(7, 7, 4);
	printf("close_range(7" FD7_PATH ", 7" FD7_PATH
	       ", 0x4 /* CLOSE_RANGE_??? */) = %s\n", errstr);

	k_close_range(-7, -7, 7);
# ifndef PATH_TRACING
	printf("close_range(-7, -7, CLOSE_RANGE_UNSHARE|0x5) = %s\n", errstr);
# endif

	k_close_range(7, 0, 0);
	printf("close_range(7" FD7_PATH ", 0" FD0_PATH ", 0) = %s\n", errstr);

	k_close_range(fd7, fd0, 0);
	printf("close_range(%d" FD7_PATH ", %d" FD0_PATH ", 0) = %s\n",
	       fd7, fd0, errstr);

	if (k_close_range(0, 0, 0) == 0)
		xdup2(fd0, 0);
# ifndef PATH_TRACING
	printf("close_range(0" FD0_PATH ", 0" FD0_PATH ", 0) = %s\n", errstr);
# endif

	if (k_close_range(fd0, fd0, 0) == 0)
		xdup2(0, fd0);
# ifndef PATH_TRACING
	printf("close_range(%d" FD0_PATH ", %d" FD0_PATH ", 0) = %s\n",
	       fd0, fd0, errstr);
# endif

	if (k_close_range(7, 7, 0) == 0)
		xdup2(fd7, 7);
	printf("close_range(7" FD7_PATH ", 7" FD7_PATH ", 0) = %s\n", errstr);

	if (k_close_range(fd7, fd7, 0) == 0)
		xdup2(7, fd7);
	printf("close_range(%d" FD7_PATH ", %d" FD7_PATH ", 0) = %s\n",
	       fd7, fd7, errstr);

	if (k_close_range(fd7_max, -1, 0) == 0)
		xdup2(fd7_min, fd7_max);
	printf("close_range(%d" FD7_PATH ", -1, 0) = %s\n",
	       fd7_max, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_close_range")

#endif
