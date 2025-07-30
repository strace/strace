/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
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
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprint_arg_next();
	printpath(tcp, tcp->u_arg[1]);

	/* flags */
	tprint_arg_next();
	printflags(fspick_flags, tcp->u_arg[2], "FSPICK_???");

	return RVAL_DECODED | RVAL_FD;
}
