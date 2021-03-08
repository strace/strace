/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/utsname.h>

SYS_FUNC(sethostname)
{
	unsigned int len = tcp->u_arg[1];

	/* name */
	if (len > __NEW_UTS_LEN) {
		printaddr(tcp->u_arg[0]);
	} else {
		printstrn(tcp, tcp->u_arg[0], len);
	}
	tprint_arg_next();

	/* len */
	PRINT_VAL_U(len);

	return RVAL_DECODED;
}

#if defined(ALPHA)
SYS_FUNC(gethostname)
{
	if (exiting(tcp)) {
		/* name */
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printstr(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		/* len */
		PRINT_VAL_U(tcp->u_arg[1]);
	}
	return 0;
}
#endif /* ALPHA */
