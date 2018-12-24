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

	/* Skip dummy stack frame. */
	addr += 64;

#ifdef POWERPC64
	/* The only sigreturn on ppc64 is compat_sys_sigreturn. */
	typedef struct {
		unsigned int _unused[4];
		int signal;
		unsigned int handler;
		unsigned int oldmask;
		/* all the rest is irrelevant */
	} sigreturn_context;
#else
	typedef struct sigcontext sigreturn_context;
#endif

	sigreturn_context sc;

	if (!umove_or_printaddr(tcp, addr, &sc)) {
		const unsigned int mask[NSIG_BYTES / sizeof(int)] = {
			sc.oldmask,
			sc._unused[3]
		};

		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
