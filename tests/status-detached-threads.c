/*
 * Check status=detached filtering when a non-leader thread invokes execve.
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
#include <time.h>
#include <unistd.h>

static void *
thread(void *arg)
{
	struct timespec ts = { .tv_nsec = 100000000 };
	nanosleep(&ts, 0);

	char *argv[] = {((char **) arg)[0], (char *) "0", NULL};
	pid_t pid = getpid();
	pid_t tid = syscall(__NR_gettid);

	printf("%-5d execve(\"%s\", [\"%s\", \"0\"], NULL <pid changed to %u ...>\n"
	       "%-5d +++ superseded by execve in pid %u +++\n"
	       "%-5d +++ exited with 0 +++\n",
	       tid, argv[0], argv[0], pid, pid, tid, pid);

	execve(argv[0], argv, NULL);
	perror_msg_and_fail("execve");
}

int
main(int ac, char **av)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	if (ac > 1)
		return 0;

	pthread_t t;
	errno = pthread_create(&t, NULL, thread, av);
	if (errno)
		perror_msg_and_fail("pthread_create");

	struct timespec ts = { .tv_sec = 123 };
	nanosleep(&ts, 0);

	return 1;
}
