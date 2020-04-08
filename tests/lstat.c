/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_lstat

# define TEST_SYSCALL_NR __NR_lstat
# define TEST_SYSCALL_STR "lstat"
# define SAMPLE_SIZE ((libc_off_t) (kernel_ulong_t) 43147718418ULL)
# include "lstatx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_lstat")

#endif
