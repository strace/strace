/*
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "scno.h"

int
main(void)
{
	assert(syscall(__NR_times, 0x42) == -1);
	printf("times(0x42) = -1 EFAULT (%m)\n");
	puts("+++ exited with 0 +++");

	return 0;
}
