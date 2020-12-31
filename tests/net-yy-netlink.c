/*
 * This file is part of net-yy-netlink strace test.
 *
 * Copyright (c) 2013-2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016-2020 The strace developers.
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

#ifndef PRINT_SOCK
# define PRINT_SOCK 2
#endif

#if PRINT_SOCK == 2
# define FMT_UNBOUND "<NETLINK:[%lu]>"
# define FMT_BOUND   "<NETLINK:[SOCK_DIAG:%u]>"
# define ARG_UNBOUND inode
# define ARG_BOUND   addr.nl_pid
#elif PRINT_SOCK == 1
# define FMT_UNBOUND "<socket:[%lu]>"
# define FMT_BOUND   "<socket:[%lu]>"
# define ARG_UNBOUND inode
# define ARG_BOUND   inode
#else
# define FMT_UNBOUND "%s"
# define FMT_BOUND   "%s"
# define ARG_UNBOUND ""
# define ARG_BOUND   ""
#endif

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
#if PRINT_SOCK
	const unsigned long inode = inode_of_sockfd(fd);
#endif
	printf("socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG) = "
	       "%d" FMT_UNBOUND "\n", fd, ARG_UNBOUND);

	if (bind(fd, sa, *len))
		perror_msg_and_skip("bind");
	printf("bind(%d" FMT_UNBOUND ", {sa_family=AF_NETLINK"
	       ", nl_pid=%d, nl_groups=00000000}, %u) = 0\n",
	       fd, ARG_UNBOUND, addr.nl_pid, (unsigned) *len);

	if (getsockname(fd, sa, len))
		perror_msg_and_fail("getsockname");
	printf("getsockname(%d" FMT_BOUND ", {sa_family=AF_NETLINK"
	       ", nl_pid=%d, nl_groups=00000000}, [%u]) = 0\n",
	       fd, ARG_BOUND, addr.nl_pid, (unsigned) *len);

	if (close(fd))
		perror_msg_and_fail("close");
	printf("close(%d" FMT_BOUND ") = 0\n", fd, ARG_BOUND);

	puts("+++ exited with 0 +++");
	return 0;
}
