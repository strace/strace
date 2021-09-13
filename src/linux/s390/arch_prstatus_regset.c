/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# define TRACEE_KLONGSIZE 4
#else
# define TRACEE_KLONGSIZE SIZEOF_KERNEL_LONG_T
#endif

static void
arch_decode_prstatus_regset(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const kernel_ulong_t size)
{
	struct_prstatus_regset regs;
	const size_t fetch_size = MIN(sizeof(regs), size);

	if (!size || size & (TRACEE_KLONGSIZE - 1)) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		tprints_field_name("psw");
		tprint_struct_begin();
		PRINT_FIELD_X(regs.psw, mask);
		if (fetch_size > sizeof(regs.psw.mask)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs.psw, addr);
		}
		tprint_struct_end();
		if (fetch_size > offsetof(struct_prstatus_regset, gprs)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, gprs);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, gprs,
					       len / sizeof(regs.gprs[0]), tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, acrs)) {
			const size_t len =
				fetch_size - offsetof(struct_prstatus_regset, acrs);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, acrs,
					       len / sizeof(regs.acrs[0]), tcp,
					       print_xint_array_member);
		}

		if (fetch_size > offsetof(struct_prstatus_regset, orig_gpr2)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, orig_gpr2);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#undef TRACEE_KLONGSIZE
