/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_SELINUX_RUNTIME

# define TEST_SECONTEXT
# include "access.c"

#else

SKIP_MAIN_UNDEFINED("HAVE_SELINUX_RUNTIME")

#endif
