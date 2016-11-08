/*
 * Copyright (c) 2015 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * This test burns some CPU cycles in user space and kernel space in order to
 * get some non-zero values returned by times(2).
 */

#include "tests.h"
#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <asm/unistd.h>
#include <sys/times.h>
#include <sys/wait.h>

enum {
	NUM_USER_ITERS = 1000000,
	PARENT_CPUTIME_LIMIT_NSEC = 200000000,
	CHILD_CPUTIME_LIMIT_NSEC = 300000000
};

int
main (void)
{
	struct timespec ts;
	volatile int dummy = 0;
	int i = 0;

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	const long cputime_limit =
		pid ? PARENT_CPUTIME_LIMIT_NSEC : CHILD_CPUTIME_LIMIT_NSEC;

	/* Enjoying my user time */
	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		if (ts.tv_sec || ts.tv_nsec >= cputime_limit)
			break;

		if (i && !(ts.tv_sec || ts.tv_nsec))
			error_msg_and_skip("clock_gettime(CLOCK_PROCESS_CPUTIME_ID, {0, 0})");

		for (i = 0; i < NUM_USER_ITERS; ++i)
			++dummy;
	}

	/* Enjoying my system time */
	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		if (ts.tv_sec || ts.tv_nsec >= cputime_limit * 2)
			break;

		sched_yield();
	}

	if (pid == 0) {
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
#undef USE_LIBC_SYSCALL
#if defined __NR_times && \
   !defined(LINUX_MIPSN32) && \
   !(defined __x86_64__ && defined __ILP32__)
# define USE_LIBC_SYSCALL 1
#endif

#if defined USE_LIBC_SYSCALL
	long res = syscall(__NR_times, &tbuf);

	if (-1L == res)
		perror_msg_and_skip("times");
	else
		llres = (unsigned long) res;
#elif defined __NR_times && defined __x86_64__ && defined __ILP32__
	register long arg asm("rdi") = (long) &tbuf;
	asm volatile("syscall\n\t"
		     : "=a"(llres)
		     : "0"(__NR_times), "r"(arg)
		     : "memory", "cc", "r11", "cx");
	if (llres > 0xfffffffffffff000)
		return 77;
#else
	clock_t res = times(&tbuf);

	if ((clock_t) -1 == res)
		perror_msg_and_skip("times");
	if (sizeof(res) < sizeof(unsigned long long))
		llres = (unsigned long) res;
	else
		llres = res;
#endif

	printf("times({tms_utime=%llu, tms_stime=%llu, ",
		(unsigned long long) tbuf.tms_utime,
		(unsigned long long) tbuf.tms_stime);
	printf("tms_cutime=%llu, tms_cstime=%llu}) = %llu\n",
		(unsigned long long) tbuf.tms_cutime,
		(unsigned long long) tbuf.tms_cstime,
		llres);
	puts("+++ exited with 0 +++");

	return 0;
}
