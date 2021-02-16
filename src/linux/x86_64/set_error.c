/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	kernel_ulong_t	rval = -(long) tcp->u_error;

	if (tcp->currpers == 1)
		i386_regs.eax = rval;
	else
		x86_64_regs.rax = rval;

	return upoke(tcp, 8 * RAX, rval);
}

static int
arch_set_success(struct tcb *tcp)
{
	kernel_ulong_t  rval = (kernel_ulong_t) tcp->u_rval;

	if (tcp->currpers == 1)
		i386_regs.eax = rval;
	else
		x86_64_regs.rax = rval;

	return upoke(tcp, 8 * RAX, rval);
}
