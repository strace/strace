/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_r3_ccr(struct tcb *tcp, const unsigned long r3,
		const unsigned long ccr_set, const unsigned long ccr_clear)
{
	if (ptrace_syscall_info_is_valid() &&
	    upeek(tcp, sizeof(long) * PT_CCR, &ppc_regs.ccr))
                return -1;
	const unsigned long old_ccr = ppc_regs.ccr;
	ppc_regs.gpr[3] = r3;
	ppc_regs.ccr |= ccr_set;
	ppc_regs.ccr &= ~ccr_clear;
	if (ppc_regs.ccr != old_ccr &&
	    upoke(tcp, sizeof(long) * PT_CCR, ppc_regs.ccr))
		return -1;
	return upoke(tcp, sizeof(long) * (PT_R0 + 3), ppc_regs.gpr[3]);
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
