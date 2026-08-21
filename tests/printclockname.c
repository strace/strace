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

	puts("+++ exited with 0 +++");
	return 0;
}
