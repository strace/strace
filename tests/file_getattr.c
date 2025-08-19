/*
 * Check decoding of file_getattr syscall.
 *
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define SYSCALL_NR	__NR_file_getattr
#define SYSCALL_NAME	"file_getattr"
#define SYSCALL_is_set	0

#include "file_xetattr-common.c"
