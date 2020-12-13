/*
 * Check decoding of clock_gettime, clock_settime, and
 * clock_getres syscalls.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_clock_gettime \
 && defined __NR_clock_settime \
 && defined __NR_clock_getres

# define SYSCALL_NR_gettime	__NR_clock_gettime
# define SYSCALL_NR_settime	__NR_clock_settime
# define SYSCALL_NR_getres	__NR_clock_getres

# define SYSCALL_NAME_gettime	"clock_gettime"
# define SYSCALL_NAME_settime	"clock_settime"
# define SYSCALL_NAME_getres	"clock_getres"

# if defined __NR_clock_gettime64 \
  || defined __NR_clock_settime64 \
  || defined __NR_clock_getres_time64
#  include "arch_defs.h"
#  define clock_timespec_t kernel_timespec32_t
# else
#  define clock_timespec_t kernel_timespec64_t
# endif

# include "clock_xettime-common.c"

#else

SKIP_MAIN_UNDEFINED("__NR_clock_gettime && __NR_clock_settime && __NR_clock_getres")

#endif
