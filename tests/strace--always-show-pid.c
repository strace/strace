/*
 * Check the PID prefix output with --always-show-pid option.
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const int pid = getpid();
	printf("%-5u fchdir(-1) = %s\n", pid, sprintrc(fchdir(-1)));

	printf("%-5u +++ exited with 0 +++\n", pid);
	return 0;
}
