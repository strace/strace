/*
 * Copyright (c) 2013 William Manley <william.manley@youview.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#include "xlat/sync_file_range_flags.h"

SYS_FUNC(sync_file_range2)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(sync_file_range_flags, tcp->u_arg[1],
		   "SYNC_FILE_RANGE_???");
	argn = printllval(tcp, ", %lld, ", 2);
	printllval(tcp, "%lld", argn);

	return RVAL_DECODED;
}
