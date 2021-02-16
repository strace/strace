/*
 * Check decoding of ERESTARTSYS error code.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <unistd.h>

static int sv[2];

static void
handler(int sig)
{
	close(sv[1]);
	sv[1] = -1;
}

int
main(void)
{
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");

	const struct sigaction act = {
		.sa_handler = handler,
		.sa_flags = SA_RESTART
	};
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

	if (recvfrom(sv[0], &sv[1], sizeof(sv[1]), 0, NULL, NULL))
		perror_msg_and_fail("recvfrom");

	printf("recvfrom(%d, %p, %d, 0, NULL, NULL) = ? ERESTARTSYS"
	       " (To be restarted if SA_RESTART is set)\n",
	       sv[0], &sv[1], (int) sizeof(sv[1]));
	printf("recvfrom(%d, \"\", %d, 0, NULL, NULL) = 0\n",
	       sv[0], (int) sizeof(sv[1]));

	puts("+++ exited with 0 +++");
	return 0;
}
