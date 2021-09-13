/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_decode_pt_regs(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct {
		unsigned int psr;
		unsigned int pc;
		unsigned int npc;
		unsigned int y;
		unsigned int u_regs[15];
	} regs;

	if (umove_or_printaddr(tcp, addr, &regs))
		return;

	tprint_struct_begin();
	PRINT_FIELD_X(regs, psr);

	tprint_struct_next();
	PRINT_FIELD_X(regs, pc);

	tprint_struct_next();
	PRINT_FIELD_X(regs, npc);

	tprint_struct_next();
	PRINT_FIELD_X(regs, y);

	tprint_struct_next();
	PRINT_FIELD_ARRAY(regs, u_regs, tcp, print_xint_array_member);

	tprint_struct_end();
}
