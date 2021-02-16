/*
 * This file is part of time strace test.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_time

# include <time.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(time_t, p);

	time_t t = syscall(__NR_time, NULL);
	if ((time_t) -1 == t)
		perror_msg_and_skip("time");
	printf("time(NULL) = %lld (", (long long) t);
	print_time_t_nsec(t, 0, 0);
	puts(")");

	t = syscall(__NR_time, p + 1);
	printf("time(%p) = %s\n", p + 1, sprintrc(t));

	t = syscall(__NR_time, p);
	printf("time([%lld", (long long) *p);
	print_time_t_nsec((time_t) *p, 0, 1),
	printf("]) = %lld (", (long long) t);
	print_time_t_nsec(t, 0, 0);
	puts(")");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_time")

#endif
