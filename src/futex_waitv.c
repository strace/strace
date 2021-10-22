/*
 * Copyright (c) 2021-2022 The strace developers.
 * Copyright (c) 2021 André Almeida <andrealmeid@collabora.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(futex_waitv)
{
	const kernel_ulong_t waiters = tcp->u_arg[0];
	const unsigned int nr_futexes = tcp->u_arg[1];
	const unsigned int flags = tcp->u_arg[2];
	const kernel_ulong_t timeout = tcp->u_arg[3];
	const unsigned int clockid = tcp->u_arg[4];

	printaddr(waiters);
	tprint_arg_next();
	PRINT_VAL_U(nr_futexes);
	tprint_arg_next();
	PRINT_VAL_X(flags);
	tprint_arg_next();
	print_timespec64(tcp, timeout);
	tprint_arg_next();
	printxval(clocknames, clockid, "CLOCK_???");

	return RVAL_DECODED;
}
