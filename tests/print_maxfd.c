/*
 * Print the maximum descriptor number available.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/resource.h>

int
main(void)
{
	int fds[2];
	pipe_maxfd(fds);
	printf("%d\n", fds[1]);
	return 0;
}
