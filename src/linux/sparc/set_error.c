/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
sparc_set_o0_psr(struct tcb *tcp, const unsigned long o0,
		 const unsigned long psr_set, const unsigned long psr_clear)
{
	sparc_regs.u_regs[U_REG_O0] = o0;
	sparc_regs.psr |= psr_set;
	sparc_regs.psr &= ~psr_clear;
	return set_regs(tcp->pid);
}

static int
arch_set_error(struct tcb *tcp)
{
	return sparc_set_o0_psr(tcp, tcp->u_error, PSR_C, 0);
}

static int
arch_set_success(struct tcb *tcp)
{
	return sparc_set_o0_psr(tcp, tcp->u_rval, 0, PSR_C);
}
