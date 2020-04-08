/*
 * Check decoding of signal syscall.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_signal

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>

static long
k_signal(const kernel_ulong_t signum, const kernel_ulong_t handler)
{
	return syscall(__NR_signal, signum, handler);
}

int
main(void)
{
	static const uintptr_t sig_ign = (uintptr_t) SIG_IGN;
	static const uintptr_t sig_dfl = (uintptr_t) SIG_DFL;
	static const kernel_ulong_t sigusr1 =
		(kernel_ulong_t) 0xfacefeed00000000ULL | SIGUSR1;
	static const struct sigaction act = { .sa_handler = SIG_DFL };
	long rc;

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask SIG_BLOCK");

	if (sigaction(SIGUSR1, &act, NULL))
		perror_msg_and_fail("sigaction");

	rc = k_signal(sigusr1, sig_ign);

	if (rc == -1L) {
		printf("signal(SIGUSR1, SIG_IGN) = %s\n", sprintrc(rc));
	} else if (rc != (long) sig_dfl) {
		error_msg_and_fail("signal(SIGUSR1, SIG_IGN) = %#lx\n", rc);
	} else {
		printf("signal(SIGUSR1, SIG_IGN) = %#lx (SIG_DFL)\n", rc);

		/*
		 * Raise and unblock SIGUSR1.
		 * If signal syscall failed to set SIGUSR1 handler to SIG_IGN,
		 * the process will be terminated by SIGUSR1.
		 */
		raise(SIGUSR1);
		if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
			perror_msg_and_fail("sigprocmask SIG_UNBLOCK");

		if (sigprocmask(SIG_BLOCK, &mask, NULL))
			perror_msg_and_fail("sigprocmask SIG_BLOCK");
	}

	rc = k_signal(SIGUSR1, sig_dfl);

	if (rc == -1L) {
		printf("signal(SIGUSR1, SIG_DFL) = %s\n", sprintrc(rc));
	} else if (rc != (long) sig_ign) {
		error_msg_and_fail("signal(SIGUSR1, SIG_DFL) = %#lx\n", rc);
	} else {
		printf("signal(SIGUSR1, SIG_DFL) = %#lx (SIG_IGN)\n", rc);
	}

	const kernel_ulong_t addr = (kernel_ulong_t) 0xfacefeeddeadbeefULL;
	rc = k_signal(SIGUSR1, addr);

	if (rc == -1L) {
		printf("signal(SIGUSR1, %#llx) = %s\n",
		       (unsigned long long) addr, sprintrc(rc));
	} else if (rc != (long) sig_dfl) {
		error_msg_and_fail("signal(SIGUSR1, %#llx) = %#lx\n",
				   (unsigned long long) addr, rc);
	} else {
		printf("signal(SIGUSR1, %#llx) = %#lx (SIG_DFL)\n",
		       (unsigned long long) addr, rc);
	}

	rc = k_signal(SIGUSR1, sig_ign);

	if (rc == -1L) {
		printf("signal(SIGUSR1, SIG_IGN) = %s\n", sprintrc(rc));
	} else {
		printf("signal(SIGUSR1, SIG_IGN) = %#lx\n", rc);
	}

	rc = k_signal(addr, sig_ign);
	printf("signal(%d, SIG_IGN) = %s\n", (int) addr, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_signal")

#endif
