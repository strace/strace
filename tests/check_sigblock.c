/*
 * Check that the specified signal number is blocked/unblocked.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <stdlib.h>

int
main(int ac, char **av)
{
	if (ac != 3)
		error_msg_and_fail("usage: check_sigblock 0|1 signum");

	const int block = !!atoi(av[1]);
	const int signum = atoi(av[2]);
	sigset_t mask;

	sigemptyset(&mask);
	if (sigprocmask(SIG_SETMASK, NULL, &mask))
		perror_msg_and_fail("sigprocmask");

	return block ^ sigismember(&mask, signum);
}
