/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>

static void
move_fd(int *from, int *to)
{
	for (; *to > *from; --*to) {
		if (dup2(*from, *to) != *to)
			continue;
		close(*from);
		*from = *to;
		break;
	}
}

void
pipe_maxfd(int pipefd[2])
{
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim))
		perror_msg_and_fail("getrlimit");
	if (rlim.rlim_cur < rlim.rlim_max) {
		struct rlimit rlim_new;
		rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
		if (!setrlimit(RLIMIT_NOFILE, &rlim_new))
			rlim.rlim_cur = rlim.rlim_max;
	}

	if (pipe(pipefd))
		perror_msg_and_fail("pipe");

	int max_fd = (rlim.rlim_cur > 0 && rlim.rlim_cur < INT_MAX)
		     ? rlim.rlim_cur - 1 : INT_MAX;

	move_fd(&pipefd[1], &max_fd);
	--max_fd;
	move_fd(&pipefd[0], &max_fd);
}
