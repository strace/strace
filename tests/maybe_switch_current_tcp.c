/*
 * Check for PTRACE_EVENT_EXEC diagnostics.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2019-2020 The strace developers.
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

#ifndef QUIET_MSG
# define QUIET_MSG 0
#endif

static pid_t leader;
static volatile unsigned int trigger;

static void *
thread(void *arg)
{
	const char *argv[] = {((char **) arg)[0], "1", "2", NULL};
	int tid = syscall(__NR_gettid);

	printf("%-5d execveat(AT_FDCWD, \"%s\", [\"%s\", \"%s\", \"%s\"]"
	       ", NULL, 0 <pid changed to %d ...>\n"
#if !QUIET_MSG
	       "%-5d +++ superseded by execve in pid %d +++\n"
#endif
	       , tid, argv[0], argv[0], argv[1], argv[2], leader
#if !QUIET_MSG
	       , leader, tid
#endif
	       );

	while (!trigger) {
		/* Wait for the parent to enter the busy loop.  */
	}

	syscall(__NR_execveat, -100, argv[0], argv, NULL, 0);
	perror_msg_and_fail("execveat");
}

int
main(int ac, char **av)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	leader = getpid();

	if (ac <= 1) {
		char *argv[] = {av[0], (char *) "1", NULL};
		printf("%-5d execveat(AT_FDCWD, \"%s\""
		       ", [\"%s\", \"%s\"], NULL, 0) = 0\n",
		       leader, argv[0], argv[0], argv[1]);
		syscall(__NR_execveat, -100, argv[0], argv, NULL, 0);
		perror_msg_and_skip("execveat");
	}

	/*
	 * Since execveat is supported by the kernel,
	 * PTRACE_EVENT_EXEC support in the kernel is good enough.
	 */
	if (ac <= 2) {
		pthread_t t;
		errno = pthread_create(&t, NULL, thread, av);
		if (errno)
			perror_msg_and_fail("pthread_create");

		for (;;)
			++trigger;
	}

	printf("%-5d <... execveat resumed>) = 0\n"
	       "%-5d +++ exited with 0 +++\n",
	       leader, leader);
	return 0;
}
