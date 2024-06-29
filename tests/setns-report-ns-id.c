/*
 * Check appending auxstr of setns(2) when --namespace=switchTo is given
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
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>

typedef struct msg {
	enum {
		MSG_OK = 0,
		MSG_ERR_CLOSE,
		MSG_ERR_UNSHARE,
	} type;
	int err;
} msg_t;

int
main(void)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0)
		perror_msg_and_skip("socketpair");

	pid_t pid = fork();
	if (pid == 0) {
		msg_t msg;
		char c;

		if (syscall(__NR_unshare, 0x10000000) < 0) {
			msg.type = MSG_ERR_UNSHARE;
			msg.err = errno;
			assert(write(sv[1], &msg, sizeof(msg)) == sizeof(msg));
			return 1;
		}

		msg.type = MSG_OK;
		if (write(sv[1], &msg, sizeof(msg)) != sizeof(msg))
			return 1;
		if (read(sv[1], &c, 1) != 1)
			return 1;
	} else if (pid > 0) {
		ssize_t r;
		msg_t msg;

		r = read(sv[0], &msg, sizeof(msg));
		if (r < 0)
			perror_msg_and_skip("read");
		if (r == 0)
			error_msg_and_skip("unexpected EOF");
		else if (r != sizeof(msg))
			error_msg_and_skip("data from the child it too short");

		if (msg.type != MSG_OK) {
			errno = msg.err;
			perror_msg_and_skip("%s",
					    (msg.type == MSG_ERR_CLOSE)? "close":
					    (msg.type == MSG_ERR_UNSHARE)? "unshare":
					    "unknown");
		}

		char *ns_path = xasprintf("/proc/%u/ns/user", pid);
		int ns_fd = open(ns_path, O_RDONLY);
		if (ns_fd < 0) {
			int e = errno;
			assert(write(sv[0], (char[]){'\n'}, 1) == 1);
			wait(NULL);
			errno = e;
			perror_msg_and_skip("open");
		}

		long rc = syscall(__NR_setns, ns_fd, 0x10000000);
		printf("setns(%d, CLONE_NEWUSER) = %s%s",
		       ns_fd, sprintrc(rc), rc == 0? "": "\n");

		if (rc == 0) {
			char buf[PATH_MAX + 1];
			int n  = readlink(ns_path, buf, sizeof(buf));
			if (n < 0) {
				int e = errno;
				assert(write(sv[0], (char[]){'\n'}, 1) == 1);
				wait(NULL);
				errno = e;
				perror_msg_and_skip("readlink");
			}
			else if ((size_t)n >= sizeof(buf)) {
				assert(write(sv[0], (char[]){'\n'}, 1) == 1);
				wait(NULL);
				error_msg_and_skip("too large readlink result");
			}
			buf[n] = '\0';
			printf(" (%s)\n", buf);
		}

		assert(write(sv[0], (char[]){'\n'}, 1) == 1);
		int status = 0;
		if (wait(&status) < 0)
			perror_msg_and_skip("wait");
		if (status != 0)
			error_msg_and_skip("having a I/O trouble in child");

		/* Verify that strace doen't print a namespace when an error occurs. */
		rc = syscall(__NR_setns, -1, 0x10000000);
		printf("setns(-1, CLONE_NEWUSER) = %s\n", sprintrc(rc));

		puts("+++ exited with 0 +++");

	} else
		perror_msg_and_skip("fork");

	return 0;
}
