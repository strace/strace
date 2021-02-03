/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (ppc_regs.ccr & 0x10000000) {
		tcp->u_rval = -1;
		tcp->u_error = ppc_regs.gpr[3];
	} else {
		tcp->u_rval = ppc_regs.gpr[3];
	}
}
