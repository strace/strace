/*
 * This file is part of poll strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <asm/unistd.h>

#ifdef __NR_poll

# include <assert.h>
# include <errno.h>
# include <poll.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

#define PRINT_EVENT(flag, member) \
	if (member & flag) { \
		if (member != pfd->member) \
			tprintf("|"); \
		tprintf(#flag); \
		member &= ~flag; \
	}

static void
print_pollfd_entering(const struct pollfd *const pfd)
{
	tprintf("{fd=%d", pfd->fd);
	if (pfd->fd >= 0) {
		tprintf(", events=");
		short events = pfd->events;

		if (pfd->events) {
			PRINT_EVENT(POLLIN, events)
			PRINT_EVENT(POLLPRI, events)
			PRINT_EVENT(POLLOUT, events)
#ifdef POLLRDNORM
			PRINT_EVENT(POLLRDNORM, events)
#endif
#ifdef POLLWRNORM
			PRINT_EVENT(POLLWRNORM, events)
#endif
#ifdef POLLRDBAND
			PRINT_EVENT(POLLRDBAND, events)
#endif
#ifdef POLLWRBAND
			PRINT_EVENT(POLLWRBAND, events)
#endif
			PRINT_EVENT(POLLERR, events)
			PRINT_EVENT(POLLHUP, events)
			PRINT_EVENT(POLLNVAL, events)
		} else
			tprintf("0");
	}
	tprintf("}");
}

static void
print_pollfd_array_entering(const struct pollfd *const pfd,
			    const unsigned int size,
			    const unsigned int valid,
			    const unsigned int abbrev)
{
	tprintf("[");
	unsigned int i;
	for (i = 0; i < size; ++i) {
		if (i)
			tprintf(", ");
		if (i >= valid) {
			tprintf("%p", &pfd[i]);
			break;
		}
		if (i >= abbrev) {
			tprintf("...");
			break;
		}
		print_pollfd_entering(&pfd[i]);
	}
	tprintf("]");
}

static void
print_pollfd_exiting(const struct pollfd *const pfd,
		     unsigned int *const seen,
		     const unsigned int abbrev)
{
	if (!pfd->revents || pfd->fd < 0 || *seen > abbrev)
		return;

	if (*seen)
		tprintf(", ");
	++(*seen);

	if (*seen > abbrev) {
		tprintf("...");
		return;
	}
	tprintf("{fd=%d, revents=", pfd->fd);
	short revents = pfd->revents;

	PRINT_EVENT(POLLIN, revents)
	PRINT_EVENT(POLLPRI, revents)
	PRINT_EVENT(POLLOUT, revents)
#ifdef POLLRDNORM
	PRINT_EVENT(POLLRDNORM, revents)
#endif
#ifdef POLLWRNORM
	PRINT_EVENT(POLLWRNORM, revents)
#endif
#ifdef POLLRDBAND
	PRINT_EVENT(POLLRDBAND, revents)
#endif
#ifdef POLLWRBAND
	PRINT_EVENT(POLLWRBAND, revents)
#endif
	PRINT_EVENT(POLLERR, revents)
	PRINT_EVENT(POLLHUP, revents)
	PRINT_EVENT(POLLNVAL, revents)
	tprintf("}");
}

static void
print_pollfd_array_exiting(const struct pollfd *const pfd,
			   const unsigned int size,
			   const unsigned int abbrev)
{
	tprintf("[");
	unsigned int seen = 0;
	unsigned int i;
	for (i = 0; i < size; ++i)
		print_pollfd_exiting(&pfd[i], &seen, abbrev);
	tprintf("]");
}

int
main(int ac, char **av)
{
	tprintf("%s", "");

	assert(syscall(__NR_poll, NULL, 42, 0) == -1);
	if (ENOSYS == errno)
		perror_msg_and_skip("poll");
	tprintf("poll(NULL, 42, 0) = -1 EFAULT (%m)\n");

	int fds[2];
	if (pipe(fds) || pipe(fds))
		perror_msg_and_fail("pipe");

	const unsigned int abbrev = (ac > 1) ? atoi(av[1]) : -1;
	const struct pollfd pfds0[] = {
		{ .fd = 0, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 1, .events = POLLOUT | POLLWRNORM | POLLWRBAND },
		{ .fd = fds[0], .events = POLLIN | POLLPRI },
		{ .fd = fds[1], .events = POLLOUT },
		{ .fd = 2, .events = POLLOUT | POLLWRBAND }
	};
	struct pollfd *const tail_fds0 = tail_memdup(pfds0, sizeof(pfds0));
	const int timeout = 42;
	int rc = syscall(__NR_poll, tail_fds0, 0, timeout);
	assert(rc == 0);

	tprintf("poll([], 0, %d) = %d (Timeout)\n", timeout, rc);

	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 3);

	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (", ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");

	tail_fds0[0].fd = -1;
	tail_fds0[2].fd = -3;
	tail_fds0[4].events = 0;
	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 2);

	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (", ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");

	tail_fds0[1].fd = -2;
	tail_fds0[4].fd = -5;
	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 1);

	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (", ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");

	struct pollfd pfds1[] = {
		{ .fd = 1, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 0, .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};
	struct pollfd *const tail_fds1 = tail_memdup(pfds1, sizeof(pfds1));
	rc = syscall(__NR_poll, tail_fds1, ARRAY_SIZE(pfds1), timeout);
	assert(rc == 0);

	tprintf("poll(");
	print_pollfd_array_entering(tail_fds1, ARRAY_SIZE(pfds1),
				    ARRAY_SIZE(pfds1), abbrev);
	tprintf(", %u, %d) = %d (Timeout)\n", ARRAY_SIZE(pfds1), timeout, rc);

	const void *const efault = tail_fds0 + ARRAY_SIZE(pfds0);
	rc = syscall(__NR_poll, efault, 1, 0);
	assert(rc == -1);
	tprintf("poll(%p, 1, 0) = -1 EFAULT (%m)\n", efault);

	const unsigned int valid = 1;
	const void *const epfds = tail_fds0 + ARRAY_SIZE(pfds0) - valid;
	rc = syscall(__NR_poll, epfds, valid + 1, 0);
	assert(rc == -1);
	tprintf("poll(");
	print_pollfd_array_entering(epfds, valid + 1, valid, abbrev);
	errno = EFAULT;
	tprintf(", %u, 0) = -1 EFAULT (%m)\n", valid + 1);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_poll")

#endif
