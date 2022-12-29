/*
 * Check status=successful filtering when a non-leader thread invokes execve.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>

static pid_t leader;

static void *
thread_exec(void *arg)
{
	pid_t tid = syscall(__NR_gettid);
	printf("%-5d +++ superseded by execve in pid %u +++\n",
	       leader, tid);

	char *argv[] = {((char **) arg)[0], (char *) "0", NULL};
	execve(argv[0], argv, NULL);
	perror_msg_and_fail("execve");
}

int
main(int ac, char **av)
{
	if (ac > 1)
		return 0;

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	pid_t child = fork();
	if (child < 0)
		perror_msg_and_fail("fork");

	leader = getpid();
	setvbuf(stdout, NULL, _IONBF, 0);

	if (!child) {
		close(fds[0]);
		pthread_t thre;
		errno = pthread_create(&thre, NULL, thread_exec, av);
		if (errno)
			perror_msg_and_fail("pthread_create");
		for (;;) { /* wait for execve */ }
		return 1;
	}

	close(fds[1]);
	unsigned int len = sizeof(fds[1]);
	struct iovec rio = { .iov_base = &fds[1], .iov_len = len };
	if (readv(fds[0], &rio, 1))
		perror_msg_and_fail("readv");

	printf("%-5d readv(%d, [{iov_base=\"\", iov_len=%u}], 1) = 0\n"
	       "%-5d +++ exited with 0 +++\n",
	       leader, fds[0], len, leader);
	return 0;
}
