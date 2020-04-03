/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(sh_regs.regs[0])) {
		tcp->u_rval = -1;
		tcp->u_error = -sh_regs.regs[0];
	} else {
		tcp->u_rval = sh_regs.regs[0];
	}
}
