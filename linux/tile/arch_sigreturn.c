/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	kernel_ulong_t addr;
	if (!get_stack_pointer(tcp, &addr))
		return;

	/* offset of ucontext in the kernel's sigframe structure */
#define SIGFRAME_UC_OFFSET C_ABI_SAVE_AREA_SIZE + sizeof(siginfo_t)
	addr += SIGFRAME_UC_OFFSET + offsetof(ucontext_t, uc_sigmask);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
