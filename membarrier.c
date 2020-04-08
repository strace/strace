/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/membarrier_cmds.h"

SYS_FUNC(membarrier)
{
	if (entering(tcp)) {
		int cmd = tcp->u_arg[0], flags = tcp->u_arg[1];

		printxval(membarrier_cmds, cmd, "MEMBARRIER_CMD_???");
		tprintf(", %d", flags);

		return cmd ? RVAL_DECODED : 0;
	}

	if (syserror(tcp) || !tcp->u_rval)
		return 0;

	tcp->auxstr = sprintflags("", membarrier_cmds,
				  (kernel_ulong_t) tcp->u_rval);
	return RVAL_HEX | RVAL_STR;
}
