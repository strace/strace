/*
 * This file is part of pselect6* strace tests.
 *
 * Copyright (c) 2015-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Based on test by Dr. David Alan Gilbert <dave@treblig.org>
 */

#include "nsig.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "kernel_timespec.h"

static const char *errstr;

static long
pselect6(const unsigned int nfds,
	 void *const rfds,
	 void *const wfds,
	 void *const efds,
	 void *const timeout,
	 void *const sigmask)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = nfds | fill;
	const kernel_ulong_t arg2 = f8ill_ptr_to_kulong(rfds);
	const kernel_ulong_t arg3 = f8ill_ptr_to_kulong(wfds);
	const kernel_ulong_t arg4 = f8ill_ptr_to_kulong(efds);
	const kernel_ulong_t arg5 = f8ill_ptr_to_kulong(timeout);
	const kernel_ulong_t arg6 = f8ill_ptr_to_kulong(sigmask);
	const long rc = syscall(SYSCALL_NR, arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

static fd_set set[3][0x1000000 / sizeof(fd_set)];

static void
handler(int signo)
{
}

struct sigset_argpack {
	kernel_ulong_t p, size;
};

int main(int ac, char **av)
{
	static const pselect6_timespec_t ts_in = {
		.tv_sec = 0xc0de1, .tv_nsec = 0xc0de2
	};
	pselect6_timespec_t *const ts = tail_memdup(&ts_in, sizeof(ts_in));

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGCHLD);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sigset_argpack, sigmask_arg);
	sigmask_arg->p = (uintptr_t) &mask;
	sigmask_arg->size = NSIG_BYTES;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sigset_argpack, nullmask_arg);
	nullmask_arg->p = 0;
	nullmask_arg->size = NSIG_BYTES;

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	/*
	 * Start with a nice simple pselect6.
	 */
	FD_SET(fds[0], set[0]);
	FD_SET(fds[1], set[0]);
	FD_SET(fds[0], set[1]);
	FD_SET(fds[1], set[1]);
	FD_SET(1, set[2]);
	FD_SET(2, set[2]);
	int rc = pselect6(fds[1] + 1, set[0], set[1], set[2], NULL, nullmask_arg);
	if (rc < 0)
		perror_msg_and_skip("pselect6");
	assert(rc == 1);
	printf("%s(%d, [%d %d], [%d %d], [1 2], NULL, {NULL, %u}) "
	       "= 1 (out [%d])\n",
	       SYSCALL_NAME, fds[1] + 1, fds[0], fds[1],
	       fds[0], fds[1],
	       NSIG_BYTES, fds[1]);

	/*
	 * Another simple one, with a timeout.
	 */
	FD_SET(1, set[1]);
	FD_SET(2, set[1]);
	FD_SET(fds[0], set[1]);
	FD_SET(fds[1], set[1]);
	assert(pselect6(fds[1] + 1, NULL, set[1], NULL, ts, NULL) == 3);
	printf("%s(%d, NULL, [1 2 %d %d], NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}, NULL) = 3 (out [1 2 %d]"
	       ", left {tv_sec=%lld, tv_nsec=%llu})\n",
	       SYSCALL_NAME,
	       fds[1] + 1, fds[0], fds[1], (long long) ts_in.tv_sec,
	       zero_extend_signed_to_ull(ts_in.tv_nsec),
	       fds[1], (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec));

	/*
	 * Now the crash case that trinity found, negative nfds
	 * but with a pointer to a large chunk of valid memory.
	 */
	FD_ZERO(set[0]);
	FD_SET(fds[1], set[0]);
	assert(pselect6(-1, NULL, set[0], NULL, NULL, sigmask_arg) == -1);
	printf("%s(-1, NULL, %p, NULL, NULL, {[HUP CHLD], %u}) = %s\n",
	       SYSCALL_NAME, set[0], NSIG_BYTES, errstr);

	/*
	 * Another variant, with nfds exceeding FD_SETSIZE limit.
	 */
	FD_ZERO(set[0]);
	FD_SET(fds[0], set[0]);
	FD_ZERO(set[1]);
	ts->tv_sec = 0;
	ts->tv_nsec = 123;
	assert(pselect6(FD_SETSIZE + 1, set[0], set[1], NULL, ts, sigmask_arg) == 0);
	printf("%s(%d, [%d], [], NULL, {tv_sec=0, tv_nsec=123}"
	       ", {[HUP CHLD], %u}) = 0 (Timeout)\n",
	       SYSCALL_NAME, FD_SETSIZE + 1, fds[0], NSIG_BYTES);

	/*
	 * See how timeouts are decoded.
	 */
	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	assert(pselect6(0, NULL, NULL, NULL, ts, nullmask_arg) == -1);
	printf("%s(0, NULL, NULL, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}, {NULL, %u}) = %s\n",
	       SYSCALL_NAME, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec),
	       NSIG_BYTES, errstr);

	ts->tv_sec = (typeof(ts->tv_sec)) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (typeof(ts->tv_nsec)) 0xbadc0dedfacefeedLL;
	assert(pselect6(0, NULL, NULL, NULL, ts, nullmask_arg) == -1);
	printf("%s(0, NULL, NULL, NULL"
	       ", {tv_sec=%lld, tv_nsec=%llu}, {NULL, %u}) = %s\n",
	       SYSCALL_NAME, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec),
	       NSIG_BYTES, errstr);

	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction SIGALRM");

	const struct itimerval itv = { .it_value.tv_usec = 111111 };
	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_fail("setitimer ITIMER_REAL");

	ts->tv_sec = 0;
	ts->tv_nsec = 222222222;
	assert(pselect6(0, NULL, NULL, NULL, ts, sigmask_arg) == -1);
	printf("%s(0, NULL, NULL, NULL, {tv_sec=0, tv_nsec=222222222}"
	       ", {[HUP CHLD], %u})"
	       " = ? ERESTARTNOHAND (To be restarted if no handler)\n",
	       SYSCALL_NAME, NSIG_BYTES);
	puts("--- SIGALRM {si_signo=SIGALRM, si_code=SI_KERNEL} ---");

	puts("+++ exited with 0 +++");
	return 0;
}
