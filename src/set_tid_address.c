/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(set_tid_address)
{
	/* tidptr */
	tprints_arg_name("tidptr");
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_TID;
}
