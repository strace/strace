/*
 * Copyright (c) 2010-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_check_scno(struct tcb *tcp)
{

	const kernel_ulong_t scno = ptrace_sci.entry.nr;

	if (tcp->currpers == 0 && !(scno & __X32_SYSCALL_BIT)) {
		error_msg("syscall_%" PRI_klu "(...) in unsupported "
			  "64-bit mode of process PID=%d", scno, tcp->pid);
		return 0;
	}

	return 1;
}
