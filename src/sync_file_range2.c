/*
 * Copyright (c) 2013 William Manley <william.manley@youview.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#include <linux/fs.h>
#include "xlat/sync_file_range_flags.h"

SYS_FUNC(sync_file_range2)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* flags */
	printflags(sync_file_range_flags, tcp->u_arg[1],
		   "SYNC_FILE_RANGE_???");
	tprint_arg_next();

	/* offset */
	unsigned int argn = print_arg_lld(tcp, 2);
	tprint_arg_next();

	/* nbytes */
	print_arg_lld(tcp, argn);

	return RVAL_DECODED;
}
