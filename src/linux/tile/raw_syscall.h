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
	kernel_ulong_t r, e, c2, c3, c4, c5, c10;
	__asm__ __volatile__("swint1"
			     : "=R00"(r), "=R01"(e),
			       "=R02"(c2), "=R03"(c3), "=R04"(c4),
			       "=R05"(c5), "=R10"(c10)
			     : "R10"(nr)
			     : "memory", "r6", "r7", "r8", "r9", "r11",
			       "r12", "r13", "r14", "r15", "r16", "r17",
			       "r18", "r19", "r20", "r21", "r22", "r23",
			       "r24", "r25", "r26", "r27", "r28", "r29");
	*err = e;
	return r;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
