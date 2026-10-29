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
	/* uregs[17] is ARM_ORIG_r0, used by the kernel for syscall restart */
	if (arg_mask & 1) {
		arm_regs.uregs[0] = args[0];
		arm_regs.uregs[17] = args[0];
	}
	for (unsigned int i = 1; i < MAX_ARGS; ++i) {
		if (arg_mask & (1U << i))
			arm_regs.uregs[i] = args[i];
	}
	return set_regs(tcp->pid);
}
