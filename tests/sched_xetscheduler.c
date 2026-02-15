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

static const char *
scheduler_str(long rc)
{
	switch (rc) {
	case SCHED_FIFO:
		return "SCHED_FIFO";
	case SCHED_RR:
		return "SCHED_RR";
#ifdef SCHED_BATCH
	case SCHED_BATCH:
		return "SCHED_BATCH";
#endif
#ifdef SCHED_IDLE
	case SCHED_IDLE:
		return "SCHED_IDLE";
#endif
#ifdef SCHED_ISO
	case SCHED_ISO:
		return "SCHED_ISO";
#endif
#ifdef SCHED_DEADLINE
	case SCHED_DEADLINE:
		return "SCHED_DEADLINE";
#endif
#ifdef SCHED_EXT
	case SCHED_EXT:
		return "SCHED_EXT";
#endif
	}

	return "SCHED_OTHER";
}

int
main(void)
{
	PIDNS_TEST_INIT;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct sched_param, param);
	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);

	long rc = syscall(__NR_sched_getscheduler, pid);
	pidns_print_leader();
	printf("sched_getscheduler(%d%s) = %#lx (%s%s)\n",
	       pid, pid_str, rc,
	       rc & SCHED_RESET_ON_FORK ? "SCHED_RESET_ON_FORK|" : "",
	       scheduler_str(rc & ~SCHED_RESET_ON_FORK));

	rc = syscall(__NR_sched_getscheduler, -1);
	pidns_print_leader();
	printf("sched_getscheduler(-1) = %s\n", sprintrc(rc));

	param->sched_priority = -1;

	rc = syscall(__NR_sched_setscheduler, pid, SCHED_FIFO, NULL);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, " XLAT_FMT ", NULL) = %s\n",
	       pid, pid_str, XLAT_ARGS(SCHED_FIFO), sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid,
		     SCHED_RESET_ON_FORK | SCHED_FIFO, param + 1);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, " XLAT_FMT ", %p) = %s\n",
	       pid, pid_str, XLAT_ARGS(SCHED_RESET_ON_FORK|SCHED_FIFO),
	       param + 1, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid, 0xfaceda7a, param);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, %#x" NRAW(" /* SCHED_??? */")
	       ", {sched_priority=%d}) = %s\n",
	       pid, pid_str, 0xfaceda7a,
	       param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, -1, SCHED_RR, param);
	pidns_print_leader();
	printf("sched_setscheduler(-1, " XLAT_FMT ", {sched_priority=%d})"
	       " = %s\n",
	       XLAT_ARGS(SCHED_RR), param->sched_priority, sprintrc(rc));

	rc = syscall(__NR_sched_setscheduler, pid, 0x43218765, param);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, " XLAT_FMT ", {sched_priority=%d})"
	       " = %s\n",
	       pid, pid_str, XLAT_ARGS(SCHED_RESET_ON_FORK|0x3218765),
	       param->sched_priority, sprintrc(rc));

	param->sched_priority = 0;

	rc = syscall(__NR_sched_setscheduler, pid,
		     SCHED_RESET_ON_FORK | SCHED_OTHER, param);
	pidns_print_leader();
	printf("sched_setscheduler(%d%s, " XLAT_FMT ", {sched_priority=0})"
	       " = %s\n",
	       pid, pid_str, XLAT_ARGS(SCHED_RESET_ON_FORK|SCHED_OTHER),
	       sprintrc(rc));

	rc = syscall(__NR_sched_getscheduler, pid);
	pidns_print_leader();
	printf("sched_getscheduler(%d%s) = %#lx (%s%s)\n",
	       pid, pid_str, rc,
	       rc & SCHED_RESET_ON_FORK ? "SCHED_RESET_ON_FORK|" : "",
	       scheduler_str(rc & ~SCHED_RESET_ON_FORK));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
