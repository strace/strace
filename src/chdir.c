/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(chdir)
{
	tprints_arg_name("path");
	printpath(tcp, tcp->u_arg[0]);

	return RVAL_DECODED;
}
