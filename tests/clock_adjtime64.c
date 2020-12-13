/*
 * Check decoding of clock_adjtime64 syscall.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_clock_adjtime64

# define SYSCALL_NR __NR_clock_adjtime64
# define SYSCALL_NAME "clock_adjtime64"

# include "clock_adjtime-common.c"

#else

SKIP_MAIN_UNDEFINED("__NR_clock_adjtime64")

#endif
