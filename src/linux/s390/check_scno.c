/*
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_check_scno(struct tcb *tcp)
{
	if (ptrace_sci.entry.nr)
		return 1;

	/*
	 * Apparently, arch/s390/include/asm/syscall.h:syscall_get_nr()
	 * returns 0 for out-of-range syscall numbers.
	 * This kernel bug is exposed via PTRACE_GET_SYSCALL_INFO interface.
	 * Workaround it by falling back to get_regs().
	 */

	if (get_regs(tcp) < 0)
		return -1;

	arch_get_scno(tcp);
	if (tcp->scno) {
		ptrace_sci.entry.nr = tcp->scno;
		debug_func_msg("fixed scno: 0 -> %#lx", tcp->scno);
	}

	return 1;
}
