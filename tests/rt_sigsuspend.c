/*
 * Check decoding of rt_sigsuspend syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static long
k_sigsuspend(const sigset_t *const set, const unsigned long size)
{
	return syscall(__NR_rt_sigsuspend, set, size);
}

static void
iterate(const char *const text, const int sig,
	const void *const set, unsigned int size)
{
	for (const void *mask = set;; size >>= 1, mask += size) {
		raise(sig);
		assert(k_sigsuspend(mask, size) == -1);
		if (EINTR == errno) {
			tprintf("rt_sigsuspend(%s, %u) = ? ERESTARTNOHAND"
				" (To be restarted if no handler)\n",
				text, size);
		} else {
			if (size < sizeof(long))
				tprintf("rt_sigsuspend(%p, %u)"
					" = -1 EINVAL (%m)\n",
					mask, size);
			else
				tprintf("rt_sigsuspend(%s, %u)"
					" = -1 EINVAL (%m)\n",
					set == mask ? text : "~[]", size);
		}
		if (!size)
			break;
	}
}

static void
handler(int signo)
{
}

int
main(void)
{
	tprintf("%s", "");

	const unsigned int big_size = 1024 / 8;
	void *k_set = tail_alloc(big_size);
	memset(k_set, 0, big_size);

	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);
	sigemptyset(libc_set);
	sigaddset(libc_set, SIGUSR1);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");

	const struct sigaction sa = {
		.sa_handler = handler
	};
	if (sigaction(SIGUSR1, &sa, NULL))
		perror_msg_and_fail("sigaction");

	raise(SIGUSR1);
	unsigned int set_size = big_size;
	for (; set_size; set_size >>= 1, k_set += set_size) {
		assert(k_sigsuspend(k_set, set_size) == -1);
		if (EINTR == errno)
			break;
		tprintf("rt_sigsuspend(%p, %u) = -1 EINVAL (%m)\n",
			k_set, set_size);
	}
	if (!set_size)
		perror_msg_and_fail("rt_sigsuspend");
	tprintf("rt_sigsuspend([], %u) = ? ERESTARTNOHAND"
		" (To be restarted if no handler)\n", set_size);

	sigemptyset(libc_set);
	sigaddset(libc_set, SIGUSR2);
	memcpy(k_set, libc_set, set_size);
	raise(SIGUSR1);
	assert(k_sigsuspend(k_set, set_size) == -1);
	assert(EINTR == errno);
	tprintf("rt_sigsuspend([USR2], %u) = ? ERESTARTNOHAND"
		" (To be restarted if no handler)\n", set_size);

	sigaddset(libc_set, SIGHUP);
	memcpy(k_set, libc_set, set_size);
	raise(SIGUSR1);
	assert(k_sigsuspend(k_set, set_size) == -1);
	assert(EINTR == errno);
	tprintf("rt_sigsuspend([HUP USR2], %u) = ? ERESTARTNOHAND"
		" (To be restarted if no handler)\n", set_size);

	sigaddset(libc_set, SIGINT);
	memcpy(k_set, libc_set, set_size);
	raise(SIGUSR1);
	assert(k_sigsuspend(k_set, set_size) == -1);
	assert(EINTR == errno);
	tprintf("rt_sigsuspend([HUP INT USR2], %u) = ? ERESTARTNOHAND"
		" (To be restarted if no handler)\n", set_size);

	memset(libc_set, -1, sizeof(*libc_set));
	sigdelset(libc_set, SIGUSR1);
	memcpy(k_set, libc_set, set_size);
	raise(SIGUSR1);
	assert(k_sigsuspend(k_set, set_size) == -1);
	assert(EINTR == errno);
	tprintf("rt_sigsuspend(~[USR1], %u) = ? ERESTARTNOHAND"
		" (To be restarted if no handler)\n", set_size);

	assert(k_sigsuspend(k_set - set_size, set_size << 1) == -1);
	tprintf("rt_sigsuspend(%p, %u) = -1 EINVAL (%m)\n",
		k_set - set_size, set_size << 1);

	iterate("~[USR1]", SIGUSR1, k_set, set_size >> 1);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
