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
	tcp->u_arg[0] = csky_regs.orig_a0;
	tcp->u_arg[1] = csky_regs.a1;
	tcp->u_arg[2] = csky_regs.a2;
	tcp->u_arg[3] = csky_regs.a3;
	tcp->u_arg[4] = csky_regs.regs[0];
	tcp->u_arg[5] = csky_regs.regs[1];
	return 1;
}
