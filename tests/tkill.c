/*
 * Check decoding of tkill syscall.
 *
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#ifdef __NR_tkill

# include <signal.h>
# include <stdio.h>
# include <unistd.h>

static const char *errstr;

static long
k_tkill(const unsigned int tid, const unsigned int sig)
{
        const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
        const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
        const kernel_ulong_t arg1 = fill | tid;
        const kernel_ulong_t arg2 = fill | sig;
        const long rc = syscall(__NR_tkill, arg1, arg2, bad, bad, bad, bad);
        errstr = sprintrc(rc);
        return rc;
}

int
main(void)
{
	PIDNS_TEST_INIT;

	const int tid = syscall(__NR_gettid);
	const char *tid_str = pidns_pid2str(PT_TID);
	const int bad_pid = -1;
	const int bad_sig = 0xface;

	k_tkill(tid, 0);
	pidns_print_leader();
	printf("tkill(%d%s, 0) = %s\n", tid, tid_str, errstr);

	k_tkill(tid, SIGCONT);
	pidns_print_leader();
	printf("tkill(%d%s, SIGCONT) = %s\n", tid, tid_str, errstr);

	k_tkill(bad_pid, bad_sig);
	pidns_print_leader();
	printf("tkill(%d, %d) = %s\n", bad_pid, bad_sig, errstr);

	k_tkill(bad_pid, -bad_sig);
	pidns_print_leader();
	printf("tkill(%d, %d) = %s\n", bad_pid, -bad_sig, errstr);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_tkill")

#endif
