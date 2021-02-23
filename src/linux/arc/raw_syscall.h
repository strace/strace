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
	register kernel_ulong_t r8 __asm__("r8") = nr;
	register kernel_ulong_t r0 __asm__("r0");

# ifdef __A7__
#  define ARC_TRAP_INSN "trap0"
# else
#  define ARC_TRAP_INSN "trap_s 0 "
# endif

	__asm__ __volatile__(ARC_TRAP_INSN
			     : "=r"(r0)
			     : "r"(r8)
			     : "memory");

# undef ARC_TRAP_INSN

	return r0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
