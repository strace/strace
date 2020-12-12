/*
 * Check decoding of pselect6_time64 syscall.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pselect6_time64

# define SYSCALL_NR __NR_pselect6_time64
# define SYSCALL_NAME "pselect6_time64"
# define pselect6_timespec_t kernel_timespec64_t

# include "pselect6-common.c"

#else

SKIP_MAIN_UNDEFINED("__NR_pselect6_time64")

#endif
