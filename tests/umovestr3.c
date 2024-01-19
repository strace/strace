/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const unsigned int size = PATH_MAX - 1;
	const char *p = tail_alloc(size);
	const char *const efault = p + size;

	for (; p <= efault; ++p) {
		int rc = chdir(p);
		printf("chdir(%p) = %s\n", p, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
