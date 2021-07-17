/*
 * Check decoding of tgkill syscall.
 *
 * Copyright (c) 2020-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_tgkill(const unsigned int tgid,
	 const unsigned int tid,
	 const unsigned int sig)
{
        const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
        const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
        const kernel_ulong_t arg1 = fill | tgid;
        const kernel_ulong_t arg2 = fill | tid;
        const kernel_ulong_t arg3 = fill | sig;
        const long rc = syscall(__NR_tgkill, arg1, arg2, arg3, bad, bad, bad);
        errstr = sprintrc(rc);
        return rc;
}

int
main(void)
{
	PIDNS_TEST_INIT;

	const int pid = getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
	const int tid = syscall(__NR_gettid);
	const char *tid_str = pidns_pid2str(PT_TID);
	const int bad_pid = -1;
	const int bad_sig = 0xface;

	k_tgkill(pid, tid, 0);
	pidns_print_leader();
	printf("tgkill(%d%s, %d%s, 0) = %s\n",
		pid, pid_str, tid, tid_str, errstr);

	k_tgkill(pid, bad_pid, 0);
	pidns_print_leader();
	printf("tgkill(%d%s, %d, 0) = %s\n",
		pid, pid_str, bad_pid, errstr);

	k_tgkill(bad_pid, tid, 0);
	pidns_print_leader();
	printf("tgkill(%d, %d%s, 0) = %s\n",
		bad_pid, tid, tid_str, errstr);

	k_tgkill(pid, tid, SIGCONT);
	pidns_print_leader();
	printf("tgkill(%d%s, %d%s, SIGCONT) = %s\n",
		pid, pid_str, tid, tid_str, errstr);

	k_tgkill(pid, tid, bad_sig);
	pidns_print_leader();
	printf("tgkill(%d%s, %d%s, %d) = %s\n",
		pid, pid_str, tid, tid_str, bad_sig, errstr);

	k_tgkill(pid, tid, -bad_sig);
	pidns_print_leader();
	printf("tgkill(%d%s, %d%s, %d) = %s\n",
		pid, pid_str, tid, tid_str, -bad_sig, errstr);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
