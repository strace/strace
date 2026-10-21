/*
 * Check setns(2) namespace ID for a user namespace descriptor.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "setns-report-ns-id-common.h"

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <linux/sched.h>

static void
send_namespace_fd(int sock, int fd)
{
	union {
		struct cmsghdr hdr;
		char buf[CMSG_SPACE(sizeof(int))];
	} control;
	char byte = 'x';
	struct iovec iov = { .iov_base = &byte, .iov_len = 1 };
	struct msghdr msg = {
		.msg_iov = &iov, .msg_iovlen = 1,
		.msg_control = control.buf,
		.msg_controllen = sizeof(control.buf),
	};
	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
	ssize_t n = sendmsg(sock, &msg, 0);
	if (n < 0)
		perror_msg_and_fail("sendmsg(userns descriptor)");
	if (n != 1)
		error_msg_and_fail("sendmsg: sent %zd bytes instead of 1", n);
}

static int
wait_for_child(pid_t child)
{
	int status;
	pid_t rc;
	do {
		rc = waitpid(child, &status, 0);
	} while (rc < 0 && errno == EINTR);
	if (rc < 0)
		perror_msg_and_fail("waitpid(%d)", child);
	return status;
}

static int
terminate_child(pid_t child)
{
	if (kill(child, SIGKILL) < 0 && errno != ESRCH)
		perror_msg_and_fail("kill(%d, SIGKILL)", child);
	return wait_for_child(child);
}

static void
fail_child_status(pid_t child, int status, const char *when)
{
	if (WIFEXITED(status))
		error_msg_and_fail("child %d exited with status %d %s",
				   child, WEXITSTATUS(status), when);
	if (WIFSIGNALED(status))
		error_msg_and_fail("child %d killed by signal %d %s",
				   child, WTERMSIG(status), when);
	error_msg_and_fail("child %d: unexpected wait status %#x %s",
			   child, status, when);
}

static int
receive_namespace_fd(int sock, pid_t child)
{
	union {
		struct cmsghdr hdr;
		char buf[CMSG_SPACE(sizeof(int))];
	} control;
	char byte;
	struct iovec iov = { .iov_base = &byte, .iov_len = 1 };
	struct msghdr msg = {
		.msg_iov = &iov, .msg_iovlen = 1,
		.msg_control = control.buf,
		.msg_controllen = sizeof(control.buf),
	};
	ssize_t n;
	do {
		msg.msg_controllen = sizeof(control.buf);
		msg.msg_flags = 0;
		n = recvmsg(sock, &msg, 0);
	} while (n < 0 && errno == EINTR);
	if (n < 0) {
		int saved_errno = errno;
		(void) terminate_child(child);
		errno = saved_errno;
		perror_msg_and_fail("recvmsg(userns descriptor)");
	}
	if (n == 0)
		return -1;
	if (n != 1) {
		(void) terminate_child(child);
		error_msg_and_fail("recvmsg: received %zd bytes instead of 1",
				   n);
	}
	if (msg.msg_flags & (MSG_CTRUNC | MSG_TRUNC)) {
		(void) terminate_child(child);
		error_msg_and_fail("recvmsg: truncated userns descriptor"
				   " (msg_flags=%#x)", msg.msg_flags);
	}
	struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg) {
		(void) terminate_child(child);
		error_msg_and_fail("recvmsg: missing SCM_RIGHTS userns"
				   " descriptor");
	}
	if (cmsg->cmsg_level != SOL_SOCKET ||
	    cmsg->cmsg_type != SCM_RIGHTS ||
	    cmsg->cmsg_len != CMSG_LEN(sizeof(int))) {
		(void) terminate_child(child);
		error_msg_and_fail("recvmsg: unexpected control message"
				   " (level=%d, type=%d, len=%zu)",
				   cmsg->cmsg_level, cmsg->cmsg_type,
				   (size_t) cmsg->cmsg_len);
	}
	int fd;
	memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
	return fd;
}

static void
check_user_namespace(void)
{
	int sock[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock) < 0)
		perror_msg_and_fail("socketpair");
	pid_t child = fork();
	if (child < 0)
		perror_msg_and_fail("fork");
	if (child == 0) {
		close(sock[0]);
		if (syscall(__NR_unshare, CLONE_NEWUSER) < 0)
			perror_msg_and_skip("unshare(CLONE_NEWUSER)");
		int fd = open("/proc/self/ns/user", O_RDONLY);
		if (fd < 0)
			perror_msg_and_skip("open(/proc/self/ns/user)");
		send_namespace_fd(sock[1], fd);
		close(fd);
		close(sock[1]);
		/* Keep the namespace alive until the parent has joined it. */
		for (;;)
			pause();
	}
	close(sock[1]);
	int fd = receive_namespace_fd(sock[0], child);
	close(sock[0]);
	if (fd < 0) {
		int status = wait_for_child(child);
		if (WIFEXITED(status) && WEXITSTATUS(status) == 77)
			error_msg_and_skip("child %d: userns unavailable",
					   child);
		fail_child_status(child, status,
				  "before sending a userns descriptor");
	}
	char ns[PATH_MAX + 1];
	read_proc_self_fd_link(fd, ns, sizeof(ns));
	long rc = setns_and_print_expected(fd, TEST_NS_FLAG, TEST_NS_FLAG_STR,
					  ns);
	int saved_errno = errno;
	close(fd);
	int status = terminate_child(child);
	if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGKILL)
		fail_child_status(child, status,
				  "after sending a userns descriptor");
	if (rc < 0) {
		errno = saved_errno;
		perror_msg_and_skip("setns(%d, %s)", fd, TEST_NS_FLAG_STR);
	}
}

int
main(void)
{
	check_user_namespace();
	puts("+++ exited with 0 +++");
	return 0;
}
