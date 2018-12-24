/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
arch_sigreturn(struct tcb *tcp)
{
	/*
	 * On i386, sigcontext is followed on stack by struct fpstate
	 * and after it an additional u32 extramask which holds
	 * upper half of the mask.
	 */
	struct {
		uint32_t struct_sigcontext_padding1[20];
		uint32_t oldmask;
		uint32_t struct_sigcontext_padding2;
		uint32_t struct_fpstate_padding[156];
		uint32_t extramask;
	} frame;
	kernel_ulong_t sp;

	if (get_stack_pointer(tcp, &sp) &&
	    !umove_or_printaddr(tcp, sp, &frame)) {
		uint32_t mask[2] = { frame.oldmask, frame.extramask };
		tprintsigmask_addr("{mask=", mask);
		tprints("}");
	}
}
