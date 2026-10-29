/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

static int
arch_set_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		      kernel_ulong_t args[])
{
	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;
	/* The kernel reads the first syscall argument from orig_gpr2 */
	if (arg_mask & 1)
		ARCH_REGSET.orig_gpr2 = args[0];
	for (unsigned int i = 1; i < MAX_ARGS; ++i) {
		if (arg_mask & (1U << i))
			ARCH_REGSET.gprs[2 + i] = args[i];
	}
	return set_regs(tcp->pid);
}
