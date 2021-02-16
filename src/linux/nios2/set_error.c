/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	nios2_regs.regs[7] = 1;
	nios2_regs.regs[2] = -tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	nios2_regs.regs[7] = 0;
	nios2_regs.regs[2] = tcp->u_rval;
	return set_regs(tcp->pid);
}
