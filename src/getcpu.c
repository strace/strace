/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(getcpu)
{
	if (exiting(tcp)) {
		/* cpu */
		tprints_arg_name("cpu");
		printnum_int_u(tcp, tcp->u_arg[0]);

		/* node */
		tprints_arg_next_name("node");
		printnum_int_u(tcp, tcp->u_arg[1]);

		/* tcache */
		tprints_arg_next_name("tcache");
		printaddr(tcp->u_arg[2]);
	}
	return 0;
}
