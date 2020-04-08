/*
 * Check that the signal handler for the specified signal number is set
 * to SIG_IGN/SIG_DFL.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
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
		error_msg_and_fail("usage: check_sigign 0|1 signum");

	const int ign = !!atoi(av[1]);
	const int signum = atoi(av[2]);
	struct sigaction act;

	if (sigaction(signum, NULL, &act))
		perror_msg_and_fail("sigaction: %s", av[2]);

	return ign ^ (act.sa_handler == SIG_IGN);
}
