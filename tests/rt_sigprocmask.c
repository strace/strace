/*
 * Check decoding of rt_sigprocmask syscall.
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
k_sigprocmask(const unsigned long how, void *const new_set,
	      void *const old_set, const unsigned long size)
{
	return syscall(__NR_rt_sigprocmask, how, new_set, old_set, size);
}

static void
iterate(const char *const text, void *set, void *old, unsigned int size)
{
	for (;;) {
		if (k_sigprocmask(SIG_UNBLOCK, set, old, size)) {
			if (size < sizeof(long))
				tprintf("rt_sigprocmask(SIG_UNBLOCK"
					", %p, %p, %u) = -1 EINVAL (%m)\n",
					set, old, size);
			else
				tprintf("rt_sigprocmask(SIG_UNBLOCK"
					", %s, %p, %u) = -1 EINVAL (%m)\n",
					text, old, size);
		} else {
			tprintf("rt_sigprocmask(SIG_UNBLOCK, %s, [], %u)"
				" = 0\n", text, size);
		}
		if (!size)
			break;
		size >>= 1;
		set += size;
		old += size;
	}
}

int
main(void)
{
	tprintf("%s", "");

	const unsigned int big_size = 1024 / 8;
	unsigned int set_size;

	for (set_size = big_size; set_size; set_size >>= 1) {
		if (!k_sigprocmask(SIG_SETMASK, NULL, NULL, set_size))
			break;
		tprintf("rt_sigprocmask(SIG_SETMASK, NULL, NULL, %u)"
			" = -1 EINVAL (%m)\n", set_size);
	}
	if (!set_size)
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_SETMASK, NULL, NULL, %u) = 0\n",
		set_size);

	void *const k_set = tail_alloc(set_size);
	void *const old_set = tail_alloc(set_size);
	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);

	memset(k_set, 0, set_size);
	if (k_sigprocmask(SIG_SETMASK, k_set, NULL, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_SETMASK, [], NULL, %u) = 0\n", set_size);

	if (k_sigprocmask(SIG_UNBLOCK, k_set - set_size, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_UNBLOCK, ~[], [], %u) = 0\n", set_size);

	assert(k_sigprocmask(SIG_SETMASK, k_set - set_size,
			     old_set, set_size << 1) == -1);
	tprintf("rt_sigprocmask(SIG_SETMASK, %p, %p, %u) = -1 EINVAL (%m)\n",
		k_set - set_size, old_set, set_size << 1);

	iterate("~[]", k_set - set_size, old_set, set_size >> 1);

	sigemptyset(libc_set);
	sigaddset(libc_set, SIGHUP);
	memcpy(k_set, libc_set, set_size);

	if (k_sigprocmask(SIG_BLOCK, k_set, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_BLOCK, [HUP], [], %u) = 0\n", set_size);

	memset(libc_set, -1, sizeof(sigset_t));
	sigdelset(libc_set, SIGHUP);
	memcpy(k_set, libc_set, set_size);

	if (k_sigprocmask(SIG_UNBLOCK, k_set, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_UNBLOCK, ~[HUP], [HUP], %u) = 0\n",
		set_size);

	sigdelset(libc_set, SIGKILL);
	memcpy(k_set, libc_set, set_size);

	if (k_sigprocmask(SIG_UNBLOCK, k_set, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_UNBLOCK, ~[HUP KILL], [HUP], %u) = 0\n",
		set_size);

	sigemptyset(libc_set);
	sigaddset(libc_set, SIGHUP);
	sigaddset(libc_set, SIGINT);
	sigaddset(libc_set, SIGQUIT);
	sigaddset(libc_set, SIGALRM);
	sigaddset(libc_set, SIGTERM);
	memcpy(k_set, libc_set, set_size);

	if (k_sigprocmask(SIG_BLOCK, k_set, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_BLOCK, %s, [HUP], %u) = 0\n",
		"[HUP INT QUIT ALRM TERM]", set_size);

	if (k_sigprocmask(SIG_SETMASK, NULL, old_set, set_size))
		perror_msg_and_fail("rt_sigprocmask");
	tprintf("rt_sigprocmask(SIG_SETMASK, NULL, %s, %u) = 0\n",
		"[HUP INT QUIT ALRM TERM]", set_size);

	assert(k_sigprocmask(SIG_SETMASK, k_set + (set_size >> 1), NULL,
			     set_size) == -1);
	tprintf("rt_sigprocmask(SIG_SETMASK, %p, NULL, %u) = -1 EFAULT (%m)\n",
		k_set + (set_size >> 1), set_size);

	assert(k_sigprocmask(SIG_SETMASK, k_set, old_set + (set_size >> 1),
			     set_size) == -1);
	tprintf("rt_sigprocmask(SIG_SETMASK, %s, %p, %u) = -1 EFAULT (%m)\n",
		"[HUP INT QUIT ALRM TERM]",
		old_set + (set_size >> 1), set_size);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
