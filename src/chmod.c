/*
 * Copyright (c) 2014-2023 The strace developers.
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
	print_syscall_param("pathname");
	printpath(tcp, tcp->u_arg[offset]);
	tprint_arg_next();

	/* mode */
	print_syscall_param("mode");
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
	print_syscall_param("dirfd");
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
	tprint_arg_next();

	print_syscall_param("flags");
	printflags(fchmodat_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(fchmod)
{
	/* fd */
	print_syscall_param("fd");
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* mode */
	print_syscall_param("mode");
	print_numeric_umode_t(tcp->u_arg[1]);

	return RVAL_DECODED;
}
