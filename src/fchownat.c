/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(fchownat)
{
	/* dirfd */
	tprints_arg_name("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[1]);

	/* owner */
	tprints_arg_next_name("owner");
	printuid(tcp->u_arg[2]);

	/* group */
	tprints_arg_next_name("group");
	printuid(tcp->u_arg[3]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}
