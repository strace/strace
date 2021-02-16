/*
 * Execute a command with blocked, reset, and raised signal.
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

int
main(int ac, char **av)
{
	if (ac < 3)
		error_msg_and_fail("usage: block_reset_raise_run signo path...");

	sigset_t mask;
	sigemptyset(&mask);
	const int signo = atoi(av[1]);
	if (sigaddset(&mask, signo))
		perror_msg_and_fail("sigaddset: %s", av[1]);
	if (sigprocmask(SIG_BLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");
	if (signal(signo, SIG_DFL) == SIG_ERR)
		perror_msg_and_fail("signal: %s", av[1]);
	if (raise(signo))
		perror_msg_and_fail("raise: %s", av[1]);

	execvp(av[2], av + 2);
	perror_msg_and_fail("execvp: %s", av[2]);
}
