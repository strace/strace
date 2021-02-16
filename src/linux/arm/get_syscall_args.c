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
	tcp->u_arg[0] = arm_regs.uregs[0];
	tcp->u_arg[1] = arm_regs.uregs[1];
	tcp->u_arg[2] = arm_regs.uregs[2];
	tcp->u_arg[3] = arm_regs.uregs[3];
	tcp->u_arg[4] = arm_regs.uregs[4];
	tcp->u_arg[5] = arm_regs.uregs[5];
	return 1;
}
