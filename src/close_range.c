/*
 * Copyright (c) 2020-2021 The strace developers.
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
	PRINT_VAL_U((unsigned int) tcp->u_arg[0]);
	tprint_arg_next();

	/* max_fd */
	PRINT_VAL_U((unsigned int) tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(close_range_flags, tcp->u_arg[2], "CLOSE_RANGE_???");
	return RVAL_DECODED;
}
