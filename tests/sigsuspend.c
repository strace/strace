/*
 * Check decoding of sigsuspend syscall.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

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
static const char *sigtxt[] = {
	[SIGUSR1] = "USR1",
	[SIGUSR2] = "USR2"
};

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

	sigemptyset(&u.libc_mask);
	sigaddset(&u.libc_mask, SIGUSR1);
	sigaddset(&u.libc_mask, SIGUSR2);
	if (sigprocmask(SIG_SETMASK, &u.libc_mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const struct sigaction sa = { .sa_handler = handler };
	if (sigaction(SIGUSR1, &sa, NULL) || sigaction(SIGUSR2, &sa, NULL))
		perror_msg_and_fail("sigaction");

	raise(SIGUSR1);
	raise(SIGUSR2);

	u.old_mask = -1UL;
	sigdelset(&u.libc_mask, SIGUSR1);
	const unsigned long mask1 = u.old_mask;

	u.old_mask = -1UL;
	sigdelset(&u.libc_mask, SIGUSR2);
	const unsigned long mask2 = u.old_mask;

# if SIGNAL_MASK_BY_REF
	k_sigsuspend((uintptr_t) &mask1, 0xdeadbeef, (uintptr_t) &mask2);
# else
	k_sigsuspend(mask1, 0xdeadbeef, mask2);
# endif
	if (EINTR != errno)
		perror_msg_and_skip("sigsuspend");

	printf("sigsuspend(~[%s]) = ? ERESTARTNOHAND"
	       " (To be restarted if no handler)\n", sigtxt[signo]);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigsuspend")

#endif
