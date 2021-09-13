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

	if (!size || size & (SIZEOF_LONG - 1)) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		if (fetch_size > offsetof(struct_prstatus_regset, regs)) {
			const size_t len = fetch_size -
				offsetof(struct_prstatus_regset, regs);
			PRINT_FIELD_ARRAY_UPTO(regs, regs, len / SIZEOF_LONG,
					       tcp, print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, lo)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, lo);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, hi)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, hi);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, cp0_epc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, cp0_epc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, cp0_badvaddr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, cp0_badvaddr);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, cp0_status)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, cp0_status);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, cp0_cause)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, cp0_cause);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
