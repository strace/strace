/*
 * Raw syscalls.
 *
 * Copyright (c) 2018-2021 The strace developers.
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
	register kernel_ulong_t r12 __asm__("r12") = nr;
	register kernel_ulong_t r3 __asm__("r3");
	__asm__ __volatile__("brki r14, 8"
			     : "=r"(r3)
			     : "r"(r12)
			     : "memory", "r4", "r5", "r6", "r7",
			       "r8", "r9", "r10", "r11");
	return r3;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
