/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int
main(void)
{
	struct sigaction sa = {
		.sa_handler = SIG_IGN
	};
	union sigval value = {
		.sival_ptr = (void *) (unsigned long) 0xdeadbeefbadc0dedULL
	};
	pid_t pid = getpid();

	assert(sigaction(SIGUSR1, &sa, NULL) == 0);
	if (sigqueue(pid, SIGUSR1, value))
		perror_msg_and_skip("sigqueue");
	printf("rt_sigqueueinfo(%u, SIGUSR1, {si_signo=SIGUSR1, "
		"si_code=SI_QUEUE, si_pid=%u, si_uid=%u, "
		"si_value={int=%d, ptr=%p}}) = 0\n",
		pid, pid, getuid(), value.sival_int, value.sival_ptr);
	printf("+++ exited with 0 +++\n");

	return 0;
}
