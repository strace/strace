/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = i386_regs.ebx;
	tcp->u_arg[1] = i386_regs.ecx;
	tcp->u_arg[2] = i386_regs.edx;
	tcp->u_arg[3] = i386_regs.esi;
	tcp->u_arg[4] = i386_regs.edi;
	tcp->u_arg[5] = i386_regs.ebp;
	return 1;
}
