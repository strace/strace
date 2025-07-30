/*
 * Copyright (c) 2012-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		/* pid */
		printpid(tcp, tcp->u_arg[0], PT_TID);
	} else {
		/* head_ptr */
		tprint_arg_next();
		printnum_ptr(tcp, tcp->u_arg[1]);

		/* len_ptr */
		tprint_arg_next();
		printnum_ulong(tcp, tcp->u_arg[2]);
	}
	return 0;
}
