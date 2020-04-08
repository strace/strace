/*
 * Print the process reaper id.
 *
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

/* PARENT - CHILD - GRANDCHILD */

static int
grandchild(int read_fd, int write_fd)
{
	/* wait for notification from PARENT about CHILD completion */
	pid_t pid;
	if (read(read_fd, &pid, sizeof(pid)) != 0)
		perror_msg_and_fail("read");

	/* write ppid to PARENT */
	pid = getppid();
	if (write(write_fd, &pid, sizeof(pid)) != sizeof(pid))
		perror_msg_and_fail("write");

	_exit(0);
}

static int
child(int read_fd, int write_fd)
{
	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid)
		return grandchild(read_fd, write_fd);
	else
		_exit(0);
}

static int
parent(pid_t pid, int read_fd, int write_fd)
{
	/* wait for CHILD completion */
	int status;
	if (waitpid(pid, &status, 0) != pid)
		perror_msg_and_fail("waitpid");
	if (status != 0)
		error_msg_and_fail("status %x", status);

	/* notify GRANDCHILD about CHILD completion */
	close(write_fd),
	      write_fd = -1;

	/* read ppid of GRANDCHILD */
	if (read(read_fd, &pid, sizeof(pid)) != sizeof(pid))
		perror_msg_and_fail("read");

	printf("%d\n", pid);
	return 0;
}

int
main(void)
{
	int parent_grandchild_fds[2];
#define parent_read_fd		parent_grandchild_fds[0]
#define grandchild_write_fd	parent_grandchild_fds[1]

	int grandchild_parent_fds[2];
#define grandchild_read_fd	grandchild_parent_fds[0]
#define parent_write_fd		grandchild_parent_fds[1]

	if (pipe(parent_grandchild_fds) ||
	    pipe(grandchild_parent_fds))
		perror_msg_and_fail("pipe");

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		/* CHILD */
		close(parent_read_fd),
		      parent_read_fd = -1;

		close(parent_write_fd),
		      parent_write_fd = -1;

		return child(grandchild_read_fd,
			     grandchild_write_fd);
	} else {
		/* PARENT */
		close(grandchild_read_fd),
		      grandchild_read_fd = -1;

		close(grandchild_write_fd),
		      grandchild_write_fd = -1;

		return parent(pid, parent_read_fd,
				   parent_write_fd);
	}
}
