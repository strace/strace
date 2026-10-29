/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		      kernel_ulong_t args[])
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if (arg_mask & (1U << i))
			or1k_regs.gpr[3 + i] = args[i];
	}
	return set_regs(tcp->pid);
}
