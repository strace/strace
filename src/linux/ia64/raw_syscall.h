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
	register kernel_ulong_t r15 __asm__("r15") = nr;
	register kernel_ulong_t r8 __asm__("r8");
	register kernel_ulong_t r10 __asm__("r10");
	__asm__ __volatile__("break 0x100000"
			     : "=r"(r8), "=r"(r10), "+r"(r15)
			     :
			     : "memory", "out0", "out1", "out2",
			       "out3", "out4", "out5", "out6", "out7",
			       "r2", "r3", "r9", "r11", "r13",
			       "r14", "r16", "r17", "r18", "r19", "r20",
			       "r21", "r22", "r23", "r24", "r25", "r26",
			       "r27", "r28", "r29", "r30", "r31",
			       "p6", "p7", "p8", "p9", "p10",
			       "p11", "p12", "p13", "p14", "p15",
			       "f6", "f7", "f8", "f9", "f10",
			       "f11", "f12", "f13", "f14", "f15",
			       "f5", "f6", "f7", "f8", "f9", "f10", "f11",
			       "b6", "b7");
	*err = !!r10;
	return r8;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
