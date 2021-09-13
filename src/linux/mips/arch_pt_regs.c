/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "ptrace.h"

static void
arch_decode_pt_regs(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct pt_regs regs;

	if (umove_or_printaddr(tcp, addr, &regs))
		return;

	tprint_struct_begin();
	PRINT_FIELD_ARRAY(regs, regs, tcp, print_xint_array_member);

	tprint_struct_next();
	PRINT_FIELD_X(regs, lo);

	tprint_struct_next();
	PRINT_FIELD_X(regs, hi);

	tprint_struct_next();
	PRINT_FIELD_X(regs, cp0_epc);

	tprint_struct_next();
	PRINT_FIELD_X(regs, cp0_badvaddr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, cp0_status);

	tprint_struct_next();
	PRINT_FIELD_X(regs, cp0_cause);

	tprint_struct_end();
}
