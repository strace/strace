/*
 * Check decoding of accept4 syscall.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "kernel_fcntl.h"

#if defined HAVE_ACCEPT4

# define TEST_SYSCALL_NAME accept4
# define SUFFIX_ARGS , O_CLOEXEC
# define SUFFIX_STR ", SOCK_CLOEXEC"
# include "accept.c"

#else

SKIP_MAIN_UNDEFINED("HAVE_ACCEPT4")

#endif
