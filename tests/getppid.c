/*
 * Check decoding of getppid syscall.
 *
 * Copyright (c) 2017-2021 The strace developers.
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
	printf("getppid() = %s\n", sprintrc(syscall(__NR_getppid)));
	puts("+++ exited with 0 +++");
	return 0;
}
