/*
 * Copyright (c) 2013-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
	socklen_t len;

	assert(ac == 2);
	assert(strlen(av[1]) > 0);

	strncpy(addr.sun_path, av[1], sizeof(addr.sun_path));
	len = offsetof(struct sockaddr_un, sun_path) + strlen(av[1]) + 1;
	if (len > sizeof(addr))
		len = sizeof(addr);

	unlink(av[1]);
	close(0);
	close(1);

	if (socket(AF_UNIX, SOCK_STREAM, 0))
		perror_msg_and_skip("socket");
	if (bind(0, (struct sockaddr *) &addr, len))
		perror_msg_and_skip("bind");
	if (listen(0, 5))
		perror_msg_and_skip("listen");

	memset(&addr, 0, sizeof addr);
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
