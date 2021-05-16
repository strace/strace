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

	if (!size || size & 7) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &regs)) {
		tprint_struct_begin();
		PRINT_FIELD_X(regs, pc);
		if (fetch_size > offsetof(struct_prstatus_regset, ra)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ra);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, sp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, sp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, gp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, gp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, tp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, tp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t0)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t0);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t1)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t1);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t2)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t2);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s0)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s0);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s1)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s1);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a0)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a0);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a1)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a1);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a2)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a2);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a3)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a3);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a4)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a4);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a5)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a5);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a6)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a6);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, a7)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, a7);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s2)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s2);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s3)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s3);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s4)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s4);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s5)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s5);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s6)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s6);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s7)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s7);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s8)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s8);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s9)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s9);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s10)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s10);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, s11)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, s11);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t3)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t3);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t4)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t4);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t5)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t5);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, t6)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, t6);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}
