/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "scno.h"

#ifndef __NR_send
# define __NR_send -1
#endif
#define SC_send 9

#ifndef __NR_recv
# define __NR_recv -1
#endif
#define SC_recv 10

static int
sys_send(int sockfd, const void *buf, size_t len, int flags)
{
	int rc = socketcall(__NR_send, SC_send,
			    sockfd, (long) buf, len, flags, 0);
	if (rc < 0 && ENOSYS == errno)
		perror_msg_and_skip("send");
	return rc;
}

static int
sys_recv(int sockfd, const void *buf, size_t len, int flags)
{
	int rc = socketcall(__NR_recv, SC_recv,
			    sockfd, (long) buf, len, flags, 0);
	if (rc < 0 && ENOSYS == errno)
		perror_msg_and_skip("recv");
	return rc;
}

static void
transpose(char *str, const size_t len)
{
	size_t i;

	for (i = 0; i < len / 2; ++i) {
		char c = str[i];
		str[i] = str[len - 1 - i];
		str[len - 1 - i] = c;
	}
}

int
main(int ac, char **av)
{
	assert(ac == 2);
	const size_t len = strlen(av[1]);
	assert(len);
	char *const buf0 = tail_alloc(len);
	char *const buf1 = tail_alloc(len);
	memcpy(buf0, av[1], len);

	(void) close(0);
	(void) close(1);

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	assert(sys_send(0, buf0, len, MSG_DONTROUTE) == (int) len);
	assert(sys_recv(1, buf1, len, MSG_WAITALL) == (int) len);

	transpose(buf1, len);
	assert(sys_send(1, buf1, len, MSG_DONTROUTE) == (int) len);
	if (close(1))
		perror_msg_and_fail("close(1)");

	assert(sys_recv(0, buf0, len, MSG_WAITALL) == (int) len);
	if (close(0))
		perror_msg_and_fail("close(0)");
	assert(sys_recv(0, NULL, len, MSG_DONTWAIT) == -1);

	return 0;
}
