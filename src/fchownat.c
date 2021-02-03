/*
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(fchownat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);
	printuid(", ", tcp->u_arg[2]);
	printuid(", ", tcp->u_arg[3]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}
