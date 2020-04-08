/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(copy_file_range)
{
	/* int fd_in */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* loff_t *off_in */
	printnum_int64(tcp, tcp->u_arg[1], "%" PRId64);
	tprints(", ");
	/* int fd_out */
	printfd(tcp, tcp->u_arg[2]);
	tprints(", ");
	/* loff_t *off_out */
	printnum_int64(tcp, tcp->u_arg[3], "%" PRId64);
	tprints(", ");
	/* size_t len */
	tprintf("%" PRI_klu ", ", tcp->u_arg[4]);
	/* unsigned int flags */
	tprintf("%u", (unsigned int) tcp->u_arg[5]);

	return RVAL_DECODED;
}
