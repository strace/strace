/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static void
decode_renameat(struct tcb *tcp)
{
	/* olddirfd */
	print_dirfd(tcp, tcp->u_arg[0]);

	/* oldpath */
	tprint_arg_next();
	printpath(tcp, tcp->u_arg[1]);

	/* newdirfd */
	tprint_arg_next();
	print_dirfd(tcp, tcp->u_arg[2]);

	/* newpath */
	tprint_arg_next();
	printpath(tcp, tcp->u_arg[3]);
}

SYS_FUNC(renameat)
{
	decode_renameat(tcp);

	return RVAL_DECODED;
}

#include <linux/fs.h>
#include "xlat/rename_flags.h"

SYS_FUNC(renameat2)
{
	decode_renameat(tcp);

	/* flags */
	tprint_arg_next();
	printflags(rename_flags, tcp->u_arg[4], "RENAME_??");

	return RVAL_DECODED;
}
