/*
 * Check decoding of sigprocmask syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_sigprocmask

# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

static const char *errstr;

static long
k_sigprocmask(const kernel_ulong_t how, const kernel_ulong_t new_set,
	      const kernel_ulong_t old_set)
{
	const long rc = syscall(__NR_sigprocmask, how, new_set, old_set);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const kernel_ulong_t sig_block =
		(kernel_ulong_t) 0xfacefeed00000000ULL | SIG_BLOCK;
	static const kernel_ulong_t sig_unblock =
		(kernel_ulong_t) 0xfacefeed00000000ULL | SIG_UNBLOCK;
	static const kernel_ulong_t sig_setmask =
		(kernel_ulong_t) 0xfacefeed00000000ULL | SIG_SETMASK;

	if (k_sigprocmask(sig_setmask, 0, 0))
		perror_msg_and_skip("sigprocmask");
	puts("sigprocmask(SIG_SETMASK, NULL, NULL) = 0");

	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, new_set);
	TAIL_ALLOC_OBJECT_CONST_PTR(kernel_ulong_t, old_set);
	TAIL_ALLOC_OBJECT_CONST_PTR(sigset_t, libc_set);

	memset(new_set, 0, sizeof(*new_set));
	k_sigprocmask(sig_setmask, (uintptr_t) new_set, 0);
	printf("sigprocmask(SIG_SETMASK, [], NULL) = %s\n", errstr);

	k_sigprocmask(sig_unblock,
		      (uintptr_t) (new_set - 1), (uintptr_t) old_set);
	puts("sigprocmask(SIG_UNBLOCK, ~[], []) = 0");

	if (F8ILL_KULONG_SUPPORTED) {
		k_sigprocmask(sig_unblock, f8ill_ptr_to_kulong(new_set), 0);
		printf("sigprocmask(SIG_UNBLOCK, %#jx, NULL) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(new_set), errstr);

		k_sigprocmask(sig_unblock, 0, f8ill_ptr_to_kulong(old_set));
		printf("sigprocmask(SIG_UNBLOCK, NULL, %#jx) = %s\n",
		       (uintmax_t) f8ill_ptr_to_kulong(old_set), errstr);
	}

	sigemptyset(libc_set);
	sigaddset(libc_set, SIGHUP);
	memcpy(new_set, libc_set, sizeof(*new_set));

	k_sigprocmask(sig_block, (uintptr_t) new_set, (uintptr_t) old_set);
	puts("sigprocmask(SIG_BLOCK, [HUP], []) = 0");

	memset(libc_set, -1, sizeof(*libc_set));
	sigdelset(libc_set, SIGHUP);
	memcpy(new_set, libc_set, sizeof(*new_set));

	k_sigprocmask(sig_unblock, (uintptr_t) new_set, (uintptr_t) old_set);
	puts("sigprocmask(SIG_UNBLOCK, ~[HUP], [HUP]) = 0");

	sigdelset(libc_set, SIGKILL);
	memcpy(new_set, libc_set, sizeof(*new_set));

	k_sigprocmask(sig_unblock, (uintptr_t) new_set, (uintptr_t) old_set);
	puts("sigprocmask(SIG_UNBLOCK, ~[HUP KILL], [HUP]) = 0");

	sigemptyset(libc_set);
	sigaddset(libc_set, SIGHUP);
	sigaddset(libc_set, SIGINT);
	sigaddset(libc_set, SIGQUIT);
	sigaddset(libc_set, SIGALRM);
	sigaddset(libc_set, SIGTERM);
	memcpy(new_set, libc_set, sizeof(*new_set));

	k_sigprocmask(sig_block, (uintptr_t) new_set, (uintptr_t) old_set);
	printf("sigprocmask(SIG_BLOCK, %s, [HUP]) = 0\n",
	       "[HUP INT QUIT ALRM TERM]");

	k_sigprocmask(sig_setmask, 0, (uintptr_t) old_set);
	printf("sigprocmask(SIG_SETMASK, NULL, %s) = 0\n",
	       "[HUP INT QUIT ALRM TERM]");

	k_sigprocmask(sig_setmask, (uintptr_t) (new_set + 1), 0);
	printf("sigprocmask(SIG_SETMASK, %p, NULL) = %s\n",
	       new_set + 1, errstr);

	k_sigprocmask(sig_setmask,
		      (uintptr_t) new_set, (uintptr_t) (old_set + 1));
	printf("sigprocmask(SIG_SETMASK, %s, %p) = %s\n",
	       "[HUP INT QUIT ALRM TERM]", old_set + 1, errstr);

	uintptr_t efault = sizeof(*new_set) / 2 + (uintptr_t) new_set;

	k_sigprocmask(sig_setmask, efault, 0);
	printf("sigprocmask(SIG_SETMASK, %#jx, NULL) = %s\n",
	       (uintmax_t) efault, errstr);

	k_sigprocmask(sig_setmask, 0, efault);
	printf("sigprocmask(SIG_SETMASK, NULL, %#jx) = %s\n",
	       (uintmax_t) efault, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigprocmask")

#endif
