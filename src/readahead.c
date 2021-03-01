/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(readahead)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* offset */
	unsigned int argn = print_arg_lld(tcp, 1);
	tprint_arg_next();

	/* count */
	PRINT_VAL_U(tcp->u_arg[argn]);

	return RVAL_DECODED;
}
