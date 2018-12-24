/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	unsigned long addr, sp;

	/* Fetch pointer to struct sigcontext.  */
	if (!get_stack_pointer(tcp, &sp) ||
	    umove_or_printaddr(tcp, sp + 2 * sizeof(int), &addr))
		return;

	unsigned long mask[NSIG_BYTES / sizeof(long)];
	/* Fetch first word of signal mask.  */
	if (umove_or_printaddr(tcp, addr, &mask[0]))
		return;

	/* Fetch remaining words of signal mask, located immediately before.  */
	addr -= sizeof(mask) - sizeof(long);
	if (umoven_or_printaddr(tcp, addr, sizeof(mask) - sizeof(long), &mask[1]))
		return;

	tprintsigmask_addr("{mask=", mask);
	tprints("}");
}
