/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(copy_file_range)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* loff_t *off_in */
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);
	tprint_arg_next();

	/* int fd_out */
	printfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* loff_t *off_out */
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);
	tprint_arg_next();

	/* size_t len */
	PRINT_VAL_U(tcp->u_arg[4]);
	tprint_arg_next();

	/* unsigned int flags */
	unsigned int flags = tcp->u_arg[5];
	PRINT_VAL_U(flags);

	return RVAL_DECODED;
}
