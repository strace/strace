/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(alarm)
{
	/* seconds */
	tprints_arg_name("seconds");
	PRINT_VAL_U((unsigned int) tcp->u_arg[0]);

	return RVAL_DECODED;
}
