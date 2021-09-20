/*
 * Check decoding of open_tree syscall.
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "kernel_fcntl.h"

static const char *errstr;

static long
k_open_tree(const unsigned int dfd,
	    const void *fname,
	    const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | dfd;
	const kernel_ulong_t arg2 = (uintptr_t) fname;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_open_tree, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

#ifndef PATH_TRACING
	char *cwd = get_fd_path(get_dir_fd("."));
#endif
	static const char path_full[] = "/dev/full";
	const char *const path = tail_memdup(path_full, sizeof(path_full));
	char *const fname = tail_alloc(PATH_MAX);
	const void *const efault = fname + PATH_MAX;
	const char *const empty = efault - 1;
	fill_memory_ex(fname, PATH_MAX, '0', 10);

        int dfd = open(path, O_WRONLY);
        if (dfd < 0)
                perror_msg_and_fail("open: %s", path);

	k_open_tree(-1, 0, 1);
#ifndef PATH_TRACING
	printf("open_tree(-1, NULL, %s) = %s\n", "OPEN_TREE_CLONE", errstr);
#endif

	k_open_tree(-100, fname, 0);
#ifndef PATH_TRACING
	printf("open_tree(AT_FDCWD<%s>, \"%.*s\"..., 0) = %s\n",
	       cwd, (int) PATH_MAX - 1, fname, errstr);
#endif

	fname[PATH_MAX - 1] = '\0';
	k_open_tree(dfd, fname, 0x8000);
	printf("open_tree(%d<%s>, \"%s\", %s) = %s\n",
	       dfd, path, fname, "AT_RECURSIVE", errstr);

	k_open_tree(-1, efault, O_CLOEXEC | 1);
#ifndef PATH_TRACING
	printf("open_tree(-1, %p, %s) = %s\n",
	       efault, "OPEN_TREE_CLONE|OPEN_TREE_CLOEXEC", errstr);
#endif

	k_open_tree(-1, empty, -1);
#ifndef PATH_TRACING
	printf("open_tree(-1, \"\", %s|%#x) = %s\n",
	       "OPEN_TREE_CLONE|OPEN_TREE_CLOEXEC"
	       "|AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW"
	       "|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE",
	       -1U & ~0x9f01 & ~O_CLOEXEC,
	       errstr);
#endif

	if (k_open_tree(-1, path, 0) < 0)
		printf("open_tree(-1, \"%s\", 0) = %s\n",
		       path, errstr);
	else
		printf("open_tree(-1, \"%s\", 0) = %s<%s>\n",
		       path, errstr, path);

	if (k_open_tree(dfd, empty, 0x1000) < 0)
		printf("open_tree(%d<%s>, \"\", %s) = %s\n",
		       dfd, path, "AT_EMPTY_PATH", errstr);
	else
		printf("open_tree(%d<%s>, \"\", %s) = %s<%s>\n",
		       dfd, path, "AT_EMPTY_PATH", errstr, path);

	puts("+++ exited with 0 +++");
	return 0;
}
