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
		PRINT_FIELD_ARRAY_UPTO(regs, gpr, fetch_size / TRACEE_KLONGSIZE,
				       tcp, print_xint_array_member);
		if (fetch_size > offsetof(struct_prstatus_regset, nip)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, nip);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, msr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, msr);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, orig_gpr3)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, orig_gpr3);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, ctr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ctr);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, link)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, link);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, xer)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, xer);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, ccr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, ccr);
		}
#if TRACEE_KLONGSIZE == 4
		if (fetch_size > offsetof(struct_prstatus_regset, mq)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, mq);
		}
#else
		if (fetch_size > offsetof(struct_prstatus_regset, softe)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, softe);
		}
#endif
		if (fetch_size > offsetof(struct_prstatus_regset, trap)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, trap);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, dar)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, dar);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, dsisr)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, dsisr);
		}
		if (fetch_size > offsetof(struct_prstatus_regset, result)) {
			tprint_struct_next();
			PRINT_FIELD_X(regs, result);
		}
		if (size > sizeof(regs)) {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
}

#undef TRACEE_KLONGSIZE
