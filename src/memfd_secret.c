/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
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
	printflags(memfd_secret_flags, flags, "");

	return RVAL_DECODED | RVAL_FD;
}
