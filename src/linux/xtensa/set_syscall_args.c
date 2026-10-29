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
	static const unsigned int syscall_regs[MAX_ARGS] = { 6, 3, 4, 5, 8, 9 };
	static unsigned int naregs_mask;

	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;

	if (naregs_mask == 0)
		naregs_mask = xtensa_probe_naregs(tcp) - 1;

	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if (arg_mask & (1U << i))
			xtensa_regs.a[(xtensa_regs.windowbase * 4 +
				       syscall_regs[i]) & naregs_mask] = args[i];
	}
	return set_regs(tcp->pid);
}
