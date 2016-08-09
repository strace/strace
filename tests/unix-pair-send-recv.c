/*
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
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <asm/unistd.h>

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
