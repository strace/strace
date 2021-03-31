/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "xmalloc.h"

int
get_dir_fd(const char *dir_path)
{
	DIR *dir = opendir(dir_path);
	if (dir == NULL)
		perror_msg_and_fail("opendir: %s", dir_path);
	int dfd = dirfd(dir);
	if (dfd == -1)
		perror_msg_and_fail("dirfd");
	return dfd;
}

char *
get_fd_path(int fd)
{
	char *proc = xasprintf("/proc/self/fd/%u", fd);
	char *buf = xmalloc(PATH_MAX);
	ssize_t n = readlink(proc, buf, PATH_MAX);
	if (n < 0)
		perror_msg_and_skip("readlink: %s", proc);
	if (n >= PATH_MAX)
		error_msg_and_fail("readlink: %s: %s", proc,
				   "symlink value is too long");
	buf[n] = '\0';
	free(proc);
	return buf;
}
