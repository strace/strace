/*
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	/* v0 is a reliable source of the syscall number only on entering.  */
	if (ptrace_get_syscall_info_supported && !ptrace_syscall_info_is_entry())
		return 0;

	tcp->scno = mips_REG_V0;

	if (!scno_in_range(tcp->scno)) {
		if (mips_REG_A3 == 0 || mips_REG_A3 == (uint64_t) -1) {
			debug_msg("stray syscall exit: v0 = %#llx",
				  (unsigned long long) tcp->scno);
			return 0;
		}
	}

	return 1;
}
