/*
 * Raw syscalls.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RAW_SYSCALL_H
# define STRACE_RAW_SYSCALL_H

# include "kernel_types.h"

static inline kernel_ulong_t
raw_syscall_0(const kernel_ulong_t nr, kernel_ulong_t *err)
{
	*err = 0;
	register kernel_ulong_t ret;
	__asm__ __volatile__("excpt 0"
			     : "=q0"(ret)
			     : "qA"(nr)
			     : "memory", "cc");
	return ret;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
