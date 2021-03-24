/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#define TEST_SECONTEXT

#ifndef HAVE_SELINUX_RUNTIME

SKIP_MAIN_UNDEFINED("HAVE_SELINUX_RUNTIME")

#else

#include "access.c"

#endif
