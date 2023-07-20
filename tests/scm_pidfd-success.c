/*
 * Check decoding of SCM_PIDFD control messages.
 *
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define XLAT_MACROS_ONLY
# include "xlat/scmvals.h"
#undef XLAT_MACROS_ONLY

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int pv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pv))
                perror_msg_and_skip("socketpair AF_UNIX SOCK_STREAM");
	if (close(pv[1]))
		perror_msg_and_fail("close");

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
                perror_msg_and_skip("socketpair AF_UNIX SOCK_STREAM");
	if (close(sv[1]))
		perror_msg_and_fail("close");

	unsigned int cmsg_size = CMSG_SPACE(sizeof(pv[0]));
	struct cmsghdr *cmsg = tail_alloc(cmsg_size);
	cmsg->cmsg_len = cmsg_size;
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_PIDFD;
	memcpy(CMSG_DATA(cmsg), pv, sizeof(pv[0]));

	char sym = 'A';
	struct iovec iov = {
		.iov_base = &sym,
		.iov_len = sizeof(sym)
	};
	struct msghdr mh = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = cmsg,
		.msg_controllen = cmsg_size
	};

	recvmsg(sv[0], &mh, 0);

	printf("recvmsg(%d<socket:[%lu]>, {msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1"
	       ", msg_control=[{cmsg_len=%u, cmsg_level=SOL_SOCKET"
	       ", cmsg_type=SCM_PIDFD, cmsg_data=%d<socket:[%lu]>}]"
	       ", msg_controllen=%u, msg_flags=0}, 0) = 1 (INJECTED)\n",
	       sv[0], inode_of_sockfd(sv[0]), cmsg_size,
	       pv[0], inode_of_sockfd(pv[0]), cmsg_size);

	puts("+++ exited with 0 +++");
	return 0;
}
