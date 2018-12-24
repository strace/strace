/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_get_syscall_args arm_get_syscall_args
#include "arm/get_syscall_args.c"
#undef arch_get_syscall_args

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		return arm_get_syscall_args(tcp);
	tcp->u_arg[0] = aarch64_regs.regs[0];
	tcp->u_arg[1] = aarch64_regs.regs[1];
	tcp->u_arg[2] = aarch64_regs.regs[2];
	tcp->u_arg[3] = aarch64_regs.regs[3];
	tcp->u_arg[4] = aarch64_regs.regs[4];
	tcp->u_arg[5] = aarch64_regs.regs[5];
	return 1;
}
