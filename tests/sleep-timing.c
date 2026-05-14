/*
 * nanosleep based sleep(1) with monotonic timing measurement.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_nanosleep

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>

# include "kernel_old_timespec.h"

static void
timespec_sub(struct timespec *res, const struct timespec *a,
	     const struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (res->tv_nsec < 0) {
		res->tv_sec--;
		res->tv_nsec += 1000000000L;
	}
}

static double
timespec_to_sec(const struct timespec *ts)
{
	return (double) ts->tv_sec + (double) ts->tv_nsec / 1e9;
}

static void
write_timing_file(const char *path, int requested, double nanosleep_sec)
{
	FILE *fp = fopen(path, "w");

	if (!fp)
		perror_msg_and_fail("fopen: %s", path);

	if (fprintf(fp,
		    "requested_sec=%d\n"
		    "nanosleep_sec=%.9f\n",
		    requested, nanosleep_sec) < 0)
		perror_msg_and_fail("fprintf: %s", path);

	if (fclose(fp))
		perror_msg_and_fail("fclose: %s", path);
}

int
main(int ac, char **av)
{
	struct timespec t0;
	struct timespec t1;
	struct timespec delta;
	const char *timing_file;
	double nanosleep_sec;
	int requested_sec;

	if (ac < 3)
		error_msg_and_fail("missing operand");

	if (ac > 3)
		error_msg_and_fail("extra operand");

	requested_sec = atoi(av[1]);
	timing_file = av[2];

	kernel_old_timespec_t ts = { requested_sec, 0 };

	if (!chdir(""))
		error_msg_and_fail("chdir");

	if (clock_gettime(CLOCK_MONOTONIC, &t0))
		perror_msg_and_fail("clock_gettime");

	if (syscall(__NR_nanosleep, (unsigned long) &ts, 0))
		perror_msg_and_fail("nanosleep");

	if (clock_gettime(CLOCK_MONOTONIC, &t1))
		perror_msg_and_fail("clock_gettime");

	if (chdir("."))
		perror_msg_and_fail("chdir");

	timespec_sub(&delta, &t1, &t0);
	nanosleep_sec = timespec_to_sec(&delta);

	write_timing_file(timing_file, requested_sec, nanosleep_sec);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_nanosleep")

#endif
