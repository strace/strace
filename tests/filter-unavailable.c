/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define P 16
#define T 7

static void *
thread(void *arg)
{
	assert(write(1, "", 1) == 1);
	pause();
	return arg;
}

static int
process(void)
{
	int fds[2];
	pthread_t t;
	struct timespec ts = { .tv_nsec = 10000000 };

	(void) close(0);
	(void) close(1);
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	for (int i = 0; i < T; ++i)
		assert(pthread_create(&t, NULL, thread, NULL) == 0);
	for (int i = 0; i < T; ++i)
		assert(read(0, fds, 1) == 1);

	(void) nanosleep(&ts, 0);
	return 0;
}

int
main(void)
{
	for (int i = 0; i < P; ++i) {
		pid_t p = fork();
		if (p < 0)
			perror_msg_and_fail("fork");
		if (p == 0)
			return process();
		int s;
		assert(waitpid(p, &s, 0) == p);
		assert(WIFEXITED(s));
		if (WEXITSTATUS(s))
			return WEXITSTATUS(s);
	}
	return 0;
}
