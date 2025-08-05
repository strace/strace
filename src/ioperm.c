/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(ioperm)
{
	/* from */
	tprints_arg_name("from");
	PRINT_VAL_X(tcp->u_arg[0]);

	/* num */
	tprints_arg_next_name("num");
	PRINT_VAL_X(tcp->u_arg[1]);

	/* turn_on */
	tprints_arg_next_name("turn_on");
	PRINT_VAL_D((int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
