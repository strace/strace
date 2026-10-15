/*
 * Check setns(2) namespace IDs for future children.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "setns-report-ns-id-common.h"

#include <errno.h>
#include <sys/wait.h>

#include <linux/sched.h>

static void
test_children_namespace(unsigned int flag, const char *flag_str,
			const char *ns_path)
{
	skip_if_unavailable(ns_path);
	if (syscall(__NR_unshare, flag) < 0)
		perror_msg_and_skip("unshare(%s)", flag_str);

	int pipefd[2] = { -1, -1 };
	pid_t child = -1;
	if (flag == CLONE_NEWPID) {
		/* Keep the new PID namespace alive while opening its ns fd. */
		if (pipe(pipefd) < 0)
			perror_msg_and_fail("pipe");
		child = fork();
		if (child < 0)
			perror_msg_and_fail("fork");
		if (child == 0) {
			close(pipefd[1]);
			ssize_t n;
			do {
				char byte;
				n = read(pipefd[0], &byte, sizeof(byte));
			} while (n < 0 && errno == EINTR);
			_exit(0);
		}
		close(pipefd[0]);
	}

	int ns_fd = open(ns_path, O_RDONLY);
	if (ns_fd < 0)
		perror_msg_and_fail("open(%s)", ns_path);

	char ns[PATH_MAX + 1];
	read_proc_self_fd_link(ns_fd, ns, sizeof(ns));

	int saved_errno;
	long rc = setns_and_print_expected(ns_fd, flag, flag_str, ns);
	if (rc < 0) {
		saved_errno = errno;
		close(ns_fd);
		errno = saved_errno;
		goto out;
	}
	close(ns_fd);

out:
	if (rc < 0)
		saved_errno = errno;
	if (child > 0) {
		if (pipefd[1] >= 0)
			close(pipefd[1]);
		if (waitpid(child, NULL, 0) < 0)
			perror_msg_and_fail("waitpid");
	}
	if (rc < 0) {
		errno = saved_errno;
		perror_msg_and_skip("setns(%d, %s)", ns_fd, flag_str);
	}
}

int
main(void)
{
	/*
	 * Try to enter a new user namespace
	 * so we have CAP_SYS_ADMIN inside it.
	 */
	(void) syscall(__NR_unshare, CLONE_NEWUSER);

#if defined TEST_NS_PID
	test_children_namespace(ARG_STR(CLONE_NEWPID),
				"/proc/self/ns/pid_for_children");
#elif defined TEST_NS_TIME
	test_children_namespace(ARG_STR(CLONE_NEWTIME),
				"/proc/self/ns/time_for_children");
#else
# error TEST_NS_PID or TEST_NS_TIME must be defined
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
