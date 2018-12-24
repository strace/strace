/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/getrandom_flags.h"

SYS_FUNC(getrandom)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printstr_ex(tcp, tcp->u_arg[0], tcp->u_rval,
				    QUOTE_FORCE_HEX);
		tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
		printflags(getrandom_flags, tcp->u_arg[2], "GRND_???");
	}
	return 0;
}
