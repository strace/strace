/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/fsopen_flags.h"

SYS_FUNC(fsopen)
{
	/* fsname */
	tprints_arg_name("fsname");
	printstr(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(fsopen_flags, tcp->u_arg[1], "FSOPEN_???");
	return RVAL_DECODED | RVAL_FD;
}
