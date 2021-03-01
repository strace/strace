/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(truncate)
{
	/* path */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(truncate64)
{
	/* path */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate64)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}
