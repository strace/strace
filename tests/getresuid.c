/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getresuid

# define SYSCALL_NR	__NR_getresuid
# define SYSCALL_NAME	"getresuid"

# if defined __NR_getresuid32 && __NR_getresuid != __NR_getresuid32
#  define UGID_TYPE	short
# else
#  define UGID_TYPE	int
# endif

# include "getresugid.c"

#else

SKIP_MAIN_UNDEFINED("__NR_getresuid")

#endif
