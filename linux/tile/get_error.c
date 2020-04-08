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
	/*
	 * The standard tile calling convention returns the value
	 * (or negative errno) in r0, and zero (or positive errno) in r1.
	 * Until at least kernel 3.8, however, the r1 value is not
	 * reflected in ptregs at this point, so we use r0 here.
	 */
	if (check_errno && is_negated_errno(tile_regs.regs[0])) {
		tcp->u_rval = -1;
		tcp->u_error = -tile_regs.regs[0];
	} else {
		tcp->u_rval = tile_regs.regs[0];
	}
}
