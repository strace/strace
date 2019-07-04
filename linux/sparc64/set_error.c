/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	sparc_regs.tstate |= 0x1100000000UL;
	sparc_regs.u_regs[U_REG_O0] = tcp->u_error;
	return set_regs(tcp->pid);
}

static int
arch_set_success(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	sparc_regs.tstate &= ~0x1100000000UL;
	sparc_regs.u_regs[U_REG_O0] = tcp->u_rval;
	return set_regs(tcp->pid);
}
