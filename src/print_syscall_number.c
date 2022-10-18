/*
 * Copyright (c) 2020-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_syscall_number(struct tcb *tcp)
{
	tprint_attribute_begin();
	if (tcp->true_scno != (kernel_ulong_t) -1)
		tprintf_string("%4" PRI_klu, tcp->true_scno);
	else
		tprint_unavailable();
	tprint_attribute_end();
	tprint_space();
}
