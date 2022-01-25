/*
 * Copyright (c) 2021-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	tcp->u_arg[0] = loongarch_regs.orig_a0;
	tcp->u_arg[1] = loongarch_regs.regs[5];
	tcp->u_arg[2] = loongarch_regs.regs[6];
	tcp->u_arg[3] = loongarch_regs.regs[7];
	tcp->u_arg[4] = loongarch_regs.regs[8];
	tcp->u_arg[5] = loongarch_regs.regs[9];
	return 1;
}
