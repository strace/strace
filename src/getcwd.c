/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(getcwd)
{
	if (exiting(tcp)) {
		/* buf */
		tprints_arg_name("buf");
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printpathn(tcp, tcp->u_arg[0], tcp->u_rval);

		/* size */
		tprints_arg_next_name("size");
		PRINT_VAL_U(tcp->u_arg[1]);
	}
	return 0;
}
