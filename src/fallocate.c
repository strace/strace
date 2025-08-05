/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/falloc.h>
#include "xlat/falloc_flags.h"

SYS_FUNC(fallocate)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* mode */
	tprints_arg_next_name("mode");
	printflags(falloc_flags, tcp->u_arg[1], "FALLOC_FL_???");

	/* offset */
	tprints_arg_next_name("offset");
	unsigned int argn = print_arg_lld(tcp, 2);

	/* len */
	tprints_arg_next_name("len");
	print_arg_lld(tcp, argn);

	return RVAL_DECODED;
}
