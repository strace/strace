/*
 * Check decoding of epoll_pwait2 syscall.
 *
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "xmalloc.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "kernel_timespec.h"

#ifndef DECODE_FDS
# define DECODE_FDS 0
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

static const char *errstr;

static long
k_epoll_pwait2(const unsigned int epfd,
	       const void *const events,
	       const unsigned int maxevents,
	       const void *const timeout,
	       const void *const sigmask,
	       const kernel_ulong_t sigsetsize)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = fill | epfd;
	const kernel_ulong_t arg2 = (unsigned long) events;
	const kernel_ulong_t arg3 = fill | maxevents;
	const kernel_ulong_t arg4 = (unsigned long) timeout;
	const kernel_ulong_t arg5 = (unsigned long) sigmask;
	const kernel_ulong_t arg6 = sigsetsize;
	const long rc = syscall(__NR_epoll_pwait2,
				arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	static const char fd_path[] = "/dev/full";
	int fd = open(fd_path, O_WRONLY);
	if (fd < 0)
		perror_msg_and_fail("open: %s", fd_path);
	char *fd_str = xasprintf("%d%s%s%s", fd,
				 DECODE_FDS ? "<" : "",
				 DECODE_FDS ? fd_path : "",
				 DECODE_FDS ? ">" : "");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct epoll_event, events);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_timespec64_t, timeout);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, sigmask);
	const unsigned int sigsetsize = sizeof(*sigmask);

	k_epoll_pwait2(-1, events, fd, timeout, sigmask, fd);
#ifndef PATH_TRACING
	printf("epoll_pwait2(-1, %p, %d, %p, %p, %u) = %s\n",
	       events, fd, timeout, sigmask, fd, errstr);
#endif

	k_epoll_pwait2(fd, events, -1, timeout, sigmask, sigsetsize);
	printf("epoll_pwait2(%s, %p, -1, %p, %p, %u) = %s\n",
	       fd_str, events, timeout, sigmask, sigsetsize, errstr);

	/*
	 * Some positive tests should be added here
	 * when epoll_pwait2 becomes widely available.
	 */

	puts("+++ exited with 0 +++");
	return 0;
}
