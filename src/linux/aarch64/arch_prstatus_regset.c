/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../arm/arch_prstatus_regset.c"
#else

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
		if (fetch_size > offsetof(struct_prstatus_regset, sp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, sp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, pc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, pc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, pstate)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, pstate);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#endif /* !MPERS_IS_m32 */
