/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(lookup_dcookie)
{
	if (entering(tcp))
		return 0;

	/* cookie */
	int argn = printllval(tcp, "%llu", 0);
	tprints(", ");

	/* buffer */
	if (syserror(tcp))
		printaddr(tcp->u_arg[argn]);
	else
		printstrn(tcp, tcp->u_arg[argn], tcp->u_rval);

	/* len */
	tprintf(", %" PRI_klu, tcp->u_arg[argn + 1]);

	return 0;
}
