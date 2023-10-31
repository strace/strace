/*
 * Check decoding of sched_getparam and sched_setparam syscalls.
 *
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <sched.h>
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	struct sched_param *const param =
		tail_alloc(sizeof(struct sched_param));

	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	long rc = syscall(__NR_sched_getparam, pid, param);
	pidns_print_leader();
	printf("sched_getparam(%d%s, [%d]) = %ld\n",
	       pid, pid_str, param->sched_priority, rc);

	param->sched_priority = -1;
	rc = syscall(__NR_sched_setparam, pid, param);
	pidns_print_leader();
	printf("sched_setparam(%d%s, [%d]) = %s\n",
	       pid, pid_str,
	       param->sched_priority, sprintrc(rc));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
