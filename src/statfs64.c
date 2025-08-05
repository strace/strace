/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(statfs64)
{
	if (entering(tcp)) {
		/* pathname */
		tprints_arg_name("pathname");
		printpath(tcp, tcp->u_arg[0]);

		/* size */
		tprints_arg_next_name("size");
		PRINT_VAL_U(tcp->u_arg[1]);
	} else {
		/* buf */
		tprints_arg_next_name("buf");
		print_struct_statfs64(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	}
	return 0;
}
