/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "arch_pt_regs64.h"

static void
decode_pt_regs64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_pt_regs64 regs;

	if (umove_or_printaddr(tcp, addr, &regs))
		return;

	tprint_struct_begin();
	PRINT_FIELD_ARRAY(regs, gpr, tcp, print_xint_array_member);

	tprint_struct_next();
	PRINT_FIELD_X(regs, nip);

	tprint_struct_next();
	PRINT_FIELD_X(regs, msr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, orig_gpr3);

	tprint_struct_next();
	PRINT_FIELD_X(regs, ctr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, link);

	tprint_struct_next();
	PRINT_FIELD_X(regs, xer);

	tprint_struct_next();
	PRINT_FIELD_X(regs, ccr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, softe);

	tprint_struct_next();
	PRINT_FIELD_X(regs, trap);

	tprint_struct_next();
	PRINT_FIELD_X(regs, dar);

	tprint_struct_next();
	PRINT_FIELD_X(regs, dsisr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, result);

	tprint_struct_end();
}
