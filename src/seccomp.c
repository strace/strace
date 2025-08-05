/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
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
	if (entering(tcp)) {
		tprints_arg_name("operation");
		printxval(seccomp_ops, op, "SECCOMP_SET_MODE_???");
	}

	switch (op) {
	case SECCOMP_GET_ACTION_AVAIL:
		/* flags */
		tprints_arg_next_name("flags");
		PRINT_VAL_X(flags);

		/* args */
		tprints_arg_next_name("args");
		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &act)) {
			tprint_indirect_begin();
			printxval(seccomp_ret_action, act, "SECCOMP_RET_???");
			tprint_indirect_end();
		}
		break;

	case SECCOMP_GET_NOTIF_SIZES:
		if (entering(tcp)) {
			/* flags */
			tprints_arg_next_name("flags");
			PRINT_VAL_X(flags);

			return 0;
		} else {
			struct seccomp_notif_sizes szs;

			/* args */
			tprints_arg_next_name("args");
			if (!umove_or_printaddr(tcp, tcp->u_arg[2], &szs)) {
				tprint_struct_begin();
				PRINT_FIELD_U(szs, seccomp_notif);
				tprint_struct_next();
				PRINT_FIELD_U(szs, seccomp_notif_resp);
				tprint_struct_next();
				PRINT_FIELD_U(szs, seccomp_data);
				tprint_struct_end();
			}
		}
		break;

	case SECCOMP_SET_MODE_FILTER:
		/* flags */
		tprints_arg_next_name("flags");
		printflags(seccomp_filter_flags, flags,
			   "SECCOMP_FILTER_FLAG_???");

		/* args */
		tprints_arg_next_name("args");
		decode_seccomp_fprog(tcp, tcp->u_arg[2]);
		break;

	case SECCOMP_SET_MODE_STRICT:
	default:
		/* flags */
		tprints_arg_next_name("flags");
		PRINT_VAL_X(flags);

		/* args */
		tprints_arg_next_name("args");
		printaddr(tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED;
}
