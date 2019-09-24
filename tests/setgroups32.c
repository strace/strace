/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_setgroups32

# include "setgroups.c"

#else

SKIP_MAIN_UNDEFINED("__NR_setgroups32")

#endif
