/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(readahead)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld", 1);
	tprintf(", %" PRI_klu, tcp->u_arg[argn]);

	return RVAL_DECODED;
}
