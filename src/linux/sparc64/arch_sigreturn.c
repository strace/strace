/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_sigreturn	sparc64_arch_sigreturn
#include "../sparc/arch_sigreturn.c"
#undef arch_sigreturn

#define SIZEOF_STRUCT_SPARC_STACKF	sizeof(struct sparc_stackf32)
#define SIZEOF_STRUCT_PT_REGS		sizeof(struct pt_regs32)
#define PERSONALITY_WORDSIZE		PERSONALITY1_WORDSIZE
#define arch_sigreturn	sparc32_arch_sigreturn
#include "../sparc/arch_sigreturn.c"
#undef arch_sigreturn

static void
arch_sigreturn(struct tcb *tcp)
{
	if (current_personality == 1)
		sparc32_arch_sigreturn(tcp);
	else
		sparc64_arch_sigreturn(tcp);
}
