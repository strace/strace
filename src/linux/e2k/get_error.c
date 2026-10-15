/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(e2k_regs.sys_rval)) {
		tcp->u_rval = -1;
		tcp->u_error = -e2k_regs.sys_rval;
	} else {
		tcp->u_rval = e2k_regs.sys_rval;
	}
}
