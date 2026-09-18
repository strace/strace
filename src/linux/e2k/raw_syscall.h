/*
 * Raw syscalls.
 *
 * Copyright (c) 2026 The strace developers.
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
	kernel_ulong_t _res;
	__asm__ __volatile__("{\n"
			"sdisp %%ctpr1, 0x3\n"
			"addd, s 0, %[sys_num], %%b[0]\n"
			"}\n"
			"{\n"
			"call %%ctpr1, wbs = %#\n"
			"}\n"
			"{\n"
			"addd, s 0, %%b[0], %[res]\n"
			"}"
			: [res] "=r" (_res)
			: [sys_num] "r" (nr)
	);
	return _res;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
