/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <sys/socket.h>
#include "netlink.h"

int
create_nl_socket_ext(const int proto, const char *const name)
{
	const int fd = socket(AF_NETLINK, SOCK_RAW, proto);
	if (fd < 0)
		perror_msg_and_skip("socket AF_NETLINK %s", name);

	const struct sockaddr_nl addr = { .nl_family = AF_NETLINK };
	socklen_t len = sizeof(addr);

	if (bind(fd, (const struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind AF_NETLINK %s", name);

	/* one more operation on this socket to win the race */
	int listening;
	len = sizeof(listening);
	if (getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN, &listening, &len))
		perror_msg_and_fail("getsockopt");

	return fd;
}
