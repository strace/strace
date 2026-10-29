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
#if defined LINUX_MIPSN64 || defined LINUX_MIPSN32
	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if (!(arg_mask & (1U << i)))
			continue;
		switch (i) {
		case 0: mips_REG_A0 = args[i]; break;
		case 1: mips_REG_A1 = args[i]; break;
		case 2: mips_REG_A2 = args[i]; break;
		case 3: mips_REG_A3 = args[i]; break;
		case 4: mips_REG_A4 = args[i]; break;
		case 5: mips_REG_A5 = args[i]; break;
		}
	}
	return set_regs(tcp->pid);
#elif defined LINUX_MIPSO32
	unsigned int reg_mask = arg_mask & 0xf;
	if (reg_mask) {
		for (unsigned int i = 0; i < 4; ++i) {
			if (!(reg_mask & (1U << i)))
				continue;
			switch (i) {
			case 0: mips_REG_A0 = args[i]; break;
			case 1: mips_REG_A1 = args[i]; break;
			case 2: mips_REG_A2 = args[i]; break;
			case 3: mips_REG_A3 = args[i]; break;
			}
		}
		if (set_regs(tcp->pid) != 0)
			return -1;
	}
	for (unsigned int i = 4; i < MAX_ARGS; ++i) {
		if (!(arg_mask & (1U << i)))
			continue;
		if (!upoken(tcp,
			    mips_REG_SP + i * sizeof(kernel_ulong_t),
			    sizeof(args[i]), &args[i]))
			return -1;
	}
	return 0;
#else
# error unsupported mips abi
#endif
}
