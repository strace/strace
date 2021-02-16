/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/pkey_access.h"

SYS_FUNC(pkey_alloc)
{
	tprintf("%#" PRI_klx ", ", tcp->u_arg[0]);
	printflags64(pkey_access, tcp->u_arg[1], "PKEY_???");

	return RVAL_DECODED;
}
