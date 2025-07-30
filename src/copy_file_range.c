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

	/* loff_t *off_in */
	tprint_arg_next();
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);

	/* int fd_out */
	tprint_arg_next();
	printfd(tcp, tcp->u_arg[2]);

	/* loff_t *off_out */
	tprint_arg_next();
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);

	/* size_t len */
	tprint_arg_next();
	PRINT_VAL_U(tcp->u_arg[4]);

	/* unsigned int flags */
	tprint_arg_next();
	unsigned int flags = tcp->u_arg[5];
	PRINT_VAL_U(flags);

	return RVAL_DECODED;
}
