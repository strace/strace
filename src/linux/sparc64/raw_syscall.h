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
	register kernel_ulong_t g1 __asm__("g1") = nr;
	register kernel_ulong_t rval __asm__("o0");
	__asm__ __volatile__("ta 0x6d\n\t"
			     "bcc,pt %%xcc, 1f\n\t"
			     "mov 0, %0\n\t"
			     "mov 1, %0\n\t"
			     "1:"
			     : "+r"(g1), "=r"(rval)
			     :
			     : "memory", "cc", "f0", "f1", "f2", "f3", "f4",
			       "f5", "f6", "f7", "f8", "f9", "f10", "f11",
			       "f12", "f13", "f14", "f15", "f16", "f17",
			       "f18", "f19", "f20", "f21", "f22", "f23",
			       "f24", "f25", "f26", "f27", "f28", "f29",
			       "f30", "f31", "f32", "f34", "f36", "f38",
			       "f40", "f42", "f44", "f46", "f48", "f50",
			       "f52", "f54", "f56", "f58", "f60", "f62");
	*err = g1;
	return rval;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
