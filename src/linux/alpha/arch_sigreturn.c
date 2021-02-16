/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr;

	if (!get_stack_pointer(tcp, &addr))
		return;
	addr += offsetof(struct sigcontext, sc_mask);

	print_sigset_addr(tcp, addr);
}
