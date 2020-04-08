/*
 * This file is part of count-f strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 32
#define P 8
#define T 4

static void *
thread(void *arg)
{
	unsigned int i;

	assert(chdir(".") == 0);
	for (i = 0; i < N; ++i) {
		assert(chdir("") == -1);
		assert(chdir(".") == 0);
	}

	return NULL;
}

static int
process(void)
{
	unsigned int i;
	pthread_t t[T];

	for (i = 0; i < T; ++i) {
		errno = pthread_create(&t[i], NULL, thread, NULL);
		if (errno)
			perror_msg_and_fail("pthread_create");
	}

	for (i = 0; i < T; ++i) {
		void *retval;
		errno = pthread_join(t[i], &retval);
		if (errno)
			perror_msg_and_fail("pthread_join");
	}

	return 0;
}

int
main(void)
{
	unsigned int i;
	pid_t p[P];

	for (i = 0; i < P; ++i) {
		p[i] = fork();
		if (p[i] < 0)
			perror_msg_and_fail("fork");
		if (!p[i])
			return process();
	}
	for (i = 0; i < P; ++i) {
		int s;

		assert(waitpid(p[i], &s, 0) == p[i]);
		assert(WIFEXITED(s));
		if (WEXITSTATUS(s))
			return WEXITSTATUS(s);
	}

	return 0;
}
