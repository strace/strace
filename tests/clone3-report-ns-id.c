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
#include <sys/socket.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "scno.h"

static void
retrieve_userns(int pid, char *userns_buf, size_t userns_buflen, int fd)
{
	char c = 0;
	if (read(fd, &c, 1) != 1)
		perror_msg_and_fail("read from the child");

	char *fname = xasprintf("/proc/%d/ns/user", pid);
	int rc = readlink(fname, userns_buf, userns_buflen - 1);
	if ((unsigned int) rc >= userns_buflen)
		perror_msg_and_fail("readlink");
	userns_buf[rc] = '\0';

	if (write(fd, &c, 1) != 1)
		perror_msg_and_fail("write to the child");
}

static void
child_pause(int fd)
{
	char c = 0;
	if (write(fd, &c, 1) != 1)
		return;
	if (read(fd, &c, 1) != 1)
		return;
}

int
main(void)
{
	static int child_sockpair[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, child_sockpair) < 0)
		perror_msg_and_fail("socketpair");

	struct clone_args arg = { .flags = CLONE_NEWUSER };
	int pid = syscall(__NR_clone3, &arg, sizeof(arg));
	if (pid < 0)
		perror_msg_and_fail("clone3");
	if (pid > 0) {
		printf("clone3({flags=CLONE_NEWUSER, exit_signal=0, stack=NULL, stack_size=0}"
		       ", %zu) = %s ", sizeof(arg), sprintrc(pid));

		char userns[PATH_MAX];
		retrieve_userns(pid, userns, sizeof(userns), child_sockpair[0]);
		printf("(%s)\n", userns);
		puts("+++ exited with 0 +++");
	} else
		child_pause(child_sockpair[1]);
	return 0;
}
