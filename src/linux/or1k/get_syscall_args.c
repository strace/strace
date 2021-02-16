/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = or1k_regs.gpr[3 + 0];
	tcp->u_arg[1] = or1k_regs.gpr[3 + 1];
	tcp->u_arg[2] = or1k_regs.gpr[3 + 2];
	tcp->u_arg[3] = or1k_regs.gpr[3 + 3];
	tcp->u_arg[4] = or1k_regs.gpr[3 + 4];
	tcp->u_arg[5] = or1k_regs.gpr[3 + 5];
	return 1;
}
