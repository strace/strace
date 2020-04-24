/*
 * Check decoding of dup2 syscall.
 *
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_dup2

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
k_dup2(const unsigned int fd1, const unsigned int fd2)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd1;
	const kernel_ulong_t arg2 = fill | fd2;
	const long rc = syscall(__NR_dup2, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	int fd0 = dup(0);
	int fd9 = dup(9);

	if (k_dup2(0, 0))
		perror_msg_and_skip("dup2");
# ifndef PATH_TRACING
	printf("dup2(0" FD0_PATH ", 0" FD0_PATH ") = 0" FD0_PATH "\n");
# endif

	k_dup2(-1, -2);
# ifndef PATH_TRACING
	printf("dup2(-1, -2) = %s\n", errstr);
# endif

	k_dup2(-2, -1);
# ifndef PATH_TRACING
	printf("dup2(-2, -1) = %s\n", errstr);
# endif

	k_dup2(-3, 0);
# ifndef PATH_TRACING
	printf("dup2(-3, 0" FD0_PATH ") = %s\n", errstr);
# endif

	k_dup2(0, -4);
# ifndef PATH_TRACING
	printf("dup2(0" FD0_PATH ", -4) = %s\n", errstr);
# endif

	k_dup2(-5, 9);
	printf("dup2(-5, 9" FD9_PATH ") = %s\n", errstr);

	k_dup2(9, -6);
	printf("dup2(9" FD9_PATH ", -6) = %s\n", errstr);

	k_dup2(9, 9);
	printf("dup2(9" FD9_PATH ", 9" FD9_PATH ") = 9" FD9_PATH "\n");

	k_dup2(0, fd0);
# ifndef PATH_TRACING
	printf("dup2(0" FD0_PATH ", %d" FD0_PATH ") = %d" FD0_PATH "\n",
	       fd0, fd0);
# endif

	k_dup2(9, fd9);
	printf("dup2(9" FD9_PATH ", %d" FD9_PATH ") = %d" FD9_PATH "\n",
	       fd9, fd9);

	k_dup2(0, fd9);
	printf("dup2(0" FD0_PATH ", %d" FD9_PATH ") = %d" FD0_PATH "\n",
	       fd9, fd9);

	k_dup2(9, fd0);
	printf("dup2(9" FD9_PATH ", %d" FD0_PATH ") = %d" FD9_PATH "\n",
	       fd0, fd0);

	close(fd0);
	close(fd9);

	k_dup2(0, fd0);
# ifndef PATH_TRACING
	printf("dup2(0" FD0_PATH ", %d) = %d" FD0_PATH "\n",
	       fd0, fd0);
# endif

	k_dup2(9, fd9);
	printf("dup2(9" FD9_PATH ", %d) = %d" FD9_PATH "\n",
	       fd9, fd9);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_dup2")

#endif
