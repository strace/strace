/*
 * This file is part of net-yy-inet strace test.
 *
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@strace.io>
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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#ifndef ADDR_FAMILY
# define ADDR_FAMILY_FIELD sin_family
# define ADDR_FAMILY AF_INET
# define AF_STR "AF_INET"
# define LOOPBACK_FIELD .sin_addr.s_addr = htonl(INADDR_LOOPBACK)
# define LOOPBACK "127.0.0.1"
# define SOCKADDR_TYPE sockaddr_in
# define TCP_STR "TCP"
# define INPORT sin_port
# define INPORT_STR "sin_port"
# define INADDR_STR "sin_addr=inet_addr(\"" LOOPBACK "\")"
# define SA_FIELDS ""
#endif

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	const struct SOCKADDR_TYPE addr = {
		.ADDR_FAMILY_FIELD = ADDR_FAMILY,
		LOOPBACK_FIELD
	};
	struct sockaddr * const listen_sa = tail_memdup(&addr, sizeof(addr));
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	*len = sizeof(addr);

	const int listen_fd = socket(ADDR_FAMILY, SOCK_STREAM, 0);
	if (listen_fd < 0)
		perror_msg_and_skip("socket");
	const unsigned long listen_inode = inode_of_sockfd(listen_fd);
	printf("socket(" AF_STR ", SOCK_STREAM, IPPROTO_IP) = %d<" TCP_STR
	       ":[%lu]>\n",
	       listen_fd, listen_inode);

	if (bind(listen_fd, listen_sa, *len))
		perror_msg_and_skip("bind");
	printf("bind(%d<" TCP_STR ":[%lu]>, {sa_family=" AF_STR ", " INPORT_STR
	       "=htons(0), " INADDR_STR SA_FIELDS "}, %u) = 0\n",
	       listen_fd, listen_inode, (unsigned) *len);

	if (listen(listen_fd, 1))
		perror_msg_and_skip("listen");
	printf("listen(%d<" TCP_STR ":[%lu]>, 1) = 0\n",
	       listen_fd, listen_inode);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getsockname(listen_fd, listen_sa, len))
		perror_msg_and_fail("getsockname");
	const unsigned int listen_port =
		ntohs(((struct SOCKADDR_TYPE *) listen_sa)->INPORT);
	printf("getsockname(%d<" TCP_STR ":[" LOOPBACK ":%u]>, {sa_family="
	       AF_STR ", " INPORT_STR "=htons(%u), " INADDR_STR SA_FIELDS "}"
	       ", [%u]) = 0\n",
	       listen_fd, listen_port, listen_port, (unsigned) *len);

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, optval);
	*len = sizeof(*optval);
	if (getsockopt(listen_fd, SOL_TCP, TCP_MAXSEG, optval, len))
		perror_msg_and_fail("getsockopt");
	printf("getsockopt(%d<" TCP_STR ":[" LOOPBACK ":%u]>, SOL_TCP, "
	       "TCP_MAXSEG, [%u], [%u]) = 0\n",
	       listen_fd, listen_port, *optval, (unsigned) *len);

	const int connect_fd = socket(ADDR_FAMILY, SOCK_STREAM, 0);
	if (connect_fd < 0)
		perror_msg_and_fail("socket");
	const unsigned long connect_inode = inode_of_sockfd(connect_fd);
	printf("socket(" AF_STR ", SOCK_STREAM, IPPROTO_IP) = %d<" TCP_STR
	       ":[%lu]>\n",
	       connect_fd, connect_inode);

	*len = sizeof(addr);
	if (connect(connect_fd, listen_sa, *len))
		perror_msg_and_fail("connect");
	printf("connect(%d<" TCP_STR ":[%lu]>, {sa_family=" AF_STR ", "
	       INPORT_STR "=htons(%u), " INADDR_STR SA_FIELDS "}, %u) = 0\n",
	       connect_fd, connect_inode, listen_port, (unsigned) *len);

	struct sockaddr * const accept_sa = tail_alloc(sizeof(addr));
	memset(accept_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	const int accept_fd = accept4(listen_fd, accept_sa, len, O_CLOEXEC);
	if (accept_fd < 0)
		perror_msg_and_fail("accept");
	const unsigned int connect_port =
		ntohs(((struct SOCKADDR_TYPE *) accept_sa)->INPORT);
	printf("accept4(%d<" TCP_STR ":[" LOOPBACK ":%u]>, {sa_family=" AF_STR
	       ", " INPORT_STR "=htons(%u), " INADDR_STR SA_FIELDS "}, [%u]"
	       ", SOCK_CLOEXEC) = %d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK
	       ":%u]>\n",
	       listen_fd, listen_port, connect_port, (unsigned) *len,
	       accept_fd, listen_port, connect_port);

	memset(accept_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getpeername(accept_fd, accept_sa, len))
		perror_msg_and_fail("getpeername");
	printf("getpeername(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>"
	       ", {sa_family=" AF_STR ", " INPORT_STR "=htons(%u)"
	       ", " INADDR_STR SA_FIELDS "}, [%u]) = 0\n",
	       accept_fd, listen_port, connect_port, connect_port,
	       (unsigned) *len);

	memset(listen_sa, 0, sizeof(addr));
	*len = sizeof(addr);
	if (getpeername(connect_fd, listen_sa, len))
		perror_msg_and_fail("getpeername");
	printf("getpeername(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>"
	       ", {sa_family=" AF_STR ", " INPORT_STR "=htons(%u)"
	       ", " INADDR_STR SA_FIELDS "}, [%u]) = 0\n",
	       connect_fd, connect_port, listen_port, listen_port,
	       (unsigned) *len);

	*len = sizeof(*optval);
	if (setsockopt(connect_fd, SOL_TCP, TCP_MAXSEG, optval, *len))
		perror_msg_and_fail("setsockopt");
	printf("setsockopt(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>"
	       ", SOL_TCP, TCP_MAXSEG, [%u], %u) = 0\n",
	       connect_fd, connect_port, listen_port, *optval,
	       (unsigned) *len);

	char text[] = "text";
	assert(sendto(connect_fd, text, sizeof(text) - 1,
	       MSG_DONTROUTE | MSG_DONTWAIT, NULL, 0) == sizeof(text) - 1);
	printf("sendto(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>, "
	       "\"%s\", %u, MSG_DONTROUTE|MSG_DONTWAIT, NULL, 0) = %u\n",
	       connect_fd, connect_port, listen_port, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(close(connect_fd) == 0);
	printf("close(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>) = "
	       "0\n",
	       connect_fd, connect_port, listen_port);

	assert(recvfrom(accept_fd, text, sizeof(text) - 1, MSG_WAITALL,
			NULL, NULL) == sizeof(text) - 1);
	printf("recvfrom(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>, "
	       "\"%s\", %u, MSG_WAITALL, NULL, NULL) = %u\n",
	       accept_fd, listen_port, connect_port, text,
	       (unsigned) sizeof(text) - 1, (unsigned) sizeof(text) - 1);

	assert(close(accept_fd) == 0);
	printf("close(%d<" TCP_STR ":[" LOOPBACK ":%u->" LOOPBACK ":%u]>) = "
	       "0\n",
	       accept_fd, listen_port, connect_port);

	assert(close(listen_fd) == 0);
	printf("close(%d<" TCP_STR ":[" LOOPBACK ":%u]>) = 0\n",
	       listen_fd, listen_port);

	puts("+++ exited with 0 +++");
	return 0;
}
