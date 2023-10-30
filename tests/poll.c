/*
 * Check decoding of poll syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_poll

# include <assert.h>
# include <errno.h>
# include <poll.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

# define PRINT_EVENT(flag, member)			\
	do {						\
		if (member & flag) {			\
			if (member != pfd->member)	\
				tprintf("|");		\
			tprintf(#flag);			\
			member &= ~flag;		\
		}					\
	} while (0)

static void
print_pollfd_entering(const struct pollfd *const pfd)
{
	tprintf("{fd=%d", pfd->fd);
	if (pfd->fd >= 0) {
		tprintf(", events=");
		short events = pfd->events;

		if (pfd->events) {
			PRINT_EVENT(POLLIN, events);
			PRINT_EVENT(POLLPRI, events);
			PRINT_EVENT(POLLOUT, events);
# ifdef POLLRDNORM
			PRINT_EVENT(POLLRDNORM, events);
# endif
# ifdef POLLWRNORM
			PRINT_EVENT(POLLWRNORM, events);
# endif
# ifdef POLLRDBAND
			PRINT_EVENT(POLLRDBAND, events);
# endif
# ifdef POLLWRBAND
			PRINT_EVENT(POLLWRBAND, events);
# endif
			PRINT_EVENT(POLLERR, events);
			PRINT_EVENT(POLLHUP, events);
			PRINT_EVENT(POLLNVAL, events);
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
	for (unsigned int i = 0; i < size; ++i) {
		if (i)
			tprintf(", ");
		if (i >= valid) {
			tprintf("... /* %p */", &pfd[i]);
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

	PRINT_EVENT(POLLIN, revents);
	PRINT_EVENT(POLLPRI, revents);
	PRINT_EVENT(POLLOUT, revents);
# ifdef POLLRDNORM
	PRINT_EVENT(POLLRDNORM, revents);
# endif
# ifdef POLLWRNORM
	PRINT_EVENT(POLLWRNORM, revents);
# endif
# ifdef POLLRDBAND
	PRINT_EVENT(POLLRDBAND, revents);
# endif
# ifdef POLLWRBAND
	PRINT_EVENT(POLLWRBAND, revents);
# endif
	PRINT_EVENT(POLLERR, revents);
	PRINT_EVENT(POLLHUP, revents);
	PRINT_EVENT(POLLNVAL, revents);
	tprintf("}");
}

static void
print_pollfd_array_exiting(const struct pollfd *const pfd,
			   const unsigned int size,
			   const unsigned int abbrev)
{
	tprintf("[");
	unsigned int seen = 0;
	for (unsigned int i = 0; i < size; ++i)
		print_pollfd_exiting(&pfd[i], &seen, abbrev);
	tprintf("]");
}

int
main(int ac, char **av)
{
# ifdef PATH_TRACING_FD
	skip_if_unavailable("/proc/self/fd/");
# endif

	tprintf("%s", "");

	assert(syscall(__NR_poll, NULL, 42, 0) == -1);
	if (ENOSYS == errno)
		perror_msg_and_skip("poll");

# ifndef PATH_TRACING_FD
	tprintf("poll(NULL, 42, 0)" RVAL_EFAULT);
# endif

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

# ifndef PATH_TRACING_FD
	tprintf("poll([], 0, %d) = %d (Timeout)\n", timeout, rc);
# endif

	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 3);

# ifndef PATH_TRACING_FD
	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (",
		(unsigned int) ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");
# endif /* !PATH_TRACING_FD */

	tail_fds0[0].fd = -1;
	tail_fds0[2].fd = -3;
	tail_fds0[4].events = 0;
	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 2);

# ifndef PATH_TRACING_FD
	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (",
		(unsigned int) ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");
# endif /* !PATH_TRACING_FD */

	tail_fds0[1].fd = -2;
	tail_fds0[4].fd = -5;
	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 1);

# ifndef PATH_TRACING_FD
	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (",
		(unsigned int) ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");
# endif /* !PATH_TRACING_FD */

	struct pollfd pfds1[] = {
		{ .fd = 1, .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = 0, .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};
	struct pollfd *const tail_fds1 = tail_memdup(pfds1, sizeof(pfds1));
	rc = syscall(__NR_poll, tail_fds1, ARRAY_SIZE(pfds1), timeout);
	assert(rc == 0);

# ifndef PATH_TRACING_FD
	tprintf("poll(");
	print_pollfd_array_entering(tail_fds1, ARRAY_SIZE(pfds1),
				    ARRAY_SIZE(pfds1), abbrev);
	tprintf(", %u, %d) = %d (Timeout)\n",
		(unsigned int) ARRAY_SIZE(pfds1), timeout, rc);
# endif /* !PATH_TRACING_FD */

	const void *const efault = tail_fds0 + ARRAY_SIZE(pfds0);
	rc = syscall(__NR_poll, efault, 1, 0);
	assert(rc == -1);

# ifndef PATH_TRACING_FD
	tprintf("poll(%p, 1, 0)" RVAL_EFAULT, efault);
# endif

	const unsigned int valid = 1;
	const void *const epfds = tail_fds0 + ARRAY_SIZE(pfds0) - valid;
	rc = syscall(__NR_poll, epfds, valid + 1, 0);
	assert(rc == -1);

# ifndef PATH_TRACING_FD
	tprintf("poll(");
	print_pollfd_array_entering(epfds, valid + 1, valid, abbrev);
	errno = EFAULT;
	tprintf(", %u, 0)" RVAL_EFAULT, valid + 1);
# endif /* !PATH_TRACING_FD */

# ifdef PATH_TRACING_FD
	memcpy(tail_fds0, pfds0, sizeof(pfds0));
	tail_fds0[4].fd = PATH_TRACING_FD;

	rc = syscall(__NR_poll, tail_fds0, ARRAY_SIZE(pfds0), timeout);
	assert(rc == 3);

	tprintf("poll(");
	print_pollfd_array_entering(tail_fds0, ARRAY_SIZE(pfds0),
				    ARRAY_SIZE(pfds0), abbrev);
	tprintf(", %u, %d) = %d (",
		(unsigned int) ARRAY_SIZE(pfds0), timeout, rc);
	print_pollfd_array_exiting(tail_fds0, ARRAY_SIZE(pfds0), abbrev);
	tprintf(")\n");

	rc = syscall(__NR_poll, epfds, valid + 1, 0);
	assert(rc == -1);

	/* the 1st pollfd element is readable and contains PATH_TRACING_FD */
	tprintf("poll(");
	print_pollfd_array_entering(epfds, valid + 1, valid, abbrev);
	errno = EFAULT;
	tprintf(", %u, 0)" RVAL_EFAULT, valid + 1);
# endif /* PATH_TRACING_FD */

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_poll")

#endif
