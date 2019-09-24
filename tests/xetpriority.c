/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getpriority && defined __NR_setpriority

# include <stdio.h>
# include <sys/resource.h>
# include <unistd.h>

int
main(void)
{
	const int pid = getpid();
	long rc = syscall(__NR_getpriority, PRIO_PROCESS,
			  F8ILL_KULONG_MASK | pid);
	printf("getpriority(PRIO_PROCESS, %d) = %ld\n", pid, rc);

	rc = syscall(__NR_setpriority, PRIO_PROCESS,
		     F8ILL_KULONG_MASK | pid, F8ILL_KULONG_MASK);
	printf("setpriority(PRIO_PROCESS, %d, 0) = %s\n", pid, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpriority && _NR_setpriority")

#endif
