/*
 * This file is part of net-yy-unix strace test.
 *
 * Copyright (c) 2013-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
# include "xmalloc.h"
#endif

#define TEST_SOCKET "net-yy-unix.socket"

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = TEST_SOCKET
	};
	struct sockaddr * const listen_sa = tail_memdup(&addr, sizeof(addr));

	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	*len = offsetof(struct sockaddr_un, sun_path) + strlen(TEST_SOCKET) + 1;
	if (*len > sizeof(addr))
		*len = sizeof(addr);

	int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listen_fd < 0)
		perror_msg_and_skip("socket");

	char sock_proto_name[256];
#ifdef HAVE_SYS_XATTR_H
	char *path = xasprintf("/proc/self/fd/%u", listen_fd);
	size_t sock_proto_name_len =
		getxattr(path, "system.sockprotoname", sock_proto_name,
			 sizeof(sock_proto_name) - 1);
	free(path);
	if (sock_proto_name_len &&
	    sock_proto_name_len < sizeof(sock_proto_name))
		sock_proto_name[sock_proto_name_len] = '\0';
	else
#endif
		strcpy(sock_proto_name, "UNIX");

	unsigned long listen_inode = inode_of_sockfd(listen_fd);
	printf("socket(AF_UNIX, SOCK_STREAM, 0) = %d<%s:[%lu]>\n",
	       listen_fd, sock_proto_name, listen_inode);

	(void) unlink(TEST_SOCKET);
	if (bind(listen_fd, listen_sa, *len))
		perror_msg_and_skip("bind");
	printf("bind(%d<%s:[%lu]>, {sa_family=AF_UNIX, sun_path=\"%s\"}"
	       ", %u) = 0\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET,
	       (unsigned) *len);

	if (listen(listen_fd, 1))
		perror_msg_and_skip("listen");
	printf("listen(%d<%s:[%lu,\"%s\"]>, 1) = 0\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET);

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, optval);
	*len = sizeof(*optval);
	if (getsockopt(listen_fd, SOL_SOCKET, SO_PASSCRED, optval, len))
		perror_msg_and_fail("getsockopt");
	printf("getsockopt(%d<%s:[%lu,\"%s\"]>, SOL_SOCKET, SO_PASSCRED"
	       ", [%u], [%u]) = 0\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET,
	       *optval, (unsigned) *len);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getsockname(listen_fd, listen_sa, len))
		perror_msg_and_fail("getsockname");
	printf("getsockname(%d<%s:[%lu,\"%s\"]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, [%d => %d]) = 0\n",
	       listen_fd, sock_proto_name, listen_inode,
	       TEST_SOCKET, TEST_SOCKET, (int) sizeof(addr), (int) *len);

	int connect_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (connect_fd < 0)
		perror_msg_and_fail("socket");
	unsigned long connect_inode = inode_of_sockfd(connect_fd);
	printf("socket(AF_UNIX, SOCK_STREAM, 0) = %d<%s:[%lu]>\n",
	       connect_fd, sock_proto_name, connect_inode);

	if (connect(connect_fd, listen_sa, *len))
		perror_msg_and_fail("connect");
	printf("connect(%d<%s:[%lu]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, %u) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, TEST_SOCKET,
	       (unsigned) *len);

	struct sockaddr * const accept_sa = tail_alloc(sizeof(addr));
	memset(accept_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	int accept_fd = accept4(listen_fd, accept_sa, len, O_CLOEXEC);
	if (accept_fd < 0)
		perror_msg_and_fail("accept");
	unsigned long accept_inode = inode_of_sockfd(accept_fd);
	printf("accept4(%d<%s:[%lu,\"%s\"]>, {sa_family=AF_UNIX}"
	       ", [%d => %d], SOCK_CLOEXEC) = %d<%s:[%lu->%lu,\"%s\"]>\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET,
	       (int) sizeof(addr), (int) *len,
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getpeername(connect_fd, listen_sa, len))
		perror_msg_and_fail("getpeername");
	printf("getpeername(%d<%s:[%lu->%lu]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, [%d => %d]) = 0\n",
	       connect_fd, sock_proto_name, connect_inode,
	       accept_inode, TEST_SOCKET, (int) sizeof(addr), (int) *len);

	char text[] = "text";
	assert(sendto(connect_fd, text, sizeof(text) - 1, MSG_DONTWAIT, NULL, 0)
	       == sizeof(text) - 1);
	printf("sendto(%d<%s:[%lu->%lu]>, \"%s\", %u, MSG_DONTWAIT"
	       ", NULL, 0) = %u\n",
	       connect_fd, sock_proto_name, connect_inode, accept_inode, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(recvfrom(accept_fd, text, sizeof(text) - 1, MSG_DONTWAIT, NULL, NULL)
	       == sizeof(text) - 1);
	printf("recvfrom(%d<%s:[%lu->%lu,\"%s\"]>, \"%s\", %u, MSG_DONTWAIT"
	       ", NULL, NULL) = %u\n",
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(close(connect_fd) == 0);
	printf("close(%d<%s:[%lu->%lu]>) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, accept_inode);

	assert(close(accept_fd) == 0);
	printf("close(%d<%s:[%lu->%lu,\"%s\"]>) = 0\n",
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET);

	connect_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (connect_fd < 0)
		perror_msg_and_fail("socket");
	connect_inode = inode_of_sockfd(connect_fd);
	printf("socket(AF_UNIX, SOCK_STREAM, 0) = %d<%s:[%lu]>\n",
	       connect_fd, sock_proto_name, connect_inode);

	*optval = 1;
	*len = sizeof(*optval);
	if (setsockopt(connect_fd, SOL_SOCKET, SO_PASSCRED, optval, *len))
		perror_msg_and_fail("setsockopt");
	printf("setsockopt(%d<%s:[%lu]>, SOL_SOCKET, SO_PASSCRED"
	       ", [%u], %u) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, *optval,
	       (unsigned) *len);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getsockname(listen_fd, listen_sa, len))
		perror_msg_and_fail("getsockname");
	printf("getsockname(%d<%s:[%lu,\"%s\"]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, [%d => %d]) = 0\n",
	       listen_fd, sock_proto_name, listen_inode,
	       TEST_SOCKET, TEST_SOCKET, (int) sizeof(addr), (int) *len);

	if (connect(connect_fd, listen_sa, *len))
		perror_msg_and_fail("connect");
	printf("connect(%d<%s:[%lu]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, %u) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, TEST_SOCKET,
	       (unsigned) *len);

	memset(accept_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	accept_fd = accept4(listen_fd, accept_sa, len, O_CLOEXEC);
	if (accept_fd < 0)
		perror_msg_and_fail("accept");
	accept_inode = inode_of_sockfd(accept_fd);
	const char * const sun_path1 =
		((struct sockaddr_un *) accept_sa)->sun_path + 1;
	printf("accept4(%d<%s:[%lu,\"%s\"]>, {sa_family=AF_UNIX"
	       ", sun_path=@\"%s\"}, [%d => %d], SOCK_CLOEXEC)"
	       " = %d<%s:[%lu->%lu,\"%s\"]>\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET,
	       sun_path1, (int) sizeof(addr), (int) *len,
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getpeername(connect_fd, listen_sa, len))
		perror_msg_and_fail("getpeername");
	printf("getpeername(%d<%s:[%lu->%lu,@\"%s\"]>, {sa_family=AF_UNIX"
	       ", sun_path=\"%s\"}, [%d => %d]) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, accept_inode,
	       sun_path1, TEST_SOCKET, (int) sizeof(addr), (int) *len);

	assert(sendto(connect_fd, text, sizeof(text) - 1, MSG_DONTWAIT, NULL, 0)
	       == sizeof(text) - 1);
	printf("sendto(%d<%s:[%lu->%lu,@\"%s\"]>, \"%s\", %u, MSG_DONTWAIT"
	       ", NULL, 0) = %u\n",
	       connect_fd, sock_proto_name, connect_inode, accept_inode,
	       sun_path1, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(recvfrom(accept_fd, text, sizeof(text) - 1, MSG_DONTWAIT, NULL, NULL)
	       == sizeof(text) - 1);
	printf("recvfrom(%d<%s:[%lu->%lu,\"%s\"]>, \"%s\", %u, MSG_DONTWAIT"
	       ", NULL, NULL) = %u\n",
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(close(connect_fd) == 0);
	printf("close(%d<%s:[%lu->%lu,@\"%s\"]>) = 0\n",
	       connect_fd, sock_proto_name, connect_inode, accept_inode,
	       sun_path1);

	assert(close(accept_fd) == 0);
	printf("close(%d<%s:[%lu->%lu,\"%s\"]>) = 0\n",
	       accept_fd, sock_proto_name, accept_inode, connect_inode,
	       TEST_SOCKET);

	assert(unlink(TEST_SOCKET) == 0);

	assert(close(listen_fd) == 0);
	printf("close(%d<%s:[%lu,\"%s\"]>) = 0\n",
	       listen_fd, sock_proto_name, listen_inode, TEST_SOCKET);

	puts("+++ exited with 0 +++");
	return 0;
}
