/*
 * Check delay injection.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
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

static bool
check_expected(const int64_t got, const bool ge, const int64_t exp_delay,
	       const int nproc)
{
	const int64_t threshold =
		exp_delay * (ge ? (nproc - 1) : (nproc + 1)) / nproc;

	if (ge ? (got >= threshold) : (got <= threshold))
		return true;

	fprintf(stderr,
		"Got delay of %" PRId64 ", %s than threshold value of"
		" %" PRId64 " (expected nominal delay value is %" PRId64 ")\n",
		got, ge ? "less" : "more", threshold, exp_delay);

	return false;
}

static int
check_expectations(const int64_t got, const int64_t exp_delay, const int nproc,
		   const int rc_short, const int rc_long)
{
	if (!check_expected(got, true, exp_delay, nproc))
		return rc_short;
	if (!check_expected(got, false, exp_delay, nproc))
		return rc_long;
	return 0;
}

enum {
	BAD_OTHER = 2,
	DELAY_ENTER_TOO_SHORT = 4,
	DELAY_EXIT_TOO_SHORT = 8,
	DELAY_ENTER_TOO_LONG = 16,
	DELAY_EXIT_TOO_LONG = 32,
	MASK_DELAY_TOO_LONG = DELAY_ENTER_TOO_LONG | DELAY_EXIT_TOO_LONG,
};

static int
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
	int rc = 0;

	rc |= check_expectations(us - us0, delay_exit, nproc,
				 DELAY_EXIT_TOO_SHORT, DELAY_EXIT_TOO_LONG);
	rc |= check_expectations(us1 - us, delay_enter, nproc,
				 DELAY_ENTER_TOO_SHORT, DELAY_ENTER_TOO_LONG);

	return rc;
}

static void
do_child(const int nproc, const int delay_enter, const int delay_exit)
{
	kernel_old_timeval_t prev = { 0, 0 }, now;
	struct timespec ts;
	int rc = 0;

	for (int i = 0; i < nproc; ++i, prev = now) {
		if (syscall(__NR_gettimeofday, &now, NULL))
			perror_msg_and_fail("gettimeofday");

		if (i)
			rc |= check_delay(&prev, &ts, &now, nproc,
					  delay_enter, delay_exit);

		if (clock_gettime(CLOCK_REALTIME, &ts))
			perror_msg_and_fail("clock_gettime");
	}

	_exit(rc);
}

static int
run_checks(const int nproc, const int delay_enter, const int delay_exit)
{
	int rc = 0;
	pid_t pid;

	for (int i = 0; i < nproc; ++i) {
		pid = fork();

		if (pid < 0)
			perror_msg_and_fail("fork");
		if (pid)
			usleep(MAX(delay_enter, delay_exit) / nproc);
		else
			do_child(nproc, delay_enter, delay_exit);
	}

	int status;
	while ((pid = wait(&status)) > 0) {
		if (status == 0)
			continue;
		if (WIFEXITED(status)) {
			int es = WEXITSTATUS(status);

			fprintf(stderr, "pid %d exit status %d", pid, es);
			rc |= es;
		} else {
			fprintf(stderr, "pid %d wait status %#x", pid, status);
			rc |= BAD_OTHER;
		}
	}

	if (errno != ECHILD)
		perror_msg_and_fail("wait");

	return rc;
}

int
main(int ac, char *av[])
{
	int rc;

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
		rc = run_checks(nproc, delay_enter, delay_exit);
		if (rc == 0) {
			if (i)
				fprintf(stderr, "attempt #%d passed\n", i + 1);
			break;
		}
		if (rc & ~MASK_DELAY_TOO_LONG)
			break;
		fprintf(stderr, "attempt #%d failed\n", i + 1);
	}

	return rc;
}
