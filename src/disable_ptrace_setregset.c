/*
 * A helper that executes the specified program
 * with the ptrace PTRACE_SETREGSET disabled.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define DISABLE_PTRACE_REQUEST		PTRACE_SETREGSET
#define DEFAULT_PROGRAM_INVOCATION_NAME	"disable_ptrace_setregset"
#include "disable_ptrace_request.c"
