/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = e2k_regs.arg1;
	tcp->u_arg[1] = e2k_regs.arg2;
	tcp->u_arg[2] = e2k_regs.arg3;
	tcp->u_arg[3] = e2k_regs.arg4;
	tcp->u_arg[4] = e2k_regs.arg5;
	tcp->u_arg[5] = e2k_regs.arg6;
	return 1;
}
