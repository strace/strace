/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
	if (tcp->currpers == 1) {
		/*
		 * Zero-extend from 32 bits.
		 * Use truncate_klong_to_current_wordsize(tcp->u_arg[N])
		 * in syscall handlers
		 * if you need to use *sign-extended* parameter.
		 */
		tcp->u_arg[0] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 0];
		tcp->u_arg[1] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 1];
		tcp->u_arg[2] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 2];
		tcp->u_arg[3] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 3];
		tcp->u_arg[4] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 4];
		tcp->u_arg[5] = (uint32_t) sparc_regs.u_regs[U_REG_O0 + 5];
	} else {
		tcp->u_arg[0] = sparc_regs.u_regs[U_REG_O0 + 0];
		tcp->u_arg[1] = sparc_regs.u_regs[U_REG_O0 + 1];
		tcp->u_arg[2] = sparc_regs.u_regs[U_REG_O0 + 2];
		tcp->u_arg[3] = sparc_regs.u_regs[U_REG_O0 + 3];
		tcp->u_arg[4] = sparc_regs.u_regs[U_REG_O0 + 4];
		tcp->u_arg[5] = sparc_regs.u_regs[U_REG_O0 + 5];
	}

	return 1;
}
