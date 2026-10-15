/*
 * Check appending auxstr of setns(2) when --namespace=new is given.
 *
 * Copyright (c) 2024-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

static void
test_children_namespace(unsigned int flag, const char *flag_str,
			const char *ns_path)
{
	if (access(ns_path, F_OK) < 0 || syscall(__NR_unshare, flag) < 0)
		return;

	int pipefd[2];
	pid_t child = -1;
	if (flag == 0x20000000) {
		/* Keep the new PID namespace alive while opening its ns fd. */
		if (pipe(pipefd) < 0)
			perror_msg_and_fail("pipe");
		child = fork();
		if (child < 0)
			perror_msg_and_fail("fork");
		if (child == 0) {
			close(pipefd[1]);
			char byte;
			ssize_t n;
			do {
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
	ssize_t n = readlink(ns_path, ns, sizeof(ns));
	if (n < 0)
		perror_msg_and_fail("readlink: %s", ns_path);
	if ((size_t) n >= sizeof(ns))
		error_msg_and_fail("readlink: %s: result is too large", ns_path);
	ns[n] = '\0';

	int rc = syscall(__NR_setns, ns_fd, flag);
	if (rc < 0) {
		close(ns_fd);
		goto out;
	}
	printf("setns(%d, %s) = %s (%s)\n",
	       ns_fd, flag_str, sprintrc(rc), ns);
	close(ns_fd);

out:
	if (child > 0) {
		close(pipefd[1]);
		if (waitpid(child, NULL, 0) < 0)
			perror_msg_and_fail("waitpid");
	}
}

int
main(void)
{
	/* First enter a new user namespace so we have CAP_SYS_ADMIN inside it. */
	(void) syscall(__NR_unshare, 0x10000000);

	/* Now enter a new network namespace owned by the child user namespace. */
	if (syscall(__NR_unshare, 0x40000000) < 0)
		perror_msg_and_skip("unshare (CLONE_NEWNET)");

	const char *netns_path = "/proc/self/ns/net";
	skip_if_unavailable(netns_path);

	int netns_fd = open(netns_path, O_RDONLY);
	if (netns_fd < 0)
		perror_msg_and_skip("open(%s)", netns_path);
	char netns[PATH_MAX + 1];
	ssize_t n  = readlink(netns_path, netns, sizeof(netns));
	if (n < 0)
		perror_msg_and_fail("readlink: %s", netns_path);
	else if ((size_t) n >= sizeof(netns))
		error_msg_and_fail("readlink: %s: result is too large",
				   netns_path);
	netns[n] = '\0';

	if (syscall(__NR_unshare, 0x40000000) < 0)
		perror_msg_and_skip("unshare (CLONE_NEWNET)");

	int rc = syscall(__NR_setns, netns_fd, 0x40000000);
	if (rc < 0)
		perror_msg_and_skip("setns (CLONE_NEWNET)");
	printf("setns(%d, CLONE_NEWNET) = %s (%s)\n",
	       netns_fd, sprintrc(rc), netns);

	test_children_namespace(0x20000000, "CLONE_NEWPID",
				"/proc/self/ns/pid_for_children");
	test_children_namespace(0x00000080, "CLONE_NEWTIME",
				"/proc/self/ns/time_for_children");

	puts("+++ exited with 0 +++");

	return 0;
}
