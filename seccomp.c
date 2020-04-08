/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_SECCOMP_H
# include <linux/seccomp.h>
#endif
#include "xlat/seccomp_ops.h"
#include "xlat/seccomp_filter_flags.h"

SYS_FUNC(seccomp)
{
	unsigned int op = tcp->u_arg[0];
	unsigned int flags = tcp->u_arg[1];
	unsigned int act;

	printxval(seccomp_ops, op, "SECCOMP_SET_MODE_???");
	tprints(", ");

	switch (op) {
	case SECCOMP_GET_ACTION_AVAIL:
		tprintf("%u, ", flags);
		if (!umove_or_printaddr(tcp, tcp->u_arg[2], &act)) {
			tprints("[");
			printxval(seccomp_ret_action, act, "SECCOMP_RET_???");
			tprints("]");
		}
		break;

	case SECCOMP_SET_MODE_FILTER:
		printflags(seccomp_filter_flags, flags,
			   "SECCOMP_FILTER_FLAG_???");
		tprints(", ");
		decode_seccomp_fprog(tcp, tcp->u_arg[2]);
		break;

	case SECCOMP_SET_MODE_STRICT:
	default:
		tprintf("%u, ", flags);
		printaddr(tcp->u_arg[2]);
		break;
	}

	return RVAL_DECODED;
}
