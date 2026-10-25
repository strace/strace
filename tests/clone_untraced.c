/*
 * Check handling of CLONE_UNTRACED under --seccomp-bpf.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/sched.h>

static int
child(void *arg)
{
	syscall(__NR_chdir, "clone child traced");
	_exit(0);
}

int
main(void)
{
	const unsigned long child_stack_size = get_page_size();
	void *const child_stack =
		midtail_alloc(child_stack_size, child_stack_size);

	pid_t pid = clone(child, child_stack,
			  CLONE_UNTRACED | SIGCHLD, NULL);
	if (pid < 0) {
		if (errno != EPERM)
			perror_msg_and_fail("clone");
		syscall(__NR_chdir, "clone skipped");
	} else {
		int status;
		if (waitpid(pid, &status, 0) != pid)
			perror_msg_and_fail("waitpid");
		if (!WIFEXITED(status))
			error_msg_and_fail("unexpected wait status %#x", status);
		if (WEXITSTATUS(status))
			error_msg_and_fail("child exit status %d",
					   WEXITSTATUS(status));
	}

	return 0;
}
