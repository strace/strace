/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"

#include "xlat/memfd_secret_flags.h"

SYS_FUNC(memfd_secret)
{
	unsigned int flags = tcp->u_arg[0];

	/* flags */
	tprints_arg_name("flags");
	printflags(memfd_secret_flags, flags, "");

	return RVAL_DECODED | RVAL_FD;
}
