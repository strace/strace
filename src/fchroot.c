/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>

SYS_FUNC(fchroot)
{
	/* fd */
	tprints_arg_name("fd");
	if ((int) tcp->u_arg[0] == FD_FAILFS_ROOT)
		print_xlat_d(FD_FAILFS_ROOT);
	else
		printfd(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	PRINT_VAL_X((unsigned int) tcp->u_arg[1]);

	return RVAL_DECODED;
}
