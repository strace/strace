/*
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include "xlat/access_flags.h"

static int
decode_access(struct tcb *tcp, int offset)
{
	printpath(tcp, tcp->u_arg[offset]);
	tprints(", ");
	printflags(access_flags, tcp->u_arg[offset + 1], "?_OK");

	return RVAL_DECODED;
}

SYS_FUNC(access)
{
	return decode_access(tcp, 0);
}

SYS_FUNC(faccessat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	return decode_access(tcp, 1);
}
