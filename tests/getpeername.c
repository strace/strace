/*
 * Check decoding of getpeername syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_SYSCALL_NAME getpeername
#include "sockname.c"

int
main(void)
{
	int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
	int cfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (lfd < 0 || cfd < 0)
		perror_msg_and_skip("socket");

	(void) unlink(TEST_SOCKET);

	const struct sockaddr_un un = {
		.sun_family = AF_UNIX,
		.sun_path = TEST_SOCKET
	};

	if (bind(lfd, (const void *) &un, sizeof(un)))
		perror_msg_and_skip("bind");
	if (listen(lfd, 1))
		perror_msg_and_skip("listen");
	if (connect(cfd, (const void *) &un, sizeof(un)))
		perror_msg_and_skip("connect");
	if (accept(lfd, 0, 0) < 0)
		perror_msg_and_skip("accept");

	test_sockname_syscall(cfd);

	(void) unlink(TEST_SOCKET);

	puts("+++ exited with 0 +++");
	return 0;
}
