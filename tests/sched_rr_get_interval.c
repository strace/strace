/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_sched_rr_get_interval

# include <stdint.h>
# include <stdio.h>
# include <sched.h>
# include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timespec, tp);
	long rc;

	rc = syscall(__NR_sched_rr_get_interval, 0, NULL);
	printf("sched_rr_get_interval(0, NULL) = %s\n", sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, 0, tp + 1);
	printf("sched_rr_get_interval(0, %p) = %s\n", tp + 1, sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, -1, tp);
	printf("sched_rr_get_interval(-1, %p) = %s\n", tp, sprintrc(rc));

	rc = syscall(__NR_sched_rr_get_interval, 0, tp);
	if (rc == 0)
		printf("sched_rr_get_interval(0, {tv_sec=%lld, tv_nsec=%llu})"
		       " = 0\n",
		       (long long) tp->tv_sec,
		       zero_extend_signed_to_ull(tp->tv_nsec));
	else
		printf("sched_rr_get_interval(-1, %p) = %s\n", tp,
			sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sched_rr_get_interval")

#endif
