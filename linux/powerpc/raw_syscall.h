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
	register kernel_ulong_t r0 __asm__("r0") = nr;
	register kernel_ulong_t r3 __asm__("r3");
	__asm__ __volatile__("sc\n\t"
			     "mfcr %0"
			     : "+r"(r0), "=r"(r3)
			     :
			     : "memory", "cr0", "ctr", "lr",
			       "r4", "r5", "r6", "r7", "r8",
			       "r9", "r10", "r11", "r12");
	*err = !!(r0 & 0x10000000);
	return r3;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
