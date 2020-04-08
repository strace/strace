/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(sendfile64)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printfd(tcp, tcp->u_arg[1]);
		tprints(", ");
		if (!printnum_int64(tcp, tcp->u_arg[2], "%" PRIu64)) {
			tprintf(", %" PRI_klu, tcp->u_arg[3]);
			return RVAL_DECODED;
		}
	} else {
		if (!syserror(tcp) && tcp->u_rval) {
			tprints(" => ");
			printnum_int64(tcp, tcp->u_arg[2], "%" PRIu64);
		}
		tprintf(", %" PRI_klu, tcp->u_arg[3]);
	}

	return 0;
}

SYS_FUNC(sendfile)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printfd(tcp, tcp->u_arg[1]);
		tprints(", ");
		if (!printnum_ulong(tcp, tcp->u_arg[2])
		    || !tcp->u_arg[3]) {
			tprintf(", %" PRI_klu, tcp->u_arg[3]);
			return RVAL_DECODED;
		}
	} else {
		if (!syserror(tcp) && tcp->u_rval) {
			tprints(" => ");
			printnum_ulong(tcp, tcp->u_arg[2]);
		}
		tprintf(", %" PRI_klu, tcp->u_arg[3]);
	}

	return 0;
}
