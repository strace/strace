/*
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <sys/swap.h>

#include "xlat/swap_flags.h"

SYS_FUNC(swapon)
{
	unsigned int flags = tcp->u_arg[1];
	unsigned int prio = flags & SWAP_FLAG_PRIO_MASK;
	flags &= ~SWAP_FLAG_PRIO_MASK;

	/* path */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* swapflags */
	tprint_flags_begin();
	if (flags) {
		printflags_in(swap_flags, flags, "SWAP_FLAG_???");
		tprint_flags_or();
	}
	PRINT_VAL_U(prio);
	tprint_flags_end();

	return RVAL_DECODED;
}
