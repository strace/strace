/*
 * Check basic seccomp filtering with large number of traced syscalls.
 *
 * Copyright (c) 2018-2019 The strace developers.
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
	printf("%-5d +++ exited with 0 +++\n", getpid());
	return 0;
}
