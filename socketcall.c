/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/socketcalls.h"

SYS_FUNC(socketcall)
{
	printxval_d(socketcalls, tcp->u_arg[0], NULL);
	tprints(", ");
	printaddr(tcp->u_arg[1]);

	return RVAL_DECODED;
}
