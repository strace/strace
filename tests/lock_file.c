/*
 * Copyright (c) 2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

/* Obtain an exclusive lock on dirname(path_name)/lock_name file.  */
int
lock_file_by_dirname(const char *path_name, const char *lock_name)
{
	const char *slash = path_name ? strrchr(path_name, '/') : NULL;
	const int plen = slash ? (int) (slash - path_name) + 1 : 0;

	char *lock_file = NULL;
	if (asprintf(&lock_file, "%.*s%s", plen, path_name, lock_name) < 0)
		perror_msg_and_fail("asprintf");

	int lock_fd = open(lock_file, O_RDONLY);
	if (lock_fd < 0)
		perror_msg_and_fail("open: %s", lock_file);

	if (flock(lock_fd, LOCK_EX))
		perror_msg_and_fail("flock: %s", lock_file);

	free(lock_file);

	return lock_fd;
}
