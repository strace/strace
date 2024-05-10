/*
 * Check appending auxstr of setns(2) when --namespace=new is given
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "xmalloc.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

static int
get_user_ns_for_pid(char *buf, size_t size, int pid)
{
	char *ns_path = xasprintf("/proc/%u/ns/user", pid);
	int ns_fd = open(ns_path, O_RDONLY);
	int n;

	if (ns_fd < 0)
		perror_msg_and_skip("open");
	n  = readlink(ns_path, buf, size);
	if (n < 0)
		perror_msg_and_skip("readlink");
	else if ((size_t)n >= size)
		error_msg_and_skip("too large readlink result");
	buf[n] = '\0';

	return ns_fd;
}

int
main(void)
{
	int pd[2];

	if (pipe(pd) < 0)
		error_msg_and_skip("pipe");

	pid_t pid = fork();
	if (pid > 0) {
		close(pd[0]);

		if (syscall(__NR_unshare, 0x10000000) < 0)
			perror_msg_and_skip("unshare");

		if (write(pd[1], (char[]){'\n'}, 1) < 0)
			perror_msg_and_skip("write");

		int status = 0;
		if (wait(&status) < 0)
			perror_msg_and_skip("wait");
		if (status != 0)
			error_msg_and_skip("having a trouble in child");

		/* Verify that strace doen't print a namespace
		   when an error occurs. */
		pid_t self = getpid();
		int rc = syscall(__NR_setns, -1, 0x10000000);
		printf("%d setns(-1, CLONE_NEWUSER) = %s\n", self, sprintrc(rc));

		printf("%d +++ exited with 0 +++\n", self);
	} else if (pid == 0) {
		close(pd[1]);

		char c;
		if (read(pd[0], &c, 1) < 0)
			perror_msg_and_skip("read");

		char buf[PATH_MAX + 1];
		int ns_fd = get_user_ns_for_pid(buf, sizeof(buf), getppid());
		int rc = syscall(__NR_setns, ns_fd, 0x10000000);
		if (rc < 0)
			perror_msg_and_skip("setns");

		pid_t self = getpid();
		printf("%d setns(%d, CLONE_NEWUSER) = %s (%s)\n", self,
		       ns_fd, sprintrc(rc), buf);

		printf("%d +++ exited with 0 +++\n", self);
	} else
		perror_msg_and_skip("fork");

	return 0;
}
