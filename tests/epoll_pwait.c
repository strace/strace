/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_epoll_pwait

# include <signal.h>
# include <stdio.h>
# include <sys/epoll.h>
# include <unistd.h>

int
main(void)
{
	sigset_t set[2];
	TAIL_ALLOC_OBJECT_CONST_PTR(struct epoll_event, ev);

	long rc = syscall(__NR_epoll_pwait, -1, ev, 1, -2,
			  set, (kernel_ulong_t) sizeof(set));
	printf("epoll_pwait(-1, %p, 1, -2, %p, %u) = %s\n",
	       ev, set, (unsigned) sizeof(set), sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_epoll_pwait")

#endif
