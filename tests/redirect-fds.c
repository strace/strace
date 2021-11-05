/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>

#define N_FDS 3

/*
 * Do not print any messages, indicate errors with return codes.
 */
static int
check_fd(int fd, const char *fname)
{
	const int should_be_closed = (fname[0] == '\0');

	struct stat st_fd, st_fn;

	if (fstat(fd, &st_fd)) {
		if (!should_be_closed)
			return 10 + fd;
	} else {
		if (should_be_closed)
			return 20 + fd;

		if (stat(fname, &st_fn))
			return 30 + fd;

		if (st_fd.st_dev != st_fn.st_dev
		    || st_fd.st_ino != st_fn.st_ino)
			return 40 + fd;
	}

	return 0;
}

int
main(int ac, char **av)
{
	assert(ac == 1 + N_FDS);

	int rc = 0;
	for (int fd = 1; fd < 1 + N_FDS; ++fd)
		if ((rc = check_fd(fd - 1, av[fd])))
			break;

	return rc;
}
