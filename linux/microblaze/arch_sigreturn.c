/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	/* TODO: Verify that this is correct...  */

	unsigned long addr;

	/* Read r1, the stack pointer.  */
	if (upeek(tcp, 1 * 4, &addr) < 0)
		return;
	addr += offsetof(struct sigcontext, oldmask);

	tprints("{mask=");
	print_sigset_addr(tcp, addr);
	tprints("}");
}
