/*
 * Check decoding of struct msghdr.msg_name* arguments of recvmsg syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

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
	       ", msg_namelen=%d->%d, msg_iov=[{iov_base=\"A\", iov_len=1}]"
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
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX}, msg_namelen=%d->%d"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, MSG_DONTWAIT) = %d\n",
	       recv_fd, (int) offsetof_sun_path, (int) msg->msg_namelen, rc);

	msg->msg_namelen = sizeof(struct sockaddr);
	msg->msg_name = ((void *) (addr + 1)) - msg->msg_namelen;
	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_name={sa_family=AF_UNIX, sun_path=\"%.*s\"}"
	       ", msg_namelen=%d->%d, msg_iov=[{iov_base=\"A\", iov_len=1}]"
	       ", msg_iovlen=1, msg_controllen=0, msg_flags=0}, MSG_DONTWAIT)"
	       " = %d\n",
	       recv_fd, (int) (sizeof(struct sockaddr) - offsetof_sun_path),
	       ((struct sockaddr_un *) msg->msg_name)->sun_path,
	       (int) sizeof(struct sockaddr), (int) msg->msg_namelen, rc);

	rc = send_recv(send_fd, recv_fd, msg, MSG_DONTWAIT);
	printf("recvmsg(%d, {msg_namelen=%d}, MSG_DONTWAIT) = %d %s (%m)\n",
	       recv_fd, (int) msg->msg_namelen, rc, errno2name());

	/*
	 * When recvmsg is called with a valid descriptor
	 * but inaccessible memory, it causes segfaults on some architectures.
	 * As in these cases we test decoding of failed recvmsg calls,
	 * it's ok to fail recvmsg with any reason as long as
	 * it doesn't read that inaccessible memory.
	 */

	/*
	 * Sadly, musl recvmsg wrapper blindly dereferences 2nd argument,
	 * so limit this test to glibc that doesn't.
	 */
#ifdef __GLIBC__
	rc = send_recv(send_fd, -1, msg + 1, 0);
	printf("recvmsg(-1, %p, 0) = %d %s (%m)\n",
	       msg + 1, rc, errno2name());
#endif

	rc = send_recv(send_fd, -1, 0, 0);
	printf("recvmsg(-1, NULL, 0) = %d %s (%m)\n",
	       rc, errno2name());
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
