/*
 * Check setns(2) namespace IDs with explicit and zero nstype.
 *
 * Copyright (c) 2024-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "setns-report-ns-id-common.h"
#include "xmalloc.h"

#include <stdlib.h>

#include <linux/sched.h>

int
main(void)
{
	/* Try a user namespace for CAP_SYS_ADMIN before creating the pair. */
	(void) syscall(__NR_unshare, CLONE_NEWUSER);

	char *path = xasprintf("/proc/self/ns/%s", TEST_NS_NAME);
	char ns[PATH_MAX + 1];
	int fd = open_namespace_pair(path, TEST_NS_FLAG, TEST_NS_FLAG_STR,
				     ns, sizeof(ns));
	free(path);

	if (setns_and_print_expected(fd, TEST_NS_FLAG, TEST_NS_FLAG_STR, ns) < 0)
		perror_msg_and_skip("setns(%s)", TEST_NS_FLAG_STR);

	if (syscall(__NR_unshare, TEST_NS_FLAG) < 0)
		perror_msg_and_skip("unshare(%s)", TEST_NS_FLAG_STR);

	if (setns_and_print_expected(fd, ARG_STR(0), ns) < 0)
		perror_msg_and_skip("setns(0)");
	close(fd);

	puts("+++ exited with 0 +++");

	return 0;
}
