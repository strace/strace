/*
 * Check status=unfinished filtering when a non-leader thread invokes execve.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_nanosleep

# include <errno.h>
# include <pthread.h>
# include <stdio.h>
# include <unistd.h>

# include "kernel_old_timespec.h"

static pid_t leader;

static void *
thread(void *arg)
{
	kernel_old_timespec_t ts = { .tv_nsec = 100000000 };
	(void) syscall(__NR_nanosleep, (unsigned long) &ts, 0UL);

	printf("%-5d nanosleep({tv_sec=123, tv_nsec=0},  <unfinished ...>) = ?\n"
	       "%-5d +++ superseded by execve in pid %u +++\n",
	       leader, leader, (int) syscall(__NR_gettid));

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
		printf("%-5d exit_group(0) = ?\n"
		       "%-5d +++ exited with 0 +++\n", leader, leader);
		return 0;
	}

	pthread_t t;
	errno = pthread_create(&t, NULL, thread, av);
	if (errno)
		perror_msg_and_fail("pthread_create");

	kernel_old_timespec_t ts = { .tv_sec = 123 };
	(void) syscall(__NR_nanosleep, (unsigned long) &ts, 0UL);

	return 1;
}

#else

SKIP_MAIN_UNDEFINED("__NR_nanosleep")

#endif
