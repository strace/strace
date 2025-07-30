/*
 * Copyright (c) 2014-2021 The strace developers.
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
	printfd(tcp, tcp->u_arg[0]);

	/* mode */
	tprint_arg_next();
	printflags(falloc_flags, tcp->u_arg[1], "FALLOC_FL_???");

	/* offset */
	tprint_arg_next();
	unsigned int argn = print_arg_lld(tcp, 2);

	/* len */
	tprint_arg_next();
	print_arg_lld(tcp, argn);

	return RVAL_DECODED;
}
