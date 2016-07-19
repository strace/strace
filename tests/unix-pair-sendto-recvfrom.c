/*
 * Check decoding and dumping of sendto and recvfrom syscalls.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
