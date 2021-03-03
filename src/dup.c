/*
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static int
dup_123(struct tcb *const tcp, const int newfd_arg, const int flags_arg)
{
	/* oldfd */
	printfd(tcp, tcp->u_arg[0]);

	if (newfd_arg > 0) {
		tprint_arg_next();
		/* newfd */
		printfd(tcp, tcp->u_arg[newfd_arg]);

		if (flags_arg > 0) {
			tprint_arg_next();
			/* flags */
			printflags(open_mode_flags, tcp->u_arg[flags_arg],
				   "O_???");
		}
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(dup)
{
	return dup_123(tcp, -1, -1);
}

SYS_FUNC(dup2)
{
	return dup_123(tcp, 1, -1);
}

SYS_FUNC(dup3)
{
	return dup_123(tcp, 1, 2);
}
