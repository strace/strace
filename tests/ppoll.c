/*
 * Check decoding of ppoll syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_ppoll

# include <errno.h>
# include <poll.h>
# include <signal.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# ifndef PATH_TRACING_FD
#  define PATH_TRACING_FD 0
# endif
# ifndef TRACING_FDS
#  define TRACING_FDS 0
# endif
# ifndef TRACING_FD1
#  define TRACING_FD1 0
# endif
# ifndef TRACING_FD2
#  define TRACING_FD2 0
# endif
# ifndef TRACE_FD1
#  define TRACE_FD1 (!PATH_TRACING_FD && (!TRACING_FDS || TRACING_FD1))
# endif
# ifndef TRACE_FD2
#  define TRACE_FD2 (!PATH_TRACING_FD && (!TRACING_FDS || TRACING_FD2))
# endif
# ifndef TRACE_OTHER_FDS
#  define TRACE_OTHER_FDS (!PATH_TRACING_FD && !TRACING_FDS)
# endif

static const char *errstr;

static long
sys_ppoll(const kernel_ulong_t ufds,
	  const kernel_ulong_t nfds,
	  const kernel_ulong_t tsp,
	  const kernel_ulong_t sigmask,
	  const kernel_ulong_t sigsetsize)
{
	long rc = syscall(__NR_ppoll, ufds, nfds, tsp, sigmask, sigsetsize);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
# if PATH_TRACING_FD
	skip_if_unavailable("/proc/self/fd/");
# endif

	static const kernel_ulong_t bogus_nfds =
		(kernel_ulong_t) 0xdeadbeeffacefeedULL;
	static const kernel_ulong_t bogus_sigsetsize =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	static const char *const POLLWRNORM_str =
		(POLLWRNORM == POLLOUT) ? "" : "|POLLWRNORM";
	static const char *const USR2_CHLD_str =
		(SIGUSR2 < SIGCHLD) ? "USR2 CHLD" : "CHLD USR2";
	void *const efault = tail_alloc(1024) + 1024;
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_old_timespec_t, ts);
	const unsigned int sigset_size = get_sigset_size();
	void *const sigmask = tail_alloc(sigset_size);
	struct pollfd *fds;
	sigset_t mask;
	int pipe_fd[4];
	long rc;

	sys_ppoll(0, bogus_nfds, 0, 0, bogus_sigsetsize);
	if (ENOSYS == errno)
		perror_msg_and_skip("ppoll");
# if !PATH_TRACING_FD && !TRACING_FDS
	printf("ppoll(NULL, %u, NULL, NULL, %llu) = %s\n",
	       (unsigned) bogus_nfds, (unsigned long long) bogus_sigsetsize,
	       errstr);
# endif

	sys_ppoll((unsigned long) efault, 42, (unsigned long) efault + 8,
		  (unsigned long) efault + 16, sigset_size);
# if !PATH_TRACING_FD && !TRACING_FDS
	printf("ppoll(%p, %u, %p, %p, %u) = %s\n",
	       efault, 42, efault + 8, efault + 16, sigset_size, errstr);
# endif

	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	sys_ppoll(0, 0, (unsigned long) ts, 0, sigset_size);
# if !PATH_TRACING_FD && !TRACING_FDS
	printf("ppoll(NULL, 0, {tv_sec=%lld, tv_nsec=%llu}, NULL, %u) = %s\n",
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec),
	       sigset_size, errstr);
# endif

	ts->tv_sec = (typeof(ts->tv_sec)) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (long) 0xbadc0dedfacefeedL;
	sys_ppoll(0, 0, (unsigned long) ts, 0, sigset_size);
# if !PATH_TRACING_FD && !TRACING_FDS
	printf("ppoll(NULL, 0, {tv_sec=%lld, tv_nsec=%llu}, NULL, %u) = %s\n",
	       (long long) ts->tv_sec, zero_extend_signed_to_ull(ts->tv_nsec),
	       sigset_size, errstr);
# endif

	if (pipe(pipe_fd) || pipe(pipe_fd + 2))
		perror_msg_and_fail("pipe");

# if TRACING_FD1
	if (dup2(pipe_fd[3], TRACING_FD1) < 0)
		perror_msg_and_fail("dup2 1");
	close(pipe_fd[3]);
	pipe_fd[3] = TRACING_FD1;
# endif

# if TRACING_FD2
	if (dup2(pipe_fd[1], TRACING_FD2) < 0)
		perror_msg_and_fail("dup2 2");
	close(pipe_fd[1]);
	pipe_fd[1] = TRACING_FD2;
# endif

	ts->tv_sec = 42;
	ts->tv_nsec = 999999999;

	const struct pollfd fds1[] = {
		{ .fd = pipe_fd[0], .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = pipe_fd[1], .events = POLLOUT | POLLWRNORM | POLLWRBAND },
		{ .fd = pipe_fd[2], .events = POLLIN | POLLPRI },
		{ .fd = pipe_fd[3], .events = POLLOUT }
	};
	fds = efault - sizeof(fds1);
	memcpy(fds, fds1, sizeof(fds1));

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR2);
	sigaddset(&mask, SIGCHLD);
	memcpy(sigmask, &mask, sigset_size);

	rc = sys_ppoll((unsigned long) fds,
		       F8ILL_KULONG_MASK | ARRAY_SIZE(fds1), (unsigned long) ts,
		       (unsigned long) sigmask, sigset_size);
	if (rc != 2)
		perror_msg_and_fail("ppoll 1");
# if TRACE_FD1 || TRACE_FD2 || TRACE_OTHER_FDS
	printf("ppoll([{fd=%d, events=POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND}"
	       ", {fd=%d, events=POLLOUT%s|POLLWRBAND}"
#  if VERBOSE
	       ", {fd=%d, events=POLLIN|POLLPRI}, {fd=%d, events=POLLOUT}]"
#  else
	       ", ...]"
#  endif
	       ", %u, {tv_sec=42, tv_nsec=999999999}, [%s], %u) = %ld"
	       " ([{fd=%d, revents=POLLOUT%s}, {fd=%d, revents=POLLOUT}]"
	       ", left {tv_sec=%u, tv_nsec=%u})\n",
	       pipe_fd[0], pipe_fd[1], POLLWRNORM_str,
#  if VERBOSE
	       pipe_fd[2], pipe_fd[3],
#  endif
	       (unsigned int) ARRAY_SIZE(fds1), USR2_CHLD_str,
	       (unsigned int) sigset_size, rc, pipe_fd[1], POLLWRNORM_str,
	       pipe_fd[3], (unsigned int) ts->tv_sec,
	       (unsigned int) ts->tv_nsec);
# endif /* !PATH_TRACING_FDS */

	ts->tv_sec = 23;
	ts->tv_nsec = 123456789;

	rc = sys_ppoll((unsigned long) fds,
		       F8ILL_KULONG_MASK | (ARRAY_SIZE(fds1) - 2),
		       (unsigned long) ts,
		       (unsigned long) sigmask, sigset_size);
	if (rc != 1)
		perror_msg_and_fail("ppoll 1.5");
# if TRACE_FD2 || TRACE_OTHER_FDS
	printf("ppoll([{fd=%d, events=POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND}"
	       ", {fd=%d, events=POLLOUT%s|POLLWRBAND}]"
	       ", %u, {tv_sec=23, tv_nsec=123456789}, [%s], %u) = %ld"
	       " ([{fd=%d, revents=POLLOUT%s}]"
	       ", left {tv_sec=%u, tv_nsec=%u})\n",
	       pipe_fd[0], pipe_fd[1], POLLWRNORM_str,
	       (unsigned int) ARRAY_SIZE(fds1) - 2, USR2_CHLD_str,
	       (unsigned int) sigset_size, rc, pipe_fd[1], POLLWRNORM_str,
	       (unsigned int) ts->tv_sec, (unsigned int) ts->tv_nsec);
# endif /* !PATH_TRACING_FDS */

# if PATH_TRACING_FD
	ts->tv_sec = 123;
	ts->tv_nsec = 987654321;
	fds[3].fd = PATH_TRACING_FD;

	rc = sys_ppoll((unsigned long) fds,
		       F8ILL_KULONG_MASK | ARRAY_SIZE(fds1), (unsigned long) ts,
		       (unsigned long) sigmask, sigset_size);
	if (rc != 2)
		perror_msg_and_fail("ppoll -P");
	printf("ppoll([{fd=%d, events=POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND}"
	       ", {fd=%d, events=POLLOUT%s|POLLWRBAND}"
#  if VERBOSE
	       ", {fd=%d, events=POLLIN|POLLPRI}, {fd=%d, events=POLLOUT}]"
#  else
	       ", ...]"
#  endif
	       ", %u, {tv_sec=123, tv_nsec=987654321}, [%s], %u) = %ld"
	       " ([{fd=%d, revents=POLLOUT%s}, {fd=%d, revents=POLLOUT}]"
	       ", left {tv_sec=%u, tv_nsec=%u})\n",
	       pipe_fd[0], pipe_fd[1], POLLWRNORM_str,
#  if VERBOSE
	       pipe_fd[2], PATH_TRACING_FD,
#  endif
	       (unsigned int) ARRAY_SIZE(fds1), USR2_CHLD_str,
	       (unsigned int) sigset_size, rc, pipe_fd[1], POLLWRNORM_str,
	       PATH_TRACING_FD, (unsigned int) ts->tv_sec,
	       (unsigned int) ts->tv_nsec);
# endif /* PATH_TRACING_FD */

	ts->tv_sec = 0;
	ts->tv_nsec = 999;
	const struct pollfd fds2[] = {
		{ .fd = pipe_fd[1], .events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND },
		{ .fd = pipe_fd[0], .events = POLLOUT | POLLWRNORM | POLLWRBAND }
	};
	fds = efault - sizeof(fds2);
	memcpy(fds, fds2, sizeof(fds2));

	memset(&mask, -1, sizeof(mask));
	sigdelset(&mask, SIGHUP);
	sigdelset(&mask, SIGKILL);
	sigdelset(&mask, SIGSTOP);
	memcpy(sigmask, &mask, sigset_size);

	rc = sys_ppoll((unsigned long) fds,
		       F8ILL_KULONG_MASK | ARRAY_SIZE(fds2), (unsigned long) ts,
		       (unsigned long) sigmask, sigset_size);
	if (rc != 0)
		perror_msg_and_fail("ppoll 2");
# if TRACE_FD2 || TRACE_OTHER_FDS
	printf("ppoll([{fd=%d, events=POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND}"
	       ", {fd=%d, events=POLLOUT%s|POLLWRBAND}], %u"
	       ", {tv_sec=0, tv_nsec=999}, ~[HUP KILL STOP], %u)"
	       " = %ld (Timeout)\n",
	       pipe_fd[1], pipe_fd[0], POLLWRNORM_str,
	       (unsigned) ARRAY_SIZE(fds2), sigset_size, rc);
# endif /* !PATH_TRACING_FD */

	ts->tv_sec = 0;
	ts->tv_nsec = 1;

	rc = sys_ppoll((unsigned long) fds,
		       F8ILL_KULONG_MASK | 1, (unsigned long) ts,
		       (unsigned long) sigmask, sigset_size);
	if (rc != 0)
		perror_msg_and_fail("ppoll 2");
# if !PATH_TRACING_FD && TRACE_FD2
	printf("ppoll([{fd=%d, events=POLLIN|POLLPRI|POLLRDNORM|POLLRDBAND}], 1"
	       ", {tv_sec=0, tv_nsec=1}, ~[HUP KILL STOP], %u)"
	       " = %ld (Timeout)\n",
	       pipe_fd[1], sigset_size, rc);
# endif /* !PATH_TRACING_FD */

	if (F8ILL_KULONG_SUPPORTED) {
		sys_ppoll(f8ill_ptr_to_kulong(fds), ARRAY_SIZE(fds2),
			  f8ill_ptr_to_kulong(ts), f8ill_ptr_to_kulong(sigmask),
			  sigset_size);
# if !PATH_TRACING_FD && !TRACING_FDS
		printf("ppoll(%#llx, %u, %#llx, %#llx, %u) = %s\n",
		       (unsigned long long) f8ill_ptr_to_kulong(fds),
		       (unsigned) ARRAY_SIZE(fds2),
		       (unsigned long long) f8ill_ptr_to_kulong(ts),
		       (unsigned long long) f8ill_ptr_to_kulong(sigmask),
		       (unsigned) sigset_size, errstr);
# endif /* !PATH_TRACING_FD */
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ppoll")

#endif
