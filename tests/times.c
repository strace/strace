/*
 * Check decoding of times syscall.
 *
 * Copyright (c) 2015-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * @file
 * This test burns some CPU cycles in user space and kernel space in order to
 * get some non-zero values returned by times(2).
 */

#include "tests.h"
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "scno.h"
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "time_enjoyment.h"

enum {
	PARENT_CPUTIME_LIMIT_NSEC = 300000000,
	CHILD_CPUTIME_LIMIT_NSEC = 500000000,
};

static char*
time_comment(int precision, clock_t val, long clk_tck)
{
	const size_t comment_size = 20;
	char *comment_buf = malloc(comment_size);
	if (val == 0) {
		comment_buf[0] = '\0';
		return comment_buf;
	}
	snprintf(comment_buf, comment_size, " /* %.*f s */",
	         precision, (double) val / clk_tck);
	return comment_buf;
}

int
main(void)
{
	enjoy_time(PARENT_CPUTIME_LIMIT_NSEC);

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (pid == 0) {
		enjoy_time(CHILD_CPUTIME_LIMIT_NSEC);

		return 0;
	} else {
		wait(NULL);
	}

	struct tms tbuf;
	unsigned long long llres;

	/*
	 * On systems where user's and kernel's long types are the same,
	 * prefer direct times syscall over libc's times function because
	 * the latter is more prone to return value truncation.
	 */

#if defined __x86_64__ && defined __ILP32__
	register long arg asm("rdi") = (long) &tbuf;
	asm volatile("syscall\n\t"
		     : "=a"(llres)
		     : "0"(__NR_times), "r"(arg)
		     : "memory", "cc", "r11", "cx");
	if (llres > 0xfffffffffffff000)
		return 77;
#elif defined __s390__ || defined LINUX_MIPSN32
	clock_t res = times(&tbuf);

	if ((clock_t) -1 == res)
		perror_msg_and_skip("times");
	if (sizeof(res) < sizeof(unsigned long long))
		llres = (unsigned long) res;
	else
		llres = res;
#else
	long res = syscall(__NR_times, &tbuf);

	if (-1L == res)
		perror_msg_and_skip("times");
	else
		llres = (unsigned long) res;
#endif

	long clk_tck = sysconf(_SC_CLK_TCK);
	int precision = clk_tck > 100000000 ? 9
			: clk_tck > 10000000 ? 8
			: clk_tck > 1000000 ? 7
			: clk_tck > 100000 ? 6
			: clk_tck > 10000 ? 5
			: clk_tck > 1000 ? 4
			: clk_tck > 100 ? 3
			: clk_tck > 10 ? 2
			: clk_tck > 1 ? 1 : 0;

	if (!XLAT_RAW && clk_tck > 0) {
		char *utime_comment = time_comment(precision, tbuf.tms_utime, clk_tck);
		char *stime_comment = time_comment(precision, tbuf.tms_stime, clk_tck);
		char *cutime_comment = time_comment(precision, tbuf.tms_cutime, clk_tck);
		char *cstime_comment = time_comment(precision, tbuf.tms_cstime, clk_tck);
		printf("times({tms_utime=%llu%s"
		       ", tms_stime=%llu%s"
		       ", tms_cutime=%llu%s"
		       ", tms_cstime=%llu%s}) = %llu\n",
		       (unsigned long long) tbuf.tms_utime,
		       utime_comment,
		       (unsigned long long) tbuf.tms_stime,
		       stime_comment,
		       (unsigned long long) tbuf.tms_cutime,
		       cutime_comment,
		       (unsigned long long) tbuf.tms_cstime,
		       cstime_comment,
		       llres);
		free(utime_comment);
		free(stime_comment);
		free(cutime_comment);
		free(cstime_comment);
	} else {
		printf("times({tms_utime=%llu, tms_stime=%llu"
		       ", tms_cutime=%llu, tms_cstime=%llu}) = %llu\n",
		       (unsigned long long) tbuf.tms_utime,
		       (unsigned long long) tbuf.tms_stime,
		       (unsigned long long) tbuf.tms_cutime,
		       (unsigned long long) tbuf.tms_cstime,
		       llres);
	}

	puts("+++ exited with 0 +++");

	return 0;
}
