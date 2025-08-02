/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(nice)
{
	tprints_arg_name("increment");
	PRINT_VAL_D((int) tcp->u_arg[0]);

	return RVAL_DECODED;
}
