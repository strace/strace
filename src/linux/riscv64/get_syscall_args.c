/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = riscv_regs.a0;
	tcp->u_arg[1] = riscv_regs.a1;
	tcp->u_arg[2] = riscv_regs.a2;
	tcp->u_arg[3] = riscv_regs.a3;
	tcp->u_arg[4] = riscv_regs.a4;
	tcp->u_arg[5] = riscv_regs.a5;
	return 1;
}
