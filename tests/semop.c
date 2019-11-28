/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_semop

# include "semop-common.c"

static long
k_semop(const unsigned int semid,
	const kernel_ulong_t sops,
	const unsigned int nsops)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | semid;
	const kernel_ulong_t arg2 = sops;
	const kernel_ulong_t arg3 = fill | nsops;
	const long rc = syscall(__NR_semop, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

#else

SKIP_MAIN_UNDEFINED("__NR_semop")

#endif
