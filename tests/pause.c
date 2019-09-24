/*
 * Check decoding of pause syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Fei Jie <feij.fnst@cn.fujitsu.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pause

# include <errno.h>
# include <signal.h>
# include <stdio.h>
# include <sys/time.h>
# include <unistd.h>

static void
handler(int sig)
{
}

int
main(void)
{
	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const struct itimerval itv = { .it_value.tv_usec = 123456 };
	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_fail("setitimer");

	syscall(__NR_pause);
	if (errno == ENOSYS)
		printf("pause() = -1 ENOSYS (%m)\n");
	else
		printf("pause() = ? ERESTARTNOHAND"
		       " (To be restarted if no handler)\n");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pause")

#endif
