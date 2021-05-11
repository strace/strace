/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../i386/arch_prstatus_regset.c"
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
		PRINT_FIELD_X(regs, r15);
		if (fetch_size > offsetof(struct_prstatus_regset, r14)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r14);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r13)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r13);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r12)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r12);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rbp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rbp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rbx)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rbx);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r11)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r11);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r10)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r10);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r9)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r9);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, r8)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, r8);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rax)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rax);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rcx)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rcx);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rdx)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rdx);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rsi)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rsi);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rdi)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rdi);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, orig_rax)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, orig_rax);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rip)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rip);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, cs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, cs);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, eflags)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, eflags);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, rsp)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, rsp);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, ss)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ss);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, fs_base)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fs_base);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, gs_base)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, gs_base);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, ds)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ds);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, es)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, es);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, fs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, fs);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, gs)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, gs);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#endif /* !MPERS_IS_m32 */
