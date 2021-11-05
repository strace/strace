/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/ipccalls.h"

SYS_FUNC(ipc)
{
	unsigned int call = tcp->u_arg[0];
	unsigned int version = call >> 16;
	call &= 0xffff;

	if (version) {
		PRINT_VAL_U(version);
		tprints("<<16|");
	}

	printxval_u(ipccalls, call, NULL);

	for (unsigned int i = 1; i < n_args(tcp); ++i) {
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[i]);
	}

	return RVAL_DECODED;
}
