/*
 * Check decoding of epoll_create1 syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include "kernel_fcntl.h"

int
main(void)
{
	long rc = syscall(__NR_epoll_create1, O_CLOEXEC);
	printf("epoll_create1(EPOLL_CLOEXEC) = %s\n", sprintrc(rc));

	rc = syscall(__NR_epoll_create1, O_CLOEXEC | O_NONBLOCK);
	printf("epoll_create1(EPOLL_CLOEXEC|%#x) = %s\n",
	       O_NONBLOCK, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
