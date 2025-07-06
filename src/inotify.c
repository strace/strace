/*
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"

#include "xlat/inotify_flags.h"
#include "xlat/inotify_init_flags.h"

SYS_FUNC(inotify_add_watch)
{
	/* file descriptor */
	print_syscall_param("fd");
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname */
	print_syscall_param("pathname");
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* mask */
	print_syscall_param("mask");
	printflags(inotify_flags, tcp->u_arg[2], "IN_???");

	return RVAL_DECODED;
}

SYS_FUNC(inotify_rm_watch)
{
	/* file descriptor */
	print_syscall_param("fd");
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* watch descriptor */
	print_syscall_param("wd");
	PRINT_VAL_D((int) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(inotify_init)
{
	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(inotify_init1)
{
	print_syscall_param("flags");
	printflags(inotify_init_flags, tcp->u_arg[0], "IN_???");

	return RVAL_DECODED | RVAL_FD;
}
