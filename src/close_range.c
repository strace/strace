/*
 * Copyright (c) 2020-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/close_range.h>
#include "xlat/close_range_flags.h"

SYS_FUNC(close_range)
{
	/* fd */
	tprints_arg_name("first");
	PRINT_VAL_U((unsigned int) tcp->u_arg[0]);

	/* max_fd */
	tprints_arg_next_name("last");
	PRINT_VAL_U((unsigned int) tcp->u_arg[1]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(close_range_flags, tcp->u_arg[2], "CLOSE_RANGE_???");
	return RVAL_DECODED;
}
