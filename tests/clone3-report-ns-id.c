/*
 * Check appending auxstr of clone3(2) when --namespace=new is given.
 *
 * Copyright (c) 2024-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/sched.h>

#include "scno.h"

int
main(void)
{
	skip_if_unavailable("/proc/self/ns/user");

	struct clone_args arg = { .flags = CLONE_NEWUSER };
	int pid = syscall(__NR_clone3, &arg, sizeof(arg));
	if (pid < 0)
		perror_msg_and_skip("clone3");
	if (pid > 0) {
		char userns[PATH_MAX + 1];
		char *fname = xasprintf("/proc/%d/ns/user", pid);
		ssize_t rc = readlink(fname, userns, sizeof(userns));
		if (rc < 0)
			perror_msg_and_fail("readlink: %s", fname);
		if ((size_t) rc >= sizeof(userns))
			error_msg_and_fail("readlink: %s: result is too long", fname);
		userns[rc] = '\0';
		printf("clone3({flags=CLONE_NEWUSER, exit_signal=0"
		       ", stack=NULL, stack_size=0}, %zu) = %s (%s)\n",
		       sizeof(arg), sprintrc(pid), userns);
		puts("+++ exited with 0 +++");
	}
	return 0;
}
