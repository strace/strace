/*
 * Print the maximum descriptor number available.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
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
