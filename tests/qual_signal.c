/*
 * Check how strace -e signal=set works.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t pid;
static uid_t uid;

static void
handler(int sig)
{
}

static void
test_sig(int signo, const char *name)
{
	const struct sigaction act = { .sa_handler = handler };

	if (sigaction(signo, &act, NULL))
		perror_msg_and_fail("sigaction: %d", signo);

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, signo);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask: %d", signo);

	if (kill(pid, signo))
		perror_msg_and_fail("kill(%d, %d)", pid, signo);

	if (name && *name)
		printf("--- %s {si_signo=%s, si_code=SI_USER"
		       ", si_pid=%d, si_uid=%d} ---\n",
		       name, name, pid, uid);
}

int
main(int ac, const char **av)
{
	assert(ac & 1);

	pid = getpid();
	uid = geteuid();

	for (int i = 1; i < ac; i += 2)
		test_sig(atoi(av[i]), av[i + 1]);

	puts("+++ exited with 0 +++");
	return 0;
}
