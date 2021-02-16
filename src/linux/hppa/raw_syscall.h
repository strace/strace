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
	register kernel_ulong_t r28 __asm__("r28");
	__asm__ __volatile__("copy %%r19, %%r4\n\t"
			     "ble 0x100(%%sr2, %%r0)\n\t"
			     "copy %1, %%r20\n\t"
			     "copy %%r4, %%r19\n\t"
			     : "=r"(r28)
			     : "r"(nr)
			     : "memory", "%r1", "%r2", "%r4",
			       "%r21", "%r22", "%r23", "%r24", "%r25", "%r26" );
	return r28;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
