/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined(__NR_sched_get_priority_min) \
 && defined(__NR_sched_get_priority_max)

# include <sched.h>
# include <stdio.h>
# include <unistd.h>

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

#else

SKIP_MAIN_UNDEFINED("__NR_sched_get_priority_min"
		    " && defined __NR_sched_get_priority_max");

#endif
