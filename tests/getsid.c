/*
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "pidns.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	pid_t pid = getpid();
	pidns_print_leader();
	printf("getsid(%d%s) = %d%s\n", pid, pidns_pid2str(PT_TGID),
		getsid(pid), pidns_pid2str(PT_SID));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
