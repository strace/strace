/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static void
decode_chmod(struct tcb *tcp, const int offset)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[offset]);
	tprint_arg_next();

	/* mode */
	print_numeric_umode_t(tcp->u_arg[offset + 1]);
}

SYS_FUNC(chmod)
{
	decode_chmod(tcp, 0);

	return RVAL_DECODED;
}

SYS_FUNC(fchmodat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	decode_chmod(tcp, 1);

	return RVAL_DECODED;
}

SYS_FUNC(fchmod)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* mode */
	print_numeric_umode_t(tcp->u_arg[1]);

	return RVAL_DECODED;
}
