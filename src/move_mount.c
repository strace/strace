/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/move_mount_flags.h"

SYS_FUNC(move_mount)
{
	/* from_dfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* from_pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* to_dfd */
	print_dirfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* to_pathname */
	printpath(tcp, tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	printflags(move_mount_flags, tcp->u_arg[4], "MOVE_MOUNT_???");
	return RVAL_DECODED;
}
