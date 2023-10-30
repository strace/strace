/*
 * Check decoding of struct msghdr.msg_name* arguments of recvmsg syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#undef TEST_RECVMSG_BOGUS_ADDR

/*
 * Sadly, musl recvmsg wrapper blindly dereferences the 2nd argument,
 * so limit these tests to glibc that hopefully doesn't.
 */
#ifndef __GLIBC__
# define TEST_RECVMSG_BOGUS_ADDR 0
#endif

/*
 * Sadly, starting with commit
 * glibc-2.33.9000-707-g13c51549e2077f2f3bf84e8fd0b46d8b0c615912, on every
 * 32-bit architecture where 32-bit time_t support is enabled, glibc blindly
 * dereferences the 2nd argument of recvmsg call.
 */
#if GLIBC_PREREQ_GE(2, 33) && defined __TIMESIZE && __TIMESIZE != 64
# define TEST_RECVMSG_BOGUS_ADDR 0
#endif

#ifndef TEST_RECVMSG_BOGUS_ADDR
# define TEST_RECVMSG_BOGUS_ADDR 1
#endif

static int
send_recv(const int send_fd, const int recv_fd,
	 struct msghdr *const msg, const int flags)
{
	if (send(send_fd, "A", 1, 0) != 1)
		perror_msg_and_skip("send");
	return recvmsg(recv_fd, msg, flags);
}

static void
test_msg_name(const int send_fd, const int recv_fd)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(char, recv_buf);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct iovec, iov);
	iov->iov_base = recv_buf;
	iov->iov_len = sizeof(*recv_buf);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sockaddr_un, addr);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct msghdr, msg);
	msg->msg_name = addr;
	msg->msg_namelen = sizeof(*addr);
	msg->msg_iov = iov;
	msg->msg_iovlen = 1;
	msg->msg_control = 0;
	msg->msg_controllen = 0;
	msg->msg_flags = 0;

	int rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	if (rc < 0)
		perror_msg_and_skip("recvmsg");
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX, sun_path=\"%s\"}"
	       ", msg_namelen=%d => %d, msg_iov=[{iov_base=\"A\", iov_len=1}]"
	       ", msg_iovlen=1, msg_controllen=0, msg_flags=0}, MSG_DONTWAIT)"
	       " = %d\n",
	       recv_fd, addr->sun_path, (int) sizeof(struct sockaddr_un),
	       (int) msg->msg_namelen, rc);

	memset(addr, 0, sizeof(*addr));
	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX, sun_path=\"%s\"}"
	       ", msg_namelen=%d, msg_iov=[{iov_base=\"A\", iov_len=1}]"
	       ", msg_iovlen=1, msg_controllen=0, msg_flags=0}, MSG_DONTWAIT)"
	       " = %d\n",
	       recv_fd, addr->sun_path, (int) msg->msg_namelen, rc);

	msg->msg_name = 0;
	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_name=NULL, msg_namelen=%d"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %d\n",
	       recv_fd, (int) msg->msg_namelen, rc);

	const size_t offsetof_sun_path = offsetof(struct sockaddr_un, sun_path);
	msg->msg_name = addr;
	msg->msg_namelen = offsetof_sun_path;
	memset(addr->sun_path, 'A', sizeof(addr->sun_path));

	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX}, msg_namelen=%d => %d"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %d\n",
	       recv_fd, (int) offsetof_sun_path, (int) msg->msg_namelen, rc);

	msg->msg_namelen = sizeof(struct sockaddr);
	msg->msg_name = ((void *) (addr + 1)) - msg->msg_namelen;
	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX, sun_path=\"%.*s\"}"
	       ", msg_namelen=%d => %d, msg_iov=[{iov_base=\"A\", iov_len=1}]"
	       ", msg_iovlen=1, msg_controllen=0, msg_flags=0}, MSG_DONTWAIT)"
	       " = %d\n",
	       recv_fd, (int) (sizeof(struct sockaddr) - offsetof_sun_path),
	       ((struct sockaddr_un *) msg->msg_name)->sun_path,
	       (int) sizeof(struct sockaddr), (int) msg->msg_namelen, rc);

	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_namelen=%d}, MSG_DONTWAIT) = %s\n",
	       recv_fd, (int) msg->msg_namelen, sprintrc(rc));

#if TEST_RECVMSG_BOGUS_ADDR
	/*
	 * When recvmsg is called with a valid descriptor
	 * but inaccessible memory, it causes segfaults on some architectures.
	 * As in these cases we test decoding of failed recvmsg calls,
	 * it's ok to fail recvmsg with any reason as long as
	 * it doesn't read that inaccessible memory.
	 */
	rc = send_recv(send_fd, -1, msg + 1, 0);
	printf("recvmsg(-1, %p, 0) = %s\n",
	       msg + 1, sprintrc(rc));

	rc = send_recv(send_fd, -1, 0, 0);
	printf("recvmsg(-1, NULL, 0) = %s\n",
	       sprintrc(rc));
#endif
}

int
main(void)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
		perror_msg_and_skip("socketpair");

	const struct sockaddr_un un = {
		.sun_family = AF_UNIX,
		.sun_path = "msg_name-recvmsg.test.send.socket"
	};

	(void) unlink(un.sun_path);
	if (bind(fds[1], (const void *) &un, sizeof(un)))
		perror_msg_and_skip("bind");
	(void) unlink(un.sun_path);

	test_msg_name(fds[1], fds[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
