/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
sparc64_set_o0_tstate(struct tcb *tcp, const unsigned long o0,
		      const unsigned long tstate_set,
		      const unsigned long tstate_clear)
{
	sparc_regs.u_regs[U_REG_O0] = o0;
	sparc_regs.tstate |= tstate_set;
	sparc_regs.tstate &= ~tstate_clear;
	return set_regs(tcp->pid);
}

static int
arch_set_error(struct tcb *tcp)
{
	return sparc64_set_o0_tstate(tcp, tcp->u_error, 0x1100000000UL, 0);
}

static int
arch_set_success(struct tcb *tcp)
{
	return sparc64_set_o0_tstate(tcp, tcp->u_rval, 0, 0x1100000000UL);
}
