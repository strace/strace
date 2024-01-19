/*
 * This file is part of rt_sigtimedwait strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_rt_sigtimedwait

# include <assert.h>
# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>

static long
k_sigtimedwait(const sigset_t *const set, siginfo_t *const info,
	       const kernel_old_timespec_t *const timeout, const unsigned long size)
{
	return syscall(__NR_rt_sigtimedwait, set, info, timeout, size);
}

static void
iterate(const char *const text, const void *set,
	const kernel_old_timespec_t *const timeout, unsigned int size)
{
	for (;;) {
		assert(k_sigtimedwait(set, NULL, timeout, size) == -1);
		if (EINTR == errno) {
			tprintf("rt_sigtimedwait(%s, NULL"
				", {tv_sec=%lld, tv_nsec=%llu}, %u)"
				RVAL_EAGAIN, text,
				(long long) timeout->tv_sec,
				zero_extend_signed_to_ull(timeout->tv_nsec),
				size);
		} else {
			if (size < sizeof(long))
				tprintf("rt_sigtimedwait(%p, NULL"
					", {tv_sec=%lld, tv_nsec=%llu}"
					", %u)" RVAL_EINVAL,
					set, (long long) timeout->tv_sec,
					zero_extend_signed_to_ull(timeout->tv_nsec),
					size);
			else
				tprintf("rt_sigtimedwait(%s, NULL"
					", {tv_sec=%lld, tv_nsec=%llu}"
					", %u)" RVAL_EINVAL,
					text, (long long) timeout->tv_sec,
					zero_extend_signed_to_ull(timeout->tv_nsec),
					size);
		}
		if (!size)
			break;
		size >>= 1;
		set += size;
	}
}

int
main(void)
{
	tprintf("%s", "");

	TAIL_ALLOC_OBJECT_CONST_PTR(siginfo_t, info);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_old_timespec_t, timeout);
	timeout->tv_sec = 0;
	timeout->tv_nsec = 42;

	const unsigned int big_size = 1024 / 8;
	void *k_set = tail_alloc(big_size);
	memset(k_set, 0, big_size);

	unsigned int set_size = big_size;
	for (; set_size; set_size >>= 1, k_set += set_size) {
		assert(k_sigtimedwait(k_set, NULL, timeout, set_size) == -1);
		if (EAGAIN == errno)
			break;
		tprintf("rt_sigtimedwait(%p, NULL, {tv_sec=%lld, tv_nsec=%llu}"
			", %u)" RVAL_EINVAL,
			k_set, (long long) timeout->tv_sec,
			zero_extend_signed_to_ull(timeout->tv_nsec), set_size);
	}
	if (!set_size)
		perror_msg_and_fail("rt_sigtimedwait");
	tprintf("rt_sigtimedwait([], NULL, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		RVAL_EAGAIN,
		(long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	timeout->tv_sec = 0xdeadbeefU;
	timeout->tv_nsec = 0xfacefeedU;
	assert(k_sigtimedwait(k_set, NULL, timeout, set_size) == -1);
	tprintf("rt_sigtimedwait([], NULL, {tv_sec=%lld, tv_nsec=%llu}"
		", %u)" RVAL_EINVAL,
		(long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	timeout->tv_sec = (typeof(timeout->tv_sec)) 0xcafef00ddeadbeefLL;
	timeout->tv_nsec = (long) 0xbadc0dedfacefeedLL;
	assert(k_sigtimedwait(k_set, NULL, timeout, set_size) == -1);
	tprintf("rt_sigtimedwait([], NULL, {tv_sec=%lld, tv_nsec=%llu}"
		", %u)" RVAL_EINVAL,
		(long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	timeout->tv_sec = 0;
	timeout->tv_nsec = 42;

	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);
	sigemptyset(libc_set);
	sigaddset(libc_set, SIGHUP);
	memcpy(k_set, libc_set, set_size);

	assert(k_sigtimedwait(k_set, info, timeout, set_size) == -1);
	assert(EAGAIN == errno);
	tprintf("rt_sigtimedwait([HUP], %p, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		RVAL_EAGAIN,
		info, (long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	sigaddset(libc_set, SIGINT);
	memcpy(k_set, libc_set, set_size);

	assert(k_sigtimedwait(k_set, info, timeout, set_size) == -1);
	assert(EAGAIN == errno);
	tprintf("rt_sigtimedwait([HUP INT], %p, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		RVAL_EAGAIN,
		info, (long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	sigaddset(libc_set, SIGQUIT);
	sigaddset(libc_set, SIGALRM);
	sigaddset(libc_set, SIGTERM);
	memcpy(k_set, libc_set, set_size);

	assert(k_sigtimedwait(k_set, info, timeout, set_size) == -1);
	assert(EAGAIN == errno);
	tprintf("rt_sigtimedwait(%s, %p, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		RVAL_EAGAIN,
		"[HUP INT QUIT ALRM TERM]",
		info, (long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	memset(k_set - set_size, -1, set_size);
	assert(k_sigtimedwait(k_set - set_size, info, timeout, set_size) == -1);
	assert(EAGAIN == errno);
	tprintf("rt_sigtimedwait(~[], %p, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		RVAL_EAGAIN,
		info, (long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec), set_size);

	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");

	assert(k_sigtimedwait(k_set - set_size, info, NULL, set_size << 1) == -1);
	tprintf("rt_sigtimedwait(%p, %p, NULL, %u)" RVAL_EINVAL,
		k_set - set_size, info, set_size << 1);

	iterate("~[]", k_set - set_size, timeout, set_size >> 1);

	timeout->tv_sec = 1;
	raise(SIGALRM);
	assert(k_sigtimedwait(k_set, info, timeout, set_size) == SIGALRM);
	tprintf("rt_sigtimedwait(%s, {si_signo=%s, si_code=SI_TKILL"
		", si_pid=%d, si_uid=%d}, {tv_sec=%lld, tv_nsec=%llu}, %u)"
		" = %d (%s)\n",
		"[HUP INT QUIT ALRM TERM]", "SIGALRM", getpid(), getuid(),
		(long long) timeout->tv_sec,
		zero_extend_signed_to_ull(timeout->tv_nsec),
		set_size, SIGALRM, "SIGALRM");

	raise(SIGALRM);
	assert(k_sigtimedwait(k_set, NULL, NULL, set_size) == SIGALRM);
	tprintf("rt_sigtimedwait(%s, NULL, NULL, %u) = %d (%s)\n",
		"[HUP INT QUIT ALRM TERM]", set_size, SIGALRM, "SIGALRM");

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rt_sigtimedwait")

#endif
