/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (PPC_TRAP_IS_SCV(ppc_regs.trap)) {
		if (check_errno && is_negated_errno(ppc_regs.gpr[3])) {
			tcp->u_rval = -1;
			tcp->u_error = -ppc_regs.gpr[3];
		} else {
			tcp->u_rval = ppc_regs.gpr[3];
		}
	} else {
		if (ppc_regs.ccr & 0x10000000) {
			tcp->u_rval = -1;
			tcp->u_error = ppc_regs.gpr[3];
		} else {
			tcp->u_rval = ppc_regs.gpr[3];
		}
	}
}
