/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

long
getrval2(struct tcb *tcp)
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	return ia64_regs.gr[9];
}
