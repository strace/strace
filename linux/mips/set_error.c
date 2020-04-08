/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	mips_REG_V0 = tcp->u_error;
	mips_REG_A3 = -1;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	mips_REG_V0 = tcp->u_rval;
	mips_REG_A3 = 0;
	return set_regs(tcp->pid);
}
