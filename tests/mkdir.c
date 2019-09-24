/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_mkdir

# define TEST_SYSCALL_NR __NR_mkdir
# define TEST_SYSCALL_STR "mkdir"
# include "umode_t.c"

#else

SKIP_MAIN_UNDEFINED("__NR_mkdir")

#endif
