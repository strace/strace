/*
 * This file is part of net-yy-netlink strace test.
 *
 * Copyright (c) 2013-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "netlink.h"
#include <linux/sock_diag.h>
#include <linux/netlink_diag.h>

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	struct sockaddr_nl addr = {
		.nl_family = AF_NETLINK,
		.nl_pid = getpid()
	};
	struct sockaddr *const sa = tail_memdup(&addr, sizeof(addr));
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	*len = sizeof(addr);

	const int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		perror_msg_and_skip("socket");
	const unsigned long inode = inode_of_sockfd(fd);
	printf("socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG) = "
	       "%d<NETLINK:[%lu]>\n", fd, inode);

	if (bind(fd, sa, *len))
		perror_msg_and_skip("bind");
	printf("bind(%d<NETLINK:[%lu]>, {sa_family=AF_NETLINK"
	       ", nl_pid=%u, nl_groups=00000000}, %u) = 0\n",
	       fd, inode, addr.nl_pid, (unsigned) *len);

	if (getsockname(fd, sa, len))
		perror_msg_and_fail("getsockname");
	printf("getsockname(%d<NETLINK:[SOCK_DIAG:%u]>, {sa_family=AF_NETLINK"
	       ", nl_pid=%u, nl_groups=00000000}, [%u]) = 0\n",
	       fd, addr.nl_pid, addr.nl_pid, (unsigned) *len);

	if (close(fd))
		perror_msg_and_fail("close");
	printf("close(%d<NETLINK:[SOCK_DIAG:%u]>) = 0\n", fd, addr.nl_pid);

	puts("+++ exited with 0 +++");
	return 0;
}
