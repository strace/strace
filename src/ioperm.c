/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(ioperm)
{
	/* from */
	PRINT_VAL_X(tcp->u_arg[0]);
	tprint_arg_next();

	/* num */
	PRINT_VAL_X(tcp->u_arg[1]);
	tprint_arg_next();

	/* turn_on */
	PRINT_VAL_D((int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
