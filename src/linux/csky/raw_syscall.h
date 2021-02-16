/*
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
# if defined(__CSKYABIV2__)
	register kernel_ulong_t scno __asm__("r7") = nr;
# else
	register kernel_ulong_t scno __asm__("r1") = nr;
# endif
	register kernel_ulong_t a0 __asm__("a0");
	asm volatile("trap 0"
		: "+r"(scno), "=r"(a0)
		:
		: "memory");

	*err = 0;
	return a0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
