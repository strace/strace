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

	if (!size || size & 7) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_ARRAY_UPTO(regs, fpr, fetch_size / 8, tcp,
				       print_xint_array_member);
		if (fetch_size > offsetof(struct_fpregset, fpscr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fpscr);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
