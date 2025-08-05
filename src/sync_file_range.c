/*
 * Copyright (c) 2013 William Manley <william.manley@youview.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
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
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* offset */
	tprints_arg_next_name("offset");
	unsigned int argn = print_arg_lld(tcp, 1);

	/* nbytes */
	tprints_arg_next_name("nbytes");
	argn = print_arg_lld(tcp, argn);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(sync_file_range_flags, tcp->u_arg[argn],
		   "SYNC_FILE_RANGE_???");

	return RVAL_DECODED;
}
