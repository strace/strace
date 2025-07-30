/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(fstatfs)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
	} else {
		/* buf */
		tprint_arg_next();
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
