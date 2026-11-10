/*
 * A helper that executes the specified program
 * with PTRACE_GETSIGINFO failing with ESRCH.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define DISABLE_PTRACE_REQUEST		PTRACE_GETSIGINFO
#define DISABLE_PTRACE_ERRNO		ESRCH
#define DEFAULT_PROGRAM_INVOCATION_NAME	"disable_ptrace_getsiginfo"
#include "disable_ptrace_request.c"
