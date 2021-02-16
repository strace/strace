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
	if (check_errno && is_negated_errno(i386_regs.eax)) {
		tcp->u_rval = -1;
		tcp->u_error = -i386_regs.eax;
	} else {
		tcp->u_rval = i386_regs.eax;
	}
}
