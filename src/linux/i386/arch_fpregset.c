/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_decode_fpregset(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const kernel_ulong_t size)
{
	struct_fpregset regs;
	const size_t fetch_size = MIN(sizeof(regs), size);

	if (!size || size & 3) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_X(regs, cwd);
		if (fetch_size > offsetof(struct_fpregset, swd)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, swd);
		}
		if (fetch_size > offsetof(struct_fpregset, twd)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, twd);
		}
		if (fetch_size > offsetof(struct_fpregset, fip)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fip);
		}
		if (fetch_size > offsetof(struct_fpregset, fcs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fcs);
		}
		if (fetch_size > offsetof(struct_fpregset, foo)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, foo);
		}
		if (fetch_size > offsetof(struct_fpregset, fos)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fos);
		}
		if (fetch_size > offsetof(struct_fpregset, st_space)) {
			const size_t len = fetch_size -
				offsetof(struct_fpregset, st_space);
			tprint_struct_next();
			PRINT_FIELD_ARRAY_UPTO(regs, st_space, len / 4, tcp,
					       print_xint_array_member);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
