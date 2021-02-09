/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/close_range.h>
#include "xlat/close_range_flags.h"

SYS_FUNC(close_range)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printfd(tcp, tcp->u_arg[1]);
	tprints(", ");
	printflags(close_range_flags, tcp->u_arg[2], "CLOSE_RANGE_???");
	return RVAL_DECODED;
}
