/*
 * Check decoding of SCM_PIDFD control messages.
 *
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#define XLAT_MACROS_ONLY
# include "xlat/sock_options.h"
# include "xlat/scmvals.h"
#undef XLAT_MACROS_ONLY

static void
print_pidfd(const struct cmsghdr *c)
{
	const void *cmsg_header = c;
	const void *cmsg_data = CMSG_DATA(c);
	int pidfd;
	const unsigned int expected_len = sizeof(pidfd);
	const unsigned int data_len = c->cmsg_len - (cmsg_data - cmsg_header);

	if (expected_len != data_len)
		perror_msg_and_fail("sizeof(pidfd) = %u, data_len = %u\n",
				    expected_len, data_len);

	memcpy(&pidfd, cmsg_data, sizeof(pidfd));
	printf("%d<anon_inode:[pidfd]>", pidfd);
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
                perror_msg_and_skip("socketpair AF_UNIX SOCK_STREAM");

	int one = 1;
	if (setsockopt(sv[0], SOL_SOCKET, SO_PASSPIDFD, &one, sizeof(one)))
		perror_msg_and_skip("setsockopt SO_PASSPIDFD");

	char sym = 'A';
	if (send(sv[1], &sym, 1, 0) != 1)
		perror_msg_and_fail("send");
	if (close(sv[1]))
		perror_msg_and_fail("close send");

	int pidfd;
	unsigned int cmsg_size = CMSG_SPACE(sizeof(pidfd));
	struct cmsghdr *cmsg = tail_alloc(cmsg_size);
	memset(cmsg, 0, cmsg_size);

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

	if (recvmsg(sv[0], &mh, 0) != 1)
		perror_msg_and_fail("recvmsg");

	printf("recvmsg(%d<socket:[%lu]>, {msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1",
	       sv[0], inode_of_sockfd(sv[0]));

	bool found = false;
	if (mh.msg_controllen) {
		printf(", msg_control=[");
		for (struct cmsghdr *c = CMSG_FIRSTHDR(&mh); c;
		     c = CMSG_NXTHDR(&mh, c)) {
			printf("%s{cmsg_len=%lu, cmsg_level=",
			       (c == cmsg ? "" : ", "),
			       (unsigned long) c->cmsg_len);
			if (c->cmsg_level == SOL_SOCKET) {
				printf("SOL_SOCKET");
			} else {
				printf("%d /* expected SOL_SOCKET == %d */",
				       c->cmsg_level, (int) SOL_SOCKET);
			}
			printf(", cmsg_type=");
			if (c->cmsg_type == SCM_PIDFD) {
				printf("SCM_PIDFD, cmsg_data=");
				print_pidfd(c);
				found = true;
			} else {
				printf("%d /* expected SCM_PIDFD == %d */",
				       c->cmsg_type, (int) SCM_PIDFD);
			}
			printf("}");
		}
		printf("]");
	}
	printf(", msg_controllen=%lu, msg_flags=0}, 0) = 1\n",
	       (unsigned long) mh.msg_controllen);

	if (!found)
		error_msg_and_fail("SCM_PIDFD not found");

	puts("+++ exited with 0 +++");
	return 0;
}
