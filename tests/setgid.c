/*
 * Check decoding of setgid syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#define SYSCALL_NR	__NR_setgid
#define SYSCALL_NAME	"setgid"

#if defined __NR_setgid32 && __NR_setgid != __NR_setgid32
# define UGID_TYPE	short
# define GETUGID	syscall(__NR_getegid)
# define CHECK_OVERFLOWUGID(arg)	check_overflowgid(arg)
#else
# define UGID_TYPE	int
# define GETUGID	getegid()
# define CHECK_OVERFLOWUGID(arg)
#endif

#include "setugid.c"
