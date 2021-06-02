/*
 * Check decoding of dup3 syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include "kernel_fcntl.h"

#ifndef FD0_PATH
# define FD0_PATH ""
#endif
#ifndef FD7_PATH
# define FD7_PATH ""
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

static const char *errstr;

static long
k_dup3(const unsigned int fd1, const unsigned int fd2, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd1;
	const kernel_ulong_t arg2 = fill | fd2;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_dup3, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	int fd0 = dup(0);
	int fd7 = dup(7);

	k_dup3(0, 0, 0);
#ifndef PATH_TRACING
	printf("dup3(0" FD0_PATH ", 0" FD0_PATH ", 0) = %s\n", errstr);
#endif

	k_dup3(-1, -2, O_CLOEXEC);
#ifndef PATH_TRACING
	printf("dup3(-1, -2, O_CLOEXEC) = %s\n", errstr);
#endif

	k_dup3(-2, -1, O_TRUNC);
#ifndef PATH_TRACING
	printf("dup3(-2, -1, O_TRUNC) = %s\n", errstr);
#endif

	k_dup3(-3, 0, O_TRUNC | O_CLOEXEC);
#ifndef PATH_TRACING
	printf("dup3(-3, 0" FD0_PATH ", O_TRUNC|O_CLOEXEC) = %s\n", errstr);
#endif

	k_dup3(0, -4, O_RDONLY);
#ifndef PATH_TRACING
	printf("dup3(0" FD0_PATH ", -4, 0) = %s\n", errstr);
#endif

	k_dup3(-5, 7, O_WRONLY);
	printf("dup3(-5, 7" FD7_PATH ", 0x1 /* O_??? */) = %s\n", errstr);

	k_dup3(7, -6, O_RDWR);
	printf("dup3(7" FD7_PATH ", -6, 0x2 /* O_??? */) = %s\n", errstr);

	k_dup3(7, 7, O_CLOEXEC);
	printf("dup3(7" FD7_PATH ", 7" FD7_PATH ", O_CLOEXEC) = %s\n", errstr);

	k_dup3(-7, -7, 7);
#ifndef PATH_TRACING
	printf("dup3(-7, -7, 0x7 /* O_??? */) = %s\n", errstr);
#endif

	if (k_dup3(0, fd0, O_CLOEXEC) != fd0)
		perror_msg_and_skip("dup3");
#ifndef PATH_TRACING
	printf("dup3(0" FD0_PATH ", %d" FD0_PATH ", O_CLOEXEC) = %d" FD0_PATH
	       "\n", fd0, fd0);
#endif

	k_dup3(7, fd7, 0);
	printf("dup3(7" FD7_PATH ", %d" FD7_PATH ", 0) = %d" FD7_PATH
	       "\n", fd7, fd7);

	k_dup3(0, fd7, O_CLOEXEC);
	printf("dup3(0" FD0_PATH ", %d" FD7_PATH ", O_CLOEXEC) = %d" FD0_PATH
	       "\n", fd7, fd7);

	k_dup3(7, fd0, 0);
	printf("dup3(7" FD7_PATH ", %d" FD0_PATH ", 0) = %d" FD7_PATH
	       "\n", fd0, fd0);

	close(fd0);
	close(fd7);

	k_dup3(0, fd0, O_CLOEXEC);
#ifndef PATH_TRACING
	printf("dup3(0" FD0_PATH ", %d, O_CLOEXEC) = %d" FD0_PATH "\n",
	       fd0, fd0);
#endif

	k_dup3(7, fd7, 0);
	printf("dup3(7" FD7_PATH ", %d, 0) = %d" FD7_PATH "\n",
	       fd7, fd7);

	puts("+++ exited with 0 +++");
	return 0;
}
