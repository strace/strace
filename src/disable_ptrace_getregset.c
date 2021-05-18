/*
 * A helper that executes the specified program
 * with the ptrace PTRACE_GETREGSET or PTRACE_GETREGS disabled.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "defs.h"
#define static
#include "getregs_old.h"
#undef static
#undef DISABLE_PTRACE_REQUEST
#ifdef HAVE_GETREGS_OLD
# if defined __x86_64__
#  define DISABLE_PTRACE_REQUEST PTRACE_GETREGSET
# endif
#endif
#define DEFAULT_PROGRAM_INVOCATION_NAME	"disable_ptrace_getregset"
#include "disable_ptrace_request.c"
