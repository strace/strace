/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(or1k_regs.gpr[11])) {
		tcp->u_rval = -1;
		tcp->u_error = -or1k_regs.gpr[11];
	} else {
		tcp->u_rval = or1k_regs.gpr[11];
	}
}
