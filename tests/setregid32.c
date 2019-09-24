/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_setregid32

# define SYSCALL_NR	__NR_setregid32
# define SYSCALL_NAME	"setregid32"
# define UGID_TYPE	int
# define GETUGID	getegid()
# define CHECK_OVERFLOWUGID(arg)
# include "setreugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setregid32")

#endif
