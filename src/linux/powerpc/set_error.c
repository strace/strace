/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_r3_ccr(struct tcb *tcp, const unsigned long r3,
		const unsigned long ccr_set, const unsigned long ccr_clear)
{
	ppc_regs.gpr[3] = r3;
	ppc_regs.ccr |= ccr_set;
	ppc_regs.ccr &= ~ccr_clear;
	return set_regs(tcp->pid);
}

static int
arch_set_error(struct tcb *tcp)
{
	return arch_set_r3_ccr(tcp, tcp->u_error, 0x10000000, 0);
}

static int
arch_set_success(struct tcb *tcp)
{
	return arch_set_r3_ccr(tcp, tcp->u_rval, 0, 0x10000000);
}
