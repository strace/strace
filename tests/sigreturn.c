/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#if defined __powerpc64__ \
 || (defined __sparc__ && defined __arch64__)
/* Old sigreturn is defined but not implemented in the kernel. */
# undef __NR_sigreturn
#endif

#ifdef __NR_sigreturn

# include <signal.h>
# include <stdio.h>
# include <stdlib.h>

# ifdef ASM_SIGRTMIN
#  define RT_0 ASM_SIGRTMIN
# else
/* Linux kernel >= 3.18 defines SIGRTMIN to 32 on all architectures. */
#  define RT_0 32
# endif

static void
handler(int sig)
{
}

int
main(void)
{
	static sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGUSR2);
	sigaddset(&set, SIGCHLD);
	sigaddset(&set, RT_0 +  3);
	sigaddset(&set, RT_0 +  4);
	sigaddset(&set, RT_0 +  5);
	sigaddset(&set, RT_0 + 26);
	sigaddset(&set, RT_0 + 27);
	if (sigprocmask(SIG_SETMASK, &set, NULL))
		perror_msg_and_fail("sigprocmask");
	sigemptyset(&set);

	/* This should result to old sigreturn. */
	if (signal(SIGUSR1, handler) == SIG_ERR)
		perror_msg_and_fail("sigaction");

	if (raise(SIGUSR1))
		perror_msg_and_fail("raise");

	static const char *const sigs =
		(SIGUSR2 < SIGCHLD) ? "INT USR2 CHLD" : "INT CHLD USR2";
	static const char *const rt_sigs = "RT_3 RT_4 RT_5 RT_26 RT_27";
	printf("sigreturn({mask=[%s %s]}) = 0\n", sigs, rt_sigs);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigreturn")

#endif
