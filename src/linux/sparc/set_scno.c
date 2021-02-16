/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Reloading the syscall number from %g1 register is supported
 * by linux kernel starting with commit v4.5-rc7~35^2~3.
 */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	sparc_regs.u_regs[U_REG_G1] = scno;
	return set_regs(tcp->pid);
}
