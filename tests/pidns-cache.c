/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#if defined __NR_getpid && (!defined __NR_getxpid || __NR_getxpid != __NR_getpid)

# include <stdio.h>
# include <unistd.h>
# include <sys/time.h>

# define SYSCALL_COUNT 1000

/**
 * Max ratio of the execution time with and without pidns translation.
 */
# define MAX_TIME_RATIO 20

static long
execute_syscalls(void)
{
	/* Load our PID in the cache */
	syscall(__NR_getpid);

	struct timeval stop, start;
	gettimeofday(&start, NULL);

	for (int i = 0; i < SYSCALL_COUNT; i++)
	       syscall(__NR_getpid);

	gettimeofday(&stop, NULL);

	return (stop.tv_usec - start.tv_usec) +
		(stop.tv_sec - start.tv_sec) * 1000000;
}

int
main(void)
{
	long max_us = execute_syscalls() * MAX_TIME_RATIO;

	pidns_test_init();

	long us = execute_syscalls();
	if (us > max_us)
		error_msg_and_fail("pidns translation took too long: %ld us "
		                   "(max: %ld us)", us, max_us);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpid")

#endif
