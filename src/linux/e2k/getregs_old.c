/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
getregs_old(struct tcb *tcp)
{
	ARCH_REGS_FOR_GETREGS.sizeof_struct = sizeof (struct user_regs_struct);
	return ptrace(PTRACE_GETREGS, tcp->pid, NULL, &ARCH_REGS_FOR_GETREGS);
}

static int set_regs(pid_t pid)
{
	ARCH_REGS_FOR_GETREGS.sizeof_struct = sizeof (struct user_regs_struct);
	return ptrace(PTRACE_SETREGS, pid, NULL, &ARCH_REGS_FOR_GETREGS);
}
