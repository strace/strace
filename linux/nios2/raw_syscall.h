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
	register kernel_ulong_t r2 __asm__("r2") = nr;
	register kernel_ulong_t r7 __asm__("r7");
	__asm__ __volatile__("trap"
			     : "+r"(r2), "=r"(r7)
			     :
			     : "memory");
	*err = r7;
	return r2;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
