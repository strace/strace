/*
 * Check fault injection along with signal injection.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include "scno.h"

static struct stat before, after;

static void
handler(int sig)
{
	if (stat(".", &after))
		syscall(__NR_exit_group, 2);

	if (before.st_dev != after.st_dev || before.st_ino != after.st_ino)
		syscall(__NR_exit_group, 3);

	syscall(__NR_exit_group, 0);
}

int
main(void)
{
	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGUSR1, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	if (stat(".", &before))
		perror_msg_and_fail("stat");

	syscall(__NR_chdir, ".");
	syscall(__NR_exit_group, 1);
	return 1;
}
