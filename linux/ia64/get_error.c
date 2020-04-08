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
	if (ia64_regs.gr[10]) {
		tcp->u_rval = -1;
		tcp->u_error = ia64_regs.gr[8];
	} else {
		tcp->u_rval = ia64_regs.gr[8];
	}
}
