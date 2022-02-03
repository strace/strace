/*
 * Check decoding of SEGV_PKUERR.
 *
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>

#if defined HAVE_SIGINFO_T_SI_PKEY && defined SEGV_PKUERR

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/mman.h>

static void
handler(int sig, siginfo_t *info, void *ucontext)
{
	if (info->si_code != SEGV_PKUERR)
		error_msg_and_skip("SIGSEGV: si_code = %d", info->si_code);

	printf("--- SIGSEGV {si_signo=SIGSEGV, si_code=SEGV_PKUERR"
	       ", si_addr=%p, si_pkey=%u} ---\n",
	       info->si_addr, info->si_pkey);
	exit(0);
}

int
main(void) {
	int *p = mmap(NULL, get_page_size(), PROT_EXEC,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED)
		perror_msg_and_fail("mmap");

	const struct sigaction act = {
		.sa_sigaction = handler,
		.sa_flags = SA_SIGINFO | SA_RESETHAND
	};
	if (sigaction(SIGSEGV, &act, NULL))
		perror_msg_and_fail("sigaction");

	__asm__ volatile("":: "r" (*p));

	error_msg_and_skip("PROT_EXEC page is readable");
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SIGINFO_T_SI_PKEY && SEGV_PKUERR")

#endif
