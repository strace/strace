/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/**
 * Returns prefix for a syscall constant literal.  It is has to be that way
 * thanks to ARM that decided to prefix their special system calls like sys32
 * and sys26 with __ARM_NR_* prefix instead of __NR_*, so we can't simply print
 * "__NR_".
 */
static const char *
nr_prefix(kernel_ulong_t scno)
{
	return "__NR_";
}
