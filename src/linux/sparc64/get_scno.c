/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	/* Retrieve the syscall trap instruction. */
	unsigned long trap;
	errno = 0;
	trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (void *) sparc_regs.tpc, 0);
	if (errno == 0) {
		trap >>= 32;
		switch (trap) {
		case 0x91d02010:
			/* Linux/SPARC syscall trap. */
			update_personality(tcp, 1);
			break;
		case 0x91d0206d:
			/* Linux/SPARC64 syscall trap. */
			update_personality(tcp, 0);
			break;
		}
	}

	tcp->scno = sparc_regs.u_regs[U_REG_G1];
	return 1;
}
