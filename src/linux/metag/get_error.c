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
	/* result pointer in D0Re0 (D0.0) */
	if (check_errno && is_negated_errno(metag_regs.dx[0][0])) {
		tcp->u_rval = -1;
		tcp->u_error = -metag_regs.dx[0][0];
	} else {
		tcp->u_rval = metag_regs.dx[0][0];
	}
}
