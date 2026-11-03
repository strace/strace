/*
 * Check -yy endpoints for PTY descriptors belonging to different processes.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/kcmp.h>

static int
open_ptmx(void)
{
	static const char dev_ptmx[] = "/dev/ptmx";
	int fd = open(dev_ptmx, O_RDWR | O_NOCTTY);
	if (fd < 0)
		perror_msg_and_skip("open: %s", dev_ptmx);
	if (unlockpt(fd) < 0)
		perror_msg_and_skip("unlockpt");
	return fd;
}

int
main(void)
{
	int fd = open_ptmx();
	const char *parent_pts = ptsname(fd);
	if (!parent_pts)
		perror_msg_and_skip("ptsname");
	const char *path = get_fd_path(fd);

	int ready[2], done[2];
	if (pipe(ready) || pipe(done))
		perror_msg_and_fail("pipe");

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	char child_pts[64] = { 0 };
	if (!pid) {
		close(ready[0]);
		close(done[1]);
		close(fd);
		int child_fd = open_ptmx();
		if (child_fd != fd) {
			if (dup2(child_fd, fd) < 0)
				perror_msg_and_fail("dup2");
			close(child_fd);
		}

		const char *pts = ptsname(fd);
		if (!pts)
			perror_msg_and_fail("ptsname");
		size_t len = strlen(pts) + 1;
		if (len > sizeof(child_pts))
			error_msg_and_fail("child PTY name requires %zu bytes, "
					   "buffer size is %zu",
					   len, sizeof(child_pts));
		memcpy(child_pts, pts, len);
		ssize_t n = write(ready[1], child_pts, sizeof(child_pts));
		if (n < 0)
			perror_msg_and_fail("write child PTY name");
		if ((size_t) n != sizeof(child_pts))
			error_msg_and_fail("write child PTY name: "
					   "wrote %zd bytes, expected %zu",
					   n, sizeof(child_pts));
		close(ready[1]);

		/* Keep the child's master open until kcmp has been decoded. */
		char c;
		n = read(done[0], &c, 1);
		if (n < 0)
			perror_msg_and_fail("read parent completion");
		if (n != 0)
			error_msg_and_fail("read parent completion: "
					   "read %zd bytes, expected EOF", n);
		_exit(0);
	}

	close(ready[1]);
	close(done[0]);
	ssize_t n = read(ready[0], child_pts, sizeof(child_pts));
	if (n < 0)
		perror_msg_and_fail("read child PTY name");
	if ((size_t) n != sizeof(child_pts))
		error_msg_and_fail("read child PTY name: "
				   "read %zd bytes, expected %zu",
				   n, sizeof(child_pts));
	close(ready[0]);
	if (!strcmp(parent_pts, child_pts))
		error_msg_and_fail("parent and child have the same PTY endpoint: "
				   "%s", parent_pts);

	long rc = syscall(__NR_kcmp, getpid(), pid, KCMP_FILE, fd, fd);
	printf("kcmp(%d, %d, KCMP_FILE, "
	       "%d<%s<char 5:2 @%s>>, %d<%s<char 5:2 @%s>>) = %s\n",
	       getpid(), pid, fd, path, parent_pts, fd, path, child_pts,
	       sprintrc(rc));

	close(done[1]);
	int status;
	if (waitpid(pid, &status, 0) < 0)
		perror_msg_and_fail("waitpid(%d)", pid);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		error_msg_and_fail("waitpid(%d): unexpected wait status %#x",
				   pid, status);

	puts("+++ exited with 0 +++");
	return 0;
}
