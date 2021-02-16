/*
 * This file is part of strace test suite.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned long
inode_of_sockfd(const int fd)
{
	assert(fd >= 0);

	char linkpath[sizeof("/proc/self/fd/%u") + sizeof(int) * 3];
	assert(snprintf(linkpath, sizeof(linkpath), "/proc/self/fd/%u", fd)
	       < (int) sizeof(linkpath));

	char path[PATH_MAX + 1];
	const ssize_t path_len = readlink(linkpath, path, sizeof(path) - 1);
	if (path_len < 0)
		perror_msg_and_fail("readlink: %s", linkpath);
	path[path_len] = '\0';

	static const char prefix[] = "socket:[";
	const size_t prefix_len = sizeof(prefix) - 1;
	assert(strncmp(path, prefix, prefix_len) == 0
	       && path[path_len - 1] == ']');

	return strtoul(path + prefix_len, NULL, 10);
}
