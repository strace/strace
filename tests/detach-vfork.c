/*
 * Check detaching from vfork'ed processes.
 *
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int
main(int ac, char **av)
{
	if (ac != 2)
		error_msg_and_fail("ac = %d", ac);

	signal(SIGTERM, SIG_IGN);

	pid_t pid = vfork();

	if (pid < 0)
		perror_msg_and_fail("vfork");

	if (!pid) {
		sleep(4);
		_exit(0);
	}

	int s;
	pid_t rc;
	while ((rc = waitpid(pid, &s, 0)) != pid) {
		if (rc < 0 && errno == EINTR)
			continue;
		perror_msg_and_fail("waitpid: %d", pid);
	}

	const char *exe = getenv("STRACE_EXE") ?: "strace";
	printf("%s: Process %d attached\n"
	       "%s: Process %d detached\n"
	       "%s: Termination of unknown pid %s ignored\n"
	       "%s: Process %d detached\n",
	       exe, pid,
	       exe, pid,
	       exe, av[1],
	       exe, getpid());

	return WIFEXITED(s) ? WEXITSTATUS(s)
			    : (WIFSIGNALED(s) ? 128 + WTERMSIG(s) : 9);
}
