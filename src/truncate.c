/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(truncate)
{
	printpath(tcp, tcp->u_arg[0]);
	tprintf(", %" PRI_klu, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(truncate64)
{
	printpath(tcp, tcp->u_arg[0]);
	printllval(tcp, ", %llu", 1);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate)
{
	printfd(tcp, tcp->u_arg[0]);
	tprintf(", %" PRI_klu, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate64)
{
	printfd(tcp, tcp->u_arg[0]);
	printllval(tcp, ", %llu", 1);

	return RVAL_DECODED;
}
