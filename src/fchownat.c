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
	print_syscall_param("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname */
	print_syscall_param("pathname");
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* owner */
	print_syscall_param("owner");
	printuid(tcp->u_arg[2]);
	tprint_arg_next();

	/* group */
	print_syscall_param("group");
	printuid(tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	print_syscall_param("flags");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}
