/*
 * Copyright (c) 2015-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	unsigned int status_reg = xtensa_regs.windowbase * 4 + 2;

	if (check_errno && is_negated_errno(xtensa_regs.a[status_reg])) {
		tcp->u_rval = -1;
		tcp->u_error = -xtensa_regs.a[status_reg];
	} else {
		tcp->u_rval = xtensa_regs.a[status_reg];
	}
}
