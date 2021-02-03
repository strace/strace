/*
 * A helper that executes the specified program
 * with the ptrace PTRACE_GET_SYSCALL_INFO disabled.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define DISABLE_PTRACE_REQUEST		PTRACE_GET_SYSCALL_INFO
#define DEFAULT_PROGRAM_INVOCATION_NAME	"disable_ptrace_get_syscall_info"
#include "disable_ptrace_request.c"
