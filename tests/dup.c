/*
 * Check decoding of dup syscall.
 *
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

#ifndef FD0_PATH
# define FD0_PATH ""
#endif
#ifndef FD9_PATH
# define FD9_PATH ""
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef TRACE_FDS
# define TRACE_FDS 0
#endif
#ifndef PATH_TRACING
# define PATH_TRACING 0
#endif
#ifndef TRACE_FD_0
# define TRACE_FD_0 (!TRACE_FDS && !PATH_TRACING)
#endif
#ifndef TRACE_OTHER_FDS
# define TRACE_OTHER_FDS (!TRACE_FDS && !PATH_TRACING)
#endif
#ifndef TRACE_FD_9
# define TRACE_FD_9 (!TRACE_FDS && !PATH_TRACING)
#endif

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
#if !PATH_TRACING && !TRACE_FDS
	printf("dup(-1) = %s\n", errstr);
#endif

	int d1 = k_dup(0);
#if TRACE_FD_0
	printf("dup(0" FD0_PATH ") = %d" FD0_PATH "\n", d1);
#endif

	int d2 = k_dup(d1);
#if TRACE_OTHER_FDS
	printf("dup(%d" FD0_PATH ") = %d" FD0_PATH "\n", d1, d2);
#endif

	d2 = k_dup(9);
#if PATH_TRACING || TRACE_FD_9
	printf("dup(9" FD9_PATH ") = %d" FD9_PATH "\n", d2);
#endif

	d1 = k_dup(d2);
#if PATH_TRACING || TRACE_OTHER_FDS
	printf("dup(%d" FD9_PATH ") = %d" FD9_PATH "\n", d2, d1);
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
