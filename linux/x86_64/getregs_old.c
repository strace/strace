/*
 * Copyright (c) 2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * PTRACE_GETREGSET was added to the kernel in v2.6.25,
 * a PTRACE_GETREGS based fallback is provided for old kernels.
 */
static int
getregs_old(struct tcb *tcp)
{
	/* Use old method, with unreliable heuristical detection of 32-bitness. */
	long r = ptrace(PTRACE_GETREGS, tcp->pid, NULL, &x86_64_regs);
	if (r)
		return r;

	if (x86_64_regs.cs == 0x23) {
		x86_io.iov_len = sizeof(i386_regs);
		/*
		 * The order is important: i386_regs and x86_64_regs
		 * are overlaid in memory!
		 */
		i386_regs.ebx = x86_64_regs.rbx;
		i386_regs.ecx = x86_64_regs.rcx;
		i386_regs.edx = x86_64_regs.rdx;
		i386_regs.esi = x86_64_regs.rsi;
		i386_regs.edi = x86_64_regs.rdi;
		i386_regs.ebp = x86_64_regs.rbp;
		i386_regs.eax = x86_64_regs.rax;
		/* i386_regs.xds = x86_64_regs.ds; unused by strace */
		/* i386_regs.xes = x86_64_regs.es; ditto... */
		/* i386_regs.xfs = x86_64_regs.fs; */
		/* i386_regs.xgs = x86_64_regs.gs; */
		i386_regs.orig_eax = x86_64_regs.orig_rax;
		i386_regs.eip = x86_64_regs.rip;
		/* i386_regs.xcs = x86_64_regs.cs; */
		/* i386_regs.eflags = x86_64_regs.eflags; */
		i386_regs.esp = x86_64_regs.rsp;
		/* i386_regs.xss = x86_64_regs.ss; */
	} else {
		x86_io.iov_len = sizeof(x86_64_regs);
	}
	return 0;
}
