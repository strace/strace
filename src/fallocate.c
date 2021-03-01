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
	tprint_arg_next();

	/* mode */
	printflags(falloc_flags, tcp->u_arg[1], "FALLOC_FL_???");
	tprint_arg_next();

	/* offset */
	unsigned int argn = print_arg_lld(tcp, 2);
	tprint_arg_next();

	/* len */
	print_arg_lld(tcp, argn);

	return RVAL_DECODED;
}
