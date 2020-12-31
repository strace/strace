/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(ioperm)
{
	tprintf("%#" PRI_klx ", %#" PRI_klx ", %d",
		tcp->u_arg[0], tcp->u_arg[1], (int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
