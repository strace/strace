/*
 * Execute a command, expect its termination with a specified signal.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int
main(int ac, char **av)
{
	if (ac < 3)
		error_msg_and_fail("usage: run_expect_termsig signo path...");

	signal(SIGCHLD, SIG_DFL);

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		execvp(av[2], av + 2);
		perror_msg_and_fail("execvp: %s", av[2]);
	}

	int status;
	if (waitpid(pid, &status, 0) != pid)
		perror_msg_and_fail("waitpid");

	return !(WIFSIGNALED(status) && WTERMSIG(status) == atoi(av[1]));
}
