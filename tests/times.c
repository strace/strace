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

#include <sys/times.h>
#include <sys/wait.h>

enum {
	NUM_USER_ITERS = 1000000,
	CPUTIME_LIMIT_SEC = 2,
};

int
main (void)
{
	struct tms tbuf;
	struct timespec ts;
	clock_t res;
	unsigned long long llres;
	volatile int dummy;
	pid_t pid;
	int i;

	pid = fork();

	if (pid < 0)
		return 77;

	/* Enjoying my user time */
	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		if (ts.tv_sec >= CPUTIME_LIMIT_SEC)
			break;

		for (i = 0; i < NUM_USER_ITERS; i++, dummy++)
			;
	}

	/* Enjoying my system time */
	while (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0) {
		if (ts.tv_sec >= (CPUTIME_LIMIT_SEC * 2))
			break;

		sched_yield();
	}

	if (pid == 0) {
		return 0;
	} else {
		wait(NULL);
	}

	res = times(&tbuf);

	if (res == (clock_t) -1)
		return 77;

	if (sizeof(llres) > sizeof(res))
		llres = (unsigned long) res;
	else
		llres = res;

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
