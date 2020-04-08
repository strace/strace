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
	tcp->u_arg[0] = m68k_regs.d1;
	tcp->u_arg[1] = m68k_regs.d2;
	tcp->u_arg[2] = m68k_regs.d3;
	tcp->u_arg[3] = m68k_regs.d4;
	tcp->u_arg[4] = m68k_regs.d5;
	tcp->u_arg[5] = m68k_regs.a0;
	return 1;
}
