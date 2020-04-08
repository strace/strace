/*
 * Check decoding of recvfrom MSG_TRUNC.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>

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
	if (recvfrom(sv[0], rbuf - 1, 2, MSG_PEEK, NULL, NULL) != 1)
		perror_msg_and_fail("recvfrom");
	printf("recvfrom(%d, \"B\", 2, MSG_PEEK, NULL, NULL) = 1\n", sv[0]);

	if (recvfrom(sv[0], rbuf, 1, MSG_TRUNC, NULL, NULL) != 1)
		perror_msg_and_skip("recvfrom");
	printf("recvfrom(%d, \"B\", 1, MSG_TRUNC, NULL, NULL) = 1\n", sv[0]);

	if (send(sv[1], sbuf, 2, 0) != 2)
                perror_msg_and_skip("send");
	if (recvfrom(sv[0], rbuf, 1, MSG_TRUNC, NULL, NULL) != 2)
		perror_msg_and_skip("recvfrom");
	printf("recvfrom(%d, \"A\", 1, MSG_TRUNC, NULL, NULL) = 2\n", sv[0]);

	puts("+++ exited with 0 +++");
	return 0;
}
