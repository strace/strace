/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(time_t)

#include MPERS_DEFS

SYS_FUNC(time)
{
	if (exiting(tcp)) {
		time_t t;

		if (!umove_or_printaddr(tcp, tcp->u_arg[0], &t)) {
			tprintf("[%lld", (long long) t);
			tprints_comment(sprinttime(t));
			tprints("]");
		}

		if (!syserror(tcp)) {
			tcp->auxstr = sprinttime((time_t) tcp->u_rval);

			return RVAL_STR;
		}
	}

	return 0;
}
