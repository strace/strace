/*
 * Common helpers for setns namespace ID tests.
 *
 * Copyright (c) 2024-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_SETNS_REPORT_NS_ID_COMMON_H
# define STRACE_TESTS_SETNS_REPORT_NS_ID_COMMON_H

# include "tests.h"
# include "scno.h"

# include <fcntl.h>
# include <limits.h>
# include <stdio.h>
# include <unistd.h>

static inline void
read_proc_self_fd_link(int fd, char *ns, size_t size)
{
	char path[sizeof("/proc/self/fd/%d") + sizeof(int) * 3];
	snprintf(path, sizeof(path), "/proc/self/fd/%d", fd);
	ssize_t n = readlink(path, ns, size);
	if (n < 0)
		perror_msg_and_fail("readlink: %s", path);
	if ((size_t) n >= size)
		error_msg_and_fail("readlink: %s: result is too large", path);
	ns[n] = '\0';
}

/*
 * Create namespace A of the requested type, open its /proc/self/ns link,
 * and save its ID in ns.  Then create namespace B of the same type and
 * return A's fd.  The caller can now setns into A from B and compare the
 * resulting annotation with the saved ID.  flag_str names flag in skip
 * diagnostics; path identifies the namespace link to open.
 */
static inline int
open_namespace_pair(const char *path, unsigned int flag,
		    const char *flag_str, char *ns, size_t size)
{
	skip_if_unavailable(path);
	if (syscall(__NR_unshare, flag) < 0)
		perror_msg_and_skip("unshare(%s)", flag_str);

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		perror_msg_and_skip("open(%s)", path);
	/* Read through the fd before switching away from namespace A. */
	read_proc_self_fd_link(fd, ns, size);

	/* Enter B so setns(fd, ...) actually switches back to A. */
	if (syscall(__NR_unshare, flag) < 0)
		perror_msg_and_skip("unshare(%s)", flag_str);
	return fd;
}

static inline long
setns_and_print_expected(int fd, unsigned int flag, const char *flag_str,
			 const char *ns)
{
	long rc = syscall(__NR_setns, fd, flag);
	if (rc == 0)
		printf("setns(%d, %s) = %s (%s)\n",
		       fd, flag_str, sprintrc(rc), ns);
	return rc;
}

#endif /* STRACE_TESTS_SETNS_REPORT_NS_ID_COMMON_H */
