/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_statfs64

# define SYSCALL_ARG_FMT		"\"%s\""
# define SYSCALL_ARG(file, desc)	(file)
# define SYSCALL_NR			__NR_statfs64
# define SYSCALL_NAME			"statfs64"
# include "xstatfs64.c"

#else

SKIP_MAIN_UNDEFINED("__NR_statfs64")

#endif
