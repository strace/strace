/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_sched_yield

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("sched_yield() = %ld\n", syscall(__NR_sched_yield));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_yield")

#endif
