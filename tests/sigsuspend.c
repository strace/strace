/*
 * Check decoding of sigsuspend syscall.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_sigsuspend

# include <assert.h>
# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>

# ifdef MIPS
#  define SIGNAL_MASK_BY_REF 1
# else
#  define SIGNAL_MASK_BY_REF 0
# endif

static long
k_sigsuspend(const kernel_ulong_t arg1,
	     const kernel_ulong_t arg2,
	     const kernel_ulong_t arg3)
{
	return syscall(__NR_sigsuspend, arg1, arg2, arg3);
}

static int signo;

static void
handler(int i)
{
	signo = i;
}

int
main(void)
{
	union {
		sigset_t libc_mask;
		unsigned long old_mask;
	} u;
	unsigned long mask1, mask2;

	sigemptyset(&u.libc_mask);
	sigaddset(&u.libc_mask, SIGUSR1);
	mask1 = u.old_mask;

	sigemptyset(&u.libc_mask);
	sigaddset(&u.libc_mask, SIGUSR2);
	mask2 = u.old_mask;

	sigaddset(&u.libc_mask, SIGUSR1);
	if (sigprocmask(SIG_SETMASK, &u.libc_mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const struct sigaction sa = { .sa_handler = handler };
	if (sigaction(SIGUSR1, &sa, NULL) || sigaction(SIGUSR2, &sa, NULL))
		perror_msg_and_fail("sigaction");

	raise(SIGUSR1);
	raise(SIGUSR2);

#if SIGNAL_MASK_BY_REF
	k_sigsuspend((uintptr_t) &mask1, 0xdeadbeef, (uintptr_t) &mask2);
#else
	k_sigsuspend(mask1, 0xdeadbeef, mask2);
#endif
	if (EINTR != errno)
		perror_msg_and_skip("sigsuspend");

	printf("sigsuspend([%s]) = ? ERESTARTNOHAND"
	       " (To be restarted if no handler)\n",
	       signo == SIGUSR2 ? "USR1" : "USR2");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigsuspend")

#endif
