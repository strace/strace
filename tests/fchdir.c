/*
 * Check decoding of fchdir syscall.
 *
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const long int fd = (long int) 0xdeadbeefffffffffULL;

	long rc = syscall(__NR_fchdir, fd);
	printf("fchdir(%d) = %s\n", (int) fd, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
