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
	if (check_errno && is_negated_errno(xtensa_regs.a[2])) {
		tcp->u_rval = -1;
		tcp->u_error = -xtensa_regs.a[2];
	} else {
		tcp->u_rval = xtensa_regs.a[2];
	}
}
