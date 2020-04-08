/*
 * Check decoding of sockaddr related arguments of recvfrom syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_SYSCALL_NAME recvfrom
#define TEST_SYSCALL_PREPARE send_un()
#define PREFIX_S_ARGS	, recv_buf, 1, 0
#define PREFIX_S_STR	", \"A\", 1, 0"
#define PREFIX_F_ARGS	, 0, 1, 0
#define PREFIX_F_STR	", NULL, 1, 0"
static void send_un(void);
static char recv_buf[1];
#include "sockname.c"

static int cfd;

static void
send_un(void)
{
	if (send(cfd, "A", 1, 0) != 1)
		perror_msg_and_skip("send");
}

int
main(void)
{
	cfd = socket(AF_UNIX, SOCK_STREAM, 0);
	int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (cfd < 0 || lfd < 0)
		perror_msg_and_skip("socket");

	struct sockaddr_un un = {
		.sun_family = AF_UNIX,
		.sun_path = TEST_SOCKET ".send"
	};

	(void) unlink(un.sun_path);
	if (bind(cfd, (const void *) &un, sizeof(un)))
		perror_msg_and_skip("bind");
	(void) unlink(un.sun_path);

	un.sun_path[sizeof(TEST_SOCKET) - 1] = '\0';
	(void) unlink(un.sun_path);

	if (bind(lfd, (const void *) &un, sizeof(un)))
		perror_msg_and_skip("bind");

	if (listen(lfd, 1))
		perror_msg_and_skip("listen");

	if (connect(cfd, (const void *) &un, sizeof(un)))
		perror_msg_and_skip("connect");

	int afd = accept(lfd, 0, 0);
	if (afd < 0)
		perror_msg_and_skip("accept");

	(void) unlink(un.sun_path);

	test_sockname_syscall(afd);

	puts("+++ exited with 0 +++");
	return 0;
}
