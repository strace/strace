/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(fchownat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprint_arg_next();
	printpath(tcp, tcp->u_arg[1]);

	/* owner */
	tprint_arg_next();
	printuid(tcp->u_arg[2]);

	/* group */
	tprint_arg_next();
	printuid(tcp->u_arg[3]);

	/* flags */
	tprint_arg_next();
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}
