/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
decode_pt_regs64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct {
		unsigned long u_regs[16];
		unsigned long tstate;
		unsigned long tpc;
		unsigned long tnpc;
	} regs;

	if (umove_or_printaddr(tcp, addr, &regs))
		return;

	tprint_struct_begin();
	PRINT_FIELD_ARRAY(regs, u_regs, tcp, print_xint_array_member);

	tprint_struct_next();
	PRINT_FIELD_X(regs, tstate);

	tprint_struct_next();
	PRINT_FIELD_X(regs, tpc);

	tprint_struct_next();
	PRINT_FIELD_X(regs, tnpc);

	tprint_struct_end();
}
