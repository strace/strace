/*
 * Raw syscalls.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_RAW_SYSCALL_H
# define STRACE_RAW_SYSCALL_H

# include "kernel_types.h"

# if __mips_isa_rev >= 6
#  define SYSCALL_CLOBBERLIST \
	"memory", "$1", "$3", "$8", "$9", \
	"$10", "$11", "$12", "$13", "$14", "$15", \
	"$24", "$25"
# else
#  define SYSCALL_CLOBBERLIST \
	"memory", "hi", "lo", "$1", "$3", "$8", "$9", \
	"$10", "$11", "$12", "$13", "$14", "$15", \
	"$24", "$25"
# endif

static inline kernel_ulong_t
raw_syscall_0(const kernel_ulong_t nr, kernel_ulong_t *err)
{
	register kernel_ulong_t s0 __asm__("$16") = nr;
	register kernel_ulong_t v0 __asm__("$2");
	register kernel_ulong_t a3 __asm__("$7");
	__asm__ __volatile__(".set noreorder\n\t"
			     "move %0, %2\n\t"
			     "syscall\n\t"
			     ".set reorder"
			     : "=r"(v0), "=r"(a3)
			     : "r"(s0)
			     : SYSCALL_CLOBBERLIST);
	*err = a3;
	return v0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
