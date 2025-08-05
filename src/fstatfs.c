/*
 * Copyright (c) 2016-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(fstatfs)
{
	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		printfd(tcp, tcp->u_arg[0]);
	} else {
		/* buf */
		tprints_arg_next_name("buf");
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
