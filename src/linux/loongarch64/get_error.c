/*
 * Copyright (c) 2021-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "negated_errno.h"

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (check_errno && is_negated_errno(loongarch_regs.regs[4])) {
		tcp->u_rval = -1;
		tcp->u_error = -loongarch_regs.regs[4];
	} else {
		tcp->u_rval = loongarch_regs.regs[4];
	}
}
