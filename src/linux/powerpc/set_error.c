/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	if (PPC_TRAP_IS_SCV(ppc_regs.trap)) {
		ppc_regs.gpr[3] = -tcp->u_error;
	} else {
		ppc_regs.gpr[3] = tcp->u_error;
		ppc_regs.ccr |= 0x10000000;
	}
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	ppc_regs.gpr[3] = tcp->u_rval;
	if (!PPC_TRAP_IS_SCV(ppc_regs.trap))
		ppc_regs.ccr &= ~0x10000000;
	return set_regs(tcp->pid);
}
