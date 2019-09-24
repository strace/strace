/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_ugetrlimit

# define NR_GETRLIMIT	__NR_ugetrlimit
# define STR_GETRLIMIT	"ugetrlimit"
# include "xgetrlimit.c"

#else

SKIP_MAIN_UNDEFINED("__NR_ugetrlimit")

#endif
