/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include "scno.h"

#if defined __NR_epoll_create1 && defined O_CLOEXEC

# include <stdio.h>
# include <unistd.h>

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

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_create1 && O_CLOEXEC")

#endif
