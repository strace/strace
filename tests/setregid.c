/*
 * Check decoding of setregid syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#define SYSCALL_NR	__NR_setregid
#define SYSCALL_NAME	"setregid"

#if defined __NR_setregid32 && __NR_setregid != __NR_setregid32
# define UGID_TYPE	short
# define GETUGID	syscall(__NR_getegid)
# define CHECK_OVERFLOWUGID(arg)	check_overflowgid(arg)
#else
# define UGID_TYPE	int
# define GETUGID	getegid()
# define CHECK_OVERFLOWUGID(arg)
#endif

#include "setreugid.c"
