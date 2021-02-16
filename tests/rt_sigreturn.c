/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef ASM_SIGRTMIN
# define RT_0 ASM_SIGRTMIN
#else
/* Linux kernel >= 3.18 defines SIGRTMIN to 32 on all architectures. */
# define RT_0 32
#endif

static void
handler(int no, siginfo_t *si, void *uc)
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

	static const struct sigaction sa = {
		.sa_sigaction = handler,
		.sa_flags = SA_SIGINFO
	};
	if (sigaction(SIGUSR1, &sa, NULL))
		perror_msg_and_fail("sigaction");

	if (raise(SIGUSR1))
		perror_msg_and_fail("raise");

	static const char *const sigs =
		(SIGUSR2 < SIGCHLD) ? "INT USR2 CHLD" : "INT CHLD USR2";
	static const char *const rt_sigs = "RT_3 RT_4 RT_5 RT_26 RT_27";
	printf("rt_sigreturn({mask=[%s %s]}) = 0\n", sigs, rt_sigs);

	puts("+++ exited with 0 +++");
	return 0;
}
