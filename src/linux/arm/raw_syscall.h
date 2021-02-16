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

# ifdef __thumb__ /* && FRAME_POINTERS_ENABLED */

	register kernel_ulong_t rt;
	register kernel_ulong_t r0 __asm__("r0");
	__asm__ __volatile__("mov %1,r7; mov r7,%2; swi 0x0; mov r7,%1"
			     : "=r"(r0), "=&r"(rt)
			     : "r"(nr)
			     : "memory");

# else

	register kernel_ulong_t r7 __asm__("r7") = nr;
	register kernel_ulong_t r0 __asm__("r0");
	__asm__ __volatile__("swi 0x0"
			     : "=r"(r0)
			     : "r"(r7)
			     : "memory");

# endif

	return r0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
