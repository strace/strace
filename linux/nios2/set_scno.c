/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	nios2_regs.regs[2] = scno;
	return set_regs(tcp->pid);
}
