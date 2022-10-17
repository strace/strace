/*
 * Check decoding of SO_ERROR socket option.
 *
 * Copyright (c) 2018 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static in_port_t
reserve_ephemeral_port(void)
{
	int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sd < 0)
		perror_msg_and_skip("server socket AF_UNIX SOCK_STREAM");

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
	};

	/*
	 * The range is defined in /proc/sys/net/ipv4/ip_local_port_range.
	 * We use the default range here.
	 */
	for (in_port_t port = 49152; port < 61000; port++) {
		/* Just bind here. No listen. */
		addr.sin_port = htons(port);
		if (bind(sd, (void *) &addr, sizeof(addr)) == 0)
			return port;
	}
	error_msg_and_skip("no ephemeral port available for test purposes");
}

int
main(void)
{
	static const int sizes[] = {
		-1, 0, 1,
		sizeof(int) - 1,
		sizeof(int),
		sizeof(int) + 1,
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(int, sock_errno);
	in_port_t port = reserve_ephemeral_port();
	const struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),
		.sin_port = htons(port),
	};

	for (size_t i = 0; i < ARRAY_SIZE(sizes); i++) {
		/*
		 * Connect to the reserved port in NONBLOCK mode.
		 * The port is reserved but not listened. So
		 * the client doing "connect" gets error asynchronously.
		 */
		int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (fd < 0)
			perror_msg_and_skip("socket AF_UNIX SOCK_STREAM");

		int flag = fcntl(fd, F_GETFL);
		if (flag < 0)
			perror_msg_and_skip("fcntl F_GETFL");
		flag |= O_NONBLOCK;
		if (fcntl(fd, F_SETFL, flag) < 0)
			perror_msg_and_skip("fcntl F_SETFL");

		if (connect(fd, (void *) &addr, sizeof(addr)) == 0)
			error_msg_and_skip("connect unexpectedly succeeded");
		if (errno != EINPROGRESS)
			perror_msg_and_skip("connect failed for unexpected reason");

		struct timeval to = {
			.tv_sec =  1,
			.tv_usec = 0,
		};
		fd_set wfds;
		FD_ZERO(&wfds);
		FD_SET(fd, &wfds);
		if (select(fd + 1, NULL, &wfds, NULL, &to) < 0)
			perror_msg_and_skip("select");

		*sock_errno = 0xbadc0ded;
		socklen_t optlen = sizes[i];
		long rc = getsockopt(fd, SOL_SOCKET, SO_ERROR, sock_errno,
				     &optlen);
		const char *errstr = sprintrc(rc);
		if (sizes[i] > 0 && rc < 0)
			perror_msg_and_skip("getsockopt");
		if (sizes[i] >= (int) sizeof(*sock_errno)
		    && *sock_errno != ECONNREFUSED) {
			errno = *sock_errno;
			perror_msg_and_skip("unexpected socket error");
		}
		if (sizes[i] >= (int) sizeof(*sock_errno)
		    && optlen != sizeof(*sock_errno)) {
			error_msg_and_skip("unexpected data size for error"
					   " option: %d", optlen);
		}

		printf("getsockopt(%d, SOL_SOCKET, SO_ERROR, ", fd);
		if (sizes[i] <= 0) {
			printf("%p, [%d]", sock_errno, sizes[i]);
		} else if (sizes[i] < (int) sizeof(*sock_errno)) {
			print_quoted_hex(sock_errno, sizes[i]);
			printf(", [%u]", sizes[i]);
		} else if (sizes[i] == sizeof(*sock_errno)) {
			printf("[ECONNREFUSED], [%zu]", sizeof(*sock_errno));
		} else {
			printf("[ECONNREFUSED], [%u => %zu]",
			       sizes[i], sizeof(*sock_errno));
		}
		printf(") = %s\n", errstr);

		close(fd);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
