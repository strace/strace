/*
 * Check handling of CLONE_UNTRACED under --seccomp-bpf.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define _GNU_SOURCE
#include "tests.h"

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/sched.h>
#include "scno.h"

static int
child(void *arg)
{
	const char *msg = arg;
	const size_t len = strlen(msg);
	if (write(STDOUT_FILENO, msg, len) != (ssize_t) len)
		_exit(1);
	_exit(0);
}

static void
wait_child(pid_t pid, const char *name)
{
	int status;
	if (waitpid(pid, &status, 0) != pid)
		perror_msg_and_fail("waitpid");
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		error_msg_and_fail("%s child failed", name);
}

static void
write_marker(const char *msg)
{
	const size_t len = strlen(msg);
	if (write(STDOUT_FILENO, msg, len) != (ssize_t) len)
		perror_msg_and_fail("write");
}

static void
test_clone(void)
{
	const size_t stack_size = 1 << 20;
	char *stack = malloc(stack_size);
	static char msg[] = "clone-child-traced\n";
	if (!stack)
		perror_msg_and_fail("malloc");

	pid_t pid = clone(child, stack + stack_size,
			  CLONE_UNTRACED | SIGCHLD, msg);
	if (pid < 0) {
		if (errno != ENOSYS)
			perror_msg_and_fail("clone");
		write_marker("clone-blocked\n");
	} else {
		wait_child(pid, "clone");
	}

	free(stack);
}

static void
test_clone3(void)
{
#ifdef __NR_clone3
	struct clone_args args = {
		.flags = CLONE_UNTRACED,
		.exit_signal = SIGCHLD,
	};
	long rc = syscall(__NR_clone3, &args, sizeof(args));
	if (!rc) {
		static char msg[] = "clone3-child-traced\n";
		child(msg);
	}
	if (rc < 0) {
		if (errno != ENOSYS)
			perror_msg_and_fail("clone3");
		write_marker("clone3-blocked\n");
		return;
	}
	wait_child(rc, "clone3");
#else
	write_marker("clone3-unavailable\n");
#endif
}

int
main(void)
{
	test_clone();
	test_clone3();
	return 0;
}
