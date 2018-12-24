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
	if (current_personality != 0) {
		/*
		 * Zero-extend from 32 bits.
		 * Use truncate_klong_to_current_wordsize(tcp->u_arg[N])
		 * in syscall handlers
		 * if you need to use *sign-extended* parameter.
		 */
		tcp->u_arg[0] = (uint32_t) ppc_regs.orig_gpr3;
		tcp->u_arg[1] = (uint32_t) ppc_regs.gpr[4];
		tcp->u_arg[2] = (uint32_t) ppc_regs.gpr[5];
		tcp->u_arg[3] = (uint32_t) ppc_regs.gpr[6];
		tcp->u_arg[4] = (uint32_t) ppc_regs.gpr[7];
		tcp->u_arg[5] = (uint32_t) ppc_regs.gpr[8];
	} else {
		tcp->u_arg[0] = ppc_regs.orig_gpr3;
		tcp->u_arg[1] = ppc_regs.gpr[4];
		tcp->u_arg[2] = ppc_regs.gpr[5];
		tcp->u_arg[3] = ppc_regs.gpr[6];
		tcp->u_arg[4] = ppc_regs.gpr[7];
		tcp->u_arg[5] = ppc_regs.gpr[8];
	}
	return 1;
}
