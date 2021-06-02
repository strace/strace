/*
 * Check decoding of rt_sigpending syscall.
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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static long
k_sigpending(void *const set, const unsigned long size)
{
	return syscall(__NR_rt_sigpending, set, size);
}

static void
iterate(const char *const text, unsigned int size, void *set)
{
	for (;;) {
		if (k_sigpending(set, size)) {
			tprintf("rt_sigpending(%p, %u) = -1 EFAULT (%m)\n",
				set, size);
			break;
		}
		if (size) {
#ifdef WORDS_BIGENDIAN
			if (size < sizeof(long))
				tprintf("rt_sigpending(%s, %u) = 0\n",
					"[]", size);
			else
#endif
				tprintf("rt_sigpending(%s, %u) = 0\n",
					text, size);
		} else {
			tprintf("rt_sigpending(%p, %u) = 0\n", set, size);
			break;
		}
		size >>= 1;
		set += size;
	}
}

int
main(void)
{
	tprintf("%s", "");

	const unsigned int big_size = 1024 / 8;
	void *k_set = tail_alloc(big_size);
	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);

	sigemptyset(libc_set);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");

	memset(k_set, 0, big_size);
	unsigned int set_size = big_size;
	for (; set_size; set_size >>= 1, k_set += set_size) {
		if (!k_sigpending(k_set, set_size))
			break;
		tprintf("rt_sigpending(%p, %u) = -1 EINVAL (%m)\n",
			k_set, set_size);
	}
	if (!set_size)
		perror_msg_and_fail("rt_sigpending");
	tprintf("rt_sigpending(%s, %u) = 0\n", "[]", set_size);

	iterate("[]", set_size >> 1, k_set + (set_size >> 1));

	void *const efault = k_set + (set_size >> 1);
	assert(k_sigpending(efault, set_size) == -1);
	tprintf("rt_sigpending(%p, %u) = -1 EFAULT (%m)\n",
		efault, set_size);

	sigaddset(libc_set, SIGHUP);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");
	raise(SIGHUP);

	iterate("[HUP]", set_size, k_set);

	sigaddset(libc_set, SIGINT);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");
	raise(SIGINT);

	iterate("[HUP INT]", set_size, k_set);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
