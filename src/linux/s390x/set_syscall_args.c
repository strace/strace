/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_set_syscall_args s390_set_syscall_args
#define ARCH_REGSET s390_regset
#include "../s390/set_syscall_args.c"
#undef ARCH_REGSET
#undef arch_set_syscall_args

#define arch_set_syscall_args s390x_set_syscall_args
#define ARCH_REGSET s390x_regset
#include "../s390/set_syscall_args.c"
#undef ARCH_REGSET
#undef arch_set_syscall_args

static int
arch_set_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		      kernel_ulong_t args[])
{
	if (tcp->currpers == 1)
		return s390_set_syscall_args(tcp, arg_mask, args);
	else
		return s390x_set_syscall_args(tcp, arg_mask, args);
}
