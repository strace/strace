/*
 * Check appending auxstr of setns(2) when --namespace=new is given.
 *
 * Copyright (c) 2024-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "setns-report-ns-id-common.h"

#include <linux/sched.h>

int
main(void)
{
	/*
	 * Try a user namespace for CAP_SYS_ADMIN
	 * before creating a network namespace.
	 */
	(void) syscall(__NR_unshare, CLONE_NEWUSER);

	/* Save one network namespace, then enter another. */
	char netns[PATH_MAX + 1];
	int netns_fd = open_namespace_pair("/proc/self/ns/net",
					   ARG_STR(CLONE_NEWNET),
					   netns, sizeof(netns));

	if (setns_and_print_expected(netns_fd, ARG_STR(CLONE_NEWNET), netns) < 0)
		perror_msg_and_skip("setns(CLONE_NEWNET)");

	puts("+++ exited with 0 +++");

	return 0;
}
