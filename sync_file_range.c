/*
 * Copyright (c) 2013 William Manley <william.manley@youview.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#include "xlat/sync_file_range_flags.h"

SYS_FUNC(sync_file_range)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld, ", 1);
	argn = printllval(tcp, "%lld, ", argn);
	printflags(sync_file_range_flags, tcp->u_arg[argn],
		   "SYNC_FILE_RANGE_???");

	return RVAL_DECODED;
}
