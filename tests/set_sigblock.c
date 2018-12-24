/*
 * Execute a command with the specified signal blocked/unblocked.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
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
	if (ac < 4)
		error_msg_and_fail("usage: set_sigblock 0|1 signum path...");

	const int block = atoi(av[1]);
	const int signum = atoi(av[2]);
	sigset_t mask;

	sigemptyset(&mask);
	if (sigaddset(&mask, signum))
		perror_msg_and_fail("sigaddset: %s", av[2]);
	if (sigprocmask(block ? SIG_BLOCK : SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	execvp(av[3], av + 3);
	perror_msg_and_fail("execvp: %s", av[3]);
}
