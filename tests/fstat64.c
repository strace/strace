/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fstat64

# define TEST_SYSCALL_NR __NR_fstat64
# define TEST_SYSCALL_STR "fstat64"
# define STRUCT_STAT struct stat64
# define STRUCT_STAT_STR "struct stat64"
# define STRUCT_STAT_IS_STAT64 1
# include "fstatx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_fstat64")

#endif
