/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(truncate)
{
	/* path */
	tprints_arg_name("path");
	printpath(tcp, tcp->u_arg[0]);

	/* length */
	tprints_arg_next_name("length");
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(truncate64)
{
	/* path */
	tprints_arg_name("path");
	printpath(tcp, tcp->u_arg[0]);

	/* length */
	tprints_arg_next_name("length");
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* length */
	tprints_arg_next_name("length");
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate64)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* length */
	tprints_arg_next_name("length");
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}
