/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "xmalloc.h"

int
get_dir_fd(const char *dir_path)
{
	DIR *dir = NULL;
	dir = opendir(dir_path);
	if (dir == NULL)
		perror_msg_and_fail("opendir");
	int dfd = dirfd(dir);
	if (dfd == -1)
		perror_msg_and_fail("dirfd");
	return dfd;
}

int
get_curdir_fd(char **curdir)
{
	int res = get_dir_fd(".");

	if (curdir != NULL) {
		char *buf = xmalloc(PATH_MAX);
		ssize_t n = readlink("/proc/self/cwd", buf, PATH_MAX);
		if (n == -1)
			perror_msg_and_skip("/proc is not available");
		if (n >= PATH_MAX)
			perror_msg_and_fail("buffer is too small");
		buf[n] = '\0';
		*curdir = buf;
	}

	return res;
}
