/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_mkdirat

# define TEST_SYSCALL_NR		__NR_mkdirat
# define TEST_SYSCALL_STR		"mkdirat"
# define TEST_SYSCALL_PREFIX_ARGS	(long int) 0xdeadbeefffffffffULL,
# define TEST_SYSCALL_PREFIX_STR	"-1, "
# include "umode_t.c"

#else

SKIP_MAIN_UNDEFINED("__NR_mkdirat")

#endif
