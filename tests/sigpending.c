/*
 * Check decoding of sigpending syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

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
	printf("sigpending(%p)" RVAL_EFAULT, k_set + 1);

	uintptr_t efault = sizeof(*k_set) / 2 + (uintptr_t) k_set;
	k_sigpending(efault);
	printf("sigpending(%#jx)" RVAL_EFAULT, (uintmax_t) efault);

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
