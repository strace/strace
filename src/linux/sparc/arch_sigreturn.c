/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef SIZEOF_STRUCT_SPARC_STACKF
# define SIZEOF_STRUCT_SPARC_STACKF	sizeof(struct sparc_stackf)
#endif
#ifndef SIZEOF_STRUCT_PT_REGS
# define SIZEOF_STRUCT_PT_REGS		sizeof(struct pt_regs)
#endif
#ifndef PERSONALITY_WORDSIZE
# define PERSONALITY_WORDSIZE		PERSONALITY0_WORDSIZE
#endif

static void
arch_sigreturn(struct tcb *tcp)
{
	kernel_ulong_t addr;
	if (!get_stack_pointer(tcp, &addr))
		return;
	addr += SIZEOF_STRUCT_SPARC_STACKF + SIZEOF_STRUCT_PT_REGS;
	struct {
		unsigned int mask;
		char fpu_save[PERSONALITY_WORDSIZE];
		char insns[PERSONALITY_WORDSIZE * 2] ATTRIBUTE_ALIGNED(8);
		unsigned int extramask[NSIG_BYTES / sizeof(int) - 1];
	} frame;

	if (!umove_or_printaddr(tcp, addr, &frame)) {
		unsigned int mask[NSIG_BYTES / sizeof(int)];

		mask[0] = frame.mask;
		memcpy(mask + 1, frame.extramask, sizeof(frame.extramask));
		tprintsigmask_addr(mask);
	}
}

#undef PERSONALITY_WORDSIZE
#undef SIZEOF_STRUCT_PT_REGS
#undef SIZEOF_STRUCT_SPARC_STACKF
