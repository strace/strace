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
	print_syscall_param("olddirfd");
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* oldpath */
	print_syscall_param("oldpath");
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* newdirfd */
	print_syscall_param("newdirfd");
	print_dirfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* newpath */
	print_syscall_param("newpath");
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
	tprint_arg_next();

	/* flags */
	print_syscall_param("flags");
	printflags(rename_flags, tcp->u_arg[4], "RENAME_??");

	return RVAL_DECODED;
}
