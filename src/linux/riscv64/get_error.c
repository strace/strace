/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(riscv_regs.a0)) {
		tcp->u_rval = -1;
		tcp->u_error = -riscv_regs.a0;
	} else {
		tcp->u_rval = riscv_regs.a0;
	}
}
