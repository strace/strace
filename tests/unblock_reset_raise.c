/*
 * Unblock, reset, and raise a signal.
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
	if (ac != 2)
		error_msg_and_fail("usage: unblock_raise signo");

	sigset_t mask;
	sigemptyset(&mask);
	const int signo = atoi(av[1]);
	if (sigaddset(&mask, signo))
		perror_msg_and_fail("sigaddset: %s", av[1]);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");
	if (signal(signo, SIG_DFL) == SIG_ERR)
		perror_msg_and_fail("signal: %s", av[1]);
	if (raise(signo))
		perror_msg_and_fail("raise: %s", av[1]);

	return 0;
}
