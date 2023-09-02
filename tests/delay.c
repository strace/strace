/*
 * Check delay injection.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include "scno.h"
#include "kernel_timeval.h"

static int64_t
usecs_from_tv(const kernel_old_timeval_t *const tv)
{
    return (int64_t) tv->tv_sec * 1000000 + tv->tv_usec;
}

static int64_t
usecs_from_ts(const struct timespec *const ts)
{
    return (int64_t) ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
}

static void
check_(const int64_t got, const bool ge, const int64_t orig, const int nproc,
       const int exitcode)
{
	const int64_t thresh = (orig * (ge ? nproc - 1 : nproc + 1)) / nproc;

	if (ge ? got >= thresh : got <= thresh)
		return;

	fprintf(stderr, "Got delay of %" PRId64 ", %s than threshold value of "
			"%" PRId64 " (expected nominal delay value is %" PRId64
			")\n", got, ge ? "less" : "more", thresh, orig);

	_exit(exitcode);
}

static void
check_delay(const kernel_old_timeval_t *const tv0,
	    const struct timespec *const ts,
	    const kernel_old_timeval_t *const tv1,
	    const int nproc,
	    const int64_t delay_enter,
	    const int64_t delay_exit)
{
	const int64_t us0 = usecs_from_tv(tv0);
	const int64_t us  = usecs_from_ts(ts);
	const int64_t us1 = usecs_from_tv(tv1);

	check_(us - us0, true,  delay_exit,  nproc, 1);
	check_(us - us0, false, delay_exit,  nproc, 2);
	check_(us1 - us, true,  delay_enter, nproc, 3);
	check_(us1 - us, false, delay_enter, nproc, 4);
}

static void
run(const int nproc, const int delay_enter, const int delay_exit)
{
	kernel_old_timeval_t prev = { 0, 0 }, now;

	for (int i = 0; i < nproc; ++i, prev = now) {
		struct timespec ts;

		if (i && clock_gettime(CLOCK_REALTIME, &ts))
			perror_msg_and_fail("clock_gettime");

		if (syscall(__NR_gettimeofday, &now, NULL))
			perror_msg_and_fail("gettimeofday");

		if (!i)
			continue;

		check_delay(&prev, &ts, &now, nproc, delay_enter, delay_exit);
	}

	_exit(0);
}

int
main(int ac, char *av[])
{
	if (ac != 4)
		error_msg_and_fail("usage: delay <nproc> <delay_enter> <delay_exit>");

	const int nproc = atoi(av[1]);
	if (nproc <= 1)
		perror_msg_and_fail("invalid nproc: %s", av[1]);

	const int delay_enter = atoi(av[2]);
	if (delay_enter <= 0)
		perror_msg_and_fail("invalid delay_enter: %s", av[2]);

	const int delay_exit = atoi(av[3]);
	if (delay_exit <= 0)
		perror_msg_and_fail("invalid delay_exit: %s", av[3]);

	for (int i = 0; i < nproc; ++i) {
		pid_t pid = fork();

		if (pid)
			usleep(MAX(delay_enter, delay_exit) / nproc);
		else
			run(nproc, delay_enter, delay_exit);
	}

	int status;
	while (wait(&status) > 0) {
		if (status)
			perror_msg_and_fail("wait status %d", status);
	}

	return 0;
}
