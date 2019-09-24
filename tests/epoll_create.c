/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_epoll_create

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int size = (long int) 0xdeadbeefffffffffULL;

	long rc = syscall(__NR_epoll_create, size);
	printf("epoll_create(%d) = %ld %s (%m)\n",
	       (int) size, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_creat")

#endif
