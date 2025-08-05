/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(sendfile64)
{
	if (entering(tcp)) {
		/* out_fd */
		tprints_arg_name("out_fd");
		printfd(tcp, tcp->u_arg[0]);

		/* in_fd */
		tprints_arg_next_name("in_fd");
		printfd(tcp, tcp->u_arg[1]);

		/* offset */
		tprints_arg_next_name("offset");
		if (!printnum_int64(tcp, tcp->u_arg[2], "%" PRIu64)) {

			/* count */
			tprints_arg_next_name("count");
			PRINT_VAL_U(tcp->u_arg[3]);
			return RVAL_DECODED;
		}
	} else {
		if (!syserror(tcp) && tcp->u_rval) {
			tprint_value_changed();

			/* offset */
			tprints_arg_name("offset");
			printnum_int64(tcp, tcp->u_arg[2], "%" PRIu64);
		}

		/* count */
		tprints_arg_next_name("count");
		PRINT_VAL_U(tcp->u_arg[3]);
	}

	return 0;
}

SYS_FUNC(sendfile)
{
	if (entering(tcp)) {
		/* out_fd */
		tprints_arg_name("out_fd");
		printfd(tcp, tcp->u_arg[0]);

		/* in_fd */
		tprints_arg_next_name("in_fd");
		printfd(tcp, tcp->u_arg[1]);

		/* offset */
		tprints_arg_next_name("offset");
		if (!printnum_ulong(tcp, tcp->u_arg[2])
		    || !tcp->u_arg[3]) {

			/* count */
			tprints_arg_next_name("count");
			PRINT_VAL_U(tcp->u_arg[3]);
			return RVAL_DECODED;
		}
	} else {
		if (!syserror(tcp) && tcp->u_rval) {
			tprint_value_changed();

			/* offset */
			tprints_arg_name("offset");
			printnum_ulong(tcp, tcp->u_arg[2]);
		}

		/* count */
		tprints_arg_next_name("count");
		PRINT_VAL_U(tcp->u_arg[3]);
	}

	return 0;
}
