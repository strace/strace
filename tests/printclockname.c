/*
 * This file is part of clock_xettime* strace tests.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <unistd.h>

#include <linux/time.h>
#include <linux/unistd.h>

#include "tests.h"

#define CLOCKFD 3
#define FD_TO_CLOCKID(fd)   ((~(unsigned int)(clockid_t) (fd) << 3) | CLOCKFD)

#define CPUCLOCK_PERTHREAD_MASK	4
#define CPUCLOCK_PROF		0
#define CPUCLOCK_VIRT		1
#define CPUCLOCK_SCHED		2
#define MAKE_PROCESS_CPUCLOCK(pid, clock) \
	((~(unsigned int)(clockid_t) (pid) << 3) | (clockid_t) (clock))
#define MAKE_THREAD_CPUCLOCK(tid, clock) \
	MAKE_PROCESS_CPUCLOCK((tid), (clock) | CPUCLOCK_PERTHREAD_MASK)

int
main(void)
{
	syscall(__NR_clock_getres, CLOCK_REALTIME, NULL);
#if XLAT_RAW
	printf("clock_getres(0, NULL)                   = 0\n");
#elif XLAT_VERBOSE
	printf("clock_getres(0 /* CLOCK_REALTIME */, NULL) = 0\n");
#else
	printf("clock_getres(CLOCK_REALTIME, NULL)      = 0\n");
#endif

	syscall(__NR_clock_getres, CLOCK_MONOTONIC, NULL);
#if XLAT_RAW
	printf("clock_getres(0x1, NULL)                 = 0\n");
#elif XLAT_VERBOSE
	printf("clock_getres(0x1 /* CLOCK_MONOTONIC */, NULL) = 0\n");
#else
	printf("clock_getres(CLOCK_MONOTONIC, NULL)     = 0\n");
#endif

	syscall(__NR_clock_getres, CLOCK_PROCESS_CPUTIME_ID, NULL);
#if XLAT_RAW
	printf("clock_getres(0x2, NULL)                 = 0\n");
#elif XLAT_VERBOSE
	printf("clock_getres(0x2 /* CLOCK_PROCESS_CPUTIME_ID */, NULL) = 0\n");
#else
	printf("clock_getres(CLOCK_PROCESS_CPUTIME_ID, NULL) = 0\n");
#endif

	syscall(__NR_clock_getres, CLOCK_THREAD_CPUTIME_ID, NULL);
#if XLAT_RAW
	printf("clock_getres(0x3, NULL)                 = 0\n");
#elif XLAT_VERBOSE
	printf("clock_getres(0x3 /* CLOCK_THREAD_CPUTIME_ID */, NULL) = 0\n");
#else
	printf("clock_getres(CLOCK_THREAD_CPUTIME_ID, NULL) = 0\n");
#endif

	syscall(__NR_clock_getres, FD_TO_CLOCKID(0), NULL);
#if XLAT_RAW
	printf("clock_getres(-5, NULL)                  = -1 EINVAL (Invalid argument)\n");
#elif XLAT_VERBOSE
	printf("clock_getres(-5 /* FD_TO_CLOCKID(0) */, NULL) = -1 EINVAL (Invalid argument)\n");
#else
	printf("clock_getres(FD_TO_CLOCKID(0), NULL)    = -1 EINVAL (Invalid argument)\n");
#endif

	syscall(__NR_clock_getres, FD_TO_CLOCKID(2), NULL);
#if XLAT_RAW
	printf("clock_getres(-21, NULL)                 = -1 EINVAL (Invalid argument)\n");
#elif XLAT_VERBOSE
	printf("clock_getres(-21 /* FD_TO_CLOCKID(2) */, NULL) = -1 EINVAL (Invalid argument)\n");
#else
	printf("clock_getres(FD_TO_CLOCKID(2), NULL)    = -1 EINVAL (Invalid argument)\n");
#endif

	syscall(__NR_clock_getres, MAKE_PROCESS_CPUCLOCK(1, CPUCLOCK_VIRT), NULL);
#if XLAT_RAW
	printf("clock_getres(-15, NULL)                 = 0\n");
#elif XLAT_VERBOSE
	printf("clock_getres(-15 /* MAKE_PROCESS_CPUCLOCK(1, 0x1 /* CPUCLOCK_VIRT */) */, NULL) = 0\n");
#else
	printf("clock_getres(MAKE_PROCESS_CPUCLOCK(1, CPUCLOCK_VIRT), NULL) = 0\n");
#endif

	syscall(__NR_clock_getres, MAKE_THREAD_CPUCLOCK(1, CPUCLOCK_SCHED), NULL);
#if XLAT_RAW
	printf("clock_getres(-10, NULL)                 = -1 EINVAL (Invalid argument)\n");
#elif XLAT_VERBOSE
	printf("clock_getres(-10 /* MAKE_THREAD_CPUCLOCK(1, 0x2 /* CPUCLOCK_SCHED */) */, NULL) = -1 EINVAL (Invalid argument)\n");
#else
	printf("clock_getres(MAKE_THREAD_CPUCLOCK(1, CPUCLOCK_SCHED), NULL) = -1 EINVAL (Invalid argument)\n");
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
