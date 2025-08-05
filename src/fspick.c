/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/fspick_flags.h"

SYS_FUNC(fspick)
{
	/* dirfd */
	tprints_arg_name("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[1]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(fspick_flags, tcp->u_arg[2], "FSPICK_???");

	return RVAL_DECODED | RVAL_FD;
}
