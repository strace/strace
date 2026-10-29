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
		if (!(arg_mask & (1U << i)))
			continue;
		switch (i) {
		case 0: i386_regs.ebx = args[i]; break;
		case 1: i386_regs.ecx = args[i]; break;
		case 2: i386_regs.edx = args[i]; break;
		case 3: i386_regs.esi = args[i]; break;
		case 4: i386_regs.edi = args[i]; break;
		case 5: i386_regs.ebp = args[i]; break;
		}
	}
	return set_regs(tcp->pid);
}
