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
		PRINT_FIELD_X(regs, ebx);
		if (fetch_size > offsetof(struct_prstatus_regset, ecx)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ecx);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, edx)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, edx);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, esi)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, esi);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, edi)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, edi);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, ebp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ebp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, eax)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, eax);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xds)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xds);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xes)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xes);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xfs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xfs);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xgs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xgs);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, orig_eax)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, orig_eax);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, eip)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, eip);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xcs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xcs);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, eflags)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, eflags);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, esp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, esp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xss)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xss);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
