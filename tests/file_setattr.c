/*
 * Check decoding of file_setattr syscall.
 *
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define SYSCALL_NR	__NR_file_setattr
#define SYSCALL_NAME	"file_setattr"
#define SYSCALL_is_set	1

#include "file_xetattr-common.c"
