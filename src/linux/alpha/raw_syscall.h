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
	kernel_ulong_t sc_0 = nr;
	register kernel_ulong_t sc_19 __asm__("$19");
	__asm__ __volatile__("callsys"
			     : "+v"(sc_0), "=r"(sc_19)
			     :
			     : "memory", "$1", "$2", "$3", "$4", "$5", "$6",
			       "$7", "$8", "$16", "$17", "$18", "$20", "$21",
			       "$22", "$23", "$24", "$25", "$27", "$28");
	*err = sc_19;
	return sc_0;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
