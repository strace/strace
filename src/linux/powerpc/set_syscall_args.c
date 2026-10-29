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
	/* The kernel restores gpr[3] from orig_gpr3 on syscall restart */
	if (arg_mask & 1) {
		ppc_regs.orig_gpr3 = args[0];
		ppc_regs.gpr[3] = args[0];
	}
	for (unsigned int i = 1; i < MAX_ARGS; ++i) {
		if (arg_mask & (1U << i))
			ppc_regs.gpr[3 + i] = args[i];
	}
	return set_regs(tcp->pid);
}
