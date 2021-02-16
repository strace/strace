/*
 * Check decoding of mlockall syscall.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <sys/mman.h>

int
main(void)
{
	int rc = mlockall(0);
	printf("mlockall(0) = %s\n", sprintrc(rc));

	rc = mlockall(MCL_CURRENT);
	printf("mlockall(MCL_CURRENT) = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
