/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fstatfs64

# define SYSCALL_ARG_FMT		"%d"
# define SYSCALL_ARG(file, desc)	(desc)
# define SYSCALL_NR			__NR_fstatfs64
# define SYSCALL_NAME			"fstatfs64"
# include "xstatfs64.c"

#else

SKIP_MAIN_UNDEFINED("__NR_fstatfs64")

#endif
