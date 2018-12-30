/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

int
main(void)
{
#if defined __x86_64__ && defined __ILP32__
	/*
	 * x32 is broken from the beginning:
	 * https://lkml.org/lkml/2015/11/30/790
	 */
	error_msg_and_skip("x32 is broken");
#else
	const sigset_t set = {};
	const struct sigaction act = { .sa_handler = SIG_IGN };
	const struct itimerval itv = { .it_value.tv_usec = 111111 };
	struct timespec req = { .tv_nsec = 222222222 }, rem;

	assert(sigaction(SIGALRM, &act, NULL) == 0);
	assert(sigprocmask(SIG_SETMASK, &set, NULL) == 0);
	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_skip("setitimer");
	if (nanosleep(&req, &rem))
		perror_msg_and_fail("nanosleep");

	printf("nanosleep\\(\\{tv_sec=%lld, tv_nsec=%llu\\}"
	       ", \\{tv_sec=%lld, tv_nsec=%llu\\}\\)"
	       " = \\? ERESTART_RESTARTBLOCK \\(Interrupted by signal\\)\n",
	       (long long) req.tv_sec, zero_extend_signed_to_ull(req.tv_nsec),
	       (long long) rem.tv_sec, zero_extend_signed_to_ull(rem.tv_nsec));
	puts("--- SIGALRM \\{si_signo=SIGALRM, si_code=SI_KERNEL\\} ---");
# ifdef __arm__
/* old kernels used to overwrite ARM_r0 with -EINTR */
#  define ALTERNATIVE_NANOSLEEP_REQ "0xfffffffc|"
# else
#  define ALTERNATIVE_NANOSLEEP_REQ ""
# endif
	printf("(nanosleep\\((%s\\{tv_sec=%lld, tv_nsec=%llu\\})"
	       ", %p|restart_syscall\\(<\\.\\.\\."
	       " resuming interrupted nanosleep \\.\\.\\.>)\\) = 0\n",
	       ALTERNATIVE_NANOSLEEP_REQ,
	       (long long) req.tv_sec, zero_extend_signed_to_ull(req.tv_nsec),
	       &rem);

	puts("\\+\\+\\+ exited with 0 \\+\\+\\+");
	return 0;
#endif
}
