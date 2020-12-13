/*
 * Check decoding of clock_gettime64, clock_settime64, and
 * clock_getres_time64 syscalls.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_clock_gettime64 \
 && defined __NR_clock_settime64 \
 && defined __NR_clock_getres_time64

# define SYSCALL_NR_gettime	__NR_clock_gettime64
# define SYSCALL_NR_settime	__NR_clock_settime64
# define SYSCALL_NR_getres	__NR_clock_getres_time64

# define SYSCALL_NAME_gettime	"clock_gettime64"
# define SYSCALL_NAME_settime	"clock_settime64"
# define SYSCALL_NAME_getres	"clock_getres_time64"

# define clock_timespec_t kernel_timespec64_t

# include "clock_xettime-common.c"

#else

SKIP_MAIN_UNDEFINED("__NR_clock_gettime64 && __NR_clock_settime64 && __NR_clock_getres_time64")

#endif
