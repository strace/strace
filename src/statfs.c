/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(statfs)
{
	if (entering(tcp)) {
		/* pathname */
		tprints_arg_name("pathname");
		printpath(tcp, tcp->u_arg[0]);
	} else {
		/* buf */
		tprints_arg_next_name("buf");
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
