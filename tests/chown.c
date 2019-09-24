/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_chown

# define SYSCALL_NR __NR_chown
# define SYSCALL_NAME "chown"

# if defined __NR_chown32 && __NR_chown != __NR_chown32
#  define UGID_TYPE_IS_SHORT
# endif

# include "xchownx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_chown")

#endif
