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
	tcp->u_arg[0] = nios2_regs.regs[4];
	tcp->u_arg[1] = nios2_regs.regs[5];
	tcp->u_arg[2] = nios2_regs.regs[6];
	tcp->u_arg[3] = nios2_regs.regs[7];
	tcp->u_arg[4] = nios2_regs.regs[8];
	tcp->u_arg[5] = nios2_regs.regs[9];
	return 1;
}
