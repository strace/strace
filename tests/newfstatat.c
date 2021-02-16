/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_newfstatat

# define TEST_SYSCALL_NR __NR_newfstatat
# define TEST_SYSCALL_STR "newfstatat"
# include "fstatat.c"

#else

SKIP_MAIN_UNDEFINED("__NR_newfstatat")

#endif
