/*
 * Copyright (c) 1999-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_instruction_pointer(struct tcb *tcp)
{
	kernel_ulong_t ip;

	if (get_instruction_pointer(tcp, &ip)) {
		tprintf(current_wordsize == 4
			? "[%08" PRI_klx "] "
			: "[%016" PRI_klx "] ", ip);
	} else {
		tprints(current_wordsize == 4
			? "[????????] "
			: "[????????????????] ");
	}
}
