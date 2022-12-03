/*
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_instruction_pointer(struct tcb *tcp)
{
	kernel_ulong_t ip;

	if(!structured_output)
		tprint_attribute_begin();
	if (get_instruction_pointer(tcp, &ip)) {
		tprintf_field_string("pc",
				     current_wordsize == 4
				     ? "%08" PRI_klx
				     : "%016" PRI_klx, ip);
	} else {
		tprintf_field_string("pc", current_wordsize == 4
				     ? "????????"
				     : "????????????????");
	}
	if(!structured_output)
		tprint_attribute_end();
	tprint_space();
}
