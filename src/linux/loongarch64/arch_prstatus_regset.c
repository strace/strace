/*
 * Copyright (c) 2021-2022 The strace developers.
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

	if (!size || size & 7) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_ARRAY_UPTO(regs, regs,
				       fetch_size / 8, tcp,
				       print_xint_array_member);
		if (fetch_size > offsetof(struct_prstatus_regset, orig_a0)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, orig_a0);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, csr_era)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, csr_era);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, csr_badv)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, csr_badv);
		}
		const size_t offset_of_reserved =
			offsetof(struct_prstatus_regset, reserved);
		if (fetch_size > offset_of_reserved) {
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, reserved,
					       (fetch_size - offset_of_reserved) / 8,
					       tcp, print_xint_array_member);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
