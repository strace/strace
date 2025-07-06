/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(statfs)
{
	if (entering(tcp)) {
		/* pathname */
		print_syscall_param("pathname");
		printpath(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* buf */
		print_syscall_param("buf");
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
