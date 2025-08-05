/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>
#include "xlat/fchmodat_flags.h"

static void
decode_chmod(struct tcb *tcp, const int offset)
{
	/* pathname */
	tprints_arg_name("pathname");
	printpath(tcp, tcp->u_arg[offset]);

	/* mode */
	tprints_arg_next_name("mode");
	print_numeric_umode_t(tcp->u_arg[offset + 1]);
}

SYS_FUNC(chmod)
{
	decode_chmod(tcp, 0);

	return RVAL_DECODED;
}

static void
decode_fchmodat(struct tcb *tcp)
{
	/* dirfd */
	tprints_arg_name("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	tprint_arg_next();
	decode_chmod(tcp, 1);
}

SYS_FUNC(fchmodat)
{
	decode_fchmodat(tcp);

	return RVAL_DECODED;
}

SYS_FUNC(fchmodat2)
{
	decode_fchmodat(tcp);

	tprints_arg_next_name("flags");
	printflags(fchmodat_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(fchmod)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* mode */
	tprints_arg_next_name("mode");
	print_numeric_umode_t(tcp->u_arg[1]);

	return RVAL_DECODED;
}
