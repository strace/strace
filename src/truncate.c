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
	print_syscall_param("path");
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_syscall_param("length");
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(truncate64)
{
	/* path */
	print_syscall_param("path");
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_syscall_param("length");
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate)
{
	/* fd */
	print_syscall_param("fd");
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_syscall_param("length");
	PRINT_VAL_U(tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate64)
{
	/* fd */
	print_syscall_param("fd");
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* length */
	print_syscall_param("length");
	print_arg_llu(tcp, 1);

	return RVAL_DECODED;
}
