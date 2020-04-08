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
	register kernel_ulong_t r11 __asm__("r11") = nr;
	__asm__ __volatile__("l.sys 1"
			     : "+r"(r11)
			     :
			     : "memory", "r3", "r4", "r5", "r6", "r7", "r8",
			       "r12", "r13", "r15", "r17", "r19", "r21",
			       "r23", "r25", "r27", "r29", "r31");
	return r11;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
