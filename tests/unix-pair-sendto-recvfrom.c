/*
 * Check decoding and dumping of sendto and recvfrom syscalls.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>

static void
transpose(char *str, int len)
{
	int i;

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
	const int len = strlen(av[1]);
	assert(len);

	(void) close(0);
	(void) close(1);

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (pid) {
		assert(close(1) == 0);
		transpose(av[1], len);
		assert(sendto(0, av[1], len, MSG_DONTROUTE, NULL, 0) == len);
		assert(recvfrom(0, av[1], len, MSG_WAITALL, NULL, NULL) == len);
		assert(close(0) == 0);

		int status;
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
	} else {
		assert(close(0) == 0);
		assert(recvfrom(1, av[1], len, MSG_WAITALL, NULL, NULL) == len);
		transpose(av[1], len);
		assert(sendto(1, av[1], len, MSG_DONTROUTE, NULL, 0) == len);
		assert(recvfrom(1, av[1], len, MSG_WAITALL, NULL, NULL) == 0);
		assert(close(1) == 0);
	}

	return 0;
}
