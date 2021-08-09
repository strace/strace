/*
 * Raw syscalls.
 *
 * Copyright (c) 2021-2022 The strace developers.
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
	register kernel_ulong_t a7 __asm__("a7") = nr;
	register kernel_ulong_t a0 __asm__("a0");
	__asm__ __volatile__("syscall 0"
			     : "=r"(a0)
			     : "r"(a7)
			     : "memory");
	return a0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
