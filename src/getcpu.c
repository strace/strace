/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(getcpu)
{
	if (exiting(tcp)) {
		/* cpu */
		printnum_int(tcp, tcp->u_arg[0], "%u");
		tprint_arg_next();

		/* node */
		printnum_int(tcp, tcp->u_arg[1], "%u");
		tprint_arg_next();

		/* tcache */
		printaddr(tcp->u_arg[2]);
	}
	return 0;
}
