/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include "xlat/access_modes.h"
#include "xlat/faccessat_flags.h"

static void
decode_access(struct tcb *tcp, int offset)
{
	printpath(tcp, tcp->u_arg[offset]);
	tprints(", ");
	printflags(access_modes, tcp->u_arg[offset + 1], "?_OK");
}

SYS_FUNC(access)
{
	decode_access(tcp, 0);
	return RVAL_DECODED;
}

static void
decode_faccessat(struct tcb *tcp)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	decode_access(tcp, 1);
}


SYS_FUNC(faccessat)
{
	decode_faccessat(tcp);
	return RVAL_DECODED;
}

SYS_FUNC(faccessat2)
{
	decode_faccessat(tcp);
	tprints(", ");
	printflags(faccessat_flags, tcp->u_arg[3], "AT_???");
	return RVAL_DECODED;
}
