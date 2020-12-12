/*
 * Check decoding of semtimedop ipc call.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_ipc && !defined __ARM_EABI__

# define SYSCALL_NAME "semtimedop"

# if SIZEOF_LONG > 4
#  define semtimedop_timespec_t kernel_timespec64_t
# else
#  include "arch_defs.h"
#  define semtimedop_timespec_t kernel_timespec32_t
# endif

# include "semtimedop-common.c"

# define XLAT_MACROS_ONLY
# include "xlat/ipccalls.h"
# undef XLAT_MACROS_ONLY

static long
k_semtimedop_imp(const kernel_ulong_t semid,
		 const kernel_ulong_t sops,
		 const kernel_ulong_t nsops,
		 const kernel_ulong_t timeout)
{
	static const kernel_ulong_t bad =
		(kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	static const kernel_ulong_t call =
		(kernel_ulong_t) 0xdefaced00000000ULL | SEMTIMEDOP;
# if defined __s390x__ || defined __s390__
	return syscall(__NR_ipc, call, semid, nsops, timeout, sops, bad);
# else
	return syscall(__NR_ipc, call, semid, nsops, bad, sops, timeout);
# endif
}

#else

SKIP_MAIN_UNDEFINED("__NR_ipc && !__ARM_EABI__")

#endif
