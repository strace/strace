/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/socketcalls.h"

SYS_FUNC(socketcall)
{
	/* call */
	printxval_d(socketcalls, tcp->u_arg[0], NULL);
	tprint_arg_next();

	/* args */
	printaddr(tcp->u_arg[1]);

	return RVAL_DECODED;
}
