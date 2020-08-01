/*
 * Check decoding of tgkill syscall.
 *
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_tgkill

# include <signal.h>
# include <stdio.h>
# include <unistd.h>

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
	const int pid = getpid();
	const int bad_pid = -1;
	const int bad_sig = 0xface;

	k_tgkill(pid, pid, 0);
	printf("tgkill(%d, %d, 0) = %s\n", pid, pid, errstr);

	k_tgkill(pid, bad_pid, 0);
	printf("tgkill(%d, %d, 0) = %s\n", pid, bad_pid, errstr);

	k_tgkill(bad_pid, pid, 0);
	printf("tgkill(%d, %d, 0) = %s\n", bad_pid, pid, errstr);

	k_tgkill(pid, pid, SIGCONT);
	printf("tgkill(%d, %d, SIGCONT) = %s\n", pid, pid, errstr);

	k_tgkill(pid, pid, bad_sig);
	printf("tgkill(%d, %d, %d) = %s\n", pid, pid, bad_sig, errstr);

	k_tgkill(pid, pid, -bad_sig);
	printf("tgkill(%d, %d, %d) = %s\n", pid, pid, -bad_sig, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_tgkill")

#endif
