/*
 * Check decoding of timeout argument of recvmmsg syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
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

	if (send(fds[1], "A", 1, 0) != 1)
		perror_msg_and_skip("send");

	char buf;
	struct iovec iov = { .iov_base = &buf, .iov_len = sizeof(buf) };
	struct mmsghdr mh = {
		.msg_hdr = {
			.msg_iov = &iov,
			.msg_iovlen = 1
		}
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timespec, ts);
	ts->tv_sec = 0;
	ts->tv_nsec = 12345678;

	int rc = recv_mmsg(-1, &mh, 1, 0, ts);
	printf("recvmmsg(-1, %p, 1, 0, {tv_sec=0, tv_nsec=12345678})"
	       " = %s\n", &mh, sprintrc(rc));

	rc = recv_mmsg(fds[0], &mh, 1, 0, ts);
	if (rc < 0)
		perror_msg_and_skip("recvmmsg");
	printf("recvmmsg(%d, [{msg_hdr={msg_name=NULL, msg_namelen=0"
	       ", msg_iov=[{iov_base=\"A\", iov_len=1}], msg_iovlen=1"
	       ", msg_controllen=0, msg_flags=0}, msg_len=1}], 1, 0"
	       ", {tv_sec=0, tv_nsec=12345678}) = "
	       "%d (left {tv_sec=0, tv_nsec=%d})\n",
	       fds[0], rc, (int) ts->tv_nsec);

	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;

	rc = recv_mmsg(fds[0], &mh, 1, 0, ts);
	printf("recvmmsg(%d, %p, 1, 0, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       fds[0], &mh, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	ts->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (long) 0xbadc0dedfacefeedLL;

	rc = recv_mmsg(fds[0], &mh, 1, 0, ts);
	printf("recvmmsg(%d, %p, 1, 0, {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       fds[0], &mh, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
