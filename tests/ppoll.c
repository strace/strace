/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <poll.h>
#include <signal.h>
#include <unistd.h>

static int
test1(void)
{
	sigset_t mask;
	const struct timespec timeout = { .tv_sec = 42, .tv_nsec = 999999999 };
	struct pollfd fds[] = {
		{ .fd = 0, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 1, .events = POLLOUT | POLLWRNORM | POLLWRBAND },
		{ .fd = 3, .events = POLLIN | POLLPRI },
		{ .fd = 4, .events = POLLOUT }
	};

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);

	return ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask) == 2 ? 0 : 77;
}

static int
test2(void)
{
	sigset_t mask;
	const struct timespec timeout = { .tv_sec = 0, .tv_nsec = 999 };
	struct pollfd fds[] = {
		{ .fd = 1, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 0, .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};

	sigfillset(&mask);
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);

	return ppoll(fds, sizeof(fds) / sizeof(*fds), &timeout, &mask) == 0 ? 0 : 77;
}

int
main(void)
{
	int rc;
	int fds[2];

	(void) close(0);
	(void) close(1);
	(void) close(3);
	(void) close(4);
	if (pipe(fds) || pipe(fds))
		return 77;


	if ((rc = test1()))
		return rc;

	if ((rc = test2()))
		return rc;

	return ppoll(NULL, 42, NULL, NULL) < 0 ? 0 : 77;
}
