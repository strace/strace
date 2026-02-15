/*
 * Check decoding of sched_getscheduler syscall using syscall retval injection.
 *
 * Copyright (c) 2016-2026 The strace developers.
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

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sched_param, param);
	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	long rc = syscall(__NR_sched_getscheduler, pid);
	const char *rof = rc & SCHED_RESET_ON_FORK
			  ? "SCHED_RESET_ON_FORK|" : NULL;
	const char *scheduler = NULL;
	switch (rc & ~SCHED_RESET_ON_FORK) {
		case SCHED_FIFO:
			scheduler = "SCHED_FIFO";
			break;
		case SCHED_RR:
			scheduler = "SCHED_RR";
			break;
		case SCHED_OTHER:
			scheduler = "SCHED_OTHER";
			break;
#ifdef SCHED_BATCH
		case SCHED_BATCH:
			scheduler = "SCHED_BATCH";
			break;
#endif
#ifdef SCHED_IDLE
		case SCHED_IDLE:
			scheduler = "SCHED_IDLE";
			break;
#endif
#ifdef SCHED_ISO
		case SCHED_ISO:
			scheduler = "SCHED_ISO";
			break;
#endif
#ifdef SCHED_DEADLINE
		case SCHED_DEADLINE:
			scheduler = "SCHED_DEADLINE";
			break;
#endif
#ifdef SCHED_EXT
		case SCHED_EXT:
			scheduler = "SCHED_EXT";
			break;
#endif
	}
	pidns_print_leader();
	printf("sched_getscheduler(%d%s) = %#lx",
	       pid, pid_str, rc);
	if (rof != NULL || scheduler != NULL) {
		fputs(" (", stdout);
		if (rof)
			fputs(rof, stdout);
		if (scheduler)
			fputs(scheduler, stdout);
		else
			printf("%#lx", rc & ~SCHED_RESET_ON_FORK);
		fputs(")", stdout);
	}
	puts(" (INJECTED)");

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
