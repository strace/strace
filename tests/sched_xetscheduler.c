/*
 * Check decoding of sched_getscheduler and sched_setscheduler syscalls.
 *
 * Copyright (c) 2016-2024 The strace developers.
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
	const char *scheduler;
	switch (rc) {
		case SCHED_FIFO:
			scheduler = "SCHED_FIFO";
			break;
		case SCHED_RR:
			scheduler = "SCHED_RR";
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
		default:
			scheduler = "SCHED_OTHER";
	}
	pidns_print_leader();
	printf("sched_getscheduler(%d%s) = %ld (%s)\n",
	       pid, pid_str, rc, scheduler);

	rc = syscall(__NR_sched_getscheduler, -1);
	pidns_print_leader();
	printf("sched_getscheduler(-1) = %s\n", sprintrc(rc));

	param->sched_priority = -1;

	rc = syscall(__NR_sched_setscheduler, pid, SCHED_FIFO, NULL);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, SCHED_FIFO, NULL) = %s\n",
	       pid, pid_str, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid, SCHED_FIFO, param + 1);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, SCHED_FIFO, %p) = %s\n",
	       pid, pid_str, param + 1, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid, 0xfaceda7a, param);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, %#x /* SCHED_??? */, [%d]) = %s\n",
	       pid, pid_str, 0xfaceda7a,
	       param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, -1, SCHED_FIFO, param);
	pidns_print_leader();
	printf("sched_setscheduler(-1, SCHED_FIFO, [%d]) = %s\n",
	       param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid, SCHED_FIFO, param);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, SCHED_FIFO, [%d]) = %s\n",
	       pid, pid_str, param->sched_priority, sprintrc(rc));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
