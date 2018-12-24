/*
 * Copyright (c) 2016-2018 The strace developers.
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
	pid_t pid = getpid();
	printf("getsid(%d) = %d\n", pid, getsid(pid));

	puts("+++ exited with 0 +++");
	return 0;
}
