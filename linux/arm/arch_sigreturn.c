/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	kernel_ulong_t sp;
	if (!get_stack_pointer(tcp, &sp))
		return;

#define SIZEOF_STRUCT_SIGINFO 128
#define SIZEOF_STRUCT_SIGCONTEXT (21 * 4)
#define OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK (5 * 4 + SIZEOF_STRUCT_SIGCONTEXT)

	const kernel_ulong_t addr =
#ifdef AARCH64
		tcp->currpers == 0 ?
			(sp + SIZEOF_STRUCT_SIGINFO +
			 offsetof(ucontext_t, uc_sigmask)) :
#endif
			(sp + OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK);
	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
