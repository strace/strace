/*
 * Check decoding of sched_get_priority_max and sched_get_priority_min syscalls.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <sched.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	int rc = syscall(__NR_sched_get_priority_min, SCHED_FIFO);
	printf("sched_get_priority_min(SCHED_FIFO) = %d\n", rc);

	rc = syscall(__NR_sched_get_priority_max, SCHED_RR);
	printf("sched_get_priority_max(SCHED_RR) = %d\n", rc);

	puts("+++ exited with 0 +++");
	return 0;
}
