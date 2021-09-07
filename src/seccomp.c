/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/seccomp.h>
#include "xlat/seccomp_ops.h"
#include "xlat/seccomp_filter_flags.h"

SYS_FUNC(seccomp)
{
	unsigned int op = tcp->u_arg[0];
	unsigned int flags = tcp->u_arg[1];
	unsigned int act;

	/* operation */
	printxval(seccomp_ops, op, "SECCOMP_SET_MODE_???");
	tprint_arg_next();

	switch (op) {
	case SECCOMP_GET_ACTION_AVAIL:
		/* flags */
		PRINT_VAL_X(flags);
		tprint_arg_next();

		/* args */
		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &act)) {
			tprint_indirect_begin();
			printxval(seccomp_ret_action, act, "SECCOMP_RET_???");
			tprint_indirect_end();
		}
		break;

	case SECCOMP_SET_MODE_FILTER:
		/* flags */
		printflags(seccomp_filter_flags, flags,
			   "SECCOMP_FILTER_FLAG_???");
		tprint_arg_next();

		/* args */
		decode_seccomp_fprog(tcp, tcp->u_arg[2]);
		break;

	case SECCOMP_SET_MODE_STRICT:
	default:
		/* flags */
		PRINT_VAL_X(flags);
		tprint_arg_next();

		/* args */
		printaddr(tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED;
}
