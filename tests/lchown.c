/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_lchown

# define SYSCALL_NR __NR_lchown
# define SYSCALL_NAME "lchown"

# if defined __NR_lchown32 && __NR_lchown != __NR_lchown32
#  define UGID_TYPE_IS_SHORT
# endif

# include "xchownx.c"

#else

SKIP_MAIN_UNDEFINED("__NR_lchown")

#endif
