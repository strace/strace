/*
 * This file is part of rt_sigpending strace test.
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

#ifdef __NR_rt_sigpending

# include <assert.h>
# include <signal.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

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
#if WORDS_BIGENDIAN
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

#else

SKIP_MAIN_UNDEFINED("__NR_rt_sigpending")

#endif
