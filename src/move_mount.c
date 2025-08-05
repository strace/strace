/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
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
	tprints_arg_name("from_dfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* from_pathname */
	tprints_arg_next_name("from_pathname");
	printpath(tcp, tcp->u_arg[1]);

	/* to_dfd */
	tprints_arg_next_name("to_dfd");
	print_dirfd(tcp, tcp->u_arg[2]);

	/* to_pathname */
	tprints_arg_next_name("to_pathname");
	printpath(tcp, tcp->u_arg[3]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(move_mount_flags, tcp->u_arg[4], "MOVE_MOUNT_???");
	return RVAL_DECODED;
}
