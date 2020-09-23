/*
 * Copyright (c) 2012-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		printpid(tcp, tcp->u_arg[0], PT_TID);
		tprints(", ");
	} else {
		printnum_ptr(tcp, tcp->u_arg[1]);
		tprints(", ");
		printnum_ulong(tcp, tcp->u_arg[2]);
	}
	return 0;
}
