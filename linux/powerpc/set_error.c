/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() &&
	    upeek(tcp, sizeof(long) * PT_CCR, &ppc_regs.ccr))
                return -1;
	ppc_regs.gpr[3] = tcp->u_error;
	ppc_regs.ccr |= 0x10000000;
	return upoke(tcp, sizeof(long) * PT_CCR, ppc_regs.ccr) ||
	       upoke(tcp, sizeof(long) * (PT_R0 + 3), ppc_regs.gpr[3]);
}

static int
arch_set_success(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() &&
	    upeek(tcp, sizeof(long) * PT_CCR, &ppc_regs.ccr))
                return -1;
	ppc_regs.gpr[3] = tcp->u_rval;
	ppc_regs.ccr &= ~0x10000000;
	return upoke(tcp, sizeof(long) * PT_CCR, ppc_regs.ccr) ||
	       upoke(tcp, sizeof(long) * (PT_R0 + 3), ppc_regs.gpr[3]);
}
