/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	/* 64-bit ABIs do not have old sigreturn. */
#ifdef LINUX_MIPSO32
	kernel_ulong_t addr;
	if (!get_stack_pointer(tcp, &addr))
		return;
	/*
	 * offsetof(struct sigframe, sf_mask) ==
	 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct sigcontext)
	 */
	addr += 6 * 4 + sizeof(struct sigcontext);

	print_sigset_addr(tcp, addr);
#endif
}
