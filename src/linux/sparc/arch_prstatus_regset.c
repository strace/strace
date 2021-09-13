/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_decode_prstatus_regset(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const kernel_ulong_t size)
{
	struct_prstatus_regset regs;
	const size_t fetch_size = MIN(sizeof(regs), size);

	if (!size || size & 3) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_ARRAY_UPTO(regs, g, fetch_size / 4, tcp,
				       print_xint_array_member);
		if (fetch_size > offsetof(struct_prstatus_regset, o)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, o);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, o, len / 4, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, l)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, l);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, l, len / 4, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, i)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, i);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, i, len / 4, tcp,
					       print_xint_array_member);
		}

		if (fetch_size > offsetof(struct_prstatus_regset, psr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, psr);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, pc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, pc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, npc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, npc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, y)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, y);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, wim)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, wim);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, tbr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, tbr);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
