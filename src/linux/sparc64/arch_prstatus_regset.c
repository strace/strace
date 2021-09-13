/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../sparc/arch_prstatus_regset.c"
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
		PRINT_FIELD_ARRAY_UPTO(regs, g, fetch_size / 8, tcp,
				       print_xint_array_member);
		if (fetch_size > offsetof(struct_prstatus_regset, o)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, o);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, o, len / 8, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, l)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, l);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, l, len / 8, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, i)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, i);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, i, len / 8, tcp,
					       print_xint_array_member);
		}

		if (fetch_size > offsetof(struct_prstatus_regset, tstate)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, tstate);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, tpc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, tpc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, tnpc)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, tnpc);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, y)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, y);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#endif /* !MPERS_IS_m32 */
