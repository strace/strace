/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	struct sigaction sa = {
		.sa_handler = SIG_IGN
	};
	union sigval value = {
		.sival_ptr = (void *) (unsigned long) 0xdeadbeefbadc0dedULL
	};
	pid_t pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	assert(sigaction(SIGUSR1, &sa, NULL) == 0);
	if (sigqueue(pid, SIGUSR1, value))
		perror_msg_and_skip("sigqueue");
	pidns_print_leader();
	printf("rt_sigqueueinfo(%d%s, SIGUSR1, {si_signo=SIGUSR1, "
		"si_code=SI_QUEUE, si_pid=%d%s, si_uid=%u, "
		"si_value={int=%d, ptr=%p}}) = 0\n",
		pid, pid_str, pid, pid_str,
		getuid(), value.sival_int, value.sival_ptr);
	pidns_print_leader();
	puts("+++ exited with 0 +++");

	return 0;
}
