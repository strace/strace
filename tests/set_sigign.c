/*
 * Execute a command with a signal handler set to SIG_IGN/SIG_DFL.
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
	if (ac < 4)
		error_msg_and_fail("usage: set_sigign 0|1 signum path...");

	const int ign = atoi(av[1]);
	const int signum = atoi(av[2]);

	if (signal(signum, ign ? SIG_IGN : SIG_DFL) == SIG_ERR)
		perror_msg_and_fail("signal: %s", av[2]);

	execvp(av[3], av + 3);
	perror_msg_and_fail("execvp: %s", av[3]);
}
