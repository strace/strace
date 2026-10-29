/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
poke_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		  kernel_ulong_t args[])
{
	static const unsigned int native_regs[MAX_ARGS] = {
		RDI, RSI, RDX, R10, R8, R9
	};
	static const unsigned int compat_regs[MAX_ARGS] = {
		RBX, RCX, RDX, RSI, RDI, RBP
	};
	const unsigned int *const regs =
		x86_io.iov_len == sizeof(i386_regs) ? compat_regs : native_regs;

	for (unsigned int i = 0; i < MAX_ARGS; ++i) {
		if ((arg_mask & (1U << i)) &&
		    upoke(tcp, 8 * regs[i], args[i]) < 0)
			return -1;
	}
	return 0;
}

static int
arch_set_syscall_args(struct tcb *tcp, unsigned int arg_mask,
		      kernel_ulong_t args[])
{
	/* 0: unknown; -1: SETREGSET works; 1: use POKEUSER. */
	static int use_pokeuser_for_syscall_args;

	if (ptrace_syscall_info_is_valid() && get_regs(tcp) < 0)
		return -1;

#ifdef HAVE_GETREGS_OLD
	/* getregs_old() has converted the register buffer for compat tracees. */
	if (use_getregs_old > 0)
		return poke_syscall_args(tcp, arg_mask, args);
#endif
	if (use_pokeuser_for_syscall_args > 0)
		return poke_syscall_args(tcp, arg_mask, args);

	if (x86_io.iov_len != sizeof(i386_regs)) {
		for (unsigned int i = 0; i < MAX_ARGS; ++i) {
			if (!(arg_mask & (1U << i)))
				continue;
			switch (i) {
			case 0: x86_64_regs.rdi = args[i]; break;
			case 1: x86_64_regs.rsi = args[i]; break;
			case 2: x86_64_regs.rdx = args[i]; break;
			case 3: x86_64_regs.r10 = args[i]; break;
			case 4: x86_64_regs.r8  = args[i]; break;
			case 5: x86_64_regs.r9  = args[i]; break;
			}
		}
	} else {
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
	}

	if (use_pokeuser_for_syscall_args < 0)
		return set_regs(tcp->pid);
	if (set_regs(tcp->pid) == 0) {
		use_pokeuser_for_syscall_args = -1;
		return 0;
	}
	if (errno != EIO && errno != EINVAL && errno != ENOSYS)
		return -1;
	use_pokeuser_for_syscall_args = 1;
	return poke_syscall_args(tcp, arg_mask, args);
}
