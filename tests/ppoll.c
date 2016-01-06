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
#include <poll.h>
#include <signal.h>
#include <unistd.h>

static void
test1(void)
{
	const struct timespec timeout = { .tv_sec = 42, .tv_nsec = 999999999 };
	struct pollfd fds[] = {
		{ .fd = 0, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 1, .events = POLLOUT | POLLWRNORM | POLLWRBAND },
		{ .fd = 3, .events = POLLIN | POLLPRI },
		{ .fd = 4, .events = POLLOUT }
	};

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	int rc = ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask);
	if (rc < 0)
		perror_msg_and_skip("ppoll");
	assert(rc == 2);
}

static void
test2(void)
{
	const struct timespec timeout = { .tv_sec = 0, .tv_nsec = 999 };
	struct pollfd fds[] = {
		{ .fd = 1, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 0, .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};

	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);

	int rc = ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask);
	if (rc < 0)
		perror_msg_and_skip("ppoll");
	assert(rc == 0);
}

int
main(void)
{
	int fds[2];

	(void) close(0);
	(void) close(1);
	(void) close(3);
	(void) close(4);
	if (pipe(fds) || pipe(fds))
		perror_msg_and_fail("pipe");

	test1();
	test2();

	assert(ppoll(NULL, 42, NULL, NULL) < 0);
	return 0;
}
