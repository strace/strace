/*
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
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
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[1]);

	/* mask */
	tprints_arg_next_name("mask");
	printflags(inotify_flags, tcp->u_arg[2], "IN_???");

	return RVAL_DECODED;
}

SYS_FUNC(inotify_rm_watch)
{
	/* file descriptor */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* watch descriptor */
	tprints_arg_next_name("wd");
	PRINT_VAL_D((int) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(inotify_init)
{
	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(inotify_init1)
{
	tprints_arg_name("flags");
	printflags(inotify_init_flags, tcp->u_arg[0], "IN_???");

	return RVAL_DECODED | RVAL_FD;
}
