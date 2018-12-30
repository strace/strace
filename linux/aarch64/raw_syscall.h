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
	register kernel_ulong_t x8 __asm__("x8") = nr;
	register kernel_ulong_t x0 __asm__("x0");
	__asm__ __volatile__("svc 0"
			     : "=r"(x0)
			     : "r"(x8)
			     : "memory");
	return x0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
