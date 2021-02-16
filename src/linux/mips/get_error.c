/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_get_error(struct tcb *tcp, const bool check_errno)
{
	if (mips_REG_A3) {
		tcp->u_rval = -1;
		tcp->u_error = mips_REG_V0;
	} else {
		tcp->u_rval = mips_REG_V0;
	}
}
