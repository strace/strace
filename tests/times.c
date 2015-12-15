/**
 * @file
 * This test burns some CPU cycles in user space and kernel space in order to
 * get some non-zero values returned by times(2).
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sched.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <sys/syscall.h>
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
	volatile int dummy;
	int i;

	pid_t pid = fork();

	if (pid < 0)
		return 77;

	const long cputime_limit =
		pid ? PARENT_CPUTIME_LIMIT_NSEC : CHILD_CPUTIME_LIMIT_NSEC;

	/* Enjoying my user time */
	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		if (ts.tv_sec || ts.tv_nsec >= cputime_limit)
			break;

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
		return 77;
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
		return 77;
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
