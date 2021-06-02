/*
 * Check decoding of epoll_ctl syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>

static long
invoke_syscall(unsigned long epfd, unsigned long op, unsigned long fd, void *ev)
{
	return syscall(__NR_epoll_ctl, epfd, F8ILL_KULONG_MASK | op,
		       fd, (unsigned long) ev);
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct epoll_event, ev);
	ev->events = EPOLLIN;

	long rc = invoke_syscall(-1U, EPOLL_CTL_ADD, -2U, ev);
	printf("epoll_ctl(-1, EPOLL_CTL_ADD, -2"
	       ", {events=EPOLLIN, data={u32=%u, u64=%" PRIu64 "}}) = %s\n",
	       ev->data.u32, ev->data.u64, sprintrc(rc));

	rc = invoke_syscall(-3U, EPOLL_CTL_DEL, -4U, ev);
	printf("epoll_ctl(-3, EPOLL_CTL_DEL, -4, %p) = %s\n",
	       ev, sprintrc(rc));

	rc = invoke_syscall(-1UL, EPOLL_CTL_MOD, -16UL, 0);
	printf("epoll_ctl(-1, EPOLL_CTL_MOD, -16, NULL) = %s\n",
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
