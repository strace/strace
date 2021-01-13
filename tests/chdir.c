/*
 * Check decoding of chdir syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(void)
{
	char *const p = tail_alloc(PATH_MAX);
	memset(p, '/', PATH_MAX);

	printf("chdir(NULL) = %s\n",
	       sprintrc(syscall(__NR_chdir, NULL)));

	printf("chdir(\"%.*s\"...) = %s\n",
	       PATH_MAX - 1, p, sprintrc(chdir(p)));

	p[PATH_MAX - 1] = '\0';
	for (unsigned int i = 0; i < PATH_MAX; ++i)
		printf("chdir(\"%.*s\") = %s\n",
		       PATH_MAX - 1 - i, p + i, sprintrc(chdir(p + i)));

	puts("+++ exited with 0 +++");
	return 0;
}
