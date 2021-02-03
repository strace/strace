/*
 * Copyright (c) 2014-2018 The strace developers.
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

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	if (flags) {
		printflags(swap_flags, flags, "SWAP_FLAG_???");
		tprintf("|%u", prio);
	} else {
		tprintf("%u", prio);
	}

	return RVAL_DECODED;
}
