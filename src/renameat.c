/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static void
decode_renameat(struct tcb *tcp)
{
	/* olddirfd */
	tprints_arg_name("olddirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* oldpath */
	tprints_arg_next_name("oldpath");
	printpath(tcp, tcp->u_arg[1]);

	/* newdirfd */
	tprints_arg_next_name("newdirfd");
	print_dirfd(tcp, tcp->u_arg[2]);

	/* newpath */
	tprints_arg_next_name("newpath");
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
	tprints_arg_next_name("flags");
	printflags(rename_flags, tcp->u_arg[4], "RENAME_??");

	return RVAL_DECODED;
}
