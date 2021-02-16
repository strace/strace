/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

#define arch_get_error arm_get_error
#include "../arm/get_error.c"
#undef arch_get_error

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (tcp->currpers == 1) {
		arm_get_error(tcp, check_errno);
		return;
	}

	if (check_errno && is_negated_errno(aarch64_regs.regs[0])) {
		tcp->u_rval = -1;
		tcp->u_error = -aarch64_regs.regs[0];
	} else {
		tcp->u_rval = aarch64_regs.regs[0];
	}
}
