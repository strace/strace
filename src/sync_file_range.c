/*
 * Copyright (c) 2013 William Manley <william.manley@youview.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#include <linux/fs.h>
#include "xlat/sync_file_range_flags.h"

SYS_FUNC(sync_file_range)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);

	/* offset */
	tprint_arg_next();
	unsigned int argn = print_arg_lld(tcp, 1);

	/* nbytes */
	tprint_arg_next();
	argn = print_arg_lld(tcp, argn);

	/* flags */
	tprint_arg_next();
	printflags(sync_file_range_flags, tcp->u_arg[argn],
		   "SYNC_FILE_RANGE_???");

	return RVAL_DECODED;
}
