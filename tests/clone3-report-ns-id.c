/*
 * Check appending auxstr of clone3(2) when --namespace=switchTo is given
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"

#include <limits.h>
#include <linux/sched.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "scno.h"

int
main(void)
{
	struct clone_args arg = { .flags = CLONE_NEWUSER };
	int pid = syscall(__NR_clone3, &arg, sizeof(arg));
	if (pid < 0)
		perror_msg_and_fail("clone3");
	if (pid > 0) {
		printf("clone3({flags=CLONE_NEWUSER, exit_signal=0, stack=NULL, stack_size=0}"
		       ", %zu) = %s ", sizeof(arg), sprintrc(pid));

		char userns[PATH_MAX + 1];
		char *fname = xasprintf("/proc/%d/ns/user", pid);
		int rc = readlink(fname, userns, sizeof(userns));
		if ((size_t) rc >= sizeof(userns))
			perror_msg_and_fail("readlink");
		userns[rc] = '\0';
		printf("(%s)\n", userns);
		puts("+++ exited with 0 +++");
	}
	return 0;
}
