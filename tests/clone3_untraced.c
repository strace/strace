/*
 * Check handling of CLONE_UNTRACED in clone3 under --seccomp-bpf.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/sched.h>

int
main(void)
{
	struct clone_args args = {
		.exit_signal = SIGCHLD,
	};
	long rc = syscall(__NR_clone3, &args, sizeof(args));
	if (rc == 0)
		_exit(0);
	if (rc < 0)
		perror_msg_and_skip("clone3");
	int status;
	if (waitpid(rc, &status, 0) != rc)
		perror_msg_and_fail("waitpid");

	args.flags = CLONE_UNTRACED;
	rc = syscall(__NR_clone3, &args, sizeof(args));
	if (rc == 0) {
		syscall(__NR_chdir, "clone3 child traced");
		_exit(0);
	}
	if (rc < 0) {
		if (errno != EPERM)
			perror_msg_and_fail("clone3");
		syscall(__NR_chdir, "clone3 skipped");
		return 0;
	}

	if (waitpid(rc, &status, 0) != rc)
		perror_msg_and_fail("waitpid");
	if (!WIFEXITED(status))
		error_msg_and_fail("unexpected wait status %#x", status);
	if (WEXITSTATUS(status))
		error_msg_and_fail("child exit status %d",
				   WEXITSTATUS(status));

	return 0;
}
