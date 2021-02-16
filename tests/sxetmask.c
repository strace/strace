/*
 * Check decoding of sgetmask and ssetmask syscalls.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_sgetmask && defined __NR_ssetmask

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>

static long
k_sgetmask(void)
{
	return syscall(__NR_sgetmask);
}

static long
k_ssetmask(const kernel_ulong_t arg)
{
	return syscall(__NR_ssetmask, arg);
}

int
main(void)
{
	union {
		sigset_t libc_mask;
		unsigned long old_mask;
	} uset, uget;
	long rc;

	/*
	 * Block, reset, and raise SIGUSR1.
	 * If a subsequent ssetmask call fails to set the proper mask,
	 * the process will be terminated by SIGUSR1.
	 */
	sigemptyset(&uset.libc_mask);
	sigaddset(&uset.libc_mask, SIGUSR1);
	if (sigprocmask(SIG_SETMASK, &uset.libc_mask, NULL))
		perror_msg_and_fail("sigprocmask");
	if (signal(SIGUSR1, SIG_DFL) == SIG_ERR)
		perror_msg_and_fail("signal");
	raise(SIGUSR1);

	sigaddset(&uset.libc_mask, SIGUSR2);
	rc = k_ssetmask((kernel_ulong_t) 0xfacefeed00000000ULL | uset.old_mask);
	if (rc == -1L) {
		printf("ssetmask([USR1 USR2]) = %s\n", sprintrc(rc));
	} else {
		printf("ssetmask([USR1 USR2]) = %#lx (old mask [USR1])\n", rc);
		/*
		 * Use a regular sigprocmask call to check the value
		 * returned by the ssetmask call being tested.
		 */
		if (sigprocmask(SIG_SETMASK, NULL, &uget.libc_mask))
			perror_msg_and_fail("sigprocmask");
		if (uset.old_mask != uget.old_mask)
			error_msg_and_fail("sigprocmask returned %#lx"
					   " instead of %#lx",
					   uget.old_mask, uset.old_mask);
	}

	rc = k_sgetmask();
	if (rc == -1L) {
		printf("sgetmask() = %s\n", sprintrc(rc));
	} else {
		printf("sgetmask() = %#lx (mask [USR1 USR2])\n", rc);
		if (uget.old_mask != (unsigned long) rc)
			error_msg_and_fail("sigprocmask returned %#lx",
					   uget.old_mask);

		if (sizeof(long) > 4) {
			sigaddset(&uset.libc_mask, 32 + 27);
			if (sigprocmask(SIG_SETMASK, &uset.libc_mask, NULL))
				perror_msg_and_fail("sigprocmask");
			rc = k_sgetmask();
			printf("sgetmask() = %#lx"
			       " (mask [USR1 USR2 RT_27])\n", rc);
			if (uset.old_mask != (unsigned long) rc)
				error_msg_and_fail("sigprocmask set %#lx",
						   uset.old_mask);
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sgetmask && __NR_ssetmask")

#endif
