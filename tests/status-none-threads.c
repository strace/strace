/*
 * Check status=none filtering when a non-leader thread invokes execve.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "scno.h"

static pid_t leader;

static void *
thread(void *arg)
{
	struct timespec ts = { .tv_nsec = 100000000 };
	(void) nanosleep(&ts, NULL);

	printf("%-5d +++ superseded by execve in pid %u +++\n",
	       leader, (int) syscall(__NR_gettid));

	char *argv[] = {((char **) arg)[0], (char *) "0", NULL};
	execve(argv[0], argv, NULL);
	perror_msg_and_fail("execve");
}

int
main(int ac, char **av)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	leader = getpid();

	if (ac > 1) {
		printf("%-5d +++ exited with 0 +++\n", leader);
		return 0;
	}

	pthread_t t;
	errno = pthread_create(&t, NULL, thread, av);
	if (errno)
		perror_msg_and_fail("pthread_create");

	struct timespec ts = { .tv_sec = 123 };
	(void) nanosleep(&ts, 0);

	return 1;
}
