/*
 * Check decoding of sigpending syscall.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_sigpending

# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

static const char *errstr;

static long
k_sigpending(const kernel_ulong_t set)
{
	const long rc = syscall(__NR_sigpending, set);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, k_set);
	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);

	sigemptyset(libc_set);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");

	if (k_sigpending((uintptr_t) libc_set))
		perror_msg_and_skip("sigpending");
	else
		puts("sigpending([]) = 0");

	k_sigpending((uintptr_t) k_set);
	puts("sigpending([]) = 0");

	k_sigpending((uintptr_t) (k_set + 1));
	printf("sigpending(%p) = -1 EFAULT (%m)\n", k_set + 1);

	uintptr_t efault = sizeof(*k_set) / 2 + (uintptr_t) k_set;
	k_sigpending(efault);
	printf("sigpending(%#jx) = -1 EFAULT (%m)\n", (uintmax_t) efault);

	sigaddset(libc_set, SIGHUP);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");
	raise(SIGHUP);

	k_sigpending((uintptr_t) k_set);
	puts("sigpending([HUP]) = 0");

	sigaddset(libc_set, SIGINT);
	if (sigprocmask(SIG_SETMASK, libc_set, NULL))
		perror_msg_and_fail("sigprocmask");
	raise(SIGINT);

	k_sigpending((uintptr_t) k_set);
	puts("sigpending([HUP INT]) = 0");

	if (F8ILL_KULONG_SUPPORTED) {
		k_sigpending(f8ill_ptr_to_kulong(k_set));
		printf("sigpending(%#jx) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(k_set), errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigpending")

#endif
