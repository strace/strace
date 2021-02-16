/*
 * Check decoding of utime syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_utime

# include <time.h>
# include <utime.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

static long
k_utime(const void *const filename, const void *const times)
{
	return syscall(__NR_utime, filename, times);
}

int
main(void)
{
	static const char *const dummy_str = "dummy filename";

	const time_t t = 1492350678;
	const struct utimbuf u = { .actime = t, .modtime = t };
	const struct utimbuf *const tail_u = tail_memdup(&u, sizeof(u));
	const char *const dummy_filename =
		tail_memdup(dummy_str, sizeof(dummy_str) - 1);

	long rc = k_utime("", NULL);
	printf("utime(\"\", NULL) = %s\n", sprintrc(rc));

	rc = k_utime(dummy_filename + sizeof(dummy_str), tail_u + 1);
	printf("utime(%p, %p) = %s\n", dummy_filename + sizeof(dummy_str),
	       tail_u + 1, sprintrc(rc));

	rc = k_utime(dummy_filename, (struct tm *) tail_u + 1);
	printf("utime(%p, %p) = %s\n",
	       dummy_filename, (struct tm *) tail_u + 1, sprintrc(rc));

	rc = k_utime("utime\nfilename", tail_u);
	const char *errstr = sprintrc(rc);
	printf("utime(\"utime\\nfilename\", {actime=%lld", (long long) t);
	print_time_t_nsec(t, 0, 1);
	printf(", modtime=%lld", (long long) t);
	print_time_t_nsec(t, 0, 1);
	printf("}) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_utime")

#endif
