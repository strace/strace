/*
 * Copyright (c) 2016-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/pkey_access.h"

SYS_FUNC(pkey_alloc)
{
	/* flags */
	tprints_arg_name("flags");
	PRINT_VAL_X(tcp->u_arg[0]);

	/* access_rights */
	tprints_arg_next_name("access_rights");
	printflags64(pkey_access, tcp->u_arg[1], "PKEY_???");

	return RVAL_DECODED;
}

SYS_FUNC(pkey_free)
{
	/* pkey */
	tprints_arg_name("pkey");
	PRINT_VAL_D((int) tcp->u_arg[0]);

	return RVAL_DECODED;
}
