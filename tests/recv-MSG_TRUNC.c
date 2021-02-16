/*
 * Check decoding of recv MSG_TRUNC.
 *
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include "scno.h"

#ifndef __NR_recv
# define __NR_recv -1
#endif
#define SC_recv 10

static int
sys_recv(int sockfd, const void *buf, unsigned int len, int flags)
{
	int rc = socketcall(__NR_recv, SC_recv,
			    sockfd, (long) buf, len, flags, 0);
	if (rc < 0 && ENOSYS == errno)
		perror_msg_and_skip("recv");
	return rc;
}

int
main(void)
{
	static const char sbuf[2] = "AB";
	int sv[2];
	TAIL_ALLOC_OBJECT_CONST_PTR(char, rbuf);

	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv))
		perror_msg_and_skip("socketpair");

	if (send(sv[1], sbuf + 1, 1, 0) != 1)
                perror_msg_and_skip("send");
	if (sys_recv(sv[0], rbuf - 1, 2, MSG_PEEK) != 1)
		perror_msg_and_fail("recv");
	printf("recv(%d, \"B\", 2, MSG_PEEK) = 1\n", sv[0]);

	if (sys_recv(sv[0], rbuf, 1, MSG_TRUNC) != 1)
		perror_msg_and_skip("recv");
	printf("recv(%d, \"B\", 1, MSG_TRUNC) = 1\n", sv[0]);

	if (send(sv[1], sbuf, 2, 0) != 2)
                perror_msg_and_skip("send");
	if (sys_recv(sv[0], rbuf, 1, MSG_TRUNC) != 2)
		perror_msg_and_skip("recv");
	printf("recv(%d, \"A\", 1, MSG_TRUNC) = 2\n", sv[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
