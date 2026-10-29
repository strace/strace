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
	/*
	 * The kernel reads the first syscall argument from orig_a0,
	 * which is not part of user_regs_struct and cannot be set
	 * via ptrace regset.
	 */
	if (arg_mask & 1)
		return -1;
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if (!(arg_mask & (1U << i)))
			continue;
		switch (i) {
		case 0: riscv_regs.a0 = args[i]; break;
		case 1: riscv_regs.a1 = args[i]; break;
		case 2: riscv_regs.a2 = args[i]; break;
		case 3: riscv_regs.a3 = args[i]; break;
		case 4: riscv_regs.a4 = args[i]; break;
		case 5: riscv_regs.a5 = args[i]; break;
		}
	}
	return set_regs(tcp->pid);
}
