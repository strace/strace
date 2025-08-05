/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(copy_file_range)
{
	/* int fd_in */
	tprints_arg_name("fd_in");
	printfd(tcp, tcp->u_arg[0]);

	/* loff_t *off_in */
	tprints_arg_next_name("off_in");
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);

	/* int fd_out */
	tprints_arg_next_name("fd_out");
	printfd(tcp, tcp->u_arg[2]);

	/* loff_t *off_out */
	tprints_arg_next_name("off_out");
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);

	/* size_t len */
	tprints_arg_next_name("len");
	PRINT_VAL_U(tcp->u_arg[4]);

	/* unsigned int flags */
	tprints_arg_next_name("flags");
	unsigned int flags = tcp->u_arg[5];
	PRINT_VAL_U(flags);

	return RVAL_DECODED;
}
