/*
 * Copyright (c) 2014-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_statfs

# define SYSCALL_ARG_FMT		"\"%s\""
# define SYSCALL_ARG(file, desc)	(file)
# define SYSCALL_NR			__NR_statfs
# define SYSCALL_NAME			"statfs"
# include "xstatfs.c"

#else

SKIP_MAIN_UNDEFINED("__NR_statfs")

#endif
