/*
 * Check decoding of dup syscall.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_dup

# include <stdio.h>
# include <unistd.h>

# ifndef FD0_PATH
#  define FD0_PATH ""
# endif
# ifndef FD9_PATH
#  define FD9_PATH ""
# endif
# ifndef SKIP_IF_PROC_IS_UNAVAILABLE
#  define SKIP_IF_PROC_IS_UNAVAILABLE
# endif

static const char *errstr;

static long
k_dup(const unsigned int fd)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd;
	const long rc = syscall(__NR_dup, arg1, bad, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	k_dup(-1);
# ifndef PATH_TRACING
	printf("dup(-1) = %s\n", errstr);
# endif

	int d1 = k_dup(0);
# ifndef PATH_TRACING
	printf("dup(0" FD0_PATH ") = %d" FD0_PATH "\n", d1);
# endif

	int d2 = k_dup(d1);
# ifndef PATH_TRACING
	printf("dup(%d" FD0_PATH ") = %d" FD0_PATH "\n", d1, d2);
# endif

	d2 = k_dup(9);
	printf("dup(9" FD9_PATH ") = %d" FD9_PATH "\n", d2);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_dup")

#endif
