/*
 * This file is part of net-yy-netlink strace test.
 *
 * Copyright (c) 2013-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/netlink_diag.h>

#if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
# define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
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
