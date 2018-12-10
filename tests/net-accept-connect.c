/*
 * Copyright (c) 2013-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2013-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

static void
handler(int sig)
{
	assert(close(1) == 0);
	_exit(0);
}

int
main(int ac, const char **av)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
	};

	assert(ac == 2);
	socklen_t len = strlen(av[1]);
	assert(len > 0 && len <= sizeof(addr.sun_path));

	if (++len > sizeof(addr.sun_path))
		len = sizeof(addr.sun_path);

	memcpy(addr.sun_path, av[1], len);
	len += offsetof(struct sockaddr_un, sun_path);

	unlink(av[1]);
	close(0);
	close(1);

	if (socket(AF_UNIX, SOCK_STREAM, 0))
		perror_msg_and_skip("socket");
	if (bind(0, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");
	if (listen(0, 5))
		perror_msg_and_skip("listen");

	memset(&addr, 0, sizeof(addr));
	assert(getsockname(0, (struct sockaddr *) &addr, &len) == 0);
	if (len > sizeof(addr))
		len = sizeof(addr);

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (pid) {
		assert(accept(0, (struct sockaddr *) &addr, &len) == 1);
		assert(close(0) == 0);
		assert(kill(pid, SIGUSR1) == 0);
		int status;
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
		assert(close(1) == 0);
	} else {
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGUSR1);

		assert(sigprocmask(SIG_BLOCK, &set, NULL) == 0);
		assert(signal(SIGUSR1, handler) != SIG_ERR);
		assert(socket(AF_UNIX, SOCK_STREAM, 0) == 1);
		assert(close(0) == 0);
		assert(connect(1, (struct sockaddr *) &addr, len) == 0);
		assert(sigprocmask(SIG_UNBLOCK, &set, NULL) == 0);
		assert(pause() == 99);
		return 1;
	}

	unlink(av[1]);
	return 0;
}
