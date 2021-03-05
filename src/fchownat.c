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
	tprint_arg_next();

	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* owner */
	printuid(tcp->u_arg[2]);
	tprint_arg_next();

	/* group */
	printuid(tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}
