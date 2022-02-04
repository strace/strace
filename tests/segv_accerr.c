/*
 * Check decoding of SEGV_ACCERR.
 *
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>

#ifdef SEGV_ACCERR

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/mman.h>

static void
handler(int sig)
{
	_exit(0);
}

int
main(void) {
	int *p = mmap(NULL, get_page_size(), PROT_NONE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (p == MAP_FAILED)
		perror_msg_and_fail("mmap");

	const struct sigaction act = {
		.sa_handler = handler,
		.sa_flags = SA_RESETHAND
	};
	if (sigaction(SIGSEGV, &act, NULL))
		perror_msg_and_fail("sigaction");

	printf("--- SIGSEGV {si_signo=SIGSEGV"
	       ", si_code=SEGV_ACCERR, si_addr=%p} ---\n", p);
	fflush(stdout);

	__asm__ volatile("":: "r" (*p));

	error_msg_and_skip("PROT_NONE page is readable");
}

#else

SKIP_MAIN_UNDEFINED("SEGV_ACCERR")

#endif
