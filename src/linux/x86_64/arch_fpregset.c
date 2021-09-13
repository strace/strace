/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../i386/arch_fpregset.c"
#else

static void
arch_decode_fpregset(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t size)
{
	struct_fpregset regs;
	const size_t fetch_size = MIN(sizeof(regs), size);

	if (!size || size & 7) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_X(regs, cwd);
		if (fetch_size > offsetof(struct_fpregset, swd)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, swd);
		}
		if (fetch_size > offsetof(struct_fpregset, ftw)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ftw);
		}
		if (fetch_size > offsetof(struct_fpregset, fop)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fop);
		}
		if (fetch_size > offsetof(struct_fpregset, rip)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rip);
		}
		if (fetch_size > offsetof(struct_fpregset, rdp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rdp);
		}
		if (fetch_size > offsetof(struct_fpregset, mxcsr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, mxcsr);
		}
		if (fetch_size > offsetof(struct_fpregset, mxcr_mask)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, mxcr_mask);
		}
		if (fetch_size > offsetof(struct_fpregset, st_space)) {
			const size_t len = fetch_size -
				offsetof(struct_fpregset, st_space);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, st_space, len / 4, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_fpregset, xmm_space)) {
			const size_t len = fetch_size -
				offsetof(struct_fpregset, xmm_space);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, xmm_space, len / 4, tcp,
					       print_xint_array_member);
		}
		if (fetch_size > offsetof(struct_fpregset, padding)) {
			const size_t len = fetch_size -
				offsetof(struct_fpregset, padding);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, padding, len / 4, tcp,
					       print_xint_array_member);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#endif /* !MPERS_IS_m32 */
