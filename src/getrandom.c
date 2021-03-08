/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/getrandom_flags.h"

SYS_FUNC(getrandom)
{
	if (exiting(tcp)) {
		/* buf */
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printstr_ex(tcp, tcp->u_arg[0], tcp->u_rval,
				    QUOTE_FORCE_HEX);
		tprint_arg_next();

		/* buflen */
		PRINT_VAL_U(tcp->u_arg[1]);
		tprint_arg_next();

		/* flags */
		printflags(getrandom_flags, tcp->u_arg[2], "GRND_???");
	}
	return 0;
}
