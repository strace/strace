/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_ipc && !defined __ARM_EABI__

# include "semop-common.c"

# define XLAT_MACROS_ONLY
# include "xlat/ipccalls.h"
# undef XLAT_MACROS_ONLY

static long
k_semop(const unsigned int semid,
	const kernel_ulong_t sops,
	const unsigned int nsops)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | SEMOP;
	const kernel_ulong_t arg2 = fill | semid;
	const kernel_ulong_t arg3 = fill | nsops;
	const kernel_ulong_t arg5 = sops;
	const long rc = syscall(__NR_ipc, arg1, arg2, arg3, bad, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

#else

SKIP_MAIN_UNDEFINED("__NR_ipc && !__ARM_EABI__")

#endif
