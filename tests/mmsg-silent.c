/*
 * Check silent decoding of sendmmsg and recvmmsg syscalls.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>

#include "msghdr.h"

int
main(void)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
		perror_msg_and_skip("socketpair");

	char buf = 'A';
	struct iovec iov = { .iov_base = &buf, .iov_len = sizeof(buf) };
	struct mmsghdr mh = {
		.msg_hdr = {
			.msg_iov = &iov,
			.msg_iovlen = 1
		}
	};

	int rc = send_mmsg(fds[1], &mh, 1, MSG_DONTWAIT);
	if (rc < 0)
		perror_msg_and_skip("sendmmsg");
	printf("sendmmsg(%d, %p, 1, MSG_DONTWAIT) = %d\n", fds[1], &mh, rc);

	struct timespec t = { .tv_sec = 0, .tv_nsec = 12345678 };
	rc = recv_mmsg(fds[0], &mh, 1, MSG_DONTWAIT, &t);
	printf("recvmmsg(%d, %p, 1, MSG_DONTWAIT, %p) = %d\n",
	       fds[0], &mh, &t, rc);

	puts("+++ exited with 0 +++");
	return 0;
}
