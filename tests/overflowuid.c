/*
 * Copyright (c) 2014-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int
read_int_from_file(const char *const fname, int *const pvalue)
{
	const int fd = open(fname, O_RDONLY);
	if (fd < 0)
		return -1;

	long lval;
	char buf[sizeof(lval) * 3];
	int n = read(fd, buf, sizeof(buf) - 1);
	int saved_errno = errno;
	close(fd);

	if (n < 0) {
		errno = saved_errno;
		return -1;
	}

	buf[n] = '\0';
	char *endptr = 0;
	errno = 0;
	lval = strtol(buf, &endptr, 10);
	if (!endptr || (*endptr && '\n' != *endptr)
#if INT_MAX < LONG_MAX
	    || lval > INT_MAX || lval < INT_MIN
#endif
	    || ERANGE == errno) {
		if (!errno)
			errno = EINVAL;
		return -1;
	}

	*pvalue = (int) lval;
	return 0;
}

static void
check_overflow_id(const int id, const char *overflowid)
{
	int n;

	if (read_int_from_file(overflowid, &n)) {
		if (ENOENT == errno)
			return;
		perror_msg_and_fail("read_int_from_file: %s", overflowid);
	}

	if (id == n)
		error_msg_and_skip("%d matches %s", id, overflowid);
}

void
check_overflowuid(const int uid)
{
	check_overflow_id(uid, "/proc/sys/kernel/overflowuid");
}

void
check_overflowgid(const int gid)
{
	check_overflow_id(gid, "/proc/sys/kernel/overflowgid");
}
