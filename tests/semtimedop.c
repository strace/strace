/*
 * Check decoding of semtimedop syscall.
 *
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_semtimedop

# define SYSCALL_NR __NR_semtimedop
# define SYSCALL_NAME "semtimedop"

# ifdef __NR_semtimedop_time64
#  define semtimedop_timespec_t kernel_timespec32_t
# else
#  define semtimedop_timespec_t kernel_timespec64_t
# endif

# include "semtimedop-syscall.c"

#else

SKIP_MAIN_UNDEFINED("__NR_semtimedop")

#endif
