/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#if defined __NR_select && !defined __NR__newselect

# define TEST_SYSCALL_NR __NR_select
# define TEST_SYSCALL_STR "select"
# include "xselect.c"

#else

SKIP_MAIN_UNDEFINED("__NR_select && !__NR__newselect")

#endif
