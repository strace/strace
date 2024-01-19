/*
 * Check decoding of utimes syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "kernel_timeval.h"

#ifdef __NR_utimes

# define TEST_SYSCALL_NR	__NR_utimes
# define TEST_SYSCALL_STR	"utimes"
# define TEST_STRUCT		kernel_old_timeval_t
# include "xutimes.c"

#else

SKIP_MAIN_UNDEFINED("__NR_utimes")

#endif
