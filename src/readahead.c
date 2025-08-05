/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(readahead)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* offset */
	tprints_arg_next_name("offset");
	unsigned int argn = print_arg_lld(tcp, 1);

	/* count */
	tprints_arg_next_name("count");
	PRINT_VAL_U(tcp->u_arg[argn]);

	return RVAL_DECODED;
}
